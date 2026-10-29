#ifndef VOICE_CHANGER_SRC_EFFECTS_PANEL_H_
#define VOICE_CHANGER_SRC_EFFECTS_PANEL_H_

#include <juce_gui_basics/juce_gui_basics.h>

#include "audio_engine.h"
#include "presets.h"

namespace voice_changer {

// Controls for each effect in the chain. Every change is written straight to
// the engine's atomic parameters, so the controls always match what is heard.
class EffectsPanel final : public juce::Component,
                           private juce::ChangeListener {
 public:
  explicit EffectsPanel(AudioEngine& engine);
  ~EffectsPanel() override;

  static int PreferredHeight();

  Preset CurrentPreset() const;
  void ApplyPreset(const Preset& preset);

  void resized() override;

 private:
  // Called when the audio device changes, which can change the latency.
  void changeListenerCallback(juce::ChangeBroadcaster* source) override;
  void UpdateEngine();
  void UpdateLatencyNote();

  AudioEngine& engine_;

  juce::GroupComponent pitch_group_{{}, "Pitch"};
  juce::ToggleButton pitch_enabled_{"Enabled"};
  juce::Slider pitch_;
  juce::Label pitch_label_;
  juce::Slider formant_;
  juce::Label formant_label_;
  juce::Label latency_note_;

  juce::GroupComponent robot_group_{{}, "Robot"};
  juce::ToggleButton robot_enabled_{"Enabled"};
  juce::Slider carrier_;
  juce::Label carrier_label_;
  juce::Slider depth_;
  juce::Label depth_label_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EffectsPanel)
};

}  // namespace voice_changer

#endif  // VOICE_CHANGER_SRC_EFFECTS_PANEL_H_
