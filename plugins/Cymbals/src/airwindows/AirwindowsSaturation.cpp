#include "AirwindowsSaturation.hpp"
#include "AirwindowsAdapter.hpp"
#include "AirwindowsFactories.inc"
#include "AirwindowsExtractedParams.hpp"
#include "vst2stub/audioeffectx.h"
#include <cstring>
#include <cmath>

START_NAMESPACE_DISTRHO

extern "C" {
    AudioEffectX* aw_create_BuildAtPDF();
    AudioEffectX* aw_create_CompresaTurator();
    AudioEffectX* aw_create_DitherFloat();
    AudioEffectX* aw_create_PowerSag();
    AudioEffectX* aw_create_Nikola();
    AudioEffectX* aw_create_ShortBuss();
    AudioEffectX* aw_create_PurestSaturation();
    AudioEffectX* aw_create_AtmosphereBuss();
    AudioEffectX* aw_create_AtmosphereChannel();
    AudioEffectX* aw_create_BussColors4();
    AudioEffectX* aw_create_C5RawBuss();
    AudioEffectX* aw_create_C5RawChannel();
    AudioEffectX* aw_create_Cider();
    AudioEffectX* aw_create_Elation();
    AudioEffectX* aw_create_Luxor();
    AudioEffectX* aw_create_PDBuss();
    AudioEffectX* aw_create_PDChannel();
    AudioEffectX* aw_create_Precious();
    AudioEffectX* aw_create_PurestWarm2();
    AudioEffectX* aw_create_Sinew();
    AudioEffectX* aw_create_SquareRoot();
    AudioEffectX* aw_create_TransDesk();
    AudioEffectX* aw_create_Console0Buss();
    AudioEffectX* aw_create_Console0Channel();
    AudioEffectX* aw_create_Console4Buss();
    AudioEffectX* aw_create_Console4Channel();
    AudioEffectX* aw_create_Console5Buss();
    AudioEffectX* aw_create_Console5Channel();
    AudioEffectX* aw_create_Console5DarkCh();
    AudioEffectX* aw_create_Console6Buss();
    AudioEffectX* aw_create_Console6Channel();
    AudioEffectX* aw_create_Console7Buss();
    AudioEffectX* aw_create_Console7Cascade();
    AudioEffectX* aw_create_Console7Channel();
    AudioEffectX* aw_create_Console7Crunch();
    AudioEffectX* aw_create_Console8BussHype();
    AudioEffectX* aw_create_Console8BussIn();
    AudioEffectX* aw_create_Console8BussOut();
    AudioEffectX* aw_create_Console8ChannelHype();
    AudioEffectX* aw_create_Console8ChannelIn();
    AudioEffectX* aw_create_Console8ChannelOut();
    AudioEffectX* aw_create_Console8LiteBuss();
    AudioEffectX* aw_create_Console8LiteChannel();
    AudioEffectX* aw_create_Console8SubHype();
    AudioEffectX* aw_create_Console8SubIn();
    AudioEffectX* aw_create_Console8SubOut();
    AudioEffectX* aw_create_Console9Buss();
    AudioEffectX* aw_create_Console9Channel();
    AudioEffectX* aw_create_ConsoleHBuss();
    AudioEffectX* aw_create_ConsoleHChannel();
    AudioEffectX* aw_create_ConsoleHPre();
    AudioEffectX* aw_create_ConsoleLABuss();
    AudioEffectX* aw_create_ConsoleLAChannel();
    AudioEffectX* aw_create_ConsoleMCBuss();
    AudioEffectX* aw_create_ConsoleMCChannel();
    AudioEffectX* aw_create_ConsoleMDBuss();
    AudioEffectX* aw_create_ConsoleMDChannel();
    AudioEffectX* aw_create_ConsoleXBuss();
    AudioEffectX* aw_create_ConsoleXChannel();
    AudioEffectX* aw_create_ConsoleXPre();
    AudioEffectX* aw_create_ConsoleX2Buss();
    AudioEffectX* aw_create_ConsoleX2Channel();
    AudioEffectX* aw_create_ConsoleX2Pre();
    AudioEffectX* aw_create_ConsoleX3Buss();
    AudioEffectX* aw_create_ConsoleX3Channel();
    AudioEffectX* aw_create_ConsoleX3Pre();
    AudioEffectX* aw_create_ConsoleX3();
    AudioEffectX* aw_create_PurestWarm3();
    AudioEffectX* aw_create_EveryConsole();
    AudioEffectX* aw_create_Longhand();
    AudioEffectX* aw_create_PurestConsoleBuss();
    AudioEffectX* aw_create_PurestConsoleChannel();
    AudioEffectX* aw_create_PurestConsole2Buss();
    AudioEffectX* aw_create_PurestConsole2Channel();
    AudioEffectX* aw_create_PurestConsole3Buss();
    AudioEffectX* aw_create_PurestConsole3Channel();
    AudioEffectX* aw_create_PurestConsole4Buss();
    AudioEffectX* aw_create_PurestConsole4Channel();
}

