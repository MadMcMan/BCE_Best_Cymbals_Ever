#include "SurgeFx.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#include "sst/effects/ConcreteConfig.h"
#include "sst/effects/Bonsai.h"
#include "sst/effects/Delay.h"
#include "sst/effects/Flanger.h"
#include "sst/effects/FloatyDelay.h"
#include "sst/effects/Phaser.h"
#include "sst/effects/Reverb1.h"
#include "sst/effects/Reverb2.h"
#include "sst/effects/RotarySpeaker.h"
#include "sst/effects/TreeMonster.h"

namespace CymbalsSurgeFx {
namespace {

using Config = sst::effects::core::ConcreteConfig;
using PMD = sst::basic_blocks::params::ParamMetaData;

struct ParamInfo {
    std::string name;
    std::string unit;
    float minVal = 0.f;
    float maxVal = 1.f;
    float defNatural = 0.f;
    float defNorm = 0.5f;
    bool usable = false;
};

struct AlgoInfo {
    const char* name;
    const char* category;
    std::vector<ParamInfo> params;
};

template <typename E> struct Builder {
    static AlgoInfo build(const char* name, const char* cat) {
        AlgoInfo info;
        info.name = name;
        info.category = cat;
        Config::GS gs(48000.0);
        Config::ES es;
        float* vp = nullptr;
        Config::BC bc(0, 0, &vp);
        // Effects carry multi-megabyte inline delay/reverb buffers; the probe
        // must live on the heap or the first metadata query overflows the
        // stack before any audio is processed.
        auto probe = std::make_unique<E>(&gs, &es, &vp);
        const int n = E::numParams;
        for (int p = 0; p < n; ++p) {
            const PMD md = probe->paramAt(p);
            ParamInfo pi;
            pi.name = md.name.empty() ? ("P" + std::to_string(p + 1)) : md.name;
            pi.unit = md.unit;
            pi.minVal = md.minVal;
            pi.maxVal = md.maxVal;
            pi.defNatural = md.defaultVal;
            // Deactivated / type-NONE params have min==max==0; sst asserts on
            // converting them. Such params stay at their stored natural 0.
            pi.usable = (md.type != PMD::NONE) &&
                        (md.type == PMD::BOOL || md.maxVal > md.minVal);
            if (pi.usable) {
                pi.defNorm = (md.type == PMD::BOOL)
                                 ? (md.defaultVal > 0.5f ? 1.0f : 0.0f)
                                 : std::clamp((md.defaultVal - md.minVal) / (md.maxVal - md.minVal),
                                              0.0f, 1.0f);
                pi.defNatural = md.normalized01ToNatural(pi.defNorm);
            } else {
                pi.defNorm = 0.0f;
                pi.defNatural = 0.0f;
            }
            info.params.push_back(std::move(pi));
        }
        return info;
    }
};

AlgoInfo& algoInfo(int index) {
    static std::vector<AlgoInfo>* infos = nullptr;
    if (infos != nullptr) return (*infos)[(size_t)index];

    // Build lazily. A temporary (uninitialized) effect supplies the metadata
    // only; DSP instances are created per slot.
    infos = new std::vector<AlgoInfo>();
    infos->push_back(Builder<sst::effects::delay::Delay<Config>>::build("Dual Delay", "Delay"));
    infos->push_back(Builder<sst::effects::flanger::Flanger<Config>>::build("Flanger", "Modulation"));
    infos->push_back(Builder<sst::effects::floatydelay::FloatyDelay<Config>>::build("Floaty Delay", "Delay"));
    infos->push_back(Builder<sst::effects::phaser::Phaser<Config>>::build("Phaser", "Modulation"));
    infos->push_back(Builder<sst::effects::reverb1::Reverb1<Config>>::build("Reverb 1", "Reverb"));
    infos->push_back(Builder<sst::effects::reverb2::Reverb2<Config>>::build("Reverb 2", "Reverb"));
    infos->push_back(Builder<sst::effects::bonsai::Bonsai<Config>>::build("Bonsai", "Saturation"));
    infos->push_back(Builder<sst::effects::rotaryspeaker::RotarySpeaker<Config>>::build("Rotary Speaker", "Modulation"));
    infos->push_back(Builder<sst::effects::treemonster::TreeMonster<Config>>::build("Treemonster", "Pitch"));
    return (*infos)[(size_t)index];
}

template <typename E> class Impl final : public Instance {
  public:
    explicit Impl(double sr) : gs(sr) {
        effect = std::make_unique<E>(&gs, &es, &vp);
        // The effect itself IS the ConcreteConfig BaseClass; friend code reads
        // effect->paramStorage, not a separate BC object.
        vp = effect->paramStorage;
        std::memset(effect->paramStorage, 0, sizeof(effect->paramStorage));
        for (int p = 0; p < E::numParams; ++p) meta.push_back(effect->paramAt(p));
        usable.assign(meta.size(), false);
        for (size_t p = 0; p < meta.size(); ++p) {
            const auto& m = meta[p];
            usable[p] = (m.type != PMD::NONE) && (m.type == PMD::BOOL || m.maxVal > m.minVal);
        }
        reset();
    }

    void setSampleRate(double sr) override {
        gs.sampleRate = sr;
        effect->onSampleRateChanged();
    }

    void setParamNormalized(int param, float normalized) override {
        if (param < 0 || param >= (int)meta.size()) return;
        if (!usable[(size_t)param]) return;
        effect->paramStorage[param] = meta[(size_t)param].normalized01ToNatural(
            std::clamp(normalized, 0.0f, 1.0f));
    }

    void reset() override {
        for (int p = 0; p < (int)meta.size(); ++p)
            effect->paramStorage[p] = usable[(size_t)p] ? meta[(size_t)p].defaultVal : 0.0f;
        effect->initialize();
    }

    void processSample(float& left, float& right,
                       float& dryLeft, float& dryRight) override {
        const int i = pos;
        dryLeft = prevDryL[i];
        dryRight = prevDryR[i];
        const float inL = left;
        const float inR = right;
        left = prevOutL[i];
        right = prevOutR[i];
        curInL[i] = inL;
        curInR[i] = inR;
        if (++pos == Config::blockSize) {
            alignas(16) float bl[Config::blockSize];
            alignas(16) float br[Config::blockSize];
            for (int k = 0; k < Config::blockSize; ++k) { bl[k] = curInL[k]; br[k] = curInR[k]; }
            effect->processBlock(bl, br);
            for (int k = 0; k < Config::blockSize; ++k) {
                prevDryL[k] = curInL[k];
                prevDryR[k] = curInR[k];
                prevOutL[k] = bl[k];
                prevOutR[k] = br[k];
            }
            pos = 0;
        }
    }

  private:
    Config::GS gs;
    Config::ES es;
    float dummy[Config::BC::maxParamCount] = {};
    float* vp = dummy;
    std::unique_ptr<E> effect;
    std::vector<PMD> meta;
    std::vector<bool> usable;
    int pos = 0;
    float curInL[Config::blockSize] = {};
    float curInR[Config::blockSize] = {};
    float prevDryL[Config::blockSize] = {};
    float prevDryR[Config::blockSize] = {};
    float prevOutL[Config::blockSize] = {};
    float prevOutR[Config::blockSize] = {};
};

} // namespace

int algorithmCount() {
    algoInfo(0);
    // Force construction of the registry once, then report its size via the
    // sentinel: algorithms are appended in a fixed order.
    static const int count = 9;
    return count;
}

int algorithmId(int index) { return kBaseType + std::clamp(index, 0, algorithmCount() - 1); }

const char* algorithmName(int index) {
    return algoInfo(std::clamp(index, 0, algorithmCount() - 1)).name;
}

const char* algorithmCategory(int index) {
    return algoInfo(std::clamp(index, 0, algorithmCount() - 1)).category;
}

int parameterCount(int algoIndex) {
    return (int)algoInfo(std::clamp(algoIndex, 0, algorithmCount() - 1)).params.size();
}

const char* parameterName(int algoIndex, int param) {
    const auto& params = algoInfo(std::clamp(algoIndex, 0, algorithmCount() - 1)).params;
    if (param < 0 || param >= (int)params.size()) return "";
    return params[(size_t)param].name.c_str();
}

const char* parameterUnit(int algoIndex, int param) {
    const auto& params = algoInfo(std::clamp(algoIndex, 0, algorithmCount() - 1)).params;
    if (param < 0 || param >= (int)params.size()) return "";
    return params[(size_t)param].unit.c_str();
}

float parameterDefaultNormalized(int algoIndex, int param) {
    const auto& params = algoInfo(std::clamp(algoIndex, 0, algorithmCount() - 1)).params;
    if (param < 0 || param >= (int)params.size()) return 0.5f;
    return params[(size_t)param].defNorm;
}

std::unique_ptr<Instance> createInstance(int algoIndex, double sampleRate) {
    switch (std::clamp(algoIndex, 0, algorithmCount() - 1)) {
    case 0: return std::make_unique<Impl<sst::effects::delay::Delay<Config>>>(sampleRate);
    case 1: return std::make_unique<Impl<sst::effects::flanger::Flanger<Config>>>(sampleRate);
    case 2: return std::make_unique<Impl<sst::effects::floatydelay::FloatyDelay<Config>>>(sampleRate);
    case 3: return std::make_unique<Impl<sst::effects::phaser::Phaser<Config>>>(sampleRate);
    case 4: return std::make_unique<Impl<sst::effects::reverb1::Reverb1<Config>>>(sampleRate);
    case 5: return std::make_unique<Impl<sst::effects::reverb2::Reverb2<Config>>>(sampleRate);
    case 6: return std::make_unique<Impl<sst::effects::bonsai::Bonsai<Config>>>(sampleRate);
    case 7: return std::make_unique<Impl<sst::effects::rotaryspeaker::RotarySpeaker<Config>>>(sampleRate);
    case 8: return std::make_unique<Impl<sst::effects::treemonster::TreeMonster<Config>>>(sampleRate);
    default: return nullptr;
    }
}

} // namespace CymbalsSurgeFx
