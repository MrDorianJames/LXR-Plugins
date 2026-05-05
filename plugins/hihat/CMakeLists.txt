juce_add_plugin (LxrHiHat
    COMPANY_NAME      "LxrVoices"
    PLUGIN_MANUFACTURER_CODE LxrV
    PLUGIN_CODE       LxHh
    FORMATS           Standalone VST3
    PRODUCT_NAME      "LxrHiHat"
    DESCRIPTION       "LXR hi-hat voice (closed/open)"
    IS_SYNTH          TRUE
    NEEDS_MIDI_INPUT  TRUE
    NEEDS_MIDI_OUTPUT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD     FALSE
    HARDENED_RUNTIME_ENABLED    TRUE
    BUNDLE_ID         "com.lxrvoices.LxrHiHat"
)

target_sources (LxrHiHat PRIVATE
    PluginProcessor.cpp
    PluginEditor.cpp
    HiHatEngine.cpp
)

target_compile_features (LxrHiHat PRIVATE cxx_std_17)

target_compile_definitions (LxrHiHat PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_include_directories (LxrHiHat PRIVATE
    ${CMAKE_SOURCE_DIR}/lxr_dsp
    ${CMAKE_SOURCE_DIR}/lxr_dsp/compat
    ${CMAKE_SOURCE_DIR}/lxr_dsp/stubs
    ${CMAKE_SOURCE_DIR}/lxr_dsp/DSPAudio
    ${CMAKE_SOURCE_DIR}/plugins/shared
)

target_link_libraries (LxrHiHat
    PRIVATE
        lxr_dsp_lib
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

clap_juce_extensions_plugin (
    TARGET             LxrHiHat
    CLAP_ID            "com.lxrvoices.LxrHiHat"
    CLAP_FEATURES      instrument synthesizer drum-machine
)
