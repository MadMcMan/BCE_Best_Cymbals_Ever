#pragma once

#include <memory>
#include "AirwindowsCommon.hpp"

START_NAMESPACE_DISTRHO

class AirSuiteDynamics {
public:
    enum Type {
        NONE = 0,
        PRESSURE4,
        PRESSURE5,
        VARIMU,
        POINT,
        ACCELERATION,
        PURESTGAIN,
        DYN_ACCELERATION2,
        DYN_A_QUICK_VOICE_CLIP,
        DYN_BEZI_COMP,
        DYN_BLOCK_PARTY,
        DYN_BRASS_RIDER,
        DYN_BUTTER_COMP,
        DYN_BUTTER_COMP2,
        DYN_CLIP_ONLY3,
        DYN_C_STRIP,
        DYN_DE_BESS,
        DYN_DE_ESS,
        DYN_DE_HISS,
        DYN_DE_NOISE,
        DYN_DRUM_SLAM,
        DYN_DYNAMICS,
        DYN_DYNAMICS2,
        DYN_DYNAMICS3,
        DYN_DYNO,
        DYN_FINAL_CLIP,
        DYN_FLOOR,
        DYN_GATELOPE,
        DYN_HIGH_IMPACT,
        DYN_HYPERSOFT,
        DYN_LOGICAL4,
        DYN_MASTERING,
        DYN_MASTERING2,
        DYN_PEAKS_ONLY,
        DYN_PODCAST,
        DYN_PODCAST_DELUXE,
        DYN_PRESSURE6,
        DYN_PYEWACKET,
        DYN_SOFT_GATE,
        DYN_STONE_FIRE_COMP,
        DYN_SWELL,
        DYN_X2_BUSS,
        DYN_Z_OUTPUT_STAGE,
        DYN_ULTRALIGHT3,
        COUNT
    };

    using Spec = AirwindowsSpec;

    static Spec getDynamicsSpec(Type type);

    // Constructor and destructor
    AirSuiteDynamics(float sampleRate = 44100.0f);
    ~AirSuiteDynamics();

    // Public methods
    void setAlgorithm(int algo);  // KEY: Explicit algorithm selection
    void setSampleRate(float sr);
    void reset();
    void process(float& outL, float& outR, float* params);

private:
    // Pimpl implementation class (defined in .cpp)
    class Impl;
    float sampleRate;
    std::unique_ptr<Impl> m_impl;
};

END_NAMESPACE_DISTRHO
