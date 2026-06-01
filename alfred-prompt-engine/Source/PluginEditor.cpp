#include "PluginEditor.h"

namespace {

constexpr int padding = 16;

const char* defaultDirective = R"(// @title Neon Recursion
// @style industrial hip-hop, glitch, cinematic bass
// @tempo 92 BPM
// @key F minor

void verseEngine() {
    auto sub808 = "pressure under every bar";
    for (int bar = 0; bar < 16; ++bar) {
        kickPattern(sub808);
        glitchHook();
    }
}

void finalHook() {
    // make it feel like code becoming a stage performance
    vocalStack("limitless");
    widePad();
})";

} // namespace

AlfredPromptEngineAudioProcessorEditor::AlfredPromptEngineAudioProcessorEditor(
    AlfredPromptEngineAudioProcessor& processor)
    : AudioProcessorEditor(&processor), audioProcessor(processor) {
    setSize(920, 620);

    titleLabel.setText("Alfred Prompt Engine", juce::dontSendNotification);
    titleLabel.setFont(juce::FontOptions(24.0f, juce::Font::bold));
    addAndMakeVisible(titleLabel);

    sourceEditor.setMultiLine(true);
    sourceEditor.setReturnKeyStartsNewLine(true);
    sourceEditor.setFont(juce::FontOptions(15.0f));
    sourceEditor.setText(defaultDirective, juce::dontSendNotification);
    addAndMakeVisible(sourceEditor);

    buildButton.setButtonText("Build Prompt");
    buildButton.addListener(this);
    addAndMakeVisible(buildButton);

    promptOutput.setMultiLine(true);
    promptOutput.setReadOnly(true);
    promptOutput.setReturnKeyStartsNewLine(true);
    promptOutput.setFont(juce::FontOptions(14.0f));
    addAndMakeVisible(promptOutput);

    rebuildPrompt();
}

void AlfredPromptEngineAudioProcessorEditor::paint(juce::Graphics& graphics) {
    graphics.fillAll(juce::Colour(0xff151515));
    graphics.setColour(juce::Colour(0xff2a2a2a));
    graphics.fillRoundedRectangle(getLocalBounds().reduced(padding).toFloat(), 8.0f);
}

void AlfredPromptEngineAudioProcessorEditor::resized() {
    auto bounds = getLocalBounds().reduced(padding * 2);
    titleLabel.setBounds(bounds.removeFromTop(36));
    bounds.removeFromTop(8);

    auto buttonRow = bounds.removeFromBottom(44);
    buildButton.setBounds(buttonRow.removeFromRight(160).reduced(0, 4));
    bounds.removeFromBottom(12);

    auto left = bounds.removeFromLeft(bounds.getWidth() / 2).reduced(0, 0, 8, 0);
    auto right = bounds.reduced(8, 0, 0, 0);
    sourceEditor.setBounds(left);
    promptOutput.setBounds(right);
}

void AlfredPromptEngineAudioProcessorEditor::buttonClicked(juce::Button* button) {
    if (button == &buildButton) {
        rebuildPrompt();
    }
}

void AlfredPromptEngineAudioProcessorEditor::rebuildPrompt() {
    const auto result = promptEngine.buildPrompt(sourceEditor.getText().toStdString());
    promptOutput.setText(result.prompt, juce::dontSendNotification);
}
