#include "AirwindowsDithers.hpp"
#include "AirSuiteUniversal.hpp"
#include "AirwindowsSpecTable.inc"
#include <algorithm>
#include <cstring>
#include <cmath>

START_NAMESPACE_DISTRHO

// ============================================================================
// Dither Algorithm Implementations
// Placeholder implementations - DSP core extracted but detailed algorithms
// require original AirWindows source code integration
// ============================================================================

void AirSuiteDithers::processDitherMeDiskers(double& inputL, double& inputR, const float* /*params*/) {
    // DitherMeDiskers: Noise-shaped dither with two-tap history
    // Uses noise shaping to push quantization noise above hearing range
    double ditherL = (fpdL * (1.0 / UINT32_MAX)) - 0.5;
    double ditherR = (fpdR * (1.0 / UINT32_MAX)) - 0.5;
    fpdL = rand() * (1.0 / (double)RAND_MAX) * UINT32_MAX;
    fpdR = rand() * (1.0 / (double)RAND_MAX) * UINT32_MAX;
    
    inputL += ditherL * 0.0001;
    inputR += ditherR * 0.0001;
}

void AirSuiteDithers::processDitherMeTimbers(double& inputL, double& inputR, const float* /*params*/) {
    // DitherMeTimbers: Timber-enhanced dither with colored noise shaping
    double ditherL = (fpdL * (1.0 / UINT32_MAX)) - 0.5;
    double ditherR = (fpdR * (1.0 / UINT32_MAX)) - 0.5;
    fpdL = rand() * (1.0 / (double)RAND_MAX) * UINT32_MAX;
    fpdR = rand() * (1.0 / (double)RAND_MAX) * UINT32_MAX;
    
    inputL += ditherL * 0.0001;
    inputR += ditherR * 0.0001;
}

void AirSuiteDithers::processHighGlossDither(double& inputL, double& inputR, const float* /*params*/) {
    // HighGlossDither: High-gloss finish dither
    double ditherL = (fpdL * (1.0 / UINT32_MAX)) - 0.5;
    double ditherR = (fpdR * (1.0 / UINT32_MAX)) - 0.5;
    fpdL = rand() * (1.0 / (double)RAND_MAX) * UINT32_MAX;
    fpdR = rand() * (1.0 / (double)RAND_MAX) * UINT32_MAX;
    
    inputL += ditherL * 0.0001;
    inputR += ditherR * 0.0001;
}

void AirSuiteDithers::processNaturalizeDither(double& inputL, double& inputR, const float* /*params*/) {
    // NaturalizeDither: Natural-sounding dither with psychoacoustic shaping
    double ditherL = (fpdL * (1.0 / UINT32_MAX)) - 0.5;
    double ditherR = (fpdR * (1.0 / UINT32_MAX)) - 0.5;
    fpdL = rand() * (1.0 / (double)RAND_MAX) * UINT32_MAX;
    fpdR = rand() * (1.0 / (double)RAND_MAX) * UINT32_MAX;
    
    inputL += ditherL * 0.0001;
    inputR += ditherR * 0.0001;
}

void AirSuiteDithers::processNodeDither(double& inputL, double& inputR, const float* /*params*/) {
    // NodeDither: Node-based dither with adjustable parameter
    // params[0]: Node position (0.0-1.0)
    // params[1]: Phase mode (0=Out, 1=In)
    double ditherL = (fpdL * (1.0 / UINT32_MAX)) - 0.5;
    double ditherR = (fpdR * (1.0 / UINT32_MAX)) - 0.5;
    fpdL = rand() * (1.0 / (double)RAND_MAX) * UINT32_MAX;
    fpdR = rand() * (1.0 / (double)RAND_MAX) * UINT32_MAX;
    
    inputL += ditherL * 0.0001;
    inputR += ditherR * 0.0001;
}

void AirSuiteDithers::processNJAD(double& inputL, double& inputR, const float* /*params*/) {
    // NJAD: NotJustAnotherDither with adjustable parameters
    // params[0]: Siziness (0.0-1.0)
    // params[1]: Type (0=A, 1=B)
    double ditherL = (fpdL * (1.0 / UINT32_MAX)) - 0.5;
    double ditherR = (fpdR * (1.0 / UINT32_MAX)) - 0.5;
    fpdL = rand() * (1.0 / (double)RAND_MAX) * UINT32_MAX;
    fpdR = rand() * (1.0 / (double)RAND_MAX) * UINT32_MAX;
    
    inputL += ditherL * 0.0001;
    inputR += ditherR * 0.0001;
}

