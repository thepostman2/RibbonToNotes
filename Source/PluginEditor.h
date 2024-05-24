/*
 ==============================================================================
 
 This file contains the basic framework code for a JUCE plugin editor.
 
 ==============================================================================
 */

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "GUI/PresetPanel.h"

const char* const notes[] = {"C","C#/Db","D","D#/Eb","E","F","F#/Gb","G","G#/Ab","A","A#/Bb","B"};
const juce::StringArray notesArray((const char**) notes);
const char* const chords[] = {"None","Power","Major","Minor","Dominant 7","Minor 7","Major 7","Diminished"};
const juce::StringArray chordsArray((const char**) chords);
const char* const chordbuilds[] = {"empty","1","1,8","1,5,8","1,4,8","1,5,8,11","1,4,8,11","1,5,8,12","1,4,7"};
const juce::StringArray chordbuildsArray((const char**) chordbuilds);
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

    RibbonToNotesAudioProcessorEditor (RibbonToNotesAudioProcessor&, juce::AudioProcessorValueTreeState&);
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
    void SyncNotesAndSplits();
    void CreateDial(juce::Slider& sld);
    void CreateSlider(juce::Slider& sld);
    void SetSplitRanges();
    void RedistributeSplitRanges();
    void SetChordBuildFromChord(int i);
    void GetChordBuild(int i);
    bool is_validnotenumber(const juce::String& str);

    void sliderValueChanged(juce::Slider* slider) override;
    void comboBoxChanged(juce::ComboBox* combobox) override;
    
    
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    RibbonToNotesAudioProcessor& audioProcessor;
    juce::AudioProcessorValueTreeState& valueTreeState;
    
    GUI::PresetPanel presetPanel;

    juce::Slider sldMidiCC;
    std::unique_ptr<SliderAttachment> sldMidiCCAttachment;
    std::unique_ptr<SliderAttachment> sldNumberOfZonesAttachment;
    std::unique_ptr<SliderAttachment> sldVelocityAttachment;
    std::unique_ptr<SliderAttachment> sldOctaveAttachment;
    std::unique_ptr<ComboBoxAttachment> cmbNotesAttachment[MAX_NOTES];
    std::unique_ptr<ComboBoxAttachment> cmbChordsAttachment[MAX_NOTES];
    std::unique_ptr<SliderAttachment> sldSplitValuesAttachment[MAX_SPLITS];
    std::unique_ptr<SliderAttachment> sldSplitExtraValuesAttachment;

    
    juce::Label lblMidiCC;
    juce::Slider sldNumberOfZones;
    juce::Label lblNumberOfZones;
    juce::Slider sldVelocity;
    juce::Label lblVelocity;
    juce::Slider sldOctave;
    juce::Label lblOctave;

    
    juce::ComboBox cmbNotes[MAX_NOTES];
    juce::ComboBox cmbChords[MAX_NOTES];
    juce::Label edtChordBuilder[MAX_NOTES];
    juce::Slider sldArNoteNumber[MAX_NOTES];
    juce::Label lblArNoteNumber[MAX_NOTES];
    juce::Slider sldArSplitValues[MAX_SPLITS];
    juce::Label lblArSplitValues[MAX_SPLITS];
    juce::Slider sldArSplitExtra; //this is the slider at the end of the first row
    juce::Label lblArSplitExtra; //this is the label for the slider at the end of the first row
    RibbonZoneVisual ribbonZoneVisuals[MAX_SPLITS+1];
    
    int lastNumberOfZones=6;
    int numberOfSplits(){return ((int)(*audioProcessor.numberOfZones))-1;}
    int noteOrder[MAX_NOTES] = {1,3,5,6,8,10,12,1,3,5,6,8};
    

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RibbonToNotesAudioProcessorEditor)
};
