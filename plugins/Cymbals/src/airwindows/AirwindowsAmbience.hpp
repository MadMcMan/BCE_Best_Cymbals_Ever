#ifndef AIRWINDOWS_AMBIENCE_HPP
#define AIRWINDOWS_AMBIENCE_HPP

#include <memory>
#include "AirwindowsCommon.hpp"

START_NAMESPACE_DISTRHO

class AirSuiteAmbience {
public:
    enum Type {
        NONE = 0,
        AURA,
        BRIGHTAMBIENCE,
        BRIGHTAMBIENCE2,
        BRIGHTAMBIENCE3,
        CHAMBER,
        CHAMBER2,
        VERBITY,
        VERBITY2,
        COUNT
    };

    using Spec = AirwindowsSpec;

    AirSuiteAmbience(float sampleRate = 44100.0f);
    ~AirSuiteAmbience();
    void setSampleRate(float sr);
    void reset();
    void process(float& outL, float& outR, Type type, const float* params);

    static Spec getAmbienceSpec(Type type);


private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

END_NAMESPACE_DISTRHO
#endif
