#ifndef MODULATION_ENGINE_HPP
#define MODULATION_ENGINE_HPP

// ============================================================================
// ModulationEngine — 11 mod sources (LFO/ENV/MSEG/STEP/VEL/NOTE/MW/PB/AT/RND),
// 48 routes, 4 macros.
//
// Audio-thread lock-free: config is written from setParameterValue (UI thread)
// and read per sample on the audio thread — the same race profile as the
// existing member floats in PluginCymbals. Vector-backed rebuilds (MSEG points,
// step values) happen only in setSourceParam, matching StickSoundBank's
// setPitchMsegPoints precedent.
// ============================================================================

#include "DSPUtils.hpp"
#include "AdsrEnvelope.hpp"
#include "MsegEnvelope.hpp"

#include <cmath>
#include <algorithm>
#include <cstring>
#include <cstdint>
#include <vector>
#include <string>
#include <atomic>
#include <random>

class ModulationEngine {
public:
    static constexpr int kNumSources   = 11;
    static constexpr int kNumMacros    = 4;
    static constexpr int kNumRoutes    = 48;
    // ACTIVE route cap: 24, so the whole matrix fits on screen at once with no
    // scrolling and no ADD ROUTE button (the user's chosen design). kNumRoutes
    // stays 48 because the parameter enum
    // (kParamRoute48Enabled == kParamRoute1Source + 48*5 - 1) and kConfigSize
    // are both pinned — shrinking either would shift the enum tail and resize
    // the engine state blob, breaking preset and host-state compatibility.
    // Only the first kMaxActiveRoutes slots are creatable in the UI and applied
    // by the DSP. The cap is enforced in BOTH places on purpose: hiding slots
    // in the UI alone would leave an old preset's route on slot 30+ as an
    // INVISIBLE live modulation, with no row to edit or remove it.
    static constexpr int kMaxActiveRoutes = 24;
    static constexpr int kSourceStride = 66;  // params per source (57..65 = LFO/ENV extras)
    static constexpr int kRouteStride  = 5;   // params per route, Phase B layout
    // Tempo divisions. 0..7 keep their original meanings (append-only, so old
    // presets and host automation stay valid); 8..13 extend the ladder past
    // 1/2 to whole-note cycles and beyond; 14..24 append the dotted ladder.
    // Dotted values must stay at the tail: inserting them next to the straight
    // divisions would renumber every preset's division index.
    static constexpr int kNumDivisions = 25;
    static const char* divisionName(int div) {
        static const char* const kNames[kNumDivisions] = {
            "1/32", "1/16t", "1/16", "1/8t", "1/8", "1/4t", "1/4", "1/2",
            "1/1", "2/1", "4/1", "8/1", "16/1", "32/1",
            "1/32.", "1/16.", "1/8.", "1/4.", "1/2.", "1/1.",
            "2/1.", "4/1.", "8/1.", "16/1.", "32/1."
        };
        return kNames[std::clamp(div, 0, kNumDivisions - 1)];
    }
    // LVGL dropdown options, built once from divisionName() so the labels and
    // the DSP beat table cannot drift apart.
    static const char* divisionOptions() {
        static const std::string options = [] {
            std::string s;
            for (int i = 0; i < kNumDivisions; ++i) {
                if (i != 0) s += '\n';
                s += divisionName(i);
            }
            return s;
        }();
        return options.c_str();
    }

    struct Route {
        uint8_t  source = 0;    // 0..10 = mod sources, 11..14 = macros 1..4
        uint16_t target = 0;    // DPF param index
        float    amount = 0.0f; // bipolar -1..1
        uint8_t  macro  = 0;    // 0 = none, 1..4 = macro scaling the amount
        bool     enabled = false;
    };

    // Performance (live MIDI) sources kinds — live-only state, not preset config.
    // Contract v1 extension: SUSTAIN/SOFTPEDAL/EXPRESSION/POLYPRESS appended after
    // AFTERTOUCH (CC64/CC67/CC11/latest per-note pressure). They are engine-level
    // inputs reserved for later wiring waves; source TYPE codes intentionally stay
    // 0..9 so preset indices and the UI dropdown are untouched this wave.
    // SLIDE (CC74, MPE Y-axis) is appended after POLYPRESS. Source TYPE codes
    // intentionally stay 0..9 so preset indices and the UI dropdown are untouched.
    enum PerfKind { VELOCITY, NOTE, MODWHEEL, PITCHBEND, AFTERTOUCH,
                    SUSTAIN, SOFTPEDAL, EXPRESSION, POLYPRESS,
                    SLIDE }; // MPE Y-axis (CC74) per-voice slide; appended at tail
    static constexpr int kNumPerfInputs = 10; // VEL NOTE MW PB AT | SUS SOFTPEDAL EXPR POLYPRESS | SLIDE

