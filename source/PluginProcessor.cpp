#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Logger.h"

PluginProcessor::PluginProcessor()
    : AudioProcessor (BusesProperties()
                      .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                      )
    , mEndpoint(std::make_unique<patch::Instance>())
{
#if LOG_LEVEL > 2
    auto* logger = patch::FileLogger::getInstance();
    juce::ignoreUnused(logger);
#endif
}
PluginProcessor::~PluginProcessor()
{
}


void PluginProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    mEndpoint->prepareToPlay(sampleRate, samplesPerBlock);
}
void PluginProcessor::releaseResources()
{
    mEndpoint->releaseResources();
}
bool PluginProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    // this version only supports 2 channel stereo
    if (//layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
        layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    return true;
}
void PluginProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);

    juce::ScopedNoDenormals noDenormals;

    mEndpoint->processBlock(buffer);
}


bool PluginProcessor::hasEditor() const { return true; }
juce::AudioProcessorEditor* PluginProcessor::createEditor()
{
    return new PluginEditor (*this);
}


// I will have to figure out how presets would make sense
void PluginProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::ignoreUnused (destData);
}
void PluginProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    juce::ignoreUnused (data, sizeInBytes);
}


juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PluginProcessor();
}
