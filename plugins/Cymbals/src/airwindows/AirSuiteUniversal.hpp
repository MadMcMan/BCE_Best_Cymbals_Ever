#ifndef AIR_SUITE_UNIVERSAL_HPP
#define AIR_SUITE_UNIVERSAL_HPP

#include "DistrhoPlugin.hpp"
#include "AirwindowsCommon.hpp"
#include "AirwindowsReverbs.hpp"
#include "AirwindowsEffects.hpp"
#include "AirwindowsAmpSims.hpp"
#include "AirwindowsDistortion.hpp"
#include "AirwindowsDynamics.hpp"
#include "AirwindowsSaturation.hpp"
#include "AirwindowsTape.hpp"
#include "AirwindowsFilters.hpp"
#include "AirwindowsBiquad.hpp"
#include "AirwindowsClipping.hpp"
#include "AirwindowsDithers.hpp"
#include "AirwindowsUtility.hpp"
#include "AirwindowsStereo.hpp"
#include "AirwindowsBass.hpp"
#include "AirwindowsLoFi.hpp"
#include "AirwindowsNoise.hpp"
#include "AirwindowsBrightness.hpp"
#include "AirwindowsSubtlety.hpp"
#include "AirwindowsToneColour.hpp"
#include "AirwindowsXYZ.hpp"
#include "AirwindowsExtractedParams.hpp"
#include <cstring>
#include <cmath>
#include <algorithm>
#include <array>

START_NAMESPACE_DISTRHO

class AirSuiteUniversal {
public:
    enum class Category : int {
        NONE = 0,
        REVERB,
        EFFECT,
        AMPSIM,
        DIST,
        DYN,
        SAT,
        TAPE,
        FILTER,
        BIQUAD,
        CLIPPING,
        DITHERS,
        MISC,
        UTILITY,
        STEREO,
        BASS,
        LOFI,
        NOISE,
        AMBIENCE,
        BRIGHTNESS,
        SUBTLETY,
        TONE_COLOUR,
        XYZ
    };

    enum Type : int {
        NONE = 0,
        
        // REVERB (44 algorithms, 1..44)
        REVERB_BRIGHTAMBIENCE = 1,
        REVERB_BRIGHTAMBIENCE2,
        REVERB_BRIGHTAMBIENCE3,
        REVERB_CHAMBER,
        REVERB_CHAMBER2,
        REVERB_CLEARCOAT,
        REVERB_CREAMCOAT,
        REVERB_CRUNCHCOAT,
        REVERB_GALACTIC,
        REVERB_GALACTIC2,
        REVERB_GALACTIC3,
        REVERB_INFINITY_VERB,
        REVERB_INFINITY2_VERB,
        REVERB_KALIENSPACESHIP,
        REVERB_KBEYOND,
        REVERB_KCATHEDRAL,
        REVERB_KCATHEDRAL2,
        REVERB_KCATHEDRAL3,
        REVERB_KCATHEDRAL4,
        REVERB_KCATHEDRAL5,
        REVERB_KCHAMBERAR,
        REVERB_KCOSMOS,
        REVERB_KGUITARHALL,
        REVERB_KGUITARHALL2,
        REVERB_KPLATE140,
        REVERB_KPLATE240,
        REVERB_KPLATEA,
        REVERB_KPLATEB,
        REVERB_KPLATEC,
        REVERB_KPLATED,
        REVERB_KSTATION,
        REVERB_KWOODROOM,
        REVERB_MATRIXVERB,
        REVERB_MV,
        REVERB_MV2,
        REVERB_NONLINEARSPACE,
        REVERB_POCKETVERBS,
        REVERB_REVERB,
        REVERB_VERBITY,
        REVERB_VERBITY2,
        REVERB_VERBSIXES,
        REVERB_VERBTHIC,
        REVERB_VERBTINY,
        
        // EFFECT (22 algorithms, 45..66)
        EFFECT_AURA = 45,
        EFFECT_CLOUDCOAT,
        EFFECT_DISINTEGRATE,
        EFFECT_ENERGY,
        EFFECT_ENERGY2,
        EFFECT_EXCITER,
        EFFECT_FACET,
        EFFECT_FRACTURE,
        EFFECT_FRACTURE2,
        EFFECT_GALACTICVIBE,
        EFFECT_GLITCHSHIFTER,
        EFFECT_GRINGER,
        EFFECT_GUITARCONDITIONER,
        EFFECT_PAFNUTY2,
        EFFECT_PITCHNASTY,
        EFFECT_POWERSAG2,
        EFFECT_PREPONDERANT,
        EFFECT_RINGMODULATOR,
        EFFECT_TAKECARE,
        EFFECT_TREMOLO,
        EFFECT_TREMOSQUARE,
        EFFECT_TRIANGLIZER,
        
        // AMPSIM (10 algorithms, 67..76)
        AMPSIM_CABS = 67,
        AMPSIM_BASSAMP,
        AMPSIM_BIGAMP,
        AMPSIM_FIREAMP,
        AMPSIM_GRINDAMP,
        AMPSIM_GUITARCONDITIONER,
        AMPSIM_PURESTMULTISQUISH,
        AMPSIM_TONESLANT,
        AMPSIM_LEADAMP,
        AMPSIM_LILAMPL,
        
        // DISTORTION (13 algorithms, 77..89)
        DIST_BASSDRIVE = 77,
        DIST_DENSITY,
        DIST_DENSITY2,
        DIST_DENSITY3,
        DIST_DISTORTION,
        DIST_DRIVE,
        DIST_FRACTURE,
        DIST_FRACTURE2,
        DIST_HARDVACUUM,
        DIST_PURESTDRIVE,
        DIST_SPIRAL,
        DIST_SPIRAL2,
        DIST_BITGLITTER,
        
        // DYNAMICS (6 algorithms, 90..95)
        DYN_PRESSURE4 = 90,
        DYN_PRESSURE5,
        DYN_VARIMU,
        DYN_POINT,
        DYN_ACCELERATION,
        DYN_PURESTGAIN,
        
        // SATURATION (7 algorithms, 96..102)
        SAT_BUILDATPDF = 96,
        SAT_DITHERFLOAT,
        SAT_POWERSAG,
        SAT_NIKOLA,
        SAT_SHORTBUSS,
        SAT_COMPRESATURATOR,
        
        // TAPE (9 algorithms, 103..111)
        TAPE_TOTAPE6 = 103,
        TAPE_IRONOXIDE5,
        TAPE_FLUTTER2,
        TAPE_DUBLY,
        TAPE_DUBLY2,
        TAPE_DUBLY3,
        TAPE_TOTAPE5,
        TAPE_TOVINYL4,
        TAPE_FLUTTER = 111,
        
        // FILTERS (11 algorithms, 112..122)
        FILTER_AIR = 112,
        FILTER_CAPACITOR2,
        FILTER_HIGHPASS2,
        FILTER_LOWPASS2,
        FILTER_AIR2,
        FILTER_AIR3,
        FILTER_AIR4,
        FILTER_CAPACITOR,
        FILTER_HIGHPASS,
        FILTER_LOWPASS,
        FILTER_BIQUADDOUBLE,
        
        // EFFECT (4 additional effects, 123..126)
        EFFECT_ELECTROHAT = 123,
        EFFECT_FLIPITY,
        EFFECT_MONITORING,
        EFFECT_MONOAM,
        
        // BIQUAD (8 algorithms, 127..134)
        BIQUAD_BIQUAD = 127,
        BIQUAD_BIQUAD2,
        BIQUAD_BIQUADHILO,
        BIQUAD_BIQUADNONLIN,
        BIQUAD_BIQUADONEHALF,
        BIQUAD_BIQUADPLUS,
        BIQUAD_BIQUADSTACK,
        BIQUAD_BIQUADTRIPLE,
        
        // CLIPPING (5 algorithms, 135..139)
        CLIP_ADCLIP7 = 135,
        CLIP_ADCLIP8,
        CLIP_CLIPONLY,
        CLIP_CLIPONLY2,
        CLIP_CLIPSOFTLY,
        
        // DITHERS (11 algorithms, 140..150)
        DITHER_DITHERMEDISKERS = 140,
        DITHER_DITHERMTIMBERS,
        DITHER_HIGHGLOSS,
        DITHER_NATURALIZE,
        DITHER_NODEDITHER,
        DITHER_NJAD,
        DITHER_PAULDITHER,
        DITHER_SPATIALIZE,
        DITHER_TAPEDITHER,
        DITHER_TPDFDITHER,
        DITHER_VINYLDITHER,
        
        // UTILITY (14 algorithms, 151..164)
        UTILITY_AVERAGE = 151,
        UTILITY_BALANCED,
        UTILITY_BITSHIFTGAIN,
        UTILITY_CSTRIP2,
        UTILITY_PURESTFADE,
        UTILITY_PURESTGAIN,
        UTILITY_PURESTWARM,
        UTILITY_RIGHTEOUS4,
        UTILITY_SLEW,
        UTILITY_SLEW2,
        UTILITY_SLEWONLY,
        UTILITY_STEREOFX,
        UTILITY_STUDIOTAN,
        UTILITY_WEIGHT,
        
        // STEREO (10 algorithms, 165..174)
        STEREO_CHANNEL4 = 165,
        STEREO_CHANNEL5,
        STEREO_CHANNEL6,
        STEREO_CHANNEL7,
        STEREO_CHANNEL8,
        STEREO_CHANNEL9,
        STEREO_MIDSIDE,
        STEREO_PHASENUDGE,
        STEREO_SRSLY,
        STEREO_WIDER,
        
        // BASS (5 algorithms, 175..179)
        BASS_BASSAMP = 175,
        BASS_BASSDRIVE,
        BASS_BASSKIT,
        BASS_DUBSUB,
        BASS_SUBSONLY,
        
        // LOFI (12 algorithms, 180..191)
        LOFI_DESK = 180,
        LOFI_DESK4,
        LOFI_DIGITALBLACK,
        LOFI_DITHERFLOAT,
        LOFI_FATHOMFIVE,
        LOFI_GRINGER,
        LOFI_GROOVEWEAR,
        LOFI_LOUD,
        LOFI_MACKITY,
        LOFI_NCSEVENTEEN,
        LOFI_POCKETVERBS,
        LOFI_SAMPLEDELAY,
        
