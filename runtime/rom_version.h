// Supported Switch dump identity, checked before loading code or installing hooks.
#pragma once
#include <stdbool.h>
#include <stddef.h>

bool rom_version_check(const char *root, char *why, size_t size);
