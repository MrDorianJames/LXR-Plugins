# LXR Voices CLAP

CLAP plugins (one per voice type) wrapping the DSP engine of the
[Sonic Potions LXR drum synthesizer](https://github.com/SonicPotions/LXR)
for use in any CLAP-capable DAW.

The four plugins:
- **LxrDrum**   - kick / tom / general drum voice (3-voice engine; this plugin instantiates voice 0)
- **LxrSnare**  - subtractive clap/snare voice
- **LxrCymbal** - 3-operator FM percussion voice
- **LxrHiHat**  - hi-hat voice (closed/open via parameter or note number)

> [!IMPORTANT]
> ## License — read this first
>
> The original LXR firmware is **not** under a permissive open-source license.
> Every source file in `lxr_dsp/` is governed by the LXR redistribution
> terms reproduced in `lxr_dsp/LXR_LICENSE.txt`. The most important clause:
>
> > **The code may not be sold, nor may it be used in a commercial product or activity.**
>
> You can build, share, and use these plugins for free. You **cannot** sell
> them, bundle them in a paid product, or use them in commercial work without
> written permission from the original author (Julian Schmidt / Sonic Potions).
>
> All redistributions must include the LXR source you modified or built from
> — this repo satisfies that by vendoring `lxr_dsp/` rather than fetching it.
> The wrapper code under `plugins/` is yours to license however you prefer
> (or you can put it under the same terms).

## Status

- ✅ LXR DSP source vendored (43 files) and compiles unmodified on Linux/macOS/Windows
- ✅ Compatibility shim replaces `stm32f4xx.h` references (just stdint typedefs)
- ✅ Hardware-only modules stubbed: `TriggerOut`, `SampleMemory`, `sequencer`, `MidiVoiceControl`, `MidiParser`, `ParameterArray`
- ✅ Hardware RNG (`random.c`) replaced with portable xorshift32 — see file header for the modification notice
- ✅ Smoke test triggers each of the four voice types and renders correct drum-like audio
- ✅ DrumVoice CLAP plugin (full working example)
- 🚧 Snare / Cymbal / HiHat plugins (templates provided — copy-and-adapt from DrumVoice)

## Architecture

```
+-------------------------+        +---------------------------+
| Host (DAW)              |        | LxrDrum.clap              |
|                         |        |                           |
|  MIDI note-on  ---------+------->|  PluginProcessor          |
|  parameter change  -----+------->|    AudioProcessorValueTreeState
|                         |        |    |                      |
|  audio out  <-----------+--------+    v                      |
|                         |        |  LxrDrumEngine (C++ glue) |
+-------------------------+        |    |                      |
                                   |    +--> writes voice struct fields
                                   |    +--> calls calcDrumVoiceSyncBlock
                                   |    +--> int16 -> float, 32-sample
                                   |         block accumulator
                                   +---------------------------+
                                              |
                                              v
                                   +---------------------------+
                                   |  lxr_dsp/  (vendored C)   |
                                   |    DrumVoice / Snare /    |
                                   |    CymbalVoice / HiHat    |
                                   |    + Oscillator / SVF /   |
                                   |    + LFO / Distortion /   |
                                   |    + transient gen / etc. |
                                   +---------------------------+
```

The LXR voices process at a fixed **32-sample block** with **int16** output. The
plugin wraps this in a small accumulator: when the host asks for N float
samples, the wrapper calls the LXR sync function as many times as needed,
converts each int16 result to float (`s / 32768.0f`), and emits into the
host buffer. Trailing samples are kept across host blocks so block size
mismatches between host and LXR are absorbed cleanly.

## Build

### Prerequisites
- CMake 3.22+
- A C/C++17 compiler (GCC, Clang, MSVC)
- Network access for the JUCE and clap-juce-extensions submodules

### One-time setup
```bash
git clone <this-repo> lxr-voices-clap
cd lxr-voices-clap
git submodule update --init --recursive
```
…or fetch JUCE 8 and clap-juce-extensions manually into `third_party/`.

### Configure & build
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

CLAP files land in `build/plugins/<voice>/<voice>_artefacts/Release/CLAP/`.

### Install (per-user)
- **Linux**: copy `LxrDrum.clap` to `~/.clap/`
- **macOS**: copy `LxrDrum.clap` to `~/Library/Audio/Plug-Ins/CLAP/`
- **Windows**: copy `LxrDrum.clap` to `%LOCALAPPDATA%\Programs\Common\CLAP\`

## Testing without a DAW

A standalone test driver is included that triggers each voice and writes
a 2-second WAV (`build/test/lxr_voices.wav`):

```bash
cmake --build build --target lxr_voice_test
./build/test/lxr_voice_test
```

## Project layout

```
lxr-voices-clap/
├── README.md                  -- this file
├── CMakeLists.txt             -- top-level: discovers JUCE, builds 4 plugins + test
├── lxr_dsp/                   -- vendored LXR source (license requires this)
│   ├── LXR_LICENSE.txt
│   ├── compat/stm32f4xx.h     -- shim: just <stdint.h>
│   ├── stubs/                 -- replacements for hardware-only modules
│   │   ├── TriggerOut.{c,h}
│   │   ├── ParameterArray.{c,h}
│   │   ├── MidiVoiceControl.{c,h}
│   │   ├── MidiParser.{c,h}
│   │   ├── sequencer.{c,h}
│   │   ├── valueShaper.h          (copied as-is, header-only)
│   │   └── MidiNoteNumbers.h      (copied as-is)
│   ├── SampleRom/SampleMemory.{c,h}  -- user-sample stubs (zeroed buffer)
│   ├── DSPAudio/              -- the real DSP engine (unmodified except random.c)
│   ├── config.h               -- one tweak: INCCM macros now respect prior defs
│   ├── datatypes.h
│   └── globals.h
├── plugins/
│   ├── shared/                -- common base class & helpers
│   │   ├── LxrEngineBase.h
│   │   └── BlockAccumulator.h
│   ├── drum/                  -- ✅ complete working example
│   │   ├── CMakeLists.txt
│   │   ├── PluginProcessor.{cpp,h}
│   │   └── DrumEngine.{cpp,h}
│   ├── snare/                 -- 🚧 template (mirror drum/)
│   ├── cymbal/                -- 🚧 template
│   └── hihat/                 -- 🚧 template
├── test/
│   └── lxr_voice_test.c       -- standalone smoke test
└── third_party/               -- JUCE + clap-juce-extensions submodules
```

## Porting the remaining three voices

Each voice follows the same pattern as `plugins/drum/`. The differences:

| Plugin    | LXR include           | Init       | Trigger fn                      | Process fn                         | Voice struct  |
|-----------|-----------------------|------------|---------------------------------|------------------------------------|---------------|
| LxrDrum   | `DrumVoice.h`         | `initDrumVoice()` | `Drum_trigger(0, vel, note)` | `calcDrumVoiceSyncBlock(0, ...)` | `voiceArray[0]` |
| LxrSnare  | `Snare.h`             | `Snare_init()`    | `Snare_trigger(vel, note)`   | `Snare_calcSyncBlock(...)`       | `snareVoice` (extern in Snare.c) |
| LxrCymbal | `CymbalVoice.h`       | `Cymbal_init()`   | `Cymbal_trigger(vel, note)`  | `Cymbal_calcSyncBlock(...)`      | `cymbalVoice` |
| LxrHiHat  | `HiHat.h`             | `HiHat_init()`    | `HiHat_trigger(vel, isOpen, note)` | `HiHat_calcSyncBlock(...)` | `hatVoice` |

To add a voice plugin:
1. `cp -r plugins/drum plugins/<voice>`
2. Update the `juce_add_plugin` call in the new CMakeLists.txt (name, plugin code, manufacturer code)
3. Replace include / init / trigger / process / struct references per the table
4. Adjust the parameter list in `PluginProcessor.cpp` for the voice's actual fields
5. Add the new directory to the top-level `CMakeLists.txt`

## Known limitations / quirks

- **Sample rate**: the LXR runs at 44100 Hz internally. We do *not* resample.
  If your host is at 48 kHz the voices will play 8.8% slower / lower than
  on hardware. Proper resampling is a TODO — for now stick to 44.1k sessions
  or ship a libsamplerate-based polyphase resampler in the wrapper.
- **Block size**: fixed at 32 samples internally. Host block size is decoupled
  via the accumulator, so any host block size works.
- **Polyphony**: each plugin instance is **monophonic**. Trigger a new note
  before the previous decays and the voice will retrigger. For polyphony,
  load multiple instances or extend `voiceArray[]` (it has 3 slots already).
- **User samples**: disabled. Factory wavetables (sine, saw, tri, rec) and
  the embedded crash sample work; SD-card user samples don't because we
  replaced the SD/flash backing with a zeroed buffer.
- **Tempo-synced LFO**: the LFO's rate-from-bpm path works if you call
  `lxr_setBpmFloat()` from the plugin's `prepareToPlay` / process callback
  with the host's reported tempo.

## Acknowledgments

All DSP credit goes to **Julian Schmidt** (Sonic Potions) and contributors
to the original LXR firmware. This project is a wrapper, not a re-implementation.
