#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Presets.h"

using escoita::U;

namespace
{
    constexpr int ANACO = 512;   // o sobremostrado traballa en anacos de ata 512 mostras
}

BTProcessor::BTProcessor()
    : AudioProcessor (BusesProperties().withInput ("Input", juce::AudioChannelSet::stereo(), true)
                                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, &undo, "BarulloTubeEQ", createLayout())
{
    auto p = [this] (const char* id) { return apvts.getRawParameterValue (id); };
    pGraves = p ("graves"); pSubirG = p ("subirGraves"); pBaixarG = p ("baixarGraves"); pAgudos = p ("agudos"); pAncho = p ("ancho");
    pSubirA = p ("subirAgudos"); pCorte = p ("corte"); pBaixarA = p ("baixarAgudos"); pValvula = p ("valvula"); pSaida = p ("saida");
    pBypass = p ("bypass");
    nome = U (tubo::PRESETS[0].nome);
}

juce::AudioProcessorValueTreeState::ParameterLayout BTProcessor::createLayout()
{
    using namespace juce;
    AudioProcessorValueTreeState::ParameterLayout l;
    const auto& p0 = tubo::PRESETS[0];
    auto real = [] (const char* id, const String& nomeP, NormalisableRange<float> r, float def, const char* unidade, int decimais = 1)
    {
        return std::make_unique<AudioParameterFloat> (ParameterID { id, 1 }, nomeP, r, def, AudioParameterFloatAttributes().withLabel (unidade)
                   .withStringFromValueFunction ([decimais] (float v, int) { return String (v, decimais); }));
    };
    auto hz = [] (const double* f, int n) { StringArray s; for (int i = 0; i < n; ++i) s.add (f[i] < 1000 ? String ((int) f[i]) + " Hz" : String ((int) (f[i] / 1000)) + " kHz"); return s; };
    l.add (std::make_unique<AudioParameterChoice> (ParameterID { "graves", 1 }, "Graves: frecuencia", hz (tubo::FREC_GRAVES, tubo::NUM_GRAVES), p0.graves));
    l.add (real ("subirGraves", "Graves: subir", { 0.0f, 10.0f }, (float) p0.subirGraves, ""));
    l.add (real ("baixarGraves", "Graves: baixar", { 0.0f, 10.0f }, (float) p0.baixarGraves, ""));
    l.add (std::make_unique<AudioParameterChoice> (ParameterID { "agudos", 1 }, "Agudos: frecuencia", hz (tubo::FREC_AGUDOS, tubo::NUM_AGUDOS), p0.agudos));
    l.add (real ("ancho", "Agudos: ancho", { 0.0f, 10.0f }, (float) p0.ancho, ""));
    l.add (real ("subirAgudos", "Agudos: subir", { 0.0f, 10.0f }, (float) p0.subirAgudos, ""));
    l.add (std::make_unique<AudioParameterChoice> (ParameterID { "corte", 1 }, "Corte: frecuencia", hz (tubo::FREC_CORTE, tubo::NUM_CORTES), p0.corte));
    l.add (real ("baixarAgudos", "Agudos: baixar", { 0.0f, 10.0f }, (float) p0.baixarAgudos, ""));
    l.add (real ("valvula", U ("Válvula"), { 0.0f, 100.0f }, (float) p0.valvula * 100.0f, "%"));
    l.add (real ("saida", U ("Saída"), { -12.0f, 12.0f, 0.1f }, (float) p0.saida, "dB"));
    l.add (std::make_unique<AudioParameterChoice> (ParameterID { "bypass", 1 }, "Bypass", StringArray { "Off", "On" }, 0));
    return l;
}

// ================================================================ presets
int BTProcessor::getNumPrograms() { return tubo::NUM_PRESETS; }
const juce::String BTProcessor::getProgramName (int i) { return U (tubo::PRESETS[juce::jlimit (0, tubo::NUM_PRESETS - 1, i)].nome); }

static std::vector<std::pair<const char*, float>> valoresDe (const tubo::Preset& p)
{
    return { { "graves", (float) p.graves }, { "subirGraves", (float) p.subirGraves }, { "baixarGraves", (float) p.baixarGraves },
             { "agudos", (float) p.agudos }, { "ancho", (float) p.ancho }, { "subirAgudos", (float) p.subirAgudos },
             { "corte", (float) p.corte }, { "baixarAgudos", (float) p.baixarAgudos }, { "valvula", (float) p.valvula * 100.0f }, { "saida", (float) p.saida } };
}

