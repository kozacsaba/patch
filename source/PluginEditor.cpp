#include "PluginEditor.h"

using namespace patch;

InstanceListModel::InstanceListModel(juce::Uuid instanceId)
    : id(instanceId)
{}

int InstanceListModel::getNumRows()
{
    if(mInstanceList)
        return (int)mInstanceList->size();
    else
        return 0;
}

void InstanceListModel::paintListBoxItem 
(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected)
{
    if(rowNumber < 0 || rowNumber > getNumRows()) return;

    auto iter = mInstanceList->begin();
    std::advance(iter, rowNumber);
    const juce::String elementName = iter->second->getName();

    ConnectionParameters* parameters = nullptr;
    Core* core = Core::getInstance();
    switch (mode)
    {
        case Mode::transmit :
            parameters = core->getConnectionParameters(id, iter->first);
            break;
        case Mode::recieve :
            parameters = core->getConnectionParameters(iter->first, id);
            break;
        case Mode::bypass :
        default :
            return;
    }

    const bool isConnected = parameters->gain > 0.f;

    juce::Colour backgroundColor = isConnected 
        ? juce::Colours::lightgreen
        : juce::Colours::lightblue;

    if(rowIsSelected)
        backgroundColor = backgroundColor.brighter();

    g.setColour(backgroundColor);
    g.fillAll();

    g.setColour(juce::Colours::black);
    g.drawText(elementName, 0, 0, width, height, juce::Justification::centredLeft);
}

void InstanceListModel::listBoxItemClicked (int row, const juce::MouseEvent& event)
{
    juce::ignoreUnused(event);
    if(row < 0 || row > getNumRows()) return;

    if(onInstanceSelected)
    {
        auto iter = mInstanceList->begin();
        std::advance(iter, row);
        onInstanceSelected(iter->first);
    }
}


void InstanceListModel::setMode(patch::Mode instanceMode)
{
    mode = instanceMode;
}

void InstanceListModel::setInstanceList(patch::Map<patch::Instance*>* instanceListPtr)
{
    mInstanceList = instanceListPtr;
}

//==================================================================================================

PluginEditor::PluginEditor (PluginProcessor& p)
    : AudioProcessorEditor (&p)
    , processorRef (p)
    , mConnectionListBoxModel(processorRef.getEndPoint()->getId())
{
    addAndMakeVisible(cModeSelectorComboBox);
    cModeSelectorComboBox.addItem("Bypass", (int)Mode::bypass);
    cModeSelectorComboBox.addItem("Tranmit", (int)Mode::transmit);
    cModeSelectorComboBox.addItem("Recieve", (int)Mode::recieve);
    cModeSelectorComboBox.setEditableText(false);
    cModeSelectorComboBox.setSelectedId(
        (int)processorRef.getEndPoint()->getMode(), 
        juce::dontSendNotification);
    cModeSelectorComboBox.onChange = [this]()
    {
        this->modeSwitched();
    };

    addAndMakeVisible(cNameLabel);
    cNameLabel.setText(processorRef.getEndPoint()->getName(), juce::dontSendNotification);
    cNameLabel.onTextChange = [this]
    {
        processorRef.getEndPoint()->setName(cNameLabel.getText());
    };

    mConnectionListBoxModel.onInstanceSelected = [this](juce::Uuid otherInstanceId)
    {
        attachToParameters(otherInstanceId);
    };
    modeSwitched();

    addAndMakeVisible(cConnectionListBox);
    cConnectionListBox.setModel(&mConnectionListBoxModel);
    cConnectionListBox.setMultipleSelectionEnabled(false);

    addAndMakeVisible(cGainSlider);
    cGainSlider.setMinAndMaxValues(0.f, 1.f, juce::dontSendNotification);
    cGainSlider.onValueChange = [this]
    {
        if(mConnectionParameters)
            mConnectionParameters->gain = (float)cGainSlider.getValue();
    };

    setSize (400, 300);
    setResizable(true, true);
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
    auto topArea = area.removeFromTop(area.proportionOfHeight(0.2f));
    auto parameterArea = area.removeFromBottom(area.proportionOfHeight(0.2f));

    cNameLabel.setBounds(topArea.removeFromLeft(area.proportionOfHeight(0.5f)));
    cModeSelectorComboBox.setBounds(topArea);

    cConnectionListBox.setBounds(area);

    cGainSlider.setBounds(parameterArea);
}

void PluginEditor::modeSwitched()
{
    auto mode = (Mode) cModeSelectorComboBox.getSelectedId(); 
    Map<Instance*>* instanceList;
    auto* core = Core::getInstance();

    switch(mode)
    {
        case Mode::recieve :
            instanceList = core->getRecievers();
            break;
        case Mode::transmit :
            instanceList = core->getTransmitters();
            break;
        case Mode::bypass :
        default:
            instanceList = nullptr;
    }

    processorRef.getEndPoint()->setMode(mode);

    mConnectionListBoxModel.setInstanceList(instanceList);
    cConnectionListBox.updateContent();
    cConnectionListBox.deselectAllRows();

    attachToParameters(juce::Uuid::null());
}

void PluginEditor::attachToParameters(juce::Uuid otherInstanceId)
{
    if(otherInstanceId.isNull())
    {
        mConnectionParameters = nullptr;
        return;
    }

    auto* instance = processorRef.getEndPoint();
    Mode mode = instance->getMode();
    juce::Uuid id = instance->getId();

    switch (mode)
    {
    case Mode::transmit :
        mConnectionParameters = Core::getInstance()
            ->getConnectionParameters(id, otherInstanceId);
        break;
    case Mode::recieve :
        mConnectionParameters = Core::getInstance()
            ->getConnectionParameters(otherInstanceId, id);
        break;
    case Mode::bypass :
    default:
        mConnectionParameters = nullptr;
    }
}
