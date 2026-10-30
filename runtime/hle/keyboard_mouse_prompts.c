// Generated Auto prompts keep both keyboard profiles resident. The game keeps its original
// message count/indices; only its message getter selects the appended Melee text pool.
// Atlas uploads select an exact generated BC3 variant without changing guest memory or NVN objects.
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "keyboard_mouse_prompts.h"
#include "../config.h"
#include "../hle.h"
#include "../mods.h"
#include "../native_mouse.h"
#include "../nvn/nvn.h"

#define FN_MESSAGE_LOAD 0xd839a8u
#define FN_MESSAGE_GET  0xd84114u
#define PROFILE_BUCKETS 256u
static const char text_marker[] = "MHGU_PC_PROMPTS_V1";
static bool enabled;

typedef struct MessageProfile {
    uint32_t resource, pool, count;
    uint32_t *melee;
    struct MessageProfile *next;
} MessageProfile;
static MessageProfile *messages[PROFILE_BUCKETS];
static pthread_mutex_t message_lock = PTHREAD_MUTEX_INITIALIZER;

static unsigned bucket(uintptr_t key) { return (unsigned)((key >> 4) ^ (key >> 12)) & (PROFILE_BUCKETS - 1); }

static void message_load(Cpu *c) {
    uint32_t resource = c->r[0];
    fnhook_call_original(c, FN_MESSAGE_LOAD);
    unsigned slot = bucket(resource);
    pthread_mutex_lock(&message_lock);
    MessageProfile **old = &messages[slot];
    while (*old && (*old)->resource != resource) old = &(*old)->next;
    if (*old) {
        MessageProfile *p = *old;
        *old = p->next;
        free(p->melee);
        free(p);
    }
    pthread_mutex_unlock(&message_lock);
    if (!(c->r[0] & 1)) return;
    uint32_t count = LD32(resource + 0x88), size = LD32(resource + 0x90);
    uint32_t pool = LD32(resource + 0x94), table = LD32(resource + 0x98);
    if (!count || !size || !pool || !table) return;
    uint64_t end = (uint64_t)pool + size;
    uint32_t last = LD32(table + (count - 1) * 4);
    if (end > UINT32_MAX || last < pool || last >= end) return;
    const char *last_text = G2H(last);
    const char *last_end = memchr(last_text, 0, (size_t)(end - last));
    if (!last_end) return;
    uint32_t tail = last + (uint32_t)(last_end - last_text) + 1;
    if (end - tail < sizeof text_marker || memcmp(G2H(tail), text_marker, sizeof text_marker)) return;
    uint32_t *melee = malloc((size_t)count * sizeof *melee);
    if (!melee) FATAL("PC prompts: cannot allocate the message profile\n");
    uint32_t pos = tail + sizeof text_marker;
    for (uint32_t i = 0; i < count; i++) {
        const char *text = G2H(pos);
        const char *stop = pos < end ? memchr(text, 0, (size_t)(end - pos)) : NULL;
        if (!stop) FATAL("PC prompts: malformed appended message pool\n");
        melee[i] = pos;
        pos += (uint32_t)(stop - text) + 1;
    }
    if (pos != end) FATAL("PC prompts: unexpected bytes after the appended message pool\n");
    MessageProfile *p = malloc(sizeof *p);
    if (!p) FATAL("PC prompts: cannot allocate the message profile\n");
    *p = (MessageProfile){resource, pool, count, melee, NULL};
    pthread_mutex_lock(&message_lock);
    p->next = messages[slot];
    messages[slot] = p;
    pthread_mutex_unlock(&message_lock);
}

static void message_get(Cpu *c) {
    uint32_t resource = c->r[0], index = c->r[1];
    fnhook_call_original(c, FN_MESSAGE_GET);
    if (native_mouse_layout() != KEYBOARD_MELEE) return;
    pthread_mutex_lock(&message_lock);
    MessageProfile *p = messages[bucket(resource)];
    while (p && p->resource != resource) p = p->next;
    if (p && index < p->count && p->pool == LD32(resource + 0x94) && p->count == LD32(resource + 0x88))
        c->r[0] = p->melee[index];
    pthread_mutex_unlock(&message_lock);
}

typedef struct {
    uint32_t kind, width, height, size;
    uint8_t *ranged, *melee;
} AtlasProfile;
typedef struct TextureProfile {
    NvnTexture *texture;
    AtlasProfile *atlas;
    int layout;
    struct TextureProfile *next;
} TextureProfile;
static AtlasProfile *atlases;
static uint32_t atlas_count;
static bool pack_loaded;
static TextureProfile *textures[PROFILE_BUCKETS];
static pthread_mutex_t texture_lock = PTHREAD_MUTEX_INITIALIZER;

