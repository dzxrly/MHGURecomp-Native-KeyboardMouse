// Addresses are guest runtime addresses in Switch GU 1.4.0 (fb08f1d20fd1204f).
// Mouse counts change authoritative angles, without passing through cJoyData.
#include <math.h>
#include <pthread.h>
#include <stdlib.h>
#include <SDL3/SDL_mouse.h>

#include "../config.h"
#include "../hle.h"
#include "../native_mouse.h"
#include "input.h"

#define FN_INPUT  0x47c97cu
#define FN_CAMERA 0x2255ccu
#define FN_AIM    0x4a1738u
#define FN_AIM_PITCH 0x228bbcu
#define FN_AIM_YAW   0x228cf8u
#define FN_MENU_OPEN 0x7584a0u
#define FN_SHELL_ANGLES 0x6703f8u
#define FN_ARROW_SETUP 0x6784b8u
#define FN_QUEST_CAMERA 0x221a90u
#define FN_VILLAGE_CAMERA 0x22add4u
#define FN_TARGET_CAMERA 0x2264dcu
#define VT_CAMERA 0x1930d0cu
#define VT_HBG    0x194bdc4u
#define VT_LBG    0x194c694u
#define VT_BOW    0x194e9e0u

static int enabled;
static int use_stick;
static int stick_aiming;
static int active_layout = KEYBOARD_MELEE;
static bool trace;
static double degrees_per_count = 0.04;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static double pending_x, pending_y, frame_x, frame_y;
static double yaw_fraction, pitch_fraction;
static uint64_t pending_at, frame_at;
static bool frame_allowed;
static int bow_buttons;  // low bits: 1 Type1 / 2 Type2; bit 2: game's R/ZR swap
static unsigned reset_generation, seen_generation;
static uint32_t quest_camera_address;  // game thread: most recent local quest camera
// Game-thread state, cleared at each local input update. No raw counts are consumed twice.
static uint32_t aim_player;
static bool aim_pitch_changed, aim_yaw_changed;
static int aim_yaw_delta;

bool native_mouse_enabled(void) { return __atomic_load_n(&enabled, __ATOMIC_ACQUIRE); }
bool native_mouse_uses_stick(void) { return __atomic_load_n(&use_stick, __ATOMIC_ACQUIRE); }
bool native_mouse_stick_aiming(void) { return __atomic_load_n(&stick_aiming, __ATOMIC_ACQUIRE); }
int native_mouse_layout(void) { return __atomic_load_n(&active_layout, __ATOMIC_ACQUIRE); }

static void update_layout(Cpu *c) {
    uint32_t players = LD32(0x1a85b58u);
    if (!players) return;
    unsigned mode = LD32(players + 0x9c5c);
    if (mode > 1 || (mode == 1 && !LD32(0x1a85468u))) return;
    Cpu saved = *c;
    c->r[0] = players;
    guest_call(c, 0x475a30u);  // actual local character params; excludes equipment previews
    uint32_t param = c->r[0];
    *c = saved;
    if (!param) return;
    unsigned weapon = LD8(param + 0x4d4);
    if (weapon > 15) return;
    int layout = !mode && (weapon == 4 || weapon == 6 || weapon == 10) ? KEYBOARD_RANGED : KEYBOARD_MELEE;
    int previous = __atomic_exchange_n(&active_layout, layout, __ATOMIC_ACQ_REL);
    if (previous != layout) LOG("[input] equipped weapon class %u (%s): %s bindings\n", weapon,
        mode ? "Prowler" : "Hunter", layout == KEYBOARD_MELEE ? "Melee" : "Ranged");
}

int native_mouse_button(unsigned button, int fallback) {
    if (!native_mouse_enabled() || native_mouse_layout() != KEYBOARD_RANGED) return fallback;
    pthread_mutex_lock(&lock);
    int mode = frame_allowed && frame_at && plat_now_ns() - frame_at < 100000000ull ? bow_buttons : 0;
    pthread_mutex_unlock(&lock);
    if (!mode) return fallback;
    int r = mode & 4 ? BTN_ZR : BTN_R;
    if (button == SDL_BUTTON_LEFT) return (mode & 3) == 2 ? r : BTN_X;
    if (button == SDL_BUTTON_RIGHT) return (mode & 3) == 2 ? BTN_X : r;
    if (button == SDL_BUTTON_X2) return BTN_A;
    return fallback;
}

