#ifndef VOICE_CHANGER_SRC_LEVEL_METER_H_
#define VOICE_CHANGER_SRC_LEVEL_METER_H_

#include <juce_gui_basics/juce_gui_basics.h>

#include "effects/peak_meter.h"

namespace voice_changer {

// Horizontal peak level bar, in decibels, with a falling release.
class LevelMeter final : public juce::Component, private juce::Timer {
 public:
  explicit LevelMeter(PeakMeter& meter);

  void paint(juce::Graphics& g) override;

 private:
  void timerCallback() override;

  PeakMeter& meter_;
  float level_ = 0.0f;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LevelMeter)
};

}  // namespace voice_changer

#endif  // VOICE_CHANGER_SRC_LEVEL_METER_H_