    ModulationEngine() {
        std::memset(m_cfg, 0, sizeof(m_cfg));
        std::memset(m_macro, 0, sizeof(m_macro));
        for (int s = 0; s < kNumSources; ++s) {
            // Phase B defaults (kept in sync with PluginParameters.cpp).
            setSourceParam(s, 3, 1.0f);         // retrig
            setSourceParam(s, 4, 0.5f);         // rate
            setSourceParam(s, 9, 4.0f);         // division (1/8)
            setSourceParam(s, 10, 0.3f);        // a
            setSourceParam(s, 11, 0.3f);        // d
            setSourceParam(s, 12, 0.3f);        // s
            setSourceParam(s, 13, 0.3f);        // r
            setSourceParam(s, 14, 0.25f);       // dur
            for (int k = 0; k < 8; ++k) setSourceParam(s, 17 + k, 0.5f);       // v0..v7
            for (int k = 0; k < 6; ++k) setSourceParam(s, 25 + k, (k + 1) / 7.0f); // t1..t6
            for (int k = 0; k < 7; ++k) setSourceParam(s, 31 + k, 0.5f);       // c0..c6
            for (int k = 0; k < 16; ++k) setSourceParam(s, 38 + k, 0.5f);      // s0..s15
            setSourceParam(s, 54, 8.0f);        // stepCount
            setSourceParam(s, 55, 1.0f);        // stepLoop
            setSourceParam(s, 57, 0.0f);        // LFO mode (loop)
            setSourceParam(s, 63, 0.5f);        // A curve
            setSourceParam(s, 64, 0.5f);        // D curve
            setSourceParam(s, 65, 0.5f);        // R curve
            m_randHeld[s] = 0.5f;
            m_randBrown[s] = 0.5f;
        }
        for (int s = 0; s < kNumSources; ++s) m_envDirty[s] = true;
        for (int r = 0; r < kNumRoutes; ++r) setRouteParam(r, 2, 0.5f); // amount 0
        for (int i = 0; i < kNumMacros; ++i) m_macro[i] = 0.5f;
    }

    // ---- config setters (UI thread, clamp exactly like Phase B ranges) ----
    void setSourceParam(int s, int sub, float v) {
        if (s < 0 || s >= kNumSources || sub < 0 || sub >= kSourceStride) return;
        float* c = m_cfg + s * kSourceStride;
        if (sub == 0)       c[0]  = std::clamp(std::round(v), 0.0f, 9.0f);        // type
        else if (sub == 5)  c[5]  = std::clamp(std::round(v), 0.0f, 5.0f);        // shape
        else if (sub == 9)  c[9]  = std::clamp(std::round(v), 0.0f, (float)(kNumDivisions - 1)); // division
        else if (sub == 16) c[16] = std::clamp(std::round(v), 0.0f, 8.0f);        // sustain
        else if (sub == 54) c[54] = std::clamp(std::round(v), 1.0f, 16.0f);       // stepCount
        else if (sub == 57) c[57] = std::clamp(std::round(v), 0.0f, 2.0f);        // LFO mode
        else                c[sub] = std::clamp(v, 0.0f, 1.0f);
        m_envDirty[s] = true; // ENV times/curves are re-applied lazily, not per sample
        // Targeted runtime rebuild: mutate only the subsystem this sub-param
        // feeds, reusing preallocated scratch. Every write previously rebuilt
        // the heap-backed MSEG points even for an unrelated LFO-rate change.
        if (sub == 14 || sub == 15 || sub == 16 || (sub >= 17 && sub <= 37)) {
            rebuildMseg(s);        // MSEG duration/loop/sustain + shape (v/t/c)
        } else if (sub >= 38 && sub <= 56) {
            rebuildSteps(s);       // STEP values/count/loop/smooth
        } else if (sub == 4 || sub == 8 || sub == 9) {
            rebuildSteps(s);       // rate / sync-division fields -> STEP rateHz
        }
        // sub 0 (type), 5 (shape), 7 (filter), 57 (LFO mode): read live in
        // processSample(); no runtime port to rebuild.
    }

    void setRouteParam(int r, int sub, float v) {
        if (r < 0 || r >= kNumRoutes || sub < 0 || sub >= kRouteStride) return;
        Route& rt = m_routes[r];
        switch (sub) {
            case 0: rt.source = (uint8_t)std::clamp(std::round(v), 0.0f, (float)(kNumSources + kNumMacros - 1)); break;
            case 1: rt.target = (uint16_t)std::clamp(std::round(v), 0.0f, 4095.0f); break;
            case 2: rt.amount = std::clamp(v * 2.0f - 1.0f, -1.0f, 1.0f); break;
            case 3: rt.macro  = (uint8_t)std::clamp(std::round(v), 0.0f, 4.0f); break;
            case 4: {
                const bool was = rt.enabled;
                rt.enabled = v > 0.5f;
                if (rt.enabled != was) m_activeRouteCount += rt.enabled ? 1 : -1;
                break;
            }
        }
    }

