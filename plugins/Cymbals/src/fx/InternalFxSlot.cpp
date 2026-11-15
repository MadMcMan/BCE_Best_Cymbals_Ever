#include "InternalFxSlot.hpp"
#include "../UnifiedFilter.hpp"
#include "../CombFilterBank.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>

CymbalsInternalFxSlot::CymbalsInternalFxSlot(int typeId, float sampleRate)
    : m_typeId(typeId), m_entryIndex(CymbalsInternalFx::indexOf(typeId)),
      m_sampleRate(sampleRate)
{
    if (CymbalsInternalFx::isEQIndex(m_entryIndex)) {
        m_eqModel = CymbalsInternalFx::eqModelForIndex(m_entryIndex);
        m_filter = std::make_unique<UnifiedFilter>();
        m_filter->setSampleRate(sampleRate);
    } else if (CymbalsInternalFx::isHumpIndex(m_entryIndex)) {
        m_hump = std::make_unique<hump::CrusherEngine>();
        m_hump->prepare(sampleRate);
    } else if (CymbalsInternalFx::isInternalId(typeId)) {
        m_bankType = m_entryIndex - CymbalsInternalFx::kEQCount;
        m_bank = std::make_unique<CombFilterBank>(sampleRate);
        for (int i = 0; i < 40; ++i)
            m_bank->setStageEnabled(i, i < 4);
        for (int i = 0; i < 4; ++i)
            m_bank->setFilterType(i, m_bankType);
        m_bank->setGroupGain(0, 1.0f);
    }
    for (int i = 0; i < 13; ++i)
        m_cached[i] = CymbalsInternalFx::defaultNormalized(m_entryIndex, i);
    configure(m_cached);
    m_cacheValid = true;
}

CymbalsInternalFxSlot::~CymbalsInternalFxSlot() = default;

void CymbalsInternalFxSlot::setSampleRate(float sampleRate) {
    m_sampleRate = std::max(1.0f, sampleRate);
    if (m_filter) m_filter->setSampleRate(m_sampleRate);
    if (m_bank) m_bank->setSampleRate(m_sampleRate);
    if (m_hump) m_hump->prepare(m_sampleRate);
}

void CymbalsInternalFxSlot::reset() {
    if (m_filter) m_filter->reset();
    if (m_bank) m_bank->reset();
    if (m_hump) m_hump->reset();
}

void CymbalsInternalFxSlot::configure(const float* p) {
    if (m_hump) {
        configureHump(p);
        return;
    }
    if (m_eqModel >= 0) {
        if (!m_filter) m_filter = std::make_unique<UnifiedFilter>();
        m_filter->setModel(m_eqModel);
        const uint32_t mask = UnifiedFilter::supportedTypesMask(m_eqModel);
        int type = (int)std::lround(std::clamp(p[0], 0.0f, 1.0f) * 7.0f);
        if (!(mask & (1u << type))) {
            type = 0;
            while (type < 7 && !(mask & (1u << type))) ++type;
        }
        m_filter->setType(type);
        m_filter->setSlope(UnifiedFilter::slopeIsRelevant(m_eqModel)
                               ? (int)std::lround(std::clamp(p[1], 0.0f, 1.0f) * 3.0f)
                               : 0);
        m_filter->setCutoffHz(20.0f * std::pow(1000.0f, std::clamp(p[2], 0.0f, 1.0f)));
        m_filter->setResonance(0.1f + std::clamp(p[3], 0.0f, 1.0f) * 9.9f);
        m_filter->setGainDb(-24.0f + std::clamp(p[4], 0.0f, 1.0f) * 48.0f);
        m_filter->setSaturation(std::clamp(p[5], 0.0f, 1.0f));
        m_filter->setSampleRate(m_sampleRate);
        m_filter->prepare();
        return;
    }
    configureFilterbank(p);
}

void CymbalsInternalFxSlot::configureHump(const float* p) {
    hump::EngineParams ep;
    ep.masterOn = true;
    ep.distortionOn = true;
    ep.mech = std::clamp(p[0], 0.0f, 1.0f);
    ep.tube = std::clamp(p[1], 0.0f, 1.0f);
    ep.filterOn = p[2] > 0.5f;
    ep.cutoff = std::clamp(p[3], 0.0f, 1.0f);
    ep.resonance = std::clamp(p[4], 0.0f, 1.0f);
    ep.compressorOn = p[5] > 0.5f;
    ep.compAmount = std::clamp(p[6], 0.0f, 1.0f);
    ep.phat = p[7] > 0.5f;
    ep.mix = 1.0f;             // slot WET is the dry/wet control
    ep.volume = std::clamp(p[8], 0.0f, 1.0f);
    m_hump->setParameters(ep);
}

void CymbalsInternalFxSlot::configureFilterbank(const float* p) {
    if (!m_bank || m_bankType < 0) return;
    const float freq = 20.0f * std::pow(400.0f, std::clamp(p[0], 0.0f, 1.0f));
    const float feedback = std::clamp(p[1], 0.0f, 1.0f) * 0.998f;
    const float lpf = 200.0f * std::pow(100.0f, std::clamp(p[2], 0.0f, 1.0f));
    const float hpf = 20.0f * std::pow(100.0f, std::clamp(p[3], 0.0f, 1.0f));
    const float sat = std::clamp(p[4], 0.0f, 1.0f);
    const int satType = (int)std::lround(std::clamp(p[5], 0.0f, 1.0f) * 7.0f);
    const float partials = std::clamp(p[6], 0.0f, 1.0f);
    const float modeAmount = std::clamp(p[7], 0.0f, 1.0f);
    const float modeRate = std::clamp(p[8], 0.0f, 1.0f);
    for (int i = 0; i < 4; ++i) {
        m_bank->setFilterType(i, m_bankType);
        m_bank->setFrequency(i, freq);
        m_bank->setFeedback(i, feedback);
        m_bank->setLPF(i, lpf);
        m_bank->setHPF(i, hpf);
        m_bank->setSaturatorType(i, satType);
        m_bank->setSaturatorAmount(i, sat);
        m_bank->setPartCount(i, partials);
        m_bank->setModeAmount(i, modeAmount);
        m_bank->setModeRate(i, modeRate);
    }
}

void CymbalsInternalFxSlot::process(float& outL, float& outR, const float* params13) {
    if (!m_cacheValid || std::memcmp(m_cached, params13, sizeof(m_cached)) != 0) {
        std::memcpy(m_cached, params13, sizeof(m_cached));
        configure(m_cached);
        m_cacheValid = true;
    }
    if (m_filter) {
        outL = m_filter->processSampleL(outL);
        outR = m_filter->processSampleR(outR);
        return;
    }
    if (m_hump) {
        m_hump->process(&outL, &outR, 1);
        return;
    }
    if (m_bank)
        m_bank->process(outL, outR, outL, outR);
}
