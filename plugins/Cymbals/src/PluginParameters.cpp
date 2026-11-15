#include "PluginCymbals.hpp"
#include "SlotOrderPolicy.hpp"
#include "fx/VendoredFxMeta.hpp"
#include <cstdio>
#include <cmath>
#include <algorithm>

START_NAMESPACE_DISTRHO

namespace {

float clampAdvancedEqTypeBucket(float value) {
    const float normalized = std::clamp(value, 0.0f, 1.0f);
    static const float kBuckets[] = {0.0f, 0.2f, 0.4f, 0.6f, 0.8f, 1.0f};
    float best = kBuckets[0];
    float bestDistance = std::abs(normalized - best);
    for (size_t i = 1; i < (sizeof(kBuckets) / sizeof(kBuckets[0])); ++i) {
        const float distance = std::abs(normalized - kBuckets[i]);
        if (distance <= bestDistance) {
            bestDistance = distance;
            best = kBuckets[i];
        }
    }
    return best;
}

float finiteOr(float value, float fallback) {
    return std::isfinite(value) ? value : fallback;
}

} // namespace

/**
 * ===== Parameter Handling Documentation =====
 * 
 * CRITICAL INVARIANTS:
 * 1. All parameters are sequential from kParamTone=0 to kParameterCount-1 with no gaps
 * 2. Array-backed parameters use subtraction arithmetic to calculate offsets
 * 3. All array access is guarded by range checks: if (index >= MIN && index <= MAX)
 * 4. FX chain slot params live at kParamSlot1Type + slot*16 + sub (sub 0=type, 1..13=params,
 *    14=wet, 15=bypass); slot process order is kParamSlotOrder0..5
 * 5. Some legacy params (Advanced EQ, Vib Smooth) remain registered for preset index
 *    stability but have no DSP consumer after the UnifiedFilter migration
 * 
 * HANDLER COVERAGE:
 * - initParameter(): generates name/symbol for all parameters with defaults
 * - getParameterValue(): returns current value for all parameters
 * - setParameterValue(): updates parameter and syncs to DSP objects (dispersion, combBank, etc.)
 * - Each handler is guarded with range checks to prevent array bounds violations
 */

