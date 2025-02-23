/*
 ==============================================================================
 
 This file contains the basic framework code for a JUCE plugin processor.
 
 ==============================================================================
 */

#pragma once

#include <JuceHeader.h>
#include "AtomicMidiInfo.h"
#include "Service/PresetManager.h"

#define DEFCONCAT(first, second) first second

const int NONOTE = -128;

#define MAX_PROGRESSIONS 6
#define MAX_PROGRESSIONSKNOBS MAX_PROGRESSIONS+2
#define MAX_NOTES 12
#define MAX_ZONES 8
#define MAX_SPLITS MAX_ZONES+1
#define DEFAULT_NUMBEROFZONES 6

#define MIDICC_ID "midicc"
#define MIDICC_NAME "midi cc"
#define NUMBEROFZONES_ID "numberofzones"
#define NUMBEROFZONES_NAME "Number of zones"
#define VELOCITY_ID "velocity"
#define VELOCITY_NAME "Velocity"
#define OCTAVES_ID "octaves"
#define OCTAVES_NAME "Octaves"

#define TOGGLEMIDI_NAME "Show midi controls"
#define TOGGLEMIDILEARN_NAME "Midi learn"

#define CHANNELIN_ID "channelin"
#define CHANNELIN_NAME "Channel in"
#define CHANNELOUT_ID "channelout"
#define CHANNELOUT_NAME "Channel out"
#define PITCHMODES_ID "pitchmodes"
#define PITCHMODES_NAME "Pitch modes"
#define ACTIVEPROGRESSION_ID "activeprogression"
#define ACTIVEPROGRESSION_NAME "Active progression"
#define KEYS_ID "keys"
#define KEYS_NAME "Keys"
#define CHORDS_ID "chords"
#define CHORDS_NAME "Chords"
#define CHORDBUILDS_ID "chordbuilds"
#define CHORDBUILDS_NAME "ChordBuilds"
#define NOTESTOPLAY_ID "notestoplay"
#define NOTESTOPLAY_NAME "Notes to play"
#define SPLITS_ID "splits"
#define SPLITS_NAME "Splits"

#define MIDIINMESSAGETYPE_ID "midiInMessageType"
#define MIDIINMESSAGETYPE_NAME "midiInMessageType"
#define MIDIINCHANNEL_ID "midiInChannel"
#define MIDIINCHANNEL_NAME "midiInChannel"
#define MIDIINNUMBER_ID "midiInNumber"
#define MIDIINNUMBER_NAME "midiInNumber"
#define MIDIINMINVALUE_ID "midiInMinValue"
#define MIDIINMINVALUE_NAME "midiInMinValue"
#define MIDIINMAXVALUE_ID "midiInMaxValue"
#define MIDIINMAXVALUE_NAME "midiInMaxValue"

#define PROGRESSION "Progression"

const int defaultNoteOrder[MAX_NOTES] = {1,3,5,6,8,10,12,1,3,5,6,8};
const juce::StringArray keysArray({"C","C#/Db","D","D#/Eb","E","F","F#/Gb","G","G#/Ab","A","A#/Bb","B"});
const juce::StringArray chordsArray({"None","Power","Major","Minor","Dominant 7","Minor 7","Major 7","Diminished", "Octave up", "Octave down", "Custom"});
const juce::StringArray chordbuildsArray({"empty","0","0,7","0,4,7","0,3,7","0,4,7,10","0,3,7,10","0,4,7,11","0,3,6", "0,12", "0,-12"});
const juce::StringArray pitchModesArray({"Up" , "In Octave"});
const juce::StringArray progressionArray({"I" , "II", "III", "IV", "V", "VI"});
const juce::StringArray progressionKnobs({"I" , "II", "III", "IV", "V", "VI", "<", ">"});
const juce::StringArray channelInArray({"All","1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16"});
const juce::StringArray channelOutArray({"Same","1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16"});

