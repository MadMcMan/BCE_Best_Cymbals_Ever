#include "AirwindowsBrightness.hpp"
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
    AudioEffectX* aw_create_Air();
    AudioEffectX* aw_create_Apicolypse();
    AudioEffectX* aw_create_Capacitor();
    AudioEffectX* aw_create_Capacitor2();
    AudioEffectX* aw_create_Crystal();
    AudioEffectX* aw_create_PurestAir();
}

class AirSuiteBrightness::Impl {
public:
    Impl(float sr) : m_adapter(sr, awspec::createFor_AirSuiteBrightness, true) { m_adapter.setSampleRate(sr); }
    void setSampleRate(float sr) { m_adapter.setSampleRate(sr); }
    void setAlgorithm(AirSuiteBrightness::Type type) { m_adapter.setType((int)type); }
    void reset() { m_adapter.reset(); }
    void process(float& L, float& R, AirSuiteBrightness::Type type, const float* params) {
        m_adapter.setType((int)type);
        m_adapter.process(L, R, params, AirSuiteBrightness::getBrightnessSpec(type).count);
    }
private:
    AirwindowsAdapter m_adapter;
};

AirSuiteBrightness::AirSuiteBrightness(float sampleRate)
    : m_impl(std::make_unique<Impl>(sampleRate)) {}

AirSuiteBrightness::~AirSuiteBrightness() = default;

void AirSuiteBrightness::setSampleRate(float sr) {
    if (m_impl) m_impl->setSampleRate(sr);
}

void AirSuiteBrightness::reset() {
    if (m_impl) m_impl->reset();
}

void AirSuiteBrightness::process(float& outL, float& outR, Type type, const float* params) {
    if (m_impl) {
        // For EXCITER, silently pass through without processing for now
        // These are not in the factory functions list
        if (type == EXCITER) {
            return;
        }
        m_impl->process(outL, outR, type, params);
    }
}

END_NAMESPACE_DISTRHO

AirwindowsSpec AirSuiteBrightness::getBrightnessSpec(AirSuiteBrightness::Type type) {
    return awspec::get_AirSuiteBrightness((int)type);
}
