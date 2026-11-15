#include "AirSuiteUniversal.hpp"
#include "AirwindowsDispatch.inc"
#include "AirSuiteUniversal.hpp"
#include "AirwindowsSpecTable.inc"
#include "AirwindowsClipping.hpp"
#include "AirwindowsDithers.hpp"
#include "AirwindowsUtility.hpp"
#include "AirwindowsExtractedParams.hpp"
#include "vst2stub/audioeffectx.h"
#include "AirwindowsFactories.inc"
#include <cmath>
#include <cstring>
#include <algorithm>
#include <cstdio>
#include <unordered_map>

START_NAMESPACE_DISTRHO

// ============================================================
// Static Utilities
// ============================================================

bool AirSuiteUniversal::isWetParam(const char* name) {
    if (!name) return false;
    return (strcmp(name, "Dry/Wet") == 0 ||
            strcmp(name, "Wet/Dry") == 0 ||
            strcmp(name, "Wetness") == 0 ||
            strcmp(name, "wet") == 0 ||
            strcmp(name, "Wet") == 0 ||
            strcmp(name, "Mix") == 0 ||
            strcmp(name, "I/Dr/Wt") == 0 ||
            strcmp(name, "Inv/Wet") == 0 ||
            strcmp(name, "inv__wet") == 0 ||
            strcmp(name, "dry__wet") == 0);
}

AirwindowsSpec AirSuiteUniversal::getSuppressedSpec(Type type) {
    AirwindowsSpec spec = getSpec(type);
    for (int i = 0; i < spec.count; ++i) {
        if (isWetParam(spec.names[i])) {
            spec.names[i] = nullptr;
        }
    }
    return spec;
}

const AirSuiteUniversal::RuntimeIdentity& AirSuiteUniversal::getRuntimeIdentity(Type type) {
    static const std::array<RuntimeIdentity, COUNT> kRuntimeIdentities = []() {
        std::array<RuntimeIdentity, COUNT> identities{};
        for (int i = 0; i < COUNT; ++i) {
            const Type rawType = (Type)i;
            const Type canonicalType = getCanonicalType(rawType);

            RuntimeIdentity identity{};
            identity.canonicalType = canonicalType;
            identity.category = computeRegistryCategory(canonicalType);
            identity.categoryName = getCategoryName(identity.category);
            identity.displayName = computeRegistryDisplayName(canonicalType);
            identity.dispatchType = resolveDispatchType(identity);

            if (canonicalType != NONE) {
                const AirwindowsSpec spec = getSpec(canonicalType);
                for (int paramIndex = 0; paramIndex < spec.count; ++paramIndex) {
                    if (isWetParam(spec.names[paramIndex])) {
                        identity.hasInternalWet = true;
                        break;
                    }
                }
            }

            identities[i] = identity;
        }
        return identities;
    }();

    return kRuntimeIdentities[(int)sanitizeTypeValue((float)type)];
}

int AirSuiteUniversal::getDispatchType(Type type) {
    return getRuntimeIdentity(type).dispatchType;
}

/**
 * Phase 1: Policy Enforcement - Check if plugin has internal wet parameter
 * 
 * External Dry/Wet Policy:
 * - Plugins WITHOUT internal wet: receive full external audio mixing at slot level
 * - Plugins WITH internal wet: can use internal wet param, external mixing still applies
 * 
 * This method determines which category a plugin falls into by checking if
 * its parameter spec includes any wet-like parameter name.
 */
bool AirSuiteUniversal::hasInternalWet(Type type) {
    return getRuntimeIdentity(type).hasInternalWet;
}

AirSuiteUniversal::Category AirSuiteUniversal::computeRegistryCategory(Type type) {
    if (type >= REVERB_BRIGHTAMBIENCE && type <= REVERB_VERBTINY) return Category::REVERB;
    if (type >= EFFECT_AURA && type <= EFFECT_TRIANGLIZER) return Category::EFFECT;
    if (type >= EFFECT_ELECTROHAT && type <= EFFECT_MONOAM) return Category::EFFECT;
    if (type >= AMPSIM_CABS && type <= AMPSIM_LILAMPL) return Category::AMPSIM;
    if (type >= DIST_BASSDRIVE && type <= DIST_BITGLITTER) return Category::DIST;
    if (type >= DYN_PRESSURE4 && type <= DYN_PURESTGAIN) return Category::DYN;
    if (type >= SAT_BUILDATPDF && type <= SAT_COMPRESATURATOR) return Category::SAT;
    if (type >= TAPE_TOTAPE6 && type <= TAPE_TOVINYL4) return Category::TAPE;
    if (type >= FILTER_AIR && type <= FILTER_BIQUADDOUBLE) return Category::FILTER;
    if (type >= BIQUAD_BIQUAD && type <= BIQUAD_BIQUADTRIPLE) return Category::BIQUAD;
    if (type >= CLIP_ADCLIP7 && type <= CLIP_CLIPSOFTLY) return Category::CLIPPING;
    if (type >= DITHER_DITHERMEDISKERS && type <= DITHER_VINYLDITHER) return Category::DITHERS;
    if (type >= UTILITY_AVERAGE && type <= UTILITY_WEIGHT) return Category::UTILITY;
    if (type >= STEREO_CHANNEL4 && type <= STEREO_WIDER) return Category::STEREO;
    if (type >= BASS_BASSAMP && type <= BASS_SUBSONLY) return Category::BASS;
    if (type >= LOFI_DESK && type <= LOFI_SAMPLEDELAY) return Category::LOFI;
    if (type >= NOISE_DARKNOISE && type <= NOISE_SURGETIDE) return Category::NOISE;
    if (type >= AMBIENCE_AURA && type <= AMBIENCE_VERBITY2) return Category::AMBIENCE;
    if (type >= BRIGHTNESS_AIR && type <= BRIGHTNESS_PURESTAIR) return Category::BRIGHTNESS;
    if (type >= SUBTLETY_BAXANDALL && type <= SUBTLETY_UNBOX) return Category::SUBTLETY;
    if (type >= TONE_COLOUR_FOCUS && type < XYZ_XBANDPASS) return Category::TONE_COLOUR;
    if (isStrictXYZCoreType(type)) return Category::XYZ;
    switch (type) {
        case DYN_ACCELERATION2: return Category::DYN;
        case EFFECT_ADT: return Category::EFFECT;
        case FILTER_ANGLE_EQ: return Category::FILTER;
        case FILTER_ANGLE_FILTER: return Category::FILTER;
        case DYN_A_QUICK_VOICE_CLIP: return Category::DYN;
        case SAT_ATMOSPHERE_BUSS: return Category::SAT;
        case SAT_ATMOSPHERE_CHANNEL: return Category::SAT;
        case EFFECT_AUTO_PAN: return Category::EFFECT;
        case FILTER_AVER_MATRIX: return Category::FILTER;
        case EFFECT_BEAM: return Category::EFFECT;
        case FILTER_BEZ_EQ: return Category::FILTER;
        case FILTER_BEZ_EQ_2: return Category::FILTER;
        case DYN_BEZI_COMP: return Category::DYN;
        case EFFECT_BIT_DUAL_PAN: return Category::EFFECT;
        case DIST_BITE: return Category::DIST;
        case EFFECT_BIT_SHIFT_PAN: return Category::EFFECT;
        case DYN_BLOCK_PARTY: return Category::DYN;
        case DYN_BRASS_RIDER: return Category::DYN;
        case SAT_BUSS_COLORS4: return Category::SAT;
        case DYN_BUTTER_COMP: return Category::DYN;
        case DYN_BUTTER_COMP2: return Category::DYN;
        case SAT_C5_RAW_BUSS: return Category::SAT;
        case SAT_C5_RAW_CHANNEL: return Category::SAT;
        case AMPSIM_CABS2: return Category::AMPSIM;
        case EFFECT_CANS: return Category::EFFECT;
        case EFFECT_CANS_AW: return Category::EFFECT;
        case AMPSIM_CHIMEY_DELUXE: return Category::AMPSIM;
        case AMPSIM_CHIMEY_GUITAR: return Category::AMPSIM;
        case AMPSIM_CHIMEY_GUITAR2: return Category::AMPSIM;
        case EFFECT_CHORUS: return Category::EFFECT;
        case EFFECT_CHORUS_ENSEMBLE: return Category::EFFECT;
        case TAPE_CHROME_OXIDE: return Category::TAPE;
        case SAT_CIDER: return Category::SAT;
        case DYN_CLIP_ONLY3: return Category::DYN;
        case DIST_COJONES: return Category::DIST;
        case EFFECT_CONTENT_HIDE_D: return Category::EFFECT;
        case DIST_CREATURE: return Category::DIST;
        case AMPSIM_CRICK_BASS: return Category::AMPSIM;
        case TAPE_CRUNCHY_GROOVE_WEAR: return Category::TAPE;
        case DYN_C_STRIP: return Category::DYN;
        case EFFECT_CURVE: return Category::EFFECT;
        case FILTER_DARK: return Category::FILTER;
        case REVERB_DATTORRO: return Category::REVERB;
        case EFFECT_DC_VOLTAGE: return Category::EFFECT;
        case DYN_DE_BESS: return Category::DYN;
        case FILTER_DE_BEZ: return Category::FILTER;
        case DIST_DECKWRECKA: return Category::DIST;
        case EFFECT_DE_CRACKLE: return Category::EFFECT;
        case DYN_DE_ESS: return Category::DYN;
        case DYN_DE_HISS: return Category::DYN;
        case DYN_DE_NOISE: return Category::DYN;
        case EFFECT_DE_REZ: return Category::EFFECT;
        case EFFECT_DE_REZ2: return Category::EFFECT;
        case EFFECT_DE_REZ3: return Category::EFFECT;
        case DIST_DIRT: return Category::DIST;
        case TAPE_DISCONTAPEITY: return Category::TAPE;
        case EFFECT_DISCONTINUITY: return Category::EFFECT;
        case EFFECT_DISTANCE: return Category::EFFECT;
        case EFFECT_DISTANCE2: return Category::EFFECT;
        case EFFECT_DISTANCE3: return Category::EFFECT;
        case EFFECT_DITHERBOX: return Category::EFFECT;
        case DIST_DONUT: return Category::DIST;
        case EFFECT_DOUBLELAY: return Category::EFFECT;
        case EFFECT_DOUBLE_PAUL: return Category::EFFECT;
        case DYN_DRUM_SLAM: return Category::DYN;
        case EFFECT_DUB_CENTER: return Category::EFFECT;
        case TAPE_DUB_PLATE: return Category::TAPE;
        case TAPE_DUB_PLATE2: return Category::TAPE;
        case EFFECT_DUB_SUB2: return Category::EFFECT;
        case EFFECT_DUST_BUNNY: return Category::EFFECT;
        case DYN_DYNAMICS: return Category::DYN;
        case DYN_DYNAMICS2: return Category::DYN;
        case DYN_DYNAMICS3: return Category::DYN;
        case DYN_DYNO: return Category::DYN;
        case DIST_EDGE: return Category::DIST;
        case EFFECT_ED_IS_DIM: return Category::EFFECT;
        case SAT_ELATION: return Category::SAT;
        case FILTER_ELLIPTICAL: return Category::FILTER;
        case EFFECT_ENSEMBLE: return Category::EFFECT;
        case FILTER_EQ: return Category::FILTER;
        case EFFECT_EVERY_SLEW: return Category::EFFECT;
        case EFFECT_EVERY_TRIM: return Category::EFFECT;
        case FILTER_FAT_EQ: return Category::FILTER;
        case DYN_FINAL_CLIP: return Category::DYN;
        case DYN_FLOOR: return Category::DYN;
        case TAPE_FLUTTER: return Category::TAPE;
        case TAPE_FROM_TAPE: return Category::TAPE;
        case DYN_GATELOPE: return Category::DYN;
        case EFFECT_GOLDEN_SLEW: return Category::EFFECT;
        // Restored alongside the category enum members: without a row here the
        // category is NONE, so the type dispatches nowhere and the factory is
        // never consulted - it was dead even though upstream ships Fracture.
        case DIST_FRACTURE: return Category::DIST;
        case DIST_FRACTURE2: return Category::DIST;
        case DIST_GOLEM: return Category::DIST;
        case FILTER_HERMEPASS: return Category::FILTER;
        case FILTER_HERME_TRIM: return Category::FILTER;
        case DYN_HIGH_IMPACT: return Category::DYN;
        case DIST_HIP_CRUSH: return Category::DIST;
        case FILTER_HOLT2: return Category::FILTER;
        case EFFECT_HOMBRE: return Category::EFFECT;
        case FILTER_HULL: return Category::FILTER;
        case FILTER_HULL2: return Category::FILTER;
        case DIST_HYPE: return Category::DIST;
        case DYN_HYPERSOFT: return Category::DYN;
        case FILTER_HYPERSONIC: return Category::FILTER;
        case FILTER_HYPERSON_X: return Category::FILTER;
        case DIST_INFLAMER: return Category::DIST;
        case FILTER_INFRASONIC: return Category::FILTER;
        case TAPE_IRON_OXIDE_CLASSIC: return Category::TAPE;
        case TAPE_IRON_OXIDE_CLASSIC2: return Category::TAPE;
        case FILTER_ISOLATOR: return Category::FILTER;
        case FILTER_ISOLATOR2: return Category::FILTER;
        case FILTER_ISOLATOR3: return Category::FILTER;
        case FILTER_KALMAN: return Category::FILTER;
        case EFFECT_LEFTO_MONO: return Category::EFFECT;
        case DYN_LOGICAL4: return Category::DYN;
        case REVERB_LR_CONVOLVE: return Category::REVERB;
        case REVERB_LR_CONVOLVE2: return Category::REVERB;
        case REVERB_LR_CONVOLVE3: return Category::REVERB;
        case EFFECT_LR_FLIP_TIMER: return Category::EFFECT;
        case SAT_LUXOR: return Category::SAT;
        case FILTER_MACK_EQ: return Category::FILTER;
        case DYN_MASTERING: return Category::DYN;
        case DYN_MASTERING2: return Category::DYN;
        case AMPSIM_MID_AMP: return Category::AMPSIM;
        case EFFECT_MONITORING2: return Category::EFFECT;
        case EFFECT_MONITORING3: return Category::EFFECT;
        case EFFECT_MS_FLIP_TIMER: return Category::EFFECT;
        case DIST_MULTI_BAND_DISTORTION: return Category::DIST;
        case REVERB_NEVERLAND: return Category::REVERB;
        case EFFECT_NOT_JUST_ANOTHER_CD: return Category::EFFECT;
        case EFFECT_ORBIT_KICK: return Category::EFFECT;
        case EFFECT_OVERHEADS: return Category::EFFECT;
        case FILTER_PARAMETRIC: return Category::FILTER;
        case EFFECT_PAUL_WIDE: return Category::EFFECT;
        case SAT_PD_BUSS: return Category::SAT;
        case SAT_PD_CHANNEL: return Category::SAT;
        case DYN_PEAKS_ONLY: return Category::DYN;
        case FILTER_PEAR_EQ: return Category::FILTER;
        case FILTER_PEAR_LITE_EQ: return Category::FILTER;
        case EFFECT_PITCH_DELAY: return Category::EFFECT;
        case EFFECT_POCKEY: return Category::EFFECT;
        case EFFECT_POCKEY2: return Category::EFFECT;
        case DYN_PODCAST: return Category::DYN;
        case DYN_PODCAST_DELUXE: return Category::DYN;
        case AMPSIM_POINTY_DELUXE: return Category::AMPSIM;
        case AMPSIM_POINTY_GUITAR: return Category::AMPSIM;
        case DIST_POP: return Category::DIST;
        case DIST_POP2: return Category::DIST;
        case DIST_POP3: return Category::DIST;
        case SAT_PRECIOUS: return Category::SAT;
        case DYN_PRESSURE6: return Category::DYN;
        case FILTER_PRIME_FIR: return Category::FILTER;
        case AMPSIM_PUNCHY_DELUXE: return Category::AMPSIM;
        case AMPSIM_PUNCHY_GUITAR: return Category::AMPSIM;
        case EFFECT_PUREST_DUAL_PAN: return Category::EFFECT;
        case EFFECT_PUREST_ECHO: return Category::EFFECT;
        case SAT_PUREST_WARM2: return Category::SAT;
        case DYN_PYEWACKET: return Category::DYN;
        case EFFECT_RAW_GLITTERS: return Category::EFFECT;
        case EFFECT_RAW_TIMBERS: return Category::EFFECT;
        case EFFECT_RECURVE: return Category::EFFECT;
        case EFFECT_REMAP: return Category::EFFECT;
        case FILTER_RES_EQ: return Category::FILTER;
        case FILTER_RES_EQ_2: return Category::FILTER;
        case EFFECT_RIGHTO_MONO: return Category::EFFECT;
        case DIST_SHAPE: return Category::DIST;
        case FILTER_SIDE_DULL: return Category::FILTER;
        case FILTER_SIDEPASS: return Category::FILTER;
        case EFFECT_SILHOUETTE: return Category::EFFECT;
        case FILTER_SILKEN: return Category::FILTER;
        case SAT_SINEW: return Category::SAT;
        case AMPSIM_SINGLE_ENDED_TRIODE: return Category::AMPSIM;
        case EFFECT_SLEW3: return Category::EFFECT;
        case EFFECT_SLEW4: return Category::EFFECT;
        case FILTER_SMOOTH_EQ_2: return Category::FILTER;
        case FILTER_SMOOTH_EQ_3: return Category::FILTER;
        case EFFECT_SOFT_CLOCK: return Category::EFFECT;
        case EFFECT_SOFT_CLOCK2: return Category::EFFECT;
        case EFFECT_SOFT_CLOCK3: return Category::EFFECT;
        case DYN_SOFT_GATE: return Category::DYN;
        case SAT_SQUARE_ROOT: return Category::SAT;
        case EFFECT_SRSLY2: return Category::EFFECT;
        case EFFECT_SRSLY3: return Category::EFFECT;
        case REVERB_STAR_CHILD: return Category::REVERB;
        case REVERB_STAR_CHILD2: return Category::REVERB;
        case EFFECT_STEREO_CHORUS: return Category::EFFECT;
        case EFFECT_STEREO_DOUBLER: return Category::EFFECT;
        case EFFECT_STEREO_ENSEMBLE: return Category::EFFECT;
        case EFFECT_STEREO_FX: return Category::EFFECT;
        case DIST_STONEFIRE: return Category::DIST;
        case DYN_STONE_FIRE_COMP: return Category::DYN;
        case EFFECT_SWEET_WIDE: return Category::EFFECT;
        case DYN_SWELL: return Category::DYN;
        case TAPE_TAPE: return Category::TAPE;
        case TAPE_TAPE_BIAS: return Category::TAPE;
        case EFFECT_TAPE_DELAY: return Category::EFFECT;
        case EFFECT_TAPE_DELAY2: return Category::EFFECT;
        case TAPE_TAPE_DUST: return Category::TAPE;
        case TAPE_TAPE_FAT: return Category::TAPE;
        case TAPE_TAPE_HACK: return Category::TAPE;
        case TAPE_TAPE_HACK2: return Category::TAPE;
        case TAPE_TAPE_HACK3: return Category::TAPE;
        case EFFECT_TEXTURIZE: return Category::EFFECT;
        case EFFECT_TEXTURIZE_MS: return Category::EFFECT;
        case REVERB_THUNDER: return Category::REVERB;
        case TAPE_TO_TAPE7: return Category::TAPE;
        case TAPE_TO_TAPE8: return Category::TAPE;
        case TAPE_TO_TAPE9: return Category::TAPE;
        case EFFECT_TPDF_WIDE: return Category::EFFECT;
        case SAT_TRANS_DESK: return Category::SAT;
        case EFFECT_TRIPLE_SPREAD: return Category::EFFECT;
        case AMPSIM_TUBE: return Category::AMPSIM;
        case AMPSIM_TUBE2: return Category::AMPSIM;
        case EFFECT_U_LAW_DECODE: return Category::EFFECT;
        case EFFECT_U_LAW_ENCODE: return Category::EFFECT;
        case FILTER_ULTRASONIC: return Category::FILTER;
        case FILTER_ULTRASONIC_LITE: return Category::FILTER;
        case FILTER_ULTRASONIC_MED: return Category::FILTER;
        case FILTER_ULTRASON_X: return Category::FILTER;
        case EFFECT_VIBRATO: return Category::EFFECT;
        case REVERB_VOICE_OF_THE_STARSHIP: return Category::REVERB;
        case EFFECT_VOICE_TRICK: return Category::EFFECT;
        case EFFECT_WOLFBOT: return Category::EFFECT;
        case REVERB_WOODEN_BOX: return Category::REVERB;
        case DYN_X2_BUSS: return Category::DYN;
        case FILTER_Y_NOT_BANDPASS: return Category::FILTER;
        case FILTER_Z_ACID_LOWPASS: return Category::FILTER;
        case FILTER_Z_BANDPASS2: return Category::FILTER;
        case FILTER_Z_HIGHPASS2: return Category::FILTER;
        case FILTER_Z_LOWPASS: return Category::FILTER;
        case FILTER_Z_LOWPASS2: return Category::FILTER;
        case FILTER_Z_NOTCH: return Category::FILTER;
        case FILTER_Z_NOTCH2: return Category::FILTER;
        case EFFECT_ZOOM: return Category::EFFECT;
        case EFFECT_ECHOJAM: return Category::EFFECT;
        case REVERB_VERBSEVENS: return Category::REVERB;
        case REVERB_WOODENCHEST: return Category::REVERB;
        case EFFECT_FAST_DISTANCE2: return Category::EFFECT;
        case DYN_Z_OUTPUT_STAGE: return Category::DYN;
        case DYN_ULTRALIGHT3: return Category::DYN;
        case FILTER_Z_REGION: return Category::FILTER;
        case FILTER_Z_REGION2: return Category::FILTER;

        // === 64 new plugins ===
        case CLIP_ADCLIP9: return Category::CLIPPING;
        case FILTER_BEZ_EQ3: return Category::FILTER;
        case FILTER_BEZ_EQ4: return Category::FILTER;
        case FILTER_SUZAN: return Category::FILTER;
        case FILTER_YNOTHIGHPASS: return Category::FILTER;
        case FILTER_YNOTLOWPASS: return Category::FILTER;
        case FILTER_YNOTNOTCH: return Category::FILTER;
        case REVERB_KCYBERCITY: return Category::REVERB;
        case REVERB_KROCKSTAR: return Category::REVERB;
        case EFFECT_DEREZ4: return Category::EFFECT;
        case SAT_CONSOLE0_BUSS: return Category::SAT;
        case SAT_CONSOLE0_CHANNEL: return Category::SAT;
        case SAT_CONSOLE4_BUSS: return Category::SAT;
        case SAT_CONSOLE4_CHANNEL: return Category::SAT;
        case SAT_CONSOLE5_BUSS: return Category::SAT;
        case SAT_CONSOLE5_CHANNEL: return Category::SAT;
        case SAT_CONSOLE5_DARKCH: return Category::SAT;
        case SAT_CONSOLE6_BUSS: return Category::SAT;
        case SAT_CONSOLE6_CHANNEL: return Category::SAT;
        case SAT_CONSOLE7_BUSS: return Category::SAT;
        case SAT_CONSOLE7_CASCADE: return Category::SAT;
        case SAT_CONSOLE7_CHANNEL: return Category::SAT;
        case SAT_CONSOLE7_CRUNCH: return Category::SAT;
        case SAT_CONSOLE8_BUSSHYPE: return Category::SAT;
        case SAT_CONSOLE8_BUSSIN: return Category::SAT;
        case SAT_CONSOLE8_BUSSOUT: return Category::SAT;
        case SAT_CONSOLE8_CHANNELHYPE: return Category::SAT;
        case SAT_CONSOLE8_CHANNELIN: return Category::SAT;
        case SAT_CONSOLE8_CHANNELOUT: return Category::SAT;
        case SAT_CONSOLE8_LITEBUSS: return Category::SAT;
        case SAT_CONSOLE8_LITECHANNEL: return Category::SAT;
        case SAT_CONSOLE8_SUBHYPE: return Category::SAT;
        case SAT_CONSOLE8_SUBIN: return Category::SAT;
        case SAT_CONSOLE8_SUBOUT: return Category::SAT;
        case SAT_CONSOLE9_BUSS: return Category::SAT;
        case SAT_CONSOLE9_CHANNEL: return Category::SAT;
        case SAT_CONSOLEH_BUSS: return Category::SAT;
        case SAT_CONSOLEH_CHANNEL: return Category::SAT;
        case SAT_CONSOLEH_PRE: return Category::SAT;
        case SAT_CONSOLE_LABUSS: return Category::SAT;
        case SAT_CONSOLE_LA_CHANNEL: return Category::SAT;
        case SAT_CONSOLE_MC_BUSS: return Category::SAT;
        case SAT_CONSOLE_MC_CHANNEL: return Category::SAT;
        case SAT_CONSOLE_MD_BUSS: return Category::SAT;
        case SAT_CONSOLE_MD_CHANNEL: return Category::SAT;
        case SAT_CONSOLEX_BUSS: return Category::SAT;
        case SAT_CONSOLEX_CHANNEL: return Category::SAT;
        case SAT_CONSOLEX_PRE: return Category::SAT;
        case SAT_CONSOLEX2_BUSS: return Category::SAT;
        case SAT_CONSOLEX2_CHANNEL: return Category::SAT;
        case SAT_CONSOLEX2_PRE: return Category::SAT;
        case SAT_CONSOLEX3_BUSS: return Category::SAT;
        case SAT_CONSOLEX3_CHANNEL: return Category::SAT;
        case SAT_CONSOLEX3_PRE: return Category::SAT;
        case SAT_CONSOLEX3: return Category::SAT;
        case SAT_PUREST_WARM3: return Category::SAT;
        case BASS_RETROBASS: return Category::BASS;
        case DIST_SPIRAL3: return Category::DIST;
        case EFFECT_WEAVE: return Category::EFFECT;
        case SAT_EVERYCONSOLE: return Category::SAT;
        case SAT_LONGHAND: return Category::SAT;
        case SAT_PURESTCONSOLE_BUSS: return Category::SAT;
        case SAT_PURESTCONSOLE_CHANNEL: return Category::SAT;
        case SAT_PURESTCONSOLE2_BUSS: return Category::SAT;
        case SAT_PURESTCONSOLE2_CHANNEL: return Category::SAT;
        case SAT_PURESTCONSOLE3_BUSS: return Category::SAT;
        case SAT_PURESTCONSOLE3_CHANNEL: return Category::SAT;
        case SAT_PURESTCONSOLE4_BUSS: return Category::SAT;
        case SAT_PURESTCONSOLE4_CHANNEL: return Category::SAT;

        default: break;
    }

    return Category::NONE;
}

