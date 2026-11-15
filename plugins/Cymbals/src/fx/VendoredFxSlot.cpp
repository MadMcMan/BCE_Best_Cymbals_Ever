#include "VendoredFxSlot.hpp"
#include "VendoredFxMeta.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <memory>

// CloudSeedCore requires the Visual Studio project macros and uses the
// MSVC-only strcpy_s in an inline formatting helper we do not call.
#ifndef BUFFER_SIZE
#define BUFFER_SIZE 1024
#endif
#ifndef MAX_STR_SIZE
#define MAX_STR_SIZE 32
#endif
#define strcpy_s(dst, n, src) std::snprintf((dst), (n), "%s", (src))
#include <cstdio>
#include "DSP/ReverbController.h"
#include "Parameters.h"
#include "Programs.h"

// Signalsmith Basics (chorus) + DSP support layer
#include "signalsmith-basics/chorus.h"

// Signalsmith Stretch (+ Signalsmith Linear)
#include "signalsmith-stretch/signalsmith-stretch.h"

// Soundpipe phaser (minimal vendored core)
extern "C" {
#include "soundpipe.h"
}

struct VendoredFxSlot::Impl
{
    int type = -1;
    float sr = 48000.0f;

    // CloudSeed
    std::unique_ptr<Cloudseed::ReverbController> cloud;
    float cloudCached[13] = {};
    bool cloudCacheValid = false;

    // Signalsmith chorus
    std::unique_ptr<signalsmith::basics::ChorusFloat> chorus;
    float chorusCached[4] = {};

    // Soundpipe phaser
    sp_data sp{};
    sp_phaser* phaser = nullptr;

    // Signalsmith Stretch
    std::unique_ptr<signalsmith::stretch::SignalsmithStretch<float>> stretch;
    float stretchCached[3] = {};

    void init(int t, float sampleRate)
    {
        type = t;
        sr = sampleRate;
        if (type == CymbalsVendoredFx::kCloudSeedReverb) {
            cloud = std::make_unique<Cloudseed::ReverbController>((int)sr);
            Cloudseed::initPrograms();
            for (int i = 0; i < Cloudseed::Parameter::COUNT; ++i)
                cloud->SetParameter(i, Cloudseed::ProgramDarkPlate[i]);
            cloudCacheValid = false;
        } else if (type == CymbalsVendoredFx::kSignalsmithChorus) {
            chorus = std::make_unique<signalsmith::basics::ChorusFloat>();
            chorus->configure(sr, 1, 2);
            std::memset(chorusCached, 0, sizeof(chorusCached));
        } else if (type == CymbalsVendoredFx::kSoundpipePhaser) {
            std::memset(&sp, 0, sizeof(sp));
            sp.sr = (int)sr;
            sp.nchan = 2;
            if (phaser) {
                sp_phaser_destroy(&phaser);
                phaser = nullptr;
            }
            if (sp_phaser_create(&phaser) == SP_OK && phaser)
                sp_phaser_init(&sp, phaser);
        } else if (type == CymbalsVendoredFx::kSignalsmithStretch) {
            stretch = std::make_unique<signalsmith::stretch::SignalsmithStretch<float>>();
            stretch->configure(2, 1, 1);
            std::memset(stretchCached, 0, sizeof(stretchCached));
        }
    }

    void reset()
    {
        if (cloud) {
            for (int i = 0; i < Cloudseed::Parameter::COUNT; ++i)
                cloud->SetParameter(i, Cloudseed::ProgramDarkPlate[i]);
            cloud->ClearBuffers();
            cloudCacheValid = false;
        }
        if (chorus) chorus->reset();
        if (phaser) sp_phaser_init(&sp, phaser);
        if (stretch) stretch->reset();
        std::memset(cloudCached, 0, sizeof(cloudCached));
        std::memset(chorusCached, 0, sizeof(chorusCached));
        std::memset(stretchCached, 0, sizeof(stretchCached));
    }

    void processCloud(float& l, float& r, const float* p)
    {
        bool changed = !cloudCacheValid;
        for (int i = 0; i < 13; ++i) {
            if (p[i] != cloudCached[i]) { changed = true; cloudCached[i] = p[i]; }
        }
        if (changed) {
            static const int map[13] = {
                Cloudseed::Parameter::LowCut,
                Cloudseed::Parameter::HighCut,
                Cloudseed::Parameter::InputMix,
                Cloudseed::Parameter::DryOut,
                Cloudseed::Parameter::EarlyOut,
                Cloudseed::Parameter::LateOut,
                Cloudseed::Parameter::TapDecay,
                Cloudseed::Parameter::EarlyDiffuseFeedback,
                Cloudseed::Parameter::LateLineDecay,
                Cloudseed::Parameter::LateDiffuseFeedback,
                Cloudseed::Parameter::EqLowGain,
                Cloudseed::Parameter::EqHighGain,
                Cloudseed::Parameter::LateLineSize
            };
            for (int i = 0; i < 13; ++i)
                cloud->SetParameter(map[i], std::clamp(p[i], 0.0f, 1.0f));
            cloudCacheValid = true;
        }
        float inL = l, inR = r, outL = 0.0f, outR = 0.0f;
        cloud->Process(&inL, &inR, &outL, &outR, 1);
        l = outL;
        r = outR;
    }

