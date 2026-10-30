#include "../updater/paths.h"
#include <assert.h>
#include <stdio.h>

int main(void) {
    assert(!update_kept("mods/Keyboard_Mouse_Input_Auto/romfs/keyboard_mouse_prompts.bin"));
    assert(!update_kept("mods/Keyboard_Mouse_Input_Auto/romfs/nativeNX/eng/table/TitleMsg_eng.gmd"));
    assert(update_kept("mods/Keyboard_Mouse_Input_Auto/.disabled"));
    assert(update_kept("mods/Keyboard_Mouse_Input_Auto/notes.txt"));
    assert(update_kept("mods/Keyboard_Mouse_Input_Auto_other/romfs/file"));
    assert(update_kept("mods/other/romfs/file"));
    assert(update_kept("mods/load_order.txt"));
    assert(update_kept("data/exefs/main") && update_kept("save/save_0/system"));
    assert(update_kept("mhgurecomp.ini") && update_kept("chat_box.txt"));
    assert(!update_kept("mhgurecomp.exe"));
    puts("Updater: bundled Auto resources replaceable; other mods, disabled state, ROM, saves and settings protected");
}
