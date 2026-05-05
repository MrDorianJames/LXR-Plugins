juce_add_plugin (LxrFilter
    COMPANY_NAME      "LxrVoices"
    PLUGIN_MANUFACTURER_CODE LxrV
    PLUGIN_CODE       LxFt
    FORMATS           Standalone VST3
    PRODUCT_NAME      "LxrFilter"
    DESCRIPTION       "LXR state-variable filter (multimode + drive)"
    IS_SYNTH          FALSE
    NEEDS_MIDI_INPUT  FALSE
    NEEDS_MIDI_OUTPUT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD     FALSE
    HARDENED_RUNTIME_ENABLED    TRUE
    BUNDLE_ID         "com.lxrvoices.LxrFilter"
)

target_sources (LxrFilter PRIVATE
    PluginProcessor.cpp
    PluginEditor.cpp
    FilterEngine.cpp
)

target_compile_features (LxrFilter PRIVATE cxx_std_17)

target_compile_definitions (LxrFilter PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_include_directories (LxrFilter PRIVATE
    ${CMAKE_SOURCE_DIR}/lxr_dsp
    ${CMAKE_SOURCE_DIR}/lxr_dsp/compat
    ${CMAKE_SOURCE_DIR}/lxr_dsp/stubs
    ${CMAKE_SOURCE_DIR}/lxr_dsp/DSPAudio
    ${CMAKE_SOURCE_DIR}/plugins/shared
)

target_link_libraries (LxrFilter
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
    TARGET             LxrFilter
    CLAP_ID            "com.lxrvoices.LxrFilter"
    CLAP_FEATURES      audio-effect filter
)
