# Keyboard and mouse controls

[中文](README.md)

Choose between Melee and Ranged bindings, with PC button prompts and control text in all seven game languages: Simplified Chinese, Traditional Chinese, English, French, German, Italian and Spanish.

## Enable

Edit `mhgurecomp.ini` in the game directory:

```ini
[Controls]
InputMode = Keyboard_Mouse_Input
KeyboardLayout = Ranged
```

Choose `Melee` for melee weapons or `Ranged` for ranged weapons, then restart the game. Change the layout manually when needed; it does not follow your equipped weapon.

The source default is `InputMode = Original`, which uses the upstream keyboard bindings, controller prompts and mouse conversion. Controllers work in both modes.

Generate and install the PC prompts as described below.

## Install prompts and control text

Use Python 3.10 or newer and Pillow 12.2 or newer. Run these commands from the repository root, replacing the paths with your game resources and the `mods` directory next to the game executable:

```sh
python -m pip install "Pillow>=12.2"
python tools/build_keyboard_mouse_assets.py /path/to/game/romfs /path/to/mods/Keyboard_Mouse_Input_Melee --layout Melee
python tools/build_keyboard_mouse_assets.py /path/to/game/romfs /path/to/mods/Keyboard_Mouse_Input_Ranged --layout Ranged
```

Use unmodified game resources as the input. Both output directories must be empty and outside the input directory. The result should look like this:

```text
mods/
  Keyboard_Mouse_Input_Melee/
    romfs/
      keyboard_mouse_input.flag
      nativeNX/
  Keyboard_Mouse_Input_Ranged/
    romfs/
      keyboard_mouse_input.flag
      nativeNX/
```

Enable both packs in the Mods tab of the F1 settings menu. The game loads the pack selected by `KeyboardLayout`; `Original` skips both. Keep each `keyboard_mouse_input.flag`, which identifies the pack's layout. Changing the input settings does not enable a pack disabled in the Mods tab.

Prompts cover the button font, title screens, menu paging, combat HUD, target selection, Hunter Art directions and lobby. Control text follows the game's language. Icons show the primary bindings rather than every alternate key. Full-page controller illustrations in the manual have not been replaced. The F1 settings menu still uses the upstream English or Spanish text.

## Key bindings

| Input | Melee | Ranged |
| --- | --- | --- |
| WASD | Left stick: movement | Left stick: movement |
| Mouse motion | Right stick: camera | Right stick: camera/aim |
| LMB | X: primary attack/draw | A: fire, confirm/interact |
| RMB | A: secondary attack, confirm/interact | R: aim |
| R | R: guard/weapon special action | X: reload/draw |
| Mouse5 | R: guard/weapon special action | Unbound |
| F / Mouse4 | Y: sheathe/use item | Y: sheathe/use item |
| E | Y: sheathe/use item | R: aim |
| Left Shift | R: ordinary sprint | R: ordinary sprint |
| Space | B: evade, cancel/back | B: evade, cancel/back |
| C | X+A: combined attack | X+A: siege mode/combined action |
| Wheel up/down | L+Y / L+A: select items | L+X / L+B: select ammo/coatings |
| Q | L: item selector/camera functions | L: item selector/camera functions |
| Esc / Enter / keypad Enter | Plus: game menu | Plus: game menu |
| Backspace | Minus: minus-button menu | Minus: minus-button menu |
| Tab / MMB | ZL: target selection with control type 1 | ZL: target selection with control type 1 |
| T | ZR | ZR |
| 1 / 2 / 3 | D-pad up/right/down: Arts with control type 1 | D-pad up/right/down: Arts with control type 1 |
| Arrow keys | D-pad | D-pad |
| Ctrl / Alt | Left/right stick click | Left/right stick click |
| F1 / F2 | PC settings / release mouse | PC settings / release mouse |

Confirm/interact is RMB in Melee and LMB in Ranged. Both use Space to go back. During text entry, Enter confirms and Esc cancels. After releasing the mouse with F2, click the game window to capture it again.

Shift sends the game's ordinary sprint input, separate from Power Run. With a weapon drawn, the same game button R may guard or aim, depending on the weapon and control type.

The Ranged fire/reload descriptions are for Light and Heavy Bowguns. With Bow control type 1, keyboard R draws the bow and RMB aims; control type 2 swaps these actions.

Keys use physical QWERTY positions, including on other keyboard layouts.

## Mouse settings

`MouseSensitivity` under `[Mouse]` sets the overall sensitivity. `[MouseConversion]` adjusts how mouse motion becomes right-stick input in `Keyboard_Mouse_Input` mode. Melee and Ranged share these settings. Restart the game after changing any of the five values.

```ini
[MouseConversion]
MinWindowMs = 8.0
FullStickSpeed = 1500.0
DeadzoneCompensation = 0.40
LowSpeedCutoff = 0.002
FrameTimeSmoothing = 0.25
```

> Relative counts are the movement reported by the mouse, not screen pixels. Mouse DPI (dots per inch) affects counts per physical movement. Normalized speed is `counts/second * MouseSensitivity / FullStickSpeed`.

| Setting | Range / unit | Effect |
| --- | --- | --- |
| `MinWindowMs` | 1–50 milliseconds | Minimum averaging window for mouse displacement. Higher is smoother and takes longer to return to zero after stopping. The actual window spans whole frame intervals, up to 100 ms. |
| `FullStickSpeed` | 100–100000 counts/second | Mouse speed that reaches full stick at sensitivity 1. Higher produces less stick deflection for the same mouse speed. |
| `DeadzoneCompensation` | 0–0.95 of full stick | Compensates for the game's stick dead zone. Higher strengthens small movements; too low may leave them unable to move the camera. |
| `LowSpeedCutoff` | 0–0.1 normalized speed | Ignores motion below this value. Lower allows finer input; zero disables this filter. |
| `FrameTimeSmoothing` | 0.01–1, weight of the latest frame interval | Adjusts the frame-time estimate. Higher follows FPS changes faster; lower is more stable. It does not directly smooth mouse velocity. |

Start with `MouseSensitivity` for overall speed, then use the settings above for further adjustment. Full stick is reached at `FullStickSpeed / MouseSensitivity`; the default low-speed cutoff corresponds to 3 counts per second.

The mouse drives the right stick and remains subject to the game's maximum turn speed. Moving it the same distance does not always produce the same camera angle. Losing focus, releasing the mouse, entering text or opening a PC menu clears the motion history.

Defaults and ranges are also listed in the INI comments. Check the `[input] mouse conversion:` line in the startup log for the values in use.

## Development and checks

Run in the normal build environment:

```sh
make test-settings test-keyboard-mouse test-settings-locale test-keyboard-mouse-assets
```

These check both layouts, item/ammo wheel input, control text in seven languages, configuration read/write and mouse sampling. Icon tests use synthetic textures and the repository's PC art, with no game files. Mouse tests check input sampling rather than actual camera angles.

Each wheel step sends L down → selection button down → selection button up → L up, with at least 40 ms per phase. Further steps are queued. Focus loss, text entry and PC menu transitions clear the queue.

The resource tool changes control text and icons while preserving other resources. Each output includes a `manifest.json` listing the changes and icon previews; generated game resources stay local. PC icon generation prompts are in [imagegen-prompts.txt](imagegen-prompts.txt).

Check the in-game Hunter's Notes for weapon and style actions. Capcom's Generations manual also describes the game buttons: [Great Sword](https://game.capcom.com/manual/MH_Gen/en/page-101.html), [Heavy Bowgun](https://game.capcom.com/manual/MH_Gen/en/page-122.html).
