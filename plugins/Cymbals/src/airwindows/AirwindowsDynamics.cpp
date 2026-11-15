#include "AirwindowsDynamics.hpp"
#include "AirSuiteUniversal.hpp"
#include "AirwindowsSpecTable.inc"
#include "vst2stub/audioeffectx.h"
#include <cstring>
#include <cmath>

START_NAMESPACE_DISTRHO

extern "C" {
    AudioEffectX* aw_create_Pressure4();
    AudioEffectX* aw_create_Pressure5();
    AudioEffectX* aw_create_VariMu();
    AudioEffectX* aw_create_Point();
    AudioEffectX* aw_create_Acceleration();
    AudioEffectX* aw_create_PurestGain();
    AudioEffectX* aw_create_Acceleration2();
    AudioEffectX* aw_create_AQuickVoiceClip();
    AudioEffectX* aw_create_BeziComp();
    AudioEffectX* aw_create_BlockParty();
    AudioEffectX* aw_create_BrassRider();
    AudioEffectX* aw_create_ButterComp();
    AudioEffectX* aw_create_ButterComp2();
    AudioEffectX* aw_create_ClipOnly3();
    AudioEffectX* aw_create_CStrip();
    AudioEffectX* aw_create_DeBess();
    AudioEffectX* aw_create_DeEss();
    AudioEffectX* aw_create_DeHiss();
    AudioEffectX* aw_create_DeNoise();
    AudioEffectX* aw_create_DrumSlam();
    AudioEffectX* aw_create_Dynamics();
    AudioEffectX* aw_create_Dynamics2();
    AudioEffectX* aw_create_Dynamics3();
    AudioEffectX* aw_create_Dyno();
    AudioEffectX* aw_create_FinalClip();
    AudioEffectX* aw_create_Floor();
    AudioEffectX* aw_create_Gatelope();
    AudioEffectX* aw_create_HighImpact();
    AudioEffectX* aw_create_Hypersoft();
    AudioEffectX* aw_create_Logical4();
    AudioEffectX* aw_create_Mastering();
    AudioEffectX* aw_create_Mastering2();
    AudioEffectX* aw_create_PeaksOnly();
    AudioEffectX* aw_create_Podcast();
    AudioEffectX* aw_create_PodcastDeluxe();
    AudioEffectX* aw_create_Pressure6();
    AudioEffectX* aw_create_Pyewacket();
    AudioEffectX* aw_create_SoftGate();
    AudioEffectX* aw_create_StoneFireComp();
    AudioEffectX* aw_create_Swell();
    AudioEffectX* aw_create_X2Buss();
    AudioEffectX* aw_create_ZOutputStage();
}

AirwindowsSpec AirSuiteDynamics::getDynamicsSpec(AirSuiteDynamics::Type type) {
        return awspec::get_AirSuiteDynamics((int)type);
    }

class AirSuiteDynamics::Impl {
public:
    Impl(float sr) : m_sampleRate(sr), m_currentType(AirSuiteDynamics::NONE),
        m_pressure4(aw_create_Pressure4()),
        m_pressure5(aw_create_Pressure5()),
        m_variMu(aw_create_VariMu()),
        m_point(aw_create_Point()),
        m_acceleration(aw_create_Acceleration()),
        m_purestGain(aw_create_PurestGain()),
        m_acceleration2(nullptr), m_aQuickVoiceClip(nullptr), m_beziComp(nullptr),
        m_blockParty(nullptr), m_brassRider(nullptr), m_butterComp(nullptr),
        m_butterComp2(nullptr), m_clipOnly3(nullptr), m_cStrip(nullptr),
        m_deBess(nullptr), m_deEss(nullptr), m_deHiss(nullptr), m_deNoise(nullptr),
        m_drumSlam(nullptr), m_dynamics(nullptr), m_dynamics2(nullptr),
        m_dynamics3(nullptr), m_dyno(nullptr), m_finalClip(nullptr), m_floor(nullptr),
        m_gatelope(nullptr), m_highImpact(nullptr), m_hypersoft(nullptr),
        m_logical4(nullptr), m_mastering(nullptr), m_mastering2(nullptr),
        m_peaksOnly(nullptr), m_podcast(nullptr), m_podcastDeluxe(nullptr),
        m_pressure6(nullptr), m_pyewacket(nullptr), m_softGate(nullptr),
        m_stoneFireComp(nullptr), m_swell(nullptr), m_x2Buss(nullptr),
        m_zOutputStage(nullptr)
    {
        setSampleRate(sr);
    }

