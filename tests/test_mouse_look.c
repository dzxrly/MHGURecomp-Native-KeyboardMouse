#include "mouse_look.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static const MouseConversion defaults = MOUSE_CONVERSION_DEFAULTS;

static void conserve_counts(unsigned fps, unsigned rate, uint64_t phase) {
    MouseLook m = {0};
    uint64_t step = 1000000000ull / fps, packet = 1000000000ull / rate;
    uint64_t window = mouse_look_window((float)step * 1e-9f, &defaults), next = phase;
    double sent = 0, received = 0;
    for (uint64_t now = 0; now < 2200000000ull; now += step) {
        while (next <= now && next < 2000000000ull) {
            float dx = 300.0f / rate;
            mouse_look_add(&m, dx, 0, next); sent += dx;
            next += packet;
        }
        float x, y;
        mouse_look_velocity(&m, now, window, &x, &y);
        received += x * (step * 1e-9);
        assert(y == 0);
    }
    // Actual game angles still depend on its response curve. This checks input resampling only.
    assert(fabs(received - sent) < sent * 0.012);
}

static int horizontal_output(float speed, const MouseConversion *config) {
    MouseLook m = {0};
    mouse_look_add(&m, speed * 0.01f, 0, 1000000);
    int x, y;
    mouse_look_stick(&m, 1000000, 10000000, 1, config, false, &x, &y);
    assert(y == 0);
    return x;
}

static void aiming_thresholds(void) {
    const float speeds[] = {0.1f, 1, 10, 300, 1500, 1000000};
    const float ratios[] = {0.01f, 1, 100};
    for (unsigned fps = 30; fps <= 240; fps *= 2) {
        uint64_t window = mouse_look_window(1.0f / fps, &defaults);
        for (unsigned i = 0; i < sizeof speeds / sizeof *speeds; i++) {
            for (unsigned r = 0; r < sizeof ratios / sizeof *ratios; r++)
            for (int dx = -1; dx <= 1; dx++) for (int dy = -1; dy <= 1; dy++) {
                MouseLook m = {0};
                float counts = speeds[i] * window * 1e-9f;
                mouse_look_add(&m, dx * counts, dy * counts * ratios[r], 1000000000);
                int x, y;
                mouse_look_stick(&m, 1000000000, window, 1, &defaults, true, &x, &y);
                // Consumer boundary: input_npad_state() synthesizes each right-stick direction at >16000.
                // Ordinary radial compensation misses this boundary, including both axes of a slow diagonal.
                assert(dx ? x * dx > 16000 : x == 0);
                assert(dy ? y * -dy > 16000 : y == 0);
                assert(sqrt((double)x*x + (double)y*y) <= 32768);
                mouse_look_stick(&m, 1000000000 + window, window, 1, &defaults, true, &x, &y);
                assert(x == 0 && y == 0);
            }
        }
    }
    MouseLook m = {0};
    mouse_look_add(&m, 0.01f, 0, 1000000);
    int ordinary, aimed, y;
    mouse_look_stick(&m, 1000000, 10000000, 1, &defaults, false, &ordinary, &y);
    mouse_look_stick(&m, 1000000, 10000000, 1, &defaults, true, &aimed, &y);
    assert(ordinary > 0 && ordinary < 16000 && aimed > 16000);
    MouseConversion off = defaults;
    off.aim_deadzone_compensation = 0;
    mouse_look_stick(&m, 1000000, 10000000, 1, &off, true, &aimed, &y);
    assert(aimed == ordinary && y == 0);
    off = defaults;
    off.low_speed_cutoff = 0.002f;
    mouse_look_stick(&m, 1000000, 10000000, 1, &off, true, &aimed, &y);
    assert(aimed == 0 && y == 0); // An explicitly configured noise cutoff still applies when aiming.
}

int main(void) {
    for (unsigned fps = 30; fps <= 240; fps *= 2)
        for (unsigned rate = 125; rate <= 8000; rate *= 2)
            for (unsigned phase = 0; phase < 4; phase++)
                conserve_counts(fps, rate, phase * 700000ull);
    MouseLook m = {0};
    uint64_t window = mouse_look_window(1.0f/60, &defaults);
    int x, y;
    mouse_look_add(&m, 0.2f, 0, 1000000);
    mouse_look_stick(&m, 1000000, window, 1, &defaults, false, &x, &y);
    assert(x > 0 && y == 0);  // 12 counts/s, previously below the 30 counts/s cutoff
    mouse_look_stick(&m, 1000000 + window, window, 1, &defaults, false, &x, &y);
    assert(x == 0 && y == 0);  // bounded stop, no exponential tail
    mouse_look_add(&m, 10000, 10000, 100000000);
    mouse_look_stick(&m, 100000000, window, 1, &defaults, false, &x, &y);
    assert(x > 0 && y < 0 && abs(x + y) <= 1);
    assert(sqrt((double)x*x + (double)y*y) <= 32768);
    mouse_look_reset(&m);
    mouse_look_stick(&m, 100000001, window, 1, &defaults, false, &x, &y);
    assert(x == 0 && y == 0);
    assert(abs(horizontal_output(750, &defaults) - 22936) <= 1); // Previous default output: 0.7 * 32767.
    MouseConversion tuned=defaults;
    tuned.full_stick_speed=3000;
    assert(horizontal_output(750,&tuned) < horizontal_output(750,&defaults));
    tuned.deadzone_compensation=0;
    assert(abs(horizontal_output(750,&tuned) - 8191) <= 1);
    tuned.low_speed_cutoff=0.03f;
    assert(horizontal_output(30,&tuned)==0 && horizontal_output(30,&defaults)>0);
    tuned.low_speed_cutoff=0;
    assert(horizontal_output(0,&tuned)==0); // Disabling cutoff must not divide by zero at rest.
    tuned.min_window_ms=50;
    assert(mouse_look_window(1.0f/60,&tuned) > window);
    for(int ms=1;ms<=100;ms++) {
        uint64_t bounded=mouse_look_window(ms/1000.0f,&tuned);
        assert(bounded>=50000000ull && bounded<=100000000ull);
    }
    aiming_thresholds();
    puts("Mouse resampling: count conservation <1.2% at 30/60/120/240 FPS and 125-8000 Hz; low speed, bounded stop, direction and reset passed");
    puts("Mouse conversion parameters: default baseline, custom speed/deadzone/cutoff/window, zero cutoff at rest and 100 ms history bound passed");
    puts("Aiming: cardinal/unequal diagonal direction buttons, sign, unit circle, stop, configurable cutoff and profile disable passed");
}
