#ifndef __AUBIO_VAMP_PLUGIN_H__
#define __AUBIO_VAMP_PLUGIN_H__

#include <vamp/vamp.h>

#include "AubioPlugin.h"
#include "Callback.h"

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "AubioVampPluginWrapper"
#endif
#include "NIOLog.h"

using std::map;

namespace Vamp {

class AubioVampPluginWrapper {
public:
    AubioVampPluginWrapper(int features, void *obs, JNICallback cb);
    ~AubioVampPluginWrapper();

    int enableFeatures(int features);
    int setup(int sampleRate, int channels, int stepSize, int blockSize);
    int process(const float *dataPointer, int nb_samples, long timestamp);
    void flush();
    void release();

private:
    int enableFeature(int feature);
    int innerProcess(const float *dataPointer, long timestamp);
    float getEnergies(const float *dataPointer, long timestamp);

private:
    void *observer;
    JNICallback callback;

    int sampleRate, channels, stepSize, blockSize;
    int enabledFeatures;
    int64_t processStartTimestamp;
    bool audio_discontinuity;

    float *cachedData;
    int cachedDataPos;

    typedef struct PluginWrapper {
        const VampPluginDescriptor * descriptor;
        VampPluginHandle handle;
    } PluginWrapper;
    std::map<int, PluginWrapper*> pluginSet;
};
}
#endif // __AUBIO_VAMP_PLUGIN_H__
