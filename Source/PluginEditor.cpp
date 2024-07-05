/*
 ==============================================================================
 
 This file contains the basic framework code for a JUCE plugin editor.
 
 ==============================================================================
 */

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

//==============================================================================
RibbonToNotesAudioProcessorEditor::RibbonToNotesAudioProcessorEditor ( RibbonToNotesAudioProcessor& p)
: AudioProcessorEditor (&p)
, audioProcessor (p)
, presetPanel(p.getPresetManager())
, ribbonZeroZone(0)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    CreateRibbon();
    setSize (800, 400);
    CreateGui();
    RedistributeSplitRanges();
    AddListeners();
    startTimerHz(24);
}

RibbonToNotesAudioProcessorEditor::~RibbonToNotesAudioProcessorEditor()
{
    sldMidiCCAttachment = nullptr;
    sldNumberOfZonesAttachment = nullptr;
    sldVelocityAttachment = nullptr;
    sldOctaveAttachment = nullptr;
    cmbPitchModesAttachment = nullptr;
    for(int i=0;i<MAX_SPLITS;i++)
    {
        if(i<MAX_ZONES)
        {
            cmbKeysAttachment[i] = nullptr;
        }
        sldSplitValuesAttachment[i] = nullptr;
    }
    sldSplitExtraValuesAttachment = nullptr;
    for(int i=0;i<MAX_ZONES;i++)
    {
        for(int j=0;j<MAX_NOTES;j++)
        {
            sldChordNotesHelpAttachment[i][j]=nullptr;
        }
    }
}

void RibbonToNotesAudioProcessorEditor::CreateRibbon()
{
    for(int i=0;i<MAX_ZONES;i++)
    {
        ribbonKeyZone.add(new KeyZone(audioProcessor,i));
        ribbonKeyZone[i]->cmbKey.onChange = [this] {SyncComboBoxValues();};
    }
}

void RibbonToNotesAudioProcessorEditor::CreateGui()
{
    addAndMakeVisible(presetPanel);
    
    CreateDial(sldMidiCC);
    sldMidiCCAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,MIDICC_ID, sldMidiCC);
    
    
    lblMidiCC.setText("Midi CC", juce::dontSendNotification);
    lblMidiCC.attachToComponent(&sldMidiCC, false);
    lblMidiCC.setJustificationType(juce::Justification::centred);
    
    CreateDial(sldNumberOfZones);
    sldNumberOfZonesAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,NUMBEROFZONES_ID, sldNumberOfZones);
    lblNumberOfZones.setText("Zones", juce::dontSendNotification);
    lblNumberOfZones.attachToComponent(&sldNumberOfZones, false);
    lblNumberOfZones.setJustificationType(juce::Justification::centred);
    
    CreateDial(sldVelocity);
    sldVelocityAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, VELOCITY_ID, sldVelocity);
    
    lblVelocity.setText("Velocity", juce::dontSendNotification);
    lblVelocity.attachToComponent(&sldVelocity, false);
    lblVelocity.setJustificationType(juce::Justification::centred);
    
    CreateDial(sldOctave);
    sldOctaveAttachment=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, OCTAVES_ID, sldOctave);
    
    lblOctave.setText("Octave", juce::dontSendNotification);
    lblOctave.attachToComponent(&sldOctave, false);
    lblOctave.setJustificationType(juce::Justification::centred);
    
    addAndMakeVisible(cmbPitchModes);
    cmbPitchModes.addItemList(pitchModesArray, 1);
    cmbPitchModesAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (audioProcessor.apvts, PITCHMODES_ID, cmbPitchModes);
    cmbPitchModes.setEnabled(true);
    
    //put the rectangles underneath the controls, by adding them first.
    addAndMakeVisible(ribbonZeroZone);
    
    for(int i=0;i<MAX_ZONES;i++)
    {
        addAndMakeVisible(ribbonKeyZone[i]);
    }
    
    for(int i=0;i<MAX_SPLITS;i++)
    {
        CreateSlider(sldArSplitValues[i]);
        sldSplitValuesAttachment[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, SPLITS_ID + std::to_string(i), sldArSplitValues[i]);
        addAndMakeVisible(lblArSplitValues[i]);
    }
    
    CreateSlider(sldArSplitExtra);
    sldSplitExtraValuesAttachment= std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, SPLITEXTRA_ID, sldArSplitExtra);
    sldArSplitExtra.setValue(*audioProcessor.splitExtra);
    addAndMakeVisible(lblArSplitExtra);
    
    for(int i=0;i<MAX_ZONES;i++)
    {
        for(int j=0;j<MAX_NOTES;j++)
        {
            CreateSlider(sldChordNotesHelp[i][j]);
            sldChordNotesHelpAttachment[i][j] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, CHORDBUILDS_ID + std::to_string(i)+ "_" + std::to_string(j), sldChordNotesHelp[i][j]);
            sldChordNotesHelp[i][j].setValue(*audioProcessor.chordNotes[i][j], juce::sendNotificationSync);
        }
    }
}


