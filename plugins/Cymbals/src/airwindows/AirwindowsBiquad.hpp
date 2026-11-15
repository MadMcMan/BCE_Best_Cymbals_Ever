#ifndef AIRWINDOWS_BIQUAD_HPP
#define AIRWINDOWS_BIQUAD_HPP

#include <memory>
#include "AirwindowsCommon.hpp"

START_NAMESPACE_DISTRHO

class AirSuiteBiquad {
public:
    enum Type {
        NONE = 0,
        BIQUAD,           // Real: deps/airwindows/plugins/WinVST/Biquad/
        BIQUAD2,          // Real: deps/airwindows/plugins/WinVST/Biquad2/
        BIQUADHILO,       // Real: deps/airwindows/plugins/WinVST/BiquadHiLo/
        BIQUADNONLIN,     // Real: deps/airwindows/plugins/WinVST/BiquadNonLin/
        BIQUADONEHALF,    // Real: deps/airwindows/plugins/WinVST/BiquadOneHalf/
        BIQUADPLUS,       // Real: deps/airwindows/plugins/WinVST/BiquadPlus/
        BIQUADSTACK,      // Real: deps/airwindows/plugins/WinVST/BiquadStack/
        BIQUADTRIPLE,     // Real: deps/airwindows/plugins/WinVST/BiquadTriple/
        COUNT
    };

    using Spec = AirwindowsSpec;

    // Constructor and destructor
    AirSuiteBiquad(float sampleRate = 44100.0f);
    ~AirSuiteBiquad();

    // Public methods
    void setSampleRate(float sr);
    void reset();
    void process(float& outL, float& outR, Type type, float* params);

    // Static specification - returns parameter specs for each algorithm type
    static Spec getBiquadSpec(Type type);


private:
    // Pimpl implementation class (defined in .cpp)
    class Impl;
    float sampleRate;
    std::unique_ptr<Impl> m_impl;
};

END_NAMESPACE_DISTRHO

#endif // AIRWINDOWS_BIQUAD_HPP
