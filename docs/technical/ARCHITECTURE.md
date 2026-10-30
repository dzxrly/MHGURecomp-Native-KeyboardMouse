<h1 align="center">Runtime and native input architecture</h1>

[English](../../README.md) · [简体中文](../README.zh-CN.md) · [繁體中文](../README.zh-TW.md)

This independently maintained project is based on [morerokk/MHGURecomp](https://github.com/morerokk/MHGURecomp). Its current focus is native keyboard/mouse input for the Switch MHGU 1.4.0 executable. Selected upstream changes are integrated manually.

## Runtime

- The game is AArch32 ARM code, statically translated by `tools/`. Generated code and game files stay local.
- `mhgurecomp` supplies the host runtime; `mhgugame.dll` / `mhgugame.so` contains the locally recompiled game.
- Switch services, threads, input, audio and networking are implemented under `runtime/`. The NVN API is translated to Vulkan.
- The supported `main` build ID begins with `FB08F1D20FD1204F`. Guest runtime addresses include the main module’s `0x200000` load offset.
- Executable/library names, configuration keys and save paths retain the original names so existing installations and saves continue to work.

## Input flow

`InputMode = Keyboard_Mouse_Input` enables the native input hooks at startup. `Original` retains the original mapping and controller prompts.

Before loading any game module, every launch path validates the full main/sdk NSO Build IDs and nine selected
base-romfs SHA-256 fingerprints against the supported Switch MHGU 1.4.0 reference, plus the merged romfs file-count
check. These are compatibility checks, not a whole-ROM integrity proof. Unsupported, missing or modified references
produce a warning and stop startup before fixed-address hooks. `--check-rom <folder>` runs only these checks.
`--check-gamelib` separately validates the prepared code library's ABI/ID without a ROM. Release builds consume
the checked-in prompt pack and a size/checksum-verified library from the pinned upstream release; they do not
need access to the dump or a self-hosted runner.

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

## Automatic prompts

The Auto pack contains both text profiles without changing the original message count or label indices. A message getter selects the appended Melee strings when appropriate. Known controller atlases carry an exact Ranged baseline and Melee block differences. The Vulkan upload selects the active variant and refreshes cached textures on preset changes.

The seven-language pack is generated from the user’s own unmodified game resources. Generated ARC files and game textures must not be committed. Existing single-profile packs are skipped; an explicitly disabled Auto mod remains disabled.

## Verification boundaries

The README lists the current exercised actions and remaining work. A shared preset provides bindings for a weapon; it does not establish that every move, style, ammo type or multiplayer context has been tested.

The current native-angle play tests were performed on Windows at 60 FPS. Existing sampler tests cover other frame and mouse-report rates only for the compatibility conversion. The FSR and multiplayer notes retain their own narrower validation context.

Use isolated configuration and save directories for runtime experiments. Do not write experimental equipment changes back to the player’s real save.

## Development checks

   ```sh
   make test-settings test-keyboard-mouse test-settings-locale test-keyboard-mouse-assets
   ```

`MHGU_NATIVE_MOUSE_LOG=1` enables accepted-motion and projectile-direction traces. The existing control file’s `stopinput` command clears scripted inputs and held test state. Normal startup does not hold aim automatically.
