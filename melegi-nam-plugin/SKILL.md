# SKILL.md — DSP Rules for MelegiNAM Plugin

## Buffer sizing

| Constraint | Rule |
|------------|------|
| Max block size | Determined by host via `prepareToPlay(sr, maxSamples)`. Pre-allocate scratch buffers of this size. |
| Min block size | Hosts may send 1-sample blocks (REAPER with small buffer). Guard: `if (numSamples < 1) return;` |
| ONNX minimum | Most models need ≥64 samples for stable inference. Accumulate into an internal ring buffer and process in chunks if host delivers sub-64 blocks. |
| Ring buffer latency | `ringSize / sampleRate` seconds. Report this as `getTailLengthSeconds()` + `getLatencySamples()`. |

## Latency budget

Target: **≤ 5 ms added latency** at 44.1 kHz (≤220 samples).

- ONNX Runtime single-threaded inference on a 512-sample block: ~0.5–2 ms on modern CPU.
- If inference is slower, increase the ring buffer to the next power of 2 and report the latency.
- The JUCE host compensates for reported latency automatically in timeline-locked sessions (DAW playback). Live use (Standalone) has no compensation — keep it tight.

## Sample rate handling

```cpp
void PluginProcessor::prepareToPlay(double sr, int maxSamples) {
    sampleRate = sr;
    scratchIn.resize(maxSamples);
    scratchOut.resize(maxSamples);
    // Re-create DSP objects that bake in sample rate (filters, LFOs)
    shoegazeEngine.prepare(sr, maxSamples);
}
```

Never hardcode 44100. ShoegazeEngine and AnalysisEngine both take `sr` as a parameter.

## Thread model

```
┌─────────────┐     atomic ptr swap     ┌──────────────────┐
│ Audio thread│ ◄──────────────────────  │ Model-load thread│
│ processBlock│     modelReady flag      │ (juce::Thread)   │
│  (real-time)│                         │ loads, warms up  │
└─────────────┘                         └──────────────────┘
```

- Model-load thread: `loadModel()` → `session = new Ort::Session(...)` → warm-up forward pass →
  `activeModel.store(newModel)` → `modelReady.store(true)`
- Audio thread: `if (!modelReady.load()) { passThrough(); return; }` before inference

## Gain staging

```
Input gain (dB, APVTS "inputGain")
  → NeuralModel::process()
  → ShoegazeEngine CASH chain
  → K-weighted LUFS meter (display only, AnalyserNode-equivalent via JUCE FFT)
  → Output gain (dB, APVTS "outputGain")
  → Plugin output
```

Real-time LUFS metering: compute RMS per block, display as approximate LUFS. Full
BS.1770 K-weighting only needed for offline export (use `measureLufsK()` from AnalysisEngine
conceptually; adapt to block-by-block accumulation).

## GROUXX fingerprint in real-time

Spectral centroid target is 757 Hz. Display live centroid in the plugin GUI using
`juce::dsp::FFT` on the output buffer. Highlight in gold when within ±50 Hz, red otherwise.
This is cosmetic — do not alter the signal based on the live centroid reading.

## What NOT to do in processBlock

```cpp
// BAD — allocates
std::vector<float> tmp(numSamples);
juce::String s = "processing";
session.Run(...);  // first call — JIT compiles; only safe after warmup

// BAD — blocks
std::lock_guard<std::mutex> lock(mtx);
model->load("new_model.onnx");

// GOOD — pre-allocated, lock-free
std::copy(buf.getReadPointer(0), buf.getReadPointer(0) + numSamples, scratchIn.data());
if (auto* m = activeModel.load(std::memory_order_acquire))
    m->process(scratchIn.data(), scratchOut.data(), numSamples);
std::copy(scratchOut.data(), scratchOut.data() + numSamples, buf.getWritePointer(0));
```

## Model file format rules

- ONNX models: `.onnx` extension, opset ≥ 13, single input + single output
- TorchScript: `.pt` extension, exported via `torch.jit.trace` or `torch.jit.script`
- Both: store absolute path in APVTS state; re-load on `setStateInformation`
- Model files are NOT bundled in the plugin binary — user selects them via file dialog

## Debugging inference quality

After loading a model, run `AnalysisEngine::spectralCentroid()` on a 1-second test tone
passed through the model. If centroid drifts more than 20% from the source, the model may
have DC offset or gain issues. Log this to `juce::Logger::writeToLog()` during development.
