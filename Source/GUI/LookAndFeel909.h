#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "Theme909.h"
#include "../DSP/Pattern909.h"

namespace Replicant909
{
    class LookAndFeel909 : public juce::LookAndFeel_V4
    {
    public:
        LookAndFeel909();
        ~LookAndFeel909() override = default;

        void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                              float sliderPosProportional, float rotaryStartAngle,
                              float rotaryEndAngle, juce::Slider& slider) override;

        void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                  const juce::Colour& backgroundColour,
                                  bool shouldDrawButtonAsHighlighted,
                                  bool shouldDrawButtonAsDown) override;

        void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                              bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown) override;

        // Custom 7-segment digital LED number renderer (3 digits, e.g. 128 BPM)
        static void drawSevenSegmentDisplay(juce::Graphics& g, const juce::Rectangle<float>& bounds,
                                            int number, const Theme909& theme);

        // Custom LED indicator dot renderer
        static void drawLed(juce::Graphics& g, float cx, float cy, float radius,
                            bool lit, const juce::Colour& litColour, const juce::Colour& offColour);

        // Custom step button LED renderer (dim amber, bright red, flam yellow, or off)
        static void drawStepLed(juce::Graphics& g, const juce::Rectangle<float>& bounds,
                                uint8_t stepState, bool isCurrentPlayhead, const Theme909& theme);

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LookAndFeel909)
    };
}
