#ifndef DSP_UTILS_HPP
#define DSP_UTILS_HPP

#include <cmath>
#include <algorithm>
#include <vector>
#include <cstring>
#include <random>
#include <complex>
#include <limits>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace DSPUtils {

inline constexpr float kPi = 3.14159265358979323846f;

// --- Shared parameter-mapping helpers (single source of truth for these formulas) ---
inline float normToHz(float v) { return 20.0f * std::pow(1000.0f, v); }
inline float scaleFactorFromParam(float v) { return 0.25f * std::pow(16.0f, v); }
inline float normGainToDb(float v) { return (v > 0.001f) ? (v - 1.0f) * 60.0f : -100.0f; }
inline float tiltDbFromParam(float v) { return (v - 0.5f) * 12.0f; }
inline float qFromParam(float v) { return 0.1f + v * 9.9f; }
// 0.0 -> ~4ms, 0.5 -> ~64ms, 1.0 -> ~1.0s (exponential gives better perceptual spread)
inline float inharmBloomDecayFromControl(float control) {
    return 0.004f * std::pow(2.0f, std::clamp(control, 0.0f, 1.0f) * 8.0f);
}

struct Biquad {
    enum Type { BELL, LOW_SHELF, HIGH_SHELF, HPF, LPF, NOTCH, BAND_PASS, ALL_PASS };
    float b0=1, b1=0, b2=0, a1=0, a2=0;
    float z1L=0, z2L=0, z1R=0, z2R=0;

    void setParams(Type type, float freqHz, float gainDb, float Q, float sr) {
        if (sr <= 0.0f) sr = 44100.0f;
        float omega = 2.0f * M_PI * std::clamp(freqHz, 10.0f, sr * 0.49f) / sr;
        float A = std::pow(10.0f, gainDb / 40.0f);
        float sn = std::sin(omega), cs = std::cos(omega);
        float alpha = sn / (2.0f * Q);
        float a0 = 1.0f;
        switch(type) {
            case BELL: 
                b0 = 1.0f + alpha * A; b1 = -2.0f * cs; b2 = 1.0f - alpha * A; 
                a0 = 1.0f + alpha / A; a1 = -2.0f * cs; a2 = 1.0f - alpha / A; 
                break;
            case LOW_SHELF: { 
                float sqA = std::sqrt(A); 
                b0 = A*((A+1.0f) - (A-1.0f)*cs + 2.0f*sqA*alpha); 
                b1 = 2.0f*A*((A-1.0f) - (A+1.0f)*cs); 
                b2 = A*((A+1.0f) - (A-1.0f)*cs - 2.0f*sqA*alpha); 
                a0 = (A+1.0f) + (A-1.0f)*cs + 2.0f*sqA*alpha; 
                a1 = -2.0f*((A-1.0f) + (A+1.0f)*cs); 
                a2 = (A+1.0f) + (A-1.0f)*cs - 2.0f*sqA*alpha; 
                break; 
            }
            case HIGH_SHELF: { 
                float sqA = std::sqrt(A); 
                b0 = A*((A+1.0f) + (A-1.0f)*cs + 2.0f*sqA*alpha); 
                b1 = -2.0f*A*((A-1.0f) + (A+1.0f)*cs); 
                b2 = A*((A+1.0f) + (A-1.0f)*cs - 2.0f*sqA*alpha); 
                a0 = (A+1.0f) - (A-1.0f)*cs + 2.0f*sqA*alpha; 
                a1 = 2.0f*((A-1.0f) - (A+1.0f)*cs); 
                a2 = (A+1.0f) - (A-1.0f)*cs - 2.0f*sqA*alpha; 
                break; 
            }
            case HPF: 
                b0 = (1.0f + cs)/2.0f; b1 = -(1.0f + cs); b2 = (1.0f + cs)/2.0f; 
                a0 = 1.0f + alpha; a1 = -2.0f * cs; a2 = 1.0f - alpha; 
                break;
            case LPF: 
                b0 = (1.0f - cs)/2.0f; b1 = 1.0f - cs; b2 = (1.0f - cs)/2.0f; 
                a0 = 1.0f + alpha; a1 = -2.0f * cs; a2 = 1.0f - alpha; 
                break;
            case NOTCH: 
                b0 = 1.0f; b1 = -2.0f * cs; b2 = 1.0f; 
                a0 = 1.0f + alpha; a1 = -2.0f * cs; a2 = 1.0f - alpha; 
                break;
            case BAND_PASS:
                // Constant 0dB peak-gain band-pass (RBJ cookbook)
                b0 = Q * alpha; b1 = 0.0f; b2 = -(Q * alpha);
                a0 = 1.0f + alpha; a1 = -2.0f * cs; a2 = 1.0f - alpha;
                break;
            case ALL_PASS:
                // Second-order all-pass phase shifter (RBJ cookbook)
                b0 = 1.0f - alpha; b1 = -2.0f * cs; b2 = 1.0f + alpha;
                a0 = 1.0f + alpha; a1 = -2.0f * cs; a2 = 1.0f - alpha;
                break;
        }
        if (std::abs(a0) > 1e-9f) {
            b0 /= a0; b1 /= a0; b2 /= a0; a1 /= a0; a2 /= a0;
        } else {
            b0 = 1.0f; b1 = 0.0f; b2 = 0.0f; a1 = 0.0f; a2 = 0.0f;
        }
    }

    void process(float& l, float& r) {
        float oL = b0*l + z1L; z1L = b1*l - a1*oL + z2L; z2L = b2*l - a2*oL;
        float oR = b0*r + z1R; z1R = b1*r - a1*oR + z2R; z2R = b2*r - a2*oR;
        if (!std::isfinite(oL) || std::abs(oL) > 1e4f) { oL = 0; z1L = 0; z2L = 0; }
        if (!std::isfinite(oR) || std::abs(oR) > 1e4f) { oR = 0; z1R = 0; z2R = 0; }
        l = oL; r = oR;
    }

