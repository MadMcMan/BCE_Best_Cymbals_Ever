#include "AirwindowsBiquad.hpp"
#include "AirwindowsAdapter.hpp"
#include "AirwindowsFactories.inc"
#include "AirSuiteUniversal.hpp"
#include "AirwindowsSpecTable.inc"
#include "vst2stub/audioeffectx.h"
#include <cstring>
#include <algorithm>
#include <cmath>

START_NAMESPACE_DISTRHO

// Factory function declarations (implemented in winvst/<Name>_winvst.cpp)
extern "C" {
    AudioEffectX* aw_create_Biquad();
    AudioEffectX* aw_create_Biquad2();
    AudioEffectX* aw_create_BiquadHiLo();
    AudioEffectX* aw_create_BiquadNonLin();
    AudioEffectX* aw_create_BiquadOneHalf();
    AudioEffectX* aw_create_BiquadPlus();
    AudioEffectX* aw_create_BiquadStack();
    AudioEffectX* aw_create_BiquadTriple();
}

// Pimpl implementation class
class AirSuiteBiquad::Impl {
public:
    Impl(float sr) : m_adapter(sr, awspec::createFor_AirSuiteBiquad, true) { m_adapter.setSampleRate(sr); }
    void setSampleRate(float sr) { m_adapter.setSampleRate(sr); }
    void setAlgorithm(AirSuiteBiquad::Type type) { m_adapter.setType((int)type); }
    void reset() { m_adapter.reset(); }
    void process(float& L, float& R, AirSuiteBiquad::Type type, float* params) {
        m_adapter.setType((int)type);
        m_adapter.process(L, R, params, AirSuiteBiquad::getBiquadSpec(type).count);
    }
private:
    AirwindowsAdapter m_adapter;
};

// Public AirSuiteBiquad delegation
AirSuiteBiquad::AirSuiteBiquad(float sampleRate)
    : sampleRate(sampleRate), m_impl(std::make_unique<Impl>(sampleRate)) {}
    
AirSuiteBiquad::~AirSuiteBiquad() = default;

void AirSuiteBiquad::setSampleRate(float sr) {
    sampleRate = sr;
    if (m_impl) m_impl->setSampleRate(sr);
}

void AirSuiteBiquad::reset() {
    if (m_impl) m_impl->reset();
}

void AirSuiteBiquad::process(float& L, float& R, Type type, float* params) {
    if (m_impl) m_impl->process(L, R, type, params);
}

END_NAMESPACE_DISTRHO

AirwindowsSpec AirSuiteBiquad::getBiquadSpec(AirSuiteBiquad::Type type) {
    return awspec::get_AirSuiteBiquad((int)type);
}
