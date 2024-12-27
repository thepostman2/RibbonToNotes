/*
  ==============================================================================

    SliderMidiLearn.h
    Created: 27 Dec 2024 12:45:42pm
    Author:  Peter

  ==============================================================================
*/

#pragma once
#include "MidiLearnInterface.h"
//==============================================================================
// Slider class with midi learn options
//==============================================================================
class SliderMidiLearn : public juce::Slider,
public MidiLearnInterface
{
public:
    SliderMidiLearn(RibbonToNotesAudioProcessor& p, juce::String midiLearnID) : Slider(),
    MidiLearnInterface(this),
    audioProcessor(p),
    MidiLearnID(midiLearnID)
    {
        setPaintingIsUnclipped(true);
        CreateGui();
    }
    ~SliderMidiLearn()
    {
        RemoveListeners();
    }

    void CreateGui()
    {
        MidiLearnInterface::CreateGui();

        cmbMidiMessageAttachment= std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (audioProcessor.apvts, MidiInMessageType + MidiLearnID , cmbMidiInMessage);

        cmbMidiInChannelAttachment= std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (audioProcessor.apvts, MidiInChannel + MidiLearnID , cmbMidiInChannel);

        cmbMidiInNumberAttachment= std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (audioProcessor.apvts, MidiInNumber + MidiLearnID , cmbMidiInNumber);

        cmbMidiInValueAttachment= std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (audioProcessor.apvts, MidiInValue + MidiLearnID , cmbMidiInValue);
    }

    void resized() override
    {
        Slider::resized();
        MidiLearnInterface::ResizeControls();
    }
    void AddListeners()
    {
        MidiLearnInterface::AddMidiInterfaceListeners();
    }
    void RemoveListeners()
    {
        MidiLearnInterface::RemoveMidiInterfaceListeners();
    }


private:
    RibbonToNotesAudioProcessor& audioProcessor;
    juce::String MidiLearnID;
    juce::String MidiInMessageType = MIDIINMESSAGETYPE_ID;
    juce::String MidiInChannel = MIDIINCHANNEL_ID;
    juce::String MidiInNumber = MIDIINNUMBER_ID;
    juce::String MidiInValue = MIDIINVALUE_ID;

    void paint (juce::Graphics& g) override
    {
        Slider::paint(g);
        MidiLearnInterface::ShowMidiSettings();
    }
};