    void setMacro(int i, float v) {
        if (i < 0 || i >= kNumMacros) return;
        m_macro[i] = std::clamp(v, 0.0f, 1.0f);
    }

    void setPerformanceInput(PerfKind k, float v) {
        if (k < VELOCITY || k > SLIDE) return;
        // SLIDE is the MPE Y-axis (CC74) — semitone offset, range [-1, 1]
        // mapped to ±24 st by the caller. Allow bipolar writes only for SLIDE
        // (and PITCHBEND, which already took the bipolar branch).
        m_perf[(int)k] = ((k == PITCHBEND) || (k == SLIDE))
            ? std::clamp(v, -1.0f, 1.0f)
            : std::clamp(v, 0.0f, 1.0f);
    }

    // Test/introspection readback of live performance inputs (see PerfKind).
    float getPerformanceInput(int k) const {
        return (k >= (int)VELOCITY && k <= (int)SLIDE) ? m_perf[k] : 0.0f;
    }

    float getSourceParam(int s, int sub) const {
        if (s < 0 || s >= kNumSources || sub < 0 || sub >= kSourceStride) return 0.0f;
        return m_cfg[s * kSourceStride + sub];
    }

    float getRouteParam(int r, int sub) const {
        if (r < 0 || r >= kNumRoutes || sub < 0 || sub >= kRouteStride) return 0.0f;
        const Route& rt = m_routes[r];
        switch (sub) {
            case 0: return (float)rt.source;
            case 1: return (float)rt.target;
            case 2: return rt.amount * 0.5f + 0.5f;
            case 3: return (float)rt.macro;
            default: return rt.enabled ? 1.0f : 0.0f;
        }
    }

    float getMacro(int i) const { return (i >= 0 && i < kNumMacros) ? m_macro[i] : 0.0f; }

    const Route& route(int r) const { return m_routes[r]; }
    float macro(int i) const { return getMacro(i); }
    int activeRouteCount() const { return m_activeRouteCount; }

    // ---- IR export config snapshot: sources + routes + macros (raw floats) ----
    static constexpr int kConfigSize = kNumSources * kSourceStride + kNumRoutes * kRouteStride + kNumMacros;
    void copyConfigTo(float* dst) const {
        std::memcpy(dst, m_cfg, sizeof(m_cfg));
        float* r = dst + kNumSources * kSourceStride;
        for (int i = 0; i < kNumRoutes; ++i) {
            r[i * 5 + 0] = (float)m_routes[i].source;
            r[i * 5 + 1] = (float)m_routes[i].target;
            r[i * 5 + 2] = m_routes[i].amount * 0.5f + 0.5f;
            r[i * 5 + 3] = (float)m_routes[i].macro;
            r[i * 5 + 4] = m_routes[i].enabled ? 1.0f : 0.0f;
        }
        std::memcpy(dst + kNumSources * kSourceStride + kNumRoutes * kRouteStride, m_macro, sizeof(m_macro));
    }

    void copyConfigFrom(const float* src) {
        std::memcpy(m_cfg, src, sizeof(m_cfg));
        for (int s = 0; s < kNumSources; ++s) rebuildSourceRuntime(s);
        for (int i = 0; i < kNumRoutes; ++i) {
            const float* r = src + kNumSources * kSourceStride + i * 5;
            setRouteParam(i, 0, r[0]);
            setRouteParam(i, 1, r[1]);
            setRouteParam(i, 2, r[2]);
            setRouteParam(i, 3, r[3]);
            setRouteParam(i, 4, r[4]);
        }
        std::memcpy(m_macro, src + kNumSources * kSourceStride + kNumRoutes * kRouteStride, sizeof(m_macro));
    }

    // ---- runtime control (audio thread) ----
    void setSampleRate(float sr) {
        m_sampleRate = sr > 0.0f ? sr : 48000.0f;
        for (int s = 0; s < kNumSources; ++s) {
            lfos[s].setSampleRate(m_sampleRate);
            envs[s].setSampleRate(m_sampleRate);
            msegs[s].setSampleRate(m_sampleRate);
            steps[s].setSampleRate(m_sampleRate);
        }
    }

    void setTempoBpm(float bpm) {
        if (bpm > 0.0f) m_tempoBpm = bpm;
    }

