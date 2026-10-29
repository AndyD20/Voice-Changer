#include "effects/monitor_mixer.h"

#include <cmath>
#include <vector>

#include <gtest/gtest.h>

#include "test_util.h"

namespace voice_changer {
namespace {

using testing::AllocationCounter;
using testing::kBlockSize;
using testing::kSampleRate;

constexpr float kInput = 0.25f;
constexpr float kProcessed = 0.5f;

// Mixes one block of constant input and processed signals.
std::vector<float> MixBlock(MonitorMixer& mixer) {
  const std::vector<float> input(kBlockSize, kInput);
  std::vector<float> processed(kBlockSize, kProcessed);
  mixer.Process(input.data(), processed.data(), kBlockSize);
  return processed;
}

void Prepare(MonitorMixer& mixer, bool input_enabled, bool output_enabled) {
  mixer.SetInputEnabled(input_enabled);
  mixer.SetOutputEnabled(output_enabled);
  mixer.Prepare(kSampleRate);
}

TEST(MonitorMixerTest, DefaultsToProcessedOnly) {
  MonitorMixer mixer;
  mixer.Prepare(kSampleRate);

  for (float sample : MixBlock(mixer)) EXPECT_FLOAT_EQ(sample, kProcessed);
}

TEST(MonitorMixerTest, InputOnly) {
  MonitorMixer mixer;
  Prepare(mixer, true, false);

  for (float sample : MixBlock(mixer)) EXPECT_FLOAT_EQ(sample, kInput);
}

TEST(MonitorMixerTest, BothAreSummed) {
  MonitorMixer mixer;
  Prepare(mixer, true, true);

  for (float sample : MixBlock(mixer)) {
    EXPECT_FLOAT_EQ(sample, kInput + kProcessed);
  }
}

TEST(MonitorMixerTest, NeitherIsSilent) {
  MonitorMixer mixer;
  Prepare(mixer, false, false);

  for (float sample : MixBlock(mixer)) EXPECT_FLOAT_EQ(sample, 0.0f);
}

TEST(MonitorMixerTest, TogglingRampsWithoutJumps) {
  MonitorMixer mixer;
  Prepare(mixer, false, true);

  mixer.SetOutputEnabled(false);
  const std::vector<float> output = MixBlock(mixer);

  float previous = kProcessed;
  for (float sample : output) {
    EXPECT_LT(std::abs(sample - previous), 0.01f);
    previous = sample;
  }
  EXPECT_FLOAT_EQ(output.back(), 0.0f);
}

TEST(MonitorMixerTest, ProcessDoesNotAllocate) {
  MonitorMixer mixer;
  Prepare(mixer, true, true);
  const std::vector<float> input(kBlockSize, kInput);
  std::vector<float> processed(kBlockSize, kProcessed);

  AllocationCounter allocations;
  mixer.SetInputEnabled(false);
  mixer.Process(input.data(), processed.data(), kBlockSize);

  EXPECT_EQ(allocations.count(), 0);
}

}  // namespace
}  // namespace voice_changer
