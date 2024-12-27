/*
 ==============================================================================
 
 This file contains the basic framework code for a JUCE plugin editor.
 
 ==============================================================================
 */

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "GUI/PresetPanel.h"
#include "GUI/KeyZone.h"
#include "GUI/SelectionKnob.h"
#include "Service/PresetManager.h"
#include "MidiLearn/MidiLearnObject.h"

//==============================================================================
class RibbonToNotesAudioProcessorEditor  : public juce::AudioProcessorEditor,
private juce::Slider::Listener,
private juce::ComboBox::Listener,
private juce::Button::Listener,
private juce::Timer
{
public:
    typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
    typedef juce::AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;

    RibbonToNotesAudioProcessorEditor (RibbonToNotesAudioProcessor&);
    ~RibbonToNotesAudioProcessorEditor() override;
    
    //==============================================================================
    void timerCallback() override;
    void paint (juce::Graphics&) override;
    void resized() override;
    
private:
    //==============================================================================
    // init functions
    //==============================================================================
    void CreateRibbon();
    void CreateProgressionSelectorKnobs();
    void CreateGui();
    void AddListeners();
    void RemoveListeners();
    void CreateDial(juce::Slider& sld);
    void CreateSlider(juce::Slider& sld);

    //==============================================================================
    // Apply changes when controls are changed
    //==============================================================================
    int GetNumberOfZones();
    void SyncZoneSliderValues();
    void SyncKeyAndChordModes(int progression, int zone);
    void TransposeKeyAndChordModes(int progression, int transpose);
    void RedistributeSplitRanges(bool initSplitValues);

    //==============================================================================
    // listeners
    //==============================================================================
    void sliderValueChanged(juce::Slider* slider) override;
    void comboBoxChanged(juce::ComboBox* combobox) override;
    void buttonClicked(juce::Button* button) override;
    
    //==============================================================================
    // build notes to play
    //==============================================================================
    void BuildChordsForAllProgressions();
public:
    void BuildChords(int alternative);
    static void BuildChordsWrapper(void* audioProcessorEditor, int progression)
    {
        if(audioProcessorEditor != NULL)
        {
            static_cast<RibbonToNotesAudioProcessorEditor*>(audioProcessorEditor)->BuildChords(progression);
        }
    }
private:
    void GetNoteNumbersForChord(int addOctaves, int progression, int zone, int note);
    //==============================================================================
    // Update functions for the visuals
    //==============================================================================
    void ShowActiveAlternative();
    void ShowRibbonZone(int area);

    //==============================================================================
    // Properties
    //==============================================================================
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    RibbonToNotesAudioProcessor& audioProcessor;
    
    // GUI elements
    GUI::PresetPanel presetPanel;
    juce::Slider sldMidiCC;
    juce::Label lblMidiCC;
    juce::Slider sldNumberOfZones;
    juce::Label lblNumberOfZones;
    juce::Slider sldVelocity;
    juce::Label lblVelocity;
    juce::Slider sldOctave;
    juce::Label lblOctave;

    juce::ToggleButton toggleShowMidiLearnSettings;
    juce::Label lblShowMidiLearnSettings;

    juce::ComboBox cmbChannelIn;
    juce::Label lblChannelIn;
    juce::ComboBox cmbChannelOut;
    juce::Label lblChannelOut;
    juce::ComboBox cmbPitchModes;
    juce::Label lblPitchModes;
    juce::ComboBox cmbActiveProgression;
    juce::Label lblActiveProgression;
    
    ZoneVisual ribbonZeroZone;
    juce::OwnedArray<KeyZone> ribbonKeyZone[MAX_PROGRESSIONS];
    juce::OwnedArray<SelectionKnob> selectProgressionKnobs;
    SelectionKnob prevProgression;
    SelectionKnob nextProgression;

    juce::Slider sldSplitValues[MAX_SPLITS];
    juce::Label lblSplitValues[MAX_SPLITS];

    // utility variables
    int lastNumberOfZones=6;
    int numberOfSplits(){return ((int)(*audioProcessor.numberOfZones))-1;}
    int rootKey[MAX_PROGRESSIONS];
    bool splitValuesSetFromCode = false;
   
public:
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sldMidiCCAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sldNumberOfZonesAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sldVelocityAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sldOctaveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> cmbChannelInAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> cmbChannelOutAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> cmbKeysAttachment[MAX_ZONES];
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> cmbChordsAttachment[MAX_ZONES];
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sldSplitValuesAttachment[MAX_SPLITS];
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> cmbPitchModesAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> cmbActiveProgressionAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RibbonToNotesAudioProcessorEditor)
};
