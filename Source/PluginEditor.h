/*
 ==============================================================================
 
 This file contains the basic framework code for a JUCE plugin editor.
 
 ==============================================================================
 */

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "GUI/PresetPanel.h"
#include "Service/PresetManager.h"

//==============================================================================
struct RibbonZoneVisual : public juce::Component
{
    RibbonZoneVisual(){setPaintingIsUnclipped(true);}
    
    void paint (juce::Graphics& g) override
    {
        juce::Rectangle<int> outline(getHeight(),getWidth());
        g.setColour(juce::Colours::whitesmoke);
        g.drawRect(outline);
        if(FillColourOn)
        {
            g.setColour(juce::Colours::red);
            g.fillRect(outline);
        }
        else
        {
            g.setColour(juce::Colours::black);
            g.fillRect(outline);
        }
    }
    
    bool FillColourOn;
};
//==============================================================================

/**
 */
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
    void CreateGui();
    void AddListeners();
    int GetNumberOfZones();
    void extracted();
    
    void SyncSliderValues();
    void SyncComboBoxValues();
    void extracted(int addOctaves, int i, int note);
    
    void SyncNotesAndSplits();
    void CreateDial(juce::Slider& sld);
    void CreateSlider(juce::Slider& sld);
    void RedistributeSplitRanges();
    void cmbChordBuilderOnChange(int i);
    void EdtChordBuilderOnChange(int i);
    void ChordBuild(int i);
    void setChordParameter(int key, int j, float value);
    bool is_validnotenumber(const juce::String& str);

    void sliderValueChanged(juce::Slider* slider) override;
    void comboBoxChanged(juce::ComboBox* combobox) override;
    
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    RibbonToNotesAudioProcessor& audioProcessor;
    
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
    

    
    juce::ComboBox cmbKeys[MAX_ZONES];
    juce::ComboBox cmbChords[MAX_ZONES];
    juce::Label edtChordBuilder[MAX_ZONES];
    bool edtChordChanged[MAX_ZONES];
    juce::Slider sldArNoteNumber[MAX_ZONES];
    juce::Label lblArNoteNumber[MAX_ZONES];
    juce::Slider sldArSplitValues[MAX_SPLITS];
    juce::Label lblArSplitValues[MAX_SPLITS];
    juce::Slider sldArSplitExtra; //this is the slider at the end of the first row
    juce::Label lblArSplitExtra; //this is the label for the slider at the end of the first row

    juce::Slider sldChordNotesHelp[MAX_ZONES][MAX_NOTES];//this is not visible, but helps to load selected notes from preset.


    RibbonZoneVisual ribbonZoneVisuals[MAX_SPLITS+1];
    
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
