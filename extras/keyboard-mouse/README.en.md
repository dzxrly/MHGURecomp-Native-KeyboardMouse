# Keyboard and mouse controls

[简体中文](README.md)

Bindings and prompts follow the equipped weapon automatically. Light/Heavy Bowgun and Bow use Ranged; other Hunter weapons and Prowler use Melee. Prompts support Simplified/Traditional Chinese, English, French, German, Italian and Spanish.

## Enable

Set this in the game's `mhgurecomp.ini`, then restart once:

```ini
[Controls]
InputMode = Keyboard_Mouse_Input
```

Changing equipment or switching Hunter/Prowler then updates bindings, text and icons without editing the configuration or restarting. The old `KeyboardLayout` setting is no longer used. Switching releases held inputs from the old preset and clears queued wheel steps.

The source default remains `InputMode = Original`, preserving upstream bindings and controller prompts. Controllers work in either mode. Direct angles target the exported Switch MHGU 1.4.0 executable and need no extra environment variable.

## Install prompts and control text

Use Python 3.10+ and Pillow 12.2+. From the repository root, replace these paths with the original game RomFS and the installed game's `mods` directory:

```sh
python -m pip install "Pillow>=12.2"
python tools/build_keyboard_mouse_assets.py /path/to/game/romfs /path/to/mods/Keyboard_Mouse_Input_Auto --layout Auto
```

The output directory must be empty and outside the input directory. One Auto pack contains both presets:

```text
mods/
  Keyboard_Mouse_Input_Auto/
    romfs/
      keyboard_mouse_input.flag
      keyboard_mouse_prompts.bin
      nativeNX/
```

Enable this pack in the F1 settings menu's Mods tab. Original skips it; the old single-preset Melee/Ranged packs are no longer loaded. Keep the marker and texture data. Disabled packs are not enabled automatically.

The pack covers the button font, title, menu pages, combat HUD, target selection, Hunter Arts directions and lobby. Prompts follow the game language and active preset. Icons show primary bindings rather than every alternative. Full-page controller illustrations in the handbook remain unchanged; F1 settings retain upstream English/Spanish.

## Key bindings

| Input | Melee | Ranged |
| --- | --- | --- |
| WASD | Left stick: movement | Left stick: movement |
| Mouse motion | Quest camera | Quest camera / weapon aim |
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

## Mouse camera and sensitivity

> Direct angle input converts relative mouse counts into the game's camera or aiming angles. Counts are not screen pixels; mouse DPI (counts per inch) changes the counts produced by the same physical movement.

`MouseSensitivity` under `[Mouse]` sets base sensitivity. Ordinary quest camera, Light/Heavy Bowgun and Bow aim, and Binoculars use direct angles without stick deadzones, stick turn-speed caps or added acceleration. Original pitch bounds, post-shot input restrictions, aim-entry delay and action turning windows remain. Blocked movement is discarded, with no catch-up turn on recovery.

The in-game speed options change mouse sensitivity immediately:

| Menu option | Mouse operation | Slow | Default | Fast |
| --- | --- | --- | --- | --- |
| Camera Speed | Ordinary quest camera | 0.5x | 1x | 1.5x |
| Reticle Speed | Light/Heavy Bowgun, Bow aim and Binoculars | 0.5x | 1x | 1.5x |

These options are independent and multiply `MouseSensitivity`. At the default base calibration, 1000 horizontal counts turn about 20, 40 or 60 degrees. Camera Controls, Quick Aim Controls and Scope Controls independently select inversion for ordinary, Quick Aim and first-person/Binocular views.

All five Quick Aim Camera types retain their entry, return and vertical-limit behavior. Initial camera position, Target Camera controls/behavior and Terrain Adjustment remain in the game. Mouse input is registered at its manual-camera stage so auto pitch and terrain processing recognize it. Turning the camera does not force an attacking melee character to face it.

## Bow and Prowler

Bow automatically uses Ranged. While sheathed, LMB remains A/confirm/interact; press keyboard R (game X) to draw. Once drawn:

| Input | Bow control Type 1 | Bow control Type 2 |
| --- | --- | --- |
| Left mouse button | X: charge, release to shoot | The game's configured R: charge, release to shoot |
| Right mouse button | The game's configured R: hold to aim | X: click to toggle aiming, click again to leave |
| Mouse side button 5 | A: Bow secondary action | A: Bow secondary action |

