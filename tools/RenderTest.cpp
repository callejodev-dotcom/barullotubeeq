// Proba sen interface de Barullo Tube EQ: as curvas medidas (graves, o truco, agudos co seu ancho,
// o corte) e que son as da pantalla, a válvula, o teito, os bloques, o bypass, o estado, o A/B, os
// presets propios e o desfacer; tamén capturas.
//   BQRenderTest [saida.wav]                          proba enteira (devolve o número de fallos)
//   BQRenderTest --snapshot saida.png                 a interface
//   BQRenderTest --demo preset entrada.wav saida.wav  un preset sobre un son
#include <juce_audio_formats/juce_audio_formats.h>
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Presets.h"
#include <cstdio>
#include <cstring>

using escoita::U;
static constexpr double SR = 48000;

static void writeWav (const juce::File& f, const juce::AudioBuffer<float>& b, double sr)
{
    f.deleteFile();
    juce::WavAudioFormat wav;
    if (auto os = std::unique_ptr<juce::OutputStream> (f.createOutputStream()))
        if (auto w = std::unique_ptr<juce::AudioFormatWriter> (wav.createWriterFor (os.get(), sr, (unsigned) b.getNumChannels(), 24, {}, 0)))
        {
            os.release();
            w->writeFromAudioSampleBuffer (b, 0, b.getNumSamples());
            std::printf ("WAV: %s\n", f.getFullPathName().toRawUTF8());
        }
}

static juce::AudioBuffer<float> procesar (BTProcessor& p, const juce::AudioBuffer<float>& in, double len, int bloque, bool preparar = true)
{
    if (preparar) { p.setPlayConfigDetails (2, 2, SR, bloque); p.prepareToPlay (SR, bloque); }
    const int total = (int) (len * SR);
    juce::AudioBuffer<float> out (2, total), buf (2, bloque);
    juce::MidiBuffer midi;
    for (int pos = 0; pos < total; pos += bloque)
    {
        const int n = juce::jmin (bloque, total - pos);
        buf.setSize (2, n, false, false, true);
        buf.clear();
        for (int c = 0; c < 2; ++c)
            for (int i = 0; i < n; ++i)
                if (pos + i < in.getNumSamples()) buf.setSample (c, i, in.getSample (c % in.getNumChannels(), pos + i));
        p.processBlock (buf, midi);
        for (int c = 0; c < 2; ++c) out.copyFrom (c, pos, buf, c, 0, n);
    }
    return out;
}

static void setParam (BTProcessor& p, const juce::String& id, float v)
{
    if (auto* prm = p.apvts.getParameter (id)) prm->setValueNotifyingHost (prm->convertTo0to1 (v));
}

