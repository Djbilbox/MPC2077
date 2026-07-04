cmake_minimum_required(VERSION 3.22)
project(MPC2077 VERSION 1.0.0)

# ------------------------------------------------------------------
#  MPC2077 by DJBILBOX BEATS  —  cyberpunk drum machine (VST3 / AU)
# ------------------------------------------------------------------

set(JUCE_DIR "C:/JUCE" CACHE PATH "JUCE root")
add_subdirectory(${JUCE_DIR} JUCE)

# Optional embedded assets (logo + factory drum samples). Only added if present so
# the project always configures even before the artwork/samples are baked in.
set(MPC_BINARY_SOURCES "")
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets/logo.png")
    list(APPEND MPC_BINARY_SOURCES assets/logo.png)
endif()
file(GLOB MPC_SAMPLE_WAVS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "assets/samples/*.wav")
list(APPEND MPC_BINARY_SOURCES ${MPC_SAMPLE_WAVS})

if(MPC_BINARY_SOURCES)
    juce_add_binary_data(MpcBinaryData SOURCES ${MPC_BINARY_SOURCES})
endif()

juce_add_plugin(MPC2077
    PLUGIN_NAME              "MPC2077"
    PLUGIN_MANUFACTURER_CODE "DJBB"
    PLUGIN_CODE              "Mp27"
    FORMATS                  VST3 Standalone
    PRODUCT_NAME             "MPC2077"
    COMPANY_NAME             "Djbilbox BEATS"
    VERSION                  "1.0.0"
    IS_SYNTH                 TRUE
    NEEDS_MIDI_INPUT         TRUE
    NEEDS_MIDI_OUTPUT        FALSE
    IS_MIDI_EFFECT           FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    BUNDLE_ID                "com.DjbilboxBeats.MPC2077"
    VST3_CATEGORIES          "Instrument|Drum"
)

juce_generate_juce_header(MPC2077)

target_sources(MPC2077 PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/DSP/DrumEngine.cpp
    Source/DSP/StepSequencer.cpp
    Source/DSP/FxChain.cpp
    Source/DSP/DrumSynth.cpp
    Source/UI/MPC2077LookAndFeel.cpp
    Source/UI/Components/PadGrid.cpp
    Source/UI/Components/SequencerView.cpp
    Source/UI/Components/TopPanel.cpp
    Source/UI/Components/MixerStrip.cpp
    Source/UI/Components/ControlSection.cpp
)

target_include_directories(MPC2077 PRIVATE Source)

target_compile_features(MPC2077 PRIVATE cxx_std_17)

target_compile_definitions(MPC2077 PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_REPORT_APP_USAGE=0
)

# Downgrade W4 -> W3 to suppress JUCE 8 Font/deprecation warnings (as in BIGBASS)
target_compile_options(MPC2077 PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/W3>)

set(MPC_LINK_LIBS
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_plugin_client
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_core
    juce::juce_data_structures
    juce::juce_dsp
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
)

if(MPC_BINARY_SOURCES)
    list(APPEND MPC_LINK_LIBS MpcBinaryData)
    target_compile_definitions(MPC2077 PUBLIC MPC_HAS_BINARY_DATA=1)
endif()

target_link_libraries(MPC2077 PRIVATE
    ${MPC_LINK_LIBS}
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

# --- Standalone: DPI-unaware manifest so the window sizes correctly at 150% DPI ---
# (JUCE's standalone wrapper mis-sizes the editor on HiDPI displays; the DAW/VST3 is unaffected.)
if(MSVC AND TARGET MPC2077_Standalone)
    target_link_options(MPC2077_Standalone PRIVATE
        "/MANIFEST:EMBED"
        "/MANIFESTINPUT:${CMAKE_CURRENT_SOURCE_DIR}/assets/Standalone.manifest")
endif()
