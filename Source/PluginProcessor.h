#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "dsp/PitchDetector.h"
#include "dsp/PitchShifter.h"

class TTuneProcessor : public juce::AudioProcessor {
public:
    TTuneProcessor();
    ~TTuneProcessor() override = default;

    void prepareToPlay(double, int) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported(const BusesLayout&) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "T-TUNE"; }
    bool acceptsMidi()  const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

    std::atomic<float> uiDetectedHz { 0.f };
    std::atomic<float> uiTargetHz   { 0.f };
    std::atomic<int>   uiDetectedNote { -1 };
    std::atomic<int>   uiTargetNote   { -1 };
    std::atomic<float> uiCents        { 0.f };

    juce::AudioProcessorValueTreeState apvts;

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParams();
    PitchDetector pitchDetector;
    PitchShifter  shifters[2];
    std::vector<float> mono;
    float smoothedTargetHz = 0.f;
    static const std::vector<int>& scaleFor(int idx);
};
