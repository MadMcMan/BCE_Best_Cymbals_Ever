#include "AirwindowsAmpSims.hpp"
#include "AirSuiteUniversal.hpp"
#include "AirwindowsSpecTable.inc"
#include "vst2stub/audioeffectx.h"
#include <cstring>
#include <cmath>

START_NAMESPACE_DISTRHO

extern "C" {
    AudioEffectX* aw_create_Cabs();
    AudioEffectX* aw_create_BassAmp();
    AudioEffectX* aw_create_BigAmp();
    AudioEffectX* aw_create_FireAmp();
    AudioEffectX* aw_create_GrindAmp();
    AudioEffectX* aw_create_PurestSquish();
    AudioEffectX* aw_create_ToneSlant();
    AudioEffectX* aw_create_LeadAmp();
    AudioEffectX* aw_create_LilAmp();
    AudioEffectX* aw_create_Cabs2();
    AudioEffectX* aw_create_ChimeyDeluxe();
    AudioEffectX* aw_create_ChimeyGuitar();
    AudioEffectX* aw_create_ChimeyGuitar2();
    AudioEffectX* aw_create_CrickBass();
    AudioEffectX* aw_create_MidAmp();
    AudioEffectX* aw_create_PointyDeluxe();
    AudioEffectX* aw_create_PointyGuitar();
    AudioEffectX* aw_create_PunchyDeluxe();
    AudioEffectX* aw_create_PunchyGuitar();
    AudioEffectX* aw_create_SingleEndedTriode();
    AudioEffectX* aw_create_Tube();
    AudioEffectX* aw_create_Tube2();
}

AirwindowsSpec AirSuiteAmpSim::getAmpSimSpec(AirSuiteAmpSim::Type type) {
        return awspec::get_AirSuiteAmpSim((int)type);
    }

class AirSuiteAmpSim::Impl {
public:
    Impl(float sr) : m_sampleRate(sr),
        m_cabs(aw_create_Cabs()),
        m_bassAmp(aw_create_BassAmp()),
        m_bigAmp(aw_create_BigAmp()),
        m_fireAmp(aw_create_FireAmp()),
        m_grindAmp(aw_create_GrindAmp()),
        m_purestSquish(aw_create_PurestSquish()),
        m_toneSlant(aw_create_ToneSlant()),
        m_leadAmp(aw_create_LeadAmp()),
        m_lilAmp(aw_create_LilAmp()),
        m_cabs2(aw_create_Cabs2()),
        m_chimeyDeluxe(aw_create_ChimeyDeluxe()),
        m_chimeyGuitar(aw_create_ChimeyGuitar()),
        m_chimeyGuitar2(aw_create_ChimeyGuitar2()),
        m_crickBass(aw_create_CrickBass()),
        m_midAmp(aw_create_MidAmp()),
        m_pointyDeluxe(aw_create_PointyDeluxe()),
        m_pointyGuitar(aw_create_PointyGuitar()),
        m_punchyDeluxe(aw_create_PunchyDeluxe()),
        m_punchyGuitar(aw_create_PunchyGuitar()),
        m_singleEndedTriode(aw_create_SingleEndedTriode()),
        m_tube(aw_create_Tube()),
        m_tube2(aw_create_Tube2())
    {
        setSampleRate(sr);
    }

    ~Impl() {
        delete m_cabs;
        delete m_bassAmp;
        delete m_bigAmp;
        delete m_fireAmp;
        delete m_grindAmp;
        delete m_purestSquish;
        delete m_toneSlant;
        delete m_leadAmp;
        delete m_lilAmp;
        delete m_cabs2;
        delete m_chimeyDeluxe;
        delete m_chimeyGuitar;
        delete m_chimeyGuitar2;
        delete m_crickBass;
        delete m_midAmp;
        delete m_pointyDeluxe;
        delete m_pointyGuitar;
        delete m_punchyDeluxe;
        delete m_punchyGuitar;
        delete m_singleEndedTriode;
        delete m_tube;
        delete m_tube2;
    }

