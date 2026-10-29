#ifndef VOICE_CHANGER_SRC_EFFECTS_EFFECT_H_
#define VOICE_CHANGER_SRC_EFFECTS_EFFECT_H_

namespace voice_changer {

// A mono audio effect. Prepare() is called off the audio thread and may
// allocate; Process() and Reset() run on the audio thread and must be
// real-time safe.
class Effect {
 public:
  virtual ~Effect() = default;

  virtual void Prepare(double sample_rate, int max_block_size) = 0;

  // Processes samples in place. num_samples must not exceed max_block_size.
  virtual void Process(float* samples, int num_samples) = 0;

  virtual void Reset() = 0;

  virtual int LatencySamples() const { return 0; }
};

}  // namespace voice_changer

#endif  // VOICE_CHANGER_SRC_EFFECTS_EFFECT_H_
