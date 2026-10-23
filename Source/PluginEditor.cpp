#include "PluginEditor.h"
#include "BinaryData.h"
#include "Presets.h"

namespace
{
    using escoita::U;
    using escoita::numero;
}

namespace ui
{
// ================================================================ tipografía
// Montserrat (títulos e rótulos, en maiúsculas espaciadas coma na web de Escoita) e Inter
// (texto corrido): as da familia Barullo. As dúas, licenza OFL.
static juce::Typeface::Ptr tf (int which)
{
    static juce::Typeface::Ptr t[3] = {
        juce::Typeface::createSystemTypefaceFor (BinaryData::InterMedium_ttf, BinaryData::InterMedium_ttfSize),
        juce::Typeface::createSystemTypefaceFor (BinaryData::MontserratSemiBold_ttf, BinaryData::MontserratSemiBold_ttfSize),
        juce::Typeface::createSystemTypefaceFor (BinaryData::MontserratBold_ttf, BinaryData::MontserratBold_ttfSize) };
    return t[which];
}
juce::Font body (float s) { return juce::Font (juce::FontOptions (tf (0)).withHeight (s)); }
juce::Font semi (float s) { return juce::Font (juce::FontOptions (tf (1)).withHeight (s)); }
juce::Font cond (float s) { return juce::Font (juce::FontOptions (tf (2)).withHeight (s)); }
juce::Font caps (float s, float tracking) { return juce::Font (juce::FontOptions (tf (1)).withHeight (s).withKerningFactor (tracking)); }
juce::String upper (const juce::String& t) { return escoita::maiusculas (t); }

void drawCaps (juce::Graphics& g, const juce::String& t, juce::Rectangle<float> r, juce::Justification j, float size, float tracking)
{
    g.setFont (caps (size, tracking));
    g.drawText (upper (t), r, j, false);
}

void glowText (juce::Graphics& g, const juce::String& t, juce::Rectangle<float> r, juce::Justification j, juce::Colour c)
{
    g.setColour (c.withAlpha (0.16f));
    for (const auto d : { juce::Point<float> (-0.8f, 0.0f), { 0.8f, 0.0f }, { 0.0f, -0.8f }, { 0.0f, 0.8f } })
        g.drawText (t, r.translated (d.x, d.y), j, false);
    g.setColour (c);
    g.drawText (t, r, j, false);
}

// ================================================================ pezas do aparello
void paintPanel (juce::Graphics& g, juce::Rectangle<float> r, float rad)
{
    for (int i = 3; i >= 1; --i)
    {
        g.setColour (juce::Colours::black.withAlpha (0.11f));
        g.fillRoundedRectangle (r.translated (0.0f, 2.0f * (float) i).expanded ((float) i), rad + (float) i);
    }
    g.setGradientFill (juce::ColourGradient (col::panel1, 0.0f, r.getY(), col::panel2, 0.0f, r.getBottom(), false));
    g.fillRoundedRectangle (r, rad);
    g.setColour (juce::Colours::white.withAlpha (0.07f));
    g.fillRect (r.getX() + rad, r.getY() + 0.5f, r.getWidth() - 2.0f * rad, 1.0f);
}

void paintWell (juce::Graphics& g, juce::Rectangle<float> r, float rad)
{
    g.setColour (juce::Colours::white.withAlpha (0.05f));
    g.fillRoundedRectangle (r.translated (0.0f, 1.0f), rad);
    g.setColour (col::visor.brighter (0.06f));
    g.fillRoundedRectangle (r, rad);
    juce::Path p; p.addRoundedRectangle (r, rad);
    g.saveState();
    g.reduceClipRegion (p);
    g.setGradientFill (juce::ColourGradient (juce::Colours::black.withAlpha (0.65f), 0.0f, r.getY(), juce::Colours::transparentBlack, 0.0f, r.getY() + 5.0f, false));
    g.fillRect (r.withHeight (6.0f));
    g.restoreState();
}

void paintGlass (juce::Graphics& g, juce::Rectangle<float> r, float rad)
{
    g.setColour (juce::Colours::white.withAlpha (0.06f));
    g.fillRoundedRectangle (r.translated (0.0f, 1.0f), rad);
    juce::ColourGradient gr (col::cristal1, r.getCentreX(), r.getY(), col::cristal2, r.getCentreX(), r.getBottom(), false);
    gr.addColour (0.55, juce::Colour (0xff0a1129));
    g.setGradientFill (gr);
    g.fillRoundedRectangle (r, rad);
    juce::Path p; p.addRoundedRectangle (r, rad);
    g.saveState();
    g.reduceClipRegion (p);
    for (int i = 0; i < 5; ++i)
    {
        g.setColour (juce::Colours::black.withAlpha (0.16f - 0.03f * (float) i));
        g.drawRoundedRectangle (r.reduced ((float) i + 0.5f), rad, 1.0f);
    }
    g.setGradientFill (juce::ColourGradient (juce::Colours::white.withAlpha (0.05f), r.getX(), r.getY(),
                                             juce::Colours::transparentWhite, r.getX() + r.getWidth() * 0.32f, r.getY() + r.getHeight() * 0.55f, false));
    g.fillRect (r);
    g.restoreState();
}

void paintKey (juce::Graphics& g, juce::Rectangle<float> r, bool over, bool down)
{
    g.setColour (juce::Colours::black.withAlpha (0.5f));
    g.fillRoundedRectangle (r.translated (0.0f, 2.0f), 7.0f);
    if (down) r.translate (0.0f, 1.0f);
    const float lift = over && ! down ? 0.10f : 0.0f;
    g.setGradientFill (juce::ColourGradient (col::tecla1.brighter (lift), 0.0f, r.getY(), col::tecla2.brighter (lift * 0.6f), 0.0f, r.getBottom(), false));
    g.fillRoundedRectangle (r, 7.0f);
    g.setColour (juce::Colours::white.withAlpha (down ? 0.04f : 0.10f));
    g.fillRect (r.getX() + 6.0f, r.getY() + 0.5f, r.getWidth() - 12.0f, 1.0f);
}

void paintLed (juce::Graphics& g, juce::Point<float> c, bool on, juce::Colour luz)
{
    if (on)
    {
        g.setColour (luz.withAlpha (0.16f)); g.fillEllipse (c.x - 7.0f, c.y - 7.0f, 14.0f, 14.0f);
        g.setColour (luz.withAlpha (0.34f)); g.fillEllipse (c.x - 4.6f, c.y - 4.6f, 9.2f, 9.2f);
        g.setColour (luz); g.fillEllipse (c.x - 3.0f, c.y - 3.0f, 6.0f, 6.0f);
        g.setColour (juce::Colours::white.withAlpha (0.55f)); g.fillEllipse (c.x - 1.3f, c.y - 2.0f, 2.2f, 2.0f);
    }
    else
    {
        g.setColour (juce::Colour (0xff3a3326)); g.fillEllipse (c.x - 3.0f, c.y - 3.0f, 6.0f, 6.0f);
        g.setColour (juce::Colours::black.withAlpha (0.6f)); g.drawEllipse (c.x - 3.0f, c.y - 3.0f, 6.0f, 6.0f, 0.8f);
    }
}

// rótulo dun grupo: na súa cor e cunha liña debaixo
void groupTitle (juce::Graphics& g, juce::Rectangle<float> r, const juce::String& t, juce::Colour c)
{
    g.setColour (c);
    drawCaps (g, t, r.withHeight (14.0f), juce::Justification::centred, 9.0f, 0.28f);
    g.setColour (c.withAlpha (0.25f));
    g.fillRect (r.getX() + 8.0f, r.getY() + 18.0f, r.getWidth() - 16.0f, 1.0f);
}

// ================================================================ aspecto
Look::Look()
{
    setColour (juce::ResizableWindow::backgroundColourId, col::noite);
    setColour (juce::Label::textColourId, col::crema);
    setColour (juce::TextEditor::backgroundColourId, col::visor);
    setColour (juce::TextEditor::textColourId, col::ouro);
    setColour (juce::TextEditor::outlineColourId, col::lina);
    setColour (juce::TextEditor::focusedOutlineColourId, col::laranxa);
    setColour (juce::PopupMenu::backgroundColourId, col::panel2);
    setColour (juce::PopupMenu::textColourId, col::crema);
    setColour (juce::PopupMenu::headerTextColourId, col::laranxa);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, col::laranxa);
    setColour (juce::PopupMenu::highlightedTextColourId, col::noite);
    setColour (juce::TextButton::buttonColourId, col::tecla2);
    setColour (juce::TextButton::textColourOffId, col::crema2);
    setColour (juce::TextButton::textColourOnId, col::crema);
    setColour (juce::AlertWindow::backgroundColourId, col::panel2);
    setColour (juce::AlertWindow::textColourId, col::crema);
    setColour (juce::AlertWindow::outlineColourId, col::lina);
}

