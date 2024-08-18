/*
  ==============================================================================

    SelectionKnob.cpp
    Created: 13 Jul 2024 3:01:45pm
    Author:  PJP

  ==============================================================================
*/

#include "SelectionKnob.h"

SelectionKnob::SelectionKnob (RibbonToNotesAudioProcessor& p, int progressionid)
: ZoneVisual(progressionid, 0),
audioProcessor (p)
{
    CreateGui();
    AddListeners();
}

SelectionKnob::~SelectionKnob()
{
    RemoveListeners();
    cmbMidiMessageAttachment = nullptr;
    cmbMidiInChannelAttachment = nullptr;
    cmbMidiInNumberAttachment = nullptr;
    cmbMidiInValueAttachment = nullptr;
}

void SelectionKnob::CreateGui()
{
    auto alpha = 0.5;
    
    setButtonText(progressionKnobs[PROGRESSION_ID]);
    
    addAndMakeVisible(cmbMidiInMessage);
    cmbMidiInMessage.addItemList(midiMessageTypeArray, 1);
    cmbMidiMessageAttachment= std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (audioProcessor.apvts, MIDIINSKALTMESSAGETYPE_ID + std::to_string(PROGRESSION_ID) , cmbMidiInMessage);
    cmbMidiInMessage.setEnabled(true);
    cmbMidiInMessage.setAlpha(alpha);

    addAndMakeVisible(cmbMidiInChannel);
    cmbMidiInChannel.addItemList(channelInArray, 1);
    cmbMidiInChannelAttachment= std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (audioProcessor.apvts, MIDIINSKALTCHANNEL_ID + std::to_string(PROGRESSION_ID) , cmbMidiInChannel);
    cmbMidiInChannel.setEnabled(true);
    cmbMidiInChannel.setAlpha(alpha);

    addAndMakeVisible(cmbMidiInNumber);
    cmbMidiInNumber.addItemList(midiValueArray, 1);
    cmbMidiInNumberAttachment= std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (audioProcessor.apvts, MIDIINSKALTNUMBER_ID + std::to_string(PROGRESSION_ID) , cmbMidiInNumber);
    cmbMidiInNumber.setEnabled(true);
    cmbMidiInNumber.setAlpha(alpha);

    addAndMakeVisible(cmbMidiInValue);
    cmbMidiInValue.addItemList(midiValueArray, 1);
    cmbMidiInValueAttachment= std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (audioProcessor.apvts, MIDIINSKALTVALUE_ID + std::to_string(PROGRESSION_ID) , cmbMidiInValue);
    cmbMidiInValue.setEnabled(true);
    cmbMidiInValue.setAlpha(alpha);
}

void SelectionKnob::AddListeners()
{
    cmbMidiInMessage.addListener(this);
    cmbMidiInChannel.addListener(this);
    cmbMidiInNumber.addListener(this);
    cmbMidiInValue.addListener(this);
}
void SelectionKnob::RemoveListeners()
{
    cmbMidiInMessage.removeListener(this);
    cmbMidiInChannel.removeListener(this);
    cmbMidiInNumber.removeListener(this);
    cmbMidiInValue.removeListener(this);
}
void SelectionKnob::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto height = getHeight();
    auto width = getWidth();
    int numberOfControls = 4;
    int controlMarginRatio = 3;
    auto topMargin = height / (controlMarginRatio * numberOfControls + numberOfControls + 1);
    auto sideMargin = topMargin;
    auto controlWidth = width - 2 * sideMargin ;
    int textHeight = controlMarginRatio * topMargin;
    
    int topRowA = topMargin;
    int topRowB = topRowA;
    int topRowC = topRowB + topMargin + textHeight;
    int topRowD = topRowC + topMargin + textHeight;
    int topRowE = topRowD + topMargin + textHeight;

    cmbMidiInMessage.setEnabled(true);
    cmbMidiInChannel.setEnabled(true);
    cmbMidiInNumber.setEnabled(true);
    cmbMidiInValue.setEnabled(true);
    
    cmbMidiInMessage.setBounds(sideMargin, topRowB, controlWidth, textHeight);
    cmbMidiInChannel.setBounds(sideMargin, topRowC, controlWidth, textHeight);
    cmbMidiInNumber.setBounds(sideMargin, topRowD, controlWidth, textHeight);
    cmbMidiInValue.setBounds(sideMargin, topRowE, controlWidth, textHeight);
}

//==============================================================================
// listeners
//==============================================================================
void SelectionKnob::comboBoxChanged(juce::ComboBox* combobox)
{
}

void SelectionKnob::buttonClicked(juce::Button* button)
{
    FillColourOn = true;
    repaint();
}
