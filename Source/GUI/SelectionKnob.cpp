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
    sldMidiInMinValueAttachment = nullptr;
}

void SelectionKnob::CreateGui()
{
    MidiLearnInterface::CreateGui();
    
    setButtonText(progressionKnobs[PROGRESSION_ID]);
    
    cmbMidiMessageAttachment= std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (audioProcessor.apvts, DEFCONCAT(MIDIINMESSAGETYPE_ID, PROGRESSION) + std::to_string(PROGRESSION_ID) , cmbMidiInMessage);

    cmbMidiInChannelAttachment= std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (audioProcessor.apvts, DEFCONCAT(MIDIINCHANNEL_ID, PROGRESSION) + std::to_string(PROGRESSION_ID) , cmbMidiInChannel);

    cmbMidiInNumberAttachment= std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (audioProcessor.apvts, DEFCONCAT(MIDIINNUMBER_ID, PROGRESSION) + std::to_string(PROGRESSION_ID) , cmbMidiInNumber);

    sldMidiInMinValueAttachment= std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, DEFCONCAT(MIDIINMINVALUE_ID, PROGRESSION) + std::to_string(PROGRESSION_ID) , sldMidiInMinValue);

    sldMidiInMaxValueAttachment= std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, DEFCONCAT(MIDIINMAXVALUE_ID, PROGRESSION) + std::to_string(PROGRESSION_ID) , sldMidiInMaxValue);
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
    MidiLearnInterface::buttonClicked(button);
    repaint();
}
