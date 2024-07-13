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
, ribbonZeroZone(0,0)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    CreateRibbon();
    setSize (800, 400);
    CreateGui();
    RedistributeSplitRanges(true);
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
    cmbActiveAlternativeAttachment = nullptr;
    for(int i=0;i<MAX_SPLITS;i++)
    {
        if(i<MAX_ZONES)
        {
            cmbKeysAttachment[i] = nullptr;
        }
        sldSplitValuesAttachment[i] = nullptr;
    }
    for(int alt = 0; alt < MAX_ALTERNATIVES; alt++)
    {
        for(int zone=0;zone<MAX_ZONES;zone++)
        {
            for(int j=0;j<MAX_NOTES;j++)
            {
                sldChordNotesHelpAttachment[alt][zone][j]=nullptr;
            }
        }
    }
}

//==============================================================================
// initialize ribbon key zones
//==============================================================================
void RibbonToNotesAudioProcessorEditor::CreateRibbon()
{
    for(int alt = 0;alt < MAX_ALTERNATIVES;alt++)
    {
        for(int zone=0;zone<MAX_ZONES;zone++)
        {
            ribbonKeyZone[alt].add(new KeyZone(audioProcessor, alt, zone));
            ribbonKeyZone[alt][zone]->cmbKey.onChange = [this] {SyncKeyAndChordModes();};
        }
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

    // pitchmode
    addAndMakeVisible(cmbActiveAlternative);
    cmbActiveAlternative.addItemList(activeAlternativeArray, 1);
    cmbActiveAlternativeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (audioProcessor.apvts, ACTIVEALTERNATIVE_ID, cmbActiveAlternative);
    cmbActiveAlternative.setEnabled(true);
    lblActiveAlternative.setText(ACTIVEALTERNATIVE_NAME, juce::dontSendNotification);
    lblActiveAlternative.attachToComponent(&cmbActiveAlternative, true);
    lblActiveAlternative.setJustificationType(juce::Justification::left);

    // ribbon key zones
    // the first zone only sents notes off, so it is just a ZoneVisual without controls
    addAndMakeVisible(ribbonZeroZone);
    
    // the other zones ar ribbon key Zones with their individual settings
    for(int alt=0;alt < MAX_ALTERNATIVES; alt++)
    {
        rootKey[alt] = (int) *audioProcessor.selectedKeys[alt][0];
        for(int zone=0;zone<MAX_ZONES;zone++)
        {
            addChildComponent(ribbonKeyZone[alt][zone]);// do not make it visible here.
        }
    }
    
    for(int i=0;i<MAX_SPLITS;i++)
    {
        CreateSlider(sldSplitValues[i]);
        sldSplitValuesAttachment[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, SPLITS_ID + std::to_string(i), sldSplitValues[i]);
        addAndMakeVisible(lblSplitValues[i]);
    }
    for(int alt=0;alt<MAX_ALTERNATIVES;alt++)
    {
        for(int zone=0;zone<MAX_ZONES;zone++)
        {
            for(int j=0;j<MAX_NOTES;j++)
            {
                CreateSlider(sldChordNotesHelp[alt][zone][j]);
                sldChordNotesHelpAttachment[alt][zone][j] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, CHORDBUILDS_ID + std::to_string(alt)+ "_" + std::to_string(zone)+ "_" + std::to_string(j), sldChordNotesHelp[alt][zone][j]);
                sldChordNotesHelp[alt][zone][j].setValue(*audioProcessor.chordNotes[alt][zone][j], juce::sendNotificationSync);
            }
        }
    }
}

