#ifndef VOICE_CHANGER_SRC_MAIN_COMPONENT_H_
#define VOICE_CHANGER_SRC_MAIN_COMPONENT_H_

#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_gui_basics/juce_gui_basics.h>

namespace voice_changer {

// Root GUI component: device selection and a feedback warning.
class MainComponent final : public juce::Component {
 public:
  explicit MainComponent(juce::AudioDeviceManager& device_manager);

  void paint(juce::Graphics& g) override;
  void resized() override;

 private:
  juce::Label feedback_warning_;
  juce::AudioDeviceSelectorComponent device_selector_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};

}  // namespace voice_changer

#endif  // VOICE_CHANGER_SRC_MAIN_COMPONENT_H_
