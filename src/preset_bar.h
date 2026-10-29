#ifndef VOICE_CHANGER_SRC_PRESET_BAR_H_
#define VOICE_CHANGER_SRC_PRESET_BAR_H_

#include <functional>

#include <juce_gui_basics/juce_gui_basics.h>

#include "presets.h"

namespace voice_changer {

// Preset picker. Choosing a name loads it; typing a name and pressing Save
// stores the current settings under that name.
class PresetBar final : public juce::Component {
 public:
  PresetBar(PresetStore& store, std::function<Preset()> capture,
            std::function<void(const Preset&)> apply);

  void resized() override;

 private:
  void RefreshNames(const juce::String& selected);
  void LoadSelected();
  void SaveCurrent();
  void DeleteSelected();
  void ShowError(const juce::String& message);

  PresetStore& store_;
  std::function<Preset()> capture_;
  std::function<void(const Preset&)> apply_;

  juce::Label label_{{}, "Preset"};
  juce::ComboBox names_;
  juce::TextButton save_button_{"Save"};
  juce::TextButton delete_button_{"Delete"};

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetBar)
};

}  // namespace voice_changer

#endif  // VOICE_CHANGER_SRC_PRESET_BAR_H_
