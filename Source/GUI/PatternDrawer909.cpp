#include "PatternDrawer909.h"
#include "LookAndFeel909.h"
#include "../Presets/PresetIO909.h"
#include <algorithm>

namespace Replicant909
{
    PatternDrawer909::PatternDrawer909(juce::AudioProcessorValueTreeState& state,
                                       DrumEngine909& eng,
                                       GrooveSequencer909& seq)
        : apvts(state), engine(eng), sequencer(seq)
    {
        // 1. Genre Selector
        genreSelector.addItem("Classic House (4x4 & Offbeat)", 1);
        genreSelector.addItem("Detroit Driving Techno", 2);
        genreSelector.addItem("Acid Jack & Warehouse", 3);
        genreSelector.addItem("Hard Techno & Rave", 4);
        genreSelector.addItem("Electro Funk Breakbeat", 5);
        genreSelector.addItem("Minimal Click & Microhouse", 6);
        genreSelector.addItem("Algorithmic Cyber Chaos", 7);
        genreSelector.setSelectedId(1, juce::dontSendNotification);
        addAndMakeVisible(genreSelector);

        // 2. Generator Density Sliders
        auto setupSlider = [this](juce::Slider& s, float defaultVal) {
            s.setSliderStyle(juce::Slider::LinearHorizontal);
            s.setTextBoxStyle(juce::Slider::TextBoxRight, false, 36, 16);
            s.setRange(0.0, 1.0, 0.01);
            s.setValue(defaultVal);
            addAndMakeVisible(s);
        };

        setupSlider(kickDensitySlider, 0.85f);
        setupSlider(snareDensitySlider, 0.70f);
        setupSlider(hatDensitySlider, 0.80f);
        setupSlider(percDensitySlider, 0.50f);
        setupSlider(accentProbSlider, 0.40f);
        setupSlider(flamProbSlider, 0.20f);

        // 3. Generator Action Buttons
        generateButton.setButtonText("🎲 GENERATE");
        generateButton.onClick = [this]() {
            int genreIdx = genreSelector.getSelectedId() - 1;
            generateAlgorithmicPattern(genreIdx);
            repaint();
        };
        addAndMakeVisible(generateButton);

        mutateButton.setButtonText("🧬 MUTATE");
        mutateButton.onClick = [this]() {
            mutatePattern(0.20f);
            repaint();
        };
        addAndMakeVisible(mutateButton);

        clearButton.setButtonText("↺ CLEAR ALL");
        clearButton.onClick = [this]() {
            sequencer.getActivePattern().clear();
            repaint();
        };
        addAndMakeVisible(clearButton);

        // 4. Quick Pattern Tools
        fill4x4KickBtn.setButtonText("4x4 BD");
        fill4x4KickBtn.onClick = [this]() {
            auto& pat = sequencer.getActivePattern();
            for (int s = 0; s < 16; ++s)
                pat.setStep(BD, s, (s % 4 == 0) ? STEP_ACCENT : STEP_OFF);
            repaint();
        };
        addAndMakeVisible(fill4x4KickBtn);

        fillOffbeatHatBtn.setButtonText("Offbeat OH");
        fillOffbeatHatBtn.onClick = [this]() {
            auto& pat = sequencer.getActivePattern();
            for (int s = 0; s < 16; ++s)
                pat.setStep(OH, s, (s % 4 == 2) ? STEP_ACCENT : STEP_OFF);
            repaint();
        };
        addAndMakeVisible(fillOffbeatHatBtn);

        fillBackbeatClapBtn.setButtonText("Clap 2&4");
        fillBackbeatClapBtn.onClick = [this]() {
            auto& pat = sequencer.getActivePattern();
            for (int s = 0; s < 16; ++s)
                pat.setStep(CP, s, (s == 4 || s == 12) ? STEP_ACCENT : STEP_OFF);
            repaint();
        };
        addAndMakeVisible(fillBackbeatClapBtn);

        shiftLeftBtn.setButtonText("<< SHIFT");
        shiftLeftBtn.onClick = [this]() {
            shiftPattern(-1, -1);
            repaint();
        };
        addAndMakeVisible(shiftLeftBtn);

        shiftRightBtn.setButtonText("SHIFT >>");
        shiftRightBtn.onClick = [this]() {
            shiftPattern(-1, 1);
            repaint();
        };
        addAndMakeVisible(shiftRightBtn);

        exportMidiBtn.setButtonText("💾 EXPORT MIDI");
        exportMidiBtn.onClick = [this]() {
            exportActivePatternToMidi();
        };
        addAndMakeVisible(exportMidiBtn);

        closeDrawerBtn.setButtonText("✕ CLOSE DRAWER");
        closeDrawerBtn.onClick = [this]() {
            if (onCloseRequested)
                onCloseRequested();
        };
        addAndMakeVisible(closeDrawerBtn);
    }

