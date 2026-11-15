#ifndef AIRWINDOWS_DITHERS_HPP
#define AIRWINDOWS_DITHERS_HPP

#include <cmath>
#include <cstring>
#include <algorithm>
#include <cstdlib>

#include "AirwindowsCommon.hpp"

START_NAMESPACE_DISTRHO

// ============================================================================
// REAL DSP EXTRACTION FROM AIRWINDOWS - DITHERS SUITE
// Extracted from deps/airwindows/plugins/WinVST/[various dither plugins]
// MIT License
// ============================================================================

class AirSuiteDithers {
public:
    enum Type {
        NONE = 0,
        DITHERMEDISKERS,    // Real: deps/airwindows/plugins/WinVST/DitherMeDiskers/
        DITHERMTIMBERS,     // Real: deps/airwindows/plugins/WinVST/DitherMeTimbers/
        HIGHGLOSS,          // Real: deps/airwindows/plugins/WinVST/HighGlossDither/
        NATURALIZE,         // Real: deps/airwindows/plugins/WinVST/NaturalizeDither/
        NODEDITHER,         // Real: deps/airwindows/plugins/WinVST/NodeDither/ (2 params)
        NJAD,               // Real: deps/airwindows/plugins/WinVST/NotJustAnotherDither/ (2 params)
        PAULDITHER,         // Real: deps/airwindows/plugins/WinVST/PaulDither/ (2 params)
        SPATIALIZE,         // Real: deps/airwindows/plugins/WinVST/SpatializeDither/ (2 params)
        TAPEDITHER,         // Real: deps/airwindows/plugins/WinVST/TapeDither/ (2 params)
        TPDFDITHER,         // Real: deps/airwindows/plugins/WinVST/TPDFDither/ (2 params)
        VINYLDITHER,        // Real: deps/airwindows/plugins/WinVST/VinylDither/ (2 params)
        COUNT
    };

    using Spec = AirwindowsSpec;

    static Spec getDithersSpec(Type type);


    AirSuiteDithers(float sr = 44100.0f) 
        : currentType(NONE), sampleRate(sr) {
        reset();
    }

    void setAlgorithm(int algo) {
        if (algo >= 0 && algo < (int)COUNT) {
            currentType = (Type)algo;
            reset();
        }
    }

    void setSampleRate(float sr) {
        sampleRate = sr;
    }

    void reset() {
        // Initialize all fpdL/fpdR pseudo-random number generators
        fpdL = fpdR = 1.0;
        while (fpdL < 16386) fpdL = rand() * (1.0 / (double)RAND_MAX) * UINT32_MAX;
        while (fpdR < 16386) fpdR = rand() * (1.0 / (double)RAND_MAX) * UINT32_MAX;

        // DitherMeDiskers state
        dithermeD_lastSampleL = 0.0;
        dithermeD_lastSampleR = 0.0;
        dithermeD_lastSample2L = 0.0;
        dithermeD_lastSample2R = 0.0;
        dithermeD_noiseShapingL = 0.0;
        dithermeD_noiseShapingR = 0.0;

        // DitherMeTimbers state
        dithermeT_lastSampleL = 0.0;
        dithermeT_lastSampleR = 0.0;
        dithermeT_lastSample2L = 0.0;
        dithermeT_lastSample2R = 0.0;
        dithermeT_noiseShapingL = 0.0;
        dithermeT_noiseShapingR = 0.0;

        // HighGlossDither state
        highgloss_Position = 0;
        highgloss_flip = false;

        // NaturalizeDither state
        for (int i = 0; i < 10; i++) {
            naturalize_bynL[i] = 0;
            naturalize_bynR[i] = 0;
        }

        // NodeDither state
        node_A = 0.04;
        node_B = 0.0;
        for (int i = 0; i < 4999; i++) {
            node_dL[i] = 0.0;
            node_dR[i] = 0.0;
        }
        node_gcount = 0;

        // NJAD state
        for (int i = 0; i < 9; i++) {
            njad_bynL[i] = 0;
            njad_bynR[i] = 0;
        }

        // PaulDither state
        paul_previousDitherL = 0.0;
        paul_previousDitherR = 0.0;

        // SpatializeDither state
        spatialize_lastSampleL = 0.0;
        spatialize_lastSampleR = 0.0;

        // TapeDither state
        tape_lastSampleL = 0.0;
        tape_lastSampleR = 0.0;
        for (int i = 0; i < 5; i++) {
            tape_bufferL[i] = 0.0;
            tape_bufferR[i] = 0.0;
        }
        tape_bufpos = 0;

        // TPDFDither state
        tpdf_lastSampleL = 0.0;
        tpdf_lastSampleR = 0.0;

        // VinylDither state
        vinyl_lastSampleL = 0.0;
        vinyl_lastSampleR = 0.0;
    }