void BTProcessor::cargarPreset (int i)
{
    i = juce::jlimit (0, tubo::NUM_PRESETS - 1, i);
    const auto antes = std::make_pair (preset, nome);
    for (auto [id, v] : valoresDe (tubo::PRESETS[i]))
        if (auto* prm = apvts.getParameter (id))
        {
            const float n = prm->convertTo0to1 (v);
            if (std::abs (prm->getValue() - n) < 1.0e-6f) continue;
            prm->beginChangeGesture(); prm->setValueNotifyingHost (n); prm->endChangeGesture();
        }
    const auto despois = std::make_pair (i, U (tubo::PRESETS[i].nome));
    estado.cambio (U ("preset"), [this, despois] { preset = despois.first; nome = despois.second; },
                                 [this, antes] { preset = antes.first; nome = antes.second; });
    updateHostDisplay (ChangeDetails().withProgramChanged (true));
}

bool BTProcessor::presetEditado() const
{
    const int i = juce::jlimit (0, tubo::NUM_PRESETS - 1, preset);
    if (nome != U (tubo::PRESETS[i].nome)) return false;
    for (auto [id, v] : valoresDe (tubo::PRESETS[i]))
        if (auto* prm = apvts.getParameter (id))
            if (std::abs (prm->getValue() - prm->convertTo0to1 (v)) > 0.002f) return true;
    return false;
}

escoita::Estado::Opcions BTProcessor::opcionsEstado()
{
    escoita::Estado::Opcions o;
    o.extension = "btqpreset";
    o.excluir = { "bypass" };
    o.extra = [this] { juce::ValueTree t ("Preset"); t.setProperty ("numero", preset, nullptr); t.setProperty ("nome", nome, nullptr); return t; };
    o.aplicarExtra = [this] (const juce::ValueTree& t) { preset = juce::jlimit (0, tubo::NUM_PRESETS - 1, (int) t.getProperty ("numero", 0)); nome = t.getProperty ("nome", nome).toString(); };
    return o;
}

tubo::Axustes BTProcessor::axustes() const
{
    tubo::Axustes a;
    a.graves = (int) std::lround (pGraves->load()); a.subirGraves = pSubirG->load(); a.baixarGraves = pBaixarG->load();
    a.agudos = (int) std::lround (pAgudos->load()); a.ancho = pAncho->load(); a.subirAgudos = pSubirA->load();
    a.corte = (int) std::lround (pCorte->load()); a.baixarAgudos = pBaixarA->load();
    a.valvula = pValvula->load() / 100.0; a.saida = std::round ((double) pSaida->load() * 10.0) / 10.0;
    return a;
}

// ================================================================ audio
bool BTProcessor::isBusesLayoutSupported (const BusesLayout& l) const
{
    const auto in = l.getMainInputChannelSet(), out = l.getMainOutputChannelSet();
    if (out != juce::AudioChannelSet::stereo() && out != juce::AudioChannelSet::mono()) return false;
    return in == out || (in == juce::AudioChannelSet::mono() && out == juce::AudioChannelSet::stereo());
}

void BTProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    sr = sampleRate;
    // a válvula, a dobre frecuencia (os seus harmónicos non volven como ruído); os filtros tamén
    os = std::make_unique<juce::dsp::Oversampling<float>> (2, 1, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true, false);
    os->initProcessing ((size_t) ANACO);
    pasivo.axustar (axustes());
    pasivo.preparar (sr * 2.0);
    latencia = (int) std::lround (os->getLatencyInSamples());
    setLatencySamples (latencia);
    secoL.assign ((size_t) latencia + 1, 0.0f); secoR.assign ((size_t) latencia + 1, 0.0f); secoW = 0;
    const int n = std::max (samplesPerBlock, ANACO);
    secoOutL.assign ((size_t) n, 0.0f); secoOutR.assign ((size_t) n, 0.0f); monoR.assign ((size_t) n, 0.0f);
    teito.preparar (sr, 0.080, 0.891);
    bypassS = pBypass->load() > 0.5f ? 1.0 : 0.0;
}

void BTProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int n = buffer.getNumSamples();
    const int ins = getTotalNumInputChannels(), outs = getTotalNumOutputChannels();
    float* L = buffer.getWritePointer (0);
    float* R = outs > 1 ? buffer.getWritePointer (1) : nullptr;
    if (R != nullptr && ins < 2) std::copy (L, L + n, R);   // mono → estéreo
    if (n > (int) secoOutL.size()) { secoOutL.resize ((size_t) n); secoOutR.resize ((size_t) n); monoR.resize ((size_t) n); }
    const bool mono = R == nullptr;
    if (mono) { std::copy (L, L + n, monoR.begin()); R = monoR.data(); }

    pasivo.axustar (axustes());

    // o seco (para o bypass), atrasado o que tarda o sobremostrado
    float pe = 0;
    const int N = (int) secoL.size();
    for (int i = 0; i < n; ++i)
    {
        pe = std::max ({ pe, std::abs (L[i]), std::abs (R[i]) });
        secoL[(size_t) secoW] = L[i]; secoR[(size_t) secoW] = R[i];
        const int ri = (secoW + 1) % N;
        secoOutL[(size_t) i] = secoL[(size_t) ri]; secoOutR[(size_t) i] = secoR[(size_t) ri];
        secoW = (secoW + 1) % N;
    }

    for (int pos = 0; pos < n; pos += ANACO)
    {
        const int k = std::min (ANACO, n - pos);
        float* ch[2] = { L + pos, R + pos };
        juce::dsp::AudioBlock<float> bloque (ch, 2, (size_t) k);
        auto alto = os->processSamplesUp (bloque);
        float* a = alto.getChannelPointer (0);
        float* b = alto.getChannelPointer (1);
        for (size_t i = 0; i < alto.getNumSamples(); ++i)
        {
            double l = a[i], r = b[i];
            pasivo.procesar (l, r);
            a[i] = (float) l; b[i] = (float) r;
        }
        os->processSamplesDown (bloque);
    }

    const double bypassT = pBypass->load() > 0.5f ? 1.0 : 0.0;
    const double sm = 1.0 - std::exp (-1.0 / (0.02 * sr));
    float ps = 0;
    for (int i = 0; i < n; ++i)
    {
        bypassS += (bypassT - bypassS) * sm;
        if (std::abs (bypassS - bypassT) < 1e-6) bypassS = bypassT;
        double l = L[i], r = R[i];
        teito.procesar (l, r);
        l = l * (1.0 - bypassS) + secoOutL[(size_t) i] * bypassS;
        r = r * (1.0 - bypassS) + secoOutR[(size_t) i] * bypassS;
        L[i] = (float) l; R[i] = (float) r;
        ps = std::max ({ ps, (float) std::abs (l), (float) std::abs (r) });
    }
    if (mono) for (int i = 0; i < n; ++i) L[i] = 0.5f * (L[i] + R[i]);
    for (int ch = 2; ch < outs; ++ch) buffer.clear (ch, 0, n);
    nivelEntrada.store (pe); nivelSaida.store (ps);
}

// ================================================================ estado
void BTProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto st = apvts.copyState();
    st.setProperty ("preset", preset, nullptr);
    st.setProperty ("nomePreset", nome, nullptr);
    st.setProperty ("uiScale", uiScale, nullptr);
    if (auto xml = st.createXml()) copyXmlToBinary (*xml, destData);
}

void BTProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto xml = getXmlFromBinary (data, sizeInBytes);
    if (! xml || ! xml->hasTagName (apvts.state.getType())) return;
    auto tree = juce::ValueTree::fromXml (*xml);
    uiScale = juce::jlimit (0.5f, 2.0f, (float) tree.getProperty ("uiScale", 1.0f));
    preset = juce::jlimit (0, tubo::NUM_PRESETS - 1, (int) tree.getProperty ("preset", 0));
    nome = tree.getProperty ("nomePreset", U (tubo::PRESETS[preset].nome)).toString();
    apvts.replaceState (tree);
    updateHostDisplay (ChangeDetails().withProgramChanged (true));   // que o DAW volva ler os valores
}

juce::AudioProcessorEditor* BTProcessor::createEditor() { return new BTEditor (*this); }

#ifndef BARULLO_NO_PLUGIN_ENTRY
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new BTProcessor(); }
#endif
