#include "PluginCymbals.hpp"
#include "fx/VendoredFxMeta.hpp"
#include "PresetPolicy.hpp"
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
#include <xmmintrin.h>
#include <pmmintrin.h>
#define CYMBALS_HAS_SSE 1
#endif
#include <cstdio>
#include <cmath>
#include <algorithm>
#include <chrono>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <cctype>
#include <cstdlib>
#include <cstdint>
#include <ctime>
#include <locale>
#include <iomanip>
#include <unordered_map>
#include "AudioInstrumentation.hpp"

START_NAMESPACE_DISTRHO

namespace {

static const char* kPresetParamTabs[] = {
    "STICK & EQ",
    "DISPERSION 1",
    "DISPERSION 2",
    "COMB BANK",
    "FINAL FX",
    "FX CHAIN",
    "GLOBAL"
};

static int tabBucketIndex(const char* tabName) {
    for (int i = 0; i < (int)(sizeof(kPresetParamTabs) / sizeof(kPresetParamTabs[0])); ++i) {
        if (std::strcmp(kPresetParamTabs[i], tabName) == 0) return i;
    }
    return (int)(sizeof(kPresetParamTabs) / sizeof(kPresetParamTabs[0])) - 1;
}

static const char* presetTabForParameterIndex(uint32_t index) {
    using P = PluginCymbals;

    if (index <= P::kParamEQMix ||
        index == P::kParamTiltAmount ||
        index == P::kParamTiltPivot ||
        // UnifiedFilter per-band Model/FilterType/Slope/Sat + legacy Advanced EQ
        // params sit after the FX-chain block in the enum; route them back here.
        (index >= P::kParamEQ1AdvType && index <= P::kParamFinalEQ3AdvCharacter) ||
        (index >= P::kParamEQ1Model && index <= P::kParamFinalEQ5Slope) ||
        (index >= P::kParamEQ1Sat && index <= P::kParamFinalEQ5Sat) ||
        (index >= P::kParamStickAmpDecayCurve && index <= P::kParamStickPitchReleaseCurve))
        return "STICK & EQ";

    if ((index >= P::kParamDispFB && index <= P::kParamPDMix) ||
        (index >= P::kParamNudge1 && index <= P::kParamNudgeAmount20) ||
        index == P::kParamDisp1Skip ||
        index == P::kParamDispCrossLink ||
        index == P::kParamDispDelayMode) {
        return "DISPERSION 1";
    }

    if ((index >= P::kParamDisp2FB && index <= P::kParamPD2Mix) ||
        (index >= P::kParamNudge2_1 && index <= P::kParamNudgeAmount2_20) ||
        (index >= P::kParamInharm2BloomAmt && index <= P::kParamInharm2BloomDir) ||
        index == P::kParamDisp2Skip ||
        index == P::kParamDisp2DelayMode) {
        return "DISPERSION 2";
    }

    if ((index >= P::kParamComb1Freq && index <= P::kParamCombScale) ||
        (index >= P::kParamComb1PartRelPos && index <= P::kParamComb40PartCount) ||
        (index >= P::kParamCombGroup1Cluster && index <= P::kParamCombGroup10Cluster) ||
        // Per-group limiter: same bank, same preset page as the group
        // gain/on/cluster it is grouped with in the UI header.
        (index >= P::kParamCombGroup1LimiterOn && index <= P::kParamCombGroup10LimiterRelease) ||
        index == P::kParamResonatorDiffusion ||
        index == P::kParamCombMemoryDamping) {
        return "COMB BANK";
    }

    if ((index >= P::kParamFinalComb1Freq && index <= P::kParamFinalTiltPivot) ||
        (index >= P::kParamFinalComb1PartRelPos && index <= P::kParamFinalComb4PartCount) ||
        index == P::kParamDispSolo) {
        return "FINAL FX";
    }

    if (index >= P::kParamSlot1Type && index <= P::kParamSlot6Bypass) {
        return "FX CHAIN";
    }
    if (index >= P::kParamFinalARDecayCurve && index <= P::kParamMasterARReleaseCurve) {
        return "FX CHAIN";
    }

    return "GLOBAL";
}

// Compile-time guard: ensure the parameter enum bounds haven't drifted.
// If kParameterCount or kParamSlotOrder5 change unexpectedly these will fire.
static_assert(PluginCymbals::kParameterCount > 0,
    "kParameterCount must be positive — check the parameter enum in PluginCymbals.hpp");
static_assert(PluginCymbals::kParamSlotOrder5 < PluginCymbals::kParameterCount,
    "kParamSlotOrder5 is out of bounds — a parameter enum shuffle has invalidated presetTabForParameterIndex()");

static bool isDisp2DynamicInharmParam(uint32_t index) {
    return index >= PluginCymbals::kParamInharm2BloomAmt &&
           index <= PluginCymbals::kParamInharm2BloomDir;
}

// Collect + dedup + sum active route deltas into (targets, deltas); returns the count.
// Shared by the run() and IR-export loops. 0..10 = mod sources, 11..14 = macros 1..4.
static int collectModRouteDeltas(const ModulationEngine& engine, const float* srcOut,
                                 uint32_t* accTargets, float* accDeltas) {
    int accCount = 0;
    for (int r = 0; r < ModulationEngine::kMaxActiveRoutes; ++r) {
        const ModulationEngine::Route& rt = engine.route(r);
        if (!rt.enabled) continue;
        const float src = (rt.source >= ModulationEngine::kNumSources)
                              ? engine.macro(rt.source - ModulationEngine::kNumSources)
                              : srcOut[rt.source];
        const float macroScale = rt.macro > 0 ? engine.macro(rt.macro - 1) : 1.0f;
        const float delta = rt.amount * macroScale * src;
        if (delta == 0.0f) continue;
        int found = -1;
        for (int k = 0; k < accCount; ++k) {
            if (accTargets[k] == rt.target) { found = k; break; }
        }
        if (found >= 0) accDeltas[found] += delta;
        else {
            accTargets[accCount] = rt.target;
            accDeltas[accCount] = delta;
            ++accCount;
        }
    }
    return accCount;
}

static bool isAdvancedEqCompatParam(uint32_t index) {
    return (index >= PluginCymbals::kParamEQ1AdvType && index <= PluginCymbals::kParamEQ3AdvRes) ||
           (index >= PluginCymbals::kParamFinalEQ1AdvType && index <= PluginCymbals::kParamFinalEQ3AdvRes);
}

static std::vector<float> remapLegacyPositionalValues(const std::vector<float>& parsed,
                                                      const std::vector<float>& canonicalDefaults) {
    std::vector<float> remapped = canonicalDefaults;

    if (parsed.size() >= PluginCymbals::kParameterCount) {
        remapped.assign(parsed.begin(), parsed.begin() + PluginCymbals::kParameterCount);
        return remapped;
    }

    const size_t missing = PluginCymbals::kParameterCount - parsed.size();
    const bool missingDisp2Inharm = (missing == 3 || missing == 15);
    const bool missingAdvancedEq = (missing == 12 || missing == 15);
    if (!missingDisp2Inharm) {
        if (!missingAdvancedEq) {
            for (size_t i = 0; i < parsed.size(); ++i) {
                remapped[i] = parsed[i];
            }
            return remapped;
        }

        size_t src = 0;
        for (uint32_t dst = 0; dst < PluginCymbals::kParameterCount && src < parsed.size(); ++dst) {
            if (isAdvancedEqCompatParam(dst)) {
                continue;
            }
            remapped[dst] = parsed[src++];
        }
        return remapped;
    }

    size_t src = 0;
    for (uint32_t dst = 0; dst < PluginCymbals::kParameterCount && src < parsed.size(); ++dst) {
        if (isDisp2DynamicInharmParam(dst)) {
            continue;
        }
        if (missingAdvancedEq && isAdvancedEqCompatParam(dst)) {
            continue;
        }
        remapped[dst] = parsed[src++];
    }
    return remapped;
}

static inline float applySoftClip(float x, bool fastMode) {
    if (!fastMode) {
        return std::tanh(x);
    }
    x = std::clamp(x, -4.0f, 4.0f);
    return x / (1.0f + std::abs(x));
}

static std::string jsonEscape(const std::string& in) {
    std::string out;
    out.reserve(in.size() + 16);
    for (char c : in) {
        switch (c) {
            case '\\': out += "\\\\"; break;
            case '"': out += "\\\""; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default: out += c; break;
        }
    }
    return out;
}

static std::string trim(const std::string& s) {
    size_t b = 0;
    while (b < s.size() && std::isspace((unsigned char)s[b])) ++b;
    size_t e = s.size();
    while (e > b && std::isspace((unsigned char)s[e - 1])) --e;
    return s.substr(b, e - b);
}

static std::vector<std::string> splitTagsCsv(const std::string& csv) {
    std::vector<std::string> out;
    std::string token;
    std::istringstream ss(csv);
    while (std::getline(ss, token, ',')) {
        token = trim(token);
        if (!token.empty()) out.push_back(token);
    }
    return out;
}

static std::string joinTagsCsv(const std::vector<std::string>& tags) {
    std::string out;
    for (size_t i = 0; i < tags.size(); ++i) {
        if (i > 0) out += ",";
        out += tags[i];
    }
    return out;
}

static std::string sanitizeFileName(const std::string& name) {
    std::string out;
    out.reserve(name.size());
    for (char c : name) {
        const bool ok = std::isalnum((unsigned char)c) || c == '-' || c == '_';
        if (ok) out.push_back(c);
        else if (std::isspace((unsigned char)c)) out.push_back('_');
    }
    if (out.empty()) out = "cymbals_ir";
    return out;
}

static bool writeWavStereo16(const std::string& path, const std::vector<float>& left, const std::vector<float>& right, float sampleRate, std::string& outError) {
    outError.clear();
    if (left.size() != right.size()) {
        outError = "IR channel length mismatch";
        return false;
    }

    const uint32_t frames = (uint32_t)left.size();
    const uint16_t channels = 2;
    const uint16_t bitsPerSample = 16;
    const uint16_t blockAlign = (uint16_t)(channels * (bitsPerSample / 8));
    const uint32_t sampleRateInt = (uint32_t)std::max(1.0f, sampleRate);
    const uint32_t byteRate = sampleRateInt * blockAlign;
    const uint32_t dataBytes = frames * blockAlign;
    const uint32_t riffBytes = 36 + dataBytes;

    const std::filesystem::path p = std::filesystem::u8path(path);
    std::ofstream file(p, std::ios::binary | std::ios::trunc);
    if (!file.is_open()) {
        outError = "Failed to open WAV file for writing";
        return false;
    }

    auto writeU16 = [&](uint16_t v) { file.write(reinterpret_cast<const char*>(&v), sizeof(v)); };
    auto writeU32 = [&](uint32_t v) { file.write(reinterpret_cast<const char*>(&v), sizeof(v)); };

    file.write("RIFF", 4);
    writeU32(riffBytes);
    file.write("WAVE", 4);

    file.write("fmt ", 4);
    writeU32(16);
    writeU16(1);
    writeU16(channels);
    writeU32(sampleRateInt);
    writeU32(byteRate);
    writeU16(blockAlign);
    writeU16(bitsPerSample);

    file.write("data", 4);
    writeU32(dataBytes);

    for (uint32_t i = 0; i < frames; ++i) {
        const float l = std::clamp(left[i], -1.0f, 1.0f);
        const float r = std::clamp(right[i], -1.0f, 1.0f);
        const int16_t li = (int16_t)std::lrint(l * 32767.0f);
        const int16_t ri = (int16_t)std::lrint(r * 32767.0f);
        file.write(reinterpret_cast<const char*>(&li), sizeof(li));
        file.write(reinterpret_cast<const char*>(&ri), sizeof(ri));
    }

    if (!file.good()) {
        outError = "Failed while writing WAV data";
        return false;
    }
    return true;
}

static bool parseJsonStringField(const std::string& json, const char* field, std::string& out) {
    out.clear();
    const std::string key = std::string("\"") + field + "\"";
    const size_t k = json.find(key);
    if (k == std::string::npos) return false;
    const size_t colon = json.find(':', k + key.size());
    if (colon == std::string::npos) return false;
    const size_t q1 = json.find('"', colon + 1);
    if (q1 == std::string::npos) return false;

    std::string value;
    bool esc = false;
    for (size_t i = q1 + 1; i < json.size(); ++i) {
        char c = json[i];
        if (esc) {
            switch (c) {
                case 'n': value += '\n'; break;
                case 'r': value += '\r'; break;
                case 't': value += '\t'; break;
                case '\\': value += '\\'; break;
                case '"': value += '"'; break;
                default: value += c; break;
            }
            esc = false;
            continue;
        }
        if (c == '\\') { esc = true; continue; }
        if (c == '"') {
            out = value;
            return true;
        }
        value += c;
    }
    return false;
}

static bool extractJsonValueSpan(const std::string& json, const char* field, size_t& outStart, size_t& outEnd) {
    outStart = outEnd = std::string::npos;
    const std::string key = std::string("\"") + field + "\"";
    const size_t k = json.find(key);
    if (k == std::string::npos) return false;
    const size_t colon = json.find(':', k + key.size());
    if (colon == std::string::npos) return false;

    size_t p = colon + 1;
    while (p < json.size() && std::isspace((unsigned char)json[p])) ++p;
    if (p >= json.size()) return false;

    const char open = json[p];
    char close = 0;
    if (open == '{') close = '}';
    else if (open == '[') close = ']';
    else return false;

    int depth = 0;
    bool inStr = false;
    bool esc = false;
    for (size_t i = p; i < json.size(); ++i) {
        const char c = json[i];
        if (inStr) {
            if (esc) { esc = false; continue; }
            if (c == '\\') { esc = true; continue; }
            if (c == '"') inStr = false;
            continue;
        }
        if (c == '"') { inStr = true; continue; }
        if (c == open) ++depth;
        else if (c == close) {
            --depth;
            if (depth == 0) {
                outStart = p;
                outEnd = i;
                return true;
            }
        }
    }
    return false;
}

static bool parseJsonStringNumberObject(const std::string& obj, std::unordered_map<std::string, float>& outMap, std::string& outError) {
    outMap.clear();
    outError.clear();

    if (obj.size() < 2 || obj.front() != '{' || obj.back() != '}') {
        outError = "Invalid parameter object";
        return false;
    }

    size_t p = 1;
    while (p < obj.size() - 1) {
        while (p < obj.size() - 1 && (std::isspace((unsigned char)obj[p]) || obj[p] == ',')) ++p;
        if (p >= obj.size() - 1) break;
        if (obj[p] != '"') {
            outError = "Expected parameter key string";
            return false;
        }

        // Parse key string with basic JSON escapes.
        std::string key;
        ++p;
        bool esc = false;
        for (; p < obj.size() - 1; ++p) {
            const char c = obj[p];
            if (esc) {
                switch (c) {
                    case 'n': key.push_back('\n'); break;
                    case 'r': key.push_back('\r'); break;
                    case 't': key.push_back('\t'); break;
                    case '\\': key.push_back('\\'); break;
                    case '"': key.push_back('"'); break;
                    default: key.push_back(c); break;
                }
                esc = false;
                continue;
            }
            if (c == '\\') { esc = true; continue; }
            if (c == '"') break;
            key.push_back(c);
        }
        if (p >= obj.size() - 1 || obj[p] != '"') {
            outError = "Unterminated parameter key string";
            return false;
        }
        ++p;

        while (p < obj.size() - 1 && std::isspace((unsigned char)obj[p])) ++p;
        if (p >= obj.size() - 1 || obj[p] != ':') {
            outError = "Missing ':' after parameter key";
            return false;
        }
        ++p;

        while (p < obj.size() - 1 && std::isspace((unsigned char)obj[p])) ++p;
        if (p >= obj.size() - 1) {
            outError = "Missing parameter value";
            return false;
        }

        size_t numLen = 0;
        while ((p + numLen) < (obj.size() - 1)) {
            const char c = obj[p + numLen];
            const bool isNumChar = (c >= '0' && c <= '9') || c == '+' || c == '-' || c == '.' || c == 'e' || c == 'E';
            if (!isNumChar) break;
            ++numLen;
        }
        if (numLen == 0) {
            outError = "Invalid numeric value in parameters object";
            return false;
        }

        // Use a locale-independent way to parse the number.
        // std::strtod is locale-dependent, which breaks on systems where ',' is the decimal separator.
        std::istringstream iss(obj.substr(p, numLen));
        iss.imbue(std::locale::classic());
        double v = 0.0;
        if (!(iss >> v)) {
            outError = "Invalid numeric value in parameters object";
            return false;
        }
        if (!std::isfinite(v)) {
            outError = "Non-finite numeric value in parameters object";
            return false;
        }
        
        outMap[key] = (float)v;

        p += numLen;
    }

    return true;
}

static bool parseGroupedParametersObject(const std::string& obj,
                                         std::unordered_map<std::string, float>& outMap,
                                         std::string& outError) {
    outMap.clear();
    outError.clear();

    bool anyTabFound = false;
    for (const char* tabName : kPresetParamTabs) {
        size_t s = std::string::npos;
        size_t e = std::string::npos;
        if (!extractJsonValueSpan(obj, tabName, s, e) || s == std::string::npos || e == std::string::npos) {
            continue;
        }
        anyTabFound = true;
        const std::string subObj = obj.substr(s, e - s + 1);
        if (subObj.empty() || subObj.front() != '{') continue;

        std::unordered_map<std::string, float> tabMap;
        std::string tabErr;
        if (!parseJsonStringNumberObject(subObj, tabMap, tabErr)) {
            outError = std::string("Invalid tab parameter block '") + tabName + "': " + tabErr;
            return false;
        }
        for (const auto& kv : tabMap) outMap[kv.first] = kv.second;
    }

    if (!anyTabFound) {
        outError = "No tab parameter groups found";
        return false;
    }
    return true;
}

static bool parseJsonStringArrayField(const std::string& json, const char* field, std::vector<std::string>& out) {
    out.clear();
    const std::string key = std::string("\"") + field + "\"";
    const size_t k = json.find(key);
    if (k == std::string::npos) return false;
    const size_t lbr = json.find('[', k + key.size());
    const size_t rbr = json.find(']', lbr == std::string::npos ? k : lbr);
    if (lbr == std::string::npos || rbr == std::string::npos || rbr <= lbr) return false;

    size_t i = lbr + 1;
    while (i < rbr) {
        const size_t q1 = json.find('"', i);
        if (q1 == std::string::npos || q1 >= rbr) break;
        std::string value;
        bool esc = false;
        size_t j = q1 + 1;
        for (; j < rbr; ++j) {
            const char c = json[j];
            if (esc) {
                switch (c) {
                    case 'n': value += '\n'; break;
                    case 'r': value += '\r'; break;
                    case 't': value += '\t'; break;
                    case '\\': value += '\\'; break;
                    case '"': value += '"'; break;
                    default: value += c; break;
                }
                esc = false;
                continue;
            }
            if (c == '\\') { esc = true; continue; }
            if (c == '"') break;
            value += c;
        }
        if (!value.empty()) out.push_back(value);
        i = j + 1;
    }
    return true;
}

static size_t boundedCStringLength(const char* s, size_t maxBytes) {
    if (s == nullptr) return 0;
    size_t n = 0;
    while (n < maxBytes && s[n] != '\0') ++n;
    return n;
}

}

PluginCymbals::PluginCymbals()
    : Plugin(kParameterCount, 5, 6),
    tone(0.15f), decay(0.85f), volume(0.85f), dispFB(0.99990785f), dispDelay(718.0f / 2000.0f), dispGain(1.0f), dispFBOverall(1.0f),
      vibRate(0.25075f), vibDepth(0.18f), vibShape(1.0f), vibSmooth(0.672f), vibStepMix(1.0f), chaosFactor(0.0f), chaosEnvAmount(0.0f), chaosEnvDecay(0.1f),
      pdDrive(0.72f), pdNormalize(0.5f), pdTone(0.598f), pdBias(0.5f), pdSpread(0.0f), pdMix(1.0f),
      disp1Skip(0.0f), disp2Skip(1.0f), dispCrossLink(0.0f), dispSolo(0.0f), dispDelayMode(0.0f), disp2DelayMode(0.0f),
      inharmBloomAmt(0.12f), inharmBloomTime(0.10f), inharmBloomDir(0.0f),
      inharm2BloomAmt(0.0f), inharm2BloomTime(0.10f), inharm2BloomDir(0.0f),
      resonatorDiffusion(0.0f), combMemoryDamping(0.0f),
      disp2FB(0.99990785f), disp2Delay(0.15f), disp2Gain(0.25f), disp2FBOverall(1.0f),
      vib2Rate(0.3f), vib2Depth(0.2f), vib2Shape(1.0f), vib2Smooth(0.5f), vib2StepMix(1.0f),
      chaos2Factor(0.0f), chaos2EnvAmount(0.0f), chaos2EnvDecay(0.1f),
      msegPitchOn(1.0f), msegPitchDuration(0.1f),
      finalClipOn(0.0f), finalClipType(0.0f), finalClipAmount(0.0f),
      pd2Drive(0.5f), pd2Normalize(0.5f), pd2Tone(0.5f), pd2Bias(0.5f), pd2Spread(0.0f), pd2Mix(1.0f),
      eqMix(1.0f), stickTiltAmount(0.5f), stickTiltPivot(2.0f/3.0f),
      feqMix(1.0f), finalEQOn(1.0f), finalTiltAmount(0.5f), finalTiltPivot(2.0f/3.0f),
      stickType(0.0f), stickDirect(0.35f), stickBus(1.0f),
      stickAmpAttack(0.001f), stickAmpDecay(0.37f), stickAmpSustain(0.0f), stickAmpRelease(0.01f),
      stickAmpDecayCurve(0.5f), stickAmpReleaseCurve(0.5f),
      stickPitchAttack(0.001f), stickPitchDecay(0.1f), stickPitchSustain(0.0f), stickPitchRelease(0.1f),
      stickPitchAmount(1.0f), stickPitchTimer(0.05f),
      stickPitchDecayCurve(0.5f), stickPitchReleaseCurve(0.5f),
      finalAROn(0.0f), finalARAttack(0.04f), finalARDecay(0.25f), finalARSustain(0.8f), finalARRelease(0.18f),
      finalARDecayCurve(0.5f), finalARReleaseCurve(0.5f),
      masterAROn(0.0f), masterARAttack(0.01f), masterARDecay(0.25f), masterARSustain(1.0f), masterARRelease(0.2f),
      masterARDecayCurve(0.5f), masterARReleaseCurve(0.5f),
      masterTuning(0.5f), masterTuneRatio(1.0f),
      finalTouchLPF(1.0f), finalTouchHPF(0.0f),
      finalClipMix(1.0f), finalClipOutput(0.5f), finalClipGain(1.0f),
      combBankOn(1.0f), finalCombBankOn(1.0f), processingMode(0.0f),
      analysisPoint(4.0f),
      combBank(44100.0f), finalCombBank(44100.0f), dispersion(44100.0f), dispersion2(44100.0f),
      stickBank(44100.0f),
      lastNoteFreq(220.0f), lastVelocity(1.0f)
{
    // A fresh instance opens in the INIT parameter state. Publish that identity
    // immediately so the preset header/browser shows INIT instead of a blank
    // name until the host or the user loads a file.
    fPresetName = "INIT";
    std::memset(fActiveNotes, 0, sizeof(fActiveNotes));
    fActiveNoteCount = 0;
    for (int i = 0; i < 16; ++i) m_voiceChannelMap[i] = -1;
    fMsegScratch.reserve(8);   // (MED-3) MSEG push scratch: capacity = point count
    for (int i = 0; i < 128; ++i) ccToParam[i] = -1;

    fAnalyzer = new FFTAnalyzer(44100.0f);
    for (int i = 0; i < FFTAnalyzer::kNumBands; ++i) fAnalyzerBandPeaks[i].store(0.0f);
    
    // Initialize slot state
    for (int i = 0; i < 6; ++i) {
        m_slotWet[i]    = 1.0f;
        m_slotBypass[i] = false;
        m_slotProcessOrder[i] = i;  // Phase 8: Initialize to identity order
        for (int k = 0; k < 13; ++k)
            m_slotParams[i][k] = 0.5f;
    }
    
    for (int i = 0; i < 3; ++i) {
        eqAdvType[i] = 0.0f; eqAdvRes[i] = 0.5f;
        eqAdvDrive[i] = 0.0f; eqAdvCharacter[i] = 0.5f;
        feqAdvType[i] = 0.0f; feqAdvRes[i] = 0.5f;
        feqAdvDrive[i] = 0.0f; feqAdvCharacter[i] = 0.5f;
    }
    for (int i = 0; i < 5; ++i) {
        eqGain[i] = 0.5f; eqQ[i] = 0.5f;
        eqModel[i] = 0.0f; eqSlope[i] = 0.0f; eqSat[i] = 0.0f;
        feqGain[i] = 0.5f; feqQ[i] = 0.5f;
        feqModel[i] = 0.0f; feqSlope[i] = 0.0f; feqSat[i] = 0.0f;
    }
    // Endpoint HP/LP nodes default to a low Q (0.5) so their response stays flat;
    // middle bell nodes keep the resonant 0.5 norm (Q ~5.05).
    eqQ[0] = eqQ[4] = feqQ[0] = feqQ[4] = 0.0404f;
    // Default frequencies: evenly spaced on log scale (200Hz to 16kHz)
    // Formula: norm = log(hz/20) / log(1000)
    eqFreq[0] = 0.333f;  // 200Hz
    eqFreq[1] = 0.499f;  // 630Hz
    eqFreq[2] = 0.667f;  // 2kHz
    eqFreq[3] = 0.833f;  // 6.3kHz
    eqFreq[4] = 0.968f;  // 16kHz
    feqFreq[0] = 0.333f; feqFreq[1] = 0.499f; feqFreq[2] = 0.667f;
    feqFreq[3] = 0.833f; feqFreq[4] = 0.968f;
    eqFilterType[0] = 2.0f;  eqFilterType[1] = 4.0f;  eqFilterType[2] = 4.0f;
    eqFilterType[3] = 4.0f;  eqFilterType[4] = 0.0f;
    feqFilterType[0] = 2.0f; feqFilterType[1] = 4.0f; feqFilterType[2] = 4.0f;
    feqFilterType[3] = 4.0f; feqFilterType[4] = 0.0f;
    for (int i = 0; i < 20; ++i) { 
        nudge[i] = 0.5f; nudgeAmount[i] = 0.0f; 
        nudge2[i] = 0.5f; nudgeAmount2[i] = 0.0f; 
    }
    nudge[0] = 0.6411f; nudgeAmount[0] = 0.8f; nudge[1] = 0.3136f; nudgeAmount[1] = 0.8f; nudge2[0] = 0.45f; nudgeAmount2[0] = 0.8f;
    for (int i = 0; i < 40; ++i) {
        combScalePerStage[i] = 0.5f;
        combSatType[i] = 0.0f;
        combFilterType[i] = 0.0f;
        combModeRate[i] = 0.5f;
        combModeAmount[i] = 0.6f;
        combPartRelPos[i] = (0.1f - 0.0001f) / (0.999f - 0.0001f); // normalized Saike rel_position default 0.1
        combPartFreqDamp[i] = 0.5f;      // normalized freq_damp default -3.5
        combPartStiffness[i] = 0.65f;    // normalized stiffness default 4.6
        combPartStiffExp[i] = 0.65f;     // normalized stiffness_exp default 2.3
        combPartCount[i] = 1.0f/3.0f;    // normalized partials default 32
        combSatAmount[i] = 0.0f;
        combStressOnset[i] = 0.86f;
        combStressKnee[i] = 0.995f;
        combMix[i] = 1.0f;
        combSolo[i] = 0.0f;
        // Only enable the first 2 groups (8 stages) by default — matches fNumCombStages=8
        combEnable[i] = (i < 8) ? 1.0f : 0.0f;
        combBank.setStageEnabled(i, i < 8);
        if (i < 4) { 
            static const float g1Offs[4] = {0.0f, 0.005f, 0.012f, -0.008f}; 
            combParams[i*4+0] = 0.3f + g1Offs[i]; combParams[i*4+1] = 0.985f; combParams[i*4+2] = 1.0f; combParams[i*4+3] = 0.43f; 
        }
        else if (i < 8) { 
            static const float g2Freqs[4] = {0.3528f, 0.3789f, 0.4012f, 0.4920f}; 
            combParams[i*4+0] = g2Freqs[i-4]; combParams[i*4+1] = 0.985f; combParams[i*4+2] = 1.0f; combParams[i*4+3] = 0.43f; 
        }
        else { combParams[i*4+0] = 0.5f; combParams[i*4+1] = 0.0f; combParams[i*4+2] = 1.0f; combParams[i*4+3] = 0.0f; }
    }
    for (int i = 0; i < 10; ++i) { combGroupShift[i] = 0.5f; combGroupGain[i] = 0.85f; combBankSeries[i] = 0.0f; combGroupCluster[i] = 0.0f; combGroupOn[i] = 1.0f; }
    // Per-group limiter defaults: OFF (bit-transparent) everywhere; threshold
    // -6 dB and a ~155 ms release are the values a user lands on when they
    // first raise the On toggle.
    for (int i = 0; i < 10; ++i) {
        combGroupLimiterOn[i] = 0.0f;
        combGroupLimiterThreshold[i] = 0.75f;
        combGroupLimiterRelease[i] = 0.15f;
        combBank.setGroupLimiterOn(i, combGroupLimiterOn[i]);
        combBank.setGroupLimiterThreshold(i, combGroupLimiterThreshold[i]);
        combBank.setGroupLimiterRelease(i, combGroupLimiterRelease[i]);
    }
    for (int i = 0; i < 4; ++i) { finalCombParams[i*4+0] = 0.4f+i*0.1f; finalCombParams[i*4+1] = 0.2f; finalCombParams[i*4+2] = 1.0f; finalCombParams[i*4+3] = 0.43f; }
    finalComb4SatType = 0.0f;
    finalComb4SatAmount = 0.0f;
    finalComb4StressOnset = 0.86f;
    finalComb4StressKnee = 0.995f;
    for (int i = 0; i < 4; ++i) { finalCombMix[i] = 1.0f; finalCombSolo[i] = 0.0f; finalCombEnable[i] = 1.0f; finalCombFilterType[i] = 0.0f; finalCombModeRate[i] = 0.5f; finalCombModeAmount[i] = 0.6f; finalCombPartRelPos[i] = (0.1f - 0.0001f) / (0.999f - 0.0001f); finalCombPartFreqDamp[i] = 0.5f; finalCombPartStiffness[i] = 0.65f; finalCombPartStiffExp[i] = 0.65f; finalCombPartCount[i] = 1.0f/3.0f; finalCombBank.setStageEnabled(i, true); finalCombBank.setFilterType(i, 0); }
    finalCombBankSeries = 0.0f; finalCombShift = 0.5f; finalCombGroupGain = 0.85f; finalCombGroupCluster = 0.0f; combScale = 0.5f; finalCombScale = 0.5f;
    for (int i = 0; i < 8; ++i) msegPitchValues[i] = 0.0f;
    for (int i = 0; i < 6; ++i) msegPitchTimes[i] = (i + 1) / 7.0f;
    for (int i = 0; i < 7; ++i) msegPitchCurves[i] = 0.5f;
    msegPitchValues[0] = 1.0f; msegPitchValues[1] = 0.8f; msegPitchValues[2] = 0.5f;
    msegPitchTimes[0] = 0.02f; msegPitchTimes[1] = 0.10f;
    sampleRateChanged(44100.0);
    // Seed the modulation base array from initParameter defaults so
    // getParameterValue returns the correct base before any host write.
    for (uint32_t i = 0; i < kParameterCount; ++i) {
        Parameter p;
        initParameter(i, p);
        m_paramBase[i] = p.ranges.def;
    }
    for (int i = 0; i < kParameterCount; ++i) setParameterValue(i, getParameterValue(i));
    updateStickEQ();
    updateFinalEQ();
    fPendingStickEQRebuild.store(false, std::memory_order_release);
    fPendingFinalEQRebuild.store(false, std::memory_order_release);
    fFxPrepareSampleRate.store(fSampleRate, std::memory_order_relaxed);
    fFxPrepareThread = std::thread(&PluginCymbals::fxPrepareWorkerLoop, this);
}

