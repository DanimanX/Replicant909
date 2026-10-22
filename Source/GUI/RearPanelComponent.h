#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../DSP/DrumEngine909.h"
#include "Theme909.h"

namespace Replicant909
{
    class RearPanelComponent : public juce::Component
    {
    public:
        explicit RearPanelComponent(DrumEngine909& engine);
        ~RearPanelComponent() override = default;

        void paint(juce::Graphics& g) override;
        void resized() override;

        void setActiveBusesMask(uint32_t mask) noexcept { activeMask = mask; repaint(); }

    private:
        DrumEngine909& engine;
        uint32_t activeMask = 0;

        void drawJack(juce::Graphics& g, float cx, float cy, const juce::String& label,
                      bool plugged, const Theme909& theme);

        void drawDinPort(juce::Graphics& g, float cx, float cy, const juce::String& label,
                         const Theme909& theme);

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RearPanelComponent)
    };
}
