cmake_minimum_required(VERSION 3.22)

project(VoiceChanger VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

FetchContent_Declare(
  juce
  URL https://github.com/juce-framework/JUCE/archive/refs/tags/9.0.4.tar.gz
  URL_HASH SHA256=e2bee25d5c776118aea550ad168a3dc9520c0669a79bd9f5a72b9f404ee1c40a
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
  PATCH_COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/patch_juce.cmake)
FetchContent_MakeAvailable(juce)

juce_add_gui_app(VoiceChanger
  PRODUCT_NAME "Voice Changer"
  COMPANY_NAME "Voice Changer"
  BUNDLE_ID "org.voicechanger.app"
  MICROPHONE_PERMISSION_ENABLED TRUE
  MICROPHONE_PERMISSION_TEXT "Voice Changer needs the microphone to process your voice.")

target_sources(VoiceChanger
  PRIVATE
    src/audio_engine.cc
    src/main.cc
    src/main_component.cc)

target_compile_definitions(VoiceChanger
  PRIVATE
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:VoiceChanger,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:VoiceChanger,JUCE_VERSION>"
    JUCE_USE_CURL=0
    JUCE_WEB_BROWSER=0)

target_link_libraries(VoiceChanger
  PRIVATE
    juce::juce_audio_utils
  PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags)