    void noteOn() {
        for (int s = 0; s < kNumSources; ++s) {
            const float* c = m_cfg + s * kSourceStride;
            if (c[1] < 0.5f) continue; // disabled
            const int type = (int)c[0];
            if (type == 1) {
                // Push delay/hold/curves before trigger() so the new note starts
                // in the right segment (processSample keeps them fresh per sample).
                envs[s].setDelay(c[61] * c[61] * 4.0f);
                envs[s].setHold(c[62] * c[62] * 4.0f);
                envs[s].setCurves(c[63] * 2.0f - 1.0f, c[64] * 2.0f - 1.0f, c[65] * 2.0f - 1.0f);
                envs[s].trigger(); // ENV always retriggers on note-on
                m_envT[s] = 0.0f;  // phase walk restarts with the new envelope
            } else if (type == 2) {
                // MSEG is a one-shot envelope. With RETRIG off it must still
                // fire the first time (and after each completed pass); the old
                // code only triggered it under the retrig flag, so a new MSEG
                // source sat inactive at its END value - a static modulation
                // offset (reported as the filter frequency jumping to max and
                // staying). RETRIG on restarts it every note; off only avoids
                // restarting a pass that is still running.
                if (c[3] > 0.5f || !msegs[s].isActive()) msegs[s].trigger();
            } else if (c[3] > 0.5f) { // retrig flag
                if (type == 0) {
                    lfos[s].setPhase(c[6]);
                    m_lfoElapsed[s] = 0;
                    m_lfoHeld[s] = false;
                    m_lfoSustainFrozen[s] = false;
                    m_lfoSustainReached[s] = false;
                }
                else if (type == 3) steps[s].trigger();
                else if (type == 9) m_randFresh[s] = true; // re-roll on next processSample
            }
        }
    }

    void noteOff() {
        for (int s = 0; s < kNumSources; ++s) {
            if (m_cfg[s * kSourceStride + 1] < 0.5f) continue;
            const float* c = m_cfg + s * kSourceStride;
            if ((int)c[0] == 1) envs[s].releaseNote();
            else if ((int)c[0] == 0 && (int)c[57] == 2) {
                // LFO sustain-env: release the hold; the phase resumes from the
                // sustain point, runs to the end, then latches valueAt(1.0).
                // Disarm the sustain latch so the tail can never re-hold.
                m_lfoHeld[s] = false;
                m_lfoSustainFrozen[s] = false;
                m_lfoSustainReached[s] = true;
            }
        }
    }

    void reset() {
        // Audio-thread safe: no heap allocation (MSEG keeps its instance; the
        // point config is already live from the last setSourceParam).
        for (int s = 0; s < kNumSources; ++s) {
            lfos[s] = DSPUtils::LFO(m_sampleRate);
            envs[s] = AdsrEnvelope(m_sampleRate);
            msegs[s].deactivate();
            steps[s].resetState();
            smoothers[s].clear();
            m_sourceOutput[s] = 0.0f;
            m_phase[s] = 0.0f;
            m_envT[s] = 0.0f;
            m_lfoElapsed[s] = 0;
            m_lfoHeld[s] = false;
            m_lfoSustainFrozen[s] = false;
            m_lfoSustainReached[s] = false;
            m_lfoEndValue[s] = 0.0f;
            m_randPhase[s] = 0.0f;
            m_randHeld[s] = 0.5f;
            m_randFresh[s] = false;
            m_randBrown[s] = 0.5f;
        }
    }