AirSuiteUniversal::Category AirSuiteUniversal::getCategory(Type type) {
    return getRuntimeIdentity(type).category;
}

const char* AirSuiteUniversal::getCategoryName(Category cat) {
    switch (cat) {
        case Category::REVERB:    return "Reverb";
        case Category::EFFECT:    return "Effect";
        case Category::AMPSIM:    return "AmpSim";
        case Category::DIST:      return "Distortion";
        case Category::DYN:       return "Dynamics";
        case Category::SAT:       return "Saturation";
        case Category::TAPE:      return "Tape";
        case Category::FILTER:    return "Filter";
        case Category::BIQUAD:    return "Biquad";
        case Category::CLIPPING:  return "Clipping";
        case Category::DITHERS:   return "Dithers";
        case Category::UTILITY:   return "Utility";
        case Category::STEREO:    return "Stereo";
        case Category::BASS:      return "Bass";
        case Category::LOFI:      return "Lo-Fi";
        case Category::NOISE:     return "Noise";
        case Category::AMBIENCE:  return "Ambience";
        case Category::BRIGHTNESS: return "Brightness";
        case Category::SUBTLETY:   return "Subtlety";
        case Category::TONE_COLOUR: return "Tone Colour";
        case Category::XYZ: return "XYZ Filters";
        default:                  return "None";
    }
}

