//
// Created by Arthur on 2022/11/3.
//

#include "AubioPlugin.h"

namespace Vamp {

const VampPluginDescriptor *GetPluginDescriptor(unsigned int vampApiVersion,
                                                    unsigned int index)
{
    if (vampApiVersion < 2) return 0;

    switch (index) {
    case  ONSET: return onsetAdapter.getDescriptor();
    case  PITCH: return pitchAdapter.getDescriptor();
    case  NOTES: return notesAdapter.getDescriptor();
    case  TEMPO: return tempoAdapter.getDescriptor();
    case  SILENCE: return silenceAdapter.getDescriptor();
    case  MFCC: return mfccAdapter.getDescriptor();
    case  MEL_ENERGY: return melEnergyAdapter.getDescriptor();
    case  SPEC_DESC: return specdescAdapter.getDescriptor();
    default: return 0;
    }
}
}