    void setCoefficients(float nb0, float nb1, float nb2, float na1, float na2) {
        const bool finite = std::isfinite(nb0) && std::isfinite(nb1) && std::isfinite(nb2) &&
                            std::isfinite(na1) && std::isfinite(na2);
        if (!finite) {
            b0 = 1.0f; b1 = 0.0f; b2 = 0.0f; a1 = 0.0f; a2 = 0.0f;
            clear();
            return;
        }

        b0 = std::clamp(nb0, -8.0f, 8.0f);
        b1 = std::clamp(nb1, -8.0f, 8.0f);
        b2 = std::clamp(nb2, -8.0f, 8.0f);
        a1 = std::clamp(na1, -1.999f, 1.999f);
        a2 = std::clamp(na2, -0.9999f, 0.9999f);
    }
    
    void clear() { z1L = z2L = z1R = z2R = 0.0f; }
};

// Dedicated ladder filter built from cascaded nonlinear 2-pole ladder cells.
// Each stage adds 2 poles so UI slope states map directly to 2/4/6/8 poles.
struct LadderFilter {
    enum Mode {
        LowPass = 0,
        HighPass = 1,
    };

    static constexpr int MAX_CASCADE_STAGES = 4;

    float sampleRate = 48000.0f;
    float cutoffHz = 1000.0f;
    float resonance = 0.4f;
    Mode mode = LowPass;
    int numStages = 1;
    float g = 0.0f;
    float gOverOnePlusG = 0.0f;
    float feedback = 0.0f;

    struct Stage {
        float z1 = 0.0f;
        float z2 = 0.0f;
        float lp1 = 0.0f;
        float lp2 = 0.0f;
    };

    Stage stages[MAX_CASCADE_STAGES];

    void setSampleRate(float sr) {
        sampleRate = (sr > 1000.0f) ? sr : 48000.0f;
        updateCoefficients();
    }

    void setCutoffHz(float hz) {
        cutoffHz = std::clamp(hz, 20.0f, sampleRate * 0.49f);
        updateCoefficients();
    }

    void setResonance(float r) {
        resonance = std::clamp(r, 0.0f, 0.98f);
        updateCoefficients();
    }

    void setMode(Mode m) {
        mode = m;
    }

    void setNumStages(int n) {
        numStages = std::clamp(n, 1, MAX_CASCADE_STAGES);
    }

    void reset() {
        for (int i = 0; i < MAX_CASCADE_STAGES; ++i) {
            stages[i] = Stage{};
        }
    }

    float processSample(float input) {
        float x = sanitize(input);
        for (int i = 0; i < numStages; ++i) {
            x = processLadderStage(x, stages[i]);
        }
        return sanitize(x);
    }

private:
    static float sanitize(float v) {
        if (!std::isfinite(v) || std::abs(v) > 1.0e6f) return 0.0f;
        if (std::abs(v) < 1.0e-18f) return 0.0f;
        return v;
    }

    void updateCoefficients() {
        const float fc = std::clamp(cutoffHz, 20.0f, sampleRate * 0.49f);
        const float wd = (float)M_PI * fc / sampleRate;
        g = std::clamp(std::tan(wd), 0.0005f, 8.0f);
        gOverOnePlusG = g / (1.0f + g);
        feedback = std::clamp(resonance * 1.8f, 0.0f, 1.5f);
    }

    float processOnePoleTpt(float input, float& z) const {
        const float v = (input - z) * gOverOnePlusG;
        const float y = v + z;
        z = y + v;
        return y;
    }

    float processLadderStage(float input, Stage& s) {
        const float driven = sanitize(input - feedback * s.lp2);
        const float nonlinearIn = std::tanh(driven);

        const float lp1 = processOnePoleTpt(nonlinearIn, s.z1);
        const float lp2 = processOnePoleTpt(std::tanh(lp1), s.z2);

        s.lp1 = lp1;
        s.lp2 = lp2;

        const float lp = lp2;
        const float hp = driven - 2.0f * lp1 + lp2;
        if (mode == HighPass) {
            return sanitize(hp);
        }
        return sanitize(lp);
    }
};

// Cascaded biquad filter for multi-pole LPF/HPF (2, 4, 6 poles)
struct BiquadCascade {
    enum Character {
        Butterworth = 0,
        Bessel = 1,
        LinkwitzRiley = 2,
        ChebyshevI = 3,
        ChebyshevII = 4,
        Elliptic = 5,
        OptimumL = 6
    };

    static constexpr int MAX_STAGES = 4;  // Max 4 stages for 8-pole filter
    static constexpr int kDefaultRemapCharacter = Butterworth;
    Biquad stages[MAX_STAGES];
    int numStages = 0;

    static bool isCharacterSupportedForPoles(int poles, int character) {
        if (poles != 2 && poles != 4 && poles != 6 && poles != 8) {
            return false;
        }
        const int ch = std::clamp(character, 0, 6);
        // Current Optimum-L prototype builder is implemented for 2/4/6 poles only.
        if (poles == 8 && ch == (int)OptimumL) {
            return false;
        }
        return true;
    }

    static int resolveSupportedPolesForCharacter(int poles, int character) {
        int resolvedPoles = 2;
        if (poles >= 8) resolvedPoles = 8;
        else if (poles >= 6) resolvedPoles = 6;
        else if (poles >= 4) resolvedPoles = 4;

        const int ch = std::clamp(character, 0, 6);
        // Preserve Optimum-L selection by falling back to nearest supported pole count.
        if (resolvedPoles == 8 && ch == (int)OptimumL) {
            return 6;
        }
        return resolvedPoles;
    }

    static int remapUnsupportedCharacterForPoles(int poles, int character) {
        const int ch = std::clamp(character, 0, 6);
        if (!isCharacterSupportedForPoles(poles, ch)) {
            return kDefaultRemapCharacter;
        }
        return ch;
    }

    struct SectionDesign {
        double b0 = 1.0;
        double b1 = 0.0;
        double b2 = 0.0;
        double a1 = 0.0;
        double a2 = 0.0;
    };

    struct AnalogSection {
        double b2 = 0.0;
        double b1 = 0.0;
        double b0 = 1.0;
        double a2 = 1.0;
        double a1 = 1.0;
        double a0 = 1.0;
    };

    static double sanitizeFinite(double v, double fallback) {
        return std::isfinite(v) ? v : fallback;
    }