void native_mouse_add(double dx, double dy) {
    if (!native_mouse_enabled() || native_mouse_uses_stick() || !isfinite(dx) || !isfinite(dy)) return;
    pthread_mutex_lock(&lock);
    uint64_t now = plat_now_ns();
    // A paused / unloaded player stops polling. Do not bank mouse movement for its return.
    if (frame_allowed && frame_at && now - frame_at < 100000000ull) {
        pending_x += dx; pending_y += dy;
        pending_at = now;
    }
    pthread_mutex_unlock(&lock);
}

void native_mouse_reset(void) {
    pthread_mutex_lock(&lock);
    pending_x = pending_y = frame_x = frame_y = 0;
    reset_generation++;
    pthread_mutex_unlock(&lock);
}

// Same query(5, 1) gate at 5a5ed0 used by the game's input producer.
static bool input_allowed(void) {
    uint32_t control = LD32(0x1a8ad90u);
    if (!control) return false;
    uint8_t state = LD8(control + 0x50);
    return state != 4 && state != 5 && state != 6 && state != 7 && state != 9;
}

static bool menu_open(Cpu *c) {
    uint32_t menu = LD32(0x1a991a0u);
    if (!menu) return true;
    Cpu saved = *c;
    c->r[0] = menu;
    guest_call(c, FN_MENU_OPEN);  // same sMenu query as the player's native input filters
    bool open = c->r[0] != 0;
    *c = saved;
    return open;
}

static void input_frame(Cpu *c) {
    uint32_t player = c->r[0];
    fnhook_call_original(c, FN_INPUT);
    if (!(LD8(player + 0x13cc) & 1)) return;
    update_layout(c);
    aim_player = 0;
    aim_pitch_changed = aim_yaw_changed = false;
    aim_yaw_delta = 0;
    bool blocked = menu_open(c);
    int bow_mode = 0;
    uint32_t control = LD32(0x1a8ca48u);
    if (!blocked && control && LD32(player) == VT_BOW && (LD8(player + 0x2749) & 1) && !LD8(player + 0x2716)) {
        Cpu saved = *c;
        guest_call(c, 0x85f93cu);  // original logical R mask, including the game's R/ZR swap
        bow_mode = (LD8(control + 0x6c) == 1 ? 2 : 1) | (c->r[0] == 0x800 ? 4 : 0);
        *c = saved;
    }
    uint64_t now = plat_now_ns();
    pthread_mutex_lock(&lock);
    frame_x = frame_y = 0;
    frame_allowed = !blocked && input_allowed() && LDF(player + 0x1464) <= 0 && LD32(player + 0x1470) == 0;
    bow_buttons = frame_allowed ? bow_mode : 0;
    if (frame_allowed && pending_at && now - pending_at < 100000000ull) {
        frame_x = pending_x; frame_y = pending_y;
    }
    frame_at = now;
    pending_x = pending_y = 0;
    if (seen_generation != reset_generation) {
        yaw_fraction = pitch_fraction = 0;
        seen_generation = reset_generation;
    }
    pthread_mutex_unlock(&lock);
}

static bool take(double *x, double *y) {
    pthread_mutex_lock(&lock);
    bool ok = frame_allowed && input_allowed() && frame_at && plat_now_ns() - frame_at < 100000000ull;
    *x = ok ? frame_x : 0; *y = ok ? frame_y : 0;
    frame_x = frame_y = 0;
    pthread_mutex_unlock(&lock);
    return *x != 0 || *y != 0;
}

static double mouse_scale(bool aiming) {
    uint32_t control = LD32(0x1a8ca48u);
    unsigned speed = control ? LD8(control + (aiming ? 0x6a : 0x65)) : 1;
    // Slow / Default / Fast are live mouse sensitivity multipliers on the direct
    // path. Stick compatibility keeps the game's own camera and reticle rates.
    double multiplier = speed == 0 ? 0.5 : speed == 2 ? 1.5 : 1.0;
    return degrees_per_count * config_live_mouse_sensitivity() * multiplier;
}

