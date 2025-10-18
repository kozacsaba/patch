#pragma once
#include "juce_gui_basics/juce_gui_basics.h"
#include <vector>
#include <type_traits>
#include <atomic>

namespace patch
{

enum class OverdriveProtection
{
    off,
    clip
};

// PatchToggleButton

class PatchToggleButton
{
public:
    PatchToggleButton()
    {
        button.onStateChange = [this]()
        {
            for(auto listener : listeners)
            {
                listener->toggleStateChanged(this);
            }
        };
    }

    ~PatchToggleButton()
    {
        button.onStateChange = nullptr;
        removeAllListeners();
    }

    bool getState() { return button.getToggleState(); }

    void setState(bool state, juce::NotificationType sendNotifications) 
    { 
        button.setToggleState(state, sendNotifications); 
    }

    juce::ToggleButton button;

    class Listener
    {
    public:
        virtual void toggleStateChanged(PatchToggleButton* emitter) = 0;
    };

    void addListener(Listener* listenerToAdd)
    {
        const auto it = std::find(listeners.begin(), listeners.end(), listenerToAdd);
        if(it != listeners.end()) return; // already added 
        listeners.push_back(listenerToAdd);
    }

    void removeListener(Listener* listenerToRemove)
    {
        const auto it = std::find(listeners.begin(), listeners.end(), listenerToRemove);
        if(it == listeners.end()) return; // listener not found
        listeners.erase(it);
    }

private:
    void removeAllListeners()
    {
        listeners.clear();
    }

    std::vector<Listener*> listeners;
};

// ToggleParameter

class ToggleParameter : public PatchToggleButton::Listener
{
public:
    ToggleParameter()
        : value(false)
    {}

    ToggleParameter(bool initVal)
        : value(initVal)
    {}

    ~ToggleParameter()
    {
        detachAll();
    }

    void attachToggleButton(PatchToggleButton* buttonToAttach)
    {
        buttonToAttach->addListener(this);
        buttons.push_back(buttonToAttach);
        buttonToAttach->setState(value.load(std::memory_order_relaxed), juce::dontSendNotification);
    }

    void detachToggleButton(PatchToggleButton* buttonToDetach)
    {
        buttonToDetach->removeListener(this);
        auto it = std::find(buttons.begin(), buttons.end(), buttonToDetach);
        if (it == buttons.end())
            return;
        buttons.erase(it);
    }

    void toggleStateChanged(PatchToggleButton* emitter)
    {
        bool valueToSet = emitter->getState();
        value.store(valueToSet);
        for(auto button : buttons)
        {
            button->setState(valueToSet, juce::dontSendNotification);
        }
    }

    bool getValue() const { return value.load(std::memory_order_relaxed); }

    void setValue(bool val) 
    {
        value = val;
        for(auto button : buttons)
        {
            button->setState(val, juce::dontSendNotification);
        }
    }

private:
    void detachAll()
    {
        buttons.clear();
    }

    std::vector<PatchToggleButton*> buttons;
    std::atomic_bool value;
};

// SliderParameter

template<typename type = float>
    requires std::is_floating_point_v<type>
class SliderParameter : public juce::Slider::Listener
{
public:
    SliderParameter()
        : value((type)0)
    {}

    SliderParameter(type init)
        : value(init)
    {}

    ~SliderParameter() override
    {
        detachAll();
    }

    void attachSlider(juce::Slider* sliderToAttach)
    {
        sliderToAttach->addListener(this);
        sliders.push_back(sliderToAttach);
        sliderToAttach->setValue((double)value.load(std::memory_order_relaxed), juce::dontSendNotification);
    }

    void detachSlider(juce::Slider* sliderToDetach)
    {
        sliderToDetach->removeListener(this);
        auto it = std::find(sliders.begin(), sliders.end(), sliderToDetach);
        if (it == sliders.end())
            return;
        sliders.erase(it);
    }

    void sliderValueChanged(juce::Slider* emitter) override
    {
        type valueToSet = (type)emitter->getValue();
        value.store(valueToSet);
        for(auto slider : sliders)
        {
            slider->setValue((double)valueToSet, juce::dontSendNotification);
        }
    }

    type getValue() const { return value.load(std::memory_order_relaxed); }

    void setValue(type val)
    {
        value.store(val);
        for(auto slider : sliders)
        {
            slider->setValue((double)val, juce::dontSendNotification);
        }
    }

private:
    void detachAll()
    {
        for(auto slider : sliders)
        {
            slider->removeListener(this);
        }
        sliders.clear();
    }

    std::vector<juce::Slider*> sliders;
    std::atomic<type> value;
};

// ComboBoxParameter

template<typename Enum>
    requires std::is_enum_v<Enum>
class ComboBoxParameter : public juce::ComboBox::Listener
{
public:
    ComboBoxParameter()
        : value(Enum(0))
    {}

    ComboBoxParameter(Enum init)
        : value(init)
    {}

    ~ComboBoxParameter() override
    {
        detachAll();
    }

    void attachComboBoxParameter(juce::ComboBox* comboBoxToAttach)
    {
        comboBoxes.push_back(comboBoxToAttach);
        comboBoxToAttach->addListener(this);
        comboBoxToAttach->setSelectedId((int)value.load(std::memory_order_relaxed), juce::dontSendNotification);
    }

    void detachComboBox(juce::ComboBox* comboBoxToDetach)
    {
        comboBoxToDetach->removeListener(this);
        auto it = std::find(comboBoxes.begin(), comboBoxes.end(), comboBoxToDetach);
        if (it == comboBoxes.end())
            return;
        comboBoxes.erase(it);
    }

    void comboBoxChanged (juce::ComboBox *comboBoxThatHasChanged) override
    {
        Enum valueToSet = (Enum)comboBoxThatHasChanged->getSelectedId();
        value.store(valueToSet);
        for(auto comboBox : comboBoxes)
        {
            comboBox->setSelectedId((int)valueToSet);
        }
    }

    Enum getValue() const { return value.load(std::memory_order_relaxed); }

    void setValue(Enum val)
    {
        value.store(val);
        for(auto comboBox : comboBoxes)
        {
            comboBox->setSelectedId((int)val, juce::dontSendNotification);
        }
    }

private:
    void detachAll()
    {
        for(auto comboBox : comboBoxes)
        {
            comboBox->onChange = nullptr;
        }
        comboBoxes.clear();
    }

    std::vector<juce::ComboBox*> comboBoxes;
    std::atomic<Enum> value;
};

/*  ConnectionParameters
    This struct shoule be used to describe the conneciton between a Transmitter
    instance and a Reciever instance.
*/
struct ConnectionParameters
{
    ToggleParameter on = false;
    SliderParameter<float> gain = 0.f;

    // these are not implemented just yet

    int delay = 0;
    ToggleParameter delayCorrection = false;
    ComboBoxParameter<OverdriveProtection> protection = OverdriveProtection::off;
};

} // namespace patch