// Maps a SAT_* Console/PurestConsole type to its extracted real-plugin param array.
// Types with no extracted array (ConsoleX3*, PurestConsole4*) yield an empty range so
// the spec stays count=0 (no setParameter calls -> no throw in the real VST).
struct SaturationExtractedRange {
    const AirwindowsExtractedParam* data;
    int size;
};

static SaturationExtractedRange saturationConsoleParams(AirSuiteSaturation::Type t) {
    switch (t) {
        case AirSuiteSaturation::SAT_CONSOLE0_BUSS: return {kAirwindowsParams_Console0Buss.data(), (int)kAirwindowsParams_Console0Buss.size()};
        case AirSuiteSaturation::SAT_CONSOLE0_CHANNEL: return {kAirwindowsParams_Console0Channel.data(), (int)kAirwindowsParams_Console0Channel.size()};
        case AirSuiteSaturation::SAT_CONSOLE4_BUSS: return {kAirwindowsParams_Console4Buss.data(), (int)kAirwindowsParams_Console4Buss.size()};
        case AirSuiteSaturation::SAT_CONSOLE4_CHANNEL: return {kAirwindowsParams_Console4Channel.data(), (int)kAirwindowsParams_Console4Channel.size()};
        case AirSuiteSaturation::SAT_CONSOLE5_BUSS: return {kAirwindowsParams_Console5Buss.data(), (int)kAirwindowsParams_Console5Buss.size()};
        case AirSuiteSaturation::SAT_CONSOLE5_CHANNEL: return {kAirwindowsParams_Console5Channel.data(), (int)kAirwindowsParams_Console5Channel.size()};
        case AirSuiteSaturation::SAT_CONSOLE5_DARKCH: return {kAirwindowsParams_Console5DarkCh.data(), (int)kAirwindowsParams_Console5DarkCh.size()};
        case AirSuiteSaturation::SAT_CONSOLE6_BUSS: return {kAirwindowsParams_Console6Buss.data(), (int)kAirwindowsParams_Console6Buss.size()};
        case AirSuiteSaturation::SAT_CONSOLE6_CHANNEL: return {kAirwindowsParams_Console6Channel.data(), (int)kAirwindowsParams_Console6Channel.size()};
        case AirSuiteSaturation::SAT_CONSOLE7_BUSS: return {kAirwindowsParams_Console7Buss.data(), (int)kAirwindowsParams_Console7Buss.size()};
        case AirSuiteSaturation::SAT_CONSOLE7_CASCADE: return {kAirwindowsParams_Console7Cascade.data(), (int)kAirwindowsParams_Console7Cascade.size()};
        case AirSuiteSaturation::SAT_CONSOLE7_CHANNEL: return {kAirwindowsParams_Console7Channel.data(), (int)kAirwindowsParams_Console7Channel.size()};
        case AirSuiteSaturation::SAT_CONSOLE7_CRUNCH: return {kAirwindowsParams_Console7Crunch.data(), (int)kAirwindowsParams_Console7Crunch.size()};
        case AirSuiteSaturation::SAT_CONSOLE8_BUSSHYPE: return {kAirwindowsParams_Console8BussHype.data(), (int)kAirwindowsParams_Console8BussHype.size()};
        case AirSuiteSaturation::SAT_CONSOLE8_BUSSIN: return {kAirwindowsParams_Console8BussIn.data(), (int)kAirwindowsParams_Console8BussIn.size()};
        case AirSuiteSaturation::SAT_CONSOLE8_BUSSOUT: return {kAirwindowsParams_Console8BussOut.data(), (int)kAirwindowsParams_Console8BussOut.size()};
        case AirSuiteSaturation::SAT_CONSOLE8_CHANNELHYPE: return {kAirwindowsParams_Console8ChannelHype.data(), (int)kAirwindowsParams_Console8ChannelHype.size()};
        case AirSuiteSaturation::SAT_CONSOLE8_CHANNELIN: return {kAirwindowsParams_Console8ChannelIn.data(), (int)kAirwindowsParams_Console8ChannelIn.size()};
        case AirSuiteSaturation::SAT_CONSOLE8_CHANNELOUT: return {kAirwindowsParams_Console8ChannelOut.data(), (int)kAirwindowsParams_Console8ChannelOut.size()};
        case AirSuiteSaturation::SAT_CONSOLE8_LITEBUSS: return {kAirwindowsParams_Console8LiteBuss.data(), (int)kAirwindowsParams_Console8LiteBuss.size()};
        case AirSuiteSaturation::SAT_CONSOLE8_LITECHANNEL: return {kAirwindowsParams_Console8LiteChannel.data(), (int)kAirwindowsParams_Console8LiteChannel.size()};
        case AirSuiteSaturation::SAT_CONSOLE8_SUBHYPE: return {kAirwindowsParams_Console8SubHype.data(), (int)kAirwindowsParams_Console8SubHype.size()};
        case AirSuiteSaturation::SAT_CONSOLE8_SUBIN: return {kAirwindowsParams_Console8SubIn.data(), (int)kAirwindowsParams_Console8SubIn.size()};
        case AirSuiteSaturation::SAT_CONSOLE8_SUBOUT: return {kAirwindowsParams_Console8SubOut.data(), (int)kAirwindowsParams_Console8SubOut.size()};
        case AirSuiteSaturation::SAT_CONSOLE9_BUSS: return {kAirwindowsParams_Console9Buss.data(), (int)kAirwindowsParams_Console9Buss.size()};
        case AirSuiteSaturation::SAT_CONSOLE9_CHANNEL: return {kAirwindowsParams_Console9Channel.data(), (int)kAirwindowsParams_Console9Channel.size()};
        case AirSuiteSaturation::SAT_CONSOLEH_BUSS: return {kAirwindowsParams_ConsoleHBuss.data(), (int)kAirwindowsParams_ConsoleHBuss.size()};
        case AirSuiteSaturation::SAT_CONSOLEH_CHANNEL: return {kAirwindowsParams_ConsoleHChannel.data(), (int)kAirwindowsParams_ConsoleHChannel.size()};
        case AirSuiteSaturation::SAT_CONSOLEH_PRE: return {kAirwindowsParams_ConsoleHPre.data(), (int)kAirwindowsParams_ConsoleHPre.size()};
        case AirSuiteSaturation::SAT_CONSOLE_LABUSS: return {kAirwindowsParams_ConsoleLABuss.data(), (int)kAirwindowsParams_ConsoleLABuss.size()};
        case AirSuiteSaturation::SAT_CONSOLE_LA_CHANNEL: return {kAirwindowsParams_ConsoleLAChannel.data(), (int)kAirwindowsParams_ConsoleLAChannel.size()};
        case AirSuiteSaturation::SAT_CONSOLE_MC_BUSS: return {kAirwindowsParams_ConsoleMCBuss.data(), (int)kAirwindowsParams_ConsoleMCBuss.size()};
        case AirSuiteSaturation::SAT_CONSOLE_MC_CHANNEL: return {kAirwindowsParams_ConsoleMCChannel.data(), (int)kAirwindowsParams_ConsoleMCChannel.size()};
        case AirSuiteSaturation::SAT_CONSOLE_MD_BUSS: return {kAirwindowsParams_ConsoleMDBuss.data(), (int)kAirwindowsParams_ConsoleMDBuss.size()};
        case AirSuiteSaturation::SAT_CONSOLE_MD_CHANNEL: return {kAirwindowsParams_ConsoleMDChannel.data(), (int)kAirwindowsParams_ConsoleMDChannel.size()};
        case AirSuiteSaturation::SAT_CONSOLEX_BUSS: return {kAirwindowsParams_ConsoleXBuss.data(), (int)kAirwindowsParams_ConsoleXBuss.size()};
        case AirSuiteSaturation::SAT_CONSOLEX_CHANNEL: return {kAirwindowsParams_ConsoleXChannel.data(), (int)kAirwindowsParams_ConsoleXChannel.size()};
        case AirSuiteSaturation::SAT_CONSOLEX_PRE: return {kAirwindowsParams_ConsoleXPre.data(), (int)kAirwindowsParams_ConsoleXPre.size()};
        case AirSuiteSaturation::SAT_CONSOLEX2_BUSS: return {kAirwindowsParams_ConsoleX2Buss.data(), (int)kAirwindowsParams_ConsoleX2Buss.size()};
        case AirSuiteSaturation::SAT_CONSOLEX2_CHANNEL: return {kAirwindowsParams_ConsoleX2Channel.data(), (int)kAirwindowsParams_ConsoleX2Channel.size()};
        case AirSuiteSaturation::SAT_CONSOLEX2_PRE: return {kAirwindowsParams_ConsoleX2Pre.data(), (int)kAirwindowsParams_ConsoleX2Pre.size()};
        case AirSuiteSaturation::SAT_EVERYCONSOLE: return {kAirwindowsParams_EveryConsole.data(), (int)kAirwindowsParams_EveryConsole.size()};
        case AirSuiteSaturation::SAT_PURESTCONSOLE_BUSS: return {kAirwindowsParams_PurestConsoleBuss.data(), (int)kAirwindowsParams_PurestConsoleBuss.size()};
        case AirSuiteSaturation::SAT_PURESTCONSOLE_CHANNEL: return {kAirwindowsParams_PurestConsoleChannel.data(), (int)kAirwindowsParams_PurestConsoleChannel.size()};
        case AirSuiteSaturation::SAT_PURESTCONSOLE2_BUSS: return {kAirwindowsParams_PurestConsole2Buss.data(), (int)kAirwindowsParams_PurestConsole2Buss.size()};
        case AirSuiteSaturation::SAT_PURESTCONSOLE2_CHANNEL: return {kAirwindowsParams_PurestConsole2Channel.data(), (int)kAirwindowsParams_PurestConsole2Channel.size()};
        case AirSuiteSaturation::SAT_PURESTCONSOLE3_BUSS: return {kAirwindowsParams_PurestConsole3Buss.data(), (int)kAirwindowsParams_PurestConsole3Buss.size()};
        case AirSuiteSaturation::SAT_PURESTCONSOLE3_CHANNEL: return {kAirwindowsParams_PurestConsole3Channel.data(), (int)kAirwindowsParams_PurestConsole3Channel.size()};
        // SAT_CONSOLEX3_* and SAT_PURESTCONSOLE4_* have no extracted arrays -> empty.
        default: return {nullptr, 0};
    }
}

