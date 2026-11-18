#include "PluginEditor.h"

TTuneLookAndFeel::TTuneLookAndFeel() {
    setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff16161f));
    setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff2a2a3a));
    setColour(juce::ComboBox::textColourId, juce::Colour(0xffe0e0ff));
    setColour(juce::ComboBox::arrowColourId, juce::Colour(0xff00e5ff));
    setColour(juce::PopupMenu::backgroundColourId, juce::Colour(0xff101018));
    setColour(juce::PopupMenu::textColourId, juce::Colour(0xffe0e0ff));
    setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(0xff00e5ff).withAlpha(0.2f));
}

void TTuneLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h,
                                         float pos, float start, float end, juce::Slider&) {
    auto bounds = juce::Rectangle<float>((float)x, (float)y, (float)w, (float)h).reduced(8.f);
    auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
    auto center = bounds.getCentre();
    auto angle = start + pos * (end - start);

    g.setColour(juce::Colour(0xff00e5ff).withAlpha(0.08f));
    g.fillEllipse(bounds.expanded(2.f));

    juce::Path bg;
    bg.addCentredArc(center.x, center.y, radius - 4.f, radius - 4.f, 0.f, start, end, true);
    g.setColour(juce::Colour(0xff2a2a3a));
    g.strokePath(bg, juce::PathStrokeType(3.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    juce::Path arc;
    arc.addCentredArc(center.x, center.y, radius - 4.f, radius - 4.f, 0.f, start, angle, true);
    g.setColour(juce::Colour(0xff00e5ff));
    g.strokePath(arc, juce::PathStrokeType(3.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    auto tipX = center.x + (radius - 4.f) * std::cos(angle - juce::MathConstants<float>::halfPi);
    auto tipY = center.y + (radius - 4.f) * std::sin(angle - juce::MathConstants<float>::halfPi);
    g.setColour(juce::Colour(0xff00e5ff));
    g.fillEllipse(tipX - 3.f, tipY - 3.f, 6.f, 6.f);
    g.setColour(juce::Colour(0xffff2e97));
    g.fillEllipse(tipX - 1.2f, tipY - 1.2f, 2.4f, 2.4f);

    g.setColour(juce::Colour(0xff1a1a28));
    g.fillEllipse(center.x - radius * 0.55f, center.y - radius * 0.55f, radius * 1.1f, radius * 1.1f);
    g.setColour(juce::Colour(0xff2a2a3a));
    g.drawEllipse(center.x - radius * 0.55f, center.y - radius * 0.55f, radius * 1.1f, radius * 1.1f, 1.f);
}

void TTuneLookAndFeel::drawComboBox(juce::Graphics& g, int w, int h, bool,
                                     int, int, int, int, juce::ComboBox&) {
    juce::Rectangle<float> r(0.f, 0.f, (float)w, (float)h);
    g.setColour(juce::Colour(0xff16161f));
    g.fillRoundedRectangle(r, 6.f);
    g.setColour(juce::Colour(0xff2a2a3a));
    g.drawRoundedRectangle(r.reduced(0.5f), 6.f, 1.f);
    juce::Path arrow;
    auto cx = (float)w - 14.f, cy = h * 0.5f;
    arrow.startNewSubPath(cx - 4.f, cy - 2.f);
    arrow.lineTo(cx, cy + 3.f);
    arrow.lineTo(cx + 4.f, cy - 2.f);
    g.setColour(juce::Colour(0xff00e5ff));
    g.strokePath(arrow, juce::PathStrokeType(1.8f));
}

void TTuneLookAndFeel::positionComboBoxText(juce::ComboBox& b, juce::Label& l) {
    l.setBounds(12, 0, b.getWidth() - 28, b.getHeight());
    l.setFont(juce::Font(juce::FontOptions(14.f, juce::Font::bold)));
    l.setColour(juce::Label::textColourId, juce::Colour(0xffe0e0ff));
    l.setJustificationType(juce::Justification::centredLeft);
}

PitchDisplay::PitchDisplay(TTuneProcessor& p) : proc(p) { startTimerHz(30); }

void PitchDisplay::paint(juce::Graphics& g) {
    auto b = getLocalBounds().toFloat();
    g.setColour(juce::Colour(0xff101018));
    g.fillRoundedRectangle(b, 10.f);
    g.setColour(juce::Colour(0xff2a2a3a));
    g.drawRoundedRectangle(b.reduced(0.5f), 10.f, 1.f);

    const int dn = proc.uiDetectedNote.load();
    const int tn = proc.uiTargetNote.load();
    const float cents = proc.uiCents.load();

    g.setColour(juce::Colour(0xff6a6a80));
    g.setFont(juce::Font(juce::FontOptions(10.f, juce::Font::bold)));
    g.drawText("PITCH MONITOR", b.removeFromTop(20.f).reduced(12.f, 0.f), juce::Justification::centredLeft);

    auto noteArea = b.removeFromLeft(b.getWidth() * 0.42f);
    if (dn >= 0 && dn < 128) {
        g.setColour(juce::Colour(0xffe0e0ff));
        g.setFont(juce::Font(juce::FontOptions(38.f, juce::Font::bold)));
        g.drawText(noteNames[dn % 12], noteArea.reduced(12.f, 0.f), juce::Justification::centredLeft);
        g.setColour(juce::Colour(0xff6a6a80));
        g.setFont(juce::Font(juce::FontOptions(14.f)));
        g.drawText(juce::String(dn / 12 - 1), noteArea.translated(0.f, 22.f).reduced(12.f, 0.f), juce::Justification::centredLeft);
    } else {
        g.setColour(juce::Colour(0xff404050));
        g.setFont(juce::Font(juce::FontOptions(32.f, juce::Font::bold)));
        g.drawText("--", noteArea.reduced(12.f, 0.f), juce::Justification::centredLeft);
    }

    auto bar = b.reduced(12.f, 14.f);
    auto barY = bar.getCentreY();
    g.setColour(juce::Colour(0xff2a2a3a));
    g.fillRoundedRectangle(bar.getX(), barY - 2.f, bar.getWidth(), 4.f, 2.f);

    auto cx = bar.getCentreX();
    g.setColour(juce::Colour(0xffff2e97));
    g.fillRect(cx - 0.5f, barY - 6.f, 1.f, 12.f);

    if (dn >= 0) {
        float norm = juce::jlimit(-1.f, 1.f, cents / 50.f);
        float x = cx + norm * bar.getWidth() * 0.5f;
        g.setColour(juce::Colour(0xff00e5ff));
        g.fillEllipse(x - 4.f, barY - 4.f, 8.f, 8.f);
        g.setColour(juce::Colour(0xff00e5ff).withAlpha(0.3f));
        g.fillEllipse(x - 7.f, barY - 7.f, 14.f, 14.f);
    }

    if (tn >= 0) {
        g.setColour(juce::Colour(0xff00e5ff));
        g.setFont(juce::Font(juce::FontOptions(11.f, juce::Font::bold)));
        g.drawText("TARGET  " + noteNames[tn % 12], getLocalBounds().toFloat().reduced(12.f, 4.f), juce::Justification::bottomLeft);
    }
}

TTuneEditor::TTuneEditor(TTuneProcessor& p) : AudioProcessorEditor(&p), proc(p), display(p) {
    setLookAndFeel(&lnf);
    setSize(520, 480);

    setupKnob(speedKnob, "SPEED");
    setupKnob(mixKnob,   "MIX");
    setupKnob(humanKnob, "HUMANIZE");

    speedAtt = std::make_unique<SliderAttachment>(proc.apvts, "speed", speedKnob);
    mixAtt   = std::make_unique<SliderAttachment>(proc.apvts, "mix",   mixKnob);
    humanAtt = std::make_unique<SliderAttachment>(proc.apvts, "humanize", humanKnob);

    keyBox.addItemList({ "C","C#","D","D#","E","F","F#","G","G#","A","A#","B" }, 1);
    scaleBox.addItemList({ "Chromatic","Major","Minor","Dorian","Phrygian",
                           "Lydian","Mixolydian","Locrian","Harmonic Minor",
                           "Pentatonic Major","Pentatonic Minor" }, 1);
    addAndMakeVisible(keyBox);
    addAndMakeVisible(scaleBox);
    keyAtt   = std::make_unique<ComboAttachment>(proc.apvts, "key",   keyBox);
    scaleAtt = std::make_unique<ComboAttachment>(proc.apvts, "scale", scaleBox);

    powerButton.setButtonText("ON");
    powerButton.setClickingTogglesState(true);
    addAndMakeVisible(powerButton);
    powerAtt = std::make_unique<ButtonAttachment>(proc.apvts, "power", powerButton);

    addAndMakeVisible(display);
}

TTuneEditor::~TTuneEditor() { setLookAndFeel(nullptr); }

void TTuneEditor::setupKnob(juce::Slider& s, const juce::String& label) {
    s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);
    s.setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xffe0e0ff));
    s.setRotaryParameters(juce::MathConstants<float>::pi * 1.25f, juce::MathConstants<float>::pi * 2.75f, true);
    s.setName(label);
    addAndMakeVisible(s);
}

void TTuneEditor::drawHeader(juce::Graphics& g) {
    auto area = getLocalBounds().toFloat().removeFromTop(80.f);
    g.setColour(juce::Colour(0xffe0e0ff));
    g.setFont(juce::Font(juce::FontOptions(34.f, juce::Font::bold)));
    g.drawText("T-TUNE", area.reduced(24.f, 12.f), juce::Justification::centredLeft);

    auto tBounds = area.reduced(24.f, 12.f);
    g.setColour(juce::Colour(0xff00e5ff));
    g.fillEllipse(tBounds.getX() + 118.f, tBounds.getY() + 18.f, 8.f, 8.f);
    g.setColour(juce::Colour(0xffff2e97));
    g.fillEllipse(tBounds.getX() + 128.f, tBounds.getY() + 18.f, 8.f, 8.f);

    g.setColour(juce::Colour(0xff6a6a80));
    g.setFont(juce::Font(juce::FontOptions(10.f)));
    g.drawText("PITCH PERFECTION ENGINE  -  V1.0", area.reduced(24.f, 12.f).translated(0.f, 24.f), juce::Justification::centredLeft);

    g.setColour(juce::Colour(0xff2a2a3a));
    g.drawHorizontalLine(80, 24.f, (float)getWidth() - 24.f);
}

void TTuneEditor::drawFooter(juce::Graphics& g) {
    g.setColour(juce::Colour(0xff404050));
    g.setFont(juce::Font(juce::FontOptions(9.f)));
    g.drawText("(c) T-TUNE AUDIO  -  ALL RIGHTS RESERVED",
               getLocalBounds().removeFromBottom(24).reduced(24, 0),
               juce::Justification::centred);
}

void TTuneEditor::paint(juce::Graphics& g) {
    auto b = getLocalBounds().toFloat();
    juce::ColourGradient bg(juce::Colour(0xff15152a), b.getCentreX(), b.getCentreY() - 100.f,
                            juce::Colour(0xff08080f), b.getCentreX(), b.getBottom(), true);
    g.setGradientFill(bg);
    g.fillAll();

    juce::ColourGradient c1(juce::Colour(0xff00e5ff).withAlpha(0.08f), 60.f, 100.f,
                            juce::Colours::transparentBlack, 300.f, 300.f, true);
    g.setGradientFill(c1);
    g.fillAll();

    juce::ColourGradient c2(juce::Colour(0xffff2e97).withAlpha(0.06f),
                            (float)getWidth() - 60.f, (float)getHeight() - 100.f,
                            juce::Colours::transparentBlack,
                            (float)getWidth() - 300.f, 200.f, true);
    g.setGradientFill(c2);
    g.fillAll();

    g.setColour(juce::Colour(0xff2a2a3a));
    g.drawRect(getLocalBounds(), 1);

    drawHeader(g);
    drawFooter(g);
}

void TTuneEditor::resized() {
    auto area = getLocalBounds().reduced(24);
    area.removeFromTop(80);
    area.removeFromBottom(24);

    display.setBounds(area.removeFromTop(90));
    area.removeFromTop(16);

    auto powerRow = area.removeFromTop(40);
    powerButton.setBounds(powerRow.removeFromLeft(90));

    area.removeFromTop(8);
    auto keyRow = area.removeFromTop(40);
    keyBox.setBounds(keyRow.removeFromLeft(keyRow.getWidth() / 2 - 4));
    keyRow.removeFromLeft(8);
    scaleBox.setBounds(keyRow);

    area.removeFromTop(20);
    auto knobRow = area.removeFromTop(area.getHeight());
    const int kw = knobRow.getWidth() / 3;
    speedKnob.setBounds(knobRow.removeFromLeft(kw).reduced(6));
    mixKnob.setBounds(knobRow.removeFromLeft(kw).reduced(6));
    humanKnob.setBounds(knobRow.reduced(6));
}