static juce::AudioBuffer<float> seno (double f, double rmsDb, double len, double t0 = 0.0, double t1 = 1e9)
{
    juce::AudioBuffer<float> b (2, (int) (len * SR));
    const double a = std::pow (10.0, rmsDb / 20.0) * std::sqrt (2.0);
    for (int i = 0; i < b.getNumSamples(); ++i)
    {
        const double t = i / SR;
        const float x = (t >= t0 && t < t1) ? (float) (a * std::sin (2 * tubo::PI * f * t)) : 0.0f;
        b.setSample (0, i, x); b.setSample (1, i, x);
    }
    return b;
}
// o son de proba: golpes de ruído e un acorde
static juce::AudioBuffer<float> sonDeProba (double len = 2.0)
{
    juce::AudioBuffer<float> b (2, (int) (len * SR));
    b.clear();
    juce::Random rnd (1984);
    for (double t0 : { 0.0, 0.25, 0.5, 0.875 })
        for (int i = 0; i < (int) (0.12 * SR); ++i)
        {
            const int k = (int) (t0 * SR) + i;
            if (k >= b.getNumSamples()) break;
            const float e = 0.5f * std::exp (-(float) i / (float) (0.025 * SR));
            for (int c = 0; c < 2; ++c) b.setSample (c, k, b.getSample (c, k) + e * (rnd.nextFloat() * 2 - 1));
        }
    for (double f : { 65.41, 261.63, 329.63, 392.0 })
        for (int i = 0; i < (int) (0.8 * SR); ++i)
        {
            const int k = (int) (1.1 * SR) + i;
            if (k >= b.getNumSamples()) break;
            const float x = 0.1f * (float) std::sin (2 * tubo::PI * f * i / SR) * std::exp (-(float) i / (float) (0.4 * SR)) * juce::jmin (1.0f, (float) i / 100.0f);
            for (int c = 0; c < 2; ++c) b.setSample (c, k, b.getSample (c, k) + x);
        }
    return b;
}
static double rmsDb (const juce::AudioBuffer<float>& b, double t0, double t1)
{
    double e = 0; int n = 0;
    for (int c = 0; c < 2; ++c)
        for (int i = (int) (t0 * SR); i < juce::jmin (b.getNumSamples(), (int) (t1 * SR)); ++i) { e += (double) b.getSample (c, i) * b.getSample (c, i); ++n; }
    return 10.0 * std::log10 (e / juce::jmax (1, n) + 1e-30);
}
static float pico (const juce::AudioBuffer<float>& b) { return juce::jmax (b.getMagnitude (0, 0, b.getNumSamples()), b.getMagnitude (1, 0, b.getNumSamples())); }
static bool finito (const juce::AudioBuffer<float>& b)
{
    for (int c = 0; c < 2; ++c) for (int i = 0; i < b.getNumSamples(); ++i) if (! std::isfinite (b.getSample (c, i))) return false;
    return true;
}
static float diferenza (const juce::AudioBuffer<float>& a, const juce::AudioBuffer<float>& b)
{
    float d = 0;
    for (int c = 0; c < 2; ++c)
        for (int i = 0; i < juce::jmin (a.getNumSamples(), b.getNumSamples()); ++i) d = juce::jmax (d, std::abs (a.getSample (c, i) - b.getSample (c, i)));
    return d;
}

// a ganancia (dB) dun seno pola ferramenta, no estado estable
static double ganancia (BTProcessor& p, double f, double rms = -24.0)
{
    const auto o = procesar (p, seno (f, rms, 1.0), 1.0, 512);
    return rmsDb (o, 0.5, 1.0) - rms;
}
static double ganancia (const std::vector<std::pair<const char*, float>>& v, double f, double rms = -24.0)
{
    BTProcessor p;
    setParam (p, "valvula", 0.0f);
    for (auto [id, x] : v) setParam (p, id, x);
    return ganancia (p, f, rms);
}

static void snapshot (const juce::File& png)
{
    BTProcessor p;
    p.setPlayConfigDetails (2, 2, SR, 512);
    p.prepareToPlay (SR, 512);
    p.cargarPreset (1);
    std::unique_ptr<juce::AudioProcessorEditor> ed (p.createEditor());
    auto* ce = dynamic_cast<BTEditor*> (ed.get());
    ed->setSize (ui::MainView::W, ui::MainView::H);
    const auto son = sonDeProba (2.0);
    juce::AudioBuffer<float> buf (2, 512);
    for (int blk = 0; blk < 60; ++blk)
    {
        buf.clear();
        for (int c = 0; c < 2; ++c) buf.copyFrom (c, 0, son, c, (blk * 512) % (son.getNumSamples() - 512), 512);
        juce::MidiBuffer m; p.processBlock (buf, m);
        ce->view().timerCallback();
    }
    auto img = ed->createComponentSnapshot (ed->getLocalBounds(), true, 1.5f);
    png.deleteFile();
    juce::FileOutputStream os (png);
    juce::PNGImageFormat().writeImageToStream (img, os);
    std::printf ("PNG: %s\n", png.getFullPathName().toRawUTF8());
}

