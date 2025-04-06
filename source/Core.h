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
    // there are way better methods to do this, but atm im just trying to get
    // it to work somehow
    struct MCCBuffer
    // Multi Channel Circular Buffer
    {
    public:
        inline void setSize(int numberOfChannels, int numberOfSamples)
        {
            mNumberOfChannels = numberOfChannels;
            mNumberOfSamples = numberOfSamples;

            buffers.reserve(numberOfChannels);
            for(int i = 0; i < numberOfChannels; i++)
            {
                buffers[i] = std::make_unique<CircularArray<float>>(numberOfSamples);
                buffers[i]->reset();
            }
        }

        inline CircularArray<float>* getChannel(int channelNumber) const
        {
            if (channelNumber >= mNumberOfChannels) return nullptr;
            if (channelNumber < 0) return nullptr;

            return buffers[channelNumber].get();
        }

        inline int getNumberOfChannels() const { return mNumberOfChannels; }
        inline int getNumberOfSamples() const { return mNumberOfSamples; }

    private:
        std::vector<std::unique_ptr<CircularArray<float>>> buffers;
        int mNumberOfChannels;
        int mNumberOfSamples;
    };

    class Core : public Singleton<Core>
    {
    public:
        void registerInstance(Instance* ptr);
        void tryDeleteInstance(Instance* ptr);

        void prepareToPlay(double sampleRate, int samplesPerBlock);
        void processRouting(int incomingSize);
        void releaseResources();

        void bufferForNextBlock(juce::AudioBuffer<float>& buffer);

    private:
        std::vector<Instance*> mInstances;

        int mMaxBufferSize;
        double mSampleRate;
        int mTransitLength = 0;

        // im not sure just yet how this is going to work, but this buffer is a
        // temporary for sure
        juce::AudioBuffer<float> mTransitBuffer;
        MCCBuffer mDelayBuffer;
    };

}
