#ifndef PLUGIN_CYMBALS_HPP
#define PLUGIN_CYMBALS_HPP

#include "DistrhoPlugin.hpp"
#include "CombFilterBank.hpp"
#include "DispersionStage.hpp"
#include "StickSoundBank.hpp"
#include "AdsrEnvelope.hpp"
#include "airwindows/AirSuiteUniversal.hpp"
#include "surge/SurgeFx.hpp"
#include "FFTAnalyzer.hpp"
#include <atomic>
#include <memory>
#include <mutex>
#include <thread>
#include <string>
#include <vector>
#include "cstring"
#include "MsegEnvelope.hpp"

#include "DSPUtils.hpp"
#include "UnifiedFilter.hpp"
#include "fx/InternalFxSlot.hpp"
#include "fx/VendoredFxSlot.hpp"
#include "ModulationEngine.hpp"

START_NAMESPACE_DISTRHO

using namespace DSPUtils;

class PluginCymbals : public Plugin {
public:
    struct PresetFileInfo;

    PluginCymbals();
    ~PluginCymbals();

    float getParameterValue(uint32_t index) const override;
    float getAnalyzerBand(int i) const { return fAnalyzerBandPeaks[i].load(std::memory_order_relaxed); }
    float getInharmBloomLevel() const { return fInharmBloomLevel.load(std::memory_order_relaxed); }
    float getInharm2BloomLevel() const { return fInharm2BloomLevel.load(std::memory_order_relaxed); }
    // Type actually installed on the audio thread. The editor must build the
    // slot panel from this, not the staged parameter, or it shows the new FX a
    // block before the prepared object has been swapped in.
    int getAppliedSlotType(int slot) const {
        if (slot < 0 || slot >= 6) return -1;
        const int applied = fAppliedSlotTypes[slot].load(std::memory_order_acquire);
        return applied >= 0 ? applied
                            : (int)getParameterValue((uint32_t)(kParamSlot1Type + slot * 16));
    }
    // UI playhead for the stick-pitch MSEG: 0..1 while a note is sounding,
    // -1 when idle (playhead hidden).
    float getStickMsegPhase() const {
        return stickBank.isMsegPitchActive() ? stickBank.getMsegPitchPhase() : -1.0f;
    }
    // ===== Modulation System UI accessors =====
    float getModSourceOutput(int s) const {
        return (s >= 0 && s < ModulationEngine::kNumSources)
            ? fModSourceOutput[s].load(std::memory_order_relaxed) : 0.0f;
    }
    float getModSourcePhase(int s) const {
        return (s >= 0 && s < ModulationEngine::kNumSources)
            ? fModSourcePhase[s].load(std::memory_order_relaxed) : 0.0f;
    }
    // Audio-liveness + host tempo for the UI idle-preview gate.
    uint32_t getModUiBlockCount() const { return fModUiBlockCount.load(std::memory_order_relaxed); }
    float getModTempoBpm() const { return fModUiTempoBpm.load(std::memory_order_relaxed); }
    // Runs initParameter into a temp Parameter and returns the display name.
    std::string parameterName(uint32_t index) const;
    // True iff the param is a routable continuous target (inside the apply table).
    bool parameterIsContinuous(uint32_t index) const;
    // Round 2 Piece 1: returns the canonical default value (normalized 0..1)
    // for the given param, derived by running initParameter into a temp
    // Parameter and reading ranges.def. Mirrors the ctor's seed loop at
    // PluginCymbals.cpp:681-684 so the value matches the host-published
    // default exactly. Returns 0.0f for out-of-range indices.
    float getParameterDefault(uint32_t index) const;
    std::string getPresetDirectory() const;
    std::vector<PresetFileInfo> listPresetFiles() const;
    std::string resolveCanonicalPresetSavePathForUi(const std::string& requestedPath) const;
    bool savePresetFromUiIntent(const std::string& requestedPath, std::string& outError);
    // Synchronous UI-side load: the CLAP/VST UI lives in the same process as
    // the plugin, so the preset browser's arrows/list can load directly instead
    // of round-tripping through host state (which silently did nothing when the
    // host did not deliver the key).
    bool loadPresetFromUiIntent(const std::string& path, std::string& outError);
    bool savePresetToPath(const std::string& path, std::string& outError);
    bool loadPresetFromPath(const std::string& path, std::string& outError);
    bool exportImpulseResponseToPath(const std::string& path, float durationSeconds, std::string& outError);
    bool exportImpulseResponse(float durationSeconds, std::string& outPath, std::string& outError);
    // Normalised log-magnitude spectrum of the preset's own IR for the PRESET
    // INFO tonal profile (peak 0 dB, -14..0 dB). Real DSP output, not metadata.
    bool renderImpulseResponseProfile(float durationSeconds, uint32_t points,
                                      std::vector<float>& outDb, std::string& outError) const;
    bool renderImpulseResponseProfileFromValues(const std::vector<float>& values,
                                                float durationSeconds, uint32_t points,
                                                std::vector<float>& outDb, std::string& outError) const;
    // Read a preset FILE and render its IR spectrum without touching live state.
    bool renderPresetFileProfile(const std::string& path, uint32_t points,
                                 std::vector<float>& outDb, std::string& outError);
    const std::string& getLastPresetError() const { return fLastPresetError; }
    const std::string& getLastPresetPath() const { return fLastPresetPath; }
    // ---- (#16/#17) Control-surface bindings: CC -> macro, CC -> parameter.
    // Exclusive by construction: binding a CC to a macro clears its parameter
    // binding and vice versa. Reserved controller numbers (mod wheel, pedals,
    // data entry, RPN/NRPN selectors, panic) can never be captured.
    void setCcMacroBind(int cc, int macro);          // macro 1..4; <=0 clears
    void clearCcMacroBind(int cc);
    void setCcParamBind(int cc, uint32_t paramIndex);
    void clearCcParamBind(int cc);
    int getCcMacroBind(int cc) const { return (cc >= 0 && cc < 128) ? (int)ccMacroBind[cc] : 0; }
    int getCcParamBind(int cc) const { return (cc >= 0 && cc < 128) ? (int)ccToParam[cc] : -1; }
    int getCcParamBindForParam(uint32_t paramIndex) const;   // reverse lookup: bound cc or -1
    const uint8_t* getCcMacroBindTable() const { return ccMacroBind; }   // iteration-friendly views
    const int16_t* getCcParamBindTable() const { return ccToParam; }
    // ---- (#17) CC-learn arming: kind 0 = parameter index, kind 1 = macro
    // 1..4. Invalid targets are rejected here; reserved-CC filtering happens
    // at capture time (the setters reject them).
    void armCcLearn(int kind, int index);
    void cancelCcLearn() { fLearnKind.store(-1, std::memory_order_release); }
    bool ccLearnArmed() const { return fLearnKind.load(std::memory_order_acquire) >= 0; }
    int lastLearnedCc() const { return fLastLearnCc.load(std::memory_order_acquire); }
    // Round 3 Piece 2 (MPE Y-axis / CC74 per-voice slide). Out-of-line
    // declaration so the per-voice slide write path is a real, greppable
    // mangled symbol in the shipped CLAP/VST3/JACK binaries (asserted by
    // tests/test_midi_wired.py). Under effective MPE, CC74 on a per-note
    // manager channel moves the pitch of the latest triggered voice on
    // that channel — folded into setPitchBendSemitones.
    void setVoiceSlideFromCc74(uint8_t channel, uint8_t value127);
    // Applies the clamped combination of per-note bend + slide to the voice's
    // stick pitch. Single source of truth so bend and slide cannot overwrite
    // each other (both update the same applied total, order-independent).
    void applyVoicePitch(int8_t slot);
    const std::string& getPresetName() const { return fPresetName; }
    const std::string& getPresetDescription() const { return fPresetDescription; }
    const std::string& getPresetAuthor() const { return fPresetAuthor; }
    const std::string& getPresetTagsCsv() const { return fPresetTagsCsv; }
    const std::string& getPresetUsage() const { return fPresetUsage; }
    // INIT: forget the loaded preset identity AND its save target, so a
    // subsequent SAVE cannot overwrite the preset the user was just on.
    void resetPresetIdentity() {
        fPresetName.clear();
        fLastPresetPath.clear();
        fPresetDescription.clear();
        fPresetAuthor.clear();
        fPresetTagsCsv.clear();
        fPresetUsage.clear();
    }
    void requestDspReset() { fPendingDspReset.store(true, std::memory_order_release); }
    // Public read of initParameter()'s default for the INIT button; the DPF
    // initParameter() itself is protected.
    float getParameterDefault(uint32_t index) {
        if (index >= (uint32_t)kParameterCount) return 0.0f;
        Parameter p;
        initParameter(index, p);
        return p.ranges.def;
    }
    void setPresetMetadata(const std::string& name, const std::string& description, const std::string& author, const std::string& tagsCsv, const std::string& usage = std::string()) {
        fPresetName = name;
        fPresetDescription = description;
        fPresetAuthor = author;
        fPresetTagsCsv = tagsCsv;
        fPresetUsage = usage;
    }

#ifdef HOST_BINARY
    // Test-only accessors for plugin-level verification in host binaries.
    void testSetParameterValue(uint32_t index, float value) { setParameterValue(index, value); }
    void testSetLastVelocity(float value) { lastVelocity = value; }
    float testGetVolume() const { return volume; }
    void testSampleRateChanged(double newSampleRate) {
        Plugin::sampleRateChanged(newSampleRate);
        sampleRateChanged(newSampleRate);
    }
    void testBufferSizeChanged(uint32_t newBufferSize) {
        Plugin::bufferSizeChanged(newBufferSize);
    }
    void testRun(const float** inputs, float** outputs, uint32_t frames,
                 const MidiEvent* midiEvents, uint32_t midiEventCount) {
        Plugin::bufferSizeChanged(frames);
        run(inputs, outputs, frames, midiEvents, midiEventCount);
    }
    void testUpdateStickEQ() { updateStickEQ(); }
    void testUpdateFinalEQ() { updateFinalEQ(); }
    void testProcessStickCutOnly(float inL, float inR, float& outL, float& outR) {
        processStickCutOnly(inL, inR, outL, outR);
    }
    void testProcessFinalCutOnly(float inL, float inR, float& outL, float& outR) {
        processFinalCutOnly(inL, inR, outL, outR);
    }
    void testResetCutFilterStates() {
        resetCutFilterStates();
    }
    float testGetCombGroupShift(int groupIdx) const { return combBank.testGetGroupShift(groupIdx); }
    float testGetVoiceBendSemis(int i) const {
        return (i >= 0 && i < kMaxVoices) ? m_voices[i].bendSemis : 0.0f;
    }
    float testGetVoiceSlideSemis(int i) const {
        return (i >= 0 && i < kMaxVoices) ? m_voices[i].slideSemis : 0.0f;
    }
    float testGetVoiceAppliedPitchSemis(int i) const {
        return (i >= 0 && i < kMaxVoices) ? m_voices[i].stick.getPitchBendSemitones() : 0.0f;
    }
    float testGetVoicePressure(int i) const {
        return (i >= 0 && i < kMaxVoices) ? m_voices[i].pressure : 0.0f;
    }
    int testGetVoiceChannel(int i) const {
        return (i >= 0 && i < kMaxVoices) ? (int)m_voices[i].channel : -1;
    }
    int testGetVoiceNote(int i) const {
        return (i >= 0 && i < kMaxVoices) ? (int)m_voices[i].note : -1;
    }
    bool testGetVoiceActive(int i) const {
        return (i >= 0 && i < kMaxVoices) && m_voices[i].active;
    }
    float testGetPerformanceInput(int kind) const { return m_modEngine.getPerformanceInput(kind); }
    float testGetMacro(int i) const { return m_modEngine.getMacro(i); }
    bool testGetRpnMpeActive() const { return rpnMpeActive; }
    void testSetState(const char* key, const char* value) { setState(key, value); }
    String testGetState(const char* key) const { return getState(key); }
    void testLoadProgram(uint32_t index) { loadProgram(index); }
    void testInitProgramName(uint32_t index, String& name) { initProgramName(index, name); }
#endif


protected:
    const char* getLabel() const override;
    const char* getMaker() const override;
    const char* getLicense() const override;
    uint32_t getVersion() const override;
    int64_t getUniqueId() const override;
    void initAudioPort(bool input, uint32_t index, AudioPort& port) override;
    void initParameter(uint32_t index, Parameter& parameter) override;
    void setParameterValue(uint32_t index, float value) override;
    void initProgramName(uint32_t index, String& name) override;
    void loadProgram(uint32_t index) override;
    void initState(uint32_t index, State& state) override;
    void setState(const char* key, const char* value) override;
    String getState(const char* key) const override;
    void run(const float** inputs, float** outputs, uint32_t frames,
             const MidiEvent* midiEvents, uint32_t midiEventCount) override;
    void sampleRateChanged(double newSampleRate) override;
              
public:
    struct PresetFileInfo {
        std::string path;
        std::string name;
        std::string description;
        std::string author;
        std::vector<std::string> tags;
        std::string folder;  // POSIX-normalized relative folder path, "" = preset-dir root
        std::string usage;
    };

