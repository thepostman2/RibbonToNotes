/*
  ==============================================================================

    AtomicMidiInfo.h
    Created: 27 Dec 2024 11:14:04pm
    Author:  PJP

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

struct AtomicMidiInfo
{
    std::string MidiInfoID;
    std::atomic<float>*  MessageType;
    std::atomic<float>*  Channel;
    std::atomic<float>*  Number;
    std::atomic<float>*  MinValue;
    std::atomic<float>*  MaxValue;
    
    bool MidiMessageComplies(const juce::MidiMessage &midiMessage)
    {
        auto messageType = midiMessage.isController() ? 1 : midiMessage.isNoteOn() ? 2 : 0;
        if(messageType == 0) return false;
        auto messageChannel = midiMessage.getChannel();
        auto messageNumber = messageType == 1 ? midiMessage.getControllerNumber() : midiMessage.getNoteNumber();
        auto messageValue = messageType == 1 ? midiMessage.getControllerValue() : midiMessage.getVelocity();

        return (messageType == (int) *MessageType
           && messageNumber == (int) *Number
           && ((int) *Channel == 0 || messageChannel == (int) *Channel));

    }
};
