# Voice Changer

Real-time voice modification app for Discord, Zoom, etc. Captures the microphone, applies effects (pitch shift, robot voice, etc.), and outputs to a virtual audio device that Discord uses as its microphone.

## Targets

- **Primary:** Windows 11
- **Secondary (nice to have):** Linux (PipeWire/PulseAudio)

## Tech stack

- **Language:** C++17 or later
- **Framework:** JUCE (GUI, audio device I/O, DSP helpers)
- **Build:** CMake (use JUCE's CMake API, `juce_add_gui_app`)
- **Pitch/formant shifting:** Signalsmith Stretch (header-only, low latency). Rubber Band is the fallback if quality is insufficient.
- **Robot effect:** own implementation (ring modulation, optional comb filter/bitcrush)
- **Virtual audio device (not bundled; users install it):**
    - Windows: VB-Cable (document the install; bundling is a stretch goal)
    - Linux: PipeWire/PulseAudio virtual sink created via CLI (document the commands)

**Licence:** the project is fully open source under **AGPLv3** (see `LICENSE`), as required to use JUCE's free licence. Any third-party dependency added must be AGPLv3-compatible (Signalsmith Stretch is MIT, which is fine).

## Architecture

```
Mic -> [audio thread: effect chain] -> output device (virtual cable, or headphones in test mode)
                  ^
        atomic parameters
                  |
        [GUI thread: controls, meters, device selection]
```

### Audio thread

- Input -> effect chain -> output, driven by the JUCE audio callback
- Effect chain order (initial): pitch/formant shift -> ring mod (robot) -> optional extras (distortion, reverb, delay)
- Each effect has a bypass and parameters stored as `std::atomic` values that the GUI writes and the audio thread reads
- Preallocate all buffers in `prepareToPlay` / `audioDeviceAboutToStart`

### GUI thread

- Device selection: input device, output device
- Effect controls: sliders and toggles per effect, presets
- Input/output level meters
- Test mode toggle (see below)

### Suggested layout

```
CMakeLists.txt
src/
  main.cc
  main_component.*       # GUI root
  audio_engine.*         # device management, audio callback, effect chain
  effects/
    effect.h             # common interface: Prepare(), Process(), Reset()
    bypassable_effect.*  # click-free bypass wrapper for any effect
    linear_smoother.h    # parameter smoothing (JUCE-free)
    pitch_shifter.*      # wraps Signalsmith Stretch
    ring_mod_robot.*
  presets.*              # JSON load/save (see Decisions for storage location)
                         # third-party deps (JUCE, Signalsmith Stretch) come
                         # from FetchContent, pinned by URL + SHA256
tests/                   # GoogleTest; effects only, no JUCE
docs/
```

## Real-time audio rules (strictly enforce)

In the audio callback, never:

- allocate or free memory (no `new`, `std::vector` resize, `std::string` construction)
- take locks or block (no mutexes, no I/O, no logging)
- call anything with unbounded or unknown execution time

Use atomics, lock-free queues/ring buffers, and preallocated storage for anything crossing the thread boundary. Prefer smoothed parameters (e.g. `juce::SmoothedValue`) to avoid zipper noise.

## Latency targets

- Round trip under about 30-40 ms
- Buffer size 128-256 samples at 48 kHz
- Windows: WASAPI low-latency shared mode first; ASIO is optional and has SDK licensing implications, so skip it initially
- Pitch shifting is the main latency risk; measure it and expose the cost of any look-ahead

## Test mode

Lets the user hear their processed voice.

1. **Phase 1 (implement first):** toggle that routes processed audio to the user's headphones instead of the virtual cable. Only one output device is active, so no clock-drift handling is needed. Discord hears nothing while testing.
2. **Phase 2 (later):** live monitoring, sending processed audio to the virtual cable and the headphones at once. This needs a second device with its own callback, a lock-free ring buffer between them, and drift compensation (small resampler) to prevent clicks, because the two devices run on separate clocks.

Default test mode to headphones and show a warning, since monitoring over speakers causes feedback into the mic.

## Effects reference

- **Pitch shift:** use Signalsmith Stretch, not naive resampling (which changes speed as well as pitch)
- **Formant shift:** separate control from pitch, so voices can sound bigger or smaller without a chipmunk effect
- **Robot:** ring modulation with a sine carrier (about 30-100 Hz), optional short comb filter, optional sample-rate reduction/bitcrush
- **Cheap extras:** distortion, delay, reverb

## Milestones

- [x] CMake + JUCE skeleton, window opens, device selection works, audio passes through unmodified
- [ ] Effect interface, pitch shifter, ring-mod robot, atomic parameters, bypasses
- [ ] GUI controls, level meters, presets
- [ ] Test mode (phase 1), then verify Discord picks up the virtual cable correctly on Windows 11
- [ ] Latency measurement and tuning
- [ ] Linux build and PipeWire/PulseAudio setup docs
- [ ] Test mode phase 2 (simultaneous monitoring with drift compensation)
- [ ] Optional: VST3/CLAP plugin hosting so users can load their own effects

## Out of scope for now

- Writing a custom virtual audio driver (kernel-mode signing makes this impractical)
- AI voice conversion (RVC or similar). It needs a capable GPU and adds noticeable latency, so treat it as a possible later phase.

## Conventions

- Keep DSP code independent of JUCE GUI types where practical so effects are testable standalone
- Add unit tests for effect processing (e.g. offline processing of known buffers)
- Keep the build reproducible: pin third-party versions
- When adding any code that runs in the audio callback, state in a comment that it is real-time safe
- Code and functionality should be developed using SOLID principles as much as possible. Along with a KISS philosophy (Keep It Simple, Stupid).
- Code should not be overly commented, only where it might be unclear upon reading it what the code is doing.
- C++ should be written adhering to Google's C++ styling guide (https://google.github.io/styleguide/cppguide.html). The code must be neat, succinct and clear.

## Decisions

- **Distribution and licence:** fully open source, with no commercial distribution. Licensed AGPLv3 to comply with JUCE's free licence (originally CC0 1.0, changed for JUCE compatibility).
- **VB-Cable:** document the install only; do not bundle it. Bundling is a stretch goal (check VB-Audio's redistribution terms first).
- **Presets:** stored as JSON files in the per-user app data directory, resolved via `juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)`:
    - Windows: `%APPDATA%\VoiceChanger\presets\`
    - Linux: `~/.config/voicechanger/presets/`