        // NOISE (6 algorithms, 192..197)
        NOISE_DARKNOISE = 192,
        NOISE_HOLT,
        NOISE_NOISE,
        NOISE_PEAR,
        NOISE_SURGE,
        NOISE_SURGETIDE,
        
        // AMBIENCE (8 algorithms, 198..205)
        AMBIENCE_AURA = 198,
        AMBIENCE_BRIGHTAMBIENCE,
        AMBIENCE_BRIGHTAMBIENCE2,
        AMBIENCE_BRIGHTAMBIENCE3,
        AMBIENCE_CHAMBER,
        AMBIENCE_CHAMBER2,
        AMBIENCE_VERBITY,
        AMBIENCE_VERBITY2,
        
        // BRIGHTNESS (6 algorithms, 206..211)
        BRIGHTNESS_AIR = 206,
        BRIGHTNESS_APICOLYPSE,
        BRIGHTNESS_CAPACITOR,
        BRIGHTNESS_CAPACITOR2,
        BRIGHTNESS_CRYSTAL,
        BRIGHTNESS_PURESTAIR,
        
        // SUBTLETY (14 algorithms, 212..225)
        SUBTLETY_BAXANDALL = 212,
        SUBTLETY_BAXANDALL2,
        SUBTLETY_BAXANDALL3,
        SUBTLETY_CALIBRE,
        SUBTLETY_COILS,
        SUBTLETY_COILS2,
        SUBTLETY_INTERSTAGE,
        SUBTLETY_MOJO,
        SUBTLETY_PLATINUMSLEW,
        SUBTLETY_SLEWSONIC,
        SUBTLETY_SMOOTH,
        SUBTLETY_SMOOTHEQ,
        SUBTLETY_TUBEDESK,
        SUBTLETY_UNBOX,
        
        // TONE_COLOUR (10 algorithms, 226..235)
        TONE_COLOUR_FOCUS = 226,
        TONE_COLOUR_GROOVEWEAR,
        TONE_COLOUR_HUGENESS,
        TONE_COLOUR_MELT,
        TONE_COLOUR_ONECORNERCLIP,
        TONE_COLOUR_PAFNUTY,
        TONE_COLOUR_PEAR,
        TONE_COLOUR_PEAR2,
        TONE_COLOUR_SURGE,
        TONE_COLOUR_SWEETEN,
        
        // XYZ (11 algorithms, 236..246)
        XYZ_XBANDPASS = 236,
        XYZ_XHIGHPASS,
        XYZ_XLOWPASS,
        XYZ_XNOTCH,
        XYZ_XREGION,
        XYZ_YBANDPASS,
        XYZ_YHIGHPASS,
        XYZ_YLOWPASS,
        XYZ_YNOTCH,
        XYZ_ZBANDPASS,
        XYZ_ZHIGHPASS,

        DYN_ACCELERATION2,
        EFFECT_ADT,
        FILTER_ANGLE_EQ,
        FILTER_ANGLE_FILTER,
        DYN_A_QUICK_VOICE_CLIP,
        SAT_ATMOSPHERE_BUSS,
        SAT_ATMOSPHERE_CHANNEL,
        EFFECT_AUTO_PAN,
        FILTER_AVER_MATRIX,
        EFFECT_BEAM,
        FILTER_BEZ_EQ,
        FILTER_BEZ_EQ_2,
        DYN_BEZI_COMP,
        EFFECT_BIT_DUAL_PAN,
        DIST_BITE,
        EFFECT_BIT_SHIFT_PAN,
        DYN_BLOCK_PARTY,
        DYN_BRASS_RIDER,
        SAT_BUSS_COLORS4,
        DYN_BUTTER_COMP,
        DYN_BUTTER_COMP2,
        SAT_C5_RAW_BUSS,
        SAT_C5_RAW_CHANNEL,
        AMPSIM_CABS2,
        EFFECT_CANS,
        EFFECT_CANS_AW,
        AMPSIM_CHIMEY_DELUXE,
        AMPSIM_CHIMEY_GUITAR,
        AMPSIM_CHIMEY_GUITAR2,
        EFFECT_CHORUS,
        EFFECT_CHORUS_ENSEMBLE,
        TAPE_CHROME_OXIDE,
        SAT_CIDER,
        DYN_CLIP_ONLY3,
        DIST_COJONES,
        EFFECT_CONTENT_HIDE_D,
        DIST_CREATURE,
        AMPSIM_CRICK_BASS,
        TAPE_CRUNCHY_GROOVE_WEAR,
        DYN_C_STRIP,
        EFFECT_CURVE,
        FILTER_DARK,
        REVERB_DATTORRO,
        EFFECT_DC_VOLTAGE,
        DYN_DE_BESS,
        FILTER_DE_BEZ,
        DIST_DECKWRECKA,
        EFFECT_DE_CRACKLE,
        DYN_DE_ESS,
        DYN_DE_HISS,
        DYN_DE_NOISE,
        EFFECT_DE_REZ,
        EFFECT_DE_REZ2,
        EFFECT_DE_REZ3,
        DIST_DIRT,
        TAPE_DISCONTAPEITY,
        EFFECT_DISCONTINUITY,
        EFFECT_DISTANCE,
        EFFECT_DISTANCE2,
        EFFECT_DISTANCE3,
        EFFECT_DITHERBOX,
        DIST_DONUT,
        EFFECT_DOUBLELAY,
        EFFECT_DOUBLE_PAUL,
        DYN_DRUM_SLAM,
        EFFECT_DUB_CENTER,
        TAPE_DUB_PLATE,
        TAPE_DUB_PLATE2,
        EFFECT_DUB_SUB2,
        EFFECT_DUST_BUNNY,
        DYN_DYNAMICS,
        DYN_DYNAMICS2,
        DYN_DYNAMICS3,
        DYN_DYNO,
        DIST_EDGE,
        EFFECT_ED_IS_DIM,
        SAT_ELATION,
        FILTER_ELLIPTICAL,
        EFFECT_ENSEMBLE,
        FILTER_EQ,
        EFFECT_EVERY_SLEW,
        EFFECT_EVERY_TRIM,
        FILTER_FAT_EQ,
        DYN_FINAL_CLIP,
        DYN_FLOOR,
        TAPE_FROM_TAPE,
        DYN_GATELOPE,
        EFFECT_GOLDEN_SLEW,
        DIST_GOLEM,
        FILTER_HERMEPASS,
        FILTER_HERME_TRIM,
        DYN_HIGH_IMPACT,
        DIST_HIP_CRUSH,
        FILTER_HOLT2,
        EFFECT_HOMBRE,
        FILTER_HULL,
        FILTER_HULL2,
        DIST_HYPE,
        DYN_HYPERSOFT,
        FILTER_HYPERSONIC,
        FILTER_HYPERSON_X,
        DIST_INFLAMER,
        FILTER_INFRASONIC,
        TAPE_IRON_OXIDE_CLASSIC,
        TAPE_IRON_OXIDE_CLASSIC2,
        FILTER_ISOLATOR,
        FILTER_ISOLATOR2,
        FILTER_ISOLATOR3,
        FILTER_KALMAN,
        EFFECT_LEFTO_MONO,
        DYN_LOGICAL4,
        REVERB_LR_CONVOLVE,
        REVERB_LR_CONVOLVE2,
        REVERB_LR_CONVOLVE3,
        EFFECT_LR_FLIP_TIMER,
        SAT_LUXOR,
        FILTER_MACK_EQ,
        DYN_MASTERING,
        DYN_MASTERING2,
        AMPSIM_MID_AMP,
        EFFECT_MONITORING2,
        EFFECT_MONITORING3,
        EFFECT_MS_FLIP_TIMER,
        DIST_MULTI_BAND_DISTORTION,
        REVERB_NEVERLAND,
        EFFECT_NOT_JUST_ANOTHER_CD,
        EFFECT_ORBIT_KICK,
        EFFECT_OVERHEADS,
        FILTER_PARAMETRIC,
        EFFECT_PAUL_WIDE,
        SAT_PD_BUSS,
        SAT_PD_CHANNEL,
        DYN_PEAKS_ONLY,
        FILTER_PEAR_EQ,
        FILTER_PEAR_LITE_EQ,
        EFFECT_PITCH_DELAY,
        EFFECT_POCKEY,
        EFFECT_POCKEY2,
        DYN_PODCAST,
        DYN_PODCAST_DELUXE,
        AMPSIM_POINTY_DELUXE,
        AMPSIM_POINTY_GUITAR,
        DIST_POP,
        DIST_POP2,
        DIST_POP3,
        SAT_PRECIOUS,
        DYN_PRESSURE6,
        FILTER_PRIME_FIR,
        AMPSIM_PUNCHY_DELUXE,
        AMPSIM_PUNCHY_GUITAR,
        EFFECT_PUREST_DUAL_PAN,
        EFFECT_PUREST_ECHO,
        SAT_PUREST_WARM2,
        DYN_PYEWACKET,
        EFFECT_RAW_GLITTERS,
        EFFECT_RAW_TIMBERS,
        EFFECT_RECURVE,
        EFFECT_REMAP,
        FILTER_RES_EQ,
        FILTER_RES_EQ_2,
        EFFECT_RIGHTO_MONO,
        DIST_SHAPE,
        FILTER_SIDE_DULL,
        FILTER_SIDEPASS,
        EFFECT_SILHOUETTE,
        FILTER_SILKEN,
        SAT_SINEW,
        AMPSIM_SINGLE_ENDED_TRIODE,
        EFFECT_SLEW3,
        EFFECT_SLEW4,
        FILTER_SMOOTH_EQ_2,
        FILTER_SMOOTH_EQ_3,
        EFFECT_SOFT_CLOCK,
        EFFECT_SOFT_CLOCK2,
        EFFECT_SOFT_CLOCK3,
        DYN_SOFT_GATE,
        SAT_SQUARE_ROOT,
        EFFECT_SRSLY2,
        EFFECT_SRSLY3,
        REVERB_STAR_CHILD,
        REVERB_STAR_CHILD2,
        EFFECT_STEREO_CHORUS,
        EFFECT_STEREO_DOUBLER,
        EFFECT_STEREO_ENSEMBLE,
        EFFECT_STEREO_FX,
        DIST_STONEFIRE,
        DYN_STONE_FIRE_COMP,
        EFFECT_SWEET_WIDE,
        DYN_SWELL,
        TAPE_TAPE,
        TAPE_TAPE_BIAS,
        EFFECT_TAPE_DELAY,
        EFFECT_TAPE_DELAY2,
        TAPE_TAPE_DUST,
        TAPE_TAPE_FAT,
        TAPE_TAPE_HACK,
        TAPE_TAPE_HACK2,
        EFFECT_TEXTURIZE,
        EFFECT_TEXTURIZE_MS,
        REVERB_THUNDER,
        TAPE_TO_TAPE7,
        TAPE_TO_TAPE8,
        TAPE_TO_TAPE9,
        EFFECT_TPDF_WIDE,
        SAT_TRANS_DESK,
        EFFECT_TRIPLE_SPREAD,
        AMPSIM_TUBE,
        AMPSIM_TUBE2,
        EFFECT_U_LAW_DECODE,
        EFFECT_U_LAW_ENCODE,
        FILTER_ULTRASONIC,
        FILTER_ULTRASONIC_LITE,
        FILTER_ULTRASONIC_MED,
        FILTER_ULTRASON_X,
        EFFECT_VIBRATO,
        REVERB_VOICE_OF_THE_STARSHIP,
        EFFECT_VOICE_TRICK,
        EFFECT_WOLFBOT,
        REVERB_WOODEN_BOX,
        DYN_X2_BUSS,
        FILTER_Y_NOT_BANDPASS,
        FILTER_Z_ACID_LOWPASS,
        FILTER_Z_BANDPASS2,
        FILTER_Z_HIGHPASS2,
        FILTER_Z_LOWPASS,
        FILTER_Z_LOWPASS2,
        FILTER_Z_NOTCH,
        FILTER_Z_NOTCH2,
        EFFECT_ZOOM,
        DYN_Z_OUTPUT_STAGE,
        FILTER_Z_REGION,
        FILTER_Z_REGION2,

