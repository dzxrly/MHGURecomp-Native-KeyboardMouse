// Mouse counts averaged over a finite window of whole game-frame intervals. This keeps short motions visible
// at low frame rates and bounds stopping latency instead of retaining an exponential velocity tail.
#pragma once
#include <math.h>
#include <stdint.h>
#include <string.h>

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

static inline uint64_t mouse_look_window(float frame_seconds) {
    // One whole frame at 30/60/120 Hz, two at 240 Hz. Never shorten the window below 8 ms.
    uint64_t frame = (uint64_t)(fminf(fmaxf(frame_seconds, 0.001f), 0.1f) * 1e9f);
    return frame * ((8000000ull + frame - 1) / frame);
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

static inline void mouse_look_stick(MouseLook *m, uint64_t now, uint64_t window, float sensitivity, int *rx, int *ry) {
    float x, y;
    mouse_look_velocity(m, now, window, &x, &y);
    x *= sensitivity / 1500.0f; y *= -sensitivity / 1500.0f;
    float magnitude = sqrtf(x*x + y*y);
    *rx = *ry = 0;
    if (magnitude < 0.002f) return;  // 3 counts/s at sensitivity 1, down from the old 30 counts/s cutoff
    // Preserve radial direction, the game's measured dead zone compensation and maximum stick speed.
    float scale = (0.40f + 0.60f * fminf(magnitude, 1.0f)) / magnitude * 32767.0f;
    *rx = (int)(x * scale); *ry = (int)(y * scale);
}
