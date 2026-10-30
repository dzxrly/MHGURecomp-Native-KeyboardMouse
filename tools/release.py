"""Build a release from prepared dependencies. No exefs/romfs or code generation."""

import argparse
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys

from dist import prompt_files


def run(*args, env=None):
    subprocess.run([str(arg) for arg in args], check=True, env=env)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--inputs", required=True, type=Path)
    parser.add_argument(
        "--version", default=Path("UPSTREAM_VERSION").read_text().strip()
    )
    parser.add_argument("--make", default="make")
    parser.add_argument("--jobs", type=int, default=4)
    args = parser.parse_args()
    supported = Path("UPSTREAM_VERSION").read_text().strip()
    if args.version != supported or not re.fullmatch(
        r"[0-9]+\.[0-9]+\.[0-9]+", args.version
    ):
        parser.error(
            f"This checkout follows upstream {supported}; merge and review another upstream version first"
        )
    if args.jobs < 1:
        parser.error("--jobs must be positive")
    inputs = args.inputs.resolve()
    if (inputs / "UPSTREAM_VERSION").read_text().strip() != args.version:
        parser.error("Prepared dependencies belong to another upstream version")
    platform = "windows" if os.name == "nt" else "linux"
    library = "mhgugame.dll" if os.name == "nt" else "mhgugame.so"
    source = inputs / platform / library
    if not source.is_file():
        parser.error(f"Prepared game library is missing: {source}")
    prompt_mod = inputs / "Keyboard_Mouse_Input_Auto"
    prompt_files(prompt_mod)
    if any(Path("generated").glob("*/*.c")):
        parser.error(
            "Release builds use prepared libraries; use a clean checkout without generated game code"
        )
    Path("build").mkdir(exist_ok=True)
    shutil.copyfile(source, Path("build") / library)
    # An explicitly absent game path prevents make's optional data target from copying a ROM.
    common = [
        args.make,
        "GAME=release-no-rom",
        f"VERSION={args.version}",
        f'PY="{Path(sys.executable).as_posix()}"',
    ]
    if Path("release-no-rom").exists():
        parser.error("release-no-rom must not exist")
    run(
        *common,
        f"-j{args.jobs}",
        "all",
        "test-rom-version",
        "test-release-packaging",
        "test-settings",
        "test-keyboard-mouse",
        "test-settings-locale",
        "test-keyboard-mouse-assets",
        "test-settings-panel",
    )
    checker = Path("build") / ("mhgurecomp.exe" if os.name == "nt" else "mhgurecomp")
    run(
        checker.resolve(),
        "--check-gamelib",
        env={**os.environ, "MHGU_GAME_LIB": str((Path("build") / library).resolve())},
    )
    # dist's explicit allowlist includes the prepared mod, not the build's ROM/settings/other mods.
    run(*common, "dist", f"KEYBOARD_MOUSE_MOD={prompt_mod.as_posix()}", "BONUS=")


if __name__ == "__main__":
    main()
