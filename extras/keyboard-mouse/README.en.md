# Keyboard_Mouse_Input: Melee and Ranged

> `InputMode` selects original input or `Keyboard_Mouse_Input`. `KeyboardLayout` selects `Melee` or `Ranged` bindings and their matching prompt resources. “Game button” means a virtual Switch input sent to MHGU. LMB/RMB/MMB mean left/right/middle mouse button. Based on MHGURecomp 0.10.8.

[完整中文说明](README.md)

Set these in `mhgurecomp.ini`, then exit the game completely and restart:

```ini
[Controls]
InputMode = Keyboard_Mouse_Input
KeyboardLayout = Ranged
```

`InputMode = Original` (default) keeps upstream keyboard bindings, controller prompts and mouse conversion. `Keyboard_Mouse_Input` enables the selected modern layout, its PC prompts and the revised mouse conversion. `KeyboardLayout` accepts `Melee` or `Ranged` (default). It is a manual selection, not automatic weapon detection. Controller input remains available in both modes.

These replace the previous configuration fields. Move old prompt mods out of `mods` before installing the new packs.

## Mapping

| Input | Melee | Ranged |
| --- | --- | --- |
| WASD | Left stick: movement | Same |
| Mouse motion | Right stick: camera | Right stick: camera/aim |
| LMB | X: primary attack/draw | A: fire, confirm/interact |
| RMB | A: secondary attack, confirm/interact | R: aim |
| R | R: guard/weapon special action | X: reload/draw |
| Mouse5 | R: guard/weapon special action | Unbound, as before |
| F / Mouse4 | Y: sheathe/use item | Same |
| E | Y: sheathe/use item alias | R: previous binding retained |
| Left Shift | R: ordinary sprint | Same |
| Space | B: evade, cancel/menu back | Same |
| C | X+A: combined attack | X+A: siege mode/combined action |
| Wheel up/down | L+Y / L+A: select items | L+X / L+B: select ammo/coatings |
| Q | L: item selector/camera functions | Same |
| Esc / Enter / keypad Enter | Plus: game menu | Same |
| Backspace | Minus: original menu | Same |
| Tab / MMB | ZL: target selection with control type 1 | Same |
| T | ZR | Same |
| 1 / 2 / 3 | D-pad up/right/down: Arts with control type 1 | Same |
| Arrow keys | D-pad | Same |
| Ctrl / Alt | Left/right stick click | Same |
| F1 / F2 | Host settings / release mouse capture | Same |

The Ranged fire/reload descriptions use Light/Heavy Bowgun actions. MHGU Bow has separate in-game control types: type 1 draws with X and aims with R (keyboard R and RMB here); type 2 swaps X/R. This fixed layout does not automatically detect Bow, so the bowgun LMB-fire description does not apply to it.

The layouts borrow primary/secondary mouse attacks, RMB aiming, Shift sprinting, Space evasion and R reload/guard conventions from World/Iceborne and Wilds. Ranged preserves the previously used F sheathe and C combined input. Existing PC icon art is reused.

MHGU shares controller buttons between combat and menus: Melee confirms/interacts with RMB, Ranged with LMB, and both cancel with Space. There is no contextual remapping of every menu. These are not exact replicas of the later games: no slinger, focus mode, automatic sprint-sheathe or other new actions are added. Start with game control type 1 when checking these bindings.

Shift sends only R, never B or an automatic Power Run sequence. With a weapon drawn, the game may interpret R as guard or aim. Combat timing and style restrictions stay in the original game. During text entry, Enter confirms and Escape cancels. Click the window to recapture the mouse after F2.

The wheel queues L down → L+selection button → selection released → L released, at least 40 ms per phase. Focus loss and text-entry/host-menu transitions clear the queue. It is a fixed controller sequence, not comprehensive menu detection. Bindings use physical QWERTY scancodes, including on AZERTY/QWERTZ keyboards.

## Generate and install prompts