const char* AirSuiteUniversal::computeRegistryDisplayName(Type type) {
    switch (type) {
        // REVERB
        case REVERB_BRIGHTAMBIENCE: return "BrightAmbience";
        case REVERB_BRIGHTAMBIENCE2: return "BrightAmbience2";
        case REVERB_BRIGHTAMBIENCE3: return "BrightAmbience3";
        case REVERB_CHAMBER: return "Chamber";
        case REVERB_CHAMBER2: return "Chamber2";
        case REVERB_CLEARCOAT: return "ClearCoat";
        case REVERB_CREAMCOAT: return "CreamCoat";
        case REVERB_CRUNCHCOAT: return "CrunchCoat";
        case REVERB_GALACTIC: return "Galactic";
        case REVERB_GALACTIC2: return "Galactic2";
        case REVERB_GALACTIC3: return "Galactic3";
        case REVERB_INFINITY_VERB: return "InfinityVerb";
        case REVERB_INFINITY2_VERB: return "Infinity2Verb";
        case REVERB_KALIENSPACESHIP: return "KAlienSpaceship";
        case REVERB_KBEYOND: return "KBeyond";
        case REVERB_KCATHEDRAL: return "KCathedral";
        case REVERB_KCATHEDRAL2: return "KCathedral2";
        case REVERB_KCATHEDRAL3: return "KCathedral3";
        case REVERB_KCATHEDRAL4: return "KCathedral4";
        case REVERB_KCATHEDRAL5: return "KCathedral5";
        case REVERB_KCHAMBERAR: return "KChamberAR";
        case REVERB_KCOSMOS: return "KCosmos";
        case REVERB_KGUITARHALL: return "KGuitarHall";
        case REVERB_KGUITARHALL2: return "KGuitarHall2";
        case REVERB_KPLATE140: return "KPlate140";
        case REVERB_KPLATE240: return "KPlate240";
        case REVERB_KPLATEA: return "KPlateA";
        case REVERB_KPLATEB: return "KPlateB";
        case REVERB_KPLATEC: return "KPlateC";
        case REVERB_KPLATED: return "KPlateD";
        case REVERB_KSTATION: return "KStation";
        case REVERB_KWOODROOM: return "KWoodRoom";
        case REVERB_MATRIXVERB: return "MatrixVerb";
        case REVERB_MV: return "MV";
        case REVERB_MV2: return "MV2";
        case REVERB_NONLINEARSPACE: return "NonlinearSpace";
        case REVERB_POCKETVERBS: return "PocketVerbs";
        case REVERB_REVERB: return "Reverb";
        case REVERB_VERBITY: return "Verbity";
        case REVERB_VERBITY2: return "Verbity2";
        case REVERB_VERBSIXES: return "VerbSixes";
        case REVERB_VERBTHIC: return "VerbThic";
        case REVERB_VERBTINY: return "VerbTiny";
        
        // EFFECT
        case EFFECT_AURA: return "Aura";
        case EFFECT_CLOUDCOAT: return "CloudCoat";
        case EFFECT_DISINTEGRATE: return "Disintegrate";
        case EFFECT_ENERGY: return "Energy";
        case EFFECT_ENERGY2: return "Energy2";
        case EFFECT_EXCITER: return "Exciter";
        case EFFECT_FACET: return "Facet";
        case EFFECT_FRACTURE: return "Fracture";
        case EFFECT_FRACTURE2: return "Fracture2";
        case EFFECT_GALACTICVIBE: return "GalacticVibe";
        case EFFECT_GLITCHSHIFTER: return "GlitchShifter";
        case EFFECT_GRINGER: return "Gringer";
        case EFFECT_GUITARCONDITIONER: return "GuitarConditioner";
        case EFFECT_PAFNUTY2: return "Pafnuty2";
        case EFFECT_PITCHNASTY: return "PitchNasty";
        case EFFECT_POWERSAG2: return "PowerSag2";
        case EFFECT_PREPONDERANT: return "Preponderant";
        case EFFECT_RINGMODULATOR: return "RingModulator";
        case EFFECT_TAKECARE: return "TakeCare";
        case EFFECT_TREMOLO: return "Tremolo";
        case EFFECT_TREMOSQUARE: return "TremoSquare";
        case EFFECT_TRIANGLIZER: return "Trianglizer";
        case EFFECT_ELECTROHAT: return "ElectroHat";
        case EFFECT_FLIPITY: return "Flipity";
        case EFFECT_MONITORING: return "Monitoring";
        case EFFECT_MONOAM: return "MoNoam";
        
        // AMPSIM
        case AMPSIM_CABS: return "Cabs";
        case AMPSIM_BASSAMP: return "BassAmp";
        case AMPSIM_BIGAMP: return "BigAmp";
        case AMPSIM_FIREAMP: return "FireAmp";
        case AMPSIM_GRINDAMP: return "GrindAmp";
        case AMPSIM_GUITARCONDITIONER: return "GuitarConditioner";
        case AMPSIM_PURESTMULTISQUISH: return "PurestSquish";
        case AMPSIM_TONESLANT: return "ToneSlant";
        case AMPSIM_LEADAMP: return "LeadAmp";
        case AMPSIM_LILAMPL: return "LilAmp";
        
        // DISTORTION
        case DIST_BASSDRIVE: return "BassDrive";
        case DIST_DENSITY: return "Density";
        case DIST_DENSITY2: return "Density2";
        case DIST_DENSITY3: return "Density3";
        case DIST_DISTORTION: return "Distortion";
        case DIST_DRIVE: return "Drive";
        case DIST_FRACTURE: return "Fracture";
        case DIST_FRACTURE2: return "Fracture2";
        case DIST_HARDVACUUM: return "HardVacuum";
        case DIST_PURESTDRIVE: return "PurestDrive";
        case DIST_SPIRAL: return "Spiral";
        case DIST_SPIRAL2: return "Spiral2";
        case DIST_BITGLITTER: return "BitGlitter";
        
        // DYNAMICS
        case DYN_PRESSURE4: return "Pressure4";
        case DYN_PRESSURE5: return "Pressure5";
        case DYN_VARIMU: return "VariMu";
        case DYN_POINT: return "Point";
        case DYN_ACCELERATION: return "Acceleration";
        case DYN_PURESTGAIN: return "PurestGain";
        
        // SATURATION
        case SAT_BUILDATPDF: return "BuildAtPDF";
        case SAT_DITHERFLOAT: return "DitherFloat";
        case SAT_POWERSAG: return "PowerSag";
        case SAT_NIKOLA: return "Nikola";
        case SAT_SHORTBUSS: return "ShortBuss";
        case SAT_COMPRESATURATOR: return "CompresaTurator";
        
        // TAPE
        case TAPE_TOTAPE6: return "ToTape6";
        case TAPE_IRONOXIDE5: return "IronOxide5";
        case TAPE_FLUTTER2: return "Flutter2";
        case TAPE_DUBLY: return "Dubly";
        case TAPE_DUBLY2: return "Dubly2";
        case TAPE_DUBLY3: return "Dubly3";
        case TAPE_TOTAPE5: return "ToTape5";
        case TAPE_TOVINYL4: return "ToVinyl4";
        
        // FILTER
        case FILTER_AIR: return "Air";
        case FILTER_CAPACITOR2: return "Capacitor2";
        case FILTER_HIGHPASS2: return "Highpass2";
        case FILTER_LOWPASS2: return "Lowpass2";
        case FILTER_AIR2: return "Air2";
        case FILTER_AIR3: return "Air3";
        case FILTER_AIR4: return "Air4";
        case FILTER_CAPACITOR: return "Capacitor";
        case FILTER_HIGHPASS: return "Highpass";
        case FILTER_LOWPASS: return "Lowpass";
        case FILTER_BIQUADDOUBLE: return "BiquadDouble";
        // BIQUAD
        case BIQUAD_BIQUAD: return "Biquad";
        case BIQUAD_BIQUAD2: return "Biquad2";
        case BIQUAD_BIQUADHILO: return "BiquadHiLo";
        case BIQUAD_BIQUADNONLIN: return "BiquadNonLin";
        case BIQUAD_BIQUADONEHALF: return "BiquadOneHalf";
        case BIQUAD_BIQUADPLUS: return "BiquadPlus";
        case BIQUAD_BIQUADSTACK: return "BiquadStack";
        case BIQUAD_BIQUADTRIPLE: return "BiquadTriple";
        
        // CLIPPING
        case CLIP_ADCLIP7: return "ADClip7";
        case CLIP_ADCLIP8: return "ADClip8";
        case CLIP_CLIPONLY: return "ClipOnly";
        case CLIP_CLIPONLY2: return "ClipOnly2";
        case CLIP_CLIPSOFTLY: return "ClipSoftly";
        
        // DITHERS
        case DITHER_DITHERMEDISKERS: return "DitherMeDiskers";
        case DITHER_DITHERMTIMBERS: return "DitherMeTimbers";
        case DITHER_HIGHGLOSS: return "HighGlossDither";
        case DITHER_NATURALIZE: return "NaturalizeDither";
        case DITHER_NODEDITHER: return "NodeDither";
        case DITHER_NJAD: return "NotJustAnotherDither";
        case DITHER_PAULDITHER: return "PaulDither";
        case DITHER_SPATIALIZE: return "SpatializeDither";
        case DITHER_TAPEDITHER: return "TapeDither";
        case DITHER_TPDFDITHER: return "TPDFDither";
        case DITHER_VINYLDITHER: return "VinylDither";
        
        // UTILITY
        case UTILITY_AVERAGE: return "Average";
        case UTILITY_BALANCED: return "Balanced";
        case UTILITY_BITSHIFTGAIN: return "BitShiftGain";
        case UTILITY_CSTRIP2: return "CStrip2";
        case UTILITY_PURESTFADE: return "PurestFade";
        case UTILITY_PURESTGAIN: return "PurestGain";
        case UTILITY_PURESTWARM: return "PurestWarm";
        case UTILITY_RIGHTEOUS4: return "Righteous4";
        case UTILITY_SLEW: return "Slew";
        case UTILITY_SLEW2: return "Slew2";
        case UTILITY_SLEWONLY: return "SlewOnly";
        case UTILITY_STEREOFX: return "StereoFX";
        case UTILITY_STUDIOTAN: return "StudioTan";
        case UTILITY_WEIGHT: return "Weight";
        
        // STEREO
        case STEREO_CHANNEL4: return "Channel4";
        case STEREO_CHANNEL5: return "Channel5";
        case STEREO_CHANNEL6: return "Channel6";
        case STEREO_CHANNEL7: return "Channel7";
        case STEREO_CHANNEL8: return "Channel8";
        case STEREO_CHANNEL9: return "Channel9";
        case STEREO_MIDSIDE: return "MidSide";
        case STEREO_PHASENUDGE: return "PhaseNudge";
        case STEREO_SRSLY: return "Srsly";
        case STEREO_WIDER: return "Wider";
        
        // BASS
        case BASS_BASSAMP: return "BassAmp";
        case BASS_BASSDRIVE: return "BassDrive";
        case BASS_BASSKIT: return "BassKit";
        case BASS_DUBSUB: return "DubSub";
        case BASS_SUBSONLY: return "SubsOnly";
        
        // LOFI
        case LOFI_DESK: return "Desk";
        case LOFI_DESK4: return "Desk4";
        case LOFI_DIGITALBLACK: return "DigitalBlack";
        case LOFI_DITHERFLOAT: return "DitherFloat";
        case LOFI_FATHOMFIVE: return "FathomFive";
        case LOFI_GRINGER: return "Gringer";
        case LOFI_GROOVEWEAR: return "GrooveWear";
        case LOFI_LOUD: return "Loud";
        case LOFI_MACKITY: return "Mackity";
        case LOFI_NCSEVENTEEN: return "NCSeventeen";
        case LOFI_POCKETVERBS: return "PocketVerbs";
        case LOFI_SAMPLEDELAY: return "SampleDelay";
        
        // NOISE
        case NOISE_DARKNOISE: return "DarkNoise";
        case NOISE_HOLT: return "Holt";
        case NOISE_NOISE: return "Noise";
        case NOISE_PEAR: return "Pear";
        case NOISE_SURGE: return "Surge";
        case NOISE_SURGETIDE: return "SurgeTide";
        
        // AMBIENCE
        case AMBIENCE_AURA: return "Aura";
        case AMBIENCE_BRIGHTAMBIENCE: return "BrightAmbience";
        case AMBIENCE_BRIGHTAMBIENCE2: return "BrightAmbience2";
        case AMBIENCE_BRIGHTAMBIENCE3: return "BrightAmbience3";
        case AMBIENCE_CHAMBER: return "Chamber";
        case AMBIENCE_CHAMBER2: return "Chamber2";
        case AMBIENCE_VERBITY: return "Verbity";
        case AMBIENCE_VERBITY2: return "Verbity2";
        
        // BRIGHTNESS
        case BRIGHTNESS_AIR: return "Air";
        case BRIGHTNESS_APICOLYPSE: return "Apicolypse";
        case BRIGHTNESS_CAPACITOR: return "Capacitor";
        case BRIGHTNESS_CAPACITOR2: return "Capacitor2";
        case BRIGHTNESS_CRYSTAL: return "Crystal";
        case BRIGHTNESS_PURESTAIR: return "PurestAir";
        
        // SUBTLETY
        case SUBTLETY_BAXANDALL: return "Baxandall";
        case SUBTLETY_BAXANDALL2: return "Baxandall2";
        case SUBTLETY_BAXANDALL3: return "Baxandall3";
        case SUBTLETY_CALIBRE: return "Calibre";
        case SUBTLETY_COILS: return "Coils";
        case SUBTLETY_COILS2: return "Coils2";
        case SUBTLETY_INTERSTAGE: return "Interstage";
        case SUBTLETY_MOJO: return "Mojo";
        case SUBTLETY_PLATINUMSLEW: return "PlatinumSlew";
        case SUBTLETY_SLEWSONIC: return "SlewSonic";
        case SUBTLETY_SMOOTH: return "Smooth";
        case SUBTLETY_SMOOTHEQ: return "SmoothEQ";
        case SUBTLETY_TUBEDESK: return "TubeDesk";
        case SUBTLETY_UNBOX: return "UnBox";
        
        // TONE_COLOUR
        case TONE_COLOUR_FOCUS: return "Focus";
        case TONE_COLOUR_GROOVEWEAR: return "GrooveWear";
        case TONE_COLOUR_HUGENESS: return "Huge";
        case TONE_COLOUR_MELT: return "Melt";
        case TONE_COLOUR_ONECORNERCLIP: return "OneCornerClip";
        case TONE_COLOUR_PAFNUTY: return "Pafnuty";
        case TONE_COLOUR_PEAR: return "Pear";
        case TONE_COLOUR_PEAR2: return "Pear2";
        case TONE_COLOUR_SURGE: return "Surge";
        case TONE_COLOUR_SWEETEN: return "Sweeten";
        
        // XYZ
        case XYZ_XBANDPASS: return "XBandpass";
        case XYZ_XHIGHPASS: return "XHighpass";
        case XYZ_XLOWPASS: return "XLowpass";
        case XYZ_XNOTCH: return "XNotch";
        case XYZ_XREGION: return "XRegion";
        case XYZ_YBANDPASS: return "YBandpass";
        case XYZ_YHIGHPASS: return "YHighpass";
        case XYZ_YLOWPASS: return "YLowpass";
        case XYZ_YNOTCH: return "YNotch";
        case XYZ_ZBANDPASS: return "ZBandpass";
        case XYZ_ZHIGHPASS: return "ZHighpass";
        
        case DYN_ACCELERATION2: return "Acceleration2";
        case EFFECT_ADT: return "ADT";
        case FILTER_ANGLE_EQ: return "AngleEQ";
        case FILTER_ANGLE_FILTER: return "AngleFilter";
        case DYN_A_QUICK_VOICE_CLIP: return "AQuickVoiceClip";
        case SAT_ATMOSPHERE_BUSS: return "AtmosphereBuss";
        case SAT_ATMOSPHERE_CHANNEL: return "AtmosphereChannel";
        case EFFECT_AUTO_PAN: return "AutoPan";
        case FILTER_AVER_MATRIX: return "AverMatrix";
        case EFFECT_BEAM: return "Beam";
        case FILTER_BEZ_EQ: return "BezEQ";
        case FILTER_BEZ_EQ_2: return "BezEQ2";
        case DYN_BEZI_COMP: return "BeziComp";
        case EFFECT_BIT_DUAL_PAN: return "BitDualPan";
        case DIST_BITE: return "Bite";
        case EFFECT_BIT_SHIFT_PAN: return "BitShiftPan";
        case DYN_BLOCK_PARTY: return "BlockParty";
        case DYN_BRASS_RIDER: return "BrassRider";
        case SAT_BUSS_COLORS4: return "BussColors4";
        case DYN_BUTTER_COMP: return "ButterComp";
        case DYN_BUTTER_COMP2: return "ButterComp2";
        case SAT_C5_RAW_BUSS: return "C5RawBuss";
        case SAT_C5_RAW_CHANNEL: return "C5RawChannel";
        case AMPSIM_CABS2: return "Cabs2";
        case EFFECT_CANS: return "Cans";
        case EFFECT_CANS_AW: return "CansAW";
        case AMPSIM_CHIMEY_DELUXE: return "ChimeyDeluxe";
        case AMPSIM_CHIMEY_GUITAR: return "ChimeyGuitar";
        case AMPSIM_CHIMEY_GUITAR2: return "ChimeyGuitar2";
        case EFFECT_CHORUS: return "Chorus";
        case EFFECT_CHORUS_ENSEMBLE: return "ChorusEnsemble";
        case TAPE_CHROME_OXIDE: return "ChromeOxide";
        case SAT_CIDER: return "Cider";
        case DYN_CLIP_ONLY3: return "ClipOnly3";
        case DIST_COJONES: return "Cojones";
        case EFFECT_CONTENT_HIDE_D: return "ContentHideD";
        case DIST_CREATURE: return "Creature";
        case AMPSIM_CRICK_BASS: return "CrickBass";
        case TAPE_CRUNCHY_GROOVE_WEAR: return "CrunchyGrooveWear";
        case DYN_C_STRIP: return "CStrip";
        case EFFECT_CURVE: return "curve";
        case FILTER_DARK: return "Dark";
        case REVERB_DATTORRO: return "Dattorro";
        case EFFECT_DC_VOLTAGE: return "DCVoltage";
        case DYN_DE_BESS: return "DeBess";
        case FILTER_DE_BEZ: return "DeBez";
        case DIST_DECKWRECKA: return "Deckwrecka";
        case EFFECT_DE_CRACKLE: return "DeCrackle";
        case DYN_DE_ESS: return "DeEss";
        case DYN_DE_HISS: return "DeHiss";
        case DYN_DE_NOISE: return "DeNoise";
        case EFFECT_DE_REZ: return "DeRez";
        case EFFECT_DE_REZ2: return "DeRez2";
        case EFFECT_DE_REZ3: return "DeRez3";
        case DIST_DIRT: return "Dirt";
        case TAPE_DISCONTAPEITY: return "Discontapeity";
        case EFFECT_DISCONTINUITY: return "Discontinuity";
        case EFFECT_DISTANCE: return "Distance";
        case EFFECT_DISTANCE2: return "Distance2";
        case EFFECT_DISTANCE3: return "Distance3";
        case EFFECT_DITHERBOX: return "Ditherbox";
        case DIST_DONUT: return "Donut";
        case EFFECT_DOUBLELAY: return "Doublelay";
        case EFFECT_DOUBLE_PAUL: return "DoublePaul";
        case DYN_DRUM_SLAM: return "DrumSlam";
        case EFFECT_DUB_CENTER: return "DubCenter";
        case TAPE_DUB_PLATE: return "DubPlate";
        case TAPE_DUB_PLATE2: return "DubPlate2";
        case EFFECT_DUB_SUB2: return "DubSub2";
        case EFFECT_DUST_BUNNY: return "DustBunny";
        case DYN_DYNAMICS: return "Dynamics";
        case DYN_DYNAMICS2: return "Dynamics2";
        case DYN_DYNAMICS3: return "Dynamics3";
        case DYN_DYNO: return "Dyno";
        case DIST_EDGE: return "Edge";
        case EFFECT_ED_IS_DIM: return "EdIsDim";
        case SAT_ELATION: return "Elation";
        case FILTER_ELLIPTICAL: return "Elliptical";
        case EFFECT_ENSEMBLE: return "Ensemble";
        case FILTER_EQ: return "EQ";
        case EFFECT_EVERY_SLEW: return "EverySlew";
        case EFFECT_EVERY_TRIM: return "EveryTrim";
        case FILTER_FAT_EQ: return "FatEQ";
        case DYN_FINAL_CLIP: return "FinalClip";
        case DYN_FLOOR: return "Floor";
        case TAPE_FLUTTER: return "Flutter";
        case TAPE_FROM_TAPE: return "FromTape";
        case DYN_GATELOPE: return "Gatelope";
        case EFFECT_GOLDEN_SLEW: return "GoldenSlew";
        case DIST_GOLEM: return "Golem";
        case FILTER_HERMEPASS: return "Hermepass";
        case FILTER_HERME_TRIM: return "HermeTrim";
        case DYN_HIGH_IMPACT: return "HighImpact";
        case DIST_HIP_CRUSH: return "HipCrush";
        case FILTER_HOLT2: return "Holt2";
        case EFFECT_HOMBRE: return "Hombre";
        case FILTER_HULL: return "Hull";
        case FILTER_HULL2: return "Hull2";
        case DIST_HYPE: return "Hype";
        case DYN_HYPERSOFT: return "Hypersoft";
        case FILTER_HYPERSONIC: return "Hypersonic";
        case FILTER_HYPERSON_X: return "HypersonX";
        case DIST_INFLAMER: return "Inflamer";
        case FILTER_INFRASONIC: return "Infrasonic";
        case TAPE_IRON_OXIDE_CLASSIC: return "IronOxideClassic";
        case TAPE_IRON_OXIDE_CLASSIC2: return "IronOxideClassic2";
        case FILTER_ISOLATOR: return "Isolator";
        case FILTER_ISOLATOR2: return "Isolator2";
        case FILTER_ISOLATOR3: return "Isolator3";
        case FILTER_KALMAN: return "Kalman";
        case EFFECT_LEFTO_MONO: return "LeftoMono";
        case DYN_LOGICAL4: return "Logical4";
        case REVERB_LR_CONVOLVE: return "LRConvolve";
        case REVERB_LR_CONVOLVE2: return "LRConvolve2";
        case REVERB_LR_CONVOLVE3: return "LRConvolve3";
        case EFFECT_LR_FLIP_TIMER: return "LRFlipTimer";
        case SAT_LUXOR: return "Luxor";
        case FILTER_MACK_EQ: return "MackEQ";
        case DYN_MASTERING: return "Mastering";
        case DYN_MASTERING2: return "Mastering2";
        case AMPSIM_MID_AMP: return "MidAmp";
        case EFFECT_MONITORING2: return "Monitoring2";
        case EFFECT_MONITORING3: return "Monitoring3";
        case EFFECT_MS_FLIP_TIMER: return "MSFlipTimer";
        case DIST_MULTI_BAND_DISTORTION: return "MultiBandDistortion";
        case REVERB_NEVERLAND: return "Neverland";
        case EFFECT_NOT_JUST_ANOTHER_CD: return "NotJustAnotherCD";
        case EFFECT_ORBIT_KICK: return "OrbitKick";
        case EFFECT_OVERHEADS: return "Overheads";
        case FILTER_PARAMETRIC: return "Parametric";
        case EFFECT_PAUL_WIDE: return "PaulWide";
        case SAT_PD_BUSS: return "PDBuss";
        case SAT_PD_CHANNEL: return "PDChannel";
        case DYN_PEAKS_ONLY: return "PeaksOnly";
        case FILTER_PEAR_EQ: return "PearEQ";
        case FILTER_PEAR_LITE_EQ: return "PearLiteEQ";
        case EFFECT_PITCH_DELAY: return "PitchDelay";
        case EFFECT_POCKEY: return "Pockey";
        case EFFECT_POCKEY2: return "Pockey2";
        case DYN_PODCAST: return "Podcast";
        case DYN_PODCAST_DELUXE: return "PodcastDeluxe";
        case AMPSIM_POINTY_DELUXE: return "PointyDeluxe";
        case AMPSIM_POINTY_GUITAR: return "PointyGuitar";
        case DIST_POP: return "Pop";
        case DIST_POP2: return "Pop2";
        case DIST_POP3: return "Pop3";
        case SAT_PRECIOUS: return "Precious";
        case DYN_PRESSURE6: return "Pressure6";
        case FILTER_PRIME_FIR: return "PrimeFIR";
        case AMPSIM_PUNCHY_DELUXE: return "PunchyDeluxe";
        case AMPSIM_PUNCHY_GUITAR: return "PunchyGuitar";
        case EFFECT_PUREST_DUAL_PAN: return "PurestDualPan";
        case EFFECT_PUREST_ECHO: return "PurestEcho";
        case SAT_PUREST_WARM2: return "PurestWarm2";
        case DYN_PYEWACKET: return "Pyewacket";
        case EFFECT_RAW_GLITTERS: return "RawGlitters";
        case EFFECT_RAW_TIMBERS: return "RawTimbers";
        case EFFECT_RECURVE: return "Recurve";
        case EFFECT_REMAP: return "Remap";
        case FILTER_RES_EQ: return "ResEQ";
        case FILTER_RES_EQ_2: return "ResEQ2";
        case EFFECT_RIGHTO_MONO: return "RightoMono";
        case DIST_SHAPE: return "Shape";
        case FILTER_SIDE_DULL: return "SideDull";
        case FILTER_SIDEPASS: return "Sidepass";
        case EFFECT_SILHOUETTE: return "Silhouette";
        case FILTER_SILKEN: return "Silken";
        case SAT_SINEW: return "Sinew";
        case AMPSIM_SINGLE_ENDED_TRIODE: return "SingleEndedTriode";
        case EFFECT_SLEW3: return "Slew3";
        case EFFECT_SLEW4: return "Slew4";
        case FILTER_SMOOTH_EQ_2: return "SmoothEQ2";
        case FILTER_SMOOTH_EQ_3: return "SmoothEQ3";
        case EFFECT_SOFT_CLOCK: return "SoftClock";
        case EFFECT_SOFT_CLOCK2: return "SoftClock2";
        case EFFECT_SOFT_CLOCK3: return "SoftClock3";
        case DYN_SOFT_GATE: return "SoftGate";
        case SAT_SQUARE_ROOT: return "SquareRoot";
        case EFFECT_SRSLY2: return "Srsly2";
        case EFFECT_SRSLY3: return "Srsly3";
        case REVERB_STAR_CHILD: return "StarChild";
        case REVERB_STAR_CHILD2: return "StarChild2";
        case EFFECT_STEREO_CHORUS: return "StereoChorus";
        case EFFECT_STEREO_DOUBLER: return "StereoDoubler";
        case EFFECT_STEREO_ENSEMBLE: return "StereoEnsemble";
        case EFFECT_STEREO_FX: return "StereoFX";
        case DIST_STONEFIRE: return "Stonefire";
        case DYN_STONE_FIRE_COMP: return "StoneFireComp";
        case EFFECT_SWEET_WIDE: return "SweetWide";
        case DYN_SWELL: return "Swell";
        case TAPE_TAPE: return "Tape";
        case TAPE_TAPE_BIAS: return "TapeBias";
        case EFFECT_TAPE_DELAY: return "TapeDelay";
        case EFFECT_TAPE_DELAY2: return "TapeDelay2";
        case TAPE_TAPE_DUST: return "TapeDust";
        case TAPE_TAPE_FAT: return "TapeFat";
        case TAPE_TAPE_HACK: return "TapeHack";
        case TAPE_TAPE_HACK2: return "TapeHack2";
        case TAPE_TAPE_HACK3: return "TapeHack3";
        case EFFECT_TEXTURIZE: return "Texturize";
        case EFFECT_TEXTURIZE_MS: return "TexturizeMS";
        case REVERB_THUNDER: return "Thunder";
        case TAPE_TO_TAPE7: return "ToTape7";
        case TAPE_TO_TAPE8: return "ToTape8";
        case TAPE_TO_TAPE9: return "ToTape9";
        case EFFECT_TPDF_WIDE: return "TPDFWide";
        case SAT_TRANS_DESK: return "TransDesk";
        case EFFECT_TRIPLE_SPREAD: return "TripleSpread";
        case AMPSIM_TUBE: return "Tube";
        case AMPSIM_TUBE2: return "Tube2";
        case EFFECT_U_LAW_DECODE: return "uLawDecode";
        case EFFECT_U_LAW_ENCODE: return "uLawEncode";
        case FILTER_ULTRASONIC: return "Ultrasonic";
        case FILTER_ULTRASONIC_LITE: return "UltrasonicLite";
        case FILTER_ULTRASONIC_MED: return "UltrasonicMed";
        case FILTER_ULTRASON_X: return "UltrasonX";
        case EFFECT_VIBRATO: return "Vibrato";
        case REVERB_VOICE_OF_THE_STARSHIP: return "VoiceOfTheStarship";
        case EFFECT_VOICE_TRICK: return "VoiceTrick";
        case EFFECT_WOLFBOT: return "Wolfbot";
        case REVERB_WOODEN_BOX: return "WoodenBox";
        case DYN_X2_BUSS: return "X2Buss";
        case FILTER_Y_NOT_BANDPASS: return "YNotBandpass";
        case FILTER_Z_ACID_LOWPASS: return "ZAcidLowpass";
        case FILTER_Z_BANDPASS2: return "ZBandpass2";
        case FILTER_Z_HIGHPASS2: return "ZHighpass2";
        case FILTER_Z_LOWPASS: return "ZLowpass";
        case FILTER_Z_LOWPASS2: return "ZLowpass2";
        case FILTER_Z_NOTCH: return "ZNotch";
        case FILTER_Z_NOTCH2: return "ZNotch2";
        case EFFECT_ZOOM: return "Zoom";
        case EFFECT_ECHOJAM: return "Echojam";
        case REVERB_VERBSEVENS: return "VerbSevens";
        case REVERB_WOODENCHEST: return "WoodenChest";
        case EFFECT_FAST_DISTANCE2: return "FastDistance2";
        case DYN_Z_OUTPUT_STAGE: return "ZOutputStage";
        case DYN_ULTRALIGHT3: return "Ultralight3";
        case FILTER_Z_REGION: return "ZRegion";
        case FILTER_Z_REGION2: return "ZRegion2";

        // === 64 new plugins ===
        case CLIP_ADCLIP9: return "ADClip9";
        case FILTER_BEZ_EQ3: return "BezEQ3";
        case FILTER_BEZ_EQ4: return "BezEQ4";
        case FILTER_SUZAN: return "Suzan";
        case FILTER_YNOTHIGHPASS: return "YNotHighpass";
        case FILTER_YNOTLOWPASS: return "YNotLowpass";
        case FILTER_YNOTNOTCH: return "YNotNotch";
        case REVERB_KCYBERCITY: return "KCyberCity";
        case REVERB_KROCKSTAR: return "KRockstar";
        case EFFECT_DEREZ4: return "DeRez4";
        case SAT_CONSOLE0_BUSS: return "Console0 Buss";
        case SAT_CONSOLE0_CHANNEL: return "Console0 Channel";
        case SAT_CONSOLE4_BUSS: return "Console4 Buss";
        case SAT_CONSOLE4_CHANNEL: return "Console4 Channel";
        case SAT_CONSOLE5_BUSS: return "Console5 Buss";
        case SAT_CONSOLE5_CHANNEL: return "Console5 Channel";
        case SAT_CONSOLE5_DARKCH: return "Console5 DarkCh";
        case SAT_CONSOLE6_BUSS: return "Console6 Buss";
        case SAT_CONSOLE6_CHANNEL: return "Console6 Channel";
        case SAT_CONSOLE7_BUSS: return "Console7 Buss";
        case SAT_CONSOLE7_CASCADE: return "Console7 Cascade";
        case SAT_CONSOLE7_CHANNEL: return "Console7 Channel";
        case SAT_CONSOLE7_CRUNCH: return "Console7 Crunch";
        case SAT_CONSOLE8_BUSSHYPE: return "Console8 BussHype";
        case SAT_CONSOLE8_BUSSIN: return "Console8 BussIn";
        case SAT_CONSOLE8_BUSSOUT: return "Console8 BussOut";
        case SAT_CONSOLE8_CHANNELHYPE: return "Console8 ChannelHype";
        case SAT_CONSOLE8_CHANNELIN: return "Console8 ChannelIn";
        case SAT_CONSOLE8_CHANNELOUT: return "Console8 ChannelOut";
        case SAT_CONSOLE8_LITEBUSS: return "Console8 LiteBuss";
        case SAT_CONSOLE8_LITECHANNEL: return "Console8 LiteChannel";
        case SAT_CONSOLE8_SUBHYPE: return "Console8 SubHype";
        case SAT_CONSOLE8_SUBIN: return "Console8 SubIn";
        case SAT_CONSOLE8_SUBOUT: return "Console8 SubOut";
        case SAT_CONSOLE9_BUSS: return "Console9 Buss";
        case SAT_CONSOLE9_CHANNEL: return "Console9 Channel";
        case SAT_CONSOLEH_BUSS: return "ConsoleH Buss";
        case SAT_CONSOLEH_CHANNEL: return "ConsoleH Channel";
        case SAT_CONSOLEH_PRE: return "ConsoleH Pre";
        case SAT_CONSOLE_LABUSS: return "ConsoleLA Buss";
        case SAT_CONSOLE_LA_CHANNEL: return "ConsoleLA Channel";
        case SAT_CONSOLE_MC_BUSS: return "ConsoleMC Buss";
        case SAT_CONSOLE_MC_CHANNEL: return "ConsoleMC Channel";
        case SAT_CONSOLE_MD_BUSS: return "ConsoleMD Buss";
        case SAT_CONSOLE_MD_CHANNEL: return "ConsoleMD Channel";
        case SAT_CONSOLEX_BUSS: return "ConsoleX Buss";
        case SAT_CONSOLEX_CHANNEL: return "ConsoleX Channel";
        case SAT_CONSOLEX_PRE: return "ConsoleX Pre";
        case SAT_CONSOLEX2_BUSS: return "ConsoleX2 Buss";
        case SAT_CONSOLEX2_CHANNEL: return "ConsoleX2 Channel";
        case SAT_CONSOLEX2_PRE: return "ConsoleX2 Pre";
        case SAT_CONSOLEX3_BUSS: return "ConsoleX3 Buss";
        case SAT_CONSOLEX3_CHANNEL: return "ConsoleX3 Channel";
        case SAT_CONSOLEX3_PRE: return "ConsoleX3 Pre";
        case SAT_CONSOLEX3: return "ConsoleX3";
        case SAT_PUREST_WARM3: return "PurestWarm3";
        case BASS_RETROBASS: return "RetroBass";
        case DIST_SPIRAL3: return "Spiral3";
        case EFFECT_WEAVE: return "Weave";
        case SAT_EVERYCONSOLE: return "EveryConsole";
        case SAT_LONGHAND: return "Longhand";
        case SAT_PURESTCONSOLE_BUSS: return "PurestConsole Buss";
        case SAT_PURESTCONSOLE_CHANNEL: return "PurestConsole Channel";
        case SAT_PURESTCONSOLE2_BUSS: return "PurestConsole2 Buss";
        case SAT_PURESTCONSOLE2_CHANNEL: return "PurestConsole2 Channel";
        case SAT_PURESTCONSOLE3_BUSS: return "PurestConsole3 Buss";
        case SAT_PURESTCONSOLE3_CHANNEL: return "PurestConsole3 Channel";
        case SAT_PURESTCONSOLE4_BUSS: return "PurestConsole4 Buss";
        case SAT_PURESTCONSOLE4_CHANNEL: return "PurestConsole4 Channel";

        default: return "None";
    }
}

