/*  This class should do all routing for all instances
*/

#pragma once

#include <vector>
#include <tuple>
#include <juce_audio_basics/juce_audio_basics.h>

#include "CircularArray.h"
#include "Singleton.h"
#include "Instance.h"
#include "ConnectionParameters.h"

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

            buffers.clear();
            buffers.reserve((size_t)numberOfChannels);
            for(size_t i = 0; i < (size_t)numberOfChannels; i++)
            {
                auto channel = std::make_unique<CircularArray<float>>(numberOfSamples);
                buffers.push_back(std::move(channel));
                buffers.back()->reset();
            }
        }

        inline CircularArray<float>* getChannel(int channelNumber) const
        {
            if (channelNumber >= mNumberOfChannels) return nullptr;
            if (channelNumber < 0) return nullptr;

            return buffers[(size_t)channelNumber].get();
        }

        inline int getNumberOfChannels() const { return mNumberOfChannels; }
        inline int getNumberOfSamples() const { return mNumberOfSamples; }

    private:
        std::vector<std::unique_ptr<CircularArray<float>>> buffers;
        int mNumberOfChannels;
        int mNumberOfSamples;
    };

    template<class T> using Map = std::unordered_map<juce::Uuid, T>;
    using ParameterVector = Map<std::unique_ptr<ConnectionParameters>>;
    using ParameterMatrix = Map<ParameterVector>;

    class Core : public Singleton<Core>
    {
    public:
        void registerInstance(Instance* ptr);
        void tryDeleteInstance(juce::Uuid id);

        void prepareToPlay(double sampleRate, int samplesPerBlock);
        void processRouting(int incomingSize);
        void releaseResources();
        void instanceSwitchedMode(Instance* ptr, Mode previousMode);

        void bufferForNextBlock(juce::Uuid id, juce::AudioBuffer<float>& buffer);

        Map<Instance*>* getRecievers() {return &mRecieverInstances;}
        Map<Instance*>* getTransmitters() {return &mTransmitterInstances;}
        ConnectionParameters* getConnectionParameters(juce::Uuid transmitter, juce::Uuid reciever);
        Instance* findInstanceById(juce::Uuid id);

    private:
        bool checkForUuidMatch(const juce::Uuid& id);

        Map<Instance*> mBypassedInstances;
        Map<Instance*> mRecieverInstances;
        Map<Instance*> mTransmitterInstances;

        int mMaxBufferSize;
        double mSampleRate;
        int mTransitLength = 0;

        // first is DelayBuffer, second is TransmitBuffer
        using Buffer = std::pair<MCCBuffer, juce::AudioBuffer<float>>;
        // Belongs to Transmitter Instances
        Map<Buffer> mBuffers;

        // Map<juce::AudioBuffer<float>> mTransitBuffers;
        // Map<MCCBuffer> mDelayBuffers;

        // Matrix[Transmitter][Reciever]
        ParameterMatrix mMatrix;

        juce::CriticalSection mBufferOperation;
    };

}