        // === NEW: 64 missing plugins ===
        CLIP_ADCLIP9,
        FILTER_BEZ_EQ3,
        FILTER_BEZ_EQ4,
        SAT_CONSOLE0_BUSS,
        SAT_CONSOLE0_CHANNEL,
        SAT_CONSOLE4_BUSS,
        SAT_CONSOLE4_CHANNEL,
        SAT_CONSOLE5_BUSS,
        SAT_CONSOLE5_CHANNEL,
        SAT_CONSOLE5_DARKCH,
        SAT_CONSOLE6_BUSS,
        SAT_CONSOLE6_CHANNEL,
        SAT_CONSOLE7_BUSS,
        SAT_CONSOLE7_CASCADE,
        SAT_CONSOLE7_CHANNEL,
        SAT_CONSOLE7_CRUNCH,
        SAT_CONSOLE8_BUSSHYPE,
        SAT_CONSOLE8_BUSSIN,
        SAT_CONSOLE8_BUSSOUT,
        SAT_CONSOLE8_CHANNELHYPE,
        SAT_CONSOLE8_CHANNELIN,
        SAT_CONSOLE8_CHANNELOUT,
        SAT_CONSOLE8_LITEBUSS,
        SAT_CONSOLE8_LITECHANNEL,
        SAT_CONSOLE8_SUBHYPE,
        SAT_CONSOLE8_SUBIN,
        SAT_CONSOLE8_SUBOUT,
        SAT_CONSOLE9_BUSS,
        SAT_CONSOLE9_CHANNEL,
        SAT_CONSOLEH_BUSS,
        SAT_CONSOLEH_CHANNEL,
        SAT_CONSOLEH_PRE,
        SAT_CONSOLE_LABUSS,
        SAT_CONSOLE_LA_CHANNEL,
        SAT_CONSOLE_MC_BUSS,
        SAT_CONSOLE_MC_CHANNEL,
        SAT_CONSOLE_MD_BUSS,
        SAT_CONSOLE_MD_CHANNEL,
        SAT_CONSOLEX_BUSS,
        SAT_CONSOLEX_CHANNEL,
        SAT_CONSOLEX_PRE,
        SAT_CONSOLEX2_BUSS,
        SAT_CONSOLEX2_CHANNEL,
        SAT_CONSOLEX2_PRE,
        SAT_CONSOLEX3_BUSS,
        SAT_CONSOLEX3_CHANNEL,
        SAT_CONSOLEX3_PRE,
        EFFECT_DEREZ4,
        SAT_EVERYCONSOLE,
        REVERB_KCYBERCITY,
        REVERB_KROCKSTAR,
        SAT_LONGHAND,
        SAT_PURESTCONSOLE_BUSS,
        SAT_PURESTCONSOLE_CHANNEL,
        SAT_PURESTCONSOLE2_BUSS,
        SAT_PURESTCONSOLE2_CHANNEL,
        SAT_PURESTCONSOLE3_BUSS,
        SAT_PURESTCONSOLE3_CHANNEL,
        SAT_PURESTCONSOLE4_BUSS,
        SAT_PURESTCONSOLE4_CHANNEL,
        FILTER_SUZAN,
        FILTER_YNOTHIGHPASS,
        FILTER_YNOTLOWPASS,
        FILTER_YNOTNOTCH,
        SAT_CONSOLEX3,
        SAT_PUREST_WARM3,
        BASS_RETROBASS,
        DIST_SPIRAL3,
        EFFECT_WEAVE,
        // Post-ECHOJAM upstream additions (appended so earlier IDs stay fixed)
        EFFECT_ECHOJAM,
        REVERB_VERBSEVENS,
        REVERB_WOODENCHEST,
        EFFECT_FAST_DISTANCE2,
        TAPE_TAPE_HACK3,
        DYN_ULTRALIGHT3,
        COUNT = 554
    };

    struct RuntimeIdentity {
        Type canonicalType = NONE;
        Category category = Category::NONE;
        const char* categoryName = "None";
        const char* displayName = "None";
        int dispatchType = 0;
        bool hasInternalWet = false;
    };

    // ========== Static Utilities ==========
    static AirwindowsSpec getSpec(Type type);
    // Exact upstream display string for an FX slot knob: value + parameter
    // label (dB, %, ms, Hz, kHz, ...). Cached display-only effect per type;
    // UI thread only.
    static void formatParamDisplay(Type type, int param, float value,
                                   char* out, int outSize);
    static AirwindowsSpec getSuppressedSpec(Type type); // hides wet/mix params
    static const RuntimeIdentity& getRuntimeIdentity(Type type);
    static int getDispatchType(Type type);
    
    // Phase 3: Runtime metadata access layer for canonical extracted parameters
    // Returns canonical parameter metadata from WinVST extraction, or nullptr if not found
    static const AirwindowsExtractedParam* getExtractedParam(Type type, int paramIndex);
    