    void PatternDrawer909::refresh()
    {
        repaint();
    }

    void PatternDrawer909::generateAlgorithmicPattern(int genre)
    {
        auto& pat = sequencer.getActivePattern();
        pat.clear();

        FastNoise rng;
        rng.seed(static_cast<uint32_t>(juce::Time::currentTimeMillis() & 0xFFFFFFFF));

        float kickDens = static_cast<float>(kickDensitySlider.getValue());
        float snareDens = static_cast<float>(snareDensitySlider.getValue());
        float hatDens = static_cast<float>(hatDensitySlider.getValue());
        float percDens = static_cast<float>(percDensitySlider.getValue());
        float accProb = static_cast<float>(accentProbSlider.getValue());
        float flamProb = static_cast<float>(flamProbSlider.getValue());

        auto decideState = [&](float prob, bool canFlam) -> uint8_t {
            if (prob <= 0.001f) return STEP_OFF;
            if (rng.next() * 0.5f + 0.5f < prob)
            {
                float r = rng.next() * 0.5f + 0.5f;
                if (canFlam && r < flamProb) return STEP_FLAM;
                if (r < accProb) return STEP_ACCENT;
                return STEP_NORMAL;
            }
            return STEP_OFF;
        };

        switch (genre)
        {
            case 0: // Classic House
            {
                // 4-on-the-floor kick
                for (int s = 0; s < 16; s += 4)
                    pat.setStep(BD, s, (s == 0 || s == 8) ? STEP_ACCENT : STEP_NORMAL);
                if (kickDens > 0.75f && rng.next() > 0.0f) pat.setStep(BD, 14, STEP_NORMAL);

                // Clap / Snare on 2 & 4
                if (snareDens > 0.2f)
                {
                    pat.setStep(CP, 4, STEP_ACCENT);
                    pat.setStep(CP, 12, STEP_ACCENT);
                    if (snareDens > 0.6f && rng.next() > 0.2f) pat.setStep(SD, 15, STEP_NORMAL);
                }

                // Offbeat Open Hat on 2, 6, 10, 14
                for (int s = 2; s < 16; s += 4)
                    pat.setStep(OH, s, STEP_ACCENT);

                // Closed Hat on remaining 16ths
                if (hatDens > 0.4f)
                {
                    for (int s = 0; s < 16; ++s)
                        if (s % 4 != 2 && (rng.next() * 0.5f + 0.5f) < hatDens)
                            pat.setStep(CH, s, STEP_NORMAL);
                }

                // Rimshot syncopations
                if (percDens > 0.3f)
                {
                    pat.setStep(RS, 3, STEP_NORMAL);
                    pat.setStep(RS, 11, STEP_NORMAL);
                }

                sequencer.setShuffle(2); // 54.2% classic house swing
                break;
            }

            case 1: // Detroit Driving Techno
            {
                for (int s = 0; s < 16; s += 4)
                    pat.setStep(BD, s, STEP_ACCENT);
                if (kickDens > 0.6f) pat.setStep(BD, 10, STEP_NORMAL);

                // Snare backbeat
                pat.setStep(SD, 4, STEP_ACCENT);
                pat.setStep(SD, 12, STEP_ACCENT);

                // Steady Ride Cymbal on 8ths
                for (int s = 0; s < 16; s += 2)
                    pat.setStep(RC, s, (s == 2 || s == 10) ? STEP_ACCENT : STEP_NORMAL);

                // Tuned Toms groove
                if (percDens > 0.3f)
                {
                    pat.setStep(LT, 2, STEP_NORMAL);
                    pat.setStep(MT, 6, STEP_NORMAL);
                    pat.setStep(HT, 14, STEP_NORMAL);
                }

                for (int s = 2; s < 16; s += 4)
                    pat.setStep(OH, s, STEP_ACCENT);

                sequencer.setShuffle(1);
                break;
            }

            case 2: // Acid Jack & Warehouse
            {
                for (int s = 0; s < 16; s += 4)
                    pat.setStep(BD, s, STEP_ACCENT);

                pat.setStep(SD, 4, STEP_ACCENT);
                pat.setStep(SD, 12, STEP_ACCENT);
                if (snareDens > 0.5f) pat.setStep(SD, 10, STEP_NORMAL);

                // Heavy syncopated rimshot
                pat.setStep(RS, 2, STEP_NORMAL);
                pat.setStep(RS, 7, STEP_ACCENT);
                pat.setStep(RS, 14, STEP_NORMAL);

                // 16th hats with alternating accents
                for (int s = 0; s < 16; ++s)
                    pat.setStep(CH, s, (s % 2 == 1) ? STEP_ACCENT : STEP_NORMAL);

                sequencer.setShuffle(3); // 58.3% acid jack swing
                break;
            }

            case 3: // Hard Techno & Rave
            {
                // Relentless kick
                for (int s = 0; s < 16; s += 4)
                    pat.setStep(BD, s, STEP_ACCENT);
                if (kickDens > 0.7f)
                {
                    pat.setStep(BD, 2, STEP_NORMAL);
                    pat.setStep(BD, 10, STEP_NORMAL);
                }

                // Snare flam on 12
                pat.setStep(SD, 4, STEP_ACCENT);
                pat.setStep(SD, 12, STEP_FLAM);
                pat.setStep(SD, 14, STEP_NORMAL);

                // Crash on downbeat
                pat.setStep(CC, 0, STEP_ACCENT);
                if (percDens > 0.5f) pat.setStep(CC, 8, STEP_NORMAL);

                for (int s = 2; s < 16; s += 4)
                    pat.setStep(OH, s, STEP_ACCENT);

                sequencer.setShuffle(1);
                break;
            }

            case 4: // Electro Funk Breakbeat
            {
                // Syncopated broken kick
                pat.setStep(BD, 0, STEP_ACCENT);
                pat.setStep(BD, 6, STEP_NORMAL);
                pat.setStep(BD, 8, STEP_NORMAL);
                pat.setStep(BD, 10, STEP_ACCENT);

                pat.setStep(CP, 4, STEP_ACCENT);
                pat.setStep(CP, 12, STEP_ACCENT);

                pat.setStep(RS, 2, STEP_NORMAL);
                pat.setStep(RS, 14, STEP_NORMAL);

                for (int s = 0; s < 16; s += 2)
                    pat.setStep(CH, s, STEP_NORMAL);

                sequencer.setShuffle(4); // 62.5% heavy swing
                break;
            }

            case 5: // Minimal Click & Microhouse
            {
                pat.setStep(BD, 0, STEP_NORMAL);
                pat.setStep(BD, 8, STEP_NORMAL);
                if (kickDens > 0.7f) pat.setStep(BD, 12, STEP_NORMAL);

                pat.setStep(RS, 4, STEP_NORMAL);
                pat.setStep(RS, 7, STEP_NORMAL);
                pat.setStep(RS, 12, STEP_NORMAL);

                pat.setStep(CH, 2, STEP_NORMAL);
                pat.setStep(CH, 6, STEP_NORMAL);
                pat.setStep(CH, 10, STEP_NORMAL);
                pat.setStep(CH, 14, STEP_NORMAL);

                sequencer.setShuffle(1);
                break;
            }

            default: // Algorithmic Chaos
            {
                for (int s = 0; s < 16; ++s)
                {
                    pat.setStep(BD, s, decideState(kickDens * ((s % 4 == 0) ? 0.95f : 0.25f), false));
                    pat.setStep(SD, s, decideState(snareDens * ((s == 4 || s == 12) ? 0.90f : 0.20f), true));
                    pat.setStep(CP, s, decideState(snareDens * ((s == 4 || s == 12) ? 0.85f : 0.15f), false));
                    pat.setStep(CH, s, decideState(hatDens * 0.75f, false));
                    pat.setStep(OH, s, decideState(hatDens * ((s % 4 == 2) ? 0.85f : 0.10f), false));
                    pat.setStep(RS, s, decideState(percDens * 0.35f, false));
                    pat.setStep(LT, s, decideState(percDens * 0.30f, false));
                    pat.setStep(MT, s, decideState(percDens * 0.25f, false));
                    pat.setStep(HT, s, decideState(percDens * 0.20f, false));
                    pat.setStep(RC, s, decideState(hatDens * 0.35f, false));
                    pat.setStep(CC, s, decideState(percDens * ((s == 0) ? 0.70f : 0.05f), false));
                }
                break;
            }
        }

        // Global accent track matching accented steps
        for (int s = 0; s < 16; ++s)
        {
            bool hasAcc = false;
            for (int t = 0; t < NUM_VOICES; ++t)
            {
                if (pat.getStep(t, s) == STEP_ACCENT)
                {
                    hasAcc = true;
                    break;
                }
            }
            if (hasAcc)
                pat.setStep(TRACK_ACCENT, s, STEP_ACCENT);
        }
    }