    static double sanitizeFiniteClamped(double v, double lo, double hi, double fallback) {
        const double finite = sanitizeFinite(v, fallback);
        return std::clamp(finite, lo, hi);
    }

    // Configure cascade for N-pole LPF or HPF
    // poles: 2, 4, or 6; type: 0 = LPF, 1 = HPF
    void configure(int poles, int type, float freqHz, float Q, float sr) {
        numStages = poles / 2;
        if (numStages > MAX_STAGES) numStages = MAX_STAGES;
        
        // Each biquad in cascade gets same frequency and Q (simple approach)
        Biquad::Type bqType = (type == 0) ? Biquad::LPF : Biquad::HPF;
        for (int i = 0; i < numStages; ++i) {
            stages[i].setParams(bqType, freqHz, 0.0f, Q, sr);
        }
    }

    // Configure by designing per-section textbook-style analog prototypes and mapping to
    // digital biquads with the bilinear transform.
    // PHASE 3: Unified robust policy - all parameters are sanitized to prevent NaN/Inf
    void configure(int poles, int type, float freqHz, float Q, float sr, int character, float rippleDb, float stopbandRsDb = 30.0f) {
        (void)Q;

        const int resolvedPoles = resolveSupportedPolesForCharacter(poles, character);
        numStages = std::clamp(resolvedPoles / 2, 0, MAX_STAGES);
        if (numStages == 0) return;

        // Input parameter safeguarding for robust policy
        const int remappedCharacter = remapUnsupportedCharacterForPoles(resolvedPoles, character);
        const Character ch = (Character)remappedCharacter;
        const bool highPass = (type != 0);
        const double safeSr = sanitizeFinite((double)sr, 44100.0);
        const double clampedSr = (safeSr > 1000.0) ? safeSr : 44100.0;
        const double safeFreq = sanitizeFiniteClamped((double)freqHz, 10.0, (clampedSr * 0.5) * 0.98, 1000.0);
        const double safeRippleDb = sanitizeFiniteClamped((double)rippleDb, 0.1, 6.0, 0.5);
        const double safeStopbandRsDb = sanitizeFiniteClamped((double)stopbandRsDb, 12.0, 80.0, 30.0);

        std::vector<SectionDesign> designed;
        if (!designCharacterSections(resolvedPoles, highPass, safeFreq, clampedSr, ch, safeRippleDb, safeStopbandRsDb, designed)) {
            // Fallback to simple LPF/HPF if designer fails
            const Biquad::Type bqType = highPass ? Biquad::HPF : Biquad::LPF;
            for (int i = 0; i < numStages; ++i) {
                stages[i].setParams(bqType, (float)safeFreq, 0.0f, 0.70710678f, (float)clampedSr);
            }
            return;
        }

        // Apply designed coefficients with finite guards via setCoefficients()
        for (int i = 0; i < numStages; ++i) {
            const SectionDesign& s = designed[(size_t)i];
            stages[i].setCoefficients((float)s.b0, (float)s.b1, (float)s.b2, (float)s.a1, (float)s.a2);
        }
    }

private:
    static AnalogSection allPoleFromPair(const std::complex<double>& p1, const std::complex<double>& p2) {
        AnalogSection s;
        const std::complex<double> sum = p1 + p2;
        const std::complex<double> prod = p1 * p2;
        s.b2 = 0.0;
        s.b1 = 0.0;
        s.b0 = 1.0;
        s.a2 = 1.0;
        s.a1 = -sum.real();
        s.a0 = prod.real();
        return s;
    }

    static AnalogSection finiteZeroPairFromPolePair(const std::complex<double>& p1,
                                                    const std::complex<double>& p2,
                                                    double zeroImag) {
        AnalogSection s = allPoleFromPair(p1, p2);
        const double wz2 = zeroImag * zeroImag;
        s.b2 = 1.0;
        s.b1 = 0.0;
        s.b0 = wz2;
        return s;
    }

    static void makeButterworthPrototypes(int order, std::vector<AnalogSection>& out) {
        out.clear();
        for (int k = 1; k <= order / 2; ++k) {
            const double theta = M_PI * (2.0 * k - 1.0) / (2.0 * order);
            const std::complex<double> p(-std::sin(theta), std::cos(theta));
            out.push_back(allPoleFromPair(p, std::conj(p)));
        }
    }

    static void makeBesselPrototypes(int order, std::vector<AnalogSection>& out) {
        out.clear();
        if (order == 2) {
            const std::complex<double> p(-1.5, 0.8660254037844386);
            out.push_back(allPoleFromPair(p, std::conj(p)));
            return;
        }
        if (order == 4) {
            const std::complex<double> p1(-0.6572111716718829, 0.8301614350048733);
            const std::complex<double> p2(-0.9047587967882449, 0.2709187330038746);
            out.push_back(allPoleFromPair(p1, std::conj(p1)));
            out.push_back(allPoleFromPair(p2, std::conj(p2)));
            return;
        }
        if (order == 8) {
            // Normalized 8th-order Bessel pole pairs (negative real half-plane).
            const std::complex<double> p1(-0.9096831546652910, 1.3700678305514440);
            const std::complex<double> p2(-1.3818772759607720, 1.0562151117842790);
            const std::complex<double> p3(-1.7903442511275760, 0.6271615281159830);
            const std::complex<double> p4(-2.1121553551100620, 0.2010870802244450);
            out.push_back(allPoleFromPair(p1, std::conj(p1)));
            out.push_back(allPoleFromPair(p2, std::conj(p2)));
            out.push_back(allPoleFromPair(p3, std::conj(p3)));
            out.push_back(allPoleFromPair(p4, std::conj(p4)));
            return;
        }

        const std::complex<double> p1(-0.9093906830472272, 1.0812748428191247);
        const std::complex<double> p2(-1.3818580975962760, 0.9714718907505122);
        const std::complex<double> p3(-1.5714904036159718, 0.3208963742223503);
        out.push_back(allPoleFromPair(p1, std::conj(p1)));
        out.push_back(allPoleFromPair(p2, std::conj(p2)));
        out.push_back(allPoleFromPair(p3, std::conj(p3)));
    }

