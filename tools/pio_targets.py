Import("env")
import os
import platform
from SCons.Script import AlwaysBuild

PROJECT_DIR = env["PROJECT_DIR"]
BUILD_DIR   = env.subst("$BUILD_DIR")   # usually .pio/build/<envname>
ENV_NAME    = env.subst("$PIOENV")

def sh(cmd):
    print(cmd)
    if os.system(cmd) != 0:
        raise Exception("Command failed")

def native_run(source, target, env):
    build_dir = os.path.join(PROJECT_DIR, ".pio/build", "native")
    sh(f'cmake -S "{PROJECT_DIR}" -B "{build_dir}"')
    sh(f'cmake --build "{build_dir}"')
    sh(f'"{build_dir}/fractonica"')

def win_run(source, target, env):
    build_dir = os.path.join(PROJECT_DIR, ".pio/build", "win")
    sh(f'cmake -S "{PROJECT_DIR}" -B "{build_dir}"')
    sh(f'cmake --build "{build_dir}"')

    exe = "Debug/fractonica.exe" if platform.system().lower().startswith("win") else "fractonica"
    sh(f'"{os.path.join(build_dir, exe)}"')

def wasm_run(source, target, env):
    build_dir = os.path.join(PROJECT_DIR, ".pio/build", "wasm")

    sh(f'emcmake cmake -S "{PROJECT_DIR}" -B "{build_dir}"')
    sh(f'cmake --build "{build_dir}"')
    sh(f'emrun --port 8000 "{build_dir}"')

t_native = env.AddCustomTarget(
    name="native_run",
    dependencies=None,
    actions=[native_run],
    title="Native: build+run (CMake)",
    description="Configure+build with CMake, then run the native binary",
)
AlwaysBuild(t_native)

t_win = env.AddCustomTarget(
    name="win_run",
    dependencies=None,
    actions=[win_run],
    title="Win: build+run (CMake)",
    description="Configure+build with CMake, then run the Windows binary",
)
AlwaysBuild(t_win)

t_wasm = env.AddCustomTarget(
    name="wasm_run",
    dependencies=None,
    actions=[wasm_run],
    title="WASM: build+serve (emrun)",
    description="Build WASM with emcmake/cmake, then run emrun",
)
AlwaysBuild(t_wasm)