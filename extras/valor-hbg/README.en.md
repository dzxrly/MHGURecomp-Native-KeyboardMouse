# Valor HBG keyboard/mouse preset and PC prompts

> `ValorHBG` is an opt-in, fixed keyboard/mouse preset based on MHGURecomp 0.10.8. `Original` keeps the upstream bindings. “Game button” means the virtual Switch input sent to the original game. LMB, RMB and MMB mean the left, right and middle mouse buttons.

[中文完整说明](README.md)

Build this fork following the repository's main README. In `mhgurecomp.ini`, set:

```ini
[Controls]
KeyboardPreset = ValorHBG
KeyboardMouseIcons = On
```

Restart the game after changing the preset. It is currently selected through the INI. Generating the optional asset mod alone does not add the preset to an upstream executable.

`KeyboardMouseIcons = On/Off` independently controls the PC icon and matching instruction resources, **at the next game start**. Its default is `Off`, keeping controller prompts. It does not change the input bindings. The current prompt pack describes `ValorHBG`; set this switch to `Off` when using `Original`.

| PC input | Game button | Use with Valor heavy bowgun / menus |
| --- | --- | --- |
| WASD | Left stick | Move |
| Mouse movement | Right stick | Camera / aim; upstream response curve |
| LMB | A | Fire, confirm, interact |
| RMB | R | Aim |
| R | X | Reload / draw weapon |
| F / Mouse4 | Y | Sheathe, hold for Valor sheathe, use item |
| Space | B | Evade, leave siege mode, menu back / cancel |
| Left Shift | R | Ordinary sprint with weapon sheathed |
| Q | L | Item selector / original camera functions |
| C | X+A | Siege-mode shortcut; game state/style restrictions apply |
| Wheel up / down | L+X / L+B | One ammo/coating selection per notch, then release L |
| Esc / Enter / keypad Enter | Plus | Game menu |
| Backspace | Minus | Original Minus menu |
| Tab / MMB | ZL | Target selection with control type 1 |
| T | ZR | Original ZR action |
| 1 / 2 / 3 | D-pad up / right / down | Hunter Arts with control type 1 |
| Arrow keys | D-pad | Menu navigation / original directional actions |
| Ctrl / Alt | Left / right stick click | Original stick-click actions |
| E | R | Retained upstream binding |
| F1 | Host settings | Upstream settings panel |
| F2 | Release mouse | Click the game window to capture it again |

**Space is the game's B/back button; Esc is Plus.** During game text entry, Enter confirms and Escape cancels the text entry as upstream. Shift and RMB both send R: the game interprets R as ordinary sprinting when sheathed and aiming when holding the bowgun. Shift never adds B or automatically performs Valor Power Run. Valor reload timing, Power Run transitions, siege availability and combat behavior stay under the game's control. E and the mouse-look curve are unchanged.

Wheel input is queued as L down → L+X/B → L held → L up. Each phase lasts at least 40 ms so presses remain visible at 30 FPS. This is a fixed controller sequence, including outside combat; it does not detect every menu context. Queued wheel input is cleared on focus loss and on text-entry/overlay transitions. Bindings use SDL physical scancodes; the labels refer to QWERTY positions, including on AZERTY or QWERTZ keyboards.

## Generate and install PC prompts

Use Python 3.10+ and Pillow 12.2+. Run from the repository root, substituting your own unmodified extracted game's `romfs` and an **empty output directory outside that romfs**:

```sh
python -m pip install "Pillow>=12.2"
python tools/build_valor_hbg_assets.py /path/to/game/romfs /path/to/ValorHBG-PC-Controls
```

Exit the game completely. Copy the generated `romfs` directory into `mods/ValorHBG-PC-Controls/romfs` beside the executable. Keep its generated `romfs/keyboard_mouse_icons.flag` marker. Set both INI options above and restart. The marker identifies this dedicated prompt layer even if the mod folder is renamed or nested. The mod must also be enabled in the Mods list; the switch does not override `load_order.txt` or affect unrelated mods.

For an older generated copy, regenerate it or create an empty `keyboard_mouse_icons.flag` file in its `romfs` directory. Icons and matching text share this dedicated layer and are switched together. Keep other features out of the marked layer. Replace an older generated copy instead of leaving two competing prompt mods installed. To uninstall, move this mod out of `mods` and restore `KeyboardPreset = Original`.

The generator covers all seven languages shipped in MHGU's international release:

| Resource | Language | Mouse labels in adapted text | Running / ammunition |
| --- | --- | --- | --- |
| chS | 简体中文 | 鼠标左键 / 右键 / 中键 | Shift 奔跑；滚轮切换弹药与瓶 |
| chT | 繁體中文 | 滑鼠左鍵 / 右鍵 / 中鍵 | Shift 奔跑；滾輪切換子彈與瓶 |
| eng | English | LMB / RMB / MMB | Shift to dash; wheel for ammo/coatings |
| fre | Français | LMB / RMB / MMB : boutons gauche / droit / central de la souris | Shift pour courir ; molette pour les munitions/fioles |
| ger | Deutsch | LMB / RMB / MMB: linke / rechte / mittlere Maustaste | Shift zum Rennen; Mausrad für Munition/Beschichtungen |
| ita | Italiano | LMB / RMB / MMB: pulsante sinistro / destro / centrale del mouse | Shift per correre; rotella per munizioni/boccette |
| spa | Español | LMB / RMB / MMB: botón izquierdo / derecho / central del ratón | Shift para correr; rueda para munición/revestimientos |

Western button phrases use the shorter mouse abbreviations to limit line growth. The generator adapts explicit button names and stick references in localized text tables, plus movement/ammo tips. It preserves entry identifiers/order and color/page tags. Archive-local text variants are patched from their own text rather than replaced with a different loose copy.

Textures include the shared button font, title confirmation prompt, menu L/R paging, item HUD, target selector, Hunter Art directions and lobby buttons. Shared and localized texture variants are patched individually. Only the BC3 blocks covering known input icons are changed; other compressed texture blocks and unchanged ARC payloads remain byte-for-byte intact. The generator validates the resulting GMD tables and repacked resources, and writes a local `manifest.json` with per-language counts and decoded texture previews.

Prompts are static and show the primary binding of each game button: R shows RMB, while sprinting text says Shift. D-pad prompts show arrow keys; 1/2/3 are aliases for up/right/down. The mod also changes controller prompts, so set `KeyboardMouseIcons = Off` and restart when you want original controller icons. Illustrated manual pages with baked controller drawings have not been redrawn. The game’s control-type setting still determines button actions; start with type 1 for the mapping above. The upstream host F1 settings panel retains its existing English/Spanish languages.

## Source and verification

The repository contains the input implementation, patch tools, PC icon source PNGs, layout metadata and documentation. Original game assets, generated ARC/TEX/GMD files, previews, saves and game DLLs must stay local. PC icon sources were generated with imagegen; prompts are in `imagegen-prompts.txt`. The upstream MIT license applies to the added source and PC icon materials.

Host-only checks, using the normal build environment:

```sh
make test-settings test-valor-hbg test-settings-locale
```

These check configuration persistence/defaults, fixed button masks, wheel transitions at 30/60/120/240 FPS, multilingual chords, stick clicks and formatting tags. The text unit tests also run with `python tests/test_valor_hbg_text.py` and need no game data or Pillow. Runtime UI verification uses a separately copied test save; the structural checks do not establish that every illustrated instruction or every UI layout has been visually tested.
