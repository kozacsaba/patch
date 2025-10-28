/*  This a class that should be present in every single PluginProcessor. This is
    a client-like object that provides an interface for the core and allows
    routing
*/

#pragma once

#include "juce_audio_basics/juce_audio_basics.h"

#include "CircularArray.h"
#include "ConnectionParameters.h"

namespace patch
{
    // this too should probably be reworked later
    enum class Mode : int
    {
        bypass = 1,
        transmit,
        recieve
    };

    enum BinarySateFlag : bool
    {
        hasFinished = true,
        hasNotFinished = false
    };

    class Core;

    // This is a trick to provide access to the setId funciton for Core.
    // No other class should have access to that funciton and it should be called only rarely, in
    // special cases. I am keeping it private and only letting it be called with a token that only
    // Core has access to.
    struct InstanceAccessToken
    {
        private:
        InstanceAccessToken() = default;
        friend class Core;
    };

    class Instance
    {
    public:
        Instance();
        ~Instance();

        void prepareToPlay(double sampleRate, int samplesPerBlock);
        void processBlock (juce::AudioBuffer<float>& buffer);
        void releaseResources();

        void setMode(Mode mode);
        void setCoreFinished() { fCoreState = hasFinished; }
        void setId(InstanceAccessToken token, const juce::Uuid& uuid);
        void setName(juce::String name) {mName = name;}

        Mode getMode() { return mMode; }
        juce::AudioBuffer<float>* getRecieveBuffer() { return &mRecieveBuffer; }
        const juce::Uuid& getId() const { return id; }
        juce::String getName() const;

        std::function<void()> updateConnectionList;

    private:
        int maxBufferSize;
        double fs;
        BinarySateFlag fCoreState = hasNotFinished;

        Mode mMode;
        Mode mPreviousMode;

        juce::AudioBuffer<float> mRecieveBuffer;
        Core* mCorePtr;

        juce::Uuid id;
        std::optional<juce::String> mName;

        inline static juce::CriticalSection mcs = {};
    };

}
