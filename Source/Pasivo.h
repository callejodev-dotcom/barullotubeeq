// Pasivo.h — O son de Barullo Tube EQ, sen JUCE: un ecualizador pasivo de válvulas coma os dos
// anos 50 e 60. Os filtros son os analóxicos (en s), levados a dixital coa transformada bilineal
// e a frecuencia de cada un no seu sitio (prewarp); despois, a etapa de válvulas.
//
//   entrada ─ graves: subir (andel con bulto) · baixar (andel de 1.ª orde, máis arriba)
//           ─ agudos: subir (campá co seu ancho) · baixar (andel de 1.ª orde)
//           ─ válvula (a dobre frecuencia, fóra deste ficheiro) ─ saída
//
// O truco dos graves: subir e baixar á vez na mesma frecuencia. O andel que sobe ten un bulto
// arredor da frecuencia e o que baixa colle desde máis arriba e máis ancho: o grave sobe e, xusto
// por riba, o barro baixa.
#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <complex>
#include <functional>

namespace tubo {

constexpr double PI = 3.14159265358979323846;

// as posicións dos selectores, coma nos aparellos (Hz)
constexpr int NUM_GRAVES = 4, NUM_AGUDOS = 7, NUM_CORTES = 3;
constexpr double FREC_GRAVES[NUM_GRAVES] = { 20, 30, 60, 100 };
constexpr double FREC_AGUDOS[NUM_AGUDOS] = { 3000, 4000, 5000, 8000, 10000, 12000, 16000 };
constexpr double FREC_CORTE[NUM_CORTES] = { 5000, 10000, 20000 };

struct Axustes
{
    int graves = 2;              // 20, 30, 60, 100 Hz
    double subirGraves = 0;      // 0..10
    double baixarGraves = 0;     // 0..10
    int agudos = 4;              // 3 … 16 kHz
    double ancho = 5;            // 0 estreito … 10 ancho
    double subirAgudos = 0;      // 0..10
    int corte = 1;               // 5, 10, 20 kHz
    double baixarAgudos = 0;     // 0..10
    double valvula = 0.3;        // canto traballa a válvula (0..1)
    double saida = 0;            // dB
};

// o que fai cada mando (dB e Q), tamén para a pantalla
inline double subirGravesDb (double k)  { return 13.5 * std::pow (std::clamp (k, 0.0, 10.0) / 10.0, 1.15); }
inline double baixarGravesDb (double k) { return -17.5 * std::pow (std::clamp (k, 0.0, 10.0) / 10.0, 1.15); }
inline double subirAgudosDb (double k)  { return 16.0 * std::pow (std::clamp (k, 0.0, 10.0) / 10.0, 1.2); }
inline double baixarAgudosDb (double k) { return -16.0 * std::pow (std::clamp (k, 0.0, 10.0) / 10.0, 1.2); }
inline double qAgudos (double ancho)     { return 2.4 * std::pow (0.45 / 2.4, std::clamp (ancho, 0.0, 10.0) / 10.0); }

// un biquad en dixital a partir do seu analóxico H(s) = (n0 + n1 s + n2 s²) / (d0 + d1 s + d2 s²),
// coa frecuencia fw no seu sitio (s → K (z−1)/(z+1), K = ω / tan (ω / 2 fs))
struct Biquad
{
    double b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0, z1 = 0, z2 = 0;
    void reset() { z1 = z2 = 0; }
    double run (double x) { const double y = b0 * x + z1; z1 = b1 * x - a1 * y + z2; z2 = b2 * x - a2 * y; return y; }
    void analoxico (double n0, double n1, double n2, double d0, double d1, double d2, double fs, double fw)
    {
        fw = std::min (fw, fs * 0.45);
        const double w = 2 * PI * fw, K = w / std::tan (w / (2 * fs)), K2 = K * K;
        const double a0 = d0 + d1 * K + d2 * K2;
        b0 = (n0 + n1 * K + n2 * K2) / a0; b1 = 2 * (n0 - n2 * K2) / a0; b2 = (n0 - n1 * K + n2 * K2) / a0;
        a1 = 2 * (d0 - d2 * K2) / a0; a2 = (d0 - d1 * K + d2 * K2) / a0;
    }
    std::complex<double> resposta (double fs, double f) const
    {
        const std::complex<double> z1i = std::polar (1.0, -2 * PI * f / fs), z2i = z1i * z1i;
        return (b0 + b1 * z1i + b2 * z2i) / (1.0 + a1 * z1i + a2 * z2i);
    }
};

// os catro filtros (en analóxico, normalizados: s = s / ω0)
struct Filtros
{
    Biquad subirG, baixarG, subirA, baixarA;

