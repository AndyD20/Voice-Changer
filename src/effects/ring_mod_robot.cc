#include "effects/ring_mod_robot.h"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace voice_changer {
namespace {

constexpr double kDepthRampSeconds = 0.02;

}  // namespace

void RingModRobot::SetCarrierHz(float hz) {
  carrier_hz_.store(std::clamp(hz, kMinCarrierHz, kMaxCarrierHz));
}

void RingModRobot::SetDepth(float depth) {
  depth_.store(std::clamp(depth, 0.0f, 1.0f));
}

void RingModRobot::Prepare(double sample_rate, int /*max_block_size*/) {
  sample_rate_ = sample_rate;
  smoothed_depth_.Reset(sample_rate, kDepthRampSeconds, depth_.load());
  Reset();
}

// Real-time safe: no allocation, locking or blocking.
void RingModRobot::Process(float* samples, int num_samples) {
  const double phase_increment = carrier_hz_.load() / sample_rate_;
  smoothed_depth_.SetTarget(depth_.load());

  for (int i = 0; i < num_samples; ++i) {
    const float carrier =
        static_cast<float>(std::sin(2.0 * std::numbers::pi * phase_));
    const float depth = smoothed_depth_.Next();
    samples[i] *= 1.0f - depth + depth * carrier;

    phase_ += phase_increment;
    if (phase_ >= 1.0) phase_ -= 1.0;
  }
}

// Real-time safe: no allocation, locking or blocking.
void RingModRobot::Reset() { phase_ = 0.0; }

}  // namespace voice_changer
