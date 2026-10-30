"""Build a release from prepared dependencies. No exefs/romfs or code generation."""

import argparse
import os
from pathlib import Path
import re
import subprocess
import sys

from dist import prompt_files
from fetch_gamelib import fetch


def run(*args, env=None):
    subprocess.run([str(arg) for arg in args], check=True, env=env)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
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
    platform = "windows" if os.name == "nt" else "linux"
    library = "mhgugame.dll" if os.name == "nt" else "mhgugame.so"
    prompt_mod = Path(
        "extras/keyboard-mouse/prebuilt/Keyboard_Mouse_Input_Auto"
    ).resolve()
    prompt_files(prompt_mod)
    if any(Path("generated").glob("*/*.c")):
        parser.error(
            "Release builds use prepared libraries; use a clean checkout without generated game code"
        )
    try:
        fetch(args.version, platform, Path("build"))
    except (OSError, ValueError, KeyError) as error:
        parser.exit(1, f"Game library download failed: {error}\n")
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
