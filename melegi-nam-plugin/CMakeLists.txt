cmake_minimum_required(VERSION 3.22)
project(MelegiNAMPlugin VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ── Options ──────────────────────────────────────────────────────────────────
option(USE_TORCHSCRIPT "Enable TorchScript backend (requires LibTorch)" OFF)
set(JUCE_PATH "" CACHE PATH "Path to JUCE root directory")
set(ONNXRUNTIME_ROOT "" CACHE PATH "Path to ONNX Runtime root (include/ and lib/ subdirs)")

# ── JUCE ─────────────────────────────────────────────────────────────────────
if(NOT JUCE_PATH)
    message(FATAL_ERROR "Set -DJUCE_PATH=/path/to/JUCE")
endif()
add_subdirectory(${JUCE_PATH} ${CMAKE_BINARY_DIR}/JUCE EXCLUDE_FROM_ALL)

# ── MELEGI engine (sibling directory) ────────────────────────────────────────
add_subdirectory(../melegi ${CMAKE_BINARY_DIR}/melegi_engine)

# ── ONNX Runtime ─────────────────────────────────────────────────────────────
if(NOT ONNXRUNTIME_ROOT)
    message(FATAL_ERROR "Set -DONNXRUNTIME_ROOT=/path/to/onnxruntime")
endif()
find_library(ONNXRUNTIME_LIB
    NAMES onnxruntime
    HINTS ${ONNXRUNTIME_ROOT}/lib
    REQUIRED
)
add_library(OnnxRuntime SHARED IMPORTED)
set_target_properties(OnnxRuntime PROPERTIES
    IMPORTED_LOCATION ${ONNXRUNTIME_LIB}
    INTERFACE_INCLUDE_DIRECTORIES ${ONNXRUNTIME_ROOT}/include
)

# ── LibTorch (optional) ───────────────────────────────────────────────────────
if(USE_TORCHSCRIPT)
    if(NOT TORCH_INSTALL_PREFIX)
        message(FATAL_ERROR "Set -DTORCH_INSTALL_PREFIX=/path/to/libtorch when USE_TORCHSCRIPT=ON")
    endif()
    find_package(Torch REQUIRED HINTS ${TORCH_INSTALL_PREFIX})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")
endif()

# ── Plugin target ─────────────────────────────────────────────────────────────
juce_add_plugin(MelegiNAM
    PLUGIN_MANUFACTURER_CODE Mlgi
    PLUGIN_CODE MNam
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "MelegiNAM"
    COMPANY_NAME "Chibitek Labs"
    BUNDLE_ID "com.chibiteklabs.meleginam"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    VST3_CATEGORIES "Fx" "Distortion"
)

target_sources(MelegiNAM PRIVATE
    src/PluginProcessor.cpp
    src/NeuralModel.cpp
)

target_compile_definitions(MelegiNAM PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    $<$<BOOL:${USE_TORCHSCRIPT}>:MELEGI_USE_TORCHSCRIPT>
)

target_link_libraries(MelegiNAM PRIVATE
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
    melegi_engine
    OnnxRuntime
    $<$<BOOL:${USE_TORCHSCRIPT}>:${TORCH_LIBRARIES}>
)