    /**
     * Copy of the most recently staged preset's values. The UI prefers this
     * over getParameterValue() right after a load, before the DSP thread has
     * caught up; both agree once run() has delivered it (the staged copy is
     * retained, not cleared, so it never reports stale data for an applied
     * preset).
     */
    bool getPendingPresetValues(std::vector<float>& out) const {
        std::lock_guard<std::mutex> guard(fPendingPresetMutex);
        if (fPendingPresetValues.size() != kParameterCount) return false;
        out = fPendingPresetValues;
        return true;
    }

    // ====== Parameter Enumeration (Sequential, Contiguous) ======
    // CRITICAL: All parameters must be defined sequentially with no gaps.
    // Array-based parameters use algebraic formulas to define bounds (e.g., kParamComb40HPF = kParamComb1Freq + (40*4)-1).
    // Access patterns use subtraction arithmetic (index - kParamBase) to calculate array offsets.
    // See PluginParameters.cpp for bounds guards and range checks in initParameter/getParameterValue/setParameterValue.
    enum Parameters {
        // ===== Stage 1: Stick & EQ (Impulse Generation) =====
        kParamTone = 0, kParamDecay, kParamVolume,
        // Dispersion Stage 1 controls
        kParamDispFB, kParamDispDelay, kParamDispGain,
        kParamDispFBOverall,
        // Stick synthesis
        kParamStickType, kParamStickDirect, kParamStickBus,
        kParamStickAmpAttack, kParamStickAmpDecay, kParamStickAmpSustain, kParamStickAmpRelease,
        kParamStickPitchAttack, kParamStickPitchDecay, kParamStickPitchSustain, kParamStickPitchRelease,
        kParamStickPitchAmount, kParamStickPitchTimer,
        // Phase nudge controls (20 nudges × 2 params each = 40 total)
        kParamNudge1, kParamNudgeAmount1,
        kParamNudgeAmount20 = kParamNudge1 + (20 * 2) - 1,
        // Vibrato/LFO
        kParamVibRate, kParamVibDepth, kParamVibShape, kParamVibSmooth, kParamVibStepMix,
        // Chaos/random modulation
        kParamChaos, kParamChaosEnvAmount, kParamChaosEnvDecay,
        // Phase distortion
        kParamPDDrive, kParamPDNormalize, kParamPDTone, kParamPDBias, kParamPDSpread, kParamPDMix,
        // Unified EQ Nodes (5 nodes × 4 params each = 20 total)
        // Node 0: High Pass (default), Nodes 1-3: Bell (default), Node 4: Low Pass (default)
        kParamEQ1Freq, kParamEQ1Gain, kParamEQ1Q, kParamEQ1Type,
        kParamEQ2Freq, kParamEQ2Gain, kParamEQ2Q, kParamEQ2Type,
        kParamEQ3Freq, kParamEQ3Gain, kParamEQ3Q, kParamEQ3Type,
        kParamEQ4Freq, kParamEQ4Gain, kParamEQ4Q, kParamEQ4Type,
        kParamEQ5Freq, kParamEQ5Gain, kParamEQ5Q, kParamEQ5Type,
        kParamEQMix,
        // ===== Comb Bank (Stage 3: Resonance Topology) =====
        // Main comb bank: 40 combs × 4 params each = 160 params total
        // Array: combParams[160], indexed as (index - kParamComb1Freq)
        kParamComb1Freq, kParamComb1FB, kParamComb1LPF, kParamComb1HPF,
        kParamComb40HPF = kParamComb1Freq + (40 * 4) - 1,
        // Comb scale per stage: 40 params
        // Array: combScalePerStage[40], indexed as (index - kParamComb1Scale)
        kParamComb1Scale,
        kParamComb40Scale = kParamComb1Scale + 40 - 1,
        // Comb saturation type: 40 params
        kParamComb1SatType,
        kParamComb40SatType = kParamComb1SatType + 40 - 1,
        // Comb saturation amount: 40 params
        kParamComb1SatAmount,
        kParamComb40SatAmount = kParamComb1SatAmount + 40 - 1,
        // Comb stress onset: 40 params
        kParamComb1StressOnset,
        kParamComb40StressOnset = kParamComb1StressOnset + 40 - 1,
        // Comb stress knee: 40 params
        kParamComb1StressKnee,
        kParamComb40StressKnee = kParamComb1StressKnee + 40 - 1,
        // Comb mix: 40 params
        kParamComb1Mix,
        kParamComb40Mix = kParamComb1Mix + 40 - 1,
        // Comb solo: 40 params
        kParamComb1Solo,
        kParamComb40Solo = kParamComb1Solo + 40 - 1,
        // Comb enable: 40 params
        kParamComb1Enable,
        kParamComb40Enable = kParamComb1Enable + 40 - 1,
        // Comb group controls: 10 groups × params
        kParamCombGroup1Shift,
        kParamCombGroup10Shift = kParamCombGroup1Shift + 10 - 1,
        kParamCombGroup1Gain,
        kParamCombGroup10Gain = kParamCombGroup1Gain + 10 - 1,
        kParamCombBank1Series,
        kParamCombBank10Series = kParamCombBank1Series + 10 - 1,
        kParamCombScale,
        // ===== Final Comb Bank (Post-Reverb Resonance) =====
        // Final comb bank: 4 combs × 4 params each = 16 params
        // Array: finalCombParams[16], indexed as (index - kParamFinalComb1Freq)
        kParamFinalComb1Freq, kParamFinalComb1FB, kParamFinalComb1LPF, kParamFinalComb1HPF,
        kParamFinalComb2Freq, kParamFinalComb2FB, kParamFinalComb2LPF, kParamFinalComb2HPF,
        kParamFinalComb3Freq, kParamFinalComb3FB, kParamFinalComb3LPF, kParamFinalComb3HPF,
        kParamFinalComb4Freq, kParamFinalComb4FB, kParamFinalComb4LPF, kParamFinalComb4HPF,
        kParamFinalComb4SatType,
        kParamFinalComb4SatAmount,
        kParamFinalComb4StressOnset,
        kParamFinalComb4StressKnee,
        kParamFinalComb1Mix,
        kParamFinalComb4Mix = kParamFinalComb1Mix + 4 - 1,
        kParamFinalComb1Solo,
        kParamFinalComb4Solo = kParamFinalComb1Solo + 4 - 1,
        kParamFinalComb1Enable,
        kParamFinalComb4Enable = kParamFinalComb1Enable + 4 - 1,
        kParamFinalCombBankSeries, kParamFinalCombShift, kParamFinalCombScale,
        kParamFinalCombGroupGain, kParamFinalCombGroupCluster,
        kParamFinalEQ1Freq, kParamFinalEQ1Gain, kParamFinalEQ1Q, kParamFinalEQ1Type,
        kParamFinalEQ2Freq, kParamFinalEQ2Gain, kParamFinalEQ2Q, kParamFinalEQ2Type,
        kParamFinalEQ3Freq, kParamFinalEQ3Gain, kParamFinalEQ3Q, kParamFinalEQ3Type,
        kParamFinalEQ4Freq, kParamFinalEQ4Gain, kParamFinalEQ4Q, kParamFinalEQ4Type,
        kParamFinalEQ5Freq, kParamFinalEQ5Gain, kParamFinalEQ5Q, kParamFinalEQ5Type,
        kParamFinalEQMix,
        kParamFinalTiltAmount, kParamFinalTiltPivot,
        // ===== Stage 2 (Dispersion 2) - Secondary Processing Chain =====
        kParamDisp1Skip, kParamDisp2Skip, kParamDispCrossLink,
        kParamDisp2FB, kParamDisp2Delay, kParamDisp2Gain,
        kParamDisp2FBOverall,
        kParamVib2Rate, kParamVib2Depth, kParamVib2Shape, kParamVib2Smooth, kParamVib2StepMix,
        kParamChaos2, kParamChaos2EnvAmount, kParamChaos2EnvDecay,
        kParamPD2Drive, kParamPD2Normalize, kParamPD2Tone, kParamPD2Bias, kParamPD2Spread, kParamPD2Mix,
        // Phase nudge controls (20 nudges × 2 params = 40 total, Disp2 version)
        kParamNudge2_1, kParamNudgeAmount2_1,
        kParamNudgeAmount2_20 = kParamNudge2_1 + (20 * 2) - 1,
        // Analysis and tuning
        kParamAnalysisPoint,
        // Comb group clustering: 10 params
        kParamCombGroup1Cluster,
        kParamCombGroup10Cluster = kParamCombGroup1Cluster + 10 - 1,
        // Misc controls
        kParamDispSolo,
        kParamDispDelayMode,
        kParamDisp2DelayMode,
        kParamInharmBloomAmt,
        kParamInharmBloomTime,
        kParamInharmBloomDir,
        kParamResonatorDiffusion,
        kParamCombMemoryDamping,
        // Filter controls
        kParamTiltAmount,
        kParamTiltPivot,
        // Final AR/Clipper
        kParamFinalAROn,
        kParamFinalARAttack,
        kParamFinalARRelease,
        kParamFinalClipOn,
        kParamFinalClipType,
        kParamFinalClipAmount,
        // ===== Multi-segment Envelope (MSEG) for Stick Pitch =====
        kParamStickPitchMsegOn,
        kParamStickPitchMsegDuration,
        kParamStickPitchMsegPoint0Value,
        kParamStickPitchMsegPoint1Value,
        kParamStickPitchMsegPoint2Value,
        kParamStickPitchMsegPoint3Value,
        kParamStickPitchMsegPoint4Value,
        kParamStickPitchMsegPoint5Value,
        kParamStickPitchMsegPoint6Value,
        kParamStickPitchMsegPoint7Value,
        kParamStickPitchMsegPoint1Time,
        kParamStickPitchMsegPoint2Time,
        kParamStickPitchMsegPoint3Time,
        kParamStickPitchMsegPoint4Time,
        kParamStickPitchMsegPoint5Time,
        kParamStickPitchMsegPoint6Time,
        kParamStickPitchMsegPoint0Curve,
        kParamStickPitchMsegPoint1Curve,
        kParamStickPitchMsegPoint2Curve,
        kParamStickPitchMsegPoint3Curve,
        kParamStickPitchMsegPoint4Curve,
        kParamStickPitchMsegPoint5Curve,
        kParamStickPitchMsegPoint6Curve,
        // ===== Global Controls =====
        kParamCombBankOn,
        kParamFinalCombBankOn,
        // Processing Mode Control
        // 0.0 = High quality (safe) path: uses full-precision math, safe clipping, full state resolution
        // 1.0 = Fast path: may use approximations, simpler clipping, reduced state storage
        // NOTE: When changed at runtime, safe mode is enforced until explicitly toggled
        kParamProcessingMode,
        // ===== Modular FX Chain (6 slots × 16 params = 96 total) =====
        // Per slot (0-indexed): type=+0, params=+1..+13, wet=+14, bypass=+15
        // Formulas:
        //   slot n type   = kParamSlot1Type + n*16
        //   slot n param k = kParamSlot1Type + n*16 + k  (k=1..13)
        //   slot n wet    = kParamSlot1Type + n*16 + 14
        //   slot n bypass = kParamSlot1Type + n*16 + 15
        kParamSlot1Type,
        kParamSlot6Bypass = kParamSlot1Type + 5*16 + 15,  // = kParamSlot1Type + 95
        // ===== Slot Process Order (Phase 8: Drag Reorder) =====
        // 6 hidden parameters to track which DSP slot is processed at each visual position 0-5
        // These are populated by syncSlotOrderFromVisualOrder() during drag operations
        kParamSlotOrder0, kParamSlotOrder1, kParamSlotOrder2, kParamSlotOrder3, kParamSlotOrder4, kParamSlotOrder5,
        // ===== Advanced/Analog-Style EQ Filters (Phase 3: appended for legacy preset compatibility) =====
        // Type stays normalized 0.0-1.0 and snaps to compatibility buckets:
        // Off(0.0), 2-pole LPF(0.2), 4-pole LPF(0.4), 6-pole LPF(0.6),
        // 2-pole HPF(0.8), Wasp(0.9), 4-pole HPF(1.0).
        // Resonance remains normalized 0.0-1.0 and maps to Q 0.5-4.5 in later DSP stages.
        kParamEQ1AdvType, kParamEQ1AdvRes,
        kParamEQ2AdvType, kParamEQ2AdvRes,
        kParamEQ3AdvType, kParamEQ3AdvRes,
        kParamFinalEQ1AdvType, kParamFinalEQ1AdvRes,
        kParamFinalEQ2AdvType, kParamFinalEQ2AdvRes,
        kParamFinalEQ3AdvType, kParamFinalEQ3AdvRes,
        // Phase 2: Wasp v1 controls for advanced EQ paths (appended for preset index stability)
        kParamEQ1AdvDrive, kParamEQ1AdvCharacter,
        kParamEQ2AdvDrive, kParamEQ2AdvCharacter,
        kParamEQ3AdvDrive, kParamEQ3AdvCharacter,
        kParamFinalEQ1AdvDrive, kParamFinalEQ1AdvCharacter,
        kParamFinalEQ2AdvDrive, kParamFinalEQ2AdvCharacter,
        kParamFinalEQ3AdvDrive, kParamFinalEQ3AdvCharacter,
        // Addendum 1: DISP 2 Dynamic Inharm plumbing (appended for preset index stability)
        kParamInharm2BloomAmt,
        kParamInharm2BloomTime,
        kParamInharm2BloomDir,
        // UnifiedFilter per-band models (appended for preset index stability)
        // Model: 0 = CleanSVF (default), 0-33 = UnifiedFilter models except NoneModel (28)
        // FilterType: 0=LowPass, 1=BandPass, 2=HighPass, 3=Notch (UnifiedFilter only)
        // Slope: 0=12dB, 1=24dB, 2=48dB, 3=96dB per octave (UnifiedFilter only)
        kParamEQ1Model, kParamEQ1FilterType, kParamEQ1Slope,
        kParamEQ2Model, kParamEQ2FilterType, kParamEQ2Slope,
        kParamEQ3Model, kParamEQ3FilterType, kParamEQ3Slope,
        kParamEQ4Model, kParamEQ4FilterType, kParamEQ4Slope,
        kParamEQ5Model, kParamEQ5FilterType, kParamEQ5Slope,
        kParamFinalEQ1Model, kParamFinalEQ1FilterType, kParamFinalEQ1Slope,
        kParamFinalEQ2Model, kParamFinalEQ2FilterType, kParamFinalEQ2Slope,
        kParamFinalEQ3Model, kParamFinalEQ3FilterType, kParamFinalEQ3Slope,
        kParamFinalEQ4Model, kParamFinalEQ4FilterType, kParamFinalEQ4Slope,
        kParamFinalEQ5Model, kParamFinalEQ5FilterType, kParamFinalEQ5Slope,
        // Saturation per EQ band (appended for preset index stability)
        kParamEQ1Sat, kParamEQ2Sat, kParamEQ3Sat, kParamEQ4Sat, kParamEQ5Sat,
        kParamFinalEQ1Sat, kParamFinalEQ2Sat, kParamFinalEQ3Sat, kParamFinalEQ4Sat, kParamFinalEQ5Sat,
        // Comb group enable: 10 params (appended for preset index stability)
        kParamCombGroup1On,
        kParamCombGroup10On = kParamCombGroup1On + 10 - 1,
        // Comb filter type: 40 params (appended for preset index stability)
        kParamComb1FilterType,
        kParamComb40FilterType = kParamComb1FilterType + 40 - 1,
        // Final comb filter type: 4 params (appended for preset index stability)
        kParamFinalComb1FilterType,
        kParamFinalComb4FilterType = kParamFinalComb1FilterType + 4 - 1,
        // Comb mode rate (FLANGE/PHASER/TALKBOX LFO): 40 params (appended for preset index stability)
        kParamComb1ModeRate,
        kParamComb40ModeRate = kParamComb1ModeRate + 40 - 1,
        // Comb mode amount (ALLPASS dry-wet, FLANGE wet tap): 40 params (appended for preset index stability)
        kParamComb1ModeAmount,
        kParamComb40ModeAmount = kParamComb1ModeAmount + 40 - 1,
        // Final comb mode rate: 4 params (appended for preset index stability)
        kParamFinalComb1ModeRate,
        kParamFinalComb4ModeRate = kParamFinalComb1ModeRate + 4 - 1,
        // Final comb mode amount: 4 params (appended for preset index stability)
        kParamFinalComb1ModeAmount,
        kParamFinalComb4ModeAmount = kParamFinalComb1ModeAmount + 4 - 1,
        // Saike Partials model controls (FT_PARTIAL_* types only): 40 params each (appended for preset index stability)
        kParamComb1PartRelPos,
        kParamComb40PartRelPos = kParamComb1PartRelPos + 40 - 1,
        kParamComb1PartFreqDamp,
        kParamComb40PartFreqDamp = kParamComb1PartFreqDamp + 40 - 1,
        kParamComb1PartStiffness,
        kParamComb40PartStiffness = kParamComb1PartStiffness + 40 - 1,
        kParamComb1PartStiffExp,
        kParamComb40PartStiffExp = kParamComb1PartStiffExp + 40 - 1,
        kParamComb1PartCount,
        kParamComb40PartCount = kParamComb1PartCount + 40 - 1,
        // Final bank Saike Partials controls: 4 params each (appended for preset index stability)
        kParamFinalComb1PartRelPos,
        kParamFinalComb4PartRelPos = kParamFinalComb1PartRelPos + 4 - 1,
        kParamFinalComb1PartFreqDamp,
        kParamFinalComb4PartFreqDamp = kParamFinalComb1PartFreqDamp + 4 - 1,
        kParamFinalComb1PartStiffness,
        kParamFinalComb4PartStiffness = kParamFinalComb1PartStiffness + 4 - 1,
        kParamFinalComb1PartStiffExp,
        kParamFinalComb4PartStiffExp = kParamFinalComb1PartStiffExp + 4 - 1,
        kParamFinalComb1PartCount,
        kParamFinalComb4PartCount = kParamFinalComb1PartCount + 4 - 1,
        // ===== Modulation System =====
        kParamModOn,                       // global enable (0..1, default 1)
        // Sources: 11 × 66 params. Source s base = kParamModSrc1Type + s*66.
        // Sub-offsets (all stored normalized 0..1; ints marked):
        //  +0 type        int 0..9  (0=LFO 1=ENV 2=MSEG 3=STEP 4=VELOCITY 5=NOTE
        //                            6=MODWHEEL 7=PITCHBEND 8=AFTERTOUCH 9=RANDOM)
        //  +1 enabled               (0..1)
        //  +2 bipolar               (0..1)
        //  +3 retrig                (0..1, retrigger on note-on)
        //  +4 rate                  (0..1 → 0.01..5000 Hz log)
        //  +5 shape       int 0..5  (SINE TRI SAW SQUARE STEPPED RANDOM)
        //  +6 phase                 (LFO start phase 0..1)
        //  +7 smooth                (0..1)
        //  +8 sync                  (0..1, tempo-sync rate)
        //  +9 division    int 0..13 (1/32 1/16t 1/16 1/8t 1/8 1/4t 1/4 1/2 1/1 2/1 4/1 8/1 16/1 32/1)
        //  +10 a  +11 d  +12 s  +13 r        (ENV attack/decay/sustain/release)
        //  +14 dur                 (MSEG duration 0..1 → 0.01..10 s log)
        //  +15 loop                (MSEG loop 0..1)
        //  +16 sustain   int 0..8  (MSEG sustain point; LFO sustain-env reuses
        //                           it as the hold phase = v/8, 0 = none)
        //  +17..+24 v0..v7         (MSEG point values 0..1)
        //  +25..+30 t1..t6         (MSEG point times, normalized 0..1 of duration)
        //  +31..+37 c0..c6         (MSEG point curves -1..1 stored 0..1 → v*2-1)
        //  +38..+53 s0..s15        (STEP values 0..1)
        //  +54 stepCount  int 1..16
        //  +55 stepLoop            (0..1)
        //  +56 stepSmooth          (0..1)
        kParamModSrc1Type,
        kParamModSrc11StepSmooth = kParamModSrc1Type + 11*66 - 1,
        // Routes: 48 × 5. Route r base = kParamRoute1Source + r*5.
        //  +0 source      int 0..14 (0..10 = mod sources, 11..14 = macros 1..4)
        //  +1 target      int 0..4095 (DPF param index)
        //  +2 amount                 (0..1 → bipolar -1..1)
        //  +3 macro       int 0..4  (0=none, 1..4 = macro scaling the amount)
        //  +4 enabled                (0..1)
        kParamRoute1Source,
        kParamRoute48Enabled = kParamRoute1Source + 48*5 - 1,
        kParamMacro1, kParamMacro2, kParamMacro3, kParamMacro4,
        // ===== Voice architecture / MIDI foundation (contract v1: appended at
        // tail, indices 2282..2292 = old kParameterCount..+10; never reorder) =====
        kParamVoiceMode,        // 0=Mono (legacy bit-compatible behavior), 1=Poly
        kParamVoiceCount,       // 2..16 integer, default 8, active only in Poly
        kParamChokeMode,        // 0=Off, 1=AllNotesOneGroup (any new hit chokes all sounding voices), 2=TwoGroupsByNoteSplit
        kParamChokeSplitPoint,  // MIDI note 0..127 default 60: notes <split -> group A, >=split -> group B
        kParamRelVelSens,       // 0..1 default 0; scales note-off-initiated release time by release velocity (0 = legacy instant)
        kParamMpeMode,          // 0=Off, 1=On (also auto-enabled on RPN 6/7 MPE init while On persists)
        kParamMpeBendAmount,    // 0..1 default 0; scales per-note pitch bend -> per-voice pitch offset (stick freq + optional comb shift)
        kParamMpePressDrive,    // 0..1 default 0; scales per-note pressure -> comb stress/saturation drive of that voice's strike
        kParamBowEnable,        // 0/1 default 0
        kParamBowRate,          // 0..1 default 0.5, log-mapped 0.5..20 Hz sustained re-excitation rate
        kParamBowLevel,         // 0..1 default 0.5, re-strike amplitude scale
        // ===== Final output envelope: full ADSR (contract v1: appended at
        // tail, indices 2293..2294; never reorder) =====
        // The FINAL TWEAKS envelope was A/R only — decay and sustain were
        // hardcoded (0.001 s / 1.0) so the object could never hold a level.
        kParamFinalARDecay,     // 0..1 default 0.25, -> 0.001..2.0 s quadratic
        kParamFinalARSustain,   // 0..1 default 0.8, sustain level of the hold stage
        // ===== Per-comb-group peak limiter (appended at tail, indices
        // 2295..2324; never reorder). OFF BY DEFAULT: the default is 0 for
        // every group's On, and the DSP early-outs on it, so the stock patch
        // and every existing preset stay bit-transparent. Threshold 0..1 ->
        // -24..0 dB, Release 0..1 -> 5 ms..1 s. Applied to the group's summed
        // output inside CombFilterBank, after the per-group gain. =====
        kParamCombGroup1LimiterOn,
        kParamCombGroup10LimiterOn = kParamCombGroup1LimiterOn + 10 - 1,
        kParamCombGroup1LimiterThreshold,
        kParamCombGroup10LimiterThreshold = kParamCombGroup1LimiterThreshold + 10 - 1,
        kParamCombGroup1LimiterRelease,
        kParamCombGroup10LimiterRelease = kParamCombGroup1LimiterRelease + 10 - 1,
        // ===== Per-filter-type MODE A/B (appended at tail; never reorder) =====
        // One pair per filter stage (40 comb + 4 final). Their meaning depends
        // on the stage's FILTER TYPE and the default 0.5 reproduces the
        // previously fixed behaviour:
        //   Flange    A = delay centre 2..12 ms, B = sweep depth 0..10 ms
        //   Phaser    A = sweep depth (0..2 octaves), B = dry/wet
        //   GlitchPM  A = PM depth scale, B = modulator ratio (x tuned freq)
        //   Bitcrush  A = bit-depth offset, B = sample&hold rate scale
        //   Pop       A = clip/trigger scale, B = scramble cooldown scale
        //   Stutter   A = window-length scale, B = re-randomise rate scale
        //   Vowel/Vocal/Talkbox  A = formant spread, B = glottal/morph scale
        // All are ordinary real parameters, so every host can automate them and
        // the modulation matrix can route to them.
        kParamComb1ModeA,
        kParamComb40ModeA = kParamComb1ModeA + 40 - 1,
        kParamComb1ModeB,
        kParamComb40ModeB = kParamComb1ModeB + 40 - 1,
        kParamFinalComb1ModeA,
        kParamFinalComb4ModeA = kParamFinalComb1ModeA + 4 - 1,
        kParamFinalComb1ModeB,
        kParamFinalComb4ModeB = kParamFinalComb1ModeB + 4 - 1,
        // Per-post-filter-bank peak limiter (group 0 of the final bank, where
        // the final bank routes all four stages). Same mapping as the ten
        // main-bank group limiters; OFF by default for patch compatibility.
        kParamFinalCombLimiterOn,
        kParamFinalCombLimiterThreshold,
        kParamFinalCombLimiterRelease,
        // Final EQ bypass (COLOUR > EQ tab ON/OFF zone). Appended at the tail.
        kParamFinalEQOn,
        // ===== Master output ADSR (appended at tail; ON by default? no: OFF
        // for bit-transparent existing patches). The pre-existing ADSR now
        // envelopes only the FX tail (the wet delta of the modular chain).
        kParamMasterAROn,
        kParamMasterARAttack,
        kParamMasterARDecay,
        kParamMasterARSustain,
        kParamMasterARRelease,
        // ===== Master tuning for the whole comb/partial body =====
        // Appended at the tail: 0.5 = A440 reference, +-4 octaves (48 st).
        // Transposes the played stick/voice pitch only, never filter selectors.
        kParamMasterTuning,
        // ===== Final-touches tone shaping (post FX chain) =====
        kParamFinalTouchLPF,
        kParamFinalTouchHPF,
        kParamFinalTouchQ,
        kParamFinalClipMix,
        kParamFinalClipOutput,
        // DEPRECATED / RESERVED: these 15 indices were appended for the
        // FINAL TOUCHES filter experiment and are no longer shown or wired.
        // Kept at the tail so kParameterCount and all subsequent preset/state
        // indices stay stable (parameters are append-only).
        kParamFinalTouchHPSlope,
        kParamFinalTouchHPChar,
        kParamFinalTouchHPMix,
        kParamFinalTouchHPOut,
        kParamFinalTouchHPBypass,
        kParamFinalTouchHPSolo,
        kParamFinalTouchHPStereo,
        kParamFinalTouchLPQ,
        kParamFinalTouchLPSlope,
        kParamFinalTouchLPChar,
        kParamFinalTouchLPMix,
        kParamFinalTouchLPOut,
        kParamFinalTouchLPBypass,
        kParamFinalTouchLPSolo,
        kParamFinalTouchLPStereo,
        // ===== Per-comb-filter Q (FILTERS page), AUTO at 0 = feedback-derived Q =====
        kParamComb1Q,
        kParamComb40Q = kParamComb1Q + 39,
        // Final (COLOUR-page) four-filter bank Q; same AUTO convention.
        kParamFinalComb1Q,
        kParamFinalComb4Q = kParamFinalComb1Q + 3,
        // ===== Per-stick-type character controls (appended at tail; never
        // reorder). These are real automatable/mod-routable params; the
        // IMPULSES header shows only the one relevant to the selected SOUND.
        kParamStickPulseWidth,
        kParamStickNoiseColor,
        kParamStickMetallicSpread,
        kParamStickNoiseMetalBlend,
        kParamStickHatTone,
        kParamStickSquareWidth,
        kParamStickSawCurve,
        kParamStickTriangleSkew,
        kParamStickSineDrive,
        kParamStickPhase,
        kParamStickGrit,
        kParamStickUnison,   // 0..3 -> 1..4 unison oscillators
        kParamStickDetune,   // 0..1 -> 0..40 cents spread
        // ===== ADSR decay/release curve powers (appended at tail; never
        // reorder). 0.5 = linear (curve 0), <0.5 = slow start / fast finish,
        // >0.5 = fast start / slow finish. The four main user-facing ADSRs get
        // one pair each; the MOD ENV sources already expose A/D/R curves.
        kParamStickAmpDecayCurve,
        kParamStickAmpReleaseCurve,
        kParamStickPitchDecayCurve,
        kParamStickPitchReleaseCurve,
        kParamFinalARDecayCurve,
        kParamFinalARReleaseCurve,
        kParamMasterARDecayCurve,
        kParamMasterARReleaseCurve,
        // ===== TOTAL PARAMETER COUNT =====
        // This must equal the highest parameter index + 1
        // Any new parameters added must be inserted sequentially above this line
        kParameterCount
        };

