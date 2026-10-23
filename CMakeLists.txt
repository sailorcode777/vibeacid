    cmake_minimum_required(VERSION 3.22)

# Static MSVC runtime so the plugin doesn't need extra Visual C++ redistributables
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

project(AcidSynth VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Downloads JUCE automatically at configure time (no Projucer needed)
include(FetchContent)
FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.4
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(AcidSynth
    COMPANY_NAME "AcidLabs"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    PLUGIN_MANUFACTURER_CODE Acdl
    PLUGIN_CODE Acsy
    FORMATS VST3 Standalone
    PRODUCT_NAME "AcidSynth"
    COPY_PLUGIN_AFTER_BUILD FALSE)

target_sourcestarget_sources(AcidSynth PRIVATE
    PluginProcessor.cpp
    PluginProcessor.h
    Acid303.h)

target_compile_definitions(AcidSynth PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(AcidSynth
    PRIVATE
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
