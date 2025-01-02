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

    selectButton.addAndMakeVisible(cmbMidiInMessage);
    cmbMidiInMessage.addItemList(midiMessageTypeArray, 1);
    cmbMidiInMessage.setEnabled(true);
    cmbMidiInMessage.setAlpha(alpha);

    selectButton.addAndMakeVisible(cmbMidiInChannel);
    cmbMidiInChannel.addItemList(channelInArray, 1);
    cmbMidiInChannel.setEnabled(true);
    cmbMidiInChannel.setAlpha(alpha);

    selectButton.addAndMakeVisible(cmbMidiInNumber);
    cmbMidiInNumber.addItemList(midiValueArray, 1);
    cmbMidiInNumber.setEnabled(true);
    cmbMidiInNumber.setAlpha(alpha);
   
    selectButton.addAndMakeVisible(sldMidiInMaxValue);
    sldMidiInMaxValue.setSliderStyle(juce::Slider::LinearHorizontal);
    sldMidiInMaxValue.setEnabled(true);
    sldMidiInMaxValue.setAlpha(alpha);
    sldMidiInMaxValue.setColour(juce::Slider::textBoxBackgroundColourId, cmbMidiInNumber.findColour(juce::ComboBox::backgroundColourId));
    
    selectButton.addAndMakeVisible(sldMidiInMinValue);
    sldMidiInMinValue.setSliderStyle(juce::Slider::LinearHorizontal);
    sldMidiInMinValue.setEnabled(true);
    sldMidiInMinValue.setAlpha(alpha);
    sldMidiInMinValue.setColour(juce::Slider::textBoxBackgroundColourId, cmbMidiInNumber.findColour(juce::ComboBox::backgroundColourId));

}

void MidiLearnInterface::AddMidiInterfaceListeners()
{
    selectButton.addListener(this);
    cmbMidiInMessage.addListener(this);
    cmbMidiInChannel.addListener(this);
    cmbMidiInNumber.addListener(this);
    sldMidiInMinValue.addListener(this);
    sldMidiInMaxValue.addListener(this);
}
void MidiLearnInterface::RemoveMidiInterfaceListeners()
{
    selectButton.removeListener(this);
    cmbMidiInMessage.removeListener(this);
    cmbMidiInChannel.removeListener(this);
    cmbMidiInNumber.removeListener(this);
    sldMidiInMaxValue.removeListener(this);
    sldMidiInMinValue.removeListener(this);
}
void MidiLearnInterface::ResizeControls()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto height = parent->getHeight();
    auto width = parent->getWidth();
    int numberOfControls = 4;
    int controlMarginRatio = 5;
    auto topMargin = 0.8*height / (controlMarginRatio * numberOfControls + numberOfControls + 1);
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
    sldMidiInMinValue.setEnabled(true);
    sldMidiInMaxValue.setEnabled(true);

    selectButton.setBounds(0, 0, width, height);
    
    cmbMidiInMessage.setBounds(sideMargin, topRowB, controlWidth, textHeight);
    cmbMidiInChannel.setBounds(sideMargin, topRowC, controlWidth, textHeight);
    cmbMidiInNumber.setBounds(sideMargin, topRowD, controlWidth, textHeight);
    sldMidiInMinValue.setTextBoxStyle(juce::Slider::TextBoxAbove, 0, 0.5*controlWidth, textHeight);
    sldMidiInMinValue.setBounds(sideMargin, topRowE, 0.5*controlWidth, 2*textHeight);
    sldMidiInMaxValue.setTextBoxStyle(juce::Slider::TextBoxAbove, 0, 0.5*controlWidth, textHeight);
    sldMidiInMaxValue.setBounds(sideMargin + 0.5*controlWidth, topRowE, 0.5*controlWidth, 2*textHeight);

    ShowMidiSettings();
}

void MidiLearnInterface::ShowMidiSettings()
{
    selectButton.setColour(juce::TextButton::buttonColourId, GetMidiActiveColour(ColourOff));
    
    selectButton.setVisible(MidiSettingOn);
    cmbMidiInMessage.setVisible(MidiSettingOn);
    cmbMidiInChannel.setVisible(MidiSettingOn);
    cmbMidiInNumber.setVisible(MidiSettingOn);
    sldMidiInMaxValue.setVisible(MidiSettingOn);
    sldMidiInMinValue.setVisible(MidiSettingOn);
}

bool MidiLearnInterface::MidiLearnNew()
{
    return cmbMidiInMessage.getSelectedId() == 1 || (MidiLearnOn && selected);
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
    selected = true;
    onMidiLearnChanges.sendChangeMessage();
}

void MidiLearnInterface::sliderValueChanged(juce::Slider* slider)
{
}