    void PatternDrawer909::mutatePattern(float mutationRate)
    {
        auto& pat = sequencer.getActivePattern();
        FastNoise rng;
        rng.seed(static_cast<uint32_t>(juce::Time::currentTimeMillis() & 0xFFFFFFFF));

        for (int t = 0; t < NUM_VOICES; ++t)
        {
            for (int s = 0; s < 16; ++s)
            {
                if ((rng.next() * 0.5f + 0.5f) < mutationRate)
                {
                    uint8_t cur = pat.getStep(t, s);
                    if (cur == STEP_OFF)
                        pat.setStep(t, s, (rng.next() > 0.0f) ? STEP_NORMAL : STEP_ACCENT);
                    else if (cur == STEP_NORMAL)
                        pat.setStep(t, s, (rng.next() > 0.0f) ? STEP_ACCENT : STEP_OFF);
                    else
                        pat.setStep(t, s, STEP_NORMAL);
                }
            }
        }
    }

    void PatternDrawer909::shiftPattern(int trackIndex, int shift)
    {
        auto& pat = sequencer.getActivePattern();
        auto shiftTrack = [&](int t) {
            std::array<uint8_t, 16> temp {};
            for (int s = 0; s < 16; ++s)
            {
                int newS = (s + shift) % 16;
                if (newS < 0) newS += 16;
                temp[static_cast<size_t>(newS)] = pat.getStep(t, s);
            }
            for (int s = 0; s < 16; ++s)
                pat.setStep(t, s, temp[static_cast<size_t>(s)]);
        };

        if (trackIndex >= 0 && trackIndex < NUM_TRACKS)
        {
            shiftTrack(trackIndex);
        }
        else
        {
            for (int t = 0; t < NUM_TRACKS; ++t)
                shiftTrack(t);
        }
    }

