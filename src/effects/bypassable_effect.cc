#include "effects/bypassable_effect.h"

#include <algorithm>

namespace voice_changer {
namespace {

constexpr double kCrossfadeSeconds = 0.01;

float WetTarget(bool bypassed) { return bypassed ? 0.0f : 1.0f; }

}  // namespace

BypassableEffect::BypassableEffect(Effect& inner, bool bypassed)
    : inner_(inner), bypassed_(bypassed) {}

void BypassableEffect::Prepare(double sample_rate, int max_block_size) {
  inner_.Prepare(sample_rate, max_block_size);
  dry_.assign(static_cast<size_t>(max_block_size), 0.0f);
  wet_mix_.Reset(sample_rate, kCrossfadeSeconds, WetTarget(IsBypassed()));
  inner_is_idle_ = IsBypassed();
}

// Real-time safe: no allocation, locking or blocking.
void BypassableEffect::Process(float* samples, int num_samples) {
  wet_mix_.SetTarget(WetTarget(IsBypassed()));

  if (!wet_mix_.IsSmoothing() && wet_mix_.current() == 0.0f) {
    inner_is_idle_ = true;
    return;
  }

  // Stale state from before the bypass would otherwise leak into the output.
  if (inner_is_idle_) {
    inner_.Reset();
    inner_is_idle_ = false;
  }

  if (!wet_mix_.IsSmoothing()) {
    inner_.Process(samples, num_samples);
    return;
  }

  std::copy_n(samples, num_samples, dry_.data());
  inner_.Process(samples, num_samples);
  for (int i = 0; i < num_samples; ++i) {
    const float wet = wet_mix_.Next();
    samples[i] = dry_[i] + wet * (samples[i] - dry_[i]);
  }
}

// Real-time safe: no allocation, locking or blocking.
void BypassableEffect::Reset() { inner_.Reset(); }

int BypassableEffect::LatencySamples() const {
  return IsBypassed() ? 0 : inner_.LatencySamples();
}

}  // namespace voice_changer