static int angle_delta(double counts, double scale, double *fraction) {
    double value = counts * scale * (65536.0 / 360.0) + *fraction;
    int whole = (int)trunc(value);
    *fraction = value - whole;
    return whole;
}

static unsigned inversion(uint32_t offset) {
    uint32_t control = LD32(0x1a8ca48u);
    return control ? LD8(control + offset) & 3 : 0;
}

static void camera(Cpu *c) {
    uint32_t cam = c->r[0];
    bool ordinary = c->r[14] == 0x224f00u;
    fnhook_call_original(c, FN_CAMERA);
    // 2255cc also serves Quick Aim. Only its ordinary-camera caller owns raw
    // camera counts; inject before recenter / auto pitch / terrain processing.
    if (!ordinary || LD32(cam) != VT_CAMERA || !LD32(cam + 0x85c) || LD8(cam + 0x43a) || LDF(cam + 0x1c) <= 0) return;
    double dx, dy;
    if (!take(&dx, &dy)) return;
    ST32(cam + 0xa30, LD32(cam + 0xa30) | (dx != 0 ? 2 : 0) | (dy != 0 ? 1 : 0));
    double scale = mouse_scale(false);
    unsigned flip = inversion(0x64);  // ordinary Camera Controls, independent of aiming
    int yaw = angle_delta(flip & 2 ? dx : -dx, scale, &yaw_fraction);
    int pitch = angle_delta(flip & 1 ? dy : -dy, scale, &pitch_fraction);
    int old_pitch = (int16_t)LD16(cam + 0x9e8);
    int new_pitch = old_pitch + pitch;
    if (new_pitch < -0x3f00) new_pitch = -0x3f00;
    if (new_pitch > 0x3f00) new_pitch = 0x3f00;
    pitch = new_pitch - old_pitch;
    // Move both states by the same amount: do not add joystick smoothing to mouse input.
    ST16(cam + 0x9e8, new_pitch);
    ST16(cam + 0x9e0, LD16(cam + 0x9e0) + pitch);
    ST16(cam + 0x9ec, LD16(cam + 0x9ec) + yaw);
    ST16(cam + 0x9e4, LD16(cam + 0x9e4) + yaw);
    if (trace) {
        uint32_t player = LD32(cam + 0x85c);
        LOG("[native-mouse] camera %08x player %08x vt %08x frame %llu counts %.4f %.4f delta %d %d target %d %d body %u move-yaw %u left %u/%u action %04x:%u flags %x slope %d/%d\n",
            cam, player, LD32(player), (unsigned long long)g_present_count, dx, dy, yaw, pitch,
            (int16_t)LD16(cam + 0x9ec), (int16_t)LD16(cam + 0x9e8), LD32(player + 0xfec), LD32(player + 0x14a0),
            LD16(player + 0x140c), LD16(player + 0x1410), LD16(player + 0x2506), LD16(player + 0x250a),
            LD32(cam + 0xa30), (int16_t)LD16(cam + 0xa54), (int16_t)LD16(cam + 0xa5c));
    }
}

static float pitch_limit(Cpu *c, uint32_t player, bool negative) {
    Cpu saved = *c;
    c->r[0] = player; c->r[1] = negative;
    guest_call(c, LD32(LD32(player) + 0x554));
    float result = c->v.f[0];
    *c = saved;
    return result;
}

static bool ranged_aim_active(Cpu *c, uint32_t player) {
    if (LD8(player + 0x2716)) return true;  // scope
    // Match the Quick Aim camera's original checks at 226e94. The aim setter
    // also runs while walking with a drawn bowgun, even without aiming; those
    // calls must leave raw counts for the ordinary camera.
    Cpu saved = *c;
    c->r[0] = player;
    guest_call(c, LD32(LD32(player) + 0x1c4));
    bool active = c->r[0] != 0;
    if (!active) {
        c->r[0] = player; c->r[1] = 0x1c;
        guest_call(c, 0x485e2cu);
        active = c->r[0] != 0;
    }
    *c = saved;
    return active;
}