void PluginCymbals::initParameter(uint32_t index, Parameter& parameter) {
    parameter.hints = kParameterIsAutomatable;
    parameter.ranges.min = 0.0f; parameter.ranges.max = 1.0f;
    // Expand string buffers to reduce risk of overflow from long parameter names.
    char name[64], symbol[64]; float defVal = 0.5f;

    if (index >= kParamNudge1 && index <= kParamNudgeAmount20) {
        int i = (index - kParamNudge1) / 2;
        if ((index - kParamNudge1) % 2 == 0) { snprintf(name, sizeof(name), "Nudge %d", i+1); snprintf(symbol, sizeof(symbol), "n1_%d", i+1); defVal = (i<2?(i==0?0.6411f:0.3136f):0.5f); }
        else { snprintf(name, sizeof(name), "Nudge %d Amt", i+1); snprintf(symbol, sizeof(symbol), "n1a_%d", i+1); defVal = (i<2?0.8f:0.0f); }
    } else if (index >= kParamNudge2_1 && index <= kParamNudgeAmount2_20) {
        int i = (index - kParamNudge2_1) / 2;
        if ((index - kParamNudge2_1) % 2 == 0) { snprintf(name, sizeof(name), "Nudge 2-%d", i+1); snprintf(symbol, sizeof(symbol), "n2_%d", i+1); defVal = (i==0?0.45f:0.5f); }
        else { snprintf(name, sizeof(name), "Nudge 2-%d Amt", i+1); snprintf(symbol, sizeof(symbol), "n2a_%d", i+1); defVal = (i==0?0.8f:0.0f); }
    } else if (index >= kParamEQ1Freq && index <= kParamEQ5Type) {
        int band = (index - kParamEQ1Freq) / 4, sub = (index - kParamEQ1Freq) % 4;
        const char* subNames[] = {"Freq", "Gain", "Q", "Type"};
        snprintf(name, sizeof(name), "Stick EQ %d %s", band+1, subNames[sub]); snprintf(symbol, sizeof(symbol), "seq%d_%s", band+1, subNames[sub]);
        if (sub == 0) { // ctor-aligned log spread 200Hz..16kHz (PluginCymbals.cpp)
            static const float kFreqDefs[5] = {0.333f, 0.499f, 0.667f, 0.833f, 0.968f};
            defVal = kFreqDefs[band];
        }
        else if (sub == 2 && (band == 0 || band == 4)) defVal = 0.0404f; // HP/LP endpoints: flat Q=0.5
        else if (sub == 3) {
            parameter.hints |= kParameterIsInteger; parameter.ranges.max = 7.0f;
            if (band == 0) defVal = 2.0f;       // High Pass
            else if (band == 4) defVal = 0.0f;  // Low Pass
            else defVal = 4.0f;                  // Bell
        }
    } else if (index >= kParamFinalEQ1Freq && index <= kParamFinalEQ5Type) {
        int band = (index - kParamFinalEQ1Freq) / 4, sub = (index - kParamFinalEQ1Freq) % 4;
        const char* subNames[] = {"Freq", "Gain", "Q", "Type"};
        snprintf(name, sizeof(name), "Final EQ %d %s", band+1, subNames[sub]); snprintf(symbol, sizeof(symbol), "feq%d_%s", band+1, subNames[sub]);
        if (sub == 0) { // ctor-aligned log spread 200Hz..16kHz (PluginCymbals.cpp)
            static const float kFreqDefs[5] = {0.333f, 0.499f, 0.667f, 0.833f, 0.968f};
            defVal = kFreqDefs[band];
        }
        else if (sub == 2 && (band == 0 || band == 4)) defVal = 0.0404f; // HP/LP endpoints: flat Q=0.5
        else if (sub == 3) {
            parameter.hints |= kParameterIsInteger; parameter.ranges.max = 7.0f;
            if (band == 0) defVal = 2.0f;       // High Pass
            else if (band == 4) defVal = 0.0f;  // Low Pass
            else defVal = 4.0f;                  // Bell
        }
    } else if (index >= kParamEQ1AdvType && index <= kParamEQ3AdvRes) {
        // Phase 3: Advanced EQ filters per band.
        // Type is stored normalized for preset/state consistency, then snapped to compatibility buckets.
        int band = (index - kParamEQ1AdvType) / 2, type = (index - kParamEQ1AdvType) % 2;
        if (type == 0) { snprintf(name, sizeof(name), "EQ %d Adv Type", band+1); snprintf(symbol, sizeof(symbol), "eqa%d_type", band+1); defVal = 0.0f; }
        else { snprintf(name, sizeof(name), "EQ %d Adv Res", band+1); snprintf(symbol, sizeof(symbol), "eqa%d_res", band+1); defVal = 0.5f; }
    } else if (index >= kParamFinalEQ1AdvType && index <= kParamFinalEQ3AdvRes) {
        // Phase 3: Advanced Final EQ filters per band.
        // Type remains normalized and snaps to a compatibility-safe bucket set.
        int band = (index - kParamFinalEQ1AdvType) / 2, type = (index - kParamFinalEQ1AdvType) % 2;
        if (type == 0) { snprintf(name, sizeof(name), "Final EQ %d Adv Type", band+1); snprintf(symbol, sizeof(symbol), "feqa%d_type", band+1); defVal = 0.0f; }
        else { snprintf(name, sizeof(name), "Final EQ %d Adv Res", band+1); snprintf(symbol, sizeof(symbol), "feqa%d_res", band+1); defVal = 0.5f; }
    } else if (index >= kParamEQ1AdvDrive && index <= kParamEQ3AdvCharacter) {
        int band = (index - kParamEQ1AdvDrive) / 2, type = (index - kParamEQ1AdvDrive) % 2;
        if (type == 0) { snprintf(name, sizeof(name), "EQ %d Adv Drive", band+1); snprintf(symbol, sizeof(symbol), "eqa%d_drv", band+1); defVal = 0.0f; }
        else { snprintf(name, sizeof(name), "EQ %d Adv Character", band+1); snprintf(symbol, sizeof(symbol), "eqa%d_chr", band+1); defVal = 0.5f; }
    } else if (index >= kParamFinalEQ1AdvDrive && index <= kParamFinalEQ3AdvCharacter) {
        int band = (index - kParamFinalEQ1AdvDrive) / 2, type = (index - kParamFinalEQ1AdvDrive) % 2;
        if (type == 0) { snprintf(name, sizeof(name), "Final EQ %d Adv Drive", band+1); snprintf(symbol, sizeof(symbol), "feqa%d_drv", band+1); defVal = 0.0f; }
        else { snprintf(name, sizeof(name), "Final EQ %d Adv Character", band+1); snprintf(symbol, sizeof(symbol), "feqa%d_chr", band+1); defVal = 0.5f; }
    } else if (index >= kParamEQ1Model && index <= kParamEQ5Slope) {
        int band = (index - kParamEQ1Model) / 3, sub = (index - kParamEQ1Model) % 3;
        if (sub == 0) { snprintf(name, sizeof(name), "EQ %d Model", band+1); snprintf(symbol, sizeof(symbol), "eq%d_mdl", band+1); defVal = 0.0f; parameter.hints |= kParameterIsInteger; parameter.ranges.max = (float)(UnifiedFilter::Count - 1); }
        else if (sub == 1) {
            snprintf(name, sizeof(name), "EQ %d FilterType", band+1); snprintf(symbol, sizeof(symbol), "eq%d_ft", band+1);
            parameter.hints |= kParameterIsInteger; parameter.ranges.max = 7.0f;
            if (band == 0) defVal = 2.0f;       // High Pass
            else if (band == 4) defVal = 0.0f;  // Low Pass
            else defVal = 4.0f;                  // Bell
        }
        else { snprintf(name, sizeof(name), "EQ %d Slope", band+1); snprintf(symbol, sizeof(symbol), "eq%d_slp", band+1); defVal = 0.0f; parameter.hints |= kParameterIsInteger; parameter.ranges.max = 3.0f; }
    } else if (index >= kParamFinalEQ1Model && index <= kParamFinalEQ5Slope) {
        int band = (index - kParamFinalEQ1Model) / 3, sub = (index - kParamFinalEQ1Model) % 3;
        if (sub == 0) { snprintf(name, sizeof(name), "Final EQ %d Model", band+1); snprintf(symbol, sizeof(symbol), "feq%d_mdl", band+1); defVal = 0.0f; parameter.hints |= kParameterIsInteger; parameter.ranges.max = (float)(UnifiedFilter::Count - 1); }
        else if (sub == 1) {
            snprintf(name, sizeof(name), "Final EQ %d FilterType", band+1); snprintf(symbol, sizeof(symbol), "feq%d_ft", band+1);
            parameter.hints |= kParameterIsInteger; parameter.ranges.max = 7.0f;
            if (band == 0) defVal = 2.0f;       // High Pass
            else if (band == 4) defVal = 0.0f;  // Low Pass
            else defVal = 4.0f;                  // Bell
        }
        else { snprintf(name, sizeof(name), "Final EQ %d Slope", band+1); snprintf(symbol, sizeof(symbol), "feq%d_slp", band+1); defVal = 0.0f; parameter.hints |= kParameterIsInteger; parameter.ranges.max = 3.0f; }
    } else if (index >= kParamEQ1Sat && index <= kParamEQ5Sat) {
        int band = index - kParamEQ1Sat;
        snprintf(name, sizeof(name), "EQ %d Sat", band+1); snprintf(symbol, sizeof(symbol), "eq%d_sat", band+1); defVal = 0.0f;
    } else if (index >= kParamFinalEQ1Sat && index <= kParamFinalEQ5Sat) {
        int band = index - kParamFinalEQ1Sat;
        snprintf(name, sizeof(name), "Final EQ %d Sat", band+1); snprintf(symbol, sizeof(symbol), "feq%d_sat", band+1); defVal = 0.0f;
    } else if (index >= kParamComb1Freq && index <= kParamComb40HPF) {
        int s = (index - kParamComb1Freq) / 4, t = (index - kParamComb1Freq) % 4;
        // Display names: Freq is the comb resonator's TUNE (pitch), not a
        // cutoff. LPF/HPF are the actual Low Cut / High Cut. Symbols stay
        // byte-identical so preset keys and host state remain valid.
        const char* typeNames[] = {"Tune", "Feedback", "Low Cut", "High Cut"};
        const char* typeSyms[] = {"Freq", "FB", "LPF", "HPF"};
        snprintf(name, sizeof(name), "Filter %d %s", s+1, typeNames[t]); snprintf(symbol, sizeof(symbol), "comb%d_%s", s+1, typeSyms[t]);
        if (s < 4) { if (t==0) defVal = 0.3f + (float[]){0.0f, 0.005f, 0.012f, -0.008f}[s]; else if (t==1) defVal = 0.985f; else if (t==2) defVal = 1.0f; else defVal = 0.43f; }
        else if (s < 8) { if (t==0) defVal = (float[]){0.3528f, 0.3789f, 0.4012f, 0.4920f}[s-4]; else if (t==1) defVal = 0.985f; else if (t==2) defVal = 1.0f; else defVal = 0.43f; }
        else { defVal = (t==0?0.5f:(t==2?1.0f:0.0f)); }
    } else if (index >= kParamComb1Scale && index <= kParamComb40Scale) {
        int s = index - kParamComb1Scale; snprintf(name, sizeof(name), "Filter %d Scale", s+1); snprintf(symbol, sizeof(symbol), "c%ds", s+1); defVal = 0.5f;
    } else if (index >= kParamComb1SatType && index <= kParamComb40SatType) {
        int s = index - kParamComb1SatType; snprintf(name, sizeof(name), "Filter %d Sat Type", s+1); snprintf(symbol, sizeof(symbol), "c%dst", s+1); defVal = 0.0f; parameter.hints |= kParameterIsInteger; parameter.ranges.max = 7.0f;
    } else if (index >= kParamComb1FilterType && index <= kParamComb40FilterType) {
        int s = index - kParamComb1FilterType; snprintf(name, sizeof(name), "Filter %d Filter Type", s+1); snprintf(symbol, sizeof(symbol), "c%df", s+1); defVal = 0.0f; parameter.hints |= kParameterIsInteger; parameter.ranges.max = 22.0f;
    } else if (index >= kParamComb1SatAmount && index <= kParamComb40SatAmount) {
        int s = index - kParamComb1SatAmount; snprintf(name, sizeof(name), "Filter %d Sat Amt", s+1); snprintf(symbol, sizeof(symbol), "c%dsa", s+1); defVal = 0.0f;
    } else if (index >= kParamComb1StressOnset && index <= kParamComb40StressOnset) {
        int s = index - kParamComb1StressOnset; snprintf(name, sizeof(name), "Filter %d Stress Onset", s+1); snprintf(symbol, sizeof(symbol), "c%dso", s+1); defVal = 0.86f;
    } else if (index >= kParamComb1StressKnee && index <= kParamComb40StressKnee) {
        int s = index - kParamComb1StressKnee; snprintf(name, sizeof(name), "Filter %d Stress Knee", s+1); snprintf(symbol, sizeof(symbol), "c%dsk", s+1); defVal = 0.995f;
    } else if (index >= kParamComb1Mix && index <= kParamComb40Mix) {
        int s = index - kParamComb1Mix; snprintf(name, sizeof(name), "Filter %d Mix", s+1); snprintf(symbol, sizeof(symbol), "c%dmx", s+1); defVal = 1.0f;
    } else if (index >= kParamComb1Solo && index <= kParamComb40Solo) {
        int s = index - kParamComb1Solo; snprintf(name, sizeof(name), "Filter %d Solo", s+1); snprintf(symbol, sizeof(symbol), "c%dso2", s+1); defVal = 0.0f; parameter.hints |= kParameterIsInteger;
    } else if (index >= kParamComb1Enable && index <= kParamComb40Enable) {
        int s = index - kParamComb1Enable; snprintf(name, sizeof(name), "Filter %d Enable", s+1); snprintf(symbol, sizeof(symbol), "c%den", s+1); defVal = (s < 8) ? 1.0f : 0.0f; parameter.hints |= kParameterIsInteger;
    } else if (index >= kParamCombGroup1Shift && index <= kParamCombGroup10Shift) {
        int g = index - kParamCombGroup1Shift; snprintf(name, sizeof(name), "Group %d Shift", g+1); snprintf(symbol, sizeof(symbol), "g%ds", g+1); defVal = 0.5f;
    } else if (index >= kParamCombGroup1Gain && index <= kParamCombGroup10Gain) {
        int g = index - kParamCombGroup1Gain; snprintf(name, sizeof(name), "Group %d Gain", g+1); snprintf(symbol, sizeof(symbol), "g%dg", g+1); defVal = 0.85f;
    } else if (index >= kParamCombBank1Series && index <= kParamCombBank10Series) {
        int g = index - kParamCombBank1Series; snprintf(name, sizeof(name), "Group %d Series", g+1); snprintf(symbol, sizeof(symbol), "g%dser", g+1); defVal = 0.0f;
    } else if (index >= kParamCombGroup1Cluster && index <= kParamCombGroup10Cluster) {
        int g = index - kParamCombGroup1Cluster; snprintf(name, sizeof(name), "Group %d Cluster", g+1); snprintf(symbol, sizeof(symbol), "g%dcl", g+1); defVal = 0.0f;
    } else if (index >= kParamCombGroup1On && index <= kParamCombGroup10On) {
        int g = index - kParamCombGroup1On; snprintf(name, sizeof(name), "Group %d On", g+1); snprintf(symbol, sizeof(symbol), "g%don", g+1); defVal = 1.0f; parameter.hints |= kParameterIsInteger;
    } else if (index >= kParamCombGroup1LimiterOn && index <= kParamCombGroup10LimiterOn) {
        int g = index - kParamCombGroup1LimiterOn; snprintf(name, sizeof(name), "Group %d Limiter On", g+1); snprintf(symbol, sizeof(symbol), "g%dlon", g+1); defVal = 0.0f; parameter.hints |= kParameterIsInteger;
    } else if (index >= kParamCombGroup1LimiterThreshold && index <= kParamCombGroup10LimiterThreshold) {
        int g = index - kParamCombGroup1LimiterThreshold; snprintf(name, sizeof(name), "Group %d Limiter Threshold", g+1); snprintf(symbol, sizeof(symbol), "g%dlth", g+1); defVal = 0.75f;
    } else if (index >= kParamCombGroup1LimiterRelease && index <= kParamCombGroup10LimiterRelease) {
        int g = index - kParamCombGroup1LimiterRelease; snprintf(name, sizeof(name), "Group %d Limiter Release", g+1); snprintf(symbol, sizeof(symbol), "g%dlrl", g+1); defVal = 0.15f;
    } else if (index >= kParamFinalComb1Freq && index <= kParamFinalComb4HPF) {
        int s = (index - kParamFinalComb1Freq) / 4, t = (index - kParamFinalComb1Freq) % 4;
        static const char* const kFinalNames[4] = {"Tune", "Feedback", "Low Cut", "High Cut"};
        snprintf(name, sizeof(name), "F-Filter %d %s", s+1, kFinalNames[t]);
        snprintf(symbol, sizeof(symbol), "fc%d_%d", s+1, t);
        if (t==0) defVal = 0.4f + s * 0.1f; else if (t==1) defVal = 0.20f; else if (t==2) defVal = 1.00f; else defVal = 0.43f;
    } else if (index >= kParamFinalComb1Mix && index <= kParamFinalComb4Mix) {
        int s = index - kParamFinalComb1Mix; snprintf(name, sizeof(name), "F-Filter %d Mix", s+1); snprintf(symbol, sizeof(symbol), "fc%dmx", s+1); defVal = 1.0f;
    } else if (index >= kParamFinalComb1Solo && index <= kParamFinalComb4Solo) {
        int s = index - kParamFinalComb1Solo; snprintf(name, sizeof(name), "F-Filter %d Solo", s+1); snprintf(symbol, sizeof(symbol), "fc%dsl", s+1); defVal = 0.0f; parameter.hints |= kParameterIsInteger;
    } else if (index >= kParamFinalComb1Enable && index <= kParamFinalComb4Enable) {
        int s = index - kParamFinalComb1Enable; snprintf(name, sizeof(name), "F-Filter %d Enable", s+1); snprintf(symbol, sizeof(symbol), "fc%dfen", s+1); defVal = 1.0f; parameter.hints |= kParameterIsInteger;
    } else if (index >= kParamFinalComb1FilterType && index <= kParamFinalComb4FilterType) {
        int s = index - kParamFinalComb1FilterType; snprintf(name, sizeof(name), "F-Filter %d Filter Type", s+1); snprintf(symbol, sizeof(symbol), "fc%df", s+1); defVal = 0.0f; parameter.hints |= kParameterIsInteger; parameter.ranges.max = 22.0f;
    } else if (index >= kParamComb1ModeRate && index <= kParamComb40ModeRate) {
        int s = index - kParamComb1ModeRate; snprintf(name, sizeof(name), "Filter %d Mode Rate", s+1); snprintf(symbol, sizeof(symbol), "c%dmr", s+1); defVal = 0.5f;
    } else if (index >= kParamComb1ModeAmount && index <= kParamComb40ModeAmount) {
        int s = index - kParamComb1ModeAmount; snprintf(name, sizeof(name), "Filter %d Mode Amt", s+1); snprintf(symbol, sizeof(symbol), "c%dma", s+1); defVal = 0.6f;
    } else if (index >= kParamFinalComb1ModeRate && index <= kParamFinalComb4ModeRate) {
        int s = index - kParamFinalComb1ModeRate; snprintf(name, sizeof(name), "F-Filter %d Mode Rate", s+1); snprintf(symbol, sizeof(symbol), "fc%dmr", s+1); defVal = 0.5f;
    } else if (index >= kParamFinalComb1ModeAmount && index <= kParamFinalComb4ModeAmount) {
        int s = index - kParamFinalComb1ModeAmount; snprintf(name, sizeof(name), "F-Filter %d Mode Amt", s+1); snprintf(symbol, sizeof(symbol), "fc%dma", s+1); defVal = 0.6f;
    } else if (index == kParamFinalCombLimiterOn) {
        strcpy(name, "Post-Filter Limiter On"); strcpy(symbol, "fclon"); defVal = 0.0f;
        parameter.hints |= kParameterIsInteger;
    } else if (index == kParamFinalCombLimiterThreshold) {
        strcpy(name, "Post-Filter Limiter Threshold"); strcpy(symbol, "fclth"); defVal = 0.75f;
    } else if (index == kParamFinalCombLimiterRelease) {
        strcpy(name, "Post-Filter Limiter Release"); strcpy(symbol, "fclrl"); defVal = 0.15f;
    } else if (index >= kParamComb1ModeA && index <= kParamComb40ModeB) {
        const int off = index - kParamComb1ModeA;
        const bool isB = off >= 40;
        const int st = isB ? off - 40 : off;
        snprintf(name, sizeof(name), "Filter %d Mode %c", st + 1, isB ? 'B' : 'A');
        snprintf(symbol, sizeof(symbol), "c%dmda%s", st + 1, isB ? "b" : "");
        defVal = 0.5f;
    } else if (index >= kParamFinalComb1ModeA && index <= kParamFinalComb4ModeB) {
        const int off = index - kParamFinalComb1ModeA;
        const bool isB = off >= 4;
        const int st = isB ? off - 4 : off;
        snprintf(name, sizeof(name), "F-Filter %d Mode %c", st + 1, isB ? 'B' : 'A');
        snprintf(symbol, sizeof(symbol), "fc%dmda%s", st + 1, isB ? "b" : "");
        defVal = 0.5f;
    } else if (index >= kParamComb1PartRelPos && index <= kParamComb40PartRelPos) {
        int s = index - kParamComb1PartRelPos; snprintf(name, sizeof(name), "Filter %d Rel Pos", s+1); snprintf(symbol, sizeof(symbol), "c%drp", s+1); defVal = (0.1f - 0.0001f) / (0.999f - 0.0001f);
    } else if (index >= kParamComb1PartFreqDamp && index <= kParamComb40PartFreqDamp) {
        int s = index - kParamComb1PartFreqDamp; snprintf(name, sizeof(name), "Filter %d Freq Damp", s+1); snprintf(symbol, sizeof(symbol), "c%dfd", s+1); defVal = 0.5f;
    } else if (index >= kParamComb1PartStiffness && index <= kParamComb40PartStiffness) {
        int s = index - kParamComb1PartStiffness; snprintf(name, sizeof(name), "Filter %d Stiffness", s+1); snprintf(symbol, sizeof(symbol), "c%dstf", s+1); defVal = 0.65f;
    } else if (index >= kParamComb1PartStiffExp && index <= kParamComb40PartStiffExp) {
        int s = index - kParamComb1PartStiffExp; snprintf(name, sizeof(name), "Filter %d Stiff Exp", s+1); snprintf(symbol, sizeof(symbol), "c%dse", s+1); defVal = 0.65f;
    } else if (index >= kParamComb1PartCount && index <= kParamComb40PartCount) {
        int s = index - kParamComb1PartCount; snprintf(name, sizeof(name), "Filter %d Partials", s+1); snprintf(symbol, sizeof(symbol), "c%dnp", s+1); defVal = 1.0f/3.0f;
    } else if (index >= kParamFinalComb1PartRelPos && index <= kParamFinalComb4PartRelPos) {
        int s = index - kParamFinalComb1PartRelPos; snprintf(name, sizeof(name), "F-Filter %d Rel Pos", s+1); snprintf(symbol, sizeof(symbol), "fc%drp", s+1); defVal = (0.1f - 0.0001f) / (0.999f - 0.0001f);
    } else if (index >= kParamFinalComb1PartFreqDamp && index <= kParamFinalComb4PartFreqDamp) {
        int s = index - kParamFinalComb1PartFreqDamp; snprintf(name, sizeof(name), "F-Filter %d Freq Damp", s+1); snprintf(symbol, sizeof(symbol), "fc%dfd", s+1); defVal = 0.5f;
    } else if (index >= kParamFinalComb1PartStiffness && index <= kParamFinalComb4PartStiffness) {
        int s = index - kParamFinalComb1PartStiffness; snprintf(name, sizeof(name), "F-Filter %d Stiffness", s+1); snprintf(symbol, sizeof(symbol), "fc%dstf", s+1); defVal = 0.65f;
    } else if (index >= kParamFinalComb1PartStiffExp && index <= kParamFinalComb4PartStiffExp) {
        int s = index - kParamFinalComb1PartStiffExp; snprintf(name, sizeof(name), "F-Filter %d Stiff Exp", s+1); snprintf(symbol, sizeof(symbol), "fc%dse", s+1); defVal = 0.65f;
    } else if (index >= kParamFinalComb1PartCount && index <= kParamFinalComb4PartCount) {
        int s = index - kParamFinalComb1PartCount; snprintf(name, sizeof(name), "F-Filter %d Partials", s+1); snprintf(symbol, sizeof(symbol), "fc%dnp", s+1); defVal = 1.0f/3.0f;
    } else if (index >= kParamSlot1Type && index <= kParamSlot6Bypass) {
        const int slot = (index - kParamSlot1Type) / 16 + 1; // 1-indexed for display
        const int sub  = (index - kParamSlot1Type) % 16;
        if (sub == 0) {
            snprintf(name, sizeof(name), "FX Slot %d Type", slot);
            snprintf(symbol, sizeof(symbol), "fxs_%d_type", slot);
            parameter.hints |= kParameterIsInteger;
            defVal = 0.0f;
            parameter.ranges.min = 0.0f;
            parameter.ranges.max = (float)std::max(
                CymbalsSurgeFx::kBaseType + CymbalsSurgeFx::algorithmCount() - 1,
                std::max(CymbalsInternalFx::kBaseType + CymbalsInternalFx::kCount - 1,
                         CymbalsVendoredFx::kBaseType + CymbalsVendoredFx::kCount - 1));
        } else if (sub >= 1 && sub <= 13) {
            const int k = sub;
            snprintf(name, sizeof(name), "FX Slot %d Param %d", slot, k);
            snprintf(symbol, sizeof(symbol), "fxs_%d_p%d", slot, k);
            defVal = 0.5f;
            parameter.ranges.min = 0.0f;
            parameter.ranges.max = 1.0f;
        } else if (sub == 14) {
            snprintf(name, sizeof(name), "FX Slot %d Wet", slot);
            snprintf(symbol, sizeof(symbol), "fxs_%d_wet", slot);
            defVal = 1.0f;
            parameter.ranges.min = 0.0f;
            parameter.ranges.max = 1.0f;
        } else { // sub == 15
            snprintf(name, sizeof(name), "FX Slot %d Bypass", slot);
            snprintf(symbol, sizeof(symbol), "fxs_%d_byp", slot);
            parameter.hints |= kParameterIsInteger;
            defVal = 0.0f;
            parameter.ranges.min = 0.0f;
            parameter.ranges.max = 1.0f;
        }
    } else if (index >= kParamSlotOrder0 && index <= kParamSlotOrder5) {
        // Phase 8: Hidden slot order parameters
        int pos = index - kParamSlotOrder0;
        snprintf(name, sizeof(name), "Slot Order %d", pos);
        snprintf(symbol, sizeof(symbol), "slot_order_%d", pos);
        parameter.hints = kParameterIsHidden | kParameterIsInteger;
        defVal = (float)pos;
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 5.0f;
    } else if (index >= kParamModSrc1Type && index <= kParamModSrc11StepSmooth) {
        // ===== Modulation System: sources (11 × 66) =====
        const int s = (index - kParamModSrc1Type) / ModulationEngine::kSourceStride;
        const int sub = (index - kParamModSrc1Type) % ModulationEngine::kSourceStride;
        static const char* kSubNames[66] = {
            "Type", "Enable", "Bipolar", "Retrig", "Rate", "Shape", "Phase", "Smooth",
            "Sync", "Division", "Attack", "Decay", "Sustain", "Release", "Duration",
            "Loop", "SustainPt",
            "P0 V", "P1 V", "P2 V", "P3 V", "P4 V", "P5 V", "P6 V", "P7 V",
            "P1 T", "P2 T", "P3 T", "P4 T", "P5 T", "P6 T",
            "P0 C", "P1 C", "P2 C", "P3 C", "P4 C", "P5 C", "P6 C",
            "S0", "S1", "S2", "S3", "S4", "S5", "S6", "S7", "S8", "S9",
            "S10", "S11", "S12", "S13", "S14", "S15", "StepCount", "StepLoop", "StepSmooth",
            "LFO Mode", "Fade", "Delay", "Quantize", "Env Delay", "Hold", "A Curve", "D Curve", "R Curve"
        };
        static const char* kSubSymbols[66] = {
            "type", "en", "bip", "rtg", "rate", "shape", "phase", "smooth",
            "sync", "div", "a", "d", "s", "r", "dur", "loop", "sus",
            "p0", "p1", "p2", "p3", "p4", "p5", "p6", "p7",
            "t1", "t2", "t3", "t4", "t5", "t6",
            "c0", "c1", "c2", "c3", "c4", "c5", "c6",
            "st0", "st1", "st2", "st3", "st4", "st5", "st6", "st7", "st8", "st9",
            "st10", "st11", "st12", "st13", "st14", "st15", "n", "sl", "ss",
            "mode", "fade", "dly", "qnt", "edly", "hold", "acrv", "dcrv", "rcrv"
        };
        snprintf(name, sizeof(name), "Mod %d %s", s + 1, kSubNames[sub]);
        snprintf(symbol, sizeof(symbol), "mod_s%d_%s", s + 1, kSubSymbols[sub]);
        switch (sub) {
            case 0:  parameter.hints |= kParameterIsInteger; parameter.ranges.max = 9.0f; defVal = 0.0f; break;   // type
            case 1:  defVal = 0.0f; break;                                                                        // enabled
            case 2:  defVal = 0.0f; break;                                                                        // bipolar
            case 3:  defVal = 1.0f; break;                                                                        // retrig
            case 4:  defVal = 0.5f; break;                                                                        // rate
            case 5:  parameter.hints |= kParameterIsInteger; parameter.ranges.max = 5.0f; defVal = 0.0f; break;   // shape
            case 6:  defVal = 0.0f; break;                                                                        // phase
            case 7:  defVal = 0.0f; break;                                                                        // smooth
            case 8:  defVal = 0.0f; break;                                                                        // sync
            case 9:  parameter.hints |= kParameterIsInteger; parameter.ranges.max = (float)(ModulationEngine::kNumDivisions - 1); defVal = 4.0f; break; // division
            case 10: case 11: case 12: case 13: defVal = 0.3f; break;                                            // a d s r
            case 14: defVal = 0.25f; break;                                                                       // dur
            case 15: defVal = 0.0f; break;                                                                        // loop
            case 16: parameter.hints |= kParameterIsInteger; parameter.ranges.max = 8.0f; defVal = 0.0f; break;   // sustain
            case 17: case 18: case 19: case 20: case 21: case 22: case 23: case 24: defVal = 0.5f; break;         // v0..v7
            case 25: case 26: case 27: case 28: case 29: case 30: defVal = (sub - 24) / 7.0f; break;              // t1..t6 = k/7
            case 31: case 32: case 33: case 34: case 35: case 36: case 37: defVal = 0.5f; break;                  // c0..c6
            case 38: case 39: case 40: case 41: case 42: case 43: case 44: case 45:
            case 46: case 47: case 48: case 49: case 50: case 51: case 52: case 53: defVal = 0.5f; break;         // s0..s15
            case 54: parameter.hints |= kParameterIsInteger; parameter.ranges.min = 1.0f; parameter.ranges.max = 16.0f; defVal = 8.0f; break; // stepCount
            case 55: defVal = 1.0f; break;                                                                        // stepLoop
            case 56: defVal = 0.0f; break;                                                                        // stepSmooth
            case 57: parameter.hints |= kParameterIsInteger; parameter.ranges.max = 2.0f; defVal = 0.0f; break;   // LFO mode
            case 58: case 59: case 60: case 61: case 62: defVal = 0.0f; break;                                   // fade dly qnt edly hold
            case 63: case 64: case 65: defVal = 0.5f; break;                                                      // acrv dcrv rcrv
        }
    } else if (index >= kParamRoute1Source && index <= kParamRoute48Enabled) {
        // ===== Modulation System: routes (48 × 5) =====
        const int r = (index - kParamRoute1Source) / 5;
        const int sub = (index - kParamRoute1Source) % 5;
        static const char* kRouteNames[5] = {"Source", "Target", "Amount", "Macro", "Enable"};
        static const char* kRouteSymbols[5] = {"src", "tgt", "amt", "mac", "en"};
        snprintf(name, sizeof(name), "Route %d %s", r + 1, kRouteNames[sub]);
        snprintf(symbol, sizeof(symbol), "mod_r%d_%s", r + 1, kRouteSymbols[sub]);
        switch (sub) {
            case 0:  parameter.hints |= kParameterIsInteger; parameter.ranges.max = 14.0f; defVal = 0.0f; break; // 0..10 sources, 11..14 macros
            case 1:  parameter.hints |= kParameterIsInteger; parameter.ranges.max = 4095.0f; defVal = 0.0f; break;
            case 2:  defVal = 0.5f; break;
            case 3:  parameter.hints |= kParameterIsInteger; parameter.ranges.max = 4.0f; defVal = 0.0f; break;
            default: defVal = 0.0f; break;
        }
    } else if (index >= kParamMacro1 && index <= kParamMacro4) {
        // ===== Modulation System: macros =====
        const int m = index - kParamMacro1;
        snprintf(name, sizeof(name), "Macro %d", m + 1);
        snprintf(symbol, sizeof(symbol), "macro_%d", m + 1);
        defVal = 0.5f;
    } else if (index >= kParamVoiceMode && index <= kParamBowLevel) {
        // ===== Voice architecture / MIDI foundation (contract v1 tail params) =====
        switch (index) {
            case kParamVoiceMode: strcpy(name, "Voice Mode"); strcpy(symbol, "voice_mode"); defVal = 0.0f; parameter.hints |= kParameterIsInteger; parameter.ranges.max = 1.0f; break;
            case kParamVoiceCount: strcpy(name, "Voice Count"); strcpy(symbol, "voice_count"); defVal = 8.0f; parameter.hints |= kParameterIsInteger; parameter.ranges.min = 2.0f; parameter.ranges.max = 16.0f; break;
            case kParamChokeMode: strcpy(name, "Choke Mode"); strcpy(symbol, "choke_mode"); defVal = 0.0f; parameter.hints |= kParameterIsInteger; parameter.ranges.max = 2.0f; break;
            case kParamChokeSplitPoint: strcpy(name, "Choke Split Note"); strcpy(symbol, "choke_split"); defVal = 60.0f; parameter.hints |= kParameterIsInteger; parameter.ranges.max = 127.0f; break;
            case kParamRelVelSens: strcpy(name, "Release Vel Sens"); strcpy(symbol, "relvel"); defVal = 0.0f; break;
            case kParamMpeMode: strcpy(name, "MPE Mode"); strcpy(symbol, "mpe_mode"); defVal = 0.0f; parameter.hints |= kParameterIsInteger; parameter.ranges.max = 1.0f; break;
            case kParamMpeBendAmount: strcpy(name, "MPE Bend Amt"); strcpy(symbol, "mpe_bend"); defVal = 0.0f; break;
            case kParamMpePressDrive: strcpy(name, "MPE Press Drive"); strcpy(symbol, "mpe_press"); defVal = 0.0f; break;
            case kParamBowEnable: strcpy(name, "Bow Enable"); strcpy(symbol, "bow_enable"); defVal = 0.0f; parameter.hints |= kParameterIsInteger; parameter.ranges.max = 1.0f; break;
            case kParamBowRate: strcpy(name, "Bow Rate"); strcpy(symbol, "bow_rate"); defVal = 0.5f; break;
            case kParamBowLevel: strcpy(name, "Bow Level"); strcpy(symbol, "bow_level"); defVal = 0.5f; break;
        }
    } else if (index >= kParamComb1Q && index <= kParamComb40Q) {
        const int c = (int)(index - kParamComb1Q) + 1;
        snprintf(name, sizeof(name), "Comb %d Q", c);
        snprintf(symbol, sizeof(symbol), "cq_%d", c);
        defVal = 0.0f; // AUTO: preserve the existing feedback-derived Q
    } else if (index >= kParamFinalComb1Q && index <= kParamFinalComb4Q) {
        const int c = (int)(index - kParamFinalComb1Q) + 1;
        snprintf(name, sizeof(name), "Final Filter %d Q", c);
        snprintf(symbol, sizeof(symbol), "fcq_%d", c);
        defVal = 0.0f;
    } else {
        switch (index) {
            case kParamTone: strcpy(name, "Tone"); strcpy(symbol, "tone"); defVal = 0.15f; break;
            case kParamDecay: strcpy(name, "Decay"); strcpy(symbol, "decay"); defVal = 0.85f; break;
            case kParamVolume: strcpy(name, "Volume"); strcpy(symbol, "volume"); defVal = 0.85f; break;
            case kParamDispFB: strcpy(name, "Disp FB"); strcpy(symbol, "dfb"); defVal = 0.99990785f; break;
            case kParamDispDelay: strcpy(name, "Disp Delay"); strcpy(symbol, "ddly"); defVal = 718.0f/2000.0f; break;
            case kParamDispGain: strcpy(name, "Disp Gain"); strcpy(symbol, "dgain"); defVal = 1.0f; break;
            case kParamDispFBOverall: strcpy(name, "Disp FB Max"); strcpy(symbol, "dfbm"); defVal = 1.0f; break;
            case kParamStickType: strcpy(name, "Stick Type"); strcpy(symbol, "styp"); defVal = 0.0f; parameter.hints |= kParameterIsInteger; parameter.ranges.max = 9.0f; break;
            case kParamStickDirect: strcpy(name, "Stick Direct"); strcpy(symbol, "sdir"); defVal = 0.35f; break;
            case kParamStickBus: strcpy(name, "Stick Bus"); strcpy(symbol, "sbus"); defVal = 1.0f; break;
            case kParamStickAmpAttack: strcpy(name, "Stick Amp A"); strcpy(symbol, "saa"); defVal = 0.001f; break;
            case kParamStickAmpDecay: strcpy(name, "Stick Amp D"); strcpy(symbol, "sad"); defVal = 0.37f; break;
            case kParamStickAmpSustain: strcpy(name, "Stick Amp S"); strcpy(symbol, "sas"); defVal = 0.0f; break;
            case kParamStickAmpRelease: strcpy(name, "Stick Amp R"); strcpy(symbol, "sar"); defVal = 0.01f; break;
            case kParamStickPitchAttack: strcpy(name, "Stick Pitch A"); strcpy(symbol, "spa"); defVal = 0.001f; break;
            case kParamStickPitchDecay: strcpy(name, "Stick Pitch D"); strcpy(symbol, "spd"); defVal = 0.1f; break;
            case kParamStickPitchSustain: strcpy(name, "Stick Pitch S"); strcpy(symbol, "sps"); defVal = 0.0f; break;
            case kParamStickPitchRelease: strcpy(name, "Stick Pitch R"); strcpy(symbol, "spr"); defVal = 0.1f; break;
            case kParamStickPitchAmount: strcpy(name, "Stick Pitch Amt"); strcpy(symbol, "spam"); defVal = 1.0f; break;
            case kParamStickPitchTimer: strcpy(name, "Stick Pitch Time"); strcpy(symbol, "sptm"); defVal = 0.05f; break;
            case kParamStickPulseWidth: strcpy(name, "Stick Pulse Width"); strcpy(symbol, "spw"); defVal = 0.25f; break;
            case kParamStickNoiseColor: strcpy(name, "Stick Noise Color"); strcpy(symbol, "snc"); defVal = 0.0f; break;
            case kParamStickMetallicSpread: strcpy(name, "Stick Metal Spread"); strcpy(symbol, "sms"); defVal = 0.5f; break;
            case kParamStickNoiseMetalBlend: strcpy(name, "Stick Noise/Metal"); strcpy(symbol, "snmb"); defVal = 0.5f; break;
            case kParamStickHatTone: strcpy(name, "Stick Hat Tone"); strcpy(symbol, "sht"); defVal = 0.5f; break;
            case kParamStickSquareWidth: strcpy(name, "Stick Square Width"); strcpy(symbol, "ssw"); defVal = 0.5f; break;
            case kParamStickSawCurve: strcpy(name, "Stick Saw Curve"); strcpy(symbol, "ssc"); defVal = 0.5f; break;
            case kParamStickTriangleSkew: strcpy(name, "Stick Triangle Skew"); strcpy(symbol, "sts"); defVal = 0.5f; break;
            case kParamStickSineDrive: strcpy(name, "Stick Sine Drive"); strcpy(symbol, "ssd"); defVal = 0.0f; break;
            case kParamStickPhase: strcpy(name, "Stick Phase"); strcpy(symbol, "sph"); defVal = 0.5f; break;
            case kParamStickGrit: strcpy(name, "Stick Grit"); strcpy(symbol, "sgr"); defVal = 0.0f; break;
            case kParamStickUnison: strcpy(name, "Stick Unison"); strcpy(symbol, "sun"); defVal = 0.0f; parameter.hints |= kParameterIsInteger; parameter.ranges.max = 3.0f; break;
            case kParamStickDetune: strcpy(name, "Stick Detune"); strcpy(symbol, "sdt"); defVal = 0.0f; break;
            case kParamStickAmpDecayCurve: strcpy(name, "Amp Decay Curve"); strcpy(symbol, "amp_dcrv"); defVal = 0.5f; break;
            case kParamStickAmpReleaseCurve: strcpy(name, "Amp Release Curve"); strcpy(symbol, "amp_rcrv"); defVal = 0.5f; break;
            case kParamStickPitchDecayCurve: strcpy(name, "Pitch Decay Curve"); strcpy(symbol, "pitch_dcrv"); defVal = 0.5f; break;
            case kParamStickPitchReleaseCurve: strcpy(name, "Pitch Release Curve"); strcpy(symbol, "pitch_rcrv"); defVal = 0.5f; break;
            case kParamFinalARDecayCurve: strcpy(name, "Final Decay Curve"); strcpy(symbol, "far_dcrv"); defVal = 0.5f; break;
            case kParamFinalARReleaseCurve: strcpy(name, "Final Release Curve"); strcpy(symbol, "far_rcrv"); defVal = 0.5f; break;
            case kParamMasterARDecayCurve: strcpy(name, "Master Decay Curve"); strcpy(symbol, "mar_dcrv"); defVal = 0.5f; break;
            case kParamMasterARReleaseCurve: strcpy(name, "Master Release Curve"); strcpy(symbol, "mar_rcrv"); defVal = 0.5f; break;
            case kParamVibRate: strcpy(name, "Vib Rate"); strcpy(symbol, "vbr"); defVal = 0.25075f; break;
            case kParamVibDepth: strcpy(name, "Vib Depth"); strcpy(symbol, "vbd"); defVal = 0.18f; break;
            case kParamVibShape: strcpy(name, "Vib Shape"); strcpy(symbol, "vbs"); defVal = 1.0f; parameter.hints |= kParameterIsInteger; parameter.ranges.max = 3.0f; break;
            case kParamVibSmooth: strcpy(name, "Vib Smooth"); strcpy(symbol, "vbsm"); defVal = 0.672f; break;
            case kParamVibStepMix: strcpy(name, "Vib Step Mix"); strcpy(symbol, "vbsx"); defVal = 1.0f; break;
            case kParamChaos: strcpy(name, "Chaos"); strcpy(symbol, "chs"); defVal = 0.0f; break;
            case kParamChaosEnvAmount: strcpy(name, "Chaos Env Amt"); strcpy(symbol, "cea"); defVal = 0.0f; break;
            case kParamChaosEnvDecay: strcpy(name, "Chaos Env Decay"); strcpy(symbol, "ced"); defVal = 0.1f; break;
            case kParamPDDrive: strcpy(name, "PD Drive"); strcpy(symbol, "pdd"); defVal = 0.72f; break;
            case kParamPDNormalize: strcpy(name, "PD Norm"); strcpy(symbol, "pdn"); defVal = 0.5f; break;
            case kParamPDTone: strcpy(name, "PD Tone"); strcpy(symbol, "pdt"); defVal = 0.598f; break;
            case kParamPDBias: strcpy(name, "PD Bias"); strcpy(symbol, "pdb"); defVal = 0.5f; break;
            case kParamPDSpread: strcpy(name, "PD Spread"); strcpy(symbol, "pds"); defVal = 0.0f; break;
            case kParamPDMix: strcpy(name, "PD Mix"); strcpy(symbol, "pdm"); defVal = 1.0f; break;
            case kParamEQMix: strcpy(name, "EQ Mix"); strcpy(symbol, "emx"); defVal = 1.0f; break;
            case kParamTiltAmount: strcpy(name, "EQ Tilt"); strcpy(symbol, "etlt"); defVal = 0.5f; break;
            case kParamTiltPivot: strcpy(name, "EQ Tilt Pivot"); strcpy(symbol, "etlp"); defVal = 2.0f/3.0f; break;
            case kParamCombScale: strcpy(name, "Filter Scale"); strcpy(symbol, "cscl"); defVal = 0.5f; break;
            case kParamDisp1Skip: strcpy(name, "Disp 1 Skip"); strcpy(symbol, "d1s"); defVal = 0.0f; break;
            case kParamDisp2Skip: strcpy(name, "Disp 2 Skip"); strcpy(symbol, "d2s"); defVal = 1.0f; break;
            case kParamDispCrossLink: strcpy(name, "Disp Cross"); strcpy(symbol, "dcx"); defVal = 0.0f; break;
            case kParamDispSolo: strcpy(name, "Disp Solo"); strcpy(symbol, "dsolo"); defVal = 0.0f; break;
            case kParamDispDelayMode: strcpy(name, "Disp Delay Mode"); strcpy(symbol, "ddm"); defVal = 0.0f; parameter.hints |= kParameterIsInteger; parameter.ranges.max = 2.0f; break;
            case kParamDisp2DelayMode: strcpy(name, "Disp 2 Delay Mode"); strcpy(symbol, "d2dm"); defVal = 0.0f; parameter.hints |= kParameterIsInteger; parameter.ranges.max = 2.0f; break;
            case kParamInharmBloomAmt: strcpy(name, "Inharm Bloom Amt"); strcpy(symbol, "ibam"); defVal = 0.12f; break;
            case kParamInharmBloomTime: strcpy(name, "Inharm Bloom Time"); strcpy(symbol, "ibtm"); defVal = 0.10f; break;
            case kParamInharmBloomDir: strcpy(name, "Inharm Bloom Dir"); strcpy(symbol, "ibdr"); defVal = 0.0f; parameter.hints |= kParameterIsInteger; parameter.ranges.max = 1.0f; break;
            case kParamInharm2BloomAmt: strcpy(name, "Disp 2 Inharm Amt"); strcpy(symbol, "d2iba"); defVal = 0.0f; break;
            case kParamInharm2BloomTime: strcpy(name, "Disp 2 Inharm Time"); strcpy(symbol, "d2ibt"); defVal = 0.10f; break;
            case kParamInharm2BloomDir: strcpy(name, "Disp 2 Inharm Dir"); strcpy(symbol, "d2ibd"); defVal = 0.0f; parameter.hints |= kParameterIsInteger; parameter.ranges.max = 1.0f; break;
            case kParamResonatorDiffusion: strcpy(name, "Resonator Diffusion"); strcpy(symbol, "rdf"); defVal = 0.0f; break;
            case kParamCombMemoryDamping: strcpy(name, "Filter Memory Damp"); strcpy(symbol, "cmdp"); defVal = 0.0f; break;
            case kParamDisp2FB: strcpy(name, "Disp 2 FB"); strcpy(symbol, "d2fb"); defVal = 0.99990785f; break;
            case kParamDisp2Delay: strcpy(name, "Disp 2 Delay"); strcpy(symbol, "d2dy"); defVal = 0.15f; break;
            case kParamDisp2Gain: strcpy(name, "Disp 2 Gain"); strcpy(symbol, "d2g"); defVal = 0.25f; break;
            case kParamDisp2FBOverall: strcpy(name, "Disp 2 FB Max"); strcpy(symbol, "d2bm"); defVal = 1.0f; break;
            case kParamVib2Rate: strcpy(name, "Vib 2 Rate"); strcpy(symbol, "v2r"); defVal = 0.3f; break;
            case kParamVib2Depth: strcpy(name, "Vib 2 Depth"); strcpy(symbol, "v2d"); defVal = 0.2f; break;
            case kParamVib2Shape: strcpy(name, "Vib 2 Shape"); strcpy(symbol, "v2s"); defVal = 1.0f; parameter.hints |= kParameterIsInteger; parameter.ranges.max = 3.0f; break;
            case kParamVib2Smooth: strcpy(name, "Vib 2 Smooth"); strcpy(symbol, "v2sm"); defVal = 0.5f; break;
            case kParamVib2StepMix: strcpy(name, "Vib 2 SMix"); strcpy(symbol, "v2sx"); defVal = 1.0f; break;
            case kParamChaos2: strcpy(name, "Chaos 2"); strcpy(symbol, "c2"); defVal = 0.0f; break;
            case kParamChaos2EnvAmount: strcpy(name, "Chaos 2 EAmt"); strcpy(symbol, "c2ea"); defVal = 0.0f; break;
            case kParamChaos2EnvDecay: strcpy(name, "Chaos 2 EDec"); strcpy(symbol, "c2ed"); defVal = 0.1f; break;
            case kParamPD2Drive: strcpy(name, "PD 2 Drive"); strcpy(symbol, "p2d"); defVal = 0.5f; break;
            case kParamPD2Normalize: strcpy(name, "PD 2 Norm"); strcpy(symbol, "p2n"); defVal = 0.5f; break;
            case kParamPD2Tone: strcpy(name, "PD 2 Tone"); strcpy(symbol, "p2t"); defVal = 0.5f; break;
            case kParamPD2Bias: strcpy(name, "PD 2 Bias"); strcpy(symbol, "p2b"); defVal = 0.5f; break;
            case kParamPD2Spread: strcpy(name, "PD 2 Spread"); strcpy(symbol, "p2s"); defVal = 0.0f; break;
            case kParamPD2Mix: strcpy(name, "PD 2 Mix"); strcpy(symbol, "p2m"); defVal = 1.0f; break;
            case kParamStickPitchMsegOn: strcpy(name, "Stick Pitch MSEG On"); strcpy(symbol, "spmo"); defVal = 1.0f; break;
            case kParamStickPitchMsegDuration: strcpy(name, "Stick Pitch MSEG Dur"); strcpy(symbol, "spmd"); defVal = 0.1f; break;
            case kParamStickPitchMsegPoint0Value: strcpy(name, "S-Pitch MSEG P0 V"); strcpy(symbol, "spmv0"); defVal = 1.0f; break;
            case kParamStickPitchMsegPoint1Value: strcpy(name, "S-Pitch MSEG P1 V"); strcpy(symbol, "spmv1"); defVal = 0.8f; break;
            case kParamStickPitchMsegPoint2Value: strcpy(name, "S-Pitch MSEG P2 V"); strcpy(symbol, "spmv2"); defVal = 0.5f; break;
            case kParamStickPitchMsegPoint3Value: strcpy(name, "S-Pitch MSEG P3 V"); strcpy(symbol, "spmv3"); defVal = 0.0f; break;
            case kParamStickPitchMsegPoint4Value: strcpy(name, "S-Pitch MSEG P4 V"); strcpy(symbol, "spmv4"); defVal = 0.0f; break;
            case kParamStickPitchMsegPoint5Value: strcpy(name, "S-Pitch MSEG P5 V"); strcpy(symbol, "spmv5"); defVal = 0.0f; break;
            case kParamStickPitchMsegPoint6Value: strcpy(name, "S-Pitch MSEG P6 V"); strcpy(symbol, "spmv6"); defVal = 0.0f; break;
            case kParamStickPitchMsegPoint7Value: strcpy(name, "S-Pitch MSEG P7 V"); strcpy(symbol, "spmv7"); defVal = 0.0f; break;
            case kParamStickPitchMsegPoint1Time: strcpy(name, "S-Pitch MSEG P1 T"); strcpy(symbol, "spmt1"); defVal = 0.02f; break;
            case kParamStickPitchMsegPoint2Time: strcpy(name, "S-Pitch MSEG P2 T"); strcpy(symbol, "spmt2"); defVal = 0.10f; break;
            case kParamStickPitchMsegPoint3Time: strcpy(name, "S-Pitch MSEG P3 T"); strcpy(symbol, "spmt3"); defVal = 3.0f/7.0f; break;
            case kParamStickPitchMsegPoint4Time: strcpy(name, "S-Pitch MSEG P4 T"); strcpy(symbol, "spmt4"); defVal = 4.0f/7.0f; break;
            case kParamStickPitchMsegPoint5Time: strcpy(name, "S-Pitch MSEG P5 T"); strcpy(symbol, "spmt5"); defVal = 5.0f/7.0f; break;
            case kParamStickPitchMsegPoint6Time: strcpy(name, "S-Pitch MSEG P6 T"); strcpy(symbol, "spmt6"); defVal = 6.0f/7.0f; break;
            case kParamStickPitchMsegPoint0Curve: strcpy(name, "S-Pitch MSEG P0 C"); strcpy(symbol, "spmc0"); defVal = 0.50f; break;
            case kParamStickPitchMsegPoint1Curve: strcpy(name, "S-Pitch MSEG P1 C"); strcpy(symbol, "spmc1"); defVal = 0.50f; break;
            case kParamStickPitchMsegPoint2Curve: strcpy(name, "S-Pitch MSEG P2 C"); strcpy(symbol, "spmc2"); defVal = 0.50f; break;
            case kParamStickPitchMsegPoint3Curve: strcpy(name, "S-Pitch MSEG P3 C"); strcpy(symbol, "spmc3"); defVal = 0.50f; break;
            case kParamStickPitchMsegPoint4Curve: strcpy(name, "S-Pitch MSEG P4 C"); strcpy(symbol, "spmc4"); defVal = 0.50f; break;
            case kParamStickPitchMsegPoint5Curve: strcpy(name, "S-Pitch MSEG P5 C"); strcpy(symbol, "spmc5"); defVal = 0.50f; break;
            case kParamStickPitchMsegPoint6Curve: strcpy(name, "S-Pitch MSEG P6 C"); strcpy(symbol, "spmc6"); defVal = 0.50f; break;
            case kParamCombBankOn: strcpy(name, "Filter Bank On"); strcpy(symbol, "cbon"); defVal = 1.0f; parameter.hints |= kParameterIsInteger; break;
            case kParamFinalCombBankOn: strcpy(name, "Final Filter On"); strcpy(symbol, "fcbon"); defVal = 1.0f; parameter.hints |= kParameterIsInteger; break;
            case kParamFinalCombShift: strcpy(name, "F-Filter Shift"); strcpy(symbol, "fcs"); defVal = 0.5f; break;
            case kParamFinalCombScale: strcpy(name, "F-Filter Scale"); strcpy(symbol, "fcc"); defVal = 0.5f; break;
            case kParamFinalCombBankSeries: strcpy(name, "F-Filter Series"); strcpy(symbol, "fcsr"); defVal = 0.0f; break;
            case kParamFinalCombGroupGain: strcpy(name, "F-Filter Gain"); strcpy(symbol, "fcgn"); defVal = 0.85f; break;
            case kParamFinalCombGroupCluster: strcpy(name, "F-Filter Cluster"); strcpy(symbol, "fccl"); defVal = 0.0f; break;
            case kParamFinalComb4SatType: strcpy(name, "F-Filter 4 Sat Type"); strcpy(symbol, "fc4st"); defVal = 0.0f; parameter.hints |= kParameterIsInteger; parameter.ranges.max = 7.0f; break;
            case kParamFinalComb4SatAmount: strcpy(name, "F-Filter 4 Sat Amt"); strcpy(symbol, "fc4sa"); defVal = 0.0f; break;
            case kParamFinalComb4StressOnset: strcpy(name, "F-Filter 4 Stress On"); strcpy(symbol, "fc4so"); defVal = 0.86f; break;
            case kParamFinalComb4StressKnee: strcpy(name, "F-Filter 4 Stress Kn"); strcpy(symbol, "fc4sk"); defVal = 0.995f; break;
            case kParamFinalEQMix: strcpy(name, "Final EQ Mix"); strcpy(symbol, "fmx"); defVal = 1.0f; break;
            case kParamFinalEQOn: strcpy(name, "Final EQ On"); strcpy(symbol, "feqon"); defVal = 1.0f; parameter.hints |= kParameterIsInteger; parameter.ranges.max = 1.0f; break;
            case kParamMasterAROn: strcpy(name, "Master ADSR On"); strcpy(symbol, "mason"); defVal = 0.0f; parameter.hints |= kParameterIsInteger; parameter.ranges.max = 1.0f; break;
            case kParamMasterARAttack: strcpy(name, "Master Attack"); strcpy(symbol, "maatk"); defVal = 0.01f; break;
            case kParamMasterARDecay: strcpy(name, "Master Decay"); strcpy(symbol, "madec"); defVal = 0.25f; break;
            case kParamMasterARSustain: strcpy(name, "Master Sustain"); strcpy(symbol, "masus"); defVal = 1.0f; break;
            case kParamMasterARRelease: strcpy(name, "Master Release"); strcpy(symbol, "marel"); defVal = 0.2f; break;
            case kParamMasterTuning: strcpy(name, "Master Tune"); strcpy(symbol, "master_tune"); defVal = 0.5f; break;
            case kParamFinalTouchLPF: strcpy(name, "Final Touch LPF"); strcpy(symbol, "ft_lpf"); defVal = 1.0f; break;
            case kParamFinalTouchHPF: strcpy(name, "Final Touch HPF"); strcpy(symbol, "ft_hpf"); defVal = 0.0f; break;
            // Deprecated/reserved tail (append-only): not shown or wired.
            case kParamFinalTouchQ: strcpy(name, "HP Filter Q (reserved)"); strcpy(symbol, "ft_q"); defVal = 0.425f; break;
            case kParamFinalTouchHPSlope: strcpy(name, "HP Slope (reserved)"); strcpy(symbol, "ft_hp_slope"); defVal = 1.0f; break;
            case kParamFinalTouchHPChar: strcpy(name, "HP Character (reserved)"); strcpy(symbol, "ft_hp_char"); defVal = 0.0f; break;
            case kParamFinalTouchHPMix: strcpy(name, "HP Mix (reserved)"); strcpy(symbol, "ft_hp_mix"); defVal = 1.0f; break;
            case kParamFinalTouchHPOut: strcpy(name, "HP Output (reserved)"); strcpy(symbol, "ft_hp_out"); defVal = 0.5f; break;
            case kParamFinalTouchHPBypass: strcpy(name, "HP Bypass (reserved)"); strcpy(symbol, "ft_hp_bypass"); defVal = 0.0f; break;
            case kParamFinalTouchHPSolo: strcpy(name, "HP Solo (reserved)"); strcpy(symbol, "ft_hp_solo"); defVal = 0.0f; break;
            case kParamFinalTouchHPStereo: strcpy(name, "HP Stereo (reserved)"); strcpy(symbol, "ft_hp_stereo"); defVal = 0.0f; break;
            case kParamFinalTouchLPQ: strcpy(name, "LP Filter Q (reserved)"); strcpy(symbol, "ft_lpq"); defVal = 0.425f; break;
            case kParamFinalTouchLPSlope: strcpy(name, "LP Slope (reserved)"); strcpy(symbol, "ft_lp_slope"); defVal = 1.0f; break;
            case kParamFinalTouchLPChar: strcpy(name, "LP Character (reserved)"); strcpy(symbol, "ft_lp_char"); defVal = 0.0f; break;
            case kParamFinalTouchLPMix: strcpy(name, "LP Mix (reserved)"); strcpy(symbol, "ft_lp_mix"); defVal = 1.0f; break;
            case kParamFinalTouchLPOut: strcpy(name, "LP Output (reserved)"); strcpy(symbol, "ft_lp_out"); defVal = 0.5f; break;
            case kParamFinalTouchLPBypass: strcpy(name, "LP Bypass (reserved)"); strcpy(symbol, "ft_lp_bypass"); defVal = 0.0f; break;
            case kParamFinalTouchLPSolo: strcpy(name, "LP Solo (reserved)"); strcpy(symbol, "ft_lp_solo"); defVal = 0.0f; break;
            case kParamFinalTouchLPStereo: strcpy(name, "LP Stereo (reserved)"); strcpy(symbol, "ft_lp_stereo"); defVal = 0.0f; break;
        case kParamFinalClipMix: strcpy(name, "Final Clip Mix"); strcpy(symbol, "fc_mix"); defVal = 1.0f; break;
            case kParamFinalClipOutput: strcpy(name, "Final Clip Output"); strcpy(symbol, "fc_out"); defVal = 0.5f; break;
            case kParamFinalTiltAmount: strcpy(name, "Spectral Tilt"); strcpy(symbol, "tilt"); defVal = 0.5f; break;
            case kParamFinalTiltPivot: strcpy(name, "Tilt Pivot"); strcpy(symbol, "tiltp"); defVal = 2.0f/3.0f; break;
            case kParamFinalAROn: strcpy(name, "Final tweaks On"); strcpy(symbol, "faron"); defVal = 0.0f; parameter.hints |= kParameterIsInteger; break;
            case kParamFinalARAttack: strcpy(name, "Final Attack"); strcpy(symbol, "faratk"); defVal = 0.04f; break;
            case kParamFinalARRelease: strcpy(name, "Final Release"); strcpy(symbol, "farrel"); defVal = 0.18f; break;
            case kParamFinalARDecay: strcpy(name, "Final Decay"); strcpy(symbol, "fardec"); defVal = 0.25f; break;
            case kParamFinalARSustain: strcpy(name, "Final Sustain"); strcpy(symbol, "farsus"); defVal = 0.8f; break;
            case kParamFinalClipOn: strcpy(name, "Clipper On"); strcpy(symbol, "fclipon"); defVal = 0.0f; parameter.hints |= kParameterIsInteger; break;
            case kParamFinalClipType: strcpy(name, "Clipper Type"); strcpy(symbol, "fcliptype"); defVal = 0.0f; break;
            case kParamFinalClipAmount: strcpy(name, "Clipper Amount"); strcpy(symbol, "fclipamt"); defVal = 0.0f; break;
            case kParamProcessingMode: strcpy(name, "Math Mode"); strcpy(symbol, "pmode"); defVal = 0.0f; parameter.hints |= kParameterIsInteger; parameter.ranges.max = 1.0f; break;
            case kParamAnalysisPoint: strcpy(name, "Analysis Pt"); strcpy(symbol, "ana"); defVal = 4.0f; parameter.hints |= kParameterIsInteger; parameter.ranges.max = 4.0f; break;
            case kParamModOn: strcpy(name, "Modulation On"); strcpy(symbol, "mod_on"); defVal = 1.0f; parameter.hints |= kParameterIsInteger; break;
            default: snprintf(name, sizeof(name), "Param %d", index); snprintf(symbol, sizeof(symbol), "p%d", index); break;
        }
    }
    parameter.name = name; parameter.symbol = symbol; parameter.ranges.def = defVal;
}

