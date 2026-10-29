cmake_minimum_required(VERSION 3.25)

project(VoiceChanger VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(VOICE_CHANGER_BUILD_TESTS "Build the effect unit tests" ON)

include(FetchContent)

FetchContent_Declare(
  juce
  URL https://github.com/juce-framework/JUCE/archive/refs/tags/9.0.4.tar.gz
  URL_HASH SHA256=e2bee25d5c776118aea550ad168a3dc9520c0669a79bd9f5a72b9f404ee1c40a
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
  PATCH_COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/patch_juce.cmake)

# Declared before Signalsmith Stretch, which otherwise fetches it by git tag.
FetchContent_Declare(
  signalsmith-linear
  URL https://github.com/Signalsmith-Audio/linear/archive/refs/tags/0.6.4.tar.gz
  URL_HASH SHA256=2cb10d84b96c626255ab46c4aa4c80f64ae532be487063cff6a08903c1445a56
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
  SYSTEM)

FetchContent_Declare(
  signalsmith-stretch
  URL https://github.com/Signalsmith-Audio/signalsmith-stretch/archive/refs/tags/1.4.0.tar.gz
  URL_HASH SHA256=077235709ecf2a358545e3ca7bdf32859f70f5c9afb4493e078bd753c9aff284
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
  SYSTEM)

FetchContent_MakeAvailable(juce signalsmith-linear signalsmith-stretch)

# Effects are plain C++ with no JUCE dependency, so they test standalone.
add_library(voice_changer_effects STATIC
  src/effects/bypassable_effect.cc
  src/effects/pitch_shifter.cc
  src/effects/ring_mod_robot.cc)

target_include_directories(voice_changer_effects PUBLIC src)
target_link_libraries(voice_changer_effects PUBLIC signalsmith-stretch)
target_compile_options(voice_changer_effects
  PRIVATE $<IF:$<CXX_COMPILER_ID:MSVC>,/W4,-Wall -Wextra>
  PUBLIC $<$<CXX_COMPILER_ID:MSVC>:/bigobj>)

juce_add_gui_app(VoiceChanger
  PRODUCT_NAME "Voice Changer"
  COMPANY_NAME "Voice Changer"
  BUNDLE_ID "org.voicechanger.app"
  MICROPHONE_PERMISSION_ENABLED TRUE
  MICROPHONE_PERMISSION_TEXT "Voice Changer needs the microphone to process your voice.")

target_sources(VoiceChanger
  PRIVATE
    src/audio_engine.cc
    src/effects_panel.cc
    src/level_meter.cc
    src/main.cc
    src/main_component.cc
    src/preset_bar.cc
    src/presets.cc)

target_compile_definitions(VoiceChanger
  PRIVATE
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:VoiceChanger,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:VoiceChanger,JUCE_VERSION>"
    JUCE_USE_CURL=0
    JUCE_WEB_BROWSER=0)

target_link_libraries(VoiceChanger
  PRIVATE
    juce::juce_audio_utils
    voice_changer_effects
  PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags)

if(VOICE_CHANGER_BUILD_TESTS)
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.tar.gz
    URL_HASH SHA256=65fab701d9829d38cb77c14acdc431d2108bfdbf8979e40eb8ae567edf10b27c
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    SYSTEM)
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)

  enable_testing()
  include(GoogleTest)

  add_executable(voice_changer_tests
    tests/allocation_counter.cc
    tests/allocation_counter_test.cc
    tests/bypassable_effect_test.cc
    tests/linear_smoother_test.cc
    tests/peak_meter_test.cc
    tests/pitch_shifter_test.cc
    tests/ring_mod_robot_test.cc)

  target_link_libraries(voice_changer_tests
    PRIVATE voice_changer_effects GTest::gtest_main)
  gtest_discover_tests(voice_changer_tests)
endif()
