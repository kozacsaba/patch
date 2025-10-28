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

    const bool isConnected = parameters 
        ? parameters->on.getValue()
        : false;

    juce::Colour backgroundColor = isConnected 
        ? juce::Colours::lightgreen
        : juce::Colours::lightblue;

    if(rowIsSelected)
        backgroundColor = backgroundColor.brighter();

    g.setColour(backgroundColor);
    g.fillAll();

    auto textArea = juce::Rectangle<int>{0, 0, width, height}.reduced(8, 0);
    g.setColour(juce::Colours::black);
    g.drawFittedText(elementName, textArea, juce::Justification::centredLeft, 1);
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
    , mConnectionParameters(nullptr)
{
    addAndMakeVisible(cModeSelectorComboBox);
    cModeSelectorComboBox.addItem("Bypass", (int)Mode::bypass);
    cModeSelectorComboBox.addItem("Transmit", (int)Mode::transmit);
    cModeSelectorComboBox.addItem("Recieve", (int)Mode::recieve);
    cModeSelectorComboBox.setEditableText(false);
    cModeSelectorComboBox.setSelectedId(
        (int)processorRef.getEndPoint()->getMode(), 
        juce::dontSendNotification);
    cModeSelectorComboBox.onChange = [this]()
    {
        this->modeSwitched();
    };
    modeSwitched();

    addAndMakeVisible(cNameLabel);
    cNameLabel.setText(processorRef.getEndPoint()->getName(), juce::dontSendNotification);
    cNameLabel.setEditable(true);
    cNameLabel.onTextChange = [this]
    {
        processorRef.getEndPoint()->setName(cNameLabel.getText());
        Mode mode = processorRef.getEndPoint()->getMode();
        if(mode == Mode::transmit)
            Core::getInstance()->updateConnectionList(Mode::recieve);
        if(mode == Mode::recieve)
            Core::getInstance()->updateConnectionList(Mode::transmit);
    };

    mConnectionListBoxModel.onInstanceSelected = [this](juce::Uuid otherInstanceId)
    {
        attachToParameters(otherInstanceId);
    };

    addAndMakeVisible(cConnectionListBox);
    cConnectionListBox.setModel(&mConnectionListBoxModel);
    cConnectionListBox.setMultipleSelectionEnabled(false);
    cConnectionListBox.setRowHeight(30);

    addAndMakeVisible(cGainSlider);
    cGainSlider.setRange(0.f, 1.f);
    cGainSlider.setNumDecimalPlacesToDisplay(2);

    addAndMakeVisible(cConnectionButton.button);
    cConnectionButton.addCallback([this]()
    {
        updateInstanceList();
        Mode mode = processorRef.getEndPoint()->getMode();
        if(mode == Mode::transmit)
            Core::getInstance()->updateConnectionList(Mode::recieve);
        if(mode == Mode::recieve)
            Core::getInstance()->updateConnectionList(Mode::transmit);
        processorRef.getEndPoint()->getRecieveBuffer()->clear();
    });
    cConnectionButton.addCallback([this]()
    {
        const bool on = cConnectionButton.getState();
        cGainSlider.setEnabled(on);
    }
    );

    processorRef.getEndPoint()->updateConnectionList = [this]()
    {
        updateInstanceList();
    };

    setSize (400, 300);
    setResizable(true, true);
}

PluginEditor::~PluginEditor()
{
    attachToParameters(juce::Uuid::null());
    processorRef.getEndPoint()->updateConnectionList = nullptr;
}


void PluginEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void PluginEditor::resized()
{
    auto area = getBounds();
    auto topArea = area.removeFromTop(area.proportionOfHeight(0.07f));
    auto parameterArea = area.removeFromBottom(area.proportionOfHeight(0.1f));

    cNameLabel.setBounds(topArea.removeFromLeft(area.proportionOfWidth(0.5f)));
    cModeSelectorComboBox.setBounds(topArea);

    cConnectionListBox.setBounds(area);

    cConnectionButton.button.setBounds(parameterArea.removeFromLeft(30));
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
            instanceList = core->getTransmitters();
            break;
        case Mode::transmit :
            instanceList = core->getRecievers();
            break;
        case Mode::bypass :
        default:
            instanceList = nullptr;
    }

    processorRef.getEndPoint()->setMode(mode);

    mConnectionListBoxModel.setMode(mode);
    mConnectionListBoxModel.setInstanceList(instanceList);
    cConnectionListBox.updateContent();
    cConnectionListBox.deselectAllRows();
    cConnectionListBox.repaint();

    attachToParameters(juce::Uuid::null());

    cGainSlider.setEnabled(false);
}

void PluginEditor::attachToParameters(juce::Uuid otherInstanceId)
{
    cGainSlider.setEnabled(false);

    if(mConnectionParameters)
    {
        mConnectionParameters->on.detachToggleButton(&cConnectionButton);
        mConnectionParameters->gain.detachSlider(&cGainSlider);

        // delay
        // delayCorrection
        // protection
    }

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
        cGainSlider.setEnabled(true);
        mConnectionParameters = Core::getInstance()
            ->getConnectionParameters(id, otherInstanceId);
        break;
    case Mode::recieve :
        cGainSlider.setEnabled(true);
        mConnectionParameters = Core::getInstance()
            ->getConnectionParameters(otherInstanceId, id);
        break;
    case Mode::bypass :
    default:
        mConnectionParameters = nullptr;
    }

    if(mConnectionParameters)
    {
        mConnectionParameters->on.attachToggleButton(&cConnectionButton);
        mConnectionParameters->gain.attachSlider(&cGainSlider);

        // delay
        // delayCompensation
        // protection
    }
}

void PluginEditor::updateInstanceList()
{
    cConnectionListBox.updateContent();
    cConnectionListBox.repaint();
}
