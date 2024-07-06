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

//==============================================================================
// destructor. This is needed to make sure the attachments are not pointing
// to any destructed control.
//==============================================================================
RibbonToNotesAudioProcessorEditor::~RibbonToNotesAudioProcessorEditor()
{
    // clean up attachments first to prevent errors when the object is deconstructed before the attachment is deconstructed.
    sldMidiCCAttachment = nullptr;
    sldNumberOfZonesAttachment = nullptr;
    sldVelocityAttachment = nullptr;
    sldOctaveAttachment = nullptr;
    cmbChannelInAttachment = nullptr;
    cmbChannelOutAttachment = nullptr;
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

//==============================================================================
// initialize ribbon key zones
//==============================================================================
void RibbonToNotesAudioProcessorEditor::CreateRibbon()
{
    for(int i=0;i<MAX_ZONES;i++)
    {
        ribbonKeyZone.add(new KeyZone(audioProcessor,i));
        ribbonKeyZone[i]->cmbKey.onChange = [this] {SyncKeyAndChordModes();};
    }
}

//==============================================================================
// this function only makes the controls, but does not position them.
// for the position of the controls, see resized function
//==============================================================================
void RibbonToNotesAudioProcessorEditor::CreateGui()
{
    // presets
    addAndMakeVisible(presetPanel);
    
    // main dials
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

    addAndMakeVisible(cmbChannelIn);
    cmbChannelIn.addItemList(channelInArray, 1);
    cmbChannelInAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.apvts, CHANNELIN_ID, cmbChannelIn);
    lblChannelIn.setText(CHANNELIN_NAME, juce::dontSendNotification);
    lblChannelIn.attachToComponent(&cmbChannelIn, true);
    lblChannelIn.setJustificationType(juce::Justification::left);

    addAndMakeVisible(cmbChannelOut);
    cmbChannelOut.addItemList(channelOutArray, 1);
    cmbChannelOutAttachment=std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (audioProcessor.apvts, CHANNELOUT_ID, cmbChannelOut);
    lblChannelOut.setText(CHANNELOUT_NAME, juce::dontSendNotification);
    lblChannelOut.attachToComponent(&cmbChannelOut, true);
    lblChannelOut.setJustificationType(juce::Justification::left);

    // pitchmode
    addAndMakeVisible(cmbPitchModes);
    cmbPitchModes.addItemList(pitchModesArray, 1);
    cmbPitchModesAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (audioProcessor.apvts, PITCHMODES_ID, cmbPitchModes);
    cmbPitchModes.setEnabled(true);
    lblPitchModes.setText(PITCHMODES_NAME, juce::dontSendNotification);
    lblPitchModes.attachToComponent(&cmbPitchModes, true);
    lblPitchModes.setJustificationType(juce::Justification::left);

    // ribbon key zones
    // the first zone only sents notes off, so it is just a ZoneVisual without controls
    addAndMakeVisible(ribbonZeroZone);
    
    // the other zones ar ribbon key Zones with their individual settings
    for(int i=0;i<MAX_ZONES;i++)
    {
        addAndMakeVisible(ribbonKeyZone[i]);
    }
    
    for(int i=0;i<MAX_SPLITS;i++)
    {
        CreateSlider(sldSplitValues[i]);
        sldSplitValuesAttachment[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, SPLITS_ID + std::to_string(i), sldSplitValues[i]);
        addAndMakeVisible(lblSplitValues[i]);
    }
    
    CreateSlider(sldSplitEnd);
    sldSplitExtraValuesAttachment= std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, SPLITEXTRA_ID, sldSplitEnd);
    sldSplitEnd.setValue(*audioProcessor.splitExtra);
    addAndMakeVisible(lblSplitEnd);
    
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

//==============================================================================
// This functions shows the proper amount of key zones based on the selected
// number of zones/.
//==============================================================================
void RibbonToNotesAudioProcessorEditor::RedistributeSplitRanges()
{
    int valueMax = (int) *audioProcessor.splitExtra;
    int valueMin = (int) *audioProcessor.splitValues[0];
    int zones = (int)(*audioProcessor.numberOfZones);
    int stepSize = (valueMax - valueMin)/(zones);
    
    int value = valueMin;
    sldSplitValues[0].setRange(fmax(value - 0.25 * stepSize,0), fmin(value + 0.25 * stepSize,128), 1);
    sldSplitValues[0].setValue(valueMin, juce::dontSendNotification);
    
    int i=1;
    for(;i < zones;i++)
    {
        value = (*audioProcessor.splitValues[0]) + i * stepSize;
        *audioProcessor.splitValues[i] = value;
        sldSplitValues[i].setRange(fmax(value - 0.25 * stepSize,0), fmin(value + 0.25 * stepSize,128), 1);
        sldSplitValues[i].setValue(value);
    }
    value = valueMax;
    sldSplitEnd.setRange(fmax(value - 0.25 * stepSize,0), 128, 1);
}

//==============================================================================
// Functions to create dials and sliders with the same style.
//==============================================================================
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

//==============================================================================
// Add all the listeners to the sliders and dials
//==============================================================================
void RibbonToNotesAudioProcessorEditor::AddListeners()
{
    // add the listener to the slider
    sldMidiCC.addListener(this);
    sldNumberOfZones.addListener (this);
    sldVelocity.addListener(this);
    sldOctave.addListener(this);
    cmbChannelIn.addListener(this);
    cmbChannelOut.addListener(this);
    cmbPitchModes.addListener(this);
    for(int i=0;i<MAX_SPLITS;i++)
    {
        sldSplitValues[i].addListener(this);
    }
    for(int i=0;i<MAX_ZONES;i++)
    {
        for(int j=0;j<MAX_NOTES;j++)
        {
            sldChordNotesHelp[i][j].addListener(this);
        }
    }
    sldSplitEnd.addListener(this);
}