    static void makeLinkwitzRileyPrototypes(int order, std::vector<AnalogSection>& out) {
        out.clear();
        if (order == 2) {
            const std::complex<double> p(-1.0, 0.0);
            out.push_back(allPoleFromPair(p, p));
            return;
        }
        if (order == 4) {
            const std::complex<double> p(-0.7071067811865476, 0.7071067811865476);
            out.push_back(allPoleFromPair(p, std::conj(p)));
            out.push_back(allPoleFromPair(p, std::conj(p)));
            return;
        }
        if (order == 8) {
            // LR8 = two cascaded LR4 sections (4 total biquad sections).
            const std::complex<double> p1(-0.3826834323650898, 0.9238795325112867);
            const std::complex<double> p2(-0.9238795325112867, 0.3826834323650898);
            out.push_back(allPoleFromPair(p1, std::conj(p1)));
            out.push_back(allPoleFromPair(p2, std::conj(p2)));
            out.push_back(allPoleFromPair(p1, std::conj(p1)));
            out.push_back(allPoleFromPair(p2, std::conj(p2)));
            return;
        }

        const std::complex<double> realPole(-1.0, 0.0);
        const std::complex<double> pairPole(-0.5, 0.8660254037844386);
        out.push_back(allPoleFromPair(realPole, realPole));
        out.push_back(allPoleFromPair(pairPole, std::conj(pairPole)));
        out.push_back(allPoleFromPair(pairPole, std::conj(pairPole)));
    }

    static void makeChebyshevIPrototypes(int order, double rpDb, std::vector<AnalogSection>& out) {
        out.clear();
        const double eps = std::sqrt(std::pow(10.0, rpDb / 10.0) - 1.0);
        const double mu = std::asinh(1.0 / std::max(eps, 1e-12)) / (double)order;
        const double sinhMu = std::sinh(mu);
        const double coshMu = std::cosh(mu);

        for (int k = 1; k <= order / 2; ++k) {
            const double theta = M_PI * (2.0 * k - 1.0) / (2.0 * order);
            const std::complex<double> p(-sinhMu * std::sin(theta), coshMu * std::cos(theta));
            out.push_back(allPoleFromPair(p, std::conj(p)));
        }
    }

    static void makeChebyshevIIPrototypes(int order, double rsDb, std::vector<AnalogSection>& out) {
        out.clear();
        const double eps = 1.0 / std::sqrt(std::pow(10.0, rsDb / 10.0) - 1.0);
        const double mu = std::asinh(1.0 / std::max(eps, 1e-12)) / (double)order;
        const double sinhMu = std::sinh(mu);
        const double coshMu = std::cosh(mu);

        for (int k = 1; k <= order / 2; ++k) {
            const double theta = M_PI * (2.0 * k - 1.0) / (2.0 * order);
            const std::complex<double> denom(-sinhMu * std::sin(theta), coshMu * std::cos(theta));
            const std::complex<double> p = 1.0 / denom;
            const double wz = 1.0 / std::max(std::abs(std::cos(theta)), 1e-12);
            out.push_back(finiteZeroPairFromPolePair(p, std::conj(p), wz));
        }
    }

    static bool legendreValueAndDerivative(int n, double x, double& pn, double& dpn) {
        if (n < 0 || !std::isfinite(x) || std::abs(x) >= 1.0) {
            return false;
        }
        if (n == 0) {
            pn = 1.0;
            dpn = 0.0;
            return true;
        }

        double pnm2 = 1.0;
        double pnm1 = x;
        if (n == 1) {
            pn = pnm1;
            dpn = 1.0;
            return true;
        }

        for (int k = 2; k <= n; ++k) {
            const double pk = ((2.0 * (double)k - 1.0) * x * pnm1 - ((double)k - 1.0) * pnm2) / (double)k;
            pnm2 = pnm1;
            pnm1 = pk;
        }

        pn = pnm1;
        const double denom = x * x - 1.0;
        if (std::abs(denom) < 1e-14) {
            return false;
        }
        dpn = ((double)n * (x * pn - pnm2)) / denom;
        return std::isfinite(pn) && std::isfinite(dpn);
    }

    static bool legendreRoots(int n, std::vector<double>& roots) {
        roots.clear();
        if (n <= 0) {
            return false;
        }

        roots.reserve((size_t)n);
        for (int k = 1; k <= n; ++k) {
            double x = std::cos(M_PI * ((double)k - 0.25) / ((double)n + 0.5));
            for (int it = 0; it < 24; ++it) {
                x = std::clamp(x, -0.999999999, 0.999999999);
                double pn = 0.0;
                double dpn = 0.0;
                if (!legendreValueAndDerivative(n, x, pn, dpn) || std::abs(dpn) < 1e-14) {
                    return false;
                }
                const double dx = pn / dpn;
                x -= dx;
                if (!std::isfinite(x)) {
                    return false;
                }
                if (std::abs(dx) < 1e-14) {
                    break;
                }
            }
            roots.push_back(std::clamp(x, -0.999999, 0.999999));
        }

        std::sort(roots.begin(), roots.end());
        return ((int)roots.size() == n);
    }

    static void makeOptimumLPrototypes(int order, double rpDb, std::vector<AnalogSection>& out) {
        out.clear();
        if (order != 2 && order != 4 && order != 6) {
            return;
        }

        const int sections = order / 2;
        std::vector<double> roots;
        if (!legendreRoots(sections, roots)) {
            return;
        }

        // Legendre-root frequency placement with Rp-driven damping produces a
        // monotonic all-pole "Optimum L" style prototype family.
        const double shape = std::clamp((rpDb - 0.1) / 5.9, 0.0, 1.0);
        const double freqExponent = 0.90 + 0.35 * shape;
        const double freqScale = 1.15 - 0.45 * shape;

        out.reserve((size_t)sections);
        for (int i = 0; i < sections; ++i) {
            const double x = std::clamp(roots[(size_t)i], -0.999, 0.999);
            const double ratio = (1.0 - x) / std::max(1.0 + x, 1e-9);
            const double baseOmega = std::sqrt(std::max(ratio, 1e-12));
            const double omega = std::pow(baseOmega, freqExponent) * freqScale;

            const double dampingFactor = 1.10 - 0.35 * shape + 0.12 * (1.0 - x);
            const double sigma = std::max(omega * dampingFactor + 0.08, 1e-4);

            const std::complex<double> p(-sigma, omega);
            out.push_back(allPoleFromPair(p, std::conj(p)));
        }
    }