const char* AirSuiteUniversal::getDisplayName(Type type) {
    return getRuntimeIdentity(type).displayName;
}

// ============================================================
// Phase 1: Canonical Type Detection and Legacy Redirection
// ============================================================

/**
 * Determine if a Type is "canonical" (should be visible in the picker).
 * Duplicate display names are consolidated to one canonical entry.
 * 
 * Deprecated/secondary entries that share a display name with an earlier
 * enum value are marked as non-canonical and hidden from the picker,
 * but still loadable via getCanonicalType() for backward compatibility.
 */
bool AirSuiteUniversal::isCanonicalType(Type type) {
    if (type == NONE || type >= COUNT) return false;
    
    // Duplicates: second and later occurrences of same display name are non-canonical
    switch (type) {
        // BrightAmbience series: REVERB variants canonical, AMBIENCE variants deprecated
        case AMBIENCE_BRIGHTAMBIENCE:      return false;
        case AMBIENCE_BRIGHTAMBIENCE2:     return false;
        case AMBIENCE_BRIGHTAMBIENCE3:     return false;
        
        // Chamber series preserved as: REVERB canonical, AMBIENCE deprecated
        case AMBIENCE_CHAMBER:             return false;
        case AMBIENCE_CHAMBER2:            return false;
        
        // Verbity series: REVERB canonical, AMBIENCE deprecated
        case AMBIENCE_VERBITY:             return false;
        case AMBIENCE_VERBITY2:            return false;
        
        // Aura: EFFECT canonical, AMBIENCE deprecated
        case AMBIENCE_AURA:                return false;
        
        // Air variants at FILTER primary, BRIGHTNESS deprecated
        case BRIGHTNESS_AIR:               return false;
        case BRIGHTNESS_CAPACITOR:         return false;
        case BRIGHTNESS_CAPACITOR2:        return false;
        
        // GuitarConditioner: EFFECT canonical, AMPSIM deprecated
        case AMPSIM_GUITARCONDITIONER:     return false;

        // PurestSquish is intentionally hidden from the picker.
        case AMPSIM_PURESTMULTISQUISH:     return false;
        
        // Gringer: EFFECT canonical, LOFI deprecated
        case LOFI_GRINGER:                 return false;
        
        // PocketVerbs: REVERB canonical, LOFI deprecated
        case LOFI_POCKETVERBS:             return false;
        
        // GrooveWear: LOFI canonical, TONE_COLOUR deprecated
        case TONE_COLOUR_GROOVEWEAR:       return false;
        
        // DitherFloat: SAT canonical, LOFI deprecated
        case LOFI_DITHERFLOAT:             return false;
        
        // BassDrive: DIST canonical, BASS deprecated
        case BASS_BASSDRIVE:               return false;
        
        // BassAmp: AMPSIM canonical, BASS deprecated
        case BASS_BASSAMP:                 return false;

        // Fracture: EFFECT canonical, DIST deprecated
        case DIST_FRACTURE:                return false;
        case DIST_FRACTURE2:               return false;

        // PurestGain: DYN canonical, UTILITY deprecated
        case UTILITY_PURESTGAIN:           return false;

        // StereoFX: UTILITY canonical, EFFECT duplicate
        case EFFECT_STEREO_FX:             return false;
        
        // Pear: NOISE canonical, TONE_COLOUR deprecated
        case TONE_COLOUR_PEAR:             return false;
        
        // Surge: NOISE canonical, TONE_COLOUR deprecated
        case TONE_COLOUR_SURGE:            return false;
        
        default: return true;
    }
}

/**
 * Get the canonical Type for any given Type.
 * For canonical types, returns self.
 * For deprecated types, returns the primary/canonical variant.
 */
AirSuiteUniversal::Type AirSuiteUniversal::getCanonicalType(Type type) {
    if ((int)type < 1 || (int)type >= (int)COUNT) return NONE;
    
    // Redirect deprecated types to their canonical counterparts
    switch (type) {
        // BrightAmbience series
        case AMBIENCE_BRIGHTAMBIENCE:      return REVERB_BRIGHTAMBIENCE;
        case AMBIENCE_BRIGHTAMBIENCE2:     return REVERB_BRIGHTAMBIENCE2;
        case AMBIENCE_BRIGHTAMBIENCE3:     return REVERB_BRIGHTAMBIENCE3;
        
        // Chamber series
        case AMBIENCE_CHAMBER:             return REVERB_CHAMBER;
        case AMBIENCE_CHAMBER2:            return REVERB_CHAMBER2;
        
        // Verbity series
        case AMBIENCE_VERBITY:             return REVERB_VERBITY;
        case AMBIENCE_VERBITY2:            return REVERB_VERBITY2;
        
        // Aura
        case AMBIENCE_AURA:                return EFFECT_AURA;
        
        // Air variants (FILTER are canonical)
        case BRIGHTNESS_AIR:               return FILTER_AIR;
        case BRIGHTNESS_CAPACITOR:         return FILTER_CAPACITOR;
        case BRIGHTNESS_CAPACITOR2:        return FILTER_CAPACITOR2;
        
        // GuitarConditioner
        case AMPSIM_GUITARCONDITIONER:     return EFFECT_GUITARCONDITIONER;

        // PurestSquish remains loadable but is not shown in the picker.
        case AMPSIM_PURESTMULTISQUISH:     return AMPSIM_PURESTMULTISQUISH;
        
        // Gringer
        case LOFI_GRINGER:                 return EFFECT_GRINGER;
        
        // PocketVerbs
        case LOFI_POCKETVERBS:             return REVERB_POCKETVERBS;
        
        // GrooveWear
        case TONE_COLOUR_GROOVEWEAR:       return LOFI_GROOVEWEAR;
        
        // DitherFloat
        case LOFI_DITHERFLOAT:             return SAT_DITHERFLOAT;
        
        // BassDrive
        case BASS_BASSDRIVE:               return DIST_BASSDRIVE;
        
        // BassAmp
        case BASS_BASSAMP:                 return AMPSIM_BASSAMP;

        // Fracture
        case DIST_FRACTURE:                return EFFECT_FRACTURE;
        case DIST_FRACTURE2:               return EFFECT_FRACTURE2;

        // PurestGain
        case UTILITY_PURESTGAIN:           return DYN_PURESTGAIN;

        // StereoFX duplicate (same WinVST plugin)
        case EFFECT_STEREO_FX:             return UTILITY_STEREOFX;
        
        // Pear
        case TONE_COLOUR_PEAR:             return NOISE_PEAR;
        
        // Surge
        case TONE_COLOUR_SURGE:            return NOISE_SURGE;
        
        // All other types (canonical) return self
        default: return type;
    }
}

// ============================================================
// getSpec() - Parameter specifications for all algorithms
// ============================================================

AirwindowsSpec AirSuiteUniversal::getSpec(Type type) {
        type = getCanonicalType(type);
        return awspec::get_AirSuiteUniversal((int)type);
    }

// ============================================================
// Instance Methods
// ============================================================

// ========== Translation helpers: map AirSuiteUniversal::Type to category types ==========









































AirSuiteUniversal::AirSuiteUniversal()
    : m_reverbs(44100.0f), m_effects(44100.0f), m_ampSims(44100.0f), m_distortion(44100.0f),
      m_dynamics(44100.0f), m_saturation(44100.0f), m_tape(44100.0f), m_filters(44100.0f),
      m_biquad(44100.0f), m_clipping(44100.0f), m_dithers(44100.0f), m_utility(44100.0f),
      m_stereo(44100.0f), m_bass(44100.0f), m_lofi(44100.0f), m_noise(44100.0f),
      m_brightness(44100.0f), m_subtlety(44100.0f), m_toneColour(44100.0f), m_xyz(44100.0f) {
    m_runtimeIdentity = &getRuntimeIdentity(NONE);
}

int AirSuiteUniversal::resolveDispatchType(const RuntimeIdentity& identity) {
    switch (identity.category) {
        case Category::AMBIENCE:
            return awspec::dispatchTo_AMBIENCE((int)identity.canonicalType);
        case Category::AMPSIM:
            return awspec::dispatchTo_AMPSIM((int)identity.canonicalType);
        case Category::BASS:
            return awspec::dispatchTo_BASS((int)identity.canonicalType);
        case Category::BIQUAD:
            return awspec::dispatchTo_BIQUAD((int)identity.canonicalType);
        case Category::BRIGHTNESS:
            return awspec::dispatchTo_BRIGHTNESS((int)identity.canonicalType);
        case Category::CLIPPING:
            return awspec::dispatchTo_CLIPPING((int)identity.canonicalType);
        case Category::DIST:
            return awspec::dispatchTo_DIST((int)identity.canonicalType);
        case Category::DITHERS:
            return awspec::dispatchTo_DITHERS((int)identity.canonicalType);
        case Category::DYN:
            return awspec::dispatchTo_DYN((int)identity.canonicalType);
        case Category::EFFECT:
            return awspec::dispatchTo_EFFECT((int)identity.canonicalType);
        case Category::FILTER:
            return awspec::dispatchTo_FILTER((int)identity.canonicalType);
        case Category::LOFI:
            return awspec::dispatchTo_LOFI((int)identity.canonicalType);
        case Category::NOISE:
            return awspec::dispatchTo_NOISE((int)identity.canonicalType);
        case Category::REVERB:
            return awspec::dispatchTo_REVERB((int)identity.canonicalType);
        case Category::SAT:
            return awspec::dispatchTo_SAT((int)identity.canonicalType);
        case Category::STEREO:
            return awspec::dispatchTo_STEREO((int)identity.canonicalType);
        case Category::SUBTLETY:
            return awspec::dispatchTo_SUBTLETY((int)identity.canonicalType);
        case Category::TAPE:
            return awspec::dispatchTo_TAPE((int)identity.canonicalType);
        case Category::TONE_COLOUR:
            return awspec::dispatchTo_TONE_COLOUR((int)identity.canonicalType);
        case Category::UTILITY:
            return awspec::dispatchTo_UTILITY((int)identity.canonicalType);
        case Category::XYZ:
            return awspec::dispatchTo_XYZ((int)identity.canonicalType);
        default: return 0;
    }
}

























void AirSuiteUniversal::setSampleRate(double sr) {
    m_sampleRate = sr;
    m_reverbs.setSampleRate((float)sr);
    m_effects.setSampleRate((float)sr);
    m_ampSims.setSampleRate((float)sr);
    m_distortion.setSampleRate((float)sr);
    m_dynamics.setSampleRate((float)sr);
    m_saturation.setSampleRate((float)sr);
    m_tape.setSampleRate((float)sr);
    m_filters.setSampleRate((float)sr);
    m_biquad.setSampleRate((float)sr);
    m_clipping.setSampleRate((float)sr);
    m_dithers.setSampleRate((float)sr);
    m_utility.setSampleRate((float)sr);
    m_stereo.setSampleRate((float)sr);
    m_bass.setSampleRate((float)sr);
    m_lofi.setSampleRate((float)sr);
    m_noise.setSampleRate((float)sr);
    m_brightness.setSampleRate((float)sr);
    m_subtlety.setSampleRate((float)sr);
    m_toneColour.setSampleRate((float)sr);
    m_xyz.setSampleRate((float)sr);
}

