#include "effects/bypassable_effect.h"

#include <cmath>
#include <vector>

#include <gtest/gtest.h>

#include "test_util.h"

namespace voice_changer {
namespace {

using testing::AllocationCounter;
using testing::kBlockSize;
using testing::kSampleRate;
using testing::ProcessInBlocks;

class HalfGain final : public Effect {
 public:
  void Prepare(double, int) override {}
  void Process(float* samples, int num_samples) override {
    ++process_calls;
    for (int i = 0; i < num_samples; ++i) samples[i] *= 0.5f;
  }
  void Reset() override { ++reset_calls; }
  int LatencySamples() const override { return 64; }

  int process_calls = 0;
  int reset_calls = 0;
};

TEST(BypassableEffectTest, BypassedPassesSignalThroughUnprocessed) {
  HalfGain inner;
  BypassableEffect effect(inner, /*bypassed=*/true);
  effect.Prepare(kSampleRate, kBlockSize);

  std::vector<float> samples(kBlockSize * 4, 1.0f);
  ProcessInBlocks(effect, samples);

  EXPECT_EQ(samples, std::vector<float>(samples.size(), 1.0f));
  EXPECT_EQ(inner.process_calls, 0);
  EXPECT_EQ(effect.LatencySamples(), 0);
}

TEST(BypassableEffectTest, ActiveAppliesInnerEffect) {
  HalfGain inner;
  BypassableEffect effect(inner, /*bypassed=*/false);
  effect.Prepare(kSampleRate, kBlockSize);

  std::vector<float> samples(kBlockSize * 4, 1.0f);
  ProcessInBlocks(effect, samples);

  EXPECT_EQ(samples, std::vector<float>(samples.size(), 0.5f));
  EXPECT_EQ(effect.LatencySamples(), 64);
}

TEST(BypassableEffectTest, TogglingCrossfadesWithoutJumps) {
  HalfGain inner;
  BypassableEffect effect(inner, /*bypassed=*/true);
  effect.Prepare(kSampleRate, kBlockSize);
  std::vector<float> samples(kBlockSize * 4, 1.0f);

  effect.SetBypassed(false);
  ProcessInBlocks(effect, samples);

  // A 10 ms fade from 1.0 to 0.5 moves about 0.001 per sample.
  for (size_t i = 1; i < samples.size(); ++i) {
    ASSERT_LT(std::abs(samples[i] - samples[i - 1]), 0.002f) << "at " << i;
  }
  EXPECT_FLOAT_EQ(samples.back(), 0.5f);
}

TEST(BypassableEffectTest, ResetsInnerEffectWhenLeavingFullBypass) {
  HalfGain inner;
  BypassableEffect effect(inner, /*bypassed=*/true);
  effect.Prepare(kSampleRate, kBlockSize);
  std::vector<float> samples(kBlockSize, 1.0f);

  ProcessInBlocks(effect, samples);
  effect.SetBypassed(false);
  ProcessInBlocks(effect, samples);
  ProcessInBlocks(effect, samples);

  EXPECT_EQ(inner.reset_calls, 1);
}

TEST(BypassableEffectTest, ProcessDoesNotAllocate) {
  HalfGain inner;
  BypassableEffect effect(inner, /*bypassed=*/true);
  effect.Prepare(kSampleRate, kBlockSize);
  std::vector<float> samples(kBlockSize * 10, 1.0f);

  AllocationCounter allocations;
  ProcessInBlocks(effect, samples);
  effect.SetBypassed(false);
  ProcessInBlocks(effect, samples);
  effect.SetBypassed(true);
  ProcessInBlocks(effect, samples);

  EXPECT_EQ(allocations.count(), 0);
}

}  // namespace
}  // namespace voice_changer
