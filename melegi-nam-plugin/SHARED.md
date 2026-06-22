# SHARED.md — RTNeural · JUCE · ONNX Runtime · TorchScript

Quick-reference specs for the inference and plugin-host layers.

---

## JUCE AudioProcessor lifecycle

```
prepareToPlay(sampleRate, samplesPerBlock)
  → allocate buffers, create DSP objects, reset state, start model-load thread

processBlock(AudioBuffer<float>&, MidiBuffer&)
  → NO heap alloc, NO blocking; read atomics, run inference, apply gain

releaseResources()
  → deallocate everything allocated in prepareToPlay

getStateInformation(MemoryBlock&) / setStateInformation(const void*, int)
  → serialize / deserialize APVTS state (model path, gain, LUFS target)
```

### AudioProcessorValueTreeState (APVTS) quick ref

```cpp
// Declaration in PluginProcessor.h
juce::AudioProcessorValueTreeState apvts;

// Parameter layout (constructed once in initializer list)
static juce::AudioProcessorValueTreeState::ParameterLayout createParams();

// Read on audio thread (atomic)
float gain = *apvts.getRawParameterValue("inputGain");

// Slider attachment in editor
juce::AudioProcessorValueTreeState::SliderAttachment gainAttach;
gainAttach = { apvts, "inputGain", gainSlider };
```

### Thread-safe model swap

```cpp
// In PluginProcessor.h
std::atomic<NeuralModel*> activeModel { nullptr };

// Audio thread read
if (auto* m = activeModel.load(std::memory_order_acquire))
    m->process(block);

// Background thread write (after new model fully loaded + warmed up)
auto* old = activeModel.exchange(newModel, std::memory_order_release);
delete old;  // safe — audio thread has already moved past the pointer
```

---

## ONNX Runtime (C++ API)

**Headers:** `#include <onnxruntime/core/session/onnxruntime_cxx_api.h>`  
**Link:** `-lonnxruntime`

### Session lifecycle

```cpp
Ort::Env env { ORT_LOGGING_LEVEL_WARNING, "MelegiNAM" };

Ort::SessionOptions opts;
opts.SetIntraOpNumThreads(1);           // audio plugin — single-threaded inference
opts.SetGraphOptimizationLevel(ORT_ENABLE_ALL);

Ort::Session session { env, modelPath.c_str(), opts };

// Input / output names (retrieve once at load time, cache them)
Ort::AllocatorWithDefaultOptions alloc;
std::string inName  = session.GetInputNameAllocated(0, alloc).get();
std::string outName = session.GetOutputNameAllocated(0, alloc).get();

// Shape: [1, numSamples, numFeatures] — adjust to your model's contract
std::array<int64_t, 3> shape { 1, numSamples, 1 };
Ort::Value inTensor = Ort::Value::CreateTensor<float>(
    alloc.GetInfo(), audioData.data(), audioData.size(), shape.data(), 3);

const char* inNames[]  = { inName.c_str() };
const char* outNames[] = { outName.c_str() };
auto outputs = session.Run(Ort::RunOptions{nullptr}, inNames, &inTensor, 1,
                           outNames, 1);

float* out = outputs[0].GetTensorMutableData<float>();
```

### Pre-allocated input buffer (audio thread safe)

```cpp
// Allocate once in prepareToPlay:
Ort::MemoryInfo memInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
inputBuffer.resize(maxBlockSize);          // std::vector<float> — no alloc in processBlock
// In processBlock: memcpy into inputBuffer, then CreateTensor pointing at inputBuffer.data()
```

---

## TorchScript (LibTorch C++)

**Headers:** `#include <torch/script.h>`  
**Link:** LibTorch shared libs (see CMake section below)

```cpp
torch::jit::script::Module model = torch::jit::load(modelPath);
model.eval();

// Warm up off audio thread — essential before first processBlock call
{
    auto dummy = torch::zeros({1, 1, 512});
    model.forward({dummy});
}

// Inference (processBlock — pre-allocated tensors only)
// Use torch::from_blob to wrap existing float* without copy:
auto inTensor = torch::from_blob(audioPtr, {1, 1, numSamples});
auto out = model.forward({inTensor}).toTensor();
float* outPtr = out.data_ptr<float>();
```

**Gotcha:** `torch::jit::load` and `model.forward` are NOT real-time safe during the first
call (JIT compilation). Always warm up on a background thread before `modelReady.store(true)`.

---

## CMake: finding JUCE + inference backends

```cmake
# JUCE — add_subdirectory or FetchContent
list(APPEND CMAKE_PREFIX_PATH ${JUCE_PATH})
add_subdirectory(${JUCE_PATH} JUCE EXCLUDE_FROM_ALL)

# ONNX Runtime
find_library(ONNXRUNTIME_LIB onnxruntime HINTS ${ONNXRUNTIME_ROOT}/lib)
include_directories(${ONNXRUNTIME_ROOT}/include)

# LibTorch (optional — only if USE_TORCHSCRIPT=ON)
if(USE_TORCHSCRIPT)
    find_package(Torch REQUIRED HINTS ${TORCH_INSTALL_PREFIX})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")
endif()

# Plugin target
juce_add_plugin(MelegiNAM
    PLUGIN_MANUFACTURER_CODE Mlgi
    PLUGIN_CODE MNam
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "MelegiNAM"
    COMPANY_NAME "Chibitek Labs"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    VST3_CATEGORIES "Fx" "Distortion"
)

target_link_libraries(MelegiNAM PRIVATE
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_dsp
    melegi_engine        # MELEGI C++ engine (StyleDNA, AnalysisEngine, ShoegazeEngine)
    ${ONNXRUNTIME_LIB}
)
```

---

## MELEGI engine integration

The plugin links `melegi_engine` (built from `../melegi/`) and uses these types directly:

| Header | Used for |
|--------|---------|
| `StyleDNA.hpp` | Parameter preset — `makeGROUXX()` as plugin default |
| `AnalysisEngine.hpp` | Offline centroid measurement after model inference |
| `ShoegazeEngine.hpp` | Post-inference CASH processing chain (FDN reverb, tape sat) |
| `ExportEngine.hpp` | Normalisation to BeatStars/SunoRef/Logic LUFS targets |

CMake path: `../melegi/` relative to `melegi-nam-plugin/`. The plugin's `CMakeLists.txt`
adds `../melegi` as a subdirectory and links `melegi_engine`.

---

## Model contract (expected tensor shapes)

| Backend | Input | Output | Notes |
|---------|-------|--------|-------|
| ONNX | `[1, N, 1]` float32 | `[1, N, 1]` float32 | N = block size |
| TorchScript | `[1, 1, N]` float32 | `[1, 1, N]` float32 | channels-first |

Both are negotiable — update `NeuralModel::process()` to match your export's actual contract.
The shapes above are the convention for a simple amp-sim / effect network.
