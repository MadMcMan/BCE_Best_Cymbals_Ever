#ifndef AIRWINDOWS_CLIPPING_HPP
#define AIRWINDOWS_CLIPPING_HPP

#include <memory>
#include "AirwindowsCommon.hpp"

START_NAMESPACE_DISTRHO

class AirSuiteClipping {
public:
    enum Type {
        NONE = 0,
        ADCLIP7,          // Real: deps/airwindows/plugins/WinVST/ADClip7/
        ADCLIP8,          // Real: deps/airwindows/plugins/WinVST/ADClip8/
        CLIPONLY,         // Real: deps/airwindows/plugins/WinVST/ClipOnly/
        CLIPONLY2,        // Real: deps/airwindows/plugins/WinVST/ClipOnly2/
        CLIPSOFTLY,       // Real: deps/airwindows/plugins/WinVST/ClipSoftly/
        ADCLIP9,          // Real: deps/airwindows/plugins/WinVST/ADClip9/
        COUNT
    };

    using Spec = AirwindowsSpec;

    // Constructor and destructor
    AirSuiteClipping(float sampleRate = 44100.0f);
    ~AirSuiteClipping();

    // Public methods
    void setSampleRate(float sr);
    void reset();
    void process(float& outL, float& outR, Type type, float* params);

    // Static specification
    static Spec getClippingSpec(Type type);


private:
    // Pimpl implementation class (defined in .cpp)
    class Impl;
    float sampleRate;
    std::unique_ptr<Impl> m_impl;
};

END_NAMESPACE_DISTRHO

#endif // AIRWINDOWS_CLIPPING_HPP
