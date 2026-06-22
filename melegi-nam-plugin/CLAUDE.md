# CLAUDE.md — MELEGI NAM Plugin

## What this is

A JUCE AudioProcessor plugin (VST3 + AU + Standalone) that runs custom ONNX or TorchScript
neural models through MELEGI's signal chain. It is the DAW-native extension of the MELEGI
product — same StyleDNA, same GROUXX fingerprint target, same export philosophy, but real-time
in a plugin host.

**Repo path:** `melegi-nam-plugin/` (monorepo sibling of `melegi/` and `melegi-app/`)

---

## Project laws (non-negotiable)

1. **No allocations on the audio thread.** `processBlock()` must not call `new`, `delete`,
   `malloc`, `std::vector::push_back`, `std::string` construction, or any JUCE method that
   allocates heap memory. Allocate in `prepareToPlay()`, free in `releaseResources()`.

2. **No blocking on the audio thread.** No mutexes, no file I/O, no system calls, no inference
   warmup. Model loading happens on a background `juce::Thread`; a `std::atomic<bool> modelReady`
   gate controls whether `processBlock` runs inference or passes dry audio through.

3. **Model hot-swap via lock-free pointer.** Use `std::atomic<NeuralModel*>` with
   `memory_order_acquire` / `memory_order_release` for zero-copy swap when a new model loads
   while the plugin is running.

4. **Buffer size agnostic.** JUCE's `processBlock` may deliver any block size the host chooses.
   Do not assume 512 or 1024 samples. Use `samplesPerBlock` from `prepareToPlay` only as an
   upper bound for pre-allocated scratch buffers.

5. **Sample-rate accuracy.** Cache `sampleRate` from `prepareToPlay`; pass it to every engine
   that needs it. Never hardcode 44100.

6. **Thread-safe parameter reads.** JUCE `AudioProcessorValueTreeState` parameters are read
   via `parameter->load()` (atomic) in `processBlock`. Never call GUI methods from the audio
   thread.

7. **Plugin ID stability.** The JUCE plugin GUID / 4-char code in `CMakeLists.txt` must not
   change after first release — DAW sessions reference it. If you add a new format target,
   add a new entry; never change `PLUGIN_CODE`.

8. **StyleDNA integration.** The plugin reads `melegi_engine`'s `StyleDNA` parameters (BPM,
   centroid target, shoegaze depth) from its `AudioProcessorValueTreeState` and passes them
   to the shared engine objects. It does not reimplement DSP that `melegi_engine` already owns.

9. **LUFS targets.** Export path uses ExportEngine presets: BeatStars −10 LUFS, SunoRef −14 LUFS,
   Logic −18 LUFS. The plugin's real-time gain staging aims for −18 LUFS headroom in the chain.

10. **Comments.** Only when the WHY is non-obvious. No docblocks. No task references in code.

---

## Build

```bash
# First time: fetch JUCE and ONNX Runtime
cmake -B build -DCMAKE_BUILD_TYPE=Release \
      -DJUCE_PATH=/path/to/JUCE \
      -DONNXRUNTIME_ROOT=/path/to/onnxruntime

# Subsequent builds
cmake --build build -j$(nproc)
```

Produces:
- `build/MelegiNAM_artefacts/VST3/MelegiNAM.vst3`
- `build/MelegiNAM_artefacts/AU/MelegiNAM.component` (macOS only)
- `build/MelegiNAM_artefacts/Standalone/MelegiNAM`

---

## File map

| File | Owns |
|------|------|
| `src/PluginProcessor.h` | AudioProcessor interface, APVTS, parameter layout |
| `src/PluginProcessor.cpp` | `prepareToPlay`, `processBlock`, `getStateInformation` |
| `src/NeuralModel.h` | ONNX / TorchScript inference wrapper interface |
| `src/NeuralModel.cpp` | Session management, input/output tensor handling |
| `src/PluginEditor.h/.cpp` | GUI — model loader, gain knobs, LUFS meter |
| `CMakeLists.txt` | JUCE targets, find_package for ONNX Runtime / LibTorch |
| `SHARED.md` | JUCE API cheat-sheet, ONNX Runtime / TorchScript API notes |
| `SKILL.md` | DSP constraints: buffer sizing, latency budget, thread model |