void RibbonToNotesAudioProcessorEditor::RedistributeSplitRanges()
{
    int valueMax = (int) *audioProcessor.splitExtra;
    int valueMin = (int) *audioProcessor.splitValues[0];
    int zones = (int)(*audioProcessor.numberOfZones);
    int stepSize = (valueMax - valueMin)/(zones);
    
    int value = valueMin;
    sldArSplitValues[0].setRange(fmax(value - 0.25 * stepSize,0), fmin(value + 0.25 * stepSize,128), 1);
    sldArSplitValues[0].setValue(valueMin, juce::dontSendNotification);
    
    int i=1;
    for(;i < zones;i++)
    {
        value = (*audioProcessor.splitValues[0]) + i * stepSize;
        *audioProcessor.splitValues[i] = value;
        sldArSplitValues[i].setRange(fmax(value - 0.25 * stepSize,0), fmin(value + 0.25 * stepSize,128), 1);
        sldArSplitValues[i].setValue(value);
    }
    value = valueMax;
    sldArSplitExtra.setRange(fmax(value - 0.25 * stepSize,0), 128, 1);
}
void RibbonToNotesAudioProcessorEditor::CreateDial(juce::Slider& sld)
{
    addAndMakeVisible(sld);
    sld.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
}
void RibbonToNotesAudioProcessorEditor::CreateSlider(juce::Slider& sld)
{
    addAndMakeVisible(sld);
    sld.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
}

void RibbonToNotesAudioProcessorEditor::AddListeners()
{
    // add the listener to the slider
    sldMidiCC.addListener(this);
    sldNumberOfZones.addListener (this);
    sldVelocity.addListener(this);
    sldOctave.addListener(this);
    cmbPitchModes.addListener(this);
    for(int i=0;i<MAX_SPLITS;i++)
    {
        if(i<MAX_ZONES)
        {
            sldArNoteNumber[i].addListener(this);
        }
        sldArSplitValues[i].addListener(this);
    }
    for(int i=0;i<MAX_ZONES;i++)
    {
        for(int j=0;j<MAX_NOTES;j++)
        {
            sldChordNotesHelp[i][j].addListener(this);
        }
    }
    sldArSplitExtra.addListener(this);
}

//==============================================================================
void RibbonToNotesAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    
    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
}

void RibbonToNotesAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
    int activezones = (int) *audioProcessor.numberOfZones;
    
    auto topMargin = getHeight() * 0.02;
    auto sideMargin = topMargin;
    auto controlWidth = getWidth() / 6 - 2 * sideMargin;
    auto splitSldrWidth = controlWidth * 0.5;
    auto firstZoneWidth = 0.5 * splitSldrWidth + sideMargin;
    auto zoneWidth = (getWidth()-2 * sideMargin - splitSldrWidth) / (activezones);
    int textHeight = 3 * topMargin;// controlWidth/5;//(getHeight() - controlWidth - 2 * topMargin) * 0.05f;
    auto dialHeight = textHeight + controlWidth;
    auto vsliderHeight = 2*textHeight;
    
    int topPresetMenu =  getHeight() * 0.1f;
    int topGeneralControls = topPresetMenu + topMargin + textHeight;
    int topZone = topGeneralControls + dialHeight + topMargin;//topRowA - topMargin + row * topOffsetRow2;
    int zoneX = 0;
    int zoneHeight = getHeight() * 0.25;// 3*textHeight + 4 * topMargin;
    int topRowSplitSliders = topZone + zoneHeight;
    int splitCtrlX = 0;

    presetPanel.setBounds(getLocalBounds().removeFromTop(proportionOfHeight(0.1f)));
    
    sldMidiCC.setBounds(sideMargin, topGeneralControls, controlWidth, dialHeight);
    sldMidiCC.setTextBoxStyle(juce::Slider::TextBoxBelow, false, controlWidth, textHeight);
    sldNumberOfZones.setBounds(sideMargin + 1*(sideMargin + controlWidth), topGeneralControls, controlWidth, dialHeight);
    sldNumberOfZones.setTextBoxStyle(juce::Slider::TextBoxBelow, false, controlWidth, textHeight);
    sldVelocity.setBounds(sideMargin + 2*(sideMargin + controlWidth), topGeneralControls, controlWidth, dialHeight);
    sldVelocity.setTextBoxStyle(juce::Slider::TextBoxBelow, false, controlWidth, textHeight);
    sldOctave.setBounds(sideMargin + 3*(sideMargin + controlWidth), topGeneralControls, controlWidth, dialHeight);
    sldOctave.setTextBoxStyle(juce::Slider::TextBoxBelow, false, controlWidth, textHeight);
    cmbPitchModes.setBounds(sideMargin + 4*(sideMargin + controlWidth), topGeneralControls, controlWidth, textHeight);
    
    ribbonZeroZone.setBounds(zoneX + sideMargin, topZone, firstZoneWidth, zoneHeight);//0.5*zoneWidth); //Somehow making it smaller gives problems with the rendering.
    
    zoneX += firstZoneWidth + sideMargin;
    for(int i=0;i<MAX_ZONES;i++)
    {
        if(i<activezones)
        {
            ribbonKeyZone[i]->setVisible(true);
            sldArSplitValues[i].setVisible(true);
            
            ribbonKeyZone[i]->setEnabled(true);
            sldArSplitValues[i].setEnabled(true);
            
            
            ribbonKeyZone[i]->setBounds(zoneX, topZone, zoneWidth, zoneHeight);
            sldArSplitValues[i].setBounds(splitCtrlX + sideMargin, topRowSplitSliders, splitSldrWidth, vsliderHeight);
            sldArSplitValues[i].setTextBoxStyle(juce::Slider::TextBoxBelow, false, splitSldrWidth, textHeight);
            
            zoneX += zoneWidth;
            splitCtrlX += zoneWidth;
        }
        else
        {
            ribbonKeyZone[i]->setVisible(false);
            sldArSplitValues[i].setVisible(false);
        }
    }
    sldArSplitExtra.setBounds(splitCtrlX + sideMargin, topRowSplitSliders, splitSldrWidth, vsliderHeight);
    sldArSplitExtra.setTextBoxStyle(juce::Slider::TextBoxBelow, false, splitSldrWidth, textHeight);
    
}