void PluginCymbals::normalizeSlotProcessOrder() {
    SlotOrderPolicy::normalizeSlotOrder(m_slotProcessOrder);
}

float PluginCymbals::getParameterValue(uint32_t index) const {
    if (index >= kParameterCount) return 0.0f;
    if (index >= kParamModSrc1Type && index <= kParamModSrc11StepSmooth)
        return m_modEngine.getSourceParam((int)(index - kParamModSrc1Type) / ModulationEngine::kSourceStride,
                                          (int)(index - kParamModSrc1Type) % ModulationEngine::kSourceStride);
    if (index >= kParamRoute1Source && index <= kParamRoute48Enabled)
        return m_modEngine.getRouteParam((int)(index - kParamRoute1Source) / 5, (int)(index - kParamRoute1Source) % 5);
    if (index >= kParamMacro1 && index <= kParamMacro4) return m_modEngine.getMacro((int)(index - kParamMacro1));
    // Discrete EQ ranges read the LIVE members: the classic Type (freq-group
    // sub 3) and the model-group FilterType (sub 1) alias the same
    // eqFilterType[band] member, so a base read of the twin that was not
    // written would report a stale value. These are never mod targets
    // (kModTargetClasses stops each node at Q), so live reads are safe here —
    // same pattern as the slot ranges below.
    if (index >= kParamEQ1Freq && index <= kParamEQ5Type) {
        const int band = (int)(index - kParamEQ1Freq) / 4;
        const int sub = (int)(index - kParamEQ1Freq) % 4;
        if (sub == 3) return eqFilterType[band];
        return m_paramBase[index];
    }
    if (index >= kParamFinalEQ1Freq && index <= kParamFinalEQ5Type) {
        const int band = (int)(index - kParamFinalEQ1Freq) / 4;
        const int sub = (int)(index - kParamFinalEQ1Freq) % 4;
        if (sub == 3) return feqFilterType[band];
        return m_paramBase[index];
    }
    if (index >= kParamEQ1Model && index <= kParamEQ5Slope) {
        const int band = (int)(index - kParamEQ1Model) / 3;
        const int sub = (int)(index - kParamEQ1Model) % 3;
        if (sub == 0) return eqModel[band];
        if (sub == 1) return eqFilterType[band];
        return eqSlope[band];
    }
    if (index >= kParamFinalEQ1Model && index <= kParamFinalEQ5Slope) {
        const int band = (int)(index - kParamFinalEQ1Model) / 3;
        const int sub = (int)(index - kParamFinalEQ1Model) % 3;
        if (sub == 0) return feqModel[band];
        if (sub == 1) return feqFilterType[band];
        return feqSlope[band];
    }
    if (index >= kParamSlot1Type && index <= kParamSlot6Bypass) {
        const int slot = (index - kParamSlot1Type) / 16;
        const int sub  = (index - kParamSlot1Type) % 16;
        if (slot < 0 || slot >= 6) return 0.0f;
        if (sub == 0) {
            // The slot type is applied on the audio thread (pending handoff).
            // A host state restore / editor reopen can ask for the parameter
            // before that drain runs; the live FX object is still the OLD one
            // then, so report the staged type. Without this the DSP loaded the
            // restored FX correctly but the newly built UI card looked empty.
            const int pendingType = fPendingSlotTypes[slot].load(std::memory_order_acquire);
            if (pendingType >= 0) return (float)pendingType;
            if (m_internalFx[slot] != nullptr)
                return (float)m_internalFx[slot]->typeId();
            if (m_surgeFxAlgo[slot] >= 0)
                return (float)CymbalsSurgeFx::algorithmId(m_surgeFxAlgo[slot]);
            return (float)(int)m_fxSlots[slot].getCurrentType(); // deferred to audio thread; not mod-routable
        }
        if (sub == 15) return m_slotBypass[slot] ? 1.0f : 0.0f;              // discrete; not mod-routable
        return m_paramBase[index]; // params 1..13 + wet: applySlotFamily modulates the members
    }
    if (index >= kParamSlotOrder0 && index <= kParamSlotOrder5) {
        int pos = index - kParamSlotOrder0;
        return (float)m_slotProcessOrder[pos]; // normalized internal state; not mod-routable
    }
    return m_paramBase[index];
}

