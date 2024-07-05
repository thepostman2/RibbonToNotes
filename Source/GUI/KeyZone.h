/*
 ==============================================================================
 
 ZoneVisual.h
 Created: 7 Jun 2024 12:55:07pm
 Author:  Peter
 
 ==============================================================================
 */

#pragma once
#include "../PluginProcessor.h"

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

class KeyZone : public ZoneVisual,
private juce::Slider::Listener,
private juce::ComboBox::Listener

{
public:
    KeyZone (RibbonToNotesAudioProcessor&, int zoneid);
    ~KeyZone() override;
    
    void CreateGui();
    void AddListeners();
    void resized() override;

    void sliderValueChanged(juce::Slider* slider) override;
    void comboBoxChanged(juce::ComboBox* combobox) override;
    void cmbKeyOnChange();
    void cmbChordBuilderOnChange();
    void EdtChordBuilderOnChange();
    void ChordBuild();
    void setChordParameter(int j, float value);
    bool is_validnotenumber(const juce::String& str);

    juce::ComboBox cmbKey;
    juce::ComboBox cmbChord;
    juce::Label edtChordBuilder;

private:
    RibbonToNotesAudioProcessor& audioProcessor;
    
    bool edtChordChanged;
    juce::Slider sldArNoteNumber;
    juce::Label lblArNoteNumber;
    //juce::Slider sldArSplitValue;
    //juce::Label lblArSplitValue;
    juce::Slider sldChordNotesHelp[MAX_NOTES];//this is not visible, but helps to load selected notes from preset.

public:
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> cmbKeysAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> cmbChordsAttachment;
    //std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sldSplitValuesAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sldChordNotesHelpAttachment[MAX_NOTES];
};