static void aim(Cpu *c) {
    uint32_t player = c->r[0], mode = c->r[1];
    uint32_t vt = LD32(player);
    bool bowgun = mode == 0 && (vt == VT_HBG || vt == VT_LBG);
    bool bow = mode == 3 && vt == VT_BOW;
    bool ranged = (bowgun || bow) && c->r[3] == 1 && (LD8(player + 0x13cc) & 1) && ranged_aim_active(c, player);
    bool angular = mode == 1 && c->r[3] == 1 && (LD8(player + 0x13cc) & 1);
    double dx = 0, dy = 0;
    int angular_pitch = 0;
    if ((ranged || angular) && LDF(player + 0x1c) > 0 && take(&dx, &dy)) {
        double scale = mouse_scale(true);
        unsigned flip = inversion(angular || (bowgun && LD8(player + 0x2716)) ? 0x66 : 0x68);
        int yaw = angle_delta(flip & 2 ? dx : -dx, scale, &yaw_fraction);
        // Walking can replace 14a0 with camera-relative movement yaw (47f678).
        // Feeding that yaw back into a following Quick Aim camera adds the
        // movement direction again on every mouse sample. Anchor mouse turns
        // to the camera instead; keep scope and Bow's independent Type 2 offset.
        uint32_t cam = quest_camera_address;
        if (ranged && dx != 0 && LD16(player + 0x1410) && !LD8(player + 0x2716) &&
            cam && LD32(cam) == VT_CAMERA && LD32(cam + 0x85c) == player &&
            !(LD32(cam + 0xa9c) == 1 && LD32(cam + 0xaa4) == 1))
            c->r[2] = LD16(cam + 0x9ec) ^ 0x8000;
        c->r[2] = (uint16_t)(c->r[2] + yaw);
        if (!angular) aim_yaw_delta += yaw;
        if (angular) {
            angular_pitch = angle_delta(flip & 1 ? dy : -dy, scale, &pitch_fraction);
        } else if (dy != 0) {
            float old = LDF(player + 0x14b8);
            float old_limit = pitch_limit(c, player, old <= 0);
            double degrees = old * old_limit / 100.0 + (flip & 1 ? dy : -dy) * scale;
            float new_limit = pitch_limit(c, player, degrees <= 0);
            if (!isfinite(new_limit) || new_limit <= 0) FATAL("native mouse: invalid ranged pitch limit\n");
            float norm = (float)(degrees * 100.0 / new_limit);
            c->v.f[0] += (norm - old) / (LDF(player + 0x1c) * 0.5f);
        }
    }
    // Called only after the original motion / local player / entry-delay checks in 4a07a8.
    fnhook_call_original(c, FN_AIM);
    if (angular && (dx != 0 || dy != 0)) {
        // Mode 1 stores pitch directly in 2714. Apply after the original controller
        // update to avoid its integer rate * timestep quantization, keeping its +/-45deg limit.
        int pitch = (int16_t)LD16(player + 0x2714) + angular_pitch;
        if (pitch < -0x2000) pitch = -0x2000;
        if (pitch > 0x2000) pitch = 0x2000;
        ST16(player + 0x2714, pitch);
        if (trace) LOG("[native-mouse] angular-aim %08x frame %llu counts %.4f %.4f yaw %u body %u pitch %d\n",
            player, (unsigned long long)g_present_count, dx, dy, LD32(player + 0x14a0), LD32(player + 0xfec), pitch);
    }
    if (ranged) {
        if (dx != 0 || dy != 0) {
            aim_player = player;
            aim_yaw_changed |= dx != 0;
            aim_pitch_changed |= dy != 0;
        }
        // Modes 0/3 otherwise truncate the continuous pitch parameter before making the
        // camera/pose angle. Preserve bowgun's 45/30 mapping and bow's symmetric 60 mapping.
        float norm = LDF(player + 0x14b8);
        float degrees = norm * (bow ? 60.0f : norm >= 0 ? 45.0f : 30.0f) / 100.0f;
        ST16(player + 0x2714, (int)(degrees * (65536.0f / 360.0f) + 0.5f));
        if (trace && (dx != 0 || dy != 0)) LOG("[native-mouse] aim %08x vt %08x mode %u frame %llu counts %.4f %.4f yaw %u pitch %.6f view %d\n",
            player, vt, mode, (unsigned long long)g_present_count, dx, dy, LD32(player + 0x14a0), norm, (int16_t)LD16(player + 0x2714));
    }
}

