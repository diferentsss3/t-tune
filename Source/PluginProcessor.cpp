#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

TTuneProcessor::TTuneProcessor()
    : AudioProcessor(BusesProperties()
        .withInput ("Input",  juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "PARAMS", createParams())
{}

juce::AudioProcessorValueTreeState::ParameterLayout TTuneProcessor::createParams() {
    juce::AudioProcessorValueTreeState::ParameterLayout p;
    p.add(std::make_unique<juce::AudioParameterBool>("power", "Power", true));
    p.add(std::make_unique<juce::AudioParameterChoice>("key", "Key",
        juce::StringArray{ "C","C#","D","D#","E","F","F#","G","G#","A","A#","B" }, 0));
    p.add(std::make_unique<juce::AudioParameterChoice>("scale", "Scale",
        juce::StringArray{ "Chromatic","Major","Minor","Dorian","Phrygian",
                           "Lydian","Mixolydian","Locrian","Harmonic Minor",
                           "Pentatonic Major","Pentatonic Minor" }, 1));
    p.add(std::make_unique<juce::AudioParameterFloat>("speed", "Speed",
        juce::NormalisableRange<float>(0.f, 100.f, 0.01f), 55.f));
    p.add(std::make_unique<juce::AudioParameterFloat>("mix", "Mix",
        juce::NormalisableRange<float>(0.f, 100.f, 0.01f), 100.f));
    p.add(std::make_unique<juce::AudioParameterFloat>("humanize", "Humanize",
        juce::NormalisableRange<float>(0.f, 100.f, 0.01f), 0.f));
    return p;
}

const std::vector<int>& TTuneProcessor::scaleFor(int idx) {
    static const std::vector<int> chromatic     = {0,1,2,3,4,5,6,7,8,9,10,11};
    static const std::vector<int> major         = {0,2,4,5,7,9,11};
    static const std::vector<int> minor         = {0,2,3,5,7,8,10};
    static const std::vector<int> dorian        = {0,2,3,5,7,9,10};
    static const std::vector<int> phrygian      = {0,1,3,5,7,8,10};
    static const std::vector<int> lydian        = {0,2,4,6,7,9,11};
    static const std::vector<int> mixolydian    = {0,2,4,5,7,9,10};
    static const std::vector<int> locrian       = {0,1,3,5,6,8,10};
    static const std::vector<int> harmonicMinor = {0,2,3,5,7,8,11};
    static const std::vector<int> pentaMajor    = {0,2,4,7,9};
    static const std::vector<int> pentaMinor    = {0,3,5,7,10};
    switch (idx) {
        case 0: return chromatic;
        case 1: return major;
        case 2: return minor;
        case 3: return dorian;
        case 4: return phrygian;
        case 5: return lydian;
        case 6: return mixolydian;
        case 7: return locrian;
        case 8: return harmonicMinor;
        case 9: return pentaMajor;
        default: return pentaMinor;
    }
}

void TTuneProcessor::prepareToPlay(double sr, int) {
    pitchDetector.prepare(sr);
    for (auto& s : shifters) s.prepare(sr);
    smoothedTargetHz = 0.f;
}

bool TTuneProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo()
        || layouts.getMainOutputChannelSet() == juce::AudioChannelSet::mono();
}

void TTuneProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) {
    juce::ScopedNoDenormals noDenormals;
    const int n = buffer.getNumSamples();
    const int chans = buffer.getNumChannels();
    const bool power = *apvts.getRawParameterValue("power") > 0.5f;
    if (!power) return;

    const int keyIdx   = (int)*apvts.getRawParameterValue("key");
    const int scaleIdx = (int)*apvts.getRawParameterValue("scale");
    const float speed  = *apvts.getRawParameterValue("speed") / 100.f;
    const float mix    = *apvts.getRawParameterValue("mix")   / 100.f;
    const float human  = *apvts.getRawParameterValue("humanize") / 100.f;

    mono.assign(n, 0.f);
    for (int i = 0; i < n; ++i) {
        float s = 0.f;
        for (int c = 0; c < chans; ++c) s += buffer.getSample(c, i);
        mono[i] = s / (float)chans;
    }

    float hz = pitchDetector.detect(mono.data(), n, getSampleRate());
    uiDetectedHz.store(hz);

    float targetHz = hz;
    int detectedNoteIdx = -1, targetNoteIdx = -1;
    float cents = 0.f;

    if (hz > 20.f && hz < 2000.f) {
        const float midi = 69.f + 12.f * std::log2(hz / 440.f);
        detectedNoteIdx = (int)std::round(midi);
        const auto& sc = scaleFor(scaleIdx);
        int best = detectedNoteIdx;
        int bestDiff = 100;
        for (int base = detectedNoteIdx - 6; base <= detectedNoteIdx + 6; ++base) {
            int pc = ((base - keyIdx) % 12 + 12) % 12;
            for (int st : sc) {
                if (st == pc) {
                    int d = std::abs(base - detectedNoteIdx);
                    if (d < bestDiff) { bestDiff = d; best = base; }
                }
            }
        }
        targetNoteIdx = best;
        targetHz = 440.f * std::pow(2.f, (best - 69) / 12.f);
        if (human > 0.01f) {
            const float blend = human * 0.5f;
            targetHz = std::pow(targetHz, 1.f - blend) * std::pow(hz, blend);
        }
        cents = 1200.f * std::log2(targetHz / hz);
    }

    uiTargetHz.store(targetHz);
    uiDetectedNote.store(detectedNoteIdx);
    uiTargetNote.store(targetNoteIdx);
    uiCents.store(cents);

    if (targetHz > 0.f) {
        const float seconds = 0.005f + (1.f - speed) * (1.f - speed) * 0.5f;
        const float coeff   = 1.f - std::exp(-(float)n / (float)(getSampleRate() * seconds));
        if (smoothedTargetHz <= 0.f) smoothedTargetHz = targetHz;
        smoothedTargetHz = smoothedTargetHz * (1.f - coeff) + targetHz * coeff;
    }

    const float ratio = (hz > 0.f && smoothedTargetHz > 0.f)
                      ? juce::jlimit(0.5f, 2.0f, smoothedTargetHz / hz)
                      : 1.f;

    for (int c = 0; c < chans; ++c) {
        auto* data = buffer.getWritePointer(c);
        auto& sh = shifters[c];
        for (int i = 0; i < n; ++i) {
            const float dry = data[i];
            const float wet = sh.process(dry, ratio);
            data[i] = dry * (1.f - mix) + wet * mix;
        }
    }
}

juce::AudioProcessorEditor* TTuneProcessor::createEditor() {
    return new TTuneEditor(*this);
}

void TTuneProcessor::getStateInformation(juce::MemoryBlock& destData) {
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void TTuneProcessor::setStateInformation(const void* data, int size) {
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, size));
    if (xml && xml->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new TTuneProcessor();
}
