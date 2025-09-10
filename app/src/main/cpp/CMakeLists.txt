# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html.
# For more examples on how to use CMake, see https://github.com/android/ndk-samples.

# Sets the minimum CMake version required for this project.
cmake_minimum_required(VERSION 3.22.1)

# Declares the project name. The project name can be accessed via ${ PROJECT_NAME},
# Since this is the top level CMakeLists.txt, the project name is also accessible
# with ${CMAKE_PROJECT_NAME} (both CMake variables are in-sync within the top level
# build script scope).
project("openclandroidexample")

# Creat es and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.
#
# In this top level CMakeLists.txt, ${CMAKE_PROJECT_NAME} is used to define
# the target library name; in the sub-module's CMakeLists.txt, ${PROJECT_NAME}
# is preferred for the same purpose.
#
# In order to load a library into your app from Java/Kotlin, you must call
# System.loadLibrary() and pass the name of the library defined here;
# for GameActivity/NativeActivity derived applications, the same library name must be
# used in the AndroidManifest.xml file.
# Find and link the OpenCL library

####libOpebnCLxxx.so are copied from various location
### armXYZ : from /vendor/lib/ folder on phone (nothing phone)
### x86_64 : from linux dist (desktop) ...
# you can dl those from here too https://packages.debian.org/search?searchon=contents&keywords=libopenCL.so&mode=filename&suite=stable&arch=any
# testing was done only on arm64

# another intresting read https://community.khronos.org/t/opencl-is-not-able-to-query-a-platform-on-an-android-device/109878/16

add_library(${CMAKE_PROJECT_NAME} SHARED
        # List C/C++ source files with relative paths to this CMakeLists.txt.
        native-lib.cpp)

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/OpenCL
)


add_library(OpenCL SHARED IMPORTED)

if(ANDROID_ABI STREQUAL "arm64-v8a")
    # Logic for 64-bit ARM
    message(STATUS "Configuring for arm64-v8a")
    set_target_properties(OpenCL PROPERTIES
            IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/OpenCL/libOpenCL64.so)
elseif(ANDROID_ABI STREQUAL "armeabi-v7a")
    # Logic for 32-bit ARM
    message(STATUS "Configuring for armeabi-v7a")
    set_target_properties(OpenCL PROPERTIES
            IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/OpenCL/libOpenCL32.so)
elseif(ANDROID_ABI STREQUAL "x86_64")
    # Logic for 64-bit x86
    message(STATUS "Configuring for x86_64")
    set_target_properties(OpenCL PROPERTIES
            IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/OpenCL/libOpenCLx86_64.so)
endif()

# Specifies libraries CMake should link to your target library. You
# can link libraries from various origins, such as libraries defined in this
# build script, prebuilt third-party libraries, or Android system libraries.
if(NOT BUILD_ENGINE_CLI_DESKTOP)
target_link_libraries(${CMAKE_PROJECT_NAME}
        # List libraries link to the target library
        OpenCL
        android
        log)
elseif(BUILD_ENGINE_CLI_DESKTOP)
    target_link_libraries(${CMAKE_PROJECT_NAME}
            # List libraries link to the target library for desktop (no android system)
            OpenCL)
endif()



#####CLI

# Engine library
add_library(engine STATIC
        native-lib.cpp
)
target_include_directories(engine PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/OpenCL
)
# CLI binary build (optional)
option(BUILD_ENGINE_CLI "Build standalone CLI binary" OFF)

if(BUILD_ENGINE_CLI)
    add_definitions(-DBUILD_ENGINE_CLI)
    # Map Android ABI to a short arch name for CLI
    if(ANDROID_ABI STREQUAL "arm64-v8a")
        set(arch "arm64")
    elseif(ANDROID_ABI STREQUAL "armeabi-v7a")
        set(arch "arm32")
    elseif(ANDROID_ABI STREQUAL "x86_64")
        set(arch "x86_64")
    else()
        message(FATAL_ERROR "Unsupported ABI for CLI: ${ANDROID_ABI}")
    endif()
    set(CLI_TARGET_NAME "${CMAKE_PROJECT_NAME}_cli_${arch}")
    message(STATUS "Building CLI: ${CLI_TARGET_NAME}")

    add_executable(${CLI_TARGET_NAME} main.cpp)
    target_compile_definitions(${CLI_TARGET_NAME} PRIVATE ENGINE_CLI)
    target_link_libraries(${CLI_TARGET_NAME} PRIVATE OpenCL log engine)
endif()

# CLI binary build (optional)
option(BUILD_ENGINE_CLI_DESKTOP "Build standalone CLI binary for desktop" OFF)

if(BUILD_ENGINE_CLI_DESKTOP)
    add_definitions(-DBUILD_ENGINE_CLI_DESKTOP)
    # we target linux or windows (x86_64) only
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)  # check 64-bit
        if(WIN32)
            message(STATUS "Building for Windows 64-bit")
            set(HOST_OS "win64")
            set(DLL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/OpenCL/OpenCL.dll")
            # Path to your import library (.lib)
            set(LIB_PATH "${CMAKE_CURRENT_SOURCE_DIR}/OpenCL/OpenCL.lib")
            set_target_properties(OpenCL PROPERTIES
                    IMPORTED_LOCATION "${DLL_PATH}"
                    IMPORTED_IMPLIB "${LIB_PATH}"  # <- This fixes your error
            )
        elseif(UNIX AND NOT APPLE)
            message(STATUS "Building for Linux 64-bit")
            set(HOST_OS "linux64")
            #TODO: probably need some review (either for linux or android above)
            #set_target_properties(OpenCL PROPERTIES
            #        IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/OpenCL/libOpenCL_${HOST_OS}.so)
            set_target_properties(OpenCL PROPERTIES
                    IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/OpenCL/libOpenCLx86_64.so)
        else()
            message(FATAL_ERROR "Unsupported 64-bit OS")
        endif()
    else()
        message(FATAL_ERROR "Only 64-bit desktop builds are supported")
    endif()
    set(CLI_TARGET_NAME "${CMAKE_PROJECT_NAME}_cli")
    message(STATUS "Building CLI: ${CLI_TARGET_NAME}")
    add_executable(${CLI_TARGET_NAME} main.cpp)
    target_compile_definitions(${CLI_TARGET_NAME} PRIVATE ENGINE_CLI)
    target_link_libraries(${CLI_TARGET_NAME} PRIVATE OpenCL engine)
endif()