AirwindowsSpec AirSuiteSaturation::getSaturationSpec(AirSuiteSaturation::Type type) {
    AirwindowsSpec s;
    switch (type) {
        case BUILDATPDF:
        case COMPRESATURATOR:
            s.count = 2;
            s.names[0]="Amount"; s.defaults[0]=0.5f;
            s.names[1]="Dry/Wet"; s.defaults[1]=1.0f;
            break;
        case DITHERFLOAT:
            s.count = 2;
            s.names[0]="Amount"; s.defaults[0]=0.5f;
            s.names[1]="Dry/Wet"; s.defaults[1]=1.0f;
            break;
        case POWERSAG:
            s.count = 2;
            s.names[0]="Sag"; s.defaults[0]=0.0f;
            s.names[1]="Depth"; s.defaults[1]=0.5f;
            break;
        case NIKOLA:
            s.count = 2;
            s.names[0]="Voltage"; s.defaults[0]=0.0f;
            s.names[1]="Dry/Wet"; s.defaults[1]=1.0f;
            break;
        case SHORTBUSS:
            s.count = 2;
            s.names[0]="Drive"; s.defaults[0]=0.5f;
            s.names[1]="Dry/Wet"; s.defaults[1]=1.0f;
            break;
            s.count = 2;
            s.names[0]="Saturation"; s.defaults[0]=0.0f;
            s.names[1]="Dry/Wet"; s.defaults[1]=1.0f;
            break;
        case SAT_CONSOLE0_BUSS: case SAT_CONSOLE0_CHANNEL:
        case SAT_CONSOLE4_BUSS: case SAT_CONSOLE4_CHANNEL:
        case SAT_CONSOLE5_BUSS: case SAT_CONSOLE5_CHANNEL: case SAT_CONSOLE5_DARKCH:
        case SAT_CONSOLE6_BUSS: case SAT_CONSOLE6_CHANNEL:
        case SAT_CONSOLE7_BUSS: case SAT_CONSOLE7_CASCADE: case SAT_CONSOLE7_CHANNEL: case SAT_CONSOLE7_CRUNCH:
        case SAT_CONSOLE8_BUSSHYPE: case SAT_CONSOLE8_BUSSIN: case SAT_CONSOLE8_BUSSOUT:
        case SAT_CONSOLE8_CHANNELHYPE: case SAT_CONSOLE8_CHANNELIN: case SAT_CONSOLE8_CHANNELOUT:
        case SAT_CONSOLE8_LITEBUSS: case SAT_CONSOLE8_LITECHANNEL:
        case SAT_CONSOLE8_SUBHYPE: case SAT_CONSOLE8_SUBIN: case SAT_CONSOLE8_SUBOUT:
        case SAT_CONSOLE9_BUSS: case SAT_CONSOLE9_CHANNEL:
        case SAT_CONSOLEH_BUSS: case SAT_CONSOLEH_CHANNEL: case SAT_CONSOLEH_PRE:
        case SAT_CONSOLE_LABUSS: case SAT_CONSOLE_LA_CHANNEL:
        case SAT_CONSOLE_MC_BUSS: case SAT_CONSOLE_MC_CHANNEL:
        case SAT_CONSOLE_MD_BUSS: case SAT_CONSOLE_MD_CHANNEL:
        case SAT_CONSOLEX_BUSS: case SAT_CONSOLEX_CHANNEL: case SAT_CONSOLEX_PRE:
        case SAT_CONSOLEX2_BUSS: case SAT_CONSOLEX2_CHANNEL: case SAT_CONSOLEX2_PRE:
        case SAT_CONSOLEX3_BUSS: case SAT_CONSOLEX3_CHANNEL: case SAT_CONSOLEX3_PRE:
        case SAT_EVERYCONSOLE:
        case SAT_PURESTCONSOLE_BUSS: case SAT_PURESTCONSOLE_CHANNEL:
        case SAT_PURESTCONSOLE2_BUSS: case SAT_PURESTCONSOLE2_CHANNEL:
        case SAT_PURESTCONSOLE3_BUSS: case SAT_PURESTCONSOLE3_CHANNEL:
        case SAT_PURESTCONSOLE4_BUSS: case SAT_PURESTCONSOLE4_CHANNEL: {
            const SaturationExtractedRange range = saturationConsoleParams(type);
            for (int i = 0; i < AirwindowsSpec::kMaxParams && i < range.size; ++i) {
                const AirwindowsExtractedParam& p = range.data[i];
                s.names[i] = (p.raw_name[0] != '\0') ? p.raw_name : p.name;
                s.defaults[i] = p.defaultValue;
                s.units[i] = (p.label[0] != '\0') ? p.label : nullptr;
                s.count = i + 1;
            }
            break;
        }
        case SAT_CONSOLEX3: {
            s.count = 13;
            s.names[0]="High"; s.defaults[0]=0.5f;
            s.names[1]="HMid"; s.defaults[1]=0.5f;
            s.names[2]="LMid"; s.defaults[2]=0.5f;
            s.names[3]="Bass"; s.defaults[3]=0.5f;
            s.names[4]="HighF"; s.defaults[4]=0.5f;
            s.names[5]="HMidF"; s.defaults[5]=0.5f;
            s.names[6]="LMidF"; s.defaults[6]=0.5f;
            s.names[7]="BassF"; s.defaults[7]=0.5f;
            s.names[8]="Thresh"; s.defaults[8]=0.5f;
            s.names[9]="Attack"; s.defaults[9]=0.5f;
            s.names[10]="Release"; s.defaults[10]=0.5f;
            s.names[11]="Inv/Wet"; s.defaults[11]=0.5f;
            s.names[12]="Input"; s.defaults[12]=0.5f;
            break;
        }
        case SAT_PUREST_WARM3: {
            s.count = 1;
            s.names[0]="Asym"; s.defaults[0]=0.5f;
            break;
        }
        case SAT_LONGHAND:
            s.count = 3;
            s.names[0]="Input"; s.defaults[0]=0.445f;
            s.names[1]="Output"; s.defaults[1]=1.0f;
            s.names[2]="Dry/Wet"; s.defaults[2]=1.0f;
            break;
        default: break;
    }
    return s;
}

