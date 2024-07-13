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
#include "Service/PresetManager.h"

//==============================================================================
class RibbonToNotesAudioProcessorEditor  : public juce::AudioProcessorEditor,
private juce::Slider::Listener,
private juce::ComboBox::Listener,
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
    void CreateGui();
    void AddListeners();
    void CreateDial(juce::Slider& sld);
    void CreateSlider(juce::Slider& sld);

    //==============================================================================
    // Apply changes when controls are changed
    //==============================================================================
    int GetNumberOfZones();
    void SyncZoneSliderValues();
    void SyncKeyAndChordModes();
    void RedistributeSplitRanges(bool initSplitValues);

    //==============================================================================
    // listeners
    //==============================================================================
    void sliderValueChanged(juce::Slider* slider) override;
    void comboBoxChanged(juce::ComboBox* combobox) override;
    
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
    juce::ComboBox cmbChannelIn;
    juce::Label lblChannelIn;
    juce::ComboBox cmbChannelOut;
    juce::Label lblChannelOut;
    juce::ComboBox cmbPitchModes;
    juce::Label lblPitchModes;
    juce::ComboBox cmbActiveAlternative;
    juce::Label lblActiveAlternative;

    ZoneVisual ribbonZeroZone;
    juce::OwnedArray<KeyZone> ribbonKeyZone[MAX_ALTERNATIVES];

    juce::Slider sldSplitValues[MAX_SPLITS];
    juce::Label lblSplitValues[MAX_SPLITS];

    // utility variables
    juce::Slider sldChordNotesHelp[MAX_ALTERNATIVES][MAX_ZONES][MAX_NOTES];//this is not visible, but helps to load selected notes from preset.
    int lastNumberOfZones=6;
    int numberOfSplits(){return ((int)(*audioProcessor.numberOfZones))-1;}
    int rootKey[MAX_ALTERNATIVES];
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
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> cmbActiveAlternativeAttachment;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sldChordNotesHelpAttachment[MAX_ALTERNATIVES][MAX_ZONES][MAX_NOTES];

    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RibbonToNotesAudioProcessorEditor)
};
