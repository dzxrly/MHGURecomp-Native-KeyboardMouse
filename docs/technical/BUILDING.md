<h1 align="center">Building and releasing</h1>

[English](../../README.md) · [简体中文](../README.zh-CN.md) · [繁體中文](../README.zh-TW.md)

Full source builds can recompile game code from your own dump. Release builds use a verified prebuilt code library
and the checked-in Auto prompt resources; they do not need the ROM. Raw game dumps and keys remain excluded.

### 1. Tools (Windows)

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

### 2. Recompile and build

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

### FFmpeg

The game's movies are MP4 files with H.264 video and AAC audio. The first build downloads FFmpeg's source
(`third_party/ffmpeg/`, a pinned release checked against its SHA-256), builds only the MP4 reader and those two
decoders into `obj/`, and links them into the executable. Nothing of FFmpeg needs to be installed or shipped, and the
Linux build runs on any distribution whatever FFmpeg version it has (or none). This needs `curl` and, for FFmpeg's
fast x86 code, `nasm` (without it FFmpeg uses plain C, which is fast enough for these movies). It takes a few minutes,
once. FFmpeg is LGPL 2.1; its license goes into `build/licenses/`.

`make FFMPEG=system` links the system's avformat / avcodec / avutil instead (FFmpeg 5.1 or newer), as a
distribution package would; that build then only runs with that FFmpeg version.

### Making a release

The independent `Independent release` workflow builds this repository's runtime and updater on GitHub-hosted
Windows and Ubuntu runners. No self-hosted runner, local directory, ROM upload or repository secret is required.
Windows installs the current MSYS2 UCRT64 toolchain; Linux builds in the current Arch Linux container, matching
its release archive's target. All action dependencies use their current Node.js 24 releases.

`UPSTREAM_VERSION` pins the reviewed upstream version (currently `0.11.3`). The workflow verifies that tag exists
and uses the same version for binaries, archives and the optional release. Merge and review an upstream version
before changing this file. Manually select Windows/Linux/both and enable `publish` to publish; otherwise the
archives remain Actions artifacts. Existing releases and tags pointing at another commit are not overwritten.

The stable Auto mod is checked into `extras/keyboard-mouse/prebuilt/Keyboard_Mouse_Input_Auto/romfs/` and is packed
directly from that directory. Rebuild it locally with `tools/build_keyboard_mouse_assets.py` only when the icons,
text or pack format change. Only these explicitly requested prompt resources are an exception to the raw-game-data
exclusions; no exefs, base romfs, save or cache belongs there. `.gitattributes` preserves their bytes on checkout.

