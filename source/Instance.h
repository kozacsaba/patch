/*  This a class that should be present in every single PluginProcessor. This is
    a client-like object that provides an interface for the core and allows
    routing
*/

#pragma once

#include "Core.h"

namespace patch
{
    // this too should probably be reworked later
    enum class Mode
    {
        bypass,
        send,
        recieve
    };

    class Instance
    {
    public:
        Instance();
        ~Instance();

        void prepareToPlay(double sampleRate, int samplesPerBlock);
        void processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi);
        void releaseResources();

        void setMode(Mode mode);
        Mode getMode() { return mMode; }

    private:
        Mode mMode;
    };

}