    // Inline implementations for test-support methods
    static inline int getExtractedParamCount(Type type) {
        // Mirror of getExtractedParam, but returns array size instead of pointer
        // BEGIN GENERATED: getExtractedParamCount switch
        switch (type) {
            case REVERB_BRIGHTAMBIENCE: return kAirwindowsParams_BrightAmbience.size();
            case REVERB_BRIGHTAMBIENCE2: return kAirwindowsParams_BrightAmbience2.size();
            case REVERB_BRIGHTAMBIENCE3: return kAirwindowsParams_BrightAmbience3.size();
            case REVERB_CHAMBER: return kAirwindowsParams_Chamber.size();
            case REVERB_CHAMBER2: return kAirwindowsParams_Chamber2.size();
            case REVERB_CLEARCOAT: return kAirwindowsParams_ClearCoat.size();
            case REVERB_CREAMCOAT: return kAirwindowsParams_CreamCoat.size();
            case REVERB_CRUNCHCOAT: return kAirwindowsParams_CrunchCoat.size();
            case REVERB_GALACTIC: return kAirwindowsParams_Galactic.size();
            case REVERB_GALACTIC2: return kAirwindowsParams_Galactic2.size();
            case REVERB_GALACTIC3: return kAirwindowsParams_Galactic3.size();
            case REVERB_INFINITY_VERB: return kAirwindowsParams_Infinity.size();
            case REVERB_INFINITY2_VERB: return kAirwindowsParams_Infinity2.size();
            case REVERB_KALIENSPACESHIP: return kAirwindowsParams_kAlienSpaceship.size();
            case REVERB_KBEYOND: return kAirwindowsParams_kBeyond.size();
            case REVERB_KCATHEDRAL: return kAirwindowsParams_kCathedral.size();
            case REVERB_KCATHEDRAL2: return kAirwindowsParams_kCathedral2.size();
            case REVERB_KCATHEDRAL3: return kAirwindowsParams_kCathedral3.size();
            case REVERB_KCATHEDRAL4: return kAirwindowsParams_kCathedral4.size();
            case REVERB_KCATHEDRAL5: return kAirwindowsParams_kCathedral5.size();
            case REVERB_KCHAMBERAR: return kAirwindowsParams_kChamberAR.size();
            case REVERB_KCOSMOS: return kAirwindowsParams_kCosmos.size();
            case REVERB_KGUITARHALL: return kAirwindowsParams_kGuitarHall.size();
            case REVERB_KGUITARHALL2: return kAirwindowsParams_kGuitarHall2.size();
            case REVERB_KPLATE140: return kAirwindowsParams_kPlate140.size();
            case REVERB_KPLATE240: return kAirwindowsParams_kPlate240.size();
            case REVERB_KPLATEA: return kAirwindowsParams_kPlateA.size();
            case REVERB_KPLATEB: return kAirwindowsParams_kPlateB.size();
            case REVERB_KPLATEC: return kAirwindowsParams_kPlateC.size();
            case REVERB_KPLATED: return kAirwindowsParams_kPlateD.size();
            case REVERB_KSTATION: return kAirwindowsParams_kStation.size();
            case REVERB_KWOODROOM: return kAirwindowsParams_kWoodRoom.size();
            case REVERB_MATRIXVERB: return kAirwindowsParams_MatrixVerb.size();
            case REVERB_MV: return kAirwindowsParams_MV.size();
            case REVERB_MV2: return kAirwindowsParams_MV2.size();
            case REVERB_NONLINEARSPACE: return kAirwindowsParams_NonlinearSpace.size();
            case REVERB_POCKETVERBS: return kAirwindowsParams_PocketVerbs.size();
            case REVERB_REVERB: return kAirwindowsParams_Reverb.size();
            case REVERB_VERBITY: return kAirwindowsParams_Verbity.size();
            case REVERB_VERBITY2: return kAirwindowsParams_Verbity2.size();
            case REVERB_VERBSIXES: return kAirwindowsParams_VerbSixes.size();
            case REVERB_VERBTHIC: return kAirwindowsParams_VerbThic.size();
            case REVERB_VERBTINY: return kAirwindowsParams_VerbTiny.size();
            case REVERB_VERBSEVENS: return kAirwindowsParams_VerbSevens.size();
            case REVERB_WOODENCHEST: return kAirwindowsParams_WoodenChest.size();
            case EFFECT_FAST_DISTANCE2: return kAirwindowsParams_FastDistance2.size();
            case EFFECT_AURA: return kAirwindowsParams_Aura.size();
            case EFFECT_CLOUDCOAT: return kAirwindowsParams_CloudCoat.size();
            case EFFECT_DISINTEGRATE: return kAirwindowsParams_Disintegrate.size();
            case EFFECT_ENERGY: return kAirwindowsParams_Energy.size();
            case EFFECT_ENERGY2: return kAirwindowsParams_Energy2.size();
            case EFFECT_EXCITER: return kAirwindowsParams_Exciter.size();
            case EFFECT_FACET: return kAirwindowsParams_Facet.size();
            case EFFECT_FRACTURE: return kAirwindowsParams_Fracture.size();
            case EFFECT_FRACTURE2: return kAirwindowsParams_Fracture2.size();
            case EFFECT_GALACTICVIBE: return kAirwindowsParams_GalacticVibe.size();
            case EFFECT_GLITCHSHIFTER: return kAirwindowsParams_GlitchShifter.size();
            case EFFECT_GRINGER: return kAirwindowsParams_Gringer.size();
            case EFFECT_GUITARCONDITIONER: return kAirwindowsParams_GuitarConditioner.size();
            case EFFECT_PAFNUTY2: return kAirwindowsParams_Pafnuty2.size();
            case EFFECT_PITCHNASTY: return kAirwindowsParams_PitchNasty.size();
            case EFFECT_POWERSAG2: return kAirwindowsParams_PowerSag2.size();
            case EFFECT_PREPONDERANT: return kAirwindowsParams_Preponderant.size();
            case EFFECT_RINGMODULATOR: return kAirwindowsParams_RingModulator.size();
            case EFFECT_TAKECARE: return kAirwindowsParams_TakeCare.size();
            case EFFECT_TREMOLO: return kAirwindowsParams_Tremolo.size();
            case EFFECT_TREMOSQUARE: return kAirwindowsParams_TremoSquare.size();
            case EFFECT_TRIANGLIZER: return kAirwindowsParams_Trianglizer.size();
            case AMPSIM_CABS: return kAirwindowsParams_Cabs.size();
            case AMPSIM_BASSAMP: return kAirwindowsParams_BassAmp.size();
            case AMPSIM_BIGAMP: return kAirwindowsParams_BigAmp.size();
            case AMPSIM_FIREAMP: return kAirwindowsParams_FireAmp.size();
            case AMPSIM_GRINDAMP: return kAirwindowsParams_GrindAmp.size();
            case AMPSIM_GUITARCONDITIONER: return kAirwindowsParams_GuitarConditioner.size();
            case AMPSIM_PURESTMULTISQUISH: return kAirwindowsParams_PurestSquish.size();
            case AMPSIM_TONESLANT: return kAirwindowsParams_ToneSlant.size();
            case AMPSIM_LEADAMP: return kAirwindowsParams_LeadAmp.size();
            case AMPSIM_LILAMPL: return 0;  // Unmapped extracted metadata
            case DIST_BASSDRIVE: return kAirwindowsParams_BassDrive.size();
            case DIST_DENSITY: return kAirwindowsParams_Density.size();
            case DIST_DENSITY2: return kAirwindowsParams_Density2.size();
            case DIST_DENSITY3: return kAirwindowsParams_Density3.size();
            case DIST_DISTORTION: return kAirwindowsParams_Distortion.size();
            case DIST_DRIVE: return kAirwindowsParams_Drive.size();
            case DIST_FRACTURE: return kAirwindowsParams_Fracture.size();
            case DIST_FRACTURE2: return kAirwindowsParams_Fracture2.size();
            case DIST_HARDVACUUM: return kAirwindowsParams_HardVacuum.size();
            case DIST_PURESTDRIVE: return kAirwindowsParams_PurestDrive.size();
            case DIST_SPIRAL: return kAirwindowsParams_Spiral.size();
            case DIST_SPIRAL2: return kAirwindowsParams_Spiral2.size();
            case DIST_BITGLITTER: return kAirwindowsParams_BitGlitter.size();
            case DYN_PRESSURE4: return kAirwindowsParams_Pressure4.size();
            case DYN_PRESSURE5: return kAirwindowsParams_Pressure5.size();
            case DYN_VARIMU: return kAirwindowsParams_VariMu.size();
            case DYN_POINT: return kAirwindowsParams_Point.size();
            case DYN_ACCELERATION: return kAirwindowsParams_Acceleration.size();
            case DYN_PURESTGAIN: return kAirwindowsParams_PurestGain.size();
            case SAT_BUILDATPDF: return kAirwindowsParams_BuildATPDF.size();
            case SAT_DITHERFLOAT: return kAirwindowsParams_DitherFloat.size();
            case SAT_POWERSAG: return kAirwindowsParams_PowerSag.size();
            case SAT_NIKOLA: return kAirwindowsParams_Nikola.size();
            case SAT_SHORTBUSS: return kAirwindowsParams_ShortBuss.size();
            case SAT_COMPRESATURATOR: return kAirwindowsParams_Compresaturator.size();
            case TAPE_TOTAPE6: return kAirwindowsParams_ToTape6.size();
            case TAPE_IRONOXIDE5: return kAirwindowsParams_IronOxide5.size();
            case TAPE_FLUTTER2: return kAirwindowsParams_Flutter2.size();
            case TAPE_DUBLY: return kAirwindowsParams_Dubly.size();
            case TAPE_DUBLY2: return kAirwindowsParams_Dubly2.size();
            case TAPE_DUBLY3: return kAirwindowsParams_Dubly3.size();
            case TAPE_TOTAPE5: return kAirwindowsParams_ToTape5.size();
            case TAPE_TOVINYL4: return kAirwindowsParams_ToVinyl4.size();
            case TAPE_FLUTTER: return kAirwindowsParams_Flutter.size();
            case FILTER_AIR: return kAirwindowsParams_Air.size();
            case FILTER_CAPACITOR2: return kAirwindowsParams_Capacitor2.size();
            case FILTER_HIGHPASS2: return kAirwindowsParams_Highpass2.size();
            case FILTER_LOWPASS2: return kAirwindowsParams_Lowpass2.size();
            case FILTER_AIR2: return kAirwindowsParams_Air2.size();
            case FILTER_AIR3: return kAirwindowsParams_Air3.size();
            case FILTER_AIR4: return kAirwindowsParams_Air4.size();
            case FILTER_CAPACITOR: return kAirwindowsParams_Capacitor.size();
            case FILTER_HIGHPASS: return kAirwindowsParams_Highpass.size();
            case FILTER_LOWPASS: return kAirwindowsParams_Lowpass.size();
            case FILTER_BIQUADDOUBLE: return kAirwindowsParams_BiquadDouble.size();
            case EFFECT_ELECTROHAT: return kAirwindowsParams_ElectroHat.size();
            case EFFECT_FLIPITY: return kAirwindowsParams_Flipity.size();
            case EFFECT_MONITORING: return kAirwindowsParams_Monitoring.size();
            case EFFECT_MONOAM: return kAirwindowsParams_MoNoam.size();
            case BIQUAD_BIQUAD: return kAirwindowsParams_Biquad.size();
            case BIQUAD_BIQUAD2: return kAirwindowsParams_Biquad2.size();
            case BIQUAD_BIQUADHILO: return kAirwindowsParams_BiquadHiLo.size();
            case BIQUAD_BIQUADNONLIN: return kAirwindowsParams_BiquadNonLin.size();
            case BIQUAD_BIQUADONEHALF: return kAirwindowsParams_BiquadOneHalf.size();
            case BIQUAD_BIQUADPLUS: return kAirwindowsParams_BiquadPlus.size();
            case BIQUAD_BIQUADSTACK: return kAirwindowsParams_BiquadStack.size();
            case BIQUAD_BIQUADTRIPLE: return kAirwindowsParams_BiquadTriple.size();
            case CLIP_ADCLIP7: return kAirwindowsParams_ADClip7.size();
            case CLIP_ADCLIP8: return kAirwindowsParams_ADClip8.size();
            case CLIP_CLIPONLY: return kAirwindowsParams_ClipOnly.size();
            case CLIP_CLIPONLY2: return kAirwindowsParams_ClipOnly2.size();
            case CLIP_CLIPSOFTLY: return kAirwindowsParams_ClipSoftly.size();
            case DITHER_DITHERMEDISKERS: return kAirwindowsParams_DitherMeDiskers.size();
            case DITHER_DITHERMTIMBERS: return 0;  // Unmapped extracted metadata
            case DITHER_HIGHGLOSS: return 0;  // Unmapped extracted metadata
            case DITHER_NATURALIZE: return 0;  // Unmapped extracted metadata
            case DITHER_NODEDITHER: return kAirwindowsParams_NodeDither.size();
            case DITHER_NJAD: return 0;  // Unmapped extracted metadata
            case DITHER_PAULDITHER: return kAirwindowsParams_PaulDither.size();
            case DITHER_SPATIALIZE: return 0;  // Unmapped extracted metadata
            case DITHER_TAPEDITHER: return kAirwindowsParams_TapeDither.size();
            case DITHER_TPDFDITHER: return kAirwindowsParams_TPDFDither.size();
            case DITHER_VINYLDITHER: return kAirwindowsParams_VinylDither.size();
            case UTILITY_AVERAGE: return kAirwindowsParams_Average.size();
            case UTILITY_BALANCED: return kAirwindowsParams_Balanced.size();
            case UTILITY_BITSHIFTGAIN: return kAirwindowsParams_BitShiftGain.size();
            case UTILITY_CSTRIP2: return kAirwindowsParams_CStrip2.size();
            case UTILITY_PURESTFADE: return kAirwindowsParams_PurestFade.size();
            case UTILITY_PURESTGAIN: return kAirwindowsParams_PurestGain.size();
            case UTILITY_PURESTWARM: return kAirwindowsParams_PurestWarm.size();
            case UTILITY_RIGHTEOUS4: return kAirwindowsParams_Righteous4.size();
            case UTILITY_SLEW: return kAirwindowsParams_Slew.size();
            case UTILITY_SLEW2: return kAirwindowsParams_Slew2.size();
            case UTILITY_SLEWONLY: return kAirwindowsParams_SlewOnly.size();
            case UTILITY_STEREOFX: return kAirwindowsParams_StereoFX.size();
            case UTILITY_STUDIOTAN: return kAirwindowsParams_StudioTan.size();
            case UTILITY_WEIGHT: return kAirwindowsParams_Weight.size();
            case STEREO_CHANNEL4: return kAirwindowsParams_Channel4.size();
            case STEREO_CHANNEL5: return kAirwindowsParams_Channel5.size();
            case STEREO_CHANNEL6: return kAirwindowsParams_Channel6.size();
            case STEREO_CHANNEL7: return kAirwindowsParams_Channel7.size();
            case STEREO_CHANNEL8: return kAirwindowsParams_Channel8.size();
            case STEREO_CHANNEL9: return kAirwindowsParams_Channel9.size();
            case STEREO_MIDSIDE: return kAirwindowsParams_MidSide.size();
            case STEREO_PHASENUDGE: return kAirwindowsParams_PhaseNudge.size();
            case STEREO_SRSLY: return kAirwindowsParams_Srsly.size();
            case STEREO_WIDER: return kAirwindowsParams_Wider.size();
            case BASS_BASSAMP: return kAirwindowsParams_BassAmp.size();
            case BASS_BASSDRIVE: return kAirwindowsParams_BassDrive.size();
            case BASS_BASSKIT: return kAirwindowsParams_BassKit.size();
            case BASS_DUBSUB: return kAirwindowsParams_DubSub.size();
            case BASS_SUBSONLY: return kAirwindowsParams_SubsOnly.size();
            case LOFI_DESK: return kAirwindowsParams_Desk.size();
            case LOFI_DESK4: return kAirwindowsParams_Desk4.size();
            case LOFI_DIGITALBLACK: return kAirwindowsParams_DigitalBlack.size();
            case LOFI_DITHERFLOAT: return kAirwindowsParams_DitherFloat.size();
            case LOFI_FATHOMFIVE: return kAirwindowsParams_FathomFive.size();
            case LOFI_GRINGER: return kAirwindowsParams_Gringer.size();
            case LOFI_GROOVEWEAR: return kAirwindowsParams_GrooveWear.size();
            case LOFI_LOUD: return kAirwindowsParams_Loud.size();
            case LOFI_MACKITY: return kAirwindowsParams_Mackity.size();
            case LOFI_NCSEVENTEEN: return kAirwindowsParams_NCSeventeen.size();
            case LOFI_POCKETVERBS: return kAirwindowsParams_PocketVerbs.size();
            case LOFI_SAMPLEDELAY: return kAirwindowsParams_SampleDelay.size();
            case NOISE_DARKNOISE: return kAirwindowsParams_DarkNoise.size();
            case NOISE_HOLT: return kAirwindowsParams_Holt.size();
            case NOISE_NOISE: return kAirwindowsParams_Noise.size();
            case NOISE_PEAR: return kAirwindowsParams_Pear.size();
            case NOISE_SURGE: return kAirwindowsParams_Surge.size();
            case NOISE_SURGETIDE: return kAirwindowsParams_SurgeTide.size();
            case AMBIENCE_AURA: return kAirwindowsParams_Aura.size();
            case AMBIENCE_BRIGHTAMBIENCE: return kAirwindowsParams_BrightAmbience.size();
            case AMBIENCE_BRIGHTAMBIENCE2: return kAirwindowsParams_BrightAmbience2.size();
            case AMBIENCE_BRIGHTAMBIENCE3: return kAirwindowsParams_BrightAmbience3.size();
            case AMBIENCE_CHAMBER: return kAirwindowsParams_Chamber.size();
            case AMBIENCE_CHAMBER2: return kAirwindowsParams_Chamber2.size();
            case AMBIENCE_VERBITY: return kAirwindowsParams_Verbity.size();
            case AMBIENCE_VERBITY2: return kAirwindowsParams_Verbity2.size();
            case BRIGHTNESS_AIR: return kAirwindowsParams_Air.size();
            case BRIGHTNESS_APICOLYPSE: return kAirwindowsParams_Apicolypse.size();
            case BRIGHTNESS_CAPACITOR: return kAirwindowsParams_Capacitor.size();
            case BRIGHTNESS_CAPACITOR2: return kAirwindowsParams_Capacitor2.size();
            case BRIGHTNESS_CRYSTAL: return kAirwindowsParams_Crystal.size();
            case BRIGHTNESS_PURESTAIR: return kAirwindowsParams_PurestAir.size();
            case SUBTLETY_BAXANDALL: return kAirwindowsParams_Baxandall.size();
            case SUBTLETY_BAXANDALL2: return kAirwindowsParams_Baxandall2.size();
            case SUBTLETY_BAXANDALL3: return kAirwindowsParams_Baxandall3.size();
            case SUBTLETY_CALIBRE: return kAirwindowsParams_Calibre.size();
            case SUBTLETY_COILS: return kAirwindowsParams_Coils.size();
            case SUBTLETY_COILS2: return kAirwindowsParams_Coils2.size();
            case SUBTLETY_INTERSTAGE: return kAirwindowsParams_Interstage.size();
            case SUBTLETY_MOJO: return kAirwindowsParams_Mojo.size();
            case SUBTLETY_PLATINUMSLEW: return kAirwindowsParams_PlatinumSlew.size();
            case SUBTLETY_SLEWSONIC: return kAirwindowsParams_SlewSonic.size();
            case SUBTLETY_SMOOTH: return kAirwindowsParams_Smooth.size();
            case SUBTLETY_SMOOTHEQ: return kAirwindowsParams_SmoothEQ.size();
            case SUBTLETY_TUBEDESK: return kAirwindowsParams_TubeDesk.size();
            case SUBTLETY_UNBOX: return kAirwindowsParams_UnBox.size();
            case TONE_COLOUR_FOCUS: return kAirwindowsParams_Focus.size();
            case TONE_COLOUR_GROOVEWEAR: return kAirwindowsParams_GrooveWear.size();
            case TONE_COLOUR_HUGENESS: return kAirwindowsParams_Huge.size();
            case TONE_COLOUR_MELT: return kAirwindowsParams_Melt.size();
            case TONE_COLOUR_ONECORNERCLIP: return kAirwindowsParams_OneCornerClip.size();
            case TONE_COLOUR_PAFNUTY: return kAirwindowsParams_Pafnuty.size();
            case TONE_COLOUR_PEAR: return kAirwindowsParams_Pear.size();
            case TONE_COLOUR_PEAR2: return kAirwindowsParams_Pear2.size();
            case TONE_COLOUR_SURGE: return kAirwindowsParams_Surge.size();
            case TONE_COLOUR_SWEETEN: return kAirwindowsParams_Sweeten.size();
            case XYZ_XBANDPASS: return kAirwindowsParams_XBandpass.size();
            case XYZ_XHIGHPASS: return kAirwindowsParams_XHighpass.size();
            case XYZ_XLOWPASS: return kAirwindowsParams_XLowpass.size();
            case XYZ_XNOTCH: return kAirwindowsParams_XNotch.size();
            case XYZ_XREGION: return kAirwindowsParams_XRegion.size();
            case XYZ_YBANDPASS: return kAirwindowsParams_YBandpass.size();
            case XYZ_YHIGHPASS: return kAirwindowsParams_YHighpass.size();
            case XYZ_YLOWPASS: return kAirwindowsParams_YLowpass.size();
            case XYZ_YNOTCH: return kAirwindowsParams_YNotch.size();
            case XYZ_ZBANDPASS: return kAirwindowsParams_ZBandpass.size();
            case XYZ_ZHIGHPASS: return kAirwindowsParams_ZHighpass.size();
            case DYN_ACCELERATION2: return kAirwindowsParams_Acceleration2.size();
            case EFFECT_ADT: return kAirwindowsParams_ADT.size();
            case FILTER_ANGLE_EQ: return kAirwindowsParams_AngleEQ.size();
            case FILTER_ANGLE_FILTER: return kAirwindowsParams_AngleFilter.size();
            case DYN_A_QUICK_VOICE_CLIP: return kAirwindowsParams_AQuickVoiceClip.size();
            case SAT_ATMOSPHERE_BUSS: return kAirwindowsParams_AtmosphereBuss.size();
            case SAT_ATMOSPHERE_CHANNEL: return kAirwindowsParams_AtmosphereChannel.size();
            case EFFECT_AUTO_PAN: return kAirwindowsParams_AutoPan.size();
            case FILTER_AVER_MATRIX: return kAirwindowsParams_AverMatrix.size();
            case EFFECT_BEAM: return kAirwindowsParams_Beam.size();
            case FILTER_BEZ_EQ: return kAirwindowsParams_BezEQ.size();
            case FILTER_BEZ_EQ_2: return kAirwindowsParams_BezEQ2.size();
            case DYN_BEZI_COMP: return kAirwindowsParams_BeziComp.size();
            case EFFECT_BIT_DUAL_PAN: return kAirwindowsParams_BitDualPan.size();
            case DIST_BITE: return kAirwindowsParams_Bite.size();
            case EFFECT_BIT_SHIFT_PAN: return kAirwindowsParams_BitShiftPan.size();
            case DYN_BLOCK_PARTY: return kAirwindowsParams_BlockParty.size();
            case DYN_BRASS_RIDER: return kAirwindowsParams_BrassRider.size();
            case SAT_BUSS_COLORS4: return kAirwindowsParams_BussColors4.size();
            case DYN_BUTTER_COMP: return kAirwindowsParams_ButterComp.size();
            case DYN_BUTTER_COMP2: return kAirwindowsParams_ButterComp2.size();
            case SAT_C5_RAW_BUSS: return kAirwindowsParams_C5RawBuss.size();
            case SAT_C5_RAW_CHANNEL: return kAirwindowsParams_C5RawChannel.size();
            case AMPSIM_CABS2: return kAirwindowsParams_Cabs2.size();
            case EFFECT_CANS: return kAirwindowsParams_Cans.size();
            case EFFECT_CANS_AW: return kAirwindowsParams_CansAW.size();
            case AMPSIM_CHIMEY_DELUXE: return kAirwindowsParams_ChimeyDeluxe.size();
            case AMPSIM_CHIMEY_GUITAR: return kAirwindowsParams_ChimeyGuitar.size();
            case AMPSIM_CHIMEY_GUITAR2: return kAirwindowsParams_ChimeyGuitar2.size();
            case EFFECT_CHORUS: return kAirwindowsParams_Chorus.size();
            case EFFECT_CHORUS_ENSEMBLE: return kAirwindowsParams_ChorusEnsemble.size();
            case TAPE_CHROME_OXIDE: return kAirwindowsParams_ChromeOxide.size();
            case SAT_CIDER: return kAirwindowsParams_Cider.size();
            case DYN_CLIP_ONLY3: return kAirwindowsParams_ClipOnly3.size();
            case DIST_COJONES: return kAirwindowsParams_Cojones.size();
            case EFFECT_CONTENT_HIDE_D: return kAirwindowsParams_ContentHideD.size();
            case DIST_CREATURE: return kAirwindowsParams_Creature.size();
            case AMPSIM_CRICK_BASS: return kAirwindowsParams_CrickBass.size();
            case TAPE_CRUNCHY_GROOVE_WEAR: return kAirwindowsParams_CrunchyGrooveWear.size();
            case DYN_C_STRIP: return kAirwindowsParams_CStrip.size();
            case EFFECT_CURVE: return kAirwindowsParams_curve.size();
            case FILTER_DARK: return kAirwindowsParams_Dark.size();
            case REVERB_DATTORRO: return kAirwindowsParams_Dattorro.size();
            case EFFECT_DC_VOLTAGE: return kAirwindowsParams_DCVoltage.size();
            case DYN_DE_BESS: return kAirwindowsParams_DeBess.size();
            case FILTER_DE_BEZ: return kAirwindowsParams_DeBez.size();
            case DIST_DECKWRECKA: return kAirwindowsParams_Deckwrecka.size();
            case EFFECT_DE_CRACKLE: return kAirwindowsParams_DeCrackle.size();
            case DYN_DE_ESS: return kAirwindowsParams_DeEss.size();
            case DYN_DE_HISS: return kAirwindowsParams_DeHiss.size();
            case DYN_DE_NOISE: return kAirwindowsParams_DeNoise.size();
            case EFFECT_DE_REZ: return kAirwindowsParams_DeRez.size();
            case EFFECT_DE_REZ2: return kAirwindowsParams_DeRez2.size();
            case EFFECT_DE_REZ3: return kAirwindowsParams_DeRez3.size();
            case DIST_DIRT: return kAirwindowsParams_Dirt.size();
            case TAPE_DISCONTAPEITY: return kAirwindowsParams_Discontapeity.size();
            case EFFECT_DISCONTINUITY: return kAirwindowsParams_Discontinuity.size();
            case EFFECT_DISTANCE: return kAirwindowsParams_Distance.size();
            case EFFECT_DISTANCE2: return kAirwindowsParams_Distance2.size();
            case EFFECT_DISTANCE3: return kAirwindowsParams_Distance3.size();
            case EFFECT_DITHERBOX: return kAirwindowsParams_Ditherbox.size();
            case DIST_DONUT: return kAirwindowsParams_Donut.size();
            case EFFECT_DOUBLELAY: return kAirwindowsParams_Doublelay.size();
            case EFFECT_DOUBLE_PAUL: return kAirwindowsParams_DoublePaul.size();
            case DYN_DRUM_SLAM: return kAirwindowsParams_DrumSlam.size();
            case EFFECT_DUB_CENTER: return kAirwindowsParams_DubCenter.size();
            case TAPE_DUB_PLATE: return kAirwindowsParams_DubPlate.size();
            case TAPE_DUB_PLATE2: return kAirwindowsParams_DubPlate2.size();
            case EFFECT_DUB_SUB2: return kAirwindowsParams_DubSub2.size();
            case EFFECT_DUST_BUNNY: return kAirwindowsParams_DustBunny.size();
            case DYN_DYNAMICS: return kAirwindowsParams_Dynamics.size();
            case DYN_DYNAMICS2: return kAirwindowsParams_Dynamics2.size();
            case DYN_DYNAMICS3: return kAirwindowsParams_Dynamics3.size();
            case DYN_DYNO: return kAirwindowsParams_Dyno.size();
            case DIST_EDGE: return kAirwindowsParams_Edge.size();
            case EFFECT_ED_IS_DIM: return kAirwindowsParams_EdIsDim.size();
            case SAT_ELATION: return kAirwindowsParams_Elation.size();
            case FILTER_ELLIPTICAL: return kAirwindowsParams_Elliptical.size();
            case EFFECT_ENSEMBLE: return kAirwindowsParams_Ensemble.size();
            case FILTER_EQ: return kAirwindowsParams_EQ.size();
            case EFFECT_EVERY_SLEW: return kAirwindowsParams_EverySlew.size();
            case EFFECT_EVERY_TRIM: return kAirwindowsParams_EveryTrim.size();
            case FILTER_FAT_EQ: return kAirwindowsParams_FatEQ.size();
            case DYN_FINAL_CLIP: return kAirwindowsParams_FinalClip.size();
            case DYN_FLOOR: return kAirwindowsParams_Floor.size();
            case TAPE_FROM_TAPE: return kAirwindowsParams_FromTape.size();
            case DYN_GATELOPE: return kAirwindowsParams_Gatelope.size();
            case EFFECT_GOLDEN_SLEW: return kAirwindowsParams_GoldenSlew.size();
            case DIST_GOLEM: return kAirwindowsParams_Golem.size();
            case FILTER_HERMEPASS: return kAirwindowsParams_Hermepass.size();
            case FILTER_HERME_TRIM: return kAirwindowsParams_HermeTrim.size();
            case DYN_HIGH_IMPACT: return kAirwindowsParams_HighImpact.size();
            case DIST_HIP_CRUSH: return kAirwindowsParams_HipCrush.size();
            case FILTER_HOLT2: return kAirwindowsParams_Holt2.size();
            case EFFECT_HOMBRE: return kAirwindowsParams_Hombre.size();
            case FILTER_HULL: return kAirwindowsParams_Hull.size();
            case FILTER_HULL2: return kAirwindowsParams_Hull2.size();
            case DIST_HYPE: return kAirwindowsParams_Hype.size();
            case DYN_HYPERSOFT: return kAirwindowsParams_Hypersoft.size();
            case FILTER_HYPERSONIC: return kAirwindowsParams_Hypersonic.size();
            case FILTER_HYPERSON_X: return kAirwindowsParams_HypersonX.size();
            case DIST_INFLAMER: return kAirwindowsParams_Inflamer.size();
            case FILTER_INFRASONIC: return kAirwindowsParams_Infrasonic.size();
            case TAPE_IRON_OXIDE_CLASSIC: return kAirwindowsParams_IronOxideClassic.size();
            case TAPE_IRON_OXIDE_CLASSIC2: return kAirwindowsParams_IronOxideClassic2.size();
            case FILTER_ISOLATOR: return kAirwindowsParams_Isolator.size();
            case FILTER_ISOLATOR2: return kAirwindowsParams_Isolator2.size();
            case FILTER_ISOLATOR3: return kAirwindowsParams_Isolator3.size();
            case FILTER_KALMAN: return kAirwindowsParams_Kalman.size();
            case EFFECT_LEFTO_MONO: return kAirwindowsParams_LeftoMono.size();
            case DYN_LOGICAL4: return kAirwindowsParams_Logical4.size();
            case REVERB_LR_CONVOLVE: return kAirwindowsParams_LRConvolve.size();
            case REVERB_LR_CONVOLVE2: return kAirwindowsParams_LRConvolve2.size();
            case REVERB_LR_CONVOLVE3: return kAirwindowsParams_LRConvolve3.size();
            case EFFECT_LR_FLIP_TIMER: return kAirwindowsParams_LRFlipTimer.size();
            case SAT_LUXOR: return kAirwindowsParams_Luxor.size();
            case FILTER_MACK_EQ: return kAirwindowsParams_MackEQ.size();
            case DYN_MASTERING: return kAirwindowsParams_Mastering.size();
            case DYN_MASTERING2: return kAirwindowsParams_Mastering2.size();
            case AMPSIM_MID_AMP: return kAirwindowsParams_MidAmp.size();
            case EFFECT_MONITORING2: return kAirwindowsParams_Monitoring2.size();
            case EFFECT_MONITORING3: return kAirwindowsParams_Monitoring3.size();
            case EFFECT_MS_FLIP_TIMER: return kAirwindowsParams_MSFlipTimer.size();
            case DIST_MULTI_BAND_DISTORTION: return kAirwindowsParams_MultiBandDistortion.size();
            case REVERB_NEVERLAND: return kAirwindowsParams_Neverland.size();
            case EFFECT_NOT_JUST_ANOTHER_CD: return kAirwindowsParams_NotJustAnotherCD.size();
            case EFFECT_ORBIT_KICK: return kAirwindowsParams_OrbitKick.size();
            case EFFECT_OVERHEADS: return kAirwindowsParams_Overheads.size();
            case FILTER_PARAMETRIC: return kAirwindowsParams_Parametric.size();
            case EFFECT_PAUL_WIDE: return kAirwindowsParams_PaulWide.size();
            case SAT_PD_BUSS: return kAirwindowsParams_PDBuss.size();
            case SAT_PD_CHANNEL: return kAirwindowsParams_PDChannel.size();
            case DYN_PEAKS_ONLY: return kAirwindowsParams_PeaksOnly.size();
            case FILTER_PEAR_EQ: return kAirwindowsParams_PearEQ.size();
            case FILTER_PEAR_LITE_EQ: return kAirwindowsParams_PearLiteEQ.size();
            case EFFECT_PITCH_DELAY: return kAirwindowsParams_PitchDelay.size();
            case EFFECT_POCKEY: return kAirwindowsParams_Pockey.size();
            case EFFECT_POCKEY2: return kAirwindowsParams_Pockey2.size();
            case DYN_PODCAST: return kAirwindowsParams_Podcast.size();
            case DYN_PODCAST_DELUXE: return kAirwindowsParams_PodcastDeluxe.size();
            case AMPSIM_POINTY_DELUXE: return kAirwindowsParams_PointyDeluxe.size();
            case AMPSIM_POINTY_GUITAR: return kAirwindowsParams_PointyGuitar.size();
            case DIST_POP: return kAirwindowsParams_Pop.size();
            case DIST_POP2: return kAirwindowsParams_Pop2.size();
            case DIST_POP3: return kAirwindowsParams_Pop3.size();
            case SAT_PRECIOUS: return kAirwindowsParams_Precious.size();
            case DYN_PRESSURE6: return kAirwindowsParams_Pressure6.size();
            case FILTER_PRIME_FIR: return kAirwindowsParams_PrimeFIR.size();
            case AMPSIM_PUNCHY_DELUXE: return kAirwindowsParams_PunchyDeluxe.size();
            case AMPSIM_PUNCHY_GUITAR: return kAirwindowsParams_PunchyGuitar.size();
            case EFFECT_PUREST_DUAL_PAN: return kAirwindowsParams_PurestDualPan.size();
            case EFFECT_PUREST_ECHO: return kAirwindowsParams_PurestEcho.size();
            case SAT_PUREST_WARM2: return kAirwindowsParams_PurestWarm2.size();
            case DYN_PYEWACKET: return kAirwindowsParams_Pyewacket.size();
            case EFFECT_RAW_GLITTERS: return kAirwindowsParams_RawGlitters.size();
            case EFFECT_RAW_TIMBERS: return kAirwindowsParams_RawTimbers.size();
            case EFFECT_RECURVE: return kAirwindowsParams_Recurve.size();
            case EFFECT_REMAP: return kAirwindowsParams_Remap.size();
            case FILTER_RES_EQ: return kAirwindowsParams_ResEQ.size();
            case FILTER_RES_EQ_2: return kAirwindowsParams_ResEQ2.size();
            case EFFECT_RIGHTO_MONO: return kAirwindowsParams_RightoMono.size();
            case DIST_SHAPE: return kAirwindowsParams_Shape.size();
            case FILTER_SIDE_DULL: return kAirwindowsParams_SideDull.size();
            case FILTER_SIDEPASS: return kAirwindowsParams_Sidepass.size();
            case EFFECT_SILHOUETTE: return kAirwindowsParams_Silhouette.size();
            case FILTER_SILKEN: return kAirwindowsParams_Silken.size();
            case SAT_SINEW: return kAirwindowsParams_Sinew.size();
            case AMPSIM_SINGLE_ENDED_TRIODE: return kAirwindowsParams_SingleEndedTriode.size();
            case EFFECT_SLEW3: return kAirwindowsParams_Slew3.size();
            case EFFECT_SLEW4: return kAirwindowsParams_Slew4.size();
            case FILTER_SMOOTH_EQ_2: return kAirwindowsParams_SmoothEQ2.size();
            case FILTER_SMOOTH_EQ_3: return kAirwindowsParams_SmoothEQ3.size();
            case EFFECT_SOFT_CLOCK: return kAirwindowsParams_SoftClock.size();
            case EFFECT_SOFT_CLOCK2: return kAirwindowsParams_SoftClock2.size();
            case EFFECT_SOFT_CLOCK3: return kAirwindowsParams_SoftClock3.size();
            case DYN_SOFT_GATE: return kAirwindowsParams_SoftGate.size();
            case SAT_SQUARE_ROOT: return kAirwindowsParams_SquareRoot.size();
            case EFFECT_SRSLY2: return kAirwindowsParams_Srsly2.size();
            case EFFECT_SRSLY3: return kAirwindowsParams_Srsly3.size();
            case REVERB_STAR_CHILD: return kAirwindowsParams_StarChild.size();
            case REVERB_STAR_CHILD2: return kAirwindowsParams_StarChild2.size();
            case EFFECT_STEREO_CHORUS: return kAirwindowsParams_StereoChorus.size();
            case EFFECT_STEREO_DOUBLER: return kAirwindowsParams_StereoDoubler.size();
            case EFFECT_STEREO_ENSEMBLE: return kAirwindowsParams_StereoEnsemble.size();
            case EFFECT_STEREO_FX: return kAirwindowsParams_StereoFX.size();
            case DIST_STONEFIRE: return kAirwindowsParams_Stonefire.size();
            case DYN_STONE_FIRE_COMP: return kAirwindowsParams_StoneFireComp.size();
            case EFFECT_SWEET_WIDE: return kAirwindowsParams_SweetWide.size();
            case DYN_SWELL: return kAirwindowsParams_Swell.size();
            case TAPE_TAPE: return kAirwindowsParams_Tape.size();
            case TAPE_TAPE_BIAS: return kAirwindowsParams_TapeBias.size();
            case EFFECT_TAPE_DELAY: return kAirwindowsParams_TapeDelay.size();
            case EFFECT_TAPE_DELAY2: return kAirwindowsParams_TapeDelay2.size();
            case TAPE_TAPE_DUST: return kAirwindowsParams_TapeDust.size();
            case TAPE_TAPE_FAT: return kAirwindowsParams_TapeFat.size();
            case TAPE_TAPE_HACK: return kAirwindowsParams_TapeHack.size();
            case TAPE_TAPE_HACK2: return kAirwindowsParams_TapeHack2.size();
            case TAPE_TAPE_HACK3: return kAirwindowsParams_TapeHack3.size();
            case EFFECT_TEXTURIZE: return kAirwindowsParams_Texturize.size();
            case EFFECT_TEXTURIZE_MS: return kAirwindowsParams_TexturizeMS.size();
            case REVERB_THUNDER: return kAirwindowsParams_Thunder.size();
            case TAPE_TO_TAPE7: return kAirwindowsParams_ToTape7.size();
            case TAPE_TO_TAPE8: return kAirwindowsParams_ToTape8.size();
            case TAPE_TO_TAPE9: return kAirwindowsParams_ToTape9.size();
            case EFFECT_TPDF_WIDE: return kAirwindowsParams_TPDFWide.size();
            case SAT_TRANS_DESK: return kAirwindowsParams_TransDesk.size();
            case EFFECT_TRIPLE_SPREAD: return kAirwindowsParams_TripleSpread.size();
            case AMPSIM_TUBE: return kAirwindowsParams_Tube.size();
            case AMPSIM_TUBE2: return kAirwindowsParams_Tube2.size();
            case EFFECT_U_LAW_DECODE: return kAirwindowsParams_uLawDecode.size();
            case EFFECT_U_LAW_ENCODE: return kAirwindowsParams_uLawEncode.size();
            case FILTER_ULTRASONIC: return kAirwindowsParams_Ultrasonic.size();
            case FILTER_ULTRASONIC_LITE: return kAirwindowsParams_UltrasonicLite.size();
            case FILTER_ULTRASONIC_MED: return kAirwindowsParams_UltrasonicMed.size();
            case FILTER_ULTRASON_X: return kAirwindowsParams_UltrasonX.size();
            case EFFECT_VIBRATO: return kAirwindowsParams_Vibrato.size();
            case REVERB_VOICE_OF_THE_STARSHIP: return kAirwindowsParams_VoiceOfTheStarship.size();
            case EFFECT_VOICE_TRICK: return kAirwindowsParams_VoiceTrick.size();
            case EFFECT_WOLFBOT: return kAirwindowsParams_Wolfbot.size();
            case REVERB_WOODEN_BOX: return kAirwindowsParams_WoodenBox.size();
            case DYN_X2_BUSS: return kAirwindowsParams_X2Buss.size();
            case FILTER_Y_NOT_BANDPASS: return kAirwindowsParams_YNotBandpass.size();
            case FILTER_Z_ACID_LOWPASS: return kAirwindowsParams_ZAcidLowpass.size();
            case FILTER_Z_BANDPASS2: return kAirwindowsParams_ZBandpass2.size();
            case FILTER_Z_HIGHPASS2: return kAirwindowsParams_ZHighpass2.size();
            case FILTER_Z_LOWPASS: return kAirwindowsParams_ZLowpass.size();
            case FILTER_Z_LOWPASS2: return kAirwindowsParams_ZLowpass2.size();
            case FILTER_Z_NOTCH: return kAirwindowsParams_ZNotch.size();
            case FILTER_Z_NOTCH2: return kAirwindowsParams_ZNotch2.size();
            case EFFECT_ZOOM: return kAirwindowsParams_Zoom.size();
            case EFFECT_ECHOJAM: return kAirwindowsParams_Echojam.size();
            case DYN_Z_OUTPUT_STAGE: return kAirwindowsParams_ZOutputStage.size();
            case DYN_ULTRALIGHT3: return kAirwindowsParams_Ultralight3.size();
            case FILTER_Z_REGION: return kAirwindowsParams_ZRegion.size();
            case FILTER_Z_REGION2: return kAirwindowsParams_ZRegion2.size();

            // === NEW: 64 plugins (extracted params where available) ===
            case CLIP_ADCLIP9: return 0;
            case FILTER_BEZ_EQ3: return 0;
            case FILTER_BEZ_EQ4: return 0;
            case SAT_CONSOLE0_BUSS: return kAirwindowsParams_Console0Buss.size();
            case SAT_CONSOLE0_CHANNEL: return kAirwindowsParams_Console0Channel.size();
            case SAT_CONSOLE4_BUSS: return kAirwindowsParams_Console4Buss.size();
            case SAT_CONSOLE4_CHANNEL: return kAirwindowsParams_Console4Channel.size();
            case SAT_CONSOLE5_BUSS: return kAirwindowsParams_Console5Buss.size();
            case SAT_CONSOLE5_CHANNEL: return kAirwindowsParams_Console5Channel.size();
            case SAT_CONSOLE5_DARKCH: return kAirwindowsParams_Console5DarkCh.size();
            case SAT_CONSOLE6_BUSS: return kAirwindowsParams_Console6Buss.size();
            case SAT_CONSOLE6_CHANNEL: return kAirwindowsParams_Console6Channel.size();
            case SAT_CONSOLE7_BUSS: return kAirwindowsParams_Console7Buss.size();
            case SAT_CONSOLE7_CASCADE: return kAirwindowsParams_Console7Cascade.size();
            case SAT_CONSOLE7_CHANNEL: return kAirwindowsParams_Console7Channel.size();
            case SAT_CONSOLE7_CRUNCH: return kAirwindowsParams_Console7Crunch.size();
            case SAT_CONSOLE8_BUSSHYPE: return kAirwindowsParams_Console8BussHype.size();
            case SAT_CONSOLE8_BUSSIN: return kAirwindowsParams_Console8BussIn.size();
            case SAT_CONSOLE8_BUSSOUT: return kAirwindowsParams_Console8BussOut.size();
            case SAT_CONSOLE8_CHANNELHYPE: return kAirwindowsParams_Console8ChannelHype.size();
            case SAT_CONSOLE8_CHANNELIN: return kAirwindowsParams_Console8ChannelIn.size();
            case SAT_CONSOLE8_CHANNELOUT: return kAirwindowsParams_Console8ChannelOut.size();
            case SAT_CONSOLE8_LITEBUSS: return kAirwindowsParams_Console8LiteBuss.size();
            case SAT_CONSOLE8_LITECHANNEL: return kAirwindowsParams_Console8LiteChannel.size();
            case SAT_CONSOLE8_SUBHYPE: return kAirwindowsParams_Console8SubHype.size();
            case SAT_CONSOLE8_SUBIN: return kAirwindowsParams_Console8SubIn.size();
            case SAT_CONSOLE8_SUBOUT: return kAirwindowsParams_Console8SubOut.size();
            case SAT_CONSOLE9_BUSS: return kAirwindowsParams_Console9Buss.size();
            case SAT_CONSOLE9_CHANNEL: return kAirwindowsParams_Console9Channel.size();
            case SAT_CONSOLEH_BUSS: return kAirwindowsParams_ConsoleHBuss.size();
            case SAT_CONSOLEH_CHANNEL: return kAirwindowsParams_ConsoleHChannel.size();
            case SAT_CONSOLEH_PRE: return kAirwindowsParams_ConsoleHPre.size();
            case SAT_CONSOLE_LABUSS: return kAirwindowsParams_ConsoleLABuss.size();
            case SAT_CONSOLE_LA_CHANNEL: return kAirwindowsParams_ConsoleLAChannel.size();
            case SAT_CONSOLE_MC_BUSS: return kAirwindowsParams_ConsoleMCBuss.size();
            case SAT_CONSOLE_MC_CHANNEL: return kAirwindowsParams_ConsoleMCChannel.size();
            case SAT_CONSOLE_MD_BUSS: return kAirwindowsParams_ConsoleMDBuss.size();
            case SAT_CONSOLE_MD_CHANNEL: return kAirwindowsParams_ConsoleMDChannel.size();
            case SAT_CONSOLEX_BUSS: return kAirwindowsParams_ConsoleXBuss.size();
            case SAT_CONSOLEX_CHANNEL: return kAirwindowsParams_ConsoleXChannel.size();
            case SAT_CONSOLEX_PRE: return kAirwindowsParams_ConsoleXPre.size();
            case SAT_CONSOLEX2_BUSS: return kAirwindowsParams_ConsoleX2Buss.size();
            case SAT_CONSOLEX2_CHANNEL: return kAirwindowsParams_ConsoleX2Channel.size();
            case SAT_CONSOLEX2_PRE: return kAirwindowsParams_ConsoleX2Pre.size();
            case SAT_CONSOLEX3_BUSS: return 0;
            case SAT_CONSOLEX3_CHANNEL: return 0;
            case SAT_CONSOLEX3_PRE: return 0;
            case EFFECT_DEREZ4: return 0;
            case SAT_EVERYCONSOLE: return kAirwindowsParams_EveryConsole.size();
            case REVERB_KCYBERCITY: return 0;
            case REVERB_KROCKSTAR: return 0;
            case SAT_LONGHAND: return 0;
            case SAT_PURESTCONSOLE_BUSS: return kAirwindowsParams_PurestConsoleBuss.size();
            case SAT_PURESTCONSOLE_CHANNEL: return kAirwindowsParams_PurestConsoleChannel.size();
            case SAT_PURESTCONSOLE2_BUSS: return kAirwindowsParams_PurestConsole2Buss.size();
            case SAT_PURESTCONSOLE2_CHANNEL: return kAirwindowsParams_PurestConsole2Channel.size();
            case SAT_PURESTCONSOLE3_BUSS: return kAirwindowsParams_PurestConsole3Buss.size();
            case SAT_PURESTCONSOLE3_CHANNEL: return kAirwindowsParams_PurestConsole3Channel.size();
            case SAT_PURESTCONSOLE4_BUSS: return 0;
            case SAT_PURESTCONSOLE4_CHANNEL: return 0;
            case FILTER_SUZAN: return 0;
            case FILTER_YNOTHIGHPASS: return kAirwindowsParams_YNotHighpass.size();
            case FILTER_YNOTLOWPASS: return kAirwindowsParams_YNotLowpass.size();
            case FILTER_YNOTNOTCH: return kAirwindowsParams_YNotNotch.size();

            default:
                return 0;
        }
        // END GENERATED: getExtractedParamCount switch

    }
    
