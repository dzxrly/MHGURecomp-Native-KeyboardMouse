<h1 align="center">Building and releasing</h1>

[Developer documentation](README.md) · [Testing and coverage](TESTING.md)

This project targets Windows only. Linux is currently unsupported because no Linux build and test environment is
available for this project. Builds read the owner's dump when recompiling game code,
and never modify it. Releases are built locally and include the checked-in Auto prompt mod.

## 1. Tools (Windows)

1. Install [MSYS2](https://www.msys2.org/) to `C:\msys64`.
2. In the **MSYS2 UCRT64** shell, install the compiler and libraries:

   ```bash
   pacman -S --needed make git diffutils \
     mingw-w64-ucrt-x86_64-clang mingw-w64-ucrt-x86_64-lld mingw-w64-ucrt-x86_64-pkgconf \
     mingw-w64-ucrt-x86_64-sdl3 mingw-w64-ucrt-x86_64-shaderc mingw-w64-ucrt-x86_64-nasm \
     mingw-w64-ucrt-x86_64-vulkan-headers mingw-w64-ucrt-x86_64-vulkan-loader \
     mingw-w64-ucrt-x86_64-vulkan-memory-allocator
   ```

   FFmpeg (for the game's movies) isn't installed: the first build downloads its source and builds the small part the
   movies need into the executable (see "FFmpeg" below).

3. Install [Python](https://www.python.org/) 3.10 or newer for Windows. Then, in a Command Prompt in the repository
   folder, make the tools' virtual environment (the Makefile uses `tools\venv` when it exists):

   ```bat
   py -3 -m venv tools\venv
   tools\venv\Scripts\pip install -r tools\requirements.txt
   ```

## 2. Recompile and build

Clone this repository:

```sh
git clone https://github.com/dzxrly/MHGURecomp-Native-KeyboardMouse.git
cd MHGURecomp-Native-KeyboardMouse
```

From the UCRT64 shell, in the repository folder, with your game files in a folder holding `exefs\` and `romfs\` (as described in the main README):

```bash
make gen GAME=/c/path/to/game      # translate the game code to C in generated/ (~300 MB)
make -j4 GAME=/c/path/to/game      # compile everything into build/
```

The first full build takes a while (about 8 minutes on a 4-core Haswell with `-j4`; more cores: higher `-j`). Later
builds only recompile what changed.

`make` turns `build\` into a ready-to-run game folder, and puts nothing else there (the compiler's own files go
to `obj\`):
- `mhgurecomp.exe`, the runtime;
- `mhgugame.dll`, the recompiled game code;
- the MSYS2 DLLs they need;
- `data\exefs` and `data\romfs`, hard links to your game files, so they take no extra disk space.

Hard-linked files share their contents with the source dump. Treat `build/data` as read-only too; make resource changes only through mod overlays.

To play from another folder, copy the contents of `build\` there; to update that folder, copy them over it (`data\`
can be skipped when it is already there). The build never contains settings, caches, mods or logs, so your
`mhgurecomp.ini` and the rest stay as they are. Or start `run-game.bat`: it runs `build\mhgurecomp.exe` with the
settings, caches, `mods\` and `log.txt` in `run\` (`MHGU_HOME`), keeping `build\` clean. Starting
`build\mhgurecomp.exe` directly also works, but then it writes those into `build\`.

Useful targets and options:

- `make test-lift`: checks the recompiler against the Unicorn CPU emulator on thousands of the game's instructions.
- `make clean`: removes the compiled files (`obj\` and the program files in `build\`). It keeps `build\data`.
- `GENARCH=-march=x86-64-v2`: compiles the game code without AVX2, for older CPUs. Untested.
- `FFMPEG=system`: links the system's FFmpeg libraries instead of building them (see below).

## FFmpeg

The game's movies are MP4 files with H.264 video and AAC audio. The first build downloads FFmpeg's source
(`third_party/ffmpeg/`, a pinned release checked against its SHA-256), builds only the MP4 reader and those two
decoders into `obj/`, and links them into the executable. Nothing of FFmpeg needs to be installed or shipped, and the
build does not depend on a separately installed FFmpeg version. This needs `curl` and, for FFmpeg's
fast x86 code, `nasm` (without it FFmpeg uses plain C, which is fast enough for these movies). It takes a few minutes,
once. FFmpeg is LGPL 2.1; its license goes into `build/licenses/`.

`make FFMPEG=system` links the system's avformat / avcodec / avutil instead (FFmpeg 5.1 or newer), as a
distribution package would; that build then only runs with that FFmpeg version.

## Making a release

This project is Windows-only. There is currently no Linux build and test environment, so Linux is unsupported and
no Linux release package is provided. Build and verify
locally, then upload the completed Windows archive to this repository's GitHub Releases. Automated Actions
release building is disabled; no runner deployment, remote build inputs or cloud toolchain is needed.

The stable Auto mod is included in the repository and is packaged directly, without reading or modifying a ROM:

```bash
make dist VERSION=0.11.3 KEYBOARD_MOUSE_MOD=extras/keyboard-mouse/prebuilt/Keyboard_Mouse_Input_Auto BONUS=
```

This writes `dist/Windows-MHGURecomp.0.11.3.zip` with this project's runtime and updater, the matching game code
library, required DLLs, licenses and the bundled Auto mod. It excludes raw exefs/romfs, user settings, saves, caches
and other mods. The version follows the reviewed upstream version in `UPSTREAM_VERSION`. All base game files remain
read-only; compatibility changes belong in project code or mod overlays.

After local verification, upload the archive to this repository's normal GitHub release with the same numeric tag
(`0.11.3`, without `v`). The updater only checks this repository. It may update the bundled Auto mod's `romfs/`
resources while preserving disabled state, load order, other mods, settings and saves. A development version is not
automatically replaced by a release; `--update-force` and `--update-no-launch` are diagnostic options.

`make` fills `licenses\` with everything a copy has to carry: this project's `LICENSE.txt`, `THIRD_PARTY_NOTICES.txt`
(code compiled in), FFmpeg's license, and on Windows the licenses of the MSYS2 packages behind the bundled DLLs and
the libraries linked in. `licenses\README.txt` lists those with their versions and where their exact source is, and
holds a written offer for the source of the LGPL DLL among them (libiconv, which SDL3 uses): whoever publishes the
release makes that offer, so keep those sources at hand for three years (the MSYS2 address it gives is one copy).

The exe creates the `mods` and `cheats` folders and the ini on first start. Players then add their own `data\` as
described above.

## Selective upstream synchronization

This repository is maintained independently for native keyboard/mouse input. Keep `origin` pointed at this repository and `upstream` at [morerokk/MHGURecomp](https://github.com/morerokk/MHGURecomp). Review upstream changes and integrate selected commits manually; a full merge or a pull request back is not required.

If the upstream remote is absent:

```sh
git remote add upstream https://github.com/morerokk/MHGURecomp.git
```

Review before selecting commits:

```sh
git fetch upstream
git log --oneline HEAD..upstream/master
```

After resolving any input/resource conflicts, run the checks appropriate to the selected changes and rebuild. Validate game behavior with an isolated save. The updater URL and source-offer links belong to this repository.
