#ifndef AIRWINDOWS_TONECOLOUR_HPP
#define AIRWINDOWS_TONECOLOUR_HPP

#include <memory>
#include "AirwindowsCommon.hpp"

START_NAMESPACE_DISTRHO

class AirSuiteToneColour {
public:
    enum Type { NONE=0, FOCUS, GROOVEWEAR, HUGENESS, MELT, ONECORNERCLIP, 
                PAFNUTY, PEAR, PEAR2, SURGE, SWEETEN, COUNT };

    using Spec = AirwindowsSpec;

    AirSuiteToneColour(float sampleRate = 44100.0f);
    ~AirSuiteToneColour();
    void setSampleRate(float sr);
    void reset();
    void process(float& outL, float& outR, Type type, const float* params);

    static Spec getToneColourSpec(Type type);


private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

END_NAMESPACE_DISTRHO

#endif // AIRWINDOWS_TONECOLOUR_HPP