    void PatternDrawer909::exportActivePatternToMidi()
    {
        auto file = juce::File::getSpecialLocation(juce::File::userDesktopDirectory)
                        .getChildFile("Replicant909_Pattern.mid");
        Preset909 p;
        p.pattern = sequencer.getActivePattern();
        p.bpm = sequencer.getBpm();
        p.shuffle = sequencer.getShuffle();
        p.totalAccent = engine.getAccentCircuit().getAccentLevel();
        p.masterVolume = engine.getMasterVolume();

        if (PresetIO909::exportToMidi(file, p))
        {
            juce::AlertWindow::showMessageBoxAsync(
                juce::AlertWindow::InfoIcon, "Pattern Exported",
                "Saved pattern to desktop:\n" + file.getFullPathName());
        }
    }

    void PatternDrawer909::paint(juce::Graphics& g)
    {
        const auto& theme = ThemeManager::getInstance().getCurrentTheme();
        auto bounds = getLocalBounds().toFloat();

        // Dark background chassis
        g.setColour(theme.chassisBottom.darker(0.35f));
        g.fillRect(bounds);

        // Top decorative handle bar
        g.setColour(theme.accentPrimary);
        g.fillRect(bounds.getX(), bounds.getY(), bounds.getWidth(), 3.0f);

        // Header Title
        g.setColour(theme.silkscreenHeader);
        g.setFont(juce::FontOptions(14.0f, juce::Font::bold));
        g.drawText("PATTERN DRAWER - MULTI-TRACK STEP MATRIX & ALGORITHMIC GROOVE ENGINE",
                   18, 8, 650, 18, juce::Justification::left);

        // Draw Left Generator Section
        drawGeneratorPanel(g);

        // Draw Right Step Matrix Section
        drawMatrixGrid(g);
    }

