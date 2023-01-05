//
// Created by Arthur on 2022/11/3.
//

#ifndef __AUBIO_PLUGIN_H__
#define __AUBIO_PLUGIN_H__

#include <vamp/vamp.h>
#include <vamp-sdk/PluginAdapter.h>
#include "Features.h"

#include "plugin/MelEnergy.h"
#include "plugin/Mfcc.h"
#include "plugin/Notes.h"
#include "plugin/Onset.h"
#include "plugin/Pitch.h"
#include "plugin/Silence.h"
#include "plugin/SpecDesc.h"
#include "plugin/Tempo.h"
#include "plugin/Types.h"

namespace Vamp {

static PluginAdapter<Onset> onsetAdapter;
static PluginAdapter<Pitch> pitchAdapter;
static PluginAdapter<Notes> notesAdapter;
static PluginAdapter<Tempo> tempoAdapter;
static PluginAdapter<Silence> silenceAdapter;
static PluginAdapter<Mfcc> mfccAdapter;
static PluginAdapter<MelEnergy> melEnergyAdapter;
static PluginAdapter<SpecDesc> specdescAdapter;

const VampPluginDescriptor *GetPluginDescriptor(unsigned int vampApiVersion,
                                                    unsigned int index);
}
#endif //__AUBIO_PLUGIN_H__
