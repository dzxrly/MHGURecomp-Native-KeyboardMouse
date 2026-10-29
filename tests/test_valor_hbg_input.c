#include "valor_hbg_input.h"
#include <assert.h>
#include <stdio.h>

static void wheel_at_fps(unsigned fps, int notches) {
    HbgAmmoWheel w = {0};
    hbg_wheel_add(&w, (float)notches);
    uint64_t prev = 0, step = 1000000000ull / fps;
    int taps = 0;
    for (uint64_t now = 0; now < 2000000000ull; now += step) {
        uint64_t b = hbg_wheel_buttons(&w, now);
        uint64_t selection = HBG_BUTTON(notches > 0 ? BTN_X : BTN_B);
        if ((b & selection) && !(prev & selection)) {
            assert(prev == HBG_BUTTON(BTN_L));
            assert(b & HBG_BUTTON(BTN_L));
            taps++;
        }
        if ((prev & selection) && !(b & selection)) assert(b == HBG_BUTTON(BTN_L));
        prev = b;
    }
    assert(taps == (notches > 0 ? notches : -notches));
    assert(!w.pending && !w.phase);
}

int main(void) {
    assert(valor_hbg_key(SDL_SCANCODE_R) == HBG_BUTTON(BTN_X));
    assert(valor_hbg_key(SDL_SCANCODE_F) == HBG_BUTTON(BTN_Y));
    assert(valor_hbg_key(SDL_SCANCODE_SPACE) == HBG_BUTTON(BTN_B));
    assert(valor_hbg_key(SDL_SCANCODE_LSHIFT) == HBG_BUTTON(BTN_R));
    assert(!(valor_hbg_key(SDL_SCANCODE_LSHIFT) & HBG_BUTTON(BTN_B)));
    assert(valor_hbg_key(SDL_SCANCODE_E) == HBG_BUTTON(BTN_R));
    assert(valor_hbg_key(SDL_SCANCODE_C) == (HBG_BUTTON(BTN_X) | HBG_BUTTON(BTN_A)));
    assert(valor_hbg_key(SDL_SCANCODE_ESCAPE) == HBG_BUTTON(BTN_PLUS));
    assert(valor_hbg_key(SDL_SCANCODE_TAB) == HBG_BUTTON(BTN_ZL));
    assert(valor_hbg_key(SDL_SCANCODE_F2) == 0);
    assert(valor_hbg_mouse(SDL_BUTTON_LEFT) == BTN_A);
    assert(valor_hbg_mouse(SDL_BUTTON_RIGHT) == BTN_R);
    assert(valor_hbg_mouse(SDL_BUTTON_X1) == BTN_Y);
    assert(valor_hbg_mouse(SDL_BUTTON_MIDDLE) == BTN_ZL);
    assert(valor_hbg_mouse(SDL_BUTTON_X2) == -1);
    for (unsigned fps = 30; fps <= 240; fps *= 2) {
        wheel_at_fps(fps, 3);
        wheel_at_fps(fps, -3);
    }
    HbgAmmoWheel w = {0};
    hbg_wheel_add(&w, 0.5f); hbg_wheel_add(&w, 0.5f);
    assert(w.pending == 1 && w.fraction == 0);
    assert(hbg_wheel_buttons(&w, 0) == HBG_BUTTON(BTN_L));
    hbg_wheel_reset(&w);  // focus loss / opening an overlay must cancel queued selections and held modifiers
    assert(!hbg_wheel_buttons(&w, 1000000000ull));
    puts("ValorHBG bindings and ammo wheel: passed (30/60/120/240 fps)");
}
