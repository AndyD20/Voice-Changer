#ifndef VOICE_CHANGER_SRC_EFFECTS_BYPASSABLE_EFFECT_H_
#define VOICE_CHANGER_SRC_EFFECTS_BYPASSABLE_EFFECT_H_

#include <atomic>
#include <vector>

#include "effects/effect.h"
#include "effects/linear_smoother.h"

namespace voice_changer {

// Adds a click-free bypass to another effect by crossfading between the dry
// and processed signal. A fully bypassed effect is not processed at all.
class BypassableEffect final : public Effect {
 public:
  explicit BypassableEffect(Effect& inner, bool bypassed = true);

  // Safe to call from any thread.
  void SetBypassed(bool bypassed) { bypassed_.store(bypassed); }
  bool IsBypassed() const { return bypassed_.load(); }

  void Prepare(double sample_rate, int max_block_size) override;
  void Process(float* samples, int num_samples) override;
  void Reset() override;
  int LatencySamples() const override;

 private:
  Effect& inner_;
  std::atomic<bool> bypassed_;
  LinearSmoother wet_mix_;
  std::vector<float> dry_;
  bool inner_is_idle_ = false;
};

}  // namespace voice_changer

#endif  // VOICE_CHANGER_SRC_EFFECTS_BYPASSABLE_EFFECT_H_
