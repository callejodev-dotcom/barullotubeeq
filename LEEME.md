**Barullo Tube EQ** é un ecualizador pasivo de válvulas, coma os dos anos 50 e 60: curvas moi anchas e suaves que se poden mover moito sen que soe mal, e unha válvula detrás. Non é para arranxar problemas: é para dar cor.

Unha ferramenta de [Escoita](https://www.escoita.link).

## Descargas

| Sistema | Instalador | Á man |
|---|---|---|
| **Mac** (Apple Silicon e Intel, macOS 11+): AU, VST3, CLAP e app | `BarulloTubeEQ-…-mac.pkg` | `BarulloTubeEQ-…-mac.zip` |
| **Windows** (64 bits): VST3, CLAP e app | `BarulloTubeEQ-…-windows-setup.exe` | `BarulloTubeEQ-…-windows-x64.zip` |
| **Linux** (x86_64): VST3, LV2, CLAP e app | — | `BarulloTubeEQ-…-linux-x86_64.tar.gz` → `./instalar.sh` |

Se o sistema avisa ao abrilo:
- **Mac:** clic dereito sobre o instalador → *Abrir* → *Abrir*.
- **Windows:** «Windows protexeu o teu PC» → *Máis información* → *Executar de todos os xeitos*.

Avisan porque os instaladores non levan sinatura de pago, non porque haxa nada raro.

## Para que serve

- **O truco dos graves**: subir e baixar á vez na mesma frecuencia (60 Hz, por exemplo). O grave sobe e, xusto por riba, o barro baixa: bombos e baixos grosos e apertados.
- **Aire**: unha campá moi ancha en 10–16 kHz que abre a voz ou a mestura sen que chíe.
- **Pegar a mestura**: un chisco de cada cousa e a válvula.
- **Quentar**: sintes e caixas de ritmos dixitais, aínda sen tocar as curvas.

## Que ten

- **Graves**: frecuencia (20, 30, 60 ou 100 Hz), *Subir* (ata +13,5 dB, cun bulto na frecuencia) e *Baixar* (ata −17,5 dB, desde máis arriba e máis ancho).
- **Agudos · subir**: frecuencia (3, 4, 5, 8, 10, 12 ou 16 kHz), *Ancho* (de estreita a moi ancha) e *Subir* (ata +16 dB).
- **Agudos · baixar**: frecuencia (5, 10 ou 20 kHz) e *Baixar* (ata −16 dB, un andel suave coa metade da baixada na frecuencia).
- **Válvula**: canto traballa (pouco, case limpa; moito, harmónicos pares e algo de compresión) e a *Saída*.
- **A pantalla**: a curva enteira e, en fino, a dos graves e a dos agudos. É a mesma que soa (a proba compárao).
- **11 presets de fábrica**: Plano, Bombo groso, Baixo apertado, Aire na voz, Voz presente, Pegar a mestura, Caixa, Guitarra brillante, Piano, Escuro e morno e Válvula quente. Son tamén os programas do DAW.
- **Presets propios, A/B e desfacer** (do núcleo común; `.btqpreset`, en Documentos/Barullo Tube EQ/Presets).
- A luz ao carón do nome é o bypass: o son pasa tal cal (coa latencia).
- Un **limitador de seguridade** á saída: nunca pasa de −1 dBFS.

## Como se fixo

`Source/Pasivo.h`, sen JUCE: os catro filtros son analóxicos (en s) e pásanse a dixital coa transformada bilineal, cada un coa súa frecuencia no seu sitio:

- graves, subir: un andel de 2.ª orde cun pouco de resonancia (o bulto);
- graves, baixar: un andel de 1.ª orde, catro veces máis arriba;
- agudos, subir: unha campá co seu Q (de 2,4 a 0,45);
- agudos, baixar: un andel de 1.ª orde, centrado na frecuencia.

Todo vai a dobre frecuencia de mostraxe, coa válvula (unha curva suave e un chisco asimétrica: harmónicos pares). Os mandos esvaran (30 ms) e os coeficientes recalcúlanse cada 32 mostras.

## Estrutura

| Ficheiro | Que é |
|---|---|
| `Source/Pasivo.h` | O ecualizador e a válvula |
| `Source/Presets.h` | Os presets de fábrica |
| `Source/PluginProcessor.*` | O plugin: parámetros, presets, estado e o sobremostrado |
| `Source/PluginEditor.*` | A interface: a válvula, a curva e os mandos |
| `tools/RenderTest.cpp` | A proba |

Usa o núcleo común de Escoita ([escoita-nucleo](https://github.com/callejodev-dotcom/escoita-nucleo), v0.4.0).

## Probas

- **Plano**: o son case tal cal (±0,15 dB).
- **Graves**: subir ao 10 en 60 Hz dá +12–15 dB abaixo e nada en 1 kHz; **o truco** (subir 6 e baixar 4) sobe en 50 Hz e baixa en 200 Hz.
- **Agudos**: a campá en 10 kHz (+14–17,5 dB) e o seu ancho; o corte desde 10 kHz.
- **A curva da pantalla é a que soa** (±0,3 dB).
- **A válvula**: o segundo harmónico medra co mando.
- **Teito**, **bloques** (512 e 37), **bypass**, **presets**, **mono**, **estado**, **A/B**, **presets propios**, **desfacer** e **programas do DAW**.
- **Validación**: pluginval ao nivel máximo (10), auval no Mac e clap-validator.

## Compilar

Nun Mac: `./build_mac.sh`. Á man:

```
cmake -B build -DCMAKE_BUILD_TYPE=Release -DBARULLO_TESTS=ON
cmake --build build --config Release
build/BQRenderTest_artefacts/Release/BQRenderTest
```

## Publicar unha versión

```
git tag v0.1.0 && git push origin v0.1.0
```

## Créditos e licenzas

- **Barullo Tube EQ é software libre**, baixo a licenza **GNU Affero General Public License v3.0** (ficheiro `LICENSE`). Usa JUCE 8 coa súa licenza AGPLv3.
- Tipografías Montserrat e Inter (licenza OFL).
- Non ten relación con ningún fabricante: é un ecualizador pasivo feito desde cero, coa idea dos de antes.