void PluginCymbals::setParameterValue(uint32_t index, float value) {
    // No mutex here: DPF invokes this on the RT audio thread (CLAP process
    // events, JACK process callback). Plain float stores only — see the
    // discipline note at fParameterSnapshotMutex. Nothing below locks: the
    // fPendingSlotTypes staging is a lock-free atomic handoff.
    if (index < kParameterCount) m_paramBase[index] = value;

    if (index >= kParamComb1Q && index <= kParamComb40Q) {
        const int c = (int)(index - kParamComb1Q);
        // 0 = AUTO (feedback-derived Q). Turning the knob engages explicit Q.
        const float q = (value <= 0.02f) ? -1.0f : 0.5f * std::pow(40.0f, std::clamp(value, 0.0f, 1.0f));
        combBank.setFilterQ(c, q);
        return;
    }
    if (index >= kParamFinalComb1Q && index <= kParamFinalComb4Q) {
        const int c = (int)(index - kParamFinalComb1Q);
        const float q = (value <= 0.02f) ? -1.0f : 0.5f * std::pow(40.0f, std::clamp(value, 0.0f, 1.0f));
        finalCombBank.setFilterQ(c, q);
        return;
    }

    if (index >= kParamEQ1Freq && index <= kParamEQ5Type) {
        int band = (index - kParamEQ1Freq) / 4;
        int sub = (index - kParamEQ1Freq) % 4;
        if (sub == 0) eqFreq[band] = value;
        else if (sub == 1) eqGain[band] = value;
        else if (sub == 2) eqQ[band] = value;
        else { eqFilterType[band] = std::clamp(std::round(finiteOr(value, 0.0f)), 0.0f, 7.0f); m_paramBase[index] = eqFilterType[band]; }
        requestStickEQRebuild();
        return;
    }

    if (index >= kParamFinalEQ1Freq && index <= kParamFinalEQ5Type) {
        int band = (index - kParamFinalEQ1Freq) / 4;
        int sub = (index - kParamFinalEQ1Freq) % 4;
        if (sub == 0) feqFreq[band] = value;
        else if (sub == 1) feqGain[band] = value;
        else if (sub == 2) feqQ[band] = value;
        else { feqFilterType[band] = std::clamp(std::round(finiteOr(value, 0.0f)), 0.0f, 7.0f); m_paramBase[index] = feqFilterType[band]; }
        requestFinalEQRebuild();
        return;
    }

    if (index >= kParamEQ1AdvType && index <= kParamEQ3AdvRes) {
        int band = (index - kParamEQ1AdvType) / 2;
        int type = (index - kParamEQ1AdvType) % 2;
        if (type == 0) { eqAdvType[band] = clampAdvancedEqTypeBucket(value); m_paramBase[index] = eqAdvType[band]; }
        else eqAdvRes[band] = std::clamp(value, 0.0f, 1.0f);
        requestStickEQRebuild();
        return;
    }

    if (index >= kParamFinalEQ1AdvType && index <= kParamFinalEQ3AdvRes) {
        int band = (index - kParamFinalEQ1AdvType) / 2;
        int type = (index - kParamFinalEQ1AdvType) % 2;
        if (type == 0) { feqAdvType[band] = clampAdvancedEqTypeBucket(value); m_paramBase[index] = feqAdvType[band]; }
        else feqAdvRes[band] = std::clamp(value, 0.0f, 1.0f);
        requestFinalEQRebuild();
        return;
    }

    if (index >= kParamEQ1AdvDrive && index <= kParamEQ3AdvCharacter) {
        int band = (index - kParamEQ1AdvDrive) / 2;
        int type = (index - kParamEQ1AdvDrive) % 2;
        if (type == 0) eqAdvDrive[band] = std::clamp(value, 0.0f, 1.0f);
        else eqAdvCharacter[band] = std::clamp(value, 0.0f, 1.0f);
        requestStickEQRebuild();
        return;
    }

    if (index >= kParamFinalEQ1AdvDrive && index <= kParamFinalEQ3AdvCharacter) {
        int band = (index - kParamFinalEQ1AdvDrive) / 2;
        int type = (index - kParamFinalEQ1AdvDrive) % 2;
        if (type == 0) feqAdvDrive[band] = std::clamp(value, 0.0f, 1.0f);
        else feqAdvCharacter[band] = std::clamp(value, 0.0f, 1.0f);
        requestFinalEQRebuild();
        return;
    }

    if (index >= kParamEQ1Model && index <= kParamEQ5Slope) {
        int band = (index - kParamEQ1Model) / 3;
        int sub = (index - kParamEQ1Model) % 3;
        if (sub == 0) { eqModel[band] = std::clamp(std::round(finiteOr(value, 0.0f)), 0.0f, (float)(UnifiedFilter::Count - 1)); m_paramBase[index] = eqModel[band]; }
        else if (sub == 1) { eqFilterType[band] = std::clamp(std::round(finiteOr(value, 0.0f)), 0.0f, 7.0f); m_paramBase[index] = eqFilterType[band]; }
        else { eqSlope[band] = std::clamp(std::round(finiteOr(value, 0.0f)), 0.0f, 3.0f); m_paramBase[index] = eqSlope[band]; }
        requestStickEQRebuild();
        return;
    }

    if (index >= kParamFinalEQ1Model && index <= kParamFinalEQ5Slope) {
        int band = (index - kParamFinalEQ1Model) / 3;
        int sub = (index - kParamFinalEQ1Model) % 3;
        if (sub == 0) { feqModel[band] = std::clamp(std::round(finiteOr(value, 0.0f)), 0.0f, (float)(UnifiedFilter::Count - 1)); m_paramBase[index] = feqModel[band]; }
        else if (sub == 1) { feqFilterType[band] = std::clamp(std::round(finiteOr(value, 0.0f)), 0.0f, 7.0f); m_paramBase[index] = feqFilterType[band]; }
        else { feqSlope[band] = std::clamp(std::round(finiteOr(value, 0.0f)), 0.0f, 3.0f); m_paramBase[index] = feqSlope[band]; }
        requestFinalEQRebuild();
        return;
    }

    if (index >= kParamEQ1Sat && index <= kParamEQ5Sat) {
        eqSat[index - kParamEQ1Sat] = std::clamp(finiteOr(value, 0.0f), 0.0f, 1.0f);
        m_paramBase[index] = eqSat[index - kParamEQ1Sat]; // NaN/Inf sanitized
        requestStickEQRebuild();
        return;
    }
    if (index >= kParamFinalEQ1Sat && index <= kParamFinalEQ5Sat) {
        feqSat[index - kParamFinalEQ1Sat] = std::clamp(finiteOr(value, 0.0f), 0.0f, 1.0f);
        m_paramBase[index] = feqSat[index - kParamFinalEQ1Sat]; // NaN/Inf sanitized
        requestFinalEQRebuild();
        return;
    }

    if (index >= kParamNudge1 && index <= kParamNudgeAmount20) {
        int i = (index - kParamNudge1) / 2;
        if ((index - kParamNudge1) % 2 == 0) {
            nudge[i] = value;
            dispersion.setNudge(i, value);
        } else {
            nudgeAmount[i] = value;
            dispersion.setAmount(i, value);
        }
        return;
    }

    if (index >= kParamNudge2_1 && index <= kParamNudgeAmount2_20) {
        int i = (index - kParamNudge2_1) / 2;
        if ((index - kParamNudge2_1) % 2 == 0) {
            nudge2[i] = value;
            dispersion2.setNudge(i, value);
        } else {
            nudgeAmount2[i] = value;
            dispersion2.setAmount(i, value);
        }
        return;
    }

    if (index >= kParamComb1Freq && index <= kParamComb40HPF) {
        int s = (index - kParamComb1Freq) / 4;
        int t = (index - kParamComb1Freq) % 4;
        // The group header's LPF/HPF knob is stage 0 of the group but its UI
        // callback writes all four stages. Host automation/modulation only
        // wrote stage 0, so a DAW modulator on the group's filter frequency
        // left 3 of 4 stages behind (reported as "doesn't work from the DAW").
        // Match the UI: the header value lands on every stage in the group.
        if ((t == 2 || t == 3) && (s % 4) == 0) {
            const int groupBase = (s / 4) * 4;
            const float hz = 20.0f * std::pow(1000.0f, value);
            for (int k = 0; k < 4; ++k) {
                const int c = groupBase + k;
                const uint32_t pi = (uint32_t)(kParamComb1Freq + c * 4 + t);
                m_paramBase[pi] = value;
                combParams[c * 4 + t] = value;
                if (t == 2) combBank.setLPF(c, hz);
                else        combBank.setHPF(c, hz);
            }
            return;
        }
        combParams[index - kParamComb1Freq] = value;
        if (t == 0) combBank.setFrequency(s, 20.0f * std::pow(1000.0f, value));
        else if (t == 1) combBank.setFeedback(s, std::min(0.998f, value));
        else if (t == 2) combBank.setLPF(s, 20.0f * std::pow(1000.0f, value));
        else combBank.setHPF(s, 20.0f * std::pow(1000.0f, value));
        return;
    }

    if (index >= kParamComb1Scale && index <= kParamComb40Scale) {
        int s = index - kParamComb1Scale;
        combScalePerStage[s] = value;
        combBank.setStageScaleFactor(s, 0.25f * std::pow(16.0f, value));
        return;
    }

    if (index >= kParamComb1SatType && index <= kParamComb40SatType) {
        int s = index - kParamComb1SatType;
        combSatType[s] = value;
        combBank.setSaturatorType(s, (int)value);
        return;
    }

    if (index >= kParamComb1FilterType && index <= kParamComb40FilterType) {
        int s = index - kParamComb1FilterType;
        combFilterType[s] = value;
        combBank.setFilterType(s, (int)value);
        return;
    }

    if (index >= kParamComb1ModeRate && index <= kParamComb40ModeRate) {
        int s = index - kParamComb1ModeRate;
        combModeRate[s] = value;
        combBank.setModeRate(s, value);
        return;
    }

    if (index >= kParamComb1ModeAmount && index <= kParamComb40ModeAmount) {
        int s = index - kParamComb1ModeAmount;
        combModeAmount[s] = value;
        combBank.setModeAmount(s, value);
        return;
    }

    if (index >= kParamComb1PartRelPos && index <= kParamComb40PartRelPos) {
        int s = index - kParamComb1PartRelPos;
        combPartRelPos[s] = value;
        combBank.setPartRelPos(s, value);
        return;
    }

    if (index >= kParamComb1PartFreqDamp && index <= kParamComb40PartFreqDamp) {
        int s = index - kParamComb1PartFreqDamp;
        combPartFreqDamp[s] = value;
        combBank.setPartFreqDamp(s, value);
        return;
    }

    if (index >= kParamComb1PartStiffness && index <= kParamComb40PartStiffness) {
        int s = index - kParamComb1PartStiffness;
        combPartStiffness[s] = value;
        combBank.setPartStiffness(s, value);
        return;
    }

    if (index >= kParamComb1PartStiffExp && index <= kParamComb40PartStiffExp) {
        int s = index - kParamComb1PartStiffExp;
        combPartStiffExp[s] = value;
        combBank.setPartStiffExp(s, value);
        return;
    }

    if (index >= kParamComb1PartCount && index <= kParamComb40PartCount) {
        int s = index - kParamComb1PartCount;
        combPartCount[s] = value;
        combBank.setPartCount(s, value);
        return;
    }

    if (index >= kParamComb1SatAmount && index <= kParamComb40SatAmount) {
        int s = index - kParamComb1SatAmount;
        combSatAmount[s] = value;
        combBank.setSaturatorAmount(s, value);
        return;
    }

    if (index >= kParamComb1StressOnset && index <= kParamComb40StressOnset) {
        int s = index - kParamComb1StressOnset;
        combStressOnset[s] = value;
        combBank.setSatStressOnset(s, value);
        return;
    }

    if (index >= kParamComb1StressKnee && index <= kParamComb40StressKnee) {
        int s = index - kParamComb1StressKnee;
        combStressKnee[s] = value;
        combBank.setSatStressKnee(s, value);
        return;
    }

    if (index >= kParamComb1Mix && index <= kParamComb40Mix) {
        int s = index - kParamComb1Mix;
        combMix[s] = value;
        combBank.setStageMix(s, value);
        return;
    }

    if (index >= kParamComb1Solo && index <= kParamComb40Solo) {
        int s = index - kParamComb1Solo;
        combSolo[s] = value;
        combBank.setStageSolo(s, value > 0.5f);
        return;
    }

    if (index >= kParamComb1Enable && index <= kParamComb40Enable) {
        int s = index - kParamComb1Enable;
        combEnable[s] = value;
        combBank.setStageEnabled(s, value > 0.5f);
        return;
    }

    if (index >= kParamCombGroup1Shift && index <= kParamCombGroup10Shift) {
        int g = index - kParamCombGroup1Shift;
        combGroupShift[g] = value;
        combBank.setShift(g, (value - 0.5f) * 20000.0f);
        return;
    }

    if (index >= kParamCombGroup1Gain && index <= kParamCombGroup10Gain) {
        int g = index - kParamCombGroup1Gain;
        combGroupGain[g] = value;
        float db = DSPUtils::normGainToDb(value);
        combBank.setGroupGain(g, combGroupOn[g] > 0.5f ? std::pow(10.0f, db / 20.0f) : 0.0f);
        return;
    }

    if (index >= kParamCombGroup1On && index <= kParamCombGroup10On) {
        int g = index - kParamCombGroup1On;
        combGroupOn[g] = value;
        float db = DSPUtils::normGainToDb(combGroupGain[g]);
        combBank.setGroupGain(g, value > 0.5f ? std::pow(10.0f, db / 20.0f) : 0.0f);
        return;
    }

    // Per-group limiter: store the normalized control and push the converted
    // value into the bank. Off by default; the bank early-outs on the flag.
    if (index >= kParamCombGroup1LimiterOn && index <= kParamCombGroup10LimiterOn) {
        int g = index - kParamCombGroup1LimiterOn;
        combGroupLimiterOn[g] = value;
        combBank.setGroupLimiterOn(g, value);
        return;
    }

    if (index >= kParamCombGroup1LimiterThreshold && index <= kParamCombGroup10LimiterThreshold) {
        int g = index - kParamCombGroup1LimiterThreshold;
        combGroupLimiterThreshold[g] = value;
        combBank.setGroupLimiterThreshold(g, value);
        return;
    }

    if (index >= kParamCombGroup1LimiterRelease && index <= kParamCombGroup10LimiterRelease) {
        int g = index - kParamCombGroup1LimiterRelease;
        combGroupLimiterRelease[g] = value;
        combBank.setGroupLimiterRelease(g, value);
        return;
    }

    if (index >= kParamCombBank1Series && index <= kParamCombBank10Series) {
        int g = index - kParamCombBank1Series;
        combBankSeries[g] = value;
        combBank.setSeries(g, value > 0.5f);
        return;
    }

    if (index >= kParamCombGroup1Cluster && index <= kParamCombGroup10Cluster) {
        int g = index - kParamCombGroup1Cluster;
        combGroupCluster[g] = value;
        combBank.setCluster(g, value);
        return;
    }

    if (index >= kParamFinalComb1Freq && index <= kParamFinalComb4HPF) {
        int s = (index - kParamFinalComb1Freq) / 4;
        int t = (index - kParamFinalComb1Freq) % 4;
        // Same group-header broadcast as the main comb bank: the final group's
        // Low Cut / High Cut header is stage 0 but its UI callback writes all
        // four stages; host automation of the header must do the same.
        if (t == 2 || t == 3) {
            const float hz = 20.0f * std::pow(1000.0f, value);
            for (int c = 0; c < 4; ++c) {
                const uint32_t pi = (uint32_t)(kParamFinalComb1Freq + c * 4 + t);
                m_paramBase[pi] = value;
                finalCombParams[c * 4 + t] = value;
                if (t == 2) finalCombBank.setLPF(c, hz);
                else        finalCombBank.setHPF(c, hz);
            }
            return;
        }
        finalCombParams[index - kParamFinalComb1Freq] = value;
        if (t == 0) finalCombBank.setFrequency(s, 20.0f * std::pow(1000.0f, value));
        else finalCombBank.setFeedback(s, std::min(0.998f, value));
        return;
    }

    if (index >= kParamFinalComb1Mix && index <= kParamFinalComb4Mix) {
        int s = index - kParamFinalComb1Mix;
        finalCombMix[s] = value;
        finalCombBank.setStageMix(s, value);
        return;
    }

    if (index >= kParamFinalComb1Solo && index <= kParamFinalComb4Solo) {
        int s = index - kParamFinalComb1Solo;
        finalCombSolo[s] = value;
        finalCombBank.setStageSolo(s, value > 0.5f);
        return;
    }

    if (index >= kParamFinalComb1Enable && index <= kParamFinalComb4Enable) {
        int s = index - kParamFinalComb1Enable;
        finalCombEnable[s] = value;
        finalCombBank.setStageEnabled(s, value > 0.5f);
        return;
    }

    if (index >= kParamFinalComb1FilterType && index <= kParamFinalComb4FilterType) {
        int s = index - kParamFinalComb1FilterType;
        finalCombFilterType[s] = value;
        finalCombBank.setFilterType(s, (int)value);
        return;
    }

    if (index >= kParamFinalComb1ModeRate && index <= kParamFinalComb4ModeRate) {
        int s = index - kParamFinalComb1ModeRate;
        finalCombModeRate[s] = value;
        finalCombBank.setModeRate(s, value);
        return;
    }

    if (index >= kParamFinalComb1ModeAmount && index <= kParamFinalComb4ModeAmount) {
        int s = index - kParamFinalComb1ModeAmount;
        finalCombModeAmount[s] = value;
        finalCombBank.setModeAmount(s, value);
        return;
    }

    // Per-filter-type MODE A/B (see PluginCymbals::Parameters). RT-safe: a
    // plain float store into the stage.
    if (index >= kParamComb1ModeA && index <= kParamComb40ModeB) {
        const int off = index - kParamComb1ModeA;
        if (off < 40) combBank.setModeA(off, value);
        else          combBank.setModeB(off - 40, value);
        return;
    }

    if (index == kParamFinalCombLimiterOn) {
        finalCombBank.setGroupLimiterOn(0, value);
        return;
    }
    if (index == kParamFinalCombLimiterThreshold) {
        finalCombBank.setGroupLimiterThreshold(0, value);
        return;
    }
    if (index == kParamFinalCombLimiterRelease) {
        finalCombBank.setGroupLimiterRelease(0, value);
        return;
    }

    if (index >= kParamFinalComb1ModeA && index <= kParamFinalComb4ModeB) {
        const int off = index - kParamFinalComb1ModeA;
        if (off < 4) finalCombBank.setModeA(off, value);
        else         finalCombBank.setModeB(off - 4, value);
        return;
    }

    if (index >= kParamFinalComb1PartRelPos && index <= kParamFinalComb4PartRelPos) {
        int s = index - kParamFinalComb1PartRelPos;
        finalCombPartRelPos[s] = value;
        finalCombBank.setPartRelPos(s, value);
        return;
    }

    if (index >= kParamFinalComb1PartFreqDamp && index <= kParamFinalComb4PartFreqDamp) {
        int s = index - kParamFinalComb1PartFreqDamp;
        finalCombPartFreqDamp[s] = value;
        finalCombBank.setPartFreqDamp(s, value);
        return;
    }

    if (index >= kParamFinalComb1PartStiffness && index <= kParamFinalComb4PartStiffness) {
        int s = index - kParamFinalComb1PartStiffness;
        finalCombPartStiffness[s] = value;
        finalCombBank.setPartStiffness(s, value);
        return;
    }

    if (index >= kParamFinalComb1PartStiffExp && index <= kParamFinalComb4PartStiffExp) {
        int s = index - kParamFinalComb1PartStiffExp;
        finalCombPartStiffExp[s] = value;
        finalCombBank.setPartStiffExp(s, value);
        return;
    }

    if (index >= kParamFinalComb1PartCount && index <= kParamFinalComb4PartCount) {
        int s = index - kParamFinalComb1PartCount;
        finalCombPartCount[s] = value;
        finalCombBank.setPartCount(s, value);
        return;
    }

    if (index >= kParamModSrc1Type && index <= kParamModSrc11StepSmooth) {
        m_modEngine.setSourceParam((int)(index - kParamModSrc1Type) / ModulationEngine::kSourceStride, (int)(index - kParamModSrc1Type) % ModulationEngine::kSourceStride, value);
        return;
    }

    if (index >= kParamRoute1Source && index <= kParamRoute48Enabled) {
        m_modEngine.setRouteParam((int)(index - kParamRoute1Source) / 5, (int)(index - kParamRoute1Source) % 5, value);
        return;
    }

    if (index >= kParamMacro1 && index <= kParamMacro4) {
        m_modEngine.setMacro((int)(index - kParamMacro1), value);
        return;
    }

    switch (index) {
        case kParamTone:
            tone = value;
            break;

        case kParamDecay:
            decay = value;
            break;

        case kParamVolume:
            volume = value;
            break;

        case kParamDispFB: {
            dispFB = value;
            const float base = DispersionStage::feedbackFromControl(dispFB);
            const float trim = DispersionStage::overallFeedbackScaleFromControl(dispFBOverall);
            dispersion.setFeedback(base * trim);
            break;
        }

        case kParamDispDelay:
            dispDelay = value;
            dispersion.setDelaySamples(value * 2000.0f);
            break;

        case kParamDispGain:
            dispGain = value;
            dispersion.setInputGain(value);
            break;

        case kParamDispFBOverall: {
            dispFBOverall = value;
            const float base = DispersionStage::feedbackFromControl(dispFB);
            const float trim = DispersionStage::overallFeedbackScaleFromControl(dispFBOverall);
            dispersion.setFeedback(base * trim);
            break;
        }

        case kParamStickType:
            stickType = value;
            stickBank.setCurrentStick((StickSoundBank::StickType)(int)value);
            break;

        case kParamStickDirect:
            stickDirect = value;
            break;

        case kParamStickBus:
            stickBus = value;
            break;

        case kParamStickAmpAttack:
            stickAmpAttack = value;
            stickBank.setAmpAttack(value);
            break;

        case kParamStickAmpDecay:
            stickAmpDecay = value;
            stickBank.setAmpDecay(value);
            break;

        case kParamStickAmpSustain:
            stickAmpSustain = value;
            stickBank.setAmpSustain(value);
            break;

        case kParamStickAmpRelease:
            stickAmpRelease = value;
            stickBank.setAmpRelease(value);
            break;

        case kParamStickAmpDecayCurve:
            stickAmpDecayCurve = std::clamp(value, 0.0f, 1.0f);
            stickBank.setAmpDecayCurve(stickAmpDecayCurve);
            break;
        case kParamStickAmpReleaseCurve:
            stickAmpReleaseCurve = std::clamp(value, 0.0f, 1.0f);
            stickBank.setAmpReleaseCurve(stickAmpReleaseCurve);
            break;

        case kParamStickPitchAttack:
            stickPitchAttack = value;
            stickBank.setPitchAttack(value);
            break;

        case kParamStickPitchDecay:
            stickPitchDecay = value;
            stickBank.setPitchDecay(value);
            break;

        case kParamStickPitchSustain:
            stickPitchSustain = value;
            stickBank.setPitchSustain(value);
            break;

        case kParamStickPitchRelease:
            stickPitchRelease = value;
            stickBank.setPitchRelease(value);
            break;

        case kParamStickPitchDecayCurve:
            stickPitchDecayCurve = std::clamp(value, 0.0f, 1.0f);
            stickBank.setPitchDecayCurve(stickPitchDecayCurve);
            break;
        case kParamStickPitchReleaseCurve:
            stickPitchReleaseCurve = std::clamp(value, 0.0f, 1.0f);
            stickBank.setPitchReleaseCurve(stickPitchReleaseCurve);
            break;

        case kParamStickPitchAmount:
            stickPitchAmount = value;
            stickBank.setPitchAmount(value * 48.0f);
            break;

        case kParamStickPitchTimer:
            stickPitchTimer = value;
            stickBank.setPitchTimer(value * 0.1f);
            break;
        case kParamStickPulseWidth:
            stickPulseWidth = std::clamp(value, 0.0f, 1.0f);
            stickBank.setPulseWidth(stickPulseWidth);
            break;
        case kParamStickNoiseColor:
            stickNoiseColor = std::clamp(value, 0.0f, 1.0f);
            stickBank.setNoiseColor(stickNoiseColor);
            break;
        case kParamStickMetallicSpread:
            stickMetallicSpread = std::clamp(value, 0.0f, 1.0f);
            stickBank.setMetallicSpread(stickMetallicSpread);
            break;
        case kParamStickNoiseMetalBlend:
            stickNoiseMetalBlend = std::clamp(value, 0.0f, 1.0f);
            stickBank.setNoiseMetalBlend(stickNoiseMetalBlend);
            break;
        case kParamStickHatTone:
            stickHatTone = std::clamp(value, 0.0f, 1.0f);
            stickBank.setHatTone(stickHatTone);
            break;
        case kParamStickSquareWidth:
            stickSquareWidth = std::clamp(value, 0.0f, 1.0f);
            stickBank.setSquareWidth(stickSquareWidth);
            break;
        case kParamStickSawCurve:
            stickSawCurve = std::clamp(value, 0.0f, 1.0f);
            stickBank.setSawCurve(stickSawCurve);
            break;
        case kParamStickTriangleSkew:
            stickTriangleSkew = std::clamp(value, 0.0f, 1.0f);
            stickBank.setTriangleSkew(stickTriangleSkew);
            break;
        case kParamStickSineDrive:
            stickSineDrive = std::clamp(value, 0.0f, 1.0f);
            stickBank.setSineDrive(stickSineDrive);
            break;
        case kParamStickPhase:
            stickPhase = std::clamp(value, 0.0f, 1.0f);
            stickBank.setPhaseOffset(stickPhase);
            break;
        case kParamStickGrit:
            stickGrit = std::clamp(value, 0.0f, 1.0f);
            stickBank.setGrit(stickGrit);
            break;
        case kParamStickUnison:
            stickUnison = std::clamp(std::round(value), 0.0f, 3.0f);
            m_paramBase[index] = stickUnison;
            stickBank.setUnison((int)stickUnison + 1);
            break;
        case kParamStickDetune:
            stickDetune = std::clamp(value, 0.0f, 1.0f);
            stickBank.setDetune(stickDetune);
            break;

        case kParamVibRate:
            vibRate = value;
            dispersion.setVibratoRate(10.0f + value * 1990.0f);
            break;

        case kParamVibDepth:
            vibDepth = value;
            dispersion.setVibratoDepth(value * 40.0f);
            break;

        case kParamVibShape:
            vibShape = value;
            dispersion.setVibratoShape((int)value);
            break;

        case kParamVibSmooth:
            vibSmooth = value;
            dispersion.setVibratoSmooth(value);
            break;

        case kParamVibStepMix:
            vibStepMix = value;
            dispersion.setVibratoStepMix(value);
            break;

        case kParamChaos:
            chaosFactor = value;
            dispersion.setChaos(value);
            break;

        case kParamChaosEnvAmount:
            chaosEnvAmount = value;
            dispersion.setChaosEnvAmount(value);
            break;

        case kParamChaosEnvDecay:
            chaosEnvDecay = value;
            dispersion.setChaosEnvDecay(value);
            break;

        case kParamPDDrive:
            pdDrive = value;
            dispersion.setPDDrive(value);
            break;

        case kParamPDNormalize:
            pdNormalize = value;
            dispersion.setPDNormalize(value);
            break;

        case kParamPDTone:
            pdTone = value;
            dispersion.setPDTone(value);
            break;

        case kParamPDBias:
            pdBias = value;
            dispersion.setPDBias(value);
            break;

        case kParamPDSpread:
            pdSpread = value;
            dispersion.setPDSpread(value);
            break;

        case kParamPDMix:
            pdMix = value;
            dispersion.setPDMix(value);
            break;

        case kParamEQMix:
            eqMix = value;
            break;

        case kParamTiltAmount:
            stickTiltAmount = value;
            requestStickEQRebuild();
            break;

        case kParamTiltPivot:
            stickTiltPivot = value;
            requestStickEQRebuild();
            break;

        case kParamCombScale:
            combScale = value;
            combBank.setScaleFactor(0.25f * std::pow(16.0f, value));
            break;

        case kParamDisp1Skip: {
            const bool wasSkipped = disp1Skip > 0.5f;
            disp1Skip = value;
            if ((value > 0.5f) != wasSkipped) {
                combBank.startSoftClear();
                finalCombBank.startSoftClear();
            }
            break;
        }

        case kParamDisp2Skip: {
            const bool wasSkipped = disp2Skip > 0.5f;
            disp2Skip = value;
            if ((value > 0.5f) != wasSkipped) {
                combBank.startSoftClear();
                finalCombBank.startSoftClear();
            }
            break;
        }

        case kParamDispCrossLink:
            dispCrossLink = value;
            break;

        case kParamDispSolo:
            dispSolo = value;
            break;

        case kParamDispDelayMode:
            dispDelayMode = value;
            dispersion.setDelayMode((int)value);
            break;

        case kParamDisp2DelayMode:
            disp2DelayMode = value;
            dispersion2.setDelayMode((int)value);
            break;

        case kParamInharmBloomAmt:
            inharmBloomAmt = value;
            break;

        case kParamInharmBloomTime: {
            inharmBloomTime = value;
            const float decaySec = DSPUtils::inharmBloomDecayFromControl(value);
            inharmBloomEnv.setDecay(decaySec);
            break;
        }

        case kParamInharmBloomDir:
            inharmBloomDir = (value > 0.5f) ? 1.0f : 0.0f;
            m_paramBase[index] = inharmBloomDir;
            break;

        case kParamInharm2BloomAmt:
            inharm2BloomAmt = value;
            break;

        case kParamInharm2BloomTime: {
            inharm2BloomTime = value;
            const float decaySec = DSPUtils::inharmBloomDecayFromControl(value);
            inharm2BloomEnv.setDecay(decaySec);
            break;
        }

        case kParamInharm2BloomDir:
            inharm2BloomDir = (value > 0.5f) ? 1.0f : 0.0f;
            m_paramBase[index] = inharm2BloomDir;
            break;

        case kParamCombBankOn:
            combBankOn = value;
            break;

        case kParamFinalCombBankOn:
            finalCombBankOn = value;
            break;

        case kParamResonatorDiffusion:
            resonatorDiffusion = value;
            combBank.setResonatorDiffusion(value);
            break;

        case kParamCombMemoryDamping:
            combMemoryDamping = value;
            combBank.setMemoryDamping(value);
            finalCombBank.setMemoryDamping(value);
            break;

        case kParamDisp2FB: {
            disp2FB = value;
            const float base = DispersionStage::feedbackFromControl(disp2FB);
            const float trim = DispersionStage::overallFeedbackScaleFromControl(disp2FBOverall);
            dispersion2.setFeedback(base * trim);
            break;
        }

        case kParamDisp2Delay:
            disp2Delay = value;
            dispersion2.setDelaySamples(value * 2000.0f);
            break;

        case kParamDisp2Gain:
            disp2Gain = value;
            dispersion2.setInputGain(value);
            break;

        case kParamDisp2FBOverall: {
            disp2FBOverall = value;
            const float base = DispersionStage::feedbackFromControl(disp2FB);
            const float trim = DispersionStage::overallFeedbackScaleFromControl(disp2FBOverall);
            dispersion2.setFeedback(base * trim);
            break;
        }

        case kParamVib2Rate:
            vib2Rate = value;
            dispersion2.setVibratoRate(10.0f + value * 1990.0f);
            break;

        case kParamVib2Depth:
            vib2Depth = value;
            dispersion2.setVibratoDepth(value * 40.0f);
            break;

        case kParamVib2Shape:
            vib2Shape = value;
            dispersion2.setVibratoShape((int)value);
            break;

        case kParamVib2Smooth:
            vib2Smooth = value;
            dispersion2.setVibratoSmooth(value);
            break;

        case kParamVib2StepMix:
            vib2StepMix = value;
            dispersion2.setVibratoStepMix(value);
            break;

        case kParamChaos2:
            chaos2Factor = value;
            dispersion2.setChaos(value);
            break;

        case kParamChaos2EnvAmount:
            chaos2EnvAmount = value;
            dispersion2.setChaosEnvAmount(value);
            break;

        case kParamChaos2EnvDecay:
            chaos2EnvDecay = value;
            dispersion2.setChaosEnvDecay(value);
            break;

        case kParamPD2Drive:
            pd2Drive = value;
            dispersion2.setPDDrive(value);
            break;

        case kParamPD2Normalize:
            pd2Normalize = value;
            dispersion2.setPDNormalize(value);
            break;

        case kParamPD2Tone:
            pd2Tone = value;
            dispersion2.setPDTone(value);
            break;

        case kParamPD2Bias:
            pd2Bias = value;
            dispersion2.setPDBias(value);
            break;

        case kParamPD2Spread:
            pd2Spread = value;
            dispersion2.setPDSpread(value);
            break;

        case kParamPD2Mix:
            pd2Mix = value;
            dispersion2.setPDMix(value);
            break;

        case kParamStickPitchMsegOn:
            msegPitchOn = value;
            updateStickPitchMseg();
            break;

        case kParamStickPitchMsegDuration:
            msegPitchDuration = value;
            updateStickPitchMseg();
            break;

        case kParamStickPitchMsegPoint0Value:
            msegPitchValues[0] = value;
            updateStickPitchMseg();
            break;

        case kParamStickPitchMsegPoint1Value:
            msegPitchValues[1] = value;
            updateStickPitchMseg();
            break;

        case kParamStickPitchMsegPoint2Value:
            msegPitchValues[2] = value;
            updateStickPitchMseg();
            break;

        case kParamStickPitchMsegPoint3Value:
            msegPitchValues[3] = value;
            updateStickPitchMseg();
            break;

        case kParamStickPitchMsegPoint4Value:
            msegPitchValues[4] = value;
            updateStickPitchMseg();
            break;

        case kParamStickPitchMsegPoint5Value:
            msegPitchValues[5] = value;
            updateStickPitchMseg();
            break;

        case kParamStickPitchMsegPoint6Value:
            msegPitchValues[6] = value;
            updateStickPitchMseg();
            break;

        case kParamStickPitchMsegPoint7Value:
            msegPitchValues[7] = value;
            updateStickPitchMseg();
            break;

        case kParamStickPitchMsegPoint1Time:
            msegPitchTimes[0] = value;
            updateStickPitchMseg();
            break;

        case kParamStickPitchMsegPoint2Time:
            msegPitchTimes[1] = value;
            updateStickPitchMseg();
            break;

        case kParamStickPitchMsegPoint3Time:
            msegPitchTimes[2] = value;
            updateStickPitchMseg();
            break;

        case kParamStickPitchMsegPoint4Time:
            msegPitchTimes[3] = value;
            updateStickPitchMseg();
            break;

        case kParamStickPitchMsegPoint5Time:
            msegPitchTimes[4] = value;
            updateStickPitchMseg();
            break;

        case kParamStickPitchMsegPoint6Time:
            msegPitchTimes[5] = value;
            updateStickPitchMseg();
            break;

        case kParamStickPitchMsegPoint0Curve:
            msegPitchCurves[0] = value;
            updateStickPitchMseg();
            break;

        case kParamStickPitchMsegPoint1Curve:
            msegPitchCurves[1] = value;
            updateStickPitchMseg();
            break;

        case kParamStickPitchMsegPoint2Curve:
            msegPitchCurves[2] = value;
            updateStickPitchMseg();
            break;

        case kParamStickPitchMsegPoint3Curve:
            msegPitchCurves[3] = value;
            updateStickPitchMseg();
            break;

        case kParamStickPitchMsegPoint4Curve:
            msegPitchCurves[4] = value;
            updateStickPitchMseg();
            break;

        case kParamStickPitchMsegPoint5Curve:
            msegPitchCurves[5] = value;
            updateStickPitchMseg();
            break;

        case kParamStickPitchMsegPoint6Curve:
            msegPitchCurves[6] = value;
            updateStickPitchMseg();
            break;

        case kParamFinalCombShift:
            finalCombShift = value;
            finalCombBank.setShift(0, (value - 0.5f) * 20000.0f);
            break;

        case kParamFinalCombScale:
            finalCombScale = value;
            finalCombBank.setScaleFactor(0.25f * std::pow(16.0f, value));
            break;

        case kParamFinalCombBankSeries:
            finalCombBankSeries = value;
            finalCombBank.setSeries(0, value > 0.5f);
            break;

        case kParamFinalCombGroupGain: {
            finalCombGroupGain = value;
            float db = DSPUtils::normGainToDb(value);
            finalCombBank.setGroupGain(0, std::pow(10.0f, db / 20.0f));
            break;
        }

        case kParamFinalCombGroupCluster:
            finalCombGroupCluster = value;
            finalCombBank.setCluster(0, value);
            break;

        case kParamFinalComb4SatType:
            finalComb4SatType = value;
            finalCombBank.setSaturatorType(3, (int)value);
            break;

        case kParamFinalComb4SatAmount:
            finalComb4SatAmount = value;
            finalCombBank.setSaturatorAmount(3, value);
            break;

        case kParamFinalComb4StressOnset:
            finalComb4StressOnset = value;
            finalCombBank.setSatStressOnset(3, value);
            break;

        case kParamFinalComb4StressKnee:
            finalComb4StressKnee = value;
            finalCombBank.setSatStressKnee(3, value);
            break;

        case kParamFinalEQMix:
            feqMix = value;
            break;

        case kParamFinalEQOn:
            finalEQOn = value > 0.5f ? 1.0f : 0.0f;
            m_paramBase[index] = finalEQOn;
            break;

        case kParamFinalTiltAmount:
            finalTiltAmount = value;
            requestFinalEQRebuild();
            break;

        case kParamFinalTiltPivot:
            finalTiltPivot = value;
            requestFinalEQRebuild();
            break;

        case kParamFinalAROn:
            finalAROn = value > 0.5f ? 1.0f : 0.0f;
            m_paramBase[index] = finalAROn;
            if (finalAROn > 0.5f)
                finalOutEnv.trigger();
            break;

        case kParamFinalARAttack:
            finalARAttack = std::clamp(value, 0.0f, 1.0f);
            finalOutEnv.setAttack(0.001f + std::pow(finalARAttack, 2.0f) * 1.999f);
            break;

        case kParamFinalARDecay:
            finalARDecay = std::clamp(value, 0.0f, 1.0f);
            finalOutEnv.setDecay(0.001f + std::pow(finalARDecay, 2.0f) * 1.999f);
            break;

        case kParamFinalARSustain:
            finalARSustain = std::clamp(value, 0.0f, 1.0f);
            finalOutEnv.setSustain(finalARSustain);
            break;

        case kParamFinalARRelease:
            finalARRelease = std::clamp(value, 0.0f, 1.0f);
            finalOutEnv.setRelease(0.002f + std::pow(finalARRelease, 2.0f) * 2.998f);
            break;

        case kParamFinalARDecayCurve:
            finalARDecayCurve = std::clamp(value, 0.0f, 1.0f);
            finalOutEnv.setDecayCurve(finalARDecayCurve * 2.0f - 1.0f);
            break;
        case kParamFinalARReleaseCurve:
            finalARReleaseCurve = std::clamp(value, 0.0f, 1.0f);
            finalOutEnv.setReleaseCurve(finalARReleaseCurve * 2.0f - 1.0f);
            break;

        case kParamMasterAROn:
            masterAROn = value > 0.5f ? 1.0f : 0.0f;
            m_paramBase[index] = masterAROn;
            if (masterAROn > 0.5f) masterOutEnv.trigger();
            break;
        case kParamMasterARAttack:
            masterARAttack = std::clamp(value, 0.0f, 1.0f);
            masterOutEnv.setAttack(0.001f + std::pow(masterARAttack, 2.0f) * 1.999f);
            break;
        case kParamMasterARDecay:
            masterARDecay = std::clamp(value, 0.0f, 1.0f);
            masterOutEnv.setDecay(0.001f + std::pow(masterARDecay, 2.0f) * 1.999f);
            break;
        case kParamMasterARSustain:
            masterARSustain = std::clamp(value, 0.0f, 1.0f);
            masterOutEnv.setSustain(masterARSustain);
            break;
        case kParamMasterARRelease:
            masterARRelease = std::clamp(value, 0.0f, 1.0f);
            masterOutEnv.setRelease(0.002f + std::pow(masterARRelease, 2.0f) * 2.998f);
            break;

        case kParamMasterARDecayCurve:
            masterARDecayCurve = std::clamp(value, 0.0f, 1.0f);
            masterOutEnv.setDecayCurve(masterARDecayCurve * 2.0f - 1.0f);
            break;
        case kParamMasterARReleaseCurve:
            masterARReleaseCurve = std::clamp(value, 0.0f, 1.0f);
            masterOutEnv.setReleaseCurve(masterARReleaseCurve * 2.0f - 1.0f);
            break;

        case kParamMasterTuning:
            // +/-4 octaves: normalized centre 0.5, exponent spans -4..+4.
            // Applies to the played stick/voice pitch only; the comb/filter
            // selectors keep their own frequencies. Also retunes notes that
            // are already sounding (host automation must be audible mid-note).
            applyMasterTuning(value);
            break;

        case kParamFinalTouchLPF:
            finalTouchLPF = std::clamp(value, 0.0f, 1.0f);
            updateFinalTouchFilters();
            break;

        case kParamFinalTouchHPF:
            finalTouchHPF = std::clamp(value, 0.0f, 1.0f);
            updateFinalTouchFilters();
            break;

        // Deprecated/reserved tail: accepted for state compatibility, no DSP.
        case kParamFinalTouchQ:
        case kParamFinalTouchHPSlope:
        case kParamFinalTouchHPChar:
        case kParamFinalTouchHPMix:
        case kParamFinalTouchHPOut:
        case kParamFinalTouchHPBypass:
        case kParamFinalTouchHPSolo:
        case kParamFinalTouchHPStereo:
        case kParamFinalTouchLPQ:
        case kParamFinalTouchLPSlope:
        case kParamFinalTouchLPChar:
        case kParamFinalTouchLPMix:
        case kParamFinalTouchLPOut:
        case kParamFinalTouchLPBypass:
        case kParamFinalTouchLPSolo:
        case kParamFinalTouchLPStereo:
            break;

        case kParamFinalClipMix:
            finalClipMix = std::clamp(value, 0.0f, 1.0f);
            break;

        case kParamFinalClipOutput:
            finalClipOutput = std::clamp(value, 0.0f, 1.0f);
            finalClipGain = std::pow(10.0f, (finalClipOutput - 0.5f) * 24.0f / 20.0f);
            break;

        case kParamFinalClipOn:
            finalClipOn = value > 0.5f ? 1.0f : 0.0f;
            m_paramBase[index] = finalClipOn;
            break;

        case kParamFinalClipType:
            finalClipType = value;
            break;

        case kParamFinalClipAmount:
            finalClipAmount = std::clamp(value, 0.0f, 1.0f);
            break;

        case kParamProcessingMode:
            processingMode = value > 0.5f ? 1.0f : 0.0f;
            m_paramBase[index] = processingMode; // host readback shows the snapped state
            break;

        case kParamModOn:
            m_modOn = value > 0.5f ? 1.0f : 0.0f;
            m_paramBase[index] = m_modOn; // host readback shows the snapped state
            break;

        // ===== Voice architecture / MIDI foundation =====
        case kParamVoiceMode: {
            const float snapped = value > 0.5f ? 1.0f : 0.0f;
            voiceMode = snapped;
            m_paramBase[index] = voiceMode; // host readback shows the snapped state
            if (snapped > 0.5f) {
                // Entering poly: start from a silent pool (shared legacy chain
                // stays untouched but idle in poly mode).
                clearAllVoices();
            } else {
                // Leaving poly: silence pool AND release the shared chain so no
                // voice state survives a mode crossing.
                clearAllVoices();
                stickBank.releaseNote();
                finalOutEnv.releaseNote();
                m_modEngine.noteOff();
            }
            break;
        }

        case kParamVoiceCount: {
            voiceCount = std::clamp(std::round(value), 2.0f, 16.0f);
            m_paramBase[index] = voiceCount;
            for (int i = (int)voiceCount; i < kMaxVoices; ++i)
                m_voices[i].active = false; // cap decrease retires excess voices instantly
            break;
        }

        case kParamChokeMode:
            chokeMode = std::clamp(std::round(value), 0.0f, 2.0f);
            m_paramBase[index] = chokeMode; // host readback shows the snapped state
            break;

        case kParamChokeSplitPoint:
            chokeSplitPoint = std::clamp(std::round(value), 0.0f, 127.0f);
            m_paramBase[index] = chokeSplitPoint; // host readback shows the snapped state
            break;

        case kParamRelVelSens:
            relVelSens = std::clamp(value, 0.0f, 1.0f);
            break;

        case kParamMpeMode:
            mpeMode = value > 0.5f ? 1.0f : 0.0f;
            m_paramBase[index] = mpeMode; // host readback shows the snapped state
            break;

        case kParamMpeBendAmount:
            mpeBendAmount = std::clamp(value, 0.0f, 1.0f);
            break;

        case kParamMpePressDrive:
            mpePressDrive = std::clamp(value, 0.0f, 1.0f);
            break;

        case kParamBowEnable:
            bowEnable = value > 0.5f ? 1.0f : 0.0f;
            m_paramBase[index] = bowEnable; // host readback shows the snapped state
            break;

        case kParamBowRate:
            bowRate = std::clamp(value, 0.0f, 1.0f);
            break;

        case kParamBowLevel:
            bowLevel = std::clamp(value, 0.0f, 1.0f);
            break;

        default:
            if (index >= kParamSlot1Type && index <= kParamSlot6Bypass) {
                const int slot = (index - kParamSlot1Type) / 16;
                const int sub  = (index - kParamSlot1Type) % 16;
                if (slot < 0 || slot >= 6) break;
                if (sub == 0) {
                    // Defer the type change to the audio thread: setType()/reset()
                    // mutate dispatch state that run() reads; applying it here (UI
                    // thread) races the audio thread. getCanonicalType is a pure
                    // static lookup and is safe to call from any thread.
                    int requested = (int)std::lround(value);
                    int canonicalInt;
                    if (CymbalsVendoredFx::isVendoredId(requested)) {
                        canonicalInt = requested;
                    } else if (CymbalsInternalFx::isInternalId(requested)) {
                        canonicalInt = requested;
                    } else if (requested >= CymbalsSurgeFx::kBaseType) {
                        const int algo = std::clamp(requested - CymbalsSurgeFx::kBaseType,
                                                    0, CymbalsSurgeFx::algorithmCount() - 1);
                        canonicalInt = CymbalsSurgeFx::algorithmId(algo);
                    } else {
                        canonicalInt = (int)AirSuiteUniversal::getCanonicalType(
                            (AirSuiteUniversal::Type)requested);
                    }
                    // Lock-free handoff (see fPendingSlotTypes): the drainer runs
                    // on the audio thread, so it must never contend for a mutex
                    // the UI can hold. Write the type, then release-store the
                    // flag so the audio thread's acquire sees it.
                    fPendingSlotTypes[slot].store(canonicalInt, std::memory_order_relaxed);
                    fPendingSlotTypeChange.store(true, std::memory_order_release);
                } else if (sub >= 1 && sub <= 13) {
                    m_slotParams[slot][sub - 1] = value;
                } else if (sub == 14) {
                    m_slotWet[slot] = value;
                } else { // sub == 15
                    m_slotBypass[slot] = (value >= 0.5f);
                }
            }
            // Phase 8: Slot process order parameters
            if (index >= kParamSlotOrder0 && index <= kParamSlotOrder5) {
                int pos = index - kParamSlotOrder0;
                int slotVal = (int)(value + 0.5f);
                if (slotVal >= 0 && slotVal < 6) {
                    m_slotProcessOrder[pos] = slotVal;
                }
                normalizeSlotProcessOrder();
            }
            break;
    }
    // Keep the poly pool banks in lockstep with any stick/MSEG/bloom-shape
    // parameter change (UI thread; pool banks are idle in mono mode).
    if ((index >= kParamStickType && index <= kParamStickPitchTimer) ||
        (index >= kParamStickPulseWidth && index <= kParamStickDetune) ||
        (index >= kParamStickAmpDecayCurve && index <= kParamStickPitchReleaseCurve) ||
        (index >= kParamStickPitchMsegOn && index <= kParamStickPitchMsegPoint6Curve)) {
        syncVoiceStickParams();
    }
    if (index == kParamInharmBloomTime || index == kParamInharm2BloomTime) {
        syncVoiceBloomEnvs();
    }
}

