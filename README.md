<h1 align="center">MHGURecomp Native Keyboard &amp; Mouse</h1>

<p align="center">
  <strong>English</strong> · <a href="docs/README.zh-CN.md">简体中文</a> · <a href="docs/README.zh-TW.md">繁體中文</a>
</p>

A Windows PC port of **Monster Hunter Generations Ultimate**, with native keyboard and mouse controls for the Nintendo Switch **1.4.0** version.

Based on [morerokk/MHGURecomp](https://github.com/morerokk/MHGURecomp), this project is maintained independently with a focus on keyboard and mouse support.

**You need your own game files to play.** Release archives include the keyboard/mouse prompt mod.

## Features

- Mouse camera control and bowgun/Bow aiming, with automatic Melee/Ranged bindings based on your equipment.
- Keyboard/mouse prompts in English, Simplified Chinese, Traditional Chinese, French, German, Italian and Spanish.
- Resolution, aspect ratio, anti-aliasing, FSR, VSync and 30–240 FPS settings, plus controller support.
- Local-play multiplayer, optional monster HP scaling and Village quests in multiplayer, mods and cheats.

## Requirements

- Windows 10/11 64-bit. **Linux is currently unsupported because this project has no Linux build and test environment; no Linux release is provided.**
- AVX2-capable CPU, Vulkan 1.2-capable GPU, at least 1 GB VRAM and 4 GB RAM.
- Your own complete **Switch MHGU 1.4.0** game files, with the update merged into the base game. The 3DS version is not supported.

Startup checks game compatibility and stops with a warning if the files do not match. Keep the original `exefs`/`romfs` unchanged; put mods in `mods/`.

## Installation

Download the Windows archive from [this repository’s Releases page](https://github.com/dzxrly/MHGURecomp-Native-KeyboardMouse/releases). Original-project binaries do not include these keyboard/mouse changes.

1. Extract the program into a folder of your choice.
2. Put your game’s `exefs` and `romfs` folders under `data`:

   ```text
   MHGURecomp/
     mhgurecomp.exe
     mhgugame.dll
     data/
       exefs/
       romfs/
   ```

3. Start `mhgurecomp.exe`. Its first start creates `mhgurecomp.ini`.
4. Enable keyboard/mouse mode and the bundled prompt mod as described below.

Copy both complete folders, keeping all other files from the release archive beside the program.

## Keyboard and mouse

### Enable and configure

Close the program, set these values in `mhgurecomp.ini`, then start it again:

```ini
[Controls]
InputMode = Keyboard_Mouse_Input

[Mouse]
MouseSensitivity = 1.0
```

The default mode is `Original`; use `Keyboard_Mouse_Input` for the controls below. Controllers work in either mode. Light Bowgun, Heavy Bowgun and Bow automatically use **Ranged**; other Hunter weapons and Prowler use **Melee**. Equipment changes switch bindings and prompts without restarting.

### Enable keyboard/mouse prompts

The release includes `mods/Keyboard_Mouse_Input_Auto`. Enable it in **F1 → Mods**, then restart. It follows your equipment and game language automatically. Keep the whole folder; no separate download or generation is needed.

### Bindings

| Input                      | Melee                                                 | Ranged                                                |
| -------------------------- | ----------------------------------------------------- | ----------------------------------------------------- |
| WASD                       | Left stick: move                                      | Left stick: move                                      |
| Mouse movement             | Quest camera                                          | Quest camera / weapon aim                             |
| Left mouse                 | X: primary attack / draw                              | A: fire, confirm / interact                           |
| Right mouse                | A: secondary attack, confirm / interact               | R: aim                                                |
| R                          | R: guard / weapon special action                      | X: reload / draw                                      |
| Mouse side button 5        | R: guard / weapon special action                      | Unbound                                               |
| F / Mouse side button 4    | Y: sheathe / use item                                 | Y: sheathe / use item                                 |
| E                          | Y: sheathe / use item                                 | R: aim                                                |
| Left Shift                 | R: ordinary sprint                                    | R: ordinary sprint                                    |
| Space                      | B: evade, cancel / back                               | B: evade, cancel / back                               |
| C                          | X+A: combined attack                                  | X+A: combined action                                  |
| Wheel up / down            | L+Y / L+A: select item                                | L+X / L+B: select ammo or coating                     |
| Q                          | L: item bar / camera operation                        | L: item bar / camera operation                        |
| Esc / Enter / Numpad Enter | Plus: game menu                                       | Plus: game menu                                       |
| Backspace                  | Minus: secondary menu                                 | Minus: secondary menu                                 |
| Tab / Middle mouse         | ZL: switch target (control type 1)                    | ZL: switch target (control type 1)                    |
| T                          | ZR                                                    | ZR                                                    |
| 1 / 2 / 3                  | D-pad up / right / down: Hunter Arts (control type 1) | D-pad up / right / down: Hunter Arts (control type 1) |
| Arrow keys                 | D-pad                                                 | D-pad                                                 |
| Ctrl / Alt                 | Left / right stick click                              | Left / right stick click                              |
| F1 / F2                    | PC settings / release mouse                           | PC settings / release mouse                           |

Bindings use physical QWERTY key positions. In game menus, confirm with right mouse in Melee or left mouse in Ranged; Space goes back. Original game menus use button navigation rather than pointer clicks. Press F2 to release the mouse; click the window to capture it again.

Press `/` to chat in town or quests, Enter to send and Esc to cancel. Change the chat key in **F1 → Controls**. During text entry, Ctrl+V pastes.

### Bow and Prowler

Bow uses Ranged. With the Bow sheathed, left mouse remains A for interaction; press R (game X) to draw. While drawn:

| Input               | Bow control type 1             | Bow control type 2                          |
| ------------------- | ------------------------------ | ------------------------------------------- |
| Left mouse          | X: charge; release to shoot    | Configured game R: charge; release to shoot |
| Right mouse         | Configured game R: hold to aim | X: click to toggle aim                      |
| Mouse side button 5 | A: secondary Bow action        | A: secondary Bow action                     |

Mouse controls follow the game’s Bow control type and R/ZR swap option. Keyboard controls stay as in the main table; menus use left mouse to confirm. Bow prompts still show the base Ranged layout, so use this table for Bow mouse controls.

Prowler uses Melee: left mouse attacks and right mouse charges a boomerang. While charging, hold R / side button 5 for precision aim. With the weapon out, R guards and R+left mouse performs the overhead attack; with it put away, R+C guards. The wheel selects Support Moves; F uses one while sheathed, R+F while drawn. Original move requirements and gauge costs apply.

### Mouse sensitivity and camera options

Adjust **Mouse sensitivity** in F1, or `MouseSensitivity` in the ini (0.1–10). In the game’s own options, **Camera Speed** adjusts the quest camera and **Reticle Speed** adjusts bowgun/Bow aim and Binoculars; both apply immediately. Camera, Quick Aim and Scope inversion are configured separately in the game.

Original firing recovery, aiming delays and attack-turning restrictions still apply. Village cameras, fixed Ballista and Prowler special aiming retain the game’s turn-speed limits. Some handbook pages still show controller illustrations.

## Settings and files

F1 opens PC settings; Alt+Enter or F11 toggles fullscreen, F12 takes a screenshot and F7 cycles MSAA. The PC settings interface currently supports English/Spanish, independently of the game’s seven languages.

Use F1 to adjust graphics, audio, controls and multiplayer. Choose **Save** to keep changes; settings marked with an asterisk require a restart. The ini contains additional explanations. Enabling FSR takes over anti-aliasing.

**Smooth gradients** reduces colour banding in skies and glows. It is off by default, takes effect after saving without a restart, and increases GPU work.

| Content                    | Location                                           |
| -------------------------- | -------------------------------------------------- |
| Settings / log             | `mhgurecomp.ini` / `log.txt` beside the executable |
| Windows saves              | `%APPDATA%\MHGURecomp\save\save_0\`                |
| Mods / cheats              | `mods/` / `cheats/` beside the executable          |
| Screenshots / shader cache | `shots/` / `cache/` beside the executable          |

Import your own Switch save by copying `system` and `system_backup` into `save_0`. Back up existing saves first.

## Multiplayer, mods and cheats

Choose the game’s **Local Wireless** options. LAN/VLAN players can discover each other directly. For internet play, use compatible program versions and the same hub server in **F1 → Network**; use the same matchmaking key for a private group. A working hub server is required; this project does not guarantee the original public service’s availability.

Put each mod in its own `mods/` subfolder. Enable and order mods in **F1 → Mods**, then restart. Later conflicting mods take priority. Resolution/FPS features already provided by the program do not need separate fix mods.

Put version-1.4.0 cheat `.txt` files in `cheats/`; prefix a filename with `_` to disable it. Restart after changes.

If a release supplies `bonus/system` and `BonusContent = On`, free event quests/item packs/Palicoes can be imported when absent. Set it Off to disable this behavior.

## Updates and troubleshooting

Download updates from **this repository**. Replace program files and the bundled Auto prompt mod with the new release, keeping your game data, settings, other mods and saves. The included updater also checks this repository.

- **Game-version warning:** re-export the complete Switch 1.4.0 game with its update merged. Keep the original files unchanged and install modifications as mods.
- **Controller prompts remain:** check `InputMode`, enable the bundled Auto mod in F1 and restart. Bow-specific prompts and some handbook illustrations are still incomplete.
- **Problems at high FPS:** try 60 FPS with VSync enabled.
- **Other problems:** report your program version, hardware, reproduction steps and `log.txt` in [Issues](https://github.com/dzxrly/MHGURecomp-Native-KeyboardMouse/issues). Copy the log before starting the game again.

[Developer documentation](docs/DEVELOPMENT.md#development) covers source builds, releases, implementation and testing. It is separate from this user guide.

## License

Project code is MIT licensed. Original-project attribution and third-party notices are retained; see [LICENSE](LICENSE) and [THIRD_PARTY_NOTICES](THIRD_PARTY_NOTICES). These licenses do not cover the game or generated game code. This project is not affiliated with Capcom or Nintendo.
