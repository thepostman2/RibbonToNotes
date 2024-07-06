/*
 ==============================================================================
 
 ZoneVisual.h
 Created: 7 Jun 2024 12:55:07pm
 Author:  PJP
 
 ==============================================================================
 */

#pragma once
#include "../PluginProcessor.h"

//==============================================================================
// base class for showing the zone. Red if active, black when not active
//==============================================================================
class ZoneVisual : public juce::Component
{
public:
    ZoneVisual(int zoneid)
    {
        ID = zoneid;
        setPaintingIsUnclipped(true);
    }
    bool FillColourOn;
    int ID;

private:
    void paint (juce::Graphics& g) override
    {
        juce::Rectangle<int> outline(getWidth(),getHeight());
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
};

//==============================================================================
// Class for Key Zone, with settings for key, chord mode and chord build.
//==============================================================================
class KeyZone : public ZoneVisual,
private juce::Slider::Listener,
private juce::ComboBox::Listener
{
public:
    KeyZone (RibbonToNotesAudioProcessor&, int zoneid);
    ~KeyZone() override;

    //==============================================================================
    // init functions
    //==============================================================================
    void CreateGui();
    void AddListeners();
    void resized() override;

    //==============================================================================
    // Apply changes when controls are changed
    //==============================================================================
    void cmbChordBuilderOnChange();
    void EdtChordBuilderOnChange();
    void GetChordFromChordString();
    void SetChordParameter(int j, float value);
    bool is_validnotenumber(const juce::String& str);

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

private:
    RibbonToNotesAudioProcessor& audioProcessor;
    
    bool edtChordChanged;
    juce::Slider sldChordNotesHelp[MAX_NOTES];//this is not visible, but helps to save and load the chord notes to and from a preset.

public:
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> cmbKeysAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> cmbChordsAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sldChordNotesHelpAttachment[MAX_NOTES];
};
