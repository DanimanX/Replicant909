#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>

namespace Replicant909
{
    class TriggerHoldButton : public juce::TextButton
    {
    public:
        TriggerHoldButton() = default;
        ~TriggerHoldButton() override = default;

        std::function<void()> onTriggerDown;
        std::function<void()> onTriggerUp;

        void mouseDown(const juce::MouseEvent& e) override
        {
            juce::TextButton::mouseDown(e);
            if (onTriggerDown)
                onTriggerDown();
        }

        void mouseUp(const juce::MouseEvent& e) override
        {
            juce::TextButton::mouseUp(e);
            if (onTriggerUp)
                onTriggerUp();
        }

        void mouseExit(const juce::MouseEvent& e) override
        {
            juce::TextButton::mouseExit(e);
            if (!isMouseButtonDown())
            {
                if (onTriggerUp)
                    onTriggerUp();
            }
        }

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TriggerHoldButton)
    };
}
