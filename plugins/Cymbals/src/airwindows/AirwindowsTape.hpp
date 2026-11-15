#pragma once

#include <memory>
#include "AirwindowsCommon.hpp"

START_NAMESPACE_DISTRHO

class AirSuiteTape {
public:
    enum Type {
        NONE = 0,
        TOTAPE5, TOTAPE6, TOVINYL4,
        IRONOXIDE5, FLUTTER2, DUBLY, DUBLY2, DUBLY3,
        TAPE_CHROME_OXIDE,
        TAPE_CRUNCHY_GROOVE_WEAR,
        TAPE_DISCONTAPEITY,
        TAPE_DUB_PLATE,
        TAPE_DUB_PLATE2,
        TAPE_FLUTTER,
        TAPE_FROM_TAPE,
        TAPE_IRON_OXIDE_CLASSIC,
        TAPE_IRON_OXIDE_CLASSIC2,
        TAPE_TAPE,
        TAPE_TAPE_BIAS,
        TAPE_TAPE_DUST,
        TAPE_TAPE_FAT,
        TAPE_TAPE_HACK,
        TAPE_TAPE_HACK2,
        TAPE_TAPE_HACK3,
        TAPE_TO_TAPE7,
        TAPE_TO_TAPE8,
        TAPE_TO_TAPE9,
        COUNT
    };

    using Spec = AirwindowsSpec;

    static Spec getTapeSpec(Type type);

    AirSuiteTape(float sampleRate = 44100.0f);
    ~AirSuiteTape();

    void setSampleRate(float sr);
    void reset();
    void process(float& outL, float& outR, Type type, float* params);

private:
    class Impl;
    float sampleRate;
    std::unique_ptr<Impl> m_impl;
};

END_NAMESPACE_DISTRHO