    void PatternDrawer909::drawGeneratorPanel(juce::Graphics& g)
    {
        const auto& theme = ThemeManager::getInstance().getCurrentTheme();
        auto genBounds = juce::Rectangle<float>(14, 32, 280, getHeight() - 44);

        g.setColour(theme.chassisBottom.darker(0.15f));
        g.fillRoundedRectangle(genBounds, 4.0f);
        g.setColour(theme.silkscreenLines.withAlpha(0.5f));
        g.drawRoundedRectangle(genBounds, 4.0f, 1.0f);

        g.setColour(theme.silkscreenText);
        g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
        g.drawText("ALGORITHMIC GENERATOR", 24, 38, 200, 16, juce::Justification::left);

        // Labels for generator sliders
        g.setFont(juce::FontOptions(9.5f, juce::Font::plain));
        int y = 92;
        static constexpr const char* sliderLabels[6] = {
            "Kick Density:", "Snare/Clap:", "Hats Shimmer:",
            "Toms/Perc:", "Accent Prob:", "Flam Prob:"
        };
        for (int i = 0; i < 6; ++i)
        {
            g.drawText(sliderLabels[i], 24, y, 95, 18, juce::Justification::left);
            y += 24;
        }
    }

    void PatternDrawer909::drawMatrixGrid(juce::Graphics& g)
    {
        const auto& theme = ThemeManager::getInstance().getCurrentTheme();
        const auto& pat = sequencer.getActivePattern();
        int activeStep = sequencer.isPlaying() ? sequencer.getCurrentStep() : -1;

        // Container bounds for the matrix
        auto gridBox = juce::Rectangle<float>(310, 32, getWidth() - 324, getHeight() - 44);
        g.setColour(theme.chassisBottom.darker(0.15f));
        g.fillRoundedRectangle(gridBox, 4.0f);
        g.setColour(theme.silkscreenLines.withAlpha(0.5f));
        g.drawRoundedRectangle(gridBox, 4.0f, 1.0f);

        matrixBounds = juce::Rectangle<float>(310 + headerWidth, 62, gridBox.getWidth() - headerWidth - 10, gridBox.getHeight() - 72);
        rowHeight = matrixBounds.getHeight() / static_cast<float>(NUM_TRACKS);
        colWidth = matrixBounds.getWidth() / 16.0f;

        // 1. Column header (Steps 1 to 16)
        g.setFont(juce::FontOptions(9.0f, juce::Font::bold));
        for (int s = 0; s < 16; ++s)
        {
            float cx = matrixBounds.getX() + static_cast<float>(s) * colWidth;
            bool isCurrent = (s == activeStep);

            if (isCurrent)
            {
                g.setColour(theme.accentPrimary.withAlpha(0.35f));
                g.fillRect(cx, matrixBounds.getY() - 16.0f, colWidth, 14.0f);
            }

            g.setColour(isCurrent ? theme.accentPrimary : theme.silkscreenText);
            g.drawText(juce::String(s + 1), cx, matrixBounds.getY() - 16.0f, colWidth, 14.0f, juce::Justification::centred);
        }

        // 2. Rows: 11 instruments + ACCENT track
        for (int t = 0; t < NUM_TRACKS; ++t)
        {
            float ry = matrixBounds.getY() + static_cast<float>(t) * rowHeight;
            juce::String trackLabel = (t < NUM_VOICES) ? VOICE_SHORT_NAMES[t] : "ACC";

            // Row header label
            g.setColour((t == selectedTrack) ? theme.accentPrimary : theme.silkscreenText);
            g.setFont(juce::FontOptions(10.0f, juce::Font::bold));
            g.drawText(trackLabel, 316, ry, 34, rowHeight, juce::Justification::centredLeft);

            // Mute / Solo badges (for audio voices 0..10)
            if (t < NUM_VOICES)
            {
                bool isM = engine.isMuted(t);
                bool isS = engine.isSoloed(t);

                // Mute badge
                g.setColour(isM ? juce::Colour(0xffff2244) : juce::Colour(0xff22252a));
                g.fillRoundedRectangle(348, ry + 3, 14, rowHeight - 6, 2.0f);
                g.setColour(isM ? juce::Colours::white : juce::Colours::grey);
                g.setFont(juce::FontOptions(8.0f, juce::Font::bold));
                g.drawText("M", 348, ry + 3, 14, rowHeight - 6, juce::Justification::centred);

                // Solo badge
                g.setColour(isS ? juce::Colour(0xffffbb22) : juce::Colour(0xff22252a));
                g.fillRoundedRectangle(364, ry + 3, 14, rowHeight - 6, 2.0f);
                g.setColour(isS ? juce::Colours::black : juce::Colours::grey);
                g.drawText("S", 364, ry + 3, 14, rowHeight - 6, juce::Justification::centred);
            }

            // Draw 16 step cells for this track
            for (int s = 0; s < 16; ++s)
            {
                float cx = matrixBounds.getX() + static_cast<float>(s) * colWidth;
                auto cellRect = juce::Rectangle<float>(cx + 1.0f, ry + 1.5f, colWidth - 2.0f, rowHeight - 3.0f);

                // Alternating 4-step bar shading
                bool altBar = ((s / 4) % 2) != 0;
                juce::Colour cellBg = altBar ? juce::Colour(0xff121418) : juce::Colour(0xff181a20);
                g.setColour(cellBg);
                g.fillRoundedRectangle(cellRect, 2.0f);

                uint8_t state = pat.getStep(t, s);
                if (state == STEP_NORMAL)
                {
                    g.setColour(theme.ledNormalLit);
                    g.fillRoundedRectangle(cellRect.reduced(2.0f), 2.0f);
                }
                else if (state == STEP_ACCENT)
                {
                    g.setColour(theme.ledAccentLit);
                    g.fillRoundedRectangle(cellRect.reduced(1.5f), 2.0f);
                    // Core highlight
                    g.setColour(juce::Colours::white.withAlpha(0.6f));
                    g.fillRoundedRectangle(cellRect.getX() + 4, cellRect.getY() + 3, cellRect.getWidth() - 8, 3, 1.0f);
                }
                else if (state == STEP_FLAM)
                {
                    g.setColour(theme.ledFlamLit);
                    g.fillRoundedRectangle(cellRect.reduced(2.0f), 2.0f);
                    g.setColour(juce::Colours::black.withAlpha(0.6f));
                    g.drawVerticalLine(static_cast<int>(cellRect.getCentreX()), cellRect.getY() + 2, cellRect.getBottom() - 2);
                }

                // Grid cell borders
                g.setColour(theme.silkscreenLines.withAlpha(0.25f));
                g.drawRoundedRectangle(cellRect, 2.0f, 0.7f);
            }
        }

        // 3. Vertical Playhead Cursor line sweeping across
        if (activeStep >= 0 && activeStep < 16)
        {
            float playX = matrixBounds.getX() + static_cast<float>(activeStep) * colWidth + colWidth * 0.5f;
            g.setColour(theme.accentPrimary.withAlpha(0.45f));
            g.fillRect(matrixBounds.getX() + static_cast<float>(activeStep) * colWidth,
                       matrixBounds.getY(), colWidth, matrixBounds.getHeight());
            g.setColour(theme.accentPrimary);
            g.drawVerticalLine(static_cast<int>(playX), matrixBounds.getY(), matrixBounds.getBottom());
        }
    }

