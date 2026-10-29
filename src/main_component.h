#ifndef VOICE_CHANGER_SRC_MAIN_COMPONENT_H_
#define VOICE_CHANGER_SRC_MAIN_COMPONENT_H_

#include <juce_gui_basics/juce_gui_basics.h>

#include "audio_engine.h"
#include "effects_panel.h"
#include "level_meter.h"
#include "preset_bar.h"
#include "presets.h"

namespace voice_changer {

// Root GUI component: presets, effect controls, level meters and a button
// that opens the audio device settings.
class MainComponent final : public juce::Component {
 public:
  explicit MainComponent(AudioEngine& engine);
  ~MainComponent() override;

  void paint(juce::Graphics& g) override;
  void resized() override;

 private:
  void ShowAudioSettings();

  AudioEngine& engine_;
  PresetStore preset_store_{PresetStore::DefaultDirectory()};
  EffectsPanel effects_panel_{engine_};
  PresetBar preset_bar_;
  LevelMeter input_meter_{engine_.input_meter()};
  juce::Label input_label_{{}, "Input"};
  LevelMeter output_meter_{engine_.output_meter()};
  juce::Label output_label_{{}, "Output"};
  juce::Label feedback_warning_;
  juce::TextButton audio_settings_button_{"Audio Settings..."};
  juce::Component::SafePointer<juce::DialogWindow> audio_settings_window_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};

}  // namespace voice_changer

#endif  // VOICE_CHANGER_SRC_MAIN_COMPONENT_H_
