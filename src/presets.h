#ifndef VOICE_CHANGER_SRC_PRESETS_H_
#define VOICE_CHANGER_SRC_PRESETS_H_

#include <optional>

#include <juce_core/juce_core.h>

#include "effects/ring_mod_robot.h"

namespace voice_changer {

// Every user-adjustable setting of the effect chain.
struct Preset {
  bool pitch_enabled = false;
  float pitch_semitones = 0.0f;
  float formant_semitones = 0.0f;
  bool robot_enabled = false;
  float robot_carrier_hz = RingModRobot::kDefaultCarrierHz;
  float robot_depth = 1.0f;
};

// Presets saved as one JSON file per preset, named after the preset.
class PresetStore {
 public:
  explicit PresetStore(juce::File directory);

  // The per-user presets folder, e.g. %APPDATA%\VoiceChanger\presets.
  static juce::File DefaultDirectory();
  // Names must be usable as file names and have no surrounding whitespace.
  static bool IsValidName(const juce::String& name);

  juce::StringArray Names() const;
  // Missing settings fall back to their defaults.
  std::optional<Preset> Load(const juce::String& name) const;
  bool Save(const juce::String& name, const Preset& preset) const;
  bool Remove(const juce::String& name) const;

 private:
  juce::File FileFor(const juce::String& name) const;

  juce::File directory_;
};

}  // namespace voice_changer

#endif  // VOICE_CHANGER_SRC_PRESETS_H_