PluginCymbals::~PluginCymbals() { 
    fFxPrepareQuit.store(true, std::memory_order_release);
    if (fFxPrepareThread.joinable()) fFxPrepareThread.join();
    for (int i = 0; i < 6; ++i) {
        delete fFxPrepared[i].exchange(nullptr, std::memory_order_acq_rel);
        delete fFxRetired[i].exchange(nullptr, std::memory_order_acq_rel);
    }
    delete fAnalyzer; 
}

void PluginCymbals::prepareFxObject(int type, float sampleRate, FxPrepared& out) {
    out.type = type;
    if (CymbalsVendoredFx::isVendoredId(type)) {
        out.vendored = std::make_unique<VendoredFxSlot>(type, sampleRate);
    } else if (CymbalsInternalFx::isInternalId(type)) {
        out.internal = std::make_unique<CymbalsInternalFxSlot>(type, sampleRate);
    } else if (type >= CymbalsSurgeFx::kBaseType) {
        const int algo = std::clamp(type - CymbalsSurgeFx::kBaseType, 0,
                                    CymbalsSurgeFx::algorithmCount() - 1);
        out.surge = CymbalsSurgeFx::createInstance(algo, sampleRate);
    }
}

void PluginCymbals::fxPrepareWorkerLoop() {
    using namespace std::chrono_literals;
    while (!fFxPrepareQuit.load(std::memory_order_acquire)) {
        for (int i = 0; i < 6; ++i) {
            delete fFxRetired[i].exchange(nullptr, std::memory_order_acq_rel);
            const int request = fFxPrepareRequest[i].exchange(-1, std::memory_order_acq_rel);
            if (request < 0) continue;
            auto prepared = std::make_unique<FxPrepared>();
            prepareFxObject(request, fFxPrepareSampleRate.load(std::memory_order_relaxed), *prepared);
            FxPrepared* expected = nullptr;
            if (fFxPrepared[i].compare_exchange_strong(expected, prepared.get(),
                                                       std::memory_order_acq_rel)) {
                (void)prepared.release();
            }
            // If a prepared object already exists, let the audio consume it;
            // it will re-request this type if it was stale.
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    for (int i = 0; i < 6; ++i) {
        delete fFxPrepared[i].exchange(nullptr, std::memory_order_acq_rel);
        delete fFxRetired[i].exchange(nullptr, std::memory_order_acq_rel);
    }
}
const char* PluginCymbals::getLabel() const { return "MadMcMan's BCE (Best Cymbals Ever)"; }
const char* PluginCymbals::getMaker() const { return "MadMcMans"; }
const char* PluginCymbals::getLicense() const { return "MIT"; }
uint32_t PluginCymbals::getVersion() const { return 0x010000; }
int64_t PluginCymbals::getUniqueId() const { return d_cconst('C', 'y', 'm', 'b'); }

void PluginCymbals::initAudioPort(bool input, uint32_t index, AudioPort& port) {
    if (!input) { if (index == 0) { port.name = "Left"; port.symbol = "out_l"; } else { port.name = "Right"; port.symbol = "out_r"; } }
    Plugin::initAudioPort(input, index, port);
}

void PluginCymbals::sampleRateChanged(double sr) {
    if (sr <= 0.0) return;
    fSampleRate = (float)sr;
    fFxPrepareSampleRate.store(fSampleRate, std::memory_order_relaxed);
    if (fAnalyzer) fAnalyzer->setSampleRate(fSampleRate);
    combBank.setSampleRate(fSampleRate); finalCombBank.setSampleRate(fSampleRate);
    dispersion.setSampleRate(fSampleRate); dispersion2.setSampleRate(fSampleRate);
    stickBank.setSampleRate(fSampleRate);
    m_modEngine.setSampleRate(fSampleRate);
    for (int i = 0; i < kMaxVoices; ++i) {
        m_voices[i].stick.setSampleRate(fSampleRate);
        m_voices[i].bloomEnv.setSampleRate(fSampleRate);
        m_voices[i].bloom2Env.setSampleRate(fSampleRate);
    }
    
    for (int i = 0; i < 6; ++i) {
        m_fxSlots[i].setSampleRate(fSampleRate);
        if (m_surgeFx[i] != nullptr) m_surgeFx[i]->setSampleRate(fSampleRate);
        if (m_internalFx[i] != nullptr) m_internalFx[i]->setSampleRate(fSampleRate);
        if (m_vendoredFx[i] != nullptr) m_vendoredFx[i]->setSampleRate(fSampleRate);
    }

    for (int i = 0; i < 5; ++i) {
        stickEQNodes[i].setSampleRate(fSampleRate);
        finalEQNodes[i].setSampleRate(fSampleRate);
    }
    
    inharmBloomEnv.setSampleRate(fSampleRate);
    inharmBloomEnv.setAttack(0.0015f);
    inharmBloomEnv.setDecay(DSPUtils::inharmBloomDecayFromControl(inharmBloomTime));
    inharmBloomEnv.setSustain(0.0f);
    inharmBloomEnv.setRelease(0.02f);
    inharm2BloomEnv.setSampleRate(fSampleRate);
    inharm2BloomEnv.setAttack(0.0015f);
    inharm2BloomEnv.setDecay(DSPUtils::inharmBloomDecayFromControl(inharm2BloomTime));
    inharm2BloomEnv.setSustain(0.0f);
    inharm2BloomEnv.setRelease(0.02f);
    finalCombBank.setActiveGroupCount(1);
    finalOutEnv.setSampleRate(fSampleRate);
    masterOutEnv.setSampleRate(fSampleRate);
    syncFinalOutEnv();
    syncMasterOutEnv();
    updateFinalTouchFilters();
    // Pool voices mirror the global bloom envelope shapes (fixed parts here,
    // decay re-synced from the time params in syncVoiceBloomEnvs()).
    for (int i = 0; i < kMaxVoices; ++i) {
        m_voices[i].bloomEnv.setAttack(0.0015f);
        m_voices[i].bloomEnv.setDecay(DSPUtils::inharmBloomDecayFromControl(inharmBloomTime));
        m_voices[i].bloomEnv.setSustain(0.0f);
        m_voices[i].bloomEnv.setRelease(0.02f);
        m_voices[i].bloom2Env.setAttack(0.0015f);
        m_voices[i].bloom2Env.setDecay(DSPUtils::inharmBloomDecayFromControl(inharm2BloomTime));
        m_voices[i].bloom2Env.setSustain(0.0f);
        m_voices[i].bloom2Env.setRelease(0.02f);
    }
    syncVoiceStickParams();
    updateStickEQ(); updateFinalEQ();
    fPendingStickEQRebuild.store(false, std::memory_order_release);
    fPendingFinalEQRebuild.store(false, std::memory_order_release);
}

void PluginCymbals::applyPendingEQRebuilds() {
    if (fPendingStickEQRebuild.exchange(false, std::memory_order_acq_rel)) {
        updateStickEQ();
    }
    if (fPendingFinalEQRebuild.exchange(false, std::memory_order_acq_rel)) {
        updateFinalEQ();
    }
}

void PluginCymbals::applyPendingSlotTypeChanges() {
    // NO mutex here: this runs on the AUDIO THREAD (called from run()), and the
    // UI stager holds fParameterSnapshotMutex - so a UI stall would block the
    // audio thread inside the FX stage. The handoff is one atomic int per slot
    // plus this release/acquire flag, which is all the ordering it needs: the
    // stager writes the type, then release-stores the flag; we acquire-load the
    // flag, which makes every staged type visible before we read it.
    if (!fPendingSlotTypeChange.exchange(false, std::memory_order_acq_rel)) return;
    for (int i = 0; i < 6; ++i) {
        const int staged = fPendingSlotTypes[i].load(std::memory_order_relaxed);
        if (staged < 0) continue;
        // Keep the staged type visible to getParameterValue() until the new
        // object exists. Clearing it first opened a window where the editor
        // could read the OLD live type while the new FX was being constructed.
        FxPrepared* prepared = fFxPrepared[i].exchange(nullptr, std::memory_order_acq_rel);
        if (prepared == nullptr || prepared->type != staged) {
            if (prepared != nullptr)
                fFxPrepared[i].store(prepared, std::memory_order_release);
            fFxPrepareRequest[i].store(staged, std::memory_order_release);
            fPendingSlotTypeChange.store(true, std::memory_order_release);
            continue; // keep the old object this block; retry when prepared
        }
        // Move the prepared object out first, then recycle its container for
        // the retired active objects. Nothing here allocates on the audio thread.
        auto newSurge = std::move(prepared->surge);
        auto newInternal = std::move(prepared->internal);
        auto newVendored = std::move(prepared->vendored);
        prepared->surge = std::move(m_surgeFx[i]);
        prepared->internal = std::move(m_internalFx[i]);
        prepared->vendored = std::move(m_vendoredFx[i]);
        m_surgeFx[i] = std::move(newSurge);
        m_internalFx[i] = std::move(newInternal);
        m_vendoredFx[i] = std::move(newVendored);
        if (staged >= CymbalsSurgeFx::kBaseType) {
            m_surgeFxAlgo[i] = std::clamp(staged - CymbalsSurgeFx::kBaseType, 0,
                                          CymbalsSurgeFx::algorithmCount() - 1);
            m_fxSlots[i].setType(AirSuiteUniversal::NONE);
        } else if (CymbalsVendoredFx::isVendoredId(staged) ||
                   CymbalsInternalFx::isInternalId(staged)) {
            m_surgeFxAlgo[i] = -1;
            m_fxSlots[i].setType(AirSuiteUniversal::NONE);
        } else {
            m_surgeFxAlgo[i] = -1;
            const AirSuiteUniversal::Type type = (AirSuiteUniversal::Type)staged;
            m_fxSlots[i].setType(type);
            if (type != AirSuiteUniversal::NONE) {
                m_fxSlots[i].setSampleRate(fSampleRate);
                m_fxSlots[i].reset();
            }
        }
        fFxRetired[i].store(prepared, std::memory_order_release);
        fAppliedSlotTypes[i].store(staged, std::memory_order_release);
        int expected = staged;
        fPendingSlotTypes[i].compare_exchange_strong(expected, -1, std::memory_order_relaxed);
    }
}

void PluginCymbals::updateStickEQ() {
    for (int i = 0; i < 5; ++i) {
        const float freqHz = DSPUtils::normToHz( std::clamp(eqFreq[i], 0.0f, 1.0f));
        const int typeIdx = std::clamp((int)std::lround(eqFilterType[i]), 0, 7);
        const float gainDb = (typeIdx >= 4 && typeIdx <= 6)
            ? (std::clamp(eqGain[i], 0.0f, 1.0f) - 0.5f) * 48.0f : 0.0f;
        const float q = DSPUtils::qFromParam(std::clamp(eqQ[i], 0.0f, 1.0f));
        stickEQNodes[i].setSampleRate(fSampleRate);
        stickEQNodes[i].setModel(std::clamp((int)std::lround(eqModel[i]), 0, (int)UnifiedFilter::Count - 1));
        stickEQNodes[i].setType(typeIdx);
        stickEQNodes[i].setSlope(std::clamp((int)std::lround(eqSlope[i]), 0, 3));
        stickEQNodes[i].setCutoffHz(freqHz);
        stickEQNodes[i].setResonance(q);
        stickEQNodes[i].setGainDb(gainDb);
        stickEQNodes[i].setSaturation(std::clamp(eqSat[i], 0.0f, 1.0f));
        stickEQNodes[i].prepare();
    }

    const float pivotHz = DSPUtils::normToHz( std::clamp(stickTiltPivot, 0.0f, 1.0f));
    const float tiltDb = DSPUtils::tiltDbFromParam(std::clamp(stickTiltAmount, 0.0f, 1.0f));
    stickTiltLow.setParams(Biquad::LOW_SHELF, pivotHz, -tiltDb, 0.707f, fSampleRate);
    stickTiltHigh.setParams(Biquad::HIGH_SHELF, pivotHz, tiltDb, 0.707f, fSampleRate);
}

void PluginCymbals::updateStickPitchMseg() {
    std::vector<MsegPoint> points;
    points.push_back({0.0f, msegPitchValues[0], (msegPitchCurves[0] - 0.5f) * 2.0f});
    points.push_back({msegPitchTimes[0], msegPitchValues[1], (msegPitchCurves[1] - 0.5f) * 2.0f});
    points.push_back({msegPitchTimes[1], msegPitchValues[2], (msegPitchCurves[2] - 0.5f) * 2.0f});
    points.push_back({msegPitchTimes[2], msegPitchValues[3], (msegPitchCurves[3] - 0.5f) * 2.0f});
    points.push_back({msegPitchTimes[3], msegPitchValues[4], (msegPitchCurves[4] - 0.5f) * 2.0f});
    points.push_back({msegPitchTimes[4], msegPitchValues[5], (msegPitchCurves[5] - 0.5f) * 2.0f});
    points.push_back({msegPitchTimes[5], msegPitchValues[6], (msegPitchCurves[6] - 0.5f) * 2.0f});
    points.push_back({1.0f, msegPitchValues[7], 0.0f});

    stickBank.setPitchMsegPoints(points);
    stickBank.setPitchMsegDuration(0.001f + std::pow(msegPitchDuration, 2.0f) * 1.999f);
    stickBank.setUseMsegPitch(msegPitchOn > 0.5f);
}

void PluginCymbals::updateFinalEQ() {
    for (int i = 0; i < 5; ++i) {
        const float freqHz = DSPUtils::normToHz( std::clamp(feqFreq[i], 0.0f, 1.0f));
        const int typeIdx = std::clamp((int)std::lround(feqFilterType[i]), 0, 7);
        const float gainDb = (typeIdx >= 4 && typeIdx <= 6)
            ? (std::clamp(feqGain[i], 0.0f, 1.0f) - 0.5f) * 48.0f : 0.0f;
        const float q = DSPUtils::qFromParam(std::clamp(feqQ[i], 0.0f, 1.0f));
        finalEQNodes[i].setSampleRate(fSampleRate);
        finalEQNodes[i].setModel(std::clamp((int)std::lround(feqModel[i]), 0, (int)UnifiedFilter::Count - 1));
        finalEQNodes[i].setType(typeIdx);
        finalEQNodes[i].setSlope(std::clamp((int)std::lround(feqSlope[i]), 0, 3));
        finalEQNodes[i].setCutoffHz(freqHz);
        finalEQNodes[i].setResonance(q);
        finalEQNodes[i].setGainDb(gainDb);
        finalEQNodes[i].setSaturation(std::clamp(feqSat[i], 0.0f, 1.0f));
        finalEQNodes[i].prepare();
    }

    const float pivotHz = DSPUtils::normToHz( std::clamp(finalTiltPivot, 0.0f, 1.0f));
    const float tiltDb = DSPUtils::tiltDbFromParam(std::clamp(finalTiltAmount, 0.0f, 1.0f));
    finalTiltLow.setParams(Biquad::LOW_SHELF, pivotHz, -tiltDb, 0.707f, fSampleRate);
    finalTiltHigh.setParams(Biquad::HIGH_SHELF, pivotHz, tiltDb, 0.707f, fSampleRate);
}

void PluginCymbals::processStickCutOnly(float inL, float inR, float& outL, float& outR) {
    outL = inL;
    outR = inR;
    for (int i = 0; i < 5; ++i) {
        stickEQNodes[i].processStereoSample(outL, outR);
    }
}

void PluginCymbals::processFinalCutOnly(float inL, float inR, float& outL, float& outR) {
    outL = inL;
    outR = inR;
    for (int i = 0; i < 5; ++i) {
        finalEQNodes[i].processStereoSample(outL, outR);
    }
}

void PluginCymbals::resetCutFilterStates() {
    for (int i = 0; i < 5; ++i) {
        stickEQNodes[i].reset();
        finalEQNodes[i].reset();
    }
}

// ============================================================================
// Modulation System — target apply table (Phase C3)
//
// kModTargetClasses is the single source of truth for BOTH the audio-thread
// route application and parameterIsContinuous (the UI only offers params
// inside this table as route targets). Each fn copies the exact statements of
// the matching setParameterValue branch; any *secondary* param a branch reads
// (e.g. dispFB reads dispFBOverall) comes from m_paramBase so modulation does
// not feed back on itself. Discrete/legacy/no-consumer params are deliberately
// absent — the UI can never route to them.
// ============================================================================

void PluginCymbals::applyToneFamily(PluginCymbals& p, uint32_t idx, float eff) {
    // kParamTone / kParamDecay are stored but not consumed by any DSP path;
    // they are intentionally NOT routable (the class range covers Volume only).
    if (idx == kParamVolume) p.volume = eff;
}

void PluginCymbals::applyDispFamily(PluginCymbals& p, uint32_t idx, float eff) {
    switch (idx) {
        case kParamDispFB:
            p.dispFB = eff;
            p.dispersion.setFeedback(DispersionStage::feedbackFromControl(eff)
                * DispersionStage::overallFeedbackScaleFromControl(p.m_paramBase[kParamDispFBOverall]));
            break;
        case kParamDispDelay:
            p.dispDelay = eff;
            p.dispersion.setDelaySamples(eff * 2000.0f);
            break;
        case kParamDispGain:
            p.dispGain = eff;
            p.dispersion.setInputGain(eff);
            break;
        case kParamDispFBOverall:
            p.dispFBOverall = eff;
            p.dispersion.setFeedback(DispersionStage::feedbackFromControl(p.m_paramBase[kParamDispFB])
                * DispersionStage::overallFeedbackScaleFromControl(eff));
            break;
    }
}

void PluginCymbals::applyDisp2Family(PluginCymbals& p, uint32_t idx, float eff) {
    switch (idx) {
        case kParamDisp2FB:
            p.disp2FB = eff;
            p.dispersion2.setFeedback(DispersionStage::feedbackFromControl(eff)
                * DispersionStage::overallFeedbackScaleFromControl(p.m_paramBase[kParamDisp2FBOverall]));
            break;
        case kParamDisp2Delay:
            p.disp2Delay = eff;
            p.dispersion2.setDelaySamples(eff * 2000.0f);
            break;
        case kParamDisp2Gain:
            p.disp2Gain = eff;
            p.dispersion2.setInputGain(eff);
            break;
        case kParamDisp2FBOverall:
            p.disp2FBOverall = eff;
            p.dispersion2.setFeedback(DispersionStage::feedbackFromControl(p.m_paramBase[kParamDisp2FB])
                * DispersionStage::overallFeedbackScaleFromControl(eff));
            break;
    }
}

// Mirrors one stick parameter onto a bank. Shared by the mono bank and the
// poly voice banks so a mod route cannot update only one of them.
static void applyStickParamToBank(StickSoundBank& bank, uint32_t idx, float eff) {
    switch (idx) {
        case PluginCymbals::kParamStickAmpAttack: bank.setAmpAttack(eff); break;
        case PluginCymbals::kParamStickAmpDecay: bank.setAmpDecay(eff); break;
        case PluginCymbals::kParamStickAmpSustain: bank.setAmpSustain(eff); break;
        case PluginCymbals::kParamStickAmpRelease: bank.setAmpRelease(eff); break;
        case PluginCymbals::kParamStickAmpDecayCurve: bank.setAmpDecayCurve(eff); break;
        case PluginCymbals::kParamStickAmpReleaseCurve: bank.setAmpReleaseCurve(eff); break;
        case PluginCymbals::kParamStickPitchAttack: bank.setPitchAttack(eff); break;
        case PluginCymbals::kParamStickPitchDecay: bank.setPitchDecay(eff); break;
        case PluginCymbals::kParamStickPitchSustain: bank.setPitchSustain(eff); break;
        case PluginCymbals::kParamStickPitchRelease: bank.setPitchRelease(eff); break;
        case PluginCymbals::kParamStickPitchDecayCurve: bank.setPitchDecayCurve(eff); break;
        case PluginCymbals::kParamStickPitchReleaseCurve: bank.setPitchReleaseCurve(eff); break;
        case PluginCymbals::kParamStickPitchAmount: bank.setPitchAmount(eff * 48.0f); break;
        case PluginCymbals::kParamStickPitchTimer: bank.setPitchTimer(eff * 0.1f); break;
        case PluginCymbals::kParamStickPulseWidth: bank.setPulseWidth(eff); break;
        case PluginCymbals::kParamStickNoiseColor: bank.setNoiseColor(eff); break;
        case PluginCymbals::kParamStickMetallicSpread: bank.setMetallicSpread(eff); break;
        case PluginCymbals::kParamStickNoiseMetalBlend: bank.setNoiseMetalBlend(eff); break;
        case PluginCymbals::kParamStickHatTone: bank.setHatTone(eff); break;
        case PluginCymbals::kParamStickSquareWidth: bank.setSquareWidth(eff); break;
        case PluginCymbals::kParamStickSawCurve: bank.setSawCurve(eff); break;
        case PluginCymbals::kParamStickTriangleSkew: bank.setTriangleSkew(eff); break;
        case PluginCymbals::kParamStickSineDrive: bank.setSineDrive(eff); break;
        case PluginCymbals::kParamStickPhase: bank.setPhaseOffset(eff); break;
        case PluginCymbals::kParamStickGrit: bank.setGrit(eff); break;
        case PluginCymbals::kParamStickDetune: bank.setDetune(eff); break;
        default: break;
    }
}

void PluginCymbals::applyStickFamily(PluginCymbals& p, uint32_t idx, float eff) {
    switch (idx) {
        case kParamStickAmpAttack: p.stickAmpAttack = eff; break;
        case kParamStickAmpDecay: p.stickAmpDecay = eff; break;
        case kParamStickAmpSustain: p.stickAmpSustain = eff; break;
        case kParamStickAmpRelease: p.stickAmpRelease = eff; break;
        case kParamStickAmpDecayCurve: p.stickAmpDecayCurve = eff; break;
        case kParamStickAmpReleaseCurve: p.stickAmpReleaseCurve = eff; break;
        case kParamStickPitchAttack: p.stickPitchAttack = eff; break;
        case kParamStickPitchDecay: p.stickPitchDecay = eff; break;
        case kParamStickPitchSustain: p.stickPitchSustain = eff; break;
        case kParamStickPitchRelease: p.stickPitchRelease = eff; break;
        case kParamStickPitchDecayCurve: p.stickPitchDecayCurve = eff; break;
        case kParamStickPitchReleaseCurve: p.stickPitchReleaseCurve = eff; break;
        case kParamStickPitchAmount: p.stickPitchAmount = eff; break;
        case kParamStickPitchTimer: p.stickPitchTimer = eff; break;
        case kParamStickPulseWidth: p.stickPulseWidth = eff; break;
        case kParamStickNoiseColor: p.stickNoiseColor = eff; break;
        case kParamStickMetallicSpread: p.stickMetallicSpread = eff; break;
        case kParamStickNoiseMetalBlend: p.stickNoiseMetalBlend = eff; break;
        case kParamStickHatTone: p.stickHatTone = eff; break;
        case kParamStickSquareWidth: p.stickSquareWidth = eff; break;
        case kParamStickSawCurve: p.stickSawCurve = eff; break;
        case kParamStickTriangleSkew: p.stickTriangleSkew = eff; break;
        case kParamStickSineDrive: p.stickSineDrive = eff; break;
        case kParamStickPhase: p.stickPhase = eff; break;
        case kParamStickGrit: p.stickGrit = eff; break;
        case kParamStickDetune: p.stickDetune = eff; break;
        case kParamStickDirect: p.stickDirect = eff; return;
        case kParamStickBus: p.stickBus = eff; return;
        default: return;
    }
    applyStickParamToBank(p.stickBank, idx, eff);
    // Poly mode reads the per-voice banks, not the shared mono bank.
    for (int i = 0; i < kMaxVoices; ++i) {
        if (p.m_voices[i].active)
            applyStickParamToBank(p.m_voices[i].stick, idx, eff);
    }
}

void PluginCymbals::applyNudge(PluginCymbals& p, uint32_t idx, float eff) {
    const int i = (int)(idx - kParamNudge1) / 2;
    if ((idx - kParamNudge1) % 2 == 0) {
        p.nudge[i] = eff;
        p.dispersion.setNudge(i, eff);
    } else {
        p.nudgeAmount[i] = eff;
        p.dispersion.setAmount(i, eff);
    }
}

void PluginCymbals::applyNudge2(PluginCymbals& p, uint32_t idx, float eff) {
    const int i = (int)(idx - kParamNudge2_1) / 2;
    if ((idx - kParamNudge2_1) % 2 == 0) {
        p.nudge2[i] = eff;
        p.dispersion2.setNudge(i, eff);
    } else {
        p.nudgeAmount2[i] = eff;
        p.dispersion2.setAmount(i, eff);
    }
}

void PluginCymbals::applyVibFamily(PluginCymbals& p, uint32_t idx, float eff) {
    switch (idx) {
        case kParamVibRate: p.vibRate = eff; p.dispersion.setVibratoRate(10.0f + eff * 1990.0f); break;
        case kParamVibDepth: p.vibDepth = eff; p.dispersion.setVibratoDepth(eff * 40.0f); break;
        case kParamVibSmooth: p.vibSmooth = eff; p.dispersion.setVibratoSmooth(eff); break;
        case kParamVibStepMix: p.vibStepMix = eff; p.dispersion.setVibratoStepMix(eff); break;
    }
}

void PluginCymbals::applyVib2Family(PluginCymbals& p, uint32_t idx, float eff) {
    switch (idx) {
        case kParamVib2Rate: p.vib2Rate = eff; p.dispersion2.setVibratoRate(10.0f + eff * 1990.0f); break;
        case kParamVib2Depth: p.vib2Depth = eff; p.dispersion2.setVibratoDepth(eff * 40.0f); break;
        case kParamVib2Smooth: p.vib2Smooth = eff; p.dispersion2.setVibratoSmooth(eff); break;
        case kParamVib2StepMix: p.vib2StepMix = eff; p.dispersion2.setVibratoStepMix(eff); break;
    }
}

void PluginCymbals::applyChaosFamily(PluginCymbals& p, uint32_t idx, float eff) {
    switch (idx) {
        case kParamChaos: p.chaosFactor = eff; p.dispersion.setChaos(eff); break;
        case kParamChaosEnvAmount: p.chaosEnvAmount = eff; p.dispersion.setChaosEnvAmount(eff); break;
        case kParamChaosEnvDecay: p.chaosEnvDecay = eff; p.dispersion.setChaosEnvDecay(eff); break;
    }
}

void PluginCymbals::applyChaos2Family(PluginCymbals& p, uint32_t idx, float eff) {
    switch (idx) {
        case kParamChaos2: p.chaos2Factor = eff; p.dispersion2.setChaos(eff); break;
        case kParamChaos2EnvAmount: p.chaos2EnvAmount = eff; p.dispersion2.setChaosEnvAmount(eff); break;
        case kParamChaos2EnvDecay: p.chaos2EnvDecay = eff; p.dispersion2.setChaosEnvDecay(eff); break;
    }
}

void PluginCymbals::applyPdFamily(PluginCymbals& p, uint32_t idx, float eff) {
    switch (idx) {
        case kParamPDDrive: p.pdDrive = eff; p.dispersion.setPDDrive(eff); break;
        case kParamPDNormalize: p.pdNormalize = eff; p.dispersion.setPDNormalize(eff); break;
        case kParamPDTone: p.pdTone = eff; p.dispersion.setPDTone(eff); break;
        case kParamPDBias: p.pdBias = eff; p.dispersion.setPDBias(eff); break;
        case kParamPDSpread: p.pdSpread = eff; p.dispersion.setPDSpread(eff); break;
        case kParamPDMix: p.pdMix = eff; p.dispersion.setPDMix(eff); break;
    }
}

void PluginCymbals::applyPd2Family(PluginCymbals& p, uint32_t idx, float eff) {
    switch (idx) {
        case kParamPD2Drive: p.pd2Drive = eff; p.dispersion2.setPDDrive(eff); break;
        case kParamPD2Normalize: p.pd2Normalize = eff; p.dispersion2.setPDNormalize(eff); break;
        case kParamPD2Tone: p.pd2Tone = eff; p.dispersion2.setPDTone(eff); break;
        case kParamPD2Bias: p.pd2Bias = eff; p.dispersion2.setPDBias(eff); break;
        case kParamPD2Spread: p.pd2Spread = eff; p.dispersion2.setPDSpread(eff); break;
        case kParamPD2Mix: p.pd2Mix = eff; p.dispersion2.setPDMix(eff); break;
    }
}

void PluginCymbals::applyEqFamily(PluginCymbals& p, uint32_t idx, float eff) {
    if (idx >= kParamEQ1Freq && idx <= kParamEQ5Type) {
        const int band = (int)(idx - kParamEQ1Freq) / 4;
        const int sub = (int)(idx - kParamEQ1Freq) % 4;
        if (sub == 0) p.eqFreq[band] = eff;
        else if (sub == 1) p.eqGain[band] = eff;
        else if (sub == 2) p.eqQ[band] = eff;
        else p.eqFilterType[band] = std::clamp(std::round(eff), 0.0f, 7.0f);
        const int typeIdx = std::clamp((int)std::lround(p.eqFilterType[band]), 0, 7);
        p.stickEQNodes[band].setType(typeIdx);
        const float freqHz = DSPUtils::normToHz( std::clamp(p.eqFreq[band], 0.0f, 1.0f));
        const float gainDb = (typeIdx >= 4 && typeIdx <= 6)
            ? (std::clamp(p.eqGain[band], 0.0f, 1.0f) - 0.5f) * 48.0f : 0.0f;
        const float q = DSPUtils::qFromParam(std::clamp(p.eqQ[band], 0.0f, 1.0f));
        p.stickEQNodes[band].setModulatedValues(freqHz, q, gainDb);
    } else if (idx >= kParamEQ1Sat && idx <= kParamEQ5Sat) {
        p.eqSat[idx - kParamEQ1Sat] = eff;
        p.stickEQNodes[idx - kParamEQ1Sat].setSaturation(eff);
    } else if (idx >= kParamFinalEQ1Freq && idx <= kParamFinalEQ5Type) {
        const int band = (int)(idx - kParamFinalEQ1Freq) / 4;
        const int sub = (int)(idx - kParamFinalEQ1Freq) % 4;
        if (sub == 0) p.feqFreq[band] = eff;
        else if (sub == 1) p.feqGain[band] = eff;
        else if (sub == 2) p.feqQ[band] = eff;
        else p.feqFilterType[band] = std::clamp(std::round(eff), 0.0f, 7.0f);
        const int typeIdx = std::clamp((int)std::lround(p.feqFilterType[band]), 0, 7);
        p.finalEQNodes[band].setType(typeIdx);
        const float freqHz = DSPUtils::normToHz( std::clamp(p.feqFreq[band], 0.0f, 1.0f));
        const float gainDb = (typeIdx >= 4 && typeIdx <= 6)
            ? (std::clamp(p.feqGain[band], 0.0f, 1.0f) - 0.5f) * 48.0f : 0.0f;
        const float q = DSPUtils::qFromParam(std::clamp(p.feqQ[band], 0.0f, 1.0f));
        p.finalEQNodes[band].setModulatedValues(freqHz, q, gainDb);
    } else if (idx >= kParamFinalEQ1Sat && idx <= kParamFinalEQ5Sat) {
        p.feqSat[idx - kParamFinalEQ1Sat] = eff;
        p.finalEQNodes[idx - kParamFinalEQ1Sat].setSaturation(eff);
    }
}

void PluginCymbals::applyTiltFamily(PluginCymbals& p, uint32_t idx, float eff) {
    if (idx >= kParamTiltAmount && idx <= kParamTiltPivot) {
        p.stickTiltAmount = (idx == kParamTiltAmount) ? eff : p.m_paramBase[kParamTiltAmount];
        p.stickTiltPivot = (idx == kParamTiltPivot) ? eff : p.m_paramBase[kParamTiltPivot];
        const float pivotHz = DSPUtils::normToHz( std::clamp(p.stickTiltPivot, 0.0f, 1.0f));
        const float tiltDb = DSPUtils::tiltDbFromParam(std::clamp(p.stickTiltAmount, 0.0f, 1.0f));
        p.stickTiltLow.setParams(Biquad::LOW_SHELF, pivotHz, -tiltDb, 0.707f, p.fSampleRate);
        p.stickTiltHigh.setParams(Biquad::HIGH_SHELF, pivotHz, tiltDb, 0.707f, p.fSampleRate);
    } else {
        p.finalTiltAmount = (idx == kParamFinalTiltAmount) ? eff : p.m_paramBase[kParamFinalTiltAmount];
        p.finalTiltPivot = (idx == kParamFinalTiltPivot) ? eff : p.m_paramBase[kParamFinalTiltPivot];
        const float pivotHz = DSPUtils::normToHz( std::clamp(p.finalTiltPivot, 0.0f, 1.0f));
        const float tiltDb = DSPUtils::tiltDbFromParam(std::clamp(p.finalTiltAmount, 0.0f, 1.0f));
        p.finalTiltLow.setParams(Biquad::LOW_SHELF, pivotHz, -tiltDb, 0.707f, p.fSampleRate);
        p.finalTiltHigh.setParams(Biquad::HIGH_SHELF, pivotHz, tiltDb, 0.707f, p.fSampleRate);
    }
}

void PluginCymbals::applyCombFamily(PluginCymbals& p, uint32_t idx, float eff) {
    if (idx >= kParamComb1Freq && idx <= kParamComb40HPF) {
        const int rel = (int)(idx - kParamComb1Freq);
        const int s = rel / 4;
        const int t = rel % 4;
        // The group header's LPF/HPF knobs are stage 0's parameters and the
        // UI broadcasts manual edits to the group's four filters. A modulation
        // route to that same target must broadcast too, or only stage 0
        // changes while the header claims to control the group.
        if ((t == 2 || t == 3) && (s % 4) == 0) {
            const int groupBase = (s / 4) * 4;
            const float delta = eff - p.m_paramBase[idx];
            for (int k = 0; k < 4; ++k) {
                const uint32_t pk = (uint32_t)(kParamComb1Freq + (groupBase + k) * 4 + t);
                const float stageEff = std::clamp(p.m_paramBase[pk] + delta, 0.0f, 1.0f);
                p.combParams[pk - kParamComb1Freq] = stageEff;
                if (t == 2) p.combBank.setLPF(groupBase + k, DSPUtils::normToHz(stageEff));
                else        p.combBank.setHPF(groupBase + k, DSPUtils::normToHz(stageEff));
            }
        } else {
            p.combParams[rel] = eff;
            if (t == 0) p.combBank.setFrequency(s, DSPUtils::normToHz( eff));
            else if (t == 1) p.combBank.setFeedback(s, std::min(0.998f, eff));
            else if (t == 2) p.combBank.setLPF(s, DSPUtils::normToHz( eff));
            else p.combBank.setHPF(s, DSPUtils::normToHz( eff));
        }
    } else if (idx >= kParamComb1Scale && idx <= kParamComb40Scale) {
        p.combScalePerStage[idx - kParamComb1Scale] = eff;
        p.combBank.setStageScaleFactor((int)(idx - kParamComb1Scale), DSPUtils::scaleFactorFromParam( eff));
    } else if (idx >= kParamComb1SatAmount && idx <= kParamComb40SatAmount) {
        p.combSatAmount[idx - kParamComb1SatAmount] = eff;
        p.combBank.setSaturatorAmount((int)(idx - kParamComb1SatAmount), eff);
    } else if (idx >= kParamComb1StressOnset && idx <= kParamComb40StressKnee) {
        const int off = (int)(idx - kParamComb1StressOnset);
        if (off < 40) { p.combStressOnset[off] = eff; p.combBank.setSatStressOnset(off, eff); }
        else { p.combStressKnee[off - 40] = eff; p.combBank.setSatStressKnee(off - 40, eff); }
    } else if (idx >= kParamComb1Mix && idx <= kParamComb40Mix) {
        p.combMix[idx - kParamComb1Mix] = eff;
        p.combBank.setStageMix((int)(idx - kParamComb1Mix), eff);
    } else if (idx >= kParamComb1ModeRate && idx <= kParamComb40ModeAmount) {
        const int off = (int)(idx - kParamComb1ModeRate);
        if (off < 40) { p.combModeRate[off] = eff; p.combBank.setModeRate(off, eff); }
        else { p.combModeAmount[off - 40] = eff; p.combBank.setModeAmount(off - 40, eff); }
    } else if (idx >= kParamComb1ModeA && idx <= kParamComb40ModeB) {
        const int off = (int)(idx - kParamComb1ModeA);
        if (off < 40) p.combBank.setModeA(off, eff);
        else          p.combBank.setModeB(off - 40, eff);
    } else if (idx >= kParamComb1Q && idx <= kParamComb40Q) {
        const int c = (int)(idx - kParamComb1Q);
        const float q = (eff <= 0.02f) ? -1.0f
                                       : 0.5f * std::pow(40.0f, std::clamp(eff, 0.0f, 1.0f));
        p.combBank.setFilterQ(c, q);
    } else if (idx >= kParamComb1PartRelPos && idx <= kParamComb40PartStiffExp) {
        const int off = (int)(idx - kParamComb1PartRelPos);
        if (off < 40) { p.combPartRelPos[off] = eff; p.combBank.setPartRelPos(off, eff); }
        else if (off < 80) { p.combPartFreqDamp[off - 40] = eff; p.combBank.setPartFreqDamp(off - 40, eff); }
        else if (off < 120) { p.combPartStiffness[off - 80] = eff; p.combBank.setPartStiffness(off - 80, eff); }
        else { p.combPartStiffExp[off - 120] = eff; p.combBank.setPartStiffExp(off - 120, eff); }
    } else if (idx >= kParamCombGroup1Shift && idx <= kParamCombGroup10Shift) {
        p.combGroupShift[idx - kParamCombGroup1Shift] = eff;
        p.combBank.setShift((int)(idx - kParamCombGroup1Shift), (eff - 0.5f) * 20000.0f);
    } else if (idx >= kParamCombGroup1Gain && idx <= kParamCombGroup10Gain) {
        const int g = (int)(idx - kParamCombGroup1Gain);
        p.combGroupGain[g] = eff;
        const float db = DSPUtils::normGainToDb(eff);
        p.combBank.setGroupGain(g, p.m_paramBase[kParamCombGroup1On + g] > 0.5f ? std::pow(10.0f, db / 20.0f) : 0.0f);
    } else if (idx >= kParamCombGroup1LimiterOn && idx <= kParamCombGroup10LimiterOn) {
        const int g = (int)(idx - kParamCombGroup1LimiterOn);
        p.combGroupLimiterOn[g] = eff;
        p.combBank.setGroupLimiterOn(g, eff);
    } else if (idx >= kParamCombGroup1LimiterThreshold && idx <= kParamCombGroup10LimiterThreshold) {
        const int g = (int)(idx - kParamCombGroup1LimiterThreshold);
        p.combGroupLimiterThreshold[g] = eff;
        p.combBank.setGroupLimiterThreshold(g, eff);
    } else if (idx >= kParamCombGroup1LimiterRelease && idx <= kParamCombGroup10LimiterRelease) {
        const int g = (int)(idx - kParamCombGroup1LimiterRelease);
        p.combGroupLimiterRelease[g] = eff;
        p.combBank.setGroupLimiterRelease(g, eff);
    } else if (idx >= kParamCombGroup1Cluster && idx <= kParamCombGroup10Cluster) {
        p.combGroupCluster[idx - kParamCombGroup1Cluster] = eff;
        p.combBank.setCluster((int)(idx - kParamCombGroup1Cluster), eff);
    } else { // kParamCombScale
        p.combScale = eff;
        p.combBank.setScaleFactor(DSPUtils::scaleFactorFromParam( eff));
    }
}

void PluginCymbals::applyFinalCombFamily(PluginCymbals& p, uint32_t idx, float eff) {
    if (idx >= kParamFinalComb1Freq && idx <= kParamFinalComb4HPF) {
        p.finalCombParams[idx - kParamFinalComb1Freq] = eff;
        const int s = (int)(idx - kParamFinalComb1Freq) / 4;
        const int t = (int)(idx - kParamFinalComb1Freq) % 4;
        if (t == 0) p.finalCombBank.setFrequency(s, DSPUtils::normToHz( eff));
        else if (t == 1) p.finalCombBank.setFeedback(s, std::min(0.998f, eff));
        else if (t == 2) p.finalCombBank.setLPF(s, DSPUtils::normToHz( eff));
        else p.finalCombBank.setHPF(s, DSPUtils::normToHz( eff));
    } else if (idx >= kParamFinalComb1Mix && idx <= kParamFinalComb4Mix) {
        p.finalCombMix[idx - kParamFinalComb1Mix] = eff;
        p.finalCombBank.setStageMix((int)(idx - kParamFinalComb1Mix), eff);
    } else if (idx >= kParamFinalComb1ModeRate && idx <= kParamFinalComb4ModeAmount) {
        const int off = (int)(idx - kParamFinalComb1ModeRate);
        if (off < 4) { p.finalCombModeRate[off] = eff; p.finalCombBank.setModeRate(off, eff); }
        else { p.finalCombModeAmount[off - 4] = eff; p.finalCombBank.setModeAmount(off - 4, eff); }
    } else if (idx == kParamFinalCombLimiterOn) {
        p.finalCombBank.setGroupLimiterOn(0, eff);
    } else if (idx == kParamFinalCombLimiterThreshold) {
        p.finalCombBank.setGroupLimiterThreshold(0, eff);
    } else if (idx == kParamFinalCombLimiterRelease) {
        p.finalCombBank.setGroupLimiterRelease(0, eff);
    } else if (idx >= kParamFinalComb1Q && idx <= kParamFinalComb4Q) {
        const int c = (int)(idx - kParamFinalComb1Q);
        const float q = (eff <= 0.02f) ? -1.0f
                                       : 0.5f * std::pow(40.0f, std::clamp(eff, 0.0f, 1.0f));
        p.finalCombBank.setFilterQ(c, q);
    } else if (idx >= kParamFinalComb1ModeA && idx <= kParamFinalComb4ModeB) {
        const int off = (int)(idx - kParamFinalComb1ModeA);
        if (off < 4) p.finalCombBank.setModeA(off, eff);
        else         p.finalCombBank.setModeB(off - 4, eff);
    } else if (idx >= kParamFinalComb1PartRelPos && idx <= kParamFinalComb4PartStiffExp) {
        const int off = (int)(idx - kParamFinalComb1PartRelPos);
        if (off < 4) { p.finalCombPartRelPos[off] = eff; p.finalCombBank.setPartRelPos(off, eff); }
        else if (off < 8) { p.finalCombPartFreqDamp[off - 4] = eff; p.finalCombBank.setPartFreqDamp(off - 4, eff); }
        else if (off < 12) { p.finalCombPartStiffness[off - 8] = eff; p.finalCombBank.setPartStiffness(off - 8, eff); }
        else { p.finalCombPartStiffExp[off - 12] = eff; p.finalCombBank.setPartStiffExp(off - 12, eff); }
    } else if (idx == kParamFinalComb4SatAmount) {
        p.finalComb4SatAmount = eff;
        p.finalCombBank.setSaturatorAmount(3, eff);
    } else if (idx >= kParamFinalComb4StressOnset && idx <= kParamFinalComb4StressKnee) {
        if (idx == kParamFinalComb4StressOnset) {
            p.finalComb4StressOnset = eff;
            p.finalCombBank.setSatStressOnset(3, eff);
        } else {
            p.finalComb4StressKnee = eff;
            p.finalCombBank.setSatStressKnee(3, eff);
        }
    } else if (idx == kParamFinalCombShift) {
        p.finalCombShift = eff;
        p.finalCombBank.setShift(0, (eff - 0.5f) * 20000.0f);
    } else if (idx == kParamFinalCombScale) {
        p.finalCombScale = eff;
        p.finalCombBank.setScaleFactor(DSPUtils::scaleFactorFromParam( eff));
    } else if (idx == kParamFinalCombGroupGain) {
        p.finalCombGroupGain = eff;
        const float db = DSPUtils::normGainToDb(eff);
        p.finalCombBank.setGroupGain(0, std::pow(10.0f, db / 20.0f));
    } else { // kParamFinalCombGroupCluster
        p.finalCombGroupCluster = eff;
        p.finalCombBank.setCluster(0, eff);
    }
}

// Final output envelope time/scalar mapping. Shared by every write path
// (param apply, host setParameterValue, sample-rate re-init) so the A/R/D/S
// shaping can never drift between them.
static inline float finalEnvAttackSec(float n)  { return 0.001f + std::pow(std::clamp(n, 0.0f, 1.0f), 2.0f) * 1.999f; }
static inline float finalEnvDecaySec(float n)   { return 0.001f + std::pow(std::clamp(n, 0.0f, 1.0f), 2.0f) * 1.999f; }
static inline float finalEnvReleaseSec(float n)  { return 0.002f + std::pow(std::clamp(n, 0.0f, 1.0f), 2.0f) * 2.998f; }
static inline float finalEnvSustainLevel(float n){ return std::clamp(n, 0.0f, 1.0f); }

// Re-shape the whole final envelope from the stored normalized controls. Used
// after a sample-rate change and after a preset/state restore, where individual
// setters would otherwise be replayed one at a time.
void PluginCymbals::syncFinalOutEnv() {
    finalOutEnv.setAttack(finalEnvAttackSec(finalARAttack));
    finalOutEnv.setDecay(finalEnvDecaySec(finalARDecay));
    finalOutEnv.setSustain(finalEnvSustainLevel(finalARSustain));
    finalOutEnv.setRelease(finalEnvReleaseSec(finalARRelease));
    finalOutEnv.setDecayCurve(finalARDecayCurve * 2.0f - 1.0f);
    finalOutEnv.setReleaseCurve(finalARReleaseCurve * 2.0f - 1.0f);
}

void PluginCymbals::updateFinalTouchFilters() {
    float lpH = DSPUtils::normToHz(std::clamp(finalTouchLPF, 0.0f, 1.0f));
    float hpH = DSPUtils::normToHz(std::clamp(finalTouchHPF, 0.0f, 1.0f));
    // The pair is a band: if the knobs cross, sort them so the user always
    // hears the intended band instead of a near-silent gap.
    if (hpH > lpH) std::swap(hpH, lpH);
    // Fixed 4th-order Butterworth: flat passband, -3 dB at cutoff, 24 dB/oct.
    finalTouchLp.configure(4, 0, lpH, 0.70710678f, (float)fSampleRate,
                           DSPUtils::BiquadCascade::Butterworth, 0.0f);
    finalTouchHp.configure(4, 1, hpH, 0.70710678f, (float)fSampleRate,
                           DSPUtils::BiquadCascade::Butterworth, 0.0f);
}

void PluginCymbals::syncMasterOutEnv() {
    masterOutEnv.setAttack(finalEnvAttackSec(masterARAttack));
    masterOutEnv.setDecay(finalEnvDecaySec(masterARDecay));
    masterOutEnv.setSustain(finalEnvSustainLevel(masterARSustain));
    masterOutEnv.setRelease(finalEnvReleaseSec(masterARRelease));
    masterOutEnv.setDecayCurve(masterARDecayCurve * 2.0f - 1.0f);
    masterOutEnv.setReleaseCurve(masterARReleaseCurve * 2.0f - 1.0f);
}

void PluginCymbals::applyGlobalFamily(PluginCymbals& p, uint32_t idx, float eff) {
    switch (idx) {
        case kParamEQMix: p.eqMix = eff; break;
        case kParamFinalEQMix: p.feqMix = eff; break;
        case kParamDispCrossLink: p.dispCrossLink = eff; break;
        case kParamInharmBloomAmt: p.inharmBloomAmt = eff; break;
        case kParamInharmBloomTime: {
            p.inharmBloomTime = eff;
            const float decay = DSPUtils::inharmBloomDecayFromControl(eff);
            p.inharmBloomEnv.setDecay(decay);
            p.applyVoiceBloomTime(false, decay);
            break;
        }
        case kParamInharm2BloomAmt: p.inharm2BloomAmt = eff; break;
        case kParamInharm2BloomTime: {
            p.inharm2BloomTime = eff;
            const float decay = DSPUtils::inharmBloomDecayFromControl(eff);
            p.inharm2BloomEnv.setDecay(decay);
            p.applyVoiceBloomTime(true, decay);
            break;
        }
        case kParamResonatorDiffusion: p.resonatorDiffusion = eff; p.combBank.setResonatorDiffusion(eff); break;
        case kParamCombMemoryDamping:
            p.combMemoryDamping = eff;
            p.combBank.setMemoryDamping(eff);
            p.finalCombBank.setMemoryDamping(eff);
            break;
        case kParamFinalARAttack:
            p.finalARAttack = eff;
            p.finalOutEnv.setAttack(finalEnvAttackSec(eff));
            break;
        case kParamFinalARDecay:
            p.finalARDecay = eff;
            p.finalOutEnv.setDecay(finalEnvDecaySec(eff));
            break;
        case kParamFinalARSustain:
            p.finalARSustain = eff;
            p.finalOutEnv.setSustain(finalEnvSustainLevel(eff));
            break;
        case kParamFinalARRelease:
            p.finalARRelease = eff;
            p.finalOutEnv.setRelease(finalEnvReleaseSec(eff));
            break;
        case kParamFinalARDecayCurve:
            p.finalARDecayCurve = eff;
            p.finalOutEnv.setDecayCurve(eff * 2.0f - 1.0f);
            break;
        case kParamFinalARReleaseCurve:
            p.finalARReleaseCurve = eff;
            p.finalOutEnv.setReleaseCurve(eff * 2.0f - 1.0f);
            break;
        case kParamMasterARAttack:
            p.masterARAttack = eff;
            p.masterOutEnv.setAttack(finalEnvAttackSec(eff));
            break;
        case kParamMasterARDecay:
            p.masterARDecay = eff;
            p.masterOutEnv.setDecay(finalEnvDecaySec(eff));
            break;
        case kParamMasterARSustain:
            p.masterARSustain = eff;
            p.masterOutEnv.setSustain(finalEnvSustainLevel(eff));
            break;
        case kParamMasterARRelease:
            p.masterARRelease = eff;
            p.masterOutEnv.setRelease(finalEnvReleaseSec(eff));
            break;
        case kParamMasterARDecayCurve:
            p.masterARDecayCurve = eff;
            p.masterOutEnv.setDecayCurve(eff * 2.0f - 1.0f);
            break;
        case kParamMasterARReleaseCurve:
            p.masterARReleaseCurve = eff;
            p.masterOutEnv.setReleaseCurve(eff * 2.0f - 1.0f);
            break;
        case kParamFinalClipAmount: p.finalClipAmount = eff; break;
        case kParamFinalClipMix: p.finalClipMix = std::clamp(eff, 0.0f, 1.0f); break;
        case kParamFinalClipOutput:
            p.finalClipOutput = std::clamp(eff, 0.0f, 1.0f);
            p.finalClipGain = std::pow(10.0f, (p.finalClipOutput - 0.5f) * 24.0f / 20.0f);
            break;
        case kParamFinalTouchLPF:
            p.finalTouchLPF = std::clamp(eff, 0.0f, 1.0f);
            p.updateFinalTouchFilters();
            break;
        case kParamFinalTouchHPF:
            p.finalTouchHPF = std::clamp(eff, 0.0f, 1.0f);
            p.updateFinalTouchFilters();
            break;
        case kParamMasterTuning:
            p.applyMasterTuning(eff);
            break;
        case kParamStickPitchMsegDuration:
            p.msegPitchDuration = eff;
            p.stickBank.setPitchMsegDuration(0.001f + std::pow(eff, 2.0f) * 1.999f);
            break;
    }
}

void PluginCymbals::applySlotFamily(PluginCymbals& p, uint32_t idx, float eff) {
    const int slot = (int)(idx - kParamSlot1Type) / 16;
    const int sub = (int)(idx - kParamSlot1Type) % 16;
    if (slot < 0 || slot >= 6) return;
    if (sub == 14) p.m_slotWet[slot] = eff;
    else if (sub >= 1 && sub <= 13) p.m_slotParams[slot][sub - 1] = eff;
}

const PluginCymbals::ModTargetClass PluginCymbals::kModTargetClasses[] = {
    { kParamVolume, kParamVolume, &PluginCymbals::applyToneFamily },
    { kParamDispFB, kParamDispFBOverall, &PluginCymbals::applyDispFamily },
    { kParamStickAmpAttack, kParamStickAmpRelease, &PluginCymbals::applyStickFamily },
    { kParamStickPitchAttack, kParamStickPitchTimer, &PluginCymbals::applyStickFamily },
    { kParamStickPulseWidth, kParamStickHatTone, &PluginCymbals::applyStickFamily },
    { kParamStickSquareWidth, kParamStickGrit, &PluginCymbals::applyStickFamily },
    { kParamStickDetune, kParamStickDetune, &PluginCymbals::applyStickFamily },
    { kParamStickAmpDecayCurve, kParamStickPitchReleaseCurve, &PluginCymbals::applyStickFamily },
    { kParamStickDirect, kParamStickBus, &PluginCymbals::applyStickFamily },
    { kParamMasterTuning, kParamMasterTuning, &PluginCymbals::applyGlobalFamily },
    { kParamNudge1, kParamNudgeAmount20, &PluginCymbals::applyNudge },
    { kParamVibRate, kParamVibDepth, &PluginCymbals::applyVibFamily },
    { kParamVibSmooth, kParamVibStepMix, &PluginCymbals::applyVibFamily },
    { kParamChaos, kParamChaosEnvDecay, &PluginCymbals::applyChaosFamily },
    { kParamPDDrive, kParamPDMix, &PluginCymbals::applyPdFamily },
    // Per-node ranges (Freq/Gain/Q only): the per-node Type int is discrete and
    // must not be routable, so the range stops at each node's Q.
    { kParamEQ1Freq, kParamEQ1Q, &PluginCymbals::applyEqFamily },
    { kParamEQ2Freq, kParamEQ2Q, &PluginCymbals::applyEqFamily },
    { kParamEQ3Freq, kParamEQ3Q, &PluginCymbals::applyEqFamily },
    { kParamEQ4Freq, kParamEQ4Q, &PluginCymbals::applyEqFamily },
    { kParamEQ5Freq, kParamEQ5Q, &PluginCymbals::applyEqFamily },
    { kParamEQ1Sat, kParamEQ5Sat, &PluginCymbals::applyEqFamily },
    { kParamEQMix, kParamEQMix, &PluginCymbals::applyGlobalFamily },
    { kParamTiltAmount, kParamTiltPivot, &PluginCymbals::applyTiltFamily },
    { kParamComb1Freq, kParamComb1Freq + 24 * 4 - 1, &PluginCymbals::applyCombFamily },
    { kParamComb1Scale, kParamComb1Scale + 24 - 1, &PluginCymbals::applyCombFamily },
    { kParamComb1SatAmount, kParamComb1SatAmount + 24 - 1, &PluginCymbals::applyCombFamily },
    { kParamComb1StressOnset, kParamComb1StressKnee + 24 - 1, &PluginCymbals::applyCombFamily },
    { kParamComb1Mix, kParamComb1Mix + 24 - 1, &PluginCymbals::applyCombFamily },
    { kParamComb1ModeRate, kParamComb1ModeAmount + 24 - 1, &PluginCymbals::applyCombFamily },
    { kParamComb1ModeA, kParamComb1ModeB + 24 - 1, &PluginCymbals::applyCombFamily },
    { kParamComb1Q, kParamComb1Q + 24 - 1, &PluginCymbals::applyCombFamily },
    { kParamComb1PartRelPos, kParamComb1PartStiffExp + 24 - 1, &PluginCymbals::applyCombFamily },
    { kParamCombGroup1Shift, kParamCombGroup1Shift + 6 - 1, &PluginCymbals::applyCombFamily },
    { kParamCombGroup1Gain, kParamCombGroup1Gain + 6 - 1, &PluginCymbals::applyCombFamily },
    { kParamCombGroup1LimiterOn, kParamCombGroup1LimiterOn + 6 - 1, &PluginCymbals::applyCombFamily },
    { kParamCombGroup1LimiterThreshold, kParamCombGroup1LimiterThreshold + 6 - 1, &PluginCymbals::applyCombFamily },
    { kParamCombGroup1LimiterRelease, kParamCombGroup1LimiterRelease + 6 - 1, &PluginCymbals::applyCombFamily },
    { kParamCombGroup1Cluster, kParamCombGroup1Cluster + 6 - 1, &PluginCymbals::applyCombFamily },
    { kParamCombScale, kParamCombScale, &PluginCymbals::applyCombFamily },
    { kParamFinalComb1Freq, kParamFinalComb4HPF, &PluginCymbals::applyFinalCombFamily },
    { kParamFinalComb1Mix, kParamFinalComb4Mix, &PluginCymbals::applyFinalCombFamily },
    { kParamFinalComb1Q, kParamFinalComb4Q, &PluginCymbals::applyFinalCombFamily },
    { kParamFinalComb1ModeRate, kParamFinalComb4ModeAmount, &PluginCymbals::applyFinalCombFamily },
    { kParamFinalComb1ModeA, kParamFinalComb4ModeB, &PluginCymbals::applyFinalCombFamily },
    { kParamFinalCombLimiterOn, kParamFinalCombLimiterRelease, &PluginCymbals::applyFinalCombFamily },
    { kParamFinalComb1PartRelPos, kParamFinalComb4PartStiffExp, &PluginCymbals::applyFinalCombFamily },
    { kParamFinalComb4SatAmount, kParamFinalComb4SatAmount, &PluginCymbals::applyFinalCombFamily },
    { kParamFinalComb4StressOnset, kParamFinalComb4StressKnee, &PluginCymbals::applyFinalCombFamily },
    { kParamFinalCombShift, kParamFinalCombShift, &PluginCymbals::applyFinalCombFamily },
    { kParamFinalCombScale, kParamFinalCombScale, &PluginCymbals::applyFinalCombFamily },
    { kParamFinalCombGroupGain, kParamFinalCombGroupGain, &PluginCymbals::applyFinalCombFamily },
    { kParamFinalCombGroupCluster, kParamFinalCombGroupCluster, &PluginCymbals::applyFinalCombFamily },
    { kParamFinalEQ1Freq, kParamFinalEQ1Q, &PluginCymbals::applyEqFamily },
    { kParamFinalEQ2Freq, kParamFinalEQ2Q, &PluginCymbals::applyEqFamily },
    { kParamFinalEQ3Freq, kParamFinalEQ3Q, &PluginCymbals::applyEqFamily },
    { kParamFinalEQ4Freq, kParamFinalEQ4Q, &PluginCymbals::applyEqFamily },
    { kParamFinalEQ5Freq, kParamFinalEQ5Q, &PluginCymbals::applyEqFamily },
    { kParamFinalEQ1Sat, kParamFinalEQ5Sat, &PluginCymbals::applyEqFamily },
    { kParamFinalEQMix, kParamFinalEQMix, &PluginCymbals::applyGlobalFamily },
    { kParamFinalTiltAmount, kParamFinalTiltPivot, &PluginCymbals::applyTiltFamily },
    { kParamDisp2FB, kParamDisp2FBOverall, &PluginCymbals::applyDisp2Family },
    { kParamVib2Rate, kParamVib2Depth, &PluginCymbals::applyVib2Family },
    { kParamVib2Smooth, kParamVib2StepMix, &PluginCymbals::applyVib2Family },
    { kParamChaos2, kParamChaos2EnvDecay, &PluginCymbals::applyChaos2Family },
    { kParamFinalARDecay, kParamFinalARSustain, &PluginCymbals::applyGlobalFamily },
    { kParamPD2Drive, kParamPD2Mix, &PluginCymbals::applyPd2Family },
    { kParamNudge2_1, kParamNudgeAmount2_20, &PluginCymbals::applyNudge2 },
    { kParamDispCrossLink, kParamDispCrossLink, &PluginCymbals::applyGlobalFamily },
    { kParamInharmBloomAmt, kParamInharmBloomTime, &PluginCymbals::applyGlobalFamily },
    { kParamInharm2BloomAmt, kParamInharm2BloomTime, &PluginCymbals::applyGlobalFamily },
    { kParamResonatorDiffusion, kParamResonatorDiffusion, &PluginCymbals::applyGlobalFamily },
    { kParamCombMemoryDamping, kParamCombMemoryDamping, &PluginCymbals::applyGlobalFamily },
    { kParamFinalARAttack, kParamFinalARRelease, &PluginCymbals::applyGlobalFamily },
    { kParamFinalARDecayCurve, kParamMasterARReleaseCurve, &PluginCymbals::applyGlobalFamily },
    { kParamMasterARAttack, kParamMasterARRelease, &PluginCymbals::applyGlobalFamily },
    { kParamFinalClipAmount, kParamFinalClipAmount, &PluginCymbals::applyGlobalFamily },
    { kParamFinalClipMix, kParamFinalClipOutput, &PluginCymbals::applyGlobalFamily },
    { kParamFinalTouchLPF, kParamFinalTouchHPF, &PluginCymbals::applyGlobalFamily },
    { kParamStickPitchMsegDuration, kParamStickPitchMsegDuration, &PluginCymbals::applyGlobalFamily },
    { kParamSlot1Type + 0 * 16 + 1, kParamSlot1Type + 0 * 16 + 14, &PluginCymbals::applySlotFamily },
    { kParamSlot1Type + 1 * 16 + 1, kParamSlot1Type + 1 * 16 + 14, &PluginCymbals::applySlotFamily },
    { kParamSlot1Type + 2 * 16 + 1, kParamSlot1Type + 2 * 16 + 14, &PluginCymbals::applySlotFamily },
    { kParamSlot1Type + 3 * 16 + 1, kParamSlot1Type + 3 * 16 + 14, &PluginCymbals::applySlotFamily },
    { kParamSlot1Type + 4 * 16 + 1, kParamSlot1Type + 4 * 16 + 14, &PluginCymbals::applySlotFamily },
    { kParamSlot1Type + 5 * 16 + 1, kParamSlot1Type + 5 * 16 + 14, &PluginCymbals::applySlotFamily },
};

void PluginCymbals::applyModulatedTarget(uint32_t target, float delta) {
    if (target >= kParameterCount) return;
    const float eff = std::clamp(m_paramBase[target] + delta, 0.0f, 1.0f);
    for (const ModTargetClass& c : kModTargetClasses) {
        if (target >= c.first && target <= c.last) {
            c.fn(*this, target, eff);
            return;
        }
    }
}

std::string PluginCymbals::parameterName(uint32_t index) const {
    Parameter p;
    const_cast<PluginCymbals*>(this)->initParameter(index, p);
    return std::string(p.name.buffer());
}

bool PluginCymbals::parameterIsContinuous(uint32_t index) const {
    for (const ModTargetClass& c : kModTargetClasses) {
        if (index >= c.first && index <= c.last) return true;
    }
    return false;
}

// Round 2 Piece 1: canonical default (normalized 0..1) for the given param,
// derived by running initParameter into a temp Parameter and reading
// Parameter::ranges.def. Mirrors the constructor's seed loop at
// PluginCymbals.cpp:681-684 (which writes the same value into m_paramBase).
// Reads as a const: initParameter is logically const for this query path and
// the cast<>-away is required because the DPF Parameter struct is mutated in
// place to receive the description (no member state changes).
float PluginCymbals::getParameterDefault(uint32_t index) const {
    if (index >= kParameterCount) return 0.0f;
    Parameter p;
    const_cast<PluginCymbals*>(this)->initParameter(index, p);
    return p.ranges.def;
}

void PluginCymbals::applyModulatedDetached(IrExportRenderState& st, const IrExportInputSnapshot& in,
                                           uint32_t target, float delta) {
    const auto clamped = [&delta](float base) { return std::clamp(base + delta, 0.0f, 1.0f); };

    // Dispersion family (stage 0 = live dispersion, stage 1 = dispersion2).
    if (target >= kParamDispFB && target <= kParamDispFBOverall) {
        const DispersionFamilySnapshot& s = in.dispersion[0];
        const float fb = (target == kParamDispFB) ? clamped(s.feedback) : s.feedback;
        const float fbOv = (target == kParamDispFBOverall) ? clamped(s.feedbackOverall) : s.feedbackOverall;
        st.dispersion.setFeedback(DispersionStage::feedbackFromControl(fb) * DispersionStage::overallFeedbackScaleFromControl(fbOv));
        st.dispersion.setDelaySamples(((target == kParamDispDelay) ? clamped(s.delay) : s.delay) * 2000.0f);
        st.dispersion.setInputGain((target == kParamDispGain) ? clamped(s.gain) : s.gain);
    } else if (target >= kParamDisp2FB && target <= kParamDisp2FBOverall) {
        const DispersionFamilySnapshot& s = in.dispersion[1];
        const float fb = (target == kParamDisp2FB) ? clamped(s.feedback) : s.feedback;
        const float fbOv = (target == kParamDisp2FBOverall) ? clamped(s.feedbackOverall) : s.feedbackOverall;
        st.dispersion2.setFeedback(DispersionStage::feedbackFromControl(fb) * DispersionStage::overallFeedbackScaleFromControl(fbOv));
        st.dispersion2.setDelaySamples(((target == kParamDisp2Delay) ? clamped(s.delay) : s.delay) * 2000.0f);
        st.dispersion2.setInputGain((target == kParamDisp2Gain) ? clamped(s.gain) : s.gain);
    } else if (target >= kParamNudge1 && target <= kParamNudgeAmount20) {
        const int i = (int)(target - kParamNudge1) / 2;
        if ((target - kParamNudge1) % 2 == 0) st.dispersion.setNudge(i, clamped(in.dispersion[0].nudge[i]));
        else st.dispersion.setAmount(i, clamped(in.dispersion[0].nudgeAmount[i]));
    } else if (target >= kParamNudge2_1 && target <= kParamNudgeAmount2_20) {
        const int i = (int)(target - kParamNudge2_1) / 2;
        if ((target - kParamNudge2_1) % 2 == 0) st.dispersion2.setNudge(i, clamped(in.dispersion[1].nudge[i]));
        else st.dispersion2.setAmount(i, clamped(in.dispersion[1].nudgeAmount[i]));
    } else if (target >= kParamVibRate && target <= kParamVibStepMix) {
        if (target == kParamVibRate) st.dispersion.setVibratoRate(10.0f + clamped(in.dispersion[0].vibRate) * 1990.0f);
        else if (target == kParamVibDepth) st.dispersion.setVibratoDepth(clamped(in.dispersion[0].vibDepth) * 40.0f);
        else if (target == kParamVibSmooth) st.dispersion.setVibratoSmooth(clamped(in.dispersion[0].vibSmooth));
        else if (target == kParamVibStepMix) st.dispersion.setVibratoStepMix(clamped(in.dispersion[0].vibStepMix));
        // kParamVibShape is int (never routed).
    } else if (target >= kParamVib2Rate && target <= kParamVib2StepMix) {
        if (target == kParamVib2Rate) st.dispersion2.setVibratoRate(10.0f + clamped(in.dispersion[1].vibRate) * 1990.0f);
        else if (target == kParamVib2Depth) st.dispersion2.setVibratoDepth(clamped(in.dispersion[1].vibDepth) * 40.0f);
        else if (target == kParamVib2Smooth) st.dispersion2.setVibratoSmooth(clamped(in.dispersion[1].vibSmooth));
        else if (target == kParamVib2StepMix) st.dispersion2.setVibratoStepMix(clamped(in.dispersion[1].vibStepMix));
    } else if (target >= kParamChaos && target <= kParamChaosEnvDecay) {
        if (target == kParamChaos) st.dispersion.setChaos(clamped(in.dispersion[0].chaosFactor));
        else if (target == kParamChaosEnvAmount) st.dispersion.setChaosEnvAmount(clamped(in.dispersion[0].chaosEnvAmount));
        else st.dispersion.setChaosEnvDecay(clamped(in.dispersion[0].chaosEnvDecay));
    } else if (target >= kParamChaos2 && target <= kParamChaos2EnvDecay) {
        if (target == kParamChaos2) st.dispersion2.setChaos(clamped(in.dispersion[1].chaosFactor));
        else if (target == kParamChaos2EnvAmount) st.dispersion2.setChaosEnvAmount(clamped(in.dispersion[1].chaosEnvAmount));
        else st.dispersion2.setChaosEnvDecay(clamped(in.dispersion[1].chaosEnvDecay));
    } else if (target >= kParamPDDrive && target <= kParamPDMix) {
        const DispersionFamilySnapshot& s = in.dispersion[0];
        st.dispersion.setPDDrive((target == kParamPDDrive) ? clamped(s.pdDrive) : s.pdDrive);
        st.dispersion.setPDNormalize((target == kParamPDNormalize) ? clamped(s.pdNormalize) : s.pdNormalize);
        st.dispersion.setPDTone((target == kParamPDTone) ? clamped(s.pdTone) : s.pdTone);
        st.dispersion.setPDBias((target == kParamPDBias) ? clamped(s.pdBias) : s.pdBias);
        st.dispersion.setPDSpread((target == kParamPDSpread) ? clamped(s.pdSpread) : s.pdSpread);
        st.dispersion.setPDMix((target == kParamPDMix) ? clamped(s.pdMix) : s.pdMix);
    } else if (target >= kParamPD2Drive && target <= kParamPD2Mix) {
        const DispersionFamilySnapshot& s = in.dispersion[1];
        st.dispersion2.setPDDrive((target == kParamPD2Drive) ? clamped(s.pdDrive) : s.pdDrive);
        st.dispersion2.setPDNormalize((target == kParamPD2Normalize) ? clamped(s.pdNormalize) : s.pdNormalize);
        st.dispersion2.setPDTone((target == kParamPD2Tone) ? clamped(s.pdTone) : s.pdTone);
        st.dispersion2.setPDBias((target == kParamPD2Bias) ? clamped(s.pdBias) : s.pdBias);
        st.dispersion2.setPDSpread((target == kParamPD2Spread) ? clamped(s.pdSpread) : s.pdSpread);
        st.dispersion2.setPDMix((target == kParamPD2Mix) ? clamped(s.pdMix) : s.pdMix);
    } else if (target >= kParamEQ1Freq && target <= kParamEQ5Q) {
        const int band = (int)(target - kParamEQ1Freq) / 4;
        const int sub = (int)(target - kParamEQ1Freq) % 4;
        const float f = (sub == 0) ? clamped(in.eqFreq[band]) : in.eqFreq[band];
        const float g = (sub == 1) ? clamped(in.eqGain[band]) : in.eqGain[band];
        const float q = (sub == 2) ? clamped(in.eqQ[band]) : in.eqQ[band];
        const int typeIdx = std::clamp((int)std::lround(in.eqFilterType[band]), 0, 7);
        const float gainDb = (typeIdx >= 4 && typeIdx <= 6) ? (std::clamp(g, 0.0f, 1.0f) - 0.5f) * 48.0f : 0.0f;
        st.stickEQNodes[band].setModulatedValues(DSPUtils::normToHz( f), DSPUtils::qFromParam(q), gainDb);
    } else if (target >= kParamEQ1Sat && target <= kParamEQ5Sat) {
        st.stickEQNodes[target - kParamEQ1Sat].setSaturation(clamped(in.eqSat[target - kParamEQ1Sat]));
    } else if (target >= kParamFinalEQ1Freq && target <= kParamFinalEQ5Q) {
        const int band = (int)(target - kParamFinalEQ1Freq) / 4;
        const int sub = (int)(target - kParamFinalEQ1Freq) % 4;
        const float f = (sub == 0) ? clamped(in.feqFreq[band]) : in.feqFreq[band];
        const float g = (sub == 1) ? clamped(in.feqGain[band]) : in.feqGain[band];
        const float q = (sub == 2) ? clamped(in.feqQ[band]) : in.feqQ[band];
        const int typeIdx = std::clamp((int)std::lround(in.feqFilterType[band]), 0, 7);
        const float gainDb = (typeIdx >= 4 && typeIdx <= 6) ? (std::clamp(g, 0.0f, 1.0f) - 0.5f) * 48.0f : 0.0f;
        st.finalEQNodes[band].setModulatedValues(DSPUtils::normToHz( f), DSPUtils::qFromParam(q), gainDb);
    } else if (target >= kParamFinalEQ1Sat && target <= kParamFinalEQ5Sat) {
        st.finalEQNodes[target - kParamFinalEQ1Sat].setSaturation(clamped(in.feqSat[target - kParamFinalEQ1Sat]));
    } else if (target >= kParamTiltAmount && target <= kParamTiltPivot) {
        const float amt = (target == kParamTiltAmount) ? clamped(in.stickTiltAmount) : in.stickTiltAmount;
        const float piv = (target == kParamTiltPivot) ? clamped(in.stickTiltPivot) : in.stickTiltPivot;
        const float pivotHz = DSPUtils::normToHz( std::clamp(piv, 0.0f, 1.0f));
        const float tiltDb = DSPUtils::tiltDbFromParam(std::clamp(amt, 0.0f, 1.0f));
        st.stickTiltLow.setParams(Biquad::LOW_SHELF, pivotHz, -tiltDb, 0.707f, in.sampleRate);
        st.stickTiltHigh.setParams(Biquad::HIGH_SHELF, pivotHz, tiltDb, 0.707f, in.sampleRate);
    } else if (target >= kParamFinalTiltAmount && target <= kParamFinalTiltPivot) {
        const float amt = (target == kParamFinalTiltAmount) ? clamped(in.finalTiltAmount) : in.finalTiltAmount;
        const float piv = (target == kParamFinalTiltPivot) ? clamped(in.finalTiltPivot) : in.finalTiltPivot;
        const float pivotHz = DSPUtils::normToHz( std::clamp(piv, 0.0f, 1.0f));
        const float tiltDb = DSPUtils::tiltDbFromParam(std::clamp(amt, 0.0f, 1.0f));
        st.finalTiltLow.setParams(Biquad::LOW_SHELF, pivotHz, -tiltDb, 0.707f, in.sampleRate);
        st.finalTiltHigh.setParams(Biquad::HIGH_SHELF, pivotHz, tiltDb, 0.707f, in.sampleRate);
    } else if (target >= kParamComb1Freq && target <= kParamComb40HPF) {
        const int s = (int)(target - kParamComb1Freq) / 4;
        const int t = (int)(target - kParamComb1Freq) % 4;
        const float v = clamped(in.combParams[target - kParamComb1Freq]);
        if (t == 0) st.combBank.setFrequency(s, DSPUtils::normToHz( v));
        else if (t == 1) st.combBank.setFeedback(s, std::min(0.998f, v));
        else if (t == 2) st.combBank.setLPF(s, DSPUtils::normToHz( v));
        else st.combBank.setHPF(s, DSPUtils::normToHz( v));
    } else if (target >= kParamComb1Scale && target <= kParamComb40Scale) {
        st.combBank.setStageScaleFactor((int)(target - kParamComb1Scale), DSPUtils::scaleFactorFromParam( clamped(in.combScalePerStage[target - kParamComb1Scale])));
    } else if (target >= kParamComb1SatAmount && target <= kParamComb40SatAmount) {
        st.combBank.setSaturatorAmount((int)(target - kParamComb1SatAmount), clamped(in.combSatAmount[target - kParamComb1SatAmount]));
    } else if (target >= kParamComb1StressOnset && target <= kParamComb40StressKnee) {
        const int off = (int)(target - kParamComb1StressOnset);
        if (off < 40) st.combBank.setSatStressOnset(off, clamped(in.combStressOnset[off]));
        else st.combBank.setSatStressKnee(off - 40, clamped(in.combStressKnee[off - 40]));
    } else if (target >= kParamComb1Mix && target <= kParamComb40Mix) {
        st.combBank.setStageMix((int)(target - kParamComb1Mix), clamped(in.combMix[target - kParamComb1Mix]));
    } else if (target >= kParamComb1ModeRate && target <= kParamComb40ModeAmount) {
        const int off = (int)(target - kParamComb1ModeRate);
        if (off < 40) st.combBank.setModeRate(off, clamped(in.combModeRate[off]));
        else st.combBank.setModeAmount(off - 40, clamped(in.combModeAmount[off - 40]));
    } else if (target >= kParamComb1ModeA && target <= kParamComb40ModeB) {
        const int off = (int)(target - kParamComb1ModeA);
        if (off < 40) st.combBank.setModeA(off, clamped(in.combModeA[off]));
        else st.combBank.setModeB(off - 40, clamped(in.combModeB[off - 40]));
    } else if (target >= kParamComb1PartRelPos && target <= kParamComb40PartStiffExp) {
        const int off = (int)(target - kParamComb1PartRelPos);
        if (off < 40) st.combBank.setPartRelPos(off, clamped(in.combPartRelPos[off]));
        else if (off < 80) st.combBank.setPartFreqDamp(off - 40, clamped(in.combPartFreqDamp[off - 40]));
        else if (off < 120) st.combBank.setPartStiffness(off - 80, clamped(in.combPartStiffness[off - 80]));
        else st.combBank.setPartStiffExp(off - 120, clamped(in.combPartStiffExp[off - 120]));
    } else if (target >= kParamCombGroup1Shift && target <= kParamCombGroup10Shift) {
        // Snapshot stores Hz; renormalize, clamp the modulated param, convert back
        // so the detached render matches the live clamp behavior.
        const int g = (int)(target - kParamCombGroup1Shift);
        const float baseParam = in.combGroupShiftHz[g] / 20000.0f + 0.5f;
        st.combBank.setShift(g, (std::clamp(baseParam + delta, 0.0f, 1.0f) - 0.5f) * 20000.0f);
    } else if (target >= kParamCombGroup1Gain && target <= kParamCombGroup10Gain) {
        const int g = (int)(target - kParamCombGroup1Gain);
        const float v = clamped(in.combGroupGain[g]);
        const float db = DSPUtils::normGainToDb(v);
        st.combBank.setGroupGain(g, in.combGroupOn[g] > 0.5f ? std::pow(10.0f, db / 20.0f) : 0.0f);
    } else if (target >= kParamCombGroup1Cluster && target <= kParamCombGroup10Cluster) {
        st.combBank.setCluster((int)(target - kParamCombGroup1Cluster), clamped(in.combGroupCluster[target - kParamCombGroup1Cluster]));
    } else if (target == kParamCombScale) {
        st.combBank.setScaleFactor(DSPUtils::scaleFactorFromParam( clamped(in.combScale)));
    } else if (target >= kParamFinalComb1Freq && target <= kParamFinalComb4HPF) {
        const int s = (int)(target - kParamFinalComb1Freq) / 4;
        const int t = (int)(target - kParamFinalComb1Freq) % 4;
        const float v = clamped(in.finalCombParams[target - kParamFinalComb1Freq]);
        if (t == 0) st.finalCombBank.setFrequency(s, DSPUtils::normToHz( v));
        else if (t == 1) st.finalCombBank.setFeedback(s, std::min(0.998f, v));
        else if (t == 2) st.finalCombBank.setLPF(s, DSPUtils::normToHz( v));
        else st.finalCombBank.setHPF(s, DSPUtils::normToHz( v));
    } else if (target >= kParamFinalComb1Mix && target <= kParamFinalComb4Mix) {
        st.finalCombBank.setStageMix((int)(target - kParamFinalComb1Mix), clamped(in.finalCombMix[target - kParamFinalComb1Mix]));
    } else if (target >= kParamFinalComb1ModeRate && target <= kParamFinalComb4ModeAmount) {
        const int off = (int)(target - kParamFinalComb1ModeRate);
        if (off < 4) st.finalCombBank.setModeRate(off, clamped(in.finalCombModeRate[off]));
        else st.finalCombBank.setModeAmount(off - 4, clamped(in.finalCombModeAmount[off - 4]));
    } else if (target >= kParamFinalComb1ModeA && target <= kParamFinalComb4ModeB) {
        const int off = (int)(target - kParamFinalComb1ModeA);
        if (off < 4) st.finalCombBank.setModeA(off, clamped(in.finalCombModeA[off]));
        else st.finalCombBank.setModeB(off - 4, clamped(in.finalCombModeB[off - 4]));
    } else if (target >= kParamFinalComb1PartRelPos && target <= kParamFinalComb4PartStiffExp) {
        const int off = (int)(target - kParamFinalComb1PartRelPos);
        if (off < 4) st.finalCombBank.setPartRelPos(off, clamped(in.finalCombPartRelPos[off]));
        else if (off < 8) st.finalCombBank.setPartFreqDamp(off - 4, clamped(in.finalCombPartFreqDamp[off - 4]));
        else if (off < 12) st.finalCombBank.setPartStiffness(off - 8, clamped(in.finalCombPartStiffness[off - 8]));
        else st.finalCombBank.setPartStiffExp(off - 12, clamped(in.finalCombPartStiffExp[off - 12]));
    } else if (target == kParamFinalComb4SatAmount) {
        st.finalCombBank.setSaturatorAmount(3, clamped(in.finalComb4SatAmount));
    } else if (target >= kParamFinalComb4StressOnset && target <= kParamFinalComb4StressKnee) {
        if (target == kParamFinalComb4StressOnset) st.finalCombBank.setSatStressOnset(3, clamped(in.finalComb4StressOnset));
        else st.finalCombBank.setSatStressKnee(3, clamped(in.finalComb4StressKnee));
    } else if (target == kParamFinalCombShift) {
        const float baseParam = in.finalCombShiftHz / 20000.0f + 0.5f;
        st.finalCombBank.setShift(0, (std::clamp(baseParam + delta, 0.0f, 1.0f) - 0.5f) * 20000.0f);
    } else if (target == kParamFinalCombScale) {
        st.finalCombBank.setScaleFactor(DSPUtils::scaleFactorFromParam( clamped(in.finalCombScale)));
    } else if (target >= kParamFinalCombLimiterOn && target <= kParamFinalCombLimiterRelease) {
        if (target == kParamFinalCombLimiterOn)
            st.finalCombBank.setGroupLimiterOn(0, clamped(in.finalLimiterOn));
        else if (target == kParamFinalCombLimiterThreshold)
            st.finalCombBank.setGroupLimiterThreshold(0, clamped(in.finalLimiterThreshold));
        else
            st.finalCombBank.setGroupLimiterRelease(0, clamped(in.finalLimiterRelease));
    } else if (target == kParamFinalCombGroupGain) {
        const float v = clamped(in.finalCombGroupGain);
        const float db = DSPUtils::normGainToDb(v);
        st.finalCombBank.setGroupGain(0, std::pow(10.0f, db / 20.0f));
    } else if (target == kParamFinalCombGroupCluster) {
        st.finalCombBank.setCluster(0, clamped(in.finalCombGroupCluster));
    } else if (target == kParamResonatorDiffusion) {
        st.combBank.setResonatorDiffusion(clamped(in.resonatorDiffusion));
    } else if (target == kParamCombMemoryDamping) {
        const float v = clamped(in.combMemoryDamping);
        st.combBank.setMemoryDamping(v);
        st.finalCombBank.setMemoryDamping(v);
    } else if (target == kParamVolume) {
        st.modVolume = clamped(in.volumeParam);
    } else if (target == kParamEQMix) {
        st.modEqMix = clamped(in.eqMix);
    } else if (target == kParamFinalEQMix) {
        st.modFeqMix = clamped(in.feqMix);
    } else if (target == kParamDispCrossLink) {
        st.modDispCrossLink = clamped(in.dispCrossLink);
    }
    // Macros are deliberately NOT mod targets (kModTargetClasses excludes them),
    // so macro-target routes no-op here exactly as they do in the live path.
    // All other classes (member-float / stickBank / FX slots / bloom /
    // final-AR / clip consumers) are baked into the snapshot — the IR chain
    // (impulse -> mod -> stickEQ/tilt -> dispersion -> combBank -> finalCombBank
    // -> finalEQ -> finalTilt -> volume) has no stickBank, FX slots, bloom,
    // finalOutEnv, or clip stages, so those families stay no-op by design.
}

void PluginCymbals::run(const float** inputs, float** outputs, uint32_t frames,
                        const MidiEvent* midiEvents, uint32_t midiEventCount) {
    struct DenormalGuard {
#if CYMBALS_HAS_SSE
        unsigned int saved;
        DenormalGuard() : saved(_mm_getcsr()) {
            _mm_setcsr(saved | _MM_FLUSH_ZERO_ON | _MM_DENORMALS_ZERO_ON);
        }
        ~DenormalGuard() { _mm_setcsr(saved); }
#else
        DenormalGuard() = default;
        ~DenormalGuard() = default;
#endif
    };
    const DenormalGuard _denorm_guard;

    if (fPendingPresetReady.load(std::memory_order_acquire)) {
        // Floats only here: the preset JSON was serialized on the staging side
        // (applyPresetValuesSafely) before the ready flag was raised. Delivery
        // is a fixed-size memcpy into the preallocated scratch under the mutex
        // (no allocation, no long lock on this thread); the ~2.3k per-parameter
        // applies then run outside the lock. A preset staged while one is being
        // delivered is never cleared: the generation check re-reads it on the
        // next block, so no load is ever dropped.
        uint32_t genToApply = 0;
        // Never block the audio thread on the staging side. If the stager is
        // mid-assign, keep the ready flag set and retry next block.
        std::unique_lock<std::mutex> guard(fPendingPresetMutex, std::try_to_lock);
        if (guard.owns_lock()) {
            fPendingPresetReady.store(false, std::memory_order_release);
            if (fPresetAppliedGen != fPresetStagedGen &&
                fPendingPresetValues.size() == kParameterCount) {
                std::memcpy(fPresetApplyScratch.data(), fPendingPresetValues.data(),
                            kParameterCount * sizeof(float));
                fPresetAppliedGen = fPresetStagedGen;
                genToApply = fPresetStagedGen;
            }
        }
        if (genToApply != 0) {
            applyAllParameterValues(fPresetApplyScratch);
            fPendingDspReset.store(true, std::memory_order_release);
        }
    }

    if (fPendingDspReset.exchange(false, std::memory_order_acq_rel)) {
        std::memset(fActiveNotes, 0, sizeof(fActiveNotes));
        fActiveNoteCount = 0;
        clearAllVoices(); // preset load / reset retires the poly pool too
        combBank.reset();
        finalCombBank.reset();
        dispersion.clear();
        dispersion2.clear();
        inharmBloomEnv.releaseNote();
        inharm2BloomEnv.releaseNote();
        stickBank.releaseNote();
        finalOutEnv.releaseNote();
        masterOutEnv.releaseNote();
        m_modEngine.noteOff();
        fReverbProtectEnv = 0.0f;
        fReverbProtectSlowEnv = 0.0f;
        fReverbProtectGain = 1.0f;
        if (fAnalyzer) fAnalyzer->reset();
        for (int i = 0; i < 6; ++i) {
            m_fxSlots[i].reset();
            if (m_internalFx[i] != nullptr) m_internalFx[i]->reset();
            if (m_vendoredFx[i] != nullptr) m_vendoredFx[i]->reset();
        }
        for (int band = 0; band < 5; ++band) {
            stickEQNodes[band].reset();
            finalEQNodes[band].reset();
        }
        m_modEngine.reset();
    }

    applyPendingEQRebuilds();
    applyPendingSlotTypeChanges();

    // Tempo for tempo-synced mod sources (LFO/STEP). When the host provides no
    // transport (bpm <= 0), the engine keeps its internal 120 BPM fallback.
    {
        const double bpm = getTimePosition().bbt.beatsPerMinute;
        if (bpm > 0.0) m_modEngine.setTempoBpm((float)bpm);
    }

    struct ActiveFxSlot {
        int slot;
    };

    ActiveFxSlot activeFxSlots[6] = {};
    int activeFxSlotCount = 0;
    for (int pos = 0; pos < 6; ++pos) {
        const int slot = m_slotProcessOrder[pos];
        if (m_slotBypass[slot]) continue;
        if (m_surgeFx[slot] == nullptr && m_internalFx[slot] == nullptr && m_vendoredFx[slot] == nullptr &&
            m_fxSlots[slot].getCurrentType() == AirSuiteUniversal::NONE) continue;
        activeFxSlots[activeFxSlotCount++] = {slot};
    }

    const bool fastMode = (processingMode > 0.5f);
    const bool active1 = (disp1Skip < 0.5f);
    const bool active2 = (disp2Skip < 0.5f);
    const bool dispersionSolo = (dispSolo > 0.5f);
    const bool combBankEnabled = (combBankOn > 0.5f);
    const bool finalCombBankEnabled = (finalCombBankOn > 0.5f);
    const float crossLinkHalf = dispCrossLink * 0.5f;
    const float dirSign = (inharmBloomDir < 0.5f) ? -1.0f : 1.0f;
    const float dirSign2 = (inharm2BloomDir < 0.5f) ? -1.0f : 1.0f;
    float combBaseShiftHz[10] = {};
    for (int g = 0; g < 10; ++g) {
        combBaseShiftHz[g] = (combGroupShift[g] - 0.5f) * 20000.0f;
    }

    // (#8) Bow/swell re-strike interval: log-mapped 0.5..20 Hz.
    uint32_t bowInterval = 0;
    if (bowEnable > 0.5f && bowLevel > 0.0f) {
        const float hz = 0.5f * std::pow(40.0f, std::clamp(bowRate, 0.0f, 1.0f));
        bowInterval = (uint32_t)std::max(1.0f, fSampleRate / hz);
    }

    float* outL = outputs[0]; float* outR = outputs[1];
    // Audio input is an excitation source alongside MIDI. It is summed into
    // the same bus that the stick bank feeds, so the stick EQ, dispersion,
    // comb/filter banks and FX chain all process it as normal.
    const float* inL = (inputs != nullptr) ? inputs[0] : nullptr;
    const float* inR = (inputs != nullptr && inputs[1] != nullptr) ? inputs[1] : nullptr;
    std::memset(outL, 0, frames * sizeof(float)); std::memset(outR, 0, frames * sizeof(float));
    // Loop-invariant reverb-protection coefficients (sample rate is fixed per block).
    const float protectSr = std::max(1.0f, fSampleRate);
    const float protectAttack = 1.0f - std::exp(-1.0f / (0.001f * protectSr));
    const float protectRelease = 1.0f - std::exp(-1.0f / (0.050f * protectSr));
    uint32_t midiIdx = 0;
    for (uint32_t i = 0; i < frames; ++i) {
        while (midiIdx < midiEventCount && midiEvents[midiIdx].frame == i) {
            if (midiEvents[midiIdx].size == 3) {
                const uint8_t status = midiEvents[midiIdx].data[0] & 0xF0;
                const uint8_t channel = midiEvents[midiIdx].data[0] & 0x0F;
                if (status == 0x90 && midiEvents[midiIdx].data[2] > 0) {
                    uint8_t noteNum = midiEvents[midiIdx].data[1] & 0x7F; // mask once at parse (LOW-9)
                    if (!fActiveNotes[noteNum]) {
                        fActiveNotes[noteNum] = true;
                        fActiveNoteCount++;
                    }
                    lastVelocity = (float)midiEvents[midiIdx].data[2] / 127.0f;
                    m_modEngine.setPerformanceInput(ModulationEngine::VELOCITY, lastVelocity);
                    m_modEngine.setPerformanceInput(ModulationEngine::NOTE, noteNum / 127.0f);
                    if (voiceMode > 0.5f) {
                        // Poly: per-voice exciter/bloom/gate; shared dispersion
                        // stages re-strike per hit (bus-level resonator); mod
                        // engine broadcasts on the first voice on.
                        triggerPolyVoice(noteNum, lastVelocity, channel);
                        finalOutEnv.trigger();
                        masterOutEnv.trigger();
                    } else {
                        // Mono legacy restrike path — byte-identical sequence.
                        handleMidiNote(noteNum);
                        stickBank.triggerNote(lastNoteFreq, lastVelocity);
                        finalOutEnv.trigger();
                        masterOutEnv.trigger();
                    }
                } else if (status == 0x80 || (status == 0x90 && midiEvents[midiIdx].data[2] == 0)) {
                    uint8_t noteNum = midiEvents[midiIdx].data[1] & 0x7F; // mask once at parse (LOW-9)
                    if (fActiveNotes[noteNum]) {
                        fActiveNotes[noteNum] = false;
                        if (fActiveNoteCount > 0) fActiveNoteCount--;
                    }
                    // Release velocity: 0x80 carries it in data[2]; a 0x90-vel0
                    // event is an instant release by contract. sens=0 -> mul=1
                    // (legacy instant behavior, byte-identical).
                    const bool vel0 = (status == 0x90);
                    const float relMul = releaseTimeMul(vel0 ? -1.0f : (float)midiEvents[midiIdx].data[2] / 127.0f);
                    if (voiceMode > 0.5f) {
                        releasePolyVoice(noteNum, relMul, vel0);
                        // Last KEY off (not tail end) releases the shared mix
                        // gate and broadcasts mod-engine noteOff — same key
                        // semantics as the mono path above.
                        if (fActiveNoteCount == 0) {
                            finalOutEnv.releaseNote();
                            masterOutEnv.releaseNote();
                            m_modEngine.noteOff();
                        }
                    } else if (fActiveNoteCount == 0) {
                        // Always re-apply: mul==1 restores the exact legacy
                        // release after any earlier sens>0 scaling.
                        stickBank.setAmpReleaseScale(relMul);
                        finalOutEnv.setRelease(finalEnvReleaseSec(finalARRelease) * relMul);
                        // Master ADSR is independent of the FX-tail ADSR. This
                        // line used the TAIL release, so moving the tail
                        // Release knob (even with tail ADSR OFF) retuned the
                        // master envelope on every note-off.
                        masterOutEnv.setRelease(finalEnvReleaseSec(masterARRelease) * relMul);
                        stickBank.releaseNote();
                        finalOutEnv.releaseNote();
                        masterOutEnv.releaseNote();
                        m_modEngine.noteOff();
                    }
                } else if (status == 0xB0) {
                    const uint8_t cc = midiEvents[midiIdx].data[1];
                    const uint8_t val = midiEvents[midiIdx].data[2];
                    if (cc == 1) {
                        // CC1 mod wheel.
                        m_modEngine.setPerformanceInput(ModulationEngine::MODWHEEL, (float)val / 127.0f);
                    } else if (cc == 120 || cc == 123) {
                        // CC120 All Sound Off / CC123 All Notes Off: host panic.
                        std::memset(fActiveNotes, 0, sizeof(fActiveNotes));
                        fActiveNoteCount = 0;
                        clearAllVoices(); // poly pool: instant all-off
                        stickBank.releaseNote();
                        finalOutEnv.releaseNote();
                        masterOutEnv.releaseNote();
                        m_modEngine.noteOff();
                    } else if (cc == 64) {
                        // (#9) Sustain pedal — mod-matrix source only.
                        m_modEngine.setPerformanceInput(ModulationEngine::SUSTAIN, (float)val / 127.0f);
                    } else if (cc == 67) {
                        // (#9) Sostenuto/soft-pedal family — mod source only.
                        m_modEngine.setPerformanceInput(ModulationEngine::SOFTPEDAL, (float)val / 127.0f);
                    } else if (cc == 11) {
                        // (#9) Expression pedal — mod source only.
                        m_modEngine.setPerformanceInput(ModulationEngine::EXPRESSION, (float)val / 127.0f);
                    } else if (cc == 74) {
                        // Round 3 Piece 2: MPE Y-axis (slide) per-voice. Under
                        // effective MPE (manual mode OR RPN [0,6] activation),
                        // CC74 on the per-note manager channel moves the
                        // pitch of the latest triggered voice on that channel.
                        // Bipolar 0..127 -> -1..+1 -> ±24 st full-scale, so a
                        // press+slide combination still adds with the bend.
                        setVoiceSlideFromCc74(channel, val);
                    } else if (cc == 101 && val == 0) {
                        rpnMsbZeroSeen = true;
                        rpnArmed = rpnMsbZeroSeen && rpnLsbSixSeen;
                        rpnGotMsb = false;
                    } else if (cc == 100 && val == 6) {
                        rpnLsbSixSeen = true;
                        rpnArmed = rpnMsbZeroSeen && rpnLsbSixSeen;
                        rpnGotMsb = false;
                    } else if (cc == 99) {
                        // (#18) NRPN MSB selector — independent from the RPN
                        // machine; its armed state survives RPN selector CCs.
                        nrpnSelMsb = (uint8_t)(val & 0x7F);
                        nrpnMsbSeen = true;
                        nrpnArmed = nrpnMsbSeen && nrpnLsbSeen;
                    } else if (cc == 98) {
                        nrpnSelLsb = (uint8_t)(val & 0x7F);
                        nrpnLsbSeen = true;
                        nrpnArmed = nrpnMsbSeen && nrpnLsbSeen;
                    } else if (cc == 6 && (nrpnArmed || rpnArmed)) {
                        // (#17/#18) Shared data-entry dispatch; an NRPN
                        // selection outranks a concurrently armed RPN one.
                        if (nrpnArmed) {
                            nrpnDataMsb = val;
                            nrpnGotMsb = true;
                            nrpnArmed = false;
                            nrpnMsbSeen = nrpnLsbSeen = false;
                            applyNrpnDataEntry((uint16_t)(val << 7));
                        } else {
                            // RPN [0,6] MPE Activation Data Entry MSB (+ optional CC38 fine).
                            rpnMsbVal = val;
                            rpnGotMsb = true;
                            rpnArmed = false;
                            rpnMsbZeroSeen = rpnLsbSixSeen = false;
                            applyRpnDataEntry((uint16_t)(val << 7));
                        }
                    } else if (cc == 38 && nrpnGotMsb) {
                        applyNrpnDataEntry((uint16_t)((nrpnDataMsb << 7) | val));
                    } else if (cc == 38 && rpnGotMsb) {
                        applyRpnDataEntry((uint16_t)((rpnMsbVal << 7) | val));
                    } else {
                        // Non-reserved controller. MIDI-learn capture first:
                        // one armed CC performs exactly one binding and does
                        // NOT also route through existing binds (one CC, one
                        // action). Otherwise route macro/param binds. Either
                        // way, retire any half-typed RPN/NRPN selections.
                        const int cci = (int)(cc & 0x7F);
                        const int learnKind = fLearnKind.load(std::memory_order_acquire);
                        if (learnKind >= 0) {
                            const int learnIdx = fLearnIndex.load(std::memory_order_acquire);
                            if (learnKind == 0) setCcParamBind(cci, (uint32_t)learnIdx);
                            else if (learnKind == 1) setCcMacroBind(cci, learnIdx);
                            fLearnKind.store(-1, std::memory_order_release);
                            fLastLearnCc.store(cci, std::memory_order_release);
                        } else {
                            const uint8_t macroSel = ccMacroBind[cci];
                            if (macroSel != 0) {
                                m_modEngine.setMacro(macroSel - 1, (float)val / 127.0f);
                            } else if (ccToParam[cci] >= 0) {
                                // (#17) MIDI-learn write path: RT-safe subset of
                                // setParameterValue (see applyMidiParameterWrite).
                                applyMidiParameterWrite((uint32_t)ccToParam[cci], (float)val / 127.0f);
                            }
                        }
                        rpnMsbZeroSeen = rpnLsbSixSeen = false;
                        rpnArmed = false;
                        rpnGotMsb = false;
                        nrpnMsbSeen = nrpnLsbSeen = false;
                        nrpnArmed = false;
                        nrpnGotMsb = false;
                    }
                } else if (status == 0xA0) {
                    // (#6) Polyphonic aftertouch: channel-mapped to the LATEST
                    // poly voice triggered on that channel; latest value also
                    // feeds the POLYPRESS mod source.
                    const float press = (float)midiEvents[midiIdx].data[2] / 127.0f;
                    m_modEngine.setPerformanceInput(ModulationEngine::POLYPRESS, press);
                    if (mpeEffective()) {
                        const int8_t slot = m_voiceChannelMap[channel];
                        if (slot >= 0 && m_voices[slot].active) {
                            m_voices[slot].pressure = press;
                        }
                    }
                } else if (status == 0xE0) {
                    // Pitch bend (14-bit, center 8192): global perf input, plus
                    // per-note bend on the MPE-mapped voice only. Range pinned:
                    // ±24 semitones full-scale at kParamMpeBendAmount=1.
                    const float bend = (float)((midiEvents[midiIdx].data[2] << 7) | midiEvents[midiIdx].data[1]) / 8192.0f - 1.0f;
                    m_modEngine.setPerformanceInput(ModulationEngine::PITCHBEND, std::clamp(bend, -1.0f, 1.0f));
                    if (mpeEffective()) {
                        const int8_t slot = m_voiceChannelMap[channel];
                        if (slot >= 0 && m_voices[slot].active) {
                            m_voices[slot].bendSemis = bend * 24.0f * std::clamp(mpeBendAmount, 0.0f, 1.0f);
                            // Combine with any active slide so a bend never
                            // wipes it (applyVoicePitch clamps and applies).
                            applyVoicePitch(slot);
                        }
                    }
                }
            } else if (midiEvents[midiIdx].size == 2 && (midiEvents[midiIdx].data[0] & 0xF0) == 0xD0) {
                // Channel pressure (aftertouch): global perf input, plus the
                // mapped voice's pressure under effective MPE, and POLYPRESS.
                const float press = (float)midiEvents[midiIdx].data[1] / 127.0f;
                m_modEngine.setPerformanceInput(ModulationEngine::AFTERTOUCH, press);
                m_modEngine.setPerformanceInput(ModulationEngine::POLYPRESS, press);
                if (mpeEffective()) {
                    const uint8_t channel = (uint8_t)(midiEvents[midiIdx].data[0] & 0x0F);
                    const int8_t slot = m_voiceChannelMap[channel];
                    if (slot >= 0 && m_voices[slot].active) {
                        m_voices[slot].pressure = press;
                    }
                }
            }
            midiIdx++;
        }
        // ===== Modulation System: step sources, apply routes to targets =====
        float srcOut[ModulationEngine::kNumSources];
        m_modEngine.processSample(srcOut);
        if (m_modOn > 0.5f && m_modEngine.activeRouteCount() > 0) {
            // Accumulate deltas per target so multiple routes on one knob SUM
            // (each applyModulatedTarget call re-reads the base value).
            uint32_t accTargets[ModulationEngine::kNumRoutes];
            float accDeltas[ModulationEngine::kNumRoutes];
            const int accCount = collectModRouteDeltas(m_modEngine, srcOut, accTargets, accDeltas);
            for (int k = 0; k < accCount; ++k) {
                applyModulatedTarget(accTargets[k], accDeltas[k]);
            }
        }
        float sL = 0, sR = 0;
        float polyBloomRaw = 0.0f, polyBloom2Raw = 0.0f;
        float pressDriveSum = 0.0f; // (#7) aggregate of active voices' pressure*drive
        if (voiceMode > 0.5f) {
            // Poly: sum per-voice exciters (each already gated by its own
            // choke/release state). The shared mono bank is idle.
            processVoicePool(sL, sR, polyBloomRaw, polyBloom2Raw, pressDriveSum, bowInterval);
        } else {
            stickBank.process(sL, sR);
        }
        // External audio joins the excitation bus after the stick/voice
        // generators. With no audio routed in these are zero and the synth
        // behaviour is unchanged.
        if (inL != nullptr) sL += inL[i];
        if (inR != nullptr) sR += inR[i];

        float drySL = sL, drySR = sR;
        float wetL = sL, wetR = sR;
        for (int b = 0; b < 5; ++b) {
            stickEQNodes[b].processStereoSample(wetL, wetR);
        }
        stickTiltLow.process(wetL, wetR);
        stickTiltHigh.process(wetL, wetR);
        sL = drySL * (1.0f - eqMix) + wetL * eqMix; sR = drySR * (1.0f - eqMix) + wetR * eqMix;
        
        float d1L = sL, d1R = sR, d2L = sL, d2R = sR;
        if (active1) dispersion.process(d1L, d1R);
        if (active2) dispersion2.process(d2L, d2R);
        float busL, busR;
        if (active1 && active2) {
            float xL1 = d1L * (1.0f - crossLinkHalf) + d2L * crossLinkHalf;
            float xR1 = d1R * (1.0f - crossLinkHalf) + d2R * crossLinkHalf;
            float xL2 = d2L * (1.0f - crossLinkHalf) + d1L * crossLinkHalf;
            float xR2 = d2R * (1.0f - crossLinkHalf) + d1R * crossLinkHalf;
            busL = (xL1 + xL2) * 0.5f; busR = (xR1 + xR2) * 0.5f;
        } else if (active1) { busL = d1L; busR = d1R; } else if (active2) { busL = d2L; busR = d2R; } else { busL = sL; busR = sR; }
        float finalL = 0.0f, finalR = 0.0f;
        float preFxL = 0.0f, preFxR = 0.0f;
        // Mono: shared bloom envelopes. Poly: per-voice bloom phases summed
        // (clamped) — the shared stages below see one combined depth either way.
        const float bloomEnvRaw = (voiceMode > 0.5f)
            ? std::min(polyBloomRaw, 1.0f) : inharmBloomEnv.process();
        const float bloom2EnvRaw = (voiceMode > 0.5f)
            ? std::min(polyBloom2Raw, 1.0f) : inharm2BloomEnv.process();
        // Remove near-zero tail where frequency shifting can feel "off" while still audible.
        const float bloomEnv = std::pow(std::max(0.0f, (bloomEnvRaw - 0.02f) / 0.98f), 1.35f);
        const float bloom2Env = std::pow(std::max(0.0f, (bloom2EnvRaw - 0.02f) / 0.98f), 1.35f);
        const float velocityScale = 0.35f + 0.65f * std::clamp(lastVelocity, 0.0f, 1.0f);
        const float bloomDepthHz = 450.0f * inharmBloomAmt * velocityScale;
        const float bloom2DepthHz = 450.0f * inharm2BloomAmt * velocityScale;
        fInharmBloomLevel.store(bloomEnvRaw, std::memory_order_relaxed);
        fInharm2BloomLevel.store(bloom2EnvRaw, std::memory_order_relaxed);

        if (dispersionSolo) {
            // Wet-only monitor path: hear the dispersion bus directly.
            finalL = busL;
            finalR = busR;
            preFxL = finalL;
            preFxR = finalR;
        } else {
            for (int g = 0; g < 10; ++g) {
                const float groupWeight = 0.85f + 0.03f * g;
                const float modHz = dirSign * bloomEnv * bloomDepthHz * groupWeight;
                const float mod2Hz = dirSign2 * bloom2Env * bloom2DepthHz * groupWeight;
                combBank.setShift(g, combBaseShiftHz[g] + modHz + mod2Hz);
            }
            // (#7) Pressure -> stress/saturation drive. Comb stages are a
            // shared bus (40 x 8KB delay buffers), so per-voice injection is
            // impossible without duplicating the bank; instead the SHARED
            // saturator amount scales by (1 + summed pressure*drive), clamped.
            // The scale base is captured from the LIVE bank on the engage edge
            // (MED-4), so mod-routed amounts are scaled rather than clobbered
            // by the static parameter array, and restored exactly on release
            // or panic. Fast path: with no pressure ever applied nothing is
            // written and the bank keeps its parameter-set state (defaults
            // bit-identical).
            if (combBankEnabled && (pressDriveSum > 0.0f || m_pressWasActive)) {
                if (!m_pressWasActive) {
                    for (int i = 0; i < 40; ++i)
                        m_combSatLiveBase[i] = combBank.getSaturatorAmount(i);
                }
                const float scale = 1.0f + pressDriveSum;
                for (int i = 0; i < 40; ++i)
                    combBank.setSaturatorAmount(i, std::min(1.0f, m_combSatLiveBase[i] * scale));
                m_pressWasActive = pressDriveSum > 0.0f;
            }

            const bool mainBankActive = combBankEnabled && combBank.hasEnabledStages();
            float fL = 0.0f, fR = 0.0f;
            if (mainBankActive) {
                combBank.process(busL, busR, fL, fR);
                finalL = (sL * stickDirect) + (fL * stickBus);
                finalR = (sR * stickDirect) + (fR * stickBus);
            } else {
                // No audible group / bank off: the COMB bank is a true bypass,
                // but the dispersion bus is upstream of it and must still
                // reach the output. busL/busR already hold the active Disp 1/2
                // mix (or the dry excitation when both stages are skipped), so
                // this does not duplicate the dry path.
                finalL = busL;
                finalR = busR;
            }
            if (finalCombBankEnabled) {
                finalCombBank.processStereo(finalL, finalR);
            }
            if (finalEQOn > 0.5f) {
                float dryFL = finalL, dryFR = finalR;
                for (int b = 0; b < 5; ++b) {
                    finalEQNodes[b].processStereoSample(finalL, finalR);
                }
                finalL = dryFL * (1.0f - feqMix) + finalL * feqMix;
                finalR = dryFR * (1.0f - feqMix) + finalR * feqMix;
            }
            
            // === Phase 1: Modular FX Chain with External Dry/Wet Policy ===
            // 
            // POLICY: Every FX slot receives FULL EXTERNAL AUDIO DRY/WET MIXING.
            // Internal wet parameters (if present) are PRESERVED and passed through;
            // external slot-level mixing is ALWAYS applied for consistent control.
            // 
            // Behavior by plugin type:
            // 1. NO INTERNAL WET (e.g., AMPSIM_CABS, UTILITY_AVERAGE):
            //    - Internal parameters have no wet/mix control
            //    - m_slotWet[slot] is the ONLY wet control
            //    - Output = dry * (1 - wet) + processed * wet
            // 
            // 2. HAS INTERNAL WET (e.g., REVERB_BRIGHTAMBIENCE, FILTER_AIR):
            //    - Internal wet parameter IS passed UNCHANGED in m_slotParams[slot]
            //    - Plugin can use internal wet for its own DSP mix
            //    - External slot-level mixing ALSO applied (both layers work together)
            //    - Output = dry * (1 - wet) + processed * wet
            // 
            // Result: Consistent slot-level control with respect for internal design.
            // Use hasInternalWet(type) to check if plugin has internal wet param.
            //
            // Pre-chain signal: the existing FX-tail ADSR envelopes only the
            // wet delta the chain adds, leaving the dry path untouched.
            preFxL = finalL; preFxR = finalR;
            for (int pos = 0; pos < activeFxSlotCount; ++pos) {
                const ActiveFxSlot activeSlot = activeFxSlots[pos];
                const int slot = activeSlot.slot;
                const float wet = m_slotWet[slot]; // per-sample: modulatable
                float dryL = finalL;
                float dryR = finalR;
                float outL = finalL;
                float outR = finalR;

                if (m_vendoredFx[slot] != nullptr) {
                    m_vendoredFx[slot]->process(outL, outR, m_slotParams[slot]);
                } else if (m_internalFx[slot] != nullptr) {
                    m_internalFx[slot]->process(outL, outR, m_slotParams[slot]);
                } else if (m_surgeFx[slot] != nullptr) {
                    const int paramCount = CymbalsSurgeFx::parameterCount(m_surgeFxAlgo[slot]);
                    for (int p = 0; p < paramCount && p < 13; ++p)
                        m_surgeFx[slot]->setParamNormalized(p, m_slotParams[slot][p]);
                    m_surgeFx[slot]->processSample(outL, outR, dryL, dryR);
                } else {
                    m_fxSlots[slot].process(outL, outR, m_slotParams[slot]);
                }

                // External dry/wet mixing (Phase 1 policy enforcement).
                // Surge returns a dry aligned to its 16-sample block latency.
                finalL = dryL * (1.0f - wet) + outL * wet;
                finalR = dryR * (1.0f - wet) + outR * wet;
            }

            // FX-tail ADSR: envelope only the wet delta the chain added, so
            // the dry/body path is untouched. Master ADSR is applied later,
            // after tilt and clip, to the complete mix.
            {
                float tailL = finalL - preFxL;
                float tailR = finalR - preFxR;
                if (finalAROn > 0.5f) {
                    const float env = finalOutEnv.process();
                    tailL *= env;
                    tailR *= env;
                }
                finalL = preFxL + tailL;
                finalR = preFxR + tailR;
            }
        }

        // Global post-FX tonal balance around a pivot frequency.
        finalTiltLow.process(finalL, finalR);
        finalTiltHigh.process(finalL, finalR);

        // Audio validation: catch NaN/infinite states.
        if (!std::isfinite(finalL) || !std::isfinite(finalR)) {
            finalL = 0.0f;
            finalR = 0.0f;
            fReverbProtectEnv = 0.0f;
            fReverbProtectSlowEnv = 0.0f;
            fReverbProtectGain = 1.0f;
            combBank.reset();
            finalCombBank.reset();
            dispersion.clear();
            dispersion2.clear();
            for (int band = 0; band < 5; ++band) {
                stickEQNodes[band].reset();
                finalEQNodes[band].reset();
            }
        } else {
            const float peak = std::max(std::abs(finalL), std::abs(finalR));
            const float avgAbs = 0.5f * (std::abs(finalL) + std::abs(finalR));

            fReverbProtectEnv = std::max(peak, fReverbProtectEnv * 0.9995f);
            fReverbProtectSlowEnv = std::max(avgAbs, fReverbProtectSlowEnv * 0.99995f);

            const float hardTrip = 18.0f;
            const float slowTrip = 9.0f;

            // A dense note cluster can push the pre-clip bus well past the
            // trip thresholds. Ducking it is a limiter; clearing the comb and
            // dispersion state was a hard DSP-flow reset (reported on fast
            // playing). Only a genuinely non-finite sample may reset state.
            float targetGain = 1.0f;
            if (fReverbProtectEnv > hardTrip || fReverbProtectSlowEnv > slowTrip) {
                // Hold the duck from the envelope, not just this sample: the
                // signal that tripped the trip may already be between
                // transients by the time the branch runs.
                const float hot = std::max(std::max(peak, avgAbs),
                                           std::max(fReverbProtectEnv, fReverbProtectSlowEnv));
                targetGain = std::clamp(1.0f / std::max(1.0e-6f, hot), 0.02f, 1.0f);
            }
            const float coef = (targetGain < fReverbProtectGain) ? protectAttack : protectRelease;
            fReverbProtectGain += (targetGain - fReverbProtectGain) * coef;
            finalL *= fReverbProtectGain;
            finalR *= fReverbProtectGain;

            // optional clipper/tweaks
            if (finalClipOn > 0.5f) {
                const float clipDryL = finalL, clipDryR = finalR;
                if (finalClipType < 0.5f) {
                    float drive = 1.0f + finalClipAmount * 2.0f;
                    finalL = applySoftClip(finalL * drive, fastMode) * 0.95f;
                    finalR = applySoftClip(finalR * drive, fastMode) * 0.95f;
                } else {
                    float thr = 0.5f + finalClipAmount * 0.5f;
                    auto bleed = [&](float x) {
                        if (x > thr) return thr + (x - thr) * 0.1f;
                        if (x < -thr) return -thr + (x + thr) * 0.1f;
                        return x;
                    };
                    finalL = bleed(finalL);
                    finalR = bleed(finalR);
                }
                const float mix = std::clamp(finalClipMix, 0.0f, 1.0f);
                finalL = (clipDryL * (1.0f - mix) + finalL * mix) * finalClipGain;
                finalR = (clipDryR * (1.0f - mix) + finalR * mix) * finalClipGain;
            } else {
                // Gentle soft clip as the final safety net.
                finalL = applySoftClip(finalL * 1.1f, fastMode) * 0.95f;
                finalR = applySoftClip(finalR * 1.1f, fastMode) * 0.95f;
            }
        }

        if (masterAROn > 0.5f) {
            const float env = masterOutEnv.process();
            finalL *= env;
            finalR *= env;
        }

        // FINAL TOUCHES HP/LP are last: they shape the ENTIRE output (dry body,
        // FX chain, clip and master envelope), not only the FX wet path.
        finalTouchHp.process(finalL, finalR);
        finalTouchLp.process(finalL, finalR);

        if (fAnalyzer) fAnalyzer->process(finalL, finalR);
        if (volume != fCachedVolumeParam) {
            fCachedLinVol = std::pow(10.0f, (volume - 1.0f) * 60.0f / 20.0f);
            fCachedVolumeParam = volume;
        }
        outL[i] = finalL * fCachedLinVol; outR[i] = finalR * fCachedLinVol;
    }
    m_modEngine.publishUiState(fModSourceOutput, fModSourcePhase, &fModUiBlockCount, &fModUiTempoBpm);
    if (fAnalyzer) { for (int i = 0; i < FFTAnalyzer::kNumBands; ++i) fAnalyzerBandPeaks[i].store(fAnalyzer->getBand(i), std::memory_order_relaxed); }
}

void PluginCymbals::applyMasterTuning(float normalized) {
    const float tune = std::clamp(normalized, 0.0f, 1.0f);
    const float ratio = std::pow(2.0f, (tune - 0.5f) * 8.0f);
    // A live note latches its base frequency at note-on. Scale every sounding
    // voice (and the mono bank) by the ratio delta so host automation of
    // Master Tune transposes the note that is already playing. Per-voice MPE
    // bend is a separate multiplier and survives the retune.
    if (masterTuneRatio > 0.0f && ratio != masterTuneRatio) {
        const float delta = ratio / masterTuneRatio;
        lastNoteFreq *= delta;
        stickBank.setBaseFrequency(lastNoteFreq);
        for (int i = 0; i < kMaxVoices; ++i) {
            Voice& v = m_voices[i];
            if (!v.active) continue;
            v.freq *= delta;
            v.stick.setBaseFrequency(v.freq);
        }
    }
    masterTuning = tune;
    masterTuneRatio = ratio;
}

void PluginCymbals::handleMidiNote(uint8_t note) {
    lastNoteFreq = 440.0f * std::pow(2.0f, (note - 69) / 12.0f) * masterTuneRatio;
    // SOUND = NONE is an external-audio input path: a note must not re-strike
    // (clear) the shared dispersion lines or the audible input would click.
    if ((int)std::lround(stickType) != (int)StickSoundBank::SILENT) {
        dispersion.trigger();
        dispersion2.trigger();
        inharmBloomEnv.trigger();
    }
}

// ===== Polyphony pool (foundation wave) =====================================
// Refactor shape (per contract): the legacy release path (stickBank +
// finalOutEnv + mod engine) is UNCHANGED in mono mode. Per-note gating does
// NOT duplicate envelopes — each pool voice owns one StickSoundBank (its
// amp/pitch/MSEG state IS its gate) plus a per-voice phase pair of the
// inharm-bloom envelopes. Chokes are a plain gain fade on that voice only.

float PluginCymbals::releaseTimeMul(float relVelNorm) const {
    const float s = std::clamp(relVelSens, 0.0f, 1.0f);
    if (s <= 0.0f || relVelNorm < 0.0f) return 1.0f; // sens off or instant (0x90 vel 0)
    return 1.0f + s * (1.0f - std::clamp(relVelNorm, 0.0f, 1.0f)) * 3.0f;
}
void PluginCymbals::clearAllVoices() {
    for (int i = 0; i < kMaxVoices; ++i) {
        m_voices[i].active = false;
        m_voices[i].releasing = false;
        m_voices[i].relGain = 1.0f;
        m_voices[i].relStep = 0.0f;
        m_voices[i].bowSamplesLeft = 0;
    }
    clearVoiceExpression();
}

void PluginCymbals::clearVoiceExpression() {
    // MPE deactivate / panic: drop per-note bend, slide, and pressure so
    // nothing stale modulates a later hit on the same slot.
    for (int i = 0; i < kMaxVoices; ++i) {
        m_voices[i].bendSemis = 0.0f;
        m_voices[i].slideSemis = 0.0f;
        m_voices[i].pressure = 0.0f;
        m_voices[i].stick.setPitchBendSemitones(0.0f);
    }
    for (int ch = 0; ch < 16; ++ch) m_voiceChannelMap[ch] = -1;
    if (m_pressWasActive) {
        // (MED-4) Panic/deactivate mid-drive: restore the captured LIVE
        // saturator bases so the bus doesn't stay stuck at its last driven
        // scale until the next engage recaptures them.
        for (int i = 0; i < 40; ++i)
            combBank.setSaturatorAmount(i, m_combSatLiveBase[i]);
    }
    m_pressWasActive = false;
}

void PluginCymbals::applyVoicePitch(int8_t slot) {
    if (slot < 0 || slot >= kMaxVoices || !m_voices[slot].active) return;
    // Combined applied pitch = bend + slide, clamped to ±48 st so a runaway
    // MPE sender cannot pitch a voice out of a sensible range. Single write
    // point so bend and slide updates are order-independent.
    const float total = std::clamp(m_voices[slot].bendSemis + m_voices[slot].slideSemis,
                                   -48.0f, 48.0f);
    m_voices[slot].stick.setPitchBendSemitones(total);
}

void PluginCymbals::setVoiceSlideFromCc74(uint8_t channel, uint8_t value127) {
    // MPE Y-axis (CC74) per-voice handler. Out-of-line definition so the
    // per-voice slide write path is a single, greppable mangled symbol
    // (test_midi_wired.py asserts its presence in the shipped binaries).
    //
    // Spec: under MPE (manual switch or RPN [0,6] activation), CC74 on a
    // per-note manager channel offsets the LATEST triggered voice on that
    // channel. Bipolar 0..127 -> -1..+1 -> ±24 st full-scale (matches bend
    // range so a slide+press combo adds cleanly), folded into the stick
    // pitch via setPitchBendSemitones. The m_modEngine SLIDE atomic is
    // also updated so any mod-matrix route on SLIDE sees the latest value.
    if (!mpeEffective()) return;
    const int8_t slot = m_voiceChannelMap[channel & 0x0F];
    if (slot < 0 || !m_voices[slot].active) return;
    // Bipolar: 0 -> -24 st, 64 -> 0 st, 127 -> +24 st.
    const float norm = (float)value127 / 127.0f;        // 0..1
    const float slideSemis = (norm - 0.5f) * 48.0f;     // ±24 st
    m_voices[slot].slideSemis = std::clamp(slideSemis, -24.0f, 24.0f);
    // Fold slide into the existing bend offset via the shared helper (clamps
    // the total so a runaway sender can't pitch the voice out of sight).
    applyVoicePitch(slot);
    // Publish the latest SLIDE atomic so a mod-matrix route on SLIDE can
    // see it. Bipolar [-1, +1] (matches PITCHBEND's perf-input convention).
    m_modEngine.setPerformanceInput(ModulationEngine::SLIDE, (norm - 0.5f) * 2.0f);
}

void PluginCymbals::applyRpnDataEntry(uint16_t value14) {
    // RPN [0,6] = MPE mode message. Non-zero value activates, zero deactivates.
    // Per spec this FORCES effective MPE on; kParamMpeMode stays the manual
    // global switch and getMpeModeEnabled() reports manual || rpnActive.
    if (value14 > 0) {
        rpnMpeActive = true;
    } else if (rpnMpeActive) {
        rpnMpeActive = false;
        clearVoiceExpression(); // deactivate drops per-note bend/pressure
    }
}

void PluginCymbals::chokeSameGroupVoices(uint8_t newNote, const Voice* exclude) {
    const int mode = (int)std::clamp(std::round(chokeMode), 0.0f, 2.0f);
    if (mode <= 0) return;
    const int split = (int)std::clamp(std::round(chokeSplitPoint), 0.0f, 127.0f);
    const bool newIsGroupA = (newNote < split);
    const float step = 1.0f / std::max(1.0f, 0.005f * fSampleRate); // ~5 ms choke fade
    for (int i = 0; i < kMaxVoices; ++i) {
        Voice& v = m_voices[i];
        if (!v.active || v.releasing || &v == exclude) continue;
        if (mode == 1 || ((v.note < split) == newIsGroupA)) {
            v.releasing = true;
            v.relStep = step;
        }
    }
}

void PluginCymbals::triggerPolyVoice(uint8_t note, float velocity, uint8_t channel) {
    const int cap = (int)std::clamp(std::round(voiceCount), 2.0f, (float)kMaxVoices);
    Voice* chosen = nullptr;      // first free slot below the cap
    Voice* oldest = nullptr;      // lowest age stamp among active (steal target)
    Voice* sameNote = nullptr;    // oldest active non-releasing voice on THIS note
    int activeCount = 0;
    for (int i = 0; i < cap; ++i) {
        Voice& v = m_voices[i];
        if (v.active) {
            ++activeCount;
            // (LOW-8) Wrap-aware age order: stamps are monotonic uint32 and
            // stay correctly ordered across the 2^32 wrap as signed distance.
            if (oldest == nullptr || (int32_t)(v.age - oldest->age) < 0) oldest = &v;
            if (v.note == note && !v.releasing &&
                (sameNote == nullptr || (int32_t)(v.age - sameNote->age) < 0))
                sameNote = &v;
        } else if (chosen == nullptr) {
            chosen = &v;
        }
    }
    // (MED-5) Selection order: a free slot first; when the pool is full, a
    // duplicate note-on RESTRIKES its own oldest voice instead of stealing a
    // different note's voice (which stacked two hits of this note and left
    // the real victim's owner unable to release it); otherwise oldest-voice-
    // first stealing.
    if (chosen == nullptr) chosen = (sameNote != nullptr) ? sameNote : oldest;

    const bool wasFirst = (activeCount == 0);
    const float freq = 440.0f * std::pow(2.0f, (note - 69) / 12.0f) * masterTuneRatio;

    chosen->active = true;
    chosen->releasing = false;
    chosen->relGain = 1.0f;
    chosen->relStep = 0.0f;
    chosen->note = note;
    chosen->channel = channel;
    chosen->freq = freq;
    chosen->velocity = velocity;
    chosen->bowSamplesLeft = 0;
    // Fresh hit starts unbent/un-slid/un-pressurized (MPE senders re-send
    // per note; stale slide from a previous note on the same slot would
    // otherwise yank the new hit's pitch off its stick frequency).
    chosen->bendSemis = 0.0f;
    chosen->slideSemis = 0.0f;
    chosen->pressure = 0.0f;
    chosen->stick.setPitchBendSemitones(0.0f);
    const int8_t chosenSlot = (int8_t)(chosen - m_voices);
    // (HIGH-1) The chosen slot may have been the latest voice of ANOTHER
    // channel (steal or restrike reuse): retire that stale mapping so
    // per-note expression never lands on a hijacked slot.
    for (int ch = 0; ch < 16; ++ch) {
        if (m_voiceChannelMap[ch] == chosenSlot && ch != (channel & 0x0F))
            m_voiceChannelMap[ch] = -1;
    }
    m_voiceChannelMap[channel & 0x0F] = chosenSlot;
    chosen->age = ++m_voiceAgeCounter;

    const bool silentStick = ((int)std::lround(stickType) == (int)StickSoundBank::SILENT);
    lastNoteFreq = freq;
    chosen->stick.triggerNote(freq, velocity);
    if (!silentStick) {
        chosen->bloomEnv.trigger();
        chosen->bloom2Env.trigger();
    }

    // Shared restrike of the bus-level resonators (deviation note: dispersion
    // stages stay global for CPU reasons; their ENVELOPE phases live per voice
    // via bloomEnv/bloom2Env). With SOUND = NONE the stick is silent, so a
    // note must not clear the dispersion lines feeding the external input.
    if (!silentStick) {
        dispersion.trigger();
        dispersion2.trigger();
    }
    // Mod engine broadcast stays global: noteOn on first voice on.
    if (wasFirst) m_modEngine.noteOn();

    // Choke after allocation so the new hit never chokes itself.
    chokeSameGroupVoices(note, chosen);
}

void PluginCymbals::releasePolyVoice(uint8_t note, float relMul, bool instant) {
    Voice* match = nullptr;
    for (int i = 0; i < kMaxVoices; ++i) {
        Voice& v = m_voices[i];
        if (!v.active || v.note != note || v.releasing) continue;
        if (match == nullptr || (int32_t)(v.age - match->age) < 0) match = &v; // oldest matching (LOW-8)
    }
    if (match == nullptr) return;
    match->stick.setAmpReleaseScale(instant ? 1.0f : relMul);
    match->stick.releaseNote(); // per-note gating: ONLY this voice's tail ends
}

void PluginCymbals::processVoicePool(float& sL, float& sR, float& bloomRawSum, float& bloom2RawSum,
                                     float& pressDriveSum, uint32_t bowInterval) {
    const bool bowOn = (bowEnable > 0.5f) && (bowLevel > 0.0f) && bowInterval > 0;
    const float pressAmount = std::clamp(mpePressDrive, 0.0f, 1.0f);
    for (int i = 0; i < kMaxVoices; ++i) {
        Voice& v = m_voices[i];
        if (!v.active) continue;
        if (v.releasing && v.relStep > 0.0f) {
            v.relGain = std::max(0.0f, v.relGain - v.relStep);
            if (v.relGain <= 0.0f) { v.active = false; continue; } // choke fully faded
        }
        // Bow/swell (#8): sustained re-excitation of HELD keys at the bowed
        // rate. trigger() restarts the amp attack FROM THE CURRENT LEVEL
        // (AdsrEnvelope::trigger keeps currentLevel when not Idle), so a
        // mid-tail re-strike cannot click to zero.
        if (bowOn && !v.releasing && fActiveNotes[v.note]) {
            if (v.bowSamplesLeft == 0) {
                v.stick.triggerNote(v.freq, std::clamp(v.velocity * bowLevel, 0.0f, 1.0f));
                v.bowSamplesLeft = bowInterval;
            } else {
                --v.bowSamplesLeft;
            }
        }
        // Pressure -> stress drive aggregate (#7): shared comb bus, so voices
        // contribute through one summed drive scale (see run()).
        if (pressAmount > 0.0f)
            pressDriveSum += v.pressure * pressAmount * v.relGain;
        float l = 0.0f, r = 0.0f;
        v.stick.process(l, r);
        l *= v.relGain; r *= v.relGain;
        sL += l; sR += r;
        bloomRawSum += v.bloomEnv.process();
        bloom2RawSum += v.bloom2Env.process();
        // Natural retirement once this voice is fully silent.
        if (!v.stick.isActive() && !v.bloomEnv.isActive() && !v.bloom2Env.isActive())
            v.active = false;
    }
    pressDriveSum = std::min(pressDriveSum, 1.0f);
}

void PluginCymbals::syncVoiceStickParams() {
    // (MED-3) Reuse the reserved scratch vector: zero heap allocations per
    // call after ctor warmup (this runs per stick/MSEG param change).
    std::vector<MsegPoint>& points = fMsegScratch;
    points.clear();
    points.push_back({0.0f, msegPitchValues[0], (msegPitchCurves[0] - 0.5f) * 2.0f});
    points.push_back({msegPitchTimes[0], msegPitchValues[1], (msegPitchCurves[1] - 0.5f) * 2.0f});
    points.push_back({msegPitchTimes[1], msegPitchValues[2], (msegPitchCurves[2] - 0.5f) * 2.0f});
    points.push_back({msegPitchTimes[2], msegPitchValues[3], (msegPitchCurves[3] - 0.5f) * 2.0f});
    points.push_back({msegPitchTimes[3], msegPitchValues[4], (msegPitchCurves[4] - 0.5f) * 2.0f});
    points.push_back({msegPitchTimes[4], msegPitchValues[5], (msegPitchCurves[5] - 0.5f) * 2.0f});
    points.push_back({msegPitchTimes[5], msegPitchValues[6], (msegPitchCurves[6] - 0.5f) * 2.0f});
    points.push_back({1.0f, msegPitchValues[7], 0.0f});
    const bool useMseg = (msegPitchOn > 0.5f);
    const float dur = 0.001f + std::pow(msegPitchDuration, 2.0f) * 1.999f;
    for (int i = 0; i < kMaxVoices; ++i) {
        StickSoundBank& bank = m_voices[i].stick;
        bank.setCurrentStick((StickSoundBank::StickType)(int)stickType);
        bank.setAmpAttack(stickAmpAttack);
        bank.setAmpDecay(stickAmpDecay);
        bank.setAmpSustain(stickAmpSustain);
        bank.setAmpRelease(stickAmpRelease);
        bank.setAmpDecayCurve(stickAmpDecayCurve);
        bank.setAmpReleaseCurve(stickAmpReleaseCurve);
        bank.setPitchAttack(stickPitchAttack);
        bank.setPitchDecay(stickPitchDecay);
        bank.setPitchSustain(stickPitchSustain);
        bank.setPitchRelease(stickPitchRelease);
        bank.setPitchDecayCurve(stickPitchDecayCurve);
        bank.setPitchReleaseCurve(stickPitchReleaseCurve);
        bank.setPitchAmount(stickPitchAmount * 48.0f);
        bank.setPitchTimer(stickPitchTimer * 0.1f);
        bank.setPulseWidth(stickPulseWidth);
        bank.setNoiseColor(stickNoiseColor);
        bank.setMetallicSpread(stickMetallicSpread);
        bank.setNoiseMetalBlend(stickNoiseMetalBlend);
        bank.setHatTone(stickHatTone);
        bank.setSquareWidth(stickSquareWidth);
        bank.setSawCurve(stickSawCurve);
        bank.setTriangleSkew(stickTriangleSkew);
        bank.setSineDrive(stickSineDrive);
        bank.setPhaseOffset(stickPhase);
        bank.setGrit(stickGrit);
        bank.setUnison((int)stickUnison + 1);
        bank.setDetune(stickDetune);
        bank.setPitchMsegPoints(points);
        bank.setPitchMsegDuration(dur);
        bank.setUseMsegPitch(useMseg);
    }
}

void PluginCymbals::syncVoiceBloomEnvs() {
    const float d1 = DSPUtils::inharmBloomDecayFromControl(inharmBloomTime);
    const float d2 = DSPUtils::inharmBloomDecayFromControl(inharm2BloomTime);
    for (int i = 0; i < kMaxVoices; ++i) {
        m_voices[i].bloomEnv.setDecay(d1);
        m_voices[i].bloom2Env.setDecay(d2);
    }
}

void PluginCymbals::applyVoiceBloomTime(bool second, float decaySec) {
    // Poly mode reads the per-voice bloom envelopes, so a mod route on the
    // bloom time must reach them, not just the shared mono envelope.
    for (int i = 0; i < kMaxVoices; ++i) {
        if (!m_voices[i].active) continue;
        (second ? m_voices[i].bloom2Env : m_voices[i].bloomEnv).setDecay(decaySec);
    }
}
void PluginCymbals::initProgramName(uint32_t index, String& name) {
    // Program 0 is always INIT (the plugin's default parameter state), matching
    // the state a fresh instance opens with. Preset files start at program 1;
    // the remaining fixed slots keep the legacy names when the listing is short.
    if (index == 0) {
        name = "Init";
        return;
    }
    const std::vector<PresetFileInfo> files = listPresetFiles();
    const size_t fileIndex = (size_t)index - 1;
    if (fileIndex < files.size()) {
        name = files[fileIndex].name.c_str();
        return;
    }
    switch (index) {
    case 1: name = "Ride"; break;
    case 2: name = "Crash"; break;
    case 3: name = "Hi-Hat"; break;
    case 4: name = "Preset 4"; break;
    default: name.clear(); break;
    }
}

void PluginCymbals::loadProgram(uint32_t index) {
    // Program 0 is INIT: every parameter back to its initParameter default and
    // a clean preset identity. This is the same state a fresh instance opens
    // with, so the host's default program no longer loads the first file on
    // disk. Preset files start at program 1.
    if (index == 0) {
        std::vector<float> defaults(kParameterCount, 0.0f);
        for (uint32_t i = 0; i < kParameterCount; ++i) {
            Parameter p;
            initParameter(i, p);
            defaults[i] = p.ranges.def;
        }
        applyPresetValuesSafely(defaults);
        resetPresetIdentity();
        setPresetMetadata("INIT", "", "", "");
        fLastPresetError.clear();
        updateStateValue("preset_loaded_name", "INIT");
        updateStateValue("preset_loaded_description", "");
        updateStateValue("preset_loaded_author", "");
        updateStateValue("preset_loaded_tags", "");
        updateStateValue("preset_loaded_usage", "");
        return;
    }
    // Deterministic name-sorted preset selection. DPF invokes this on host/main
    // threads only (never in process()), so the filesystem access here is safe;
    // the actual value application is staged via
    // fPendingPresetValues/fPendingPresetReady and applied by run().
    const std::vector<PresetFileInfo> files = listPresetFiles();
    const size_t fileIndex = (size_t)index - 1;
    if (fileIndex >= files.size()) return;   // out of range: no-op
    std::string err;
    if (!loadPresetFromPath(files[fileIndex].path, err)) {
        fLastPresetError = err;
    }
}

std::vector<float> PluginCymbals::captureAllParameterValues() const {
    // Non-RT path (state save, IR export snapshot): serialize against other
    // non-RT writers. RT parameter writes are plain float stores without this
    // mutex, so a torn read from automation mid-save stays tolerated — this
    // includes batch-level mixing if a preset commit is mid-apply during a
    // save; per-parameter values themselves are always individually valid.
    std::lock_guard<std::recursive_mutex> parameterGuard(fParameterSnapshotMutex);
    std::vector<float> values;
    values.reserve(kParameterCount);
    for (uint32_t i = 0; i < kParameterCount; ++i) {
        values.push_back(getParameterValue(i));
    }
    return values;
}

void PluginCymbals::applyAllParameterValues(const std::vector<float>& values) {
    const uint32_t count = std::min<uint32_t>(kParameterCount, (uint32_t)values.size());
    for (uint32_t i = 0; i < count; ++i) {
        setParameterValue(i, values[i]);
    }
}

void PluginCymbals::applyPresetValuesSafely(const std::vector<float>& values) {
    std::vector<float> safeValues;
    safeValues.resize(kParameterCount, 0.0f);

    for (uint32_t i = 0; i < kParameterCount; ++i) {
        Parameter p;
        initParameter(i, p);
        const float def = p.ranges.def;

        float v = (i < values.size()) ? values[i] : def;
        if (!std::isfinite(v)) v = def;
        v = std::clamp(v, p.ranges.min, p.ranges.max);
        if ((p.hints & kParameterIsInteger) != 0) v = std::round(v);

        safeValues[i] = v;
    }

    {
        std::lock_guard<std::mutex> guard(fPendingPresetMutex);
        // Non-RT assign: allocates the first time (capacity growth), then
        // reuses the slot. The generation bump guarantees a preset staged
        // while run() is delivering an earlier one will not be lost.
        fPendingPresetValues = safeValues;
        ++fPresetStagedGen;
    }

    // Serialize + publish the JSON here (non-RT) BEFORE flagging the commit:
    // run()'s commit block must only loop raw floats into setParameterValue,
    // never build strings.
    {
        std::lock_guard<std::mutex> guard(fLastPresetJsonMutex);
        fLastPresetJson = serializePresetJsonFromValues(safeValues);
    }
    fPendingPresetReady.store(true, std::memory_order_release);
}

std::string PluginCymbals::getPresetDirectory() const {
    return PresetPolicy::resolvePresetDirectory();
}

std::vector<PluginCymbals::PresetFileInfo> PluginCymbals::listPresetFiles() const {
    std::vector<PresetFileInfo> out;
    try {
        const std::filesystem::path dir(getPresetDirectory());
        // ensure directory exists so the browser always shows something
        std::filesystem::create_directories(dir);
        if (!std::filesystem::exists(dir)) {
            return out;
        }
        for (const auto& entry : std::filesystem::recursive_directory_iterator(
                 dir, std::filesystem::directory_options::skip_permission_denied)) {
            if (!entry.is_regular_file()) continue;
            if (entry.path().extension() != ".json") continue;
            if (entry.path().filename() == "favourites.json") continue;

            std::ifstream file(entry.path(), std::ios::binary);
            if (!file.is_open()) continue;
            // Skip oversized files instead of buffering them (see load path limit).
            file.seekg(0, std::ios::end);
            const std::streampos fileSize = file.tellg();
            file.seekg(0, std::ios::beg);
            static const std::streampos kMaxPresetFileBytes = (std::streampos)(1024 * 1024);
            if (fileSize > kMaxPresetFileBytes) continue;
            std::ostringstream buffer;
            buffer << file.rdbuf();
            const std::string json = buffer.str();

            PresetFileInfo info;
            info.path = entry.path().string();
            std::string relFolder = std::filesystem::relative(entry.path(), dir).parent_path().u8string();
            std::replace(relFolder.begin(), relFolder.end(), '\\', '/');
            info.folder = relFolder;
            if (!parseJsonStringField(json, "name", info.name) || info.name.empty()) {
                info.name = entry.path().stem().string();
            }
            parseJsonStringField(json, "description", info.description);
            parseJsonStringField(json, "author", info.author);
            parseJsonStringArrayField(json, "tags", info.tags);
            parseJsonStringField(json, "usage", info.usage);
            out.push_back(info);
        }

        std::sort(out.begin(), out.end(), [](const PresetFileInfo& a, const PresetFileInfo& b) {
            return a.name < b.name;
        });
    } catch (...) {
        // Keep browser robust even if a single directory operation fails.
    }
    return out;
}

std::string PluginCymbals::resolveCanonicalPresetSavePathForUi(const std::string& requestedPath) const {
    const std::string canonicalPath = PresetPolicy::canonicalizePresetSavePath(
        requestedPath,
        fPresetName,
        getPresetDirectory());
    std::filesystem::path resolvedPath = std::filesystem::u8path(canonicalPath);

    if (!fLastPresetPath.empty()) {
        const std::filesystem::path lastPath = std::filesystem::u8path(fLastPresetPath);
        const std::string resolvedStem = resolvedPath.stem().string();
        const std::string lastStem = lastPath.stem().string();
        if (PresetPolicy::sanitizePresetFileStem(lastStem) == resolvedStem) {
            resolvedPath = lastPath;
        }
    }

    return resolvedPath.u8string();
}

bool PluginCymbals::loadPresetFromUiIntent(const std::string& path, std::string& outError) {
    outError.clear();
    if (!loadPresetFromPath(path, outError))
        return false;
    // Publish the same result the host-mediated path would. The UI also pushes
    // these into the browser directly, so this is belt-and-braces for hosts
    // that echo UI state back into stateChanged.
    updateStateValue("preset_status", ("Loaded: " + path).c_str());
    updateStateValue("preset_loaded_name", fPresetName.c_str());
    updateStateValue("preset_loaded_description", fPresetDescription.c_str());
    updateStateValue("preset_loaded_author", fPresetAuthor.c_str());
    updateStateValue("preset_loaded_tags", fPresetTagsCsv.c_str());
    updateStateValue("preset_loaded_usage", fPresetUsage.c_str());
    return true;
}

bool PluginCymbals::savePresetFromUiIntent(const std::string& requestedPath, std::string& outError) {
    outError.clear();
    const std::string resolvedPath = resolveCanonicalPresetSavePathForUi(requestedPath);
    if (!savePresetToPath(resolvedPath, outError)) {
        fLastPresetError = outError;
        return false;
    }

    fLastPresetError.clear();
    return true;
}

std::string PluginCymbals::serializePresetJsonFromValues(const std::vector<float>& values) const {
    const std::vector<std::string> tags = splitTagsCsv(fPresetTagsCsv);
    std::string presetName = fPresetName;
    if (presetName.empty()) {
        if (!fLastPresetPath.empty()) presetName = std::filesystem::path(fLastPresetPath).stem().string();
        if (presetName.empty()) presetName = "Untitled Preset";
    }

    std::ostringstream os;
    os.imbue(std::locale::classic());
    os.setf(std::ios::fixed);
    os.precision(9);
    os << std::setfill(' ');
    os << "{\n";
    os << "  \"format\": \"cymbals-preset-v4\",\n";
    os << "  \"name\": \"" << jsonEscape(presetName) << "\",\n";
    os << "  \"description\": \"" << jsonEscape(fPresetDescription) << "\",\n";
    os << "  \"author\": \"" << jsonEscape(fPresetAuthor) << "\",\n";
    os << "  \"usage\": \"" << jsonEscape(fPresetUsage) << "\",\n";
    os << "  \"tags\": [";
    for (size_t i = 0; i < tags.size(); ++i) {
        if (i > 0) os << ", ";
        os << "\"" << jsonEscape(tags[i]) << "\"";
    }
    os << "],\n";
    os << "  \"parameterCount\": " << values.size() << ",\n";
    os << "  \"parameters\": {\n";

    constexpr int kTabCount = (int)(sizeof(kPresetParamTabs) / sizeof(kPresetParamTabs[0]));
    std::vector<std::vector<std::pair<std::string, float>>> buckets;
    buckets.resize(kTabCount);

    for (uint32_t i = 0; i < values.size(); ++i) {
        Parameter p;
        const_cast<PluginCymbals*>(this)->initParameter(i, p);
        std::string key = p.symbol.buffer();
        if (key.empty()) {
            char fallback[24];
            std::snprintf(fallback, sizeof(fallback), "param_%u", i);
            key = fallback;
        }
        const int tb = tabBucketIndex(presetTabForParameterIndex(i));
        buckets[tb].push_back(std::make_pair(key, values[i]));
    }

    bool firstTabOut = true;
    for (int t = 0; t < kTabCount; ++t) {
        if (buckets[t].empty()) continue;
        if (!firstTabOut) os << ",\n";
        firstTabOut = false;

        os << "    \"" << jsonEscape(kPresetParamTabs[t]) << "\": {\n";
        for (size_t i = 0; i < buckets[t].size(); ++i) {
            os << "      \"" << jsonEscape(buckets[t][i].first) << "\": " << buckets[t][i].second;
            if (i + 1 < buckets[t].size()) os << ",";
            os << "\n";
        }
        os << "    }";
    }

    os << "\n  }\n";
    os << "}\n";
    return os.str();
}

std::string PluginCymbals::serializePresetJson() const {
    return serializePresetJsonFromValues(captureAllParameterValues());
}

bool PluginCymbals::deserializePresetJson(const std::string& json, std::vector<float>& outValues, std::string& outError) {
    outValues.clear();
    outError.clear();

    const size_t pkey = json.find("\"parameters\"");
    if (pkey == std::string::npos) {
        outError = "Missing 'parameters' field";
        return false;
    }

    size_t spanStart = std::string::npos;
    size_t spanEnd = std::string::npos;
    if (!extractJsonValueSpan(json, "parameters", spanStart, spanEnd) || spanStart == std::string::npos || spanEnd == std::string::npos) {
        outError = "Invalid 'parameters' container";
        return false;
    }

    const std::string container = json.substr(spanStart, spanEnd - spanStart + 1);
    if (!container.empty() && container[0] == '[') {
        // Legacy v1: positional array
        const std::string arr = container.substr(1, container.size() - 2);
        const char* p = arr.c_str();
        char* end = nullptr;
        while (*p != '\0') {
            // skip anything that isn't part of a number
            while (*p != '\0' && !(std::isdigit((unsigned char)*p) || *p == '-' || *p == '+' || *p == '.'))
                ++p;
            if (*p == '\0') break;
            const double v = std::strtod(p, &end);
            if (end == p) {
                outError = "Failed to parse numeric value in 'parameters' array";
                return false;
            }
            if (!std::isfinite(v)) {
                outError = "Non-finite numeric value in 'parameters' array";
                return false;
            }
            outValues.push_back((float)v);
            p = end;
        }

        if (outValues.empty()) {
            outError = "No parameter values found in preset";
            return false;
        }
        if (outValues.size() > kParameterCount) {
            outValues.resize(kParameterCount);
        }

        if (outValues.size() < kParameterCount) {
            std::vector<float> canonicalDefaults(kParameterCount, 0.0f);
            for (uint32_t i = 0; i < kParameterCount; ++i) {
                Parameter p;
                initParameter(i, p);
                float def = p.ranges.def;
                if (!std::isfinite(def)) def = 0.0f;
                canonicalDefaults[i] = def;
            }
            outValues = remapLegacyPositionalValues(outValues, canonicalDefaults);
        }
    } else if (!container.empty() && container[0] == '{') {
        // v2/v3: named object (flat or grouped by tab).
        outValues.resize(kParameterCount, 0.0f);
        std::unordered_map<std::string, uint32_t> keyToIndex;
        keyToIndex.reserve(kParameterCount * 2);

        for (uint32_t i = 0; i < kParameterCount; ++i) {
            Parameter p;
            initParameter(i, p);
            float def = p.ranges.def;
            if (!std::isfinite(def)) def = 0.0f;
            outValues[i] = def;

            const std::string symbol = p.symbol.buffer();
            const std::string name = p.name.buffer();
            if (!symbol.empty()) keyToIndex[symbol] = i;
            if (!name.empty()) keyToIndex[name] = i;
        }

        std::unordered_map<std::string, float> parsed;
        std::string parseErr;
        if (!parseJsonStringNumberObject(container, parsed, parseErr)) {
            // If flat-object parsing fails, try grouped-by-tab object parsing.
            if (!parseGroupedParametersObject(container, parsed, outError)) {
                return false;
            }
        }

        // Legacy EQ type migration: pre-UnifiedFilter builds stored seq%d_type /
        // feq%d_type with 0=Bell..5=AllPass (3 bands). The UnifiedFilter refactor
        // kept the same symbols but redefined the enum (0=LowPass..7=AllPass, 5
        // bands). Legacy presets lack the new eq%d_mdl keys; translate their type
        // values BEFORE filling outValues so saved EQ tonality is preserved
        // instead of silently remapped.
        if (parsed.find("eq1_mdl") == parsed.end()) {
            static const float kLegacyEqTypeMap[6] = { 4.0f, 5.0f, 6.0f, 3.0f, 1.0f, 7.0f };
            static const char* kLegacyEqTypeKeys[] = {
                "seq1_type", "seq2_type", "seq3_type",
                "feq1_type", "feq2_type", "feq3_type"
            };
            for (const char* key : kLegacyEqTypeKeys) {
                const auto it = parsed.find(key);
                if (it == parsed.end()) continue;
                const int oldType = (int)std::lround(it->second);
                if (oldType >= 0 && oldType <= 5) it->second = kLegacyEqTypeMap[oldType];
            }
        }

        for (const auto& kv : parsed) {
            const auto it = keyToIndex.find(kv.first);
            if (it == keyToIndex.end()) {
                // Ignore unknown params to remain forward/backward compatible.
                continue;
            }
            outValues[it->second] = kv.second;
        }
    } else {
        outError = "Unsupported 'parameters' type";
        return false;
    }

    std::string loadedName;
    std::string loadedDescription;
    std::string loadedAuthor;
    std::vector<std::string> loadedTags;
    std::string loadedUsage;
    const bool hasName = parseJsonStringField(json, "name", loadedName);
    const bool hasDescription = parseJsonStringField(json, "description", loadedDescription);
    const bool hasAuthor = parseJsonStringField(json, "author", loadedAuthor);
    const bool hasTags = parseJsonStringArrayField(json, "tags", loadedTags);
    const bool hasUsage = parseJsonStringField(json, "usage", loadedUsage);

    PluginCymbals* self = const_cast<PluginCymbals*>(this);
    self->fPresetName = hasName ? loadedName : std::string();
    self->fPresetDescription = hasDescription ? loadedDescription : std::string();
    self->fPresetAuthor = hasAuthor ? loadedAuthor : std::string();
    self->fPresetTagsCsv = hasTags ? joinTagsCsv(loadedTags) : std::string();
    self->fPresetUsage = hasUsage ? loadedUsage : std::string();

    return true;
}

bool PluginCymbals::savePresetToPath(const std::string& path, std::string& outError) {
    outError.clear();
    try {
        const std::filesystem::path p = std::filesystem::u8path(path);
        if (p.has_parent_path()) {
            std::filesystem::create_directories(p.parent_path());
        }
        const std::string json = serializePresetJson();
        std::ofstream file(p, std::ios::binary | std::ios::trunc);
        if (!file.is_open()) {
            outError = "Failed to open preset file for writing";
            return false;
        }
        file.write(json.c_str(), (std::streamsize)json.size());
        if (!file.good()) {
            outError = "Failed while writing preset file";
            return false;
        }
        fLastPresetPath = path;
        {
            std::lock_guard<std::mutex> guard(fLastPresetJsonMutex);
            fLastPresetJson = json;
        }
        return true;
    } catch (const std::exception& e) {
        outError = e.what();
        return false;
    } catch (...) {
        outError = "Unknown error while saving preset";
        return false;
    }
}

bool PluginCymbals::loadPresetFromPath(const std::string& path, std::string& outError) {
    outError.clear();
    try {
        const std::filesystem::path p = std::filesystem::u8path(path);
        std::ifstream file(p, std::ios::binary);
        if (!file.is_open()) {
            outError = "Failed to open preset file for reading";
            return false;
        }
        // Bound the file size: state payloads are capped at 1 MiB on the setState
        // path, so a large/corrupt file on disk must not exhaust memory here.
        file.seekg(0, std::ios::end);
        const std::streampos fileSize = file.tellg();
        file.seekg(0, std::ios::beg);
        static const std::streampos kMaxPresetFileBytes = (std::streampos)(1024 * 1024);
        if (fileSize > kMaxPresetFileBytes) {
            outError = "Preset file exceeds 1 MiB size limit";
            return false;
        }
        std::ostringstream buffer;
        buffer << file.rdbuf();
        const std::string json = buffer.str();

        std::vector<float> values;
        if (!deserializePresetJson(json, values, outError)) {
            return false;
        }

        applyPresetValuesSafely(values);
        fLastPresetPath = path;
        return true;
    } catch (const std::exception& e) {
        outError = e.what();
        return false;
    } catch (...) {
        outError = "Unknown error while loading preset";
        return false;
    }
}

PluginCymbals::IrExportRenderState::IrExportRenderState(float sampleRate)
    : combBank(sampleRate),
      finalCombBank(sampleRate),
      dispersion(sampleRate),
      dispersion2(sampleRate) {
    combBank.setSampleRate(sampleRate);
    finalCombBank.setSampleRate(sampleRate);
    dispersion.setSampleRate(sampleRate);
    dispersion2.setSampleRate(sampleRate);
    modEngine.setSampleRate(sampleRate);
    for (UnifiedFilter& node : finalEQNodes) {
        node.reset();
    }
    for (UnifiedFilter& node : stickEQNodes) {
        node.reset();
    }
    finalTiltLow.clear();
    finalTiltHigh.clear();
}

std::vector<float> PluginCymbals::copyIrExportParameterValues() const {
    std::lock_guard<std::recursive_mutex> parameterGuard(fParameterSnapshotMutex);

    if (fPendingPresetReady.load(std::memory_order_acquire)) {
        std::lock_guard<std::mutex> pendingGuard(fPendingPresetMutex);
        if (fPendingPresetReady.load(std::memory_order_acquire) && fPendingPresetValues.size() == kParameterCount) {
            return fPendingPresetValues;
        }
    }

    return captureAllParameterValues();
}

PluginCymbals::IrExportInputSnapshot PluginCymbals::createIrExportInputSnapshot(const std::vector<float>& parameterValues, float durationSeconds) const {
    IrExportInputSnapshot snapshot{};
    const auto param = [&parameterValues](uint32_t index) {
        return (index < parameterValues.size()) ? parameterValues[index] : 0.0f;
    };

    snapshot.sampleRate = fSampleRate;
    snapshot.durationSeconds = std::clamp(durationSeconds, 0.5f, 5.0f);
    snapshot.volumeParam = param(kParamVolume);
    snapshot.disp1Skip = param(kParamDisp1Skip);
    snapshot.disp2Skip = param(kParamDisp2Skip);
    snapshot.dispCrossLink = param(kParamDispCrossLink);
    snapshot.dispSolo = param(kParamDispSolo);
    snapshot.stickDirect = param(kParamStickDirect);
    snapshot.stickBus = param(kParamStickBus);
    snapshot.feqMix = param(kParamFinalEQMix);
    snapshot.finalEQOn = param(kParamFinalEQOn);
    snapshot.combScale = param(kParamCombScale);
    for (int band = 0; band < 5; ++band) {
        const uint32_t stickEqBase = kParamEQ1Freq + band * 4;
        snapshot.eqFreq[band] = param(stickEqBase + 0);
        snapshot.eqGain[band] = param(stickEqBase + 1);
        snapshot.eqQ[band] = param(stickEqBase + 2);
        snapshot.eqFilterType[band] = param(stickEqBase + 3);
        snapshot.eqModel[band] = param(kParamEQ1Model + band * 3);
        snapshot.eqSlope[band] = param(kParamEQ1Model + band * 3 + 2);
        snapshot.eqSat[band] = param(kParamEQ1Sat + band);
    }
    snapshot.eqMix = param(kParamEQMix);
    snapshot.resonatorDiffusion = param(kParamResonatorDiffusion);
    snapshot.combMemoryDamping = param(kParamCombMemoryDamping);
    for (int stage = 0; stage < 40; ++stage) {
        const uint32_t combBase = kParamComb1Freq + stage * 4;
        snapshot.combParams[stage * 4 + 0] = param(combBase + 0);
        snapshot.combParams[stage * 4 + 1] = param(combBase + 1);
        snapshot.combParams[stage * 4 + 2] = param(combBase + 2);
        snapshot.combParams[stage * 4 + 3] = param(combBase + 3);
        snapshot.combScalePerStage[stage] = param(kParamComb1Scale + stage);
        snapshot.combSatType[stage] = param(kParamComb1SatType + stage);
        snapshot.combFilterType[stage] = param(kParamComb1FilterType + stage);
        snapshot.combModeRate[stage] = param(kParamComb1ModeRate + stage);
        snapshot.combModeAmount[stage] = param(kParamComb1ModeAmount + stage);
        snapshot.combModeA[stage] = param(kParamComb1ModeA + stage);
        snapshot.combModeB[stage] = param(kParamComb1ModeB + stage);
        snapshot.combPartRelPos[stage] = param(kParamComb1PartRelPos + stage);
        snapshot.combPartFreqDamp[stage] = param(kParamComb1PartFreqDamp + stage);
        snapshot.combPartStiffness[stage] = param(kParamComb1PartStiffness + stage);
        snapshot.combPartStiffExp[stage] = param(kParamComb1PartStiffExp + stage);
        snapshot.combPartCount[stage] = param(kParamComb1PartCount + stage);
        snapshot.combSatAmount[stage] = param(kParamComb1SatAmount + stage);
        snapshot.combStressOnset[stage] = param(kParamComb1StressOnset + stage);
        snapshot.combStressKnee[stage] = param(kParamComb1StressKnee + stage);
        snapshot.combMix[stage] = param(kParamComb1Mix + stage);
        snapshot.combSolo[stage] = param(kParamComb1Solo + stage);
        snapshot.combEnable[stage] = param(kParamComb1Enable + stage);
    }
    for (int group = 0; group < 10; ++group) {
        snapshot.combGroupShiftHz[group] = (param(kParamCombGroup1Shift + group) - 0.5f) * 20000.0f;
        snapshot.combGroupGain[group] = param(kParamCombGroup1Gain + group);
        snapshot.combGroupOn[group] = param(kParamCombGroup1On + group);
        snapshot.combGroupLimiterOn[group] = param(kParamCombGroup1LimiterOn + group);
        snapshot.combGroupLimiterThreshold[group] = param(kParamCombGroup1LimiterThreshold + group);
        snapshot.combGroupLimiterRelease[group] = param(kParamCombGroup1LimiterRelease + group);
        snapshot.combBankSeries[group] = param(kParamCombBank1Series + group);
        snapshot.combGroupCluster[group] = param(kParamCombGroup1Cluster + group);
    }
    snapshot.finalCombScale = param(kParamFinalCombScale);
    snapshot.combBankOn = param(kParamCombBankOn);
    snapshot.finalCombBankOn = param(kParamFinalCombBankOn);
    for (int stage = 0; stage < 4; ++stage) {
        const uint32_t finalCombBase = kParamFinalComb1Freq + stage * 4;
        snapshot.finalCombParams[stage * 4 + 0] = param(finalCombBase + 0);
        snapshot.finalCombParams[stage * 4 + 1] = param(finalCombBase + 1);
        snapshot.finalCombParams[stage * 4 + 2] = param(finalCombBase + 2);
        snapshot.finalCombParams[stage * 4 + 3] = param(finalCombBase + 3);
        snapshot.finalCombMix[stage] = param(kParamFinalComb1Mix + stage);
        snapshot.finalCombSolo[stage] = param(kParamFinalComb1Solo + stage);
        snapshot.finalCombEnable[stage] = param(kParamFinalComb1Enable + stage);
        snapshot.finalCombFilterType[stage] = param(kParamFinalComb1FilterType + stage);
        snapshot.finalCombModeRate[stage] = param(kParamFinalComb1ModeRate + stage);
        snapshot.finalCombModeAmount[stage] = param(kParamFinalComb1ModeAmount + stage);
        snapshot.finalCombModeA[stage] = param(kParamFinalComb1ModeA + stage);
        snapshot.finalCombModeB[stage] = param(kParamFinalComb1ModeB + stage);
        snapshot.finalCombPartRelPos[stage] = param(kParamFinalComb1PartRelPos + stage);
        snapshot.finalCombPartFreqDamp[stage] = param(kParamFinalComb1PartFreqDamp + stage);
        snapshot.finalCombPartStiffness[stage] = param(kParamFinalComb1PartStiffness + stage);
        snapshot.finalCombPartStiffExp[stage] = param(kParamFinalComb1PartStiffExp + stage);
        snapshot.finalCombPartCount[stage] = param(kParamFinalComb1PartCount + stage);
    }
    snapshot.finalComb4SatType = param(kParamFinalComb4SatType);
    snapshot.finalComb4SatAmount = param(kParamFinalComb4SatAmount);
    snapshot.finalComb4StressOnset = param(kParamFinalComb4StressOnset);
    snapshot.finalComb4StressKnee = param(kParamFinalComb4StressKnee);
    snapshot.finalCombBankSeries = param(kParamFinalCombBankSeries);
    snapshot.finalCombShiftHz = (param(kParamFinalCombShift) - 0.5f) * 20000.0f;
    snapshot.finalCombGroupGain = param(kParamFinalCombGroupGain);
    snapshot.finalCombGroupCluster = param(kParamFinalCombGroupCluster);
    snapshot.finalLimiterOn = param(kParamFinalCombLimiterOn);
    snapshot.finalLimiterThreshold = param(kParamFinalCombLimiterThreshold);
    snapshot.finalLimiterRelease = param(kParamFinalCombLimiterRelease);
    // Per-family base param indices for the 21 captured fields (order matches
    // the DispersionFamilySnapshot member order in captureDispersionFamily below).
    static constexpr uint32_t kDispFamilyParams[2][21] = {
        {kParamDispFB, kParamDispDelay, kParamDispGain, kParamDispFBOverall, kParamNudge1,
         kParamVibRate, kParamVibDepth, kParamVibShape, kParamVibSmooth, kParamVibStepMix,
         kParamChaos, kParamChaosEnvAmount, kParamChaosEnvDecay, kParamPDDrive, kParamPDNormalize,
         kParamPDTone, kParamPDBias, kParamPDSpread, kParamPDMix, kParamDispDelayMode, 0},
        {kParamDisp2FB, kParamDisp2Delay, kParamDisp2Gain, kParamDisp2FBOverall, kParamNudge2_1,
         kParamVib2Rate, kParamVib2Depth, kParamVib2Shape, kParamVib2Smooth, kParamVib2StepMix,
         kParamChaos2, kParamChaos2EnvAmount, kParamChaos2EnvDecay, kParamPD2Drive, kParamPD2Normalize,
         kParamPD2Tone, kParamPD2Bias, kParamPD2Spread, kParamPD2Mix, kParamDisp2DelayMode, 0},
    };
    for (int fam = 0; fam < 2; ++fam) {
        DispersionFamilySnapshot& target = snapshot.dispersion[fam];
        const uint32_t* P = kDispFamilyParams[fam];
        target.feedback = param(P[0]);
        target.delay = param(P[1]);
        target.gain = param(P[2]);
        target.feedbackOverall = param(P[3]);
        for (int i = 0; i < 20; ++i) {
            const uint32_t stageBase = P[4] + i * 2;
            target.nudge[i] = param(stageBase + 0);
            target.nudgeAmount[i] = param(stageBase + 1);
        }
        target.vibRate = param(P[5]);
        target.vibDepth = param(P[6]);
        target.vibShape = param(P[7]);
        target.vibSmooth = param(P[8]);
        target.vibStepMix = param(P[9]);
        target.chaosFactor = param(P[10]);
        target.chaosEnvAmount = param(P[11]);
        target.chaosEnvDecay = param(P[12]);
        target.pdDrive = param(P[13]);
        target.pdNormalize = param(P[14]);
        target.pdTone = param(P[15]);
        target.pdBias = param(P[16]);
        target.pdSpread = param(P[17]);
        target.pdMix = param(P[18]);
        target.delayMode = param(P[19]);
    }
    for (int band = 0; band < 5; ++band) {
        const uint32_t eqBase = kParamFinalEQ1Freq + band * 4;
        snapshot.feqFreq[band] = param(eqBase + 0);
        snapshot.feqGain[band] = param(eqBase + 1);
        snapshot.feqQ[band] = param(eqBase + 2);
        snapshot.feqFilterType[band] = param(eqBase + 3);
        snapshot.feqModel[band] = param(kParamFinalEQ1Model + band * 3);
        snapshot.feqSlope[band] = param(kParamFinalEQ1Model + band * 3 + 2);
        snapshot.feqSat[band] = param(kParamFinalEQ1Sat + band);
    }
    snapshot.finalTiltAmount = param(kParamFinalTiltAmount);
    snapshot.finalTiltPivot = param(kParamFinalTiltPivot);
    snapshot.stickTiltAmount = param(kParamTiltAmount);
    snapshot.stickTiltPivot = param(kParamTiltPivot);
    // Fill the modulation config from parameterValues (same ordering as
    // ModulationEngine::copyConfigTo) so a pending preset commit is captured
    // consistently with every other snapshot field instead of live engine state.
    for (int s = 0; s < ModulationEngine::kNumSources; ++s) {
        const uint32_t base = kParamModSrc1Type + (uint32_t)s * (uint32_t)ModulationEngine::kSourceStride;
        for (int sub = 0; sub < ModulationEngine::kSourceStride; ++sub) {
            snapshot.modConfig[s * ModulationEngine::kSourceStride + sub] = param(base + (uint32_t)sub);
        }
    }
    {
        float* r = snapshot.modConfig + ModulationEngine::kNumSources * ModulationEngine::kSourceStride;
        for (int i = 0; i < ModulationEngine::kNumRoutes; ++i) {
            const uint32_t rb = kParamRoute1Source + (uint32_t)i * 5u;
            for (int sub = 0; sub < 5; ++sub) {
                r[i * 5 + sub] = param(rb + (uint32_t)sub);
            }
        }
    }
    for (int m = 0; m < ModulationEngine::kNumMacros; ++m) {
        snapshot.modConfig[ModulationEngine::kNumSources * ModulationEngine::kSourceStride + ModulationEngine::kNumRoutes * ModulationEngine::kRouteStride + m] =
            param(kParamMacro1 + (uint32_t)m);
    }
    snapshot.modOn = param(kParamModOn);
    return snapshot;
}

void PluginCymbals::configureDetachedIrCombBank(CombFilterBank& bank, const IrExportInputSnapshot& exportInput) const {
    bank.setSampleRate(exportInput.sampleRate);
    bank.setScaleFactor(DSPUtils::scaleFactorFromParam( exportInput.combScale));
    bank.setResonatorDiffusion(exportInput.resonatorDiffusion);
    bank.setMemoryDamping(exportInput.combMemoryDamping);

    for (int stage = 0; stage < 40; ++stage) {
        const int offset = stage * 4;
        bank.setFrequency(stage, DSPUtils::normToHz( exportInput.combParams[offset + 0]));
        bank.setFeedback(stage, std::min(0.998f, exportInput.combParams[offset + 1]));
        bank.setLPF(stage, DSPUtils::normToHz( exportInput.combParams[offset + 2]));
        bank.setHPF(stage, DSPUtils::normToHz( exportInput.combParams[offset + 3]));
        bank.setStageScaleFactor(stage, DSPUtils::scaleFactorFromParam( exportInput.combScalePerStage[stage]));
        bank.setSaturatorType(stage, (int)exportInput.combSatType[stage]);
        bank.setFilterType(stage, (int)exportInput.combFilterType[stage]);
        bank.setModeRate(stage, exportInput.combModeRate[stage]);
        bank.setModeAmount(stage, exportInput.combModeAmount[stage]);
        bank.setModeA(stage, exportInput.combModeA[stage]);
        bank.setModeB(stage, exportInput.combModeB[stage]);
        bank.setPartRelPos(stage, exportInput.combPartRelPos[stage]);
        bank.setPartFreqDamp(stage, exportInput.combPartFreqDamp[stage]);
        bank.setPartStiffness(stage, exportInput.combPartStiffness[stage]);
        bank.setPartStiffExp(stage, exportInput.combPartStiffExp[stage]);
        bank.setPartCount(stage, exportInput.combPartCount[stage]);
        bank.setSaturatorAmount(stage, exportInput.combSatAmount[stage]);
        bank.setSatStressOnset(stage, exportInput.combStressOnset[stage]);
        bank.setSatStressKnee(stage, exportInput.combStressKnee[stage]);
        bank.setStageMix(stage, exportInput.combMix[stage]);
        bank.setStageSolo(stage, exportInput.combSolo[stage] > 0.5f);
        bank.setStageEnabled(stage, exportInput.combEnable[stage] > 0.5f);
    }

    for (int group = 0; group < 10; ++group) {
        const float gainDb = DSPUtils::normGainToDb(exportInput.combGroupGain[group]);
        bank.setShift(group, exportInput.combGroupShiftHz[group]);
        const bool grpOn = exportInput.combGroupOn[group] > 0.5f;
        bank.setGroupGain(group, grpOn ? std::pow(10.0f, gainDb / 20.0f) : 0.0f);
        bank.setSeries(group, exportInput.combBankSeries[group] > 0.5f);
        bank.setCluster(group, exportInput.combGroupCluster[group]);
        bank.setGroupLimiterOn(group, exportInput.combGroupLimiterOn[group]);
        bank.setGroupLimiterThreshold(group, exportInput.combGroupLimiterThreshold[group]);
        bank.setGroupLimiterRelease(group, exportInput.combGroupLimiterRelease[group]);
    }
}

void PluginCymbals::configureDetachedIrFinalCombBank(CombFilterBank& bank, const IrExportInputSnapshot& exportInput) const {
    bank.setSampleRate(exportInput.sampleRate);
    bank.setScaleFactor(DSPUtils::scaleFactorFromParam( exportInput.finalCombScale));
    bank.setMemoryDamping(exportInput.combMemoryDamping);

    for (int stage = 0; stage < 40; ++stage) {
        const bool activeStage = stage < 4;
        if (activeStage) {
            const int offset = stage * 4;
            bank.setFrequency(stage, DSPUtils::normToHz( exportInput.finalCombParams[offset + 0]));
            bank.setFeedback(stage, std::min(0.998f, exportInput.finalCombParams[offset + 1]));
            bank.setLPF(stage, DSPUtils::normToHz( exportInput.finalCombParams[offset + 2]));
            bank.setHPF(stage, DSPUtils::normToHz( exportInput.finalCombParams[offset + 3]));
            bank.setStageMix(stage, exportInput.finalCombMix[stage]);
            bank.setStageSolo(stage, exportInput.finalCombSolo[stage] > 0.5f);
            bank.setStageEnabled(stage, exportInput.finalCombEnable[stage] > 0.5f);
            bank.setSaturatorType(stage, (stage == 3) ? (int)exportInput.finalComb4SatType : 0);
            bank.setFilterType(stage, (int)exportInput.finalCombFilterType[stage]);
            bank.setModeRate(stage, exportInput.finalCombModeRate[stage]);
            bank.setModeAmount(stage, exportInput.finalCombModeAmount[stage]);
            bank.setModeA(stage, exportInput.finalCombModeA[stage]);
            bank.setModeB(stage, exportInput.finalCombModeB[stage]);
            bank.setPartRelPos(stage, exportInput.finalCombPartRelPos[stage]);
            bank.setPartFreqDamp(stage, exportInput.finalCombPartFreqDamp[stage]);
            bank.setPartStiffness(stage, exportInput.finalCombPartStiffness[stage]);
            bank.setPartStiffExp(stage, exportInput.finalCombPartStiffExp[stage]);
            bank.setPartCount(stage, exportInput.finalCombPartCount[stage]);
            bank.setSaturatorAmount(stage, (stage == 3) ? exportInput.finalComb4SatAmount : 0.0f);
            bank.setSatStressOnset(stage, (stage == 3) ? exportInput.finalComb4StressOnset : 0.86f);
            bank.setSatStressKnee(stage, (stage == 3) ? exportInput.finalComb4StressKnee : 0.995f);
        } else {
            bank.setStageMix(stage, 0.0f);
            bank.setStageSolo(stage, false);
            bank.setStageEnabled(stage, false);
            bank.setSaturatorType(stage, 0);
            bank.setFilterType(stage, 0);
            bank.setSaturatorAmount(stage, 0.0f);
            bank.setSatStressOnset(stage, 0.86f);
            bank.setSatStressKnee(stage, 0.995f);
        }
    }

    const float gainDb = DSPUtils::normGainToDb(exportInput.finalCombGroupGain);
    bank.setShift(0, exportInput.finalCombShiftHz);
    bank.setGroupGain(0, std::pow(10.0f, gainDb / 20.0f));
    bank.setSeries(0, exportInput.finalCombBankSeries > 0.5f);
    bank.setGroupLimiterOn(0, exportInput.finalLimiterOn);
    bank.setGroupLimiterThreshold(0, exportInput.finalLimiterThreshold);
    bank.setGroupLimiterRelease(0, exportInput.finalLimiterRelease);
    bank.setCluster(0, exportInput.finalCombGroupCluster);
}

void PluginCymbals::configureDetachedIrDispersionStages(IrExportRenderState& state, const IrExportInputSnapshot& exportInput) const {
    state.dispersion.setSampleRate(exportInput.sampleRate);
    state.dispersion2.setSampleRate(exportInput.sampleRate);
    configureDetachedIrDispersionStage(state.dispersion, exportInput.dispersion[0]);
    configureDetachedIrDispersionStage(state.dispersion2, exportInput.dispersion[1]);
}

void PluginCymbals::configureDetachedIrDispersionStage(DispersionStage& stage, const DispersionFamilySnapshot& snapshot) const {
    stage.setFeedback(DispersionStage::feedbackFromControl(snapshot.feedback) * DispersionStage::overallFeedbackScaleFromControl(snapshot.feedbackOverall));
    stage.setDelaySamples(snapshot.delay * 2000.0f);
    stage.setInputGain(snapshot.gain);
    stage.setVibratoRate(10.0f + snapshot.vibRate * 1990.0f);
    stage.setVibratoDepth(snapshot.vibDepth * 40.0f);
    stage.setVibratoShape((int)snapshot.vibShape);
    stage.setVibratoStepMix(snapshot.vibStepMix);
    stage.setVibratoSmooth(snapshot.vibSmooth);
    stage.setChaos(snapshot.chaosFactor);
    stage.setChaosEnvAmount(snapshot.chaosEnvAmount);
    stage.setChaosEnvDecay(snapshot.chaosEnvDecay);
    stage.setPDDrive(snapshot.pdDrive);
    stage.setPDNormalize(snapshot.pdNormalize);
    stage.setPDTone(snapshot.pdTone);
    stage.setPDBias(snapshot.pdBias);
    stage.setPDSpread(snapshot.pdSpread);
    stage.setPDMix(snapshot.pdMix);
    stage.setDelayMode((int)snapshot.delayMode);
    for (int i = 0; i < 20; ++i) {
        stage.setNudge(i, snapshot.nudge[i]);
        stage.setAmount(i, snapshot.nudgeAmount[i]);
    }
}

void PluginCymbals::configureDetachedIrFinalEq(IrExportRenderState& state, const IrExportInputSnapshot& exportInput) const {
    for (int band = 0; band < 5; ++band) {
        const int typeIdx = std::clamp((int)std::round(exportInput.feqFilterType[band]), 0, 7);
        const float gainDb = (typeIdx >= 4 && typeIdx <= 6)
            ? (exportInput.feqGain[band] - 0.5f) * 48.0f : 0.0f;
        const float freqHz = DSPUtils::normToHz( exportInput.feqFreq[band]);
        state.finalEQNodes[band].setSampleRate(exportInput.sampleRate);
        state.finalEQNodes[band].setModel(std::clamp((int)std::round(exportInput.feqModel[band]), 0, (int)UnifiedFilter::Count - 1));
        state.finalEQNodes[band].setType(typeIdx);
        state.finalEQNodes[band].setSlope(std::clamp((int)std::round(exportInput.feqSlope[band]), 0, 3));
        state.finalEQNodes[band].setCutoffHz(freqHz);
        state.finalEQNodes[band].setResonance(DSPUtils::qFromParam(std::clamp(exportInput.feqQ[band], 0.0f, 1.0f)));
        state.finalEQNodes[band].setGainDb(gainDb);
        state.finalEQNodes[band].setSaturation(std::clamp(exportInput.feqSat[band], 0.0f, 1.0f));
        state.finalEQNodes[band].prepare();
    }

    const float pivotHz = DSPUtils::normToHz( std::clamp(exportInput.finalTiltPivot, 0.0f, 1.0f));
    const float tiltDb = DSPUtils::tiltDbFromParam(std::clamp(exportInput.finalTiltAmount, 0.0f, 1.0f));
    state.finalTiltLow.setParams(Biquad::LOW_SHELF, pivotHz, -tiltDb, 0.707f, exportInput.sampleRate);
    state.finalTiltHigh.setParams(Biquad::HIGH_SHELF, pivotHz, tiltDb, 0.707f, exportInput.sampleRate);
}

void PluginCymbals::configureDetachedIrStickEq(IrExportRenderState& state, const IrExportInputSnapshot& exportInput) const {
    for (int band = 0; band < 5; ++band) {
        const float freqHz = DSPUtils::normToHz( std::clamp(exportInput.eqFreq[band], 0.0f, 1.0f));
        const int typeIdx = std::clamp((int)std::lround(exportInput.eqFilterType[band]), 0, 7);
        const float gainDb = (typeIdx >= 4 && typeIdx <= 6)
            ? (std::clamp(exportInput.eqGain[band], 0.0f, 1.0f) - 0.5f) * 48.0f : 0.0f;
        const float q = DSPUtils::qFromParam(std::clamp(exportInput.eqQ[band], 0.0f, 1.0f));
        state.stickEQNodes[band].setSampleRate(exportInput.sampleRate);
        state.stickEQNodes[band].setModel(std::clamp((int)std::lround(exportInput.eqModel[band]), 0, (int)UnifiedFilter::Count - 1));
        state.stickEQNodes[band].setType(typeIdx);
        state.stickEQNodes[band].setSlope(std::clamp((int)std::lround(exportInput.eqSlope[band]), 0, 3));
        state.stickEQNodes[band].setCutoffHz(freqHz);
        state.stickEQNodes[band].setResonance(q);
        state.stickEQNodes[band].setGainDb(gainDb);
        state.stickEQNodes[band].setSaturation(std::clamp(exportInput.eqSat[band], 0.0f, 1.0f));
        state.stickEQNodes[band].prepare();
    }

    const float pivotHz = DSPUtils::normToHz( std::clamp(exportInput.stickTiltPivot, 0.0f, 1.0f));
    const float tiltDb = DSPUtils::tiltDbFromParam(std::clamp(exportInput.stickTiltAmount, 0.0f, 1.0f));
    state.stickTiltLow.setParams(Biquad::LOW_SHELF, pivotHz, -tiltDb, 0.707f, exportInput.sampleRate);
    state.stickTiltHigh.setParams(Biquad::HIGH_SHELF, pivotHz, tiltDb, 0.707f, exportInput.sampleRate);
}

std::unique_ptr<PluginCymbals::IrExportRenderState> PluginCymbals::createDetachedIrExportRenderState(const IrExportInputSnapshot& exportInput) const {
    std::unique_ptr<IrExportRenderState> state = std::make_unique<IrExportRenderState>(exportInput.sampleRate);
    configureDetachedIrCombBank(state->combBank, exportInput);
    configureDetachedIrFinalCombBank(state->finalCombBank, exportInput);
    configureDetachedIrDispersionStages(*state, exportInput);
    configureDetachedIrStickEq(*state, exportInput);
    configureDetachedIrFinalEq(*state, exportInput);
    return state;
}

// Shared render core for IR export and the UI's tonal profile: renders the
// preset's impulse response into mono-able L/R buffers without touching the
// live DSP. Const so the profile path can call it through a const plugin.
bool PluginCymbals::renderIrSamplesFromValues(const std::vector<float>& parameterSnapshot,
                                              float durationSeconds, std::vector<float>& irL,
                                              std::vector<float>& irR, float& outSampleRate,
                                              std::string& outError) const {
    outError.clear();
    try {
        IrExportInputSnapshot exportInput = createIrExportInputSnapshot(parameterSnapshot, durationSeconds);
        std::unique_ptr<IrExportRenderState> renderState = createDetachedIrExportRenderState(exportInput);
        renderState->modEngine.copyConfigFrom(exportInput.modConfig);
        renderState->modEngine.setTempoBpm(120.0f); // deterministic fallback
        // t = 0 trigger. Performance sources (VEL/NOTE/MW/PB/AT) intentionally
        // render at their defaults here (velocity 1.0, note 0.5, wheels 0): an
        // IR is one static hit with no MIDI input, so live inputs are correct
        // to ignore. RANDOM sources re-roll on this noteOn (fixed seed).
        renderState->modEngine.noteOn();

        const uint32_t frames = (uint32_t)std::max(1.0f, std::floor(exportInput.durationSeconds * exportInput.sampleRate));
        irL.assign(frames, 0.0f);
        irR.assign(frames, 0.0f);

        // Start from a clean detached render state so IR export is deterministic and does not touch live DSP.
        renderState->combBank.reset();
        renderState->finalCombBank.reset();
        renderState->dispersion.clear();
        renderState->dispersion2.clear();
        // Seed the per-sample modulated scalars from the snapshot; routes then
        // adjust them per sample (applyModulatedDetached).
        renderState->modVolume = exportInput.volumeParam;
        renderState->modEqMix = exportInput.eqMix;
        renderState->modFeqMix = exportInput.feqMix;
        renderState->modDispCrossLink = exportInput.dispCrossLink;
        for (int band = 0; band < 5; ++band) {
            renderState->stickEQNodes[band].reset();
        }
        renderState->stickTiltLow.clear();
        renderState->stickTiltHigh.clear();
        
        for (uint32_t i = 0; i < frames; ++i) {
            float sL = (i == 0) ? 1.0f : 0.0f;
            float sR = (i == 0) ? 1.0f : 0.0f;

            // ===== Modulation System (detached render) =====
            float srcOut[ModulationEngine::kNumSources];
            renderState->modEngine.processSample(srcOut);
            if (exportInput.modOn > 0.5f && renderState->modEngine.activeRouteCount() > 0) {
                uint32_t accTargets[ModulationEngine::kNumRoutes];
                float accDeltas[ModulationEngine::kNumRoutes];
                const int accCount = collectModRouteDeltas(renderState->modEngine, srcOut, accTargets, accDeltas);
                for (int k = 0; k < accCount; ++k) {
                    applyModulatedDetached(*renderState, exportInput, accTargets[k], accDeltas[k]);
                }
            }

            float drySL = sL, drySR = sR;
            float wetL = sL, wetR = sR;
            for (int b = 0; b < 5; ++b) {
                renderState->stickEQNodes[b].processStereoSample(wetL, wetR);
            }
            renderState->stickTiltLow.process(wetL, wetR);
            renderState->stickTiltHigh.process(wetL, wetR);
            sL = drySL * (1.0f - renderState->modEqMix) + wetL * renderState->modEqMix;
            sR = drySR * (1.0f - renderState->modEqMix) + wetR * renderState->modEqMix;

            float d1L = sL, d1R = sR, d2L = sL, d2R = sR;
            const bool active1 = (exportInput.disp1Skip < 0.5f);
            const bool active2 = (exportInput.disp2Skip < 0.5f);
            if (active1) renderState->dispersion.process(d1L, d1R);
            if (active2) renderState->dispersion2.process(d2L, d2R);

            float busL = 0.0f, busR = 0.0f;
            if (active1 && active2) {
                const float xL1 = d1L * (1.0f - renderState->modDispCrossLink * 0.5f) + d2L * (renderState->modDispCrossLink * 0.5f);
                const float xR1 = d1R * (1.0f - renderState->modDispCrossLink * 0.5f) + d2R * (renderState->modDispCrossLink * 0.5f);
                const float xL2 = d2L * (1.0f - renderState->modDispCrossLink * 0.5f) + d1L * (renderState->modDispCrossLink * 0.5f);
                const float xR2 = d2R * (1.0f - renderState->modDispCrossLink * 0.5f) + d1R * (renderState->modDispCrossLink * 0.5f);
                busL = (xL1 + xL2) * 0.5f;
                busR = (xR1 + xR2) * 0.5f;
            } else if (active1) {
                busL = d1L;
                busR = d1R;
            } else if (active2) {
                busL = d2L;
                busR = d2R;
            } else {
                busL = sL;
                busR = sR;
            }

            float finalL = 0.0f, finalR = 0.0f;
            if (exportInput.dispSolo > 0.5f) {
                finalL = busL;
                finalR = busR;
            } else {
                // Keep baseline inharmonic shift in place for IR render.
                for (int g = 0; g < 10; ++g) {
                    renderState->combBank.setShift(g, exportInput.combGroupShiftHz[g]);
                }

                float fL = busL, fR = busR;
                if (exportInput.combBankOn > 0.5f) {
                    fL = 0.0f; fR = 0.0f;
                    renderState->combBank.process(busL, busR, fL, fR);
                }
                finalL = (sL * exportInput.stickDirect) + (fL * exportInput.stickBus);
                finalR = (sR * exportInput.stickDirect) + (fR * exportInput.stickBus);
                if (exportInput.finalCombBankOn > 0.5f) {
                    renderState->finalCombBank.processStereo(finalL, finalR);
                }

                if (exportInput.finalEQOn > 0.5f) {
                    float dryFL = finalL, dryFR = finalR;
                    for (int b = 0; b < 5; ++b) {
                        renderState->finalEQNodes[b].processStereoSample(finalL, finalR);
                    }
                    finalL = dryFL * (1.0f - renderState->modFeqMix) + finalL * renderState->modFeqMix;
                    finalR = dryFR * (1.0f - renderState->modFeqMix) + finalR * renderState->modFeqMix;
                }
                
            }

            renderState->finalTiltLow.process(finalL, finalR);
            renderState->finalTiltHigh.process(finalL, finalR);

            if (!std::isfinite(finalL) || !std::isfinite(finalR)) {
                finalL = 0.0f;
                finalR = 0.0f;
            }

            // Identical to the live formula (pow(10, (volume-1) * 60/20)); the
            // per-sample read lets a Volume route pulse the exported tail.
            const float linVol = std::pow(10.0f, (renderState->modVolume - 1.0f) * 3.0f);
            irL[i] = finalL * linVol;
            irR[i] = finalR * linVol;
        }

        outSampleRate = exportInput.sampleRate;
        return true;
    } catch (const std::exception& e) {
        outError = e.what();
        return false;
    } catch (...) {
        outError = "Unknown error while rendering impulse response";
        return false;
    }
}

bool PluginCymbals::renderIrSamples(float durationSeconds, std::vector<float>& irL,
                                    std::vector<float>& irR, float& outSampleRate,
                                    std::string& outError) const {
    return renderIrSamplesFromValues(copyIrExportParameterValues(), durationSeconds,
                                     irL, irR, outSampleRate, outError);
}

// Shared normalised spectrum used by the info profile and the row thumbnails.
static void irSpectrumFromSamples(const std::vector<float>& left,
                                  const std::vector<float>& right,
                                  float sampleRate, uint32_t points,
                                  std::vector<float>& outDb) {
    outDb.clear();
    if (points < 8) points = 8;
    const size_t n = std::min(left.size(), right.size());
    if (n < 64 || sampleRate <= 0.0f) return;
    constexpr double kMinHz = 48.0;
    constexpr double kMaxHz = 15000.0;
    std::vector<double> hann(n);
    double wsum = 0.0;
    for (size_t i = 0; i < n; ++i) {
        hann[i] = 0.5 - 0.5 * std::cos(2.0 * M_PI * (double)i / (double)(n - 1));
        wsum += hann[i];
    }
    outDb.resize(points);
    std::vector<double> raw(points);
    const double ratio = kMaxHz / kMinHz;
    for (uint32_t p = 0; p < points; ++p) {
        const double t = (points > 1) ? (double)p / (double)(points - 1) : 0.0;
        const double hz = kMinHz * std::pow(ratio, t);
        const double w = 2.0 * M_PI * hz / (double)sampleRate;
        const double c = std::cos(w), sn = std::sin(w);
        double re = 0.0, im = 0.0, wr = 1.0, wi = 0.0;
        for (size_t i = 0; i < n; ++i) {
            const double x = 0.5 * ((double)left[i] + (double)right[i]) * hann[i];
            re += x * wr;
            im -= x * wi;
            const double nwr = wr * c + wi * sn;
            wi = wi * c - wr * sn;
            wr = nwr;
        }
        const double mag = std::sqrt(re * re + im * im) / std::max(1e-9, wsum);
        raw[p] = 20.0 * std::log10(std::max(mag, 1.0e-9));
    }
    for (uint32_t p = 0; p < points; ++p) {
        const uint32_t a = (p > 0) ? p - 1 : p;
        const uint32_t b = (p + 1 < points) ? p + 1 : p;
        outDb[p] = (float)((raw[a] + raw[p] + raw[b]) / 3.0);
    }
    float peak = -1e9f;
    for (float v : outDb) peak = std::max(peak, v);
    for (float& v : outDb) v = std::clamp(v - peak, -14.0f, 0.0f);
}

bool PluginCymbals::renderImpulseResponseProfileFromValues(const std::vector<float>& values,
                                                           float durationSeconds, uint32_t points,
                                                           std::vector<float>& outDb,
                                                           std::string& outError) const {
    outDb.clear();
    std::vector<float> left, right;
    float sampleRate = 0.0f;
    if (!renderIrSamplesFromValues(values, durationSeconds, left, right, sampleRate, outError))
        return false;
    irSpectrumFromSamples(left, right, sampleRate, points, outDb);
    if (outDb.empty()) {
        outError = "impulse response too short";
        return false;
    }
    return true;
}

bool PluginCymbals::renderPresetFileProfile(const std::string& path, uint32_t points,
                                            std::vector<float>& outDb, std::string& outError) {
    outDb.clear();
    outError.clear();
    std::ifstream file(std::filesystem::u8path(path), std::ios::binary);
    if (!file.is_open()) { outError = "preset not readable"; return false; }
    std::ostringstream buffer;
    buffer << file.rdbuf();
    const std::string json = buffer.str();
    std::vector<float> values;
    if (!deserializePresetJson(json, values, outError))
        return false;
    return renderImpulseResponseProfileFromValues(values, 0.5f, points, outDb, outError);
}

bool PluginCymbals::exportImpulseResponseToPath(const std::string& path, float durationSeconds, std::string& outError) {
    outError.clear();
    std::vector<float> irL, irR;
    float sampleRate = 48000.0f;
    if (!renderIrSamples(durationSeconds, irL, irR, sampleRate, outError))
        return false;
    try {
        const std::filesystem::path p(path);
        if (p.has_parent_path()) {
            std::filesystem::create_directories(p.parent_path());
        }
        return writeWavStereo16(path, irL, irR, sampleRate, outError);
    } catch (const std::exception& e) {
        outError = e.what();
        return false;
    } catch (...) {
        outError = "Unknown error while exporting impulse response";
        return false;
    }
}

// Normalised log-magnitude curve of the preset's own impulse response, for
// the PRESET INFO tonal-profile plot. points samples are spaced log-wise
// between kIrProfileMinHz..kIrProfileMaxHz (the same span the plot draws), the
// peak is 0 dB and the dynamic range is clamped to +/-14 dB. This is a real
// spectrum of the preset, not metadata: it moves when any voicing/EQ control
// moves.
bool PluginCymbals::renderImpulseResponseProfile(float durationSeconds, uint32_t points,
                                                 std::vector<float>& outDb,
                                                 std::string& outError) const {
    return renderImpulseResponseProfileFromValues(copyIrExportParameterValues(), durationSeconds,
                                                  points, outDb, outError);
}

bool PluginCymbals::exportImpulseResponse(float durationSeconds, std::string& outPath, std::string& outError) {
    outError.clear();
    outPath.clear();
    try {
        std::time_t now = std::time(nullptr);
        std::tm tmNow{};
#if defined(_WIN32)
        localtime_s(&tmNow, &now);
#else
        localtime_r(&now, &tmNow);
#endif

        char ts[32];
        std::snprintf(ts, sizeof(ts), "%04d%02d%02d_%02d%02d%02d",
            tmNow.tm_year + 1900, tmNow.tm_mon + 1, tmNow.tm_mday,
            tmNow.tm_hour, tmNow.tm_min, tmNow.tm_sec);

        std::string stem = sanitizeFileName(fPresetName.empty() ? "cymbals_ir" : fPresetName);
        // Timestamp has second resolution; suffix with a counter if the target
        // already exists so repeat exports within the same second never overwrite.
        const std::string stem2 = stem + "_IR_" + ts;
        std::filesystem::path candidate = std::filesystem::path(getPresetDirectory()) / (stem2 + ".wav");
        int n = 1;
        while (std::filesystem::exists(candidate)) {
            candidate = std::filesystem::path(getPresetDirectory()) / (stem2 + "_" + std::to_string(n++) + ".wav");
        }
        outPath = candidate.string();
        return exportImpulseResponseToPath(outPath, durationSeconds, outError);
    } catch (const std::exception& e) {
        outError = e.what();
        return false;
    } catch (...) {
        outError = "Unknown error while preparing IR export path";
        return false;
    }
}

void PluginCymbals::initState(uint32_t index, State& state) {
    switch (index) {
    case 0:
        state.key = "preset_json";
        state.defaultValue = "";
        break;
    case 1:
        state.key = "preset_name";
        state.defaultValue = "";
        break;
    case 2:
        state.key = "preset_description";
        state.defaultValue = "";
        break;
    case 3:
        state.key = "preset_author";
        state.defaultValue = "";
        break;
    case 4:
        state.key = "preset_tags_csv";
        state.defaultValue = "";
        break;
    case 5:
        state.key = "preset_metadata_json";
        state.defaultValue = "{\"name\":\"\",\"description\":\"\",\"author\":\"\",\"tags\":[]}";
        break;
    case 6:
        state.key = "preset_usage";
        state.defaultValue = "";
        break;
    case 7:
        state.key = "cc_macro_binds";
        state.defaultValue = "";
        break;
    case 8:
        state.key = "midi_learn_csv";
        state.defaultValue = "";
        break;
    default:
        state.key = "";
        state.defaultValue = "";
        break;
    }
}

void PluginCymbals::setState(const char* key, const char* value) {
    if (key == nullptr || key[0] == '\0') return;
    const std::string k(key);
    constexpr size_t kMaxStatePayloadBytes = 1024 * 1024;
    const size_t valueLen = boundedCStringLength(value, kMaxStatePayloadBytes + 1);
    if (valueLen > kMaxStatePayloadBytes) {
        fLastPresetError = "state payload too large";
        return;
    }
    const std::string v(value != nullptr ? std::string(value, valueLen) : std::string());

    if (k == "preset_save_path") {
        std::string err;
        if (!savePresetFromUiIntent(v, err)) {
            fLastPresetError = err;
            updateStateValue("preset_status", ("Error: " + err).c_str());
        } else {
            fLastPresetError.clear();
            updateStateValue("preset_status", ("Saved: " + v).c_str());
        }
        return;
    }

    if (k == "preset_name") {
        fPresetName = v;
        return;
    }

    if (k == "preset_description") {
        fPresetDescription = v;
        return;
    }

    if (k == "preset_tags_csv") {
        fPresetTagsCsv = v;
        return;
    }

    if (k == "preset_author") {
        fPresetAuthor = v;
        return;
    }

    if (k == "preset_usage") {
        fPresetUsage = v;
        return;
    }

    if (k == "preset_load_path") {
        std::string err;
        if (!loadPresetFromPath(v, err)) {
            fLastPresetError = err;
            updateStateValue("preset_status", ("Error: " + err).c_str());
        } else {
            fLastPresetError.clear();
            // Host-mediated state delivery is asynchronous on LV2/CLAP/DSSI, so the
            // UI must not read fPresetName/fLastPresetPath right after setState.
            // Publish the result + metadata back via state keys instead.
            updateStateValue("preset_status", ("Loaded: " + v).c_str());
            updateStateValue("preset_loaded_name", fPresetName.c_str());
            updateStateValue("preset_loaded_description", fPresetDescription.c_str());
            updateStateValue("preset_loaded_author", fPresetAuthor.c_str());
            updateStateValue("preset_loaded_tags", fPresetTagsCsv.c_str());
            updateStateValue("preset_loaded_usage", fPresetUsage.c_str());
        }
        return;
    }

    if (k == "preset_metadata_json") {
        std::string loadedName;
        std::string loadedDescription;
        std::string loadedAuthor;
        std::vector<std::string> loadedTags;
        std::string loadedUsage;
        const bool hasName = parseJsonStringField(v, "name", loadedName);
        const bool hasDescription = parseJsonStringField(v, "description", loadedDescription);
        const bool hasAuthor = parseJsonStringField(v, "author", loadedAuthor);
        const bool hasTags = parseJsonStringArrayField(v, "tags", loadedTags);
        const bool hasUsage = parseJsonStringField(v, "usage", loadedUsage);

        fPresetName = hasName ? loadedName : std::string();
        fPresetDescription = hasDescription ? loadedDescription : std::string();
        fPresetAuthor = hasAuthor ? loadedAuthor : std::string();
        fPresetTagsCsv = hasTags ? joinTagsCsv(loadedTags) : std::string();
        fPresetUsage = hasUsage ? loadedUsage : std::string();
        fLastPresetError.clear();
        return;
    }

    if (k == "preset_json" || k == "preset_load_json") {
        std::vector<float> values;
        std::string err;
        if (!deserializePresetJson(v, values, err)) {
            fLastPresetError = err;
            updateStateValue("preset_status", ("Error: " + err).c_str());
            return;
        }
        applyPresetValuesSafely(values);
        fLastPresetError.clear();
        updateStateValue("preset_status", "Loaded: preset");
        updateStateValue("preset_loaded_name", fPresetName.c_str());
        updateStateValue("preset_loaded_description", fPresetDescription.c_str());
        updateStateValue("preset_loaded_author", fPresetAuthor.c_str());
        updateStateValue("preset_loaded_tags", fPresetTagsCsv.c_str());
        updateStateValue("preset_loaded_usage", fPresetUsage.c_str());
        return;
    }

    if (k == "preset_export_ir_secs") {
        // request export of an impulse response; value should be seconds
        float secs = 0.0f;
        try {
            secs = std::stof(v);
        } catch (...) {
            fLastPresetError = "invalid export duration";
            updateStateValue("preset_status", "Error: invalid export duration");
            return;
        }
        std::string outPath;
        if (!exportImpulseResponse(secs, outPath, fLastPresetError)) {
            // fLastPresetError already set by export function
            updateStateValue("preset_status", ("Error: " + fLastPresetError).c_str());
        } else {
            fLastPresetPath = outPath;
            fLastPresetError.clear();
            updateStateValue("preset_status", ("Exported IR: " + outPath).c_str());
        }
        return;
    }
    // (LOW-10) Race acceptance: these plain arrays are written on the
    // host/UI thread while run() reads them. A restore racing active audio
    // can mix old/new entries mid-block; that is accepted within the same
    // tolerance class as the plain-float parameter baseline — each entry is
    // individually valid, only batch-level mixing is possible, and the next
    // state delivery converges the tables.

    // (#16/#17) Control-surface binding persistence. Each key restores its own
    // table as an authoritative snapshot (cleared first), and entries are
    // applied through the public setters so range clamping, reserved-CC
    // rejection and macro/param exclusivity self-enforce across keys.
    // Formats: "cc_macro_binds" = "cc:macro;..." / "midi_learn_csv" = "param:cc;..."
    if (k == "cc_macro_binds" || k == "midi_learn_csv") {
        if (k == "cc_macro_binds")
            std::fill(ccMacroBind, ccMacroBind + 128, (uint8_t)0);
        else
            std::fill(ccToParam, ccToParam + 128, (int16_t)-1);
        auto numericToken = [](const std::string& s) {
            return !s.empty() && s.size() <= 7 &&
                   s.find_first_not_of("0123456789") == std::string::npos;
        };
        size_t pos = 0;
        while (pos < v.size()) {
            const size_t semi = v.find(';', pos);
            const size_t end = (semi == std::string::npos) ? v.size() : semi;
            const std::string pair = v.substr(pos, end - pos);
            pos = (semi == std::string::npos) ? v.size() : semi + 1;
            const size_t colon = pair.find(':');
            if (colon == std::string::npos) continue;
            const std::string a = pair.substr(0, colon);
            const std::string b = pair.substr(colon + 1);
            if (!numericToken(a) || !numericToken(b)) continue;   // clamp: drop malformed
            const int ia = std::stoi(a);
            const int ib = std::stoi(b);
            if (k == "cc_macro_binds") setCcMacroBind(ia, ib);
            else setCcParamBind(ib, (uint32_t)ia);   // "param:cc" -> (cc, paramIndex)
        }
        return;
    }
}

String PluginCymbals::getState(const char* key) const {
    if (key == nullptr || key[0] == '\0') return String();
    const std::string k(key);

    if (k == "preset_json") {
        return String(serializePresetJson().c_str());
    }
    if (k == "preset_dir") {
        return String(getPresetDirectory().c_str());
    }
    if (k == "preset_name") {
        return String(fPresetName.c_str());
    }
    if (k == "preset_description") {
        return String(fPresetDescription.c_str());
    }
    if (k == "preset_author") {
        return String(fPresetAuthor.c_str());
    }
    if (k == "preset_tags_csv") {
        return String(fPresetTagsCsv.c_str());
    }
    if (k == "preset_usage") {
        return String(fPresetUsage.c_str());
    }
    if (k == "preset_metadata_json") {
        const std::vector<std::string> tags = splitTagsCsv(fPresetTagsCsv);
        std::ostringstream os;
        os << "{\"name\":\"" << jsonEscape(fPresetName)
           << "\",\"description\":\"" << jsonEscape(fPresetDescription)
           << "\",\"author\":\"" << jsonEscape(fPresetAuthor)
           << "\",\"usage\":\"" << jsonEscape(fPresetUsage)
           << "\",\"tags\":[";
        for (size_t i = 0; i < tags.size(); ++i) {
            if (i > 0) os << ",";
            os << "\"" << jsonEscape(tags[i]) << "\"";
        }
        os << "]}";
        return String(os.str().c_str());
    }
    if (k == "preset_last_path") {
        return String(fLastPresetPath.c_str());
    }
    if (k == "preset_last_error") {
        return String(fLastPresetError.c_str());
    }
    if (k == "preset_last_json") {
        std::lock_guard<std::mutex> guard(fLastPresetJsonMutex);
        return String(fLastPresetJson.c_str());
    }
    if (k == "cc_macro_binds") {
        std::string out;
        for (int cc = 0; cc < 128; ++cc) {
            if (ccMacroBind[cc] == 0) continue;
            if (!out.empty()) out += ';';
            out += std::to_string(cc);
            out += ':';
            out += std::to_string((int)ccMacroBind[cc]);
        }
        return String(out.c_str());
    }
    if (k == "midi_learn_csv") {
        std::string out;
        for (int cc = 0; cc < 128; ++cc) {
            if (ccToParam[cc] < 0) continue;
            if (!out.empty()) out += ';';
            out += std::to_string((int)ccToParam[cc]);
            out += ':';
            out += std::to_string(cc);
        }
        return String(out.c_str());
    }
    return String();
}

#ifdef CYMBALS_AUDIO_DEBUG
/**
 * Audio Debug Implementations
 */

void PluginCymbals::startAudioDump(const std::string& label) {
    m_audioDump.label = label;
    m_audioDump.recording = true;
    m_audioDump.frameCounter = 0;
    m_audioDump.sampleRate = fSampleRate;
    m_audioDump.bufferL.clear();
    m_audioDump.bufferR.clear();
    
    // Generate session ID from timestamp
    auto now = std::time(nullptr);
    auto tm_info = *std::localtime(&now);
    std::ostringstream oss;
    oss << std::put_time(&tm_info, "%Y%m%d_%H%M%S");
    m_audioDump.sessionID = oss.str();
    
    logAudioDebugEvent("dump_start", label);
}

bool PluginCymbals::stopAudioDump(std::string& outError) {
    if (!m_audioDump.recording) {
        outError = "No audio dump in progress";
        return false;
    }
    
    m_audioDump.recording = false;
    
    // Ensure artifacts directory exists
    const std::filesystem::path debugDir = std::filesystem::u8path("artifacts/debug");
    std::filesystem::create_directories(debugDir);
    
    // Generate filename
    std::string filename = debugDir.string() + "/audio_dump_" + 
                          m_audioDump.sessionID + "_" + 
                          sanitizeFileName(m_audioDump.label) + ".wav";
    
    // Write WAV file using existing writeWavStereo16 function
    bool success = writeWavStereo16(filename, m_audioDump.bufferL, 
                                   m_audioDump.bufferR, 
                                   (float)m_audioDump.sampleRate, outError);
    
    if (success) {
        std::ostringstream context;
        context << "Saved " << m_audioDump.bufferL.size() << " frames to " << filename;
        logAudioDebugEvent("dump_end", context.str());
    } else {
        logAudioDebugEvent("dump_error", outError);
    }
    
    return success;
}

void PluginCymbals::logAudioDebugEvent(const std::string& label, const std::string& context) {
    const std::filesystem::path debugDir = std::filesystem::u8path("artifacts/debug");
    std::filesystem::create_directories(debugDir);
    
    const std::string logPath = debugDir.string() + "/instrumentation_log.txt";
    
    auto now = std::time(nullptr);
    auto tm_info = *std::localtime(&now);
    
    std::ofstream logFile(logPath, std::ios::app);
    if (logFile.is_open()) {
        logFile << "[" << std::put_time(&tm_info, "%Y-%m-%d %H:%M:%S") << "] "
                << label << " : " << context << "\n";
        logFile.close();
    }
}

#endif // CYMBALS_AUDIO_DEBUG

Plugin* createPlugin() { return new PluginCymbals(); }

END_NAMESPACE_DISTRHO
