/*
  ==============================================================================

    MidiLearnInterface.cpp
    Created: 26 Dec 2024 12:04:52pm
    Author:  PJP

  ==============================================================================
*/

#include "MidiLearnInterface.h"



void MidiLearnInterface::CreateGui()
{
    auto alpha = 1;
    
    parent->addAndMakeVisible(selectButton);
    selectButton.setEnabled(true);
    selectButton.setAlpha(alpha);

    parent->addAndMakeVisible(cmbMidiInMessage);
    cmbMidiInMessage.addItemList(midiMessageTypeArray, 1);
    cmbMidiInMessage.setEnabled(true);
    cmbMidiInMessage.setAlpha(alpha);

    parent->addAndMakeVisible(cmbMidiInChannel);
    cmbMidiInChannel.addItemList(channelInArray, 1);
    cmbMidiInChannel.setEnabled(true);
    cmbMidiInChannel.setAlpha(alpha);

    parent->addAndMakeVisible(cmbMidiInNumber);
    cmbMidiInNumber.addItemList(midiValueArray, 1);
    cmbMidiInNumber.setEnabled(true);
    cmbMidiInNumber.setAlpha(alpha);

    parent->addAndMakeVisible(cmbMidiInValue);
    cmbMidiInValue.addItemList(midiValueArray, 1);
    cmbMidiInValue.setEnabled(true);
    cmbMidiInValue.setAlpha(alpha);
}

void MidiLearnInterface::AddMidiInterfaceListeners()
{
    selectButton.addListener(this);
    cmbMidiInMessage.addListener(this);
    cmbMidiInChannel.addListener(this);
    cmbMidiInNumber.addListener(this);
    cmbMidiInValue.addListener(this);
}
void MidiLearnInterface::RemoveMidiInterfaceListeners()
{
    selectButton.removeListener(this);
    cmbMidiInMessage.removeListener(this);
    cmbMidiInChannel.removeListener(this);
    cmbMidiInNumber.removeListener(this);
    cmbMidiInValue.removeListener(this);
}
void MidiLearnInterface::ResizeControls()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto height = parent->getHeight();
    auto width = parent->getWidth();
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
    
    selectButton.setEnabled(true);

    cmbMidiInMessage.setEnabled(true);
    cmbMidiInChannel.setEnabled(true);
    cmbMidiInNumber.setEnabled(true);
    cmbMidiInValue.setEnabled(true);
    
    selectButton.setBounds(0, 0, width, height);
    
    cmbMidiInMessage.setBounds(sideMargin, topRowB, controlWidth, textHeight);
    cmbMidiInChannel.setBounds(sideMargin, topRowC, controlWidth, textHeight);
    cmbMidiInNumber.setBounds(sideMargin, topRowD, controlWidth, textHeight);
    cmbMidiInValue.setBounds(sideMargin, topRowE, controlWidth, textHeight);
    
    ShowMidiSettings();
}

void MidiLearnInterface::ShowMidiSettings()
{
    selectButton.setColour(juce::TextButton::buttonColourId, GetMidiActiveColour(ColourOff));

    selectButton.setVisible(MidiSettingOn);
    cmbMidiInMessage.setVisible(MidiSettingOn);
    cmbMidiInChannel.setVisible(MidiSettingOn);
    cmbMidiInNumber.setVisible(MidiSettingOn);
    cmbMidiInValue.setVisible(MidiSettingOn);
}

bool MidiLearnInterface::MidiLearnNew()
{
    return cmbMidiInMessage.getSelectedId() == 1;
}


//==============================================================================
// listeners
//==============================================================================
void MidiLearnInterface::comboBoxChanged(juce::ComboBox* combobox)
{
    parent->repaint();
}
void MidiLearnInterface::buttonClicked(juce::Button* button)
{
}

