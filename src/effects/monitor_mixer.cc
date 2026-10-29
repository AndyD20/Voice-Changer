#include "effects/monitor_mixer.h"

namespace voice_changer {
namespace {

constexpr double kRampSeconds = 0.01;

float Gain(bool enabled) { return enabled ? 1.0f : 0.0f; }

}  // namespace

void MonitorMixer::Prepare(double sample_rate) {
  input_gain_.Reset(sample_rate, kRampSeconds, Gain(IsInputEnabled()));
  output_gain_.Reset(sample_rate, kRampSeconds, Gain(IsOutputEnabled()));
}

// Real-time safe: no allocation, locking or blocking.
void MonitorMixer::Process(const float* input, float* processed,
                           int num_samples) {
  input_gain_.SetTarget(Gain(IsInputEnabled()));
  output_gain_.SetTarget(Gain(IsOutputEnabled()));
  for (int i = 0; i < num_samples; ++i) {
    processed[i] =
        input_gain_.Next() * input[i] + output_gain_.Next() * processed[i];
  }
}

}  // namespace voice_changer
