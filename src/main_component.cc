#include "main_component.h"

#include <juce_audio_utils/juce_audio_utils.h>

#include <memory>

namespace voice_changer {
namespace {

constexpr int kWidth = 520;
constexpr int kMargin = 12;
constexpr int kGap = 8;
constexpr int kRowHeight = 28;
constexpr int kMeterHeight = 14;
constexpr int kMeterGap = 8;
constexpr int kLabelWidth = 80;
constexpr int kWarningHeight = 24;
constexpr int kSettingsButtonWidth = 140;
constexpr int kSettingsWidth = 500;
constexpr int kSettingsHeight = 420;

int PreferredHeight() {
  return 2 * kMargin + kRowHeight + kGap + EffectsPanel::PreferredHeight() +
         kGap + 2 * kMeterHeight + kMeterGap + kGap + kWarningHeight + kGap +
         kRowHeight;
}

}  // namespace

MainComponent::MainComponent(AudioEngine& engine)
    : engine_(engine),
      preset_bar_(
          preset_store_, [this] { return effects_panel_.CurrentPreset(); },
          [this](const Preset& preset) { effects_panel_.ApplyPreset(preset); }) {
  input_label_.attachToComponent(&input_meter_, /*onLeft=*/true);
  output_label_.attachToComponent(&output_meter_, /*onLeft=*/true);
  feedback_warning_.setText(
      "Use headphones: monitoring through speakers feeds back into the mic.",
      juce::dontSendNotification);
  feedback_warning_.setColour(juce::Label::textColourId, juce::Colours::orange);
  audio_settings_button_.onClick = [this] { ShowAudioSettings(); };

  addAndMakeVisible(preset_bar_);
  addAndMakeVisible(effects_panel_);
  addAndMakeVisible(input_meter_);
  addAndMakeVisible(output_meter_);
  addAndMakeVisible(feedback_warning_);
  addAndMakeVisible(audio_settings_button_);
  setSize(kWidth, PreferredHeight());
}

// The settings dialog listens to the device manager, so it must not outlive
// the engine.
MainComponent::~MainComponent() { delete audio_settings_window_.getComponent(); }

void MainComponent::paint(juce::Graphics& g) {
  g.fillAll(getLookAndFeel().findColour(
      juce::ResizableWindow::backgroundColourId));
}

void MainComponent::resized() {
  auto bounds = getLocalBounds().reduced(kMargin);
  preset_bar_.setBounds(bounds.removeFromTop(kRowHeight));
  bounds.removeFromTop(kGap);
  effects_panel_.setBounds(bounds.removeFromTop(EffectsPanel::PreferredHeight()));
  bounds.removeFromTop(kGap);
  input_meter_.setBounds(
      bounds.removeFromTop(kMeterHeight).withTrimmedLeft(kLabelWidth));
  bounds.removeFromTop(kMeterGap);
  output_meter_.setBounds(
      bounds.removeFromTop(kMeterHeight).withTrimmedLeft(kLabelWidth));
  bounds.removeFromTop(kGap);
  feedback_warning_.setBounds(bounds.removeFromTop(kWarningHeight));
  bounds.removeFromTop(kGap);
  audio_settings_button_.setBounds(bounds.removeFromTop(kRowHeight)
                                       .removeFromRight(kSettingsButtonWidth));
}

void MainComponent::ShowAudioSettings() {
  auto selector = std::make_unique<juce::AudioDeviceSelectorComponent>(
      engine_.device_manager(), /*minAudioInputChannels=*/1,
      /*maxAudioInputChannels=*/1, /*minAudioOutputChannels=*/2,
      /*maxAudioOutputChannels=*/2, /*showMidiInputOptions=*/false,
      /*showMidiOutputSelector=*/false, /*showChannelsAsStereoPairs=*/true,
      /*hideAdvancedOptionsWithButton=*/false);
  selector->setSize(kSettingsWidth, kSettingsHeight);

  juce::DialogWindow::LaunchOptions options;
  options.content.setOwned(selector.release());
  options.dialogTitle = "Audio Settings";
  options.dialogBackgroundColour =
      getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId);
  options.componentToCentreAround = this;
  options.useNativeTitleBar = true;
  options.resizable = true;
  audio_settings_window_ = options.launchAsync();
}

}  // namespace voice_changer
