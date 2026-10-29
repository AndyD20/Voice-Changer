#include "effects/ring_mod_robot.h"

#include <cmath>
#include <numbers>
#include <vector>

#include <gtest/gtest.h>

#include "test_util.h"

namespace voice_changer {
namespace {

using testing::AllocationCounter;
using testing::kBlockSize;
using testing::kSampleRate;
using testing::ProcessInBlocks;

TEST(RingModRobotTest, FullDepthMultipliesByCarrierSine) {
  RingModRobot robot;
  robot.SetCarrierHz(100.0f);
  robot.SetDepth(1.0f);
  robot.Prepare(kSampleRate, kBlockSize);

  std::vector<float> samples(2000, 1.0f);
  ProcessInBlocks(robot, samples);

  for (size_t i = 0; i < samples.size(); ++i) {
    const double expected = std::sin(2.0 * std::numbers::pi * 100.0 * i /
                                     kSampleRate);
    ASSERT_NEAR(samples[i], expected, 1e-4) << "at sample " << i;
  }
}

TEST(RingModRobotTest, ZeroDepthLeavesSignalUnchanged) {
  RingModRobot robot;
  robot.SetDepth(0.0f);
  robot.Prepare(kSampleRate, kBlockSize);

  const std::vector<float> input = testing::Sine(440.0, 2000);
  std::vector<float> samples = input;
  ProcessInBlocks(robot, samples);

  EXPECT_EQ(samples, input);
}

TEST(RingModRobotTest, ParametersAreClamped) {
  RingModRobot robot;

  robot.SetCarrierHz(1.0f);
  robot.SetDepth(2.0f);
  EXPECT_EQ(robot.carrier_hz(), RingModRobot::kMinCarrierHz);
  EXPECT_EQ(robot.depth(), 1.0f);

  robot.SetCarrierHz(10000.0f);
  robot.SetDepth(-1.0f);
  EXPECT_EQ(robot.carrier_hz(), RingModRobot::kMaxCarrierHz);
  EXPECT_EQ(robot.depth(), 0.0f);
}

TEST(RingModRobotTest, ProcessDoesNotAllocate) {
  RingModRobot robot;
  robot.Prepare(kSampleRate, kBlockSize);
  std::vector<float> samples = testing::Sine(440.0, kBlockSize * 10);

  AllocationCounter allocations;
  robot.SetDepth(0.5f);
  ProcessInBlocks(robot, samples);
  robot.Reset();

  EXPECT_EQ(allocations.count(), 0);
}

}  // namespace
}  // namespace voice_changer
