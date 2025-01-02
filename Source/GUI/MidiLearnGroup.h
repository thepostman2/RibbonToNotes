/*
  ==============================================================================

    MidiLearnGroup.h
    Created: 31 Dec 2024 3:21:01pm
    Author:  Peter

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "MidiLearnInterface.h"

class MidiLearnGroup :
public juce::ChangeListener
{
public:
    ~MidiLearnGroup()
    {
        for(int i=0; i < MidiLearnControls.size(); ++i)
        {
            MidiLearnControls[i]->onMidiLearnChanges.removeChangeListener(this);
        }
    }

    void Add(MidiLearnInterface* obj)
    {
        auto addit = true;
        for(int i=0; i < MidiLearnControls.size(); ++i)
        {
            if(MidiLearnControls[i] == obj)
            {
                addit = false;
            }
        }
        if(addit)
        {
            obj->onMidiLearnChanges.addChangeListener(this);
            MidiLearnControls.add(obj);
        }
    }

    void changeListenerCallback(juce::ChangeBroadcaster * source) override
    {
        for(int i=0; i < MidiLearnControls.size(); ++i)
        {
            MidiLearnControls[i]->setSelected(&MidiLearnControls[i]->onMidiLearnChanges == source);
        }
    }
    
    private:
    juce::Array<MidiLearnInterface*> MidiLearnControls;

};

