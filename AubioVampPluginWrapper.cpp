#include "AubioVampPluginWrapper.h"
#include "AlgorithmTypes.h"

using std::map;

namespace Vamp {

AubioVampPluginWrapper::AubioVampPluginWrapper(int features, void *obs, JNICallback cb)
        : observer(obs)
        , callback(cb)
        , sampleRate(0)
        , channels(0)
        , stepSize(0)
        , blockSize(0)
        , cachedData(nullptr)
        , cachedDataPos(0)
        , enabledFeatures(features)
        , processStartTimestamp(-1)
        , audio_discontinuity(false)
{
    if (nullptr == observer || nullptr == callback) {
        ALOGE("Cannot create plugin %d with obs %p cb %p", AUBIO_ALGORITHM, observer, callback);
        return;
    }
    enabledFeatures |= MEL_ENERGY;
    ALOGD("Create plugin %d with obs %p cb %p", AUBIO_ALGORITHM, observer, callback);
    pluginSet.clear();
}

AubioVampPluginWrapper::~AubioVampPluginWrapper()
{
    ALOGD("~AubioVampPluginWrapper ...");
    release();
}

int AubioVampPluginWrapper::enableFeatures(int newFeatures)
{
    ALOGD("enableFeatures : enabledFeatures %d new %d", enabledFeatures, newFeatures);
    enabledFeatures |= newFeatures;

    int feature;
    for (int i = 0; i < 8; i ++) {
        feature = 1 << i;
        if (0 != (feature & newFeatures)) {
            enableFeature(feature);
        }
    }
    return 0;
}

int AubioVampPluginWrapper::setup(int sampleRate, int channels, int stepSize, int blockSize)
{
    ALOGD("setup with %d:%d/%d:%d", sampleRate, channels, stepSize, blockSize);

    this->sampleRate = sampleRate;
    this->channels = channels;
    this->stepSize = stepSize;
    this->blockSize = blockSize;

    cachedData = static_cast<float *>(malloc(sizeof(float) * stepSize * 2 * channels));

    int feature;
    for (int i = 0; i < 8; i ++) {
        feature = 1 << i;
        if (0 != (feature & enabledFeatures)) {
            enableFeature(feature);
        }
    }

    return pluginSet.size();
}

int AubioVampPluginWrapper::process(const float *dataPointer, int nb_samples, long timestamp)
{
    int nRC = 0;
    if (nullptr == dataPointer || nullptr == cachedData || 0 >= nb_samples) {
        ALOGE("process error with data %p cachedData %p samples %d", dataPointer, cachedData, nb_samples);
        return -1;
    }

    for (int i = 0; i < nb_samples; ++i) {
        cachedData[cachedDataPos] = dataPointer[i];
        cachedDataPos += 1;
        if (cachedDataPos == stepSize) {
            nRC = innerProcess(cachedData, timestamp);
            cachedDataPos = 0;
        }
    }
    return nRC;
}

void AubioVampPluginWrapper::flush()
{
    ALOGD("flush ...");
    processStartTimestamp = -1;
    audio_discontinuity = true;
    for (std::pair<const int, PluginWrapper *> wrapper : pluginSet) {
        if (nullptr != wrapper.second && nullptr != wrapper.second->descriptor
        && nullptr != wrapper.second->handle) {
            wrapper.second->descriptor->reset(wrapper.second->handle);
        }
    }
}

void AubioVampPluginWrapper::release()
{
    ALOGD("release ...");
    for (std::pair<const int, PluginWrapper *> wrapper : pluginSet) {
        if (nullptr != wrapper.second) {
            if(nullptr != wrapper.second->descriptor && nullptr != wrapper.second->handle) {
                wrapper.second->descriptor->cleanup(wrapper.second->handle);
            }
            delete wrapper.second;
            wrapper.second = nullptr;
        }
    }
    if (nullptr != cachedData) {
        free(cachedData);
        cachedData = nullptr;
    }
    cachedDataPos = 0;

    pluginSet.clear();
}

int AubioVampPluginWrapper::enableFeature(int feature)
{
    if (nullptr != pluginSet[feature]) { return 1; }
    
    PluginWrapper *wrapper = static_cast<PluginWrapper *>(malloc(
            sizeof(struct PluginWrapper)));
    wrapper->descriptor = GetPluginDescriptor(3, feature);
    if (nullptr == wrapper->descriptor) {
        ALOGE("Cannot get feature %d descriptor !!!");
        return -1;
    }
    wrapper->handle = wrapper->descriptor->instantiate(wrapper->descriptor, sampleRate);
    if (nullptr == wrapper->handle) {
        ALOGE("Cannot get feature %d handle !!!");
        return -1;
    }

    int nRC = wrapper->descriptor->initialise(wrapper->handle, channels, stepSize, blockSize);
    if (0 < nRC) {
        pluginSet[feature] = wrapper;
        ALOGD("Initialize feature %d succeed !!!", feature);
    } else {
        ALOGE("Initialize feature %d failed !!!", feature);
    }
    return nRC;
}

int AubioVampPluginWrapper::innerProcess(const float *dataPointer, long timestamp)
{
    if (0 > processStartTimestamp) {
        processStartTimestamp = timestamp;
        if (0 < timestamp) {
            ALOGD("%d plugin got new timestamp %ld", AUBIO_ALGORITHM, processStartTimestamp);
        }
    }

    int nRC = 0;
    int second = timestamp / 1000, nSec = (timestamp % 1000) * 1000;
    float energies = getEnergies(dataPointer, timestamp);

    for (std::pair<const int, PluginWrapper *> wrapper : pluginSet) {
        if (MEL_ENERGY != wrapper.first && nullptr != wrapper.second) {
            const VampPluginDescriptor * descriptor = wrapper.second->descriptor;
            VampPluginHandle handle = wrapper.second->handle;

            if (nullptr != descriptor && nullptr != handle) {
                VampFeatureList *featureList = descriptor->process(handle,
                                                                   &dataPointer,
                                                                   second, nSec);
                if (nullptr == featureList) {
                    continue;
                }

                int sec = 0, nSec = 0, dSec = 0, dNSec = 0;
                int featureCount = featureList->featureCount, availableFeatures = 0;
                if (0 <  featureCount && nullptr != observer && nullptr != callback) {
                    int64_t featureV1[featureCount + 1], featureV2[featureCount + 1];
                    for (int i = 0; i < featureCount; ++i) {
                        if (featureList->features[i].v1.hasTimestamp || featureList->features[i].v2.hasDuration) {
                            sec = featureList->features[i].v1.sec;
                            nSec = featureList->features[i].v1.nsec;
                            dSec = featureList->features[i].v2.durationSec;
                            dNSec = featureList->features[i].v2.durationNsec;

                            featureV1[availableFeatures] = sec * 1000 + nSec / 1000000 + processStartTimestamp;
                            featureV2[availableFeatures] = dSec * 1000 + dNSec / 1000000 + processStartTimestamp;

                            availableFeatures ++;
                        }
                        if (0 < energies) {
                            featureV1[availableFeatures] = featureV2[availableFeatures] = (int64_t)(energies);
                            availableFeatures ++;
                        }

                        if (0 < availableFeatures) {
//                            nRC = availableFeatures;
//                            ALOGD("Process got features %d : %d", wrapper.first, availableFeatures);
                            long flags = audio_discontinuity ? AUDIO_DISCONTINUITY : 0;
                            audio_discontinuity = false;
                            nRC = callback(observer, wrapper.first, -11, featureV1, featureV2, availableFeatures, flags);
                        }
                    }
                }
            } else {
                ALOGE("Process feature %d error with descriptor %p handler %p ",
                      wrapper.first, descriptor, handle);
            }
        } else if (MEL_ENERGY != wrapper.first) {
            ALOGE("Process feature %d error without wrapper !!!", wrapper.first);
        }
    }
    return nRC;
}

float AubioVampPluginWrapper::getEnergies(const float *dataPointer, long timestamp)
{
    int second = timestamp / 1000, nSec = (timestamp % 1000) * 1000;
    float energies = 0;

    for (std::pair<const int, PluginWrapper *> wrapper : pluginSet) {
        if (MEL_ENERGY == wrapper.first && nullptr != wrapper.second) {
            const VampPluginDescriptor * descriptor = wrapper.second->descriptor;
            VampPluginHandle handle = wrapper.second->handle;

            if (nullptr != descriptor && nullptr != handle) {
                VampFeatureList *featureList = descriptor->process(handle,
                                                                   &dataPointer,
                                                                   second, nSec);
                if (nullptr == featureList) {
                    continue;
                }

                int featureCount = featureList->featureCount, availableFeatures = 0;
                int energyCount = 0;
                for (int i = 0; i < featureCount; ++i) {
                    int values = featureList->features[i].v1.valueCount;
                    for (int j = 0; j < values; ++j) {
                        energies += 1000 * featureList->features[i].v1.values[j];
                    }
                    energyCount += values;
                }
                energies = 0 < energyCount ? energies / energyCount : 0;
            } else {
                ALOGE("Process feature %d error with descriptor %p handler %p ",
                      wrapper.first, descriptor, handle);
            }
        }
    }
    return energies;
}

}