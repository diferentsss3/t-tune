#pragma once
#include <juce_gui_extra/juce_gui_extra.h>
#include "PluginProcessor.h"

class TTuneLookAndFeel : public juce::LookAndFeel_V4 {
public:
    TTuneLookAndFeel();
    void drawRotarySlider(juce::Graphics&, int x, int y, int w, int h,
                          float pos, float start, float end,
                          juce::Slider&) override;
    void drawComboBox(juce::Graphics&, int w, int h, bool,
                      int, int, int, int, juce::ComboBox&) override;
    void positionComboBoxText(juce::ComboBox&, juce::Label&) override;
};

class PitchDisplay : public juce::Component, private juce::Timer {
public:
    explicit PitchDisplay(TTuneProcessor& p);
    void paint(juce::Graphics&) override;
    void resized() override {}
private:
    void timerCallback() override { repaint(); }
    TTuneProcessor& proc;
    juce::StringArray noteNames{ "C","C#","D","D#","E","F","F#","G","G#","A","A#","B" };
};

class TTuneEditor : public juce::AudioProcessorEditor {
public:
    explicit TTuneEditor(TTuneProcessor&);
    ~TTuneEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAttachment  = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    TTuneProcessor& proc;
    TTuneLookAndFeel lnf;

    juce::Slider speedKnob, mixKnob, humanKnob;
    std::unique_ptr<SliderAttachment> speedAtt, mixAtt, humanAtt;

    juce::ComboBox keyBox, scaleBox;
    std::unique_ptr<ComboAttachment> keyAtt, scaleAtt;

    juce::ToggleButton powerButton;
    std::unique_ptr<ButtonAttachment> powerAtt;

    PitchDisplay display;

    void setupKnob(juce::Slider&, const juce::String& label);
    void drawHeader(juce::Graphics&);
    void drawFooter(juce::Graphics&);
};