//==============================================================================
// set the font size and color.
//==============================================================================
void RibbonToNotesAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    
    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
}

//==============================================================================
// Position and size all controls based on the size of the GUI.
//==============================================================================
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
    cmbChannelIn.setBounds(sideMargin + 5*(sideMargin + controlWidth), topGeneralControls , controlWidth,  textHeight);
    cmbChannelOut.setBounds(sideMargin + 5*(sideMargin + controlWidth), topGeneralControls + (textHeight + topMargin), controlWidth,  textHeight);
    cmbPitchModes.setBounds(sideMargin + 5*(sideMargin + controlWidth), topGeneralControls + 2* (textHeight + topMargin), controlWidth, textHeight);
    
    ribbonZeroZone.setBounds(zoneX + sideMargin, topZone, firstZoneWidth, zoneHeight);//0.5*zoneWidth); //Somehow making it smaller gives problems with the rendering.
    
    zoneX += firstZoneWidth + sideMargin;
    for(int i=0;i<MAX_ZONES;i++)
    {
        if(i<activezones)
        {
            ribbonKeyZone[i]->setVisible(true);
            sldSplitValues[i].setVisible(true);
            
            ribbonKeyZone[i]->setEnabled(true);
            sldSplitValues[i].setEnabled(true);
            
            
            ribbonKeyZone[i]->setBounds(zoneX, topZone, zoneWidth, zoneHeight);
            sldSplitValues[i].setBounds(splitCtrlX + sideMargin, topRowSplitSliders, splitSldrWidth, vsliderHeight);
            sldSplitValues[i].setTextBoxStyle(juce::Slider::TextBoxBelow, false, splitSldrWidth, textHeight);
            
            zoneX += zoneWidth;
            splitCtrlX += zoneWidth;
        }
        else
        {
            ribbonKeyZone[i]->setVisible(false);
            sldSplitValues[i].setVisible(false);
        }
    }
    sldSplitEnd.setBounds(splitCtrlX + sideMargin, topRowSplitSliders, splitSldrWidth, vsliderHeight);
    sldSplitEnd.setTextBoxStyle(juce::Slider::TextBoxBelow, false, splitSldrWidth, textHeight);
    
}

//==============================================================================
// gets the values of the sliders and makes necessary changes accordingly
//==============================================================================
void RibbonToNotesAudioProcessorEditor::SyncZoneSliderValues()
{
    // check if number of zones has changed. If so, update the GUI.
    auto zones = sldNumberOfZones.getValue();
    if(zones != lastNumberOfZones)
    {
        lastNumberOfZones=zones;
        RedistributeSplitRanges();
        resized();
    }
    
    // get the split value settings
    for(int i=0 ; i < MAX_ZONES; i++)
    {
        if(i<((int)(*audioProcessor.numberOfZones)))
        {
            *audioProcessor.splitValues[i] = sldSplitValues[i].getValue();
        }
        else
        {
            *audioProcessor.splitValues[i] = 128;
        }
    }
}

//==============================================================================
// Sync the selected settings for the key and chord modes
//==============================================================================
void RibbonToNotesAudioProcessorEditor::SyncKeyAndChordModes()
{
    // get the selected pitchMode
    *audioProcessor.pitchMode = cmbPitchModes.getSelectedId();
    
    //determine if the first key has been transposed to another key.
    int transpose = ribbonKeyZone[0]->cmbKey.getSelectedId() - keyOrder[0];
    
    // get the selected key and chord mode for each zone
    for(int i=0 ; i < MAX_ZONES; i++)
    {
        // get the selected key
        int key = ribbonKeyZone[i]->cmbKey.getSelectedId();
        
        // get the selected chord mode
        *audioProcessor.selectedChord[i] = ribbonKeyZone[i]->cmbChord.getSelectedId();
        
        //in case the first key has changed, transpose the key accordingly
        if(transpose != 0 && i > 0)
        {
            key = ((key + 11 + transpose) % 12)+1;
            keyOrder[i] = key;
            ribbonKeyZone[i]->cmbKey.setSelectedId(key, juce::dontSendNotification);
        }
        
        // set the selected key
        keyOrder[i] = key;
        *audioProcessor.selectedKeys[i] = key;
    }
    
    // rebuild the chord for each zone based on key en chord mode setting
    audioProcessor.BuildChords();
}

//==============================================================================
// Listeners for the controls
//==============================================================================
void RibbonToNotesAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    if(slider == &sldOctave)
    {
        *audioProcessor.octaves = sldOctave.getValue();
        audioProcessor.BuildChords();
    }
    
    *audioProcessor.numberOfZones = sldNumberOfZones.getValue();
    if(slider == &sldSplitEnd || slider == &sldSplitValues[0])
    {
        *audioProcessor.splitValues[0] = (int) sldSplitValues[0].getValue();
        *audioProcessor.splitExtra = (int) sldSplitEnd.getValue();
        *audioProcessor.splitValues[(int)(*audioProcessor.numberOfZones)] = (int) sldSplitEnd.getValue();
        RedistributeSplitRanges();
    }

    SyncZoneSliderValues();
}
//==============================================================================
void RibbonToNotesAudioProcessorEditor::comboBoxChanged(juce::ComboBox* combobox)
{
    SyncKeyAndChordModes();
}

//==============================================================================
// Colors the selected zone when a user presses the ribbon
//==============================================================================
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

//==============================================================================
// Timer call back function. In this case only used to update the GUI when
// the user has pressed somewhere on the ribbon.
//==============================================================================
void RibbonToNotesAudioProcessorEditor::timerCallback()
{
    ShowRibbonZone(audioProcessor.getActiveZone());
}