static void read_exact(FILE *f, void *data, size_t size) {
    if (fread(data, 1, size, f) != size) FATAL("PC prompts: truncated atlas pack\n");
}
static uint32_t read_word(FILE *f) {
    uint8_t b[4];
    read_exact(f, b, sizeof b);
    return (uint32_t)b[0] | (uint32_t)b[1] << 8 | (uint32_t)b[2] << 16 | (uint32_t)b[3] << 24;
}
static bool load_pack(void) {
    if (pack_loaded) return true;
    const char *path = mods_keyboard_mouse_prompt_pack();
    if (!path) return false;  // the RomFS is mounted before the first resource is loaded
    FILE *f = fopen(path, "rb");
    if (!f) FATAL("PC prompts: cannot read generated Auto atlas pack %s\n", path);
    char magic[8];
    read_exact(f, magic, sizeof magic);
    uint32_t count = read_word(f);
    if (memcmp(magic, "MHGUPC01", sizeof magic) || !count || count > 256)
        FATAL("PC prompts: invalid Auto atlas pack header\n");
    atlases = calloc(count, sizeof *atlases);
    if (!atlases) FATAL("PC prompts: cannot allocate the atlas profiles\n");
    for (uint32_t i = 0; i < count; i++) {
        AtlasProfile *a = &atlases[i];
        a->kind = read_word(f);
        a->width = read_word(f);
        a->height = read_word(f);
        a->size = read_word(f);
        uint32_t patches = read_word(f);
        uint32_t dim = a->kind == 0 ? 256 : a->kind < 5 ? 512 : 1024;
        if (a->kind >= 6 || a->width != dim || a->height != dim || a->size != dim * dim ||
            !patches || patches > a->size / 16)
            FATAL("PC prompts: invalid known-atlas profile\n");
        a->ranged = malloc(a->size);
        a->melee = malloc(a->size);
        if (!a->ranged || !a->melee) FATAL("PC prompts: cannot allocate the atlas profiles\n");
        read_exact(f, a->ranged, a->size);
        memcpy(a->melee, a->ranged, a->size);
        uint32_t previous = 0;
        for (uint32_t j = 0; j < patches; j++) {
            uint32_t off = read_word(f);
            if ((off & 15) || off > a->size - 16 || (j && off <= previous))
                FATAL("PC prompts: invalid atlas block offset\n");
            read_exact(f, a->melee + off, 16);
            previous = off;
        }
    }
    if (fgetc(f) != EOF || ferror(f)) FATAL("PC prompts: unexpected bytes after the atlas pack\n");
    fclose(f);
    atlas_count = count;
    pack_loaded = true;
    LOG("[pc-prompts] %u exact atlas variants; text and icons follow the equipped weapon\n", count);
    return true;
}

const uint8_t *keyboard_mouse_prompts_texture(NvnTexture *texture, const uint8_t *original,
                                             uint64_t size, bool *changed) {
    *changed = false;
    if (!enabled || (texture->format != 0x44 && texture->format != 0x48) || texture->levels != 1 ||
        texture->width != texture->height || size != (uint64_t)texture->width * texture->height ||
        (texture->width != 256 && texture->width != 512 && texture->width != 1024)) return original;
    pthread_mutex_lock(&texture_lock);
    if (!load_pack()) { pthread_mutex_unlock(&texture_lock); return original; }
    unsigned slot = bucket((uintptr_t)texture);
    TextureProfile *p = textures[slot];
    while (p && p->texture != texture) p = p->next;
    if (!p) {
        p = calloc(1, sizeof *p);
        if (!p) FATAL("PC prompts: cannot allocate a texture profile\n");
        p->texture = texture;
        p->layout = -1;
        for (uint32_t i = 0; i < atlas_count; i++) {
            AtlasProfile *a = &atlases[i];
            if (a->size == size && a->width == texture->width && !memcmp(original, a->ranged, a->size)) {
                p->atlas = a;
                LOG("[pc-prompts] matched atlas kind %u, %ux%u, texture %llu\n", a->kind, a->width, a->height,
                    (unsigned long long)texture->debug_id);
                break;
            }
        }
        p->next = textures[slot];
        textures[slot] = p;
    }
    const uint8_t *result = original;
    if (p->atlas) {
        int layout = native_mouse_layout();
        *changed = p->layout != layout;
        p->layout = layout;
        result = layout == KEYBOARD_MELEE ? p->atlas->melee : p->atlas->ranged;
    }
    pthread_mutex_unlock(&texture_lock);
    return result;
}

void keyboard_mouse_prompts_texture_destroyed(NvnTexture *texture) {
    if (!enabled) return;
    pthread_mutex_lock(&texture_lock);
    TextureProfile **p = &textures[bucket((uintptr_t)texture)];
    while (*p && (*p)->texture != texture) p = &(*p)->next;
    if (*p) {
        TextureProfile *old = *p;
        *p = old->next;
        free(old);
    }
    pthread_mutex_unlock(&texture_lock);
}

void keyboard_mouse_prompts_init(void) {
    if (g_config.input_mode != INPUT_KEYBOARD_MOUSE) return;
    if (LD32(FN_MESSAGE_LOAD) != 0xe92d41f0 || LD32(FN_MESSAGE_GET) != 0xe5902088)
        FATAL("PC prompts: unsupported Switch GU message functions\n");
    if (!fnhook_set(FN_MESSAGE_LOAD, hle_register("pc-prompts:message-load", message_load)) ||
        !fnhook_set(FN_MESSAGE_GET, hle_register("pc-prompts:message-get", message_get)))
        FATAL("PC prompts: cannot install the automatic message hooks\n");
    enabled = true;
}
