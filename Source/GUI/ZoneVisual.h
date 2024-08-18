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
    bool FillColourOn;
    int ZONE_ID;
    int PROGRESSION_ID;
    juce::Colour ColourOn = juce::Colours::red;
    juce::Colour ColourOff = juce::Colours::black;

private:
    LookAndFeelZoneButton bigTextLookAndFeel;

    void paint (juce::Graphics& g) override
    {
        TextButton::paint(g);
        bigTextLookAndFeel.drawButtonText(g, *this, false, false);
        if(FillColourOn)
        {
            TextButton::setColour(juce::TextButton::buttonColourId, ColourOn);
        }
        else
        {
            TextButton::setColour(juce::TextButton::buttonColourId,ColourOff);
        }
    }
};
