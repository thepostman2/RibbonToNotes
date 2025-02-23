/*
  ==============================================================================

    MidiLearnGroup.cpp
    Created: 31 Dec 2024 3:21:01pm
    Author:  Peter

  ==============================================================================
*/

#include "MidiLearnGroup.h"

void MidiLearnGroup::Add(MidiLearnInterface* obj)
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

void MidiLearnGroup::changeListenerCallback(juce::ChangeBroadcaster * source) 
{
    for(int i=0; i < MidiLearnControls.size(); ++i)
    {
        MidiLearnControls[i]->setSelected(&MidiLearnControls[i]->onMidiLearnChanges == source);
    }
}

bool MidiLearnGroup::midiLearnMessage(juce::MidiBuffer messagebuffer)
{
    bool Learned = false;
    for(const auto metadata : messagebuffer)
    {
        auto message = metadata.getMessage();
        for(int i=0; i < MidiLearnControls.size(); ++i)
        {
            if(MidiLearnControls[i]->getSelected())
            {
                Learned = MidiLearnControls[i]->MidiLearnMessage(message);
                break;
            }
        }
        break;
    }
    return Learned;
}