        private:    
        float tone, decay, volume, dispFB, dispDelay, dispGain, dispFBOverall;
        // Master-volume gain cache: Volume can be route-modulated per sample, so
        // this is a memoisation (recompute on change), never a block hoist.
        float fCachedVolumeParam = -1.0f;
        float fCachedLinVol = 1.0f;
        float nudge[20], nudgeAmount[20];
        float vibRate, vibDepth, vibShape, vibSmooth, vibStepMix, chaosFactor, chaosEnvAmount, chaosEnvDecay;
        float pdDrive, pdNormalize, pdTone, pdBias, pdSpread, pdMix;
        float disp1Skip, disp2Skip, dispCrossLink, dispSolo, dispDelayMode, disp2DelayMode;
        float inharmBloomAmt, inharmBloomTime, inharmBloomDir;
        float inharm2BloomAmt, inharm2BloomTime, inharm2BloomDir;
        float resonatorDiffusion;
        float combMemoryDamping;
        float disp2FB, disp2Delay, disp2Gain, disp2FBOverall;
        float nudge2[20], nudgeAmount2[20];
        float vib2Rate, vib2Depth, vib2Shape, vib2Smooth, vib2StepMix, chaos2Factor, chaos2EnvAmount, chaos2EnvDecay;
        float msegPitchOn, msegPitchDuration;
        float msegPitchValues[8], msegPitchTimes[6], msegPitchCurves[7];
        // Final tweaks / clipper knobs
        float finalClipOn, finalClipType, finalClipAmount;
        float pd2Drive, pd2Normalize, pd2Tone, pd2Bias, pd2Spread, pd2Mix;
        float eqFreq[5], eqGain[5], eqQ[5], eqMix, stickTiltAmount, stickTiltPivot;
        float eqAdvType[3], eqAdvRes[3];  // Phase 3: Advanced/analog-style EQ filters per band
        float eqAdvDrive[3], eqAdvCharacter[3];
        float eqModel[5], eqFilterType[5], eqSlope[5], eqSat[5];  // UnifiedFilter per-node (Stick EQ)
        float feqModel[5], feqFilterType[5], feqSlope[5], feqSat[5];  // UnifiedFilter per-node (Final EQ)
        float feqFreq[5], feqGain[5], feqQ[5], feqMix, finalEQOn, finalTiltAmount, finalTiltPivot;
        float feqAdvType[3], feqAdvRes[3];  // Phase 3: Advanced/analog-style Final EQ filters per band
        float feqAdvDrive[3], feqAdvCharacter[3];
        float stickType, stickDirect, stickBus;
        float stickPulseWidth = 0.25f, stickNoiseColor = 0.0f, stickMetallicSpread = 0.5f;
        float stickNoiseMetalBlend = 0.5f, stickHatTone = 0.5f;
        float stickSquareWidth = 0.5f, stickSawCurve = 0.5f, stickTriangleSkew = 0.5f;
        float stickSineDrive = 0.0f, stickPhase = 0.5f, stickGrit = 0.0f;
        float stickUnison = 0.0f, stickDetune = 0.0f;
        float stickAmpAttack, stickAmpDecay, stickAmpSustain, stickAmpRelease;
        float stickAmpDecayCurve = 0.5f, stickAmpReleaseCurve = 0.5f;
        float stickPitchAttack, stickPitchDecay, stickPitchSustain, stickPitchRelease, stickPitchAmount, stickPitchTimer;
        float stickPitchDecayCurve = 0.5f, stickPitchReleaseCurve = 0.5f;
        float combParams[160], combScalePerStage[40], combSatType[40], combSatAmount[40], combStressOnset[40], combStressKnee[40], combMix[40], combSolo[40], combEnable[40], combFilterType[40], combModeRate[40], combModeAmount[40], combPartRelPos[40], combPartFreqDamp[40], combPartStiffness[40], combPartStiffExp[40], combPartCount[40], combGroupShift[10], combGroupGain[10], combBankSeries[10], combGroupCluster[10], combGroupOn[10], combScale;
        float finalCombParams[16], finalComb4SatType, finalComb4SatAmount, finalComb4StressOnset, finalComb4StressKnee, finalCombMix[4], finalCombSolo[4], finalCombEnable[4], finalCombFilterType[4], finalCombModeRate[4], finalCombModeAmount[4], finalCombPartRelPos[4], finalCombPartFreqDamp[4], finalCombPartStiffness[4], finalCombPartStiffExp[4], finalCombPartCount[4], finalCombBankSeries, finalCombShift, finalCombScale, finalCombGroupGain, finalCombGroupCluster;
        float finalAROn, finalARAttack, finalARDecay, finalARSustain, finalARRelease;
        float finalARDecayCurve = 0.5f, finalARReleaseCurve = 0.5f;
        float masterAROn, masterARAttack, masterARDecay, masterARSustain, masterARRelease;
        float masterARDecayCurve = 0.5f, masterARReleaseCurve = 0.5f;
        float masterTuning = 0.5f;      // normalized, 0.5 = no transposition
        // 2^((masterTuning-0.5)*8), +-4 octaves. applyMasterTuning() also
        // rescales sounding voices by the ratio delta (live automation).
        float masterTuneRatio = 1.0f;
        float finalTouchLPF = 1.0f;     // normalized 0..1 -> 20 Hz..20 kHz
        float finalTouchHPF = 0.0f;
        DSPUtils::BiquadCascade finalTouchLp, finalTouchHp;
        float finalClipMix = 1.0f;      // dry/wet around the clipper
        float finalClipOutput = 0.5f;   // normalized, 0.5 = unity (-12..+12 dB)
        float finalClipGain = 1.0f;
        float combBankOn;
        // Per-comb-group peak limiter controls (off by default; index g = group
        // g+1, same addressing as combGroupOn/combGroupGain).
        float combGroupLimiterOn[10], combGroupLimiterThreshold[10], combGroupLimiterRelease[10];
        float finalCombBankOn;
        float processingMode;
        float analysisPoint;

