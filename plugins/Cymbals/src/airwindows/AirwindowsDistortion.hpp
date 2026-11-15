#pragma once

#include <memory>
#include "AirwindowsCommon.hpp"

START_NAMESPACE_DISTRHO

class AirSuiteDistortion {
public:
    enum Type {
        NONE = 0,
        BASSDRIVE, BITGLITTER,
        DENSITY, DENSITY2, DENSITY3,
        DISTORTION, DRIVE,
        HARDVACUUM, PURESTDRIVE,
        SPIRAL, SPIRAL2, SPIRAL3,
        DIST_BITE,
        DIST_COJONES,
        DIST_CREATURE,
        DIST_DECKWRECKA,
        DIST_DIRT,
        DIST_DONUT,
        DIST_EDGE,
        DIST_GOLEM,
        DIST_HIP_CRUSH,
        DIST_HYPE,
        DIST_INFLAMER,
        DIST_MULTI_BAND_DISTORTION,
        DIST_POP,
        DIST_POP2,
        DIST_POP3,
        DIST_SHAPE,
        DIST_STONEFIRE,
        // Restored: the universal tokens DIST_FRACTURE/DIST_FRACTURE2 exist and
        // upstream still ships Fracture/Fracture2, but the category enum had lost
        // its members - so those two types resolved to Category::NONE and were
        // dead. Appended (not reordered): category enum values are not persisted.
        DIST_FRACTURE, DIST_FRACTURE2,
        COUNT
    };

    using Spec = AirwindowsSpec;

    static Spec getDistortionSpec(Type type);

    AirSuiteDistortion(float sampleRate = 44100.0f);
    ~AirSuiteDistortion();

    void setSampleRate(float sr);
    void reset();
    void process(float& outL, float& outR, Type type, float* params);

private:
    class Impl;
    float sampleRate;
    std::unique_ptr<Impl> m_impl;
};

END_NAMESPACE_DISTRHO
