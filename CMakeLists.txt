cmake_minimum_required (VERSION 3.22)

project (lxr-voices-clap
         VERSION    0.1.0
         LANGUAGES  C CXX)

set (CMAKE_C_STANDARD            99)
set (CMAKE_C_STANDARD_REQUIRED   ON)
set (CMAKE_CXX_STANDARD          17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_POSITION_INDEPENDENT_CODE ON)

# ----------------------------------------------------------------------
# 1. JUCE  (provides the Standalone / VST3 plugin scaffolding)
# ----------------------------------------------------------------------
# Either:
#   git submodule add https://github.com/juce-framework/JUCE third_party/JUCE
# or set JUCE_PATH on the cmake command line:
#   cmake -B build -DJUCE_PATH=/path/to/JUCE
if (DEFINED JUCE_PATH)
    add_subdirectory (${JUCE_PATH} ${CMAKE_BINARY_DIR}/JUCE_build)
elseif (EXISTS "${CMAKE_SOURCE_DIR}/third_party/JUCE/CMakeLists.txt")
    add_subdirectory (third_party/JUCE)
else()
    message (FATAL_ERROR
        "JUCE not found. Add it as a submodule under third_party/JUCE or "
        "pass -DJUCE_PATH=/path/to/JUCE to cmake.")
endif()

# ----------------------------------------------------------------------
# 2. clap-juce-extensions  (turns JUCE plugin targets into CLAP targets)
# ----------------------------------------------------------------------
if (EXISTS "${CMAKE_SOURCE_DIR}/third_party/clap-juce-extensions/CMakeLists.txt")
    add_subdirectory (third_party/clap-juce-extensions)
elseif (DEFINED CLAP_JUCE_EXTENSIONS_PATH)
    add_subdirectory (${CLAP_JUCE_EXTENSIONS_PATH}
                      ${CMAKE_BINARY_DIR}/clap_juce_build)
else()
    message (FATAL_ERROR
        "clap-juce-extensions not found. Add it under "
        "third_party/clap-juce-extensions or pass "
        "-DCLAP_JUCE_EXTENSIONS_PATH=...")
endif()

# ----------------------------------------------------------------------
# 3. lxr_dsp_lib  (the vendored LXR DSP, compiled as a static lib)
# ----------------------------------------------------------------------
file (GLOB LXR_DSP_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_SOURCE_DIR}/lxr_dsp/DSPAudio/*.c"
        "${CMAKE_SOURCE_DIR}/lxr_dsp/stubs/*.c"
        "${CMAKE_SOURCE_DIR}/lxr_dsp/SampleRom/*.c")

add_library (lxr_dsp_lib STATIC ${LXR_DSP_SOURCES})

target_include_directories (lxr_dsp_lib
    PUBLIC
        ${CMAKE_SOURCE_DIR}/lxr_dsp
        ${CMAKE_SOURCE_DIR}/lxr_dsp/compat
        ${CMAKE_SOURCE_DIR}/lxr_dsp/stubs
        ${CMAKE_SOURCE_DIR}/lxr_dsp/DSPAudio)

# The LXR firmware uses GCC's gnu89 inline-in-header pattern. Force it
# even when the host project default is C99/C11.
target_compile_options (lxr_dsp_lib PRIVATE
    $<$<COMPILE_LANGUAGE:C>:-std=gnu89>
    $<$<C_COMPILER_ID:GNU,Clang>:-Wno-unused-but-set-variable -Wno-unused-variable -Wno-incompatible-pointer-types -Wno-builtin-macro-redefined>
)

# ----------------------------------------------------------------------
# 4. Plugins
# ----------------------------------------------------------------------
add_subdirectory (plugins/drum)
add_subdirectory (plugins/snare)
add_subdirectory (plugins/cymbal)
add_subdirectory (plugins/hihat)
add_subdirectory (plugins/fx_filter)
add_subdirectory (plugins/fx_distortion)

# ----------------------------------------------------------------------
# 5. Standalone smoke test  (no JUCE - links directly to lxr_dsp_lib)
# ----------------------------------------------------------------------
add_executable (lxr_voice_test test/lxr_voice_test.c)
target_link_libraries (lxr_voice_test PRIVATE lxr_dsp_lib m)
set_target_properties (lxr_voice_test PROPERTIES
                       RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/test)
