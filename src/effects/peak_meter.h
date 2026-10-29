#ifndef VOICE_CHANGER_SRC_EFFECTS_PEAK_METER_H_
#define VOICE_CHANGER_SRC_EFFECTS_PEAK_METER_H_

#include <algorithm>
#include <atomic>
#include <cmath>

namespace voice_changer {

// Tracks the peak sample level between reads: the audio thread calls Process
// and the GUI thread calls TakePeak.
class PeakMeter {
 public:
  // Real-time safe: lock-free, no allocation.
  void Process(const float* samples, int num_samples) {
    float peak = 0.0f;
    for (int i = 0; i < num_samples; ++i) {
      peak = std::max(peak, std::abs(samples[i]));
    }
    float stored = peak_.load();
    while (peak > stored && !peak_.compare_exchange_weak(stored, peak)) {
    }
  }

  // Returns the peak since the previous call and starts a new measurement.
  float TakePeak() { return peak_.exchange(0.0f); }

 private:
  std::atomic<float> peak_{0.0f};
};

}  // namespace voice_changer

#endif  // VOICE_CHANGER_SRC_EFFECTS_PEAK_METER_H_