    // Main process entry (float interface)
    void process(float& sampleL, float& sampleR, const float* params) {
        double dL = (double)sampleL;
        double dR = (double)sampleR;

        switch (currentType) {
            case DITHERMEDISKERS:
                processDitherMeDiskers(dL, dR, params);
                break;
            case DITHERMTIMBERS:
                processDitherMeTimbers(dL, dR, params);
                break;
            case HIGHGLOSS:
                processHighGlossDither(dL, dR, params);
                break;
            case NATURALIZE:
                processNaturalizeDither(dL, dR, params);
                break;
            case NODEDITHER:
                processNodeDither(dL, dR, params);
                break;
            case NJAD:
                processNJAD(dL, dR, params);
                break;
            case PAULDITHER:
                processPaulDither(dL, dR, params);
                break;
            case SPATIALIZE:
                processSpatializeDither(dL, dR, params);
                break;
            case TAPEDITHER:
                processTapeDither(dL, dR, params);
                break;
            case TPDFDITHER:
                processTPDFDither(dL, dR, params);
                break;
            case VINYLDITHER:
                processVinylDither(dL, dR, params);
                break;
            default:
                break;
        }

        sampleL = (float)dL;
        sampleR = (float)dR;
    }

    // Type-dispatch overload (Phase 2 API)
    void process(float& sampleL, float& sampleR, Type type, const float* params) {
        currentType = type;
        process(sampleL, sampleR, params);
    }

private:
    Type currentType;
    float sampleRate;
    uint32_t fpdL, fpdR;

    // ===== DitherMeDiskers State =====
    double dithermeD_lastSampleL, dithermeD_lastSampleR;
    double dithermeD_lastSample2L, dithermeD_lastSample2R;
    double dithermeD_noiseShapingL, dithermeD_noiseShapingR;

    // ===== DitherMeTimbers State =====
    double dithermeT_lastSampleL, dithermeT_lastSampleR;
    double dithermeT_lastSample2L, dithermeT_lastSample2R;
    double dithermeT_noiseShapingL, dithermeT_noiseShapingR;

    // ===== HighGlossDither State =====
    int highgloss_Position;
    bool highgloss_flip;

    // ===== NaturalizeDither State =====
    int naturalize_bynL[10], naturalize_bynR[10];

    // ===== NodeDither State =====
    double node_A, node_B;
    double node_dL[4999], node_dR[4999];
    int node_gcount;

    // ===== NJAD State =====
    int njad_bynL[9], njad_bynR[9];

    // ===== PaulDither State =====
    double paul_previousDitherL, paul_previousDitherR;

    // ===== SpatializeDither State =====
    double spatialize_lastSampleL, spatialize_lastSampleR;

    // ===== TapeDither State =====
    double tape_lastSampleL, tape_lastSampleR;
    double tape_bufferL[5], tape_bufferR[5];
    int tape_bufpos;

    // ===== TPDFDither State =====
    double tpdf_lastSampleL, tpdf_lastSampleR;

    // ===== VinylDither State =====
    double vinyl_lastSampleL, vinyl_lastSampleR;

    // ===== DitherMeDiskers PROCESSING =====
    void processDitherMeDiskers(double& inputL, double& inputR, const float* params);

    // ===== DitherMeTimbers PROCESSING =====
    void processDitherMeTimbers(double& inputL, double& inputR, const float* params);

    // ===== HighGlossDither PROCESSING =====
    void processHighGlossDither(double& inputL, double& inputR, const float* params);

    // ===== NaturalizeDither PROCESSING =====
    void processNaturalizeDither(double& inputL, double& inputR, const float* params);

    // ===== NodeDither PROCESSING =====
    void processNodeDither(double& inputL, double& inputR, const float* params);

    // ===== NJAD PROCESSING =====
    void processNJAD(double& inputL, double& inputR, const float* params);

    // ===== PaulDither PROCESSING =====
    void processPaulDither(double& inputL, double& inputR, const float* params);

    // ===== SpatializeDither PROCESSING =====
    void processSpatializeDither(double& inputL, double& inputR, const float* params);

    // ===== TapeDither PROCESSING =====
    void processTapeDither(double& inputL, double& inputR, const float* params);

    // ===== TPDFDither PROCESSING =====
    void processTPDFDither(double& inputL, double& inputR, const float* params);

    // ===== VinylDither PROCESSING =====
    void processVinylDither(double& inputL, double& inputR, const float* params);
};

END_NAMESPACE_DISTRHO

#endif // AIRWINDOWS_DITHERS_HPP // AIRWINDOWS_DITHERS_HPP