    // Three-tier fallback for parameter defaults
    static inline float getParamDefault(Type type, int paramIndex) {
        if ((int)type <= (int)NONE || (int)type >= (int)COUNT) {
            return 0.5f;
        }

        // Step 1: Check if paramIndex is within extracted param count
        int extractedCount = getExtractedParamCount(type);
        if (paramIndex >= 0 && paramIndex < extractedCount) {
            const AirwindowsExtractedParam* extracted = getExtractedParam(type, paramIndex);
            if (extracted) return extracted->defaultValue;
        }
        
        // Step 2: Fall back to spec default
        if (paramIndex >= 0) {
            const AirwindowsSpec spec = getSpec(type);
            if (paramIndex < spec.count) {
                return spec.defaults[paramIndex];
            }
        }
        
        // Step 3: Use neutral 0.5f as final fallback
        return 0.5f;
    }
    
    static const char* getParamDisplayName(Type type, int paramIndex, bool suppressWet = false);
    
    static bool isWetParam(const char* name);
    static bool hasInternalWet(Type type); // Phase 1: Check if plugin has internal wet parameter
    static Category getCategory(Type type);
    static const char* getCategoryName(Category cat);
    static const char* getDisplayName(Type type);
    static Type getTypeFromName(const char* name);
    static inline bool isStrictXYZCoreType(Type t) {
        return t >= XYZ_XBANDPASS && t <= XYZ_ZHIGHPASS;
    }
    static bool isInCategory(Type t, Category c);
    static int getTotalCount() { return COUNT - 1; }
    static inline Type sanitizeTypeValue(float rawType) {
        if (!std::isfinite(rawType)) {
            return NONE;
        }

        const int typeInt = (int)std::lround(rawType);
        if (typeInt <= (int)NONE || typeInt >= (int)COUNT) {
            return NONE;
        }

        return (Type)typeInt;
    }
    
