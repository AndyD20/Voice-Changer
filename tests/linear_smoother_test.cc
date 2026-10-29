#include "effects/linear_smoother.h"

#include <gtest/gtest.h>

namespace voice_changer {
namespace {

TEST(LinearSmootherTest, StartsAtResetValue) {
  LinearSmoother smoother;
  smoother.Reset(/*sample_rate=*/100.0, /*ramp_seconds=*/0.1, 0.5f);

  EXPECT_FALSE(smoother.IsSmoothing());
  EXPECT_FLOAT_EQ(smoother.Next(), 0.5f);
}

TEST(LinearSmootherTest, RampsLinearlyAndLandsExactlyOnTarget) {
  LinearSmoother smoother;
  smoother.Reset(/*sample_rate=*/100.0, /*ramp_seconds=*/0.04, 0.0f);
  smoother.SetTarget(1.0f);

  EXPECT_FLOAT_EQ(smoother.Next(), 0.25f);
  EXPECT_FLOAT_EQ(smoother.Next(), 0.5f);
  EXPECT_FLOAT_EQ(smoother.Next(), 0.75f);
  EXPECT_EQ(smoother.Next(), 1.0f);
  EXPECT_FALSE(smoother.IsSmoothing());
  EXPECT_EQ(smoother.Next(), 1.0f);
}

TEST(LinearSmootherTest, RetargetingMidRampStartsFromCurrentValue) {
  LinearSmoother smoother;
  smoother.Reset(/*sample_rate=*/100.0, /*ramp_seconds=*/0.04, 0.0f);
  smoother.SetTarget(1.0f);
  smoother.Next();
  smoother.Next();

  smoother.SetTarget(0.0f);

  EXPECT_FLOAT_EQ(smoother.Next(), 0.375f);
}

}  // namespace
}  // namespace voice_changer
