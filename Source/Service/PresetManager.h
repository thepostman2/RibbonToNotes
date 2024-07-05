/*
 ==============================================================================
 
 PresetManager.h
 Created: 10 May 2024 10:56:15pm
 Author:  PJP
 
 ==============================================================================
 */

#pragma once

#include <JuceHeader.h>

namespace Service
{
using namespace juce;

class PresetManager : ValueTree::Listener
{
public:
    static const File defaultDirectory;
    static const String extension;
    static const String presetNameProperty;
    
    PresetManager(AudioProcessorValueTreeState&);
    
    void savePreset(const String& presetName);
    void deletePreset(const String& presetName);
    void loadPreset(const String& presetName);
    int loadNextPreset();
    int loadPreviousPreset();
    StringArray getAllPresets() const;
    String getCurrentPreset() const;

private:
    void valueTreeRedirected(juce::ValueTree& treeWhichHasChanged) override;
    AudioProcessorValueTreeState& valueTreeState;
    Value currentPreset; //Value object of juce
};
}