static bool aim_camera_matches(uint32_t cam) {
    return aim_player && LD32(cam) == VT_CAMERA && LD32(cam + 0x85c) == aim_player && input_allowed();
}

static uint16_t camera_pitch_limit(Cpu *c, uint32_t cam) {
    Cpu saved = *c;
    c->r[0] = cam; c->r[1] = 0x34;
    guest_call(c, 0x21eadcu);
    uint16_t result = (int)(0.5f + c->v.f[0] * (65536.0f / 360.0f));
    *c = saved;
    return result;
}

static void aim_pitch(Cpu *c) {
    uint32_t cam = c->r[0];
    uint16_t old = LD16(cam + 0x9e8);
    fnhook_call_original(c, FN_AIM_PITCH);
    if (!aim_pitch_changed || !aim_camera_matches(cam)) return;
    // Quick Aim Camera Types 4/5 clamp the target later in 226e84. Apply that
    // same resource limit before moving current, so the direct update cannot overshoot it.
    if ((uint32_t)(LD32(cam + 0xaa4) - 3) < 2) {
        uint32_t upper = camera_pitch_limit(c, cam);
        uint32_t lower = 0x10000u - camera_pitch_limit(c, cam);
        uint32_t target = LD32(cam + 0x9e8);
        if ((int32_t)target < 0x3f00) {
            if ((int32_t)target > (int32_t)upper) target = upper;
        } else if ((int32_t)target < (int32_t)lower) target = lower;
        ST32(cam + 0x9e8, target);
    }
    // Keep the resource-driven up/down limits and norm-to-camera mapping from 228bbc.
    int delta = (int16_t)(LD16(cam + 0x9e8) - old);
    ST32(cam + 0x9d8, (uint16_t)(LD16(cam + 0x9d8) + delta));
    aim_pitch_changed = false;
    if (trace) LOG("[native-mouse] aim-camera pitch %d delta %d\n", (int16_t)LD16(cam + 0x9e8), delta);
}

static void aim_yaw(Cpu *c) {
    uint32_t cam = c->r[0];
    uint16_t old = LD16(cam + 0x9ec);
    fnhook_call_original(c, FN_AIM_YAW);
    if (!aim_yaw_changed || !aim_camera_matches(cam)) return;
    // Bow Quick Aim Camera Type 2 explicitly keeps the original camera offset.
    // Its original yaw follower exits early; direct mouse input must retain that offset.
    bool independent = LD32(cam + 0xa9c) == 1 && LD32(cam + 0xaa4) == 1;
    uint16_t target = independent ? (uint16_t)(LD16(cam + 0x9ec) + aim_yaw_delta) :
                                   LD32(aim_player + 0x14a0) ^ 0x8000;
    int delta = (int16_t)(target - old);
    ST32(cam + 0x9ec, target);
    ST32(cam + 0x9dc, (uint16_t)(LD16(cam + 0x9dc) + delta));
    aim_yaw_changed = false;
    if (trace) LOG("[native-mouse] aim-camera yaw %d delta %d\n", (int16_t)target, delta);
}

