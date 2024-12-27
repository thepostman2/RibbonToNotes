/*
  ==============================================================================

    ZoneVisual.h
    Created: 13 Jul 2024 3:07:35pm
    Author:  Peter

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "LookAndFeelZoneButton.h"

//==============================================================================
// base class for showing the zone. Red if active, black when not active
//==============================================================================
class ZoneVisual : public juce::TextButton
{
public:
    ZoneVisual(int progressionid, int zoneid) : TextButton()
    {
        ZONE_ID = zoneid;
        PROGRESSION_ID = progressionid;
        setPaintingIsUnclipped(true);
    }
    bool FillColourOn = false;
    int ZONE_ID;
    int PROGRESSION_ID;
    juce::Colour ColourOn = juce::Colours::red;
    juce::Colour ColourOff = juce::Colours::black;
    float alpha = 0.5;
    
    void PaintZoneVisual(juce::Graphics& g)
    {
        TextButton::paint(g);
        bigTextLookAndFeel.drawButtonText(g, *this, false, false);
        TextButton::setColour(juce::TextButton::buttonColourId, GetKnobColour());
    }

    virtual juce::Colour GetKnobColour()
    {
        return KnobColour();
    }

    juce::Colour KnobColour()
    {
        return FillColourOn ? ColourOn.withAlpha(alpha) : ColourOff.withAlpha(alpha);
    }

private:
    LookAndFeelZoneButton bigTextLookAndFeel;

    void paint (juce::Graphics& g) override
    {
        PaintZoneVisual(g);
    }
    
    
};