    void setSampleRate(float sr) {
        m_sampleRate = sr;
        if (m_cabs) m_cabs->setSampleRate(sr);
        if (m_bassAmp) m_bassAmp->setSampleRate(sr);
        if (m_bigAmp) m_bigAmp->setSampleRate(sr);
        if (m_fireAmp) m_fireAmp->setSampleRate(sr);
        if (m_grindAmp) m_grindAmp->setSampleRate(sr);
        if (m_purestSquish) m_purestSquish->setSampleRate(sr);
        if (m_toneSlant) m_toneSlant->setSampleRate(sr);
        if (m_leadAmp) m_leadAmp->setSampleRate(sr);
        if (m_lilAmp) m_lilAmp->setSampleRate(sr);
        if (m_cabs2) m_cabs2->setSampleRate(sr);
        if (m_chimeyDeluxe) m_chimeyDeluxe->setSampleRate(sr);
        if (m_chimeyGuitar) m_chimeyGuitar->setSampleRate(sr);
        if (m_chimeyGuitar2) m_chimeyGuitar2->setSampleRate(sr);
        if (m_crickBass) m_crickBass->setSampleRate(sr);
        if (m_midAmp) m_midAmp->setSampleRate(sr);
        if (m_pointyDeluxe) m_pointyDeluxe->setSampleRate(sr);
        if (m_pointyGuitar) m_pointyGuitar->setSampleRate(sr);
        if (m_punchyDeluxe) m_punchyDeluxe->setSampleRate(sr);
        if (m_punchyGuitar) m_punchyGuitar->setSampleRate(sr);
        if (m_singleEndedTriode) m_singleEndedTriode->setSampleRate(sr);
        if (m_tube) m_tube->setSampleRate(sr);
        if (m_tube2) m_tube2->setSampleRate(sr);
    }

    void reset() {
        // Only the sub-effect that actually ran can hold history. The Impl
        // eagerly owns 22 instances but process() dispatches to exactly one,
        // so rebuilding all of them churned ~88 allocations on the audio
        // thread for the 21 that were never touched. Reconstruct only the
        // one that processed audio, and do nothing at all if none ever has
        // (a freshly selected AmpSim type is already pristine).
        switch (m_lastProcessedType) {
            case AirSuiteAmpSim::CABS: delete m_cabs; m_cabs = aw_create_Cabs(); break;
            case AirSuiteAmpSim::BASSAMP: delete m_bassAmp; m_bassAmp = aw_create_BassAmp(); break;
            case AirSuiteAmpSim::BIGAMP: delete m_bigAmp; m_bigAmp = aw_create_BigAmp(); break;
            case AirSuiteAmpSim::FIREAMP: delete m_fireAmp; m_fireAmp = aw_create_FireAmp(); break;
            case AirSuiteAmpSim::GRINDAMP: delete m_grindAmp; m_grindAmp = aw_create_GrindAmp(); break;
            case AirSuiteAmpSim::PURESTMULTISQUISH: delete m_purestSquish; m_purestSquish = aw_create_PurestSquish(); break;
            case AirSuiteAmpSim::TONESLANT: delete m_toneSlant; m_toneSlant = aw_create_ToneSlant(); break;
            case AirSuiteAmpSim::LEADAMP: delete m_leadAmp; m_leadAmp = aw_create_LeadAmp(); break;
            case AirSuiteAmpSim::LILAMPL: delete m_lilAmp; m_lilAmp = aw_create_LilAmp(); break;
            case AirSuiteAmpSim::AMPSIM_CABS2: delete m_cabs2; m_cabs2 = aw_create_Cabs2(); break;
            case AirSuiteAmpSim::AMPSIM_CHIMEY_DELUXE: delete m_chimeyDeluxe; m_chimeyDeluxe = aw_create_ChimeyDeluxe(); break;
            case AirSuiteAmpSim::AMPSIM_CHIMEY_GUITAR: delete m_chimeyGuitar; m_chimeyGuitar = aw_create_ChimeyGuitar(); break;
            case AirSuiteAmpSim::AMPSIM_CHIMEY_GUITAR2: delete m_chimeyGuitar2; m_chimeyGuitar2 = aw_create_ChimeyGuitar2(); break;
            case AirSuiteAmpSim::AMPSIM_CRICK_BASS: delete m_crickBass; m_crickBass = aw_create_CrickBass(); break;
            case AirSuiteAmpSim::AMPSIM_MID_AMP: delete m_midAmp; m_midAmp = aw_create_MidAmp(); break;
            case AirSuiteAmpSim::AMPSIM_POINTY_DELUXE: delete m_pointyDeluxe; m_pointyDeluxe = aw_create_PointyDeluxe(); break;
            case AirSuiteAmpSim::AMPSIM_POINTY_GUITAR: delete m_pointyGuitar; m_pointyGuitar = aw_create_PointyGuitar(); break;
            case AirSuiteAmpSim::AMPSIM_PUNCHY_DELUXE: delete m_punchyDeluxe; m_punchyDeluxe = aw_create_PunchyDeluxe(); break;
            case AirSuiteAmpSim::AMPSIM_PUNCHY_GUITAR: delete m_punchyGuitar; m_punchyGuitar = aw_create_PunchyGuitar(); break;
            case AirSuiteAmpSim::AMPSIM_SINGLE_ENDED_TRIODE: delete m_singleEndedTriode; m_singleEndedTriode = aw_create_SingleEndedTriode(); break;
            case AirSuiteAmpSim::AMPSIM_TUBE: delete m_tube; m_tube = aw_create_Tube(); break;
            case AirSuiteAmpSim::AMPSIM_TUBE2: delete m_tube2; m_tube2 = aw_create_Tube2(); break;
            default: return; // nothing ever processed: no history to clear
        }
        m_lastProcessedType = AirSuiteAmpSim::NONE;
        setSampleRate(m_sampleRate);
    }