void PluginCymbals::applyMidiParameterWrite(uint32_t index, float value) {
    // (#17/#18) RT-safe subset of setParameterValue for MIDI-driven writes
    // (learn CCs, NRPN data entry) from run()'s 0xB0 chain. Deliberately skips
    // setParameterValue's side effects, which are not RT-safe on every index:
    //   - slot-type params stage an atomic + release flag (lock-free),
    //   - stick/MSEG params run syncVoiceStickParams (heap-allocates a vector),
    //   - EQ-shape changes only stage rebuild flags for the UI thread anyway.
    // m_paramBase is the base value the modulation engine reads each block, so
    // performance-facing targets hear learned writes immediately; structural
    // hardware state (comb/EQ/slot internals) stays host/UI-owned until the
    // next full parameter pass. Macro indices must mirror into
    // ModulationEngine because getParameterValue reports getMacro() for them.
    if (index >= kParameterCount) return;
    if (index >= kParamMacro1 && index <= kParamMacro4) {
        const float v = std::clamp(finiteOr(value, 0.0f), 0.0f, 1.0f);
        m_paramBase[index] = v;
        m_modEngine.setMacro((int)(index - kParamMacro1), v);
        return;
    }
    // (HIGH-2) Absolute write + RT-safe delta push. ORDER MATTERS: dispatch
    // BEFORE storing the new base — applyModulatedTarget computes
    // eff = clamp(m_paramBase[target] + delta), so with the OLD base still in
    // place eff = clamp(oldBase + (newV - oldBase)) = newV exactly. Storing
    // first made eff = clamp(2*newV - oldV): every intermediate learned/NRPN
    // value overshot toward 1 or undershot toward 0 while the readback (which
    // sees m_paramBase) still looked correct. The family appliers
    // (kModTargetClasses — plain member/DSP-setter dispatch, RT-safe by
    // construction) therefore see the exact absolute value, and m_paramBase
    // converges to it for later mod-math reads. Targets outside the dispatch
    // table (parameterIsContinuous false) stay base-only: they have no
    // RT-safe family path and remain host/UI-owned until the next full
    // parameter pass.
    const float newV = finiteOr(value, 0.0f);
    const float oldBase = m_paramBase[index];
    if (parameterIsContinuous(index)) {
        applyModulatedTarget(index, newV - oldBase);
    }
    m_paramBase[index] = newV;
}