    void PatternDrawer909::resized()
    {
        // Generator Left controls
        genreSelector.setBounds(24, 60, 260, 22);

        int y = 92;
        kickDensitySlider.setBounds(115, y, 168, 20); y += 24;
        snareDensitySlider.setBounds(115, y, 168, 20); y += 24;
        hatDensitySlider.setBounds(115, y, 168, 20); y += 24;
        percDensitySlider.setBounds(115, y, 168, 20); y += 24;
        accentProbSlider.setBounds(115, y, 168, 20); y += 24;
        flamProbSlider.setBounds(115, y, 168, 20); y += 24;

        y += 10;
        generateButton.setBounds(24, y, 126, 26);
        mutateButton.setBounds(158, y, 126, 26); y += 32;
        clearButton.setBounds(24, y, 260, 24);

        // Multi-Track Quick Fill & Action buttons at top of matrix box
        int topBarY = 36;
        fill4x4KickBtn.setBounds(316, topBarY, 68, 20);
        fillOffbeatHatBtn.setBounds(388, topBarY, 82, 20);
        fillBackbeatClapBtn.setBounds(474, topBarY, 74, 20);
        shiftLeftBtn.setBounds(552, topBarY, 70, 20);
        shiftRightBtn.setBounds(626, topBarY, 70, 20);
        exportMidiBtn.setBounds(700, topBarY, 110, 20);

        closeDrawerBtn.setBounds(getWidth() - 145, 6, 130, 22);
    }

