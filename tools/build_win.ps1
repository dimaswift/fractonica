$proj = (Resolve-Path ".").Path
$build = Join-Path $proj ".pio\build\win"

cmake -S "$proj" -B "$build"
cmake --build "$build"

# run (matches your Debug/fractonica.exe choice)
& (Join-Path $build "Debug\fractonica.exe")