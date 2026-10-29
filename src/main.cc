#include <memory>

#include <juce_gui_basics/juce_gui_basics.h>

#include "audio_engine.h"
#include "main_component.h"

namespace voice_changer {
namespace {

class MainWindow final : public juce::DocumentWindow {
 public:
  MainWindow(const juce::String& name, AudioEngine& engine)
      : DocumentWindow(name,
                       juce::Desktop::getInstance()
                           .getDefaultLookAndFeel()
                           .findColour(juce::ResizableWindow::backgroundColourId),
                       DocumentWindow::allButtons) {
    setUsingNativeTitleBar(true);
    setContentOwned(new MainComponent(engine.device_manager()), true);
    setResizable(true, true);
    centreWithSize(getWidth(), getHeight());
    setVisible(true);
  }

  void closeButtonPressed() override {
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
  }

 private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
};

class VoiceChangerApplication final : public juce::JUCEApplication {
 public:
  const juce::String getApplicationName() override {
    return JUCE_APPLICATION_NAME_STRING;
  }
  const juce::String getApplicationVersion() override {
    return JUCE_APPLICATION_VERSION_STRING;
  }
  bool moreThanOneInstanceAllowed() override { return false; }

  void initialise(const juce::String&) override {
    engine_ = std::make_unique<AudioEngine>();
    window_ = std::make_unique<MainWindow>(getApplicationName(), *engine_);
  }

  void shutdown() override {
    window_.reset();
    engine_.reset();
  }

  void systemRequestedQuit() override { quit(); }

 private:
  std::unique_ptr<AudioEngine> engine_;
  std::unique_ptr<MainWindow> window_;
};

}  // namespace
}  // namespace voice_changer

START_JUCE_APPLICATION(voice_changer::VoiceChangerApplication)