    static double pow10m1(double x) {
        static constexpr double LN10 = 2.30258509299404568402;
        return std::expm1(LN10 * x);
    }

    static double ellipticK(double m) {
        if (!std::isfinite(m)) {
            return std::numeric_limits<double>::quiet_NaN();
        }
        if (m <= 0.0) {
            return M_PI * 0.5;
        }
        if (m >= 1.0) {
            return std::numeric_limits<double>::infinity();
        }

        double a = 1.0;
        double b = std::sqrt(std::max(1.0 - m, 0.0));
        for (int i = 0; i < 40; ++i) {
            const double an = 0.5 * (a + b);
            const double bn = std::sqrt(std::max(a * b, 0.0));
            if (std::abs(an - bn) <= 1e-15 * std::abs(an)) {
                a = an;
                break;
            }
            a = an;
            b = bn;
        }
        if (!std::isfinite(a) || std::abs(a) < 1e-16) {
            return std::numeric_limits<double>::quiet_NaN();
        }
        return M_PI / (2.0 * a);
    }

    static bool jacobiEllipticSnCnDn(double u, double m, double& sn, double& cn, double& dn) {
        if (!std::isfinite(u) || !std::isfinite(m) || m < 0.0 || m > 1.0) {
            return false;
        }

        if (m < 1e-12) {
            sn = std::sin(u);
            cn = std::cos(u);
            dn = 1.0;
            return true;
        }

        if (1.0 - m < 1e-12) {
            sn = std::tanh(u);
            cn = 1.0 / std::cosh(u);
            dn = cn;
            return std::isfinite(sn) && std::isfinite(cn) && std::isfinite(dn);
        }

        constexpr int MAX_ITER = 16;
        double a[MAX_ITER + 1];
        double c[MAX_ITER + 1];

        a[0] = 1.0;
        double b = std::sqrt(std::max(1.0 - m, 0.0));
        c[0] = std::sqrt(std::max(m, 0.0));
        double twon = 1.0;
        int n = 0;

        for (n = 0; n < MAX_ITER; ++n) {
            const double ai = a[n];
            c[n + 1] = 0.5 * (ai - b);
            a[n + 1] = 0.5 * (ai + b);
            b = std::sqrt(std::max(ai * b, 0.0));
            twon *= 2.0;
            if (std::abs(c[n + 1]) <= 1e-14 * std::abs(a[n + 1])) {
                break;
            }
        }

        double phi = twon * a[n + 1] * u;
        for (int i = n + 1; i > 0; --i) {
            const double denom = a[i];
            if (std::abs(denom) < 1e-16) {
                return false;
            }
            const double t = (c[i] * std::sin(phi)) / denom;
            phi = 0.5 * (std::asin(std::clamp(t, -1.0, 1.0)) + phi);
        }

        sn = std::sin(phi);
        cn = std::cos(phi);
        dn = std::sqrt(std::max(1.0 - m * sn * sn, 0.0));
        return std::isfinite(sn) && std::isfinite(cn) && std::isfinite(dn);
    }

    static double ellipDegNome(int n, double m1) {
        const double K1 = ellipticK(m1);
        const double K1p = ellipticK(std::clamp(1.0 - m1, 0.0, 1.0 - 1e-15));
        if (!std::isfinite(K1) || !std::isfinite(K1p) || K1 <= 0.0) {
            return std::numeric_limits<double>::quiet_NaN();
        }

        const double q1 = std::exp(-M_PI * K1p / K1);
        const double q = std::pow(std::max(q1, 1e-18), 1.0 / (double)n);

        double num = 0.0;
        for (int j = 0; j <= 7; ++j) {
            num += std::pow(q, (double)(j * (j + 1)));
        }

        double den = 1.0;
        for (int j = 1; j <= 8; ++j) {
            den += 2.0 * std::pow(q, (double)(j * j));
        }

        if (!std::isfinite(num) || !std::isfinite(den) || std::abs(den) < 1e-18) {
            return std::numeric_limits<double>::quiet_NaN();
        }

        const double ratio = num / den;
        return 16.0 * q * ratio * ratio * ratio * ratio;
    }

    static std::complex<double> arcJacobiSn(const std::complex<double>& w, double m) {
        auto complement = [](const std::complex<double>& x) {
            return std::sqrt((1.0 - x) * (1.0 + x));
        };

        std::complex<double> k = std::sqrt(std::complex<double>(m, 0.0));
        if (std::abs(k) > 1.0 + 1e-12) {
            return std::complex<double>(std::numeric_limits<double>::quiet_NaN(), 0.0);
        }
        if (std::abs(k - std::complex<double>(1.0, 0.0)) < 1e-12) {
            return std::atanh(w);
        }

        std::vector<std::complex<double>> ks;
        ks.reserve(12);
        ks.push_back(k);
        for (int i = 0; i < 10 && std::abs(ks.back()) > 1e-14; ++i) {
            const std::complex<double> kCur = ks.back();
            const std::complex<double> kp = complement(kCur);
            ks.push_back((1.0 - kp) / (1.0 + kp));
        }

        std::complex<double> K = M_PI * 0.5;
        for (size_t i = 1; i < ks.size(); ++i) {
            K *= (1.0 + ks[i]);
        }

        std::vector<std::complex<double>> wns;
        wns.reserve(ks.size());
        wns.push_back(w);
        for (size_t i = 0; i + 1 < ks.size(); ++i) {
            const std::complex<double> kn = ks[i];
            const std::complex<double> knext = ks[i + 1];
            const std::complex<double> wn = wns.back();
            const std::complex<double> denom = (1.0 + knext) * (1.0 + complement(kn * wn));
            if (std::abs(denom) < 1e-18) {
                return std::complex<double>(std::numeric_limits<double>::quiet_NaN(), 0.0);
            }
            wns.push_back((2.0 * wn) / denom);
        }

        const std::complex<double> u = (2.0 / M_PI) * std::asin(wns.back());
        return K * u;
    }

