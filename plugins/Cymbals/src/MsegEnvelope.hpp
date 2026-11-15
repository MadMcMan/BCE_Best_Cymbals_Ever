#ifndef MSEG_ENVELOPE_HPP
#define MSEG_ENVELOPE_HPP

#include <vector>
#include <algorithm>
#include <atomic>
#include <cmath>

struct MsegPoint {
    float time;   // 0 to 1 relative to total duration
    float value;  // 0 to 1 or -1 to 1
    float curve;  // 0 is linear, positive is convex, negative is concave
};

class MsegEnvelope {
public:
    MsegEnvelope(float sampleRate = 48000.0f)
        : sampleRate(sampleRate), active(false), phase(0.0f), totalDuration(0.1f) {
        // Default points: rapid drop for percussive pitch
        points = {
            {0.0f, 1.0f, 0.0f},
            {0.02f, 0.8f, 0.0f},
            {0.1f, 0.2f, 0.0f},
            {1.0f, 0.0f, 0.0f}
        };
    }

    void setSampleRate(float sr) { sampleRate = sr; }
    void setDuration(float seconds) { totalDuration = std::max(0.001f, seconds); }
    // Mod-source extensions (defaults off: existing StickSoundBank usage is
    // byte-identical). setLoop wraps phase instead of deactivating at the end;
    // setSustainPoint(idx >= 0) holds the interpolated value at that point's
    // time until the next trigger() (loop ignored while held).
    void setLoop(bool on) { loop = on; }
    void setSustainPoint(int idx) { sustainPoint = idx; }
    
    void setPoints(const std::vector<MsegPoint>& pts) {
        points = pts;
        std::sort(points.begin(), points.end(), [](const MsegPoint& a, const MsegPoint& b) {
            return a.time < b.time;
        });
        // Ensure first point at time 0
        if (points.empty()) {
            points = {{0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}};
        } else {
            if (points.front().time > 0.0f) {
                points.insert(points.begin(), {0.0f, points.front().value, 0.0f});
            }
            if (points.back().time < 1.0f) {
                points.push_back({1.0f, points.back().value, 0.0f});
            }
        }
    }

    void trigger() {
        active.store(true, std::memory_order_relaxed);
        phase.store(0.0f, std::memory_order_relaxed);
    }

    void deactivate() {
        active.store(false, std::memory_order_relaxed);
        phase.store(0.0f, std::memory_order_relaxed);
    }

    float process() {
        if (!active.load(std::memory_order_relaxed)) return points.empty() ? 0.0f : points.back().value;

        // Sustain hold: stop advancing once the sustain point's time is reached.
        const float p0 = phase.load(std::memory_order_relaxed);
        if (sustainPoint >= 0 && sustainPoint < (int)points.size()) {
            const float sustainTime = points[sustainPoint].time;
            if (p0 >= sustainTime) {
                return valueAt(sustainTime);
            }
        }

        float increment = 1.0f / (std::max(0.0001f, totalDuration) * sampleRate);
        const float p = p0 + increment;
        if (p >= 1.0f) {
            // Sustain at the final point (time 1.0) with loop on: hold the
            // endpoint instead of wrapping.
            if (loop && sustainPoint >= 0 && sustainPoint < (int)points.size()
                && points[sustainPoint].time >= 1.0f) {
                phase.store(1.0f, std::memory_order_relaxed);
                return points.back().value;
            }
            if (loop) {
                phase.store(std::fmod(p, 1.0f), std::memory_order_relaxed);
            } else {
                phase.store(1.0f, std::memory_order_relaxed);
                active.store(false, std::memory_order_relaxed);
                return points.back().value;
            }
            return valueAt(std::fmod(p, 1.0f));
        }
        phase.store(p, std::memory_order_relaxed);
        return valueAt(p);
    }

    float valueAt(float p) const {
        // Find current interval
        int idx = 0;
        int numPoints = (int)points.size();
        while (idx < numPoints - 1 && p > points[idx+1].time) {
            idx++;
        }

        if (idx >= numPoints - 1) return points.back().value;

        float t0 = points[idx].time;
        float t1 = points[idx+1].time;
        float v0 = points[idx].value;
        float v1 = points[idx+1].value;
        float curve = points[idx].curve;

        float den = t1 - t0;
        float relT = (den > 1e-6f) ? (p - t0) / den : 1.0f;
        relT = std::clamp(relT, 0.0f, 1.0f);

        // Apply curve
        if (std::abs(curve) < 0.01f) {
            return v0 + (v1 - v0) * relT;
        } else {
            // Power curve: y = x^p
            // We use a mapping where 0.5 is linear, >0.5 convex, <0.5 concave
            // but the struct uses 0 as linear.
            float pwr = 1.0f;
            if (curve > 0) pwr = 1.0f + curve * 4.0f;
            else pwr = 1.0f / (1.0f - curve * 4.0f);

            float ct = std::pow(relT, pwr);
            return v0 + (v1 - v0) * ct;
        }
    }

    bool isActive() const { return active.load(std::memory_order_relaxed); }
    float getPhase() const { return phase.load(std::memory_order_relaxed); }

private:
    float sampleRate;
    std::atomic<bool> active{false};
    std::atomic<float> phase{0.0f};
    float totalDuration;
    std::vector<MsegPoint> points;
    bool loop = false;
    int sustainPoint = -1;
};

#endif
