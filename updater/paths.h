// Files owned by the player, with one explicit release-owned prompt directory.
#pragma once
#include <stdbool.h>
#include <string.h>
#include <strings.h>

static inline bool update_kept(const char *rel) {
    static const char bundled[] = "mods/Keyboard_Mouse_Input_Auto/romfs/";
    if (!strncasecmp(rel, bundled, sizeof bundled - 1)) return false;
    static const char *const keep[] = {"data", "mods", "cheats", "bonus", "cache", "shots", "save", "update",
                                      "mhgurecomp.ini", "log.txt", "chat_box.txt"};
    size_t n = strcspn(rel, "/");
    for (size_t i = 0; i < sizeof keep / sizeof *keep; i++)
        if (strlen(keep[i]) == n && !strncasecmp(rel, keep[i], n)) return true;
    return false;
}