juce::Typeface::Ptr Look::getTypefaceForFont (const juce::Font& f) { return tf (f.isBold() ? 1 : 0); }

// as teclas: en relevo, cun LED (aceso se está escollida; «led» = a cor da capa) e o texto en
// maiúsculas espaciadas; «gold» = un visor dourado (o selector da combinación)
void Look::drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour&, bool over, bool down)
{
    auto r = b.getLocalBounds().toFloat().reduced (1.0f).withTrimmedBottom (1.5f);
    if (b.getProperties().contains ("gold"))
    {
        paintWell (g, r, 8.0f);
        if (over) { g.setColour (col::laranxa.withAlpha (0.08f)); g.fillRoundedRectangle (r, 8.0f); }
        return;
    }
    paintKey (g, r, over, down);
    if (down) r.translate (0.0f, 1.0f);
    if (! b.getProperties().contains ("noLed"))
    {
        const auto luz = b.getProperties().contains ("led") ? juce::Colour ((juce::uint32) (juce::int64) b.getProperties()["led"]) : col::laranxa;
        paintLed (g, { r.getX() + 12.0f, r.getCentreY() }, b.getToggleState(), luz);
    }
}

void Look::drawButtonText (juce::Graphics& g, juce::TextButton& b, bool over, bool down)
{
    auto r = b.getLocalBounds().toFloat().reduced (1.0f).withTrimmedBottom (1.5f);
    if (b.getProperties().contains ("gold"))
    {
        g.setFont (caps (12.0f, 0.1f));
        glowText (g, upper (b.getButtonText()), r.reduced (14.0f, 0.0f).withTrimmedRight (16.0f), juce::Justification::centredLeft, col::ouro);
        juce::Path arrow;
        const float ax = r.getRight() - 18.0f, ay = r.getCentreY();
        arrow.addTriangle (ax - 5.0f, ay - 2.5f, ax + 5.0f, ay - 2.5f, ax, ay + 3.5f);
        g.setColour (col::ouro); g.fillPath (arrow);
        return;
    }
    if (down) r.translate (0.0f, 1.0f);
    if (! b.getProperties().contains ("noLed")) r.removeFromLeft (14.0f);
    juce::Colour c = b.getToggleState() || over ? col::crema : col::crema2;
    if (! b.isEnabled()) c = col::crema3.withAlpha (0.6f);
    g.setColour (c);
    const int curve = b.getProperties()["curve"];   // desfacer (−1) e refacer (+1): unha frecha en arco
    if (curve != 0)
    {
        const auto ctr = r.getCentre(); const float rad = 6.5f;
        const float a0 = curve > 0 ? -2.4f : 2.4f, a1 = curve > 0 ? 1.2f : -1.2f;
        juce::Path arc; arc.addCentredArc (ctr.x, ctr.y + 1.0f, rad, rad, 0.0f, a0, a1, true);
        g.strokePath (arc, juce::PathStrokeType (1.8f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        const juce::Point<float> tip (ctr.x + rad * std::sin (a1), ctr.y + 1.0f - rad * std::cos (a1));
        juce::Path head; head.addTriangle (tip.x + (float) curve * 4.0f, tip.y, tip.x - (float) curve * 1.5f, tip.y - 4.0f, tip.x - (float) curve * 1.5f, tip.y + 4.0f);
        g.fillPath (head);
        return;
    }
    g.setFont (caps ((float) b.getProperties().getWithDefault ("fontSize", 9.5f), 0.15f));
    g.drawFittedText (upper (b.getButtonText()), r.toNearestInt().reduced (3, 0), juce::Justification::centred, 1, 0.7f);
}

// os mandos: disco mecanizado, anel de LED laranxa e o valor nun visor debaixo
void Look::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float a0, float a1, juce::Slider& s)
{
    auto area = juce::Rectangle<int> (x, y, w, h).toFloat();
    auto lcd = area.removeFromBottom (17.0f);
    lcd = lcd.withSizeKeepingCentre (juce::jmin (66.0f, area.getWidth() - 2.0f), 17.0f);
    area.removeFromBottom (3.0f);
    const float d = juce::jmin (area.getWidth(), area.getHeight());
    const auto c = area.withSizeKeepingCentre (d, d).getCentre();
    const float ring = d * 0.5f - 4.0f;
    const auto luz = s.getProperties().contains ("led") ? juce::Colour ((juce::uint32) (juce::int64) s.getProperties()["led"]) : col::laranxa;

    juce::Path track; track.addCentredArc (c.x, c.y, ring, ring, 0.0f, a0, a1, true);
    g.setColour (juce::Colours::black.withAlpha (0.55f));
    g.strokePath (track, juce::PathStrokeType (3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    const bool bip = s.getMinimum() < 0 && s.getMaximum() > 0;
    const double zeroAt = bip ? 0.0 : s.getMinimum();
    const float zero = a0 + (float) s.valueToProportionOfLength (zeroAt) * (a1 - a0);
    const float ang = a0 + pos * (a1 - a0);
    if (std::abs (ang - zero) > 0.01f)
    {
        juce::Path val; val.addCentredArc (c.x, c.y, ring, ring, 0.0f, juce::jmin (zero, ang), juce::jmax (zero, ang), true);
        const auto lit = s.isEnabled() ? luz : col::crema3;
        g.setColour (lit.withAlpha (0.12f)); g.strokePath (val, juce::PathStrokeType (8.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour (lit.withAlpha (0.26f)); g.strokePath (val, juce::PathStrokeType (5.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour (lit); g.strokePath (val, juce::PathStrokeType (2.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    const float kr = ring - 7.0f;
    g.setColour (juce::Colours::black.withAlpha (0.45f));
    g.fillEllipse (c.x - kr, c.y - kr + 2.5f, kr * 2.0f, kr * 2.0f);
    juce::ColourGradient disc (juce::Colour (0xff4a5577), c.x - kr * 0.3f, c.y - kr * 0.45f, juce::Colour (0xff0e1323), c.x + kr * 0.55f, c.y + kr * 1.05f, true);
    disc.addColour (0.55, juce::Colour (0xff232b45));
    g.setGradientFill (disc);
    g.fillEllipse (c.x - kr, c.y - kr, kr * 2.0f, kr * 2.0f);
    g.setColour (juce::Colours::white.withAlpha (0.10f));
    g.drawEllipse (c.x - kr, c.y - kr, kr * 2.0f, kr * 2.0f, 1.0f);
    const juce::Point<float> tip (c.x + kr * 0.82f * std::sin (ang), c.y - kr * 0.82f * std::cos (ang));
    g.setColour (col::crema.withAlpha (s.isEnabled() ? 0.92f : 0.4f));
    g.drawLine ({ c, tip }, 2.0f);

    paintWell (g, lcd, 5.0f);
    g.setFont (caps (9.0f, 0.04f));
    glowText (g, upper (s.getTextFromValue (s.getValue())), lcd, juce::Justification::centred, s.isEnabled() ? col::ouro : col::crema3);
}

// os deslizadores: a pista afundida, a luz laranxa desde abaixo ata o capuchón e o capuchón mecanizado
void Look::drawLinearSlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float, float, juce::Slider::SliderStyle, juce::Slider& s)
{
    const float cx = (float) x + (float) w * 0.5f;
    const float top = (float) y, bot = (float) (y + h);
    auto slot = juce::Rectangle<float> (cx - 4.5f, top - 7.0f, 9.0f, bot - top + 14.0f);
    paintWell (g, slot, 4.5f);
    const int steps = s.getMaximum() - s.getMinimum() <= 3.5 ? (int) (s.getMaximum() - s.getMinimum()) : 10;
    g.setColour (col::crema3.withAlpha (0.55f));
    for (int t = 0; t <= steps; ++t)
    {
        const float ty = bot - (bot - top) * (float) t / (float) juce::jmax (1, steps);
        const float len = (t % 5 == 0 || steps <= 3) ? 5.0f : 2.5f;
        g.fillRect (cx - 9.0f - len, ty - 0.5f, len, 1.0f);
        g.fillRect (cx + 9.0f, ty - 0.5f, len, 1.0f);
    }
    if (pos < bot - 0.5f)
    {
        auto lit = juce::Rectangle<float> (cx - 1.5f, pos, 3.0f, bot + 4.0f - pos);
        g.setColour (col::laranxa.withAlpha (0.14f)); g.fillRoundedRectangle (lit.expanded (3.0f, 0.0f), 3.0f);
        g.setColour (col::laranxa.withAlpha (0.30f)); g.fillRoundedRectangle (lit.expanded (1.5f, 0.0f), 2.0f);
        g.setColour (col::laranxa); g.fillRoundedRectangle (lit, 1.5f);
    }
    auto cap = juce::Rectangle<float> (cx - 13.0f, pos - 7.0f, 26.0f, 14.0f);
    g.setColour (juce::Colours::black.withAlpha (0.55f)); g.fillRoundedRectangle (cap.translated (0.0f, 2.5f), 4.0f);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff4a5577), cap.getX(), cap.getY(), juce::Colour (0xff141a2e), cap.getX(), cap.getBottom(), false));
    g.fillRoundedRectangle (cap, 4.0f);
    g.setColour (juce::Colours::white.withAlpha (0.10f)); g.drawRoundedRectangle (cap.reduced (0.5f), 4.0f, 1.0f);
    g.setColour (col::crema); g.fillRect (cap.getX() + 4.0f, pos - 0.8f, cap.getWidth() - 8.0f, 1.6f);
}

void Look::drawTooltip (juce::Graphics& g, const juce::String& text, int w, int h)
{
    auto r = juce::Rectangle<int> (w, h).toFloat();
    g.setColour (col::panel2); g.fillRoundedRectangle (r, 6.0f);
    g.setColour (col::lina); g.drawRoundedRectangle (r.reduced (0.5f), 6.0f, 1.0f);
    juce::AttributedString s; s.setJustification (juce::Justification::topLeft);
    s.append (text, body (13.5f), col::crema);
    juce::TextLayout tl; tl.createLayoutWithBalancedLineLengths (s, 300.0f);
    tl.draw (g, r.reduced (10.0f, 7.0f));
}

juce::Rectangle<int> Look::getTooltipBounds (const juce::String& text, juce::Point<int> pos, juce::Rectangle<int> parent)
{
    juce::AttributedString s; s.append (text, body (13.5f));
    juce::TextLayout tl; tl.createLayoutWithBalancedLineLengths (s, 300.0f);
    const int w = (int) tl.getWidth() + 22, h = (int) tl.getHeight() + 16;
    return juce::Rectangle<int> (pos.x > parent.getCentreX() ? pos.x - (w + 12) : pos.x + 12,
                                 pos.y > parent.getCentreY() ? pos.y - (h + 8) : pos.y + 18, w, h).constrainedWithin (parent);
}

void Look::drawPopupMenuBackground (juce::Graphics& g, int w, int h)
{
    g.fillAll (col::panel2);
    g.setColour (col::lina); g.drawRect (0, 0, w, h);
}

// ================================================================ mando
Knob::Knob()
{
    slider.setRotaryParameters (juce::MathConstants<float>::pi * 1.25f, juce::MathConstants<float>::pi * 2.75f, true);
    slider.setMouseDragSensitivity (180);
    addAndMakeVisible (slider);
}

void Knob::paint (juce::Graphics& g)
{
    g.setColour (slider.isEnabled() ? col::crema2 : col::crema3);
    drawCaps (g, name, getLocalBounds().removeFromBottom (12).toFloat(), juce::Justification::centred, 7.5f, 0.16f);
}



// ================================================================ utilidades
namespace
{
    const juce::Colour BRASA { 0xffff8a3d }, BRASA_CLARA { 0xffffc48a };
    juce::String dbTxt (double db) { return (db >= 0.05 ? "+" : db <= -0.05 ? U ("−") : juce::String()) + numero (std::abs (db), 1) + " dB"; }
    juce::String hzTxt (double f) { return f < 1000 ? juce::String ((int) f) + " Hz" : juce::String ((int) (f / 1000)) + " kHz"; }
}

// ================================================================ a curva
void Curva::paint (juce::Graphics& g)
{
    const auto r = getLocalBounds().toFloat();
    paintGlass (g, r, 12.0f);
    auto a = r.reduced (16.0f, 0.0f).withTrimmedTop (26.0f).withTrimmedBottom (18.0f);
    const double fs = 96000.0;
    tubo::Filtros f; f.deseñar (proc.axustes(), fs);
    auto x = [&] (double hz) { return a.getX() + (float) (std::log (hz / 20.0) / std::log (1000.0)) * a.getWidth(); };
    auto y = [&] (double db) { return a.getCentreY() - (float) juce::jlimit (-1.1, 1.1, db / 18.0) * a.getHeight() * 0.5f; };
    // a reixa: as décadas e cada 6 dB
    g.setFont (caps (6.8f, 0.06f));
    for (double hz : { 20.0, 50.0, 100.0, 200.0, 500.0, 1000.0, 2000.0, 5000.0, 10000.0, 20000.0 })
    {
        g.setColour (col::lina.withAlpha (0.55f)); g.fillRect (x (hz), a.getY(), 0.8f, a.getHeight());
        g.setColour (col::crema3);
        g.drawText (hzTxt (hz), juce::Rectangle<float> (x (hz) - 24.0f, a.getBottom() + 3.0f, 48.0f, 11.0f), juce::Justification::centred, false);
    }
    for (int db = -18; db <= 18; db += 6)
    {
        g.setColour (db == 0 ? col::crema3.withAlpha (0.5f) : col::lina.withAlpha (0.55f));
        g.fillRect (a.getX(), y (db), a.getWidth(), 0.8f);
    }
    auto trazo = [&] (std::function<double (double)> fn, juce::Colour c, float ancho, bool encher)
    {
        juce::Path p;
        for (int k = 0; k <= 240; ++k)
        {
            const double hz = 20.0 * std::pow (1000.0, k / 240.0);
            const juce::Point<float> pt (x (hz), y (fn (hz)));
            if (k == 0) p.startNewSubPath (pt); else p.lineTo (pt);
        }
        if (encher)
        {
            juce::Path q (p); q.lineTo (a.getRight(), y (0)); q.lineTo (a.getX(), y (0)); q.closeSubPath();
            g.setColour (c.withAlpha (0.16f)); g.fillPath (q);
        }
        g.setColour (c.withAlpha (0.25f)); g.strokePath (p, juce::PathStrokeType (ancho + 4.0f));
        g.setColour (c); g.strokePath (p, juce::PathStrokeType (ancho));
    };
    trazo ([&] (double hz) { return f.dbParte (fs, hz, 0); }, col::ouro.withAlpha (0.6f), 1.0f, false);
    trazo ([&] (double hz) { return f.dbParte (fs, hz, 1); }, col::verde.withAlpha (0.6f), 1.0f, false);
    trazo ([&] (double hz) { return f.db (fs, hz); }, BRASA, 2.2f, true);
    // os rótulos: o máis alto e o máis baixo
    double mx = -99, mn = 99, fmx = 1000, fmn = 1000;
    for (int k = 0; k <= 240; ++k)
    {
        const double hz = 20.0 * std::pow (1000.0, k / 240.0), d = f.db (fs, hz);
        if (d > mx) { mx = d; fmx = hz; }
        if (d < mn) { mn = d; fmn = hz; }
    }
    const auto top = r.reduced (16.0f, 0.0f).withTrimmedTop (8.0f).withHeight (12.0f);
    g.setColour (col::crema3); drawCaps (g, "Curva", top, juce::Justification::centredLeft, 6.8f, 0.24f);
    juce::String t;
    if (mx > 0.25) t << dbTxt (mx) << U (" en ") << hzTxt (fmx);
    if (mn < -0.25) t << (t.isEmpty() ? "" : U (" · ")) << dbTxt (mn) << U (" en ") << hzTxt (fmn);
    g.setColour (BRASA); drawCaps (g, t.isEmpty() ? U ("Plana") : t, top, juce::Justification::centredRight, 6.8f, 0.14f);
}

// ================================================================ a válvula
void Valvula::tick()
{
    nivel = juce::jmax (proc.nivelSaida.load(), nivel * 0.92f);
    fase += 0.08f;
    repaint();
}

void Valvula::paint (juce::Graphics& g)
{
    const auto r = getLocalBounds().toFloat();
    g.setColour (col::visor); g.fillRoundedRectangle (r, 12.0f);
    g.setColour (col::lina); g.drawRoundedRectangle (r.reduced (0.5f), 12.0f, 1.0f);
    const float v = (float) (proc.apvts.getRawParameterValue ("valvula")->load() / 100.0);
    const float l = juce::jlimit (0.0f, 1.0f, 0.35f + 0.45f * v + 0.4f * std::sqrt (nivel) + 0.03f * std::sin (fase));
    // o vidro
    auto vidro = juce::Rectangle<float> (r.getCentreX() - 34.0f, r.getY() + 10.0f, 68.0f, r.getHeight() - 40.0f);
    juce::ColourGradient halo (BRASA.withAlpha (0.45f * l), vidro.getCentreX(), vidro.getCentreY() + 8.0f, BRASA.withAlpha (0.0f), vidro.getCentreX() + 60.0f, vidro.getCentreY(), true);
    g.setGradientFill (halo); g.fillEllipse (vidro.expanded (30.0f, 10.0f));
    juce::Path ampola; ampola.addRoundedRectangle (vidro.getX(), vidro.getY(), vidro.getWidth(), vidro.getHeight(), 30.0f, 30.0f, true, true, false, false);
    g.setColour (juce::Colour (0x22c8d8ff)); g.fillPath (ampola);
    g.setColour (juce::Colour (0x66c8d8ff)); g.strokePath (ampola, juce::PathStrokeType (1.2f));
    // as placas e o filamento
    auto placa = vidro.reduced (14.0f, 22.0f);
    g.setColour (juce::Colour (0xff3a3d46)); g.fillRect (placa.removeFromLeft (10.0f));
    g.setColour (juce::Colour (0xff3a3d46)); g.fillRect (placa.removeFromRight (10.0f));
    const auto fil = placa.reduced (4.0f, 4.0f);
    g.setColour (BRASA.withAlpha (0.35f * l)); g.fillRoundedRectangle (fil.expanded (6.0f), 6.0f);
    g.setColour (BRASA.interpolatedWith (BRASA_CLARA, l)); g.fillRoundedRectangle (fil.withSizeKeepingCentre (3.0f, fil.getHeight()), 1.5f);
    // a base
    auto base = juce::Rectangle<float> (vidro.getX() - 4.0f, vidro.getBottom(), vidro.getWidth() + 8.0f, 10.0f);
    g.setColour (juce::Colour (0xff24262e)); g.fillRoundedRectangle (base, 3.0f);
    g.setColour (col::crema3);
    drawCaps (g, U ("Válvula"), r.withTop (r.getBottom() - 14.0f), juce::Justification::centred, 6.5f, 0.3f);
}

// ================================================================ vista principal
MainView::MainView (BTProcessor& p) : proc (p), curva (p), lampada (p)
{
    setLookAndFeel (&look);
    auto key = [this] (juce::TextButton& b, const juce::String& tip) { b.setTooltip (tip); b.setWantsKeyboardFocus (false); addAndMakeVisible (b); };
    presetBtn.getProperties().set ("gold", true);
    presetBtn.onClick = [this] { presetsMenu(); };
    key (presetBtn, U ("Os presets de fábrica: bombo, baixo, voz, caixa, guitarra, piano, a mestura e a válvula soa."));
    prevBtn.setButtonText (U ("‹")); nextBtn.setButtonText (U ("›"));
    for (auto* b : { &prevBtn, &nextBtn, &presetsBtn, &undoBtn, &redoBtn }) b->getProperties().set ("noLed", true);
    prevBtn.onClick = [this] { proc.estado.novoPaso(); proc.cargarPreset ((proc.getPreset() + tubo::NUM_PRESETS - 1) % tubo::NUM_PRESETS); refreshAll(); };
    nextBtn.onClick = [this] { proc.estado.novoPaso(); proc.cargarPreset ((proc.getPreset() + 1) % tubo::NUM_PRESETS); refreshAll(); };
    key (prevBtn, U ("O preset anterior."));
    key (nextBtn, U ("O preset seguinte."));
    presetsBtn.onClick = [this] { showPresetsMenu(); };
    key (presetsBtn, U ("Os teus presets (gardar, abrir a carpeta, copiar entre A e B) e os avisos das versións novas."));
    for (int i = 0; i < 2; ++i)
    {
        abBtn[i].setButtonText (i == 0 ? "A" : "B");
        abBtn[i].onClick = [this, i] { if (i != proc.estado.lado()) proc.estado.cambiarLado(); refreshAll(); };
        key (abBtn[i], U ("A/B: dous axustes para comparar. Cambiar de lado garda o actual."));
    }
    undoBtn.getProperties().set ("curve", -1); redoBtn.getProperties().set ("curve", 1);
    undoBtn.setTitle (U ("Desfacer")); redoBtn.setTitle (U ("Refacer"));
    undoBtn.onClick = [this] { proc.undo.undo(); refreshAll(); };
    redoBtn.onClick = [this] { proc.undo.redo(); refreshAll(); };
    key (undoBtn, U ("Desfacer o último cambio. Cada clic é un paso."));
    key (redoBtn, U ("Refacer."));

    auto knob = [this] (Knob& kn, const char* pid, const juce::String& name, const juce::String& tip, std::function<juce::String (double)> fmt)
    {
        kn.name = name;
        kn.slider.setTooltip (tip);
        kn.att = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, pid, kn.slider);
        kn.slider.textFromValueFunction = std::move (fmt);
        kn.slider.onValueChange = [this] { curva.repaint(); };
        addAndMakeVisible (kn);
    };
    auto dez = [] (double v) { return numero (v, 1); };
    auto lista = [] (const double* f, int n) { return [f, n] (double v) { return hzTxt (f[juce::jlimit (0, n - 1, juce::roundToInt (v))]); }; };
    knob (graves, "graves", "Frecuencia", U ("Onde traballan os graves: 20, 30, 60 ou 100 Hz."), lista (tubo::FREC_GRAVES, tubo::NUM_GRAVES));
    knob (subirG, "subirGraves", "Subir", U ("Sube os graves (ata +13,5 dB), cun bulto na frecuencia. Co Baixar á vez, o truco: peso abaixo e menos barro xusto por riba."), dez);
    knob (baixarG, "baixarGraves", "Baixar", U ("Baixa os graves (ata −17,5 dB), desde máis arriba e máis ancho ca o Subir."), dez);
    knob (agudos, "agudos", "Frecuencia", U ("Onde sobe a campá dos agudos: de 3 a 16 kHz."), lista (tubo::FREC_AGUDOS, tubo::NUM_AGUDOS));
    knob (ancho, "ancho", "Ancho", U ("O ancho da campá: á esquerda, estreita; á dereita, moi ancha (aire sen chiar)."), dez);
    knob (subirA, "subirAgudos", "Subir", U ("Sube os agudos (ata +16 dB) na frecuencia escollida."), dez);
    knob (corte, "corte", "Frecuencia", U ("Desde onde baixan os agudos: 5, 10 ou 20 kHz."), lista (tubo::FREC_CORTE, tubo::NUM_CORTES));
    knob (baixarA, "baixarAgudos", "Baixar", U ("Baixa os agudos (ata −16 dB) cun andel suave."), dez);
    knob (valvula, "valvula", U ("Válvula"), U ("Canto traballa a válvula: pouco, quenta un chisco; moito, harmónicos e algo de compresión."),
          [] (double v) { return juce::String (juce::roundToInt (v)) + " %"; });
    knob (saida, "saida", U ("Saída"), U ("O volume de saída (despois, o limitador a −1 dBFS)."), [] (double v) { return dbTxt (v); });
    saida.slider.setDoubleClickReturnValue (true, 0.0);
    for (auto* k : { &subirG, &baixarG, &subirA, &baixarA }) k->slider.setDoubleClickReturnValue (true, 0.0);
    ancho.slider.setDoubleClickReturnValue (true, 5.0);

    {
        auto* prm = proc.apvts.getParameter ("bypass");
        bypassAtt = std::make_unique<juce::ParameterAttachment> (*prm, [this] (float v) { acendido = v < 0.5f; repaint (luzArea); });
        bypassAtt->sendInitialUpdate();
    }
    curva.setTooltip (U ("A curva enteira (laranxa); en fino, a dos graves e a dos agudos."));
    lampada.setTooltip (U ("A válvula: acéndese co que traballa e co son."));
    addAndMakeVisible (curva);
    addAndMakeVisible (lampada);
    newVersion.getProperties().set ("noLed", true);
    newVersion.onClick = [] { juce::URL (escoita::Actualizacions::downloadsUrl()).launchInDefaultBrowser(); };
    newVersion.setWantsKeyboardFocus (false);
    addChildComponent (newVersion);

    juce::Desktop::getInstance().addGlobalMouseListener (this);
    setSize (W, H);
    refreshAll();
    startTimerHz (30);
}

MainView::~MainView()
{
    juce::Desktop::getInstance().removeGlobalMouseListener (this);
    stopTimer();
    setLookAndFeel (nullptr);
}

void MainView::mouseDown (const juce::MouseEvent& e)
{
    proc.estado.novoPaso();
    if (e.eventComponent == this && luzArea.contains (e.getPosition()))
        bypassAtt->setValueAsCompleteGesture (acendido ? 1.0f : 0.0f);
}

void MainView::presetsMenu()
{
    juce::PopupMenu m;
    m.addSectionHeader (U ("Presets"));
    for (int i = 0; i < tubo::NUM_PRESETS; ++i)
        m.addItem (100 + i, U (tubo::PRESETS[i].nome), true, i == proc.getPreset() && proc.nomePreset() == U (tubo::PRESETS[i].nome));
    m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&presetBtn), [this] (int r)
    {
        if (r < 100) return;
        proc.estado.novoPaso();
        proc.cargarPreset (r - 100);
        refreshAll();
    });
}

void MainView::showPresetsMenu()
{
    juce::PopupMenu m;
    m.addSectionHeader (U ("Os teus presets"));
    proc.estado.engadirAoMenu (m, [sp = juce::Component::SafePointer<MainView> (this)] { if (sp != nullptr) sp->refreshAll(); });
    m.addSeparator();
    m.addSectionHeader (U ("Función"));
    m.addItem (U ("Avisar das versións novas"), true, versions->enabled(), [this] { versions->setEnabled (! versions->enabled()); });
    m.addItem (U ("escoita.link"), [] { juce::URL ("https://www.escoita.link").launchInDefaultBrowser(); });
    m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&presetsBtn));
}

void MainView::refreshState()
{
    for (int i = 0; i < 2; ++i)
        if (abBtn[i].getToggleState() != (i == proc.estado.lado())) abBtn[i].setToggleState (i == proc.estado.lado(), juce::dontSendNotification);
    if (undoBtn.isEnabled() != proc.undo.canUndo()) undoBtn.setEnabled (proc.undo.canUndo());
    if (redoBtn.isEnabled() != proc.undo.canRedo()) redoBtn.setEnabled (proc.undo.canRedo());
    const auto t = proc.nomePreset() + (proc.presetEditado() ? U (" *") : juce::String());
    if (presetBtn.getButtonText() != t) presetBtn.setButtonText (t);
}

void MainView::refreshAll() { refreshState(); curva.repaint(); repaint(); }

void MainView::timerCallback()
{
    refreshState();
    lampada.tick();
    const auto nv = versions->newVersion();
    if (nv.isNotEmpty() != newVersion.isVisible())
    {
        newVersion.setButtonText (U ("Hai unha versión nova: ") + nv + U (" · descargar"));
        newVersion.setVisible (nv.isNotEmpty());
        repaint();
    }
}

void MainView::paint (juce::Graphics& g)
{
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff161e33), 0.0f, 0.0f, juce::Colour (0xff0b101e), 0.0f, (float) H, false));
    g.fillAll();
    const auto panel = juce::Rectangle<float> (20.0f, 16.0f, (float) W - 40.0f, 404.0f);
    for (int i = 3; i >= 1; --i)
    {
        g.setColour (juce::Colours::black.withAlpha (0.12f));
        g.fillRoundedRectangle (panel.translated (0.0f, 2.0f * (float) i).expanded ((float) i), 18.0f + (float) i);
    }
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff1f2947), 0.0f, panel.getY(), juce::Colour (0xff0f152a), 0.0f, panel.getBottom(), false));
    g.fillRoundedRectangle (panel, 18.0f);
    g.setColour (juce::Colour (0xff323e64)); g.drawRoundedRectangle (panel, 18.0f, 1.0f);
    g.setColour (juce::Colours::white.withAlpha (0.08f)); g.fillRect (panel.getX() + 20.0f, panel.getY() + 0.5f, panel.getWidth() - 40.0f, 1.0f);
    for (auto p : { panel.getTopLeft().translated (16, 16), panel.getTopRight().translated (-16, 16), panel.getBottomLeft().translated (16, -16), panel.getBottomRight().translated (-16, -16) })
    {
        g.setColour (juce::Colour (0xff0a0e1c)); g.fillEllipse (p.x - 4.0f, p.y - 4.0f, 8.0f, 8.0f);
        g.setColour (juce::Colour (0xff3c4870)); g.drawLine (p.x - 2.4f, p.y - 2.4f, p.x + 2.4f, p.y + 2.4f, 0.9f);
    }
    g.setColour (col::crema3);
    drawCaps (g, "BARULLO", { 52.0f, 40.0f, 140.0f, 12.0f }, juce::Justification::centredLeft, 8.0f, 0.42f);
    g.setColour (col::crema); g.setFont (cond (24.0f));
    g.drawText ("TUBE EQ", juce::Rectangle<float> (50.0f, 52.0f, 200.0f, 30.0f), juce::Justification::centredLeft, false);
    g.setColour (col::crema3);
    drawCaps (g, U ("Ecualizador pasivo de válvulas"), { 52.0f, 84.0f, 300.0f, 12.0f }, juce::Justification::centredLeft, 7.0f, 0.3f);
    {
        const auto c = luzArea.toFloat().getCentre();
        if (acendido)
        {
            g.setColour (BRASA.withAlpha (0.18f)); g.fillEllipse (c.x - 10.0f, c.y - 10.0f, 20.0f, 20.0f);
            g.setColour (BRASA.withAlpha (0.4f)); g.fillEllipse (c.x - 6.5f, c.y - 6.5f, 13.0f, 13.0f);
            g.setColour (BRASA_CLARA); g.fillEllipse (c.x - 4.5f, c.y - 4.5f, 9.0f, 9.0f);
        }
        else { g.setColour (juce::Colour (0xff2a2f44)); g.fillEllipse (c.x - 4.5f, c.y - 4.5f, 9.0f, 9.0f); }
    }
    auto well = [&] (juce::Component& a, juce::Component& b) { paintWell (g, a.getBounds().getUnion (b.getBounds()).expanded (4).toFloat(), 10.0f); };
    well (prevBtn, nextBtn);
    well (presetsBtn, redoBtn);
    // os grupos dos mandos
    auto grupo = [&] (juce::Component& a, juce::Component& b, const juce::String& t)
    { groupTitle (g, juce::Rectangle<float> ((float) a.getX() + 2.0f, (float) a.getY() - 30.0f, (float) (b.getRight() - a.getX()) - 4.0f, 20.0f), t, BRASA); };
    grupo (graves, baixarG, "Graves");
    grupo (agudos, subirA, U ("Agudos · subir"));
    grupo (corte, baixarA, U ("Agudos · baixar"));
    grupo (valvula, saida, U ("Válvula"));

    const auto r = juce::Rectangle<float> (24.0f, (float) H - 32.0f, (float) W - 48.0f, 14.0f);
    g.setColour (col::crema3);
    const float we = juce::GlyphArrangement::getStringWidth (caps (8.0f, 0.3f), "ESCOITA");
    drawCaps (g, "ESCOITA", r, juce::Justification::centredLeft, 8.0f, 0.3f);
    g.setColour (col::laranxa);
    drawCaps (g, ".", r.withTrimmedLeft (we - 2.0f), juce::Justification::centredLeft, 8.0f, 0.3f);
    g.setColour (col::crema3);
    drawCaps (g, U (" Laboratorio creativo · v") + escoita::Actualizacions::currentVersion(), r.withTrimmedLeft (we + 3.0f), juce::Justification::centredLeft, 8.0f, 0.3f);
    drawCaps (g, U ("42°14′N 8°43′W"), r, juce::Justification::centredRight, 8.0f, 0.3f);
}

