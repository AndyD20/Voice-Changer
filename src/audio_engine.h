#ifndef VOICE_CHANGER_SRC_AUDIO_ENGINE_H_
#define VOICE_CHANGER_SRC_AUDIO_ENGINE_H_

#include <juce_audio_devices/juce_audio_devices.h>

namespace voice_changer {

// Owns the audio devices and routes microphone input to the output device.
class AudioEngine final : public juce::AudioIODeviceCallback {
 public:
  AudioEngine();
  ~AudioEngine() override;

  AudioEngine(const AudioEngine&) = delete;
  AudioEngine& operator=(const AudioEngine&) = delete;

  juce::AudioDeviceManager& device_manager() { return device_manager_; }

  void audioDeviceIOCallbackWithContext(
      const float* const* input_channel_data, int num_input_channels,
      float* const* output_channel_data, int num_output_channels,
      int num_samples,
      const juce::AudioIODeviceCallbackContext& context) override;
  void audioDeviceAboutToStart(juce::AudioIODevice* device) override;
  void audioDeviceStopped() override;

 private:
  juce::AudioDeviceManager device_manager_;
};

}  // namespace voice_changer

#endif  // VOICE_CHANGER_SRC_AUDIO_ENGINE_H_
