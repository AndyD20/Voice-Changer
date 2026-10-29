#include "preset_bar.h"

#include <utility>

namespace voice_changer {
namespace {

constexpr int kLabelWidth = 80;
constexpr int kButtonWidth = 70;
constexpr int kGap = 6;
constexpr char kTitle[] = "Presets";

}  // namespace

PresetBar::PresetBar(PresetStore& store, std::function<Preset()> capture,
                     std::function<void(const Preset&)> apply)
    : store_(store), capture_(std::move(capture)), apply_(std::move(apply)) {
  names_.setEditableText(true);
  names_.setTextWhenNothingSelected("Type a name, then Save");
  names_.onChange = [this] {
    if (names_.getSelectedId() != 0) LoadSelected();
  };
  save_button_.onClick = [this] { SaveCurrent(); };
  delete_button_.onClick = [this] { DeleteSelected(); };

  addAndMakeVisible(label_);
  addAndMakeVisible(names_);
  addAndMakeVisible(save_button_);
  addAndMakeVisible(delete_button_);
  RefreshNames({});
}

void PresetBar::resized() {
  auto bounds = getLocalBounds();
  label_.setBounds(bounds.removeFromLeft(kLabelWidth));
  delete_button_.setBounds(bounds.removeFromRight(kButtonWidth));
  bounds.removeFromRight(kGap);
  save_button_.setBounds(bounds.removeFromRight(kButtonWidth));
  bounds.removeFromRight(kGap);
  names_.setBounds(bounds);
}

void PresetBar::RefreshNames(const juce::String& selected) {
  names_.clear(juce::dontSendNotification);
  names_.addItemList(store_.Names(), /*firstItemIdOffset=*/1);
  names_.setText(selected, juce::dontSendNotification);
}

void PresetBar::LoadSelected() {
  const juce::String name = names_.getText();
  if (const std::optional<Preset> preset = store_.Load(name)) {
    apply_(*preset);
  } else {
    ShowError("Could not read the preset \"" + name + "\".");
  }
}

void PresetBar::SaveCurrent() {
  const juce::String name = names_.getText().trim();
  if (!PresetStore::IsValidName(name)) {
    ShowError(
        "Type a name for the preset first. Names can't contain "
        "\\ / : * ? \" < > |");
    return;
  }
  if (!store_.Save(name, capture_())) {
    ShowError("Could not save the preset \"" + name + "\".");
    return;
  }
  RefreshNames(name);
}

void PresetBar::DeleteSelected() {
  const juce::String name = names_.getText();
  if (!store_.Names().contains(name)) return;

  juce::AlertWindow::showAsync(
      juce::MessageBoxOptions::makeOptionsOkCancel(
          juce::MessageBoxIconType::QuestionIcon, kTitle,
          "Delete the preset \"" + name + "\"?", "Delete", "Cancel", this),
      [bar = juce::Component::SafePointer<PresetBar>(this), name](int result) {
        constexpr int kDeleteButton = 1;
        if (bar == nullptr || result != kDeleteButton) return;
        if (!bar->store_.Remove(name)) {
          bar->ShowError("Could not delete the preset \"" + name + "\".");
        }
        bar->RefreshNames({});
      });
}

void PresetBar::ShowError(const juce::String& message) {
  juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon,
                                         kTitle, message, {}, this);
}

}  // namespace voice_changer
