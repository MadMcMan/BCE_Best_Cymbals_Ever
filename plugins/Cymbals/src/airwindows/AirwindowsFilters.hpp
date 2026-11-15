#pragma once

#include <memory>
#include "AirwindowsCommon.hpp"

START_NAMESPACE_DISTRHO

class AirSuiteFilters {
public:
    enum Type {
        NONE = 0,
        AIR, AIR2, AIR3, AIR4, BAXANDALL, BAXANDALL2, BAXANDALL3, BIQUADDOUBLE,
        CAPACITOR2, HIGHPASS2, LOWPASS2, CAPACITOR, HIGHPASS, LOWPASS,
        FILTER_ANGLE_EQ,
        FILTER_ANGLE_FILTER,
        FILTER_AVER_MATRIX,
        FILTER_BEZ_EQ,
        FILTER_BEZ_EQ_2,
        FILTER_DARK,
        FILTER_DE_BEZ,
        FILTER_ELLIPTICAL,
        FILTER_EQ,
        FILTER_FAT_EQ,
        FILTER_HERMEPASS,
        FILTER_HERME_TRIM,
        FILTER_HOLT2,
        FILTER_HULL,
        FILTER_HULL2,
        FILTER_HYPERSONIC,
        FILTER_HYPERSON_X,
        FILTER_INFRASONIC,
        FILTER_ISOLATOR,
        FILTER_ISOLATOR2,
        FILTER_ISOLATOR3,
        FILTER_KALMAN,
        FILTER_MACK_EQ,
        FILTER_PARAMETRIC,
        FILTER_PEAR_EQ,
        FILTER_PEAR_LITE_EQ,
        FILTER_PRIME_FIR,
        FILTER_RES_EQ,
        FILTER_RES_EQ_2,
        FILTER_SIDE_DULL,
        FILTER_SIDEPASS,
        FILTER_SILKEN,
        FILTER_SMOOTH_EQ_2,
        FILTER_SMOOTH_EQ_3,
        FILTER_ULTRASONIC,
        FILTER_ULTRASONIC_LITE,
        FILTER_ULTRASONIC_MED,
        FILTER_ULTRASON_X,
        FILTER_Y_NOT_BANDPASS,
        FILTER_Z_ACID_LOWPASS,
        FILTER_Z_BANDPASS2,
        FILTER_Z_HIGHPASS2,
        FILTER_Z_LOWPASS,
        FILTER_Z_LOWPASS2,
        FILTER_Z_NOTCH,
        FILTER_Z_NOTCH2,
        FILTER_Z_REGION,
        FILTER_Z_REGION2,
        FILTER_BEZ_EQ3,
        FILTER_BEZ_EQ4,
        FILTER_SUZAN,
        FILTER_YNOTHIGHPASS,
        FILTER_YNOTLOWPASS,
        FILTER_YNOTNOTCH,
        COUNT
    };

    using Spec = AirwindowsSpec;

    static Spec getFilterSpec(Type type);

    AirSuiteFilters(float sampleRate = 44100.0f);
    ~AirSuiteFilters();

    void setSampleRate(float sr);
    void reset();
    void process(float& outL, float& outR, Type type, float* params);

private:
    class Impl;
    float sampleRate;
    std::unique_ptr<Impl> m_impl;
};

END_NAMESPACE_DISTRHO
