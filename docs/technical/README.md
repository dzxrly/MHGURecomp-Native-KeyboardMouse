# Developer documentation

Installation, configuration and controls are documented in the user guides: [English](../../README.md), [简体中文](../README.zh-CN.md), [繁體中文](../README.zh-TW.md).

This section is for building and maintaining the project. The supported client platform is Windows; Linux is currently unsupported because the project has no Linux build and test environment.

## Build and maintain

| Document | Contents |
| --- | --- |
| [Building and releasing](BUILDING.md) | Windows toolchain, game-code generation, local release packaging, licensing and selective upstream synchronization |
| [Runtime and native input](ARCHITECTURE.md) | Runtime structure, ROM compatibility checks, input routing, advanced mouse parameters and automatic prompts |
| [Testing and recorded coverage](TESTING.md) | Existing checks, isolated runtime testing, exercised actions and outstanding verification |
| [Host settings overlay](SETTINGS_UI.md) | Settings UI behavior, persistence and regression checks |
| [Prompt resources](../../extras/keyboard-mouse/README.en.md) | Maintainer assets and the checked-in Auto mod |
| [Hub server](../../server/README.md) | Optional server build, deployment and protocol |

## Graphics and research notes

- [FSR 3.1](FSR-PORT.md), [FSR 4](FSR4-PORT.md) and [pipeline cache](PIPELINE_LIST.md)
- [Armor skill points](research/skill-points.md), [Valor Sheathe](research/valor-sheathe.md) and [small-monster synchronization](research/small-monster-sync.md)

## Source layout

| Directory | Responsibility |
| --- | --- |
| `runtime/` | Host runtime, native input, graphics, audio, networking and configuration |
| `tools/` | Recompiler, packaging and prompt-resource tools |
| `updater/` | Updater for this repository's releases |
| `server/` | Optional multiplayer hub server |
| `extras/` | Prompt artwork, prebuilt Auto resources and optional cheats |
| `docs/technical/` | Maintainer guides and implementation notes |

Keep original game files read-only. Implement compatibility changes in project code or mod overlays, and use isolated copies of saves for experiments. Release packaging uses the checked-in Auto mod; players only enable the copy bundled with their release.

When updating documentation, put player instructions in all three user guides and implementation or validation details here. Keep known user-visible limitations in the relevant user-guide section; detailed test coverage belongs in [TESTING.md](TESTING.md).
