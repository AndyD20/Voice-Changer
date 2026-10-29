#include "effects/peak_meter.h"

#include <vector>

#include <gtest/gtest.h>

#include "test_util.h"

namespace voice_changer {
namespace {

using testing::AllocationCounter;
using testing::kBlockSize;

TEST(PeakMeterTest, ReportsLargestMagnitude) {
  PeakMeter meter;
  const std::vector<float> samples = {0.1f, -0.7f, 0.3f};

  meter.Process(samples.data(), static_cast<int>(samples.size()));

  EXPECT_FLOAT_EQ(meter.TakePeak(), 0.7f);
}

TEST(PeakMeterTest, KeepsPeakAcrossBlocksUntilTaken) {
  PeakMeter meter;
  const std::vector<float> loud = testing::Sine(440.0, kBlockSize, 0.8f);
  const std::vector<float> quiet = testing::Sine(440.0, kBlockSize, 0.2f);

  meter.Process(loud.data(), kBlockSize);
  meter.Process(quiet.data(), kBlockSize);

  EXPECT_NEAR(meter.TakePeak(), 0.8f, 1e-3f);
}

TEST(PeakMeterTest, TakePeakStartsNewMeasurement) {
  PeakMeter meter;
  const std::vector<float> samples = testing::Sine(440.0, kBlockSize);
  meter.Process(samples.data(), kBlockSize);

  meter.TakePeak();

  EXPECT_EQ(meter.TakePeak(), 0.0f);
}

TEST(PeakMeterTest, ProcessDoesNotAllocate) {
  PeakMeter meter;
  const std::vector<float> samples = testing::Sine(440.0, kBlockSize);

  AllocationCounter allocations;
  meter.Process(samples.data(), kBlockSize);

  EXPECT_EQ(allocations.count(), 0);
}

}  // namespace
}  // namespace voice_changer