    void processSample(float out[kNumSources]) {
        for (int s = 0; s < kNumSources; ++s) {
            const float* c = m_cfg + s * kSourceStride;
            if (c[1] < 0.5f) { // disabled: silent, frozen
                m_sourceOutput[s] = 0.0f;
                out[s] = 0.0f;
                continue;
            }
            float v = 0.0f;
            const int type = (int)c[0];
            const bool bipolar = c[2] > 0.5f;
            if (type == 0) { // LFO
                const float effHz = (c[8] > 0.5f)
                    ? (float)(m_tempoBpm / (beatsForDivision((int)c[9]) * 60.0))
                    : freeRateHz(c[4]);
                // UI dropdown order is SINE TRI SAW SQR STEP RAND (display order);
                // translate to the append-only enum (SINE RANDOM STEPPED_SINE SQUARE TRIANGLE SAW).
                static const DSPUtils::LFO::Shape kShapeMap[6] = {
                    DSPUtils::LFO::SINE, DSPUtils::LFO::TRIANGLE, DSPUtils::LFO::SAW,
                    DSPUtils::LFO::SQUARE, DSPUtils::LFO::STEPPED_SINE, DSPUtils::LFO::RANDOM
                };
                lfos[s].setShape(kShapeMap[std::clamp((int)c[5], 0, 5)]);
                lfos[s].setStepMix(1.0f);
                const int mode = (int)c[57];
                const float prevPhase = lfos[s].getPhase();
                float raw;
                ++m_lfoElapsed[s];
                const float delaySamples = c[59] * c[59] * 4.0f * m_sampleRate;
                if (m_lfoHeld[s]) {
                    if (mode == 0) {
                        // Switched to LOOP mid-hold: release the latch, resume cycling.
                        m_lfoHeld[s] = false;
                        m_lfoSustainFrozen[s] = false;
                        raw = lfos[s].processWithRate(effHz);
                    } else {
                        if (mode == 1 && m_lfoSustainFrozen[s]) {
                            // Switched SUSTAIN-ENV -> ONE-SHOT mid-hold: re-latch
                            // the one-shot tail value.
                            m_lfoEndValue[s] = lfos[s].valueAt(1.0f);
                            m_lfoSustainFrozen[s] = false;
                        }
                        raw = m_lfoEndValue[s];
                    }
                } else if ((float)m_lfoElapsed[s] < delaySamples) {
                    // Delay: silent AND phase-frozen, so a one-shot whose delay
                    // spans a full period still plays its shape after the delay.
                    raw = 0.0f;
                } else {
                    raw = lfos[s].processWithRate(effHz);
                    if (prevPhase > lfos[s].getPhase() && mode != 0) { // wrapped: one-shot tail
                        m_lfoEndValue[s] = lfos[s].valueAt(1.0f);
                        m_lfoHeld[s] = true;
                    } else if (mode == 2 && c[16] > 0.5f && !m_lfoSustainReached[s]) {
                        const float sustainPhase = c[16] / 8.0f;
                        if (lfos[s].getPhase() >= sustainPhase) {
                            // Enter the sustain hold: output the value at the
                            // sustain point, freeze phase until noteOff.
                            m_lfoEndValue[s] = lfos[s].valueAt(sustainPhase);
                            m_lfoHeld[s] = true;
                            m_lfoSustainFrozen[s] = true;
                            m_lfoSustainReached[s] = true;
                            raw = m_lfoEndValue[s];
                        }
                    }
                }
                // Fade-in (elapsed already advanced above; delay frozen the phase).
                const float fadeSamples = c[58] * c[58] * 8.0f * m_sampleRate;
                if (fadeSamples > 0.0f) {
                    raw *= std::clamp(((float)m_lfoElapsed[s] - delaySamples) / fadeSamples, 0.0f, 1.0f);
                }
                // Quantize the bipolar raw (2..16 levels) before unipolar mapping.
                const float q = c[60];
                if (q >= 0.02f) {
                    const float levels = 2.0f + std::round(q * 14.0f);
                    raw = std::round((raw + 1.0f) * 0.5f * (levels - 1.0f)) / (levels - 1.0f) * 2.0f - 1.0f;
                }
                // Silence gate LAST: quantize would otherwise map the delay's
                // zero to a nonzero step for even level counts. Clear the
                // smoother so a retrigger into the delay window cannot bleed
                // residual state from the previous note.
                if ((float)m_lfoElapsed[s] < delaySamples) {
                    smoothers[s].clear();
                    raw = 0.0f;
                }
                v = bipolar ? raw : raw * 0.5f + 0.5f; // raw is already -1..1; unipolar maps to 0..1
                m_phase[s] = lfos[s].getPhase();
            } else if (type == 1) { // ENV
                if (m_envDirty[s]) {
                    envs[s].setAttack(0.001f + std::pow(c[10], 2.0f) * 1.999f);
                    envs[s].setDecay(0.001f + std::pow(c[11], 2.0f) * 1.999f);
                    envs[s].setSustain(c[12]);
                    envs[s].setRelease(0.002f + std::pow(c[13], 2.0f) * 2.998f);
                    envs[s].setDelay(c[61] * c[61] * 4.0f);        // 0..4 s
                    envs[s].setHold(c[62] * c[62] * 4.0f);         // 0..4 s
                    envs[s].setCurves(c[63] * 2.0f - 1.0f, c[64] * 2.0f - 1.0f, c[65] * 2.0f - 1.0f);
                    m_envDirty[s] = false;
                }
                const float raw = envs[s].process();
                v = bipolar ? raw * 2.0f - 1.0f : raw;
                // Publish TIME-domain progress (0..1) so the badge playhead rides the
                // ADSR curve on its time axis. Layout mirrors the UI preview
                // (badgeValueAt/previewOutput): attack+decay+fixed hold 0.5+release;
                // engine delay/hold params are ignored by that layout (Assumption).
                {
                    const float aS = envs[s].attack;
                    const float dS = envs[s].decay;
                    const float rS = envs[s].release;
                    const float total = aS + dS + 0.5f + rS;
                    const float seg = envs[s].getSegElapsedSamples() / m_sampleRate;
                    float tSec = total;
                    switch (envs[s].getState()) {
                        case AdsrEnvelope::State::Idle:    tSec = total; break;
                        case AdsrEnvelope::State::Delay:   tSec = 0.0f; break;
                        case AdsrEnvelope::State::Attack:  tSec = std::min(seg, aS); break;
                        case AdsrEnvelope::State::Hold:    tSec = aS; break;
                        case AdsrEnvelope::State::Decay:   tSec = aS + std::min(seg, dS); break;
                        // Walk the fixed 0.5 hold span monotonically during the
                        // (indefinite) sustain so the playhead never teleports.
                        // (segElapsed is reset at the Decay->Sustain transition.)
                        case AdsrEnvelope::State::Sustain: tSec = aS + dS + std::min(seg, 0.5f); break;
                        // Continue from wherever the walk was — a release from
                        // mid-attack/decay/sustain must not jump to the layout's
                        // fixed positions. Scale the release segment so it
                        // reaches `total` exactly when the envelope completes
                        // (Release->Idle continuity).
                        case AdsrEnvelope::State::Release: {
                            const float remain = std::max(0.0f, total - m_envT[s]);
                            tSec = m_envT[s] + std::min(seg, rS) * (remain / std::max(rS, 1e-6f));
                            break;
                        }
                    }
                    tSec = std::min(tSec, total);
                    // m_envT is the Release anchor: freeze it at the entry
                    // position so the release walk stays linear instead of
                    // compounding (re-anchoring each sample with cumulative seg
                    // would lurch through the span in milliseconds).
                    if (envs[s].getState() != AdsrEnvelope::State::Release)
                        m_envT[s] = tSec;
                    m_phase[s] = tSec / total;
                }
            } else if (type == 2) { // MSEG
                const float raw = msegs[s].process();
                v = bipolar ? raw * 2.0f - 1.0f : raw;
                m_phase[s] = msegs[s].getPhase();
            } else if (type == 3) { // STEP
                // Recompute per sample so host tempo changes reach synced steps
                // (setTempoBpm runs per block on the audio thread).
                const float stepHz = (c[8] > 0.5f)
                    ? (float)(m_tempoBpm / (beatsForDivision((int)c[9]) * 60.0))
                    : freeRateHz(c[4]);
                steps[s].setRateHz(stepHz);
                const float raw = steps[s].process();
                v = bipolar ? raw * 2.0f - 1.0f : raw;
                m_phase[s] = steps[s].getPhase();
            } else if (type >= 4 && type <= 8) { // Performance sources (4..8)
                // raw values: VELOCITY/NOTE/MODWHEEL/AFTERTOUCH 0..1,
                // PITCHBEND -1..1.
                float raw;
                if (type == 4)       raw = m_perf[0];
                else if (type == 5)  raw = m_perf[1];
                else if (type == 6)  raw = m_perf[2];
                else if (type == 7)  raw = m_perf[3];
                else                 raw = m_perf[4];
                if (type == 7) v = bipolar ? raw : raw * 0.5f + 0.5f;
                else           v = bipolar ? raw * 2.0f - 1.0f : raw;
                m_phase[s] = (type == 7) ? 0.5f + 0.5f * raw : v;
            } else { // type 9 RANDOM: sample-and-hold coloured noise
                // RETRIG (c[3]) off = free-running re-roll clock at RATE (c[4]);
                // on = re-roll per note-on (m_randFresh set by noteOn).
                const bool freeRun = c[3] < 0.5f;
                if (freeRun) {
                    // Discard any pending per-note roll: free-run owns the clock.
                    m_randFresh[s] = false;
                    m_randPhase[s] += freeRateHz(c[4]) / m_sampleRate;
                    if (m_randPhase[s] >= 1.0f) {
                        m_randPhase[s] -= 1.0f;
                        m_randHeld[s] = nextNoise(s, c[5]);
                    }
                } else if (m_randFresh[s]) {
                    m_randFresh[s] = false;
                    m_randHeld[s] = nextNoise(s, c[5]);
                }
                float raw = m_randHeld[s];
                // SPREAD (c[58]): contrast around 0.5; 0 = full soft range,
                // 1 = slams values to the 0/1 extremes.
                raw = std::clamp(0.5f + (raw - 0.5f) * (1.0f + 2.0f * c[58]), 0.0f, 1.0f);
                // QUANTIZE (c[60]): snap to 2..16 levels (mirrors LFO quant).
                const float q = c[60];
                if (q >= 0.02f) {
                    const float levels = 2.0f + std::round(q * 14.0f);
                    raw = std::round(raw * (levels - 1.0f)) / (levels - 1.0f);
                }
                v = bipolar ? raw * 2.0f - 1.0f : raw;
                m_phase[s] = freeRun ? m_randPhase[s] : raw;
            }
            // Smoother: bypassed entirely at smooth <= 0 (ENV attacks stay
            // sharp, audio-rate LFOs pass unfiltered); else 10..1000 Hz.
            if (c[7] < 0.02f) {
                smoothers[s].clear();
                m_sourceOutput[s] = v;
            } else {
                smoothers[s].setCutoffHz(10.0f + c[7] * 990.0f, m_sampleRate);
                m_sourceOutput[s] = smoothers[s].processLP(v);
            }
            out[s] = m_sourceOutput[s];
        }
    }

