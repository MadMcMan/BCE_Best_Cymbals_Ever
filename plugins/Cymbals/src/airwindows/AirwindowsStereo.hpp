#ifndef AIRWINDOWS_STEREO_HPP
#define AIRWINDOWS_STEREO_HPP

#include <memory>
#include "AirwindowsCommon.hpp"

START_NAMESPACE_DISTRHO

class AirSuiteStereo {
public:
    enum Type {
        NONE = 0,
        CHANNEL4,
        CHANNEL5,
        CHANNEL6,
        CHANNEL7,
        CHANNEL8,
        CHANNEL9,
        MIDSIDE,
        PHASENUDGE,
        SRSLY,
        WIDER,
        COUNT
    };

    using Spec = AirwindowsSpec;

    AirSuiteStereo(float sampleRate = 44100.0f);
    ~AirSuiteStereo();
    void setSampleRate(float sr);
    void reset();
    void process(float& outL, float& outR, Type type, const float* params);

    static Spec getStereoSpec(Type type);


private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

END_NAMESPACE_DISTRHO
#endif