    void PatternDrawer909::mouseDown(const juce::MouseEvent& e)
    {
        auto pos = e.position;

        // Check if user clicked in the step matrix cells
        if (matrixBounds.contains(pos))
        {
            int col = static_cast<int>((pos.x - matrixBounds.getX()) / colWidth);
            int row = static_cast<int>((pos.y - matrixBounds.getY()) / rowHeight);

            col = juce::jlimit(0, 15, col);
            row = juce::jlimit(0, NUM_TRACKS - 1, row);

            selectedTrack = row;

            if (e.mods.isRightButtonDown())
            {
                uint8_t cur = sequencer.getActivePattern().getStep(row, col);
                sequencer.getActivePattern().setStep(row, col, (cur == STEP_FLAM ? STEP_OFF : STEP_FLAM));
            }
            else
            {
                sequencer.getActivePattern().toggleStep(row, col);
            }
            repaint();
            return;
        }

        // Check if user clicked Mute or Solo badges
        for (int t = 0; t < NUM_VOICES; ++t)
        {
            float ry = matrixBounds.getY() + static_cast<float>(t) * rowHeight;
            juce::Rectangle<float> muteRect(348, ry + 3, 14, rowHeight - 6);
            juce::Rectangle<float> soloRect(364, ry + 3, 14, rowHeight - 6);

            if (muteRect.contains(pos))
            {
                engine.setMuted(t, !engine.isMuted(t));
                repaint();
                return;
            }
            if (soloRect.contains(pos))
            {
                engine.setSoloed(t, !engine.isSoloed(t));
                repaint();
                return;
            }
        }
    }
}
