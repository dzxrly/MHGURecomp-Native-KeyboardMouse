// Last meaningful host input used for the settings menu's hints, independent of gameplay presets.
#pragma once
#include <SDL3/SDL.h>

typedef struct {
    bool gamepad;
    int stick[2];  // left-stick directions: -1, neutral, +1 (ignore drift and a held direction)
} SettingsInput;

static inline void settings_input_event(SettingsInput *state, const SDL_Event *event) {
    switch (event->type) {
    case SDL_EVENT_KEY_DOWN:
        if (!event->key.repeat) state->gamepad = false;
        break;
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
        state->gamepad = false;
        break;
    case SDL_EVENT_MOUSE_WHEEL:
        if (event->wheel.x || event->wheel.y) state->gamepad = false;
        break;
    case SDL_EVENT_MOUSE_MOTION:
        if (event->motion.xrel > 2 || event->motion.xrel < -2 ||
            event->motion.yrel > 2 || event->motion.yrel < -2) state->gamepad = false;
        break;
    case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
        state->gamepad = true;
        break;
    case SDL_EVENT_GAMEPAD_AXIS_MOTION:
        if (event->gaxis.axis <= SDL_GAMEPAD_AXIS_LEFTY) {
            int direction = event->gaxis.value > 8000 ? 1 : event->gaxis.value < -8000 ? -1 : 0;
            int *previous = &state->stick[event->gaxis.axis];
            if (direction && direction != *previous) state->gamepad = true;
            *previous = direction;
        }
        break;
    }
}
