cmake_minimum_required(VERSION 3.20)
project(MelegiEngine VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

if(MSVC)
    add_compile_options(/W4 /WX- /O2 /arch:AVX2)
else()
    add_compile_options(-Wall -Wextra -O2 -ffast-math)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|amd64")
        add_compile_options(-march=native)
    endif()
endif()

find_package(Threads REQUIRED)

set(ENGINE_SOURCES
    src/AudioEngine.cpp
    src/AnalysisEngine.cpp
    src/TimingEngine.cpp
    src/AuthenticityEngine.cpp
    src/ShoegazeEngine.cpp
    src/GeoAstralEngine.cpp
    src/GenerationEngine.cpp
    src/AgentFramework.cpp
    src/ExportEngine.cpp
)

add_library(melegi_engine STATIC ${ENGINE_SOURCES})
target_include_directories(melegi_engine PUBLIC include)
target_link_libraries(melegi_engine PUBLIC Threads::Threads)
target_compile_features(melegi_engine PUBLIC cxx_std_20)

add_executable(melegi_pipeline main.cpp)
target_link_libraries(melegi_pipeline PRIVATE melegi_engine)

# Install
install(TARGETS melegi_pipeline DESTINATION bin)
install(TARGETS melegi_engine DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)
