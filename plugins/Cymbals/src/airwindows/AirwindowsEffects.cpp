#include "AirwindowsEffects.hpp"
#include "AirwindowsAdapter.hpp"
#include "AirwindowsFactories.inc"
#include "AirSuiteUniversal.hpp"
#include "AirwindowsSpecTable.inc"
#include "vst2stub/audioeffectx.h"
#include <cstring>
#include <cmath>

START_NAMESPACE_DISTRHO

// Factory function declarations (implemented in winvst/<Name>_winvst.cpp)
extern "C" {
    AudioEffectX* aw_create_Aura();
    AudioEffectX* aw_create_CloudCoat();
    AudioEffectX* aw_create_Disintegrate();
    AudioEffectX* aw_create_Energy();
    AudioEffectX* aw_create_Energy2();
    AudioEffectX* aw_create_Exciter();
    AudioEffectX* aw_create_Facet();
    AudioEffectX* aw_create_Fracture();
    AudioEffectX* aw_create_Fracture2();
    AudioEffectX* aw_create_GalacticVibe();
    AudioEffectX* aw_create_GlitchShifter();
    AudioEffectX* aw_create_Gringer();
    AudioEffectX* aw_create_GuitarConditioner();
    AudioEffectX* aw_create_Pafnuty2();
    AudioEffectX* aw_create_PitchNasty();
    AudioEffectX* aw_create_PowerSag2();
    AudioEffectX* aw_create_Preponderant();
    AudioEffectX* aw_create_RingModulator();
    AudioEffectX* aw_create_TakeCare();
    AudioEffectX* aw_create_Tremolo();
    AudioEffectX* aw_create_TremoSquare();
    AudioEffectX* aw_create_Trianglizer();
    AudioEffectX* aw_create_ElectroHat();
    AudioEffectX* aw_create_Flipity();
    AudioEffectX* aw_create_Monitoring();
    AudioEffectX* aw_create_MoNoam();
    AudioEffectX* aw_create_ADT();
    AudioEffectX* aw_create_AutoPan();
    AudioEffectX* aw_create_Beam();
    AudioEffectX* aw_create_BitDualPan();
    AudioEffectX* aw_create_BitShiftPan();
    AudioEffectX* aw_create_Cans();
    AudioEffectX* aw_create_CansAW();
    AudioEffectX* aw_create_Chorus();
    AudioEffectX* aw_create_ChorusEnsemble();
    AudioEffectX* aw_create_ContentHideD();
    AudioEffectX* aw_create_curve();
    AudioEffectX* aw_create_DCVoltage();
    AudioEffectX* aw_create_DeCrackle();
    AudioEffectX* aw_create_DeRez();
    AudioEffectX* aw_create_DeRez2();
    AudioEffectX* aw_create_DeRez3();
    AudioEffectX* aw_create_DeRez4();
    AudioEffectX* aw_create_Discontinuity();
    AudioEffectX* aw_create_Distance();
    AudioEffectX* aw_create_Distance2();
    AudioEffectX* aw_create_Distance3();
    AudioEffectX* aw_create_Ditherbox();
    AudioEffectX* aw_create_Doublelay();
    AudioEffectX* aw_create_DoublePaul();
    AudioEffectX* aw_create_DubCenter();
    AudioEffectX* aw_create_DubSub2();
    AudioEffectX* aw_create_DustBunny();
    AudioEffectX* aw_create_EdIsDim();
    AudioEffectX* aw_create_Ensemble();
    AudioEffectX* aw_create_EverySlew();
    AudioEffectX* aw_create_EveryTrim();
    AudioEffectX* aw_create_GoldenSlew();
    AudioEffectX* aw_create_Hombre();
    AudioEffectX* aw_create_LeftoMono();
    AudioEffectX* aw_create_LRFlipTimer();
    AudioEffectX* aw_create_Monitoring2();
    AudioEffectX* aw_create_Monitoring3();
    AudioEffectX* aw_create_MSFlipTimer();
    AudioEffectX* aw_create_NotJustAnotherCD();
    AudioEffectX* aw_create_OrbitKick();
    AudioEffectX* aw_create_Overheads();
    AudioEffectX* aw_create_PaulWide();
    AudioEffectX* aw_create_PitchDelay();
    AudioEffectX* aw_create_Pockey();
    AudioEffectX* aw_create_Pockey2();
    AudioEffectX* aw_create_PurestDualPan();
    AudioEffectX* aw_create_PurestEcho();
    AudioEffectX* aw_create_RawGlitters();
    AudioEffectX* aw_create_RawTimbers();
    AudioEffectX* aw_create_Recurve();
    AudioEffectX* aw_create_Remap();
    AudioEffectX* aw_create_RightoMono();
    AudioEffectX* aw_create_Silhouette();
    AudioEffectX* aw_create_Slew3();
    AudioEffectX* aw_create_Slew4();
    AudioEffectX* aw_create_SoftClock();
    AudioEffectX* aw_create_SoftClock2();
    AudioEffectX* aw_create_SoftClock3();
    AudioEffectX* aw_create_Srsly2();
    AudioEffectX* aw_create_Srsly3();
    AudioEffectX* aw_create_StereoChorus();
    AudioEffectX* aw_create_StereoDoubler();
    AudioEffectX* aw_create_StereoEnsemble();
    AudioEffectX* aw_create_StereoFX();
    AudioEffectX* aw_create_SweetWide();
    AudioEffectX* aw_create_TapeDelay();
    AudioEffectX* aw_create_TapeDelay2();
    AudioEffectX* aw_create_Texturize();
    AudioEffectX* aw_create_TexturizeMS();
    AudioEffectX* aw_create_TPDFWide();
    AudioEffectX* aw_create_TripleSpread();
    AudioEffectX* aw_create_uLawDecode();
    AudioEffectX* aw_create_uLawEncode();
    AudioEffectX* aw_create_Vibrato();
    AudioEffectX* aw_create_VoiceTrick();
    AudioEffectX* aw_create_Wolfbot();
AudioEffectX* aw_create_Weave();
    AudioEffectX* aw_create_Zoom();
    AudioEffectX* aw_create_Echojam();
}

