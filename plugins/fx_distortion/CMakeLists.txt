juce_add_plugin (LxrDistortion
    COMPANY_NAME      "LxrVoices"
    PLUGIN_MANUFACTURER_CODE LxrV
    PLUGIN_CODE       LxDt
    FORMATS           Standalone VST3
    PRODUCT_NAME      "LxrDistortion"
    DESCRIPTION       "LXR waveshaping distortion"
    IS_SYNTH          FALSE
    NEEDS_MIDI_INPUT  FALSE
    NEEDS_MIDI_OUTPUT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD     FALSE
    HARDENED_RUNTIME_ENABLED    TRUE
    BUNDLE_ID         "com.lxrvoices.LxrDistortion"
)

target_sources (LxrDistortion PRIVATE
    PluginProcessor.cpp
    PluginEditor.cpp
    DistortionEngine.cpp
)

target_compile_features (LxrDistortion PRIVATE cxx_std_17)

target_compile_definitions (LxrDistortion PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_include_directories (LxrDistortion PRIVATE
    ${CMAKE_SOURCE_DIR}/lxr_dsp
    ${CMAKE_SOURCE_DIR}/lxr_dsp/compat
    ${CMAKE_SOURCE_DIR}/lxr_dsp/stubs
    ${CMAKE_SOURCE_DIR}/lxr_dsp/DSPAudio
    ${CMAKE_SOURCE_DIR}/plugins/shared
)

target_link_libraries (LxrDistortion
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
    TARGET             LxrDistortion
    CLAP_ID            "com.lxrvoices.LxrDistortion"
    CLAP_FEATURES      audio-effect distortion
)