const juce::StringArray midiMessageTypeArray({"None", "CC", "Note"});
const juce::StringArray midiValueArray({"0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
    "10", "11", "12", "13", "14", "15", "16", "17", "18", "29",
    "20", "21", "22", "23", "24", "25", "26", "27", "28", "29",
    "30", "31", "32", "33", "34", "35", "36", "37", "38", "39",
    "40", "41", "42", "43", "44", "45", "46", "47", "48", "49",
    "50", "51", "52", "53", "54", "55", "56", "57", "58", "59",
    "60", "61", "62", "63", "64", "65", "66", "67", "68", "69",
    "70", "71", "72", "73", "74", "75", "76", "77", "78", "79",
    "80", "81", "82", "83", "84", "85", "86", "87", "88", "89",
    "90", "91", "92", "93", "94", "95", "96", "97", "98", "99",
    "100", "101", "102", "103", "104", "105", "106", "107", "108", "109",
    "110", "111", "112", "113", "114", "115", "116", "117", "118", "119",
    "120", "121", "122", "123", "124", "125", "126", "127"});

const int DelayTimeMS = 80;

//==============================================================================
/**
 */
class RibbonToNotesAudioProcessor  : public juce::AudioProcessor
#if JucePlugin_Enable_ARA
, public juce::AudioProcessorARAExtension
#endif
{
public:
    //==============================================================================
    RibbonToNotesAudioProcessor();
    ~RibbonToNotesAudioProcessor() override;
    
    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    
#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
#endif
    
    void extracted(juce::MidiBuffer &midiMessages);
    
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    
    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;
    
    //==============================================================================
    const juce::String getName() const override;
    
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;
    
    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;
    
    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    //==============================================================================
    double startTime;
    int lastCCValue;
    int lastChannel;
    juce::MidiBuffer midiLearnBuffer;

    void PlayNextNote(juce::MidiBuffer &midiMessages);
    void AddNotesToPlayToBuffer(int ccval);
    void AddNotesToPlayToBuffer(int ccval, int channel, juce::MidiBuffer &midiMessages);
    void AddSentAllNotesOff(juce::MidiBuffer& processedMidi, int channel);
    void AddPreviousNotesSentNotesOff(juce::MidiBuffer& processedMidi, int channel);
    void AddSentNotesOn(juce::MidiBuffer& processedMidi, int selectedAlt, int selectedZone, int channel);

    //==============================================================================
    // Select progression
    //==============================================================================
    void SetControlByMidi(const juce::MidiMessage &midiMessage);

    void extracted(int &addOctaves, int alternative, int &key, int &maxNote, int octave, int zone);
    
    //==============================================================================
    // Utility functions
    //==============================================================================
//    void BuildChordsForAllProgressions();
//    void BuildChords(int alternative);
//    void GetNoteNumbersForChord(int addOctaves, int progression, int zone, int note);
    void UpdateParameter(int value, juce::String parameterID);
    bool HasChanged(int ccval);
    

    std::atomic<float>* midiCC = nullptr;
    std::atomic<float>* numberOfZones = nullptr;
    std::atomic<float>* noteVelocity = nullptr;
    std::atomic<float>* octaves = nullptr;
    std::atomic<float>* channelIn = nullptr;
    std::atomic<float>* channelOut = nullptr;
    std::atomic<float>* pitchMode = nullptr;
    std::atomic<float>* activeProgression = nullptr;
    std::atomic<float>* splitValues[MAX_SPLITS];
    std::atomic<float>* selectedKeys[MAX_PROGRESSIONS][MAX_ZONES];
    std::atomic<float>* selectedChord[MAX_PROGRESSIONS][MAX_ZONES];
    std::atomic<float>* chordNotes[MAX_PROGRESSIONS][MAX_ZONES][MAX_NOTES];

    std::atomic<float>* notesToPlay[MAX_PROGRESSIONS][MAX_ZONES][MAX_NOTES];

    AtomicMidiInfo midiInProgression[MAX_PROGRESSIONSKNOBS];
    AtomicMidiInfo midiInVelocity;

    
    juce::Array<int> notesPressed;
    int notePressedChannel[MAX_ZONES];
    int getActiveZone() const;
    int getActiveProgression() const;
    int activeProgressionKnob;

    Service::PresetManager& getPresetManager(){ return *presetManager; }

    juce::AudioProcessorValueTreeState apvts;

    
private:
    std::unique_ptr<Service::PresetManager> presetManager;
    int activeZone = 0;
    juce::MidiBuffer notesToPlayBuffer;
    int previousSampleNumber = 0;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RibbonToNotesAudioProcessor)
};
