#include "effects/pitch_shifter.h"

#include <algorithm>

namespace voice_changer {
namespace {

// The block length sets the latency (roughly one block) and the quality.
// Signalsmith's default preset uses 120 ms blocks, too slow for live voice.
constexpr double kBlockSeconds = 0.04;
constexpr int kIntervalsPerBlock = 4;

float ClampSemitones(float semitones) {
  return std::clamp(semitones, PitchShifter::kMinSemitones,
                    PitchShifter::kMaxSemitones);
}

}  // namespace

void PitchShifter::SetPitchSemitones(float semitones) {
  pitch_semitones_.store(ClampSemitones(semitones));
}

void PitchShifter::SetFormantSemitones(float semitones) {
  formant_semitones_.store(ClampSemitones(semitones));
}

void PitchShifter::Prepare(double sample_rate, int max_block_size) {
  const int block_samples = static_cast<int>(sample_rate * kBlockSeconds);
  stretch_.configure(/*nChannels=*/1, block_samples,
                     block_samples / kIntervalsPerBlock);
  output_.assign(static_cast<size_t>(max_block_size), 0.0f);
  ApplyParameters();
}

// Real-time safe: Signalsmith Stretch does not allocate after configure().
void PitchShifter::Process(float* samples, int num_samples) {
  if (pitch_semitones_.load() != applied_pitch_ ||
      formant_semitones_.load() != applied_formant_) {
    ApplyParameters();
  }

  const float* inputs[] = {samples};
  float* outputs[] = {output_.data()};
  stretch_.process(inputs, num_samples, outputs, num_samples);
  std::copy_n(output_.data(), num_samples, samples);
}

// Real-time safe: Signalsmith Stretch does not allocate after configure().
void PitchShifter::Reset() { stretch_.reset(); }

int PitchShifter::LatencySamples() const {
  return stretch_.inputLatency() + stretch_.outputLatency();
}

void PitchShifter::ApplyParameters() {
  applied_pitch_ = pitch_semitones_.load();
  applied_formant_ = formant_semitones_.load();
  stretch_.setTransposeSemitones(applied_pitch_);
  // Compensating for the pitch shift keeps formants fixed unless asked.
  stretch_.setFormantSemitones(applied_formant_, /*compensatePitch=*/true);
}

}  // namespace voice_changer