    // Publish last-block UI state into atomics (plugin calls once per block).
    void publishUiState(std::atomic<float>* out, std::atomic<float>* ph,
                        std::atomic<uint32_t>* blockCounter, std::atomic<float>* tempo) {
        for (int s = 0; s < kNumSources; ++s) {
            out[s].store(m_sourceOutput[s], std::memory_order_relaxed);
            ph[s].store(m_phase[s], std::memory_order_relaxed);
        }
        blockCounter->store(++m_uiBlockCount, std::memory_order_relaxed);
        tempo->store(m_tempoBpm, std::memory_order_relaxed);
    }

    float getSourceOutput(int s) const { return (s >= 0 && s < kNumSources) ? m_sourceOutput[s] : 0.0f; }
    float getSourcePhase(int s) const { return (s >= 0 && s < kNumSources) ? m_phase[s] : 0.0f; }

    // Free (non-synced) rate param 0..1 -> 0.01..5000 Hz log (LFO + STEP).
    static float freeRateHz(float c) { return 0.01f * std::pow(500000.0f, std::clamp(c, 0.0f, 1.0f)); }

    static double beatsForDivision(int div) {
        // Order matches divisionName()/divisionOptions(): 1/32 ... 32/1.
        static const double kBeats[kNumDivisions] = {
            0.125, 0.1666666667, 0.25, 0.3333333333, 0.5, 0.6666666667, 1.0, 2.0,
            4.0, 8.0, 16.0, 32.0, 64.0, 128.0,
            0.1875, 0.375, 0.75, 1.5, 3.0, 6.0,
            12.0, 24.0, 48.0, 96.0, 192.0
        };
        return kBeats[std::clamp(div, 0, kNumDivisions - 1)];
    }

