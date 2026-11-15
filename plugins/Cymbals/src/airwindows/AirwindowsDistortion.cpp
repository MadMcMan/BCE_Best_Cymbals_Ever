#include "AirwindowsDistortion.hpp"
#include "AirwindowsAdapter.hpp"
#include "AirwindowsFactories.inc"
#include "AirSuiteUniversal.hpp"
#include "AirwindowsSpecTable.inc"
#include "vst2stub/audioeffectx.h"
#include <cstring>
#include <cmath>

START_NAMESPACE_DISTRHO

extern "C" {
    AudioEffectX* aw_create_BassDrive();
    AudioEffectX* aw_create_BitGlitter();
    AudioEffectX* aw_create_Density();
    AudioEffectX* aw_create_Density2();
    AudioEffectX* aw_create_Density3();
    AudioEffectX* aw_create_Distortion();
    AudioEffectX* aw_create_Drive();
    AudioEffectX* aw_create_HardVacuum();
    AudioEffectX* aw_create_PurestDrive();
    AudioEffectX* aw_create_Spiral();
    AudioEffectX* aw_create_Spiral2();
AudioEffectX* aw_create_Spiral3();
    AudioEffectX* aw_create_Bite();
    AudioEffectX* aw_create_Cojones();
    AudioEffectX* aw_create_Creature();
    AudioEffectX* aw_create_Deckwrecka();
    AudioEffectX* aw_create_Dirt();
    AudioEffectX* aw_create_Donut();
    AudioEffectX* aw_create_Edge();
    AudioEffectX* aw_create_Golem();
    AudioEffectX* aw_create_HipCrush();
    AudioEffectX* aw_create_Hype();
    AudioEffectX* aw_create_Inflamer();
    AudioEffectX* aw_create_MultiBandDistortion();
    AudioEffectX* aw_create_Pop();
    AudioEffectX* aw_create_Pop2();
    AudioEffectX* aw_create_Pop3();
    AudioEffectX* aw_create_Shape();
    AudioEffectX* aw_create_Stonefire();
}

AirwindowsSpec AirSuiteDistortion::getDistortionSpec(AirSuiteDistortion::Type type) {
        return awspec::get_AirSuiteDistortion((int)type);
    }

class AirSuiteDistortion::Impl {
public:
    Impl(float sr) : m_adapter(sr, awspec::createFor_AirSuiteDistortion, false) { m_adapter.setSampleRate(sr); }
    void setSampleRate(float sr) { m_adapter.setSampleRate(sr); }
    void setAlgorithm(AirSuiteDistortion::Type type) { m_adapter.setType((int)type); }
    void reset() { m_adapter.reset(); }
    void process(float& L, float& R, AirSuiteDistortion::Type type, float* params) {
        m_adapter.setType((int)type);
        m_adapter.process(L, R, params, AirSuiteDistortion::getDistortionSpec(type).count);
    }
private:
    AirwindowsAdapter m_adapter;
};

AirSuiteDistortion::AirSuiteDistortion(float sampleRate)
    : sampleRate(sampleRate), m_impl(std::make_unique<Impl>(sampleRate)) {}
    
AirSuiteDistortion::~AirSuiteDistortion() = default;

void AirSuiteDistortion::setSampleRate(float sr) {
    sampleRate = sr;
    if (m_impl) m_impl->setSampleRate(sr);
}

void AirSuiteDistortion::reset() {
    if (m_impl) m_impl->reset();
}

void AirSuiteDistortion::process(float& L, float& R, Type type, float* params) {
    if (m_impl) m_impl->process(L, R, type, params);
}

END_NAMESPACE_DISTRHO