// Static method implementation for spec
AirwindowsSpec AirSuiteEffect::getEffectSpec(AirSuiteEffect::Type type) {
        return awspec::get_AirSuiteEffect((int)type);
    }

// Pimpl implementation class
class AirSuiteEffect::Impl {
public:
    Impl(float sr) : m_adapter(sr, awspec::createFor_AirSuiteEffect, false) { m_adapter.setSampleRate(sr); }
    void setSampleRate(float sr) { m_adapter.setSampleRate(sr); }
    void setAlgorithm(AirSuiteEffect::Type type) { m_adapter.setType((int)type); }
    void reset() { m_adapter.reset(); }
    void process(float& L, float& R, AirSuiteEffect::Type type, float* params) {
        m_adapter.setType((int)type);
        m_adapter.process(L, R, params, AirSuiteEffect::getEffectSpec(type).count);
    }
private:
    AirwindowsAdapter m_adapter;
};

// Public AirSuiteEffect delegation
AirSuiteEffect::AirSuiteEffect(float sampleRate)
    : sampleRate(sampleRate), m_impl(std::make_unique<Impl>(sampleRate)) {}
    
AirSuiteEffect::~AirSuiteEffect() = default;

void AirSuiteEffect::setSampleRate(float sr) {
    sampleRate = sr;
    if (m_impl) m_impl->setSampleRate(sr);
}

void AirSuiteEffect::reset() {
    if (m_impl) m_impl->reset();
}

void AirSuiteEffect::process(float& L, float& R, Type type, float* params) {
    if (m_impl) m_impl->process(L, R, type, params);
}

END_NAMESPACE_DISTRHO
