/*
  ==============================================================================

    SelectionKnob.h
    Created: 13 Jul 2024 3:01:45pm
    Author:  PJP

  ==============================================================================
*/
#pragma once
#include "ZoneVisualMidiLearn.h"
#include "../PluginProcessor.h"

//==============================================================================
// Class for Selection Knob that responds to midi CC or note message
//==============================================================================
class SelectionKnob : public ZoneVisualMidiLearn
{
public:
    SelectionKnob (RibbonToNotesAudioProcessor&, int alternativeid);
    ~SelectionKnob() override;

    void CreateGui();
    void AddListeners();
    void RemoveListeners();
    void resized() override;

    //==============================================================================
    // listeners
    //==============================================================================
    void buttonClicked(juce::Button* button) override;


    //==============================================================================
    // public properties
    //==============================================================================


private:
    RibbonToNotesAudioProcessor& audioProcessor;
};