    void process(float& L, float& R, AirSuiteAmpSim::Type type, float* params) {
        AudioEffectX* vst = nullptr;
        switch (type) {
            case AirSuiteAmpSim::CABS: vst = m_cabs; break;
            case AirSuiteAmpSim::BASSAMP: vst = m_bassAmp; break;
            case AirSuiteAmpSim::BIGAMP: vst = m_bigAmp; break;
            case AirSuiteAmpSim::FIREAMP: vst = m_fireAmp; break;
            case AirSuiteAmpSim::GRINDAMP: vst = m_grindAmp; break;
            case AirSuiteAmpSim::PURESTMULTISQUISH: {
                const int passes = (int)(std::clamp(params[0], 0.0f, 1.0f) * 7.0f) + 1;
                float inputL = L;
                float inputR = R;
                for (int p = 0; p < passes; ++p) {
                    inputL = inputL / (1.0f + std::abs(inputL));
                    inputR = inputR / (1.0f + std::abs(inputR));
                }
                L = inputL;
                R = inputR;
                return;
            }
            case AirSuiteAmpSim::TONESLANT: vst = m_toneSlant; break;
            case AirSuiteAmpSim::LEADAMP: vst = m_leadAmp; break;
            case AirSuiteAmpSim::LILAMPL: vst = m_lilAmp; break;
            case AirSuiteAmpSim::GUITARCONDITIONER:
                // GUITARCONDITIONER is intentionally not implemented in Phase 1.
                // The WinVST plugin is available but not wrapped as an active DSP instance.
                return;
            case AirSuiteAmpSim::AMPSIM_CABS2: vst = m_cabs2; break;
            case AirSuiteAmpSim::AMPSIM_CHIMEY_DELUXE: vst = m_chimeyDeluxe; break;
            case AirSuiteAmpSim::AMPSIM_CHIMEY_GUITAR: vst = m_chimeyGuitar; break;
            case AirSuiteAmpSim::AMPSIM_CHIMEY_GUITAR2: vst = m_chimeyGuitar2; break;
            case AirSuiteAmpSim::AMPSIM_CRICK_BASS: vst = m_crickBass; break;
            case AirSuiteAmpSim::AMPSIM_MID_AMP: vst = m_midAmp; break;
            case AirSuiteAmpSim::AMPSIM_POINTY_DELUXE: vst = m_pointyDeluxe; break;
            case AirSuiteAmpSim::AMPSIM_POINTY_GUITAR: vst = m_pointyGuitar; break;
            case AirSuiteAmpSim::AMPSIM_PUNCHY_DELUXE: vst = m_punchyDeluxe; break;
            case AirSuiteAmpSim::AMPSIM_PUNCHY_GUITAR: vst = m_punchyGuitar; break;
            case AirSuiteAmpSim::AMPSIM_SINGLE_ENDED_TRIODE: vst = m_singleEndedTriode; break;
            case AirSuiteAmpSim::AMPSIM_TUBE: vst = m_tube; break;
            case AirSuiteAmpSim::AMPSIM_TUBE2: vst = m_tube2; break;
            default: return;
        }
        if (!vst) return;
        m_lastProcessedType = type; // only this effect can hold history; see reset()
        auto spec = AirSuiteAmpSim::getAmpSimSpec(type);
        runPlugin(vst, L, R, params, spec.count);
    }

private:
    float m_sampleRate;
    // The sub-effect most recently fed by runPlugin(). NONE until a sample has
    // actually been processed, so reset() can skip a pristine bank entirely.
    AirSuiteAmpSim::Type m_lastProcessedType = AirSuiteAmpSim::NONE;
    AudioEffectX* m_cabs;
    AudioEffectX* m_bassAmp;
    AudioEffectX* m_bigAmp;
    AudioEffectX* m_fireAmp;
    AudioEffectX* m_grindAmp;
    AudioEffectX* m_purestSquish;
    AudioEffectX* m_toneSlant;
    AudioEffectX* m_leadAmp;
    AudioEffectX* m_lilAmp;
    AudioEffectX* m_cabs2;
    AudioEffectX* m_chimeyDeluxe;
    AudioEffectX* m_chimeyGuitar;
    AudioEffectX* m_chimeyGuitar2;
    AudioEffectX* m_crickBass;
    AudioEffectX* m_midAmp;
    AudioEffectX* m_pointyDeluxe;
    AudioEffectX* m_pointyGuitar;
    AudioEffectX* m_punchyDeluxe;
    AudioEffectX* m_punchyGuitar;
    AudioEffectX* m_singleEndedTriode;
    AudioEffectX* m_tube;
    AudioEffectX* m_tube2;