        // ===== Modular FX Chain (Phase 2) =====
        AirSuiteUniversal m_fxSlots[6];
        // Surge effects occupy 600..608 and project-owned filters 700..756. Both run
        // alongside the Airwindows slots in the same serial chain.
        std::unique_ptr<CymbalsSurgeFx::Instance> m_surgeFx[6];
        std::unique_ptr<CymbalsInternalFxSlot> m_internalFx[6];
        std::unique_ptr<VendoredFxSlot> m_vendoredFx[6];
        int m_surgeFxAlgo[6] = {-1, -1, -1, -1, -1, -1};

        // Prepared-object handoff for FX slot type changes. The audio thread
        // only swaps unique_ptrs; a non-RT worker constructs and destroys the
        // objects (a CombFilterBank-backed internal filter is far too large to
        // preallocate for every slot/type).
        struct FxPrepared {
            int type = -1;
            std::unique_ptr<CymbalsSurgeFx::Instance> surge;
            std::unique_ptr<CymbalsInternalFxSlot> internal;
            std::unique_ptr<VendoredFxSlot> vendored;
        };
        std::thread fFxPrepareThread;
        std::atomic<bool> fFxPrepareQuit{false};
        std::atomic<float> fFxPrepareSampleRate{48000.0f};
        std::atomic<int> fFxPrepareRequest[6] = {-1, -1, -1, -1, -1, -1};
        std::atomic<FxPrepared*> fFxPrepared[6] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
        std::atomic<FxPrepared*> fFxRetired[6] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
        void fxPrepareWorkerLoop();
        static void prepareFxObject(int type, float sampleRate, FxPrepared& out);