void MainView::resized()
{
    {
        int x = 492;
        const int y = 46, h = 30;
        presetBtn.setBounds (x, y, 150, h); x += 160;
        prevBtn.setBounds (x, y, 28, h); x += 32;
        nextBtn.setBounds (x, y, 28, h); x += 42;
        presetsBtn.setBounds (x, y, 66, h); x += 70;
        abBtn[0].setBounds (x, y, 30, h); x += 34;
        abBtn[1].setBounds (x, y, 30, h); x += 40;
        undoBtn.setBounds (x, y, 28, h); x += 32;
        redoBtn.setBounds (x, y, 28, h);
        luzArea = { 186, 55, 24, 24 };
    }
    lampada.setBounds (54, 112, 140, 124);
    curva.setBounds (210, 112, 716, 124);
    const int kw = 82, kh = 112, ky = 284, gap = 22;
    int x = 50;
    auto fila = [&] (std::initializer_list<Knob*> ks) { for (auto* k : ks) { k->setBounds (x, ky, kw, kh); x += kw; } x += gap; };
    fila ({ &graves, &subirG, &baixarG });
    fila ({ &agudos, &ancho, &subirA });
    fila ({ &corte, &baixarA });
    fila ({ &valvula, &saida });
    newVersion.setBounds (W / 2 - 170, H - 36, 340, 22);
}
} // namespace ui

