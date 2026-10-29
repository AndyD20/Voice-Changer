# Applied to the JUCE source tree by FetchContent's PATCH_COMMAND.
#
# WASAPI shared mode: when the requested capture format is not supported
# exactly (e.g. 1 channel on a stereo headset mic), Windows returns its nearest
# format as a plain WAVEFORMATEX tagged WAVE_FORMAT_IEEE_FLOAT. JUCE only
# recognises float via WAVE_FORMAT_EXTENSIBLE, so it decodes the float samples
# as Int32, producing loud static. Not fixed upstream as of JUCE 9.0.4.

set(file "modules/juce_audio_devices/native/juce_WASAPI_windows.cpp")
set(old "const bool isFloat = format->Format.wFormatTag == WAVE_FORMAT_EXTENSIBLE && format->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;")
set(new "const bool isFloat = format->Format.wFormatTag == WAVE_FORMAT_IEEE_FLOAT || (format->Format.wFormatTag == WAVE_FORMAT_EXTENSIBLE && format->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT);")

file(READ "${file}" contents)
string(FIND "${contents}" "${old}" position)
if(position EQUAL -1)
  message(FATAL_ERROR "patch_juce.cmake: WASAPI float-format line not found; "
                      "check whether the JUCE version in use still needs this patch.")
endif()
string(REPLACE "${old}" "${new}" contents "${contents}")
file(WRITE "${file}" "${contents}")
