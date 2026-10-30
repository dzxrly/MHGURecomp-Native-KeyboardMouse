<h1 align="center">Keyboard/mouse resources for maintainers</h1>

[简体中文](README.md) · [Developer documentation](../../docs/technical/README.md)

Players only need to enable the Auto prompt mod bundled with their release; see the [user guide](../../README.md#keyboard-and-mouse).

This maintainer directory holds development artwork, mappings and the fixed resources distributed from `prebuilt/Keyboard_Mouse_Input_Auto`. Release packaging consumes that prebuilt directory directly. Preserve `romfs/keyboard_mouse_input.flag` and `romfs/keyboard_mouse_prompts.bin` when maintaining the pack.

Implementation: [native input and automatic prompts](../../docs/technical/ARCHITECTURE.md#input-flow). Artwork prompts: [imagegen-prompts.txt](imagegen-prompts.txt).