`tools/fetch_gamelib.py` obtains the selected platform's game code library from the published
[morerokk/MHGURecomp release](https://github.com/morerokk/MHGURecomp/releases), verifies the archive's size and
GitHub-provided SHA-256, and extracts only `mhgugame.dll` or `.so`. Original-project executables are not packaged.
The rebuilt runtime then checks the library's ABI and ID through `--check-gamelib`, without opening a ROM, window
or save. Interface IDs canonicalize source line endings to match upstream's existing CRLF-built libraries on
both hosts. A mismatch fails the release; the expected ID is never replaced with a value read from the library.

Release jobs never run `make gen`, read exefs/romfs or regenerate the mod. A developer with the normal build tools
can run the same entry point in the repository root:

```bash
python tools/release.py
```

`make dist VERSION=<the release's tag>` builds everything with that version and packs it for GitHub:

```bash
git fetch --tags                          # (so builds between releases know the last one)
make dist VERSION=0.11.3 KEYBOARD_MOUSE_MOD=/path/to/Keyboard_Mouse_Input_Auto BONUS=
```

On Windows that writes `dist\Windows-MHGURecomp.0.11.3.zip`; on Linux (or in WSL, with `B=build-linux
O=obj-linux`) `dist/ArchLinux-MHGURecomp.0.11.3.tar.gz`, which keeps the programs executable. Each holds one folder
with `mhgurecomp(.exe)`, `mhgugame.dll` / `.so`, `mhgurecomp-updater(.exe)`, the bundled DLLs, `licenses\`,
`bonus\system` (from `BONUS`, else left empty), the explicitly selected Auto prompt mod, and empty `data\` and
`cheats\` folders; never the build folder's ROM, settings, caches or other mods. The updater may replace only the
bundled Auto mod's `romfs/` resources; its disabled state/load order and all other mods remain protected.
`make dist` prints each archive's SHA-256.

Upload both archives to a GitHub release whose tag is exactly that version (`0.10.4`, no `v`), as a normal release,
not a pre-release. The updater only looks at GitHub's "latest" release, compares its tag with its own version as
numbers (0.10.10 is newer than 0.10.9), and picks the asset starting with `Windows-` (`.zip`) or `ArchLinux-`
(`.tar.gz`, else `.zip`). GitHub records each upload's SHA-256, which the updater checks. To hold a broken release
back, make it a draft or pre-release: the updater then sees the previous one as the latest.

The version comes from `git describe --tags`. A build that isn't exactly a tag (`0.10.3-4-gabc1234`) never updates
itself, so a development build is never replaced by a release (`--update-force` overrides that). Options for testing
the updater: `--update-no-launch` (don't start the game afterwards) and `MHGU_UPDATE_API=<url>` (another "latest
release" address).

`make` fills `licenses\` with everything a copy has to carry: this project's `LICENSE.txt`, `THIRD_PARTY_NOTICES.txt`
(code compiled in), FFmpeg's license, and on Windows the licenses of the MSYS2 packages behind the bundled DLLs and
the libraries linked in. `licenses\README.txt` lists those with their versions and where their exact source is, and
holds a written offer for the source of the LGPL DLL among them (libiconv, which SDL3 uses): whoever publishes the
release makes that offer, so keep those sources at hand for three years (the MSYS2 address it gives is one copy).

The exe creates the `mods` and `cheats` folders and the ini on first start. Players then add their own `data\` as
described above.

### Linux

The Windows build runs under Proton / Wine. A native Linux build is supported as well: the Makefile picks
`runtime/platform/linux.c` when `uname -s` isn't Windows and everything else is shared. The original project tested Gentoo x86_64 with clang under Wayland (NVIDIA); this fork's native-input play tests are on Windows. Besides `make`, `git`, `clang`, `lld` and Python 3.10 or newer it
needs:

- SDL3, the Vulkan loader and headers and shaderc — the same libraries the MSYS2 list above installs, under your
  distro's names — plus `curl` and optionally `nasm` for building FFmpeg (see "FFmpeg" above; no FFmpeg package
  needed);
- `vk_mem_alloc.h` (3.4.0) in `third_party/vk_mem_alloc/`, which the Makefile has on the include path (a
  distro package that installs the header where the compiler looks by default works just as well);
- `zenity` or `kdialog` for the dialog on a fatal error. Without either, the message goes to the terminal and
  to `log.txt` as usual;
- for the updater: zlib's headers to build it, and `libcurl.so.4` (curl) when it runs, which SteamOS and the
  usual desktop distributions have. It loads libcurl itself, so without it the updater just starts the game.

```bash
python3 -m venv tools/venv && tools/venv/bin/pip install -r tools/requirements.txt
make gen GAME=../mhgu/game       # as above: the folder holding exefs/ and romfs/
make -j8 GAME=../mhgu/game
cd build && ./mhgurecomp
```

`build/` becomes the game folder exactly as on Windows: `mhgurecomp`, `mhgugame.so` with the recompiled game
code and `data/` as hard links to your game files, nothing else; the ini, caches and `mods/` are written beside
them when it runs (`MHGU_HOME=<folder>` puts those there instead). Saves go to
`$XDG_DATA_HOME/MHGURecomp/save`, default `~/.local/share/MHGURecomp/save` — the folder the Windows build
keeps them in under `%APPDATA%`, same layout, so moving a save across is copying `save_0`.

From Windows, the Linux build can be made in WSL with Arch Linux (`wsl --install -d archlinux`), straight from the
Windows checkout, into its own folders so the Windows build is left alone:

```bash
pacman -S --needed make git clang lld python sdl3 vulkan-headers vulkan-icd-loader shaderc nasm vulkan-swrast
mkdir -p /usr/local/include/vma && cp /mnt/c/msys64/ucrt64/include/vma/vk_mem_alloc.h /usr/local/include/vma/
make -j16 B=build-linux O=obj-linux     # in the repository, under /mnt/c/...
```

Arch has no package for `vk_mem_alloc.h`; MSYS2's (3.4.0) is the one the Windows build uses. `build-linux/data` can
hold links to the game files (`ln -s /mnt/c/.../exefs build-linux/data/exefs`, same for `romfs`, then
`touch build-linux/data/.complete`) instead of a second copy. It also runs there, in a WSLg window, slowly: WSL's
own Vulkan driver (dzn, Vulkan on Direct3D 12) can't create the renderer's pipeline layouts, so use Mesa's software
one with `VK_DRIVER_FILES=/usr/share/vulkan/icd.d/lvp_icd.json`. WSLg can't capture the mouse (clicks still work as
buttons), and screenshots come out black there.

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
