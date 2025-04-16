/*  This a class that should be present in every single PluginProcessor. This is
    a client-like object that provides an interface for the core and allows
    routing
*/

#pragma once

#include "Core.h"
#include "CircularArray.h"

namespace patch
{
    // this too should probably be reworked later
    enum class Mode : int
    {
        bypass = 1,
        send,
        recieve
    };

    enum BinarySateFlag : bool
    {
        hasFinished = true,
        hasNotFinished = false
    };

    class Core;

    class Instance
    {
    public:
        Instance();
        ~Instance();

        void prepareToPlay(double sampleRate, int samplesPerBlock);
        void processBlock (juce::AudioBuffer<float>& buffer);
        void releaseResources();

        void setMode(Mode mode);
        Mode getMode() { return mMode; }
        juce::AudioBuffer<float>* getRecieveBuffer() { return &mRecieveBuffer; }
        void coreFinished() { fCoreState = hasFinished; }
        int getId() const { return id; }

    private:
        int maxBufferSize;
        double fs;
        BinarySateFlag fCoreState = hasNotFinished;

        Mode mMode;
        juce::AudioBuffer<float> mRecieveBuffer;
        Core* mCorePtr;

        inline static int gCounter = 1;
        const int id;
    };

}