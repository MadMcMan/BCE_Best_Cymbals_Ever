#ifndef AIRWINDOWS_UTILITY_HPP
#define AIRWINDOWS_UTILITY_HPP

#include <memory>
#include "AirwindowsCommon.hpp"

START_NAMESPACE_DISTRHO

class AirSuiteUtility {
public:
    enum Type {
        NONE = 0,
        AVERAGE,
        BALANCED,
        BITSHIFTGAIN,
        CSTRIP2,
        PURESTFADE,
        PURESTGAIN,
        PURESTWARM,
        RIGHTEOUS4,
        SLEW,
        SLEW2,
        SLEWONLY,
        STEREOFX,
        STUDIOTAN,
        WEIGHT,
        COUNT
    };

    using Spec = AirwindowsSpec;

    AirSuiteUtility(float sampleRate = 44100.0f);
    ~AirSuiteUtility();
    void setSampleRate(float sr);
    void reset();
    void process(float& outL, float& outR, Type type, const float* params);

    static Spec getUtilitySpec(Type type);


private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

END_NAMESPACE_DISTRHO
#endif
