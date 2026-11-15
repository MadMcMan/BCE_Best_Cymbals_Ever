#pragma once

#include <memory>

class VendoredFxSlot
{
public:
    explicit VendoredFxSlot(int typeId, float sampleRate = 48000.0f);
    ~VendoredFxSlot();

    VendoredFxSlot(const VendoredFxSlot&) = delete;
    VendoredFxSlot& operator=(const VendoredFxSlot&) = delete;

    void setSampleRate(float sampleRate);
    void reset();
    void process(float& outL, float& outR, const float* params13);

    int typeId() const { return m_typeId; }

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
    int m_typeId = -1;
    float m_sampleRate = 48000.0f;
};