static void log_camera_mode(Cpu *c, uint32_t cam) {
    uint32_t state[16] = {cam, LD32(cam)};
    bool quest = state[1] == VT_CAMERA;
    if (!quest && state[1] != 0x1930e10u) return;
    if (quest) {
        state[2] = LD8(cam + 0xa98); state[3] = LD32(cam + 0xa9c);
        state[4] = LD8(cam + 0xacc); state[5] = LD8(cam + 0xac8);
        state[6] = LD8(cam + 0x470); state[7] = LD8(cam + 0x230) | LD8(cam + 0x2f0) << 8;
        state[8] = LD8(cam + 0x43a); state[9] = LD32(cam + 0xaa0); state[10] = LD32(cam + 0xaa4);
    }
    state[11] = menu_open(c);
    uint32_t control = LD32(0x1a8ca48u);
    if (control) for (unsigned i = 0; i < 4; i++) state[12 + i] = LD32(control + 0x63 + 4 * i);
    static uint32_t previous[16];
    if (!memcmp(state, previous, sizeof state)) return;
    memcpy(previous, state, sizeof state);
    LOG("[native-mouse] camera-mode frame %llu cam %08x vt %08x aim %u/%u scope %u/%u special %u event %04x track %u bowgun %u quick-camera %u menu %u options63 %08x %08x %08x %08x\n",
        (unsigned long long)g_present_count, cam, state[1], state[2], state[3], state[4], state[5], state[6], state[7], state[8],
        state[9], state[10], state[11], state[12], state[13], state[14], state[15]);
}

static void quest_camera(Cpu *c) {
    uint32_t cam = c->r[0];
    quest_camera_address = cam;
    fnhook_call_original(c, FN_QUEST_CAMERA);
    bool compatibility = (LD8(cam + 0xa98) && LD32(cam + 0xa9c) >= 2) ||
                         (LD8(cam + 0xacc) && LD8(cam + 0xac8) >= 2);
    __atomic_store_n(&use_stick, compatibility, __ATOMIC_RELEASE);
    __atomic_store_n(&stick_aiming, compatibility, __ATOMIC_RELEASE);
    if (trace) {
        log_camera_mode(c, cam);
        uint32_t player = LD32(cam + 0x85c);
        if (player && native_mouse_layout() == KEYBOARD_RANGED &&
            (LD8(cam + 0xa98) || LD8(cam + 0xacc)))
            LOG("[native-mouse] aim-state frame %llu yaw %u body %u target %u current %u left %u/%u aim %u/%u scope %u/%u action %04x:%u\n",
                (unsigned long long)g_present_count, LD32(player + 0x14a0), LD32(player + 0xfec),
                LD16(cam + 0x9ec), LD16(cam + 0x9dc), LD16(player + 0x140c), LD16(player + 0x1410),
                LD8(cam + 0xa98), LD32(cam + 0xa9c), LD8(cam + 0xacc), LD8(cam + 0xac8),
                LD16(player + 0x2506), LD16(player + 0x250a));
    }
}

static void village_camera(Cpu *c) {
    quest_camera_address = 0;
    uint32_t cam = c->r[0];
    fnhook_call_original(c, FN_VILLAGE_CAMERA);
    if (LD32(cam + 0x85c)) update_layout(c);
    __atomic_store_n(&use_stick, 1, __ATOMIC_RELEASE);
    __atomic_store_n(&stick_aiming, 0, __ATOMIC_RELEASE);
    if (trace) log_camera_mode(c, cam);
}

static void shell_angles(Cpu *c) {
    uint32_t shell = c->r[0], player = c->r[1];
    fnhook_call_original(c, FN_SHELL_ANGLES);
    if (LD8(player + 0x13cc) & 1)
        LOG("[native-mouse] shell %08x frame %llu pitch %d yaw %u player-yaw %u pitch-norm %.6f\n",
            shell, (unsigned long long)g_present_count, (int16_t)LD32(shell + 0xfe8), LD32(shell + 0xfec),
            LD32(player + 0x14a0), LDF(player + 0x14b8));
}

static void arrow_setup(Cpu *c) {
    uint32_t shell = c->r[0];
    fnhook_call_original(c, FN_ARROW_SETUP);
    Cpu saved = *c;
    c->r[0] = shell;
    guest_call(c, 0x6a1b6cu);  // original typed owner-player getter (setup's r1 is not a player)
    uint32_t player = c->r[0];
    *c = saved;
    if (player && (LD8(player + 0x13cc) & 1))
        LOG("[native-mouse] arrow %08x frame %llu pitch %d yaw %u player-yaw %u pitch-norm %.6f velocity %.6f %.6f %.6f\n",
            shell, (unsigned long long)g_present_count, (int16_t)LD32(shell + 0x15c0), LD32(shell + 0x15c4),
            LD32(player + 0x14a0), LDF(player + 0x14b8), LDF(shell + 0x1010), LDF(shell + 0x1014), LDF(shell + 0x1018));
}

