/*
  ==============================================================================

    MidiLearnInterface.h
    Created: 26 Dec 2024 12:04:52pm
    Author:  PJP

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "../PluginProcessor.h"

//==============================================================================
// base class for showing the zone. Red if active, black when not active
//==============================================================================
class MidiLearnInterface :
public juce::ComboBox::Listener,
public juce::Button::Listener
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

    //==============================================================================
    // public properties
    //==============================================================================
    juce::ComboBox cmbMidiInMessage;
    juce::ComboBox cmbMidiInChannel;
    juce::ComboBox cmbMidiInNumber;
    juce::ComboBox cmbMidiInValue;

    juce::Label lblMidiMessage;
    juce::Label lblChannel;
    juce::Label lblMidiNumber; //cc number or note number
    juce::Label lblMidiValueTreshold; //control value or note velocity above wich the selector knob is active;

    static inline bool MidiSettingOn = false;
    bool MidiLearnOn = false;
    bool MidiLearnNew();

    juce::Colour ColourOff = juce::Colours::transparentBlack;
    juce::Colour ColourMidiNotSet = juce::Colours::red;
    juce::Colour ColourMidiSet = juce::Colours::purple;
    float alphaMidiLearnOn = 0.8;
    float alphaMidiLearnOff = 0.5;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> cmbMidiMessageAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> cmbMidiInChannelAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> cmbMidiInNumberAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> cmbMidiInValueAttachment;

    void PaintMidiSettings(juce::Graphics& g)
    {
        ShowMidiSettings();
    }

    juce::Colour GetMidiActiveColour()
    {
        auto alpha = MidiLearnOn ? alphaMidiLearnOn : alphaMidiLearnOff;
        return MidiSettingOn ? (MidiLearnNew() ? ColourMidiNotSet.withAlpha(alpha) : ColourMidiSet.withAlpha(alpha)) : ColourOff.withAlpha(0.0f);
    }

    void ShowMidiSettings();

    
private:
    juce::Component* parent;
};

