#ifndef AIRWINDOWS_LOFI_HPP
#define AIRWINDOWS_LOFI_HPP

#include <memory>
#include "AirwindowsCommon.hpp"

START_NAMESPACE_DISTRHO

class AirSuiteLoFi {
public:
    enum Type {
        NONE = 0,
        DESK,
        DESK4,
        DIGITALBLACK,
        DITHERFLOAT,
        FATHOMFIVE,
        GRINGER,
        GROOVEWEAR,
        LOUD,
        MACKITY,
        NCSEVENTEEN,
        POCKETVERBS,
        SAMPLEDELAY,
        COUNT
    };

    using Spec = AirwindowsSpec;

    AirSuiteLoFi(float sampleRate = 44100.0f);
    ~AirSuiteLoFi();
    void setSampleRate(float sr);
    void reset();
    void process(float& outL, float& outR, Type type, const float* params);

    static Spec getLoFiSpec(Type type);


private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

END_NAMESPACE_DISTRHO
#endif // AIRWINDOWS_LOFI_HPP
