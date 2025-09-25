    // FFT for spectrum analysis
    static constexpr int fftOrder = 11; // 2048 samples
    static constexpr int fftSize = 1 << fftOrder;
    juce::dsp::FFT fft { fftOrder };
    std::vector<float> fftInput { std::vector<float>(fftSize, 0.0f) };
    std::vector<float> fftOutput { std::vector<float>(fftSize * 2, 0.0f) };
    int fftWritePos = 0;
    bool fftReady = false;
    std::vector<float> spectrumData { std::vector<float>(fftSize / 2, 0.0f) };
    void pushNextSampleIntoFifo(float sample);
    void runFFT();
#pragma once
#include <JuceHeader.h>

class CKPanningAudioProcessor : public juce::AudioProcessor {
public:
    // DSP components for right channel
    juce::dsp::ProcessorChain<
        juce::dsp::IIR::Filter<float>, // Highshelf
        juce::dsp::IIR::Filter<float>, // Bell 1
        juce::dsp::IIR::Filter<float>, // Bell 2
        juce::dsp::IIR::Filter<float>, // Bell 3
        juce::dsp::IIR::Filter<float>, // Bell 4
        juce::dsp::DelayLine<float>,   // Delay
        juce::dsp::Gain<float>         // Output gain
    > rightChain;

    float rightLevel = 0.0f;
    float leftLevel = 0.0f;
    CKPanningAudioProcessor();
    ~CKPanningAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    struct PanoramaStage {
        float highshelfFreq;
        float highshelfQ;
        float highshelfGain;
        struct Bell {
            float freq;
            float q;
            float gain;
        } bells[4];
        float delayMs;
        float outputGain;
    };

    // Panorama stages: 3/3L, 2/3L, 1/3L
    PanoramaStage stage33L {
        900.0f, 0.1f, -4.3f,
        {
            {8630.0f, 4.5f, -9.0f},
            {4027.0f, 3.69f, -6.0f},
            {2938.0f, 1.4f, -6.0f},
            {753.0f, 5.76f, -5.0f}
        },
        0.46f, -3.0f
    };
    PanoramaStage stage23L {
        1100.0f, 0.1f, -3.0f,
        {
            {5250.0f, 4.8f, -6.0f},
            {3000.0f, 2.81f, -6.0f},
            {1200.0f, 5.76f, -4.0f},
            {0.0f, 1.0f, 0.0f} // unused
        },
        0.35f, -2.5f
    };
    PanoramaStage stage13L {
        1400.0f, 0.1f, -2.0f,
        {
            {5351.0f, 5.76f, -4.5f},
            {2376.0f, 5.76f, -5.8f},
            {0.0f, 1.0f, 0.0f},
            {0.0f, 1.0f, 0.0f}
        },
        0.23f, -2.0f
    };

    // Interpolated parameters for current panorama position (0.0 = Center, 1.0 = 3/3L)
    PanoramaStage interpolatedStage;
    float panoramaPosition = 1.0f; // 1.0 = 3/3L, 0.66 = 2/3L, 0.33 = 1/3L, 0.0 = Center

    void updateInterpolatedStage();
    void updateDSP(double sampleRate);
    void updateLevelMeter(const juce::AudioBuffer<float>& buffer);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CKPanningAudioProcessor)
};
