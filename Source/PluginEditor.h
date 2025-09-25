#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class CKPanningAudioProcessorEditor : public juce::AudioProcessorEditor {
public:
    explicit CKPanningAudioProcessorEditor(CKPanningAudioProcessor&);
    ~CKPanningAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    CKPanningAudioProcessor& processor;

    // Panorama Slider
    juce::Slider panoramaSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> panoramaAttachment;

    // Level Meter
    float leftLevel = 0.0f;
    float rightLevel = 0.0f;

    // Spectrum Analyzer
    void drawSpectrum(juce::Graphics& g, juce::Rectangle<int> area);
    juce::Timer spectrumTimer;
    std::vector<float> spectrumData;
    void timerCallback();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CKPanningAudioProcessorEditor)
};
