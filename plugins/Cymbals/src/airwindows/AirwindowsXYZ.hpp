#ifndef AIRWINDOWS_XYZ_HPP
#define AIRWINDOWS_XYZ_HPP

#include <memory>
#include "AirwindowsCommon.hpp"

START_NAMESPACE_DISTRHO

class AirSuiteXYZ {
public:
    enum Type { NONE=0, XBANDPASS=1, XHIGHPASS=2, XLOWPASS=3, XNOTCH=4, XREGION=5,
                YBANDPASS=6, YHIGHPASS=7, YLOWPASS=8, YNOTCH=9, ZBANDPASS=10, ZHIGHPASS=11, COUNT_T=12 };

    using Spec = AirwindowsSpec;

    AirSuiteXYZ(float sampleRate = 44100.0f);
    ~AirSuiteXYZ();
    void setSampleRate(float sr);
    void reset();
    void process(float& outL, float& outR, Type type, const float* params);

    static Spec getXYZSpec(Type type);


private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

END_NAMESPACE_DISTRHO

#endif // AIRWINDOWS_XYZ_HPP
