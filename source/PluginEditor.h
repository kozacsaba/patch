#pragma once

#include "PluginProcessor.h"
#include "Core.h"

class InstanceListModel : public juce::ListBoxModel
{
public:
    InstanceListModel(juce::Uuid instanceId);

    int getNumRows() override;
    void paintListBoxItem (int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
    void listBoxItemClicked (int row, const juce::MouseEvent& event) override;

    void setMode(patch::Mode instanceMode);
    void setInstanceList(patch::Map<patch::Instance*>* instanceListPtr);

    std::function<void(juce::Uuid)> onInstanceSelected;

private:
    juce::Uuid id;
    patch::Mode mode;
    patch::Map<patch::Instance*>* mInstanceList;
};

class PluginEditor final : public juce::AudioProcessorEditor
{
public:
    explicit PluginEditor (PluginProcessor&);
    ~PluginEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    
    void modeSwitched();
    void attachToParameters(juce::Uuid otherInstanceId);

private:
    PluginProcessor& processorRef;

    InstanceListModel mConnectionListBoxModel;

    juce::ComboBox cModeSelectorComboBox;
    juce::Label cNameLabel;
    juce::ListBox cConnectionListBox;
    juce::Slider cGainSlider;

    ConnectionParameters* mConnectionParameters;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
