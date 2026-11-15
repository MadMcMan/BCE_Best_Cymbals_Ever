#include "AirwindowsFilters.hpp"
#include "AirwindowsAdapter.hpp"
#include "AirwindowsFactories.inc"
#include "AirSuiteUniversal.hpp"
#include "AirwindowsSpecTable.inc"
#include "vst2stub/audioeffectx.h"
#include <cstring>
#include <cmath>

START_NAMESPACE_DISTRHO

extern "C" {
    AudioEffectX* aw_create_Air();
    AudioEffectX* aw_create_Air2();
    AudioEffectX* aw_create_Air3();
    AudioEffectX* aw_create_Air4();
    AudioEffectX* aw_create_Baxandall();
    AudioEffectX* aw_create_Baxandall2();
    AudioEffectX* aw_create_Baxandall3();
    AudioEffectX* aw_create_BiquadDouble();
    AudioEffectX* aw_create_Capacitor2();
    AudioEffectX* aw_create_Highpass2();
    AudioEffectX* aw_create_Lowpass2();
    AudioEffectX* aw_create_Capacitor();
    AudioEffectX* aw_create_Highpass();
    AudioEffectX* aw_create_Lowpass();
    AudioEffectX* aw_create_AngleEQ();
    AudioEffectX* aw_create_AngleFilter();
    AudioEffectX* aw_create_AverMatrix();
    AudioEffectX* aw_create_BezEQ();
    AudioEffectX* aw_create_BezEQ2();
    AudioEffectX* aw_create_Dark();
    AudioEffectX* aw_create_DeBez();
    AudioEffectX* aw_create_Elliptical();
    AudioEffectX* aw_create_EQ();
    AudioEffectX* aw_create_FatEQ();
    AudioEffectX* aw_create_Hermepass();
    AudioEffectX* aw_create_HermeTrim();
    AudioEffectX* aw_create_Holt2();
    AudioEffectX* aw_create_Hull();
    AudioEffectX* aw_create_Hull2();
    AudioEffectX* aw_create_Hypersonic();
    AudioEffectX* aw_create_HypersonX();
    AudioEffectX* aw_create_Infrasonic();
    AudioEffectX* aw_create_Isolator();
    AudioEffectX* aw_create_Isolator2();
    AudioEffectX* aw_create_Isolator3();
    AudioEffectX* aw_create_Kalman();
    AudioEffectX* aw_create_MackEQ();
    AudioEffectX* aw_create_Parametric();
    AudioEffectX* aw_create_PearEQ();
    AudioEffectX* aw_create_PearLiteEQ();
    AudioEffectX* aw_create_PrimeFIR();
    AudioEffectX* aw_create_ResEQ();
    AudioEffectX* aw_create_ResEQ2();
    AudioEffectX* aw_create_SideDull();
    AudioEffectX* aw_create_Sidepass();
    AudioEffectX* aw_create_Silken();
    AudioEffectX* aw_create_SmoothEQ2();
    AudioEffectX* aw_create_SmoothEQ3();
    AudioEffectX* aw_create_Ultrasonic();
    AudioEffectX* aw_create_UltrasonicLite();
    AudioEffectX* aw_create_UltrasonicMed();
    AudioEffectX* aw_create_UltrasonX();
    AudioEffectX* aw_create_YNotBandpass();
    AudioEffectX* aw_create_ZAcidLowpass();
    AudioEffectX* aw_create_ZBandpass2();
    AudioEffectX* aw_create_ZHighpass2();
    AudioEffectX* aw_create_ZLowpass();
    AudioEffectX* aw_create_ZLowpass2();
    AudioEffectX* aw_create_ZNotch();
    AudioEffectX* aw_create_ZNotch2();
    AudioEffectX* aw_create_ZRegion();
    AudioEffectX* aw_create_ZRegion2();
    AudioEffectX* aw_create_BezEQ3();
    AudioEffectX* aw_create_BezEQ4();
    AudioEffectX* aw_create_Suzan();
    AudioEffectX* aw_create_YNotHighpass();
    AudioEffectX* aw_create_YNotLowpass();
    AudioEffectX* aw_create_YNotNotch();
}

AirwindowsSpec AirSuiteFilters::getFilterSpec(AirSuiteFilters::Type type) {
        return awspec::get_AirSuiteFilters((int)type);
    }

class AirSuiteFilters::Impl {
public:
    Impl(float sr) : m_adapter(sr, awspec::createFor_AirSuiteFilters, false) { m_adapter.setSampleRate(sr); }
    void setSampleRate(float sr) { m_adapter.setSampleRate(sr); }
    void setAlgorithm(AirSuiteFilters::Type type) { m_adapter.setType((int)type); }
    void reset() { m_adapter.reset(); }
    void process(float& L, float& R, AirSuiteFilters::Type type, float* params) {
        m_adapter.setType((int)type);
        m_adapter.process(L, R, params, AirSuiteFilters::getFilterSpec(type).count);
    }
private:
    AirwindowsAdapter m_adapter;
};

AirSuiteFilters::AirSuiteFilters(float sampleRate)
    : sampleRate(sampleRate), m_impl(std::make_unique<Impl>(sampleRate)) {}
    
AirSuiteFilters::~AirSuiteFilters() = default;

void AirSuiteFilters::setSampleRate(float sr) {
    sampleRate = sr;
    if (m_impl) m_impl->setSampleRate(sr);
}

void AirSuiteFilters::reset() {
    if (m_impl) m_impl->reset();
}

void AirSuiteFilters::process(float& L, float& R, Type type, float* params) {
    if (m_impl) m_impl->process(L, R, type, params);
}

END_NAMESPACE_DISTRHO
