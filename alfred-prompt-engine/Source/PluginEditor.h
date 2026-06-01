#pragma once

#include "PluginProcessor.h"
#include "PromptEngine.h"

#include <JuceHeader.h>

class AlfredPromptEngineAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                                     private juce::Button::Listener {
public:
    explicit AlfredPromptEngineAudioProcessorEditor(AlfredPromptEngineAudioProcessor& processor);
    ~AlfredPromptEngineAudioProcessorEditor() override = default;

    void paint(juce::Graphics& graphics) override;
    void resized() override;

private:
    void buttonClicked(juce::Button* button) override;
    void rebuildPrompt();

    AlfredPromptEngineAudioProcessor& audioProcessor;
    alfred::PromptEngine promptEngine;

    juce::Label titleLabel;
    juce::TextEditor sourceEditor;
    juce::TextButton buildButton;
    juce::TextEditor promptOutput;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AlfredPromptEngineAudioProcessorEditor)
};