    ~Impl() {
        delete m_pressure4;
        delete m_pressure5;
        delete m_variMu;
        delete m_point;
        delete m_acceleration;
        delete m_purestGain;
        delete m_acceleration2;
        delete m_aQuickVoiceClip;
        delete m_beziComp;
        delete m_blockParty;
        delete m_brassRider;
        delete m_butterComp;
        delete m_butterComp2;
        delete m_clipOnly3;
        delete m_cStrip;
        delete m_deBess;
        delete m_deEss;
        delete m_deHiss;
        delete m_deNoise;
        delete m_drumSlam;
        delete m_dynamics;
        delete m_dynamics2;
        delete m_dynamics3;
        delete m_dyno;
        delete m_finalClip;
        delete m_floor;
        delete m_gatelope;
        delete m_highImpact;
        delete m_hypersoft;
        delete m_logical4;
        delete m_mastering;
        delete m_mastering2;
        delete m_peaksOnly;
        delete m_podcast;
        delete m_podcastDeluxe;
        delete m_pressure6;
        delete m_pyewacket;
        delete m_softGate;
        delete m_stoneFireComp;
        delete m_swell;
        delete m_x2Buss;
        delete m_zOutputStage;
    }

    void setSampleRate(float sr) {
        m_sampleRate = sr;
        if (m_pressure4) m_pressure4->setSampleRate(sr);
        if (m_pressure5) m_pressure5->setSampleRate(sr);
        if (m_variMu) m_variMu->setSampleRate(sr);
        if (m_point) m_point->setSampleRate(sr);
        if (m_acceleration) m_acceleration->setSampleRate(sr);
        if (m_purestGain) m_purestGain->setSampleRate(sr);
    }

    void setAlgorithm(AirSuiteDynamics::Type type) {
        m_currentType = type;
        if (m_acceleration2) m_acceleration2->setSampleRate(m_sampleRate);
        if (m_aQuickVoiceClip) m_aQuickVoiceClip->setSampleRate(m_sampleRate);
        if (m_beziComp) m_beziComp->setSampleRate(m_sampleRate);
        if (m_blockParty) m_blockParty->setSampleRate(m_sampleRate);
        if (m_brassRider) m_brassRider->setSampleRate(m_sampleRate);
        if (m_butterComp) m_butterComp->setSampleRate(m_sampleRate);
        if (m_butterComp2) m_butterComp2->setSampleRate(m_sampleRate);
        if (m_clipOnly3) m_clipOnly3->setSampleRate(m_sampleRate);
        if (m_cStrip) m_cStrip->setSampleRate(m_sampleRate);
        if (m_deBess) m_deBess->setSampleRate(m_sampleRate);
        if (m_deEss) m_deEss->setSampleRate(m_sampleRate);
        if (m_deHiss) m_deHiss->setSampleRate(m_sampleRate);
        if (m_deNoise) m_deNoise->setSampleRate(m_sampleRate);
        if (m_drumSlam) m_drumSlam->setSampleRate(m_sampleRate);
        if (m_dynamics) m_dynamics->setSampleRate(m_sampleRate);
        if (m_dynamics2) m_dynamics2->setSampleRate(m_sampleRate);
        if (m_dynamics3) m_dynamics3->setSampleRate(m_sampleRate);
        if (m_dyno) m_dyno->setSampleRate(m_sampleRate);
        if (m_finalClip) m_finalClip->setSampleRate(m_sampleRate);
        if (m_floor) m_floor->setSampleRate(m_sampleRate);
        if (m_gatelope) m_gatelope->setSampleRate(m_sampleRate);
        if (m_highImpact) m_highImpact->setSampleRate(m_sampleRate);
        if (m_hypersoft) m_hypersoft->setSampleRate(m_sampleRate);
        if (m_logical4) m_logical4->setSampleRate(m_sampleRate);
        if (m_mastering) m_mastering->setSampleRate(m_sampleRate);
        if (m_mastering2) m_mastering2->setSampleRate(m_sampleRate);
        if (m_peaksOnly) m_peaksOnly->setSampleRate(m_sampleRate);
        if (m_podcast) m_podcast->setSampleRate(m_sampleRate);
        if (m_podcastDeluxe) m_podcastDeluxe->setSampleRate(m_sampleRate);
        if (m_pressure6) m_pressure6->setSampleRate(m_sampleRate);
        if (m_pyewacket) m_pyewacket->setSampleRate(m_sampleRate);
        if (m_softGate) m_softGate->setSampleRate(m_sampleRate);
        if (m_stoneFireComp) m_stoneFireComp->setSampleRate(m_sampleRate);
        if (m_swell) m_swell->setSampleRate(m_sampleRate);
        if (m_x2Buss) m_x2Buss->setSampleRate(m_sampleRate);
        if (m_zOutputStage) m_zOutputStage->setSampleRate(m_sampleRate);
    }