Use Python 3.10+ and Pillow 12.2+, running from the repository root. Substitute your own unmodified game's `romfs` and two empty output directories outside it:

```sh
python -m pip install "Pillow>=12.2"
python tools/build_keyboard_mouse_assets.py /path/to/game/romfs /path/to/mods/Keyboard_Mouse_Input_Melee --layout Melee
python tools/build_keyboard_mouse_assets.py /path/to/game/romfs /path/to/mods/Keyboard_Mouse_Input_Ranged --layout Ranged
```

Put both generated directories under the game's `mods`. Keep each `romfs/keyboard_mouse_input.flag`, containing `Melee` or `Ranged`. The loader selects only the pack matching `InputMode` and `KeyboardLayout`, even if its directory is renamed or nested. Enable both packs in the Mods list to allow selection through the INI; configuration does not override `load_order.txt`. Original mode skips both packs. Other mods are independent. Keep unrelated features out of these dedicated layers.

Both packs support all seven shipped game languages: Simplified Chinese, Traditional Chinese, English, French, German, Italian and Spanish. They adapt explicit button/stick text, movement/item/ammo tips and the shared font, title, paging, HUD, target selector, Art directions and lobby icons. Melee X/A/R prompts show LMB/RMB/R; Ranged shows R/LMB/RMB. Static icons show primary bindings: sprint text says Shift, while the shared R icon shows R or RMB. D-pad icons show arrows; 1/2/3 remain aliases. Full-page illustrated manual diagrams have not been redrawn. The upstream F1 host panel keeps English/Spanish.

GMD entry identifiers, order and formatting tags are preserved; archive-local variants use their own original text. Only known input BC3 blocks are re-encoded, and untouched archive payloads remain exact. Each pack includes a local `manifest.json` and decoded previews. The repository contains code, tools, layout facts and PC icon art; game assets, generated mods, previews and saves remain local. Icon generation prompts are in `imagegen-prompts.txt`; added code and icon materials use the upstream MIT license.

## Mouse conversion and verification

Keyboard_Mouse_Input replaces the old 25 ms exponential speed filter with a finite mouse-displacement window. The window is at least 8 ms and spans whole frame intervals: approximately 33.3/16.7/8.3/8.3 ms at stable 30/60/120/240 FPS, growing if actual presentation slows. The low-speed cutoff drops from 30 to 3 counts/s at sensitivity 1. Focus loss, capture release and text/host-menu transitions clear the history.

Radial direction, dead-zone compensation and maximum stick deflection remain. `MouseSensitivity` still applies. This is still stick emulation with the game's maximum turn speed, not direct mouse-angle control. Input falls to zero when its finite window expires; visible latency also includes game/display timing. Original mode keeps the upstream conversion.

The atlas regression needs Pillow and uses a synthetic empty texture plus our PC art, with no game files.

Run `make test-settings test-keyboard-mouse test-settings-locale test-keyboard-mouse-assets` in the normal build environment. Checks cover both layouts, item/ammo wheel transitions at 30–240 FPS, seven text languages, persistence/restart behavior and mouse resampling at 125–8000 Hz with 30–240 FPS sampling, including count conservation, stopping, direction and reset. Input-count tests do not measure actual camera-angle error or player-perceived aiming quality.

References: [Capcom Generations Great Sword](https://game.capcom.com/manual/MH_Gen/en/page-101.html), [Capcom Generations Heavy Bowgun](https://game.capcom.com/manual/MH_Gen/en/page-122.html), plus the local MHGU 1.4.0 Hunter's Notes for game/style-specific behavior. [World/Iceborne PC controls and game screenshots](https://www.shacknews.com/article/106612/pc-keyboard-controls-and-key-bindings-monster-hunter-world) and [Wilds controls and gameplay screenshots](https://dotesports.com/monster-hunter/news/all-controls-and-keybinds-in-monster-hunter-wilds-pc-and-console) inform the ergonomic comparison; these layouts do not claim to reproduce every default binding of those games.