    // ========================================================================
    // StepSequencer — sample-and-hold step values, optional 1-pole smoothing
    // between steps, loop/one-shot. period = 1/rateHz per step.
    // ========================================================================
    class StepSequencer {
    public:
        StepSequencer(float sampleRate = 48000.0f) : sampleRate(sampleRate) {}
        void setSampleRate(float sr) { sampleRate = sr > 0.0f ? sr : 48000.0f; }
        void setRateHz(float hz) { rateHz = std::max(1e-6f, hz); }
        void setStepValues(const float* vals, int count) {
            stepCount = std::clamp(count, 1, 16);
            for (int i = 0; i < 16; ++i) values[i] = (i < stepCount) ? std::clamp(vals[i], 0.0f, 1.0f) : 0.0f;
            if (stepIndex >= stepCount) stepIndex = 0;
        }
        void setLoop(bool on) { loop = on; }
        void setSmooth(bool on) { smooth = on; }
        void trigger() { stepIndex = 0; intraPhase = 0.0f; finished = false; }
        void resetState() { stepIndex = 0; intraPhase = 0.0f; finished = false; }
        float process() {
            if (finished || stepCount <= 0) return 0.0f;
            // per-step period = 1/rateHz; advance intraPhase by rateHz/sampleRate.
            const float dt = rateHz / sampleRate;
            intraPhase += dt;
            while (intraPhase >= 1.0f) {
                intraPhase -= 1.0f;
                ++stepIndex;
                if (stepIndex >= stepCount) {
                    if (loop) {
                        stepIndex = 0;
                    } else {
                        finished = true;
                        return 0.0f; // one-shot: silent after the last step
                    }
                }
            }
            if (smooth) {
                const float cur = values[stepIndex];
                const float nxt = (stepIndex + 1 < stepCount) ? values[stepIndex + 1] : (loop ? values[0] : cur);
                return cur + (nxt - cur) * intraPhase;
            }
            return values[stepIndex];
        }
        float getPhase() const { return std::min((float)(stepIndex + intraPhase) / (float)stepCount, 1.0f); }
    private:
        float sampleRate = 48000.0f;
        float rateHz = 1.0f;
        float values[16] = {};
        int stepCount = 8;
        bool loop = true;
        bool smooth = false;
        int stepIndex = 0;
        float intraPhase = 0.0f;
        bool finished = false;
    };

private:
    // Rebuild only the MSEG runtime port: points (v/t/c), duration, loop,
    // sustain. Uses the reused m_msegScratch (no heap allocation steady-state).
    void rebuildMseg(int s) {
        const float* c = m_cfg + s * kSourceStride;
        m_msegScratch.clear();
        m_msegScratch.push_back({0.0f, c[17], (c[31] - 0.5f) * 2.0f});
        for (int k = 1; k <= 6; ++k) {
            m_msegScratch.push_back({std::clamp(c[24 + k], 0.0f, 1.0f), c[17 + k], (c[31 + k] - 0.5f) * 2.0f});
        }
        m_msegScratch.push_back({1.0f, c[24], 0.0f});
        msegs[s].setPoints(m_msegScratch);
        msegs[s].setDuration(0.01f * std::pow(1000.0f, c[14]));
        msegs[s].setLoop(c[15] > 0.5f);
        msegs[s].setSustainPoint(((int)c[16] >= 1) ? (int)c[16] - 1 : -1);
    }

