# Testing and recorded coverage

[Developer documentation](README.md) · [Runtime and native input](ARCHITECTURE.md)

## Existing checks

Run the checks relevant to the changed code:

```sh
make test-settings test-keyboard-mouse test-settings-locale test-keyboard-mouse-assets
```

`make test-settings-panel` exercises the settings overlay through ImGui without game files or a graphics device. Its scope is described in [SETTINGS_UI.md](SETTINGS_UI.md#validation). `make test-lift` compares recompiled instructions with the Unicorn CPU emulator.

Check game data and the prepared game-code library separately:

```sh
build/mhgurecomp.exe --check-rom /c/path/to/game
build/mhgurecomp.exe --check-gamelib
```

The first command checks supported executable IDs and selected base resources without starting the game. The second validates the library's ABI/ID without a ROM. Neither proves gameplay correctness or whole-ROM integrity.

## Runtime testing

Use isolated configuration and save directories. Copy the player's save into the test area before making experimental equipment changes; never write those changes back to the original. Treat both the source dump and any hard-linked `build/data` as read-only. Put test resource changes in mod overlays.

`MHGU_NATIVE_MOUSE_LOG=1` enables accepted-motion and projectile-direction traces. The existing control file's `stopinput` command clears scripted inputs and held test state. Normal startup does not hold aim automatically. Verify that input actually reached the intended action before describing a capture as an attack, guard or charge test.

## Recorded native-input coverage

The following records summarize Windows play checks through 0.11.3, including the 2026-10-31 follow-up. They are a partial record of exercised behavior. A shared Melee/Ranged preset does not establish that every move, style, ammo type or multiplayer context has been tested.

| Area | Recorded checks |
| --- | --- |
| Bowguns and Bow | Direct aim and projectile direction, both Bow control types, camera options and automatic equipment switching |
| Prowler | Boomerang precision aim and guard |
| Insect Glaive / Ballista | Glaive aim and Kinsect launch; fixed Ballista compatibility aiming |
| Long Sword / Lance / Charge Blade | Long Sword movement/draw direction, Lance guard/thrust, Charge Blade guard |
| Great Sword | Draw, attack and guard via actual mouse buttons; charge levels and full move chains remain unverified |
| Sword and Shield | Attack, X+A and guard via actual mouse buttons |
| Ordinary camera | The same 1,000-count input produced matching angle changes at selected 30/120/240 FPS settings |
| Heavy Bowgun at 30/240 FPS settings | Direct aim, projectile direction and discarded firing-lock input; no replay of blocked motion after recovery |

Selected FPS values do not establish stable delivered frame rates or coverage of physical mouse polling rates. Mouse sampler tests cover the compatibility conversion; they do not replace native-angle play tests. Guard-pose evidence does not establish damage blocking against an incoming attack. FSR and multiplayer retain the validation limits documented in their own notes.

## Outstanding work

- Bow-specific prompts and broader R/ZR-swap prompt coverage; some handbook pages still contain controller illustrations.
- Actual Prowler Support Move activation with an equipped move, and variable-zoom scope behavior.
- Wider weapon/style/ammo, full move-chain, frame-rate and multiplayer-context coverage, including Great Sword charge levels.

Unverified actions are not automatically unsupported. Keep these validation gaps separate from confirmed defects and from the practical limitations explained in the user guides.
