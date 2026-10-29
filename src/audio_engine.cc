#include "audio_engine.h"

#include <algorithm>

namespace voice_changer {
namespace {

constexpr int kInputChannels = 1;
constexpr int kOutputChannels = 2;
constexpr double kPreferredSampleRate = 48000.0;
constexpr int kPreferredBufferSize = 256;

}  // namespace

// On Windows, JUCE opens WASAPI shared mode ("Windows Audio") by default.
AudioEngine::AudioEngine() {
  juce::AudioDeviceManager::AudioDeviceSetup preferred;
  preferred.sampleRate = kPreferredSampleRate;
  preferred.bufferSize = kPreferredBufferSize;

  const juce::String error = device_manager_.initialise(
      kInputChannels, kOutputChannels, /*savedState=*/nullptr,
      /*selectDefaultDeviceOnFailure=*/true, /*preferredDefaultDeviceName=*/{},
      &preferred);
  jassert(error.isEmpty());

  device_manager_.addAudioCallback(this);
}

AudioEngine::~AudioEngine() { device_manager_.removeAudioCallback(this); }

// Real-time safe: no allocation, locking or blocking.
void AudioEngine::audioDeviceIOCallbackWithContext(
    const float* const* input_channel_data, int num_input_channels,
    float* const* output_channel_data, int num_output_channels,
    int num_samples, const juce::AudioIODeviceCallbackContext&) {
  // The mono mic signal is copied to every output channel.
  const float* input =
      num_input_channels > 0 ? input_channel_data[0] : nullptr;

  for (int channel = 0; channel < num_output_channels; ++channel) {
    float* output = output_channel_data[channel];
    if (output == nullptr) continue;

    if (input != nullptr) {
      std::copy_n(input, num_samples, output);
    } else {
      std::fill_n(output, num_samples, 0.0f);
    }
  }
}

void AudioEngine::audioDeviceAboutToStart(juce::AudioIODevice*) {}

void AudioEngine::audioDeviceStopped() {}

}  // namespace voice_changer
