/*
  ==============================================================================

    MidiLearnInterface.h
    Created: 26 Dec 2024 12:04:52pm
    Author:  PJP

  ==============================================================================
*/

#define NUMBERFILTER "0123456789"

#pragma once
#include <JuceHeader.h>
#include "../PluginProcessor.h"

//==============================================================================
// base class for showing the zone. Red if active, black when not active
//==============================================================================
class MidiLearnInterface :
public juce::ComboBox::Listener,
public juce::Button::Listener,
public juce::Slider::Listener
{
public:
    MidiLearnInterface(juce::Component* p)
    {
        parent = p;
    }

    void CreateGui();
    void AddMidiInterfaceListeners();
    void RemoveMidiInterfaceListeners();
    void ResizeControls();//resized

    //==============================================================================
    // listeners
    //==============================================================================
    void comboBoxChanged(juce::ComboBox* combobox) override;
    void buttonClicked(juce::Button* button) override;
    void sliderValueChanged(juce::Slider* slider) override;

    //==============================================================================
    // public properties
    //==============================================================================
    static inline bool MidiSettingOn = false;
    static inline bool  MidiLearnOn = false;
    bool getSelected()
    {
        return selected;
    }
    void setSelected(bool value)
    {
        if(selected != value)
        {
            selected = value;
            parent->repaint();
        }
    }

    juce::ChangeBroadcaster onMidiLearnChanges;
    
    bool MidiLearnNew();


    void PaintMidiSettings(juce::Graphics& g)
    {
        ShowMidiSettings();
    }

    juce::Colour GetMidiActiveColour(juce::Colour colourNormal)
    {
        auto alpha = selected ? alphaMidiLearnOn : alphaMidiLearnOff;
        return MidiSettingOn ? (MidiLearnNew() ? ColourMidiNotSet.withAlpha(alpha) : ColourMidiSet.withAlpha(alpha)) : colourNormal;
    }

    void ShowMidiSettings();
    bool MidiLearnMessage(juce::MidiMessage message);

protected:
    juce::TextButton selectButton;
    juce::ComboBox cmbMidiInMessage;
    juce::ComboBox cmbMidiInChannel;
    juce::ComboBox cmbMidiInNumber;
    juce::Slider sldMidiInMaxValue;
    juce::Slider sldMidiInMinValue;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> cmbMidiMessageAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> cmbMidiInChannelAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> cmbMidiInNumberAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sldMidiInMinValueAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sldMidiInMaxValueAttachment;

private:
    bool selected = false;

    juce::Component* parent;

    juce::Colour ColourOff = juce::Colours::transparentBlack;
    juce::Colour ColourMidiNotSet = juce::Colours::red;
    juce::Colour ColourMidiSet = juce::Colours::purple;
    float alphaMidiLearnOn = 0.8;
    float alphaMidiLearnOff = 0.3;
};

