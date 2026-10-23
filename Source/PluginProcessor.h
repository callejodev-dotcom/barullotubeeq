#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <escoita_nucleo/escoita_nucleo.h>
#include "Pasivo.h"

// Barullo Tube EQ: un ecualizador pasivo de válvulas. Graves (subir e baixar), agudos (subir co seu
// ancho, e baixar), a válvula e a saída. Os presets de fábrica son os programas.
class BTProcessor : public juce::AudioProcessor
{
public:
    BTProcessor();
    ~BTProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;
    juce::AudioProcessorParameter* getBypassParameter() const override { return apvts.getParameter ("bypass"); }

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Barullo Tube EQ"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override;
    int getCurrentProgram() override { return preset; }
    void setCurrentProgram (int index) override { cargarPreset (index); }
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

    juce::UndoManager undo;   // desfacer e refacer (declárase antes ca o apvts)
    juce::AudioProcessorValueTreeState apvts;
    // presets propios (.btqpreset en Documentos/Barullo Tube EQ/Presets), A/B e desfacer, do núcleo
    escoita::Estado estado { apvts, &undo, opcionsEstado() };

    void cargarPreset (int i);
    int getPreset() const { return preset; }
    juce::String nomePreset() const { return nome; }
    bool presetEditado() const;

    tubo::Axustes axustes() const;

    // para a interface (escríbese desde o fío de audio)
    std::atomic<float> nivelEntrada { 0 }, nivelSaida { 0 };
    float uiScale = 1.0f;
    double frecuencia() const { return sr; }

private:
    escoita::Estado::Opcions opcionsEstado();
    int preset = 0;
    juce::String nome;

    tubo::Pasivo pasivo;
    std::unique_ptr<juce::dsp::Oversampling<float>> os;
    escoita::Teito teito;
    double sr = 48000, bypassS = 0;
    int latencia = 0;
    std::vector<float> secoL, secoR, secoOutL, secoOutR, monoR;
    int secoW = 0;
    std::atomic<float> *pGraves = nullptr, *pSubirG = nullptr, *pBaixarG = nullptr, *pAgudos = nullptr, *pAncho = nullptr, *pSubirA = nullptr,
                       *pCorte = nullptr, *pBaixarA = nullptr, *pValvula = nullptr, *pSaida = nullptr, *pBypass = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BTProcessor)
};
