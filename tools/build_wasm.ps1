$proj = (Resolve-Path ".").Path
$build = Join-Path $proj ".pio\build\wasm"

emsdk activate latest --permanent
emcmake cmake -S "$proj" -B "$build"
cmake --build "$build"
emrun --port 8000 "$build"