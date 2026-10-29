#include "effects_panel.h"

#include <cmath>
#include <initializer_list>

namespace voice_changer {
namespace {

constexpr int kRowHeight = 28;
constexpr int kGroupPadding = 12;
constexpr int kGroupTitleGap = 8;
constexpr int kGroupGap = 8;
constexpr int kLabelWidth = 80;
constexpr int kTextBoxWidth = 70;
constexpr int kTextBoxHeight = 20;
constexpr int kPitchRows = 4;
constexpr int kRobotRows = 3;
constexpr double kSemitoneStep = 0.1;
constexpr double kCarrierMidpointHz = 70.0;

int GroupHeight(int rows) {
  return 2 * kGroupPadding + kGroupTitleGap + rows * kRowHeight;
}

void SetUpSlider(juce::Slider& slider, juce::Label& label,
                 const juce::String& name, double min, double max,
                 double step, double default_value,
                 const juce::String& suffix) {
  slider.setSliderStyle(juce::Slider::LinearHorizontal);
  slider.setTextBoxStyle(juce::Slider::TextBoxRight, /*isReadOnly=*/false,
                         kTextBoxWidth, kTextBoxHeight);
  slider.setRange(min, max, step);
  slider.setTextValueSuffix(suffix);
  slider.setDoubleClickReturnValue(true, default_value);
  label.setText(name, juce::dontSendNotification);
  label.attachToComponent(&slider, /*onLeft=*/true);
}

// Rows are indented so labels attached on their left fit inside the group.
void LayOutGroup(juce::GroupComponent& group, juce::Rectangle<int> bounds,
                 std::initializer_list<juce::Component*> rows) {
  group.setBounds(bounds);
  auto content = bounds.reduced(kGroupPadding).withTrimmedTop(kGroupTitleGap);
  for (juce::Component* row : rows) {
    row->setBounds(content.removeFromTop(kRowHeight).withTrimmedLeft(kLabelWidth));
  }
}

}  // namespace

EffectsPanel::EffectsPanel(AudioEngine& engine) : engine_(engine) {
  const Preset defaults;
  SetUpSlider(pitch_, pitch_label_, "Pitch", PitchShifter::kMinSemitones,
              PitchShifter::kMaxSemitones, kSemitoneStep,
              defaults.pitch_semitones, " st");
  SetUpSlider(formant_, formant_label_, "Formant", PitchShifter::kMinSemitones,
              PitchShifter::kMaxSemitones, kSemitoneStep,
              defaults.formant_semitones, " st");
  SetUpSlider(carrier_, carrier_label_, "Carrier", RingModRobot::kMinCarrierHz,
              RingModRobot::kMaxCarrierHz, 1.0, defaults.robot_carrier_hz,
              " Hz");
  carrier_.setSkewFactorFromMidPoint(kCarrierMidpointHz);
  SetUpSlider(depth_, depth_label_, "Depth", 0.0, 100.0, 1.0,
              100.0 * defaults.robot_depth, " %");
  latency_note_.setFont(juce::FontOptions(13.0f));

  for (juce::Button* toggle : {&pitch_enabled_, &robot_enabled_}) {
    toggle->onClick = [this] { UpdateEngine(); };
  }
  for (juce::Slider* slider : {&pitch_, &formant_, &carrier_, &depth_}) {
    slider->onValueChange = [this] { UpdateEngine(); };
  }

  for (juce::Component* child :
       std::initializer_list<juce::Component*>{
           &pitch_group_, &pitch_enabled_, &pitch_, &formant_, &latency_note_,
           &robot_group_, &robot_enabled_, &carrier_, &depth_}) {
    addAndMakeVisible(child);
  }

  ApplyPreset(defaults);
  UpdateLatencyNote();
  engine_.device_manager().addChangeListener(this);
}

EffectsPanel::~EffectsPanel() {
  engine_.device_manager().removeChangeListener(this);
}

int EffectsPanel::PreferredHeight() {
  return GroupHeight(kPitchRows) + kGroupGap + GroupHeight(kRobotRows);
}

Preset EffectsPanel::CurrentPreset() const {
  Preset preset;
  preset.pitch_enabled = pitch_enabled_.getToggleState();
  preset.pitch_semitones = static_cast<float>(pitch_.getValue());
  preset.formant_semitones = static_cast<float>(formant_.getValue());
  preset.robot_enabled = robot_enabled_.getToggleState();
  preset.robot_carrier_hz = static_cast<float>(carrier_.getValue());
  preset.robot_depth = static_cast<float>(depth_.getValue() / 100.0);
  return preset;
}

void EffectsPanel::ApplyPreset(const Preset& preset) {
  pitch_enabled_.setToggleState(preset.pitch_enabled, juce::dontSendNotification);
  pitch_.setValue(preset.pitch_semitones, juce::dontSendNotification);
  formant_.setValue(preset.formant_semitones, juce::dontSendNotification);
  robot_enabled_.setToggleState(preset.robot_enabled, juce::dontSendNotification);
  carrier_.setValue(preset.robot_carrier_hz, juce::dontSendNotification);
  depth_.setValue(100.0 * preset.robot_depth, juce::dontSendNotification);
  UpdateEngine();
}

void EffectsPanel::resized() {
  auto bounds = getLocalBounds();
  LayOutGroup(pitch_group_, bounds.removeFromTop(GroupHeight(kPitchRows)),
              {&pitch_enabled_, &pitch_, &formant_, &latency_note_});
  bounds.removeFromTop(kGroupGap);
  LayOutGroup(robot_group_, bounds.removeFromTop(GroupHeight(kRobotRows)),
              {&robot_enabled_, &carrier_, &depth_});
}

void EffectsPanel::changeListenerCallback(juce::ChangeBroadcaster*) {
  UpdateLatencyNote();
}

void EffectsPanel::UpdateEngine() {
  const Preset preset = CurrentPreset();
  engine_.pitch_shifter_bypass().SetBypassed(!preset.pitch_enabled);
  engine_.pitch_shifter().SetPitchSemitones(preset.pitch_semitones);
  engine_.pitch_shifter().SetFormantSemitones(preset.formant_semitones);
  engine_.robot_bypass().SetBypassed(!preset.robot_enabled);
  engine_.robot().SetCarrierHz(preset.robot_carrier_hz);
  engine_.robot().SetDepth(preset.robot_depth);
}

void EffectsPanel::UpdateLatencyNote() {
  juce::AudioIODevice* device =
      engine_.device_manager().getCurrentAudioDevice();
  if (device == nullptr) {
    latency_note_.setText({}, juce::dontSendNotification);
    return;
  }
  const double ms = 1000.0 * engine_.pitch_shifter().LatencySamples() /
                    device->getCurrentSampleRate();
  latency_note_.setText(
      "Adds " + juce::String(std::lround(ms)) + " ms of delay when enabled.",
      juce::dontSendNotification);
}

}  // namespace voice_changer
