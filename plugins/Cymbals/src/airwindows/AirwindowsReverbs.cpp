#include "AirwindowsReverbs.hpp"
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
    AudioEffectX* aw_create_BrightAmbience();
    AudioEffectX* aw_create_BrightAmbience2();
    AudioEffectX* aw_create_BrightAmbience3();
    AudioEffectX* aw_create_Chamber();
    AudioEffectX* aw_create_Chamber2();
    AudioEffectX* aw_create_ClearCoat();
    AudioEffectX* aw_create_CreamCoat();
    AudioEffectX* aw_create_CrunchCoat();
    AudioEffectX* aw_create_Galactic();
    AudioEffectX* aw_create_Galactic2();
    AudioEffectX* aw_create_Galactic3();
    AudioEffectX* aw_create_Infinity();
    AudioEffectX* aw_create_Infinity2();
    AudioEffectX* aw_create_KAlienSpaceship();
    AudioEffectX* aw_create_KBeyond();
    AudioEffectX* aw_create_KCathedral();
    AudioEffectX* aw_create_KCathedral2();
    AudioEffectX* aw_create_KCathedral3();
    AudioEffectX* aw_create_KCathedral4();
    AudioEffectX* aw_create_KCathedral5();
    AudioEffectX* aw_create_KChamberAR();
    AudioEffectX* aw_create_KCosmos();
    AudioEffectX* aw_create_KGuitarHall();
    AudioEffectX* aw_create_KGuitarHall2();
    AudioEffectX* aw_create_KPlate140();
    AudioEffectX* aw_create_KPlate240();
    AudioEffectX* aw_create_KPlateA();
    AudioEffectX* aw_create_KPlateB();
    AudioEffectX* aw_create_KPlateC();
    AudioEffectX* aw_create_KPlateD();
    AudioEffectX* aw_create_KStation();
    AudioEffectX* aw_create_KWoodRoom();
    AudioEffectX* aw_create_MatrixVerb();
    AudioEffectX* aw_create_MV();
    AudioEffectX* aw_create_MV2();
    AudioEffectX* aw_create_NonlinearSpace();
    AudioEffectX* aw_create_PocketVerbs();
    AudioEffectX* aw_create_Reverb();
    AudioEffectX* aw_create_SubTight();
    AudioEffectX* aw_create_Verbity();
    AudioEffectX* aw_create_Verbity2();
    AudioEffectX* aw_create_VerbSixes();
    AudioEffectX* aw_create_VerbThic();
    AudioEffectX* aw_create_VerbTiny();
    AudioEffectX* aw_create_VerbSevens();
    AudioEffectX* aw_create_WoodenChest();
    AudioEffectX* aw_create_Dattorro();
    AudioEffectX* aw_create_LRConvolve();
    AudioEffectX* aw_create_LRConvolve2();
    AudioEffectX* aw_create_LRConvolve3();
    AudioEffectX* aw_create_Neverland();
    AudioEffectX* aw_create_StarChild();
    AudioEffectX* aw_create_StarChild2();
    AudioEffectX* aw_create_Thunder();
    AudioEffectX* aw_create_VoiceOfTheStarship();
    AudioEffectX* aw_create_WoodenBox();
    AudioEffectX* aw_create_KCyberCity();
    AudioEffectX* aw_create_KRockstar();
}

// Pimpl implementation class
class AirSuiteReverb::Impl {
public:
    Impl(float sr) : m_adapter(sr, awspec::createFor_AirSuiteReverb, false) { m_adapter.setSampleRate(sr); }
    void setSampleRate(float sr) { m_adapter.setSampleRate(sr); }
    void setAlgorithm(AirSuiteReverb::Type type) { m_adapter.setType((int)type); }
    void reset() { m_adapter.reset(); }
    void process(float& L, float& R, AirSuiteReverb::Type type, float* params) {
        m_adapter.setType((int)type);
        m_adapter.process(L, R, params, AirSuiteReverb::getReverbSpec(type).count);
    }
private:
    AirwindowsAdapter m_adapter;
};

// Public AirSuiteReverb delegation
AirSuiteReverb::AirSuiteReverb(float sampleRate)
    : sampleRate(sampleRate), m_impl(std::make_unique<Impl>(sampleRate)) {}
    
AirSuiteReverb::~AirSuiteReverb() = default;

void AirSuiteReverb::setSampleRate(float sr) {
    sampleRate = sr;
    if (m_impl) m_impl->setSampleRate(sr);
}

void AirSuiteReverb::reset() {
    if (m_impl) m_impl->reset();
}

void AirSuiteReverb::process(float& L, float& R, Type type, float* params) {
    if (m_impl) m_impl->process(L, R, type, params);
}

END_NAMESPACE_DISTRHO

AirwindowsSpec AirSuiteReverb::getReverbSpec(AirSuiteReverb::Type type) {
    return awspec::get_AirSuiteReverb((int)type);
}
