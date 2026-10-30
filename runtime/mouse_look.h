// Mouse counts averaged over a finite window of whole game-frame intervals. This keeps short motions visible
// at low frame rates and bounds stopping latency instead of retaining an exponential velocity tail.
#pragma once
#include <math.h>
#include <stdint.h>
#include <string.h>
#include "config.h"

#define MOUSE_LOOK_SAMPLES 1024  // >100 ms at 8000 Hz; equal-timestamp events are coalesced
typedef struct { uint64_t time; float x, y; } MouseLookSample;
typedef struct {
    MouseLookSample samples[MOUSE_LOOK_SAMPLES];
    unsigned first, count;
} MouseLook;

static inline void mouse_look_reset(MouseLook *m) { memset(m, 0, sizeof *m); }
static inline void mouse_look_add(MouseLook *m, float x, float y, uint64_t time) {
    if (m->count) {
        MouseLookSample *last = &m->samples[(m->first + m->count - 1) % MOUSE_LOOK_SAMPLES];
        if (time <= last->time) { last->x += x; last->y += y; return; }
    }
    if (m->count == MOUSE_LOOK_SAMPLES) { m->first = (m->first + 1) % MOUSE_LOOK_SAMPLES; m->count--; }
    m->samples[(m->first + m->count++) % MOUSE_LOOK_SAMPLES] = (MouseLookSample){time, x, y};
}

static inline uint64_t mouse_look_window(float frame_seconds, const MouseConversion *config) {
    // Config validation limits the minimum to 50 ms, keeping a whole-frame window <=100 ms.
    uint64_t frame = (uint64_t)(fminf(fmaxf(frame_seconds, 0.001f), 0.1f) * 1e9f);
    uint64_t minimum = (uint64_t)(config->min_window_ms * 1e6f);
    return frame * ((minimum + frame - 1) / frame);
}

static inline void mouse_look_velocity(MouseLook *m, uint64_t now, uint64_t window, float *x, float *y) {
    while (m->count && now >= m->samples[m->first].time && now - m->samples[m->first].time >= window) {
        m->first = (m->first + 1) % MOUSE_LOOK_SAMPLES; m->count--;
    }
    float dx = 0, dy = 0;
    for (unsigned i = 0; i < m->count; i++) {
        const MouseLookSample *s = &m->samples[(m->first + i) % MOUSE_LOOK_SAMPLES];
        if (s->time <= now) dx += s->x, dy += s->y;
    }
    *x = dx * (1e9f / window); *y = dy * (1e9f / window);
}

static inline void mouse_look_stick(MouseLook *m, uint64_t now, uint64_t window, float sensitivity,
                                  const MouseConversion *config, bool aiming, int *rx, int *ry) {
    float x, y;
    mouse_look_velocity(m, now, window, &x, &y);
    x *= sensitivity / config->full_stick_speed; y *= -sensitivity / config->full_stick_speed;
    float magnitude = sqrtf(x*x + y*y);
    *rx = *ry = 0;
    if (magnitude == 0 || magnitude < config->low_speed_cutoff) return;
    if (aiming && config->aim_deadzone_compensation > 0) {
        // The SDK exposes per-axis direction buttons (>16000) alongside the analog stick, whereas the
        // ordinary camera responds to a round ~39% dead zone. A radial 40% floor misses those buttons.
        // Rectangular anti-deadzone offsets only moving axes, then bounds the result to the stick circle.
        if (magnitude > 1) x /= magnitude, y /= magnitude;
        float floor = config->aim_deadzone_compensation;
        if (x != 0) x = copysignf(floor + (1.0f - floor) * fabsf(x), x);
        if (y != 0) y = copysignf(floor + (1.0f - floor) * fabsf(y), y);
        float length = sqrtf(x*x + y*y);
        if (length > 1) {
            x /= length; y /= length;
            // Circle clipping must not push the smaller moving axis back into its dead zone.
            // At most 1/sqrt(2) can be guaranteed on both axes inside a unit circle.
            float minimum = fminf(floor, 0.70710678f);
            float maximum = sqrtf(1.0f - minimum * minimum);
            if (x != 0 && fabsf(x) < minimum) x = copysignf(minimum, x), y = copysignf(maximum, y);
            else if (y != 0 && fabsf(y) < minimum) y = copysignf(minimum, y), x = copysignf(maximum, x);
        }
        *rx = (int)(x * 32767.0f); *ry = (int)(y * 32767.0f);
        return;
    }
    // Preserve radial direction, the game's measured dead zone compensation and maximum stick speed.
    float deadzone = config->deadzone_compensation;
    float scale = (deadzone + (1.0f - deadzone) * fminf(magnitude, 1.0f)) / magnitude * 32767.0f;
    *rx = (int)(x * scale); *ry = (int)(y * scale);
}
