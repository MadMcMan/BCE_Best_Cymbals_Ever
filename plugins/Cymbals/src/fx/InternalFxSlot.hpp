#pragma once

#include <memory>
#include "InternalFxMeta.hpp"
#include "hump/CrusherEngine.h"

class UnifiedFilter;
class CombFilterBank;

// One FX-slot instance for a single project-owned filter model/topology. The
// model is chosen by the slot type id (one picker entry per model), so there
// is no model selector parameter here.
class CymbalsInternalFxSlot
{
public:
    explicit CymbalsInternalFxSlot(int typeId, float sampleRate = 48000.0f);
    ~CymbalsInternalFxSlot();

    CymbalsInternalFxSlot(const CymbalsInternalFxSlot&) = delete;
    CymbalsInternalFxSlot& operator=(const CymbalsInternalFxSlot&) = delete;

    void setSampleRate(float sampleRate);
    void reset();
    void process(float& outL, float& outR, const float* params13);

    int typeId() const { return m_typeId; }

private:
    void configure(const float* p);
    void configureFilterbank(const float* p);
    void configureHump(const float* p);

    int m_typeId = -1;
    int m_entryIndex = -1;
    int m_eqModel = -1;
    int m_bankType = -1;
    float m_sampleRate = 48000.0f;
    std::unique_ptr<UnifiedFilter> m_filter;
    std::unique_ptr<CombFilterBank> m_bank;
    std::unique_ptr<hump::CrusherEngine> m_hump;
    float m_cached[13] = {};
    bool m_cacheValid = false;
};
