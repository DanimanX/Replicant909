#include "PluginEditor.h"

namespace Replicant909
{
    PluginEditor::PluginEditor(PluginProcessor& p)
        : AudioProcessorEditor(&p),
          processor(p),
          headerComponent(p.getAPVTS(), p.getEngine(), p.getSequencer()),
          voiceStripRack(p.getAPVTS(), p.getEngine()),
          stepSequencer(p.getAPVTS(), p.getSequencer()),
          rearPanel(p.getEngine())
    {
        setLookAndFeel(&lnf);

        addAndMakeVisible(headerComponent);
        addAndMakeVisible(voiceStripRack);
        addAndMakeVisible(stepSequencer);
        addChildComponent(rearPanel);

        rearPanelToggleBtn.setButtonText("REAR PANEL");
        rearPanelToggleBtn.onClick = [this]() {
            showRearPanel = !showRearPanel;
            rearPanel.setVisible(showRearPanel);
            rearPanelToggleBtn.setButtonText(showRearPanel ? "FRONT PANEL" : "REAR PANEL");
            resized();
        };
        addAndMakeVisible(rearPanelToggleBtn);

        // Standard 909 desktop footprint: 1040 x 620
        setSize(1040, 620);
        setResizable(false, false);

        startTimerHz(30); // 30 FPS telemetry updates
    }

    PluginEditor::~PluginEditor()
    {
        setLookAndFeel(nullptr);
    }

    void PluginEditor::timerCallback()
    {
        voiceStripRack.updateMeters();
        stepSequencer.updatePlayhead();
        if (showRearPanel)
            rearPanel.setActiveBusesMask(processor.getActiveBusesMask());
    }

    void PluginEditor::paint(juce::Graphics& g)
    {
        const auto& theme = ThemeManager::getInstance().getCurrentTheme();
        auto bounds = getLocalBounds().toFloat();

        // 1. Side cheeks (left & right chassis end-caps)
        float cheekWidth = 14.0f;
        auto leftCheek = bounds.removeFromLeft(cheekWidth);
        auto rightCheek = bounds.removeFromRight(cheekWidth);

        // Left Cheek
        juce::ColourGradient leftGrad(theme.cheekColor, leftCheek.getX(), leftCheek.getY(),
                                      theme.cheekShadow, leftCheek.getRight(), leftCheek.getY(), false);
        g.setGradientFill(leftGrad);
        g.fillRect(leftCheek);

        // Right Cheek
        juce::ColourGradient rightGrad(theme.cheekShadow, rightCheek.getX(), rightCheek.getY(),
                                       theme.cheekColor, rightCheek.getRight(), rightCheek.getY(), false);
        g.setGradientFill(rightGrad);
        g.fillRect(rightCheek);

        // 2. Main chassis gradient
        juce::ColourGradient chassisGrad(theme.chassisTop, bounds.getX(), bounds.getY(),
                                         theme.chassisBottom, bounds.getX(), bounds.getBottom(), false);
        g.setGradientFill(chassisGrad);
        g.fillRect(bounds);

        // 3. Chassis bevel highlight
        g.setColour(theme.chassisBevel);
        g.drawRect(bounds, 1.0f);

        // 4. Subtle brushed metal / carbon fiber line texture
        g.setColour(juce::Colours::white.withAlpha(theme.id == SKIN_RETRO_CLASSIC ? 0.035f : 0.015f));
        for (int y = 0; y < getHeight(); y += 4)
            g.drawHorizontalLine(y, bounds.getX(), bounds.getRight());
    }

    void PluginEditor::resized()
    {
        auto bounds = getLocalBounds();

        // Leave room for side cheeks (14px on left and right)
        bounds.reduce(14, 0);

        // Top Header: 72px
        headerComponent.setBounds(bounds.removeFromTop(72));

        // Rear panel toggle button in bottom right corner
        rearPanelToggleBtn.setBounds(bounds.getRight() - 95, bounds.getBottom() - 24, 85, 20);

        if (showRearPanel)
        {
            rearPanel.setBounds(bounds.removeFromBottom(120));
        }

        // Bottom Sequencer: 114px
        stepSequencer.setBounds(bounds.removeFromBottom(114));

        // Middle Drum Voice Strip Rack takes remaining vertical space (~434px)
        voiceStripRack.setBounds(bounds);
    }
}