    static double arcJacobiSc1(double w, double m) {
        const std::complex<double> z = arcJacobiSn(std::complex<double>(0.0, w), m);
        return z.imag();
    }

    static void makeEllipticPrototypes(int order, double rpDb, double rsDb, std::vector<AnalogSection>& out) {
        out.clear();
        if (order <= 0 || (order % 2) != 0) {
            return;
        }

        const double epsSq = std::max(pow10m1(0.1 * rpDb), 1e-16);
        const double eps = std::sqrt(epsSq);
        const double rsTerm = std::max(pow10m1(0.1 * rsDb), epsSq + 1e-12);
        const double ck1Sq = std::clamp(epsSq / rsTerm, 1e-12, 1.0 - 1e-12);

        const double m = std::clamp(ellipDegNome(order, ck1Sq), 1e-10, 1.0 - 1e-10);
        const double capk = ellipticK(m);
        const double K1 = ellipticK(ck1Sq);
        if (!(std::isfinite(capk) && std::isfinite(K1) && capk > 0.0 && K1 > 0.0)) {
            return;
        }

        const double r = arcJacobiSc1(1.0 / std::max(eps, 1e-12), ck1Sq);
        const double v0 = capk * r / ((double)order * K1);

        double sv = 0.0, cv = 0.0, dv = 0.0;
        if (!jacobiEllipticSnCnDn(v0, 1.0 - m, sv, cv, dv)) {
            return;
        }

        const int sections = order / 2;
        const double sqrtM = std::sqrt(m);
        for (int i = 0; i < sections; ++i) {
            const double j = (double)(2 * i + 1);
            const double u = j * capk / (double)order;

            double s = 0.0, c = 0.0, d = 0.0;
            if (!jacobiEllipticSnCnDn(u, m, s, c, d)) {
                out.clear();
                return;
            }

            const double zeroDen = std::max(std::abs(s), 1e-12);
            const double wz = std::abs(1.0 / (sqrtM * zeroDen));

            const std::complex<double> num(c * d * sv * cv, s * dv);
            const double den = 1.0 - (d * sv) * (d * sv);
            if (!std::isfinite(den) || std::abs(den) < 1e-12) {
                out.clear();
                return;
            }

            std::complex<double> p = -num / den;
            if (p.real() >= 0.0) {
                p = std::complex<double>(-std::abs(p.real()) - 1e-12, p.imag());
            }

            out.push_back(finiteZeroPairFromPolePair(p, std::conj(p), wz));
        }
    }

    static SectionDesign bilinearSection(const AnalogSection& asec, double sr, bool highPass, double wc) {
        const double K = 2.0 * sr;

        // Low-pass -> low-pass or low-pass -> high-pass analog frequency transformation.
        double nb2, nb1, nb0;
        double na2, na1, na0;

        if (!highPass) {
            nb2 = asec.b2;
            nb1 = asec.b1 * wc;
            nb0 = asec.b0 * wc * wc;
            na2 = asec.a2;
            na1 = asec.a1 * wc;
            na0 = asec.a0 * wc * wc;
        } else {
            nb2 = asec.b0;
            nb1 = asec.b1 * wc;
            nb0 = asec.b2 * wc * wc;
            na2 = asec.a0;
            na1 = asec.a1 * wc;
            na0 = asec.a2 * wc * wc;
        }

        const double d0 = na2 * K * K + na1 * K + na0;
        const double d1 = 2.0 * (na0 - na2 * K * K);
        const double d2 = na2 * K * K - na1 * K + na0;

        SectionDesign out;
        if (!std::isfinite(d0) || std::abs(d0) < 1e-18) {
            out.b0 = 1.0; out.b1 = 0.0; out.b2 = 0.0; out.a1 = 0.0; out.a2 = 0.0;
            return out;
        }

        const double n0 = nb2 * K * K + nb1 * K + nb0;
        const double n1 = 2.0 * (nb0 - nb2 * K * K);
        const double n2 = nb2 * K * K - nb1 * K + nb0;

        out.b0 = n0 / d0;
        out.b1 = n1 / d0;
        out.b2 = n2 / d0;
        out.a1 = d1 / d0;
        out.a2 = d2 / d0;

        if (!(std::isfinite(out.b0) && std::isfinite(out.b1) && std::isfinite(out.b2) &&
              std::isfinite(out.a1) && std::isfinite(out.a2))) {
            out.b0 = 1.0; out.b1 = 0.0; out.b2 = 0.0; out.a1 = 0.0; out.a2 = 0.0;
        }
        return out;
    }

    static std::complex<double> evalSectionAt(const SectionDesign& s, const std::complex<double>& zInv) {
        const std::complex<double> zInv2 = zInv * zInv;
        const std::complex<double> num = s.b0 + s.b1 * zInv + s.b2 * zInv2;
        const std::complex<double> den = 1.0 + s.a1 * zInv + s.a2 * zInv2;
        if (std::abs(den) < 1e-18) return 0.0;
        return num / den;
    }

    static bool sectionDesignIsValid(const SectionDesign& s) {
        if (!(std::isfinite(s.b0) && std::isfinite(s.b1) && std::isfinite(s.b2) &&
              std::isfinite(s.a1) && std::isfinite(s.a2))) {
            return false;
        }

        // Keep denominator away from singularities at DC/Nyquist.
        const double denDc = 1.0 + s.a1 + s.a2;
        const double denNy = 1.0 - s.a1 + s.a2;
        if (!std::isfinite(denDc) || !std::isfinite(denNy) ||
            std::abs(denDc) < 1e-8 || std::abs(denNy) < 1e-8) {
            return false;
        }

        // Reject obviously unstable/degenerate denominator terms.
        return std::abs(s.a1) < 2.0 && std::abs(s.a2) < 1.0;
    }

