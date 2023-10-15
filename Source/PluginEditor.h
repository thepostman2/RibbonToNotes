/*
 ==============================================================================
 
 This file contains the basic framework code for a JUCE plugin editor.
 
 ==============================================================================
 */

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

const char* const notes[] = {"C","C#/Db","D","D#/Eb","E","F","F#/Gb","G","G#/Ab","A","A#/Bb","B"};
const juce::StringArray notesArray((const char**) notes);
//==============================================================================
/**
 */
class RibbonToNotesAudioProcessorEditor  : public juce::AudioProcessorEditor,
private juce::Slider::Listener,
private juce::ComboBox::Listener
{
public:
    typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
    typedef juce::AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;

    RibbonToNotesAudioProcessorEditor (RibbonToNotesAudioProcessor&, juce::AudioProcessorValueTreeState&);
    ~RibbonToNotesAudioProcessorEditor() override;
    
    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    
private:
    void CreateGui();
    void AddListeners();
    int GetNumberOfZones();
    void extracted();
    
    void SyncSliderValues();
    void SyncComboBoxValues();
    void SyncNotesAndSplits();
    void CreateDial(juce::Slider& sld);
    void CreateSlider(juce::Slider& sld);
    void SetSplitRanges();
    void RedistributeSplitRanges();

    void sliderValueChanged(juce::Slider* slider) override;
    void comboBoxChanged(juce::ComboBox* combobox) override;
    
    
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    RibbonToNotesAudioProcessor& audioProcessor;
    juce::AudioProcessorValueTreeState& valueTreeState;

    juce::Slider sldMidiCC;
    std::unique_ptr<SliderAttachment> sldMidiCCAttachment;
    std::unique_ptr<SliderAttachment> sldNumberOfZonesAttachment;
    std::unique_ptr<SliderAttachment> sldVelocityAttachment;
    std::unique_ptr<SliderAttachment> sldOctaveAttachment;
    std::unique_ptr<ComboBoxAttachment> cmbNotesAttachment[MAX_NOTES];

    
    juce::Label lblMidiCC;
    juce::Slider sldNumberOfZones;
    juce::Label lblNumberOfZones;
    juce::Slider sldVelocity;
    juce::Label lblVelocity;
    juce::Slider sldOctave;
    juce::Label lblOctave;

    
    juce::ComboBox cmbNotes[MAX_NOTES];
    juce::Slider sldArNoteNumber[MAX_NOTES];
    juce::Label lblArNoteNumber[MAX_NOTES];
    juce::Slider sldArSplitValues[MAX_NOTES];
    juce::Label lblArSplitValues[MAX_NOTES];
    
    int numberOfZones=6;
    int numberOfSplits(){return numberOfZones-1;}
    int noteOrder[MAX_NOTES] = {1,3,5,6,8,10,12,1,3,5,12};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RibbonToNotesAudioProcessorEditor)
};
