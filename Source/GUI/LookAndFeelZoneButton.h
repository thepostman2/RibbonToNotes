/*
  ==============================================================================

    LookAndFeelZoneButton.h
    Created: 18 Aug 2024 9:41:13pm
    Author:  PJP

  ==============================================================================
*/
#pragma once
#include <JuceHeader.h>

class LookAndFeelZoneButton : public juce::LookAndFeel_V4 
{
public:
    void drawButtonText (juce::Graphics& g, juce::TextButton& button,
                         [[maybe_unused]] bool shouldDrawButtonAsHighlighted, [[maybe_unused]] bool shouldDrawButtonAsDown)
    {
        using namespace juce;
        Font font (button.getHeight () * 0.6f);
        g.setFont (font);
        g.setColour (button.findColour (button.getToggleState () ?
            TextButton::textColourOnId    : TextButton::textColourOffId)
            .withMultipliedAlpha (button.isEnabled () ? 1.0f : 0.5f));

        const int yIndent = button.proportionOfHeight (0.1f);
        const int cornerSize = jmin (button.getHeight (), button.getWidth ()) / 2;

        const int leftIndent = cornerSize / (button.isConnectedOnLeft () ?
                  yIndent * 2 : yIndent);
        const int rightIndent = cornerSize / (button.isConnectedOnRight () ?
                  yIndent * 2 : yIndent);
        const int textWidth = button.getWidth () - leftIndent - rightIndent;

        if (textWidth > 0)
            g.drawFittedText (button.getButtonText (),
                leftIndent, yIndent, textWidth, button.getHeight () - yIndent * 2,
                Justification::centred, 2, 0.5f);
    }
};