    static bool designCharacterSections(int poles,
                                        bool highPass,
                                        double freqHz,
                                        double sr,
                                        Character character,
                                        double rippleDb,
                                        double stopbandRsDb,
                                        std::vector<SectionDesign>& out) {
        out.clear();
        if ((poles != 2 && poles != 4 && poles != 6 && poles != 8) || sr <= 1000.0 || !std::isfinite(sr)) {
            return false;
        }

        std::vector<AnalogSection> proto;
        switch (character) {
            case Butterworth:   makeButterworthPrototypes(poles, proto); break;
            case Bessel:        makeBesselPrototypes(poles, proto); break;
            case LinkwitzRiley: makeLinkwitzRileyPrototypes(poles, proto); break;
            case ChebyshevI:    makeChebyshevIPrototypes(poles, rippleDb, proto); break;
            case ChebyshevII:   makeChebyshevIIPrototypes(poles, stopbandRsDb, proto); break;
            case Elliptic:      makeEllipticPrototypes(poles, rippleDb, stopbandRsDb, proto); break;
            case OptimumL:      makeOptimumLPrototypes(poles, rippleDb, proto); break;
            default:            makeButterworthPrototypes(poles, proto); break;
        }

        if ((int)proto.size() != poles / 2) {
            return false;
        }

        const double wc = 2.0 * sr * std::tan(M_PI * std::clamp(freqHz, 10.0, sr * 0.49) / sr);
        if (!std::isfinite(wc) || wc <= 0.0) {
            return false;
        }

        out.reserve((size_t)(poles / 2));
        for (const AnalogSection& asec : proto) {
            const SectionDesign section = bilinearSection(asec, sr, highPass, wc);
            if (!sectionDesignIsValid(section)) {
                out.clear();
                return false;
            }
            out.push_back(section);
        }

        // Normalize passband gain: LPF at DC (z=1), HPF near Nyquist (z=-1).
        const std::complex<double> zInv = highPass ? std::complex<double>(-1.0, 0.0)
                                                   : std::complex<double>(1.0, 0.0);
        std::complex<double> hTotal(1.0, 0.0);
        for (const SectionDesign& s : out) {
            hTotal *= evalSectionAt(s, zInv);
        }

        const double mag = std::abs(hTotal);
        if (!(std::isfinite(mag) && mag > 1e-9) || out.empty()) {
            out.clear();
            return false;
        }

        {
            const double g = 1.0 / mag;
            out[0].b0 *= g;
            out[0].b1 *= g;
            out[0].b2 *= g;
        }

        for (const SectionDesign& section : out) {
            if (!sectionDesignIsValid(section)) {
                out.clear();
                return false;
            }
        }

        return true;
    }

public:

    // Normalized analog-prototype section Qs for a character. The plugin
    // scales these by the user Q so Character and Q stay independent.
    static int prototypeSectionQs(int poles, int character, float rippleDb, float* qs) {
        std::vector<AnalogSection> proto;
        const int ch = std::clamp(character, 0, 6);
        switch (ch) {
            case Bessel:        makeBesselPrototypes(poles, proto); break;
            case LinkwitzRiley: makeLinkwitzRileyPrototypes(poles, proto); break;
            case ChebyshevI:    makeChebyshevIPrototypes(poles, std::max(0.1f, rippleDb), proto); break;
            case ChebyshevII:   makeChebyshevIIPrototypes(poles, 30.0f, proto); break;
            case Elliptic:      makeEllipticPrototypes(poles, std::max(0.1f, rippleDb), 30.0f, proto); break;
            case OptimumL:      makeOptimumLPrototypes(poles, std::max(0.1f, rippleDb), proto); break;
            case Butterworth:
            default:            makeButterworthPrototypes(poles, proto); break;
        }
        const int n = std::min((int)proto.size(), MAX_STAGES);
        for (int i = 0; i < n; ++i) {
            const double a1 = proto[(size_t)i].a1;
            const double a0 = proto[(size_t)i].a0;
            qs[i] = (std::isfinite(a1) && a1 > 1e-9)
                        ? (float)(std::sqrt(std::max(0.0, a0)) / a1)
                        : 0.70710678f;
        }
        return n;
    }

    void process(float& l, float& r) {
        if (!std::isfinite(l) || !std::isfinite(r)) {
            l = 0.0f;
            r = 0.0f;
            clear();
            return;
        }

        auto flushTiny = [](float& v) {
            if (std::abs(v) < 1e-30f) v = 0.0f;
        };

        for (int i = 0; i < numStages; ++i) {
            stages[i].process(l, r);
            if (!std::isfinite(l) || !std::isfinite(r)) {
                l = 0.0f;
                r = 0.0f;
                clear();
                return;
            }
            flushTiny(l);
            flushTiny(r);
            flushTiny(stages[i].z1L);
            flushTiny(stages[i].z2L);
            flushTiny(stages[i].z1R);
            flushTiny(stages[i].z2R);
        }
    }

    void clear() {
        for (int i = 0; i < MAX_STAGES; ++i) {
            stages[i].clear();
        }
    }
};

struct OnePoleFilter {
    float state = 0.0f;
    float alpha = 0.5f;
    void setCutoff(float normalized) { alpha = std::clamp(normalized, 0.001f, 1.0f); }
    void setCutoffHz(float freqHz, float sr) { alpha = std::clamp(2.0f * (float)M_PI * freqHz / sr, 0.001f, 1.0f); }
    float processLP(float in) {
        state += alpha * (in - state);
        if (std::abs(state) < 1e-18f) state = 0.0f;
        return state;
    }
    float processHP(float in) {
        return in - processLP(in);
    }
    void clear() { state = 0.0f; }
};

class DCBlock {
public:
    float process(float in) {
        float out = in - lastIn + 0.995f * lastOut;
        if (std::abs(out) < 1e-15f) out = 0.0f;
        lastIn = in; lastOut = out;
        return out;
    }
    void clear() { lastIn = lastOut = 0.0f; }
private:
    float lastIn = 0, lastOut = 0;
};