// ================================================================ xanela (escalable)
BTEditor::BTEditor (BTProcessor& p) : AudioProcessorEditor (&p), proc (p), main (p)
{
    addAndMakeVisible (main);
    setResizable (true, true);
    setResizeLimits (ui::MainView::W * 6 / 10, ui::MainView::H * 6 / 10, ui::MainView::W * 2, ui::MainView::H * 2);
    if (auto* c = getConstrainer()) c->setFixedAspectRatio ((double) ui::MainView::W / ui::MainView::H);
    float s = proc.uiScale;
    if (auto* d = juce::Desktop::getInstance().getDisplays().getPrimaryDisplay())
    {
        const auto ua = d->userArea;
        const float fit = juce::jmin ((float) (ua.getWidth() - 40) / (float) ui::MainView::W, (float) (ua.getHeight() - 90) / (float) ui::MainView::H);
        if (s > fit && fit > 0.4f) s = fit;
    }
    setSize (juce::roundToInt (ui::MainView::W * s), juce::roundToInt (ui::MainView::H * s));
}

void BTEditor::resized()
{
    const float s = (float) getWidth() / (float) ui::MainView::W;
    main.setBounds (0, 0, ui::MainView::W, ui::MainView::H);
    main.setTransform (juce::AffineTransform::scale (s));
    proc.uiScale = s;
}