        float             m_slotParams[6][13];
        float             m_slotWet[6];
        bool              m_slotBypass[6];
        // ===== Slot Process Order (Phase 8: Drag Reorder) =====
        // Maps visual position to original DSP slot index. Initialized to {0,1,2,3,4,5}
        int               m_slotProcessOrder[6];
        
        CombFilterBank combBank, finalCombBank;
    DispersionStage dispersion, dispersion2;
    StickSoundBank stickBank;
    Biquad stickTiltLow, stickTiltHigh;
    Biquad finalTiltLow, finalTiltHigh;
    UnifiedFilter stickEQNodes[5], finalEQNodes[5];
    AdsrEnvelope inharmBloomEnv;
    AdsrEnvelope inharm2BloomEnv;
    AdsrEnvelope finalOutEnv;
    AdsrEnvelope masterOutEnv;
    float lastNoteFreq, lastVelocity;
    float fSampleRate = 44100.0f;

    // ===== Polyphony pool (foundation wave) =====
    // bypasses the pool entirely and uses the shared legacy members above.
    static constexpr int kMaxVoices = 16;
    struct Voice {
        bool active = false;
        bool releasing = false;      // fading out via choke fade (relGain ramp)
        uint8_t note = 0;
        uint8_t channel = 0;         // MIDI channel of the triggering note-on
        uint32_t age = 0;            // monotonic stamp; oldest-voice-first stealing
        float relGain = 1.0f;        // choke fade multiplier (1 -> 0 in ~5 ms)
        float relStep = 0.0f;        // per-sample choke fade decrement (0 for natural releases)
        float freq = 220.0f;         // this hit's base frequency (bow retrigger reuses it)
        float velocity = 1.0f;       // this hit's strike velocity (bow scales by bowLevel)
        float bendSemis = 0.0f;      // MPE per-note pitch bend, semitones (±24 full-scale)
        float slideSemis = 0.0f;     // MPE Y-axis (CC74) per-voice pitch slide, semitones (±24 full-scale); folded into setPitchBendSemitones
        float pressure = 0.0f;       // MPE per-note pressure 0..1 (poly AT / channel AT)
        uint32_t bowSamplesLeft = 0; // bow/swell countdown to next re-strike
        StickSoundBank stick{44100.0f};
        AdsrEnvelope bloomEnv{};     // per-voice phase of the inharm-bloom envelope pair
        AdsrEnvelope bloom2Env{};
    };
    // Registered contract params (MPE/Bow consumers wired in expression wave).
    float voiceMode = 0.0f, voiceCount = 8.0f, chokeMode = 0.0f, chokeSplitPoint = 60.0f;
    float relVelSens = 0.0f;
    float mpeMode = 0.0f, mpeBendAmount = 0.0f, mpePressDrive = 0.0f;
    float bowEnable = 0.0f, bowRate = 0.5f, bowLevel = 0.5f;

