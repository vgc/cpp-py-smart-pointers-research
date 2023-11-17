#pragma once

// Determine which compiler is being used
#if defined(__clang__)
#    define COMPILER_CLANG
#    define COMPILER_CLANG_MAJOR __clang_major__
#    define COMPILER_CLANG_MINOR __clang_minor__
#    define COMPILER_CLANG_PATCHLEVEL __clang_patchlevel__
#elif defined(__GNUC__) || defined(__GNUG__)
#    define COMPILER_GCC
#    define COMPILER_GCC_MAJOR __GNUC__
#    define COMPILER_GCC_MINOR __GNUC_MINOR__
#    define COMPILER_GCC_PATCHLEVEL __GNUC_PATCHLEVEL__
#elif defined(__INTEL_COMPILER)
#    define COMPILER_INTEL
#elif defined(_MSC_VER)
#    define COMPILER_MSVC
#    define COMPILER_MSVC_VERSION _MSC_VER
#endif

// Determine which operating system we're targetting
#if defined(_WIN32)
#    define OS_WINDOWS
#elif defined(__APPLE__)
#    include "TargetConditionals.h"
#    if TARGET_IPHONE_SIMULATOR
#        error "Unsupported platform: iPhone simulator"
#    elif TARGET_OS_IPHONE
#        error "Unsupported platform: iPhone"
#    elif TARGET_OS_MAC
#        define OS_MACOS
#    else
#        error "Unsupported platform: unknown Apple platform"
#    endif
#elif __linux__
#    define OS_LINUX
#elif __unix__
#    error "Unsupported platform: unknown Unix platform"
#elif defined(_POSIX_VERSION)
#    error "Unsupported platform: unknown Posix platform"
#else
#    error "Unsupported platform: unknown platform"
#endif

// Macros for exporting symbols
#if defined(OS_WINDOWS)
#    if defined(COMPILER_GCC) && COMPILER_GCC_MAJOR >= 4 || defined(COMPILER_CLANG)
#        define DLL_EXPORT __attribute__((dllexport))
#        define DLL_IMPORT __attribute__((dllimport))
#        define DLL_HIDDEN
#    else
#        define DLL_EXPORT __declspec(dllexport)
#        define DLL_IMPORT __declspec(dllimport)
#        define DLL_HIDDEN
#    endif
#elif defined(COMPILER_GCC) && COMPILER_GCC_MAJOR >= 4 || defined(COMPILER_CLANG)
#    define DLL_EXPORT __attribute__((visibility("default")))
#    define DLL_IMPORT __attribute__((visibility("default")))
#    define DLL_HIDDEN __attribute__((visibility("hidden")))
#else
#    define DLL_EXPORT
#    define DLL_IMPORT
#    define DLL_HIDDEN
#endif

#if defined(STATIC)
#    define API
#    define API_HIDDEN
#else
#    if defined(EXPORTS)
#        define API DLL_EXPORT
#    else
#        define API DLL_IMPORT
#    endif
#    define API_HIDDEN DLL_HIDDEN
#endif
