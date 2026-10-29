#ifndef VOICE_CHANGER_SRC_EFFECTS_LINEAR_SMOOTHER_H_
#define VOICE_CHANGER_SRC_EFFECTS_LINEAR_SMOOTHER_H_

#include <algorithm>

namespace voice_changer {

// Ramps linearly towards a target value over a fixed time, to avoid zipper
// noise. Real-time safe.
class LinearSmoother {
 public:
  void Reset(double sample_rate, double ramp_seconds, float value) {
    ramp_samples_ = std::max(1, static_cast<int>(sample_rate * ramp_seconds));
    current_ = target_ = value;
    remaining_ = 0;
  }

  void SetTarget(float target) {
    if (target == target_) return;
    target_ = target;
    remaining_ = ramp_samples_;
    step_ = (target_ - current_) / static_cast<float>(ramp_samples_);
  }

  float Next() {
    if (remaining_ > 0) {
      current_ = --remaining_ == 0 ? target_ : current_ + step_;
    }
    return current_;
  }

  float current() const { return current_; }
  bool IsSmoothing() const { return remaining_ > 0; }

 private:
  int ramp_samples_ = 1;
  int remaining_ = 0;
  float current_ = 0.0f;
  float target_ = 0.0f;
  float step_ = 0.0f;
};

}  // namespace voice_changer

#endif  // VOICE_CHANGER_SRC_EFFECTS_LINEAR_SMOOTHER_H_
