cmake_minimum_required(VERSION 3.21)

project(qwen3asr4j C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(BUILD_SHARED_LIBS ON)

option(GGML_CUDA "ggml: enable CUDA backend" OFF)
option(GGML_VULKAN "ggml: enable Vulkan backend" OFF)

if (WIN32 AND MINGW)
    set(GGML_WIN_VER "0x0601" CACHE STRING "Windows version" FORCE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_WIN32_WINNT=0x0601 -DNTDDI_VERSION=0x0601")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_WIN32_WINNT=0x0601 -DNTDDI_VERSION=0x0601")
    set(GGML_OPENMP OFF CACHE BOOL "Disable OpenMP" FORCE)
    add_compile_definitions(__MINGW32__)
endif ()

set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_NO_SOVERWRITE ON)
set(CMAKE_INSTALL_CREATE_SYMLINK OFF)
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

set(JAVA_AWT_LIBRARY NotNeeded)
set(JAVA_JVM_LIBRARY NotNeeded)

find_package(JNI REQUIRED)
if (JNI_FOUND)
    message(STATUS "JNI_INCLUDE_DIRS=${JNI_INCLUDE_DIRS}")
endif ()

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
    target_link_libraries(qwen3asr4j PRIVATE whisper ggml ggml-base ggml-cpu ggml-vulkan Vulkan::Vulkan)
    target_include_directories(qwen3asr4j PRIVATE ${Vulkan_INCLUDE_DIRS})
elseif (GGML_CUDA)
    target_compile_definitions(qwen3asr4j PUBLIC GGML_USE_CUDA)
    target_link_libraries(qwen3asr4j PRIVATE whisper ggml ggml-base ggml-cpu ggml-cuda)
else ()
    target_link_libraries(qwen3asr4j PRIVATE ggml ggml-base ggml-cpu)
endif ()

target_include_directories(qwen3asr4j PRIVATE
        ${JNI_INCLUDE_DIRS}
        ggml/include
        src/main/cpp
        src/main/cpp/qwen3asr/include
)


# Test
add_executable(test_qwen3asr_cpp
        src/test/cpp/test_qwen3asr.cpp
        src/main/cpp/qwen3asr/src/audio_encoder.cpp
        src/main/cpp/qwen3asr/src/forced_aligner.cpp
        src/main/cpp/qwen3asr/src/gguf_loader.cpp
        src/main/cpp/qwen3asr/src/mel_spectrogram.cpp
        src/main/cpp/qwen3asr/src/qwen3_asr.cpp
        src/main/cpp/qwen3asr/src/text_decoder.cpp
)

target_include_directories(test_qwen3asr_cpp PRIVATE
        ggml/include
        src/main/cpp/qwen3asr/include
)

target_link_libraries(test_qwen3asr_cpp PRIVATE
        ggml
)
