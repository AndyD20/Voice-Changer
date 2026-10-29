#ifndef VOICE_CHANGER_SRC_AUDIO_ENGINE_H_
#define VOICE_CHANGER_SRC_AUDIO_ENGINE_H_

#include <array>
#include <vector>

#include <juce_audio_devices/juce_audio_devices.h>

#include "effects/bypassable_effect.h"
#include "effects/peak_meter.h"
#include "effects/pitch_shifter.h"
#include "effects/ring_mod_robot.h"

namespace voice_changer {

// Owns the audio devices and runs the microphone through the effect chain:
// pitch/formant shift, then robot.
class AudioEngine final : public juce::AudioIODeviceCallback {
 public:
  AudioEngine();
  ~AudioEngine() override;

  AudioEngine(const AudioEngine&) = delete;
  AudioEngine& operator=(const AudioEngine&) = delete;

  juce::AudioDeviceManager& device_manager() { return device_manager_; }

  // Parameters and bypasses are atomic, so the GUI can set them directly.
  PitchShifter& pitch_shifter() { return pitch_shifter_; }
  RingModRobot& robot() { return robot_; }
  BypassableEffect& pitch_shifter_bypass() { return pitch_shifter_bypass_; }
  BypassableEffect& robot_bypass() { return robot_bypass_; }

  // Peak levels before and after the effect chain.
  PeakMeter& input_meter() { return input_meter_; }
  PeakMeter& output_meter() { return output_meter_; }

  void audioDeviceIOCallbackWithContext(
      const float* const* input_channel_data, int num_input_channels,
      float* const* output_channel_data, int num_output_channels,
      int num_samples,
      const juce::AudioIODeviceCallbackContext& context) override;
  void audioDeviceAboutToStart(juce::AudioIODevice* device) override;
  void audioDeviceStopped() override;

 private:
  // Declared before the device manager so they exist before audio starts.
  PitchShifter pitch_shifter_;
  RingModRobot robot_;
  BypassableEffect pitch_shifter_bypass_{pitch_shifter_};
  BypassableEffect robot_bypass_{robot_};
  const std::array<Effect*, 2> chain_{&pitch_shifter_bypass_, &robot_bypass_};
  PeakMeter input_meter_;
  PeakMeter output_meter_;
  std::vector<float> mono_;

  juce::AudioDeviceManager device_manager_;
};

}  // namespace voice_changer

#endif  // VOICE_CHANGER_SRC_AUDIO_ENGINE_H_