int main (int argc, char** argv)
{
    std::setvbuf (stdout, nullptr, _IONBF, 0);
    juce::ScopedJuceInitialiser_GUI init;
    const juce::String mode = argc > 1 ? juce::String (argv[1]) : juce::String();

    if (mode == "--snapshot" && argc > 2) { snapshot (juce::File (juce::String (argv[2]))); return 0; }
    if (mode == "--demo" && argc > 4)
    {
        juce::AudioFormatManager fm; fm.registerBasicFormats();
        std::unique_ptr<juce::AudioFormatReader> r (fm.createReaderFor (juce::File (juce::String (argv[3]))));
        if (r == nullptr) { std::printf ("non se pode ler %s\n", argv[3]); return 1; }
        juce::AudioBuffer<float> in ((int) r->numChannels, (int) r->lengthInSamples);
        r->read (&in, 0, (int) r->lengthInSamples, 0, true, true);
        BTProcessor p;
        p.cargarPreset (atoi (argv[2]));
        p.setPlayConfigDetails (2, 2, r->sampleRate, 512); p.prepareToPlay (r->sampleRate, 512);
        const int L = p.getLatencySamples();
        juce::AudioBuffer<float> out (2, in.getNumSamples() + L), buf (2, 512);
        juce::MidiBuffer m;
        for (int pos = 0; pos < out.getNumSamples(); pos += 512)
        {
            const int n = juce::jmin (512, out.getNumSamples() - pos);
            buf.setSize (2, n, false, false, true); buf.clear();
            for (int c = 0; c < 2; ++c) for (int i = 0; i < n; ++i) if (pos + i < in.getNumSamples()) buf.setSample (c, i, in.getSample (c % in.getNumChannels(), pos + i));
            p.processBlock (buf, m);
            for (int c = 0; c < 2; ++c) out.copyFrom (c, pos, buf, c, 0, n);
        }
        juce::AudioBuffer<float> sen (2, in.getNumSamples());
        for (int c = 0; c < 2; ++c) sen.copyFrom (c, 0, out, c, L, in.getNumSamples());
        writeWav (juce::File (juce::String (argv[4])), sen, r->sampleRate);
        return 0;
    }

    int fails = 0;
    auto check = [&fails] (bool ok, const juce::String& what, const juce::String& extra = {})
    {
        std::printf ("%-52s %s %s\n", what.toRawUTF8(), ok ? "OK" : "FALLO", extra.toRawUTF8());
        fails += ok ? 0 : 1;
    };

    {   // ao abrir: o primeiro preset, cos parámetros nos seus valores de partida
        BTProcessor p;
        juce::String mal;
        for (auto* prm : p.getParameters())
            if (auto* r = dynamic_cast<juce::RangedAudioParameter*> (prm))
                if (std::abs (r->getValue() - r->getDefaultValue()) > 1e-6f) mal << r->getParameterID() << " ";
        check (mal.isEmpty() && p.getPreset() == 0 && ! p.presetEditado(), "ao abrir: o primeiro preset, nos valores de partida", mal);
    }
    {   // plano e sen válvula: case nada
        double peor = 0;
        for (double f : { 30.0, 100.0, 1000.0, 5000.0, 15000.0 }) peor = std::max (peor, std::abs (ganancia ({}, f)));
        check (peor < 0.15, "plano: o son case tal cal", juce::String::formatted ("(%.2f dB como moito)", peor));
    }
    {   // graves: subir en 60 Hz ao 10; o truco (subir 6 e baixar 4): peso abaixo e menos barro por riba
        const double g40 = ganancia ({ { "subirGraves", 10.0f } }, 40.0), g1k = ganancia ({ { "subirGraves", 10.0f } }, 1000.0);
        check (g40 > 12.0 && g40 < 15.0 && std::abs (g1k) < 0.3, U ("graves: subir 10 en 60 Hz"), juce::String::formatted ("(%.1f dB en 40 Hz; %.2f en 1 kHz)", g40, g1k));
        const std::vector<std::pair<const char*, float>> truco { { "subirGraves", 6.0f }, { "baixarGraves", 4.0f } };
        const double t50 = ganancia (truco, 50.0), t200 = ganancia (truco, 200.0);
        check (t50 > 1.0 && t200 < -1.5, U ("o truco: subir e baixar á vez en 60 Hz"), juce::String::formatted ("(%+.1f dB en 50 Hz; %+.1f en 200 Hz)", t50, t200));
    }
    {   // agudos: a campá en 10 kHz ao 10, estreita e ancha; e o corte
        const double e10 = ganancia ({ { "subirAgudos", 10.0f }, { "ancho", 0.0f } }, 10000.0);
        const double e5 = ganancia ({ { "subirAgudos", 10.0f }, { "ancho", 0.0f } }, 5000.0), a5 = ganancia ({ { "subirAgudos", 10.0f }, { "ancho", 10.0f } }, 5000.0);
        check (e10 > 14.0 && e10 < 17.5 && a5 > e5 + 4.0, U ("agudos: a campá e o seu ancho"), juce::String::formatted ("(%.1f dB en 10 kHz; en 5 kHz %.1f estreita / %.1f ancha)", e10, e5, a5));
        const double c16 = ganancia ({ { "baixarAgudos", 10.0f } }, 16000.0), c1k = ganancia ({ { "baixarAgudos", 10.0f } }, 1000.0);
        check (c16 < -8.0 && std::abs (c1k) < 0.3, U ("agudos: baixar desde 10 kHz"), juce::String::formatted ("(%.1f dB en 16 kHz; %.2f en 1 kHz)", c16, c1k));
    }
    {   // a curva da pantalla é a que soa
        BTProcessor p;
        p.cargarPreset (1);
        setParam (p, "valvula", 0.0f); setParam (p, "saida", 0.0f);
        tubo::Filtros f; f.deseñar (p.axustes(), 2.0 * SR);
        double peor = 0;
        for (double hz : { 40.0, 60.0, 150.0, 300.0, 1000.0, 5000.0, 12000.0 })
        {
            BTProcessor q; q.cargarPreset (1); setParam (q, "valvula", 0.0f); setParam (q, "saida", 0.0f);
            peor = std::max (peor, std::abs (ganancia (q, hz) - f.db (2.0 * SR, hz)));
        }
        check (peor < 0.3, U ("a curva da pantalla é a que soa"), juce::String::formatted ("(%.2f dB como moito)", peor));
    }
    {   // a válvula: o segundo harmónico medra co mando
        auto h2 = [&] (float v)
        {
            BTProcessor p;
            setParam (p, "valvula", v);
            const auto o = procesar (p, seno (200.0, -12.0, 1.0), 1.0, 512);
            double re = 0, im = 0, re1 = 0, im1 = 0;
            for (int i = (int) (0.5 * SR); i < (int) SR; ++i)
            {
                const double x = o.getSample (0, i), a2 = 2 * tubo::PI * 400.0 * i / SR, a1 = 2 * tubo::PI * 200.0 * i / SR;
                re += x * std::cos (a2); im += x * std::sin (a2); re1 += x * std::cos (a1); im1 += x * std::sin (a1);
            }
            return 20.0 * std::log10 (std::sqrt (re * re + im * im) / std::sqrt (re1 * re1 + im1 * im1));
        };
        const double pouco = h2 (10.0f), moito = h2 (100.0f);
        check (moito > pouco + 10.0 && moito < -10.0, U ("válvula: o segundo harmónico co mando"), juce::String::formatted ("(%.0f / %.0f dB)", pouco, moito));
    }
    {   // o teito
        BTProcessor p;
        setParam (p, "subirGraves", 10.0f); setParam (p, "subirAgudos", 10.0f); setParam (p, "saida", 12.0f); setParam (p, "valvula", 100.0f);
        auto son = sonDeProba (2.0); son.applyGain (1.8f);
        const auto o = procesar (p, son, 2.0, 512);
        check (finito (o) && pico (o) <= 0.8913f, "teito a -1 dBFS", juce::String::formatted ("(pico %.4f)", pico (o)));
    }
    {   // bloques (o sobremostrado de JUCE, en Intel, leva a cero os estados por debaixo de 1e-8)
        auto render = [&] (int bloque) { BTProcessor p; p.cargarPreset (1); return procesar (p, sonDeProba (2.0), 2.5, bloque); };
        const auto x = render (512), y = render (37);
        const float d = diferenza (x, y);
        check (d < 1e-5f && finito (x) && pico (x) > 0.05f, "mesmo son con bloques de 512 e de 37", juce::String::formatted ("(%.1e)", (double) d));
    }
    {   // bypass
        BTProcessor p;
        p.cargarPreset (9); setParam (p, "bypass", 1.0f);
        const auto son = sonDeProba (1.0);
        const auto o = procesar (p, son, 1.0, 512);
        const int L = p.getLatencySamples();
        float d = 0;
        for (int c = 0; c < 2; ++c) for (int i = L; i < o.getNumSamples(); ++i) d = juce::jmax (d, std::abs (o.getSample (c, i) - son.getSample (c, i - L)));
        check (std::equal_to<float>() (d, 0.0f), "bypass: o son tal cal (coa latencia)", "(" + juce::String (L) + " mostras)");
    }
    {   // os presets
        int mal = 0; float peor = 0;
        for (int i = 0; i < tubo::NUM_PRESETS; ++i)
        {
            BTProcessor p; p.cargarPreset (i);
            const auto o = procesar (p, sonDeProba (2.0), 2.0, 512);
            if (! finito (o) || pico (o) < 0.01f || p.presetEditado()) { ++mal; std::printf ("   mal: %s\n", tubo::PRESETS[i].nome); }
            peor = juce::jmax (peor, pico (o));
        }
        check (mal == 0 && peor <= 0.8913f, "os presets soan", "(" + juce::String (tubo::NUM_PRESETS) + ")");
    }
    {   // mono → estéreo
        BTProcessor p;
        juce::AudioProcessor::BusesLayout l;
        l.inputBuses.add (juce::AudioChannelSet::mono()); l.outputBuses.add (juce::AudioChannelSet::stereo());
        const bool soporta = p.setBusesLayout (l);
        p.prepareToPlay (SR, 512);
        juce::AudioBuffer<float> buf (2, 512);
        const auto son = sonDeProba (0.5);
        bool fin = true;
        for (int pos = 0; pos + 512 <= son.getNumSamples(); pos += 512)
        {
            buf.clear(); buf.copyFrom (0, 0, son, 0, pos, 512);
            juce::MidiBuffer m; p.processBlock (buf, m);
            fin = fin && std::isfinite (buf.getMagnitude (0, 512));
        }
        check (soporta && fin, U ("mono a estéreo"));
    }
    {   // estado
        BTProcessor a;
        a.cargarPreset (3);
        setParam (a, "saida", -3.0f); a.uiScale = 0.8f;
        juce::MemoryBlock mb; a.getStateInformation (mb);
        BTProcessor b; b.setStateInformation (mb.getData(), (int) mb.getSize());
        const auto x = procesar (a, sonDeProba (1.0), 1.5, 512), y = procesar (b, sonDeProba (1.0), 1.5, 512);
        check (diferenza (x, y) < 1e-6f && b.getPreset() == 3 && b.presetEditado() && std::abs (b.uiScale - 0.8f) < 1e-6f, "estado gardado e recuperado",
               "(" + juce::String ((int) mb.getSize()) + " bytes)");
    }
    {   // A/B e presets propios
        BTProcessor p;
        p.cargarPreset (0);
        p.estado.cambiarLado(); p.cargarPreset (6); p.estado.cambiarLado();
        bool ok = p.estado.lado() == 0 && p.getPreset() == 0 && ! p.presetEditado();
        p.estado.cambiarLado();
        ok = ok && p.getPreset() == 6;
        check (ok, "A/B (os mandos e o preset)");
        const auto dir = juce::File::getSpecialLocation (juce::File::tempDirectory).getChildFile ("barullotubeeq_presets_proba");
        dir.deleteRecursively();
        p.estado.setCarpeta (dir);
        setParam (p, "subirAgudos", 7.0f);
        const bool gardado = p.estado.gardar ("Proba: caixa con aire");
        p.cargarPreset (3);
        const auto list = p.estado.presets();
        const bool cargado = list.size() == 1 && list[0].hasFileExtension ("btqpreset") && p.estado.cargar (list[0]);
        check (gardado && cargado && std::abs (p.apvts.getRawParameterValue ("subirAgudos")->load() - 7.0f) < 0.01f, "presets propios: gardar e cargar");
        dir.deleteRecursively();
    }
    {   // desfacer
        BTProcessor p;
        p.undo.clearUndoHistory();
        p.estado.novoPaso(); p.cargarPreset (4); p.apvts.copyState(); p.estado.novoPaso();
        setParam (p, "subirGraves", 9.0f); p.apvts.copyState(); p.estado.novoPaso();
        p.undo.undo();
        const bool u1 = p.getPreset() == 4 && ! p.presetEditado();
        p.undo.undo();
        const bool u2 = p.getPreset() == 0 && ! p.presetEditado();
        check (u1 && u2, "desfacer (preset e mandos)");
    }
    {   // os programas do DAW son os presets
        BTProcessor p;
        p.setCurrentProgram (5);
        check (p.getNumPrograms() == tubo::NUM_PRESETS && p.getCurrentProgram() == 5 && p.getProgramName (5) == U (tubo::PRESETS[5].nome), "programas do DAW = presets");
    }
    return fails;
}
