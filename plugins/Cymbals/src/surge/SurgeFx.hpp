// Surge XT effect source for the unified FX slot lookup.
//
// Source id layout: Airwindows occupies 0..547; Surge starts at kBaseType.
// This header deliberately exposes no sst/Surge types (keeps their C++20 SSE
// headers out of the plugin's other translation units); the implementation
// lives in SurgeFx.cpp.
//
// The DSP is sst-effects (GPL-3.0-or-later), the embedding-ready extraction of
// Surge XT's effects. Effects not yet extracted upstream (Chorus, Distortion,
// Frequency Shifter, Vocoder, ...) are not registered here yet.
#ifndef CYMBALS_SURGE_FX_HPP
#define CYMBALS_SURGE_FX_HPP

#include <memory>

namespace CymbalsSurgeFx {

inline constexpr int kBaseType = 600;

int algorithmCount();
int algorithmId(int index);          // kBaseType + index
const char* algorithmName(int index);
const char* algorithmCategory(int index);

int parameterCount(int algoIndex);
const char* parameterName(int algoIndex, int param);
const char* parameterUnit(int algoIndex, int param);
float parameterDefaultNormalized(int algoIndex, int param);

class Instance {
  public:
    virtual ~Instance() = default;
    virtual void setSampleRate(double sr) = 0;
    virtual void setParamNormalized(int param, float normalized) = 0;
    virtual void reset() = 0;
    // Per-sample streaming with an aligned dry output. sst effects run on a
    // fixed 16-sample block, so the slot introduces exactly one block (16
    // samples) of latency; dryL/dryR are the input delayed by the SAME amount
    // so an external wet/dry mix never combs.
    virtual void processSample(float& left, float& right,
                               float& dryLeft, float& dryRight) = 0;
};

std::unique_ptr<Instance> createInstance(int algoIndex, double sampleRate);

} // namespace CymbalsSurgeFx

#endif
