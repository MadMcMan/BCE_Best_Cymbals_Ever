#ifndef AIRWINDOWS_REVERBS_HPP
#define AIRWINDOWS_REVERBS_HPP

#include <memory>
#include "AirwindowsCommon.hpp"

START_NAMESPACE_DISTRHO

class AirSuiteReverb {
public:
    enum Type {
        NONE = 0,
        BRIGHTAMBIENCE, BRIGHTAMBIENCE2, BRIGHTAMBIENCE3,
        CHAMBER, CHAMBER2,
        CLEARCOAT, CREAMCOAT, CRUNCHCOAT,
        GALACTIC, GALACTIC2, GALACTIC3,
        INFINITY_VERB, INFINITY2_VERB,
        KALIENSPACESHIP, KBEYOND, KCATHEDRAL, KCATHEDRAL2, KCATHEDRAL3, KCATHEDRAL4, KCATHEDRAL5,
        KCHAMBERAR, KCOSMOS, KGUITARHALL, KGUITARHALL2, KPLATE140, KPLATE240, KPLATEA, KPLATEB, KPLATEC, KPLATED,
        KSTATION, KWOODROOM,
        MATRIXVERB, MV, MV2,
        NONLINEARSPACE, POCKETVERBS, REVERB,
        VERBITY, VERBITY2, VERBSIXES, VERBTHIC, VERBTINY,
        REVERB_DATTORRO,
        REVERB_LR_CONVOLVE,
        REVERB_LR_CONVOLVE2,
        REVERB_LR_CONVOLVE3,
        REVERB_NEVERLAND,
        REVERB_STAR_CHILD,
        REVERB_STAR_CHILD2,
        REVERB_THUNDER,
        REVERB_VOICE_OF_THE_STARSHIP,
        REVERB_WOODEN_BOX,
        KCYBERCITY, KROCKSTAR,
        REVERB_VERBSEVENS,
        REVERB_WOODENCHEST,
        COUNT
    };

    using Spec = AirwindowsSpec;

    // Constructor and destructor
    AirSuiteReverb(float sampleRate = 44100.0f);
    ~AirSuiteReverb();

    // Public methods
    void setSampleRate(float sr);
    void reset();
    void process(float& outL, float& outR, Type type, float* params);

    // Static specification - returns parameter specs for each algorithm type
    // Authoritative source: AirSuiteUniversal::getSpec() for REVERB_* types
    static Spec getReverbSpec(Type type);


private:
    // Pimpl implementation class (defined in .cpp)
    class Impl;
    float sampleRate;
    std::unique_ptr<Impl> m_impl;
};

END_NAMESPACE_DISTRHO

#endif // AIRWINDOWS_REVERBS_HPP