    // Pinned UI-lane accessors (contract v1).
public:
    bool getVoiceModeActive() const { return voiceMode > 0.5f; }
    bool getMpeModeEnabled() const { return (mpeMode > 0.5f) || rpnMpeActive; } // EFFECTIVE: manual switch OR RPN [0,6] activation
    // Number of SOUNDING pooled voices (stick envelope alive, not choked out).
    // Voices whose exciter went silent but whose bloom phases still run keep
    // being processed but no longer count here.
    int getActiveVoiceCount() const {
        int n = 0;
        for (int i = 0; i < kMaxVoices; ++i)
            n += (m_voices[i].active && m_voices[i].relGain > 0.0f && m_voices[i].stick.isActive()) ? 1 : 0;
        return n;
    }
private:
    FFTAnalyzer* fAnalyzer = nullptr;
    mutable std::atomic<float> fAnalyzerBandPeaks[FFTAnalyzer::kNumBands];
    void handleMidiNote(uint8_t note);
    void applyMasterTuning(float normalized);
    void applyVoiceBloomTime(bool second, float decaySec);
    void requestStickEQRebuild() noexcept { fPendingStickEQRebuild.store(true, std::memory_order_release); }
    void chokeSameGroupVoices(uint8_t newNote, const Voice* exclude);
    void triggerPolyVoice(uint8_t note, float velocity, uint8_t channel);
    void releasePolyVoice(uint8_t note, float relVelNorm, bool instant);
    void clearAllVoices();
    float releaseTimeMul(float relVelNorm) const;
    void processVoicePool(float& sL, float& sR, float& bloomRawSum, float& bloom2RawSum,
                          float& pressDriveSum, uint32_t bowInterval);
    bool mpeEffective() const { return (mpeMode > 0.5f) || rpnMpeActive; }
    void applyRpnDataEntry(uint16_t value14);
    void applyNrpnDataEntry(uint16_t value14);
    void applyMidiParameterWrite(uint32_t index, float value);
    void clearVoiceExpression();
    Voice m_voices[kMaxVoices];
    uint32_t m_voiceAgeCounter = 0;
    int8_t m_voiceChannelMap[16];        // MIDI channel -> LATEST triggered voice slot (-1 none); set in ctor
    // RPN [0,6] MPE activation state machine (CC101=0 null MSB, CC100=6 LSB, then CC6 [+CC38]).
    bool rpnMsbZeroSeen = false, rpnLsbSixSeen = false;
    bool rpnArmed = false, rpnGotMsb = false, rpnMpeActive = false;
    uint8_t rpnMsbVal = 0;
    static bool isReservedControlCc(int cc) {
        switch (cc) {
        case 1:    // mod wheel
        case 6:    // data entry MSB
        case 11:   // expression pedal
        case 38:   // data entry LSB
        case 64:   // sustain
        case 67:   // sostenuto/soft
        case 98:   // NRPN LSB select
        case 99:   // NRPN MSB select
        case 100:  // RPN LSB select
        case 101:  // RPN MSB select
        case 120:  // all sound off
        case 123:  // all notes off
            return true;
        default:
            return false;
        }
    }
    // (#18) NRPN selector state machine — fully separate selector tracking from
    // the RPN [0,6] MPE machine above; shares the data-entry dispatch with
    // NRPN-priority, and each machine's flags are touched only by its own
    // branches so neither can corrupt the other's armed state.
    bool nrpnMsbSeen = false, nrpnLsbSeen = false;
    bool nrpnArmed = false, nrpnGotMsb = false;
    uint8_t nrpnSelMsb = 0, nrpnSelLsb = 0, nrpnDataMsb = 0;
    // (#16/#17) CC routing tables, written off-RT by UI/state, read in run().
    uint8_t ccMacroBind[128] = {};   // 0 = none, else macro 1..4
    int16_t ccToParam[128] = {};     // -1 = none (filled in ctor)
    // (#17) CC-learn arm plumbing: UI arms a target (kind 0=param index,
    // 1=macro 1..4), the next non-reserved CC captured in run() performs the
    // binding and disarms. Atomics: armed flag written by UI thread, polled on
    // the audio thread.
    std::atomic<int> fLearnKind{-1};    // -1 = none, 0 = param, 1 = macro
    std::atomic<int> fLearnIndex{-1};
    std::atomic<int> fLastLearnCc{-1};  // last successfully captured CC (-1 none)
    // (#7/MED-4) LIVE comb saturator amounts captured when pressure drive
    // engages; scaled while driven and restored exactly on release/panic so
    // mod-routed amounts survive the shared-bus drive pass.
    float m_combSatLiveBase[40] = {};
    // (MED-3) Scratch storage for pool-bank MSEG pushes: reserved once so
    // syncVoiceStickParams never heap-allocates per parameter change.
    std::vector<MsegPoint> fMsegScratch;
    bool m_pressWasActive = false;       // comb sat-drive injection latch (restore pass)
    void syncVoiceStickParams();   // push stick/MSEG params into every pool bank (UI thread)
    void syncVoiceBloomEnvs();     // mirror global bloom envelope shapes onto pool envs
    void syncFinalOutEnv();      // re-shape finalOutEnv (A/D/S/R) from the normalized controls
    void syncMasterOutEnv();     // re-shape masterOutEnv (A/D/S/R)
    void updateFinalTouchFilters(); // final HP/LP coefficients from normalized controls

