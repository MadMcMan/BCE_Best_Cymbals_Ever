#ifndef AIRWINDOWS_EXTRACTED_PARAMS_HPP
#define AIRWINDOWS_EXTRACTED_PARAMS_HPP

#include <array>
#include <cstddef>

START_NAMESPACE_DISTRHO

struct AirwindowsExtractedParam {
    const char* name;
    float defaultValue;
    float minValue;
    float maxValue;
    const char* raw_name;
    const char* label;
};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_Acceleration = {{
    AirwindowsExtractedParam{"limit", 0.32f, 0.0f, 1.0f, "Limit", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_Acceleration2 = {{
    AirwindowsExtractedParam{"limit", 0.32f, 0.0f, 1.0f, "Limit", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_ADClip7 = {{
    AirwindowsExtractedParam{"boost", 0.0f, 0.0f, 1.0f, "Boost", "dB"},
    AirwindowsExtractedParam{"soften", 0.5f, 0.0f, 1.0f, "Soften", ""},
    AirwindowsExtractedParam{"enhance", 0.5f, 0.0f, 1.0f, "Enhance", ""},
    AirwindowsExtractedParam{"mode", 0.0f, 0.0f, 1.0f, "Mode", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_ADClip8 = {{
    AirwindowsExtractedParam{"boost", 0.0f, 0.0f, 1.0f, "Boost", "dB"},
    AirwindowsExtractedParam{"ceiling", 0.883f, 0.0f, 1.0f, "Ceiling", ""},
    AirwindowsExtractedParam{"mode", 0.0f, 0.0f, 1.0f, "Mode", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 5> kAirwindowsParams_ADClip9 = {{
    AirwindowsExtractedParam{"boost", 0.0f, 0.0f, 1.0f, "Boost", "dB"},
    AirwindowsExtractedParam{"match", 0.0f, 0.0f, 1.0f, "Match", "dB"},
    AirwindowsExtractedParam{"noise", 0.7f, 0.0f, 1.0f, "Noise", ""},
    AirwindowsExtractedParam{"ceiling", 0.75f, 0.0f, 1.0f, "Ceiling", ""},
    AirwindowsExtractedParam{"mode", 0.0f, 0.0f, 1.0f, "Mode", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 6> kAirwindowsParams_ADT = {{
    AirwindowsExtractedParam{"headrm", 0.5f, 0.0f, 1.0f, "Headrm", ""},
    AirwindowsExtractedParam{"a__delay", 0.5f, 0.0f, 1.0f, "A Delay", ""},
    AirwindowsExtractedParam{"a__level", 0.5f, 0.0f, 1.0f, "A Level", ""},
    AirwindowsExtractedParam{"b__delay", 0.5f, 0.0f, 1.0f, "B Delay", ""},
    AirwindowsExtractedParam{"b__level", 0.5f, 0.0f, 1.0f, "B Level", ""},
    AirwindowsExtractedParam{"output", 0.5f, 0.0f, 1.0f, "Output", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 6> kAirwindowsParams_Air = {{
    AirwindowsExtractedParam{"22_k_tap", 0.5f, 0.0f, 1.0f, "22K tap", ""},
    AirwindowsExtractedParam{"15_k_tap", 0.5f, 0.0f, 1.0f, "15K tap", ""},
    AirwindowsExtractedParam{"11_k_tap", 0.5f, 0.0f, 1.0f, "11K tap", ""},
    AirwindowsExtractedParam{"filters_q", 0.0f, 0.0f, 1.0f, "filters Q", ""},
    AirwindowsExtractedParam{"output__level", 1.0f, 0.0f, 1.0f, "Output Level", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 5> kAirwindowsParams_Air2 = {{
    AirwindowsExtractedParam{"hiss", 0.5f, 0.0f, 1.0f, "Hiss", ""},
    AirwindowsExtractedParam{"glitter", 0.5f, 0.0f, 1.0f, "Glitter", ""},
    AirwindowsExtractedParam{"air", 0.5f, 0.0f, 1.0f, "Air", ""},
    AirwindowsExtractedParam{"silk", 0.0f, 0.0f, 1.0f, "Silk", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_Air3 = {{
    AirwindowsExtractedParam{"air", 0.5f, 0.0f, 1.0f, "Air", ""},
    AirwindowsExtractedParam{"gnd", 0.5f, 0.0f, 1.0f, "Gnd", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_Air4 = {{
    AirwindowsExtractedParam{"air", 0.5f, 0.0f, 1.0f, "Air", ""},
    AirwindowsExtractedParam{"gnd", 0.5f, 0.0f, 1.0f, "Gnd", ""},
    AirwindowsExtractedParam{"dark_f", 0.52f, 0.0f, 1.0f, "DarkF", ""},
    AirwindowsExtractedParam{"ratio", 0.0f, 0.0f, 1.0f, "Ratio", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 8> kAirwindowsParams_AngleEQ = {{
    AirwindowsExtractedParam{"high", 0.5f, 0.0f, 1.0f, "High", ""},
    AirwindowsExtractedParam{"mid", 0.4f, 0.0f, 1.0f, "Mid", ""},
    AirwindowsExtractedParam{"low", 0.5f, 0.0f, 1.0f, "Low", ""},
    AirwindowsExtractedParam{"hi_freq", 0.8f, 0.0f, 1.0f, "HiFreq", ""},
    AirwindowsExtractedParam{"m_hi_freq", 0.8f, 0.0f, 1.0f, "MHiFreq", ""},
    AirwindowsExtractedParam{"m_lo_freq", 0.2f, 0.0f, 1.0f, "MLoFreq", ""},
    AirwindowsExtractedParam{"lo_freq", 0.2f, 0.0f, 1.0f, "LoFreq", ""},
    AirwindowsExtractedParam{"inv__wet", 1.0f, 0.0f, 1.0f, "Inv/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_AngleFilter = {{
    AirwindowsExtractedParam{"freq", 0.5f, 0.0f, 1.0f, "Freq", ""},
    AirwindowsExtractedParam{"hard", 0.5f, 0.0f, 1.0f, "Hard", ""},
    AirwindowsExtractedParam{"reso", 0.0f, 0.0f, 1.0f, "Reso", ""},
    AirwindowsExtractedParam{"poles", 0.5f, 0.0f, 1.0f, "Poles", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_Apicolypse = {{
    AirwindowsExtractedParam{"hardns", 0.7f, 0.0f, 1.0f, "Hardns", ""},
    AirwindowsExtractedParam{"persnlty", 0.333333f, 0.0f, 1.0f, "Persnlty", ""},
    AirwindowsExtractedParam{"drive", 0.333333f, 0.0f, 1.0f, "Drive", ""},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_AQuickVoiceClip = {{
    AirwindowsExtractedParam{"highpass", 0.42f, 0.0f, 1.0f, "Highpass", "hz"},
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_AtmosphereBuss = {{
    AirwindowsExtractedParam{"input", 1.0f, 0.0f, 1.0f, "Input", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_AtmosphereChannel = {{
    AirwindowsExtractedParam{"input", 1.0f, 0.0f, 1.0f, "Input", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_Aura = {{
    AirwindowsExtractedParam{"voicing", 0.5f, 0.0f, 1.0f, "Voicing", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_AutoPan = {{
    AirwindowsExtractedParam{"rate", 0.1f, 0.0f, 1.0f, "Rate", ""},
    AirwindowsExtractedParam{"phase", 0.5f, 0.0f, 1.0f, "Phase", ""},
    AirwindowsExtractedParam{"wide", 0.0f, 0.0f, 1.0f, "Wide", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_Average = {{
    AirwindowsExtractedParam{"average", 0.0f, 0.0f, 1.0f, "Average", "taps"},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_AverMatrix = {{
    AirwindowsExtractedParam{"average", 0.0f, 0.0f, 1.0f, "Average", "taps"},
    AirwindowsExtractedParam{"depth", 0.0f, 0.0f, 1.0f, "Depth", "poles"},
    AirwindowsExtractedParam{"inv__wet", 1.0f, 0.0f, 1.0f, "Inv/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_Balanced = {{
    AirwindowsExtractedParam{"bit_shift", 0.0f, 0.0f, 1.0f, "BitShift", "bits"},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_BassAmp = {{
    AirwindowsExtractedParam{"high", 0.75f, 0.0f, 1.0f, "High", ""},
    AirwindowsExtractedParam{"dry", 0.0f, 0.0f, 1.0f, "Dry", ""},
    AirwindowsExtractedParam{"dub", 0.75f, 0.0f, 1.0f, "Dub", ""},
    AirwindowsExtractedParam{"sub", 0.0f, 0.0f, 1.0f, "Sub", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 5> kAirwindowsParams_BassDrive = {{
    AirwindowsExtractedParam{"presnce", 0.5f, 0.0f, 1.0f, "Presnce", ""},
    AirwindowsExtractedParam{"high", 0.5f, 0.0f, 1.0f, "High", ""},
    AirwindowsExtractedParam{"mid", 0.5f, 0.0f, 1.0f, "Mid", ""},
    AirwindowsExtractedParam{"low", 0.5f, 0.0f, 1.0f, "Low", ""},
    AirwindowsExtractedParam{"drive", 0.5f, 0.0f, 1.0f, "Drive", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_BassKit = {{
    AirwindowsExtractedParam{"drive", 0.5f, 0.0f, 1.0f, "Drive", ""},
    AirwindowsExtractedParam{"voicing", 0.5f, 0.0f, 1.0f, "Voicing", ""},
    AirwindowsExtractedParam{"bass_out", 0.5f, 0.0f, 1.0f, "BassOut", ""},
    AirwindowsExtractedParam{"sub_out", 0.5f, 0.0f, 1.0f, "SubOut", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_Baxandall = {{
    AirwindowsExtractedParam{"treble", 0.5f, 0.0f, 1.0f, "Treble", "dB"},
    AirwindowsExtractedParam{"bass", 0.5f, 0.0f, 1.0f, "Bass", "dB"},
    AirwindowsExtractedParam{"output", 0.5f, 0.0f, 1.0f, "Output", "dB"},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_Baxandall2 = {{
    AirwindowsExtractedParam{"treble", 0.5f, 0.0f, 1.0f, "Treble", "dB"},
    AirwindowsExtractedParam{"bass", 0.5f, 0.0f, 1.0f, "Bass", "dB"},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_Baxandall3 = {{
    AirwindowsExtractedParam{"input", 0.5f, 0.0f, 1.0f, "Input", ""},
    AirwindowsExtractedParam{"treble", 0.5f, 0.0f, 1.0f, "Treble", ""},
    AirwindowsExtractedParam{"bass", 0.5f, 0.0f, 1.0f, "Bass", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_Beam = {{
    AirwindowsExtractedParam{"quant", 1.0f, 0.0f, 1.0f, "Quant", ""},
    AirwindowsExtractedParam{"focus", 0.5f, 0.0f, 1.0f, "Focus", ""},
    AirwindowsExtractedParam{"de_rez", 0.0f, 0.0f, 1.0f, "DeRez", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 5> kAirwindowsParams_BezEQ = {{
    AirwindowsExtractedParam{"treble", 0.5f, 0.0f, 1.0f, "Treble", ""},
    AirwindowsExtractedParam{"x", 0.618f, 0.0f, 1.0f, "x", ""},
    AirwindowsExtractedParam{"mid", 0.5f, 0.0f, 1.0f, "Mid", ""},
    AirwindowsExtractedParam{"x", 0.382f, 0.0f, 1.0f, "x", ""},
    AirwindowsExtractedParam{"bass", 0.5f, 0.0f, 1.0f, "Bass", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 5> kAirwindowsParams_BezEQ2 = {{
    AirwindowsExtractedParam{"treble", 0.5f, 0.0f, 1.0f, "Treble", ""},
    AirwindowsExtractedParam{"x", 0.75f, 0.0f, 1.0f, "x", ""},
    AirwindowsExtractedParam{"mid", 0.5f, 0.0f, 1.0f, "Mid", ""},
    AirwindowsExtractedParam{"x", 0.25f, 0.0f, 1.0f, "x", ""},
    AirwindowsExtractedParam{"bass", 0.5f, 0.0f, 1.0f, "Bass", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 8> kAirwindowsParams_BezEQ3 = {{
    AirwindowsExtractedParam{"high", 0.5f, 0.0f, 1.0f, "High", ""},
    AirwindowsExtractedParam{"h_mid", 0.5f, 0.0f, 1.0f, "HMid", ""},
    AirwindowsExtractedParam{"l_mid", 0.5f, 0.0f, 1.0f, "LMid", ""},
    AirwindowsExtractedParam{"bass", 0.5f, 0.0f, 1.0f, "Bass", ""},
    AirwindowsExtractedParam{"high_f", 0.5f, 0.0f, 1.0f, "HighF", ""},
    AirwindowsExtractedParam{"h_mid_f", 0.5f, 0.0f, 1.0f, "HMidF", ""},
    AirwindowsExtractedParam{"l_mid_f", 0.5f, 0.0f, 1.0f, "LMidF", ""},
    AirwindowsExtractedParam{"bass_f", 0.5f, 0.0f, 1.0f, "BassF", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 8> kAirwindowsParams_BezEQ4 = {{
    AirwindowsExtractedParam{"high", 0.5f, 0.0f, 1.0f, "High", ""},
    AirwindowsExtractedParam{"h_mid", 0.5f, 0.0f, 1.0f, "HMid", ""},
    AirwindowsExtractedParam{"l_mid", 0.5f, 0.0f, 1.0f, "LMid", ""},
    AirwindowsExtractedParam{"bass", 0.5f, 0.0f, 1.0f, "Bass", ""},
    AirwindowsExtractedParam{"high_f", 0.5f, 0.0f, 1.0f, "HighF", ""},
    AirwindowsExtractedParam{"h_mid_f", 0.5f, 0.0f, 1.0f, "HMidF", ""},
    AirwindowsExtractedParam{"l_mid_f", 0.5f, 0.0f, 1.0f, "LMidF", ""},
    AirwindowsExtractedParam{"bass_f", 0.5f, 0.0f, 1.0f, "BassF", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_BeziComp = {{
    AirwindowsExtractedParam{"comp", 0.5f, 0.0f, 1.0f, "Comp", ""},
    AirwindowsExtractedParam{"speed", 0.5f, 0.0f, 1.0f, "Speed", ""},
    AirwindowsExtractedParam{"dry__wet", 0.5f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_BigAmp = {{
    AirwindowsExtractedParam{"gain", 0.5f, 0.0f, 1.0f, "Gain", ""},
    AirwindowsExtractedParam{"tone", 0.5f, 0.0f, 1.0f, "Tone", ""},
    AirwindowsExtractedParam{"output", 0.8f, 0.0f, 1.0f, "Output", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_Biquad = {{
    AirwindowsExtractedParam{"type", 1.0f, 0.0f, 1.0f, "Type", ""},
    AirwindowsExtractedParam{"freq", 0.5f, 0.0f, 1.0f, "Freq", ""},
    AirwindowsExtractedParam{"q", 0.5f, 0.0f, 1.0f, "Q", ""},
    AirwindowsExtractedParam{"inv__wet", 1.0f, 0.0f, 1.0f, "Inv/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 5> kAirwindowsParams_Biquad2 = {{
    AirwindowsExtractedParam{"type", 1.0f, 0.0f, 1.0f, "Type", ""},
    AirwindowsExtractedParam{"freq", 0.5f, 0.0f, 1.0f, "Freq", ""},
    AirwindowsExtractedParam{"q", 0.5f, 0.0f, 1.0f, "Q", ""},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
    AirwindowsExtractedParam{"inv__wet", 1.0f, 0.0f, 1.0f, "Inv/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_BiquadDouble = {{
    AirwindowsExtractedParam{"type", 1.0f, 0.0f, 1.0f, "Type", ""},
    AirwindowsExtractedParam{"freq", 0.5f, 0.0f, 1.0f, "Freq", ""},
    AirwindowsExtractedParam{"q", 0.5f, 0.0f, 1.0f, "Q", ""},
    AirwindowsExtractedParam{"inv__wet", 1.0f, 0.0f, 1.0f, "Inv/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_BiquadHiLo = {{
    AirwindowsExtractedParam{"highpas", 0.0f, 0.0f, 1.0f, "Highpas", ""},
    AirwindowsExtractedParam{"lowpass", 0.0f, 0.0f, 1.0f, "Lowpass", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 5> kAirwindowsParams_BiquadNonLin = {{
    AirwindowsExtractedParam{"type", 0.0f, 0.0f, 1.0f, "Type", ""},
    AirwindowsExtractedParam{"freq", 0.5f, 0.0f, 1.0f, "Freq", ""},
    AirwindowsExtractedParam{"q", 0.7071f, 0.0f, 1.0f, "Q", ""},
    AirwindowsExtractedParam{"non_lin", 0.0f, 0.0f, 1.0f, "NonLin", ""},
    AirwindowsExtractedParam{"inv__wet", 1.0f, 0.0f, 1.0f, "Inv/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_BiquadOneHalf = {{
    AirwindowsExtractedParam{"type", 0.0f, 0.0f, 1.0f, "Type", ""},
    AirwindowsExtractedParam{"freq", 0.0f, 0.0f, 1.0f, "Freq", ""},
    AirwindowsExtractedParam{"q", 0.0f, 0.0f, 1.0f, "Q", ""},
    AirwindowsExtractedParam{"inv__wet", 0.0f, 0.0f, 1.0f, "Inv/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_BiquadPlus = {{
    AirwindowsExtractedParam{"type", 1.0f, 0.0f, 1.0f, "Type", ""},
    AirwindowsExtractedParam{"freq", 0.5f, 0.0f, 1.0f, "Freq", ""},
    AirwindowsExtractedParam{"q", 0.5f, 0.0f, 1.0f, "Q", ""},
    AirwindowsExtractedParam{"inv__wet", 1.0f, 0.0f, 1.0f, "Inv/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_BiquadStack = {{
    AirwindowsExtractedParam{"freq", 0.5f, 0.0f, 1.0f, "Freq", ""},
    AirwindowsExtractedParam{"level", 0.5f, 0.0f, 1.0f, "Level", ""},
    AirwindowsExtractedParam{"reso", 0.5f, 0.0f, 1.0f, "Reso", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_BiquadTriple = {{
    AirwindowsExtractedParam{"type", 1.0f, 0.0f, 1.0f, "Type", ""},
    AirwindowsExtractedParam{"freq", 0.5f, 0.0f, 1.0f, "Freq", ""},
    AirwindowsExtractedParam{"q", 0.5f, 0.0f, 1.0f, "Q", ""},
    AirwindowsExtractedParam{"inv__wet", 1.0f, 0.0f, 1.0f, "Inv/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_BitDualPan = {{
    AirwindowsExtractedParam{"l__vol", 0.5f, 0.0f, 1.0f, "L Vol", ""},
    AirwindowsExtractedParam{"l__pan", 0.0f, 0.0f, 1.0f, "L Pan", ""},
    AirwindowsExtractedParam{"r__vol", 0.5f, 0.0f, 1.0f, "R Vol", ""},
    AirwindowsExtractedParam{"r__pan", 1.0f, 0.0f, 1.0f, "R Pan", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_Bite = {{
    AirwindowsExtractedParam{"bite", 0.5f, 0.0f, 1.0f, "Bite", ""},
    AirwindowsExtractedParam{"output__level", 1.0f, 0.0f, 1.0f, "Output Level", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_BitGlitter = {{
    AirwindowsExtractedParam{"input", 0.5f, 0.0f, 1.0f, "Input", ""},
    AirwindowsExtractedParam{"glitter", 0.0f, 0.0f, 1.0f, "Glitter", ""},
    AirwindowsExtractedParam{"output", 0.5f, 0.0f, 1.0f, "Output", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_BitShiftGain = {{
    AirwindowsExtractedParam{"bit_shift", 0.5f, 0.0f, 1.0f, "BitShift", "bits"},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_BitShiftPan = {{
    AirwindowsExtractedParam{"vol", 0.5f, 0.0f, 1.0f, "Vol", ""},
    AirwindowsExtractedParam{"pan", 0.5f, 0.0f, 1.0f, "Pan", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_BlockParty = {{
    AirwindowsExtractedParam{"pound", 0.0f, 0.0f, 1.0f, "Pound", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_BrassRider = {{
    AirwindowsExtractedParam{"thresh", 0.0f, 0.0f, 1.0f, "Thresh", ""},
    AirwindowsExtractedParam{"dry__wet", 0.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_BrightAmbience = {{
    AirwindowsExtractedParam{"dry__wet", 0.0f, 0.0f, 1.0f, "Dry/Wet", ""},
    AirwindowsExtractedParam{"sustain", 0.0f, 0.0f, 1.0f, "Sustain", ""},
    AirwindowsExtractedParam{"decay", 0.0f, 0.0f, 1.0f, "Decay", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_BrightAmbience2 = {{
    AirwindowsExtractedParam{"start", 0.2f, 0.0f, 1.0f, "Start", ""},
    AirwindowsExtractedParam{"length", 0.2f, 0.0f, 1.0f, "Length", ""},
    AirwindowsExtractedParam{"feedbck", 0.0f, 0.0f, 1.0f, "Feedbck", ""},
    AirwindowsExtractedParam{"dry__wet", 0.5f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_BrightAmbience3 = {{
    AirwindowsExtractedParam{"start", 1.0f, 0.0f, 1.0f, "Start", ""},
    AirwindowsExtractedParam{"length", 0.0f, 0.0f, 1.0f, "Length", ""},
    AirwindowsExtractedParam{"feedback", 0.0f, 0.0f, 1.0f, "Feedback", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 10> kAirwindowsParams_BuildATPDF = {{
    AirwindowsExtractedParam{"first", 0.5f, 0.0f, 1.0f, "First", ""},
    AirwindowsExtractedParam{"second", 0.5f, 0.0f, 1.0f, "Second", ""},
    AirwindowsExtractedParam{"third", 0.5f, 0.0f, 1.0f, "Third", ""},
    AirwindowsExtractedParam{"fourth", 0.5f, 0.0f, 1.0f, "Fourth", ""},
    AirwindowsExtractedParam{"fifth", 0.5f, 0.0f, 1.0f, "Fifth", ""},
    AirwindowsExtractedParam{"sixth", 0.5f, 0.0f, 1.0f, "Sixth", ""},
    AirwindowsExtractedParam{"seventh", 0.5f, 0.0f, 1.0f, "Seventh", ""},
    AirwindowsExtractedParam{"eighth", 0.5f, 0.0f, 1.0f, "Eighth", ""},
    AirwindowsExtractedParam{"ninth", 0.5f, 0.0f, 1.0f, "Ninth", ""},
    AirwindowsExtractedParam{"tenth", 0.5f, 0.0f, 1.0f, "Tenth", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_BussColors4 = {{
    AirwindowsExtractedParam{"color", 0.0f, 0.0f, 1.0f, "Color", ""},
    AirwindowsExtractedParam{"input__trim", 0.5f, 0.0f, 1.0f, "Input Trim", "dB"},
    AirwindowsExtractedParam{"output__trim", 0.5f, 0.0f, 1.0f, "Output Trim", "dB"},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_ButterComp = {{
    AirwindowsExtractedParam{"compress", 0.0f, 0.0f, 1.0f, "Compress", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_ButterComp2 = {{
    AirwindowsExtractedParam{"compress", 0.0f, 0.0f, 1.0f, "Compress", ""},
    AirwindowsExtractedParam{"output", 0.5f, 0.0f, 1.0f, "Output", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_C5RawBuss = {{
    AirwindowsExtractedParam{"center", 0.0f, 0.0f, 1.0f, "Center", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_C5RawChannel = {{
    AirwindowsExtractedParam{"center", 0.0f, 0.0f, 1.0f, "Center", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 6> kAirwindowsParams_Cabs = {{
    AirwindowsExtractedParam{"cab_type", 0.0f, 0.0f, 1.0f, "CabType", ""},
    AirwindowsExtractedParam{"intense", 0.66f, 0.0f, 1.0f, "Intense", ""},
    AirwindowsExtractedParam{"rm__loud", 0.33f, 0.0f, 1.0f, "Rm Loud", ""},
    AirwindowsExtractedParam{"cab_size", 0.66f, 0.0f, 1.0f, "CabSize", ""},
    AirwindowsExtractedParam{"off_axis", 0.33f, 0.0f, 1.0f, "OffAxis", ""},
    AirwindowsExtractedParam{"output", 0.5f, 0.0f, 1.0f, "Output", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_Cabs2 = {{
    AirwindowsExtractedParam{"lowpass", 1.0f, 0.0f, 1.0f, "Lowpass", ""},
    AirwindowsExtractedParam{"hipass", 0.0f, 0.0f, 1.0f, "Hipass", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_Calibre = {{
    AirwindowsExtractedParam{"hardns", 0.74f, 0.0f, 1.0f, "Hardns", ""},
    AirwindowsExtractedParam{"persnlty", 0.333333f, 0.0f, 1.0f, "Persnlty", ""},
    AirwindowsExtractedParam{"drive", 0.333333f, 0.0f, 1.0f, "Drive", ""},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 5> kAirwindowsParams_Cans = {{
    AirwindowsExtractedParam{"room", 0.5f, 0.0f, 1.0f, "Room", ""},
    AirwindowsExtractedParam{"diffuse", 0.5f, 0.0f, 1.0f, "Diffuse", ""},
    AirwindowsExtractedParam{"damping", 0.5f, 0.0f, 1.0f, "Damping", ""},
    AirwindowsExtractedParam{"crossfd", 0.5f, 0.0f, 1.0f, "Crossfd", ""},
    AirwindowsExtractedParam{"dry__wet", 0.5f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_CansAW = {{
    AirwindowsExtractedParam{"seat_pos", 0.2f, 0.0f, 1.0f, "seatPos", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_Capacitor = {{
    AirwindowsExtractedParam{"lowpass", 1.0f, 0.0f, 1.0f, "Lowpass", ""},
    AirwindowsExtractedParam{"highpass", 0.0f, 0.0f, 1.0f, "Highpass", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_Capacitor2 = {{
    AirwindowsExtractedParam{"lowpass", 1.0f, 0.0f, 1.0f, "Lowpass", ""},
    AirwindowsExtractedParam{"highpass", 0.0f, 0.0f, 1.0f, "Highpass", ""},
    AirwindowsExtractedParam{"non_lin", 0.0f, 0.0f, 1.0f, "NonLin", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 5> kAirwindowsParams_Chamber = {{
    AirwindowsExtractedParam{"bigness", 0.35f, 0.0f, 1.0f, "Bigness", ""},
    AirwindowsExtractedParam{"longness", 0.35f, 0.0f, 1.0f, "Longness", ""},
    AirwindowsExtractedParam{"liteness", 0.35f, 0.0f, 1.0f, "Liteness", ""},
    AirwindowsExtractedParam{"darkness", 0.35f, 0.0f, 1.0f, "Darkness", ""},
    AirwindowsExtractedParam{"wetness", 0.35f, 0.0f, 1.0f, "Wetness", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_Chamber2 = {{
    AirwindowsExtractedParam{"delay", 0.34f, 0.0f, 1.0f, "Delay", ""},
    AirwindowsExtractedParam{"regen", 0.31f, 0.0f, 1.0f, "Regen", ""},
    AirwindowsExtractedParam{"thick", 0.28f, 0.0f, 1.0f, "Thick", ""},
    AirwindowsExtractedParam{"wet", 0.25f, 0.0f, 1.0f, "Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_Channel4 = {{
    AirwindowsExtractedParam{"console__type", 0.0f, 0.0f, 1.0f, "Console Type", ""},
    AirwindowsExtractedParam{"drive", 0.0f, 0.0f, 1.0f, "Drive", "%"},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_Channel5 = {{
    AirwindowsExtractedParam{"console__type", 0.0f, 0.0f, 1.0f, "Console Type", ""},
    AirwindowsExtractedParam{"drive", 0.0f, 0.0f, 1.0f, "Drive", "%"},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_Channel6 = {{
    AirwindowsExtractedParam{"console__type", 0.0f, 0.0f, 1.0f, "Console Type", ""},
    AirwindowsExtractedParam{"drive", 0.0f, 0.0f, 1.0f, "Drive", "%"},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_Channel7 = {{
    AirwindowsExtractedParam{"console__type", 0.0f, 0.0f, 1.0f, "Console Type", ""},
    AirwindowsExtractedParam{"drive", 0.0f, 0.0f, 1.0f, "Drive", "%"},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_Channel8 = {{
    AirwindowsExtractedParam{"console__type", 0.0f, 0.0f, 1.0f, "Console Type", ""},
    AirwindowsExtractedParam{"drive", 0.0f, 0.0f, 1.0f, "Drive", "%"},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_Channel9 = {{
    AirwindowsExtractedParam{"console__type", 0.0f, 0.0f, 1.0f, "Console Type", ""},
    AirwindowsExtractedParam{"drive", 0.0f, 0.0f, 1.0f, "Drive", "%"},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_ChannelX = {{
    AirwindowsExtractedParam{"console__type", 0.0f, 0.0f, 1.0f, "Console Type", ""},
    AirwindowsExtractedParam{"drive", 0.0f, 0.0f, 1.0f, "Drive", "%"},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 10> kAirwindowsParams_ChimeyDeluxe = {{
    AirwindowsExtractedParam{"hell", 0.5f, 0.0f, 1.0f, "Hell", ""},
    AirwindowsExtractedParam{"fizz", 0.5f, 0.0f, 1.0f, "Fizz", ""},
    AirwindowsExtractedParam{"pick", 0.5f, 0.0f, 1.0f, "Pick", ""},
    AirwindowsExtractedParam{"satan", 0.5f, 0.0f, 1.0f, "Satan", ""},
    AirwindowsExtractedParam{"danger", 0.5f, 0.0f, 1.0f, "Danger", ""},
    AirwindowsExtractedParam{"crtical", 0.5f, 0.0f, 1.0f, "Crtical", ""},
    AirwindowsExtractedParam{"h__meat", 0.5f, 0.0f, 1.0f, "H Meat", ""},
    AirwindowsExtractedParam{"l__meat", 0.5f, 0.0f, 1.0f, "L Meat", ""},
    AirwindowsExtractedParam{"swing", 0.5f, 0.0f, 1.0f, "Swing", ""},
    AirwindowsExtractedParam{"rarely", 0.5f, 0.0f, 1.0f, "Rarely", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 10> kAirwindowsParams_ChimeyGuitar = {{
    AirwindowsExtractedParam{"compres", 0.5f, 0.0f, 1.0f, "Compres", ""},
    AirwindowsExtractedParam{"presnce", 0.8f, 0.0f, 1.0f, "Presnce", ""},
    AirwindowsExtractedParam{"high", 0.7f, 0.0f, 1.0f, "High", ""},
    AirwindowsExtractedParam{"mid", 0.5f, 0.0f, 1.0f, "Mid", ""},
    AirwindowsExtractedParam{"low", 0.7f, 0.0f, 1.0f, "Low", ""},
    AirwindowsExtractedParam{"sub", 0.4f, 0.0f, 1.0f, "Sub", ""},
    AirwindowsExtractedParam{"h_speakr", 0.8f, 0.0f, 1.0f, "HSpeakr", ""},
    AirwindowsExtractedParam{"l_speakr", 0.4f, 0.0f, 1.0f, "LSpeakr", ""},
    AirwindowsExtractedParam{"speed", 0.3f, 0.0f, 1.0f, "Speed", ""},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 10> kAirwindowsParams_ChimeyGuitar2 = {{
    AirwindowsExtractedParam{"compres", 0.6f, 0.0f, 1.0f, "Compres", ""},
    AirwindowsExtractedParam{"presnce", 0.8f, 0.0f, 1.0f, "Presnce", ""},
    AirwindowsExtractedParam{"high", 0.7f, 0.0f, 1.0f, "High", ""},
    AirwindowsExtractedParam{"mid", 0.5f, 0.0f, 1.0f, "Mid", ""},
    AirwindowsExtractedParam{"low", 0.7f, 0.0f, 1.0f, "Low", ""},
    AirwindowsExtractedParam{"sub", 0.4f, 0.0f, 1.0f, "Sub", ""},
    AirwindowsExtractedParam{"h_speakr", 0.8f, 0.0f, 1.0f, "HSpeakr", ""},
    AirwindowsExtractedParam{"l_speakr", 0.4f, 0.0f, 1.0f, "LSpeakr", ""},
    AirwindowsExtractedParam{"speed", 0.8f, 0.0f, 1.0f, "Speed", ""},
    AirwindowsExtractedParam{"output", 0.8f, 0.0f, 1.0f, "Output", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_Chorus = {{
    AirwindowsExtractedParam{"speed", 0.5f, 0.0f, 1.0f, "Speed", ""},
    AirwindowsExtractedParam{"range", 0.5f, 0.0f, 1.0f, "Range", ""},
    AirwindowsExtractedParam{"dry__wet", 0.5f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_ChorusEnsemble = {{
    AirwindowsExtractedParam{"speed", 0.5f, 0.0f, 1.0f, "Speed", ""},
    AirwindowsExtractedParam{"range", 0.5f, 0.0f, 1.0f, "Range", ""},
    AirwindowsExtractedParam{"dry__wet", 0.8f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_ChromeOxide = {{
    AirwindowsExtractedParam{"intense", 0.5f, 0.0f, 1.0f, "Intense", ""},
    AirwindowsExtractedParam{"bias", 0.5f, 0.0f, 1.0f, "Bias", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_Cider = {{
    AirwindowsExtractedParam{"hardns", 0.7f, 0.0f, 1.0f, "Hardns", ""},
    AirwindowsExtractedParam{"persnlty", 0.333333f, 0.0f, 1.0f, "Persnlty", ""},
    AirwindowsExtractedParam{"drive", 0.333333f, 0.0f, 1.0f, "Drive", ""},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_ClearCoat = {{
    AirwindowsExtractedParam{"select", 0.5f, 0.0f, 1.0f, "Select", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 0> kAirwindowsParams_ClipOnly = {{
}};

inline constexpr std::array<AirwindowsExtractedParam, 0> kAirwindowsParams_ClipOnly2 = {{
}};

inline constexpr std::array<AirwindowsExtractedParam, 0> kAirwindowsParams_ClipOnly3 = {{
}};

inline constexpr std::array<AirwindowsExtractedParam, 0> kAirwindowsParams_ClipSoftly = {{
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_CloudCoat = {{
    AirwindowsExtractedParam{"select", 0.5f, 0.0f, 1.0f, "Select", ""},
    AirwindowsExtractedParam{"sustain", 0.5f, 0.0f, 1.0f, "Sustain", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_Coils = {{
    AirwindowsExtractedParam{"saturat", 0.0f, 0.0f, 1.0f, "Saturat", ""},
    AirwindowsExtractedParam{"core_dc", 0.5f, 0.0f, 1.0f, "Core DC", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_Coils2 = {{
    AirwindowsExtractedParam{"saturate", 0.0f, 0.0f, 1.0f, "Saturate", ""},
    AirwindowsExtractedParam{"cheapness", 0.0f, 0.0f, 1.0f, "Cheapness", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 5> kAirwindowsParams_Cojones = {{
    AirwindowsExtractedParam{"breathy", 0.5f, 0.0f, 1.0f, "Breathy", ""},
    AirwindowsExtractedParam{"cojones", 0.5f, 0.0f, 1.0f, "Cojones", ""},
    AirwindowsExtractedParam{"body", 0.5f, 0.0f, 1.0f, "Body", ""},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 5> kAirwindowsParams_Compresaturator = {{
    AirwindowsExtractedParam{"drive", 0.5f, 0.0f, 1.0f, "Drive", "dB"},
    AirwindowsExtractedParam{"clamp", 0.5f, 0.0f, 1.0f, "Clamp", "%"},
    AirwindowsExtractedParam{"expand", 0.5f, 0.0f, 1.0f, "Expand", "sa"},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_Console0Buss = {{
    AirwindowsExtractedParam{"vol", 0.5f, 0.0f, 1.0f, "Vol", ""},
    AirwindowsExtractedParam{"pan", 0.5f, 0.0f, 1.0f, "Pan", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_Console0Channel = {{
    AirwindowsExtractedParam{"vol", 0.5f, 0.0f, 1.0f, "Vol", ""},
    AirwindowsExtractedParam{"pan", 0.5f, 0.0f, 1.0f, "Pan", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 0> kAirwindowsParams_Console4Buss = {{
}};

inline constexpr std::array<AirwindowsExtractedParam, 0> kAirwindowsParams_Console4Channel = {{
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_Console5Buss = {{
    AirwindowsExtractedParam{"input", 1.0f, 0.0f, 1.0f, "Input", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_Console5Channel = {{
    AirwindowsExtractedParam{"input", 1.0f, 0.0f, 1.0f, "Input", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_Console5DarkCh = {{
    AirwindowsExtractedParam{"input", 1.0f, 0.0f, 1.0f, "Input", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_Console6Buss = {{
    AirwindowsExtractedParam{"input", 1.0f, 0.0f, 1.0f, "Input", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_Console6Channel = {{
    AirwindowsExtractedParam{"input", 1.0f, 0.0f, 1.0f, "Input", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_Console7Buss = {{
    AirwindowsExtractedParam{"master", 1.0f, 0.0f, 1.0f, "Master", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_Console7Cascade = {{
    AirwindowsExtractedParam{"cascade", 0.39f, 0.0f, 1.0f, "Cascade", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_Console7Channel = {{
    AirwindowsExtractedParam{"fader", 0.772f, 0.0f, 1.0f, "Fader", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_Console7Crunch = {{
    AirwindowsExtractedParam{"crunch", 0.39f, 0.0f, 1.0f, "Crunch", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 0> kAirwindowsParams_Console8BussHype = {{
}};

inline constexpr std::array<AirwindowsExtractedParam, 0> kAirwindowsParams_Console8BussIn = {{
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_Console8BussOut = {{
    AirwindowsExtractedParam{"fader", 0.5f, 0.0f, 1.0f, "Fader", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 0> kAirwindowsParams_Console8ChannelHype = {{
}};

inline constexpr std::array<AirwindowsExtractedParam, 0> kAirwindowsParams_Console8ChannelIn = {{
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_Console8ChannelOut = {{
    AirwindowsExtractedParam{"fader", 0.5f, 0.0f, 1.0f, "Fader", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_Console8LiteBuss = {{
    AirwindowsExtractedParam{"fader", 0.5f, 0.0f, 1.0f, "Fader", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_Console8LiteChannel = {{
    AirwindowsExtractedParam{"fader", 0.5f, 0.0f, 1.0f, "Fader", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 0> kAirwindowsParams_Console8SubHype = {{
}};

inline constexpr std::array<AirwindowsExtractedParam, 0> kAirwindowsParams_Console8SubIn = {{
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_Console8SubOut = {{
    AirwindowsExtractedParam{"fader", 0.5f, 0.0f, 1.0f, "Fader", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_Console9Buss = {{
    AirwindowsExtractedParam{"pan", 0.5f, 0.0f, 1.0f, "Pan", ""},
    AirwindowsExtractedParam{"fader", 0.5f, 0.0f, 1.0f, "Fader", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_Console9Channel = {{
    AirwindowsExtractedParam{"pan", 0.5f, 0.0f, 1.0f, "Pan", ""},
    AirwindowsExtractedParam{"fader", 0.5f, 0.0f, 1.0f, "Fader", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 8> kAirwindowsParams_ConsoleHBuss = {{
    AirwindowsExtractedParam{"high", 0.5f, 0.0f, 1.0f, "High", "eq"},
    AirwindowsExtractedParam{"mid", 0.5f, 0.0f, 1.0f, "Mid", ""},
    AirwindowsExtractedParam{"low", 0.5f, 0.0f, 1.0f, "Low", ""},
    AirwindowsExtractedParam{"thresh", 1.0f, 0.0f, 1.0f, "Thresh", "dyn"},
    AirwindowsExtractedParam{"lowpass", 1.0f, 0.0f, 1.0f, "Lowpass", "fltr"},
    AirwindowsExtractedParam{"hipass", 0.0f, 0.0f, 1.0f, "Hipass", ""},
    AirwindowsExtractedParam{"pan", 0.5f, 0.0f, 1.0f, "Pan", ""},
    AirwindowsExtractedParam{"fader", 0.5f, 0.0f, 1.0f, "Fader", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 26> kAirwindowsParams_ConsoleHChannel = {{
    AirwindowsExtractedParam{"trim", 0.25f, 0.0f, 1.0f, "Trim", ""},
    AirwindowsExtractedParam{"more", 0.0f, 0.0f, 1.0f, "More", ""},
    AirwindowsExtractedParam{"high", 0.5f, 0.0f, 1.0f, "High", "eq"},
    AirwindowsExtractedParam{"mid", 0.5f, 0.0f, 1.0f, "Mid", ""},
    AirwindowsExtractedParam{"low", 0.5f, 0.0f, 1.0f, "Low", ""},
    AirwindowsExtractedParam{"cross_fd", 0.0f, 0.0f, 1.0f, "CrossFd", ""},
    AirwindowsExtractedParam{"tr__freq", 0.5f, 0.0f, 1.0f, "Tr Freq", "crush"},
    AirwindowsExtractedParam{"treble", 0.0f, 0.0f, 1.0f, "Treble", ""},
    AirwindowsExtractedParam{"tr_crush", 0.5f, 0.0f, 1.0f, "TrCrush", ""},
    AirwindowsExtractedParam{"hm__freq", 0.5f, 0.0f, 1.0f, "HM Freq", ""},
    AirwindowsExtractedParam{"high_mid", 0.0f, 0.0f, 1.0f, "HighMid", ""},
    AirwindowsExtractedParam{"hm_crush", 0.5f, 0.0f, 1.0f, "HMCrush", ""},
    AirwindowsExtractedParam{"lm__freq", 0.5f, 0.0f, 1.0f, "LM Freq", ""},
    AirwindowsExtractedParam{"low_mid", 0.0f, 0.0f, 1.0f, "LowMid", ""},
    AirwindowsExtractedParam{"lm_crush", 0.5f, 0.0f, 1.0f, "LMCrush", ""},
    AirwindowsExtractedParam{"bs__freq", 0.5f, 0.0f, 1.0f, "Bs Freq", ""},
    AirwindowsExtractedParam{"bass", 0.0f, 0.0f, 1.0f, "Bass", ""},
    AirwindowsExtractedParam{"bs_crush", 0.5f, 0.0f, 1.0f, "BsCrush", ""},
    AirwindowsExtractedParam{"thresh", 1.0f, 0.0f, 1.0f, "Thresh", "dyn"},
    AirwindowsExtractedParam{"attack", 0.5f, 0.0f, 1.0f, "Attack", ""},
    AirwindowsExtractedParam{"release", 0.5f, 0.0f, 1.0f, "Release", ""},
    AirwindowsExtractedParam{"gate", 0.0f, 0.0f, 1.0f, "Gate", ""},
    AirwindowsExtractedParam{"lowpass", 1.0f, 0.0f, 1.0f, "Lowpass", "fltr"},
    AirwindowsExtractedParam{"hipass", 0.0f, 0.0f, 1.0f, "Hipass", ""},
    AirwindowsExtractedParam{"pan", 0.5f, 0.0f, 1.0f, "Pan", ""},
    AirwindowsExtractedParam{"fader", 0.5f, 0.0f, 1.0f, "Fader", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 25> kAirwindowsParams_ConsoleHPre = {{
    AirwindowsExtractedParam{"trim", 0.25f, 0.0f, 1.0f, "Trim", ""},
    AirwindowsExtractedParam{"more", 0.0f, 0.0f, 1.0f, "More", ""},
    AirwindowsExtractedParam{"high", 0.5f, 0.0f, 1.0f, "High", "eq"},
    AirwindowsExtractedParam{"mid", 0.5f, 0.0f, 1.0f, "Mid", ""},
    AirwindowsExtractedParam{"low", 0.5f, 0.0f, 1.0f, "Low", ""},
    AirwindowsExtractedParam{"cross_fd", 0.0f, 0.0f, 1.0f, "CrossFd", ""},
    AirwindowsExtractedParam{"tr__freq", 0.5f, 0.0f, 1.0f, "Tr Freq", "crush"},
    AirwindowsExtractedParam{"treble", 0.0f, 0.0f, 1.0f, "Treble", ""},
    AirwindowsExtractedParam{"tr_crush", 0.5f, 0.0f, 1.0f, "TrCrush", ""},
    AirwindowsExtractedParam{"hm__freq", 0.5f, 0.0f, 1.0f, "HM Freq", ""},
    AirwindowsExtractedParam{"high_mid", 0.0f, 0.0f, 1.0f, "HighMid", ""},
    AirwindowsExtractedParam{"hm_crush", 0.5f, 0.0f, 1.0f, "HMCrush", ""},
    AirwindowsExtractedParam{"lm__freq", 0.5f, 0.0f, 1.0f, "LM Freq", ""},
    AirwindowsExtractedParam{"low_mid", 0.0f, 0.0f, 1.0f, "LowMid", ""},
    AirwindowsExtractedParam{"lm_crush", 0.5f, 0.0f, 1.0f, "LMCrush", ""},
    AirwindowsExtractedParam{"bs__freq", 0.5f, 0.0f, 1.0f, "Bs Freq", ""},
    AirwindowsExtractedParam{"bass", 0.0f, 0.0f, 1.0f, "Bass", ""},
    AirwindowsExtractedParam{"bs_crush", 0.5f, 0.0f, 1.0f, "BsCrush", ""},
    AirwindowsExtractedParam{"thresh", 1.0f, 0.0f, 1.0f, "Thresh", "dyn"},
    AirwindowsExtractedParam{"attack", 0.5f, 0.0f, 1.0f, "Attack", ""},
    AirwindowsExtractedParam{"release", 0.5f, 0.0f, 1.0f, "Release", ""},
    AirwindowsExtractedParam{"gate", 0.0f, 0.0f, 1.0f, "Gate", ""},
    AirwindowsExtractedParam{"lowpass", 1.0f, 0.0f, 1.0f, "Lowpass", "fltr"},
    AirwindowsExtractedParam{"hipass", 0.0f, 0.0f, 1.0f, "Hipass", ""},
    AirwindowsExtractedParam{"fader", 0.5f, 0.0f, 1.0f, "Fader", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_ConsoleLABuss = {{
    AirwindowsExtractedParam{"master", 1.0f, 0.0f, 1.0f, "Master", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 5> kAirwindowsParams_ConsoleLAChannel = {{
    AirwindowsExtractedParam{"treble", 0.5f, 0.0f, 1.0f, "Treble", ""},
    AirwindowsExtractedParam{"mid", 0.5f, 0.0f, 1.0f, "Mid", ""},
    AirwindowsExtractedParam{"bass", 0.5f, 0.0f, 1.0f, "Bass", ""},
    AirwindowsExtractedParam{"pan", 0.5f, 0.0f, 1.0f, "Pan", ""},
    AirwindowsExtractedParam{"fader", 0.5f, 0.0f, 1.0f, "Fader", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_ConsoleMCBuss = {{
    AirwindowsExtractedParam{"master", 1.0f, 0.0f, 1.0f, "Master", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 6> kAirwindowsParams_ConsoleMCChannel = {{
    AirwindowsExtractedParam{"treble", 0.5f, 0.0f, 1.0f, "Treble", ""},
    AirwindowsExtractedParam{"mid_freq", 0.25f, 0.0f, 1.0f, "MidFreq", ""},
    AirwindowsExtractedParam{"mid_peak", 0.0f, 0.0f, 1.0f, "MidPeak", ""},
    AirwindowsExtractedParam{"bass", 0.5f, 0.0f, 1.0f, "Bass", ""},
    AirwindowsExtractedParam{"pan", 0.5f, 0.0f, 1.0f, "Pan", ""},
    AirwindowsExtractedParam{"fader", 0.5f, 0.0f, 1.0f, "Fader", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_ConsoleMDBuss = {{
    AirwindowsExtractedParam{"master", 1.0f, 0.0f, 1.0f, "Master", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 6> kAirwindowsParams_ConsoleMDChannel = {{
    AirwindowsExtractedParam{"treble", 0.5f, 0.0f, 1.0f, "Treble", ""},
    AirwindowsExtractedParam{"mid_freq", 0.25f, 0.0f, 1.0f, "MidFreq", ""},
    AirwindowsExtractedParam{"mid_peak", 0.0f, 0.0f, 1.0f, "MidPeak", ""},
    AirwindowsExtractedParam{"bass", 0.5f, 0.0f, 1.0f, "Bass", ""},
    AirwindowsExtractedParam{"pan", 0.5f, 0.0f, 1.0f, "Pan", ""},
    AirwindowsExtractedParam{"fader", 0.5f, 0.0f, 1.0f, "Fader", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 11> kAirwindowsParams_ConsoleX2Buss = {{
    AirwindowsExtractedParam{"high", 0.5f, 0.0f, 1.0f, "High", "eq"},
    AirwindowsExtractedParam{"h_mid", 0.5f, 0.0f, 1.0f, "HMid", ""},
    AirwindowsExtractedParam{"l_mid", 0.5f, 0.0f, 1.0f, "LMid", ""},
    AirwindowsExtractedParam{"bass", 0.5f, 0.0f, 1.0f, "Bass", ""},
    AirwindowsExtractedParam{"high_f", 0.5f, 0.0f, 1.0f, "HighF", "freq"},
    AirwindowsExtractedParam{"h_mid_f", 0.5f, 0.0f, 1.0f, "HMidF", ""},
    AirwindowsExtractedParam{"l_mid_f", 0.5f, 0.0f, 1.0f, "LMidF", ""},
    AirwindowsExtractedParam{"bass_f", 0.5f, 0.0f, 1.0f, "BassF", ""},
    AirwindowsExtractedParam{"thresh", 1.0f, 0.0f, 1.0f, "Thresh", "dyn"},
    AirwindowsExtractedParam{"pan", 0.5f, 0.0f, 1.0f, "Pan", ""},
    AirwindowsExtractedParam{"fader", 0.5f, 0.0f, 1.0f, "Fader", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 18> kAirwindowsParams_ConsoleX2Channel = {{
    AirwindowsExtractedParam{"trim", 0.25f, 0.0f, 1.0f, "Trim", ""},
    AirwindowsExtractedParam{"more", 0.0f, 0.0f, 1.0f, "More", ""},
    AirwindowsExtractedParam{"high", 0.5f, 0.0f, 1.0f, "High", "eq"},
    AirwindowsExtractedParam{"h_mid", 0.5f, 0.0f, 1.0f, "HMid", ""},
    AirwindowsExtractedParam{"l_mid", 0.5f, 0.0f, 1.0f, "LMid", ""},
    AirwindowsExtractedParam{"bass", 0.5f, 0.0f, 1.0f, "Bass", ""},
    AirwindowsExtractedParam{"high_f", 0.5f, 0.0f, 1.0f, "HighF", "freq"},
    AirwindowsExtractedParam{"h_mid_f", 0.5f, 0.0f, 1.0f, "HMidF", ""},
    AirwindowsExtractedParam{"l_mid_f", 0.5f, 0.0f, 1.0f, "LMidF", ""},
    AirwindowsExtractedParam{"bass_f", 0.5f, 0.0f, 1.0f, "BassF", ""},
    AirwindowsExtractedParam{"thresh", 1.0f, 0.0f, 1.0f, "Thresh", "dyn"},
    AirwindowsExtractedParam{"attack", 0.5f, 0.0f, 1.0f, "Attack", ""},
    AirwindowsExtractedParam{"release", 0.5f, 0.0f, 1.0f, "Release", ""},
    AirwindowsExtractedParam{"gate", 0.0f, 0.0f, 1.0f, "Gate", ""},
    AirwindowsExtractedParam{"lowpass", 1.0f, 0.0f, 1.0f, "Lowpass", "fltr"},
    AirwindowsExtractedParam{"hipass", 0.0f, 0.0f, 1.0f, "Hipass", ""},
    AirwindowsExtractedParam{"pan", 0.5f, 0.0f, 1.0f, "Pan", ""},
    AirwindowsExtractedParam{"fader", 0.5f, 0.0f, 1.0f, "Fader", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 17> kAirwindowsParams_ConsoleX2Pre = {{
    AirwindowsExtractedParam{"trim", 0.25f, 0.0f, 1.0f, "Trim", ""},
    AirwindowsExtractedParam{"more", 0.0f, 0.0f, 1.0f, "More", ""},
    AirwindowsExtractedParam{"high", 0.5f, 0.0f, 1.0f, "High", "eq"},
    AirwindowsExtractedParam{"h_mid", 0.5f, 0.0f, 1.0f, "HMid", ""},
    AirwindowsExtractedParam{"l_mid", 0.5f, 0.0f, 1.0f, "LMid", ""},
    AirwindowsExtractedParam{"bass", 0.5f, 0.0f, 1.0f, "Bass", ""},
    AirwindowsExtractedParam{"high_f", 0.5f, 0.0f, 1.0f, "HighF", "freq"},
    AirwindowsExtractedParam{"h_mid_f", 0.5f, 0.0f, 1.0f, "HMidF", ""},
    AirwindowsExtractedParam{"l_mid_f", 0.5f, 0.0f, 1.0f, "LMidF", ""},
    AirwindowsExtractedParam{"bass_f", 0.5f, 0.0f, 1.0f, "BassF", ""},
    AirwindowsExtractedParam{"thresh", 1.0f, 0.0f, 1.0f, "Thresh", "dyn"},
    AirwindowsExtractedParam{"attack", 0.5f, 0.0f, 1.0f, "Attack", ""},
    AirwindowsExtractedParam{"release", 0.5f, 0.0f, 1.0f, "Release", ""},
    AirwindowsExtractedParam{"gate", 0.0f, 0.0f, 1.0f, "Gate", ""},
    AirwindowsExtractedParam{"lowpass", 1.0f, 0.0f, 1.0f, "Lowpass", "fltr"},
    AirwindowsExtractedParam{"hipass", 0.0f, 0.0f, 1.0f, "Hipass", ""},
    AirwindowsExtractedParam{"fader", 0.5f, 0.0f, 1.0f, "Fader", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 0> kAirwindowsParams_ConsoleX3 = {{
}};

inline constexpr std::array<AirwindowsExtractedParam, 21> kAirwindowsParams_ConsoleX3Buss = {{
    AirwindowsExtractedParam{"smooth", 0.5f, 0.0f, 1.0f, "Smooth", ""},
    AirwindowsExtractedParam{"input", 0.25f, 0.0f, 1.0f, "Input", "trim"},
    AirwindowsExtractedParam{"high", 0.5f, 0.0f, 1.0f, "High", "eq"},
    AirwindowsExtractedParam{"h_mid", 0.5f, 0.0f, 1.0f, "HMid", ""},
    AirwindowsExtractedParam{"l_mid", 0.5f, 0.0f, 1.0f, "LMid", ""},
    AirwindowsExtractedParam{"bass", 0.5f, 0.0f, 1.0f, "Bass", ""},
    AirwindowsExtractedParam{"high_f", 0.5f, 0.0f, 1.0f, "HighF", "freq"},
    AirwindowsExtractedParam{"h_mid_f", 0.5f, 0.0f, 1.0f, "HMidF", ""},
    AirwindowsExtractedParam{"l_mid_f", 0.5f, 0.0f, 1.0f, "LMidF", ""},
    AirwindowsExtractedParam{"bass_f", 0.5f, 0.0f, 1.0f, "BassF", ""},
    AirwindowsExtractedParam{"thresh", 1.0f, 0.0f, 1.0f, "Thresh", "dyn"},
    AirwindowsExtractedParam{"attack", 0.382f, 0.0f, 1.0f, "Attack", ""},
    AirwindowsExtractedParam{"release", 0.618f, 0.0f, 1.0f, "Release", ""},
    AirwindowsExtractedParam{"inv__wet", 1.0f, 0.0f, 1.0f, "Inv/Wet", ""},
    AirwindowsExtractedParam{"more", 0.0f, 0.0f, 1.0f, "More", "tape"},
    AirwindowsExtractedParam{"lowpass", 1.0f, 0.0f, 1.0f, "Lowpass", ""},
    AirwindowsExtractedParam{"lowpas_q", 0.0f, 0.0f, 1.0f, "LowpasQ", ""},
    AirwindowsExtractedParam{"hipass", 0.0f, 0.0f, 1.0f, "Hipass", ""},
    AirwindowsExtractedParam{"hipass_q", 0.0f, 0.0f, 1.0f, "HipassQ", ""},
    AirwindowsExtractedParam{"pan", 0.5f, 0.0f, 1.0f, "Pan", ""},
    AirwindowsExtractedParam{"fader", 1.0f, 0.0f, 1.0f, "Fader", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 21> kAirwindowsParams_ConsoleX3Channel = {{
    AirwindowsExtractedParam{"smooth", 0.5f, 0.0f, 1.0f, "Smooth", ""},
    AirwindowsExtractedParam{"input", 0.25f, 0.0f, 1.0f, "Input", "trim"},
    AirwindowsExtractedParam{"high", 0.5f, 0.0f, 1.0f, "High", "eq"},
    AirwindowsExtractedParam{"h_mid", 0.5f, 0.0f, 1.0f, "HMid", ""},
    AirwindowsExtractedParam{"l_mid", 0.5f, 0.0f, 1.0f, "LMid", ""},
    AirwindowsExtractedParam{"bass", 0.5f, 0.0f, 1.0f, "Bass", ""},
    AirwindowsExtractedParam{"high_f", 0.5f, 0.0f, 1.0f, "HighF", "freq"},
    AirwindowsExtractedParam{"h_mid_f", 0.5f, 0.0f, 1.0f, "HMidF", ""},
    AirwindowsExtractedParam{"l_mid_f", 0.5f, 0.0f, 1.0f, "LMidF", ""},
    AirwindowsExtractedParam{"bass_f", 0.5f, 0.0f, 1.0f, "BassF", ""},
    AirwindowsExtractedParam{"thresh", 1.0f, 0.0f, 1.0f, "Thresh", "dyn"},
    AirwindowsExtractedParam{"attack", 0.382f, 0.0f, 1.0f, "Attack", ""},
    AirwindowsExtractedParam{"release", 0.618f, 0.0f, 1.0f, "Release", ""},
    AirwindowsExtractedParam{"inv__wet", 1.0f, 0.0f, 1.0f, "Inv/Wet", ""},
    AirwindowsExtractedParam{"more", 0.0f, 0.0f, 1.0f, "More", "tape"},
    AirwindowsExtractedParam{"lowpass", 1.0f, 0.0f, 1.0f, "Lowpass", ""},
    AirwindowsExtractedParam{"lowpas_q", 0.0f, 0.0f, 1.0f, "LowpasQ", ""},
    AirwindowsExtractedParam{"hipass", 0.0f, 0.0f, 1.0f, "Hipass", ""},
    AirwindowsExtractedParam{"hipass_q", 0.0f, 0.0f, 1.0f, "HipassQ", ""},
    AirwindowsExtractedParam{"pan", 0.5f, 0.0f, 1.0f, "Pan", ""},
    AirwindowsExtractedParam{"fader", 1.0f, 0.0f, 1.0f, "Fader", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 21> kAirwindowsParams_ConsoleX3Pre = {{
    AirwindowsExtractedParam{"smooth", 0.5f, 0.0f, 1.0f, "Smooth", ""},
    AirwindowsExtractedParam{"input", 0.25f, 0.0f, 1.0f, "Input", "trim"},
    AirwindowsExtractedParam{"high", 0.5f, 0.0f, 1.0f, "High", "eq"},
    AirwindowsExtractedParam{"h_mid", 0.5f, 0.0f, 1.0f, "HMid", ""},
    AirwindowsExtractedParam{"l_mid", 0.5f, 0.0f, 1.0f, "LMid", ""},
    AirwindowsExtractedParam{"bass", 0.5f, 0.0f, 1.0f, "Bass", ""},
    AirwindowsExtractedParam{"high_f", 0.5f, 0.0f, 1.0f, "HighF", "freq"},
    AirwindowsExtractedParam{"h_mid_f", 0.5f, 0.0f, 1.0f, "HMidF", ""},
    AirwindowsExtractedParam{"l_mid_f", 0.5f, 0.0f, 1.0f, "LMidF", ""},
    AirwindowsExtractedParam{"bass_f", 0.5f, 0.0f, 1.0f, "BassF", ""},
    AirwindowsExtractedParam{"thresh", 1.0f, 0.0f, 1.0f, "Thresh", "dyn"},
    AirwindowsExtractedParam{"attack", 0.382f, 0.0f, 1.0f, "Attack", ""},
    AirwindowsExtractedParam{"release", 0.618f, 0.0f, 1.0f, "Release", ""},
    AirwindowsExtractedParam{"inv__wet", 1.0f, 0.0f, 1.0f, "Inv/Wet", ""},
    AirwindowsExtractedParam{"more", 0.0f, 0.0f, 1.0f, "More", "tape"},
    AirwindowsExtractedParam{"lowpass", 1.0f, 0.0f, 1.0f, "Lowpass", ""},
    AirwindowsExtractedParam{"lowpas_q", 0.0f, 0.0f, 1.0f, "LowpasQ", ""},
    AirwindowsExtractedParam{"hipass", 0.0f, 0.0f, 1.0f, "Hipass", ""},
    AirwindowsExtractedParam{"hipass_q", 0.0f, 0.0f, 1.0f, "HipassQ", ""},
    AirwindowsExtractedParam{"pan", 0.5f, 0.0f, 1.0f, "Pan", ""},
    AirwindowsExtractedParam{"fader", 1.0f, 0.0f, 1.0f, "Fader", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 37> kAirwindowsParams_ConsoleXBuss = {{
    AirwindowsExtractedParam{"highpas", 0.0f, 0.0f, 1.0f, "Highpas", ""},
    AirwindowsExtractedParam{"lowpass", 0.0f, 0.0f, 1.0f, "Lowpass", ""},
    AirwindowsExtractedParam{"air", 0.5f, 0.0f, 1.0f, "Air", ""},
    AirwindowsExtractedParam{"fire", 0.5f, 0.0f, 1.0f, "Fire", ""},
    AirwindowsExtractedParam{"stone", 0.5f, 0.0f, 1.0f, "Stone", ""},
    AirwindowsExtractedParam{"range", 0.5f, 0.0f, 1.0f, "Range", ""},
    AirwindowsExtractedParam{"fc__thrs", 1.0f, 0.0f, 1.0f, "FC Thrs", ""},
    AirwindowsExtractedParam{"sc__thrs", 1.0f, 0.0f, 1.0f, "SC Thrs", ""},
    AirwindowsExtractedParam{"fc__rati", 1.0f, 0.0f, 1.0f, "FC Rati", ""},
    AirwindowsExtractedParam{"sc__rati", 1.0f, 0.0f, 1.0f, "SC Rati", ""},
    AirwindowsExtractedParam{"fc__atk", 0.5f, 0.0f, 1.0f, "FC Atk", ""},
    AirwindowsExtractedParam{"sc__atk", 0.5f, 0.0f, 1.0f, "SC Atk", ""},
    AirwindowsExtractedParam{"fc__rls", 0.5f, 0.0f, 1.0f, "FC Rls", ""},
    AirwindowsExtractedParam{"sc__rls", 0.5f, 0.0f, 1.0f, "SC Rls", ""},
    AirwindowsExtractedParam{"fg__thrs", 0.0f, 0.0f, 1.0f, "FG Thrs", ""},
    AirwindowsExtractedParam{"sg__thrs", 0.0f, 0.0f, 1.0f, "SG Thrs", ""},
    AirwindowsExtractedParam{"fg__rati", 1.0f, 0.0f, 1.0f, "FG Rati", ""},
    AirwindowsExtractedParam{"sg__rati", 1.0f, 0.0f, 1.0f, "SG Rati", ""},
    AirwindowsExtractedParam{"fg__sust", 0.5f, 0.0f, 1.0f, "FG Sust", ""},
    AirwindowsExtractedParam{"sg__sust", 0.5f, 0.0f, 1.0f, "SG Sust", ""},
    AirwindowsExtractedParam{"fg__rls", 0.5f, 0.0f, 1.0f, "FG Rls", ""},
    AirwindowsExtractedParam{"sg__rls", 0.5f, 0.0f, 1.0f, "SG Rls", ""},
    AirwindowsExtractedParam{"tr__freq", 0.5f, 0.0f, 1.0f, "Tr Freq", ""},
    AirwindowsExtractedParam{"treble", 0.5f, 0.0f, 1.0f, "Treble", ""},
    AirwindowsExtractedParam{"tr__reso", 0.5f, 0.0f, 1.0f, "Tr Reso", ""},
    AirwindowsExtractedParam{"hm__freq", 0.5f, 0.0f, 1.0f, "HM Freq", ""},
    AirwindowsExtractedParam{"high_mid", 0.5f, 0.0f, 1.0f, "HighMid", ""},
    AirwindowsExtractedParam{"hm__reso", 0.5f, 0.0f, 1.0f, "HM Reso", ""},
    AirwindowsExtractedParam{"lm__freq", 0.5f, 0.0f, 1.0f, "LM Freq", ""},
    AirwindowsExtractedParam{"low_mid", 0.5f, 0.0f, 1.0f, "LowMid", ""},
    AirwindowsExtractedParam{"lm__reso", 0.5f, 0.0f, 1.0f, "LM Reso", ""},
    AirwindowsExtractedParam{"bs__freq", 0.5f, 0.0f, 1.0f, "Bs Freq", ""},
    AirwindowsExtractedParam{"bass", 0.5f, 0.0f, 1.0f, "Bass", ""},
    AirwindowsExtractedParam{"bs__reso", 0.5f, 0.0f, 1.0f, "Bs Reso", ""},
    AirwindowsExtractedParam{"top_d_b", 0.5f, 0.0f, 1.0f, "Top dB", "dB"},
    AirwindowsExtractedParam{"pan", 0.5f, 0.0f, 1.0f, "Pan", ""},
    AirwindowsExtractedParam{"fader", 0.5f, 0.0f, 1.0f, "Fader", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 37> kAirwindowsParams_ConsoleXChannel = {{
    AirwindowsExtractedParam{"highpas", 0.0f, 0.0f, 1.0f, "Highpas", ""},
    AirwindowsExtractedParam{"lowpass", 0.0f, 0.0f, 1.0f, "Lowpass", ""},
    AirwindowsExtractedParam{"air", 0.5f, 0.0f, 1.0f, "Air", ""},
    AirwindowsExtractedParam{"fire", 0.5f, 0.0f, 1.0f, "Fire", ""},
    AirwindowsExtractedParam{"stone", 0.5f, 0.0f, 1.0f, "Stone", ""},
    AirwindowsExtractedParam{"range", 0.5f, 0.0f, 1.0f, "Range", ""},
    AirwindowsExtractedParam{"fc__thrs", 1.0f, 0.0f, 1.0f, "FC Thrs", ""},
    AirwindowsExtractedParam{"sc__thrs", 1.0f, 0.0f, 1.0f, "SC Thrs", ""},
    AirwindowsExtractedParam{"fc__rati", 1.0f, 0.0f, 1.0f, "FC Rati", ""},
    AirwindowsExtractedParam{"sc__rati", 1.0f, 0.0f, 1.0f, "SC Rati", ""},
    AirwindowsExtractedParam{"fc__atk", 0.5f, 0.0f, 1.0f, "FC Atk", ""},
    AirwindowsExtractedParam{"sc__atk", 0.5f, 0.0f, 1.0f, "SC Atk", ""},
    AirwindowsExtractedParam{"fc__rls", 0.5f, 0.0f, 1.0f, "FC Rls", ""},
    AirwindowsExtractedParam{"sc__rls", 0.5f, 0.0f, 1.0f, "SC Rls", ""},
    AirwindowsExtractedParam{"fg__thrs", 0.0f, 0.0f, 1.0f, "FG Thrs", ""},
    AirwindowsExtractedParam{"sg__thrs", 0.0f, 0.0f, 1.0f, "SG Thrs", ""},
    AirwindowsExtractedParam{"fg__rati", 1.0f, 0.0f, 1.0f, "FG Rati", ""},
    AirwindowsExtractedParam{"sg__rati", 1.0f, 0.0f, 1.0f, "SG Rati", ""},
    AirwindowsExtractedParam{"fg__sust", 0.5f, 0.0f, 1.0f, "FG Sust", ""},
    AirwindowsExtractedParam{"sg__sust", 0.5f, 0.0f, 1.0f, "SG Sust", ""},
    AirwindowsExtractedParam{"fg__rls", 0.5f, 0.0f, 1.0f, "FG Rls", ""},
    AirwindowsExtractedParam{"sg__rls", 0.5f, 0.0f, 1.0f, "SG Rls", ""},
    AirwindowsExtractedParam{"tr__freq", 0.5f, 0.0f, 1.0f, "Tr Freq", ""},
    AirwindowsExtractedParam{"treble", 0.5f, 0.0f, 1.0f, "Treble", ""},
    AirwindowsExtractedParam{"tr__reso", 0.5f, 0.0f, 1.0f, "Tr Reso", ""},
    AirwindowsExtractedParam{"hm__freq", 0.5f, 0.0f, 1.0f, "HM Freq", ""},
    AirwindowsExtractedParam{"high_mid", 0.5f, 0.0f, 1.0f, "HighMid", ""},
    AirwindowsExtractedParam{"hm__reso", 0.5f, 0.0f, 1.0f, "HM Reso", ""},
    AirwindowsExtractedParam{"lm__freq", 0.5f, 0.0f, 1.0f, "LM Freq", ""},
    AirwindowsExtractedParam{"low_mid", 0.5f, 0.0f, 1.0f, "LowMid", ""},
    AirwindowsExtractedParam{"lm__reso", 0.5f, 0.0f, 1.0f, "LM Reso", ""},
    AirwindowsExtractedParam{"bs__freq", 0.5f, 0.0f, 1.0f, "Bs Freq", ""},
    AirwindowsExtractedParam{"bass", 0.5f, 0.0f, 1.0f, "Bass", ""},
    AirwindowsExtractedParam{"bs__reso", 0.5f, 0.0f, 1.0f, "Bs Reso", ""},
    AirwindowsExtractedParam{"top_d_b", 0.5f, 0.0f, 1.0f, "Top dB", "dB"},
    AirwindowsExtractedParam{"pan", 0.5f, 0.0f, 1.0f, "Pan", ""},
    AirwindowsExtractedParam{"fader", 0.5f, 0.0f, 1.0f, "Fader", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 37> kAirwindowsParams_ConsoleXPre = {{
    AirwindowsExtractedParam{"highpas", 0.0f, 0.0f, 1.0f, "Highpas", ""},
    AirwindowsExtractedParam{"lowpass", 0.0f, 0.0f, 1.0f, "Lowpass", ""},
    AirwindowsExtractedParam{"air", 0.5f, 0.0f, 1.0f, "Air", ""},
    AirwindowsExtractedParam{"fire", 0.5f, 0.0f, 1.0f, "Fire", ""},
    AirwindowsExtractedParam{"stone", 0.5f, 0.0f, 1.0f, "Stone", ""},
    AirwindowsExtractedParam{"range", 0.5f, 0.0f, 1.0f, "Range", ""},
    AirwindowsExtractedParam{"fc__thrs", 1.0f, 0.0f, 1.0f, "FC Thrs", ""},
    AirwindowsExtractedParam{"sc__thrs", 1.0f, 0.0f, 1.0f, "SC Thrs", ""},
    AirwindowsExtractedParam{"fc__rati", 1.0f, 0.0f, 1.0f, "FC Rati", ""},
    AirwindowsExtractedParam{"sc__rati", 1.0f, 0.0f, 1.0f, "SC Rati", ""},
    AirwindowsExtractedParam{"fc__atk", 0.5f, 0.0f, 1.0f, "FC Atk", ""},
    AirwindowsExtractedParam{"sc__atk", 0.5f, 0.0f, 1.0f, "SC Atk", ""},
    AirwindowsExtractedParam{"fc__rls", 0.5f, 0.0f, 1.0f, "FC Rls", ""},
    AirwindowsExtractedParam{"sc__rls", 0.5f, 0.0f, 1.0f, "SC Rls", ""},
    AirwindowsExtractedParam{"fg__thrs", 0.0f, 0.0f, 1.0f, "FG Thrs", ""},
    AirwindowsExtractedParam{"sg__thrs", 0.0f, 0.0f, 1.0f, "SG Thrs", ""},
    AirwindowsExtractedParam{"fg__rati", 1.0f, 0.0f, 1.0f, "FG Rati", ""},
    AirwindowsExtractedParam{"sg__rati", 1.0f, 0.0f, 1.0f, "SG Rati", ""},
    AirwindowsExtractedParam{"fg__sust", 0.5f, 0.0f, 1.0f, "FG Sust", ""},
    AirwindowsExtractedParam{"sg__sust", 0.5f, 0.0f, 1.0f, "SG Sust", ""},
    AirwindowsExtractedParam{"fg__rls", 0.5f, 0.0f, 1.0f, "FG Rls", ""},
    AirwindowsExtractedParam{"sg__rls", 0.5f, 0.0f, 1.0f, "SG Rls", ""},
    AirwindowsExtractedParam{"tr__freq", 0.5f, 0.0f, 1.0f, "Tr Freq", ""},
    AirwindowsExtractedParam{"treble", 0.5f, 0.0f, 1.0f, "Treble", ""},
    AirwindowsExtractedParam{"tr__reso", 0.5f, 0.0f, 1.0f, "Tr Reso", ""},
    AirwindowsExtractedParam{"hm__freq", 0.5f, 0.0f, 1.0f, "HM Freq", ""},
    AirwindowsExtractedParam{"high_mid", 0.5f, 0.0f, 1.0f, "HighMid", ""},
    AirwindowsExtractedParam{"hm__reso", 0.5f, 0.0f, 1.0f, "HM Reso", ""},
    AirwindowsExtractedParam{"lm__freq", 0.5f, 0.0f, 1.0f, "LM Freq", ""},
    AirwindowsExtractedParam{"low_mid", 0.5f, 0.0f, 1.0f, "LowMid", ""},
    AirwindowsExtractedParam{"lm__reso", 0.5f, 0.0f, 1.0f, "LM Reso", ""},
    AirwindowsExtractedParam{"bs__freq", 0.5f, 0.0f, 1.0f, "Bs Freq", ""},
    AirwindowsExtractedParam{"bass", 0.5f, 0.0f, 1.0f, "Bass", ""},
    AirwindowsExtractedParam{"bs__reso", 0.5f, 0.0f, 1.0f, "Bs Reso", ""},
    AirwindowsExtractedParam{"top_d_b", 0.5f, 0.0f, 1.0f, "Top dB", "dB"},
    AirwindowsExtractedParam{"pan", 0.5f, 0.0f, 1.0f, "Pan", ""},
    AirwindowsExtractedParam{"fader", 0.5f, 0.0f, 1.0f, "Fader", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 0> kAirwindowsParams_ContentHideD = {{
}};

inline constexpr std::array<AirwindowsExtractedParam, 5> kAirwindowsParams_CreamCoat = {{
    AirwindowsExtractedParam{"select", 0.5f, 0.0f, 1.0f, "Select", ""},
    AirwindowsExtractedParam{"regen", 0.5f, 0.0f, 1.0f, "Regen", ""},
    AirwindowsExtractedParam{"de_rez", 1.0f, 0.0f, 1.0f, "DeRez", ""},
    AirwindowsExtractedParam{"predlay", 0.0f, 0.0f, 1.0f, "Predlay", ""},
    AirwindowsExtractedParam{"wetness", 0.25f, 0.0f, 1.0f, "Wetness", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_Creature = {{
    AirwindowsExtractedParam{"drive", 0.26f, 0.0f, 1.0f, "Drive", ""},
    AirwindowsExtractedParam{"depth", 0.26f, 0.0f, 1.0f, "Depth", ""},
    AirwindowsExtractedParam{"inv__wet", 1.0f, 0.0f, 1.0f, "Inv/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_CrickBass = {{
    AirwindowsExtractedParam{"drive", 0.5f, 0.0f, 1.0f, "Drive", ""},
    AirwindowsExtractedParam{"tone", 0.5f, 0.0f, 1.0f, "Tone", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_CrunchCoat = {{
    AirwindowsExtractedParam{"select", 0.5f, 0.0f, 1.0f, "Select", ""},
    AirwindowsExtractedParam{"regen", 0.0f, 0.0f, 1.0f, "Regen", ""},
    AirwindowsExtractedParam{"de_rez", 1.0f, 0.0f, 1.0f, "DeRez", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_CrunchyGrooveWear = {{
    AirwindowsExtractedParam{"freq", 0.064f, 0.0f, 1.0f, "Freq", ""},
    AirwindowsExtractedParam{"apply", 1.0f, 0.0f, 1.0f, "Apply", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_Crystal = {{
    AirwindowsExtractedParam{"hardnss", 0.7f, 0.0f, 1.0f, "Hardnss", ""},
    AirwindowsExtractedParam{"personl", 1.0f, 0.0f, 1.0f, "Personl", ""},
    AirwindowsExtractedParam{"drive", 0.333f, 0.0f, 1.0f, "Drive", ""},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 12> kAirwindowsParams_CStrip = {{
    AirwindowsExtractedParam{"treble", 0.5f, 0.0f, 1.0f, "Treble", "dB"},
    AirwindowsExtractedParam{"mid", 0.5f, 0.0f, 1.0f, "Mid", "dB"},
    AirwindowsExtractedParam{"bass", 0.5f, 0.0f, 1.0f, "Bass", "dB"},
    AirwindowsExtractedParam{"lowpass", 1.0f, 0.0f, 1.0f, "Lowpass", "Khz"},
    AirwindowsExtractedParam{"treb_frq", 0.4f, 0.0f, 1.0f, "TrebFrq", "Khz"},
    AirwindowsExtractedParam{"bass_frq", 0.4f, 0.0f, 1.0f, "BassFrq", "hz"},
    AirwindowsExtractedParam{"hipass", 0.0f, 0.0f, 1.0f, "Hipass", "hz"},
    AirwindowsExtractedParam{"gate", 0.0f, 0.0f, 1.0f, "Gate", ""},
    AirwindowsExtractedParam{"compres", 0.0f, 0.0f, 1.0f, "Compres", ""},
    AirwindowsExtractedParam{"comp_spd", 0.0f, 0.0f, 1.0f, "CompSpd", ""},
    AirwindowsExtractedParam{"time_lag", 0.0f, 0.0f, 1.0f, "TimeLag", ""},
    AirwindowsExtractedParam{"out_gain", 0.5f, 0.0f, 1.0f, "OutGain", "dB"},
}};

inline constexpr std::array<AirwindowsExtractedParam, 10> kAirwindowsParams_CStrip2 = {{
    AirwindowsExtractedParam{"treble", 0.5f, 0.0f, 1.0f, "Treble", "dB"},
    AirwindowsExtractedParam{"mid", 0.5f, 0.0f, 1.0f, "Mid", "dB"},
    AirwindowsExtractedParam{"bass", 0.5f, 0.0f, 1.0f, "Bass", "dB"},
    AirwindowsExtractedParam{"treb_frq", 0.5f, 0.0f, 1.0f, "TrebFrq", "Khz"},
    AirwindowsExtractedParam{"bass_frq", 0.5f, 0.0f, 1.0f, "BassFrq", "hz"},
    AirwindowsExtractedParam{"low_cap", 1.0f, 0.0f, 1.0f, "LowCap", ""},
    AirwindowsExtractedParam{"hi_cap", 0.0f, 0.0f, 1.0f, "HiCap", ""},
    AirwindowsExtractedParam{"compres", 0.0f, 0.0f, 1.0f, "Compres", ""},
    AirwindowsExtractedParam{"comp_spd", 0.0f, 0.0f, 1.0f, "CompSpd", ""},
    AirwindowsExtractedParam{"output", 0.33f, 0.0f, 1.0f, "Output", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 0> kAirwindowsParams_curve = {{
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_Dark = {{
    AirwindowsExtractedParam{"quant", 1.0f, 0.0f, 1.0f, "Quant", ""},
    AirwindowsExtractedParam{"de_rez", 0.0f, 0.0f, 1.0f, "DeRez", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_DarkNoise = {{
    AirwindowsExtractedParam{"freq", 0.5f, 0.0f, 1.0f, "Freq", ""},
    AirwindowsExtractedParam{"dark", 1.0f, 0.0f, 1.0f, "Dark", ""},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_Dattorro = {{
    AirwindowsExtractedParam{"freq", 0.5f, 0.0f, 1.0f, "Freq", ""},
    AirwindowsExtractedParam{"reso", 0.5f, 0.0f, 1.0f, "Reso", ""},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_DCVoltage = {{
    AirwindowsExtractedParam{"voltage", 0.5f, 0.0f, 1.0f, "Voltage", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 5> kAirwindowsParams_DeBess = {{
    AirwindowsExtractedParam{"intense", 0.0f, 0.0f, 1.0f, "Intense", ""},
    AirwindowsExtractedParam{"sharp", 0.5f, 0.0f, 1.0f, "Sharp", ""},
    AirwindowsExtractedParam{"depth", 0.5f, 0.0f, 1.0f, "Depth", ""},
    AirwindowsExtractedParam{"filter", 0.5f, 0.0f, 1.0f, "Filter", ""},
    AirwindowsExtractedParam{"sense", 0.0f, 0.0f, 1.0f, "Sense", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_DeBez = {{
    AirwindowsExtractedParam{"de_bez", 0.5f, 0.0f, 1.0f, "DeBez", ""},
    AirwindowsExtractedParam{"de_rez", 0.5f, 0.0f, 1.0f, "DeRez", ""},
    AirwindowsExtractedParam{"inv__wet", 1.0f, 0.0f, 1.0f, "Inv/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_Deckwrecka = {{
    AirwindowsExtractedParam{"wreck", 0.0f, 0.0f, 1.0f, "Wreck", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 5> kAirwindowsParams_DeCrackle = {{
    AirwindowsExtractedParam{"filter", 0.5f, 0.0f, 1.0f, "Filter", ""},
    AirwindowsExtractedParam{"window", 0.5f, 0.0f, 1.0f, "Window", ""},
    AirwindowsExtractedParam{"thresld", 0.5f, 0.0f, 1.0f, "Thresld", ""},
    AirwindowsExtractedParam{"surface", 0.5f, 0.0f, 1.0f, "Surface", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_DeEss = {{
    AirwindowsExtractedParam{"intense", 0.0f, 0.0f, 1.0f, "Intense", ""},
    AirwindowsExtractedParam{"max_ds", 0.5f, 0.0f, 1.0f, "Max DS", ""},
    AirwindowsExtractedParam{"freq", 0.5f, 0.0f, 1.0f, "Freq", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_DeHiss = {{
    AirwindowsExtractedParam{"thresh", 0.0f, 0.0f, 1.0f, "Thresh", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 8> kAirwindowsParams_DeNoise = {{
    AirwindowsExtractedParam{"high", 0.0f, 0.0f, 1.0f, "High", ""},
    AirwindowsExtractedParam{"h_mid", 0.0f, 0.0f, 1.0f, "HMid", ""},
    AirwindowsExtractedParam{"mid", 0.0f, 0.0f, 1.0f, "Mid", ""},
    AirwindowsExtractedParam{"l_mid", 0.0f, 0.0f, 1.0f, "LMid", ""},
    AirwindowsExtractedParam{"bass", 0.0f, 0.0f, 1.0f, "Bass", ""},
    AirwindowsExtractedParam{"sub", 0.0f, 0.0f, 1.0f, "Sub", ""},
    AirwindowsExtractedParam{"speed", 0.25f, 0.0f, 1.0f, "Speed", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_Density = {{
    AirwindowsExtractedParam{"density", 0.2f, 0.0f, 1.0f, "Density", ""},
    AirwindowsExtractedParam{"highpass", 0.0f, 0.0f, 1.0f, "Highpass", ""},
    AirwindowsExtractedParam{"out__level", 1.0f, 0.0f, 1.0f, "Out Level", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_Density2 = {{
    AirwindowsExtractedParam{"density", 0.2f, 0.0f, 1.0f, "Density", ""},
    AirwindowsExtractedParam{"highpass", 0.0f, 0.0f, 1.0f, "Highpass", ""},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_Density3 = {{
    AirwindowsExtractedParam{"density", 0.0f, 0.0f, 1.0f, "Density", ""},
    AirwindowsExtractedParam{"highpas", 0.0f, 0.0f, 1.0f, "Highpas", ""},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_DeRez = {{
    AirwindowsExtractedParam{"rate", 1.0f, 0.0f, 1.0f, "Rate", ""},
    AirwindowsExtractedParam{"rez", 1.0f, 0.0f, 1.0f, "Rez", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_DeRez2 = {{
    AirwindowsExtractedParam{"rate", 1.0f, 0.0f, 1.0f, "Rate", ""},
    AirwindowsExtractedParam{"rez", 1.0f, 0.0f, 1.0f, "Rez", ""},
    AirwindowsExtractedParam{"hard", 1.0f, 0.0f, 1.0f, "Hard", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_DeRez3 = {{
    AirwindowsExtractedParam{"rate", 1.0f, 0.0f, 1.0f, "Rate", ""},
    AirwindowsExtractedParam{"rez", 1.0f, 0.0f, 1.0f, "Rez", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_DeRez4 = {{
    AirwindowsExtractedParam{"down_rez", 0.618f, 0.0f, 1.0f, "DownRez", ""},
    AirwindowsExtractedParam{"bright", 0.618f, 0.0f, 1.0f, "Bright", ""},
    AirwindowsExtractedParam{"bassis", 0.382f, 0.0f, 1.0f, "Bassis", ""},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 6> kAirwindowsParams_DeRez5 = {{
    AirwindowsExtractedParam{"derez", 1.0f, 0.0f, 1.0f, "Derez", ""},
    AirwindowsExtractedParam{"trim", 1.0f, 0.0f, 1.0f, "Trim", ""},
    AirwindowsExtractedParam{"square", 0.0f, 0.0f, 1.0f, "Square", ""},
    AirwindowsExtractedParam{"bright", 1.0f, 0.0f, 1.0f, "Bright", ""},
    AirwindowsExtractedParam{"bassis", 0.0f, 0.0f, 1.0f, "Bassis", ""},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 0> kAirwindowsParams_Desk = {{
}};

inline constexpr std::array<AirwindowsExtractedParam, 6> kAirwindowsParams_Desk4 = {{
    AirwindowsExtractedParam{"overdrive", 0.27f, 0.0f, 1.0f, "Overdrive", ""},
    AirwindowsExtractedParam{"hi__choke", 0.18f, 0.0f, 1.0f, "Hi Choke", ""},
    AirwindowsExtractedParam{"power__sag", 0.26f, 0.0f, 1.0f, "Power Sag", ""},
    AirwindowsExtractedParam{"frequency", 0.54f, 0.0f, 1.0f, "Frequency", ""},
    AirwindowsExtractedParam{"output__trim", 0.84f, 0.0f, 1.0f, "Output Trim", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_DigitalBlack = {{
    AirwindowsExtractedParam{"thresh", 0.0f, 0.0f, 1.0f, "Thresh", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 5> kAirwindowsParams_Dirt = {{
    AirwindowsExtractedParam{"gain", 0.1f, 0.0f, 1.0f, "Gain", ""},
    AirwindowsExtractedParam{"lowpass", 1.0f, 0.0f, 1.0f, "Lowpass", ""},
    AirwindowsExtractedParam{"highpass", 0.0f, 0.0f, 1.0f, "Highpass", ""},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_Discontapeity = {{
    AirwindowsExtractedParam{"more", 0.0f, 0.0f, 1.0f, "More", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_Discontinuity = {{
    AirwindowsExtractedParam{"top_d_b", 0.5f, 0.0f, 1.0f, "Top dB", "dB"},
}};

inline constexpr std::array<AirwindowsExtractedParam, 5> kAirwindowsParams_Disintegrate = {{
    AirwindowsExtractedParam{"top_d_b", 0.5f, 0.0f, 1.0f, "Top dB", ""},
    AirwindowsExtractedParam{"buf_size", 0.5f, 0.0f, 1.0f, "BufSize", ""},
    AirwindowsExtractedParam{"layers", 0.2f, 0.0f, 1.0f, "Layers", ""},
    AirwindowsExtractedParam{"filter", 0.5f, 0.0f, 1.0f, "Filter", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_Distance = {{
    AirwindowsExtractedParam{"distance", 0.0f, 0.0f, 1.0f, "Distance", "miles"},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_Distance2 = {{
    AirwindowsExtractedParam{"atmosph", 0.85f, 0.0f, 1.0f, "Atmosph", ""},
    AirwindowsExtractedParam{"darken", 0.618f, 0.0f, 1.0f, "Darken", ""},
    AirwindowsExtractedParam{"dry__wet", 0.618f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_Distance3 = {{
    AirwindowsExtractedParam{"distance", 0.5f, 0.0f, 1.0f, "Distance", "miles"},
    AirwindowsExtractedParam{"top_d_b", 0.5f, 0.0f, 1.0f, "Top dB", "dB"},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_Distortion = {{
    AirwindowsExtractedParam{"input", 0.5f, 0.0f, 1.0f, "Input", "dB"},
    AirwindowsExtractedParam{"mode", 0.5f, 0.0f, 1.0f, "Mode", ""},
    AirwindowsExtractedParam{"output", 0.5f, 0.0f, 1.0f, "Output", "dB"},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_Ditherbox = {{
    AirwindowsExtractedParam{"type", 0.86f, 0.0f, 1.0f, "Type", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_DitherFloat = {{
    AirwindowsExtractedParam{"offset", 0.0f, 0.0f, 1.0f, "Offset", ""},
    AirwindowsExtractedParam{"dither", 1.0f, 0.0f, 1.0f, "Dither", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 0> kAirwindowsParams_DitherMeDiskers = {{
}};

inline constexpr std::array<AirwindowsExtractedParam, 0> kAirwindowsParams_DitherMeTimbers = {{
}};

inline constexpr std::array<AirwindowsExtractedParam, 7> kAirwindowsParams_Donut = {{
    AirwindowsExtractedParam{"attack", 0.5f, 0.0f, 1.0f, "Attack", ""},
    AirwindowsExtractedParam{"release", 0.5f, 0.0f, 1.0f, "Release", ""},
    AirwindowsExtractedParam{"base_frq", 1.0f, 0.0f, 1.0f, "BaseFrq", ""},
    AirwindowsExtractedParam{"move_frq", 0.5f, 0.0f, 1.0f, "MoveFrq", ""},
    AirwindowsExtractedParam{"base_res", 0.0f, 0.0f, 1.0f, "BaseRes", ""},
    AirwindowsExtractedParam{"move_res", 0.5f, 0.0f, 1.0f, "MoveRes", ""},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 5> kAirwindowsParams_Doublelay = {{
    AirwindowsExtractedParam{"detune", 0.2f, 0.0f, 1.0f, "Detune", ""},
    AirwindowsExtractedParam{"delay_l", 0.1f, 0.0f, 1.0f, "Delay L", "sec"},
    AirwindowsExtractedParam{"delay_r", 0.2f, 0.0f, 1.0f, "Delay R", "sec"},
    AirwindowsExtractedParam{"feedbk", 0.0f, 0.0f, 1.0f, "Feedbk", ""},
    AirwindowsExtractedParam{"dry__wet", 0.6f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 0> kAirwindowsParams_DoublePaul = {{
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_Drive = {{
    AirwindowsExtractedParam{"drive", 0.0f, 0.0f, 1.0f, "Drive", "%"},
    AirwindowsExtractedParam{"highpass", 0.0f, 0.0f, 1.0f, "Highpass", ""},
    AirwindowsExtractedParam{"out__level", 1.0f, 0.0f, 1.0f, "Out Level", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_DrumSlam = {{
    AirwindowsExtractedParam{"drive", 0.0f, 0.0f, 1.0f, "Drive", ""},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 10> kAirwindowsParams_DubCenter = {{
    AirwindowsExtractedParam{"t_grind", 0.9f, 0.0f, 1.0f, "TGrind", ""},
    AirwindowsExtractedParam{"grd__out", 0.5f, 0.0f, 1.0f, "Grd/Out", ""},
    AirwindowsExtractedParam{"x_over", 0.74f, 0.0f, 1.0f, "XOver", ""},
    AirwindowsExtractedParam{"bs_drive", 1.0f, 0.0f, 1.0f, "BsDrive", ""},
    AirwindowsExtractedParam{"bs_voice", 0.95f, 0.0f, 1.0f, "BsVoice", ""},
    AirwindowsExtractedParam{"bass_out", 0.5f, 0.0f, 1.0f, "BassOut", ""},
    AirwindowsExtractedParam{"sb_drive", 0.2f, 0.0f, 1.0f, "SbDrive", ""},
    AirwindowsExtractedParam{"sb_voice", 0.2f, 0.0f, 1.0f, "SbVoice", ""},
    AirwindowsExtractedParam{"sub_out", 0.5f, 0.0f, 1.0f, "SubOut", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_Dubly = {{
    AirwindowsExtractedParam{"dubly", 0.5f, 0.0f, 1.0f, "Dubly", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 7> kAirwindowsParams_Dubly2 = {{
    AirwindowsExtractedParam{"enc_amnt", 0.5f, 0.0f, 1.0f, "EncAmnt", ""},
    AirwindowsExtractedParam{"enc_freq", 0.5f, 0.0f, 1.0f, "EncFreq", ""},
    AirwindowsExtractedParam{"tape_drv", 0.0f, 0.0f, 1.0f, "TapeDrv", ""},
    AirwindowsExtractedParam{"dec_amnt", 0.5f, 0.0f, 1.0f, "DecAmnt", ""},
    AirwindowsExtractedParam{"dec_freq", 0.5f, 0.0f, 1.0f, "DecFreq", ""},
    AirwindowsExtractedParam{"out__pad", 0.0f, 0.0f, 1.0f, "Out Pad", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_Dubly3 = {{
    AirwindowsExtractedParam{"input", 0.5f, 0.0f, 1.0f, "Input", ""},
    AirwindowsExtractedParam{"tilt", 0.5f, 0.0f, 1.0f, "Tilt", ""},
    AirwindowsExtractedParam{"shape", 0.5f, 0.0f, 1.0f, "Shape", ""},
    AirwindowsExtractedParam{"output", 0.5f, 0.0f, 1.0f, "Output", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 0> kAirwindowsParams_DubPlate = {{
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_DubPlate2 = {{
    AirwindowsExtractedParam{"input", 0.5f, 0.0f, 1.0f, "Input", ""},
    AirwindowsExtractedParam{"treble", 0.5f, 0.0f, 1.0f, "Treble", ""},
    AirwindowsExtractedParam{"bass", 0.5f, 0.0f, 1.0f, "Bass", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 10> kAirwindowsParams_DubSub = {{
    AirwindowsExtractedParam{"t_grind", 0.9f, 0.0f, 1.0f, "TGrind", ""},
    AirwindowsExtractedParam{"grd__out", 0.5f, 0.0f, 1.0f, "Grd/Out", ""},
    AirwindowsExtractedParam{"x_over", 0.74f, 0.0f, 1.0f, "XOver", ""},
    AirwindowsExtractedParam{"bs_drive", 1.0f, 0.0f, 1.0f, "BsDrive", ""},
    AirwindowsExtractedParam{"bs_voice", 0.95f, 0.0f, 1.0f, "BsVoice", ""},
    AirwindowsExtractedParam{"bass_out", 0.5f, 0.0f, 1.0f, "BassOut", ""},
    AirwindowsExtractedParam{"sb_drive", 0.2f, 0.0f, 1.0f, "SbDrive", ""},
    AirwindowsExtractedParam{"sb_voice", 0.2f, 0.0f, 1.0f, "SbVoice", ""},
    AirwindowsExtractedParam{"sub_out", 0.5f, 0.0f, 1.0f, "SubOut", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_DubSub2 = {{
    AirwindowsExtractedParam{"head_bmp", 0.5f, 0.0f, 1.0f, "HeadBmp", ""},
    AirwindowsExtractedParam{"head_frq", 0.5f, 0.0f, 1.0f, "HeadFrq", "hz"},
    AirwindowsExtractedParam{"dry__wet", 0.5f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_DustBunny = {{
    AirwindowsExtractedParam{"bunny", 0.0f, 0.0f, 1.0f, "Bunny", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_Dynamics = {{
    AirwindowsExtractedParam{"comp", 0.0f, 0.0f, 1.0f, "Comp", ""},
    AirwindowsExtractedParam{"speed", 0.5f, 0.0f, 1.0f, "Speed", ""},
    AirwindowsExtractedParam{"gate", 0.0f, 0.0f, 1.0f, "Gate", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_Dynamics2 = {{
    AirwindowsExtractedParam{"thresh", 1.0f, 0.0f, 1.0f, "Thresh", ""},
    AirwindowsExtractedParam{"attack", 0.5f, 0.0f, 1.0f, "Attack", ""},
    AirwindowsExtractedParam{"release", 0.5f, 0.0f, 1.0f, "Release", ""},
    AirwindowsExtractedParam{"gate", 0.0f, 0.0f, 1.0f, "Gate", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_Dynamics3 = {{
    AirwindowsExtractedParam{"thresh", 1.0f, 0.0f, 1.0f, "Thresh", ""},
    AirwindowsExtractedParam{"attack", 0.382f, 0.0f, 1.0f, "Attack", ""},
    AirwindowsExtractedParam{"release", 0.618f, 0.0f, 1.0f, "Release", ""},
    AirwindowsExtractedParam{"inv__wet", 1.0f, 0.0f, 1.0f, "Inv/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_Dyno = {{
    AirwindowsExtractedParam{"input", 0.5f, 0.0f, 1.0f, "Input", "dB"},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_Echojam = {{
    AirwindowsExtractedParam{"select", 0.5f, 0.0f, 1.0f, "Select", ""},
    AirwindowsExtractedParam{"regen", 0.5f, 0.0f, 1.0f, "Regen", ""},
    AirwindowsExtractedParam{"speed", 0.5f, 0.0f, 1.0f, "Speed", ""},
    AirwindowsExtractedParam{"dry__wet", 0.5f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 5> kAirwindowsParams_Edge = {{
    AirwindowsExtractedParam{"gain", 0.1f, 0.0f, 1.0f, "Gain", ""},
    AirwindowsExtractedParam{"lowpass", 1.0f, 0.0f, 1.0f, "Lowpass", ""},
    AirwindowsExtractedParam{"highpass", 0.0f, 0.0f, 1.0f, "Highpass", ""},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_EdIsDim = {{
    AirwindowsExtractedParam{"edi_s_di_m", 0.5f, 0.0f, 1.0f, "ediS/diM", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_Elation = {{
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
    AirwindowsExtractedParam{"persnlty", 0.333333f, 0.0f, 1.0f, "Persnlty", ""},
    AirwindowsExtractedParam{"drive", 0.333333f, 0.0f, 1.0f, "Drive", ""},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 5> kAirwindowsParams_ElectroHat = {{
    AirwindowsExtractedParam{"hi_hat", 0.0f, 0.0f, 1.0f, "HiHat", ""},
    AirwindowsExtractedParam{"trim", 0.5f, 0.0f, 1.0f, "Trim", ""},
    AirwindowsExtractedParam{"bright", 1.0f, 0.0f, 1.0f, "Bright", ""},
    AirwindowsExtractedParam{"output", 0.1f, 0.0f, 1.0f, "Output", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_Elliptical = {{
    AirwindowsExtractedParam{"cutoff", 0.5f, 0.0f, 1.0f, "Cutoff", ""},
    AirwindowsExtractedParam{"slope", 0.5f, 0.0f, 1.0f, "Slope", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 9> kAirwindowsParams_Energy = {{
    AirwindowsExtractedParam{"hiss", 0.5f, 0.0f, 1.0f, "Hiss", ""},
    AirwindowsExtractedParam{"glitter", 0.5f, 0.0f, 1.0f, "Glitter", ""},
    AirwindowsExtractedParam{"rat", 0.5f, 0.0f, 1.0f, "Rat", ""},
    AirwindowsExtractedParam{"fizz", 0.5f, 0.0f, 1.0f, "Fizz", ""},
    AirwindowsExtractedParam{"scrape", 0.5f, 0.0f, 1.0f, "Scrape", ""},
    AirwindowsExtractedParam{"chug", 0.5f, 0.0f, 1.0f, "Chug", ""},
    AirwindowsExtractedParam{"yowr", 0.5f, 0.0f, 1.0f, "Yowr", ""},
    AirwindowsExtractedParam{"snarl", 0.5f, 0.0f, 1.0f, "Snarl", ""},
    AirwindowsExtractedParam{"i__dr__wt", 1.0f, 0.0f, 1.0f, "I/Dr/Wt", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 9> kAirwindowsParams_Energy2 = {{
    AirwindowsExtractedParam{"hiss", 0.5f, 0.0f, 1.0f, "Hiss", ""},
    AirwindowsExtractedParam{"glitter", 0.5f, 0.0f, 1.0f, "Glitter", ""},
    AirwindowsExtractedParam{"rat", 0.5f, 0.0f, 1.0f, "Rat", ""},
    AirwindowsExtractedParam{"fizz", 0.5f, 0.0f, 1.0f, "Fizz", ""},
    AirwindowsExtractedParam{"scrape", 0.5f, 0.0f, 1.0f, "Scrape", ""},
    AirwindowsExtractedParam{"chug", 0.5f, 0.0f, 1.0f, "Chug", ""},
    AirwindowsExtractedParam{"yowr", 0.5f, 0.0f, 1.0f, "Yowr", ""},
    AirwindowsExtractedParam{"snarl", 0.5f, 0.0f, 1.0f, "Snarl", ""},
    AirwindowsExtractedParam{"i__dr__wt", 1.0f, 0.0f, 1.0f, "I/Dr/Wt", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_Ensemble = {{
    AirwindowsExtractedParam{"ensemble", 0.5f, 0.0f, 1.0f, "Ensemble", "vox"},
    AirwindowsExtractedParam{"fullness", 0.0f, 0.0f, 1.0f, "Fullness", ""},
    AirwindowsExtractedParam{"brighten", 1.0f, 0.0f, 1.0f, "Brighten", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 8> kAirwindowsParams_EQ = {{
    AirwindowsExtractedParam{"treble", 0.5f, 0.0f, 1.0f, "Treble", "dB"},
    AirwindowsExtractedParam{"mid", 0.5f, 0.0f, 1.0f, "Mid", "dB"},
    AirwindowsExtractedParam{"bass", 0.5f, 0.0f, 1.0f, "Bass", "dB"},
    AirwindowsExtractedParam{"lowpass", 1.0f, 0.0f, 1.0f, "Lowpass", "Khz"},
    AirwindowsExtractedParam{"treb_frq", 0.4f, 0.0f, 1.0f, "TrebFrq", "Khz"},
    AirwindowsExtractedParam{"bass_frq", 0.4f, 0.0f, 1.0f, "BassFrq", "hz"},
    AirwindowsExtractedParam{"hipass", 0.0f, 0.0f, 1.0f, "Hipass", "hz"},
    AirwindowsExtractedParam{"out_gain", 0.5f, 0.0f, 1.0f, "OutGain", "dB"},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_EveryConsole = {{
    AirwindowsExtractedParam{"con_type", 0.0f, 0.0f, 1.0f, "ConType", ""},
    AirwindowsExtractedParam{"in_trim", 0.5f, 0.0f, 1.0f, "InTrim", ""},
    AirwindowsExtractedParam{"out_trim", 0.5f, 0.0f, 1.0f, "OutTrim", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_EverySlew = {{
    AirwindowsExtractedParam{"slew", 0.0f, 0.0f, 1.0f, "Slew", ""},
    AirwindowsExtractedParam{"depth", 1.0f, 0.0f, 1.0f, "Depth", ""},
    AirwindowsExtractedParam{"halo", 0.16f, 0.0f, 1.0f, "Halo", ""},
    AirwindowsExtractedParam{"inv__wet", 1.0f, 0.0f, 1.0f, "Inv/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 5> kAirwindowsParams_EveryTrim = {{
    AirwindowsExtractedParam{"left", 0.5f, 0.0f, 1.0f, "Left", "dB"},
    AirwindowsExtractedParam{"right", 0.5f, 0.0f, 1.0f, "Right", "dB"},
    AirwindowsExtractedParam{"mid", 0.5f, 0.0f, 1.0f, "Mid", "dB"},
    AirwindowsExtractedParam{"side", 0.5f, 0.0f, 1.0f, "Side", "dB"},
    AirwindowsExtractedParam{"master", 0.5f, 0.0f, 1.0f, "Master", "dB"},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_Exciter = {{
    AirwindowsExtractedParam{"freq", 0.6f, 0.0f, 1.0f, "Freq", ""},
    AirwindowsExtractedParam{"excite", 0.0f, 0.0f, 1.0f, "Excite", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_Facet = {{
    AirwindowsExtractedParam{"facet", 1.0f, 0.0f, 1.0f, "Facet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_FastDistance = {{
    AirwindowsExtractedParam{"distnce", 0.0f, 0.0f, 1.0f, "Distnce", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_FastDistance2 = {{
    AirwindowsExtractedParam{"distnce", 0.0f, 0.0f, 1.0f, "Distnce", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 5> kAirwindowsParams_FatEQ = {{
    AirwindowsExtractedParam{"high", 0.5f, 0.0f, 1.0f, "High", ""},
    AirwindowsExtractedParam{"h_mid", 0.5f, 0.0f, 1.0f, "HMid", ""},
    AirwindowsExtractedParam{"l_mid", 0.5f, 0.0f, 1.0f, "LMid", ""},
    AirwindowsExtractedParam{"bass", 0.5f, 0.0f, 1.0f, "Bass", ""},
    AirwindowsExtractedParam{"out", 1.0f, 0.0f, 1.0f, "Out", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_FathomFive = {{
    AirwindowsExtractedParam{"root__note", 1.0f, 0.0f, 1.0f, "Root Note", ""},
    AirwindowsExtractedParam{"sub_octave", 0.0f, 0.0f, 1.0f, "SubOctave", ""},
    AirwindowsExtractedParam{"frequency", 0.5f, 0.0f, 1.0f, "Frequency", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 0> kAirwindowsParams_FinalClip = {{
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_FireAmp = {{
    AirwindowsExtractedParam{"gain", 0.5f, 0.0f, 1.0f, "Gain", ""},
    AirwindowsExtractedParam{"tone", 0.5f, 0.0f, 1.0f, "Tone", ""},
    AirwindowsExtractedParam{"output", 0.8f, 0.0f, 1.0f, "Output", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_Flipity = {{
    AirwindowsExtractedParam{"flipity", 0.0f, 0.0f, 1.0f, "Flipity", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_Floor = {{
    AirwindowsExtractedParam{"floor", 0.0f, 0.0f, 1.0f, "Floor", ""},
    AirwindowsExtractedParam{"drive", 0.0f, 0.0f, 1.0f, "Drive", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_Flutter = {{
    AirwindowsExtractedParam{"flutter", 0.0f, 0.0f, 1.0f, "Flutter", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_Flutter2 = {{
    AirwindowsExtractedParam{"flutter", 0.5f, 0.0f, 1.0f, "Flutter", ""},
    AirwindowsExtractedParam{"speed", 0.5f, 0.0f, 1.0f, "Speed", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 5> kAirwindowsParams_Focus = {{
    AirwindowsExtractedParam{"boost", 0.0f, 0.0f, 1.0f, "Boost", "dB"},
    AirwindowsExtractedParam{"focus", 0.5f, 0.0f, 1.0f, "Focus", ""},
    AirwindowsExtractedParam{"mode", 0.5f, 0.0f, 1.0f, "Mode", ""},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_Fracture = {{
    AirwindowsExtractedParam{"drive", 0.25f, 0.0f, 1.0f, "Drive", ""},
    AirwindowsExtractedParam{"fracture", 0.5f, 0.0f, 1.0f, "Fracture", ""},
    AirwindowsExtractedParam{"out__lvl", 1.0f, 0.0f, 1.0f, "Out Lvl", "dB"},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 5> kAirwindowsParams_Fracture2 = {{
    AirwindowsExtractedParam{"drive", 0.1f, 0.0f, 1.0f, "Drive", ""},
    AirwindowsExtractedParam{"fractre", 0.0f, 0.0f, 1.0f, "Fractre", ""},
    AirwindowsExtractedParam{"thresh", 0.5f, 0.0f, 1.0f, "Thresh", ""},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 5> kAirwindowsParams_FromTape = {{
    AirwindowsExtractedParam{"louder", 0.5f, 0.0f, 1.0f, "Louder", ""},
    AirwindowsExtractedParam{"softer", 0.5f, 0.0f, 1.0f, "Softer", ""},
    AirwindowsExtractedParam{"weight", 0.5f, 0.0f, 1.0f, "Weight", ""},
    AirwindowsExtractedParam{"output", 0.5f, 0.0f, 1.0f, "Output", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 5> kAirwindowsParams_Galactic = {{
    AirwindowsExtractedParam{"replace", 0.5f, 0.0f, 1.0f, "Replace", ""},
    AirwindowsExtractedParam{"brightness", 0.5f, 0.0f, 1.0f, "Brightness", ""},
    AirwindowsExtractedParam{"detune", 0.5f, 0.0f, 1.0f, "Detune", ""},
    AirwindowsExtractedParam{"bigness", 1.0f, 0.0f, 1.0f, "Bigness", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_Galactic2 = {{
    AirwindowsExtractedParam{"drive", 0.5f, 0.0f, 1.0f, "Drive", ""},
    AirwindowsExtractedParam{"sustain", 0.5f, 0.0f, 1.0f, "Sustain", ""},
    AirwindowsExtractedParam{"darken", 0.5f, 0.0f, 1.0f, "Darken", ""},
    AirwindowsExtractedParam{"wetness", 1.0f, 0.0f, 1.0f, "Wetness", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 6> kAirwindowsParams_Galactic3 = {{
    AirwindowsExtractedParam{"replace", 0.5f, 0.0f, 1.0f, "Replace", ""},
    AirwindowsExtractedParam{"brightns", 0.5f, 0.0f, 1.0f, "Brightns", ""},
    AirwindowsExtractedParam{"detune", 0.5f, 0.0f, 1.0f, "Detune", ""},
    AirwindowsExtractedParam{"derez", 1.0f, 0.0f, 1.0f, "Derez", ""},
    AirwindowsExtractedParam{"bigness", 1.0f, 0.0f, 1.0f, "Bigness", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_GalacticVibe = {{
    AirwindowsExtractedParam{"drift", 0.0f, 0.0f, 1.0f, "Drift", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 5> kAirwindowsParams_Gatelope = {{
    AirwindowsExtractedParam{"thresh", 0.0f, 0.0f, 1.0f, "Thresh", ""},
    AirwindowsExtractedParam{"treb_sus", 1.0f, 0.0f, 1.0f, "TrebSus", ""},
    AirwindowsExtractedParam{"bass_sus", 0.5f, 0.0f, 1.0f, "BassSus", ""},
    AirwindowsExtractedParam{"attack_s", 0.0f, 0.0f, 1.0f, "AttackS", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 5> kAirwindowsParams_GlitchShifter = {{
    AirwindowsExtractedParam{"note", 0.5f, 0.0f, 1.0f, "Note", "semi"},
    AirwindowsExtractedParam{"trim", 0.5f, 0.0f, 1.0f, "Trim", ""},
    AirwindowsExtractedParam{"tighten", 0.5f, 0.0f, 1.0f, "Tighten", ""},
    AirwindowsExtractedParam{"feedbck", 0.0f, 0.0f, 1.0f, "Feedbck", ""},
    AirwindowsExtractedParam{"dry__wet", 0.5f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_GoldenSlew = {{
    AirwindowsExtractedParam{"slew", 0.0f, 0.0f, 1.0f, "Slew", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_Golem = {{
    AirwindowsExtractedParam{"balance", 0.5f, 0.0f, 1.0f, "Balance", ""},
    AirwindowsExtractedParam{"offset", 0.5f, 0.0f, 1.0f, "Offset", ""},
    AirwindowsExtractedParam{"phase", 0.0f, 0.0f, 1.0f, "Phase", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_GrindAmp = {{
    AirwindowsExtractedParam{"gain", 0.5f, 0.0f, 1.0f, "Gain", ""},
    AirwindowsExtractedParam{"tone", 0.5f, 0.0f, 1.0f, "Tone", ""},
    AirwindowsExtractedParam{"output", 0.8f, 0.0f, 1.0f, "Output", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 0> kAirwindowsParams_Gringer = {{
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_GrooveWear = {{
    AirwindowsExtractedParam{"wear", 0.064f, 0.0f, 1.0f, "Wear", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 0> kAirwindowsParams_GuitarConditioner = {{
}};

inline constexpr std::array<AirwindowsExtractedParam, 5> kAirwindowsParams_HardVacuum = {{
    AirwindowsExtractedParam{"drive", 0.0f, 0.0f, 1.0f, "Drive", ""},
    AirwindowsExtractedParam{"warmth", 0.0f, 0.0f, 1.0f, "Warmth", ""},
    AirwindowsExtractedParam{"aura", 0.0f, 0.0f, 1.0f, "Aura", ""},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_Hermepass = {{
    AirwindowsExtractedParam{"cutoff", 0.5f, 0.0f, 1.0f, "Cutoff", ""},
    AirwindowsExtractedParam{"slope", 0.5f, 0.0f, 1.0f, "Slope", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 5> kAirwindowsParams_HermeTrim = {{
    AirwindowsExtractedParam{"left", 0.5f, 0.0f, 1.0f, "Left", "dB"},
    AirwindowsExtractedParam{"right", 0.5f, 0.0f, 1.0f, "Right", "dB"},
    AirwindowsExtractedParam{"mid", 0.5f, 0.0f, 1.0f, "Mid", "dB"},
    AirwindowsExtractedParam{"side", 0.5f, 0.0f, 1.0f, "Side", "dB"},
    AirwindowsExtractedParam{"master", 0.5f, 0.0f, 1.0f, "Master", "dB"},
}};

inline constexpr std::array<AirwindowsExtractedParam, 0> kAirwindowsParams_HighGlossDither = {{
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_HighImpact = {{
    AirwindowsExtractedParam{"impact", 0.0f, 0.0f, 1.0f, "Impact", ""},
    AirwindowsExtractedParam{"output__level", 0.0f, 0.0f, 1.0f, "Output Level", ""},
    AirwindowsExtractedParam{"dry__wet", 0.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_Highpass = {{
    AirwindowsExtractedParam{"highpass", 0.0f, 0.0f, 1.0f, "Highpass", ""},
    AirwindowsExtractedParam{"loose__tight", 0.5f, 0.0f, 1.0f, "Loose/Tight", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_Highpass2 = {{
    AirwindowsExtractedParam{"hipass", 0.0f, 0.0f, 1.0f, "Hipass", ""},
    AirwindowsExtractedParam{"ls__tite", 0.5f, 0.0f, 1.0f, "Ls/Tite", ""},
    AirwindowsExtractedParam{"poles", 0.25f, 0.0f, 1.0f, "Poles", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 10> kAirwindowsParams_HipCrush = {{
    AirwindowsExtractedParam{"hi__freq", 0.5f, 0.0f, 1.0f, "Hi Freq", "High"},
    AirwindowsExtractedParam{"high", 0.0f, 0.0f, 1.0f, "High", ""},
    AirwindowsExtractedParam{"hi_crush", 0.5f, 0.0f, 1.0f, "HiCrush", ""},
    AirwindowsExtractedParam{"mid_freq", 0.5f, 0.0f, 1.0f, "MidFreq", "Mid"},
    AirwindowsExtractedParam{"mid", 0.0f, 0.0f, 1.0f, "Mid", ""},
    AirwindowsExtractedParam{"md_crush", 0.5f, 0.0f, 1.0f, "MdCrush", ""},
    AirwindowsExtractedParam{"lo__freq", 0.5f, 0.0f, 1.0f, "Lo Freq", "Low"},
    AirwindowsExtractedParam{"low", 0.0f, 0.0f, 1.0f, "Low", ""},
    AirwindowsExtractedParam{"lo_crush", 0.5f, 0.0f, 1.0f, "LoCrush", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", "Wet"},
}};

inline constexpr std::array<AirwindowsExtractedParam, 5> kAirwindowsParams_Holt = {{
    AirwindowsExtractedParam{"freq", 1.0f, 0.0f, 1.0f, "Freq", ""},
    AirwindowsExtractedParam{"reso", 0.5f, 0.0f, 1.0f, "Reso", ""},
    AirwindowsExtractedParam{"poles", 1.0f, 0.0f, 1.0f, "Poles", ""},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 5> kAirwindowsParams_Holt2 = {{
    AirwindowsExtractedParam{"freq", 1.0f, 0.0f, 1.0f, "Freq", ""},
    AirwindowsExtractedParam{"reso", 0.5f, 0.0f, 1.0f, "Reso", ""},
    AirwindowsExtractedParam{"poles", 1.0f, 0.0f, 1.0f, "Poles", ""},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_Hombre = {{
    AirwindowsExtractedParam{"voicing", 0.421f, 0.0f, 1.0f, "Voicing", ""},
    AirwindowsExtractedParam{"intensity", 0.5f, 0.0f, 1.0f, "Intensity", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_Huge = {{
    AirwindowsExtractedParam{"huge", 0.2f, 0.0f, 1.0f, "Huge", ""},
    AirwindowsExtractedParam{"nasty", 0.2f, 0.0f, 1.0f, "Nasty", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_Hull = {{
    AirwindowsExtractedParam{"freq", 0.0f, 0.0f, 1.0f, "Freq", ""},
    AirwindowsExtractedParam{"bright", 0.5f, 0.0f, 1.0f, "Bright", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_Hull2 = {{
    AirwindowsExtractedParam{"treble", 0.5f, 0.0f, 1.0f, "Treble", ""},
    AirwindowsExtractedParam{"mid", 0.5f, 0.0f, 1.0f, "Mid", ""},
    AirwindowsExtractedParam{"bass", 0.5f, 0.0f, 1.0f, "Bass", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 0> kAirwindowsParams_Hype = {{
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_Hypersoft = {{
    AirwindowsExtractedParam{"input", 0.5f, 0.0f, 1.0f, "Input", ""},
    AirwindowsExtractedParam{"depth", 0.5f, 0.0f, 1.0f, "Depth", ""},
    AirwindowsExtractedParam{"bright", 0.5f, 0.0f, 1.0f, "Bright", ""},
    AirwindowsExtractedParam{"output", 0.5f, 0.0f, 1.0f, "Output", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 0> kAirwindowsParams_Hypersonic = {{
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_HypersonX = {{
    AirwindowsExtractedParam{"q", 0.5f, 0.0f, 1.0f, "Q", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_Infinity = {{
    AirwindowsExtractedParam{"filter", 1.0f, 0.0f, 1.0f, "Filter", ""},
    AirwindowsExtractedParam{"damping", 0.0f, 0.0f, 1.0f, "Damping", ""},
    AirwindowsExtractedParam{"size", 0.5f, 0.0f, 1.0f, "Size", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 6> kAirwindowsParams_Infinity2 = {{
    AirwindowsExtractedParam{"filter", 1.0f, 0.0f, 1.0f, "Filter", ""},
    AirwindowsExtractedParam{"size", 0.5f, 0.0f, 1.0f, "Size", ""},
    AirwindowsExtractedParam{"damping", 0.0f, 0.0f, 1.0f, "Damping", ""},
    AirwindowsExtractedParam{"allpass", 1.0f, 0.0f, 1.0f, "Allpass", ""},
    AirwindowsExtractedParam{"feedback", 1.0f, 0.0f, 1.0f, "Feedback", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_Inflamer = {{
    AirwindowsExtractedParam{"drive", 0.5f, 0.0f, 1.0f, "Drive", ""},
    AirwindowsExtractedParam{"curve", 0.5f, 0.0f, 1.0f, "Curve", ""},
    AirwindowsExtractedParam{"effect", 1.0f, 0.0f, 1.0f, "Effect", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 0> kAirwindowsParams_Infrasonic = {{
}};

inline constexpr std::array<AirwindowsExtractedParam, 0> kAirwindowsParams_Interstage = {{
}};

inline constexpr std::array<AirwindowsExtractedParam, 7> kAirwindowsParams_IronOxide5 = {{
    AirwindowsExtractedParam{"input__trim", 0.5f, 0.0f, 1.0f, "Input Trim", "dB"},
    AirwindowsExtractedParam{"tape__high", 0.562341f, 0.0f, 1.0f, "Tape High", "ips"},
    AirwindowsExtractedParam{"tape__low", 0.562341f, 0.0f, 1.0f, "Tape Low", "ips"},
    AirwindowsExtractedParam{"flutter", 0.5f, 0.0f, 1.0f, "Flutter", ""},
    AirwindowsExtractedParam{"noise", 0.5f, 0.0f, 1.0f, "Noise", ""},
    AirwindowsExtractedParam{"output__trim", 0.5f, 0.0f, 1.0f, "Output Trim", "dB"},
    AirwindowsExtractedParam{"inv__dry__wet", 1.0f, 0.0f, 1.0f, "Inv/Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_IronOxideClassic = {{
    AirwindowsExtractedParam{"input__trim", 0.5f, 0.0f, 1.0f, "Input Trim", "dB"},
    AirwindowsExtractedParam{"tape__speed", 0.562341f, 0.0f, 1.0f, "Tape Speed", "ips"},
    AirwindowsExtractedParam{"output__trim", 0.5f, 0.0f, 1.0f, "Output Trim", "dB"},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_IronOxideClassic2 = {{
    AirwindowsExtractedParam{"input__trim", 0.5f, 0.0f, 1.0f, "Input Trim", "dB"},
    AirwindowsExtractedParam{"tape__speed", 0.562341f, 0.0f, 1.0f, "Tape Speed", "ips"},
    AirwindowsExtractedParam{"output__trim", 0.5f, 0.0f, 1.0f, "Output Trim", "dB"},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_Isolator = {{
    AirwindowsExtractedParam{"freq", 1.0f, 0.0f, 1.0f, "Freq", ""},
    AirwindowsExtractedParam{"high", 0.0f, 0.0f, 1.0f, "High", ""},
    AirwindowsExtractedParam{"low", 1.0f, 0.0f, 1.0f, "Low", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_Isolator2 = {{
    AirwindowsExtractedParam{"freq", 1.0f, 0.0f, 1.0f, "Freq", ""},
    AirwindowsExtractedParam{"reso", 0.0f, 0.0f, 1.0f, "Reso", ""},
    AirwindowsExtractedParam{"high", 0.0f, 0.0f, 1.0f, "High", ""},
    AirwindowsExtractedParam{"low", 1.0f, 0.0f, 1.0f, "Low", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_Isolator3 = {{
    AirwindowsExtractedParam{"iso", 0.5f, 0.0f, 1.0f, "Iso", ""},
    AirwindowsExtractedParam{"q", 0.0f, 0.0f, 1.0f, "Q", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 6> kAirwindowsParams_kAlienSpaceship = {{
    AirwindowsExtractedParam{"regen", 0.5f, 0.0f, 1.0f, "Regen", ""},
    AirwindowsExtractedParam{"derez", 0.5f, 0.0f, 1.0f, "Derez", ""},
    AirwindowsExtractedParam{"filter", 0.5f, 0.0f, 1.0f, "Filter", ""},
    AirwindowsExtractedParam{"early_rf", 0.5f, 0.0f, 1.0f, "EarlyRF", ""},
    AirwindowsExtractedParam{"positin", 0.5f, 0.0f, 1.0f, "Positin", ""},
    AirwindowsExtractedParam{"dry__wet", 0.5f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_Kalman = {{
    AirwindowsExtractedParam{"kalman", 0.5f, 0.0f, 1.0f, "Kalman", ""},
    AirwindowsExtractedParam{"inv__wet", 1.0f, 0.0f, 1.0f, "Inv/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 6> kAirwindowsParams_kBeyond = {{
    AirwindowsExtractedParam{"regen", 0.5f, 0.0f, 1.0f, "Regen", ""},
    AirwindowsExtractedParam{"derez", 1.0f, 0.0f, 1.0f, "Derez", ""},
    AirwindowsExtractedParam{"filter", 0.5f, 0.0f, 1.0f, "Filter", ""},
    AirwindowsExtractedParam{"early_rf", 0.5f, 0.0f, 1.0f, "EarlyRF", ""},
    AirwindowsExtractedParam{"predlay", 0.0f, 0.0f, 1.0f, "Predlay", ""},
    AirwindowsExtractedParam{"dry__wet", 0.5f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_kCathedral = {{
    AirwindowsExtractedParam{"wetness", 1.0f, 0.0f, 1.0f, "Wetness", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_kCathedral2 = {{
    AirwindowsExtractedParam{"wetness", 1.0f, 0.0f, 1.0f, "Wetness", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 5> kAirwindowsParams_kCathedral3 = {{
    AirwindowsExtractedParam{"top_d_b", 0.5f, 0.0f, 1.0f, "Top dB", "dB"},
    AirwindowsExtractedParam{"regen", 0.5f, 0.0f, 1.0f, "Regen", ""},
    AirwindowsExtractedParam{"de_rez", 1.0f, 0.0f, 1.0f, "DeRez", ""},
    AirwindowsExtractedParam{"predlay", 0.0f, 0.0f, 1.0f, "Predlay", ""},
    AirwindowsExtractedParam{"wetness", 0.25f, 0.0f, 1.0f, "Wetness", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 6> kAirwindowsParams_kCathedral4 = {{
    AirwindowsExtractedParam{"regen", 0.5f, 0.0f, 1.0f, "Regen", ""},
    AirwindowsExtractedParam{"derez", 1.0f, 0.0f, 1.0f, "Derez", ""},
    AirwindowsExtractedParam{"filter", 0.5f, 0.0f, 1.0f, "Filter", ""},
    AirwindowsExtractedParam{"early_rf", 0.5f, 0.0f, 1.0f, "EarlyRF", ""},
    AirwindowsExtractedParam{"predlay", 0.0f, 0.0f, 1.0f, "Predlay", ""},
    AirwindowsExtractedParam{"dry__wet", 0.5f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 6> kAirwindowsParams_kCathedral5 = {{
    AirwindowsExtractedParam{"regen", 0.5f, 0.0f, 1.0f, "Regen", ""},
    AirwindowsExtractedParam{"derez", 0.5f, 0.0f, 1.0f, "Derez", ""},
    AirwindowsExtractedParam{"filter", 0.24f, 0.0f, 1.0f, "Filter", ""},
    AirwindowsExtractedParam{"early_rf", 0.5f, 0.0f, 1.0f, "EarlyRF", ""},
    AirwindowsExtractedParam{"positin", 0.5f, 0.0f, 1.0f, "Positin", ""},
    AirwindowsExtractedParam{"dry__wet", 0.5f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 5> kAirwindowsParams_kChamberAR = {{
    AirwindowsExtractedParam{"input", 1.0f, 0.0f, 1.0f, "Input", ""},
    AirwindowsExtractedParam{"delay", 0.5f, 0.0f, 1.0f, "Delay", ""},
    AirwindowsExtractedParam{"regen", 0.0f, 0.0f, 1.0f, "Regen", ""},
    AirwindowsExtractedParam{"low__cut", 0.0f, 0.0f, 1.0f, "Low Cut", ""},
    AirwindowsExtractedParam{"wetness", 0.5f, 0.0f, 1.0f, "Wetness", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 6> kAirwindowsParams_kCosmos = {{
    AirwindowsExtractedParam{"regen", 0.5f, 0.0f, 1.0f, "Regen", ""},
    AirwindowsExtractedParam{"derez", 1.0f, 0.0f, 1.0f, "Derez", ""},
    AirwindowsExtractedParam{"filter", 0.5f, 0.0f, 1.0f, "Filter", ""},
    AirwindowsExtractedParam{"early_rf", 0.5f, 0.0f, 1.0f, "EarlyRF", ""},
    AirwindowsExtractedParam{"predlay", 0.0f, 0.0f, 1.0f, "Predlay", ""},
    AirwindowsExtractedParam{"dry__wet", 0.5f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_kCyberCity = {{
    AirwindowsExtractedParam{"regen", 0.5f, 0.0f, 1.0f, "Regen", ""},
    AirwindowsExtractedParam{"positin", 1.0f, 0.0f, 1.0f, "Positin", ""},
    AirwindowsExtractedParam{"dry__wet", 0.5f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 5> kAirwindowsParams_kGuitarHall = {{
    AirwindowsExtractedParam{"top_d_b", 0.5f, 0.0f, 1.0f, "Top dB", "dB"},
    AirwindowsExtractedParam{"regen", 0.5f, 0.0f, 1.0f, "Regen", ""},
    AirwindowsExtractedParam{"de_rez", 1.0f, 0.0f, 1.0f, "DeRez", ""},
    AirwindowsExtractedParam{"predlay", 0.0f, 0.0f, 1.0f, "Predlay", ""},
    AirwindowsExtractedParam{"wetness", 0.25f, 0.0f, 1.0f, "Wetness", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 6> kAirwindowsParams_kGuitarHall2 = {{
    AirwindowsExtractedParam{"regen", 0.5f, 0.0f, 1.0f, "Regen", ""},
    AirwindowsExtractedParam{"derez", 0.5f, 0.0f, 1.0f, "Derez", ""},
    AirwindowsExtractedParam{"filter", 0.5f, 0.0f, 1.0f, "Filter", ""},
    AirwindowsExtractedParam{"early_rf", 0.5f, 0.0f, 1.0f, "EarlyRF", ""},
    AirwindowsExtractedParam{"positin", 0.5f, 0.0f, 1.0f, "Positin", ""},
    AirwindowsExtractedParam{"dry__wet", 0.5f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 5> kAirwindowsParams_kPlate140 = {{
    AirwindowsExtractedParam{"input", 1.0f, 0.0f, 1.0f, "Input", ""},
    AirwindowsExtractedParam{"regen", 0.5f, 0.0f, 1.0f, "Regen", ""},
    AirwindowsExtractedParam{"de_rez", 1.0f, 0.0f, 1.0f, "DeRez", ""},
    AirwindowsExtractedParam{"predlay", 0.0f, 0.0f, 1.0f, "Predlay", ""},
    AirwindowsExtractedParam{"wetness", 0.25f, 0.0f, 1.0f, "Wetness", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 5> kAirwindowsParams_kPlate240 = {{
    AirwindowsExtractedParam{"input", 1.0f, 0.0f, 1.0f, "Input", ""},
    AirwindowsExtractedParam{"regen", 0.5f, 0.0f, 1.0f, "Regen", ""},
    AirwindowsExtractedParam{"de_rez", 1.0f, 0.0f, 1.0f, "DeRez", ""},
    AirwindowsExtractedParam{"predlay", 0.0f, 0.0f, 1.0f, "Predlay", ""},
    AirwindowsExtractedParam{"wetness", 0.25f, 0.0f, 1.0f, "Wetness", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 5> kAirwindowsParams_kPlateA = {{
    AirwindowsExtractedParam{"input_pad", 1.0f, 0.0f, 1.0f, "InputPad", ""},
    AirwindowsExtractedParam{"damping", 0.5f, 0.0f, 1.0f, "Damping", ""},
    AirwindowsExtractedParam{"low__cut", 1.0f, 0.0f, 1.0f, "Low Cut", ""},
    AirwindowsExtractedParam{"predelay", 0.0f, 0.0f, 1.0f, "Predelay", "sec"},
    AirwindowsExtractedParam{"wetness", 0.25f, 0.0f, 1.0f, "Wetness", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 5> kAirwindowsParams_kPlateB = {{
    AirwindowsExtractedParam{"input_pad", 1.0f, 0.0f, 1.0f, "InputPad", ""},
    AirwindowsExtractedParam{"damping", 0.5f, 0.0f, 1.0f, "Damping", ""},
    AirwindowsExtractedParam{"low__cut", 1.0f, 0.0f, 1.0f, "Low Cut", ""},
    AirwindowsExtractedParam{"predelay", 0.0f, 0.0f, 1.0f, "Predelay", "sec"},
    AirwindowsExtractedParam{"wetness", 0.25f, 0.0f, 1.0f, "Wetness", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 5> kAirwindowsParams_kPlateC = {{
    AirwindowsExtractedParam{"input_pad", 1.0f, 0.0f, 1.0f, "InputPad", ""},
    AirwindowsExtractedParam{"damping", 0.5f, 0.0f, 1.0f, "Damping", ""},
    AirwindowsExtractedParam{"low__cut", 1.0f, 0.0f, 1.0f, "Low Cut", ""},
    AirwindowsExtractedParam{"predelay", 0.0f, 0.0f, 1.0f, "Predelay", "sec"},
    AirwindowsExtractedParam{"wetness", 0.25f, 0.0f, 1.0f, "Wetness", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 5> kAirwindowsParams_kPlateD = {{
    AirwindowsExtractedParam{"input_pad", 1.0f, 0.0f, 1.0f, "InputPad", ""},
    AirwindowsExtractedParam{"damping", 0.5f, 0.0f, 1.0f, "Damping", ""},
    AirwindowsExtractedParam{"low__cut", 1.0f, 0.0f, 1.0f, "Low Cut", ""},
    AirwindowsExtractedParam{"predelay", 0.0f, 0.0f, 1.0f, "Predelay", "sec"},
    AirwindowsExtractedParam{"wetness", 0.25f, 0.0f, 1.0f, "Wetness", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_kRockstar = {{
    AirwindowsExtractedParam{"regen", 0.5f, 0.0f, 1.0f, "Regen", ""},
    AirwindowsExtractedParam{"positin", 1.0f, 0.0f, 1.0f, "Positin", ""},
    AirwindowsExtractedParam{"down_rez", 1.0f, 0.0f, 1.0f, "DownRez", ""},
    AirwindowsExtractedParam{"dry__wet", 0.5f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 5> kAirwindowsParams_kRockstar2 = {{
    AirwindowsExtractedParam{"regen", 0.5f, 0.0f, 1.0f, "Regen", ""},
    AirwindowsExtractedParam{"positin", 0.75f, 0.0f, 1.0f, "Positin", ""},
    AirwindowsExtractedParam{"down_rez", 1.0f, 0.0f, 1.0f, "DownRez", ""},
    AirwindowsExtractedParam{"distnce", 0.25f, 0.0f, 1.0f, "Distnce", ""},
    AirwindowsExtractedParam{"dry__wet", 0.5f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 6> kAirwindowsParams_kStation = {{
    AirwindowsExtractedParam{"regen", 0.5f, 0.0f, 1.0f, "Regen", ""},
    AirwindowsExtractedParam{"derez", 0.5f, 0.0f, 1.0f, "Derez", ""},
    AirwindowsExtractedParam{"filter", 0.5f, 0.0f, 1.0f, "Filter", ""},
    AirwindowsExtractedParam{"early_rf", 0.5f, 0.0f, 1.0f, "EarlyRF", ""},
    AirwindowsExtractedParam{"positin", 0.9f, 0.0f, 1.0f, "Positin", ""},
    AirwindowsExtractedParam{"dry__wet", 0.5f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 6> kAirwindowsParams_kWoodRoom = {{
    AirwindowsExtractedParam{"regen", 0.5f, 0.0f, 1.0f, "Regen", ""},
    AirwindowsExtractedParam{"derez", 0.5f, 0.0f, 1.0f, "Derez", ""},
    AirwindowsExtractedParam{"filter", 0.25f, 0.0f, 1.0f, "Filter", ""},
    AirwindowsExtractedParam{"early_rf", 0.5f, 0.0f, 1.0f, "EarlyRF", ""},
    AirwindowsExtractedParam{"positin", 0.75f, 0.0f, 1.0f, "Positin", ""},
    AirwindowsExtractedParam{"dry__wet", 0.5f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_LeadAmp = {{
    AirwindowsExtractedParam{"gain", 0.5f, 0.0f, 1.0f, "Gain", ""},
    AirwindowsExtractedParam{"tone", 0.5f, 0.0f, 1.0f, "Tone", ""},
    AirwindowsExtractedParam{"output", 0.8f, 0.0f, 1.0f, "Output", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 0> kAirwindowsParams_LeftoMono = {{
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_LilAmp = {{
    AirwindowsExtractedParam{"gain", 0.5f, 0.0f, 1.0f, "Gain", ""},
    AirwindowsExtractedParam{"tone", 0.5f, 0.0f, 1.0f, "Tone", ""},
    AirwindowsExtractedParam{"output", 0.8f, 0.0f, 1.0f, "Output", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 5> kAirwindowsParams_Logical4 = {{
    AirwindowsExtractedParam{"threshold", 0.5f, 0.0f, 1.0f, "Threshold", "dB"},
    AirwindowsExtractedParam{"ratio", 0.2f, 0.0f, 1.0f, "Ratio", "/1"},
    AirwindowsExtractedParam{"speed", 0.19202f, 0.0f, 1.0f, "Speed", "ms"},
    AirwindowsExtractedParam{"makeup_gn", 0.5f, 0.0f, 1.0f, "MakeupGn", "dB"},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_Longhand = {{
    AirwindowsExtractedParam{"input", 0.445f, 0.0f, 1.0f, "Input", ""},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_Loud = {{
    AirwindowsExtractedParam{"boost", 0.0f, 0.0f, 1.0f, "Boost", ""},
    AirwindowsExtractedParam{"output__level", 1.0f, 0.0f, 1.0f, "Output Level", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_Lowpass = {{
    AirwindowsExtractedParam{"lowpass", 1.0f, 0.0f, 1.0f, "Lowpass", ""},
    AirwindowsExtractedParam{"soft__hard", 0.5f, 0.0f, 1.0f, "Soft/Hard", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_Lowpass2 = {{
    AirwindowsExtractedParam{"lowpass", 0.0f, 0.0f, 1.0f, "Lowpass", ""},
    AirwindowsExtractedParam{"sft__hrd", 0.5f, 0.0f, 1.0f, "Sft/Hrd", ""},
    AirwindowsExtractedParam{"poles", 0.25f, 0.0f, 1.0f, "Poles", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 0> kAirwindowsParams_LRConvolve = {{
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_LRConvolve2 = {{
    AirwindowsExtractedParam{"soar", 0.0f, 0.0f, 1.0f, "Soar", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_LRConvolve3 = {{
    AirwindowsExtractedParam{"smooth", 0.0f, 0.0f, 1.0f, "Smooth", ""},
    AirwindowsExtractedParam{"channel", 0.0f, 0.0f, 1.0f, "Channel", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_LRFlipTimer = {{
    AirwindowsExtractedParam{"minutes", 0.0f, 0.0f, 1.0f, "Minutes", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_Luxor = {{
    AirwindowsExtractedParam{"hardns", 0.7f, 0.0f, 1.0f, "Hardns", ""},
    AirwindowsExtractedParam{"persnlty", 0.333333f, 0.0f, 1.0f, "Persnlty", ""},
    AirwindowsExtractedParam{"drive", 0.333333f, 0.0f, 1.0f, "Drive", ""},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 5> kAirwindowsParams_MackEQ = {{
    AirwindowsExtractedParam{"trim", 0.1f, 0.0f, 1.0f, "Trim", ""},
    AirwindowsExtractedParam{"hi", 0.5f, 0.0f, 1.0f, "Hi", ""},
    AirwindowsExtractedParam{"lo", 0.5f, 0.0f, 1.0f, "Lo", ""},
    AirwindowsExtractedParam{"gain", 1.0f, 0.0f, 1.0f, "Gain", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_Mackity = {{
    AirwindowsExtractedParam{"in__trim", 0.1f, 0.0f, 1.0f, "In Trim", ""},
    AirwindowsExtractedParam{"out__pad", 1.0f, 0.0f, 1.0f, "Out Pad", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 6> kAirwindowsParams_Mastering = {{
    AirwindowsExtractedParam{"glue", 0.0f, 0.0f, 1.0f, "Glue", ""},
    AirwindowsExtractedParam{"scope", 0.5f, 0.0f, 1.0f, "Scope", ""},
    AirwindowsExtractedParam{"skronk", 0.5f, 0.0f, 1.0f, "Skronk", ""},
    AirwindowsExtractedParam{"girth", 0.5f, 0.0f, 1.0f, "Girth", ""},
    AirwindowsExtractedParam{"drive", 0.5f, 0.0f, 1.0f, "Drive", ""},
    AirwindowsExtractedParam{"dither", 1.0f, 0.0f, 1.0f, "Dither", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 7> kAirwindowsParams_Mastering2 = {{
    AirwindowsExtractedParam{"sidepas", 0.0f, 0.0f, 1.0f, "Sidepas", ""},
    AirwindowsExtractedParam{"glue", 0.0f, 0.0f, 1.0f, "Glue", ""},
    AirwindowsExtractedParam{"scope", 0.5f, 0.0f, 1.0f, "Scope", ""},
    AirwindowsExtractedParam{"skronk", 0.5f, 0.0f, 1.0f, "Skronk", ""},
    AirwindowsExtractedParam{"girth", 0.5f, 0.0f, 1.0f, "Girth", ""},
    AirwindowsExtractedParam{"drive", 0.5f, 0.0f, 1.0f, "Drive", ""},
    AirwindowsExtractedParam{"dither", 1.0f, 0.0f, 1.0f, "Dither", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 7> kAirwindowsParams_MatrixVerb = {{
    AirwindowsExtractedParam{"filter", 1.0f, 0.0f, 1.0f, "Filter", ""},
    AirwindowsExtractedParam{"damping", 0.0f, 0.0f, 1.0f, "Damping", ""},
    AirwindowsExtractedParam{"speed", 0.0f, 0.0f, 1.0f, "Speed", ""},
    AirwindowsExtractedParam{"vibrato", 0.0f, 0.0f, 1.0f, "Vibrato", ""},
    AirwindowsExtractedParam{"rm_size", 0.5f, 0.0f, 1.0f, "RmSize", ""},
    AirwindowsExtractedParam{"flavor", 0.5f, 0.0f, 1.0f, "Flavor", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_Melt = {{
    AirwindowsExtractedParam{"depth", 0.25f, 0.0f, 1.0f, "Depth", ""},
    AirwindowsExtractedParam{"range", 0.75f, 0.0f, 1.0f, "Range", ""},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_MidAmp = {{
    AirwindowsExtractedParam{"gain", 0.5f, 0.0f, 1.0f, "Gain", ""},
    AirwindowsExtractedParam{"tone", 0.5f, 0.0f, 1.0f, "Tone", ""},
    AirwindowsExtractedParam{"output", 0.8f, 0.0f, 1.0f, "Output", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_MidSide = {{
    AirwindowsExtractedParam{"mid__side", 0.5f, 0.0f, 1.0f, "Mid/Side", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_Mojo = {{
    AirwindowsExtractedParam{"input", 0.5f, 0.0f, 1.0f, "Input", "dB"},
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_Monitoring = {{
    AirwindowsExtractedParam{"monitor", 0.0f, 0.0f, 1.0f, "Monitor", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_Monitoring2 = {{
    AirwindowsExtractedParam{"monitor", 0.0f, 0.0f, 1.0f, "Monitor", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_Monitoring3 = {{
    AirwindowsExtractedParam{"monitor", 0.0f, 0.0f, 1.0f, "Monitor", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_MoNoam = {{
    AirwindowsExtractedParam{"monotor", 0.0f, 0.0f, 1.0f, "Monotor", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_MSFlipTimer = {{
    AirwindowsExtractedParam{"minutes", 0.0f, 0.0f, 1.0f, "Minutes", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 9> kAirwindowsParams_MultiBandDistortion = {{
    AirwindowsExtractedParam{"freqcy", 0.5f, 0.0f, 1.0f, "Freqcy", ""},
    AirwindowsExtractedParam{"h__gain", 0.0f, 0.0f, 1.0f, "H Gain", "dB"},
    AirwindowsExtractedParam{"l__gain", 0.0f, 0.0f, 1.0f, "L Gain", "dB"},
    AirwindowsExtractedParam{"h__hard", 0.0f, 0.0f, 1.0f, "H Hard", ""},
    AirwindowsExtractedParam{"l__hard", 0.0f, 0.0f, 1.0f, "L Hard", ""},
    AirwindowsExtractedParam{"h__mtrx", 1.0f, 0.0f, 1.0f, "H Mtrx", ""},
    AirwindowsExtractedParam{"l__mtrx", 1.0f, 0.0f, 1.0f, "L Mtrx", ""},
    AirwindowsExtractedParam{"stable", 1.0f, 0.0f, 1.0f, "Stable", ""},
    AirwindowsExtractedParam{"output", 0.75f, 0.0f, 1.0f, "Output", "dB"},
}};

inline constexpr std::array<AirwindowsExtractedParam, 5> kAirwindowsParams_MV = {{
    AirwindowsExtractedParam{"depth", 0.5f, 0.0f, 1.0f, "Depth", ""},
    AirwindowsExtractedParam{"bright", 0.5f, 0.0f, 1.0f, "Bright", ""},
    AirwindowsExtractedParam{"regen", 0.5f, 0.0f, 1.0f, "Regen", ""},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 5> kAirwindowsParams_MV2 = {{
    AirwindowsExtractedParam{"depth", 0.5f, 0.0f, 1.0f, "Depth", ""},
    AirwindowsExtractedParam{"bright", 0.5f, 0.0f, 1.0f, "Bright", ""},
    AirwindowsExtractedParam{"regen", 0.5f, 0.0f, 1.0f, "Regen", ""},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 0> kAirwindowsParams_NaturalizeDither = {{
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_NCSeventeen = {{
    AirwindowsExtractedParam{"louder", 0.0f, 0.0f, 1.0f, "LOUDER", "dB"},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_Neverland = {{
    AirwindowsExtractedParam{"hardns", 0.74f, 0.0f, 1.0f, "Hardns", ""},
    AirwindowsExtractedParam{"persnlty", 0.333333f, 0.0f, 1.0f, "Persnlty", ""},
    AirwindowsExtractedParam{"drive", 0.333333f, 0.0f, 1.0f, "Drive", ""},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_Nikola = {{
    AirwindowsExtractedParam{"voltage", 0.0f, 0.0f, 1.0f, "Voltage", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_NodeDither = {{
    AirwindowsExtractedParam{"node", 0.04f, 0.0f, 1.0f, "Node", "samples"},
    AirwindowsExtractedParam{"phase", 0.0f, 0.0f, 1.0f, "Phase", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 6> kAirwindowsParams_Noise = {{
    AirwindowsExtractedParam{"high_cut", 0.5f, 0.0f, 1.0f, "HighCut", ""},
    AirwindowsExtractedParam{"low_cut", 0.5f, 0.0f, 1.0f, "LowCut", ""},
    AirwindowsExtractedParam{"l_shape", 0.5f, 0.0f, 1.0f, "LShape", ""},
    AirwindowsExtractedParam{"decay", 1.0f, 0.0f, 1.0f, "Decay", ""},
    AirwindowsExtractedParam{"distnc", 0.0f, 0.0f, 1.0f, "Distnc", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 6> kAirwindowsParams_NonlinearSpace = {{
    AirwindowsExtractedParam{"smp_rate", 0.3f, 0.0f, 1.0f, "SmpRate", ""},
    AirwindowsExtractedParam{"livenes", 0.5f, 0.0f, 1.0f, "Livenes", ""},
    AirwindowsExtractedParam{"treble", 0.5f, 0.0f, 1.0f, "Treble", ""},
    AirwindowsExtractedParam{"bass", 0.5f, 0.0f, 1.0f, "Bass", ""},
    AirwindowsExtractedParam{"nonlin", 0.5f, 0.0f, 1.0f, "Nonlin", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 0> kAirwindowsParams_NotJustAnotherCD = {{
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_NotJustAnotherDither = {{
    AirwindowsExtractedParam{"quant", 1.0f, 0.0f, 1.0f, "Quant", ""},
    AirwindowsExtractedParam{"de_rez", 0.0f, 0.0f, 1.0f, "DeRez", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 5> kAirwindowsParams_OneCornerClip = {{
    AirwindowsExtractedParam{"input", 0.333333f, 0.0f, 1.0f, "Input", ""},
    AirwindowsExtractedParam{"pos__thr", 0.966f, 0.0f, 1.0f, "Pos Thr", ""},
    AirwindowsExtractedParam{"neg__thr", 0.966f, 0.0f, 1.0f, "Neg Thr", ""},
    AirwindowsExtractedParam{"voicing", 0.618f, 0.0f, 1.0f, "Voicing", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 6> kAirwindowsParams_OrbitKick = {{
    AirwindowsExtractedParam{"drop", 0.5f, 0.0f, 1.0f, "Drop", ""},
    AirwindowsExtractedParam{"shape", 0.5f, 0.0f, 1.0f, "Shape", ""},
    AirwindowsExtractedParam{"start", 0.5f, 0.0f, 1.0f, "Start", ""},
    AirwindowsExtractedParam{"finish", 0.5f, 0.0f, 1.0f, "Finish", ""},
    AirwindowsExtractedParam{"thresh", 0.5f, 0.0f, 1.0f, "Thresh", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_Overheads = {{
    AirwindowsExtractedParam{"compr", 0.0f, 0.0f, 1.0f, "Compr", ""},
    AirwindowsExtractedParam{"sharp", 0.5f, 0.0f, 1.0f, "Sharp", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 13> kAirwindowsParams_Pafnuty = {{
    AirwindowsExtractedParam{"second", 0.5f, 0.0f, 1.0f, "Second", ""},
    AirwindowsExtractedParam{"third", 0.5f, 0.0f, 1.0f, "Third", ""},
    AirwindowsExtractedParam{"fourth", 0.5f, 0.0f, 1.0f, "Fourth", ""},
    AirwindowsExtractedParam{"fifth", 0.5f, 0.0f, 1.0f, "Fifth", ""},
    AirwindowsExtractedParam{"sixth", 0.5f, 0.0f, 1.0f, "Sixth", ""},
    AirwindowsExtractedParam{"seventh", 0.5f, 0.0f, 1.0f, "Seventh", ""},
    AirwindowsExtractedParam{"eighth", 0.5f, 0.0f, 1.0f, "Eighth", ""},
    AirwindowsExtractedParam{"ninth", 0.5f, 0.0f, 1.0f, "Ninth", ""},
    AirwindowsExtractedParam{"tenth", 0.5f, 0.0f, 1.0f, "Tenth", ""},
    AirwindowsExtractedParam{"elevnth", 0.5f, 0.0f, 1.0f, "Elevnth", ""},
    AirwindowsExtractedParam{"twelvth", 0.5f, 0.0f, 1.0f, "Twelvth", ""},
    AirwindowsExtractedParam{"thirtnh", 0.5f, 0.0f, 1.0f, "Thirtnh", ""},
    AirwindowsExtractedParam{"inv__wet", 1.0f, 0.0f, 1.0f, "Inv/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 10> kAirwindowsParams_Pafnuty2 = {{
    AirwindowsExtractedParam{"second", 0.5f, 0.0f, 1.0f, "Second", ""},
    AirwindowsExtractedParam{"third", 0.5f, 0.0f, 1.0f, "Third", ""},
    AirwindowsExtractedParam{"fourth", 0.5f, 0.0f, 1.0f, "Fourth", ""},
    AirwindowsExtractedParam{"fifth", 0.5f, 0.0f, 1.0f, "Fifth", ""},
    AirwindowsExtractedParam{"sixth", 0.5f, 0.0f, 1.0f, "Sixth", ""},
    AirwindowsExtractedParam{"seventh", 0.5f, 0.0f, 1.0f, "Seventh", ""},
    AirwindowsExtractedParam{"eighth", 0.5f, 0.0f, 1.0f, "Eighth", ""},
    AirwindowsExtractedParam{"ninth", 0.5f, 0.0f, 1.0f, "Ninth", ""},
    AirwindowsExtractedParam{"tenth", 0.5f, 0.0f, 1.0f, "Tenth", ""},
    AirwindowsExtractedParam{"inv__wet", 0.5f, 0.0f, 1.0f, "Inv/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 10> kAirwindowsParams_Parametric = {{
    AirwindowsExtractedParam{"tr__freq", 0.5f, 0.0f, 1.0f, "Tr Freq", ""},
    AirwindowsExtractedParam{"treble", 0.5f, 0.0f, 1.0f, "Treble", ""},
    AirwindowsExtractedParam{"tr__reso", 0.5f, 0.0f, 1.0f, "Tr Reso", ""},
    AirwindowsExtractedParam{"hm__freq", 0.5f, 0.0f, 1.0f, "HM Freq", ""},
    AirwindowsExtractedParam{"high_mid", 0.5f, 0.0f, 1.0f, "HighMid", ""},
    AirwindowsExtractedParam{"hm__reso", 0.5f, 0.0f, 1.0f, "HM Reso", ""},
    AirwindowsExtractedParam{"lm__freq", 0.5f, 0.0f, 1.0f, "LM Freq", ""},
    AirwindowsExtractedParam{"low_mid", 0.5f, 0.0f, 1.0f, "LowMid", ""},
    AirwindowsExtractedParam{"lm__reso", 0.5f, 0.0f, 1.0f, "LM Reso", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_PaulDither = {{
    AirwindowsExtractedParam{"quant", 1.0f, 0.0f, 1.0f, "Quant", ""},
    AirwindowsExtractedParam{"de_rez", 0.0f, 0.0f, 1.0f, "DeRez", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_PaulWide = {{
    AirwindowsExtractedParam{"quant", 1.0f, 0.0f, 1.0f, "Quant", ""},
    AirwindowsExtractedParam{"de_rez", 0.0f, 0.0f, 1.0f, "DeRez", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_PDBuss = {{
    AirwindowsExtractedParam{"gain", 1.0f, 0.0f, 1.0f, "Gain", ""},
    AirwindowsExtractedParam{"drive", 1.0f, 0.0f, 1.0f, "Drive", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_PDChannel = {{
    AirwindowsExtractedParam{"gain", 1.0f, 0.0f, 1.0f, "Gain", ""},
    AirwindowsExtractedParam{"drive", 1.0f, 0.0f, 1.0f, "Drive", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 0> kAirwindowsParams_PeaksOnly = {{
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_Pear = {{
    AirwindowsExtractedParam{"bit_freq", 1.0f, 0.0f, 1.0f, "bitFreq", ""},
    AirwindowsExtractedParam{"poles", 0.5f, 0.0f, 1.0f, "Poles", ""},
    AirwindowsExtractedParam{"inv__wet", 1.0f, 0.0f, 1.0f, "Inv/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_Pear2 = {{
    AirwindowsExtractedParam{"freq", 1.0f, 0.0f, 1.0f, "Freq", ""},
    AirwindowsExtractedParam{"non_lin", 0.5f, 0.0f, 1.0f, "NonLin", ""},
    AirwindowsExtractedParam{"poles", 0.5f, 0.0f, 1.0f, "Poles", ""},
    AirwindowsExtractedParam{"inv__wet", 1.0f, 0.0f, 1.0f, "Inv/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 6> kAirwindowsParams_PearEQ = {{
    AirwindowsExtractedParam{"high", 0.5f, 0.0f, 1.0f, "High", ""},
    AirwindowsExtractedParam{"h_mid", 0.5f, 0.0f, 1.0f, "HMid", ""},
    AirwindowsExtractedParam{"mid", 0.5f, 0.0f, 1.0f, "Mid", ""},
    AirwindowsExtractedParam{"l_mid", 0.5f, 0.0f, 1.0f, "LMid", ""},
    AirwindowsExtractedParam{"bass", 0.5f, 0.0f, 1.0f, "Bass", ""},
    AirwindowsExtractedParam{"sub", 0.5f, 0.0f, 1.0f, "Sub", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_PearLiteEQ = {{
    AirwindowsExtractedParam{"high", 0.5f, 0.0f, 1.0f, "High", ""},
    AirwindowsExtractedParam{"h_mid", 0.5f, 0.0f, 1.0f, "HMid", ""},
    AirwindowsExtractedParam{"l_mid", 0.5f, 0.0f, 1.0f, "LMid", ""},
    AirwindowsExtractedParam{"bass", 0.5f, 0.0f, 1.0f, "Bass", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_PhaseNudge = {{
    AirwindowsExtractedParam{"phase__nudge", 0.0f, 0.0f, 1.0f, "Phase Nudge", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 6> kAirwindowsParams_PitchDelay = {{
    AirwindowsExtractedParam{"time", 1.0f, 0.0f, 1.0f, "Time", ""},
    AirwindowsExtractedParam{"regen", 0.0f, 0.0f, 1.0f, "Regen", ""},
    AirwindowsExtractedParam{"freq", 0.5f, 0.0f, 1.0f, "Freq", ""},
    AirwindowsExtractedParam{"reso", 0.0f, 0.0f, 1.0f, "Reso", ""},
    AirwindowsExtractedParam{"pitch", 0.5f, 0.0f, 1.0f, "Pitch", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 6> kAirwindowsParams_PitchNasty = {{
    AirwindowsExtractedParam{"note", 0.0f, 0.0f, 1.0f, "Note", ""},
    AirwindowsExtractedParam{"bend", 0.0f, 0.0f, 1.0f, "Bend", ""},
    AirwindowsExtractedParam{"grind", 0.25f, 0.0f, 1.0f, "Grind", ""},
    AirwindowsExtractedParam{"feedbck", 0.0f, 0.0f, 1.0f, "Feedbck", ""},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_PlatinumSlew = {{
    AirwindowsExtractedParam{"slew", 0.0f, 0.0f, 1.0f, "Slew", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_PocketVerbs = {{
    AirwindowsExtractedParam{"type", 0.0f, 0.0f, 1.0f, "Type", ""},
    AirwindowsExtractedParam{"size", 0.5f, 0.0f, 1.0f, "Size", ""},
    AirwindowsExtractedParam{"gating", 0.0f, 0.0f, 1.0f, "Gating", ""},
    AirwindowsExtractedParam{"dry__wet", 0.5f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_Pockey = {{
    AirwindowsExtractedParam{"de_freq", 0.0f, 0.0f, 1.0f, "DeFreq", ""},
    AirwindowsExtractedParam{"de_rez", 0.0f, 0.0f, 1.0f, "DeRez", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_Pockey2 = {{
    AirwindowsExtractedParam{"de_freq", 0.0f, 0.0f, 1.0f, "DeFreq", ""},
    AirwindowsExtractedParam{"de_rez", 0.66f, 0.0f, 1.0f, "DeRez", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_Podcast = {{
    AirwindowsExtractedParam{"boost", 0.5f, 0.0f, 1.0f, "Boost", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_PodcastDeluxe = {{
    AirwindowsExtractedParam{"boost", 0.5f, 0.0f, 1.0f, "Boost", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_Point = {{
    AirwindowsExtractedParam{"input__trim", 0.5f, 0.0f, 1.0f, "Input Trim", "dB"},
    AirwindowsExtractedParam{"point", 0.5f, 0.0f, 1.0f, "Point", ""},
    AirwindowsExtractedParam{"reaction__speed", 0.5f, 0.0f, 1.0f, "Reaction Speed", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 10> kAirwindowsParams_PointyDeluxe = {{
    AirwindowsExtractedParam{"hell", 0.5f, 0.0f, 1.0f, "Hell", ""},
    AirwindowsExtractedParam{"fizz", 0.5f, 0.0f, 1.0f, "Fizz", ""},
    AirwindowsExtractedParam{"pick", 0.5f, 0.0f, 1.0f, "Pick", ""},
    AirwindowsExtractedParam{"satan", 0.5f, 0.0f, 1.0f, "Satan", ""},
    AirwindowsExtractedParam{"danger", 0.5f, 0.0f, 1.0f, "Danger", ""},
    AirwindowsExtractedParam{"crtical", 0.5f, 0.0f, 1.0f, "Crtical", ""},
    AirwindowsExtractedParam{"h__meat", 0.5f, 0.0f, 1.0f, "H Meat", ""},
    AirwindowsExtractedParam{"l__meat", 0.5f, 0.0f, 1.0f, "L Meat", ""},
    AirwindowsExtractedParam{"swing", 0.5f, 0.0f, 1.0f, "Swing", ""},
    AirwindowsExtractedParam{"rarely", 0.5f, 0.0f, 1.0f, "Rarely", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 10> kAirwindowsParams_PointyGuitar = {{
    AirwindowsExtractedParam{"drive", 0.6f, 0.0f, 1.0f, "Drive", ""},
    AirwindowsExtractedParam{"presnce", 0.5f, 0.0f, 1.0f, "Presnce", ""},
    AirwindowsExtractedParam{"high", 0.8f, 0.0f, 1.0f, "High", ""},
    AirwindowsExtractedParam{"mid", 0.6f, 0.0f, 1.0f, "Mid", ""},
    AirwindowsExtractedParam{"low", 0.4f, 0.0f, 1.0f, "Low", ""},
    AirwindowsExtractedParam{"sub", 0.5f, 0.0f, 1.0f, "Sub", ""},
    AirwindowsExtractedParam{"h_speakr", 0.7f, 0.0f, 1.0f, "HSpeakr", ""},
    AirwindowsExtractedParam{"l_speakr", 0.4f, 0.0f, 1.0f, "LSpeakr", ""},
    AirwindowsExtractedParam{"gate", 0.0f, 0.0f, 1.0f, "Gate", ""},
    AirwindowsExtractedParam{"output", 0.7f, 0.0f, 1.0f, "Output", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_Pop = {{
    AirwindowsExtractedParam{"intenst", 0.3f, 0.0f, 1.0f, "Intenst", ""},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 5> kAirwindowsParams_Pop2 = {{
    AirwindowsExtractedParam{"compres", 0.5f, 0.0f, 1.0f, "Compres", ""},
    AirwindowsExtractedParam{"attack", 0.5f, 0.0f, 1.0f, "Attack", ""},
    AirwindowsExtractedParam{"release", 0.5f, 0.0f, 1.0f, "Release", ""},
    AirwindowsExtractedParam{"drive", 0.5f, 0.0f, 1.0f, "Drive", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 8> kAirwindowsParams_Pop3 = {{
    AirwindowsExtractedParam{"thresld", 1.0f, 0.0f, 1.0f, "Thresld", ""},
    AirwindowsExtractedParam{"c__ratio", 0.5f, 0.0f, 1.0f, "C Ratio", ""},
    AirwindowsExtractedParam{"c__atk", 0.5f, 0.0f, 1.0f, "C Atk", ""},
    AirwindowsExtractedParam{"c__rls", 0.5f, 0.0f, 1.0f, "C Rls", ""},
    AirwindowsExtractedParam{"thresld", 0.0f, 0.0f, 1.0f, "Thresld", ""},
    AirwindowsExtractedParam{"g__ratio", 0.5f, 0.0f, 1.0f, "G Ratio", ""},
    AirwindowsExtractedParam{"g__sust", 0.5f, 0.0f, 1.0f, "G Sust", ""},
    AirwindowsExtractedParam{"g__rls", 0.5f, 0.0f, 1.0f, "G Rls", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_PowerSag = {{
    AirwindowsExtractedParam{"depth", 0.0f, 0.0f, 1.0f, "Depth", ""},
    AirwindowsExtractedParam{"speed", 0.3f, 0.0f, 1.0f, "Speed", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_PowerSag2 = {{
    AirwindowsExtractedParam{"range", 0.3f, 0.0f, 1.0f, "Range", ""},
    AirwindowsExtractedParam{"inv__wet", 0.5f, 0.0f, 1.0f, "Inv/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_Precious = {{
    AirwindowsExtractedParam{"hardns", 0.7f, 0.0f, 1.0f, "Hardns", ""},
    AirwindowsExtractedParam{"persnlty", 0.333333f, 0.0f, 1.0f, "Persnlty", ""},
    AirwindowsExtractedParam{"drive", 0.333333f, 0.0f, 1.0f, "Drive", ""},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 5> kAirwindowsParams_Preponderant = {{
    AirwindowsExtractedParam{"narrow", 0.25f, 0.0f, 1.0f, "Narrow", ""},
    AirwindowsExtractedParam{"medium", 0.5f, 0.0f, 1.0f, "Medium", ""},
    AirwindowsExtractedParam{"wide", 0.75f, 0.0f, 1.0f, "Wide", ""},
    AirwindowsExtractedParam{"resonan", 0.5f, 0.0f, 1.0f, "Resonan", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_Pressure4 = {{
    AirwindowsExtractedParam{"pressure", 0.0f, 0.0f, 1.0f, "Pressure", ""},
    AirwindowsExtractedParam{"speed", 0.2f, 0.0f, 1.0f, "Speed", ""},
    AirwindowsExtractedParam{"mewiness", 1.0f, 0.0f, 1.0f, "Mewiness", ""},
    AirwindowsExtractedParam{"output__gain", 1.0f, 0.0f, 1.0f, "Output Gain", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 6> kAirwindowsParams_Pressure5 = {{
    AirwindowsExtractedParam{"pressre", 0.0f, 0.0f, 1.0f, "Pressre", ""},
    AirwindowsExtractedParam{"speed", 0.25f, 0.0f, 1.0f, "Speed", ""},
    AirwindowsExtractedParam{"mewines", 1.0f, 0.0f, 1.0f, "Mewines", ""},
    AirwindowsExtractedParam{"paw_claw", 0.5f, 0.0f, 1.0f, "PawClaw", ""},
    AirwindowsExtractedParam{"output", 0.5f, 0.0f, 1.0f, "Output", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_Pressure6 = {{
    AirwindowsExtractedParam{"compres", 0.0f, 0.0f, 1.0f, "Compres", ""},
    AirwindowsExtractedParam{"ratio", 1.0f, 0.0f, 1.0f, "Ratio", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_PrimeFIR = {{
    AirwindowsExtractedParam{"freq", 1.0f, 0.0f, 1.0f, "Freq", ""},
    AirwindowsExtractedParam{"window", 0.5f, 0.0f, 1.0f, "Window", ""},
    AirwindowsExtractedParam{"prime", 0.0f, 0.0f, 1.0f, "Prime", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 10> kAirwindowsParams_PunchyDeluxe = {{
    AirwindowsExtractedParam{"hell", 0.5f, 0.0f, 1.0f, "Hell", ""},
    AirwindowsExtractedParam{"fizz", 0.5f, 0.0f, 1.0f, "Fizz", ""},
    AirwindowsExtractedParam{"pick", 0.5f, 0.0f, 1.0f, "Pick", ""},
    AirwindowsExtractedParam{"satan", 0.5f, 0.0f, 1.0f, "Satan", ""},
    AirwindowsExtractedParam{"danger", 0.5f, 0.0f, 1.0f, "Danger", ""},
    AirwindowsExtractedParam{"crtical", 0.5f, 0.0f, 1.0f, "Crtical", ""},
    AirwindowsExtractedParam{"h__meat", 0.5f, 0.0f, 1.0f, "H Meat", ""},
    AirwindowsExtractedParam{"l__meat", 0.5f, 0.0f, 1.0f, "L Meat", ""},
    AirwindowsExtractedParam{"swing", 0.5f, 0.0f, 1.0f, "Swing", ""},
    AirwindowsExtractedParam{"rarely", 0.5f, 0.0f, 1.0f, "Rarely", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 10> kAirwindowsParams_PunchyGuitar = {{
    AirwindowsExtractedParam{"drive", 0.7f, 0.0f, 1.0f, "Drive", ""},
    AirwindowsExtractedParam{"presnce", 0.1f, 0.0f, 1.0f, "Presnce", ""},
    AirwindowsExtractedParam{"high", 0.8f, 0.0f, 1.0f, "High", ""},
    AirwindowsExtractedParam{"mid", 0.6f, 0.0f, 1.0f, "Mid", ""},
    AirwindowsExtractedParam{"low", 0.4f, 0.0f, 1.0f, "Low", ""},
    AirwindowsExtractedParam{"sub", 0.1f, 0.0f, 1.0f, "Sub", ""},
    AirwindowsExtractedParam{"h_speakr", 0.8f, 0.0f, 1.0f, "HSpeakr", ""},
    AirwindowsExtractedParam{"l_speakr", 0.5f, 0.0f, 1.0f, "LSpeakr", ""},
    AirwindowsExtractedParam{"gate", 0.0f, 0.0f, 1.0f, "Gate", ""},
    AirwindowsExtractedParam{"output", 0.3f, 0.0f, 1.0f, "Output", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_PurestAir = {{
    AirwindowsExtractedParam{"boost", 0.5f, 0.0f, 1.0f, "Boost", ""},
    AirwindowsExtractedParam{"limit", 0.0f, 0.0f, 1.0f, "Limit", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 0> kAirwindowsParams_PurestConsole2Buss = {{
}};

inline constexpr std::array<AirwindowsExtractedParam, 0> kAirwindowsParams_PurestConsole2Channel = {{
}};

inline constexpr std::array<AirwindowsExtractedParam, 0> kAirwindowsParams_PurestConsole3Buss = {{
}};

inline constexpr std::array<AirwindowsExtractedParam, 0> kAirwindowsParams_PurestConsole3Channel = {{
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_PurestConsole4Buss = {{
    AirwindowsExtractedParam{"smooth", 0.5f, 0.0f, 1.0f, "Smooth", ""},
    AirwindowsExtractedParam{"pan", 0.5f, 0.0f, 1.0f, "Pan", ""},
    AirwindowsExtractedParam{"fader", 1.0f, 0.0f, 1.0f, "Fader", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_PurestConsole4Channel = {{
    AirwindowsExtractedParam{"smooth", 0.5f, 0.0f, 1.0f, "Smooth", ""},
    AirwindowsExtractedParam{"pan", 0.5f, 0.0f, 1.0f, "Pan", ""},
    AirwindowsExtractedParam{"fader", 1.0f, 0.0f, 1.0f, "Fader", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 0> kAirwindowsParams_PurestConsoleBuss = {{
}};

inline constexpr std::array<AirwindowsExtractedParam, 0> kAirwindowsParams_PurestConsoleChannel = {{
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_PurestDrive = {{
    AirwindowsExtractedParam{"drive", 0.0f, 0.0f, 1.0f, "Drive", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_PurestDualPan = {{
    AirwindowsExtractedParam{"l__vol", 0.5f, 0.0f, 1.0f, "L Vol", ""},
    AirwindowsExtractedParam{"l__pan", 0.0f, 0.0f, 1.0f, "L Pan", ""},
    AirwindowsExtractedParam{"r__vol", 0.5f, 0.0f, 1.0f, "R Vol", ""},
    AirwindowsExtractedParam{"r__pan", 1.0f, 0.0f, 1.0f, "R Pan", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 5> kAirwindowsParams_PurestEcho = {{
    AirwindowsExtractedParam{"time", 1.0f, 0.0f, 1.0f, "Time", ""},
    AirwindowsExtractedParam{"tap_1", 1.0f, 0.0f, 1.0f, "Tap 1", ""},
    AirwindowsExtractedParam{"tap_2", 0.0f, 0.0f, 1.0f, "Tap 2", ""},
    AirwindowsExtractedParam{"tap_3", 0.0f, 0.0f, 1.0f, "Tap 3", ""},
    AirwindowsExtractedParam{"tap_4", 0.0f, 0.0f, 1.0f, "Tap 4", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_PurestFade = {{
    AirwindowsExtractedParam{"gain", 0.5f, 0.0f, 1.0f, "Gain", "dB"},
    AirwindowsExtractedParam{"fade", 1.0f, 0.0f, 1.0f, "Fade", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_PurestGain = {{
    AirwindowsExtractedParam{"gain", 0.5f, 0.0f, 1.0f, "Gain", "dB"},
    AirwindowsExtractedParam{"slow__fade", 1.0f, 0.0f, 1.0f, "Slow Fade", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_PurestSaturation = {{
    AirwindowsExtractedParam{"input", 0.1f, 0.0f, 1.0f, "Input", ""},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_PurestSquish = {{
    AirwindowsExtractedParam{"squish", 0.0f, 0.0f, 1.0f, "Squish", ""},
    AirwindowsExtractedParam{"bass_blm", 0.0f, 0.0f, 1.0f, "BassBlm", ""},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_PurestWarm = {{
    AirwindowsExtractedParam{"polarity", 0.0f, 0.0f, 1.0f, "Polarity", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_PurestWarm2 = {{
    AirwindowsExtractedParam{"dry__pos", 0.0f, 0.0f, 1.0f, "Dry/Pos", ""},
    AirwindowsExtractedParam{"dry__neg", 0.0f, 0.0f, 1.0f, "Dry/Neg", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_PurestWarm3 = {{
    AirwindowsExtractedParam{"asym", 0.5f, 0.0f, 1.0f, "Asym", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_Pyewacket = {{
    AirwindowsExtractedParam{"input__gain", 0.5f, 0.0f, 1.0f, "Input Gain", "dB"},
    AirwindowsExtractedParam{"release", 0.5f, 0.0f, 1.0f, "Release", ""},
    AirwindowsExtractedParam{"output__gain", 0.5f, 0.0f, 1.0f, "Output Gain", "dB"},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_RawGlitters = {{
    AirwindowsExtractedParam{"quant", 1.0f, 0.0f, 1.0f, "Quant", ""},
    AirwindowsExtractedParam{"de_rez", 0.0f, 0.0f, 1.0f, "DeRez", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_RawTimbers = {{
    AirwindowsExtractedParam{"quant", 1.0f, 0.0f, 1.0f, "Quant", ""},
    AirwindowsExtractedParam{"de_rez", 0.0f, 0.0f, 1.0f, "DeRez", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 0> kAirwindowsParams_Recurve = {{
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_Remap = {{
    AirwindowsExtractedParam{"remap", 0.5f, 0.0f, 1.0f, "Remap", ""},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 9> kAirwindowsParams_ResEQ = {{
    AirwindowsExtractedParam{"reso_1", 0.0f, 0.0f, 1.0f, "Reso 1", ""},
    AirwindowsExtractedParam{"reso_2", 0.0f, 0.0f, 1.0f, "Reso 2", ""},
    AirwindowsExtractedParam{"reso_3", 0.0f, 0.0f, 1.0f, "Reso 3", ""},
    AirwindowsExtractedParam{"reso_4", 0.0f, 0.0f, 1.0f, "Reso 4", ""},
    AirwindowsExtractedParam{"reso_5", 0.0f, 0.0f, 1.0f, "Reso 5", ""},
    AirwindowsExtractedParam{"reso_6", 0.0f, 0.0f, 1.0f, "Reso 6", ""},
    AirwindowsExtractedParam{"reso_7", 0.0f, 0.0f, 1.0f, "Reso 7", ""},
    AirwindowsExtractedParam{"reso_8", 0.0f, 0.0f, 1.0f, "Reso 8", ""},
    AirwindowsExtractedParam{"dry__wet", 0.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_ResEQ2 = {{
    AirwindowsExtractedParam{"m_sweep", 0.5f, 0.0f, 1.0f, "MSweep", ""},
    AirwindowsExtractedParam{"m_boost", 0.0f, 0.0f, 1.0f, "MBoost", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_RetroBass = {{
    AirwindowsExtractedParam{"freq", 0.5f, 0.0f, 1.0f, "Freq", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_Reverb = {{
    AirwindowsExtractedParam{"big", 0.5f, 0.0f, 1.0f, "Big", ""},
    AirwindowsExtractedParam{"wet", 0.5f, 0.0f, 1.0f, "Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_Righteous4 = {{
    AirwindowsExtractedParam{"l_target", 0.0f, 0.0f, 1.0f, "LTarget", "dB"},
    AirwindowsExtractedParam{"bt_depth", 0.0f, 0.0f, 1.0f, "BtDepth", "bit"},
}};

inline constexpr std::array<AirwindowsExtractedParam, 0> kAirwindowsParams_RightoMono = {{
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_RingModulator = {{
    AirwindowsExtractedParam{"freq", 0.5f, 0.0f, 1.0f, "Freq", ""},
    AirwindowsExtractedParam{"freq", 0.5f, 0.0f, 1.0f, "Freq", ""},
    AirwindowsExtractedParam{"soar", 0.0f, 0.0f, 1.0f, "Soar", ""},
    AirwindowsExtractedParam{"dry__wet", 0.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_SampleDelay = {{
    AirwindowsExtractedParam{"ms", 0.0f, 0.0f, 1.0f, "ms", ""},
    AirwindowsExtractedParam{"samples", 0.0f, 0.0f, 1.0f, "Samples", ""},
    AirwindowsExtractedParam{"subsamp", 0.0f, 0.0f, 1.0f, "Subsamp", ""},
    AirwindowsExtractedParam{"inv__wet", 1.0f, 0.0f, 1.0f, "Inv/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_Shape = {{
    AirwindowsExtractedParam{"shape", 0.5f, 0.0f, 1.0f, "Shape", ""},
    AirwindowsExtractedParam{"neg__pos", 0.5f, 0.0f, 1.0f, "Neg/Pos", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_ShortBuss = {{
    AirwindowsExtractedParam{"short_bs", 0.5f, 0.0f, 1.0f, "ShortBs", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_SideDull = {{
    AirwindowsExtractedParam{"cutoff", 0.0f, 0.0f, 1.0f, "Cutoff", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_Sidepass = {{
    AirwindowsExtractedParam{"cutoff", 0.0f, 0.0f, 1.0f, "Cutoff", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_Silhouette = {{
    AirwindowsExtractedParam{"dry__wet", 0.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_Silken = {{
    AirwindowsExtractedParam{"silken", 0.0f, 0.0f, 1.0f, "Silken", ""},
    AirwindowsExtractedParam{"freq", 0.5f, 0.0f, 1.0f, "Freq", ""},
    AirwindowsExtractedParam{"window", 0.5f, 0.0f, 1.0f, "Window", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_Sinew = {{
    AirwindowsExtractedParam{"sinew", 0.5f, 0.0f, 1.0f, "Sinew", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_SingleEndedTriode = {{
    AirwindowsExtractedParam{"triode", 0.0f, 0.0f, 1.0f, "Triode", ""},
    AirwindowsExtractedParam{"clas_ab", 0.0f, 0.0f, 1.0f, "Clas AB", ""},
    AirwindowsExtractedParam{"clas_b", 0.0f, 0.0f, 1.0f, "Clas B", ""},
    AirwindowsExtractedParam{"dry__wet", 0.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 0> kAirwindowsParams_Slew = {{
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_Slew2 = {{
    AirwindowsExtractedParam{"clamping", 0.0f, 0.0f, 1.0f, "Clamping", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_Slew3 = {{
    AirwindowsExtractedParam{"clampng", 0.0f, 0.0f, 1.0f, "Clampng", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_Slew4 = {{
    AirwindowsExtractedParam{"compres", 0.5f, 0.0f, 1.0f, "Compres", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 0> kAirwindowsParams_SlewOnly = {{
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_SlewSonic = {{
    AirwindowsExtractedParam{"mute", 0.5f, 0.0f, 1.0f, "Mute", "kHz"},
    AirwindowsExtractedParam{"bright", 0.0f, 0.0f, 1.0f, "Bright", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_Smooth = {{
    AirwindowsExtractedParam{"smooth", 0.0f, 0.0f, 1.0f, "Smooth", ""},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 5> kAirwindowsParams_SmoothEQ = {{
    AirwindowsExtractedParam{"high", 0.5f, 0.0f, 1.0f, "High", ""},
    AirwindowsExtractedParam{"mid", 0.5f, 0.0f, 1.0f, "Mid", ""},
    AirwindowsExtractedParam{"low", 0.5f, 0.0f, 1.0f, "Low", ""},
    AirwindowsExtractedParam{"xover_h", 0.6f, 0.0f, 1.0f, "XoverH", ""},
    AirwindowsExtractedParam{"xover_l", 0.4f, 0.0f, 1.0f, "XoverL", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 8> kAirwindowsParams_SmoothEQ2 = {{
    AirwindowsExtractedParam{"high", 0.5f, 0.0f, 1.0f, "High", ""},
    AirwindowsExtractedParam{"h_mid", 0.5f, 0.0f, 1.0f, "HMid", ""},
    AirwindowsExtractedParam{"l_mid", 0.5f, 0.0f, 1.0f, "LMid", ""},
    AirwindowsExtractedParam{"bass", 0.5f, 0.0f, 1.0f, "Bass", ""},
    AirwindowsExtractedParam{"high_f", 0.5f, 0.0f, 1.0f, "HighF", ""},
    AirwindowsExtractedParam{"h_mid_f", 0.5f, 0.0f, 1.0f, "HMidF", ""},
    AirwindowsExtractedParam{"l_mid_f", 0.5f, 0.0f, 1.0f, "LMidF", ""},
    AirwindowsExtractedParam{"bass_f", 0.5f, 0.0f, 1.0f, "BassF", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_SmoothEQ3 = {{
    AirwindowsExtractedParam{"high", 0.5f, 0.0f, 1.0f, "High", ""},
    AirwindowsExtractedParam{"mid", 0.5f, 0.0f, 1.0f, "Mid", ""},
    AirwindowsExtractedParam{"bass", 0.5f, 0.0f, 1.0f, "Bass", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 8> kAirwindowsParams_SoftClock = {{
    AirwindowsExtractedParam{"tempo", 0.2f, 0.0f, 1.0f, "Tempo", ""},
    AirwindowsExtractedParam{"count", 0.2f, 0.0f, 1.0f, "Count", ""},
    AirwindowsExtractedParam{"tuple", 0.2f, 0.0f, 1.0f, "Tuple", ""},
    AirwindowsExtractedParam{"swing", 0.0f, 0.0f, 1.0f, "Swing", ""},
    AirwindowsExtractedParam{"big_beat", 0.0f, 0.0f, 1.0f, "BigBeat", ""},
    AirwindowsExtractedParam{"accents", 0.5f, 0.0f, 1.0f, "Accents", ""},
    AirwindowsExtractedParam{"boost", 0.5f, 0.0f, 1.0f, "Boost", ""},
    AirwindowsExtractedParam{"speed", 0.5f, 0.0f, 1.0f, "Speed", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 9> kAirwindowsParams_SoftClock2 = {{
    AirwindowsExtractedParam{"tempo", 0.2f, 0.0f, 1.0f, "Tempo", "bpm"},
    AirwindowsExtractedParam{"count", 0.2f, 0.0f, 1.0f, "Count", ""},
    AirwindowsExtractedParam{"tuple", 0.2f, 0.0f, 1.0f, "Tuple", ""},
    AirwindowsExtractedParam{"swing", 0.0f, 0.0f, 1.0f, "Swing", ""},
    AirwindowsExtractedParam{"peak", 0.0f, 0.0f, 1.0f, "Peak", ""},
    AirwindowsExtractedParam{"valley", 0.0f, 0.0f, 1.0f, "Valley", ""},
    AirwindowsExtractedParam{"accents", 0.5f, 0.0f, 1.0f, "Accents", ""},
    AirwindowsExtractedParam{"boost", 0.5f, 0.0f, 1.0f, "Boost", ""},
    AirwindowsExtractedParam{"speed", 0.5f, 0.0f, 1.0f, "Speed", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 5> kAirwindowsParams_SoftClock3 = {{
    AirwindowsExtractedParam{"tempo", 0.2f, 0.0f, 1.0f, "Tempo", "bpm"},
    AirwindowsExtractedParam{"count", 0.2f, 0.0f, 1.0f, "Count", ""},
    AirwindowsExtractedParam{"tuple", 0.2f, 0.0f, 1.0f, "Tuple", ""},
    AirwindowsExtractedParam{"triplet", 0.0f, 0.0f, 1.0f, "Triplet", ""},
    AirwindowsExtractedParam{"flavor", 0.5f, 0.0f, 1.0f, "Flavor", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_SoftGate = {{
    AirwindowsExtractedParam{"thresh", 0.5f, 0.0f, 1.0f, "Thresh", ""},
    AirwindowsExtractedParam{"darken", 0.5f, 0.0f, 1.0f, "Darken", ""},
    AirwindowsExtractedParam{"silence", 0.0f, 0.0f, 1.0f, "Silence", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_SpatializeDither = {{
    AirwindowsExtractedParam{"quant", 1.0f, 0.0f, 1.0f, "Quant", ""},
    AirwindowsExtractedParam{"de_rez", 0.0f, 0.0f, 1.0f, "DeRez", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 0> kAirwindowsParams_Spiral = {{
}};

inline constexpr std::array<AirwindowsExtractedParam, 5> kAirwindowsParams_Spiral2 = {{
    AirwindowsExtractedParam{"input", 0.5f, 0.0f, 1.0f, "Input", ""},
    AirwindowsExtractedParam{"highpass", 0.0f, 0.0f, 1.0f, "Highpass", ""},
    AirwindowsExtractedParam{"presence", 0.5f, 0.0f, 1.0f, "Presence", ""},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_Spiral3 = {{
    AirwindowsExtractedParam{"input", 0.1f, 0.0f, 1.0f, "Input", ""},
    AirwindowsExtractedParam{"smooth", 0.5f, 0.0f, 1.0f, "Smooth", ""},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_SquareRoot = {{
    AirwindowsExtractedParam{"in", 0.5f, 0.0f, 1.0f, "In", ""},
    AirwindowsExtractedParam{"soar", 0.0f, 0.0f, 1.0f, "Soar", ""},
    AirwindowsExtractedParam{"out", 0.5f, 0.0f, 1.0f, "Out", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 5> kAirwindowsParams_Srsly = {{
    AirwindowsExtractedParam{"center", 0.0f, 0.0f, 1.0f, "Center", ""},
    AirwindowsExtractedParam{"space", 0.0f, 0.0f, 1.0f, "Space", ""},
    AirwindowsExtractedParam{"level", 1.0f, 0.0f, 1.0f, "Level", ""},
    AirwindowsExtractedParam{"q", 0.5f, 0.0f, 1.0f, "Q", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 5> kAirwindowsParams_Srsly2 = {{
    AirwindowsExtractedParam{"center", 0.5f, 0.0f, 1.0f, "Center", ""},
    AirwindowsExtractedParam{"space", 0.5f, 0.0f, 1.0f, "Space", ""},
    AirwindowsExtractedParam{"level", 1.0f, 0.0f, 1.0f, "Level", ""},
    AirwindowsExtractedParam{"q", 0.5f, 0.0f, 1.0f, "Q", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 6> kAirwindowsParams_Srsly3 = {{
    AirwindowsExtractedParam{"center", 0.5f, 0.0f, 1.0f, "Center", ""},
    AirwindowsExtractedParam{"space", 0.5f, 0.0f, 1.0f, "Space", ""},
    AirwindowsExtractedParam{"level", 1.0f, 0.0f, 1.0f, "Level", ""},
    AirwindowsExtractedParam{"q", 0.5f, 0.0f, 1.0f, "Q", ""},
    AirwindowsExtractedParam{"nonlin", 0.5f, 0.0f, 1.0f, "Nonlin", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_StarChild = {{
    AirwindowsExtractedParam{"sustain", 1.0f, 0.0f, 1.0f, "Sustain", ""},
    AirwindowsExtractedParam{"grain", 0.7f, 0.0f, 1.0f, "Grain", ""},
    AirwindowsExtractedParam{"dry__wet", 0.2f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_StarChild2 = {{
    AirwindowsExtractedParam{"sustain", 1.0f, 0.0f, 1.0f, "Sustain", ""},
    AirwindowsExtractedParam{"grain", 0.7f, 0.0f, 1.0f, "Grain", ""},
    AirwindowsExtractedParam{"dry__wet", 0.2f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_StereoChorus = {{
    AirwindowsExtractedParam{"speed", 0.0f, 0.0f, 1.0f, "Speed", ""},
    AirwindowsExtractedParam{"depth", 0.0f, 0.0f, 1.0f, "Depth", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_StereoDoubler = {{
    AirwindowsExtractedParam{"detune", 0.6f, 0.0f, 1.0f, "Detune", ""},
    AirwindowsExtractedParam{"dry__wet", 0.6f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_StereoEnsemble = {{
    AirwindowsExtractedParam{"depth", 0.5f, 0.0f, 1.0f, "Depth", ""},
    AirwindowsExtractedParam{"f_xlevel", 1.0f, 0.0f, 1.0f, "FXlevel", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_StereoFX = {{
    AirwindowsExtractedParam{"wide", 0.0f, 0.0f, 1.0f, "Wide", ""},
    AirwindowsExtractedParam{"mono_bs", 0.0f, 0.0f, 1.0f, "MonoBs", ""},
    AirwindowsExtractedParam{"c_squish", 0.0f, 0.0f, 1.0f, "CSquish", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_Stonefire = {{
    AirwindowsExtractedParam{"air", 0.5f, 0.0f, 1.0f, "Air", ""},
    AirwindowsExtractedParam{"fire", 0.5f, 0.0f, 1.0f, "Fire", ""},
    AirwindowsExtractedParam{"stone", 0.5f, 0.0f, 1.0f, "Stone", ""},
    AirwindowsExtractedParam{"range", 0.5f, 0.0f, 1.0f, "Range", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 10> kAirwindowsParams_StoneFireComp = {{
    AirwindowsExtractedParam{"fire__th", 1.0f, 0.0f, 1.0f, "Fire Th", ""},
    AirwindowsExtractedParam{"attack", 0.5f, 0.0f, 1.0f, "Attack", ""},
    AirwindowsExtractedParam{"release", 0.5f, 0.0f, 1.0f, "Release", ""},
    AirwindowsExtractedParam{"fire", 0.5f, 0.0f, 1.0f, "Fire", ""},
    AirwindowsExtractedParam{"stone_th", 1.0f, 0.0f, 1.0f, "StoneTh", ""},
    AirwindowsExtractedParam{"attack", 0.5f, 0.0f, 1.0f, "Attack", ""},
    AirwindowsExtractedParam{"release", 0.5f, 0.0f, 1.0f, "Release", ""},
    AirwindowsExtractedParam{"stone", 0.5f, 0.0f, 1.0f, "Stone", ""},
    AirwindowsExtractedParam{"range", 0.5f, 0.0f, 1.0f, "Range", ""},
    AirwindowsExtractedParam{"ratio", 1.0f, 0.0f, 1.0f, "Ratio", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_StudioTan = {{
    AirwindowsExtractedParam{"quantzr", 0.0f, 0.0f, 1.0f, "Quantzr", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 0> kAirwindowsParams_SubsOnly = {{
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_SubTight = {{
    AirwindowsExtractedParam{"trim", 0.3f, 0.0f, 1.0f, "Trim", ""},
    AirwindowsExtractedParam{"steep", 0.5f, 0.0f, 1.0f, "Steep", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_Surge = {{
    AirwindowsExtractedParam{"surge", 0.0f, 0.0f, 1.0f, "Surge", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_SurgeTide = {{
    AirwindowsExtractedParam{"surge_node", 0.5f, 0.0f, 1.0f, "SurgeNode", ""},
    AirwindowsExtractedParam{"surge_rate", 0.3f, 0.0f, 1.0f, "SurgeRate", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_Suzan = {{
    AirwindowsExtractedParam{"freq", 0.5f, 0.0f, 1.0f, "Freq", ""},
    AirwindowsExtractedParam{"reso", 0.5f, 0.0f, 1.0f, "Reso", ""},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_Sweeten = {{
    AirwindowsExtractedParam{"sweeten", 0.0f, 0.0f, 1.0f, "Sweeten", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_SweetWide = {{
    AirwindowsExtractedParam{"soar", 0.0f, 0.0f, 1.0f, "Soar", ""},
    AirwindowsExtractedParam{"un__wide", 0.5f, 0.0f, 1.0f, "Un/Wide", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_Swell = {{
    AirwindowsExtractedParam{"thresh", 0.9f, 0.0f, 1.0f, "Thresh", ""},
    AirwindowsExtractedParam{"swell", 0.5f, 0.0f, 1.0f, "Swell", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 8> kAirwindowsParams_TakeCare = {{
    AirwindowsExtractedParam{"speed", 0.15f, 0.0f, 1.0f, "Speed", ""},
    AirwindowsExtractedParam{"rando", 0.5f, 0.0f, 1.0f, "Rando", ""},
    AirwindowsExtractedParam{"depth", 0.5f, 0.0f, 1.0f, "Depth", ""},
    AirwindowsExtractedParam{"regen", 0.5f, 0.0f, 1.0f, "Regen", ""},
    AirwindowsExtractedParam{"derez", 0.5f, 0.0f, 1.0f, "Derez", ""},
    AirwindowsExtractedParam{"buffer", 0.5f, 0.0f, 1.0f, "Buffer", ""},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_Tape = {{
    AirwindowsExtractedParam{"slam", 0.5f, 0.0f, 1.0f, "Slam", "dB"},
    AirwindowsExtractedParam{"bump", 0.5f, 0.0f, 1.0f, "Bump", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_TapeBias = {{
    AirwindowsExtractedParam{"bias", 0.5f, 0.0f, 1.0f, "Bias", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 6> kAirwindowsParams_TapeDelay = {{
    AirwindowsExtractedParam{"dry", 1.0f, 0.0f, 1.0f, "Dry", ""},
    AirwindowsExtractedParam{"wet", 0.0f, 0.0f, 1.0f, "Wet", ""},
    AirwindowsExtractedParam{"delay", 0.5f, 0.0f, 1.0f, "Delay", ""},
    AirwindowsExtractedParam{"feedbak", 0.0f, 0.0f, 1.0f, "Feedbak", ""},
    AirwindowsExtractedParam{"lean__ft", 1.0f, 0.0f, 1.0f, "Lean/Ft", ""},
    AirwindowsExtractedParam{"depth", 0.0f, 0.0f, 1.0f, "Depth", "taps"},
}};

inline constexpr std::array<AirwindowsExtractedParam, 6> kAirwindowsParams_TapeDelay2 = {{
    AirwindowsExtractedParam{"time", 1.0f, 0.0f, 1.0f, "Time", ""},
    AirwindowsExtractedParam{"regen", 0.0f, 0.0f, 1.0f, "Regen", ""},
    AirwindowsExtractedParam{"freq", 0.5f, 0.0f, 1.0f, "Freq", ""},
    AirwindowsExtractedParam{"reso", 0.0f, 0.0f, 1.0f, "Reso", ""},
    AirwindowsExtractedParam{"flutter", 0.0f, 0.0f, 1.0f, "Flutter", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_TapeDither = {{
    AirwindowsExtractedParam{"quant", 1.0f, 0.0f, 1.0f, "Quant", ""},
    AirwindowsExtractedParam{"de_rez", 0.0f, 0.0f, 1.0f, "DeRez", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_TapeDust = {{
    AirwindowsExtractedParam{"dust", 0.0f, 0.0f, 1.0f, "Dust", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_TapeFat = {{
    AirwindowsExtractedParam{"lean__ft", 1.0f, 0.0f, 1.0f, "Lean/Ft", ""},
    AirwindowsExtractedParam{"depth", 0.0f, 0.0f, 1.0f, "Depth", "taps"},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_TapeHack = {{
    AirwindowsExtractedParam{"input", 0.1f, 0.0f, 1.0f, "Input", ""},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_TapeHack2 = {{
    AirwindowsExtractedParam{"input", 0.1f, 0.0f, 1.0f, "Input", ""},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_TapeHack3 = {{
    AirwindowsExtractedParam{"input", 0.1f, 0.0f, 1.0f, "Input", ""},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
    AirwindowsExtractedParam{"smooth", 0.5f, 0.0f, 1.0f, "Smooth", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_Texturize = {{
    AirwindowsExtractedParam{"bright", 0.5f, 0.0f, 1.0f, "Bright", ""},
    AirwindowsExtractedParam{"punchy", 0.5f, 0.0f, 1.0f, "Punchy", ""},
    AirwindowsExtractedParam{"dry__wet", 0.5f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 6> kAirwindowsParams_TexturizeMS = {{
    AirwindowsExtractedParam{"bright_m", 0.5f, 0.0f, 1.0f, "BrightM", ""},
    AirwindowsExtractedParam{"punchy_m", 0.5f, 0.0f, 1.0f, "PunchyM", ""},
    AirwindowsExtractedParam{"dry_wet_m", 0.5f, 0.0f, 1.0f, "DryWetM", ""},
    AirwindowsExtractedParam{"bright_s", 0.5f, 0.0f, 1.0f, "BrightS", ""},
    AirwindowsExtractedParam{"punchy_s", 0.5f, 0.0f, 1.0f, "PunchyS", ""},
    AirwindowsExtractedParam{"dry_wet_s", 0.5f, 0.0f, 1.0f, "DryWetS", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_Thunder = {{
    AirwindowsExtractedParam{"thunder", 0.0f, 0.0f, 1.0f, "Thunder", ""},
    AirwindowsExtractedParam{"output__trim", 1.0f, 0.0f, 1.0f, "Output Trim", "dB"},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_ToneSlant = {{
    AirwindowsExtractedParam{"voicing", 0.0f, 0.0f, 1.0f, "Voicing", "taps"},
    AirwindowsExtractedParam{"highs", 0.0f, 0.0f, 1.0f, "Highs", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 6> kAirwindowsParams_ToTape5 = {{
    AirwindowsExtractedParam{"louder", 0.25f, 0.0f, 1.0f, "Louder", ""},
    AirwindowsExtractedParam{"softer", 0.0f, 0.0f, 1.0f, "Softer", ""},
    AirwindowsExtractedParam{"fatter", 0.0f, 0.0f, 1.0f, "Fatter", ""},
    AirwindowsExtractedParam{"flutter", 0.0f, 0.0f, 1.0f, "Flutter", ""},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 6> kAirwindowsParams_ToTape6 = {{
    AirwindowsExtractedParam{"input", 0.5f, 0.0f, 1.0f, "Input", "dB"},
    AirwindowsExtractedParam{"soften", 0.5f, 0.0f, 1.0f, "Soften", ""},
    AirwindowsExtractedParam{"head_b", 0.5f, 0.0f, 1.0f, "Head B", ""},
    AirwindowsExtractedParam{"flutter", 0.5f, 0.0f, 1.0f, "Flutter", ""},
    AirwindowsExtractedParam{"output", 0.5f, 0.0f, 1.0f, "Output", "dB"},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 10> kAirwindowsParams_ToTape7 = {{
    AirwindowsExtractedParam{"enc_amt", 0.5f, 0.0f, 1.0f, "EncAmt", ""},
    AirwindowsExtractedParam{"enc_freq", 0.5f, 0.0f, 1.0f, "EncFreq", ""},
    AirwindowsExtractedParam{"tape_drv", 0.5f, 0.0f, 1.0f, "TapeDrv", ""},
    AirwindowsExtractedParam{"flutter", 0.5f, 0.0f, 1.0f, "Flutter", ""},
    AirwindowsExtractedParam{"flut_spd", 0.5f, 0.0f, 1.0f, "FlutSpd", ""},
    AirwindowsExtractedParam{"bias", 0.5f, 0.0f, 1.0f, "Bias", ""},
    AirwindowsExtractedParam{"head_bmp", 0.5f, 0.0f, 1.0f, "HeadBmp", ""},
    AirwindowsExtractedParam{"head_frq", 0.5f, 0.0f, 1.0f, "HeadFrq", "hz"},
    AirwindowsExtractedParam{"dec_amt", 0.5f, 0.0f, 1.0f, "DecAmt", ""},
    AirwindowsExtractedParam{"dec_freq", 0.5f, 0.0f, 1.0f, "DecFreq", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 9> kAirwindowsParams_ToTape8 = {{
    AirwindowsExtractedParam{"input", 0.5f, 0.0f, 1.0f, "Input", ""},
    AirwindowsExtractedParam{"tilt", 0.5f, 0.0f, 1.0f, "Tilt", ""},
    AirwindowsExtractedParam{"shape", 0.5f, 0.0f, 1.0f, "Shape", ""},
    AirwindowsExtractedParam{"flutter", 0.5f, 0.0f, 1.0f, "Flutter", ""},
    AirwindowsExtractedParam{"flut_spd", 0.5f, 0.0f, 1.0f, "FlutSpd", ""},
    AirwindowsExtractedParam{"bias", 0.5f, 0.0f, 1.0f, "Bias", ""},
    AirwindowsExtractedParam{"head_bmp", 0.5f, 0.0f, 1.0f, "HeadBmp", ""},
    AirwindowsExtractedParam{"head_frq", 0.5f, 0.0f, 1.0f, "HeadFrq", "hz"},
    AirwindowsExtractedParam{"output", 0.5f, 0.0f, 1.0f, "Output", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 9> kAirwindowsParams_ToTape9 = {{
    AirwindowsExtractedParam{"input", 0.5f, 0.0f, 1.0f, "Input", ""},
    AirwindowsExtractedParam{"tilt", 0.5f, 0.0f, 1.0f, "Tilt", ""},
    AirwindowsExtractedParam{"shape", 0.5f, 0.0f, 1.0f, "Shape", ""},
    AirwindowsExtractedParam{"flutter", 0.5f, 0.0f, 1.0f, "Flutter", ""},
    AirwindowsExtractedParam{"flut_spd", 0.5f, 0.0f, 1.0f, "FlutSpd", ""},
    AirwindowsExtractedParam{"bias", 0.5f, 0.0f, 1.0f, "Bias", ""},
    AirwindowsExtractedParam{"head_bmp", 0.5f, 0.0f, 1.0f, "HeadBmp", ""},
    AirwindowsExtractedParam{"head_frq", 0.5f, 0.0f, 1.0f, "HeadFrq", "hz"},
    AirwindowsExtractedParam{"output", 0.5f, 0.0f, 1.0f, "Output", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_ToVinyl4 = {{
    AirwindowsExtractedParam{"mid__hi_p", 0.203419f, 0.0f, 1.0f, "Mid HiP", "hz"},
    AirwindowsExtractedParam{"side_hi_p", 0.342405f, 0.0f, 1.0f, "SideHiP", "hz"},
    AirwindowsExtractedParam{"h__limit", 0.32f, 0.0f, 1.0f, "H Limit", ""},
    AirwindowsExtractedParam{"gv__wear", 0.064f, 0.0f, 1.0f, "Gv Wear", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_TPDFDither = {{
    AirwindowsExtractedParam{"quant", 1.0f, 0.0f, 1.0f, "Quant", ""},
    AirwindowsExtractedParam{"de_rez", 0.0f, 0.0f, 1.0f, "DeRez", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_TPDFWide = {{
    AirwindowsExtractedParam{"quant", 1.0f, 0.0f, 1.0f, "Quant", ""},
    AirwindowsExtractedParam{"de_rez", 0.0f, 0.0f, 1.0f, "DeRez", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 0> kAirwindowsParams_TransDesk = {{
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_Tremolo = {{
    AirwindowsExtractedParam{"speed", 0.5f, 0.0f, 1.0f, "Speed", ""},
    AirwindowsExtractedParam{"depth", 1.0f, 0.0f, 1.0f, "Depth", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_TremoSquare = {{
    AirwindowsExtractedParam{"freq", 0.5f, 0.0f, 1.0f, "Freq", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_Trianglizer = {{
    AirwindowsExtractedParam{"tri__fat", 0.0f, 0.0f, 1.0f, "Tri/Fat", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_TripleSpread = {{
    AirwindowsExtractedParam{"spread", 0.5f, 0.0f, 1.0f, "Spread", ""},
    AirwindowsExtractedParam{"tighten", 0.5f, 0.0f, 1.0f, "Tighten", ""},
    AirwindowsExtractedParam{"dry__wet", 0.5f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_Tube = {{
    AirwindowsExtractedParam{"tube", 0.0f, 0.0f, 1.0f, "TUBE", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_Tube2 = {{
    AirwindowsExtractedParam{"input", 0.5f, 0.0f, 1.0f, "Input", ""},
    AirwindowsExtractedParam{"tube", 0.5f, 0.0f, 1.0f, "Tube", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 0> kAirwindowsParams_TubeDesk = {{
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_uLawDecode = {{
    AirwindowsExtractedParam{"gain", 1.0f, 0.0f, 1.0f, "Gain", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_uLawEncode = {{
    AirwindowsExtractedParam{"gain", 1.0f, 0.0f, 1.0f, "Gain", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 7> kAirwindowsParams_Ultralight = {{
    AirwindowsExtractedParam{"rt60", 0.65f, 0.0f, 1.0f, "RT60", ""},
    AirwindowsExtractedParam{"furnish", 0.75f, 0.0f, 1.0f, "Furnish", ""},
    AirwindowsExtractedParam{"solid_wl", 0.75f, 0.0f, 1.0f, "SolidWl", ""},
    AirwindowsExtractedParam{"reflc_wl", 0.75f, 0.0f, 1.0f, "ReflcWl", ""},
    AirwindowsExtractedParam{"predlay", 0.54f, 0.0f, 1.0f, "Predlay", ""},
    AirwindowsExtractedParam{"dry__wet", 0.5f, 0.0f, 1.0f, "Dry/Wet", ""},
    AirwindowsExtractedParam{"distnce", 0.0f, 0.0f, 1.0f, "Distnce", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 6> kAirwindowsParams_Ultralight2 = {{
    AirwindowsExtractedParam{"damping", 0.65f, 0.0f, 1.0f, "Damping", ""},
    AirwindowsExtractedParam{"un_solid", 0.75f, 0.0f, 1.0f, "UnSolid", ""},
    AirwindowsExtractedParam{"un_reflc", 0.75f, 0.0f, 1.0f, "UnReflc", ""},
    AirwindowsExtractedParam{"predlay", 0.54f, 0.0f, 1.0f, "Predlay", ""},
    AirwindowsExtractedParam{"dry__wet", 0.5f, 0.0f, 1.0f, "Dry/Wet", ""},
    AirwindowsExtractedParam{"distnce", 0.0f, 0.0f, 1.0f, "Distnce", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 8> kAirwindowsParams_Ultralight3 = {{
    AirwindowsExtractedParam{"damping", 0.3f, 0.0f, 1.0f, "Damping", ""},
    AirwindowsExtractedParam{"un_solid", 0.0f, 0.0f, 1.0f, "UnSolid", ""},
    AirwindowsExtractedParam{"un_reflc", 0.0f, 0.0f, 1.0f, "UnReflc", ""},
    AirwindowsExtractedParam{"predlay", 0.0f, 0.0f, 1.0f, "Predlay", ""},
    AirwindowsExtractedParam{"dry__wet", 0.5f, 0.0f, 1.0f, "Dry/Wet", ""},
    AirwindowsExtractedParam{"frnt__bk", 0.0f, 0.0f, 1.0f, "Frnt/Bk", ""},
    AirwindowsExtractedParam{"bot__top", 0.0f, 0.0f, 1.0f, "Bot/Top", ""},
    AirwindowsExtractedParam{"distnce", 0.0f, 0.0f, 1.0f, "Distnce", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 0> kAirwindowsParams_Ultrasonic = {{
}};

inline constexpr std::array<AirwindowsExtractedParam, 0> kAirwindowsParams_UltrasonicLite = {{
}};

inline constexpr std::array<AirwindowsExtractedParam, 0> kAirwindowsParams_UltrasonicMed = {{
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_UltrasonX = {{
    AirwindowsExtractedParam{"q", 0.5f, 0.0f, 1.0f, "Q", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_UnBox = {{
    AirwindowsExtractedParam{"input", 0.5f, 0.0f, 1.0f, "Input", ""},
    AirwindowsExtractedParam{"un_box", 0.0f, 0.0f, 1.0f, "UnBox", ""},
    AirwindowsExtractedParam{"output", 0.5f, 0.0f, 1.0f, "Output", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_VariMu = {{
    AirwindowsExtractedParam{"intensty", 0.0f, 0.0f, 1.0f, "Intensty", ""},
    AirwindowsExtractedParam{"speed", 0.5f, 0.0f, 1.0f, "Speed", ""},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_Verbity = {{
    AirwindowsExtractedParam{"bigness", 0.25f, 0.0f, 1.0f, "Bigness", ""},
    AirwindowsExtractedParam{"longness", 0.0f, 0.0f, 1.0f, "Longness", ""},
    AirwindowsExtractedParam{"darkness", 0.25f, 0.0f, 1.0f, "Darkness", ""},
    AirwindowsExtractedParam{"wetness", 0.25f, 0.0f, 1.0f, "Wetness", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_Verbity2 = {{
    AirwindowsExtractedParam{"rm_size", 0.5f, 0.0f, 1.0f, "RmSize", ""},
    AirwindowsExtractedParam{"sustain", 0.5f, 0.0f, 1.0f, "Sustain", ""},
    AirwindowsExtractedParam{"mulch", 0.5f, 0.0f, 1.0f, "Mulch", ""},
    AirwindowsExtractedParam{"wetness", 1.0f, 0.0f, 1.0f, "Wetness", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 0> kAirwindowsParams_VerbSevens = {{
}};

inline constexpr std::array<AirwindowsExtractedParam, 0> kAirwindowsParams_VerbSixes = {{
}};

inline constexpr std::array<AirwindowsExtractedParam, 5> kAirwindowsParams_VerbThic = {{
    AirwindowsExtractedParam{"replace", 0.5f, 0.0f, 1.0f, "Replace", ""},
    AirwindowsExtractedParam{"derez", 1.0f, 0.0f, 1.0f, "Derez", ""},
    AirwindowsExtractedParam{"filter", 1.0f, 0.0f, 1.0f, "Filter", ""},
    AirwindowsExtractedParam{"wider", 0.0f, 0.0f, 1.0f, "Wider", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 5> kAirwindowsParams_VerbTiny = {{
    AirwindowsExtractedParam{"replace", 0.5f, 0.0f, 1.0f, "Replace", ""},
    AirwindowsExtractedParam{"derez", 1.0f, 0.0f, 1.0f, "Derez", ""},
    AirwindowsExtractedParam{"filter", 1.0f, 0.0f, 1.0f, "Filter", ""},
    AirwindowsExtractedParam{"wider", 0.0f, 0.0f, 1.0f, "Wider", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 5> kAirwindowsParams_Vibrato = {{
    AirwindowsExtractedParam{"speed", 0.3f, 0.0f, 1.0f, "Speed", ""},
    AirwindowsExtractedParam{"depth", 0.0f, 0.0f, 1.0f, "Depth", ""},
    AirwindowsExtractedParam{"fm_speed", 0.4f, 0.0f, 1.0f, "FMSpeed", ""},
    AirwindowsExtractedParam{"fm_depth", 0.0f, 0.0f, 1.0f, "FMDepth", ""},
    AirwindowsExtractedParam{"inv__wet", 1.0f, 0.0f, 1.0f, "Inv/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_VinylDither = {{
    AirwindowsExtractedParam{"quant", 1.0f, 0.0f, 1.0f, "Quant", ""},
    AirwindowsExtractedParam{"de_rez", 0.0f, 0.0f, 1.0f, "DeRez", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_VoiceOfTheStarship = {{
    AirwindowsExtractedParam{"filter", 0.5f, 0.0f, 1.0f, "Filter", ""},
    AirwindowsExtractedParam{"algrthm", 0.0f, 0.0f, 1.0f, "Algrthm", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_VoiceTrick = {{
    AirwindowsExtractedParam{"tone", 1.0f, 0.0f, 1.0f, "Tone", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_Weave = {{
    AirwindowsExtractedParam{"input", 0.1f, 0.0f, 1.0f, "Input", ""},
    AirwindowsExtractedParam{"smooth", 0.5f, 0.0f, 1.0f, "Smooth", ""},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_Weight = {{
    AirwindowsExtractedParam{"freq", 0.5f, 0.0f, 1.0f, "Freq", "hz"},
    AirwindowsExtractedParam{"weight", 0.0f, 0.0f, 1.0f, "Weight", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_Wider = {{
    AirwindowsExtractedParam{"width", 0.5f, 0.0f, 1.0f, "Width", ""},
    AirwindowsExtractedParam{"center", 0.5f, 0.0f, 1.0f, "Center", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 0> kAirwindowsParams_Wolfbot = {{
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_WoodenBox = {{
    AirwindowsExtractedParam{"select", 0.5f, 0.0f, 1.0f, "Select", ""},
    AirwindowsExtractedParam{"reso", 0.5f, 0.0f, 1.0f, "Reso", ""},
    AirwindowsExtractedParam{"depth", 0.5f, 0.0f, 1.0f, "Depth", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 3> kAirwindowsParams_WoodenChest = {{
    AirwindowsExtractedParam{"select", 0.0f, 0.0f, 1.0f, "Select", ""},
    AirwindowsExtractedParam{"reso", 0.5f, 0.0f, 1.0f, "Reso", ""},
    AirwindowsExtractedParam{"depth", 0.5f, 0.0f, 1.0f, "Depth", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 10> kAirwindowsParams_X2Buss = {{
    AirwindowsExtractedParam{"high", 0.5f, 0.0f, 1.0f, "High", "eq"},
    AirwindowsExtractedParam{"h_mid", 0.5f, 0.0f, 1.0f, "HMid", ""},
    AirwindowsExtractedParam{"l_mid", 0.5f, 0.0f, 1.0f, "LMid", ""},
    AirwindowsExtractedParam{"bass", 0.5f, 0.0f, 1.0f, "Bass", ""},
    AirwindowsExtractedParam{"high_f", 0.5f, 0.0f, 1.0f, "HighF", "freq"},
    AirwindowsExtractedParam{"h_mid_f", 0.5f, 0.0f, 1.0f, "HMidF", ""},
    AirwindowsExtractedParam{"l_mid_f", 0.5f, 0.0f, 1.0f, "LMidF", ""},
    AirwindowsExtractedParam{"bass_f", 0.5f, 0.0f, 1.0f, "BassF", ""},
    AirwindowsExtractedParam{"thresh", 1.0f, 0.0f, 1.0f, "Thresh", "dyn"},
    AirwindowsExtractedParam{"fader", 0.5f, 0.0f, 1.0f, "Fader", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_XBandpass = {{
    AirwindowsExtractedParam{"gain", 0.5f, 0.0f, 1.0f, "Gain", ""},
    AirwindowsExtractedParam{"freq", 0.5f, 0.0f, 1.0f, "Freq", ""},
    AirwindowsExtractedParam{"nuke", 0.0f, 0.0f, 1.0f, "Nuke", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_XHighpass = {{
    AirwindowsExtractedParam{"gain", 0.5f, 0.0f, 1.0f, "Gain", ""},
    AirwindowsExtractedParam{"freq", 1.0f, 0.0f, 1.0f, "Freq", ""},
    AirwindowsExtractedParam{"nuke", 0.0f, 0.0f, 1.0f, "Nuke", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_XLowpass = {{
    AirwindowsExtractedParam{"gain", 0.5f, 0.0f, 1.0f, "Gain", ""},
    AirwindowsExtractedParam{"freq", 1.0f, 0.0f, 1.0f, "Freq", ""},
    AirwindowsExtractedParam{"nuke", 0.0f, 0.0f, 1.0f, "Nuke", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_XNotch = {{
    AirwindowsExtractedParam{"gain", 0.5f, 0.0f, 1.0f, "Gain", ""},
    AirwindowsExtractedParam{"freq", 1.0f, 0.0f, 1.0f, "Freq", ""},
    AirwindowsExtractedParam{"nuke", 0.0f, 0.0f, 1.0f, "Nuke", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 5> kAirwindowsParams_XRegion = {{
    AirwindowsExtractedParam{"gain", 0.5f, 0.0f, 1.0f, "Gain", ""},
    AirwindowsExtractedParam{"first", 0.5f, 0.0f, 1.0f, "First", ""},
    AirwindowsExtractedParam{"last", 0.5f, 0.0f, 1.0f, "Last", ""},
    AirwindowsExtractedParam{"nuke", 0.0f, 0.0f, 1.0f, "Nuke", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 6> kAirwindowsParams_YBandpass = {{
    AirwindowsExtractedParam{"gain", 0.1f, 0.0f, 1.0f, "Gain", ""},
    AirwindowsExtractedParam{"freq", 0.5f, 0.0f, 1.0f, "Freq", ""},
    AirwindowsExtractedParam{"reson8", 0.1f, 0.0f, 1.0f, "Reson8", ""},
    AirwindowsExtractedParam{"res_edge", 0.1f, 0.0f, 1.0f, "ResEdge", ""},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 6> kAirwindowsParams_YHighpass = {{
    AirwindowsExtractedParam{"gain", 0.1f, 0.0f, 1.0f, "Gain", ""},
    AirwindowsExtractedParam{"freq", 0.5f, 0.0f, 1.0f, "Freq", ""},
    AirwindowsExtractedParam{"reson8", 0.1f, 0.0f, 1.0f, "Reson8", ""},
    AirwindowsExtractedParam{"res_edge", 0.1f, 0.0f, 1.0f, "ResEdge", ""},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 6> kAirwindowsParams_YLowpass = {{
    AirwindowsExtractedParam{"gain", 0.1f, 0.0f, 1.0f, "Gain", ""},
    AirwindowsExtractedParam{"freq", 0.5f, 0.0f, 1.0f, "Freq", ""},
    AirwindowsExtractedParam{"reson8", 0.1f, 0.0f, 1.0f, "Reson8", ""},
    AirwindowsExtractedParam{"res_edge", 0.1f, 0.0f, 1.0f, "ResEdge", ""},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 6> kAirwindowsParams_YNotBandpass = {{
    AirwindowsExtractedParam{"gain", 0.1f, 0.0f, 1.0f, "Gain", ""},
    AirwindowsExtractedParam{"freq", 0.5f, 0.0f, 1.0f, "Freq", ""},
    AirwindowsExtractedParam{"reson8", 0.1f, 0.0f, 1.0f, "Reson8", ""},
    AirwindowsExtractedParam{"res_edge", 0.1f, 0.0f, 1.0f, "ResEdge", ""},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 6> kAirwindowsParams_YNotch = {{
    AirwindowsExtractedParam{"gain", 0.1f, 0.0f, 1.0f, "Gain", ""},
    AirwindowsExtractedParam{"freq", 0.5f, 0.0f, 1.0f, "Freq", ""},
    AirwindowsExtractedParam{"reson8", 0.1f, 0.0f, 1.0f, "Reson8", ""},
    AirwindowsExtractedParam{"res_edge", 0.1f, 0.0f, 1.0f, "ResEdge", ""},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 6> kAirwindowsParams_YNotHighpass = {{
    AirwindowsExtractedParam{"gain", 0.1f, 0.0f, 1.0f, "Gain", ""},
    AirwindowsExtractedParam{"freq", 0.5f, 0.0f, 1.0f, "Freq", ""},
    AirwindowsExtractedParam{"reson8", 0.1f, 0.0f, 1.0f, "Reson8", ""},
    AirwindowsExtractedParam{"res_edge", 0.1f, 0.0f, 1.0f, "ResEdge", ""},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 6> kAirwindowsParams_YNotLowpass = {{
    AirwindowsExtractedParam{"gain", 0.1f, 0.0f, 1.0f, "Gain", ""},
    AirwindowsExtractedParam{"freq", 0.5f, 0.0f, 1.0f, "Freq", ""},
    AirwindowsExtractedParam{"reson8", 0.1f, 0.0f, 1.0f, "Reson8", ""},
    AirwindowsExtractedParam{"res_edge", 0.1f, 0.0f, 1.0f, "ResEdge", ""},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 6> kAirwindowsParams_YNotNotch = {{
    AirwindowsExtractedParam{"gain", 0.1f, 0.0f, 1.0f, "Gain", ""},
    AirwindowsExtractedParam{"freq", 0.5f, 0.0f, 1.0f, "Freq", ""},
    AirwindowsExtractedParam{"reson8", 0.1f, 0.0f, 1.0f, "Reson8", ""},
    AirwindowsExtractedParam{"res_edge", 0.1f, 0.0f, 1.0f, "ResEdge", ""},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 6> kAirwindowsParams_ZAcidLowpass = {{
    AirwindowsExtractedParam{"cutoff", 0.5f, 0.0f, 1.0f, "Cutoff", ""},
    AirwindowsExtractedParam{"over", 0.0f, 0.0f, 1.0f, "Over", ""},
    AirwindowsExtractedParam{"under", 1.0f, 0.0f, 1.0f, "Under", ""},
    AirwindowsExtractedParam{"meltdwn", 0.0f, 0.0f, 1.0f, "Meltdwn", ""},
    AirwindowsExtractedParam{"drive", 0.1f, 0.0f, 1.0f, "Drive", ""},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_ZBandpass = {{
    AirwindowsExtractedParam{"input", 0.1f, 0.0f, 1.0f, "Input", ""},
    AirwindowsExtractedParam{"freq", 0.5f, 0.0f, 1.0f, "Freq", ""},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
    AirwindowsExtractedParam{"poles", 0.5f, 0.0f, 1.0f, "Poles", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_ZBandpass2 = {{
    AirwindowsExtractedParam{"input", 0.1f, 0.0f, 1.0f, "Input", ""},
    AirwindowsExtractedParam{"freq", 0.5f, 0.0f, 1.0f, "Freq", ""},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
    AirwindowsExtractedParam{"poles", 0.5f, 0.0f, 1.0f, "Poles", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_ZHighpass = {{
    AirwindowsExtractedParam{"input", 0.1f, 0.0f, 1.0f, "Input", ""},
    AirwindowsExtractedParam{"freq", 0.5f, 0.0f, 1.0f, "Freq", ""},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
    AirwindowsExtractedParam{"poles", 0.5f, 0.0f, 1.0f, "Poles", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_ZHighpass2 = {{
    AirwindowsExtractedParam{"input", 0.1f, 0.0f, 1.0f, "Input", ""},
    AirwindowsExtractedParam{"freq", 0.5f, 0.0f, 1.0f, "Freq", ""},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
    AirwindowsExtractedParam{"poles", 0.5f, 0.0f, 1.0f, "Poles", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_ZLowpass = {{
    AirwindowsExtractedParam{"input", 0.1f, 0.0f, 1.0f, "Input", ""},
    AirwindowsExtractedParam{"freq", 0.5f, 0.0f, 1.0f, "Freq", ""},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
    AirwindowsExtractedParam{"poles", 0.5f, 0.0f, 1.0f, "Poles", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_ZLowpass2 = {{
    AirwindowsExtractedParam{"input", 0.1f, 0.0f, 1.0f, "Input", ""},
    AirwindowsExtractedParam{"freq", 0.5f, 0.0f, 1.0f, "Freq", ""},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
    AirwindowsExtractedParam{"poles", 0.5f, 0.0f, 1.0f, "Poles", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_ZNotch = {{
    AirwindowsExtractedParam{"input", 0.1f, 0.0f, 1.0f, "Input", ""},
    AirwindowsExtractedParam{"freq", 0.5f, 0.0f, 1.0f, "Freq", ""},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
    AirwindowsExtractedParam{"poles", 0.5f, 0.0f, 1.0f, "Poles", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 4> kAirwindowsParams_ZNotch2 = {{
    AirwindowsExtractedParam{"input", 0.1f, 0.0f, 1.0f, "Input", ""},
    AirwindowsExtractedParam{"freq", 0.5f, 0.0f, 1.0f, "Freq", ""},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
    AirwindowsExtractedParam{"poles", 0.5f, 0.0f, 1.0f, "Poles", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 1> kAirwindowsParams_Zoom = {{
    AirwindowsExtractedParam{"zoom", 0.5f, 0.0f, 1.0f, "Zoom", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 2> kAirwindowsParams_ZOutputStage = {{
    AirwindowsExtractedParam{"input", 0.1f, 0.0f, 1.0f, "Input", ""},
    AirwindowsExtractedParam{"output", 1.0f, 0.0f, 1.0f, "Output", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 5> kAirwindowsParams_ZRegion = {{
    AirwindowsExtractedParam{"input", 0.1f, 0.0f, 1.0f, "Input", ""},
    AirwindowsExtractedParam{"first", 0.5f, 0.0f, 1.0f, "First", ""},
    AirwindowsExtractedParam{"last", 0.5f, 0.0f, 1.0f, "Last", ""},
    AirwindowsExtractedParam{"poles", 0.5f, 0.0f, 1.0f, "Poles", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

inline constexpr std::array<AirwindowsExtractedParam, 5> kAirwindowsParams_ZRegion2 = {{
    AirwindowsExtractedParam{"input", 0.1f, 0.0f, 1.0f, "Input", ""},
    AirwindowsExtractedParam{"first", 0.5f, 0.0f, 1.0f, "First", ""},
    AirwindowsExtractedParam{"last", 0.5f, 0.0f, 1.0f, "Last", ""},
    AirwindowsExtractedParam{"poles", 0.5f, 0.0f, 1.0f, "Poles", ""},
    AirwindowsExtractedParam{"dry__wet", 1.0f, 0.0f, 1.0f, "Dry/Wet", ""},
}};

END_NAMESPACE_DISTRHO

#endif // AIRWINDOWS_EXTRACTED_PARAMS_HPP
