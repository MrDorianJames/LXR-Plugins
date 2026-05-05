juce_add_plugin (LxrCymbal
    COMPANY_NAME      "LxrVoices"
    PLUGIN_MANUFACTURER_CODE LxrV
    PLUGIN_CODE       LxCy
    FORMATS           Standalone VST3
    PRODUCT_NAME      "LxrCymbal"
    DESCRIPTION       "LXR FM percussion / cymbal voice"
    IS_SYNTH          TRUE
    NEEDS_MIDI_INPUT  TRUE
    NEEDS_MIDI_OUTPUT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD     FALSE
    HARDENED_RUNTIME_ENABLED    TRUE
    BUNDLE_ID         "com.lxrvoices.LxrCymbal"
)

target_sources (LxrCymbal PRIVATE
    PluginProcessor.cpp
    PluginEditor.cpp
    CymbalEngine.cpp
)

target_compile_features (LxrCymbal PRIVATE cxx_std_17)

target_compile_definitions (LxrCymbal PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_include_directories (LxrCymbal PRIVATE
    ${CMAKE_SOURCE_DIR}/lxr_dsp
    ${CMAKE_SOURCE_DIR}/lxr_dsp/compat
    ${CMAKE_SOURCE_DIR}/lxr_dsp/stubs
    ${CMAKE_SOURCE_DIR}/lxr_dsp/DSPAudio
    ${CMAKE_SOURCE_DIR}/plugins/shared
)

target_link_libraries (LxrCymbal
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
    TARGET             LxrCymbal
    CLAP_ID            "com.lxrvoices.LxrCymbal"
    CLAP_FEATURES      instrument synthesizer drum-machine
)
