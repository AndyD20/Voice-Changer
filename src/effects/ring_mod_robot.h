#ifndef VOICE_CHANGER_SRC_EFFECTS_RING_MOD_ROBOT_H_
#define VOICE_CHANGER_SRC_EFFECTS_RING_MOD_ROBOT_H_

#include <atomic>

#include "effects/effect.h"
#include "effects/linear_smoother.h"

namespace voice_changer {

// Robot voice: ring modulation of the input by a low-frequency sine carrier.
class RingModRobot final : public Effect {
 public:
  static constexpr float kMinCarrierHz = 20.0f;
  static constexpr float kMaxCarrierHz = 200.0f;
  static constexpr float kDefaultCarrierHz = 50.0f;

  // Safe to call from any thread. Values are clamped to their valid range.
  void SetCarrierHz(float hz);
  // 0 leaves the signal unchanged; 1 is full ring modulation.
  void SetDepth(float depth);

  float carrier_hz() const { return carrier_hz_.load(); }
  float depth() const { return depth_.load(); }

  void Prepare(double sample_rate, int max_block_size) override;
  void Process(float* samples, int num_samples) override;
  void Reset() override;

 private:
  std::atomic<float> carrier_hz_{kDefaultCarrierHz};
  std::atomic<float> depth_{1.0f};

  double sample_rate_ = 48000.0;
  double phase_ = 0.0;  // In cycles, [0, 1).
  LinearSmoother smoothed_depth_;
};

}  // namespace voice_changer

#endif  // VOICE_CHANGER_SRC_EFFECTS_RING_MOD_ROBOT_H_
