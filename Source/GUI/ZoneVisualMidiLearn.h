/*
  ==============================================================================

    ZoneVisualMidiLearn.h
    Created: 27 Dec 2024 9:21:59am
    Author:  Peter

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "ZoneVisual.h"
#include "MidiLearnInterface.h"
//==============================================================================
// base class for showing the zone. Red if active, black when not active
//==============================================================================
class ZoneVisualMidiLearn : public ZoneVisual,
public MidiLearnInterface
{
public:
    ZoneVisualMidiLearn(int progressionid, int zoneid) : ZoneVisual(progressionid, zoneid), MidiLearnInterface(this)
    {
    }

    juce::Colour GetKnobColour()override
    {
        return MidiLearnInterface::MidiSettingOn ? GetMidiActiveColour() :  KnobColour();
    }

private:
    LookAndFeelZoneButton bigTextLookAndFeel;

    void paint (juce::Graphics& g) override
    {
        ZoneVisual::PaintZoneVisual(g);
        MidiLearnInterface::ShowMidiSettings();
    }
};