void AirSuiteUniversal::setType(Type type) {
    m_runtimeIdentity = &getRuntimeIdentity(type);
    m_type = m_runtimeIdentity->canonicalType;
    m_cachedDispatchType = m_runtimeIdentity->dispatchType;
    const Category cat = m_runtimeIdentity->category;
    
    switch (cat) {
        case Category::REVERB: {
            // AirSuiteReverb takes Type as parameter to process(), no setType needed
            m_reverbs.setSampleRate((float)m_sampleRate);
            break;
        }
        case Category::EFFECT: {
            // AirSuiteEffect takes Type as parameter to process(), no setType needed
            m_effects.setSampleRate((float)m_sampleRate);
            break;
        }
        case Category::AMPSIM: {
            AirSuiteAmpSim::Type at = (AirSuiteAmpSim::Type)m_cachedDispatchType;
            if (at != AirSuiteAmpSim::NONE) {
                m_ampSims.setSampleRate((float)m_sampleRate);
            }
            break;
        }
        case Category::DIST: {
            AirSuiteDistortion::Type dt = (AirSuiteDistortion::Type)m_cachedDispatchType;
            if (dt != AirSuiteDistortion::NONE) {
                m_distortion.setSampleRate((float)m_sampleRate);
            }
            break;
        }
        case Category::DYN: {
            AirSuiteDynamics::Type dyt = (AirSuiteDynamics::Type)m_cachedDispatchType;
            if (dyt != AirSuiteDynamics::NONE) {
                m_dynamics.setAlgorithm((int)dyt);
                m_dynamics.setSampleRate((float)m_sampleRate);
            }
            break;
        }
        case Category::SAT: {
            AirSuiteSaturation::Type st = (AirSuiteSaturation::Type)m_cachedDispatchType;
            if (st != AirSuiteSaturation::NONE) {
                m_saturation.setSampleRate((float)m_sampleRate);
            }
            break;
        }
        case Category::TAPE: {
            AirSuiteTape::Type tt = (AirSuiteTape::Type)m_cachedDispatchType;
            if (tt != AirSuiteTape::NONE) {
                m_tape.setSampleRate((float)m_sampleRate);
            }
            break;
        }
        case Category::FILTER: {
            AirSuiteFilters::Type ft = (AirSuiteFilters::Type)m_cachedDispatchType;
            if (ft != AirSuiteFilters::NONE) {
                m_filters.setSampleRate((float)m_sampleRate);
            }
            break;
        }
        case Category::BIQUAD: {
            AirSuiteBiquad::Type bqt = (AirSuiteBiquad::Type)m_cachedDispatchType;
            if (bqt != AirSuiteBiquad::NONE) {
                m_biquad.setSampleRate((float)m_sampleRate);
                m_biquad.reset();
            }
            break;
        }
        case Category::CLIPPING: {
            AirSuiteClipping::Type ct = (AirSuiteClipping::Type)m_cachedDispatchType;
            if (ct != AirSuiteClipping::NONE) {
                m_clipping.setSampleRate((float)m_sampleRate);
                m_clipping.reset();
            }
            break;
        }
        case Category::DITHERS: {
            AirSuiteDithers::Type dit = (AirSuiteDithers::Type)m_cachedDispatchType;
            if (dit != AirSuiteDithers::NONE) {
                m_dithers.setAlgorithm((int)dit);
                m_dithers.setSampleRate((float)m_sampleRate);
            }
            break;
        }
        case Category::UTILITY: {
            AirSuiteUtility::Type ut = (AirSuiteUtility::Type)m_cachedDispatchType;
            if (ut != AirSuiteUtility::NONE) {
                m_utility.setSampleRate((float)m_sampleRate);
            }
            break;
        }
        case Category::STEREO: {
            AirSuiteStereo::Type st = (AirSuiteStereo::Type)m_cachedDispatchType;
            if (st != AirSuiteStereo::NONE) {
                m_stereo.setSampleRate((float)m_sampleRate);
            }
            break;
        }
        case Category::BASS: {
            AirSuiteBass::Type bt = (AirSuiteBass::Type)m_cachedDispatchType;
            if (bt != AirSuiteBass::NONE) {
                m_bass.setSampleRate((float)m_sampleRate);
            }
            break;
        }
        case Category::LOFI: {
            AirSuiteLoFi::Type lt = (AirSuiteLoFi::Type)m_cachedDispatchType;
            if (lt != AirSuiteLoFi::NONE) {
                m_lofi.setSampleRate((float)m_sampleRate);
                m_lofi.reset();
            }
            break;
        }
        case Category::NOISE: {
            AirSuiteNoise::Type nt = (AirSuiteNoise::Type)m_cachedDispatchType;
            if (nt != AirSuiteNoise::NONE) {
                m_noise.setSampleRate((float)m_sampleRate);
                m_noise.reset();
            }
            break;
        }
        case Category::BRIGHTNESS: {
            AirSuiteBrightness::Type bt = (AirSuiteBrightness::Type)m_cachedDispatchType;
            if (bt != AirSuiteBrightness::NONE) {
                m_brightness.setSampleRate((float)m_sampleRate);
            }
            break;
        }
        case Category::SUBTLETY: {
            AirSuiteSubtlety::Type st = (AirSuiteSubtlety::Type)m_cachedDispatchType;
            if (st != AirSuiteSubtlety::NONE) {
                m_subtlety.setSampleRate((float)m_sampleRate);
                m_subtlety.reset();
            }
            break;
        }
        case Category::TONE_COLOUR: {
            m_toneColour.setSampleRate((float)m_sampleRate);
            m_toneColour.reset();
            break;
        }
        case Category::XYZ: {
            m_xyz.setSampleRate((float)m_sampleRate);
            m_xyz.reset();
            break;
        }
        default:
            break;
    }
}

void AirSuiteUniversal::reset() {
    // Reset ONLY the active category. The category Impls are heavyweight:
    // AmpSim/Dynamics eagerly own ~40 AudioEffectX instances and their reset()
    // deletes and re-constructs every one of them. Fanning out to all 20
    // categories meant a single slot reset churned ~370 allocations, most of
    // them for categories this slot never selected. An inactive category has
    // no live state to clear, so restricting the reset to the active one is
    // both sufficient and allocation-proportional to the actual effect.
    if (m_type == NONE || m_runtimeIdentity == nullptr) return;
    switch (m_runtimeIdentity->category) {
        case Category::REVERB:      m_reverbs.reset();      break;
        case Category::EFFECT:      m_effects.reset();      break;
        case Category::AMPSIM:      m_ampSims.reset();      break;
        case Category::DIST:        m_distortion.reset();   break;
        case Category::DYN:         m_dynamics.reset();     break;
        case Category::SAT:         m_saturation.reset();   break;
        case Category::TAPE:        m_tape.reset();         break;
        case Category::FILTER:      m_filters.reset();      break;
        case Category::BIQUAD:      m_biquad.reset();       break;
        case Category::CLIPPING:    m_clipping.reset();     break;
        case Category::DITHERS:     m_dithers.reset();      break;
        case Category::UTILITY:     m_utility.reset();      break;
        case Category::STEREO:      m_stereo.reset();       break;
        case Category::BASS:        m_bass.reset();         break;
        case Category::LOFI:        m_lofi.reset();         break;
        case Category::NOISE:       m_noise.reset();        break;
        case Category::BRIGHTNESS:  m_brightness.reset();   break;
        case Category::SUBTLETY:    m_subtlety.reset();     break;
        case Category::TONE_COLOUR: m_toneColour.reset();   break;
        case Category::XYZ:         m_xyz.reset();          break;
        default: break;
    }
}


void AirSuiteUniversal::process(float& L, float& R, const float params[13]) {
    if (m_type == NONE || m_runtimeIdentity == nullptr) return;

    // Phase 1 Policy: Pass parameters directly without modification.
    // Plugins with internal wet parameters use their passed values.
    // External slot-level mixing handles the overall dry/wet balance.
    // Do NOT override internal wet params - respect the plugin's internal design.
    // Note: const_cast is safe here because we pass params unchanged to category processors.
    float* mutableParams = const_cast<float*>(params);

    // Delegate to the appropriate category class instance
    const RuntimeIdentity& identity = *m_runtimeIdentity;
    switch (identity.category) {
        case Category::REVERB: {
            AirSuiteReverb::Type rt = (AirSuiteReverb::Type)m_cachedDispatchType;
            m_reverbs.process(L, R, rt, mutableParams);
            break;
        }
        case Category::EFFECT: {
            AirSuiteEffect::Type et = (AirSuiteEffect::Type)m_cachedDispatchType;
            m_effects.process(L, R, et, mutableParams);
            break;
        }
        case Category::AMPSIM: {
            AirSuiteAmpSim::Type at = (AirSuiteAmpSim::Type)m_cachedDispatchType;
            m_ampSims.process(L, R, at, mutableParams);
            break;
        }
        case Category::DIST: {
            AirSuiteDistortion::Type dt = (AirSuiteDistortion::Type)m_cachedDispatchType;
            m_distortion.process(L, R, dt, mutableParams);
            break;
        }
        case Category::DYN: {
            m_dynamics.process(L, R, mutableParams);
            break;
        }
        case Category::SAT: {
            AirSuiteSaturation::Type st = (AirSuiteSaturation::Type)m_cachedDispatchType;
            m_saturation.process(L, R, st, mutableParams);
            break;
        }
        case Category::TAPE: {
            AirSuiteTape::Type tt = (AirSuiteTape::Type)m_cachedDispatchType;
            m_tape.process(L, R, tt, mutableParams);
            break;
        }
        case Category::FILTER: {
            AirSuiteFilters::Type ft = (AirSuiteFilters::Type)m_cachedDispatchType;
            m_filters.process(L, R, ft, mutableParams);
            break;
        }
        case Category::BIQUAD: {
            AirSuiteBiquad::Type bqt = (AirSuiteBiquad::Type)m_cachedDispatchType;
            m_biquad.process(L, R, bqt, mutableParams);
            break;
        }
        case Category::CLIPPING: {
            AirSuiteClipping::Type ct = (AirSuiteClipping::Type)m_cachedDispatchType;
            m_clipping.process(L, R, ct, mutableParams);
            break;
        }
        case Category::DITHERS: {
            AirSuiteDithers::Type dit = (AirSuiteDithers::Type)m_cachedDispatchType;
            m_dithers.process(L, R, dit, mutableParams);
            break;
        }
        case Category::UTILITY: {
            AirSuiteUtility::Type ut = (AirSuiteUtility::Type)m_cachedDispatchType;
            m_utility.process(L, R, ut, mutableParams);
            break;
        }
        case Category::STEREO: {
            AirSuiteStereo::Type st = (AirSuiteStereo::Type)m_cachedDispatchType;
            m_stereo.process(L, R, st, mutableParams);
            break;
        }
        case Category::BASS: {
            AirSuiteBass::Type bt = (AirSuiteBass::Type)m_cachedDispatchType;
            m_bass.process(L, R, bt, mutableParams);
            break;
        }
        case Category::LOFI: {
            AirSuiteLoFi::Type lt = (AirSuiteLoFi::Type)m_cachedDispatchType;
            m_lofi.process(L, R, lt, mutableParams);
            break;
        }
        case Category::NOISE: {
            AirSuiteNoise::Type nt = (AirSuiteNoise::Type)m_cachedDispatchType;
            m_noise.process(L, R, nt, mutableParams);
            break;
        }
        case Category::BRIGHTNESS: {
            AirSuiteBrightness::Type bt = (AirSuiteBrightness::Type)m_cachedDispatchType;
            m_brightness.process(L, R, bt, mutableParams);
            break;
        }
        case Category::SUBTLETY: {
            AirSuiteSubtlety::Type st = (AirSuiteSubtlety::Type)m_cachedDispatchType;
            m_subtlety.process(L, R, st, mutableParams);
            break;
        }
        case Category::TONE_COLOUR: {
            AirSuiteToneColour::Type tct = (AirSuiteToneColour::Type)m_cachedDispatchType;
            m_toneColour.process(L, R, tct, mutableParams);
            break;
        }
        case Category::XYZ: {
            m_xyz.process(L, R, (AirSuiteXYZ::Type)m_cachedDispatchType, mutableParams);
            break;
        }
        default:
            break;
    }

    // Validate output
    if (!std::isfinite((double)L)) L = 0.0f;
    if (!std::isfinite((double)R)) R = 0.0f;
}

// ============================================================
// isInCategory() - Check if a type belongs to a category
// ============================================================
// isInCategory() - Check if a type belongs to a category
// ============================================================
bool AirSuiteUniversal::isInCategory(Type t, Category c) {
    if (getCategory(t) == c) return true;
    // Cross-category overrides
    if (t == FILTER_BIQUADDOUBLE && c == Category::BIQUAD) return true;
    if ((t == CLIP_ADCLIP7 || t == CLIP_ADCLIP8) && c == Category::DIST) return true;
    if (t == DIST_HARDVACUUM && c == Category::CLIPPING) return true;
    // XYZ family: expose core XYZ filters in Filter view too.
    if (c == Category::FILTER) {
        if (t == XYZ_XBANDPASS || t == XYZ_YBANDPASS || t == XYZ_ZBANDPASS || t == XYZ_ZHIGHPASS) return true;
    }
    // TONE_COLOUR cross-listings
    if (c == Category::TONE_COLOUR) {
        if (t == EFFECT_GLITCHSHIFTER) return true;
        if (t == EFFECT_GALACTICVIBE) return true;
        if (t == REVERB_GALACTIC) return true;
        if (t == REVERB_GALACTIC2) return true;
        if (t == REVERB_GALACTIC3) return true;
    }
    return false;
}

