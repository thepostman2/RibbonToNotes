/*
 ==============================================================================
 
 ZoneVisual.h
 Created: 7 Jun 2024 12:55:07pm
 Author:  PJP
 
 ==============================================================================
 */
#pragma once
#include "ZoneVisual.h"
#include "../PluginProcessor.h"

//==============================================================================
// Class for Key Zone, with settings for key, chord mode and chord build.
//==============================================================================
class KeyZone : public ZoneVisual,
private juce::Slider::Listener,
private juce::ComboBox::Listener 
{
public:
    KeyZone (RibbonToNotesAudioProcessor&, int alternativeid, int zoneid);
    ~KeyZone() override;

    //==============================================================================
    // init functions
    //==============================================================================
    void CreateGui();
    void AddListeners();
    void RemoveListeners();
    void resized() override;
    void setVisible(bool visible) override;

    //==============================================================================
    // Apply changes when controls are changed
    //==============================================================================
    void cmbChordBuilderOnChange();
    void EdtChordBuilderOnChange();
    void SetChordStringText();
    void GetChordFromChordString();
    void SetChordParameter(int j, float value);
    void SetNoteParameter(int j, float value);
    bool is_validnotenumber(const juce::String& str);
    void BuildChords();
    int GetRelativeNoteNumber(int selectedkey, int notenumber);
    void* ribbonToNotesAudioProcessorEditor;
    void (*BuildChordsFuncP)(void*, int progression_id);
    int (*GetRelativeNoteNumberP)(void*, int progression_id, int selectedzone, int notenumber);

    bool midiLearnMessage(juce::MidiBuffer messagebuffer, int selectedzone);

    //==============================================================================
    // listeners
    //==============================================================================
    void sliderValueChanged(juce::Slider* slider) override;
    void comboBoxChanged(juce::ComboBox* combobox) override;

    
    
    //==============================================================================
    // public properties
    //==============================================================================
    juce::ComboBox cmbKey;
    juce::ComboBox cmbChord;
    juce::Label edtChordBuilder;
    
    int currentKey;
    int prevKey;
    
    int Transposed();

private:
    RibbonToNotesAudioProcessor& audioProcessor;
    
    bool edtChordChanged;
    juce::Slider sldChordNotesHelp[MAX_NOTES];//this is not visible, but helps to save and load the chord notes to and from a preset.
    juce::Slider sldNotesToPlayHelp[MAX_NOTES];//this is not visible, but helps to save and load the chord notes to and from a preset.
    juce::Array<int> LearnedNotes;
    
public:

    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> cmbKeysAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> cmbChordsAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sldChordNotesHelpAttachment[MAX_NOTES];
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sldNotesToPlayHelpAttachment[MAX_NOTES];
};
