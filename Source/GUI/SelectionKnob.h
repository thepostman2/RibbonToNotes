/*
  ==============================================================================

    SelectionKnob.h
    Created: 13 Jul 2024 3:01:45pm
    Author:  PJP

  ==============================================================================
*/
#pragma once
#include "ZoneVisual.h"
#include "../PluginProcessor.h"

//==============================================================================
// Class for Selection Knob that responds to midi CC or note message
//==============================================================================
class SelectionKnob : public ZoneVisual,
private juce::ComboBox::Listener,
private juce::Button::Listener
{
public:
    SelectionKnob (RibbonToNotesAudioProcessor&, int alternativeid);
    ~SelectionKnob() override;

    void CreateGui();
    void AddListeners();
    void RemoveListeners();
    void resized() override;

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


private:
    RibbonToNotesAudioProcessor& audioProcessor;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> cmbMidiMessageAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> cmbMidiInChannelAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> cmbMidiInNumberAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> cmbMidiInValueAttachment;
};
