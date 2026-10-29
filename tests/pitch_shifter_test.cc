#include "effects/pitch_shifter.h"

#include <iostream>
#include <vector>

#include <gtest/gtest.h>

#include "test_util.h"

namespace voice_changer {
namespace {

using testing::AllocationCounter;
using testing::kBlockSize;
using testing::kSampleRate;
using testing::PowerAt;
using testing::ProcessInBlocks;

constexpr double kInputHz = 220.0;
constexpr int kTwoSeconds = static_cast<int>(kSampleRate) * 2;

// Shifts a sine and returns the last half second, well past the latency.
std::vector<float> ShiftSine(float pitch_semitones, float formant_semitones) {
  PitchShifter shifter;
  shifter.SetPitchSemitones(pitch_semitones);
  shifter.SetFormantSemitones(formant_semitones);
  shifter.Prepare(kSampleRate, kBlockSize);

  std::vector<float> samples = testing::Sine(kInputHz, kTwoSeconds);
  ProcessInBlocks(shifter, samples);
  return {samples.end() - kTwoSeconds / 4, samples.end()};
}

TEST(PitchShifterTest, OctaveUpMovesEnergyToDoubleFrequency) {
  const std::vector<float> output = ShiftSine(12.0f, 0.0f);

  EXPECT_GT(PowerAt(output, 2 * kInputHz), 100 * PowerAt(output, kInputHz));
}

TEST(PitchShifterTest, OctaveDownMovesEnergyToHalfFrequency) {
  const std::vector<float> output = ShiftSine(-12.0f, 0.0f);

  EXPECT_GT(PowerAt(output, kInputHz / 2), 100 * PowerAt(output, kInputHz));
}

TEST(PitchShifterTest, ZeroShiftKeepsFrequency) {
  const std::vector<float> output = ShiftSine(0.0f, 0.0f);

  EXPECT_GT(PowerAt(output, kInputHz), 100 * PowerAt(output, 2 * kInputHz));
}

TEST(PitchShifterTest, FormantShiftDoesNotChangePitch) {
  const std::vector<float> output = ShiftSine(0.0f, 6.0f);

  EXPECT_GT(PowerAt(output, kInputHz), 100 * PowerAt(output, 2 * kInputHz));
}

TEST(PitchShifterTest, ReportsLatency) {
  PitchShifter shifter;
  shifter.Prepare(kSampleRate, kBlockSize);

  EXPECT_GT(shifter.LatencySamples(), 0);
  std::cout << "Pitch shifter latency: " << shifter.LatencySamples()
            << " samples (" << 1000.0 * shifter.LatencySamples() / kSampleRate
            << " ms at 48 kHz)\n";
}

TEST(PitchShifterTest, SemitonesAreClamped) {
  PitchShifter shifter;

  shifter.SetPitchSemitones(100.0f);
  shifter.SetFormantSemitones(-100.0f);

  EXPECT_EQ(shifter.pitch_semitones(), PitchShifter::kMaxSemitones);
  EXPECT_EQ(shifter.formant_semitones(), PitchShifter::kMinSemitones);
}

TEST(PitchShifterTest, ProcessDoesNotAllocate) {
  PitchShifter shifter;
  shifter.Prepare(kSampleRate, kBlockSize);
  std::vector<float> samples = testing::Sine(kInputHz, kTwoSeconds);

  AllocationCounter allocations;
  shifter.SetPitchSemitones(5.0f);
  ProcessInBlocks(shifter, samples);
  shifter.SetFormantSemitones(-3.0f);
  shifter.Reset();
  ProcessInBlocks(shifter, samples);

  EXPECT_EQ(allocations.count(), 0);
}

}  // namespace
}  // namespace voice_changer