    void reset() {
        // Only the sub-effect that actually ran can hold history. Two things
        // make the old all-39 rebuild wrong: process() dispatches to just 6
        // types (the other 33 were constructed here but are unreachable), and
        // the one live effect is the only one carrying delay lines. Rebuild
        // that single effect, and nothing at all if none ever processed.
        switch (m_lastProcessedType) {
            case AirSuiteDynamics::PRESSURE4: delete m_pressure4; m_pressure4 = aw_create_Pressure4(); break;
            case AirSuiteDynamics::PRESSURE5: delete m_pressure5; m_pressure5 = aw_create_Pressure5(); break;
            case AirSuiteDynamics::VARIMU: delete m_variMu; m_variMu = aw_create_VariMu(); break;
            case AirSuiteDynamics::POINT: delete m_point; m_point = aw_create_Point(); break;
            case AirSuiteDynamics::ACCELERATION: delete m_acceleration; m_acceleration = aw_create_Acceleration(); break;
            case AirSuiteDynamics::PURESTGAIN: delete m_purestGain; m_purestGain = aw_create_PurestGain(); break;
            default: return; // nothing ever processed: no history to clear
        }
        m_lastProcessedType = AirSuiteDynamics::NONE;
        setSampleRate(m_sampleRate);
    }

    // KEY FIX: dispatch uses m_currentType set bysetAlgorithm(), NOT params[0]
    void process(float& L, float& R, float* params) {
        AudioEffectX* vst = nullptr;
        switch (m_currentType) {
            case AirSuiteDynamics::PRESSURE4: vst = m_pressure4; break;
            case AirSuiteDynamics::PRESSURE5: vst = m_pressure5; break;
            case AirSuiteDynamics::VARIMU: vst = m_variMu; break;
            case AirSuiteDynamics::POINT: vst = m_point; break;
            case AirSuiteDynamics::ACCELERATION: vst = m_acceleration; break;
            case AirSuiteDynamics::PURESTGAIN: vst = m_purestGain; break;
            default: return;
        }
        if (!vst) return;
        m_lastProcessedType = m_currentType; // only this effect can hold history; see reset()
        auto spec = AirSuiteDynamics::getDynamicsSpec(m_currentType);
        runPlugin(vst, L, R, params, spec.count);
    }

private:
    float m_sampleRate;
    // The sub-effect most recently fed by runPlugin(). NONE until a sample has
    // actually been processed, so reset() can skip a pristine bank entirely.
    AirSuiteDynamics::Type m_lastProcessedType = AirSuiteDynamics::NONE;
    AirSuiteDynamics::Type m_currentType;
    AudioEffectX* m_pressure4;
    AudioEffectX* m_pressure5;
    AudioEffectX* m_variMu;
    AudioEffectX* m_point;
    AudioEffectX* m_acceleration;
    AudioEffectX* m_purestGain;
    AudioEffectX* m_acceleration2;
    AudioEffectX* m_aQuickVoiceClip;
    AudioEffectX* m_beziComp;
    AudioEffectX* m_blockParty;
    AudioEffectX* m_brassRider;
    AudioEffectX* m_butterComp;
    AudioEffectX* m_butterComp2;
    AudioEffectX* m_clipOnly3;
    AudioEffectX* m_cStrip;
    AudioEffectX* m_deBess;
    AudioEffectX* m_deEss;
    AudioEffectX* m_deHiss;
    AudioEffectX* m_deNoise;
    AudioEffectX* m_drumSlam;
    AudioEffectX* m_dynamics;
    AudioEffectX* m_dynamics2;
    AudioEffectX* m_dynamics3;
    AudioEffectX* m_dyno;
    AudioEffectX* m_finalClip;
    AudioEffectX* m_floor;
    AudioEffectX* m_gatelope;
    AudioEffectX* m_highImpact;
    AudioEffectX* m_hypersoft;
    AudioEffectX* m_logical4;
    AudioEffectX* m_mastering;
    AudioEffectX* m_mastering2;
    AudioEffectX* m_peaksOnly;
    AudioEffectX* m_podcast;
    AudioEffectX* m_podcastDeluxe;
    AudioEffectX* m_pressure6;
    AudioEffectX* m_pyewacket;
    AudioEffectX* m_softGate;
    AudioEffectX* m_stoneFireComp;
    AudioEffectX* m_swell;
    AudioEffectX* m_x2Buss;
    AudioEffectX* m_zOutputStage;

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

AirSuiteDynamics::AirSuiteDynamics(float sampleRate)
    : sampleRate(sampleRate), m_impl(std::make_unique<Impl>(sampleRate)) {}
    
AirSuiteDynamics::~AirSuiteDynamics() = default;

void AirSuiteDynamics::setAlgorithm(int algo) {
    if (algo >= 0 && algo < (int)COUNT) {

        if (m_impl) m_impl->setAlgorithm((Type)algo);
    }
}

void AirSuiteDynamics::setSampleRate(float sr) {
    sampleRate = sr;
    if (m_impl) m_impl->setSampleRate(sr);
}

void AirSuiteDynamics::reset() {
    if (m_impl) m_impl->reset();
}

void AirSuiteDynamics::process(float& outL, float& outR, float* params) {
    if (m_impl) m_impl->process(outL, outR, params);
}

END_NAMESPACE_DISTRHO
