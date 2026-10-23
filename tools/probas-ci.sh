#!/bin/bash
# As probas de Barullo Tube EQ na CI común (escoita-nucleo): chámase desde a raíz con
# mac, windows, linux ou sanitizadores, coa compilación en build/
set -eo pipefail
SO="$1"
EXE=""; [ "$SO" = windows ] && EXE=".exe"
CONF=Release; [ "$SO" = sanitizadores ] && CONF=RelWithDebInfo
RT="build/BQRenderTest_artefacts/$CONF/BQRenderTest$EXE"

echo "::group::Son (as curvas, as válvulas, os presets e o estado)"
"$RT" proba.wav
echo "::endgroup::"
