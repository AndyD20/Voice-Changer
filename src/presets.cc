#include "presets.h"

#include <utility>

namespace voice_changer {
namespace {

constexpr int kFormatVersion = 1;
constexpr char kExtension[] = ".json";

juce::var ToJson(const Preset& preset) {
  juce::DynamicObject::Ptr pitch = new juce::DynamicObject();
  pitch->setProperty("enabled", preset.pitch_enabled);
  pitch->setProperty("semitones", preset.pitch_semitones);
  pitch->setProperty("formant_semitones", preset.formant_semitones);

  juce::DynamicObject::Ptr robot = new juce::DynamicObject();
  robot->setProperty("enabled", preset.robot_enabled);
  robot->setProperty("carrier_hz", preset.robot_carrier_hz);
  robot->setProperty("depth", preset.robot_depth);

  juce::DynamicObject::Ptr root = new juce::DynamicObject();
  root->setProperty("version", kFormatVersion);
  root->setProperty("pitch", pitch.get());
  root->setProperty("robot", robot.get());
  return root.get();
}

Preset FromJson(const juce::var& json) {
  const Preset defaults;
  const juce::var pitch = json.getProperty("pitch", {});
  const juce::var robot = json.getProperty("robot", {});

  Preset preset;
  preset.pitch_enabled = pitch.getProperty("enabled", defaults.pitch_enabled);
  preset.pitch_semitones =
      pitch.getProperty("semitones", defaults.pitch_semitones);
  preset.formant_semitones =
      pitch.getProperty("formant_semitones", defaults.formant_semitones);
  preset.robot_enabled = robot.getProperty("enabled", defaults.robot_enabled);
  preset.robot_carrier_hz =
      robot.getProperty("carrier_hz", defaults.robot_carrier_hz);
  preset.robot_depth = robot.getProperty("depth", defaults.robot_depth);
  return preset;
}

}  // namespace

PresetStore::PresetStore(juce::File directory)
    : directory_(std::move(directory)) {}

juce::File PresetStore::DefaultDirectory() {
#if JUCE_WINDOWS
  constexpr char kAppFolder[] = "VoiceChanger";
#else
  constexpr char kAppFolder[] = "voicechanger";
#endif
  return juce::File::getSpecialLocation(
             juce::File::userApplicationDataDirectory)
      .getChildFile(kAppFolder)
      .getChildFile("presets");
}

bool PresetStore::IsValidName(const juce::String& name) {
  return name.isNotEmpty() && name == name.trim() &&
         name == juce::File::createLegalFileName(name);
}

juce::StringArray PresetStore::Names() const {
  juce::StringArray names;
  for (const juce::File& file : directory_.findChildFiles(
           juce::File::findFiles, /*searchRecursively=*/false,
           juce::String("*") + kExtension)) {
    names.add(file.getFileNameWithoutExtension());
  }
  names.sortNatural();
  return names;
}

std::optional<Preset> PresetStore::Load(const juce::String& name) const {
  juce::var json;
  if (juce::JSON::parse(FileFor(name).loadFileAsString(), json).failed() ||
      !json.isObject()) {
    return std::nullopt;
  }
  return FromJson(json);
}

bool PresetStore::Save(const juce::String& name, const Preset& preset) const {
  return IsValidName(name) && directory_.createDirectory().wasOk() &&
         FileFor(name).replaceWithText(juce::JSON::toString(ToJson(preset)));
}

bool PresetStore::Remove(const juce::String& name) const {
  return IsValidName(name) && FileFor(name).deleteFile();
}

juce::File PresetStore::FileFor(const juce::String& name) const {
  return directory_.getChildFile(name + kExtension);
}

}  // namespace voice_changer
