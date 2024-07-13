/*
 ==============================================================================
 
 This file contains the basic framework code for a JUCE plugin processor.
 
 ==============================================================================
 */

#pragma once

#include <JuceHeader.h>
#include "Service/PresetManager.h"

#define MAX_ALTERNATIVES 8
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
#define CHANNELIN_ID "channelin"
#define CHANNELIN_NAME "Channel in"
#define CHANNELOUT_ID "channelout"
#define CHANNELOUT_NAME "Channel out"
#define PITCHMODES_ID "pitchmodes"
#define PITCHMODES_NAME "Pitch modes"
#define ACTIVEALTERNATIVE_ID "activealternative"
#define ACTIVEALTERNATIVE_NAME "Active chord section"
#define KEYS_ID "keys"
#define KEYS_NAME "Keys"
#define CHORDS_ID "chords"
#define CHORDS_NAME "Chords"
#define CHORDBUILDS_ID "chordbuilds"
#define CHORDBUILDS_NAME "ChordBuilds"
#define SPLITS_ID "splits"
#define SPLITS_NAME "Splits"

const int defaultNoteOrder[MAX_NOTES] = {1,3,5,6,8,10,12,1,3,5,6,8};
const juce::StringArray keysArray({"C","C#/Db","D","D#/Eb","E","F","F#/Gb","G","G#/Ab","A","A#/Bb","B"});
const juce::StringArray chordsArray({"None","Power","Major","Minor","Dominant 7","Minor 7","Major 7","Diminished", "Octave up", "Octave down", "Custom"});
const juce::StringArray chordbuildsArray({"empty","1","1,8","1,5,8","1,4,8","1,5,8,11","1,4,8,11","1,5,8,12","1,4,7", "1,13", "1,-12"});
const juce::StringArray pitchModesArray({"Up" , "In Octave"});
const juce::StringArray activeAlternativeArray({"I" , "II", "III", "IV", "V", "VI", "VII", "VII"});
const juce::StringArray channelInArray({"All","1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16"});
const juce::StringArray channelOutArray({"Same","1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16"});

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

    void PlayNextNote(juce::MidiBuffer &midiMessages);
    void AddNotesToPlayToBuffer(int ccval, int channel, juce::MidiBuffer &midiMessages);
    void AddSentAllNotesOff(juce::MidiBuffer& processedMidi, int channel);
    void AddPreviousNotesSentNotesOff(juce::MidiBuffer& processedMidi, int channel);
    void AddSentNotesOn(juce::MidiBuffer& processedMidi, int selectedAlt, int selectedZone, int channel);

    void BuildChords(int alternative);
    void GetNoteNumbersForChord(int addOctaves, int alternative, int zone, int note);
    bool HasChanged(int ccval);

    std::atomic<float>* midiCC = nullptr;
    std::atomic<float>* numberOfZones = nullptr;
    std::atomic<float>* noteVelocity = nullptr;
    std::atomic<float>* octaves = nullptr;
    std::atomic<float>* channelIn = nullptr;
    std::atomic<float>* channelOut = nullptr;
    std::atomic<float>* pitchMode = nullptr;
    std::atomic<float>* activeAlternative = nullptr;
    std::atomic<float>* splitValues[MAX_SPLITS];
    std::atomic<float>* selectedKeys[MAX_ALTERNATIVES][MAX_ZONES];
    std::atomic<float>* selectedChord[MAX_ALTERNATIVES][MAX_ZONES];
    std::atomic<float>* chordNotes[MAX_ALTERNATIVES][MAX_ZONES][MAX_NOTES];

    int notesToPlay[MAX_ALTERNATIVES][MAX_ZONES][MAX_NOTES];

    juce::Array<int> notesPressed;
    int notePressedChannel[MAX_ZONES];
    int getActiveZone() const;
    int getActiveAlternative() const;

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