    // Phase 1: Duplicate removal (canonical vs deprecated types)
    static bool isCanonicalType(Type type);  // Returns true if type should be visible in picker
    static Type getCanonicalType(Type type); // Returns canonical type for any given type (for state loading)
    static AirSuiteBiquad::Type translateToBiquadType(Type t);
    static AirSuiteClipping::Type translateToClippingType(Type t);
    static AirSuiteDithers::Type translateToDithersType(Type t);
    static AirSuiteUtility::Type translateToUtilityType(Type t);
    static AirSuiteStereo::Type translateToStereoType(Type t);
    static AirSuiteBass::Type translateToBassType(Type t);
    static AirSuiteLoFi::Type translateToLoFiType(Type t);
    static AirSuiteNoise::Type translateToNoiseType(Type t);
    static AirSuiteBrightness::Type translateToBrightnessType(Type t);
    static AirSuiteSubtlety::Type translateToSubtletyType(Type t);
    static AirSuiteToneColour::Type translateToToneColourType(Type t);
    static AirSuiteXYZ::Type translateToXYZType(Type t);

    // ========== Instance Methods ==========
    AirSuiteUniversal();
    ~AirSuiteUniversal() = default;

    AirSuiteUniversal(const AirSuiteUniversal&) = delete;
    AirSuiteUniversal& operator=(const AirSuiteUniversal&) = delete;

