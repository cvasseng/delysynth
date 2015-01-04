#define PLUG_MFR "chmv"
#define PLUG_NAME "DelysynthPlugin"

#define PLUG_CLASS_NAME DelysynthPlugin

#define BUNDLE_MFR "chmv"
#define BUNDLE_NAME "DelysynthPlugin"

#define PLUG_ENTRY DelysynthPlugin_Entry
#define PLUG_VIEW_ENTRY DelysynthPlugin_ViewEntry

#define PLUG_ENTRY_STR "DelysynthPlugin_Entry"
#define PLUG_VIEW_ENTRY_STR "DelysynthPlugin_ViewEntry"

#define VIEW_CLASS DelysynthPlugin_View
#define VIEW_CLASS_STR "DelysynthPlugin_View"

// Format        0xMAJR.MN.BG - in HEX! so version 10.1.5 would be 0x000A0105
#define PLUG_VER 0x00010000
#define VST3_VER_STR "1.0.0"

// http://service.steinberg.de/databases/plugin.nsf/plugIn?openForm
// 4 chars, single quotes. At least one capital letter
#define PLUG_UNIQUE_ID 'CHmv'
// make sure this is not the same as BUNDLE_MFR
#define PLUG_MFR_ID 'wb'

// ProTools stuff
#if (defined(AAX_API) || defined(RTAS_API)) && !defined(_PIDS_)
  #define _PIDS_
  const int PLUG_TYPE_IDS[2] = {'MSN1', 'MSN2'};
#endif
#define PLUG_MFR_PT "chmv\nchmv\nAcme\n"
#define PLUG_NAME_PT "DelysynthPlugin\nIPMS"
#define PLUG_TYPE_PT "Effect"

#if (defined(AAX_API) || defined(RTAS_API)) 
#define PLUG_CHANNEL_IO "1-1 2-2"
#else
#define PLUG_CHANNEL_IO "0-1 0-2"
#endif

#define PLUG_LATENCY 0
#define PLUG_IS_INST 1

// if this is 0 RTAS can't get tempo info
#define PLUG_DOES_MIDI 1

#define PLUG_DOES_STATE_CHUNKS 1

// Unique IDs for each image resource.
#define KNOB_ID       101
#define BG_ID         102
#define ABOUTBOX_ID   103
#define WHITE_KEY_ID  104
#define BLACK_KEY_ID  105
#define AFADER_ID      110

// Image resource locations for this plug.
#define KNOB_FN       "resources/img/knob.png"
#define BG_FN         "resources/img/bg.png"
#define ABOUTBOX_FN   "resources/img/about.png"
#define WHITE_KEY_FN  "resources/img/wk.png"
#define BLACK_KEY_FN  "resources/img/bk.png"
#define AFADER_FN     "resources/img/wk.png"

// GUI default dimensions
#define GUI_WIDTH   780
#define GUI_HEIGHT  650
// on MSVC, you must define SA_API in the resource editor preprocessor macros as well as the c++ ones
#if defined(SA_API) && !defined(OS_IOS)
#include "app_wrapper/app_resource.h"
#endif

// vst3 stuff
#define MFR_URL "www.olilarkin.co.uk"
#define MFR_EMAIL "spam@me.com"
#define EFFECT_TYPE_VST3 "Instrument|Synth"

/* "Fx|Analyzer"", "Fx|Delay", "Fx|Distortion", "Fx|Dynamics", "Fx|EQ", "Fx|Filter",
"Fx", "Fx|Instrument", "Fx|InstrumentExternal", "Fx|Spatial", "Fx|Generator",
"Fx|Mastering", "Fx|Modulation", "Fx|PitchShift", "Fx|Restoration", "Fx|Reverb",
"Fx|Surround", "Fx|Tools", "Instrument", "Instrument|Drum", "Instrument|Sampler",
"Instrument|Synth", "Instrument|Synth|Sampler", "Instrument|External", "Spatial",
"Spatial|Fx", "OnlyRT", "OnlyOfflineProcess", "Mono", "Stereo",
"Surround"
*/
