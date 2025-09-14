#include "PluginProcessor.h"
#include "PluginEditor.h"

PluginEditor::PluginEditor (PluginProcessor& p)
    : AudioProcessorEditor (&p)
    , processorRef (p)
{
    addAndMakeVisible(mModeSelector);
    mModeSelector.addItem("Bypass", (int)patch::Mode::bypass);
    mModeSelector.addItem("Send", (int)patch::Mode::transmit);
    mModeSelector.addItem("Recieve", (int)patch::Mode::recieve);
    mModeSelector.setEditableText(false);
    mModeSelector.setSelectedId(
        (int)processorRef.getEndPoint()->getMode(), 
        juce::dontSendNotification);
    mModeSelector.onChange = [this]()
    {
        processorRef.getEndPoint()->setMode(
            (patch::Mode) mModeSelector.getSelectedId()
        );
    };

    setSize (400, 300);
}

PluginEditor::~PluginEditor()
{
}


void PluginEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void PluginEditor::resized()
{
    auto area = getBounds();
    mModeSelector.setBounds(area.removeFromTop(50));
}
