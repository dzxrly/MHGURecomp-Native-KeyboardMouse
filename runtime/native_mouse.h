// Direct angles and automatic weapon bindings for Switch GU 1.4.0.
#pragma once
#include <stdbool.h>

void native_mouse_init(void);
bool native_mouse_enabled(void);
bool native_mouse_uses_stick(void);
bool native_mouse_stick_aiming(void);
int native_mouse_layout(void);  // KEYBOARD_MELEE or KEYBOARD_RANGED; confirmed equipped weapon
void native_mouse_add(double dx, double dy);
void native_mouse_reset(void);
// Bow combat uses its configured charge/aim buttons; menus retain the selected preset.
int native_mouse_button(unsigned button, int fallback);
