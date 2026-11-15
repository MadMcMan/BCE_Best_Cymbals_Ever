#ifndef AIRWINDOWS_NOISE_HPP
#define AIRWINDOWS_NOISE_HPP

#include <memory>
#include "AirwindowsCommon.hpp"

START_NAMESPACE_DISTRHO

class AirSuiteNoise {
public:
    enum Type {
        NONE = 0,
        DARKNOISE,
        HOLT,
        NOISE,
        PEAR,
        SURGE,
        SURGETIDE,
        COUNT
    };

    using Spec = AirwindowsSpec;

    AirSuiteNoise(float sampleRate = 44100.0f);
    ~AirSuiteNoise();
    void setSampleRate(float sr);
    void reset();
    void process(float& outL, float& outR, Type type, const float* params);

    static Spec getNoiseSpec(Type type);


private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

END_NAMESPACE_DISTRHO
#endif // AIRWINDOWS_NOISE_HPP
