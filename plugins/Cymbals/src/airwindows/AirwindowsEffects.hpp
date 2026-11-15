#ifndef AIRWINDOWS_EFFECTS_HPP
#define AIRWINDOWS_EFFECTS_HPP

#include <memory>
#include "AirwindowsCommon.hpp"

START_NAMESPACE_DISTRHO

class AirSuiteEffect {
public:
    enum Type {
        NONE = 0,
        AURA, CLOUDCOAT, DISINTEGRATE,
        ENERGY, ENERGY2, EXCITER, FACET, FRACTURE, FRACTURE2,
        GALACTICVIBE, GLITCHSHIFTER, GRINGER, GUITARCONDITIONER,
        PAFNUTY2, PITCHNASTY, POWERSAG2,
        PREPONDERANT, RINGMODULATOR, TAKECARE,
        TREMOLO, TREMOSQUARE, TRIANGLIZER,
        ELECTROHAT, FLIPITY, MONITORING, MONOAM,
        EFFECT_ADT,
        EFFECT_AUTO_PAN,
        EFFECT_BEAM,
        EFFECT_BIT_DUAL_PAN,
        EFFECT_BIT_SHIFT_PAN,
        EFFECT_CANS,
        EFFECT_CANS_AW,
        EFFECT_CHORUS,
        EFFECT_CHORUS_ENSEMBLE,
        EFFECT_CONTENT_HIDE_D,
        EFFECT_CURVE,
        EFFECT_DC_VOLTAGE,
        EFFECT_DE_CRACKLE,
        EFFECT_DE_REZ,
        EFFECT_DE_REZ2,
        EFFECT_DE_REZ3,
        EFFECT_DEREZ4,
        EFFECT_DISCONTINUITY,
        EFFECT_DISTANCE,
        EFFECT_DISTANCE2,
        EFFECT_DISTANCE3,
        EFFECT_DITHERBOX,
        EFFECT_DOUBLELAY,
        EFFECT_DOUBLE_PAUL,
        EFFECT_DUB_CENTER,
        EFFECT_DUB_SUB2,
        EFFECT_DUST_BUNNY,
        EFFECT_ED_IS_DIM,
        EFFECT_ENSEMBLE,
        EFFECT_EVERY_SLEW,
        EFFECT_EVERY_TRIM,
        EFFECT_GOLDEN_SLEW,
        EFFECT_HOMBRE,
        EFFECT_LEFTO_MONO,
        EFFECT_LR_FLIP_TIMER,
        EFFECT_MONITORING2,
        EFFECT_MONITORING3,
        EFFECT_MS_FLIP_TIMER,
        EFFECT_NOT_JUST_ANOTHER_CD,
        EFFECT_ORBIT_KICK,
        EFFECT_OVERHEADS,
        EFFECT_PAUL_WIDE,
        EFFECT_PITCH_DELAY,
        EFFECT_POCKEY,
        EFFECT_POCKEY2,
        EFFECT_PUREST_DUAL_PAN,
        EFFECT_PUREST_ECHO,
        EFFECT_RAW_GLITTERS,
        EFFECT_RAW_TIMBERS,
        EFFECT_RECURVE,
        EFFECT_REMAP,
        EFFECT_RIGHTO_MONO,
        EFFECT_SILHOUETTE,
        EFFECT_SLEW3,
        EFFECT_SLEW4,
        EFFECT_SOFT_CLOCK,
        EFFECT_SOFT_CLOCK2,
        EFFECT_SOFT_CLOCK3,
        EFFECT_SRSLY2,
        EFFECT_SRSLY3,
        EFFECT_STEREO_CHORUS,
        EFFECT_STEREO_DOUBLER,
        EFFECT_STEREO_ENSEMBLE,
        EFFECT_STEREO_FX,
        EFFECT_SWEET_WIDE,
        EFFECT_TAPE_DELAY,
        EFFECT_TAPE_DELAY2,
        EFFECT_TEXTURIZE,
        EFFECT_TEXTURIZE_MS,
        EFFECT_TPDF_WIDE,
        EFFECT_TRIPLE_SPREAD,
        EFFECT_U_LAW_DECODE,
        EFFECT_U_LAW_ENCODE,
        EFFECT_VIBRATO,
        EFFECT_VOICE_TRICK,
        EFFECT_WOLFBOT,
        EFFECT_WEAVE,
        EFFECT_ZOOM,
        EFFECT_ECHOJAM,
        EFFECT_FAST_DISTANCE2,
        COUNT
    };

    using Spec = AirwindowsSpec;

    static Spec getEffectSpec(Type type);

    // Constructor and destructor
    AirSuiteEffect(float sampleRate = 44100.0f);
    ~AirSuiteEffect();

    // Public methods
    void setSampleRate(float sr);
    void reset();
    void process(float& outL, float& outR, Type type, float* params);

private:
    // Pimpl implementation class (defined in .cpp)
    class Impl;
    float sampleRate;
    std::unique_ptr<Impl> m_impl;
};

END_NAMESPACE_DISTRHO

#endif // AIRWINDOWS_EFFECTS_HPP
