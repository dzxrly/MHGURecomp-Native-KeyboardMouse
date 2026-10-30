<h1 align="center">MHGURecomp Native Keyboard &amp; Mouse</h1>

<p align="center">
  <strong>English</strong> · <a href="docs/README.zh-CN.md">简体中文</a> · <a href="docs/README.zh-TW.md">繁體中文</a>
</p>

A native PC port of **Monster Hunter Generations Ultimate**, targeting the Nintendo Switch **1.4.0** executable. The game’s ARM code is statically recompiled to C; the host runtime implements the Switch services and renders through Vulkan.

This repository was forked from [morerokk/MHGURecomp](https://github.com/morerokk/MHGURecomp). It is developed independently with a focus on **native keyboard and mouse input**, and periodically incorporates selected upstream changes through manual synchronization.

**No raw game dump, generated game source or keys are included.** The repository includes the prebuilt PC prompt mod. Supply your own game dump to play.

## Features

- Direct mouse angles for the quest camera, bowgun/Bow aiming and Binoculars, with fine movement preserved.
- Automatic Melee/Ranged bindings and matching text/icons based on the equipped weapon. Prowler uses Melee.
- In-game Camera Speed and Reticle Speed options adjust the corresponding mouse sensitivity immediately.
- Original firing recovery, aiming delays, turning windows, pitch limits and camera behavior remain in effect.
- PC prompts in all seven game languages: English, Simplified Chinese, Traditional Chinese, French, German, Italian and Spanish.
- Resolution, aspect ratio, MSAA/FXAA, FSR, VSync and 30–240 FPS settings; controller support and an F1 settings panel.
- Local-play networking, optional monster HP scaling and Village quests in multiplayer, mods and cheat support inherited from the original project.

The native mouse implementation targets the supported Switch executable. Compatibility with other game builds or the 3DS version is outside this project’s current scope.

## Requirements

Every startup path checks the full main/sdk Build IDs and selected base-romfs fingerprints against the supported
Switch MHGU 1.4.0 reference before loading game code. A mismatch identifies the affected file and stops startup.
These fingerprints are not a whole-ROM integrity check. Keep the original merged romfs intact and put modifications
in `mods/`. `mhgurecomp --check-rom <game-folder>` checks without launching the game.

The `/` key opens keyboard chat in town and quests; Enter sends and Esc cancels. Change `Controls.ChatKey` in F1
settings; occupied keyboard/mouse keys are marked. `Anti-aliasing.FXAAMode = 3_11` selects FXAA 3.11.

- Windows 10/11 64-bit. **Linux is incompatible with this project.**
- AVX2-capable CPU, Vulkan 1.2-capable GPU, at least 1 GB VRAM and 4 GB RAM.
- Your own complete game files with the **1.4.0 update merged into the base game**.
- Title ID: `0100770008DD8000`; the `main` executable’s build ID begins with `FB08F1D20FD1204F`.

The project targets Windows only. Its native input play tests are performed on Windows.

## Installation

Use binaries published on [this repository’s Releases page](https://github.com/dzxrly/MHGURecomp-Native-KeyboardMouse/releases). If no package is available, [build from source](#building-from-source). Original-project binaries do not include this repository’s native input changes.

1. Extract the program into a folder of your choice.
2. Put your game’s `exefs` and `romfs` folders under `data`:

   ```text
   MHGURecomp/
     mhgurecomp.exe
     mhgugame.dll
     (other runtime libraries)
     data/
       exefs/
         main
         main.npdm
         rtld
         sdk
         subsdk0
       romfs/
         nativeNX/
         ...
   ```

3. Start `mhgurecomp.exe`. Its first start creates `mhgurecomp.ini`.
4. Enable the keyboard/mouse mode and install its prompt pack as described below.

`exefs` must come from the update. `romfs` must contain the base game with the update applied, approximately 35,500 files and 12.3 GB. Missing, incomplete or mismatched files cause a startup error.

## Keyboard and mouse

### Enable and configure

Set these values in `mhgurecomp.ini` and restart once:

   ```ini
   [Controls]
   InputMode = Keyboard_Mouse_Input

   [Mouse]
   MouseSensitivity = 1.0
   ```

The source default is still `InputMode = Original`. Controllers work in either mode. In keyboard/mouse mode, Light Bowgun, Heavy Bowgun and Bow automatically use **Ranged**; other Hunter weapons and Prowler use **Melee**. Changing equipment switches bindings and prompts without a restart. The old `KeyboardLayout` setting is no longer used.

### Install automatic prompts

Release archives include `mods/Keyboard_Mouse_Input_Auto`. Keep that directory in `mods/`, enable it in the F1 **Mods** tab and restart. No separate download or resource-generation step is needed.

   ```text
   mods/
     Keyboard_Mouse_Input_Auto/
       romfs/
         keyboard_mouse_input.flag
         keyboard_mouse_prompts.bin
         nativeNX/
   ```

Keep both marker/data files. One Auto pack contains both presets; old single-preset Melee/Ranged packs are skipped. Original mode skips the Auto pack. Text and icons follow the equipped weapon and game language; a pack disabled in Mods remains disabled.

### Bindings

| Input | Melee | Ranged |
| --- | --- | --- |
| WASD | Left stick: move | Left stick: move |
| Mouse movement | Quest camera | Quest camera / weapon aim |
| Left mouse | X: primary attack / draw | A: fire, confirm / interact |
| Right mouse | A: secondary attack, confirm / interact | R: aim |
| R | R: guard / weapon special action | X: reload / draw |
| Mouse side button 5 | R: guard / weapon special action | Unbound |
| F / Mouse side button 4 | Y: sheathe / use item | Y: sheathe / use item |
| E | Y: sheathe / use item | R: aim |
| Left Shift | R: ordinary sprint | R: ordinary sprint |
| Space | B: evade, cancel / back | B: evade, cancel / back |
| C | X+A: combined attack | X+A: combined action |
| Wheel up / down | L+Y / L+A: select item | L+X / L+B: select ammo or coating |
| Q | L: item bar / camera operation | L: item bar / camera operation |
| Esc / Enter / Numpad Enter | Plus: game menu | Plus: game menu |
| Backspace | Minus: secondary menu | Minus: secondary menu |
| Tab / Middle mouse | ZL: switch target (control type 1) | ZL: switch target (control type 1) |
| T | ZR | ZR |
| 1 / 2 / 3 | D-pad up / right / down: Hunter Arts (control type 1) | D-pad up / right / down: Hunter Arts (control type 1) |
| Arrow keys | D-pad | D-pad |
| Ctrl / Alt | Left / right stick click | Left / right stick click |
| F1 / F2 | PC settings / release mouse | PC settings / release mouse |

Bindings use physical QWERTY key positions. Click the window to capture the mouse again. During text entry, Enter confirms, Esc cancels and Ctrl+V pastes. Menu confirmation is right mouse in Melee and left mouse in Ranged; Space goes back. Switching presets clears held inputs and queued wheel steps.

### Bow and Prowler

Bow uses Ranged. With the Bow sheathed, left mouse remains A for interaction; press R (game X) to draw. While drawn:

| Input | Bow control type 1 | Bow control type 2 |
| --- | --- | --- |
| Left mouse | X: charge; release to shoot | Configured game R: charge; release to shoot |
| Right mouse | Configured game R: hold to aim | X: click to toggle aim |
| Mouse side button 5 | A: secondary Bow action | A: secondary Bow action |

The game’s R/ZR swap option participates in these mouse bindings. Keyboard bindings stay as in the main table. Opening a menu restores left mouse to A; releasing a mouse button releases the action it originally pressed. Bow control types currently share the base Ranged prompts, so use this table for their mouse overrides.

Prowler uses Melee: left mouse for X melee, right mouse for A boomerang. While charging a boomerang with right mouse, hold R / side button 5 for precision aim. Game R guards with the weapon out; R+X performs the overhead attack. With the weapon put away, R+X+A guards. The wheel selects Support Moves; F uses one while sheathed, R+F while drawn. Original eligibility, gauge costs and action restrictions apply.

### Mouse sensitivity and camera options

`MouseSensitivity` accepts 0.1–10 and multiplies the base sensitivity. Direct input uses relative mouse counts, not screen pixels; your mouse DPI affects the counts produced by a physical movement. No additional mouse acceleration curve is applied.

| In-game option | Affected direct input | Slow | Default | Fast |
| --- | --- | --- | --- | --- |
| Camera Speed | Ordinary quest camera | 0.5× | 1× | 1.5× |
| Reticle Speed | Bowgun/Bow aim and Binoculars | 0.5× | 1× | 1.5× |

Both options apply immediately and independently. At `MouseSensitivity = 1`, 1,000 horizontal counts correspond to approximately 20° / 40° / 60°. Camera, Quick Aim and Scope inversion settings remain separate. The five Quick Aim Camera types, initial camera position, target-camera options and terrain adjustment retain their original behavior.

Mouse movement blocked by an action is discarded; it is not replayed when control returns. Turning the camera does not force a melee attack to turn outside the game’s allowed window.

Village cameras, fixed Ballista and Prowler special aiming retain mouse-to-stick compatibility. Only those paths use `[MouseConversion]`:

   ```ini
   [MouseConversion]
   MinWindowMs = 8.0
   FullStickSpeed = 1500.0
   DeadzoneCompensation = 0.40
   AimDeadzoneCompensation = 0.50
   LowSpeedCutoff = 0.0
   FrameTimeSmoothing = 0.25
   ```

Restart after changing these parameters. `FullStickSpeed` is the mouse speed in counts/second that produces full stick at sensitivity 1. The two compensation values offset ordinary/aiming deadzones; `MinWindowMs` controls the minimum sampling window; `LowSpeedCutoff` filters very slow motion; `FrameTimeSmoothing` adjusts the frame-time estimate. These cameras keep the game’s stick speed limits.

### Coverage and remaining work

In-game checks cover bowgun/Bow aim and projectile direction, both Bow control types, camera options, automatic equipment switching, Prowler boomerang aim/guard, Insect Glaive aim/Kinsect launch, and Ballista compatibility aiming. Long Sword movement/draw direction, Lance guard/thrust and Charge Blade guard have also been checked; this is not an exhaustive weapon/style/action matrix.

Remaining work includes Bow-specific prompts, actual Prowler Support Move activation with an equipped move, variable zoom, and wider weapon/style/ammo and frame-rate testing. Multi-FPS mouse sampler tests are not a substitute for native-angle play tests. Full-page controller illustrations in the handbook are still present.

`InputMode = Original` retains the original mapping: WASD move, mouse camera, left/right mouse X/A, Space/R B/Y, Q/E L/R, Tab/T ZL/ZR, Enter/Backspace Plus/Minus. In that mode Esc releases the mouse.

## Settings and files

F1 opens PC settings; Alt+Enter or F11 toggles fullscreen, F12 takes a screenshot and F7 cycles MSAA. The PC settings interface currently supports English/Spanish, independently of the game’s seven languages.

`mhgurecomp.ini` explains every setting. Main sections cover display/render resolution, MSAA/FXAA, FSR 3.1/4 and quality, draw distance, FPS/VSync, controls, mouse, networking, monster HP, Village quests, bonus content and audio. FSR replaces MSAA/FXAA while enabled. See [settings details](docs/technical/SETTINGS_UI.md), [FSR 3.1](docs/technical/FSR-PORT.md) and [FSR 4](docs/technical/FSR4-PORT.md).

| Content | Location |
| --- | --- |
| Settings / log | `mhgurecomp.ini` / `log.txt` beside the executable |
| Windows saves | `%APPDATA%\MHGURecomp\save\save_0\` |
| Mods / cheats | `mods/` / `cheats/` beside the executable |
| Screenshots / shader cache | `shots/` / `cache/` beside the executable |

Import your own Switch save by copying `system` and `system_backup` into `save_0`. Back up existing saves first. Repository renaming does not change executable names or save locations.

## Multiplayer, mods and cheats

Choose the game’s **Local Wireless** options. LAN/VLAN discovery works directly. For internet play, all players need a compatible build; set `[Multiplayer] Server` to a hub server and share a `Key` to restrict discovery to friends. Original public-server availability and cross-build interoperability are not guaranteed here. Self-hosting instructions are in [server/README.md](server/README.md). Direct connections without a relay require the host’s TCP/UDP port (default 39500) to be reachable and the client’s `Join` address configured.

Put each mod in its own `mods/` subfolder. F1 → Mods enables/disables mods and sets load order; restart to apply. RomFS replacements, merged ARC entries and Forge plugins are supported. Later conflicting entries win. Insert opens Forge; unsupported code patches are logged. Resolution/FPS features already provided by the runtime do not need separate fix mods.

Put version-1.4.0 cheat `.txt` files in `cheats/`; prefix a filename with `_` to disable it. Enabled files load on startup, with each cheat headed by `[Name]`. See [skill-point notes](docs/technical/research/skill-points.md) and [Valor Sheathe notes](docs/technical/research/valor-sheathe.md) for the optional files in `extras/cheats/`.

If a release supplies `bonus/system` and `BonusContent = On`, free event quests/item packs/Palicoes can be imported when absent. Set it Off to disable this behavior.

## Updates and upstream synchronization

Download releases from **this repository** and replace program files while keeping your game data, settings, mods and saves. The updater built here checks this repository’s releases. It does not install original-project releases. Without a published release or a working connection it starts the installed game. Development versions such as `0.11.2-13-g0641c27` are not automatically replaced by tagged releases.

This project periodically reviews and manually integrates selected changes from [the original repository](https://github.com/morerokk/MHGURecomp). Synchronization is selective; features and release numbers may differ. The local `upstream` remote can stay pointed at the original repository for that work.

## Building from source

Clone this repository, install the required toolchain and Python dependencies, then build against your own game files:

   ```sh
   git clone https://github.com/dzxrly/MHGURecomp-Native-KeyboardMouse.git
   cd MHGURecomp-Native-KeyboardMouse
   make gen GAME=/path/to/game
   make -j4 GAME=/path/to/game
   ```

The complete [Windows build and release guide](docs/technical/BUILDING.md) covers MSYS2 packages, Python, FFmpeg and output folders. Binaries keep the names `mhgurecomp` and `mhgugame`.

Existing input/configuration checks:

   ```sh
   make test-settings test-keyboard-mouse test-settings-locale test-keyboard-mouse-assets
   ```

## Source layout and license

| Directory | Purpose |
| --- | --- |
| `runtime/` | Host runtime, direct mouse input, automatic presets, graphics, networking and settings |
| `tools/` | Recompiler and local prompt-pack generator |
| `updater/` / `server/` | This repository’s updater / optional hub server |
| `docs/` | Translated READMEs; implementation/build notes under `technical/` |
| `extras/` | PC prompt artwork, resource tools documentation and optional cheats |

Project code is MIT licensed. Original-project attribution and third-party notices are retained; see [LICENSE](LICENSE) and [THIRD_PARTY_NOTICES](THIRD_PARTY_NOTICES). These licenses do not cover the game or generated game code. This project is not affiliated with Capcom or Nintendo.
