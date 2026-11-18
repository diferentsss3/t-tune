cmake_minimum_required(VERSION 3.22)
project(T-TUNE VERSION 1.0.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_subdirectory(JUCE)
juce_add_plugin(T_TUNE
    COMPANY_NAME "T-TUNE Audio"
    PRODUCT_NAME "T-TUNE"
    PLUGIN_MANUFACTURER_CODE Ttun
    PLUGIN_CODE Tt01
    FORMATS VST3 Standalone
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    VST3_CATEGORIES Fx PitchShift
)
target_sources(T_TUNE PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
)
target_compile_definitions(T_TUNE PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JucePlugin_Name="T-TUNE"
)
if(MSVC)
    target_compile_options(T_TUNE PRIVATE /utf-8)
endif()
target_link_libraries(T_TUNE PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)
