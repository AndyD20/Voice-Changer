#include "audio_engine.h"

#include <algorithm>

namespace voice_changer {
namespace {

constexpr int kInputChannels = 1;
constexpr int kOutputChannels = 2;
constexpr double kPreferredSampleRate = 48000.0;
constexpr int kPreferredBufferSize = 256;

void ClearOutputs(float* const* outputs, int num_channels, int num_samples) {
  for (int channel = 0; channel < num_channels; ++channel) {
    if (outputs[channel] != nullptr) {
      std::fill_n(outputs[channel], num_samples, 0.0f);
    }
  }
}

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
  if (mono_.empty()) {
    ClearOutputs(output_channel_data, num_output_channels, num_samples);
    return;
  }

  const float* input =
      num_input_channels > 0 ? input_channel_data[0] : nullptr;
  const int max_block = static_cast<int>(mono_.size());

  // Chunked so a device block larger than prepared for cannot overflow.
  for (int start = 0; start < num_samples; start += max_block) {
    const int block = std::min(max_block, num_samples - start);

    if (input != nullptr) {
      std::copy_n(input + start, block, dry_.data());
    } else {
      std::fill_n(dry_.data(), block, 0.0f);
    }
    std::copy_n(dry_.data(), block, mono_.data());

    input_meter_.Process(dry_.data(), block);
    for (Effect* effect : chain_) effect->Process(mono_.data(), block);
    output_meter_.Process(mono_.data(), block);
    monitor_mixer_.Process(dry_.data(), mono_.data(), block);

    // The mono mix is copied to every output channel.
    for (int channel = 0; channel < num_output_channels; ++channel) {
      if (output_channel_data[channel] != nullptr) {
        std::copy_n(mono_.data(), block, output_channel_data[channel] + start);
      }
    }
  }
}

void AudioEngine::audioDeviceAboutToStart(juce::AudioIODevice* device) {
  const double sample_rate = device->getCurrentSampleRate();
  const int block_size = device->getCurrentBufferSizeSamples();

  dry_.assign(static_cast<size_t>(block_size), 0.0f);
  mono_.assign(static_cast<size_t>(block_size), 0.0f);
  for (Effect* effect : chain_) effect->Prepare(sample_rate, block_size);
  monitor_mixer_.Prepare(sample_rate);
}

void AudioEngine::audioDeviceStopped() {}

}  // namespace voice_changer
