<h1 align="center" id="development">Developer documentation</h1>

[English user guide](../README.md) · [简体中文用户指南](README.zh-CN.md) · [繁體中文使用指南](README.zh-TW.md)

This is the single maintainer document for building, releasing and developing this independently maintained native keyboard/mouse project. The supported client platform is Windows; no Linux client build and test environment is available. The optional hub server can run separately on Linux.

> Dated graphics and reverse-engineering observations below come from the original project. They retain their original test conditions and limits; they do not establish new validation of this fork.

> Terminology: ExeFS is the dump's executable filesystem (`exefs/`); RomFS is its resource filesystem (`romfs/`). The game-code library is the locally recompiled game, loaded by the host runtime. FSR means AMD FidelityFX Super Resolution, the optional temporal upscaler in the Vulkan renderer. FPS measures rendered frames per second.

## Contents

- [Building and releasing](#building)
- [Runtime and native input](#runtime)
- [Testing and recorded coverage](#testing)
- [Host settings overlay](#settings-overlay)
- [Keyboard and mouse prompt resources](#prompt-resources)
- [Hub server](#hub-server)
- [FSR 3.1 implementation notes](#fsr31)
- [FSR 4 implementation notes](#fsr4)
- [Recording the pipeline list](#pipeline-cache)
- [Armor skill point research](#skill-points)
- [Valor Sheathe research](#valor-sheathe)
- [Small monster synchronization research](#small-monster-sync)

## Source layout

| Directory  | Responsibility                                                            |
| ---------- | ------------------------------------------------------------------------- |
| `runtime/` | Host runtime, native input, graphics, audio, networking and configuration |
| `tools/`   | Recompiler, packaging and prompt-resource tools                           |
| `updater/` | Updater for this repository's releases                                    |
| `server/`  | Optional multiplayer hub server                                           |
| `extras/`  | Prompt resources and optional cheats                                      |

Keep original game files read-only. Implement compatibility changes in project code or mod overlays, and use isolated copies of saves for experiments. Player instructions belong in all three user guides; implementation and validation details belong here.

<a id="building"></a>

## Building and releasing

This project targets Windows only. Linux is currently unsupported because no Linux build and test environment is
available for this project. Builds read the owner's dump when recompiling game code,
and never modify it. Releases are built locally and include the checked-in Auto prompt mod.

<a id="building-1-tools-windows"></a>

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

<a id="building-2-recompile-and-build"></a>

### 2. Recompile and build

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

<a id="building-ffmpeg"></a>

### FFmpeg

The game's movies are MP4 files with H.264 video and AAC audio. The first build downloads FFmpeg's source
(`third_party/ffmpeg/`, a pinned release checked against its SHA-256), builds only the MP4 reader and those two
decoders into `obj/`, and links them into the executable. Nothing of FFmpeg needs to be installed or shipped, and the
build does not depend on a separately installed FFmpeg version. This needs `curl` and, for FFmpeg's
fast x86 code, `nasm` (without it FFmpeg uses plain C, which is fast enough for these movies). It takes a few minutes,
once. FFmpeg is LGPL 2.1; its license goes into `build/licenses/`.

`make FFMPEG=system` links the system's avformat / avcodec / avutil instead (FFmpeg 5.1 or newer), as a
distribution package would; that build then only runs with that FFmpeg version.

<a id="building-making-a-release"></a>

### Making a release

This project is Windows-only. There is currently no Linux build and test environment, so Linux is unsupported and
no Linux release package is provided. Build and verify
locally, then upload the completed Windows archive to this repository's GitHub Releases. Automated Actions
release building is disabled; no runner deployment, remote build inputs or cloud toolchain is needed.

The stable Auto mod is included in the repository and is packaged directly, without reading or modifying a ROM:

```bash
make dist VERSION=0.11.7.1 KEYBOARD_MOUSE_MOD=extras/keyboard-mouse/prebuilt/Keyboard_Mouse_Input_Auto BONUS=
```

This writes `dist/Windows-MHGURecomp.0.11.7.1.zip` with this project's runtime and updater, the matching game code
library, required DLLs, licenses and the bundled Auto mod. It excludes raw exefs/romfs, user settings, saves, caches
and other mods. The version follows the reviewed upstream version in `UPSTREAM_VERSION`; a fourth numeric component identifies this project's own release revision. All base game files remain
read-only; compatibility changes belong in project code or mod overlays.

After local verification, upload the archive to this repository's normal GitHub release with the same numeric tag
(`0.11.7.1`, without `v`). The updater only checks this repository. It may update the bundled Auto mod's `romfs/`
resources while preserving disabled state, load order, other mods, settings and saves. A development version is not
automatically replaced by a release; `--update-force` and `--update-no-launch` are diagnostic options.

`make` fills `licenses\` with everything a copy has to carry: this project's `LICENSE.txt`, `THIRD_PARTY_NOTICES.txt`
(code compiled in), FFmpeg's license, and on Windows the licenses of the MSYS2 packages behind the bundled DLLs and
the libraries linked in. `licenses\README.txt` lists those with their versions and where their exact source is, and
holds a written offer for the source of the LGPL DLL among them (libiconv, which SDL3 uses): whoever publishes the
release makes that offer, so keep those sources at hand for three years (the MSYS2 address it gives is one copy).

The exe creates the `mods` and `cheats` folders and the ini on first start. Players then add their own `data\` as
described above.

<a id="building-selective-upstream-synchronization"></a>

### Selective upstream synchronization

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

<a id="runtime"></a>

## Runtime and native input

This independently maintained project is based on [morerokk/MHGURecomp](https://github.com/morerokk/MHGURecomp). Its current focus is native keyboard/mouse input for the Switch MHGU 1.4.0 executable. Selected upstream changes are integrated manually.

<a id="runtime-runtime"></a>

### Runtime

- The game is AArch32 ARM code, statically translated by `tools/`. Generated code and game files stay local.
- `mhgurecomp` supplies the host runtime; `mhgugame.dll` / `mhgugame.so` contains the locally recompiled game.
- Switch services, threads, input, audio and networking are implemented under `runtime/`. The NVN API is translated to Vulkan.
- The supported title ID is `0100770008DD8000`; the `main` build ID begins with `FB08F1D20FD1204F`. Guest runtime addresses include the main module’s `0x200000` load offset.
- Executable/library names, configuration keys and save paths retain the original names so existing installations and saves continue to work.

<a id="runtime-rom-compatibility"></a>

### ROM compatibility

Before loading any game module, every launch path validates the full main/sdk NSO Build IDs and nine selected
base-romfs SHA-256 fingerprints against the supported Switch MHGU 1.4.0 reference, plus the merged romfs file-count
check. These are compatibility checks, not a whole-ROM integrity proof. Unsupported, missing or modified references
produce a warning and stop startup before fixed-address hooks. `--check-rom <folder>` runs only these checks.
`--check-gamelib` separately validates the prepared code library's ABI/ID without a ROM. Release builds consume
the checked-in prompt pack and a matching local game library. Releases are built locally for Windows and uploaded
manually. Linux is currently unsupported because the project has no Linux build and test environment.

`exefs` must come from the 1.4.0 update; `romfs` must contain the complete base game with that update applied (approximately 35,500 files and 12.3 GB). The compatibility guard is implemented in `runtime/rom_version.c` with reference data in `runtime/rom_version_data.h`.

<a id="runtime-input-flow"></a>

### Input flow

`InputMode = Keyboard_Mouse_Input` enables the native input hooks at startup. The source default is `Original`, which retains the original mapping and controller prompts. `KeyboardLayout` is no longer used.

| Component                              | Responsibility                                                                                   |
| -------------------------------------- | ------------------------------------------------------------------------------------------------ |
| `runtime/host_input.c`                 | SDL events, mouse capture, button-release tracking, preset changes and wheel sequencing          |
| `runtime/keyboard_mouse_input.h`       | Melee/Ranged keyboard and mouse mappings                                                         |
| `runtime/hle/native_mouse.c`           | Equipped-weapon detection, direct angles, original action gates and compatibility-camera routing |
| `runtime/hle/keyboard_mouse_prompts.c` | Text selection and cached texture changes for the active preset                                  |
| `tools/build_keyboard_mouse_assets.py` | Local generation of the dual-profile Auto resource pack                                          |

The local character’s actual equipment selects Ranged for Heavy Bowgun, Light Bowgun and Bow, and Melee for other Hunter weapons and Prowler. Equipment previews do not select the active preset. Changes release old held inputs and queued wheel steps.

Relative mouse counts are staged for the local game input update and expire after 100 ms. Menu, focus, capture and action gates discard blocked movement. Ordinary camera input is applied during the game’s manual-camera stage, before recenter, automatic pitch and terrain processing.

Bowgun/Bow aiming changes the player’s authoritative aim before camera synchronization. Original motion checks, aiming-entry delays, per-weapon pitch bounds and projectile setup remain in effect. Bow mouse buttons follow its configured control type and R/ZR swap. Keyboard buttons continue to invoke the original game actions.

The ranged aim setter also runs during ordinary movement with a drawn weapon. It consumes mouse counts only while the original scope or Quick Aim checks are active; otherwise, the counts remain available to the ordinary camera. This keeps movement and mouse look independent.

Village cameras, fixed Ballista and Prowler special aiming retain the stick path. These use the existing configurable sampler and deadzone compensation; direct-angle paths do not.

`Original` maps WASD to movement, mouse to the camera, left/right mouse to X/A, Space/R to B/Y, Q/E to L/R, Tab/T to ZL/ZR, and Enter/Backspace to Plus/Minus. Esc releases the mouse in that mode. In keyboard/mouse mode, mouse-button releases target the action originally pressed, including across Bow menu/control-state changes.

<a id="runtime-mouse-response-and-compatibility-parameters"></a>

### Mouse response and compatibility parameters

`[Mouse] MouseSensitivity` (0.1–10) multiplies the base sensitivity. Direct input uses relative mouse counts rather than screen pixels; physical response depends on mouse DPI. No additional acceleration curve is applied. In-game Camera Speed and Reticle Speed apply independently and immediately:

| In-game option | Direct path                   | Slow | Default | Fast |
| -------------- | ----------------------------- | ---- | ------- | ---- |
| Camera Speed   | Ordinary quest camera         | 0.5× | 1×      | 1.5× |
| Reticle Speed  | Bowgun/Bow aim and Binoculars | 0.5× | 1×      | 1.5× |

At sensitivity 1, 1,000 horizontal counts correspond to approximately 20° / 40° / 60°. Camera, Quick Aim and Scope inversion remain separate. The five Quick Aim Camera types, initial camera position, target-camera behavior and terrain adjustment retain the original game logic. Blocked motion is discarded rather than replayed; moving the camera does not force melee turning outside the original action window.

Only the stick-based compatibility paths use the following settings, which require a restart:

```ini
[MouseConversion]
MinWindowMs = 8.0
FullStickSpeed = 1500.0
DeadzoneCompensation = 0.40
AimDeadzoneCompensation = 0.50
LowSpeedCutoff = 0.0
FrameTimeSmoothing = 0.25
```

`FullStickSpeed` is the speed in counts/second that produces full stick at sensitivity 1. The compensation values offset ordinary/aiming deadzones. `MinWindowMs` sets the minimum sampling window, `LowSpeedCutoff` filters very slow movement, and `FrameTimeSmoothing` adjusts the frame-time estimate. These paths retain the game's stick speed limits.

<a id="runtime-automatic-prompts"></a>

### Automatic prompts

The Auto pack contains both text profiles without changing the original message count or label indices. A message getter selects the appended Melee strings when appropriate. Known controller atlases carry an exact Ranged baseline and Melee block differences. The Vulkan upload selects the active variant and refreshes cached textures on preset changes.

The explicitly requested prebuilt seven-language Auto pack is maintained under `extras/keyboard-mouse/prebuilt/` and distributed with Windows releases. Raw exefs/romfs stay outside Git and remain read-only; compatibility changes belong in project code or mod overlays. Existing single-profile packs are skipped; an explicitly disabled Auto mod remains disabled.

Existing checks, recorded play-test coverage and outstanding verification are maintained in [TESTING.md](#testing).

<a id="runtime-mods-and-networking"></a>

### Mods and networking

> A subsdk is an additional executable module supplied by a mod as `exefs/subsdk0` through `exefs/subsdk9`.

The mod loader supports RomFS replacements, merged ARC entries, Forge plugins, exefs code patches and subsdk modules. Patched game functions and added modules run in the interpreter. Forge's own `mhguForge` subsdk loader is skipped because this runtime loads Forge plugins itself. Later conflicting entries win. Insert opens Forge. Mods apply in memory or as overlays, leaving the original game files unchanged. Optional cheat implementations are documented in the [skill-point](#skill-points) and [Valor Sheathe](#valor-sheathe) notes.

For direct multiplayer without a relay, the host's TCP/UDP port (default 39500) must be reachable and clients set `[Multiplayer] Join` to the host address. A hub server instead uses `[Multiplayer] Server`, with a shared `Key` to restrict discovery. Server build, deployment and protocol details are in [Hub server](#hub-server).

The 2026-10-31 synchronization selects mod support and its Forge fix, 30-FPS-equivalent stun recovery, smooth gradients and the 16:10 Meownster Hunters return-screen fix from upstream 0.11.7. Upstream's original-input keybind editor and Linux build changes are deferred. The recompiler sources and prepared game-library interface remain at the existing base; added mod code uses the interpreter.

<a id="testing"></a>

## Testing and recorded coverage

<a id="testing-existing-checks"></a>

### Existing checks

Run the checks relevant to the changed code:

```sh
make test-settings test-keyboard-mouse test-settings-locale test-keyboard-mouse-assets
```

`make test-settings-panel` exercises the settings overlay through ImGui without game files or a graphics device. Its scope is described in [SETTINGS_UI.md](#settings-overlay-validation). `make test-lift` compares recompiled instructions with the Unicorn CPU emulator.

Check game data and the prepared game-code library separately:

```sh
build/mhgurecomp.exe --check-rom /c/path/to/game
build/mhgurecomp.exe --check-gamelib
```

The first command checks supported executable IDs and selected base resources without starting the game. The second validates the library's ABI/ID without a ROM. Neither proves gameplay correctness or whole-ROM integrity.

<a id="testing-runtime-testing"></a>

### Runtime testing

Use isolated configuration and save directories. Copy the player's save into the test area before making experimental equipment changes; never write those changes back to the original. Treat both the source dump and any hard-linked `build/data` as read-only. Put test resource changes in mod overlays.

`MHGU_NATIVE_MOUSE_LOG=1` enables accepted-motion and projectile-direction traces. The existing control file's `stopinput` command clears scripted inputs and held test state. Normal startup does not hold aim automatically. Verify that input actually reached the intended action before describing a capture as an attack, guard or charge test.

<a id="testing-recorded-native-input-coverage"></a>

### Recorded native-input coverage

The following records summarize Windows play checks through 0.11.7.1, including the 2026-10-31 movement-camera regression checks. They are a partial record of exercised behavior. A shared Melee/Ranged preset does not establish that every move, style, ammo type or multiplayer context has been tested.

| Area                                | Recorded checks                                                                                               |
| ----------------------------------- | ------------------------------------------------------------------------------------------------------------- |
| Bowguns and Bow                     | Direct aim and projectile direction, both Bow control types, camera options and automatic equipment switching |
| All 14 Hunter weapon classes        | Sheathed standing, drawn standing and all four movement directions; accepted mouse counts changed the actual camera angles in all 84 checks |
| Ranged movement-camera regression   | All three ranged classes also passed the six movement-camera states at Standard Camera Speed; bowgun Quick Aim/scope and shots, both Bow control types and arrows |
| Prowler                             | Standing and four-way movement camera checks at Standard Camera Speed; charged-boomerang precision aim and exit, plus earlier guard checks |
| Insect Glaive / Ballista            | Glaive aim and Kinsect launch; fixed Ballista compatibility aiming                                            |
| Long Sword / Lance / Charge Blade   | Long Sword movement/draw direction, Lance guard/thrust, Charge Blade guard                                    |
| Great Sword                         | Draw, attack and guard via actual mouse buttons; charge levels and full move chains remain unverified         |
| Sword and Shield                    | Attack, X+A and guard via actual mouse buttons                                                                |
| Ordinary camera                     | The same 1,000-count input produced matching angle changes at selected 30/120/240 FPS settings                |
| Heavy Bowgun at 30/240 FPS settings | Direct aim, projectile direction and discarded firing-lock input; no replay of blocked motion after recovery  |

Selected FPS values do not establish stable delivered frame rates or coverage of physical mouse polling rates. Mouse sampler tests cover the compatibility conversion; they do not replace native-angle play tests. Guard-pose evidence does not establish damage blocking against an incoming attack. FSR and multiplayer retain the validation limits documented in their own notes.

<a id="testing-outstanding-work"></a>

### Outstanding work

- Bow-specific prompts and broader R/ZR-swap prompt coverage; some handbook pages still contain controller illustrations.
- Actual Prowler Support Move activation with an equipped move, and variable-zoom scope behavior.
- Wider weapon/style/ammo, full move-chain, frame-rate and multiplayer-context coverage, including Great Sword charge levels.

Unverified actions are not automatically unsupported. Keep these validation gaps separate from confirmed defects and from the practical limitations explained in the user guides.

<a id="settings-overlay"></a>

## Host settings overlay

This document describes the overlay's implementation behavior and validation. Player configuration steps are in the [user guide](../README.md#settings-and-files).

Minus + Plus (Back/Select + Start) or F1 opens a single panel inside the game.
Insert still opens Forge. Options, Save and Reset share one scrolling flow, in that order.
Back appears only as a guide beneath the options. Discard is available only
in the unsaved-changes confirmation when leaving the menu. App also contains Restart and Exit. The panel is attached on
every opening; detach allows dragging, and Move menu works with the controller.

Controller shoulder glyphs bracket the tab labels when using a gamepad. Keyboard/mouse input shows arrow-key, Enter/left-click and Esc/F1 hints instead; the tab labels remain clickable and Tab/Shift+Tab moves between options. Hints follow the last meaningful host input independently of the gameplay preset. Pointer noise, stick drift and an already held stick direction do not reclaim the hints. The controller icon preference still selects Xbox, Switch or PlayStation glyphs.

The tab header stays pinned
inside the panel, remains clickable while scrolled and never overlaps option
selection. Only the options/actions between the header and hints scroll. The bottom guide labels
the D-pad as Navigation, the confirm button as Confirm, and the back button
as Back, using the selected controller layout and English/Spanish language.
The guide wraps when needed and stays pinned inside the bottom of the panel.
Long option lists scroll above it, including Save and Reset; focused rows scroll
fully clear of the guide.

<a id="settings-overlay-navigation-and-editing"></a>

### Navigation and editing

L/R cycles seven tabs once per press and selects their first field. The tabs are
Display, Graphics & Audio, Controls, Network, Miscellaneous, Mods and App. Miscellaneous
contains the monster-HP multipliers (under a "Monster HP Scaling" title: a header row,
skipped by navigation) and quest-specific overrides. D-pad or
left stick moves between rows; left/right adjusts the selected value. Confirm
opens the host-address text editor, or a dropdown on rows with choices (an arrow after
their value): their choices in a small window of its own under the row (above it when
there is more room there), the current one marked by a bar. Up/down move in it,
Confirm picks, Back or a click elsewhere closes it. Numeric rows adjust
directly with left/right; clicking or confirming them never opens a keypad, except
the frame rate, whose Confirm opens a dropdown of 30, 60, 90, 120, 150 and 240 (left/right
still go one frame at a time). Each compound setting has
one focus stop per editable value. Mouse and physical keyboard remain supported.

Above the guides, a pinned help box explains the row last focused or pointed at with
the mouse: the comment lines above its key in the default ini (`config.c` DEFAULT_INI,
`config_help`; wrapped lines joined, "Default: x" last), or a text of the row's own
(Per-quest overrides, Check hub server). It keeps room for the longest help of the tab,
so moving between rows doesn't move the guides. The ini's text is English; each line goes
through `settings_locale.h`, so a translation added there is used.

Back, Escape and F1/chord share a single-level route: cancel an editor, cancel a
confirmation, or leave the main panel. The panel owns its navigation keys before
ImGui NewFrame so native cancel/windowing cannot consume the same input again.
Returning from a subview restores its source field and scroll position.
Shoulders change sections only in the main view. In the on-screen keyboard they
move the text cursor. Character keys, Delete, Clear, Space, Case, Accept and
Cancel can all be selected with the controller. Physical text input, UTF-8
backspace, Ctrl+A, Ctrl+V and Enter are supported. Invalid text stays in the
editor with an inline explanation. Quest ID adjustment skips existing IDs;
new quest IDs are unique and UI row IDs survive deletion and ID changes.

Save and Reset have no shortcut glyphs: activate their selected row.
There are no face-button save/discard shortcuts. The Back guide reflects the actual
SDL face mapping and the Xbox/Switch/PlayStation visual preference. Confirmation
views deliberately have no face-button shortcut guides: Confirm activates the
selected row, and Back cancels. Cancel is selected by default. Layout selection
only changes the host menu's symbols; contextual game confirmation is separate.

<a id="settings-overlay-changes-and-persistence"></a>

### Changes and persistence

Closing the menu with changes offers Cancel, Save and close, or Discard and close.
Exit offers Save and exit or Exit without saving when options or uncommitted text
are pending. ConfirmExit=Off skips the ordinary close confirmation only when
there are no pending changes. Restart saves settings after explicit confirmation.
No settings operation saves game progress. Reset this section changes only the
draft and requires a subsequent Save. A write failure preserves draft and file.

Save applies volume, FPS, FOV, FXAA, MSAA, VSync, mouse sensitivity, face mapping,
background input and confirmation policy without restarting. Render changes
are consumed at render boundaries. Language, symbols and interface scale preview
immediately and persist on Save. Cold fields carry an asterisk; the saved status
indicates a pending restart only when cold settings differ from startup values.
Raw Auto/Desktop values and unchanged environment overrides are preserved.
Comments, unknown INI keys and existing aliases survive atomic saves, and a line whose value did not change stays
as the player wrote it ("21:9", "Borderless Window"). Every setting is one row of the table in `runtime/config.c`
(`g_settings`: section, key and other spellings, type, range or choices, restart needed, error message): reading,
writing and checking the file, and the asterisks and number ranges here, all come from it. A new setting is a row
there, a line in the default file text (`DEFAULT_INI`) and a row in this menu.

App offers English/Spanish and interface scale 100–200% (`MenuScale` under
`[Application]`). The root scales with font/DPI, is bounded by the viewport and
keeps its top position when conditional fields appear. Tabs wrap to two rows
when needed. Labels wrap; long values truncate without horizontal scrolling.
Focus has both a filled background and a bright outline; the active tab has an
underline. The full panel scrolls, including actions; no nested scroll region is
required.

<a id="settings-overlay-validation"></a>

### Validation

`make test-settings-locale` checks English/Spanish label and error-message
coverage, duplicate catalog entries, and nonempty translations.

`make test-settings` covers INI round trips, MenuScale bounds, Auto/Desktop,
environment values, comments, aliases, quest removal, invalid input, failed
writes and live settings.

`make test-settings-panel` drives real ImGui without game files or a graphics
device: seven tabs and shoulder repeat, first-field focus, editor cancellation,
origin restoration, dirty close guards, inactive former save/discard shortcuts, no ImGui
windowing on held face buttons, failed writes, numeric activation without a keypad, pending text during
close, explicit exit/restart, quest addition/removal/reset, 200% Spanish layout,
scrolling and a 640×480 viewport. Runtime smoke checks use an isolated save and
config. Physical controller feel and platform-specific DPI still need human
acceptance before a release.

<a id="settings-overlay-confirmation-button"></a>

### Confirmation button

Controls > Confirm button (recomp) writes `MenuConfirm = Original|South|East` under `[Controls]`. It affects only the PC settings overlay and its guides. Forge and the game's own menus retain their existing mapping. Held buttons must release before a changed mapping accepts them.

<a id="settings-overlay-fsr-settings"></a>

### FSR settings

Graphics & Audio exposes all seven
[Upscaler] fields: FSR, provider (FSR 3.1 / FSR 4), quality, sharpness, object
motion, reactive mask and animated surfaces. They persist without removing INI
comments/aliases, participate in draft equality, dirty guards and section reset,
and are marked restart-only. FSR 4 retains upstream's provider/fallback rules;
selecting it does not install a driver or loader.

Active FSR keeps live MSAA at 1 and FXAA off, including after saving other
settings or staging FSR Off for the next boot. The editor shows the AA rows as disabled "Off (FSR)" while FSR owns them,
and keeps the raw AA values for when FSR is disabled. Existing settings are retained; newly generated defaults
and section reset follow upstream's small-monster-sync Off default.

Regression tests cover all FSR quality presets, every new field's persistence,
invalid enum/float/toggle input, saving with active FSR, staged disable followed
by restart, and the maximum 256 quest overrides alongside the expanded schema.
The panel test exercises provider/quality selection and saving with a gamepad.

<a id="prompt-resources"></a>

## Keyboard and mouse prompt resources

Players only need to enable the Auto prompt mod bundled with their release; see the [user guide](../README.md#keyboard-and-mouse).

This maintainer directory holds development artwork, mappings and the fixed resources distributed from `prebuilt/Keyboard_Mouse_Input_Auto`. Release packaging consumes that prebuilt directory directly. Preserve `romfs/keyboard_mouse_input.flag` and `romfs/keyboard_mouse_prompts.bin` when maintaining the pack.

Implementation: [native input and automatic prompts](#runtime-input-flow). Artwork prompts: [imagegen-prompts.txt](../extras/keyboard-mouse/imagegen-prompts.txt).

<a id="hub-server"></a>

## Hub server

This is the optional hub server shipped by [MHGURecomp Native Keyboard & Mouse](https://github.com/dzxrly/MHGURecomp-Native-KeyboardMouse). Use compatible client builds; availability of the original project's public service is not guaranteed by this repository.

The lobby / relay server for internet hubs: MHGURecomp players find each other's Local Play hubs through it and
play together without forwarding a port. Like an LDN server for Ryujinx, but for this port only (it lists MHGURecomp
hubs of the same port version, no Ryujinx players).

- A hub created in the game is listed here for players with the same **matchmaking key** (`[Multiplayer] Key` in
  `mhgurecomp.ini`, or the settings menu). Friends who all enter the same key only see each other's hubs; without a
  key a hub is public and seen by everyone else without a key. The game sends a hash of the key, never the key.
- Joining goes through the server: it pipes the player's connection to one the host opens for them, and passes the
  game's UDP packets between the two. Nobody needs a forwarded port, and players never see each other's IP address.
- Players on the same LAN still find each other directly (LAN search comes first).

One small executable, no database, no configuration file. Uses one port, TCP and UDP (default 39510).

<a id="hub-server-build"></a>

### Build

Go 1.22 or newer (https://go.dev/dl/), nothing else:

```
cd server
go build -o mhgurecomp-hubserver .
go test ./...
```

`GOOS=linux GOARCH=amd64 go build -o mhgurecomp-hubserver .` builds a Linux server on any OS (also `arm64`).

<a id="hub-server-run"></a>

### Run

```
./mhgurecomp-hubserver -addr :39510
```

Open the port for **TCP and UDP** in the firewall (and forward it, if the server is behind a router). Options:

| Flag                | Default  |                                                                                                                                                                                                                                                                                                        |
| ------------------- | -------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| `-addr`             | `:39510` | address and port (TCP and UDP)                                                                                                                                                                                                                                                                         |
| `-max-conns`        | 4000     | connections at once                                                                                                                                                                                                                                                                                    |
| `-max-conns-per-ip` | 24       | connections at once from one IP address                                                                                                                                                                                                                                                                |
| `-max-hubs`         | 2000     | hubs listed at once                                                                                                                                                                                                                                                                                    |
| `-max-hubs-per-ip`  | 4        | hubs from one IP address                                                                                                                                                                                                                                                                               |
| `-list-max`         | 18       | hubs in one search answer, as many as the game's Join Hub list shows (3 pages of 6). With more hubs than that, each search gets a random choice of them (hubs with a free place before full ones), so a Retry shows others. The answer is sorted: free places first, then the fullest, then the newest |
| `-bandwidth`        | 1048576  | bytes a second per player link and direction (a hub uses far less)                                                                                                                                                                                                                                     |
| `-v`                | off      | log hubs and joins (never IP addresses)                                                                                                                                                                                                                                                                |

With Docker: `docker build -t mhgurecomp-hubserver server` and
`docker run -d --restart unless-stopped -p 39510:39510/tcp -p 39510:39510/udp mhgurecomp-hubserver`.

As a systemd service (Linux), `/etc/systemd/system/mhgurecomp-hubserver.service`:

```
[Unit]
Description=MHGURecomp hub server
After=network-online.target

[Service]
ExecStart=/usr/local/bin/mhgurecomp-hubserver -addr :39510
DynamicUser=yes
Restart=always

[Install]
WantedBy=multi-user.target
```

then `systemctl enable --now mhgurecomp-hubserver`.

<a id="hub-server-using-it-in-the-game"></a>

### Using it in the game

`[Multiplayer] Server = your.host.name` (or `host:port` for another port) in `mhgurecomp.ini`, or "Hub server" in the
settings menu (Multiplayer page). Empty means the default server built into the game, `Off` no internet hubs.
Everyone who wants to see each other's hubs must use the same server.

<a id="hub-server-protocol"></a>

### Protocol

server.go's header comment. In short: control frames (u32 size, u8 type, payload) on TCP; requests LIST, HOST,
JOIN, ATTACH carry "MHGS", the server protocol version, the port's link protocol version and the key's hash. After a
join the two connections are spliced and carry the games' own link (runtime/net/ldn.c), which the server does not
read. UDP datagrams "MHGR" + pair id + the sender's secret + payload are passed, without that prefix, to the other end
of the pair.

Safety: sizes capped, every NetworkInfo checked before it is listed, LIST / JOIN rate limited per IP address,
connections and hubs per IP address limited, handshakes time out, the relay only forwards between the two ends of a
pair it made (never to an address a client names) and a relayed datagram is always smaller than the one received.

<a id="fsr31"></a>

## FSR 3.1 implementation notes

These renderer implementation notes originate in the original project. Their dated observations describe that testing context; they are not validation of this repository's native input changes.

AMD FidelityFX Super Resolution 3.1 (the FSR 3.1.4 temporal upscaler, Vulkan, from the FidelityFX SDK 1.1.4) in the
Vulkan renderer. With `[Upscaler] FSR = On` the game renders its 3D picture at a lower resolution, FSR rebuilds it at
full resolution from several jittered frames (and anti-aliases it), and the HUD is drawn at full resolution on top.
Default: Off. With FSR off nothing of it runs (every hook point is behind `g_fsr_on` / `g_config.fsr`).

<a id="fsr31-what-works-checked-in-game-rx-9070-xt-windows-1920x1080-window-unless-noted"></a>

### What works (checked in game, RX 9070 XT, Windows, 1920x1080 window unless noted)

- FSR 3.1 in a quest (Jurassic Frontier, quest 101) and in the village: 60 FSR frames a second, HUD, menus and
  text at full resolution (`fsr1.png`, `menu_fsr.png`, `vfsr_turn.png` in the test shots, see below).
- Quality (1280x720 -> 1920x1080), Performance (960x540 -> 1920x1080), NativeAA (1920x1080 -> 1920x1080), 21:9
  (2560x1080 window: 1706x720 -> 2560x1080, HUD placement as without FSR), RenderResolution 3840x2160 (2560x1440 ->
  3840x2160), Balanced set in an ini file (`FSR = On`, `FSRQuality = Balanced`, `Sharpness = 0.5`, `MSAA = 4x`:
  1128x634 -> 1920x1080, MSAA forced off). UltraPerformance only through the settings code, not run.
- Title, logos, loading and other frames without a 3D scene: no FSR, the picture scaled to the output size, their
  2D drawn at full resolution (as sharp as without FSR); the jitter stops after 3 such frames.
- Camera jitter and motion vectors, measured (not only looked at):
  - jitter sign: `MHGU_JITTER_TEST=40,40` moves the 3D picture exactly 40 px right and 40 px down (image correlation
    of two screenshots), HUD unmoved;
  - motion vectors (`MHGU_FSR_MVCHECK=1`, camera turning and pitching in a quest): mean luma difference to the
    previous frame 0.09-0.12 at the same texel, 0.023-0.036 along the vectors, 0.066-0.139 along the vectors with y
    negated, so direction, scale, y convention and the camera/frame pairing are right;
  - the frame's camera is found in its own uniform buffers every FSR frame (`camera matched 60` a second).
- Vulkan validation (`MHGU_VK_VALIDATION=1` with the SDK's layer): FSR on gives exactly the messages FSR off gives
  (two old ones of the game's renderer), none from FSR.
- MSAA and FXAA are forced off with FSR on (logged; F7 / `msaa` say "MSAA stays off while FSR is on").
- GPU time (MHGU_CONTROL `gpuprof`, quest): 1080p output: prep + FSR ~0.5 ms, frame 1.02 ms (native 1080p frame
  0.85 ms); 4K output: prep + FSR 1.8-2.0 ms, frame 3.04 ms (native 4K 2.55 ms). On this GPU the game is so light
  that FSR Quality costs more than it saves; the gain is anti-aliasing and, on slower GPUs, speed. Frame rate 60
  (vsync) in every test.

<a id="fsr31-settings-mhgurecompini-new-section-older-files-get-it-added-with-the-defaults-at-the-next-start"></a>

### Settings (mhgurecomp.ini, new section; older files get it added with the defaults at the next start)

```ini
[Upscaler]
FSR = Off              ; On / Off
FSRQuality = NativeAA  ; NativeAA (1x, default), Quality (1.5x), Balanced (1.7x), Performance (2x), UltraPerformance (3x)
Sharpness = 0.8        ; RCAS 0 (off) .. 1
ObjectMotion = On      ; motion vectors of moving characters / monsters from their geometry (Off: camera only)
Reactive = 0.9         ; reactive mask strength on the game's moving-object mask, 0 (off) .. 1
AnimSurf = On          ; surfaces animated through their texture: reactive + the current frame mixed in there
```

With FSR on and Borderless, FSR upscales to the screen (in the picture's shape), not to Resolution. Texture LOD bias
log2(render / output) on mipmapped samplers (-0.58 at Quality). More test overrides: `MHGU_FSR_ANIMSURF`,
`MHGU_FSR_SHARPNESS`, `MHGU_FSR_LODBIAS`, `MHGU_FSR_JITTER_SIGN=sx,sy`, `MHGU_FSR_ANIMVIEW=1` (animated-surface tiles
tinted magenta).

Output size = what the 3D resolution would be without FSR (RenderResolution, or the window's with Auto); the game's
3D resolution (`g_render_size`, patched into the game) = output / ratio, in the picture's shape. Test overrides:
`MHGU_FSR=0|1`, `MHGU_FSR_QUALITY=<name>`, `MHGU_FSR_OBJMOTION=0|1`, `MHGU_FSR_REACTIVE=<0..1>`. Debug: `MHGU_FSR_LOG=1` (a line a second: FSR / scaled frames, history
resets, camera match, where the HUD started), `MHGU_FSR_MVCHECK=1` (with the log: the motion vector check above),
`MHGU_FSR_DEBUG=1` (FFX's own input checks), `MHGU_JITTER_TEST=x,y` (a constant picture shift, FSR on or off),
`MHGU_JITTER_LOG=1` (every camera capture).

<a id="fsr31-files"></a>

### Files

New:

- `third_party/fsr-vulkan/` - FSR 3.1.4 upscaler sources: FidelityFX SDK 1.1.4 host + Vulkan backend + its GLSL (compiled at run time with shaderc, `src/ffx_fsr3_shaderblobs.cpp`),
  and the portable C wrapper `ffxVkPortable*` (FireBurn/FSR-Vulkan subset, minus its loader shim). All MIT, licence files kept (`LICENSE.txt`, `upstream/ffx-1.1.4/sdk/LICENSE.txt`,
  `NOTICE.md`, `UPSTREAM.md`). `src/ffx_mingw_portability.hpp` (new, ours): what MinGW-w64 lacks for the
  MSVC-written SDK sources (`swprintf_s`, `<new>`, `<cmath>`). No binaries at build time.
- `runtime/nvn/vulkan/fsr.c`, `fsr.h` - the frame analysis, stand-ins, prep / motion-vector shader (GLSL compiled at
  run time with shaderc, like the renderer's other built-in shaders), FSR context and dispatch, present.
- `runtime/hle/camera_jitter.c`, `runtime/camera_jitter.h` - main camera capture and the jitter.
- This document's [FSR 3.1 section](#fsr31) records the implementation.

Changed (small, marked "FSR"):

- `Makefile` - builds the FSR sources into obj/fsr/ and links them into the executable; `-I` for the wrapper header.
- `runtime/config.c/.h` - `[Upscaler]` keys, render size = output / ratio, MSAA / FXAA off, `g_fsr_output`.
- `runtime/main.c` - `camjit_init()`.
- `runtime/nvn/vulkan/exec.c` - `vk_fsr_draw` before a draw's pass; stand-ins in `begin_pass` (render targets),
  `texture_info` / `resolve_feedback` (sampled); `pass_sx/sy` scale viewport (`draw`), scissor (`scissor_rect`) and
  `pass_size` onto them; the ultrawide HUD rewrite also applies to them. Object motion: `Prog.fsrcap` (the vertex
  shader's source kept for its capture variant, `compile_program`), the capture draw after a depth-writing draw
  (`draw()`), its two bindings in `stage_set`, `vk_run_job` (fsr.c's builds on the shader threads).
- `runtime/nvn/vulkan/device.c` - with FSR on: `shaderStorageImageWriteWithoutFormat` (FSR needs it),
  `shaderFloat16` + `shaderInt16` (its 16-bit variants); `vk_fsr_init`; the present shows the stand-in; MSAA keys.
- `runtime/nvn/vulkan/vk.h` - two feature flags in `g_vk`.

Not changed: `translate.c` (the jitter goes into the camera's projection matrix, not the shaders), `textures.c`,
`shaders.c`, `tools/`.

<a id="fsr31-how-it-works"></a>

### How it works

Camera (`hle/camera_jitter.c`): main+0x1C33C (look-at, r0 = out, r1 = camera) and main+0x1C358 (projection, r0 = out,
r1 = camera; 0x9c617c builds a row-vector, right-handed, depth 0..1 perspective: m[10] = f/(n-f), m[11] = -1) are
hooked (fnhook). For a camera with the screen's aspect (camera +0x38: 16:9, or the picture's shape after the
AspectRatio patch) each projection call is recorded with that camera's last view (ring of 64 captures: view,
unjittered projection, near +0x34 / far +0x30, fovy) and then gets the jitter: AMD's Halton(2,3) - 0.5 sequence,
8 (output/render)^2 phases, one phase per presented frame, written into m[8] / m[9] of the matrix the GPU gets
(`m[8] -= 2 jx / w`, `m[9] += 2 jy / h`; the translator's clip space has +y up). main+0x1C380 / 0x1C3A8 (the CPU
world-to-screen for HUD markers) are not touched, so markers stay steady.

Frame structure (from MHGU_CONTROL `dump`, quest, FSR off): pass 0 shadow map; 1 colour-grading 256x1; 2 depth
prepass into D (render size, D32S8 on AMD); 3 dynamic-object mask (characters and their shadows, blue; with D);
4-5 the scene (with D); 6-28 post (depth of field, bloom chain, half / quarter sizes); 29 into B; 30 into H:
FXAA (B -> H) + bloom add; 31 H -> B copy; 32 the HUD into H (sprites from a 512x512 atlas, depth / stencil off);
33 H -> the window texture P (the brightness / gamma setting: `pow(c, cb3[0][0])`), presented.

Identification (`fsr.c`, per frame, no texture ids):

- H = the first texture the last draw before a present sampled, when that draw wrote the presented texture
  (learnt at each present, used from the next frame; logged when it changes).
- The scene depth = the render-size depth target with the most depth-writing draws this frame.
- The start of the HUD = the first draw into H that is not a post pass (post pass: <= 6 vertices and samples a
  render target other than H, e.g. the bloom add), or else the draw into a window texture that samples H.
- Scene frame = >= 16 depth-writing draws into the scene depth and a camera capture from this frame or the last.
- The frame's camera capture: the one whose jittered m[8] / m[9] appear in a scene draw's vertex uniform buffers
  (adjacent or 16 bytes apart). In game it is always the capture before the newest one at present time; without a
  match (no jitter yet) the newest one is used.
- Menu skip: frames that are not scene frames get no FSR (scaled); after 3 in a row the jitter stops, the next scene
  frame starts it again (FSR history reset at gaps, camera changes, cuts > 3 m).

At the start of the HUD (outside a render pass): scene frame: prep (compute, render size: D's depth -> R32F; motion
vectors = previous position - current position in render pixels from inverse(view*proj) * (view_prev\*proj_prev),
unjittered), then FSR (colour = H as the game finished it: post-processed LDR RGBA8; depth R32F; motion vectors
RG32F; no reactive mask; jitterOffset = the capture's jitter = the picture shift in render pixels, x right / y down;
near / far / fovy from the capture; 0.01 m per unit) into H' = an output-size stand-in of H. Other frames: H scaled
into H' (linear blit). From then until the present, H, the window texture and the render-size depth target of their
passes are replaced by output-size stand-ins (H', P', D' - D' cleared to far): the HUD and the gamma pass render at
the output size, and the present shows P'.

<a id="fsr31-temporal-quality-object-motion-and-the-reactive-mask-2026-10-26"></a>

#### Temporal quality: object motion and the reactive mask (2026-10-26)

Order at the HUD start (scene frames): prep (depth, camera vectors) -> velocity pass -> finish (compute) -> FSR.

- Capture (`exec.c draw()` -> `vk_fsr_capture`): every depth-writing draw into a render-size colour + depth target
  before the HUD start (not into H) is drawn once more through a variant of its own vertex shader: the translated GLSL
  plus `positions[base + gl_VertexIndex] = gl_Position` (set 0 bindings 30 / 31; rasterizer discard). The variant is
  built on the shader threads the first time the program is captured; until then the draw just isn't captured. The
  positions are the game's own (camera, jitter and skinning in them). Indices are copied to host memory (read from
  the guest's own index data: reading the renderer's write-combined transient copy cost 30 ms a frame).
  Equivalent of the reference's transform feedback; needs `vertexPipelineStoresAndAtomics` and
  `fragmentStoresAndAtomics` (else ObjectMotion is off with a log line, the reactive mask still works). No CPU
  expansion of primitive-restart strips is needed: the velocity pass draws the same indices, topology and restart.
- Pairing with the previous frame: vertex program + index buffer + counts + first + base vertex + topology + vertex
  buffers (repeats in order); without the vertex buffers when unique in both frames; same gl_VertexIndex range.
- Velocity pass: each pair rasterized at the current positions (no culling, no depth attachment) into RGBA16F; a
  fragment keeps (previous px - previous jitter) - (current px - current jitter) where its linear depth is within
  max(2 units, 1%) of the scene's and |mv| <= 64 px.
- Static draws: on every 4th texel the pass notes per draw "seen" / "differs from the camera by > 0.25 px" (read back
  frames later); 240 checked frames without differing -> left out for 600 frames (camera vectors are right for them).
  The notes are atomics into device memory, copied to host memory for the CPU after the pass. (Atomics straight
  into host memory each crossed the bus: in town, 225 draws a frame, the pass took 12 ms of a 15 ms frame on an RTX
  3090 at 1080p, FSR ran at ~28 fps; with the copy 0.2 ms, ~50 fps in the same hidden test run.)
- Finish: object vector where valid, else camera. Reactive from the game's moving-object mask (the first render-size
  colour target drawn with the scene depth, when it is not the main scene target; blue/green channel; characters,
  monsters, their shadows): only where the silhouette changed - outside the 3x3 min/max of the previous frame's
  silhouette at texel + vector (ghost trail behind a moving object, its new leading edge). A standing hunter is in his
  own box: no reactive, so he does not alias more. x Reactive (0.9), capped to 0.1 on texels with valid object vectors.
- HUD / 2D never get vectors or reactive: the capture needs depth writes into the scene targets before the HUD start
  (HUD draws here have depth / stencil off and come after it); the mask is the game's own 3D pass. Checked: 16:9 and
  21:9 quest with pause menu, title, character select, quest loading.

Implementation notes:

- The HUD is drawn after FSR at the output size (stand-ins), so no HUD layer, composite or HUD blend handling
  (alphaToOne / multiply transmittance) is needed.
- Menu 3D (character select, previews): the models are drawn after the HUD start, i.e. straight into H' at the output
  size, unjittered (jitter is off in non-scene frames) - no FSR, no history, so no smear
  (checked: character select, FSR on vs off, same sharpness).
- Object positions are captured by a storage-buffer store in the vertex shader (no transform feedback, no strip
  expansion).
- Reactive mask: the silhouette change of moving objects, written in blue (and green).
- Animated surfaces: a second pass (see the audit below), with characters left out and a half-pixel search.
- Not implemented: block matching for object vectors, a low-rate cloth "jump" reactive mask, pose signals. Every
  depth-writing draw is captured (object vectors measured right on moving geometry), and no cloth that re-poses at a
  low rate was seen (the village cloth moves every frame), so none of them was needed for the artifacts measured.
- The ultrawide UI matrix rewrite (exec.c cbuf_info) only touches depth-test-off draws (HUD), which are never
  captured; runtime/hle/hpscale.c changes monster HP only, nothing in rendering.

Measured (quest 101 area 2, herbivores moving, MHGU_FSR_MVCHECK on texels of the game's moving-object mask, mean luma
difference to the previous frame; same script run with ObjectMotion = Off + Reactive = 0, i.e. the previous build's
behaviour, and with the defaults): hunter walking with the camera following: camera vectors 0.035 / 0.042 -> FSR's
vectors 0.026 (camera-only run: 0.042); standing hunter, moving herbivores: 0.021-0.023 -> 0.019-0.020. Earlier walk
in area 1: 0.113-0.131 -> 0.073-0.093. Screenshots: walking hunter limbs and a herbivore sharper than with camera
vectors only (`cmp_walk.png`: Off / old / new), standing hunter not more aliased (`cmp_stand.png`). 60 fps, capture
CPU 0.03-0.14 ms a frame, ~50-155 draws captured a frame. Validation: no new messages (one, dynamic depth bias after
a capture draw, was found and fixed).

<a id="fsr31-known-issues--not-done"></a>

### Known issues / not done

- Object motion covers geometry drawn with depth writes. Not covered: transparent / blended effects (no depth write:
  particles, water surface, some hair cards) - they keep camera vectors; texture or UV animation (flipbooks); objects
  whose vertex data moved to another buffer and whose rest is not unique (unpaired, ~1 % of draws). Fast objects
  (> 64 render px a frame) fall back to camera vectors.
- A draw left out as static that starts moving keeps camera vectors until its 600 frames are over (10 s at 60 fps);
  which draws were left out was not checked one by one (the log only counts them: 6-56 at a time in the tests).
- Area-intro cutscenes and other event cameras are shown scaled (no FSR) while the main camera hook does not run.
- Output size is fixed at start-up; a window resized later shows the output scaled (as the 3D picture was).
- After the start of the HUD: copies into H / P go to the render-size originals, and HUD draws that would depth-test
  against the scene's depth see an empty D'; neither was seen in the tested frames (HUD draws have depth / stencil off).
- GPU requirements: subgroup operations, `shaderStorageImageWriteWithoutFormat`, FSR's storage formats; without them
  FSR is off with a log line (the picture is scaled). Not tried on the R9 270X / GCN1 baseline, NVIDIA, Intel, Linux.
- Not tested in game: UltraPerformance, the hunter creation and equipment preview screens (the character select
  was), a large monster in a fight (only herbivores in quest 101's area 2), multiplayer, Alt+Enter / borderless on
  another monitor size.
- FSR 4: see docs/DEVELOPMENT.md#fsr4 (`Upscaler = FSR4`, native Vulkan through the driver); block matching / animated surfaces / HUD layer: see above.

<a id="fsr4"></a>

## FSR 4 implementation notes

These renderer implementation notes originate in the original project. Their dated observations describe that testing context; they are not validation of this repository's native input changes.

AMD's machine-learning upscaler (FSR 4, provider "FSR 4.1.2") as an alternative to FSR 3.1 in the Vulkan renderer
(`docs/DEVELOPMENT.md#fsr31` describes FSR 3.1 and everything around it, which this builds on). It runs **natively on Vulkan**,
on the game's own VkDevice and command buffer: no Direct3D 12 bridge, no shared images, no split submits. It takes
exactly the inputs fsr.c prepares for FSR 3.1 (colour, depth, camera + object motion vectors, reactive mask, jitter,
camera near / far / FOV) and is dispatched where the FSR 3.1 dispatch is. Default stays FSR 3.1, FSR stays Off by default.

Tested on: RX 9070 XT, Adrenalin 26.10.1 (Vulkan driver 1.4.359), Windows 11. FSR 4 needs an RDNA 4 GPU and the
driver package that contains `amdvkffx64.dll` (v2.4.0.4370 here).

<a id="fsr4-settings"></a>

### Settings

```ini
[Upscaler]
FSR = On
Upscaler = FSR4        ; FSR3 (default) or FSR4
FSRQuality = Quality   ; same ratios as FSR 3.1: NativeAA 1x, Quality 1.5x, Balanced 1.7x, Performance 2x, UltraPerformance 3x
Sharpness = 0.8        ; the provider's RCAS sharpening (same key as FSR 3.1)
```

Choose the upscaler in F1 and save, or edit `[Upscaler] Upscaler` in `mhgurecomp.ini`. Saving adds missing keys.
Test overrides: `MHGU_FSR_UPSCALER=3|4`, `MHGU_FSR4_DLL=<path>` (another DLL; a missing path shows the fallback),
`MHGU_FSR4_LINEAR=1` (colour not flagged as gamma-encoded, A/B), `MHGU_FSR4_FAILTEST=n` (the n-th FSR 4 dispatch is
reported as failed, to see the fallback), `MHGU_FSR_DEBUG=1` (FFX input checks), `MHGU_FSR_LOG=1` (per second:
`upscaler FSR 4.1.2, FSR 4 frames N`).

To try it: set `FSR = On`, `Upscaler = FSR4` (any FSRQuality) and start the game; log.txt has the `[fsr4]` lines.

<a id="fsr4-what-is-included"></a>

### What is included

FSR 4 on Vulkan is part of this port and is **opt-in**: it needs `FSR = On` and `Upscaler = FSR4` (default `FSR3`; FSR itself is Off by
default), and if it cannot start the game uses FSR 3.1 instead (every reason is logged with a `[fsr4]` line). No AMD file is in this
repository or in a release.

Routes, tried in this order (each logs which one was used; `MHGU_FSR4_ROUTE=driver|loader` tries just one, for testing):

1. **the driver's provider**: the game loads `amdvkffx64.dll` from the player's AMD driver and asks it for the FSR 4 provider. Log:
   `SDK-module check hook installed`, `route: the driver's provider`.
2. **a loader DLL beside the exe** (experimental, see below): used through the public ffx-api (`ffxQuery` to find a 4.x version, then
   `ffxCreateContext` / `ffxDispatch`). Log: `loader ...`, `route: the player-supplied loader`.
3. **FSR 3.1.**

<a id="fsr4-getting-the-fsr-4-dll"></a>

### Getting the FSR 4 DLL

No AMD file is in this repository or in a release. On a Radeon with FSR 4 support (RDNA 4) and a recent Adrenalin driver there is nothing to
download: the driver installs `amdvkffx64.dll` itself and the game finds it automatically: beside `mhgurecomp.exe` if you put a copy there,
else in the driver-store folder of your GPU, else the newest copy under
`C:\Windows\System32\DriverStore\FileRepository\u*.inf_amd64_*\B*\amdvkffx64.dll`. `MHGU_FSR4_DLL=<path>` uses a different file.

The driver only hands out its provider when it sees the FidelityFX loader modules in the process (it asks with `GetModuleHandleW`), so the game,
in its own memory only (nothing on disk), points that one import of the driver DLL at a function that answers "loaded" for an `amd_fidelityfx_*`
module and defers to the real function for everything else (`patch_import` in `fsr4.c`; the idea comes from reading OptiScaler's source, no code
copied). A driver update that changes that check could stop it working; then FSR 3.1 is used.

_Experimental, route 2:_ the code also accepts a loader DLL named `amd_fidelityfx_vk.dll` placed beside `mhgurecomp.exe` (or
`MHGU_FSR4_LOADER=<path>`), used through the public ffx-api without any patching, if one exists that offers FSR 4. The `amd_fidelityfx_vk.dll`
v1.0.1 found on the test machine does not (it offers FSR 3.1.4 and 2.3.3 only, so the game logged `this loader offers no FSR 4` and ran FSR 3.1).
This route is implemented but unverified; its backend descriptor id (0x3, from ffx-api's public Vulkan header) is an assumption.

Notes: the vendored `third_party/ffx-api` headers and the FSR 3.1 sources in `third_party/fsr-vulkan` are MIT per AMD's licence file in the
FidelityFX SDK (texts kept and listed in `THIRD_PARTY_NOTICES`). The in-memory import step changes AMD's driver DLL in this process; the author
has not reviewed it against AMD's driver terms. For that reason the patch is **off by default**: a normal `make` compiles none of it and FSR 4 is then
only available if a loader as described above exists, otherwise FSR 3.1 runs. `make FSR4_DRIVER_PATCH=1` builds the driver route in (this is what the
measurements below were made with); it is a build switch, so a distribution can decide, or players can build it themselves.

<a id="fsr4-how-it-works-runtimenvnvulkanfsr4c-fsr4h"></a>

### How it works (runtime/nvn/vulkan/fsr4.c, fsr4.h)

- The AMD Vulkan driver package carries `amdvkffx64.dll`. It exports `UpdateFfxApiProvider` / `UpdateFfxApiProviderEx`,
  the FidelityFX "external provider" hook (the same one the D3D12 side uses with `amdxcffx64.dll`; the MIT ffx-api
  source of SDK 1.1.4, `ffx_provider.cpp`, shows the D3D12 caller). Called with a description type it fills in a table of
  function pointers (create / destroy / configure / query / dispatch, plus `isSupported`), for the upscaler: FSR 4.
- (Route 1 only, `FSR4_DRIVER_PATCH=1`.) Found at start-up: beside the exe, else in the driver store folder of the GPU (D3DKMT user-mode driver path of the
  Vulkan device's LUID), else the newest copy in `System32\DriverStore\FileRepository`. Loaded with LoadLibrary at
  run time; **nothing of AMD's is built in or shipped** (`*.dll` is git-ignored).
- The driver only hands out providers when it sees FidelityFX loader modules loaded: it asks
  `GetModuleHandleW(L"amd_fidelityfx_loader_vk.dll")` / `..._upscaler_vk.dll`. Without them the call fails with
  E_INVALIDARG (checked). fsr4.c patches that one import of the driver DLL so it gets the DLL itself as the answer.
  This is how OptiScaler (GPL-3.0; its source was read as a reference for how the driver is reached and which
  descriptors it takes - no code copied) does it; the code here is fresh.
- Context: `ffxCreateContext` through the provider with `ffxCreateContextDescUpscale` (flag
  `FFX_UPSCALE_ENABLE_NON_LINEAR_COLORSPACE`) -> the Vulkan backend descriptor (SDK 2.x type id `0x01000003`, same layout
  as `ffxCreateBackendVKDesc`: device, physical device, vkGetDeviceProcAddr) -> a version descriptor (`0x0001000B`,
  version 4.1.2; the driver accepts 4.0.0 .. 4.1.2). Created once, at the first FSR frame (66-73 ms), because the
  ratio does not change during a run.
- Dispatch: `ffxDispatchDescUpscale` (the ffx-api 1.1.4 layout works unchanged) with VkImage resources and the
  command buffer; all images are in GENERAL (the renderer's rule) = state "unordered access" for every resource. Colour
  is the game's gamma-encoded LDR RGBA8, so `FFX_UPSCALE_FLAG_NON_LINEAR_COLOR_SRGB` is set (depth: standard Z, no
  flags; motion vectors in render pixels, jitter removed; motionVectorScale 1; preExposure 1).
- Jitter: the same picture shift FSR 3.1 gets (the camera hook applies AMD's Halton jitter, 8 (output/render)^2 phases).
- fsr.c: `run_fsr` is shared; after the prep / object-velocity / finish passes it records either the FSR 3.1 dispatch
  or `vk_fsr4_dispatch`. On any FSR 4 failure it logs why, switches FSR 4 off for the run and records the FSR 3.1
  dispatch for that same frame with a history reset (FSR 3.1's context is created only then).
- Device (device.c, only with FSR on and `Upscaler = FSR4`): extensions `VK_KHR_cooperative_matrix`,
  `VK_EXT_shader_float8`, `VK_KHR_synchronization2`, `VK_EXT_subgroup_size_control` and the features the provider's
  shaders declare (cooperative matrix, float8 + float8 cooperative matrix, synchronization2, subgroup size control /
  full subgroups, shaderFloat16 / Int8 / Int16, buffer device address, Vulkan memory model, 8 / 16-bit storage,
  variable pointers, descriptor indexing, storage image read / write without format); instance apiVersion 1.3 instead
  of 1.2 (its SPIR-V is 1.6). If the GPU lacks any of it: logged, FSR 3.1 runs. With FSR off or `FSR3` nothing changes.

Log lines (log.txt / stdout): `[fsr4] loaded <path> (<where>), file version`, `SDK-module check hook installed`,
`driver provider "FSR 4.1.2" id 0x...`, `the provider accepts this device`, `FSR 4.1.2 context WxH -> WxH created in N ms
(...formats...)`, `first FSR 4.1.2 dispatch recorded`, `[fsr] on (FSR 4.1.2): ...`, failures: `[fsr4] ... FSR 3.1 is used` /
`... FSR 4 is off from here on, FSR 3.1 takes over`.

<a id="fsr4-files"></a>

### Files

New: `runtime/nvn/vulkan/fsr4.c`, `fsr4.h`; `third_party/ffx-api/` (ffx_api.h, ffx_api_types.h, ffx_upscale.h from the
FidelityFX SDK v1.1.4, MIT, licence file kept); this file.
Changed: `runtime/config.c/.h` (`Upscaler` key, `MHGU_FSR_UPSCALER`), `runtime/nvn/vulkan/device.c` (the FSR 4 extensions,
features, apiVersion), `runtime/nvn/vulkan/fsr.c` (the branch in `run_fsr`, init, a log line), `Makefile`
(`-Ithird_party/ffx-api`, `-lversion`, `FSR4_DRIVER_PATCH`, default 0). Not changed: shaders, textures, exec.c, the camera / jitter code.

Note: the conventions used here (4.x provider chosen by version id, gamma-encoded LDR colour with the non-linear flags,
depth / motion-vector conventions, one jitter convention for all providers) were worked out with a Direct3D 12 route
first; the Vulkan provider made that route unnecessary, so it is not part of this port.

<a id="fsr4-not-done--known-issues"></a>

### Not done / known issues

- Only tested with the one provider this driver gives (4.1.2), only the Vulkan renderer on Windows, an RX 9070 XT.
  Other GPUs: the provider's `isSupported` decides (FSR 4 needs cooperative matrix + float8; RDNA 3 uses INT8 models in
  the driver package, untested).
- Linux: unconfirmed whether FSR 4 on Vulkan runs there. FSR 4 is expected to come to Linux, but whether AMD's DLL route or
  a similar trick works there is not confirmed here. In this build the FSR 4 code is compiled out on non-Windows and
  FSR 3.1 is used (the FSR 3.1 code was syntax-checked for Linux with gcc, not linked or run).
- Tested by the author in game since: a large monster fight (no ghosting or shimmer seen), the village. Not tested:
  4K, multiplayer.
- Loading uses a hook on the driver DLL's import table (see above); a driver whose DLL checks other modules would refuse
  (then: `the driver gave no upscaler provider`, FSR 3.1). The driver DLL is trusted code loaded from the driver store
  (or beside the exe: do not put a copy there you do not trust).
- FSR 4 on the AMD driver recreates its network (pipelines) when the ratio changes; the context is created once so
  this port never does. Changing Upscaler / FSRQuality needs a restart (as FSR 3.1's quality already did).
- The colour input is gamma-encoded LDR: FSR 4 was designed for that as well as HDR; `MHGU_FSR4_LINEAR=1` compares.
- The FSR 4 GPU cost in the table includes the shared prep / finish passes (~0.5 ms at 720p), measured with the
  renderer's gpuprof, not with the provider's own timestamps.
- `DX12 bridge`: not built (not needed: the driver's Vulkan provider works).

<a id="pipeline-cache"></a>

## Recording the pipeline list

`extras/pipelines.list` lists the shader pipelines (a shader program plus its blend / depth / vertex layout / target
formats) the game uses in the common areas. `make` copies it next to the executable and `make dist` ships it. On a
first run the game builds them in the background, so entering those areas for the first time doesn't show surfaces
black for a moment. Background: [runtime architecture](#runtime).

The game never says which combinations it will use, so they are recorded by playing. Every computer records the
pipelines it draws in `cache/pipelines.bin` next to the game; exporting turns those into a list that works on any
GPU and keeps working after shader translator changes. The list holds only hashes of the game's shader code and
render state, nothing of the game itself.

<a id="pipeline-cache-what-to-record"></a>

### What to record

- Pipelines with MSAA are separate entries: record each MSAA setting players use (Off, 2x, 4x, 8x). The HUD and menus
  don't change with MSAA, so the town and a few hunts per setting are enough there.
- Areas: the hubs (village, Hub, Bherna...) and the hunting areas players visit most. Each area's terrain, effects
  and monsters add their own entries; the more areas, the fewer first-visit black surfaces.
- FSR may change some entries (not checked): if players use it, record a hunt or two with it on. Resolution, frame
  rate and draw distance don't change pipelines.

<a id="pipeline-cache-steps-windows"></a>

### Steps (Windows)

You need a build from this repository at or after commit 7cc82f1 (`make`; the game is `build\mhgurecomp.exe`).

1. Set the MSAA you want to record (F1 menu, Graphics & Audio, or `MSAA` in `build\mhgurecomp.ini`), restart if asked.
2. Play: walk through the town and do a few hunts in different areas. Everything you see is recorded in
   `build\cache\pipelines.bin` as you play; nothing else to do.
3. Quit the game. Export, from the repository folder in PowerShell:
   ```powershell
   $env:MHGU_PIPELIST_EXPORT = "$PWD\pipelines.list.new"; Start-Process build\mhgurecomp.exe
   ```
   The export happens at the first frame the game draws. `build\log.txt` then has a line like
   `[vk] pipeline list exported: ...\pipelines.list.new (1520 pipelines)`. Quit the game.
4. Replace the list and copy it next to the game:
   ```powershell
   Move-Item -Force pipelines.list.new extras\pipelines.list; make
   ```
   (From the MSYS2 shell: `mv -f pipelines.list.new extras/pipelines.list && make`.)
5. Repeat from step 1 for the next MSAA setting. Each export contains the current list plus everything new, so the
   list only grows.
6. Commit `extras/pipelines.list`.

On Linux the same, with `MHGU_PIPELIST_EXPORT=$PWD/pipelines.list.new ./build-linux/mhgurecomp` (or wherever the build
is) in step 3.

<a id="pipeline-cache-things-to-know"></a>

### Things to know

- Export with the same build you played with: `cache/pipelines.bin` is thrown away (silently started over) when the
  shader translator changes (`runtime/nvn/maxwell/translate.c`, `shader.h`). Don't delete `cache\` before exporting.
- The export merges the list next to the executable (`build\pipelines.list`) with `cache\pipelines.bin`. Pipelines the
  game built from the list aren't written to `pipelines.bin`, so step 4's `make` (which copies the new list there)
  matters before the next round.
- Lists from several computers combine the same way: export on one computer with the other's list in `extras/` (and
  `make`), and the result has both.
- If the log says `not a pipeline list of this version, not used`, the shader key or pipeline record layout changed in
  the code (`ShaderKey`, `PipeKey` in `runtime/nvn/vulkan/exec.c`): the list has to be recorded again.
- Checking it worked: start with an empty `cache\` (rename it) and `MHGU_VK_STATS=1`; the log's `[vk] 2s builds: ...
skipped N` lines show how many draws were skipped while shaders were built. `MHGU_NO_PIPELIST=1` turns the list off
  for comparing.

<a id="skill-points"></a>

## Armor skill point research

Goal: double the skill points from armor, talismans and decorations (all equipment), without touching the data
tables (MHGU Rebalanced replaces `loc/arc/resident.arc`, which holds the skill tables). Done as cheats (extras/cheats,
runtime/cheats.c). Addresses are runtime addresses (image offset + 0x200000); Atmosphere cheats use the image offset
(runtime - 0x200000: the wiki's "Always Lucky" writes `mov r0, #1` at 0x3822DC over `bl hasSkill(0xfb)` at runtime
0x5822dc). Main's build id: `fb08f1d20fd1204fe7070672777f869b`.

<a id="skill-points-the-calculation"></a>

### The calculation

- `f_2dee34` sets up an equipment-skill object and tail-calls `f_2df824`, which computes the skills. Callers of
  `f_2dee34`: 0x46f078, 0x46fc38, 0x46fcc0, 0x46fd34 (player), 0x7f1abc, 0x7f2130, 0x8ff17c, 0x8ff20c, 0x92cf84,
  0x92cfe8, 0x93ff14, 0x93ff94, 0x95cec8, 0x95cf58 (equipment box / menus, by the look of it; not checked).
- The object (`sb` in `f_2df824`) has 7 equipment slots of 0x2c bytes from +0x14: 0 weapon, 1 head, 2 body, 3 arms,
  4 waist, 5 legs, 6 talisman (body is the slot Torso Up multiplies, the talisman the one Double Talisman adds again).
  `f_2ddd48(slot)` rebuilds a slot's entries from the equipment (called for all 7 at the start of `f_2df824`, and from
  many other places); `f_2de150(slot, k)` returns entry k of 11 (`slot+0x28` + k\*4: u16 skill tree, s8 points). The
  11 entries are the piece's own skills and its decorations' skills, one entry per source.
- Results at `[sb+0x188]`: 77 rows of 12 bytes. +4 u16 activated skill (skillData id), +6 u16 skill tree
  (skillTypeData id), +8..+0xe s8 points per slot, +0xf s8 total (the bytes past +0xb are the next row's unused
  first word). **Byte arithmetic**: a total over 127 wraps.
- Pass 1 (0x2dfa2c-0x2dfb9c): for each slot and entry, `ldrsb r0, [entry+2]` at 0x2dfaa8; for the body slot
  `mla r0, r0, r8, r0` (r8 = number of Torso Up entries, tree 0xcb = 203), else `uxtb r0, r0` at 0x2dfab4; then added
  with `sxtab` to the slot column and the total. Torso Up entries themselves are counted, not summed.
- Thresholds (0x2dfbd4...): total <= -20/-15/-10 or >= 10/15/20/25 picks the activation from the skillTypeData row
  (`f_6cf018`, +0xa..+0x16). Activation 0x146 (326) is Skill +2, 0x147 (327) is Double Talisman.
- Double Talisman (flag `sp+0x81e`): the talisman slot's points are added once more (0x2dfccc-0x2dfd48, and again in
  0x2e01e0-0x2e025c when Skill +2 is also active: `add r2, r2, r0` / `add r0, r2, r0` at 0x2dfd38/0x2dfd44 and
  0x2e024c/0x2e0258), then the thresholds are evaluated again. Skill +2 (flag `sp+0x81f`, 0x2dfda0...) adds 2 to the
  total of every tree with points.
- Other readers of the entries: 0x78062c, 0x780848, 0x82a854, 0x82a8c4 (probably per-piece displays; not checked).

<a id="skill-points-doubling-everything"></a>

### Doubling everything

One instruction: 0x2dfab4 `uxtbne r0, r0` -> `lsl r0, r0, #1` (E1A00080). Every entry counts twice, the body after
Torso Up's multiplier (`sxtab` only uses the low byte, so the dropped `uxtb` changes nothing). Double Talisman then
adds the talisman's plain points once more (x3); patching its adds to `add r2, r2, r0, lsl #1` (E0822080) and
`add r0, r2, r0, lsl #1` (E0820080) at all four places makes it x4. As an Atmosphere cheat:

```text
[Equipment skill points x2]
04000000 000DFAB4 E1A00080

[Double Talisman doubles again (with the above: talisman x4)]
04000000 000DFD38 E0822080
04000000 000DFD44 E0820080
04000000 000E024C E0822080
04000000 000E0258 E0820080
```

Shipped as extras/cheats/Skill points x2.txt (both parts in one cheat). The port runs it since cheat support
(runtime/cheats.c: the changed function f_2df824 runs in the interpreter).

<a id="skill-points-doubling-only-positive-points"></a>

### Doubling only positive points

No room in place (0x2dfaa8-0x2dfab8 is `ldrsb; cmp r4, #2; mlaeq; uxtbne; b`, and doubling only positives needs two
more instructions), so the new code goes into the zero padding after main's .text (main+0x13F1DEC..0x13F2000,
executable on the console too):

```text
main+0x13F1F00: cmp r0, #0; addgt r0, r0, r0; b 0x2dfaf8      (0x2dfab4 -> b main+0x13F1F00)
main+0x13F1F10: ldrsb r0, [r0, #2]; cmp r0, #0; addgt r0, r0, r0; b 0x2dfcd0   (0x2dfccc -> b there)
main+0x13F1F20: the same, back to 0x2e01e4                    (0x2e01e0 -> b there)
```

The last two are Double Talisman's passes, so its extra talisman points are doubled the same way (positive talisman
points x4, negative x2 as in the game). Plain `b` both ways: no register changes (lr is free in f_2df824 anyway: it
is set before every use). Shipped as extras/cheats/Skill points x2 (positive only).txt.

Tested on the test save's hunter (Double Talisman and Skill +2 active), table read with `peek` after quick-hunting
(skilltest.sh): tree 35 base 1,2,2 + talisman 8 (+2) = 15; positive only 2,4,4,16 (+2) = 28 (tree 70 went up a
tier, 142 -> 140), its negatives unchanged (tree 25: -1,-1,-1,-2 (+2) = -3); full x2 the same positives and tree 25
-2,-2,-2,-4 (+2) = -8.

Open: the s8 total can wrap for extreme builds (Torso Up stacks x5 on the body, doubled: x10); whether other hunters'
games compute your skills (multiplayer); whether any screen totals skills elsewhere.

<a id="valor-sheathe"></a>

## Valor Sheathe research

Goal: undo one change of MHGU Rebalanced (GU Rebalanced) without editing the mod: the damage taken in a Valor
Sheathe evade (Valor style, hold sheathe, the next hit is auto-evaded with reduced damage). Rebalanced lowers the
reduction from 70% to 50% (Blademaster) and from 85% to 65% (Gunner). Done as a cheat:
`extras/cheats/Valor Sheathe original damage reduction.txt`.

<a id="valor-sheathe-where-the-values-are"></a>

### Where the values are

- `player\param\pl_com` (type 0x496F8F22, a "FUP" parameter file) in `nativeNX/arc/player/quest/com.arc`, the only
  archive that has it. Rebalanced replaces the whole archive (also `player\alchemyData`), so another mod replacing
  it would have to carry a copy of all of it.
- Damage multipliers (the share of the damage that still gets through), file offsets:
  +0x224 Blademaster 0.30 (Rebalanced 0.50), +0x250 Gunner 0.15 (Rebalanced 0.35). Rebalanced's other pl_com
  changes: +0x44/+0x48/+0x4c (ints 2/5/12 -> 3/8/16), +0x1e8/+0x1ec (240/480 -> 1.0), +0x248 (26 -> 12).
- Loaded, the file's 122 floats (from file +0x6c, 0x1e8 bytes) are a heap block of their own: Blademaster at
  block +0x1b8, Gunner at +0x1e4 (the last float). (main holds a copy of the original defaults at 0xf04e8c; not used
  for this.)

<a id="valor-sheathe-the-pointer-chain-found-in-a-quest-find-the-ferns-with-gu-rebalanced"></a>

### The pointer chain (found in a quest, Find the Ferns, with GU Rebalanced)

```text
sPlayer *            = [0x1a85b58]          (Atmosphere: main+0x1885B58)
player (slot 0)      = [sPlayer + 0x1c]
pl_com resource      = [player + 0x2638]     (+0x8 its path "player\param\pl_com", +0x5c type 0x496F8F22)
float block          = [resource + 0x74]     (+0x70 the int block, +0x78 a third block)
```

The resource is one shared object (every player slot points at the same one), so slot 0 covers everyone's damage on
that game. The cheat checks each pointer for 0 and the resource's type before writing, and writes every frame (the
archive is loaded again for every quest). Checked in the port: with Rebalanced and the cheat, the block holds
0.3 / 0.15 and Rebalanced's +0x248 change is still there. The damage itself was not measured in a hunt.

<a id="valor-sheathe-other-values"></a>

### Other values

The last two store lines of the cheat write the multipliers as float bits (`64000000 00000000 <float>`), first
Blademaster, then Gunner. Damage reduction -> value:

| Reduction | Multiplier | Float bits |
| --------- | ---------- | ---------- |
| 90%       | 0.10       | 3DCCCCCD   |
| 85%       | 0.15       | 3E19999A   |
| 80%       | 0.20       | 3E4CCCCD   |
| 75%       | 0.25       | 3E800000   |
| 70%       | 0.30       | 3E99999A   |
| 65%       | 0.35       | 3EB33333   |
| 60%       | 0.40       | 3ECCCCCD   |
| 50%       | 0.50       | 3F000000   |

<a id="small-monster-sync"></a>

## Small monster synchronization research

Small monsters ("zako") are synced between the players in a zone like large monsters are: positions, movement and
actions. The original game syncs only their HP / deaths; each player's game runs their AI itself, so everyone sees
them elsewhere. Implemented in runtime/net/emsync.c; optional and off by default in this repository: ini `[Multiplayer] SyncSmallMonsters = Off`
(env `MHGU_SYNC_ZAKO=0/1`). Addresses below are runtime addresses (image offset + 0x200000: what
`tools/disasm.py`, `fntrace` and thread dumps use). `tools/retool.py` (strings / xref / callers / dti / vtslot)
finds all of this again.

<a id="small-monster-sync-how-the-game-syncs-monsters"></a>

### How the game syncs monsters

Network layer: MT Framework RPC (`cRemoteCall` messages, `cRemoteProcedure` handlers) in `nFestaNetwork`, over
Nintendo Pia. Large monsters have a _master_ player whose game runs the AI; at every new action it sends
`netEmAction` (0xcc bytes: position, rotation, HP, action, area / waypoint state, targets, ailments...) and the
others replay it. Small monsters are spawned per zone from `quest\zako\emSetList\`; only their herds ("Mure":
`netMureEm*`) and HP / deaths are synced.

Small and large monsters are one class family (`uEmsNNN_00` / `uEmNNN_00` -> `uFestaEnemy` -> `uEnemy` ->
`uEnemyBase` -> `uEnemyCharacter`), with the same network code. The difference is data in the status block
(`*(monster + 0x1428)`):

| status block | meaning                                                                                                        |
| ------------ | -------------------------------------------------------------------------------------------------------------- |
| +0x40        | position (3 floats)                                                                                            |
| +0x1a8       | flags: 8 = master (its actions are sent), 0x80 = simulated locally (all small monsters, always)                |
| +0x1b6       | u16 network id: large monsters from the quest's set (e.g. 62), small monsters 10000 + a quest-wide spawn index |
| +0x1e4       | HP (float)                                                                                                     |

- Send: `0x2cafe8(monster, kind, data)` (callers check flag 8 first) -> per-kind vtable slot (action:
  +0x42c = `0x2cb330`, fills netEmAction, sends via `0x3eb810`).
- Receive: netEmActionProcedure (`0x3f4554`) -> enemy manager (`sEnemy`, global `0x1a51fe0`) `0x265b14` ->
  find by id among its 10 live enemies (+0x1c..+0x40; small monsters included) -> per-monster receive `0x2cb120`
  (drops own messages, dispatches by kind to the vtable slots +0x430...; action: `0x2cc154` copies the state in).
- Small monsters already _receive_ fine; they never send because nothing sets flag 8 on them, and their 0x80
  makes every game run them itself. Setting 8 on one player's copies and clearing 0x80 on the others' is enough:
  the others' copies then replay the master's actions (tested with poke before writing any code).

Other data used: `sPlayer` (global `0x1a85b58`): +0x1c the 4 player slots; a player's current zone is the byte at
player + 0x1054 (0..12; setter: uPlayer vtable +0xa4; other players' zones arrive by netPlAreaMove). The local
player's slot: `*(0x1a8ca48) + 0x30`. Area 0 = base camp, 255 = not in a quest.

<a id="small-monster-sync-the-ports-part-emsyncc"></a>

### The port's part (emsync.c)

- Each frame (from the game's input polling, on its thread, ~30 times a second): every player's zone, debounced
  (a new value counts after 0.7 s); per zone one player runs the small monsters: the first one there. Claims go to
  the other games over the local play link (`F_AUX` frames, ldn_send_aux) on every change and twice a second.
  Two claims for one zone (arrived together): the lower slot keeps it.
- Master: flags 8 | 0x80 on the zone's small monsters. Others in the zone: 0x80 and 8 cleared. Alone in a zone:
  as the original game (0x80, no 8).
- tools/gen.py PATCHES `0x2cb120` calls `emsync_filter`: drops a small monster message on the master's game, and
  one from a player in another zone.

<a id="small-monster-sync-tests-two-instances-on-one-pc-harvest-tour-jurassic-frontier-zone-1s-three-aptonoth"></a>

### Tests (two instances on one PC, Harvest Tour: Jurassic Frontier, zone 1's three Aptonoth)

- Before: same ids, positions hundreds of units apart on the two games.
- After: within 0-10 units (centimetres) while walking and grazing, for 5+ minutes; the second player's freshly
  spawned copies snap to the master's within ~4 s of arriving. Either player first in the zone works.
- Hand-over: the master leaves -> the other player's game runs them; comes back -> follows.
- Seen once before the debounce: the master role flipped between two players standing still (a passing read of
  the zone, most likely); not seen again in 6 minutes with the debounce.
- Not tested: combat (hits by a follower, deaths, carving), aggressive small monsters (Jaggi...), 3-4 players,
  two PCs.

<a id="small-monster-sync-debug-tools-made-for-this"></a>

### Debug tools made for this

- Control file (MHGU_CONTROL): `peek <hex addr> [words]`, `poke <hex addr> <hex word>`,
  `memsave <hex addr> <hex bytes> <file>`, `fntrace <lo> <hi> [n]` (log calls of guest functions in [lo, hi)),
  stick diagonals `STICK_UPLEFT` etc.
- `MHGU_DUMP_THREADS=<s>`: thread dumps with mutex owners and callers.
