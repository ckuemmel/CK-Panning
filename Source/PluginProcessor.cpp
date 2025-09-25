#include "PluginProcessor.h"

CKPanningAudioProcessor::CKPanningAudioProcessor() : juce::AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true).withOutput("Output", juce::AudioChannelSet::stereo(), true)) {}
CKPanningAudioProcessor::~CKPanningAudioProcessor() {}

void CKPanningAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    // Prepare DSP chain for right channel
    juce::dsp::ProcessSpec spec { sampleRate, static_cast<juce::uint32>(samplesPerBlock), 1 };
    rightChain.prepare(spec);
    // Reset levels
    rightLevel = 0.0f;
    leftLevel = 0.0f;
    updateDSP(sampleRate);
}
void CKPanningAudioProcessor::releaseResources() {}


void CKPanningAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) {
    updateInterpolatedStage();
    updateDSP(getSampleRate());
    // Process right channel only (left remains dry)
    if (buffer.getNumChannels() > 1) {
        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::AudioBlock<float> rightBlock = block.getSingleChannelBlock(1);
        juce::dsp::ProcessContextReplacing<float> context(rightBlock);
        rightChain.process(context);
    }
    // Level metering
    updateLevelMeter(buffer);

    // Push left channel samples into FFT FIFO
    auto* left = buffer.getReadPointer(0);
    for (int i = 0; i < buffer.getNumSamples(); ++i)
        pushNextSampleIntoFifo(left[i]);
}
void CKPanningAudioProcessor::pushNextSampleIntoFifo(float sample) {
    if (fftWritePos < fftSize) {
        fftInput[fftWritePos++] = sample;
        if (fftWritePos == fftSize) {
            runFFT();
            fftWritePos = 0;
        }
    }
}

void CKPanningAudioProcessor::runFFT() {
    std::fill(fftOutput.begin(), fftOutput.end(), 0.0f);
    std::copy(fftInput.begin(), fftInput.end(), fftOutput.begin());
    fft.performRealOnlyForwardTransform(fftOutput.data());
    for (int i = 0; i < fftSize / 2; ++i) {
        float re = fftOutput[2 * i];
        float im = fftOutput[2 * i + 1];
        spectrumData[i] = std::sqrt(re * re + im * im);
    }
    fftReady = true;
}
void CKPanningAudioProcessor::updateDSP(double sampleRate) {
    using Coeff = juce::dsp::IIR::Coefficients<float>;
    // Highshelf
    *rightChain.get<0>().coefficients = *Coeff::makeHighShelf(sampleRate, interpolatedStage.highshelfFreq, interpolatedStage.highshelfQ, juce::Decibels::decibelsToGain(interpolatedStage.highshelfGain));
    // Bells
    for (int i = 0; i < 4; ++i) {
        *rightChain.get<1 + i>().coefficients = *Coeff::makePeakFilter(sampleRate, interpolatedStage.bells[i].freq, interpolatedStage.bells[i].q, juce::Decibels::decibelsToGain(interpolatedStage.bells[i].gain));
    }
    // Delay
    rightChain.get<5>().setDelay(interpolatedStage.delayMs * (float)sampleRate / 1000.0f); // ms to samples
    // Output gain
    rightChain.get<6>().setGainDecibels(interpolatedStage.outputGain);
}

void CKPanningAudioProcessor::updateLevelMeter(const juce::AudioBuffer<float>& buffer) {
    leftLevel = buffer.getRMSLevel(0, 0, buffer.getNumSamples());
    if (buffer.getNumChannels() > 1)
        rightLevel = buffer.getRMSLevel(1, 0, buffer.getNumSamples());
    else
        rightLevel = 0.0f;
}

void CKPanningAudioProcessor::updateInterpolatedStage() {
    // Interpolate between 1/3L, 2/3L, 3/3L based on panoramaPosition (1.0 = 3/3L, 0.66 = 2/3L, 0.33 = 1/3L)
    const PanoramaStage* stages[3] = { &stage13L, &stage23L, &stage33L };
    float pos = panoramaPosition;
    int idxA = 0, idxB = 0;
    float t = 0.0f;
    if (pos >= 0.66f) { idxA = 1; idxB = 2; t = (pos - 0.66f) / (1.0f - 0.66f); }
    else if (pos >= 0.33f) { idxA = 0; idxB = 1; t = (pos - 0.33f) / (0.66f - 0.33f); }
    else { idxA = 0; idxB = 0; t = 0.0f; }
    const PanoramaStage& a = *stages[idxA];
    const PanoramaStage& b = *stages[idxB];
    auto lerp = [](float x, float y, float t) { return x + (y - x) * t; };
    interpolatedStage.highshelfFreq = lerp(a.highshelfFreq, b.highshelfFreq, t);
    interpolatedStage.highshelfQ = lerp(a.highshelfQ, b.highshelfQ, t);
    interpolatedStage.highshelfGain = lerp(a.highshelfGain, b.highshelfGain, t);
    for (int i = 0; i < 4; ++i) {
        interpolatedStage.bells[i].freq = lerp(a.bells[i].freq, b.bells[i].freq, t);
        interpolatedStage.bells[i].q = lerp(a.bells[i].q, b.bells[i].q, t);
        interpolatedStage.bells[i].gain = lerp(a.bells[i].gain, b.bells[i].gain, t);
    }
    interpolatedStage.delayMs = lerp(a.delayMs, b.delayMs, t);
    interpolatedStage.outputGain = lerp(a.outputGain, b.outputGain, t);
}

juce::AudioProcessorEditor* CKPanningAudioProcessor::createEditor() { return nullptr; }
bool CKPanningAudioProcessor::hasEditor() const { return false; }

const juce::String CKPanningAudioProcessor::getName() const { return "CK Panning"; }
bool CKPanningAudioProcessor::acceptsMidi() const { return false; }
bool CKPanningAudioProcessor::producesMidi() const { return false; }
bool CKPanningAudioProcessor::isMidiEffect() const { return false; }
double CKPanningAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int CKPanningAudioProcessor::getNumPrograms() { return 1; }
int CKPanningAudioProcessor::getCurrentProgram() { return 0; }
void CKPanningAudioProcessor::setCurrentProgram(int) {}
const juce::String CKPanningAudioProcessor::getProgramName(int) { return {}; }
void CKPanningAudioProcessor::changeProgramName(int, const juce::String&) {}

void CKPanningAudioProcessor::getStateInformation(juce::MemoryBlock&) {}
void CKPanningAudioProcessor::setStateInformation(const void*, int) {}
