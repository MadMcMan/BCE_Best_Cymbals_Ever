#ifndef AIRWINDOWS_BRIGHTNESS_HPP
#define AIRWINDOWS_BRIGHTNESS_HPP

#include <memory>
#include "AirwindowsCommon.hpp"

START_NAMESPACE_DISTRHO

class AirSuiteBrightness {
public:
    enum Type {
        NONE = 0,
        EXCITER,
        AIR,
        APICOLYPSE,
        CAPACITOR,
        CAPACITOR2,
        CRYSTAL,
        PURESTAIR,
        COUNT
    };

    using Spec = AirwindowsSpec;

    AirSuiteBrightness(float sampleRate = 44100.0f);
    ~AirSuiteBrightness();
    void setSampleRate(float sr);
    void reset();
    void process(float& outL, float& outR, Type type, const float* params);

    static Spec getBrightnessSpec(Type type);


private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

END_NAMESPACE_DISTRHO
#endif
