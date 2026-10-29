#ifndef VOICE_CHANGER_TESTS_TEST_UTIL_H_
#define VOICE_CHANGER_TESTS_TEST_UTIL_H_

#include <algorithm>
#include <cmath>
#include <numbers>
#include <span>
#include <vector>

#include "effects/effect.h"

namespace voice_changer::testing {

inline constexpr double kSampleRate = 48000.0;
inline constexpr int kBlockSize = 480;

inline std::vector<float> Sine(double hz, int num_samples,
                               float amplitude = 0.5f) {
  std::vector<float> samples(static_cast<size_t>(num_samples));
  for (int i = 0; i < num_samples; ++i) {
    samples[i] = amplitude * static_cast<float>(std::sin(
                                 2.0 * std::numbers::pi * hz * i / kSampleRate));
  }
  return samples;
}

// Processes the buffer in place in device-sized blocks.
inline void ProcessInBlocks(Effect& effect, std::vector<float>& samples) {
  const int total = static_cast<int>(samples.size());
  for (int start = 0; start < total; start += kBlockSize) {
    effect.Process(samples.data() + start,
                   std::min(kBlockSize, total - start));
  }
}

// Signal power at one frequency (Goertzel algorithm).
inline double PowerAt(std::span<const float> samples, double hz) {
  const double coefficient =
      2.0 * std::cos(2.0 * std::numbers::pi * hz / kSampleRate);
  double previous = 0.0;
  double before_previous = 0.0;
  for (float sample : samples) {
    const double current = sample + coefficient * previous - before_previous;
    before_previous = previous;
    previous = current;
  }
  return previous * previous + before_previous * before_previous -
         coefficient * previous * before_previous;
}

// Counts heap allocations made on the current thread while it is alive.
class AllocationCounter {
 public:
  AllocationCounter();
  ~AllocationCounter();
  int count() const;
};

}  // namespace voice_changer::testing

#endif  // VOICE_CHANGER_TESTS_TEST_UTIL_H_
