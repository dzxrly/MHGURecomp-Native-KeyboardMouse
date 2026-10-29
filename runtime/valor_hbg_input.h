// Fixed keyboard/mouse preset. These are ordinary controller inputs, not timed combat actions.
#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_mouse.h>
#include "hle/input.h"

#define HBG_BUTTON(b) (1ull << (b))

static inline uint64_t valor_hbg_key(SDL_Scancode sc) {
    switch (sc) {
    case SDL_SCANCODE_SPACE: return HBG_BUTTON(BTN_B);
    case SDL_SCANCODE_R: return HBG_BUTTON(BTN_X);
    case SDL_SCANCODE_F: return HBG_BUTTON(BTN_Y);
    case SDL_SCANCODE_C: return HBG_BUTTON(BTN_X) | HBG_BUTTON(BTN_A);
    case SDL_SCANCODE_Q: return HBG_BUTTON(BTN_L);
    // E keeps its original binding. Shift is the game's ordinary R/run input, never B/Power Run.
    case SDL_SCANCODE_E: case SDL_SCANCODE_LSHIFT: return HBG_BUTTON(BTN_R);
    case SDL_SCANCODE_ESCAPE: case SDL_SCANCODE_RETURN: case SDL_SCANCODE_KP_ENTER: return HBG_BUTTON(BTN_PLUS);
    case SDL_SCANCODE_TAB: return HBG_BUTTON(BTN_ZL);
    case SDL_SCANCODE_BACKSPACE: return HBG_BUTTON(BTN_MINUS);
    case SDL_SCANCODE_T: return HBG_BUTTON(BTN_ZR);
    case SDL_SCANCODE_1: case SDL_SCANCODE_UP: return HBG_BUTTON(BTN_UP);
    case SDL_SCANCODE_2: case SDL_SCANCODE_RIGHT: return HBG_BUTTON(BTN_RIGHT);
    case SDL_SCANCODE_3: case SDL_SCANCODE_DOWN: return HBG_BUTTON(BTN_DOWN);
    case SDL_SCANCODE_LEFT: return HBG_BUTTON(BTN_LEFT);
    case SDL_SCANCODE_LCTRL: case SDL_SCANCODE_RCTRL: return HBG_BUTTON(BTN_LSTICK);
    case SDL_SCANCODE_LALT: case SDL_SCANCODE_RALT: return HBG_BUTTON(BTN_RSTICK);
    default: return 0;
    }
}

static inline int valor_hbg_mouse(unsigned button) {
    switch (button) {
    case SDL_BUTTON_LEFT: return BTN_A;
    case SDL_BUTTON_RIGHT: return BTN_R;
    case SDL_BUTTON_MIDDLE: return BTN_ZL;
    case SDL_BUTTON_X1: return BTN_Y;
    default: return -1;
    }
}

typedef struct {
    int pending, direction, phase;
    float fraction;
    uint64_t until;
} HbgAmmoWheel;

static inline void hbg_wheel_reset(HbgAmmoWheel *w) { *w = (HbgAmmoWheel){0}; }

static inline void hbg_wheel_add(HbgAmmoWheel *w, float amount) {
    w->fraction += amount;
    int steps = (int)w->fraction;
    w->fraction -= steps;
    w->pending += steps;
}

// L down -> X/B tap -> X/B up -> L up. A >=40 ms phase remains visible at 30 fps as well as high frame rates.
// Queue wheel notches instead of repeating a held X/B, which the game would treat as one selection.
static inline uint64_t hbg_wheel_buttons(HbgAmmoWheel *w, uint64_t now) {
    if (w->phase && now >= w->until) {
        w->phase = (w->phase + 1) % 5;
        w->until = now + 40000000ull;
    }
    if (!w->phase && w->pending) {
        w->direction = w->pending > 0 ? 1 : -1;
        w->pending -= w->direction;
        w->phase = 1;
        w->until = now + 40000000ull;
    }
    if (w->phase == 1 || w->phase == 3) return HBG_BUTTON(BTN_L);
    if (w->phase == 2) return HBG_BUTTON(BTN_L) | HBG_BUTTON(w->direction > 0 ? BTN_X : BTN_B);
    return 0;
}