    // Rebuild only the STEP runtime port: values, stepCount, loop, smooth,
    // rateHz. StepSequencer stores fixed arrays — alloc-free by construction.
    void rebuildSteps(int s) {
        const float* c = m_cfg + s * kSourceStride;
        float stepVals[16];
        for (int k = 0; k < 16; ++k) stepVals[k] = c[38 + k];
        steps[s].setStepValues(stepVals, (int)c[54]);
        steps[s].setLoop(c[55] > 0.5f);
        steps[s].setSmooth(c[56] > 0.5f);
        const float rateHz = (c[8] > 0.5f)
            ? (float)(m_tempoBpm / (beatsForDivision((int)c[9]) * 60.0))
            : freeRateHz(c[4]);
        steps[s].setRateHz(rateHz);
    }

    // Full runtime rebuild (both ports) for bulk config restore.
    void rebuildSourceRuntime(int s) {
        rebuildMseg(s);
        rebuildSteps(s);
    }

    float m_cfg[kNumSources * kSourceStride] = {};
    bool m_envDirty[kNumSources] = {};
    Route m_routes[kNumRoutes];
    float m_macro[kNumMacros] = {};
    int m_activeRouteCount = 0;
    // Reused across rebuildMseg calls so shape writes never heap-allocate the
    // temporary MSEG point list (steady-state). Config/ctor/preset-apply path
    // only (setSourceParam), never touched by processSample's reads.
    std::vector<MsegPoint> m_msegScratch;

    // Live performance inputs (defaults = deterministic no-MIDI fallback).
    float m_perf[kNumPerfInputs] = {1.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}; // VEL NOTE MW PB AT | SUS SOFTPEDAL EXPR POLYPRESS | SLIDE
    std::mt19937 m_rng{12345};                        // fixed seed: deterministic
    std::uniform_real_distribution<float> m_rnd01{0.0f, 1.0f};

    // RANDOM source state (type 9): sample-and-hold coloured noise.
    float m_randPhase[kNumSources] = {};  // free-run re-roll clock phase (0..1)
    float m_randHeld[kNumSources] = {};   // current S&H value (init 0.5 in ctor)
    bool  m_randFresh[kNumSources] = {};  // per-note re-roll pending (set by noteOn)
    float m_randBrown[kNumSources] = {};  // brown-noise accumulator (reset 0.5)
    // Coloured S&H value: 0=white, 1=pink (8-sample average, Kellet-lite),
    // 2=brown (leaky random walk). Bounded [0,1], deterministic per fixed seed.
    float nextNoise(int s, float colour) {
        const int c = std::clamp((int)std::lround(colour), 0, 2);
        if (c == 2) {
            m_randBrown[s] = std::clamp(m_randBrown[s] + (m_rnd01(m_rng) - 0.5f) * 0.1f, 0.0f, 1.0f);
            return m_randBrown[s];
        }
        if (c == 1) {
            float sum = 0.0f;
            for (int k = 0; k < 8; ++k) sum += m_rnd01(m_rng);
            return sum / 8.0f;
        }
        return m_rnd01(m_rng);
    }

    float m_sampleRate = 48000.0f;
    float m_tempoBpm = 120.0f; // deterministic fallback when no transport
    uint32_t m_uiBlockCount = 0; // published block tick for the UI idle-preview gate

    DSPUtils::LFO lfos[kNumSources];
    AdsrEnvelope envs[kNumSources];
    MsegEnvelope msegs[kNumSources];
    StepSequencer steps[kNumSources];
    DSPUtils::OnePoleFilter smoothers[kNumSources];
    float m_sourceOutput[kNumSources] = {};
    float m_phase[kNumSources] = {};
    float m_envT[kNumSources] = {}; // ENV phase-walk position (continuous release anchor)
    // LFO delay/fade/mode state (live only, not part of the preset config).
    uint32_t m_lfoElapsed[kNumSources] = {}; // samples since last trigger/retrig
    bool m_lfoHeld[kNumSources] = {};     // one-shot tail / sustain-env hold latched
    bool m_lfoSustainFrozen[kNumSources] = {}; // phase frozen at the sustain point
    bool m_lfoSustainReached[kNumSources] = {}; // sustain point held this note (cleared on retrig)
    float m_lfoEndValue[kNumSources] = {};     // latched output while held
};

#endif // MODULATION_ENGINE_HPP
