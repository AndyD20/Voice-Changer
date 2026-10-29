#include "level_meter.h"

#include <algorithm>

#include <juce_audio_basics/juce_audio_basics.h>

namespace voice_changer {
namespace {

constexpr int kRefreshHz = 30;
constexpr float kReleasePerFrame = 0.85f;
constexpr float kSilence = 1e-4f;  // -80 dB.
constexpr float kMinDb = -60.0f;
constexpr float kWarningDb = -12.0f;
constexpr float kClipDb = -3.0f;
constexpr float kCornerSize = 3.0f;

juce::Colour ColourFor(float db) {
  if (db > kClipDb) return juce::Colours::red;
  if (db > kWarningDb) return juce::Colours::yellow;
  return juce::Colours::limegreen;
}

}  // namespace

LevelMeter::LevelMeter(PeakMeter& meter) : meter_(meter) {
  startTimerHz(kRefreshHz);
}

void LevelMeter::paint(juce::Graphics& g) {
  const juce::Rectangle<float> bounds = getLocalBounds().toFloat();
  g.setColour(juce::Colours::black.withAlpha(0.4f));
  g.fillRoundedRectangle(bounds, kCornerSize);

  const float db = juce::Decibels::gainToDecibels(level_, kMinDb);
  const float proportion = juce::jlimit(0.0f, 1.0f, 1.0f - db / kMinDb);
  g.setColour(ColourFor(db));
  g.fillRoundedRectangle(bounds.withWidth(bounds.getWidth() * proportion),
                         kCornerSize);
}

void LevelMeter::timerCallback() {
  float level = std::max(meter_.TakePeak(), level_ * kReleasePerFrame);
  if (level < kSilence) level = 0.0f;
  if (level != level_) {
    level_ = level;
    repaint();
  }
}

}  // namespace voice_changer
