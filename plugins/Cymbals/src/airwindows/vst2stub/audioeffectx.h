#pragma once

#include <cstring>
#include <cstdint>
#include <cstdio>
#include <set>
#include <string>

// VST2 type definitions minimal stub for WinVST plugin compilation

typedef void* audioMasterCallback;
typedef int32_t VstInt32;
typedef uint32_t VstUInt32;

enum VstPlugCategory {
    kPlugCategUnknown = 0,
    kPlugCategEffect,
    kPlugCategSynth,
    kPlugCategAnalysis,
    kPlugCategMastering,
    kPlugCategSpacializer,
    kPlugCategRoomFx,
    kPlugSurroundFx,
    kPlugCategRestoration,
    kPlugCategOfflineProcess,
    kPlugCategShell,
    kPlugCategGenerator
};

const inline int kVstMaxProgNameLen   = 24;
const inline int kVstMaxNameLen       = 24;
const inline int kVstMaxProductStrLen = 64;
const inline int kVstMaxVendorStrLen  = 64;
const inline int kVstMaxParamStrLen   = 12;

inline void vst_strncpy(char* dst, const char* src, int maxLen) {
    strncpy(dst, src, maxLen);
    dst[maxLen] = '\0';
}

inline void float2string(float value, char* text, int maxLen) {
    snprintf(text, maxLen, "%.3f", value);
}

inline void int2string(int value, char* text, int maxLen) {
    snprintf(text, maxLen, "%d", value);
}

inline void dB2string(float value, char* text, int maxLen) {
    snprintf(text, maxLen, "%.1f dB", value);
}

class AudioEffect {
public:
    AudioEffect(audioMasterCallback, int, int) {}
    virtual ~AudioEffect() {}
};

class AudioEffectX : public AudioEffect {
public:
    AudioEffectX(audioMasterCallback cb, int numPrograms, int numParams)
        : AudioEffect(cb, numPrograms, numParams), numParams(numParams), m_sampleRate(44100.0f) {
        memset(_programName, 0, sizeof(_programName));
    }
    virtual ~AudioEffectX() {}

    // Sample rate
    float getSampleRate() const { return m_sampleRate; }
    virtual void setSampleRate(float sr) { m_sampleRate = sr; }

    // Setup stubs (no-ops in our context)
    void setNumInputs(int) {}
    void setNumOutputs(int) {}
    void setUniqueID(unsigned long) {}
    void canProcessReplacing() {}
    void canDoubleReplacing() {}
    void programsAreChunks(bool) {}

    // Processing — subclasses override these
    virtual void processReplacing(float**, float**, VstInt32) {}
    virtual void processDoubleReplacing(double**, double**, VstInt32) {}

    // Parameters — subclasses override
    virtual float getParameter(VstInt32) { return 0.0f; }
    virtual void setParameter(VstInt32, float) {}
    // Display helpers: every vendored algorithm defines these non-virtually,
    // so a virtual base slot lets the UI ask through AudioEffectX* without
    // knowing the concrete class.
    virtual void getParameterName(VstInt32, char*) {}
    virtual void getParameterDisplay(VstInt32, char*) {}
    virtual void getParameterLabel(VstInt32, char*) {}

    // Info — subclasses override
    virtual bool getEffectName(char*) { return false; }
    virtual bool getProductString(char*) { return false; }
    virtual bool getVendorString(char*) { return false; }
    virtual VstInt32 getVendorVersion() { return 0; }
    virtual VstPlugCategory getPlugCategory() { return kPlugCategEffect; }
    virtual void getProgramName(char*) {}
    virtual void setProgramName(char*) {}
    virtual VstInt32 getChunk(void**, bool) { return 0; }
    virtual VstInt32 setChunk(void*, VstInt32, bool) { return 0; }

    // Chunk helpers used by some plugins
    virtual bool copyProgram(VstInt32) { return false; }

public:
    int numParams;  // Number of parameters this plugin supports

protected:
    float m_sampleRate;
    char _programName[kVstMaxProgNameLen + 2];
    std::set<std::string> _canDo;
};

// Factory function type alias — each plugin defines this, we don't call it
typedef AudioEffect* (*PluginEntryProc)(audioMasterCallback audioMaster);
