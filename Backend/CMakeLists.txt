set(PROJECT_NAME PlaynPlug)
set (PLUGIN_VERSION 1.0.0)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

set(CMAKE_CXX_STANDARD 17)

set(HARDENED_RUNTIME_ENABLED NO)
set(HARDENED_RUNTIME_OPTIONS "com.apple.security.device.audio-input")

set(JUCE_ENABLE_MODULE_SOURCE_GROUPS ON CACHE BOOL "" FORCE)
set_property(GLOBAL PROPERTY USE_FOLDERS YES)

cmake_policy(SET CMP0091 NEW)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES arm64 x86_64) # Universal binary
endif()

cmake_minimum_required(VERSION 3.22)

project(${PROJECT_NAME} VERSION 0.0.1 LANGUAGES CXX C)

add_subdirectory(Libraries/JUCE)

set(PLUGIN_COMPILE_DEFINITIONS
        JUCE_DONT_DECLARE_PROJECTINFO=0
        DONT_SET_USING_JUCE_NAMESPACE=1
        JUCE_ALLOW_STATIC_NULL_VARIABLES=0
        JUCE_ENABLE_REPAINT_DEBUGGING=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_REPORT_APP_USAGE=0
        JUCE_LOG_ASSERTIONS=1
        JUCE_STRICT_REFCOUNTEDPOINTER=1
        JUCE_WEB_BROWSER=1
        JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1
        JUCE_USE_COREIMAGE_LOADER=0
        WIN32_LEAN_AND_MEAN=1
        PLUGIN_VERSION="${PLUGIN_VERSION}"
)

set(libs
        juce_audio_plugin_client
        juce_audio_utils
)

list(APPEND INCLUDE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
list(APPEND INCLUDE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/Source")
list(APPEND INCLUDE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/Libraries")

juce_add_plugin(${PROJECT_NAME}
        VERSION                     ${PLUGIN_VERSION}
        COMPANY_NAME                "PlaynPlug"
        IS_SYNTH                    FALSE
        NEEDS_MIDI_INPUT            TRUE               # Does the plugin need midi input?
        NEEDS_MIDI_OUTPUT           FALSE              # Does the plugin need midi output?
        IS_MIDI_EFFECT              FALSE                 # Is this plugin a MIDI effect?
        EDITOR_WANTS_KEYBOARD_FOCUS TRUE    # Does the editor need keyboard focus?
        COPY_PLUGIN_AFTER_BUILD     TRUE        # Should the plugin be installed to a default location after building?
        PLUGIN_MANUFACTURER_CODE    RoVu               # A four-character manufacturer id with at least one upper-case character
        PLUGIN_CODE                 EdPl                            # A unique four-character plugin id with exactly one upper-case character
        FORMATS                     AU VST3                  # The formats to build. Other valid formats are: AAX Unity VST AU AUv3
        PRODUCT_NAME                "PlaynPlug"
)

juce_generate_juce_header(${PROJECT_NAME})

file(GLOB_RECURSE PLUGIN_SOURCES "Source/*.cpp")

target_sources(${PROJECT_NAME}
        PRIVATE
        ${PLUGIN_SOURCES}
)

target_compile_definitions(${PROJECT_NAME}
        PUBLIC
        ${PLUGIN_COMPILE_DEFINITIONS}
)

target_link_libraries(${PROJECT_NAME}
        PRIVATE
        # AudioPluginData           # If we'd created a binary data target, we'd link to it here
        PUBLIC
        ${libs}
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

target_include_directories(${PROJECT_NAME} PUBLIC "$<BUILD_INTERFACE:${INCLUDE_DIRECTORY}>")