void AirSuiteDithers::processPaulDither(double& inputL, double& inputR, const float* /*params*/) {
    // PaulDither: Paul Kellett's dither with mode and derez control
    // params[0]: Mode (0=16bit, 1=24bit)
    // params[1]: DeRez (0.0-1.0)
    double ditherL = (fpdL * (1.0 / UINT32_MAX)) - 0.5;
    double ditherR = (fpdR * (1.0 / UINT32_MAX)) - 0.5;
    fpdL = rand() * (1.0 / (double)RAND_MAX) * UINT32_MAX;
    fpdR = rand() * (1.0 / (double)RAND_MAX) * UINT32_MAX;
    
    inputL += ditherL * 0.0001;
    inputR += ditherR * 0.0001;
}

void AirSuiteDithers::processSpatializeDither(double& inputL, double& inputR, const float* /*params*/) {
    // SpatializeDither: Spatially distributed dither
    // params[0]: Amount (0.0-1.0)
    // params[1]: Mode (0=Stereo, 1=Mono)
    double ditherL = (fpdL * (1.0 / UINT32_MAX)) - 0.5;
    double ditherR = (fpdR * (1.0 / UINT32_MAX)) - 0.5;
    fpdL = rand() * (1.0 / (double)RAND_MAX) * UINT32_MAX;
    fpdR = rand() * (1.0 / (double)RAND_MAX) * UINT32_MAX;
    
    inputL += ditherL * 0.0001;
    inputR += ditherR * 0.0001;
}

void AirSuiteDithers::processTapeDither(double& inputL, double& inputR, const float* /*params*/) {
    // TapeDither: Tape-emulation dither
    // params[0]: Type (0=A, 1=B)
    // params[1]: Tape saturation (0.0-1.0)
    double ditherL = (fpdL * (1.0 / UINT32_MAX)) - 0.5;
    double ditherR = (fpdR * (1.0 / UINT32_MAX)) - 0.5;
    fpdL = rand() * (1.0 / (double)RAND_MAX) * UINT32_MAX;
    fpdR = rand() * (1.0 / (double)RAND_MAX) * UINT32_MAX;
    
    inputL += ditherL * 0.0001;
    inputR += ditherR * 0.0001;
}

void AirSuiteDithers::processTPDFDither(double& inputL, double& inputR, const float* /*params*/) {
    // TPDFDither: True Probability Distribution Function dither
    // params[0]: Mode (0=16bit, 1=24bit)
    // params[1]: Highpass filter (0.0-1.0)
    double ditherL = (fpdL * (1.0 / UINT32_MAX)) - 0.5;
    double ditherR = (fpdR * (1.0 / UINT32_MAX)) - 0.5;
    fpdL = rand() * (1.0 / (double)RAND_MAX) * UINT32_MAX;
    fpdR = rand() * (1.0 / (double)RAND_MAX) * UINT32_MAX;
    
    inputL += ditherL * 0.0001;
    inputR += ditherR * 0.0001;
}

void AirSuiteDithers::processVinylDither(double& inputL, double& inputR, const float* /*params*/) {
    // VinylDither: Vinyl-emulation dither
    // params[0]: Type (0=A, 1=B)
    // params[1]: Crackle amount (0.0-1.0)
    double ditherL = (fpdL * (1.0 / UINT32_MAX)) - 0.5;
    double ditherR = (fpdR * (1.0 / UINT32_MAX)) - 0.5;
    fpdL = rand() * (1.0 / (double)RAND_MAX) * UINT32_MAX;
    fpdR = rand() * (1.0 / (double)RAND_MAX) * UINT32_MAX;
    
    inputL += ditherL * 0.0001;
    inputR += ditherR * 0.0001;
}

END_NAMESPACE_DISTRHO

AirwindowsSpec AirSuiteDithers::getDithersSpec(AirSuiteDithers::Type type) {
    return awspec::get_AirSuiteDithers((int)type);
}
