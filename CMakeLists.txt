cmake_minimum_required(VERSION 3.21)

project(qwen3asr4j C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(BUILD_SHARED_LIBS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

option(GGML_CUDA "ggml: enable CUDA backend" OFF)
option(GGML_VULKAN "ggml: enable Vulkan backend" OFF)
option(GGML_METAL "ggml: enable Metal backend" OFF)

set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_NO_SOVERWRITE ON)
set(CMAKE_INSTALL_CREATE_SYMLINK OFF)
add_definitions(-D_FILE_OFFSET_BITS=64)

if (UNIX)
    if (APPLE)
        set(CMAKE_SHARED_LIBRARY_SUFFIX ".dylib")
        set(CMAKE_SHARED_LIBRARY_SUFFIX_USE_VERSION OFF)
    else ()
        set(CMAKE_SHARED_LIBRARY_SUFFIX ".so")
        set(CMAKE_SHARED_LIBRARY_SUFFIX_USE_VERSION OFF)
    endif ()
    set(CMAKE_INSTALL_FOLLOW_SYMLINK_CHAIN OFF CACHE BOOL "Do not follow symlinks on install" FORCE)
endif ()

if (MSVC)
    add_compile_options(/utf-8)
endif ()

set(JAVA_AWT_LIBRARY NotNeeded)
set(JAVA_JVM_LIBRARY NotNeeded)

find_package(JNI REQUIRED)
if (JNI_FOUND)
    message(STATUS "JNI_INCLUDE_DIRS=${JNI_INCLUDE_DIRS}")
endif ()
find_package(Threads REQUIRED)

if (GGML_VULKAN)
    find_package(Vulkan REQUIRED)
endif ()

add_subdirectory(ggml)

if(WIN32)
    target_compile_definitions(ggml-base PRIVATE _DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR)
endif()

add_library(qwen3asr4j SHARED
        src/main/cpp/qwen3asr4j.cpp
        src/main/cpp/qwen3asr/src/audio_encoder.cpp
        src/main/cpp/qwen3asr/src/forced_aligner.cpp
        src/main/cpp/qwen3asr/src/gguf_loader.cpp
        src/main/cpp/qwen3asr/src/mel_spectrogram.cpp
        src/main/cpp/qwen3asr/src/qwen3_asr.cpp
        src/main/cpp/qwen3asr/src/text_decoder.cpp
)

if (GGML_VULKAN)
    target_link_libraries(qwen3asr4j PRIVATE ggml ggml-base ggml-cpu ggml-vulkan Threads::Threads Vulkan::Vulkan)
    target_include_directories(qwen3asr4j PRIVATE ${Vulkan_INCLUDE_DIRS})
elseif (GGML_CUDA)
    target_compile_definitions(qwen3asr4j PUBLIC GGML_USE_CUDA)
    target_link_libraries(qwen3asr4j PRIVATE ggml ggml-base ggml-cpu ggml-cuda Threads::Threads)
else ()
    target_link_libraries(qwen3asr4j PRIVATE ggml ggml-base ggml-cpu Threads::Threads)
endif ()

if (APPLE)
    target_link_libraries(qwen3asr4j PRIVATE "-framework Accelerate")
endif ()

target_include_directories(qwen3asr4j PRIVATE
        ${JNI_INCLUDE_DIRS}
        ggml/include
        ggml/src
        src/main/cpp/qwen3asr/include
)