#include "AirwindowsToneColour.hpp"
#include "AirwindowsAdapter.hpp"
#include "AirwindowsFactories.inc"
#include "AirSuiteUniversal.hpp"
#include "AirwindowsSpecTable.inc"
#include "vst2stub/audioeffectx.h"
#include <cstring>
#include <memory>
#include <cmath>

START_NAMESPACE_DISTRHO

extern "C" {
    AudioEffectX* aw_create_Focus(); AudioEffectX* aw_create_GrooveWear(); AudioEffectX* aw_create_Huge();
    AudioEffectX* aw_create_Melt(); AudioEffectX* aw_create_OneCornerClip(); AudioEffectX* aw_create_Pafnuty();
    AudioEffectX* aw_create_Pear(); AudioEffectX* aw_create_Pear2(); AudioEffectX* aw_create_Surge();
    AudioEffectX* aw_create_Sweeten();
}

class AirSuiteToneColour::Impl {
public:
    Impl(float sr) : m_adapter(sr, awspec::createFor_AirSuiteToneColour, true) { m_adapter.setSampleRate(sr); }
    void setSampleRate(float sr) { m_adapter.setSampleRate(sr); }
    void setAlgorithm(AirSuiteToneColour::Type type) { m_adapter.setType((int)type); }
    void reset() { m_adapter.reset(); }
    void process(float& L, float& R, AirSuiteToneColour::Type type, const float* params) {
        m_adapter.setType((int)type);
        m_adapter.process(L, R, params, AirSuiteToneColour::getToneColourSpec(type).count);
    }
private:
    AirwindowsAdapter m_adapter;
};

AirSuiteToneColour::AirSuiteToneColour(float sampleRate) : m_impl(std::make_unique<Impl>(sampleRate)) {}
AirSuiteToneColour::~AirSuiteToneColour() = default;
void AirSuiteToneColour::setSampleRate(float sr) { if (m_impl) m_impl->setSampleRate(sr); }
void AirSuiteToneColour::reset() { if (m_impl) m_impl->reset(); }
void AirSuiteToneColour::process(float& outL, float& outR, Type type, const float* params) {
    if (m_impl) m_impl->process(outL, outR, type, params);
}

END_NAMESPACE_DISTRHO

AirwindowsSpec AirSuiteToneColour::getToneColourSpec(AirSuiteToneColour::Type type) {
    return awspec::get_AirSuiteToneColour((int)type);
}