void PluginCymbals::applyNrpnDataEntry(uint16_t value14) {
    // (#18) NRPN address = raw DPF parameter index; 14-bit data entry writes it.
    const uint32_t idx = ((uint32_t)nrpnSelMsb << 7) | (uint32_t)nrpnSelLsb;
    if (idx < kParameterCount) {
        applyMidiParameterWrite(idx, (float)value14 / 16383.0f);
    }
}

// ---- (#16/#17) CC -> macro / CC -> parameter bindings. Non-RT (UI/state
// thread); exclusivity and reserved-CC rules enforced here so run()'s routing
// can trust the tables unconditionally.

void PluginCymbals::setCcMacroBind(int cc, int macro) {
    if (cc < 0 || cc >= 128 || isReservedControlCc(cc)) return;
    if (macro <= 0 || macro > ModulationEngine::kNumMacros) {
        clearCcMacroBind(cc);
        return;
    }
    ccMacroBind[cc] = (uint8_t)macro;
    ccToParam[cc] = -1;   // exclusive: a CC drives either a macro or a param
}

void PluginCymbals::clearCcMacroBind(int cc) {
    if (cc >= 0 && cc < 128) ccMacroBind[cc] = 0;
}

void PluginCymbals::setCcParamBind(int cc, uint32_t paramIndex) {
    if (cc < 0 || cc >= 128 || isReservedControlCc(cc)) return;
    if (paramIndex >= kParameterCount) return;
    ccToParam[cc] = (int16_t)paramIndex;
    ccMacroBind[cc] = 0;  // exclusive
}

void PluginCymbals::clearCcParamBind(int cc) {
    if (cc >= 0 && cc < 128) ccToParam[cc] = -1;
}

int PluginCymbals::getCcParamBindForParam(uint32_t paramIndex) const {
    if (paramIndex >= kParameterCount) return -1;
    for (int cc = 0; cc < 128; ++cc) {
        if (ccToParam[cc] == (int16_t)paramIndex) return cc;
    }
    return -1;
}

void PluginCymbals::armCcLearn(int kind, int index) {
    // (#17) Arm the next free CC to capture a binding to this target.
    // Index stored before kind so an audio-thread poller that sees
    // kind >= 0 always reads a valid index (release/acquire pairing).
    if (kind == 0) {
        if (index < 0 || (uint32_t)index >= kParameterCount) return;
    } else if (kind == 1) {
        if (index < 1 || index > ModulationEngine::kNumMacros) return;
    } else {
        return;
    }
    fLearnIndex.store(index, std::memory_order_release);
    fLearnKind.store(kind, std::memory_order_release);
}

END_NAMESPACE_DISTRHO