class AirSuiteSaturation::Impl {
public:
    Impl(float sr) : m_adapter(sr, awspec::createFor_AirSuiteSaturation, false) { m_adapter.setSampleRate(sr); }
    void setSampleRate(float sr) { m_adapter.setSampleRate(sr); }
    void setAlgorithm(AirSuiteSaturation::Type type) { m_adapter.setType((int)type); }
    void reset() { m_adapter.reset(); }
    void process(float& L, float& R, AirSuiteSaturation::Type type, float* params) {
        m_adapter.setType((int)type);
        m_adapter.process(L, R, params, AirSuiteSaturation::getSaturationSpec(type).count);
    }
private:
    AirwindowsAdapter m_adapter;
};

AirSuiteSaturation::AirSuiteSaturation(float sampleRate)
    : sampleRate(sampleRate), m_impl(std::make_unique<Impl>(sampleRate)) {}
    
AirSuiteSaturation::~AirSuiteSaturation() = default;

void AirSuiteSaturation::setSampleRate(float sr) {
    sampleRate = sr;
    if (m_impl) m_impl->setSampleRate(sr);
}

void AirSuiteSaturation::reset() {
    if (m_impl) m_impl->reset();
}

void AirSuiteSaturation::process(float& L, float& R, Type type, float* params) {
    if (m_impl) m_impl->process(L, R, type, params);
}

END_NAMESPACE_DISTRHO