    void requestFinalEQRebuild() noexcept { fPendingFinalEQRebuild.store(true, std::memory_order_release); }
    void applyPendingEQRebuilds();
    void applyPendingSlotTypeChanges();
    void updateStickEQ();
    void updateStickPitchMseg();
    void updateFinalEQ();
    void processStickCutOnly(float inL, float inR, float& outL, float& outR);
    void processFinalCutOnly(float inL, float inR, float& outL, float& outR);
    void resetCutFilterStates();
    void normalizeSlotProcessOrder();
    struct DispersionFamilySnapshot {
        float feedback;
        float delay;
        float gain;
        float feedbackOverall;
        float nudge[20];
        float nudgeAmount[20];
        float vibRate;
        float vibDepth;
        float vibShape;
        float vibSmooth;
        float vibStepMix;
        float chaosFactor;
        float chaosEnvAmount;
        float chaosEnvDecay;
        float pdDrive;
        float pdNormalize;
        float pdTone;
        float pdBias;
        float pdSpread;
        float pdMix;
        float delayMode;
    };
    struct IrExportInputSnapshot {
        float sampleRate;
        float durationSeconds;
        float volumeParam; // normalized kParamVolume (modulated per-sample in IR render)
        float disp1Skip;
        float disp2Skip;
        float dispCrossLink;
        float dispSolo;
        float stickDirect;
        float stickBus;
        float feqMix;
        float finalEQOn;
        float combScale;
        float resonatorDiffusion;
        float combMemoryDamping;
        float combParams[160];
        float combScalePerStage[40];
        float combSatType[40];
        float combSatAmount[40];
        float combStressOnset[40];
        float combStressKnee[40];
        float combMix[40];
        float combSolo[40];
        float combEnable[40];
        float combFilterType[40];
        float combModeRate[40];
        float combModeAmount[40];
        float combModeA[40];
        float combModeB[40];
        float combPartRelPos[40];
        float combPartFreqDamp[40];
        float combPartStiffness[40];
        float combPartStiffExp[40];
        float combPartCount[40];
        float combGroupShiftHz[10];
        float combGroupGain[10];
        float combGroupOn[10];
        float combGroupLimiterOn[10];
        float combGroupLimiterThreshold[10];
        float combGroupLimiterRelease[10];
        float combBankSeries[10];
        float combGroupCluster[10];
        float finalCombScale;
        float finalCombParams[16];
        float finalComb4SatType;
        float finalComb4SatAmount;
        float finalComb4StressOnset;
        float finalComb4StressKnee;
        float finalCombMix[4];
        float finalCombSolo[4];
        float finalCombEnable[4];
        float finalCombFilterType[4];
        float finalCombModeRate[4];
        float finalCombModeAmount[4];
        float finalCombModeA[4];
        float finalCombModeB[4];
        float finalCombPartRelPos[4];
        float finalCombPartFreqDamp[4];
        float finalCombPartStiffness[4];
        float finalCombPartStiffExp[4];
        float finalCombPartCount[4];
        float finalCombBankSeries;
        float finalCombShiftHz;
        float finalCombGroupGain;
        float finalCombGroupCluster;
        float finalLimiterOn;
        float finalLimiterThreshold;
        float finalLimiterRelease;
        float combBankOn;
        float finalCombBankOn;
        DispersionFamilySnapshot dispersion[2];
        float eqFreq[5];
        float eqGain[5];
        float eqQ[5];
        float eqModel[5];
        float eqFilterType[5];
        float eqSlope[5];
        float eqSat[5];
        float eqMix;
        float feqFreq[5];
        float feqGain[5];
        float feqQ[5];
        float feqModel[5];
        float feqFilterType[5];
        float feqSlope[5];
        float feqSat[5];
        float finalTiltAmount;
        float finalTiltPivot;
        float stickTiltAmount;
        float stickTiltPivot;
        // ===== Modulation System =====
        float modConfig[ModulationEngine::kConfigSize];
        float modOn;
    };
    struct IrExportRenderState {
        CombFilterBank combBank;
        CombFilterBank finalCombBank;
        DispersionStage dispersion;
        DispersionStage dispersion2;
        UnifiedFilter stickEQNodes[5];
        UnifiedFilter finalEQNodes[5];
        Biquad stickTiltLow;
        Biquad stickTiltHigh;
        Biquad finalTiltLow;
        Biquad finalTiltHigh;
        ModulationEngine modEngine;

        // Per-sample modulated scalars for the four chain-present targets
        // (seeded from the snapshot; routes adjust them in applyModulatedDetached).
        float modVolume = 1.0f, modEqMix = 1.0f, modFeqMix = 1.0f, modDispCrossLink = 0.0f;