// ============================================================
// Phase 3: getExtractedParam() - Runtime metadata access layer
// ============================================================
// Returns canonical parameter metadata from WinVST extraction for a given type and index.
// Returns nullptr if type is not found or paramIndex is out of range.
// 
// This layer provides runtime lookup of canonical extracted metadata (raw_name, label)
// for UI display formatting. Covers all 245 Type values with mapped arrays or nullptr fallback.
// To add a new type:
//   1. Add a case clause for the Type enum value
//   2. Reference the corresponding kAirwindowsParams_PluginName array
//   3. Test with representative parameters
//
const AirwindowsExtractedParam* AirSuiteUniversal::getExtractedParam(Type type, int paramIndex) {
    // Large switch statement mapping Type to extracted parameter arrays (all 245 types)
    switch (type) {
        // REVERB (44 algorithms, 1..44)
        case REVERB_BRIGHTAMBIENCE: {
            auto& arr = kAirwindowsParams_BrightAmbience;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case REVERB_BRIGHTAMBIENCE2: {
            auto& arr = kAirwindowsParams_BrightAmbience2;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case REVERB_BRIGHTAMBIENCE3: {
            auto& arr = kAirwindowsParams_BrightAmbience3;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case REVERB_CHAMBER: {
            auto& arr = kAirwindowsParams_Chamber;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case REVERB_CHAMBER2: {
            auto& arr = kAirwindowsParams_Chamber2;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case REVERB_CLEARCOAT: {
            auto& arr = kAirwindowsParams_ClearCoat;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case REVERB_CREAMCOAT: {
            auto& arr = kAirwindowsParams_CreamCoat;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case REVERB_CRUNCHCOAT: {
            auto& arr = kAirwindowsParams_CrunchCoat;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case REVERB_GALACTIC: {
            auto& arr = kAirwindowsParams_Galactic;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case REVERB_GALACTIC2: {
            auto& arr = kAirwindowsParams_Galactic2;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case REVERB_GALACTIC3: {
            auto& arr = kAirwindowsParams_Galactic3;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case REVERB_INFINITY_VERB: {
            auto& arr = kAirwindowsParams_Infinity;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case REVERB_INFINITY2_VERB: {
            auto& arr = kAirwindowsParams_Infinity2;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case REVERB_KALIENSPACESHIP: {
            auto& arr = kAirwindowsParams_kAlienSpaceship;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case REVERB_KBEYOND: {
            auto& arr = kAirwindowsParams_kBeyond;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case REVERB_KCATHEDRAL: {
            auto& arr = kAirwindowsParams_kCathedral;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case REVERB_KCATHEDRAL2: {
            auto& arr = kAirwindowsParams_kCathedral2;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case REVERB_KCATHEDRAL3: {
            auto& arr = kAirwindowsParams_kCathedral3;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case REVERB_KCATHEDRAL4: {
            auto& arr = kAirwindowsParams_kCathedral4;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case REVERB_KCATHEDRAL5: {
            auto& arr = kAirwindowsParams_kCathedral5;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case REVERB_KCHAMBERAR: {
            auto& arr = kAirwindowsParams_kChamberAR;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case REVERB_KCOSMOS: {
            auto& arr = kAirwindowsParams_kCosmos;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case REVERB_KGUITARHALL: {
            auto& arr = kAirwindowsParams_kGuitarHall;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case REVERB_KGUITARHALL2: {
            auto& arr = kAirwindowsParams_kGuitarHall2;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case REVERB_KPLATE140: {
            auto& arr = kAirwindowsParams_kPlate140;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case REVERB_KPLATE240: {
            auto& arr = kAirwindowsParams_kPlate240;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case REVERB_KPLATEA: {
            auto& arr = kAirwindowsParams_kPlateA;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case REVERB_KPLATEB: {
            auto& arr = kAirwindowsParams_kPlateB;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case REVERB_KPLATEC: {
            auto& arr = kAirwindowsParams_kPlateC;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case REVERB_KPLATED: {
            auto& arr = kAirwindowsParams_kPlateD;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case REVERB_KSTATION: {
            auto& arr = kAirwindowsParams_kStation;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case REVERB_KWOODROOM: {
            auto& arr = kAirwindowsParams_kWoodRoom;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case REVERB_MATRIXVERB: {
            auto& arr = kAirwindowsParams_MatrixVerb;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case REVERB_MV: {
            auto& arr = kAirwindowsParams_MV;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case REVERB_MV2: {
            auto& arr = kAirwindowsParams_MV2;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case REVERB_NONLINEARSPACE: {
            auto& arr = kAirwindowsParams_NonlinearSpace;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case REVERB_POCKETVERBS: {
            auto& arr = kAirwindowsParams_PocketVerbs;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case REVERB_REVERB: {
            auto& arr = kAirwindowsParams_Reverb;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case REVERB_VERBITY: {
            auto& arr = kAirwindowsParams_Verbity;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case REVERB_VERBITY2: {
            auto& arr = kAirwindowsParams_Verbity2;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case REVERB_VERBSIXES: {
            auto& arr = kAirwindowsParams_VerbSixes;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case REVERB_VERBTHIC: {
            auto& arr = kAirwindowsParams_VerbThic;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case REVERB_VERBTINY: {
            auto& arr = kAirwindowsParams_VerbTiny;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        
        // EFFECT (22 algorithms, 45..66)
        case EFFECT_AURA: {
            auto& arr = kAirwindowsParams_Aura;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_CLOUDCOAT: {
            auto& arr = kAirwindowsParams_CloudCoat;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_DISINTEGRATE: {
            auto& arr = kAirwindowsParams_Disintegrate;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_ENERGY: {
            auto& arr = kAirwindowsParams_Energy;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_ENERGY2: {
            auto& arr = kAirwindowsParams_Energy2;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_EXCITER: {
            auto& arr = kAirwindowsParams_Exciter;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_FACET: {
            auto& arr = kAirwindowsParams_Facet;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_FRACTURE: {
            auto& arr = kAirwindowsParams_Fracture;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_FRACTURE2: {
            auto& arr = kAirwindowsParams_Fracture2;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_GALACTICVIBE: {
            auto& arr = kAirwindowsParams_GalacticVibe;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_GLITCHSHIFTER: {
            auto& arr = kAirwindowsParams_GlitchShifter;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_GRINGER: {
            auto& arr = kAirwindowsParams_Gringer;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_GUITARCONDITIONER: {
            auto& arr = kAirwindowsParams_GuitarConditioner;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_PAFNUTY2: {
            auto& arr = kAirwindowsParams_Pafnuty2;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_PITCHNASTY: {
            auto& arr = kAirwindowsParams_PitchNasty;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_POWERSAG2: {
            auto& arr = kAirwindowsParams_PowerSag2;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_PREPONDERANT: {
            auto& arr = kAirwindowsParams_Preponderant;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_RINGMODULATOR: {
            auto& arr = kAirwindowsParams_RingModulator;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_TAKECARE: {
            auto& arr = kAirwindowsParams_TakeCare;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_TREMOLO: {
            auto& arr = kAirwindowsParams_Tremolo;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_TREMOSQUARE: {
            auto& arr = kAirwindowsParams_TremoSquare;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_TRIANGLIZER: {
            auto& arr = kAirwindowsParams_Trianglizer;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        
        // EFFECT (4 additional effects, 123..126)
        case EFFECT_ELECTROHAT: {
            auto& arr = kAirwindowsParams_ElectroHat;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_FLIPITY: {
            auto& arr = kAirwindowsParams_Flipity;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_MONITORING: {
            auto& arr = kAirwindowsParams_Monitoring;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_MONOAM: {
            auto& arr = kAirwindowsParams_MoNoam;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        
        // AMPSIM (10 algorithms, 67..76)
        case AMPSIM_CABS: {
            auto& arr = kAirwindowsParams_Cabs;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case AMPSIM_BASSAMP: {
            auto& arr = kAirwindowsParams_BassAmp;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case AMPSIM_BIGAMP: {
            auto& arr = kAirwindowsParams_BigAmp;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case AMPSIM_FIREAMP: {
            auto& arr = kAirwindowsParams_FireAmp;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case AMPSIM_GRINDAMP: {
            auto& arr = kAirwindowsParams_GrindAmp;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case AMPSIM_GUITARCONDITIONER: {
            auto& arr = kAirwindowsParams_GuitarConditioner;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case AMPSIM_PURESTMULTISQUISH: {
            auto& arr = kAirwindowsParams_PurestSquish;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case AMPSIM_TONESLANT: {
            auto& arr = kAirwindowsParams_ToneSlant;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case AMPSIM_LEADAMP: {
            auto& arr = kAirwindowsParams_LeadAmp;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case AMPSIM_LILAMPL: {
            auto& arr = kAirwindowsParams_LilAmp;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        
        // DISTORTION (13 algorithms, 77..89)
        case DIST_BASSDRIVE: {
            auto& arr = kAirwindowsParams_BassDrive;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DIST_DENSITY: {
            auto& arr = kAirwindowsParams_Density;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DIST_DENSITY2: {
            auto& arr = kAirwindowsParams_Density2;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DIST_DENSITY3: {
            auto& arr = kAirwindowsParams_Density3;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DIST_DISTORTION: {
            auto& arr = kAirwindowsParams_Distortion;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DIST_DRIVE: {
            auto& arr = kAirwindowsParams_Drive;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DIST_FRACTURE: {
            auto& arr = kAirwindowsParams_Fracture;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DIST_FRACTURE2: {
            auto& arr = kAirwindowsParams_Fracture2;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DIST_HARDVACUUM: {
            auto& arr = kAirwindowsParams_HardVacuum;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DIST_PURESTDRIVE: {
            auto& arr = kAirwindowsParams_PurestDrive;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DIST_SPIRAL: {
            auto& arr = kAirwindowsParams_Spiral;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DIST_SPIRAL2: {
            auto& arr = kAirwindowsParams_Spiral2;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DIST_BITGLITTER: {
            auto& arr = kAirwindowsParams_BitGlitter;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        
        // DYNAMICS (6 algorithms, 90..95)
        case DYN_PRESSURE4: {
            auto& arr = kAirwindowsParams_Pressure4;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DYN_PRESSURE5: {
            auto& arr = kAirwindowsParams_Pressure5;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DYN_VARIMU: {
            auto& arr = kAirwindowsParams_VariMu;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DYN_POINT: {
            auto& arr = kAirwindowsParams_Point;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DYN_ACCELERATION: {
            auto& arr = kAirwindowsParams_Acceleration;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DYN_PURESTGAIN: {
            auto& arr = kAirwindowsParams_PurestGain;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        
        // SATURATION (7 algorithms, 96..102)
        case SAT_BUILDATPDF: {
            auto& arr = kAirwindowsParams_BuildATPDF;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SAT_DITHERFLOAT: {
            auto& arr = kAirwindowsParams_DitherFloat;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SAT_POWERSAG: {
            auto& arr = kAirwindowsParams_PowerSag;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SAT_NIKOLA: {
            auto& arr = kAirwindowsParams_Nikola;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SAT_SHORTBUSS: {
            auto& arr = kAirwindowsParams_ShortBuss;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SAT_COMPRESATURATOR: {
            auto& arr = kAirwindowsParams_Compresaturator;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        
        // TAPE (8 algorithms, 103..110)
        case TAPE_TOTAPE6: {
            auto& arr = kAirwindowsParams_ToTape6;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case TAPE_IRONOXIDE5: {
            auto& arr = kAirwindowsParams_IronOxide5;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case TAPE_FLUTTER2: {
            auto& arr = kAirwindowsParams_Flutter2;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case TAPE_DUBLY: {
            auto& arr = kAirwindowsParams_Dubly;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case TAPE_DUBLY2: {
            auto& arr = kAirwindowsParams_Dubly2;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case TAPE_DUBLY3: {
            auto& arr = kAirwindowsParams_Dubly3;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case TAPE_TOTAPE5: {
            auto& arr = kAirwindowsParams_ToTape5;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case TAPE_TOVINYL4: {
            auto& arr = kAirwindowsParams_ToVinyl4;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        
        // FILTERS (11 algorithms, 112..122)
        case FILTER_AIR: {
            auto& arr = kAirwindowsParams_Air;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case FILTER_CAPACITOR2: {
            auto& arr = kAirwindowsParams_Capacitor2;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case FILTER_HIGHPASS2: {
            auto& arr = kAirwindowsParams_Highpass2;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case FILTER_LOWPASS2: {
            auto& arr = kAirwindowsParams_Lowpass2;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case FILTER_AIR2: {
            auto& arr = kAirwindowsParams_Air2;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case FILTER_AIR3: {
            auto& arr = kAirwindowsParams_Air3;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case FILTER_AIR4: {
            auto& arr = kAirwindowsParams_Air4;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case FILTER_CAPACITOR: {
            auto& arr = kAirwindowsParams_Capacitor;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case FILTER_HIGHPASS: {
            auto& arr = kAirwindowsParams_Highpass;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case FILTER_LOWPASS: {
            auto& arr = kAirwindowsParams_Lowpass;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case FILTER_BIQUADDOUBLE: {
            auto& arr = kAirwindowsParams_BiquadDouble;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        
        // BIQUAD (8 algorithms, 127..134)
        case BIQUAD_BIQUAD: {
            auto& arr = kAirwindowsParams_Biquad;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case BIQUAD_BIQUAD2: {
            auto& arr = kAirwindowsParams_Biquad2;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case BIQUAD_BIQUADHILO: {
            auto& arr = kAirwindowsParams_BiquadHiLo;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case BIQUAD_BIQUADNONLIN: {
            auto& arr = kAirwindowsParams_BiquadNonLin;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case BIQUAD_BIQUADONEHALF: {
            auto& arr = kAirwindowsParams_BiquadOneHalf;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case BIQUAD_BIQUADPLUS: {
            auto& arr = kAirwindowsParams_BiquadPlus;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case BIQUAD_BIQUADSTACK: {
            auto& arr = kAirwindowsParams_BiquadStack;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case BIQUAD_BIQUADTRIPLE: {
            auto& arr = kAirwindowsParams_BiquadTriple;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        
        // CLIPPING (5 algorithms, 135..139)
        case CLIP_ADCLIP7: {
            auto& arr = kAirwindowsParams_ADClip7;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case CLIP_ADCLIP8: {
            auto& arr = kAirwindowsParams_ADClip8;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case CLIP_CLIPONLY: {
            auto& arr = kAirwindowsParams_ClipOnly;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case CLIP_CLIPONLY2: {
            auto& arr = kAirwindowsParams_ClipOnly2;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case CLIP_CLIPSOFTLY: {
            auto& arr = kAirwindowsParams_ClipSoftly;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        
        // DITHERS (11 algorithms, 140..150)
        case DITHER_DITHERMEDISKERS: {
            auto& arr = kAirwindowsParams_DitherMeDiskers;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DITHER_DITHERMTIMBERS: {
            auto& arr = kAirwindowsParams_DitherMeTimbers;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DITHER_HIGHGLOSS: {
            auto& arr = kAirwindowsParams_HighGlossDither;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DITHER_NATURALIZE: {
            auto& arr = kAirwindowsParams_NaturalizeDither;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DITHER_NODEDITHER: {
            auto& arr = kAirwindowsParams_NodeDither;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DITHER_NJAD: {
            auto& arr = kAirwindowsParams_NotJustAnotherDither;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DITHER_PAULDITHER: {
            auto& arr = kAirwindowsParams_PaulDither;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DITHER_SPATIALIZE: {
            auto& arr = kAirwindowsParams_SpatializeDither;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DITHER_TAPEDITHER: {
            auto& arr = kAirwindowsParams_TapeDither;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DITHER_TPDFDITHER: {
            auto& arr = kAirwindowsParams_TPDFDither;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DITHER_VINYLDITHER: {
            auto& arr = kAirwindowsParams_VinylDither;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        
        // UTILITY (14 algorithms, 151..164)
        case UTILITY_AVERAGE: {
            auto& arr = kAirwindowsParams_Average;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case UTILITY_BALANCED: {
            auto& arr = kAirwindowsParams_Balanced;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case UTILITY_BITSHIFTGAIN: {
            auto& arr = kAirwindowsParams_BitShiftGain;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case UTILITY_CSTRIP2: {
            auto& arr = kAirwindowsParams_CStrip2;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case UTILITY_PURESTFADE: {
            auto& arr = kAirwindowsParams_PurestFade;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case UTILITY_PURESTGAIN: {
            auto& arr = kAirwindowsParams_PurestGain;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case UTILITY_PURESTWARM: {
            auto& arr = kAirwindowsParams_PurestWarm;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case UTILITY_RIGHTEOUS4: {
            auto& arr = kAirwindowsParams_Righteous4;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case UTILITY_SLEW: {
            auto& arr = kAirwindowsParams_Slew;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case UTILITY_SLEW2: {
            auto& arr = kAirwindowsParams_Slew2;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case UTILITY_SLEWONLY: {
            auto& arr = kAirwindowsParams_SlewOnly;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case UTILITY_STEREOFX: {
            auto& arr = kAirwindowsParams_StereoFX;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case UTILITY_STUDIOTAN: {
            auto& arr = kAirwindowsParams_StudioTan;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case UTILITY_WEIGHT: {
            auto& arr = kAirwindowsParams_Weight;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        
        // STEREO (10 algorithms, 165..174)
        case STEREO_CHANNEL4: {
            auto& arr = kAirwindowsParams_Channel4;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case STEREO_CHANNEL5: {
            auto& arr = kAirwindowsParams_Channel5;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case STEREO_CHANNEL6: {
            auto& arr = kAirwindowsParams_Channel6;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case STEREO_CHANNEL7: {
            auto& arr = kAirwindowsParams_Channel7;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case STEREO_CHANNEL8: {
            auto& arr = kAirwindowsParams_Channel8;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case STEREO_CHANNEL9: {
            auto& arr = kAirwindowsParams_Channel9;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case STEREO_MIDSIDE: {
            auto& arr = kAirwindowsParams_MidSide;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case STEREO_PHASENUDGE: {
            auto& arr = kAirwindowsParams_PhaseNudge;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case STEREO_SRSLY: {
            auto& arr = kAirwindowsParams_Srsly;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case STEREO_WIDER: {
            auto& arr = kAirwindowsParams_Wider;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        
        // BASS (5 algorithms, 175..179)
        case BASS_BASSAMP: {
            auto& arr = kAirwindowsParams_BassAmp;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case BASS_BASSDRIVE: {
            auto& arr = kAirwindowsParams_BassDrive;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case BASS_BASSKIT: {
            auto& arr = kAirwindowsParams_BassKit;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case BASS_DUBSUB: {
            auto& arr = kAirwindowsParams_DubSub;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case BASS_SUBSONLY: {
            auto& arr = kAirwindowsParams_SubsOnly;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        
        // LOFI (12 algorithms, 180..191)
        case LOFI_DESK: {
            auto& arr = kAirwindowsParams_Desk;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case LOFI_DESK4: {
            auto& arr = kAirwindowsParams_Desk4;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case LOFI_DIGITALBLACK: {
            auto& arr = kAirwindowsParams_DigitalBlack;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case LOFI_DITHERFLOAT: {
            auto& arr = kAirwindowsParams_DitherFloat;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case LOFI_FATHOMFIVE: {
            auto& arr = kAirwindowsParams_FathomFive;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case LOFI_GRINGER: {
            auto& arr = kAirwindowsParams_Gringer;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case LOFI_GROOVEWEAR: {
            auto& arr = kAirwindowsParams_GrooveWear;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case LOFI_LOUD: {
            auto& arr = kAirwindowsParams_Loud;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case LOFI_MACKITY: {
            auto& arr = kAirwindowsParams_Mackity;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case LOFI_NCSEVENTEEN: {
            auto& arr = kAirwindowsParams_NCSeventeen;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case LOFI_POCKETVERBS: {
            auto& arr = kAirwindowsParams_PocketVerbs;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case LOFI_SAMPLEDELAY: {
            auto& arr = kAirwindowsParams_SampleDelay;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        
        // NOISE (6 algorithms, 192..197)
        case NOISE_DARKNOISE: {
            auto& arr = kAirwindowsParams_DarkNoise;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case NOISE_HOLT: {
            auto& arr = kAirwindowsParams_Holt;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case NOISE_NOISE: {
            auto& arr = kAirwindowsParams_Noise;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case NOISE_PEAR: {
            auto& arr = kAirwindowsParams_Pear;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case NOISE_SURGE: {
            auto& arr = kAirwindowsParams_Surge;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case NOISE_SURGETIDE: {
            auto& arr = kAirwindowsParams_SurgeTide;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        
        // AMBIENCE (8 algorithms, 198..205)
        case AMBIENCE_AURA: {
            auto& arr = kAirwindowsParams_Aura;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case AMBIENCE_BRIGHTAMBIENCE: {
            auto& arr = kAirwindowsParams_BrightAmbience;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case AMBIENCE_BRIGHTAMBIENCE2: {
            auto& arr = kAirwindowsParams_BrightAmbience2;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case AMBIENCE_BRIGHTAMBIENCE3: {
            auto& arr = kAirwindowsParams_BrightAmbience3;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case AMBIENCE_CHAMBER: {
            auto& arr = kAirwindowsParams_Chamber;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case AMBIENCE_CHAMBER2: {
            auto& arr = kAirwindowsParams_Chamber2;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case AMBIENCE_VERBITY: {
            auto& arr = kAirwindowsParams_Verbity;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case AMBIENCE_VERBITY2: {
            auto& arr = kAirwindowsParams_Verbity2;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        
        // BRIGHTNESS (6 algorithms, 206..211)
        case BRIGHTNESS_AIR: {
            auto& arr = kAirwindowsParams_Air;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case BRIGHTNESS_APICOLYPSE: {
            auto& arr = kAirwindowsParams_Apicolypse;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case BRIGHTNESS_CAPACITOR: {
            auto& arr = kAirwindowsParams_Capacitor;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case BRIGHTNESS_CAPACITOR2: {
            auto& arr = kAirwindowsParams_Capacitor2;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case BRIGHTNESS_CRYSTAL: {
            auto& arr = kAirwindowsParams_Crystal;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case BRIGHTNESS_PURESTAIR: {
            auto& arr = kAirwindowsParams_PurestAir;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        
        // SUBTLETY (15 algorithms, 212..226)
        case SUBTLETY_BAXANDALL: {
            auto& arr = kAirwindowsParams_Baxandall;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SUBTLETY_BAXANDALL2: {
            auto& arr = kAirwindowsParams_Baxandall2;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SUBTLETY_BAXANDALL3: {
            auto& arr = kAirwindowsParams_Baxandall3;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SUBTLETY_CALIBRE: {
            auto& arr = kAirwindowsParams_Calibre;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SUBTLETY_COILS: {
            auto& arr = kAirwindowsParams_Coils;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SUBTLETY_COILS2: {
            auto& arr = kAirwindowsParams_Coils2;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SUBTLETY_INTERSTAGE: {
            auto& arr = kAirwindowsParams_Interstage;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SUBTLETY_MOJO: {
            auto& arr = kAirwindowsParams_Mojo;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SUBTLETY_PLATINUMSLEW: {
            auto& arr = kAirwindowsParams_PlatinumSlew;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SUBTLETY_SLEWSONIC: {
            auto& arr = kAirwindowsParams_SlewSonic;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SUBTLETY_SMOOTH: {
            auto& arr = kAirwindowsParams_Smooth;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SUBTLETY_SMOOTHEQ: {
            auto& arr = kAirwindowsParams_SmoothEQ;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SUBTLETY_TUBEDESK: {
            auto& arr = kAirwindowsParams_TubeDesk;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SUBTLETY_UNBOX: {
            auto& arr = kAirwindowsParams_UnBox;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        
        // TONE_COLOUR (10 algorithms, 226..235)
        case TONE_COLOUR_FOCUS: {
            auto& arr = kAirwindowsParams_Focus;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case TONE_COLOUR_GROOVEWEAR: {
            auto& arr = kAirwindowsParams_GrooveWear;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case TONE_COLOUR_MELT: {
            auto& arr = kAirwindowsParams_Melt;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case TONE_COLOUR_ONECORNERCLIP: {
            auto& arr = kAirwindowsParams_OneCornerClip;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case TONE_COLOUR_PAFNUTY: {
            auto& arr = kAirwindowsParams_Pafnuty;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case TONE_COLOUR_PEAR: {
            auto& arr = kAirwindowsParams_Pear;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case TONE_COLOUR_PEAR2: {
            auto& arr = kAirwindowsParams_Pear2;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case TONE_COLOUR_SURGE: {
            auto& arr = kAirwindowsParams_Surge;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case TONE_COLOUR_SWEETEN: {
            auto& arr = kAirwindowsParams_Sweeten;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        
        // XYZ (10 algorithms, 236..245)
        case XYZ_XBANDPASS: {
            auto& arr = kAirwindowsParams_XBandpass;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case XYZ_XHIGHPASS: {
            auto& arr = kAirwindowsParams_XHighpass;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case XYZ_XLOWPASS: {
            auto& arr = kAirwindowsParams_XLowpass;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case XYZ_XNOTCH: {
            auto& arr = kAirwindowsParams_XNotch;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case XYZ_XREGION: {
            auto& arr = kAirwindowsParams_XRegion;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case XYZ_YBANDPASS: {
            auto& arr = kAirwindowsParams_YBandpass;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case XYZ_YHIGHPASS: {
            auto& arr = kAirwindowsParams_YHighpass;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case XYZ_YLOWPASS: {
            auto& arr = kAirwindowsParams_YLowpass;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case XYZ_YNOTCH: {
            auto& arr = kAirwindowsParams_YNotch;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case XYZ_ZBANDPASS: {
            auto& arr = kAirwindowsParams_ZBandpass;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case XYZ_ZHIGHPASS: {
            auto& arr = kAirwindowsParams_ZHighpass;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }

        // === NEW: 64 plugins (extracted params where available) ===
        case SAT_CONSOLE0_BUSS: {
            auto& arr = kAirwindowsParams_Console0Buss;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SAT_CONSOLE0_CHANNEL: {
            auto& arr = kAirwindowsParams_Console0Channel;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SAT_CONSOLE4_BUSS: {
            auto& arr = kAirwindowsParams_Console4Buss;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SAT_CONSOLE4_CHANNEL: {
            auto& arr = kAirwindowsParams_Console4Channel;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SAT_CONSOLE5_BUSS: {
            auto& arr = kAirwindowsParams_Console5Buss;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SAT_CONSOLE5_CHANNEL: {
            auto& arr = kAirwindowsParams_Console5Channel;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SAT_CONSOLE5_DARKCH: {
            auto& arr = kAirwindowsParams_Console5DarkCh;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SAT_CONSOLE6_BUSS: {
            auto& arr = kAirwindowsParams_Console6Buss;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SAT_CONSOLE6_CHANNEL: {
            auto& arr = kAirwindowsParams_Console6Channel;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SAT_CONSOLE7_BUSS: {
            auto& arr = kAirwindowsParams_Console7Buss;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SAT_CONSOLE7_CASCADE: {
            auto& arr = kAirwindowsParams_Console7Cascade;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SAT_CONSOLE7_CHANNEL: {
            auto& arr = kAirwindowsParams_Console7Channel;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SAT_CONSOLE7_CRUNCH: {
            auto& arr = kAirwindowsParams_Console7Crunch;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SAT_CONSOLE8_BUSSHYPE: {
            auto& arr = kAirwindowsParams_Console8BussHype;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SAT_CONSOLE8_BUSSIN: {
            auto& arr = kAirwindowsParams_Console8BussIn;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SAT_CONSOLE8_BUSSOUT: {
            auto& arr = kAirwindowsParams_Console8BussOut;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SAT_CONSOLE8_CHANNELHYPE: {
            auto& arr = kAirwindowsParams_Console8ChannelHype;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SAT_CONSOLE8_CHANNELIN: {
            auto& arr = kAirwindowsParams_Console8ChannelIn;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SAT_CONSOLE8_CHANNELOUT: {
            auto& arr = kAirwindowsParams_Console8ChannelOut;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SAT_CONSOLE8_LITEBUSS: {
            auto& arr = kAirwindowsParams_Console8LiteBuss;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SAT_CONSOLE8_LITECHANNEL: {
            auto& arr = kAirwindowsParams_Console8LiteChannel;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SAT_CONSOLE8_SUBHYPE: {
            auto& arr = kAirwindowsParams_Console8SubHype;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SAT_CONSOLE8_SUBIN: {
            auto& arr = kAirwindowsParams_Console8SubIn;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SAT_CONSOLE8_SUBOUT: {
            auto& arr = kAirwindowsParams_Console8SubOut;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SAT_CONSOLE9_BUSS: {
            auto& arr = kAirwindowsParams_Console9Buss;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SAT_CONSOLE9_CHANNEL: {
            auto& arr = kAirwindowsParams_Console9Channel;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SAT_CONSOLEH_BUSS: {
            auto& arr = kAirwindowsParams_ConsoleHBuss;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SAT_CONSOLEH_CHANNEL: {
            auto& arr = kAirwindowsParams_ConsoleHChannel;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SAT_CONSOLEH_PRE: {
            auto& arr = kAirwindowsParams_ConsoleHPre;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SAT_CONSOLE_LABUSS: {
            auto& arr = kAirwindowsParams_ConsoleLABuss;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SAT_CONSOLE_LA_CHANNEL: {
            auto& arr = kAirwindowsParams_ConsoleLAChannel;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SAT_CONSOLE_MC_BUSS: {
            auto& arr = kAirwindowsParams_ConsoleMCBuss;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SAT_CONSOLE_MC_CHANNEL: {
            auto& arr = kAirwindowsParams_ConsoleMCChannel;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SAT_CONSOLE_MD_BUSS: {
            auto& arr = kAirwindowsParams_ConsoleMDBuss;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SAT_CONSOLE_MD_CHANNEL: {
            auto& arr = kAirwindowsParams_ConsoleMDChannel;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SAT_CONSOLEX_BUSS: {
            auto& arr = kAirwindowsParams_ConsoleXBuss;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SAT_CONSOLEX_CHANNEL: {
            auto& arr = kAirwindowsParams_ConsoleXChannel;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SAT_CONSOLEX_PRE: {
            auto& arr = kAirwindowsParams_ConsoleXPre;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SAT_CONSOLEX2_BUSS: {
            auto& arr = kAirwindowsParams_ConsoleX2Buss;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SAT_CONSOLEX2_CHANNEL: {
            auto& arr = kAirwindowsParams_ConsoleX2Channel;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SAT_CONSOLEX2_PRE: {
            auto& arr = kAirwindowsParams_ConsoleX2Pre;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SAT_EVERYCONSOLE: {
            auto& arr = kAirwindowsParams_EveryConsole;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SAT_PURESTCONSOLE_BUSS: {
            auto& arr = kAirwindowsParams_PurestConsoleBuss;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SAT_PURESTCONSOLE_CHANNEL: {
            auto& arr = kAirwindowsParams_PurestConsoleChannel;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SAT_PURESTCONSOLE2_BUSS: {
            auto& arr = kAirwindowsParams_PurestConsole2Buss;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SAT_PURESTCONSOLE2_CHANNEL: {
            auto& arr = kAirwindowsParams_PurestConsole2Channel;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SAT_PURESTCONSOLE3_BUSS: {
            auto& arr = kAirwindowsParams_PurestConsole3Buss;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SAT_PURESTCONSOLE3_CHANNEL: {
            auto& arr = kAirwindowsParams_PurestConsole3Channel;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case FILTER_YNOTHIGHPASS: {
            auto& arr = kAirwindowsParams_YNotHighpass;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case FILTER_YNOTLOWPASS: {
            auto& arr = kAirwindowsParams_YNotLowpass;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case FILTER_YNOTNOTCH: {
            auto& arr = kAirwindowsParams_YNotNotch;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }

                case TAPE_FLUTTER: {
            auto& arr = kAirwindowsParams_Flutter;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case TONE_COLOUR_HUGENESS: {
            auto& arr = kAirwindowsParams_Huge;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DYN_ACCELERATION2: {
            auto& arr = kAirwindowsParams_Acceleration2;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_ADT: {
            auto& arr = kAirwindowsParams_ADT;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case FILTER_ANGLE_EQ: {
            auto& arr = kAirwindowsParams_AngleEQ;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case FILTER_ANGLE_FILTER: {
            auto& arr = kAirwindowsParams_AngleFilter;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DYN_A_QUICK_VOICE_CLIP: {
            auto& arr = kAirwindowsParams_AQuickVoiceClip;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SAT_ATMOSPHERE_BUSS: {
            auto& arr = kAirwindowsParams_AtmosphereBuss;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SAT_ATMOSPHERE_CHANNEL: {
            auto& arr = kAirwindowsParams_AtmosphereChannel;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_AUTO_PAN: {
            auto& arr = kAirwindowsParams_AutoPan;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case FILTER_AVER_MATRIX: {
            auto& arr = kAirwindowsParams_AverMatrix;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_BEAM: {
            auto& arr = kAirwindowsParams_Beam;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case FILTER_BEZ_EQ: {
            auto& arr = kAirwindowsParams_BezEQ;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case FILTER_BEZ_EQ_2: {
            auto& arr = kAirwindowsParams_BezEQ2;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DYN_BEZI_COMP: {
            auto& arr = kAirwindowsParams_BeziComp;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_BIT_DUAL_PAN: {
            auto& arr = kAirwindowsParams_BitDualPan;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DIST_BITE: {
            auto& arr = kAirwindowsParams_Bite;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_BIT_SHIFT_PAN: {
            auto& arr = kAirwindowsParams_BitShiftPan;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DYN_BLOCK_PARTY: {
            auto& arr = kAirwindowsParams_BlockParty;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DYN_BRASS_RIDER: {
            auto& arr = kAirwindowsParams_BrassRider;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SAT_BUSS_COLORS4: {
            auto& arr = kAirwindowsParams_BussColors4;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DYN_BUTTER_COMP: {
            auto& arr = kAirwindowsParams_ButterComp;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DYN_BUTTER_COMP2: {
            auto& arr = kAirwindowsParams_ButterComp2;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SAT_C5_RAW_BUSS: {
            auto& arr = kAirwindowsParams_C5RawBuss;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SAT_C5_RAW_CHANNEL: {
            auto& arr = kAirwindowsParams_C5RawChannel;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case AMPSIM_CABS2: {
            auto& arr = kAirwindowsParams_Cabs2;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_CANS: {
            auto& arr = kAirwindowsParams_Cans;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_CANS_AW: {
            auto& arr = kAirwindowsParams_CansAW;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case AMPSIM_CHIMEY_DELUXE: {
            auto& arr = kAirwindowsParams_ChimeyDeluxe;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case AMPSIM_CHIMEY_GUITAR: {
            auto& arr = kAirwindowsParams_ChimeyGuitar;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case AMPSIM_CHIMEY_GUITAR2: {
            auto& arr = kAirwindowsParams_ChimeyGuitar2;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_CHORUS: {
            auto& arr = kAirwindowsParams_Chorus;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_CHORUS_ENSEMBLE: {
            auto& arr = kAirwindowsParams_ChorusEnsemble;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case TAPE_CHROME_OXIDE: {
            auto& arr = kAirwindowsParams_ChromeOxide;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SAT_CIDER: {
            auto& arr = kAirwindowsParams_Cider;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DYN_CLIP_ONLY3: {
            auto& arr = kAirwindowsParams_ClipOnly3;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DIST_COJONES: {
            auto& arr = kAirwindowsParams_Cojones;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_CONTENT_HIDE_D: {
            auto& arr = kAirwindowsParams_ContentHideD;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DIST_CREATURE: {
            auto& arr = kAirwindowsParams_Creature;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case AMPSIM_CRICK_BASS: {
            auto& arr = kAirwindowsParams_CrickBass;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case TAPE_CRUNCHY_GROOVE_WEAR: {
            auto& arr = kAirwindowsParams_CrunchyGrooveWear;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DYN_C_STRIP: {
            auto& arr = kAirwindowsParams_CStrip;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_CURVE: {
            auto& arr = kAirwindowsParams_curve;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case FILTER_DARK: {
            auto& arr = kAirwindowsParams_Dark;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case REVERB_DATTORRO: {
            auto& arr = kAirwindowsParams_Dattorro;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_DC_VOLTAGE: {
            auto& arr = kAirwindowsParams_DCVoltage;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DYN_DE_BESS: {
            auto& arr = kAirwindowsParams_DeBess;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case FILTER_DE_BEZ: {
            auto& arr = kAirwindowsParams_DeBez;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DIST_DECKWRECKA: {
            auto& arr = kAirwindowsParams_Deckwrecka;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_DE_CRACKLE: {
            auto& arr = kAirwindowsParams_DeCrackle;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DYN_DE_ESS: {
            auto& arr = kAirwindowsParams_DeEss;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DYN_DE_HISS: {
            auto& arr = kAirwindowsParams_DeHiss;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DYN_DE_NOISE: {
            auto& arr = kAirwindowsParams_DeNoise;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_DE_REZ: {
            auto& arr = kAirwindowsParams_DeRez;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_DE_REZ2: {
            auto& arr = kAirwindowsParams_DeRez2;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_DE_REZ3: {
            auto& arr = kAirwindowsParams_DeRez3;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DIST_DIRT: {
            auto& arr = kAirwindowsParams_Dirt;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case TAPE_DISCONTAPEITY: {
            auto& arr = kAirwindowsParams_Discontapeity;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_DISCONTINUITY: {
            auto& arr = kAirwindowsParams_Discontinuity;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_DISTANCE: {
            auto& arr = kAirwindowsParams_Distance;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_DISTANCE2: {
            auto& arr = kAirwindowsParams_Distance2;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_DISTANCE3: {
            auto& arr = kAirwindowsParams_Distance3;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_DITHERBOX: {
            auto& arr = kAirwindowsParams_Ditherbox;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DIST_DONUT: {
            auto& arr = kAirwindowsParams_Donut;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_DOUBLELAY: {
            auto& arr = kAirwindowsParams_Doublelay;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_DOUBLE_PAUL: {
            auto& arr = kAirwindowsParams_DoublePaul;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DYN_DRUM_SLAM: {
            auto& arr = kAirwindowsParams_DrumSlam;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_DUB_CENTER: {
            auto& arr = kAirwindowsParams_DubCenter;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case TAPE_DUB_PLATE: {
            auto& arr = kAirwindowsParams_DubPlate;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case TAPE_DUB_PLATE2: {
            auto& arr = kAirwindowsParams_DubPlate2;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_DUB_SUB2: {
            auto& arr = kAirwindowsParams_DubSub2;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_DUST_BUNNY: {
            auto& arr = kAirwindowsParams_DustBunny;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DYN_DYNAMICS: {
            auto& arr = kAirwindowsParams_Dynamics;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DYN_DYNAMICS2: {
            auto& arr = kAirwindowsParams_Dynamics2;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DYN_DYNAMICS3: {
            auto& arr = kAirwindowsParams_Dynamics3;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DYN_DYNO: {
            auto& arr = kAirwindowsParams_Dyno;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DIST_EDGE: {
            auto& arr = kAirwindowsParams_Edge;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_ED_IS_DIM: {
            auto& arr = kAirwindowsParams_EdIsDim;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SAT_ELATION: {
            auto& arr = kAirwindowsParams_Elation;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case FILTER_ELLIPTICAL: {
            auto& arr = kAirwindowsParams_Elliptical;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_ENSEMBLE: {
            auto& arr = kAirwindowsParams_Ensemble;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case FILTER_EQ: {
            auto& arr = kAirwindowsParams_EQ;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_EVERY_SLEW: {
            auto& arr = kAirwindowsParams_EverySlew;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_EVERY_TRIM: {
            auto& arr = kAirwindowsParams_EveryTrim;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case FILTER_FAT_EQ: {
            auto& arr = kAirwindowsParams_FatEQ;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DYN_FINAL_CLIP: {
            auto& arr = kAirwindowsParams_FinalClip;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DYN_FLOOR: {
            auto& arr = kAirwindowsParams_Floor;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case TAPE_FROM_TAPE: {
            auto& arr = kAirwindowsParams_FromTape;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DYN_GATELOPE: {
            auto& arr = kAirwindowsParams_Gatelope;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_GOLDEN_SLEW: {
            auto& arr = kAirwindowsParams_GoldenSlew;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DIST_GOLEM: {
            auto& arr = kAirwindowsParams_Golem;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case FILTER_HERMEPASS: {
            auto& arr = kAirwindowsParams_Hermepass;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case FILTER_HERME_TRIM: {
            auto& arr = kAirwindowsParams_HermeTrim;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DYN_HIGH_IMPACT: {
            auto& arr = kAirwindowsParams_HighImpact;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DIST_HIP_CRUSH: {
            auto& arr = kAirwindowsParams_HipCrush;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case FILTER_HOLT2: {
            auto& arr = kAirwindowsParams_Holt2;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_HOMBRE: {
            auto& arr = kAirwindowsParams_Hombre;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case FILTER_HULL: {
            auto& arr = kAirwindowsParams_Hull;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case FILTER_HULL2: {
            auto& arr = kAirwindowsParams_Hull2;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DIST_HYPE: {
            auto& arr = kAirwindowsParams_Hype;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DYN_HYPERSOFT: {
            auto& arr = kAirwindowsParams_Hypersoft;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case FILTER_HYPERSONIC: {
            auto& arr = kAirwindowsParams_Hypersonic;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case FILTER_HYPERSON_X: {
            auto& arr = kAirwindowsParams_HypersonX;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DIST_INFLAMER: {
            auto& arr = kAirwindowsParams_Inflamer;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case FILTER_INFRASONIC: {
            auto& arr = kAirwindowsParams_Infrasonic;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case TAPE_IRON_OXIDE_CLASSIC: {
            auto& arr = kAirwindowsParams_IronOxideClassic;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case TAPE_IRON_OXIDE_CLASSIC2: {
            auto& arr = kAirwindowsParams_IronOxideClassic2;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case FILTER_ISOLATOR: {
            auto& arr = kAirwindowsParams_Isolator;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case FILTER_ISOLATOR2: {
            auto& arr = kAirwindowsParams_Isolator2;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case FILTER_ISOLATOR3: {
            auto& arr = kAirwindowsParams_Isolator3;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case FILTER_KALMAN: {
            auto& arr = kAirwindowsParams_Kalman;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_LEFTO_MONO: {
            auto& arr = kAirwindowsParams_LeftoMono;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DYN_LOGICAL4: {
            auto& arr = kAirwindowsParams_Logical4;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case REVERB_LR_CONVOLVE: {
            auto& arr = kAirwindowsParams_LRConvolve;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case REVERB_LR_CONVOLVE2: {
            auto& arr = kAirwindowsParams_LRConvolve2;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case REVERB_LR_CONVOLVE3: {
            auto& arr = kAirwindowsParams_LRConvolve3;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_LR_FLIP_TIMER: {
            auto& arr = kAirwindowsParams_LRFlipTimer;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SAT_LUXOR: {
            auto& arr = kAirwindowsParams_Luxor;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case FILTER_MACK_EQ: {
            auto& arr = kAirwindowsParams_MackEQ;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DYN_MASTERING: {
            auto& arr = kAirwindowsParams_Mastering;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DYN_MASTERING2: {
            auto& arr = kAirwindowsParams_Mastering2;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case AMPSIM_MID_AMP: {
            auto& arr = kAirwindowsParams_MidAmp;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_MONITORING2: {
            auto& arr = kAirwindowsParams_Monitoring2;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_MONITORING3: {
            auto& arr = kAirwindowsParams_Monitoring3;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_MS_FLIP_TIMER: {
            auto& arr = kAirwindowsParams_MSFlipTimer;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DIST_MULTI_BAND_DISTORTION: {
            auto& arr = kAirwindowsParams_MultiBandDistortion;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case REVERB_NEVERLAND: {
            auto& arr = kAirwindowsParams_Neverland;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_NOT_JUST_ANOTHER_CD: {
            auto& arr = kAirwindowsParams_NotJustAnotherCD;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_ORBIT_KICK: {
            auto& arr = kAirwindowsParams_OrbitKick;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_OVERHEADS: {
            auto& arr = kAirwindowsParams_Overheads;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case FILTER_PARAMETRIC: {
            auto& arr = kAirwindowsParams_Parametric;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_PAUL_WIDE: {
            auto& arr = kAirwindowsParams_PaulWide;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SAT_PD_BUSS: {
            auto& arr = kAirwindowsParams_PDBuss;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SAT_PD_CHANNEL: {
            auto& arr = kAirwindowsParams_PDChannel;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DYN_PEAKS_ONLY: {
            auto& arr = kAirwindowsParams_PeaksOnly;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case FILTER_PEAR_EQ: {
            auto& arr = kAirwindowsParams_PearEQ;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case FILTER_PEAR_LITE_EQ: {
            auto& arr = kAirwindowsParams_PearLiteEQ;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_PITCH_DELAY: {
            auto& arr = kAirwindowsParams_PitchDelay;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_POCKEY: {
            auto& arr = kAirwindowsParams_Pockey;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_POCKEY2: {
            auto& arr = kAirwindowsParams_Pockey2;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DYN_PODCAST: {
            auto& arr = kAirwindowsParams_Podcast;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DYN_PODCAST_DELUXE: {
            auto& arr = kAirwindowsParams_PodcastDeluxe;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case AMPSIM_POINTY_DELUXE: {
            auto& arr = kAirwindowsParams_PointyDeluxe;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case AMPSIM_POINTY_GUITAR: {
            auto& arr = kAirwindowsParams_PointyGuitar;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DIST_POP: {
            auto& arr = kAirwindowsParams_Pop;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DIST_POP2: {
            auto& arr = kAirwindowsParams_Pop2;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DIST_POP3: {
            auto& arr = kAirwindowsParams_Pop3;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SAT_PRECIOUS: {
            auto& arr = kAirwindowsParams_Precious;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DYN_PRESSURE6: {
            auto& arr = kAirwindowsParams_Pressure6;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case FILTER_PRIME_FIR: {
            auto& arr = kAirwindowsParams_PrimeFIR;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case AMPSIM_PUNCHY_DELUXE: {
            auto& arr = kAirwindowsParams_PunchyDeluxe;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case AMPSIM_PUNCHY_GUITAR: {
            auto& arr = kAirwindowsParams_PunchyGuitar;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_PUREST_DUAL_PAN: {
            auto& arr = kAirwindowsParams_PurestDualPan;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_PUREST_ECHO: {
            auto& arr = kAirwindowsParams_PurestEcho;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SAT_PUREST_WARM2: {
            auto& arr = kAirwindowsParams_PurestWarm2;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DYN_PYEWACKET: {
            auto& arr = kAirwindowsParams_Pyewacket;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_RAW_GLITTERS: {
            auto& arr = kAirwindowsParams_RawGlitters;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_RAW_TIMBERS: {
            auto& arr = kAirwindowsParams_RawTimbers;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_RECURVE: {
            auto& arr = kAirwindowsParams_Recurve;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_REMAP: {
            auto& arr = kAirwindowsParams_Remap;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case FILTER_RES_EQ: {
            auto& arr = kAirwindowsParams_ResEQ;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case FILTER_RES_EQ_2: {
            auto& arr = kAirwindowsParams_ResEQ2;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_RIGHTO_MONO: {
            auto& arr = kAirwindowsParams_RightoMono;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DIST_SHAPE: {
            auto& arr = kAirwindowsParams_Shape;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case FILTER_SIDE_DULL: {
            auto& arr = kAirwindowsParams_SideDull;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case FILTER_SIDEPASS: {
            auto& arr = kAirwindowsParams_Sidepass;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_SILHOUETTE: {
            auto& arr = kAirwindowsParams_Silhouette;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case FILTER_SILKEN: {
            auto& arr = kAirwindowsParams_Silken;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SAT_SINEW: {
            auto& arr = kAirwindowsParams_Sinew;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case AMPSIM_SINGLE_ENDED_TRIODE: {
            auto& arr = kAirwindowsParams_SingleEndedTriode;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_SLEW3: {
            auto& arr = kAirwindowsParams_Slew3;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_SLEW4: {
            auto& arr = kAirwindowsParams_Slew4;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case FILTER_SMOOTH_EQ_2: {
            auto& arr = kAirwindowsParams_SmoothEQ2;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case FILTER_SMOOTH_EQ_3: {
            auto& arr = kAirwindowsParams_SmoothEQ3;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_SOFT_CLOCK: {
            auto& arr = kAirwindowsParams_SoftClock;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_SOFT_CLOCK2: {
            auto& arr = kAirwindowsParams_SoftClock2;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_SOFT_CLOCK3: {
            auto& arr = kAirwindowsParams_SoftClock3;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DYN_SOFT_GATE: {
            auto& arr = kAirwindowsParams_SoftGate;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SAT_SQUARE_ROOT: {
            auto& arr = kAirwindowsParams_SquareRoot;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_SRSLY2: {
            auto& arr = kAirwindowsParams_Srsly2;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_SRSLY3: {
            auto& arr = kAirwindowsParams_Srsly3;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case REVERB_STAR_CHILD: {
            auto& arr = kAirwindowsParams_StarChild;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case REVERB_STAR_CHILD2: {
            auto& arr = kAirwindowsParams_StarChild2;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_STEREO_CHORUS: {
            auto& arr = kAirwindowsParams_StereoChorus;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_STEREO_DOUBLER: {
            auto& arr = kAirwindowsParams_StereoDoubler;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_STEREO_ENSEMBLE: {
            auto& arr = kAirwindowsParams_StereoEnsemble;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_STEREO_FX: {
            auto& arr = kAirwindowsParams_StereoFX;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DIST_STONEFIRE: {
            auto& arr = kAirwindowsParams_Stonefire;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DYN_STONE_FIRE_COMP: {
            auto& arr = kAirwindowsParams_StoneFireComp;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_SWEET_WIDE: {
            auto& arr = kAirwindowsParams_SweetWide;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DYN_SWELL: {
            auto& arr = kAirwindowsParams_Swell;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case TAPE_TAPE: {
            auto& arr = kAirwindowsParams_Tape;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case TAPE_TAPE_BIAS: {
            auto& arr = kAirwindowsParams_TapeBias;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_TAPE_DELAY: {
            auto& arr = kAirwindowsParams_TapeDelay;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_TAPE_DELAY2: {
            auto& arr = kAirwindowsParams_TapeDelay2;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case TAPE_TAPE_DUST: {
            auto& arr = kAirwindowsParams_TapeDust;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case TAPE_TAPE_FAT: {
            auto& arr = kAirwindowsParams_TapeFat;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case TAPE_TAPE_HACK: {
            auto& arr = kAirwindowsParams_TapeHack;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case TAPE_TAPE_HACK2: {
            auto& arr = kAirwindowsParams_TapeHack2;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case TAPE_TAPE_HACK3: {
            auto& arr = kAirwindowsParams_TapeHack3;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_TEXTURIZE: {
            auto& arr = kAirwindowsParams_Texturize;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_TEXTURIZE_MS: {
            auto& arr = kAirwindowsParams_TexturizeMS;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case REVERB_THUNDER: {
            auto& arr = kAirwindowsParams_Thunder;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case TAPE_TO_TAPE7: {
            auto& arr = kAirwindowsParams_ToTape7;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case TAPE_TO_TAPE8: {
            auto& arr = kAirwindowsParams_ToTape8;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case TAPE_TO_TAPE9: {
            auto& arr = kAirwindowsParams_ToTape9;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_TPDF_WIDE: {
            auto& arr = kAirwindowsParams_TPDFWide;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case SAT_TRANS_DESK: {
            auto& arr = kAirwindowsParams_TransDesk;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_TRIPLE_SPREAD: {
            auto& arr = kAirwindowsParams_TripleSpread;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case AMPSIM_TUBE: {
            auto& arr = kAirwindowsParams_Tube;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case AMPSIM_TUBE2: {
            auto& arr = kAirwindowsParams_Tube2;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_U_LAW_DECODE: {
            auto& arr = kAirwindowsParams_uLawDecode;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_U_LAW_ENCODE: {
            auto& arr = kAirwindowsParams_uLawEncode;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case FILTER_ULTRASONIC: {
            auto& arr = kAirwindowsParams_Ultrasonic;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case FILTER_ULTRASONIC_LITE: {
            auto& arr = kAirwindowsParams_UltrasonicLite;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case FILTER_ULTRASONIC_MED: {
            auto& arr = kAirwindowsParams_UltrasonicMed;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case FILTER_ULTRASON_X: {
            auto& arr = kAirwindowsParams_UltrasonX;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_VIBRATO: {
            auto& arr = kAirwindowsParams_Vibrato;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case REVERB_VOICE_OF_THE_STARSHIP: {
            auto& arr = kAirwindowsParams_VoiceOfTheStarship;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_VOICE_TRICK: {
            auto& arr = kAirwindowsParams_VoiceTrick;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_WOLFBOT: {
            auto& arr = kAirwindowsParams_Wolfbot;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_ECHOJAM: {
            auto& arr = kAirwindowsParams_Echojam;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case REVERB_WOODEN_BOX: {
            auto& arr = kAirwindowsParams_WoodenBox;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case REVERB_VERBSEVENS: {
            auto& arr = kAirwindowsParams_VerbSevens;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case REVERB_WOODENCHEST: {
            auto& arr = kAirwindowsParams_WoodenChest;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_FAST_DISTANCE2: {
            auto& arr = kAirwindowsParams_FastDistance2;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DYN_X2_BUSS: {
            auto& arr = kAirwindowsParams_X2Buss;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case FILTER_Y_NOT_BANDPASS: {
            auto& arr = kAirwindowsParams_YNotBandpass;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case FILTER_Z_ACID_LOWPASS: {
            auto& arr = kAirwindowsParams_ZAcidLowpass;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case FILTER_Z_BANDPASS2: {
            auto& arr = kAirwindowsParams_ZBandpass2;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case FILTER_Z_HIGHPASS2: {
            auto& arr = kAirwindowsParams_ZHighpass2;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case FILTER_Z_LOWPASS: {
            auto& arr = kAirwindowsParams_ZLowpass;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case FILTER_Z_LOWPASS2: {
            auto& arr = kAirwindowsParams_ZLowpass2;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case FILTER_Z_NOTCH: {
            auto& arr = kAirwindowsParams_ZNotch;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case FILTER_Z_NOTCH2: {
            auto& arr = kAirwindowsParams_ZNotch2;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case EFFECT_ZOOM: {
            auto& arr = kAirwindowsParams_Zoom;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DYN_Z_OUTPUT_STAGE: {
            auto& arr = kAirwindowsParams_ZOutputStage;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case DYN_ULTRALIGHT3: {
            auto& arr = kAirwindowsParams_Ultralight3;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case FILTER_Z_REGION: {
            auto& arr = kAirwindowsParams_ZRegion;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
        case FILTER_Z_REGION2: {
            auto& arr = kAirwindowsParams_ZRegion2;
            return paramIndex >= 0 && paramIndex < (int)arr.size() ? &arr[paramIndex] : nullptr;
        }
default:
            return nullptr;
    }
}

const char* AirSuiteUniversal::getParamDisplayName(Type type, int paramIndex, bool suppressWet) {
    const AirwindowsSpec spec = suppressWet ? getSuppressedSpec(type) : getSpec(type);
    const bool specCoversIndex = (paramIndex >= 0 && paramIndex < spec.count);
    const char* fallbackName = specCoversIndex ? spec.names[paramIndex] : nullptr;

    // suppressWet only suppresses a parameter the SPEC actually marks as the
    // algorithm's internal WET (getSuppressedSpec nulls that name). A type with
    // NO generated spec entry has no internal wet to hide — the old check
    // ("suppressWet && fallbackName == nullptr") treated every extracted param
    // of every spec-less algorithm as wet and returned nullptr, which hid all
    // of ToTape8's / ButterComp2's controls from the FX cards.
    if (suppressWet && specCoversIndex && fallbackName == nullptr)
        return nullptr;

    // Fall through to the WinVST-extracted table, which the airwindows
    // consolidated sources generate for every algorithm (spec-less ones
    // included).
    const int extractedCount = getExtractedParamCount(type);
    if (paramIndex >= 0 && paramIndex < extractedCount) {
        const AirwindowsExtractedParam* extractedParam = getExtractedParam(type, paramIndex);
        if (extractedParam != nullptr && extractedParam->raw_name != nullptr && extractedParam->raw_name[0] != '\0') {
            // Suppression follows the NAME, not the spec's presence: a
            // spec-less algorithm can still carry an internal Dry/Wet in its
            // extracted list, and the FX card draws its own WET knob.
            if (suppressWet && isWetParam(extractedParam->raw_name))
                return nullptr;
            return extractedParam->raw_name;
        }
    }

    return fallbackName;
}

// ============================================================
// getTypeFromName() - Look up Type by display name
// ============================================================
AirSuiteUniversal::Type AirSuiteUniversal::getTypeFromName(const char* name) {
    if (!name || name[0] == '\0') return NONE;
    
    // Linear scan through all valid types
    for (int i = 1; i < COUNT; ++i) {
        Type t = (Type)i;
        const char* displayName = getDisplayName(t);
        if (displayName && std::strcmp(displayName, name) == 0) {
            return t;
        }
    }
    return NONE;
}


void AirSuiteUniversal::formatParamDisplay(Type type, int param, float value,
                                           char* out, int outSize) {
    if (out == nullptr || outSize <= 0) return;
    out[0] = '\0';
    const AirwindowsSpec spec = getSpec(type);
    const int extractedCount = getExtractedParamCount(type);
    if (param < 0 || (param >= spec.count && param >= extractedCount)) return;

    struct Cache {
        std::unordered_map<int, AudioEffectX*> effects;
        ~Cache() {
            for (auto& kv : effects) delete kv.second;
        }
    };
    static Cache cache;

    const int key = (int)type;
    auto it = cache.effects.find(key);
    if (it == cache.effects.end()) {
        const RuntimeIdentity& id = getRuntimeIdentity(type);
        AudioEffectX* fx = nullptr;
        switch (id.category) {
            case Category::AMBIENCE: fx = awspec::createFor_AirSuiteAmbience(id.dispatchType); break;
            case Category::AMPSIM: fx = awspec::createFor_AirSuiteAmpSim(id.dispatchType); break;
            case Category::BASS: fx = awspec::createFor_AirSuiteBass(id.dispatchType); break;
            case Category::BIQUAD: fx = awspec::createFor_AirSuiteBiquad(id.dispatchType); break;
            case Category::BRIGHTNESS: fx = awspec::createFor_AirSuiteBrightness(id.dispatchType); break;
            case Category::CLIPPING: fx = awspec::createFor_AirSuiteClipping(id.dispatchType); break;
            case Category::DIST: fx = awspec::createFor_AirSuiteDistortion(id.dispatchType); break;
            case Category::DITHERS: fx = awspec::createFor_AirSuiteDithers(id.dispatchType); break;
            case Category::DYN: fx = awspec::createFor_AirSuiteDynamics(id.dispatchType); break;
            case Category::EFFECT: fx = awspec::createFor_AirSuiteEffect(id.dispatchType); break;
            case Category::FILTER: fx = awspec::createFor_AirSuiteFilters(id.dispatchType); break;
            case Category::LOFI: fx = awspec::createFor_AirSuiteLoFi(id.dispatchType); break;
            case Category::NOISE: fx = awspec::createFor_AirSuiteNoise(id.dispatchType); break;
            case Category::REVERB: fx = awspec::createFor_AirSuiteReverb(id.dispatchType); break;
            case Category::SAT: fx = awspec::createFor_AirSuiteSaturation(id.dispatchType); break;
            case Category::STEREO: fx = awspec::createFor_AirSuiteStereo(id.dispatchType); break;
            case Category::SUBTLETY: fx = awspec::createFor_AirSuiteSubtlety(id.dispatchType); break;
            case Category::TAPE: fx = awspec::createFor_AirSuiteTape(id.dispatchType); break;
            case Category::TONE_COLOUR: fx = awspec::createFor_AirSuiteToneColour(id.dispatchType); break;
            case Category::UTILITY: fx = awspec::createFor_AirSuiteUtility(id.dispatchType); break;
            case Category::XYZ: fx = awspec::createFor_AirSuiteXYZ(id.dispatchType); break;
            default: break;
        }
        if (fx == nullptr) return;
        fx->setSampleRate(48000.0f);
        // Bound the UI-only cache; browsing many algorithms must not pin
        // dozens of multi-MB delay buffers.
        if (cache.effects.size() >= 8) {
            auto old = cache.effects.begin();
            delete old->second;
            cache.effects.erase(old);
        }
        it = cache.effects.emplace(key, fx).first;
    }

    AudioEffectX* fx = it->second;
    if (fx == nullptr) return;
    char disp[64] = {};
    char label[32] = {};
    fx->setParameter(param, std::clamp(value, 0.0f, 1.0f));
    fx->getParameterDisplay(param, disp);
    fx->getParameterLabel(param, label);
    // Some upstream displays are padded with trailing spaces ("0.500  ");
    // strip them so the numeric unit fallback recognises the value.
    for (int n = (int)std::strlen(disp); n > 0; --n) {
        const char c = disp[n - 1];
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') disp[n - 1] = '\0';
        else break;
    }
    for (int n = (int)std::strlen(label); n > 0; --n) {
        const char c = label[n - 1];
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') label[n - 1] = '\0';
        else break;
    }
    if (label[0] == '\0') {
        // Upstream Airwindows leaves getParameterLabel empty for many
        // algorithms. Prefer the generated spec / extracted unit, and for
        // normalized gain-like controls fall back to a true percentage so the
        // FX card never shows a bare 0.500 where a percent belongs.
        const AirwindowsExtractedParam* ep = getExtractedParam(type, param);
        if (param < spec.count && spec.units[param] != nullptr && spec.units[param][0] != '\0') {
            label[0] = '\0';
            std::snprintf(label, sizeof(label), "%s", spec.units[param]);
        } else if (ep != nullptr && ep->label != nullptr && ep->label[0] != '\0') {
            std::snprintf(label, sizeof(label), "%s", ep->label);
        } else {
            // Still no unit metadata. If the native display is purely numeric
            // (a normalized 0..1 control), show it as a true percentage so no
            // FX card is left with a bare 0.500. Text/discrete displays are
            // left untouched.
            char* numEnd = nullptr;
            std::strtod(disp, &numEnd);
            if (numEnd != disp && numEnd != nullptr && *numEnd == '\0') {
                std::snprintf(out, (size_t)outSize, "%.1f%%",
                              std::clamp(value, 0.0f, 1.0f) * 100.0f);
                return;
            }
        }
    }
    if (disp[0] == '\0') {
        std::snprintf(out, (size_t)outSize, "%.3f", value);
    } else if (label[0] != '\0') {
        std::snprintf(out, (size_t)outSize, "%s %s", disp, label);
    } else {
        std::snprintf(out, (size_t)outSize, "%s", disp);
    }
}

END_NAMESPACE_DISTRHO
