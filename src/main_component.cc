#include "main_component.h"

namespace voice_changer {
namespace {

constexpr int kWidth = 600;
constexpr int kHeight = 480;
constexpr int kMargin = 12;
constexpr int kWarningHeight = 24;

}  // namespace

MainComponent::MainComponent(juce::AudioDeviceManager& device_manager)
    : device_selector_(device_manager, /*minAudioInputChannels=*/1,
                       /*maxAudioInputChannels=*/1,
                       /*minAudioOutputChannels=*/2,
                       /*maxAudioOutputChannels=*/2,
                       /*showMidiInputOptions=*/false,
                       /*showMidiOutputSelector=*/false,
                       /*showChannelsAsStereoPairs=*/true,
                       /*hideAdvancedOptionsWithButton=*/false) {
  feedback_warning_.setText(
      "Use headphones: monitoring through speakers feeds back into the mic.",
      juce::dontSendNotification);
  feedback_warning_.setColour(juce::Label::textColourId, juce::Colours::orange);

  addAndMakeVisible(feedback_warning_);
  addAndMakeVisible(device_selector_);
  setSize(kWidth, kHeight);
}

void MainComponent::paint(juce::Graphics& g) {
  g.fillAll(getLookAndFeel().findColour(
      juce::ResizableWindow::backgroundColourId));
}

void MainComponent::resized() {
  auto bounds = getLocalBounds().reduced(kMargin);
  feedback_warning_.setBounds(bounds.removeFromTop(kWarningHeight));
  device_selector_.setBounds(bounds);
}

}  // namespace voice_changer
