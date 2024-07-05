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
    void ShowRibbonZone(int area);
    
    
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
    void RedistributeSplitRanges();
    void ChordBuild(int i);

    //==============================================================================
    // listeners
    //==============================================================================
    void sliderValueChanged(juce::Slider* slider) override;
    void comboBoxChanged(juce::ComboBox* combobox) override;
    
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
    juce::ComboBox cmbPitchModes;
    
    ZoneVisual ribbonZeroZone;
    juce::OwnedArray<KeyZone> ribbonKeyZone;

    juce::Slider sldSplitValues[MAX_SPLITS];
    juce::Label lblSplitValues[MAX_SPLITS];
    juce::Slider sldSplitEnd; //this is the slider at the end of the zone
    juce::Label lblSplitEnd; //this is the label for the slider at the end of the zone

    // utility variables
    juce::Slider sldChordNotesHelp[MAX_ZONES][MAX_NOTES];//this is not visible, but helps to load selected notes from preset.
    int lastNumberOfZones=6;
    int numberOfSplits(){return ((int)(*audioProcessor.numberOfZones))-1;}
    int keyOrder[MAX_ZONES] = {1,3,5,6,8,10,12,1};
   
public:
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sldMidiCCAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sldNumberOfZonesAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sldVelocityAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sldOctaveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> cmbKeysAttachment[MAX_ZONES];
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> cmbChordsAttachment[MAX_ZONES];
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sldSplitValuesAttachment[MAX_SPLITS];
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sldSplitExtraValuesAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> cmbPitchModesAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sldChordNotesHelpAttachment[MAX_ZONES][MAX_NOTES];

    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RibbonToNotesAudioProcessorEditor)
};