    Type getCurrentType() const { return m_type; }
    int getCachedDispatchTypeForTesting() const { return m_cachedDispatchType; }
    void setSampleRate(double sr);
    void setType(Type type);
    void reset();
    void process(float& L, float& R, const float params[13]);

private:
    static Category computeRegistryCategory(Type type);
    static const char* computeRegistryDisplayName(Type type);
    static int resolveDispatchType(const RuntimeIdentity& identity);

    Type   m_type       = NONE;
    double m_sampleRate = 44100.0;
    const RuntimeIdentity* m_runtimeIdentity = nullptr;
    int m_cachedDispatchType = 0;

    // ========== Category Class Instances ==========
    // Each instance manages its own internal state and dispatches based on its own type
    AirSuiteReverb    m_reverbs;
    AirSuiteEffect    m_effects;
    AirSuiteAmpSim    m_ampSims;
    AirSuiteDistortion m_distortion;
    AirSuiteDynamics   m_dynamics;
    AirSuiteSaturation m_saturation;
    AirSuiteTape       m_tape;
    AirSuiteFilters    m_filters;
    AirSuiteBiquad     m_biquad;
    AirSuiteClipping   m_clipping;
    AirSuiteDithers    m_dithers;
    AirSuiteUtility    m_utility;
    AirSuiteStereo     m_stereo;
    AirSuiteBass       m_bass;
    AirSuiteLoFi       m_lofi;
    AirSuiteNoise      m_noise;
    AirSuiteBrightness   m_brightness;
    AirSuiteSubtlety m_subtlety;
    AirSuiteToneColour m_toneColour;
    AirSuiteXYZ m_xyz;
};

END_NAMESPACE_DISTRHO

#endif // AIR_SUITE_UNIVERSAL_HPP