    void deseñar (const Axustes& a, double fs)
    {
        const double fg = FREC_GRAVES[std::clamp (a.graves, 0, NUM_GRAVES - 1)];
        const double fa = FREC_AGUDOS[std::clamp (a.agudos, 0, NUM_AGUDOS - 1)];
        const double fc = FREC_CORTE[std::clamp (a.corte, 0, NUM_CORTES - 1)];
        // graves, subir: andel de 2.ª orde cun pouco de resonancia (o bulto), na frecuencia × 1,6
        {
            const double A = std::pow (10.0, subirGravesDb (a.subirGraves) / 40.0), Q = 0.9, w = 2 * PI * fg * 1.6;
            // H(s) = A (s² + √A/Q s + A) / (A s² + √A/Q s + 1), en s/ω
            subirG.analoxico (A * A, A * std::sqrt (A) / (Q * w), A / (w * w), 1.0, std::sqrt (A) / (Q * w), A / (w * w), fs, fg * 1.6);
        }
        // graves, baixar: andel de 1.ª orde, desde catro veces máis arriba: H(s) = (s + G ω) / (s + ω)
        {
            const double G = std::pow (10.0, baixarGravesDb (a.baixarGraves) / 20.0), w = 2 * PI * fg * 4.0;
            baixarG.analoxico (G * w, 1.0, 0.0, w, 1.0, 0.0, fs, fg * 4.0);
        }
        // agudos, subir: unha campá, co ancho do mando
        {
            const double A = std::pow (10.0, subirAgudosDb (a.subirAgudos) / 40.0), Q = qAgudos (a.ancho), w = 2 * PI * fa;
            // H(s) = (s² + (A/Q) s + 1) / (s² + (1/(A Q)) s + 1), en s/ω
            subirA.analoxico (1.0, A / (Q * w), 1.0 / (w * w), 1.0, 1.0 / (A * Q * w), 1.0 / (w * w), fs, fa);
        }
        // agudos, baixar: andel de 1.ª orde, H(s) = (G s + ω) / (s + ω), coa metade da baixada (en dB) na
        // frecuencia do selector: o polo en fc·√G e o cero en fc/√G
        {
            const double G = std::pow (10.0, baixarAgudosDb (a.baixarAgudos) / 20.0), w = 2 * PI * fc * std::sqrt (G);
            baixarA.analoxico (w, G, 0.0, w, 1.0, 0.0, fs, fc);
        }
    }
    double run (double x) { return baixarA.run (subirA.run (baixarG.run (subirG.run (x)))); }
    void reset() { subirG.reset(); baixarG.reset(); subirA.reset(); baixarA.reset(); }
    // a curva enteira, en dB (para a pantalla e a proba)
    double db (double fs, double f) const
    {
        const auto h = subirG.resposta (fs, f) * baixarG.resposta (fs, f) * subirA.resposta (fs, f) * baixarA.resposta (fs, f);
        return 20.0 * std::log10 (std::abs (h) + 1e-12);
    }
    // o mesmo, por partes: 0 os graves, 1 os agudos
    double dbParte (double fs, double f, int parte) const
    {
        const auto h = parte == 0 ? subirG.resposta (fs, f) * baixarG.resposta (fs, f) : subirA.resposta (fs, f) * baixarA.resposta (fs, f);
        return 20.0 * std::log10 (std::abs (h) + 1e-12);
    }
};

// a etapa de válvulas: con ganancia 1 para sinais pequenos; «drive» empurra cara á curva
inline double valvula (double x, double drive)
{
    constexpr double b = 0.18;
    const double t = std::tanh (b), k = drive;
    return (std::tanh (k * x + b) - t) / (k * (1.0 - t * t));
}
inline double driveDe (double v) { v = std::clamp (v, 0.0, 1.0); return 0.1 + 5.9 * v * v; }   // pouco, case limpa; ao máximo, empurra

// todo o aparello, estéreo, cos mandos suavizados (os coeficientes, cada 32 mostras)
class Pasivo
{
public:
    void preparar (double frecuencia)
    {
        fs = frecuencia;
        k = 1.0 - std::exp (-1.0 / (0.03 * fs));
        actual = obxectivo;
        for (auto& f : canle) { f.deseñar (actual, fs); f.reset(); }
        for (auto& d : dc) d.analoxico (0.0, 1.0, 0.0, 2 * PI * 5.0, 1.0, 0.0, fs, 5.0);
        for (auto& d : dc) d.reset();
        cont = 0;
    }
    void axustar (const Axustes& a) { obxectivo = a; }
    void procesar (double& l, double& r)
    {
        if (cont-- <= 0)
        {
            cont = 31;
            auto suave = [this] (double& v, double o) { v += (o - v) * (1.0 - std::pow (1.0 - k, 32.0)); if (std::abs (o - v) < 1e-6) v = o; };
            suave (actual.subirGraves, obxectivo.subirGraves); suave (actual.baixarGraves, obxectivo.baixarGraves);
            suave (actual.subirAgudos, obxectivo.subirAgudos); suave (actual.baixarAgudos, obxectivo.baixarAgudos);
            suave (actual.ancho, obxectivo.ancho); suave (actual.valvula, obxectivo.valvula); suave (actual.saida, obxectivo.saida);
            actual.graves = obxectivo.graves; actual.agudos = obxectivo.agudos; actual.corte = obxectivo.corte;
            for (auto& f : canle) { const auto gardado = f; f.deseñar (actual, fs); copiarEstados (gardado, f); }
            drive = driveDe (actual.valvula);
            saida = std::pow (10.0, actual.saida / 20.0);
        }
        l = dc[0].run (valvula (canle[0].run (l), drive)) * saida;
        r = dc[1].run (valvula (canle[1].run (r), drive)) * saida;
    }
    const Filtros& filtros() const { return canle[0]; }

private:
    static void copiarEstados (const Filtros& de, Filtros& a)
    {
        a.subirG.z1 = de.subirG.z1; a.subirG.z2 = de.subirG.z2; a.baixarG.z1 = de.baixarG.z1; a.baixarG.z2 = de.baixarG.z2;
        a.subirA.z1 = de.subirA.z1; a.subirA.z2 = de.subirA.z2; a.baixarA.z1 = de.baixarA.z1; a.baixarA.z2 = de.baixarA.z2;
    }
    double fs = 96000, k = 0.001, drive = 1.0, saida = 1.0;
    int cont = 0;
    Axustes obxectivo, actual;
    std::array<Filtros, 2> canle;
    std::array<Biquad, 2> dc;
};

} // namespace tubo