void RibbonToNotesAudioProcessorEditor::SyncNotesAndSplits()
{
    audioProcessor.BuildChords();
}

void RibbonToNotesAudioProcessorEditor::SyncSliderValues()
{
    auto zones = sldNumberOfZones.getValue();
    
    if(zones != lastNumberOfZones)
    {
        lastNumberOfZones=zones;
        RedistributeSplitRanges();
        resized();
    }
    for(int i=0 ; i < MAX_ZONES; i++)
    {
        if(i<((int)(*audioProcessor.numberOfZones)))
        {
            *audioProcessor.splitValues[i] = sldArSplitValues[i].getValue();
        }
        else
        {
            *audioProcessor.splitValues[i] = 128;
        }
    }
    SyncNotesAndSplits();
}

void RibbonToNotesAudioProcessorEditor::SyncComboBoxValues()
{
    *audioProcessor.pitchMode = cmbPitchModes.getSelectedId();
    
    //determine if the first key has been transposed to another key.
    int transpose = ribbonKeyZone[0]->cmbKey.getSelectedId() - keyOrder[0];
    
    for(int i=0 ; i < MAX_ZONES; i++)
    {
        int key = ribbonKeyZone[i]->cmbKey.getSelectedId();
        *audioProcessor.selectedChord[i] = ribbonKeyZone[i]->cmbChord.getSelectedId();
        
        //in case the first key has changed, transpose other keys accordingly
        if(transpose != 0 && i > 0)
        {
            key = ((key + 11 + transpose) % 12)+1;
            keyOrder[i] = key;
            ribbonKeyZone[i]->cmbKey.setSelectedId(key, juce::dontSendNotification);
        }
        keyOrder[i] = key;
        *audioProcessor.selectedKeys[i] = key;
    }
    SyncNotesAndSplits();
}

void RibbonToNotesAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    *audioProcessor.numberOfZones = sldNumberOfZones.getValue();
    if(slider == &sldArSplitExtra || slider == &sldArSplitValues[0])
    {
        *audioProcessor.splitValues[0] = (int) sldArSplitValues[0].getValue();
        *audioProcessor.splitExtra = (int) sldArSplitExtra.getValue();
        *audioProcessor.splitValues[(int)(*audioProcessor.numberOfZones)] = (int) sldArSplitExtra.getValue();
        RedistributeSplitRanges();
    }
    
    SyncSliderValues();
}
void RibbonToNotesAudioProcessorEditor::comboBoxChanged(juce::ComboBox* combobox)
{
    SyncComboBoxValues();
}

void RibbonToNotesAudioProcessorEditor::setChordParameter(int key, int j, float value)
{
    //    juce::AudioProcessorParameterWithID* pParam = audioProcessor.apvts.getParameter(CHORDBUILDS_ID + std::to_string(key) + "_" + std::to_string(j));
    //    pParam->beginChangeGesture();
    //    pParam->setValue(value);
    //    pParam->endChangeGesture();
    sldChordNotesHelp[key][j].setValue(value, juce::sendNotificationSync);
}

//check if it is a valid integer
bool RibbonToNotesAudioProcessorEditor::is_validnotenumber(const juce::String& str)
{
    auto isvalid = str.containsOnly("0123456789-");
    if(isvalid)
    {
        int notenumber = str.getIntValue();
        if(notenumber < -103 || notenumber > 103)
        {
            isvalid = false;
        }
    }
    return isvalid;
}

void RibbonToNotesAudioProcessorEditor::ShowRibbonZone(int area)
{
    if(area == 0)
    {
        ribbonZeroZone.FillColourOn = true;
    }
    else
    {
        ribbonZeroZone.FillColourOn = false;
    }
    ribbonZeroZone.repaint();
    
    for(int i=0;i<MAX_ZONES;i++)
    {
        ribbonKeyZone[i]->FillColourOn = i == (area-1);
        ribbonKeyZone[i]->repaint();
    }
}


void RibbonToNotesAudioProcessorEditor::timerCallback()
{
    ShowRibbonZone(audioProcessor.getActiveZone());
}