//==============================================================================
// This functions shows the proper amount of key zones based on the selected
// number of zones/.
//==============================================================================
void RibbonToNotesAudioProcessorEditor::RedistributeSplitRanges(bool initSplitValues)
{
    int zones = (int)(*audioProcessor.numberOfZones);
    int lastSplit = zones - 1;
    int valueMin = (int) *audioProcessor.splitValues[0];
    int valueMax = fmax((int) *audioProcessor.splitValues[lastSplit], valueMin + zones);
    int stepSize = (valueMax - valueMin)/(zones-1);

    if(initSplitValues)
    {
        stepSize = (128 - valueMin)/(zones);
        valueMax = valueMin + (zones - 1) * stepSize;
    }
    
    int value = valueMin;
    sldSplitValues[0].setRange(fmax(value - 0.25 * stepSize,0), fmin(value + 0.25 * stepSize,128), 1);
    splitValuesSetFromCode = true;
    sldSplitValues[0].setValue(valueMin, juce::sendNotificationSync);
    
    int i=1;
    int prevValue = 0;
    for(;i < MAX_ZONES;i++)
    {
        if(i < zones)
        {
            value = i == lastSplit ? valueMax : valueMin + i * stepSize;
        }
        else
        {
            value =  128;
        }
        *audioProcessor.splitValues[i] = value;
        int min = fmin(fmax(value - 0.25 * stepSize, prevValue), value - 1);
        int max = fmin(fmax(value + 0.25 * stepSize, value + 1),128);
        sldSplitValues[i].setRange(min, max, 1);
        splitValuesSetFromCode = true;
        sldSplitValues[i].setValue(value, juce::sendNotificationSync);
        prevValue = value;
    }
    splitValuesSetFromCode = false;
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
    cmbActiveAlternative.addListener(this);

    for(int i=0;i<MAX_SPLITS;i++)
    {
        sldSplitValues[i].addListener(this);
    }
    for(int alt=0;alt<MAX_ALTERNATIVES;alt++)
    {
        for(int zone=0;zone<MAX_ZONES;zone++)
        {
            for(int j=0;j<MAX_NOTES;j++)
            {
                sldChordNotesHelp[alt][zone][j].addListener(this);
            }
        }
    }
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
    int activeAlternative = audioProcessor.getActiveAlternative();

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
    cmbActiveAlternative.setBounds(sideMargin + 5*(sideMargin + controlWidth), topGeneralControls + 3* (textHeight + topMargin), controlWidth, textHeight);

    ribbonZeroZone.setBounds(zoneX + sideMargin, topZone, firstZoneWidth, zoneHeight);//0.5*zoneWidth); //Somehow making it smaller gives problems with the rendering.
    
    for(int alt=0;alt<MAX_ALTERNATIVES;alt++)
    {
        int splitCtrlX = 0;
        zoneX = firstZoneWidth + sideMargin;

        for(int zone=0;zone<MAX_ZONES;zone++)
        {
            ribbonKeyZone[alt][zone]->setBounds(zoneX, topZone, zoneWidth, zoneHeight);
            ribbonKeyZone[alt][zone]->setVisible(alt == activeAlternative && zone < activezones);
            ribbonKeyZone[alt][zone]->setEnabled(alt == activeAlternative && zone < activezones);
            if(zone<activezones)
            {
                if(alt == 0) //sliders have to be set only once. There are no alternatives
                {
                    sldSplitValues[zone].setVisible(true);
                    sldSplitValues[zone].setEnabled(true);
                    sldSplitValues[zone].setBounds(splitCtrlX + sideMargin, topRowSplitSliders, splitSldrWidth, vsliderHeight);
                    sldSplitValues[zone].setTextBoxStyle(juce::Slider::TextBoxBelow, false, splitSldrWidth, textHeight);
                }
                zoneX += zoneWidth;
                splitCtrlX += zoneWidth;
            }
            else
            {
                if(alt == 0)
                {
                    sldSplitValues[zone].setVisible(false);
                }
            }
        }
    }
}



//==============================================================================
// gets the values of the sliders and makes necessary changes accordingly
//==============================================================================
void RibbonToNotesAudioProcessorEditor::SyncZoneSliderValues()
{
    // get the split value settings
    for(int i=0 ; i < MAX_ZONES; i++)
    {
        *audioProcessor.splitValues[i] = sldSplitValues[i].getValue();
    }
}

