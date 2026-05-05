# Drum voice plugin

juce_add_plugin (LxrDrum
    COMPANY_NAME      "LxrVoices"
    PLUGIN_MANUFACTURER_CODE LxrV
    PLUGIN_CODE       LxDr
    FORMATS           Standalone VST3        # CLAP added below via clap-juce-extensions
    PRODUCT_NAME      "LxrDrum"
    DESCRIPTION       "LXR drum voice (kick / tom)"
    IS_SYNTH          TRUE
    NEEDS_MIDI_INPUT  TRUE
    NEEDS_MIDI_OUTPUT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD     FALSE
    HARDENED_RUNTIME_ENABLED    TRUE
    BUNDLE_ID         "com.lxrvoices.LxrDrum"
)

target_sources (LxrDrum
    PRIVATE
        PluginProcessor.cpp
        PluginEditor.cpp
        DrumEngine.cpp
)

target_compile_features (LxrDrum PRIVATE cxx_std_17)

target_compile_definitions (LxrDrum
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_include_directories (LxrDrum
    PRIVATE
        ${CMAKE_SOURCE_DIR}/lxr_dsp
        ${CMAKE_SOURCE_DIR}/lxr_dsp/compat
        ${CMAKE_SOURCE_DIR}/lxr_dsp/stubs
        ${CMAKE_SOURCE_DIR}/lxr_dsp/DSPAudio
        ${CMAKE_SOURCE_DIR}/plugins/shared
)

target_link_libraries (LxrDrum
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

# Add CLAP format via clap-juce-extensions.
# Sets up an LxrDrum_CLAP target that links the shared core to a CLAP wrapper.
clap_juce_extensions_plugin (
    TARGET             LxrDrum
    CLAP_ID            "com.lxrvoices.LxrDrum"
    CLAP_FEATURES      instrument synthesizer drum-machine
    CLAP_MANUAL_URL    "https://example.com/lxr-voices/manual"
    CLAP_SUPPORT_URL   "https://example.com/lxr-voices/support"
)
