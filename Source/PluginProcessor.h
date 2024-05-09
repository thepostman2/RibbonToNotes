/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>


#define MAX_NOTES 12
#define MAX_SPLITS MAX_NOTES+1
#define DEFAULT_NUMBEROFZONES 6

const int defaultNoteOrder[MAX_NOTES] = {1,3,5,6,8,10,12,1,3,5,6,8};

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
    const int ADDTIME =1;
    int AddSentAllNotesOff(juce::MidiBuffer& processedMidi, int exceptNote, int time);
    int AddPreviousNotesSentNotesOff(juce::MidiBuffer& processedMidi, int exceptNote, int time);
    int AddSentNotesOn(juce::MidiBuffer& processedMidi, int selectedZone, int time);

    std::atomic<float>* midiCC = nullptr;
    std::atomic<float>* numberOfZones = nullptr;
    std::atomic<float>* noteVelocity = nullptr;
    std::atomic<float>* octaves = nullptr;
    std::atomic<float>* splitExtra = nullptr;
    std::atomic<float>* splitValues[MAX_SPLITS];
    std::atomic<float>* noteValues[MAX_NOTES];
    std::atomic<float>* chordValues[MAX_NOTES];
    std::atomic<float>* chordBuilds[MAX_NOTES][MAX_NOTES];
    int notePressedChannel[MAX_NOTES];

private:
    juce::AudioProcessorValueTreeState parameters;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RibbonToNotesAudioProcessor)
};
