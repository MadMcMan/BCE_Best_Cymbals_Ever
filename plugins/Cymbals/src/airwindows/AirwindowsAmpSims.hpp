#pragma once

#include <memory>
#include "AirwindowsCommon.hpp"

START_NAMESPACE_DISTRHO

class AirSuiteAmpSim {
public:
    enum Type {
        NONE = 0,
        CABS, BASSAMP, BIGAMP, FIREAMP, GRINDAMP, GUITARCONDITIONER,
        PURESTMULTISQUISH, TONESLANT, LEADAMP, LILAMPL,
        AMPSIM_CABS2,
        AMPSIM_CHIMEY_DELUXE,
        AMPSIM_CHIMEY_GUITAR,
        AMPSIM_CHIMEY_GUITAR2,
        AMPSIM_CRICK_BASS,
        AMPSIM_MID_AMP,
        AMPSIM_POINTY_DELUXE,
        AMPSIM_POINTY_GUITAR,
        AMPSIM_PUNCHY_DELUXE,
        AMPSIM_PUNCHY_GUITAR,
        AMPSIM_SINGLE_ENDED_TRIODE,
        AMPSIM_TUBE,
        AMPSIM_TUBE2,
        COUNT
    };

    using Spec = AirwindowsSpec;

    static Spec getAmpSimSpec(Type type);

    AirSuiteAmpSim(float sampleRate = 44100.0f);
    ~AirSuiteAmpSim();

    void setSampleRate(float sr);
    void reset();
    void process(float& outL, float& outR, Type type, float* params);

private:
    class Impl;
    float sampleRate;
    std::unique_ptr<Impl> m_impl;
};

END_NAMESPACE_DISTRHO
