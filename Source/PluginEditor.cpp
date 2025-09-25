#include "PluginEditor.h"

CKPanningAudioProcessorEditor::CKPanningAudioProcessorEditor(CKPanningAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p) {
    setSize(600, 400);

    // Panorama Slider
    panoramaSlider.setSliderStyle(juce::Slider::LinearBar);
    panoramaSlider.setColour(juce::Slider::trackColourId, juce::Colour(0xff00bcd4));
    panoramaSlider.setColour(juce::Slider::thumbColourId, juce::Colour(0xff009688));
    panoramaSlider.setRange(0.0, 1.0, 0.001);
    panoramaSlider.setValue(processor.panoramaPosition);
    panoramaSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible(panoramaSlider);
    panoramaSlider.onValueChange = [this]() {
        processor.panoramaPosition = (float)panoramaSlider.getValue();
    };

    // Spectrum
    spectrumData.resize(processor.spectrumData.size());
    spectrumTimer.startTimerHz(30);
}

CKPanningAudioProcessorEditor::~CKPanningAudioProcessorEditor() {}

void CKPanningAudioProcessorEditor::paint(juce::Graphics& g) {
    // Background gradient
    juce::ColourGradient grad(juce::Colour(0xff232526), juce::Colour(0xff414345), getWidth() / 2, 0, getWidth() / 2, getHeight(), false);
    g.setGradientFill(grad);
    g.fillAll();

    // Title
    g.setColour(juce::Colours::white);
    g.setFont(28.0f);
    g.drawFittedText("CK Panning", {0, 10, getWidth(), 40}, juce::Justification::centred, 1);

    // Spectrum
    drawSpectrum(g, {40, 60, getWidth() - 80, 120});

    // Level meters (rounded, modern)
    leftLevel = processor.leftLevel;
    rightLevel = processor.rightLevel;
    g.setColour(juce::Colour(0xff4caf50));
    g.fillRoundedRectangle(80, 200, (float)(leftLevel * 300), 18, 8.0f);
    g.setColour(juce::Colour(0xffe53935));
    g.fillRoundedRectangle(80, 230, (float)(rightLevel * 300), 18, 8.0f);
    g.setColour(juce::Colours::white);
    g.setFont(16.0f);
    g.drawText("L", 40, 200, 30, 18, juce::Justification::centred);
    g.drawText("R", 40, 230, 30, 18, juce::Justification::centred);

    // Label for panorama
    g.setColour(juce::Colours::white);
    g.setFont(18.0f);
    g.drawText("Panorama", 40, 280, 100, 24, juce::Justification::left);
}

void CKPanningAudioProcessorEditor::resized() {
    panoramaSlider.setBounds(160, 275, getWidth() - 200, 40);
}

void CKPanningAudioProcessorEditor::drawSpectrum(juce::Graphics& g, juce::Rectangle<int> area) {
    g.setColour(juce::Colours::black.withAlpha(0.5f));
    g.fillRoundedRectangle(area.toFloat(), 12.0f);
    g.setColour(juce::Colour(0xff00bcd4));
    if (!spectrumData.empty()) {
        float width = (float)area.getWidth();
        float height = (float)area.getHeight();
        int bins = (int)spectrumData.size();
        float binWidth = width / (float)bins;
        for (int i = 0; i < bins; ++i) {
            float mag = juce::jmap(juce::Decibels::gainToDecibels(spectrumData[i] + 1e-6f), -80.0f, 0.0f, 0.0f, height);
            g.fillRect(area.getX() + (int)(i * binWidth), area.getBottom() - (int)mag, (int)binWidth, (int)mag);
        }
    }
    g.setColour(juce::Colours::white.withAlpha(0.7f));
    g.drawText("Spektrum", area, juce::Justification::topLeft);
}

void CKPanningAudioProcessorEditor::timerCallback() {
    if (processor.fftReady) {
        spectrumData = processor.spectrumData;
        processor.fftReady = false;
        repaint();
    }
}
