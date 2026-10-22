cmake_minimum_required(VERSION 3.22)
project(Replicant909 VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ------------------------------------------------------------------------------
# JUCE Dependency via FetchContent (JUCE 8)
# ------------------------------------------------------------------------------
include(FetchContent)

if(EXISTS "/PROCESOS/ReplicantTr8s/build/_deps/juce-src/CMakeLists.txt")
    set(FETCHCONTENT_SOURCE_DIR_JUCE "/PROCESOS/ReplicantTr8s/build/_deps/juce-src" CACHE PATH "" FORCE)
endif()

set(JUCE_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(JUCE_BUILD_EXTRAS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.4
    GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(JUCE)

# ------------------------------------------------------------------------------
# CLAP Format Support via clap-juce-extensions
# ------------------------------------------------------------------------------
option(BUILD_CLAP "Build CLAP plugin format" ON)
if(BUILD_CLAP)
    FetchContent_Declare(
        clap-juce-extensions
        GIT_REPOSITORY https://github.com/free-audio/clap-juce-extensions.git
        GIT_TAG        main
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(clap-juce-extensions)
endif()

# ------------------------------------------------------------------------------
# Formats: VST3, AU (macOS only), and Standalone
# ------------------------------------------------------------------------------
set(PLUGIN_FORMATS VST3 Standalone)
if(APPLE)
    list(APPEND PLUGIN_FORMATS AU)
endif()

# ------------------------------------------------------------------------------
# Source Files
# ------------------------------------------------------------------------------
set(R909_SOURCES
    Source/Common909.h
    Source/PluginProcessor.h
    Source/PluginProcessor.cpp
    Source/PluginEditor.h
    Source/PluginEditor.cpp

    # DSP Circuit Engine
    Source/DSP/VoiceBase.h
    Source/DSP/TR909RomData.h
    Source/DSP/BassDrumVoice.h
    Source/DSP/BassDrumVoice.cpp
    Source/DSP/SnareDrumVoice.h
    Source/DSP/SnareDrumVoice.cpp
    Source/DSP/TomVoice.h
    Source/DSP/TomVoice.cpp
    Source/DSP/RimShotVoice.h
    Source/DSP/RimShotVoice.cpp
    Source/DSP/HandClapVoice.h
    Source/DSP/HandClapVoice.cpp
    Source/DSP/CymbalVoice.h
    Source/DSP/CymbalVoice.cpp
    Source/DSP/TotalAccentCircuit.h
    Source/DSP/Pattern909.h
    Source/DSP/GrooveSequencer909.h
    Source/DSP/GrooveSequencer909.cpp
    Source/DSP/DrumEngine909.h
    Source/DSP/DrumEngine909.cpp

    # GUI & Procedural Skins
    Source/GUI/Theme909.h
    Source/GUI/Theme909.cpp
    Source/GUI/LookAndFeel909.h
    Source/GUI/LookAndFeel909.cpp
    Source/GUI/PanelHeaderComponent.h
    Source/GUI/PanelHeaderComponent.cpp
    Source/GUI/VoiceStripComponent.h
    Source/GUI/VoiceStripComponent.cpp
    Source/GUI/StepSequencerComponent.h
    Source/GUI/StepSequencerComponent.cpp
    Source/GUI/RearPanelComponent.h
    Source/GUI/RearPanelComponent.cpp
    Source/GUI/PatternDrawer909.h
    Source/GUI/PatternDrawer909.cpp

    # Presets & MIDI
    Source/Presets/PresetBank909.h
    Source/Presets/PresetBank909.cpp
    Source/Presets/PresetIO909.h
    Source/Presets/PresetIO909.cpp
)

set(R909_INCLUDE_DIRS
    Source
    Source/DSP
    Source/GUI
    Source/Presets
)

set(R909_JUCE_LIBS
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_dsp
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_recommended_config_flags
)

# ------------------------------------------------------------------------------
# Plugin Target Definition
# ------------------------------------------------------------------------------
juce_add_plugin(Replicant909
    COMPANY_NAME "DanimanX"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE DnmX
    PLUGIN_CODE R909
    FORMATS ${PLUGIN_FORMATS}
    PRODUCT_NAME "Replicant909"
    DESCRIPTION "Circuit-Accurate Roland TR-909 Drum Machine with Multi-Bus Individual Outs and Dual Skins"
    BUNDLE_ID "com.danimanx.replicant909"
)

target_sources(Replicant909 PRIVATE ${R909_SOURCES})

target_compile_definitions(Replicant909 PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_include_directories(Replicant909 PRIVATE ${R909_INCLUDE_DIRS})
target_link_libraries(Replicant909 PRIVATE ${R909_JUCE_LIBS})

# Attach CLAP extension target if available
if(TARGET clap_juce_extensions)
    clap_juce_extensions_plugin(
        TARGET Replicant909
        CLAP_ID "com.danimanx.replicant909"
        CLAP_FEATURES "instrument" "drum" "synthesizer"
    )
endif()

# ------------------------------------------------------------------------------
# Internal Tools
# ------------------------------------------------------------------------------
function(r909_add_tool target source)
    add_executable(${target} EXCLUDE_FROM_ALL ${source} ${R909_SOURCES})
    target_include_directories(${target} PRIVATE ${R909_INCLUDE_DIRS})
    target_compile_definitions(${target} PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_MODAL_LOOPS_PERMITTED=1
    )
    target_link_libraries(${target} PRIVATE ${R909_JUCE_LIBS})
endfunction()

# Headless regression suite
r909_add_tool(run_tests tools/run_tests.cpp)

# Off-screen UI snapshot generator
r909_add_tool(generate_snapshot tools/generate_snapshot.cpp)

# Preset MIDI generator
r909_add_tool(generate_presets tools/generate_presets.cpp)
