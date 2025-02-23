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
        AddListeners();
    }
    ~SliderMidiLearn()
    {
        RemoveListeners();
    }

    void CreateGui()
    {
        MidiLearnInterface::CreateGui();

        cmbMidiMessageAttachment= std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (audioProcessor.apvts, MIDIINMESSAGETYPE_ID + MidiLearnID , cmbMidiInMessage);

        cmbMidiInChannelAttachment= std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (audioProcessor.apvts, MIDIINCHANNEL_ID + MidiLearnID , cmbMidiInChannel);

        cmbMidiInNumberAttachment= std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (audioProcessor.apvts, MIDIINNUMBER_ID + MidiLearnID , cmbMidiInNumber);

        sldMidiInMinValueAttachment= std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, MIDIINMINVALUE_ID + MidiLearnID , sldMidiInMinValue);

        sldMidiInMaxValueAttachment= std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, MIDIINMAXVALUE_ID + MidiLearnID , sldMidiInMaxValue);
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

    //==============================================================================
    // listeners
    //==============================================================================
    void buttonClicked(juce::Button* button) override
    {
        MidiLearnInterface::buttonClicked(button);
        repaint();
    }
    

private:
    RibbonToNotesAudioProcessor& audioProcessor;
    juce::String MidiLearnID;

    void paint (juce::Graphics& g) override
    {
        Slider::paint(g);
        MidiLearnInterface::ShowMidiSettings();
    }
};
