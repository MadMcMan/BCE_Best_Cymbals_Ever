#ifndef AIRWINDOWS_COMMON_HPP
#define AIRWINDOWS_COMMON_HPP

#include "DistrhoPlugin.hpp"
#include "DistrhoPluginInfo.h"

START_NAMESPACE_DISTRHO

struct AirwindowsSpec {
    static constexpr int kMaxOptionLabels = 32;
    static constexpr int kMaxParams = 13;
    int count;
    const char* names[kMaxParams];
    float defaults[kMaxParams];
    const char* optionLabels[kMaxParams][kMaxOptionLabels]; // optional discrete text values (null-terminated lists)
    const char* units[kMaxParams]; // optional unit suffixes (dB, ms, Hz, %, st, etc.)

    AirwindowsSpec() : count(0) {
        for (int i = 0; i < kMaxParams; ++i) {
            names[i] = nullptr;
            defaults[i] = 0.5f;
            units[i] = nullptr;
            for (int j = 0; j < kMaxOptionLabels; ++j) {
                optionLabels[i][j] = nullptr;
            }
        }
    }
};

END_NAMESPACE_DISTRHO

#endif // AIRWINDOWS_COMMON_HPP
