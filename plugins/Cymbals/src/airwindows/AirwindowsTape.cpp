#include "AirwindowsTape.hpp"
#include "AirwindowsAdapter.hpp"
#include "AirwindowsFactories.inc"
#include "AirSuiteUniversal.hpp"
#include "AirwindowsSpecTable.inc"
#include "vst2stub/audioeffectx.h"
#include <cstring>
#include <cmath>

START_NAMESPACE_DISTRHO

extern "C" {
    AudioEffectX* aw_create_ToTape5();
    AudioEffectX* aw_create_ToTape6();
    AudioEffectX* aw_create_ToVinyl4();
    AudioEffectX* aw_create_IronOxide5();
    AudioEffectX* aw_create_Flutter2();
    AudioEffectX* aw_create_Dubly();
    AudioEffectX* aw_create_Dubly2();
    AudioEffectX* aw_create_Dubly3();
    AudioEffectX* aw_create_ChromeOxide();
    AudioEffectX* aw_create_CrunchyGrooveWear();
    AudioEffectX* aw_create_Discontapeity();
    AudioEffectX* aw_create_DubPlate();
    AudioEffectX* aw_create_DubPlate2();
    AudioEffectX* aw_create_Flutter();
    AudioEffectX* aw_create_FromTape();
    AudioEffectX* aw_create_IronOxideClassic();
    AudioEffectX* aw_create_IronOxideClassic2();
    AudioEffectX* aw_create_Tape();
    AudioEffectX* aw_create_TapeBias();
    AudioEffectX* aw_create_TapeDust();
    AudioEffectX* aw_create_TapeFat();
    AudioEffectX* aw_create_TapeHack();
    AudioEffectX* aw_create_TapeHack2();
    AudioEffectX* aw_create_ToTape7();
    AudioEffectX* aw_create_ToTape8();
    AudioEffectX* aw_create_ToTape9();
}

AirwindowsSpec AirSuiteTape::getTapeSpec(AirSuiteTape::Type type) {
        return awspec::get_AirSuiteTape((int)type);
    }

class AirSuiteTape::Impl {
public:
    Impl(float sr) : m_adapter(sr, awspec::createFor_AirSuiteTape, false) { m_adapter.setSampleRate(sr); }
    void setSampleRate(float sr) { m_adapter.setSampleRate(sr); }
    void setAlgorithm(AirSuiteTape::Type type) { m_adapter.setType((int)type); }
    void reset() { m_adapter.reset(); }
    void process(float& L, float& R, AirSuiteTape::Type type, float* params) {
        m_adapter.setType((int)type);
        m_adapter.process(L, R, params, AirSuiteTape::getTapeSpec(type).count);
    }
private:
    AirwindowsAdapter m_adapter;
};

AirSuiteTape::AirSuiteTape(float sampleRate)
    : sampleRate(sampleRate), m_impl(std::make_unique<Impl>(sampleRate)) {}
    
AirSuiteTape::~AirSuiteTape() = default;

void AirSuiteTape::setSampleRate(float sr) {
    sampleRate = sr;
    if (m_impl) m_impl->setSampleRate(sr);
}

void AirSuiteTape::reset() {
    if (m_impl) m_impl->reset();
}

void AirSuiteTape::process(float& L, float& R, Type type, float* params) {
    if (m_impl) m_impl->process(L, R, type, params);
}

END_NAMESPACE_DISTRHO
