#ifndef AIRWINDOWS_BASS_HPP
#define AIRWINDOWS_BASS_HPP

#include <memory>
#include "AirwindowsCommon.hpp"

START_NAMESPACE_DISTRHO

class AirSuiteBass {
public:
    enum Type {
        NONE = 0,
        BASSAMP,
        BASSDRIVE,
        BASSKIT,
        DUBSUB,
        SUBSONLY,
        RETROBASS,
        COUNT
    };

    using Spec = AirwindowsSpec;

    AirSuiteBass(float sampleRate = 44100.0f);
    ~AirSuiteBass();
    void setSampleRate(float sr);
    void reset();
    void process(float& outL, float& outR, Type type, const float* params);

    static Spec getBassSpec(Type type);


private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

END_NAMESPACE_DISTRHO
#endif
