/*  This class should do all routing for all instances
*/

#pragma once

#include <vector>
#include <tuple>
#include <juce_audio_basics/juce_audio_basics.h>

#include "Singleton.h"
#include "Instance.h"

namespace patch
{
    struct InstanceReg
    {
        int id;
        std::unique_ptr<Instance> ptr;
        bool hasRanProcess;
    };

    class Core : public Singleton<Core>
    {
    public:
        Instance* makeNewInstance();
        void tryDeleteInstance(Instance* ptr);

        void prepareToPlay(double sampleRate, int samplesPerBlock);
        bool processRouting();
        void releaseResources();

    private:
        std::vector<InstanceReg> mInstances;

        // im not sure just yet how this is going to work, but this buffer is a
        // temporary for sure
        juce::AudioBuffer<float> mSend;
    };


}