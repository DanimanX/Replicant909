#include <juce_gui_basics/juce_gui_basics.h>
#include "../Source/PluginProcessor.h"
#include "../Source/PluginEditor.h"
#include "../Source/GUI/Theme909.h"
#include <iostream>

using namespace Replicant909;

int main()
{
    juce::ScopedJuceInitialiser_GUI guiInit;

    std::cout << "Rendering Replicant909 Offscreen UI Snapshots..." << std::endl;

    PluginProcessor processor;
    PluginEditor editor(processor);
    editor.setSize(1040, 460);

    // 1. Snapshot Classic Skin (Compact, 1040 x 460)
    {
        ThemeManager::getInstance().setSkin(SKIN_RETRO_CLASSIC);
        editor.repaint();

        juce::Image img(juce::Image::ARGB, 1040, 460, true);
        juce::Graphics g(img);
        editor.paintEntireComponent(g, true);

        juce::File outDir("/PROCESOS/Replicant909/assets");
        outDir.createDirectory();
        juce::File outFile = outDir.getChildFile("replicant909_classic_skin.png");
        outFile.deleteFile();
        juce::FileOutputStream stream(outFile);
        if (stream.openedOk())
        {
            juce::PNGImageFormat png;
            png.writeImageToStream(img, stream);
            std::cout << "Saved: " << outFile.getFullPathName() << std::endl;
        }

        // Also save as main preview
        juce::File previewFile = outDir.getChildFile("replicant909_preview.png");
        previewFile.deleteFile();
        juce::FileOutputStream prevStream(previewFile);
        if (prevStream.openedOk())
        {
            juce::PNGImageFormat png;
            png.writeImageToStream(img, prevStream);
        }
    }

    // 2. Snapshot Modern Obsidian Skin (Compact, 1040 x 460)
    {
        ThemeManager::getInstance().setSkin(SKIN_MODERN_OBSIDIAN);
        editor.repaint();

        juce::Image img(juce::Image::ARGB, 1040, 460, true);
        juce::Graphics g(img);
        editor.paintEntireComponent(g, true);

        juce::File outFile("/PROCESOS/Replicant909/assets/replicant909_modern_skin.png");
        outFile.deleteFile();
        juce::FileOutputStream stream(outFile);
        if (stream.openedOk())
        {
            juce::PNGImageFormat png;
            png.writeImageToStream(img, stream);
            std::cout << "Saved: " << outFile.getFullPathName() << std::endl;
        }
    }

    // 3. Snapshot Pattern Drawer Open (1040 x 840)
    {
        editor.setPatternDrawerOpen(true);
        editor.repaint();

        juce::Image img(juce::Image::ARGB, 1040, 840, true);
        juce::Graphics g(img);
        editor.paintEntireComponent(g, true);

        juce::File outFile("/PROCESOS/Replicant909/assets/replicant909_pattern_drawer.png");
        outFile.deleteFile();
        juce::FileOutputStream stream(outFile);
        if (stream.openedOk())
        {
            juce::PNGImageFormat png;
            png.writeImageToStream(img, stream);
            std::cout << "Saved: " << outFile.getFullPathName() << std::endl;
        }
    }

    std::cout << "Offscreen snapshots complete!" << std::endl;
    return 0;
}