static void target_camera(Cpu *c) {
    uint32_t cam = c->r[0], target = c->r[1];
    uint16_t pitch = LD16(cam + 0x9e8), yaw = LD16(cam + 0x9ec);
    fnhook_call_original(c, FN_TARGET_CAMERA);
    uint32_t control = LD32(0x1a8ca48u);
    LOG("[native-mouse] target-camera frame %llu cam %08x target %08x options %u/%u event %u flags %x pitch %d -> %d yaw %u -> %u\n",
        (unsigned long long)g_present_count, cam, target, LD8(control + 0x6f), LD8(control + 0x70),
        LD8(cam + 0xa7c), LD32(cam + 0xa30), (int16_t)pitch, (int16_t)LD16(cam + 0x9e8),
        yaw, LD16(cam + 0x9ec));
}

void native_mouse_init(void) {
    if (g_config.input_mode != INPUT_KEYBOARD_MOUSE) return;
    const char *e = getenv("MHGU_NATIVE_MOUSE");
    if (e && !strcmp(e, "0")) return;  // diagnostic control run only
    // These function entries belong to the supported Switch GU 1.4.0 dump.
    if (LD32(FN_INPUT) != 0xe30113cc || LD32(FN_CAMERA) != 0xe92d4ff0 || LD32(FN_AIM) != 0xe92d4830 ||
        LD32(FN_AIM_PITCH) != 0xe92d4070 || LD32(FN_AIM_YAW) != 0xe92d4010)
        FATAL("native mouse: unsupported game function entries\n");
    e = getenv("MHGU_NATIVE_MOUSE_DEG_PER_COUNT");
    if (e) {
        char *end;
        degrees_per_count = strtod(e, &end);
        if (*end || !isfinite(degrees_per_count) || degrees_per_count <= 0 || degrees_per_count > 10)
            FATAL("native mouse: invalid MHGU_NATIVE_MOUSE_DEG_PER_COUNT\n");
    }
    trace = getenv("MHGU_NATIVE_MOUSE_LOG") != NULL;
    if (!fnhook_set(FN_INPUT, hle_register("native-mouse:input", input_frame)) ||
        !fnhook_set(FN_CAMERA, hle_register("native-mouse:camera", camera)) ||
        !fnhook_set(FN_AIM, hle_register("native-mouse:aim", aim)) ||
        !fnhook_set(FN_AIM_PITCH, hle_register("native-mouse:aim-pitch", aim_pitch)) ||
        !fnhook_set(FN_AIM_YAW, hle_register("native-mouse:aim-yaw", aim_yaw)))
        FATAL("native mouse: unable to install input hooks\n");
    if (trace && !fnhook_set(FN_SHELL_ANGLES, hle_register("native-mouse:shell-log", shell_angles)))
        FATAL("native mouse: unable to install shell observation hook\n");
    if (trace && !fnhook_set(FN_ARROW_SETUP, hle_register("native-mouse:arrow-log", arrow_setup)))
        FATAL("native mouse: unable to install arrow observation hook\n");
    if (trace && !fnhook_set(FN_TARGET_CAMERA, hle_register("native-mouse:target-log", target_camera)))
        FATAL("native mouse: unable to install target camera observation hook\n");
    if (!fnhook_set(FN_QUEST_CAMERA, hle_register("native-mouse:quest-mode", quest_camera)) ||
        !fnhook_set(FN_VILLAGE_CAMERA, hle_register("native-mouse:village-mode", village_camera)))
        FATAL("native mouse: unable to install camera mode routing hooks\n");
    __atomic_store_n(&enabled, 1, __ATOMIC_RELEASE);
    LOG("[native-mouse] Switch GU direct quest camera + HBG/LBG/Bow aim, %g degrees/count times MouseSensitivity\n", degrees_per_count);
}