The game's R/ZR swap is respected. Keyboard bindings remain as in the base table. Menus restore LMB to A, and a mouse release releases the game button selected when pressed. Auto prompts follow Melee/Ranged; Bow Types 1/2 currently retain the base Ranged prompts, so use the mouse mapping above.

Prowler uses Melee: LMB is X/melee, RMB is A/boomerang, and R/Mouse5 is the game's R. Hold RMB to charge a boomerang, then hold R/Mouse5 to aim it. With the weapon drawn, R blocks and R+X performs the Overhead Smash; while sheathed, R+X+A blocks. The wheel uses L+Y/L+A to select Support Moves. Press F (Y) while sheathed, or R+F (R+Y) while drawn, to use one. Original eligibility, costs and action restrictions remain.

## Compatibility cameras and conversion settings

Village rail cameras, fixed Ballista and special Prowler aiming retain the game's stick-processing path. Mouse movement becomes right-stick input here. `[MouseConversion]` affects these compatibility cameras, while ordinary quest camera and the direct gun/Bow/Binocular paths do not use it. Actual compatibility aiming state selects per-axis compensation. Restart after changing conversion parameters.

```ini
[MouseConversion]
MinWindowMs = 8.0
FullStickSpeed = 1500.0
DeadzoneCompensation = 0.40
AimDeadzoneCompensation = 0.50
LowSpeedCutoff = 0.0
FrameTimeSmoothing = 0.25
```

> Relative counts are the movement reported by the mouse, not screen pixels. Mouse DPI (dots per inch) affects counts per physical movement. Normalized speed is `counts/second * MouseSensitivity / FullStickSpeed`.

| Setting | Range / unit | Effect |
| --- | --- | --- |
| `MinWindowMs` | 1–50 milliseconds | Minimum averaging window for mouse displacement. Higher is smoother and takes longer to return to zero after stopping. The actual window spans whole frame intervals, up to 100 ms. |
| `FullStickSpeed` | 100–100000 counts/second | Mouse speed that reaches full stick at sensitivity 1. Higher produces less stick deflection for the same mouse speed. |
| `DeadzoneCompensation` | 0–0.95 of full stick | Compensates for the game's stick dead zone. Higher strengthens small movements; too low may leave them unable to move the camera. |
| `AimDeadzoneCompensation` | 0–0.95 of each axis | While a compatibility camera is aiming, each moving axis is offset separately. The default 0.50 lets slow horizontal, vertical and diagonal inputs activate right-stick direction buttons. Zero uses the ordinary radial conversion. |
| `LowSpeedCutoff` | 0–0.1 normalized speed | Ignores motion below this value. Lower allows finer input; zero disables this filter. |
| `FrameTimeSmoothing` | 0.01–1, weight of the latest frame interval | Adjusts the frame-time estimate. Higher follows FPS changes faster; lower is more stable. It does not directly smooth mouse velocity. |


Ordinary compatibility cameras use radial compensation; compatibility aim uses per-axis compensation, bounded to the stick circle. The game's turn-speed cap still applies, so equal mouse distance need not give equal angle. Focus loss, mouse release, text input, the PC overlay and preset switches clear motion history.

Set `MHGU_NATIVE_MOUSE_LOG=1` to log accepted motion, camera synchronization and initial shell/arrow directions. The control file's `stopinput` cancels automated keys, test mouse velocity and held inputs. Normal startup does not automatically hold aim.

## Development and checks

Run in the normal build environment:

```sh
make test-settings test-keyboard-mouse test-settings-locale test-keyboard-mouse-assets
```

These check both layouts, item/ammo wheel input, control text in seven languages, configuration read/write and mouse sampling. Icon tests use synthetic textures and the repository's PC art, with no game files. Mouse tests check input sampling rather than actual camera angles.

Each wheel step sends L down → selection button down → selection button up → L up, with at least 40 ms per phase. Further steps are queued. Focus loss, text entry and PC menu transitions clear the queue.

The resource tool changes control text and icons while preserving other resources. Each output includes a `manifest.json` listing the changes and icon previews; generated game resources stay local. PC icon generation prompts are in [imagegen-prompts.txt](imagegen-prompts.txt).

Check the in-game Hunter's Notes for weapon and style actions. Capcom's Generations manual also describes the game buttons: [Great Sword](https://game.capcom.com/manual/MH_Gen/en/page-101.html), [Heavy Bowgun](https://game.capcom.com/manual/MH_Gen/en/page-122.html).
