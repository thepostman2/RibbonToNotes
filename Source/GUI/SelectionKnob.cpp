/*
  ==============================================================================

    SelectionKnob.cpp
    Created: 13 Jul 2024 3:01:45pm
    Author:  PJP

  ==============================================================================
*/

#include "SelectionKnob.h"

SelectionKnob::SelectionKnob (RibbonToNotesAudioProcessor& p, int progressionid)
: ZoneVisualMidiLearn(progressionid, 0),
audioProcessor (p)
{
    CreateGui();
    AddListeners();
}

SelectionKnob::~SelectionKnob()
{
    RemoveListeners();
    cmbMidiMessageAttachment = nullptr;
    cmbMidiInChannelAttachment = nullptr;
    cmbMidiInNumberAttachment = nullptr;
    cmbMidiInValueAttachment = nullptr;
}

void SelectionKnob::CreateGui()
{
    MidiLearnInterface::CreateGui();
    
    setButtonText(progressionKnobs[PROGRESSION_ID]);
    
    cmbMidiMessageAttachment= std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (audioProcessor.apvts, MIDIINSKALTMESSAGETYPE_ID + std::to_string(PROGRESSION_ID) , cmbMidiInMessage);

    cmbMidiInChannelAttachment= std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (audioProcessor.apvts, MIDIINSKALTCHANNEL_ID + std::to_string(PROGRESSION_ID) , cmbMidiInChannel);

    cmbMidiInNumberAttachment= std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (audioProcessor.apvts, MIDIINSKALTNUMBER_ID + std::to_string(PROGRESSION_ID) , cmbMidiInNumber);

    cmbMidiInValueAttachment= std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (audioProcessor.apvts, MIDIINSKALTVALUE_ID + std::to_string(PROGRESSION_ID) , cmbMidiInValue);
}
void SelectionKnob::AddListeners()
{
    MidiLearnInterface::AddMidiInterfaceListeners();
}
void SelectionKnob::RemoveListeners()
{
    MidiLearnInterface::RemoveMidiInterfaceListeners();
}

void SelectionKnob::resized()
{
    MidiLearnInterface::ResizeControls();
    
}

//==============================================================================
// listeners
//==============================================================================
void SelectionKnob::buttonClicked(juce::Button* button)
{
    FillColourOn = true;
    repaint();
}