class FractionalDelayLine {
public:
    enum DelayMode { LINEAR = 0, LAGRANGE = 1, ALLPASS = 2 };
    static constexpr int kMaxDelay = 48000 + 16;
    FractionalDelayLine() {
        std::memset(buffer, 0, sizeof(buffer));
        writeIndex = 0;
        mode = LINEAR;
        apPrevIn = 0.0f;
        apPrevOut = 0.0f;
    }
    void setMode(DelayMode m) { mode = m; }
    void setMode(int m) { mode = (DelayMode)std::clamp(m, 0, 2); }
    void process(float sample) {
        buffer[writeIndex] = sample;
        writeIndex = (writeIndex + 1) % kMaxDelay;
    }
    float getDelayed(float delaySamples) {
        switch (mode) {
            case LAGRANGE: return getDelayedLagrange(delaySamples);
            case ALLPASS:  return getDelayedAllpass(delaySamples);
            default:       return getDelayedLinear(delaySamples);
        }
    }
    void clear() {
        std::memset(buffer, 0, sizeof(buffer));
        apPrevIn = apPrevOut = 0.0f;
    }
private:
    float buffer[kMaxDelay];
    int writeIndex;
    DelayMode mode;
    float apPrevIn, apPrevOut;

    float getDelayedLinear(float delaySamples) {
        float readIndex = (float)writeIndex - delaySamples - 1.0f;
        while (readIndex < 0) readIndex += (float)kMaxDelay;
        int i1 = (int)readIndex % kMaxDelay;
        int i2 = (i1 + 1) % kMaxDelay;
        float frac = readIndex - std::floor(readIndex);
        return buffer[i1] * (1.0f - frac) + buffer[i2] * frac;
    }

    // 4-point 3rd-order Lagrange interpolation (maximally flat group delay FIR)
    float getDelayedLagrange(float delaySamples) {
        float readIndex = (float)writeIndex - delaySamples - 1.0f;
        while (readIndex < 0) readIndex += (float)kMaxDelay;
        int i0 = (int)readIndex;
        float d = readIndex - std::floor(readIndex);
        int im1 = (i0 - 1 + kMaxDelay) % kMaxDelay;
        i0 = i0 % kMaxDelay;
        int i1  = (i0 + 1) % kMaxDelay;
        int i2  = (i0 + 2) % kMaxDelay;
        float ym1 = buffer[im1], y0 = buffer[i0], y1 = buffer[i1], y2 = buffer[i2];
        // Lagrange basis polynomials for knots at -1, 0, 1, 2 evaluated at d
        float h0 = -d * (d - 1.0f) * (d - 2.0f) * (1.0f / 6.0f);
        float h1 =  (d + 1.0f) * (d - 1.0f) * (d - 2.0f) * 0.5f;
        float h2 = -(d + 1.0f) * d * (d - 2.0f) * 0.5f;
        float h3 =  (d + 1.0f) * d * (d - 1.0f) * (1.0f / 6.0f);
        return h0 * ym1 + h1 * y0 + h2 * y1 + h3 * y2;
    }

    // 1st-order Thiran allpass fractional delay (flat magnitude, non-linear phase)
    float getDelayedAllpass(float delaySamples) {
        int intDelay = (int)delaySamples;
        float frac = delaySamples - (float)intDelay;
        // Keep fractional part in [0.5, 1.5] for stable Thiran coefficients
        if (frac < 0.5f) { frac += 1.0f; intDelay -= 1; }
        if (intDelay < 0) intDelay = 0;
        int readIdx = writeIndex - intDelay - 1;
        while (readIdx < 0) readIdx += kMaxDelay;
        readIdx = readIdx % kMaxDelay;
        float x = buffer[readIdx];
        // Thiran coefficient: a1 = (1-D)/(1+D)
        float a1 = (1.0f - frac) / (1.0f + frac);
        float y = a1 * x + apPrevIn - a1 * apPrevOut;
        apPrevIn = x;
        apPrevOut = y;
        return y;
    }
};

class LFO {
public:
    // TRIANGLE/SAW appended at the end: existing member values unchanged
    // (DispersionStage casts int -> Shape and must stay compatible).
    enum Shape { SINE, RANDOM, STEPPED_SINE, SQUARE, TRIANGLE, SAW };
    LFO(float sampleRate = 48000.0f)
        : sampleRate(sampleRate), phase(0.0f), rate(1.0f), stepMix(1.0f), shape(RANDOM),
          gen(1337), dist(-1.0f, 1.0f) { targetRandom = dist(gen); lastRandom = dist(gen); }
    void setSampleRate(float sr) { sampleRate = sr; }
    void setRate(float r) { rate = r; }
    float getRate() const { return rate; }
    void setShape(Shape s) { shape = s; }
    void setStepMix(float m) { stepMix = m; }
    void setPhase(float p) { phase = std::fmod(std::max(0.0f, p), 1.0f); }
    float getPhase() const { return phase; }
    // Shape value at normalized phase p (0..1), no phase advance.
    float valueAt(float p) const {
        float out = 0.0f;
        if (shape == SINE) out = std::sin(p * 6.2831853f);
        else if (shape == RANDOM) out = lastRandom + (targetRandom - lastRandom) * p;
        else if (shape == STEPPED_SINE) {
            float stepped = std::sin(std::floor(p * 12.0f) / 12.0f * 6.2831853f);
            out = stepped * stepMix + std::sin(p * 6.2831853f) * (1.0f - stepMix);
        }
        else if (shape == TRIANGLE) out = 1.0f - 4.0f * std::abs(p - 0.5f);
        else if (shape == SAW) out = 2.0f * p - 1.0f;
        else out = (p < 0.5f) ? 1.0f : -1.0f;
        return out;
    }
    float processWithRate(float effectiveRate) {
        // 0.25 increment clamp: audio-rate up to ~12 kHz @ 48 kHz.
        float increment = std::clamp(effectiveRate / sampleRate, 0.0f, 0.25f);
        phase += increment;
        if (phase >= 1.0f) {
            phase -= 1.0f;
            lastRandom = targetRandom;
            targetRandom = dist(gen);
        }
        return valueAt(phase);
    }
private:
    float sampleRate, phase, rate, stepMix, lastRandom, targetRandom;
    Shape shape;
    std::mt19937 gen;
    std::uniform_real_distribution<float> dist;
};

} // namespace DSPUtils

#endif
