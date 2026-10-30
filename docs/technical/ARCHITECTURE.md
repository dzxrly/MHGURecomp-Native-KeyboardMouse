<h1 align="center">Runtime and native input architecture</h1>

[Developer documentation](README.md) · [Testing and coverage](TESTING.md)

This independently maintained project is based on [morerokk/MHGURecomp](https://github.com/morerokk/MHGURecomp). Its current focus is native keyboard/mouse input for the Switch MHGU 1.4.0 executable. Selected upstream changes are integrated manually.

## Runtime

- The game is AArch32 ARM code, statically translated by `tools/`. Generated code and game files stay local.
- `mhgurecomp` supplies the host runtime; `mhgugame.dll` / `mhgugame.so` contains the locally recompiled game.
- Switch services, threads, input, audio and networking are implemented under `runtime/`. The NVN API is translated to Vulkan.
- The supported title ID is `0100770008DD8000`; the `main` build ID begins with `FB08F1D20FD1204F`. Guest runtime addresses include the main module’s `0x200000` load offset.
- Executable/library names, configuration keys and save paths retain the original names so existing installations and saves continue to work.

## ROM compatibility

Before loading any game module, every launch path validates the full main/sdk NSO Build IDs and nine selected
base-romfs SHA-256 fingerprints against the supported Switch MHGU 1.4.0 reference, plus the merged romfs file-count
check. These are compatibility checks, not a whole-ROM integrity proof. Unsupported, missing or modified references
produce a warning and stop startup before fixed-address hooks. `--check-rom <folder>` runs only these checks.
`--check-gamelib` separately validates the prepared code library's ABI/ID without a ROM. Release builds consume
the checked-in prompt pack and a matching local game library. Releases are built locally for Windows and uploaded
manually. Linux is currently unsupported because the project has no Linux build and test environment.

`exefs` must come from the 1.4.0 update; `romfs` must contain the complete base game with that update applied (approximately 35,500 files and 12.3 GB). The compatibility guard is implemented in `runtime/rom_version.c` with reference data in `runtime/rom_version_data.h`.

## Input flow

`InputMode = Keyboard_Mouse_Input` enables the native input hooks at startup. The source default is `Original`, which retains the original mapping and controller prompts. `KeyboardLayout` is no longer used.

| Component | Responsibility |
| --- | --- |
| `runtime/host_input.c` | SDL events, mouse capture, button-release tracking, preset changes and wheel sequencing |
| `runtime/keyboard_mouse_input.h` | Melee/Ranged keyboard and mouse mappings |
| `runtime/hle/native_mouse.c` | Equipped-weapon detection, direct angles, original action gates and compatibility-camera routing |
| `runtime/hle/keyboard_mouse_prompts.c` | Text selection and cached texture changes for the active preset |
| `tools/build_keyboard_mouse_assets.py` | Local generation of the dual-profile Auto resource pack |

The local character’s actual equipment selects Ranged for Heavy Bowgun, Light Bowgun and Bow, and Melee for other Hunter weapons and Prowler. Equipment previews do not select the active preset. Changes release old held inputs and queued wheel steps.

Relative mouse counts are staged for the local game input update and expire after 100 ms. Menu, focus, capture and action gates discard blocked movement. Ordinary camera input is applied during the game’s manual-camera stage, before recenter, automatic pitch and terrain processing.

Bowgun/Bow aiming changes the player’s authoritative aim before camera synchronization. Original motion checks, aiming-entry delays, per-weapon pitch bounds and projectile setup remain in effect. Bow mouse buttons follow its configured control type and R/ZR swap. Keyboard buttons continue to invoke the original game actions.

Village cameras, fixed Ballista and Prowler special aiming retain the stick path. These use the existing configurable sampler and deadzone compensation; direct-angle paths do not.

`Original` maps WASD to movement, mouse to the camera, left/right mouse to X/A, Space/R to B/Y, Q/E to L/R, Tab/T to ZL/ZR, and Enter/Backspace to Plus/Minus. Esc releases the mouse in that mode. In keyboard/mouse mode, mouse-button releases target the action originally pressed, including across Bow menu/control-state changes.

## Mouse response and compatibility parameters

`[Mouse] MouseSensitivity` (0.1–10) multiplies the base sensitivity. Direct input uses relative mouse counts rather than screen pixels; physical response depends on mouse DPI. No additional acceleration curve is applied. In-game Camera Speed and Reticle Speed apply independently and immediately:

| In-game option | Direct path | Slow | Default | Fast |
| --- | --- | --- | --- | --- |
| Camera Speed | Ordinary quest camera | 0.5× | 1× | 1.5× |
| Reticle Speed | Bowgun/Bow aim and Binoculars | 0.5× | 1× | 1.5× |

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

## Automatic prompts

The Auto pack contains both text profiles without changing the original message count or label indices. A message getter selects the appended Melee strings when appropriate. Known controller atlases carry an exact Ranged baseline and Melee block differences. The Vulkan upload selects the active variant and refreshes cached textures on preset changes.

The explicitly requested prebuilt seven-language Auto pack is maintained under `extras/keyboard-mouse/prebuilt/` and distributed with Windows releases. Raw exefs/romfs stay outside Git and remain read-only; compatibility changes belong in project code or mod overlays. Existing single-profile packs are skipped; an explicitly disabled Auto mod remains disabled.

Existing checks, recorded play-test coverage and outstanding verification are maintained in [TESTING.md](TESTING.md).

## Mods and networking

The mod loader supports RomFS replacements, merged ARC entries and Forge plugins. Later conflicting entries win. Insert opens Forge; unsupported code patches are logged. Mods apply as overlays, leaving the original game files unchanged. Optional cheat implementations are documented in the [skill-point](research/skill-points.md) and [Valor Sheathe](research/valor-sheathe.md) notes.

For direct multiplayer without a relay, the host's TCP/UDP port (default 39500) must be reachable and clients set `[Multiplayer] Join` to the host address. A hub server instead uses `[Multiplayer] Server`, with a shared `Key` to restrict discovery. Server build, deployment and protocol details are in [server/README.md](../../server/README.md).
