// Presets.h — Os presets de fábrica de Barullo Tube EQ. O primeiro é o de partida (os parámetros saen del).
#pragma once
#include "Pasivo.h"

namespace tubo {

struct Preset
{
    const char* nome;
    const char* descricion;
    int graves; double subirGraves, baixarGraves;
    int agudos; double ancho, subirAgudos;
    int corte; double baixarAgudos;
    double valvula, saida;
};

// graves: 0 = 20, 1 = 30, 2 = 60, 3 = 100 Hz · agudos: 0 = 3, 1 = 4, 2 = 5, 3 = 8, 4 = 10, 5 = 12, 6 = 16 kHz · corte: 0 = 5, 1 = 10, 2 = 20 kHz
inline const Preset PRESETS[] = {
    //  nome                  descrición                                                                     gr  sub  bai   ag anc  sub   co  bai   válv  saída
    { "Plano",                "Sen tocar as curvas: só a válvula, cun chisco. Para quentar.",                  2, 0.0, 0.0,  4, 5.0, 0.0,  1, 0.0,  0.30,  0.0 },
    { "Bombo groso",          "O truco: subir e baixar á vez en 60 Hz. Peso abaixo e, por riba, menos barro.", 2, 6.0, 4.0,  2, 3.0, 2.0,  1, 0.0,  0.40, -1.0 },
    { "Baixo apertado",       "En 100 Hz, máis baixar ca subir: o baixo enteiro e sen bola.",                   3, 4.0, 5.0,  1, 4.0, 1.5,  1, 0.0,  0.35,  0.0 },
    { "Aire na voz",          "Un realce moi ancho en 12 kHz e un pouco menos de graves: a voz ábrese.",       3, 0.0, 2.0,  5, 8.0, 4.0,  2, 0.0,  0.30, -0.5 },
    { "Voz presente",         "Algo de 5 kHz para que a voz pase por riba de todo, sen chiar.",                 3, 0.0, 1.0,  2, 5.0, 3.0,  2, 1.0,  0.35,  0.0 },
    { "Pegar a mestura",      "Un chisco abaixo e un chisco de aire: a mestura redonda.",                      1, 2.0, 1.5,  6, 10.0, 2.0, 2, 0.0,  0.25, -0.5 },
    { "Caixa",                "Corpo en 100 Hz e o estalo en 8 kHz.",                                           3, 2.0, 0.0,  3, 4.0, 3.0,  1, 0.0,  0.40,  0.0 },
    { "Guitarra brillante",   "Menos graves e brillo en 10 kHz, para que non tape a voz.",                     3, 0.0, 3.0,  4, 6.0, 4.0,  2, 1.0,  0.45,  0.0 },
    { "Piano",                "Algo de peso e de aire, moi ancho.",                                             2, 2.0, 0.0,  5, 9.0, 2.0,  2, 0.0,  0.30,  0.0 },
    { "Escuro e morno",       "Os agudos abaixo desde 5 kHz e a válvula traballando: coma unha cinta vella.", 2, 3.0, 0.0,  4, 5.0, 0.0,  0, 5.0,  0.70, -1.0 },
    { "Válvula quente",       "Plano, pero coa válvula ao máximo: harmónicos e algo de compresión.",           2, 0.0, 0.0,  4, 5.0, 0.0,  1, 0.0,  1.00, -2.0 },
};

constexpr int NUM_PRESETS = (int) (sizeof (PRESETS) / sizeof (PRESETS[0]));

} // namespace tubo