    void runPlugin(AudioEffectX* vst, float& L, float& R, const float* params, int paramCount) {
        // Clamp parameters to valid range [0.0, 1.0] before setParameter
        for (int i = 0; i < paramCount && i < 13; ++i) {
            float clampedValue = params[i];
            if (clampedValue < 0.0f) clampedValue = 0.0f;
            else if (clampedValue > 1.0f) clampedValue = 1.0f;
            vst->setParameter(i, clampedValue);
        }
        float inL = L, inR = R;
        float outL = 0.0f, outR = 0.0f;
        float* inputs[2] = {&inL, &inR};
        float* outputs[2] = {&outL, &outR};
        vst->processReplacing(inputs, outputs, 1);
        // Guard against NaN/Inf output
        if (!std::isfinite(outL)) outL = 0.0f;
        if (!std::isfinite(outR)) outR = 0.0f;
        L = outL;
        R = outR;
    }
};

AirSuiteAmpSim::AirSuiteAmpSim(float sampleRate)
    : sampleRate(sampleRate), m_impl(std::make_unique<Impl>(sampleRate)) {}
    
AirSuiteAmpSim::~AirSuiteAmpSim() = default;

void AirSuiteAmpSim::setSampleRate(float sr) {
    sampleRate = sr;
    if (m_impl) m_impl->setSampleRate(sr);
}

void AirSuiteAmpSim::reset() {
    if (m_impl) m_impl->reset();
}

void AirSuiteAmpSim::process(float& L, float& R, Type type, float* params) {
    if (m_impl) m_impl->process(L, R, type, params);
}

END_NAMESPACE_DISTRHO
