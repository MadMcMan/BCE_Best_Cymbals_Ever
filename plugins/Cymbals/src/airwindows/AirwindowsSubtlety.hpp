#ifndef AIRWINDOWS_SUBTLETY_HPP
#define AIRWINDOWS_SUBTLETY_HPP

#include <memory>
#include "AirwindowsCommon.hpp"

START_NAMESPACE_DISTRHO

class AirSuiteSubtlety {
public:
    enum Type { NONE=0, BAXANDALL, BAXANDALL2, BAXANDALL3,
                CALIBRE, COILS, COILS2, INTERSTAGE, MOJO,
                PLATINUMSLEW, SLEWSONIC, SMOOTH, SMOOTHEQ,
                TUBEDESK, UNBOX, COUNT };

    using Spec = AirwindowsSpec;

    AirSuiteSubtlety(float sampleRate = 44100.0f);
    ~AirSuiteSubtlety();
    void setSampleRate(float sr);
    void reset();
    void process(float& outL, float& outR, Type type, const float* params);

    static Spec getSubtletySpec(Type type);


private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

END_NAMESPACE_DISTRHO

#endif // AIRWINDOWS_SUBTLETY_HPP
