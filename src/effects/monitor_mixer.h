#ifndef VOICE_CHANGER_SRC_EFFECTS_MONITOR_MIXER_H_
#define VOICE_CHANGER_SRC_EFFECTS_MONITOR_MIXER_H_

#include <atomic>

#include "effects/linear_smoother.h"

namespace voice_changer {

// Mixes the unprocessed input and the processed output, each of which can be
// switched on or off without clicks. By default only the output is heard.
class MonitorMixer {
 public:
  // Safe to call from any thread.
  void SetInputEnabled(bool enabled) { input_enabled_.store(enabled); }
  void SetOutputEnabled(bool enabled) { output_enabled_.store(enabled); }
  bool IsInputEnabled() const { return input_enabled_.load(); }
  bool IsOutputEnabled() const { return output_enabled_.load(); }

  void Prepare(double sample_rate);
  // Writes the mix into processed.
  void Process(const float* input, float* processed, int num_samples);

 private:
  std::atomic<bool> input_enabled_{false};
  std::atomic<bool> output_enabled_{true};
  LinearSmoother input_gain_;
  LinearSmoother output_gain_;
};

}  // namespace voice_changer

#endif  // VOICE_CHANGER_SRC_EFFECTS_MONITOR_MIXER_H_