//==============================================================================
// Sync the selected settings for the key and chord modes
//==============================================================================
void RibbonToNotesAudioProcessorEditor::SyncKeyAndChordModes()
{
    // get the selected pitchMode
    int activeAlternative = (int) *audioProcessor.activeAlternative;
    
    
    //determine if the first key has been transposed to another key.
    int key = ribbonKeyZone[activeAlternative][0]->cmbKey.getSelectedId();
    int transpose = key - rootKey[activeAlternative];
    rootKey[activeAlternative] = key;
    
    // get the selected key and chord mode for each zone
    for(int zone=0 ; zone < MAX_ZONES; zone++)
    {
        // get the selected key
        key = ribbonKeyZone[activeAlternative][zone]->cmbKey.getSelectedId();
        
        // get the selected chord mode
        *audioProcessor.selectedChord[activeAlternative][zone] = ribbonKeyZone[activeAlternative][zone]->cmbChord.getSelectedId();
        
        //in case the first key has changed, transpose the key accordingly
        if(transpose != 0 && zone > 0)
        {
            key = ((key + 11 + transpose) % 12)+1;
            ribbonKeyZone[activeAlternative][zone]->cmbKey.setSelectedId(key, juce::dontSendNotification);
        }
        
        // set the selected key
        *audioProcessor.selectedKeys[activeAlternative][zone] = key;
    }
    
    // rebuild the chord for each zone based on key en chord mode setting
    audioProcessor.BuildChords(activeAlternative);
}

//==============================================================================
// Listeners for the controls
//==============================================================================
void RibbonToNotesAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    auto zones = (int) sldNumberOfZones.getValue();
    auto activeAlternative = audioProcessor.getActiveAlternative();
    int lastSplit = zones - 1;

    if(splitValuesSetFromCode == true)
    {
        splitValuesSetFromCode = false;
        return;
    }
    if(slider == &sldOctave)
    {
        *audioProcessor.octaves = sldOctave.getValue();
        audioProcessor.BuildChords(activeAlternative);
        return;
    }
        // check if number of zones has changed. If so, update the GUI.
    else if(zones != lastNumberOfZones)
    {
        *audioProcessor.numberOfZones = zones; //necessary if preset is loaded
        lastNumberOfZones=zones;
        RedistributeSplitRanges(true);
        resized();
    }
    else if(slider == &sldSplitValues[0] || slider == &sldSplitValues[zones-1])
    {
        *audioProcessor.splitValues[0] = (int) sldSplitValues[0].getValue();
        *audioProcessor.splitValues[lastSplit] = (int) sldSplitValues[lastSplit].getValue();
        *audioProcessor.splitValues[zones] = 128;
        RedistributeSplitRanges(false);
    }

    SyncZoneSliderValues();
}
//==============================================================================
void RibbonToNotesAudioProcessorEditor::comboBoxChanged(juce::ComboBox* combobox)
{
    SyncKeyAndChordModes();
    if(combobox == &cmbActiveAlternative)
    {
        ShowActiveAlternative();
    }
}

//==============================================================================
// Update functions for the visuals
//==============================================================================
void RibbonToNotesAudioProcessorEditor::ShowActiveAlternative()
{
    int activeAlternative = audioProcessor.getActiveAlternative();
    int activezones = (int) *audioProcessor.numberOfZones;

    for(int alt=0;alt<MAX_ALTERNATIVES;alt++)
    {
        for(int zone=0;zone<MAX_ZONES;zone++)
        {
            ribbonKeyZone[alt][zone]->setVisible(alt == activeAlternative && zone < activezones);
            ribbonKeyZone[alt][zone]->setEnabled(alt == activeAlternative && zone < activezones);        }
    }
}

// Colors the selected zone when a user presses the ribbon
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
    int activeAlternative = audioProcessor.getActiveAlternative();
    for(int zone=0;zone<MAX_ZONES;zone++)
    {
        ribbonKeyZone[activeAlternative][zone]->FillColourOn = zone == (area-1);
        ribbonKeyZone[activeAlternative][zone]->repaint();
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
