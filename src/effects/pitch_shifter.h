#ifndef VOICE_CHANGER_SRC_EFFECTS_PITCH_SHIFTER_H_
#define VOICE_CHANGER_SRC_EFFECTS_PITCH_SHIFTER_H_

#include <atomic>
#include <vector>

#include <signalsmith-stretch/signalsmith-stretch.h>

#include "effects/effect.h"

namespace voice_changer {

// Pitch and formant shifting via Signalsmith Stretch. Formants are controlled
// separately from pitch, so a pitch shift alone avoids the chipmunk effect.
class PitchShifter final : public Effect {
 public:
  static constexpr float kMinSemitones = -12.0f;
  static constexpr float kMaxSemitones = 12.0f;

  // Safe to call from any thread. Values are clamped to the semitone range.
  void SetPitchSemitones(float semitones);
  void SetFormantSemitones(float semitones);

  float pitch_semitones() const { return pitch_semitones_.load(); }
  float formant_semitones() const { return formant_semitones_.load(); }

  void Prepare(double sample_rate, int max_block_size) override;
  void Process(float* samples, int num_samples) override;
  void Reset() override;
  int LatencySamples() const override;

 private:
  void ApplyParameters();

  std::atomic<float> pitch_semitones_{0.0f};
  std::atomic<float> formant_semitones_{0.0f};

  signalsmith::stretch::SignalsmithStretch<float> stretch_;
  std::vector<float> output_;
  float applied_pitch_ = 0.0f;
  float applied_formant_ = 0.0f;
};

}  // namespace voice_changer

#endif  // VOICE_CHANGER_SRC_EFFECTS_PITCH_SHIFTER_H_