    void processChorus(float& l, float& r, const float* p)
    {
        // Chorus parameters are smoothed internally; assign only on change.
        const float mix = std::clamp(p[0], 0.0f, 1.0f);
        const float depthMs = 2.0f + std::clamp(p[1], 0.0f, 1.0f) * 48.0f;
        const float detune = 1.0f + std::clamp(p[2], 0.0f, 1.0f) * 49.0f;
        const float stereo = std::clamp(p[3], 0.0f, 1.0f) * 1.5f;
        if (mix != chorusCached[0]) { chorus->mix = mix; chorusCached[0] = mix; }
        if (depthMs != chorusCached[1]) { chorus->depthMs = depthMs; chorusCached[1] = depthMs; }
        if (detune != chorusCached[2]) { chorus->detune = detune; chorusCached[2] = detune; }
        if (stereo != chorusCached[3]) { chorus->stereo = stereo; chorusCached[3] = stereo; }

        float inL = l, inR = r, outL = 0.0f, outR = 0.0f;
        float* in[2] = {&inL, &inR};
        float* out[2] = {&outL, &outR};
        chorus->process(in, out, 1);
        l = outL;
        r = outR;
    }

    void processPhaser(float& l, float& r, const float* p)
    {
        if (!phaser) return;
        *phaser->MaxNotch1Freq = 20.0f + std::clamp(p[0], 0.0f, 1.0f) * 9980.0f;
        *phaser->MinNotch1Freq = 20.0f + std::clamp(p[1], 0.0f, 1.0f) * 4980.0f;
        *phaser->Notch_width = 10.0f + std::clamp(p[2], 0.0f, 1.0f) * 4990.0f;
        *phaser->NotchFreq = 1.1f + std::clamp(p[3], 0.0f, 1.0f) * 2.9f;
        *phaser->VibratoMode = p[4] > 0.5f ? 1.0f : 0.0f;
        *phaser->depth = std::clamp(p[5], 0.0f, 1.0f);
        *phaser->feedback_gain = std::clamp(p[6], 0.0f, 1.0f);
        float inL = l, inR = r, outL = 0.0f, outR = 0.0f;
        sp_phaser_compute(&sp, phaser, &inL, &inR, &outL, &outR);
        l = outL;
        r = outR;
    }

    void processStretch(float& l, float& r, const float* p)
    {
        const float semis = (std::clamp(p[0], 0.0f, 1.0f) - 0.5f) * 48.0f;
        const float mix = std::clamp(p[1], 0.0f, 1.0f);
        const float tonality = 0.02f + std::clamp(p[2], 0.0f, 1.0f) * 0.48f;
        // Keep the pitch map current; setTransposeSemitones is cheap and the
        // library caches its own fade state.
        stretch->setTransposeSemitones(semis, tonality);
        float inL = l, inR = r, outL = 0.0f, outR = 0.0f;
        float* in[2] = {&inL, &inR};
        float* out[2] = {&outL, &outR};
        stretch->process(in, 1, out, 1);
        // Stretch has no dry/wet; provide one here.
        l = l * (1.0f - mix) + outL * mix;
        r = r * (1.0f - mix) + outR * mix;
        (void)stretchCached;
    }
};

VendoredFxSlot::VendoredFxSlot(int typeId, float sampleRate)
    : m_impl(std::make_unique<Impl>()), m_typeId(typeId), m_sampleRate(sampleRate)
{
    m_impl->init(typeId, sampleRate);
}

VendoredFxSlot::~VendoredFxSlot()
{
    if (m_impl && m_impl->phaser)
        sp_phaser_destroy(&m_impl->phaser);
}

void VendoredFxSlot::setSampleRate(float sampleRate)
{
    m_sampleRate = sampleRate;
    if (m_impl) m_impl->init(m_typeId, sampleRate);
}

void VendoredFxSlot::reset()
{
    if (m_impl) m_impl->reset();
}

void VendoredFxSlot::process(float& outL, float& outR, const float* params13)
{
    if (!m_impl || !params13) return;
    switch (m_typeId) {
    case CymbalsVendoredFx::kCloudSeedReverb:
        m_impl->processCloud(outL, outR, params13);
        break;
    case CymbalsVendoredFx::kSignalsmithChorus:
        m_impl->processChorus(outL, outR, params13);
        break;
    case CymbalsVendoredFx::kSoundpipePhaser:
        m_impl->processPhaser(outL, outR, params13);
        break;
    case CymbalsVendoredFx::kSignalsmithStretch:
        m_impl->processStretch(outL, outR, params13);
        break;
    default:
        break;
    }
}