        explicit IrExportRenderState(float sampleRate);
    };
    std::vector<float> copyIrExportParameterValues() const;
    IrExportInputSnapshot createIrExportInputSnapshot(const std::vector<float>& parameterValues, float durationSeconds) const;
    std::unique_ptr<IrExportRenderState> createDetachedIrExportRenderState(const IrExportInputSnapshot& exportInput) const;
    void configureDetachedIrCombBank(CombFilterBank& bank, const IrExportInputSnapshot& exportInput) const;
    void configureDetachedIrFinalCombBank(CombFilterBank& bank, const IrExportInputSnapshot& exportInput) const;
    void configureDetachedIrDispersionStage(DispersionStage& stage, const DispersionFamilySnapshot& snapshot) const;
    void configureDetachedIrDispersionStages(IrExportRenderState& state, const IrExportInputSnapshot& exportInput) const;
    void configureDetachedIrFinalEq(IrExportRenderState& state, const IrExportInputSnapshot& exportInput) const;
    void configureDetachedIrStickEq(IrExportRenderState& state, const IrExportInputSnapshot& exportInput) const;
    void applyPresetValuesSafely(const std::vector<float>& values);
    std::vector<float> captureAllParameterValues() const;
    void applyAllParameterValues(const std::vector<float>& values);
    std::string serializePresetJsonFromValues(const std::vector<float>& values) const;
    std::string serializePresetJson() const;
    bool deserializePresetJson(const std::string& json, std::vector<float>& outValues, std::string& outError);
    mutable std::string fLastPresetPath;
    mutable std::string fLastPresetError;
    // Serializes every access to fLastPresetJson (host state save, UI save,
    // preset staging); run() never touches the string, it only applies floats.
    mutable std::mutex fLastPresetJsonMutex;
    mutable std::string fLastPresetJson;
    std::string fPresetName;
    std::string fPresetDescription;
    std::string fPresetAuthor;
    std::string fPresetTagsCsv;
    std::string fPresetUsage;
    float fReverbProtectEnv = 0.0f;
    float fReverbProtectSlowEnv = 0.0f;
    // Output limiter engaged by the reverb-protection trip. It ducks the
    // pre-clip signal instead of clearing the comb/dispersion state, so a
    // dense note cluster cannot reset the audible tail.
    float fReverbProtectGain = 1.0f;
    bool fActiveNotes[128] = {false};
    int fActiveNoteCount = 0;
    std::atomic<float> fInharmBloomLevel{0.0f};
    std::atomic<float> fInharm2BloomLevel{0.0f};
    std::atomic<bool> fPendingStickEQRebuild{false};
    std::atomic<bool> fPendingFinalEQRebuild{false};
    std::atomic<bool> fPendingSlotTypeChange{false};
    // Pending AirSuiteUniversal types per FX slot (-1 = none), applied on the
    // audio thread in run() so setType/reset never race process() reads.
    // ATOMIC, and deliberately NOT under fParameterSnapshotMutex. The drainer is
    // run() (the audio thread), and taking a mutex there is a real-time violation:
    // the UI stager holds the same mutex, so a UI stall would block the audio
    // thread inside the FX stage. The handoff is a single int per slot plus the
    // release/acquire flag, which is all the ordering it needs.
    std::atomic<int> fPendingSlotTypes[6] = { -1, -1, -1, -1, -1, -1 };
    std::atomic<int> fAppliedSlotTypes[6] = { -1, -1, -1, -1, -1, -1 };
    mutable std::mutex fPendingPresetMutex;
    // Discipline: setParameterValue runs on the RT audio thread and never
    // takes this mutex (plain float stores; torn reads tolerated by readers).
    // It only guards genuinely shared non-RT critical sections: the non-RT value
    // snapshots (state save, IR export). The fPendingSlotTypes staging pair is
    // NOT guarded by it - that handoff is lock-free atomics, because its drainer
    // runs on the audio thread.
    mutable std::recursive_mutex fParameterSnapshotMutex;
    // Preset handoff (non-RT stager -> audio-thread run()):
    //  - fPendingPresetValues holds the LATEST staged preset and is never
    //    cleared, so staging a newer preset while run() delivers an older one
    //    can never wipe it (generation check below picks it up next block).
    //  - run() memcpy's into the preallocated fPresetApplyScratch under the
    //    mutex (fixed-size copy, no allocation, no long lock) and applies the
    //    per-parameter values outside the lock.
    //  - fPresetStagedGen / fPresetAppliedGen (both mutex-guarded) detect a
    //    preset staged while an earlier one was being delivered.
    std::vector<float> fPendingPresetValues;
    // Brace-init {kParameterCount, 0.0f} would create a 2-element vector and
    // memcpy(kParameterCount*4) below would overflow it — must be a
    // parameter-sized buffer of zeros.
    std::vector<float> fPresetApplyScratch = std::vector<float>(kParameterCount, 0.0f);
    std::atomic<bool> fPendingPresetReady{false};
    uint32_t fPresetStagedGen = 0;
    uint32_t fPresetAppliedGen = 0;
    std::atomic<bool> fPendingDspReset{false};

    // ===== Modulation System =====
    ModulationEngine m_modEngine;
    float m_modOn = 1.0f;
    float m_paramBase[kParameterCount] = {};  // base (unmodulated) values, written in setParameterValue
    std::atomic<float> fModSourceOutput[ModulationEngine::kNumSources]{};
    std::atomic<float> fModSourcePhase[ModulationEngine::kNumSources]{};
    std::atomic<uint32_t> fModUiBlockCount{};
    std::atomic<float> fModUiTempoBpm{120.0f};
    // One entry per continuous param class: range -> per-sample apply fn.
    struct ModTargetClass { uint32_t first, last; void (*fn)(PluginCymbals&, uint32_t, float); };
    static const ModTargetClass kModTargetClasses[];
    void applyModulatedTarget(uint32_t target, float delta);
    // Shared IR render core (export + UI tonal profile). Fills irL/irR.
    bool renderIrSamples(float durationSeconds, std::vector<float>& irL,
                         std::vector<float>& irR, float& outSampleRate,
                         std::string& outError) const;
    bool renderIrSamplesFromValues(const std::vector<float>& values,
                                   float durationSeconds, std::vector<float>& irL,
                                   std::vector<float>& irR, float& outSampleRate,
                                   std::string& outError) const;
    // Apply-table family functions (called via kModTargetClasses).
    static void applyToneFamily(PluginCymbals&, uint32_t, float);
    static void applyDispFamily(PluginCymbals&, uint32_t, float);
    static void applyDisp2Family(PluginCymbals&, uint32_t, float);
    static void applyStickFamily(PluginCymbals&, uint32_t, float);
    static void applyNudge(PluginCymbals&, uint32_t, float);
    static void applyNudge2(PluginCymbals&, uint32_t, float);
    static void applyVibFamily(PluginCymbals&, uint32_t, float);
    static void applyVib2Family(PluginCymbals&, uint32_t, float);
    static void applyChaosFamily(PluginCymbals&, uint32_t, float);
    static void applyChaos2Family(PluginCymbals&, uint32_t, float);
    static void applyPdFamily(PluginCymbals&, uint32_t, float);
    static void applyPd2Family(PluginCymbals&, uint32_t, float);
    static void applyEqFamily(PluginCymbals&, uint32_t, float);       // stick + final EQ nodes
    static void applyTiltFamily(PluginCymbals&, uint32_t, float);     // stick + final tilts
    static void applyCombFamily(PluginCymbals&, uint32_t, float);     // main comb bank
    static void applyFinalCombFamily(PluginCymbals&, uint32_t, float);
    static void applyGlobalFamily(PluginCymbals&, uint32_t, float);   // eqMix, bloom, diffusion, damping, finalAR/clip, stick mseg dur
    static void applySlotFamily(PluginCymbals&, uint32_t, float);
    // Detached IR-render variant (writes into IrExportRenderState components).
    static void applyModulatedDetached(IrExportRenderState& st, const IrExportInputSnapshot& in,
                                       uint32_t target, float delta);

#ifdef CYMBALS_AUDIO_DEBUG
    /**
     * Audio Debug Infrastructure - WAV dumping and instrumentation logging
     * 
     * Used during development to capture and analyze audio routing issues,
     * algorithm transitions, and signal integrity problems.
     * Only compiled when CYMBALS_AUDIO_DEBUG is defined.
     */
    struct AudioDumpState {
        std::vector<float> bufferL, bufferR;
        int frameCounter = 0;
        bool recording = false;
        std::string sessionID;
        std::string label;
        double sampleRate = 48000.0;
    };
    
    AudioDumpState m_audioDump;
    
    /**
     * Start recording audio to WAV file.
     * @param label Descriptive label for the capture (e.g., "algorithm_switch", "silence_dropout")
     */
    void startAudioDump(const std::string& label);
    
    /**
     * Stop recording and write WAV file.
     * @param outError Receives error message if write fails
     * @return true if successful, false if write failed
     */
    bool stopAudioDump(std::string& outError);
    
    /**
     * Write recorded audio to debug log file.
     * @param label Event label (e.g., "silence", "nan_detected", "algorithm_switch")
     * @param context Additional context info
     */
    void logAudioDebugEvent(const std::string& label, const std::string& context = "");
#endif
};

END_NAMESPACE_DISTRHO
#endif
