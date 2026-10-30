// Only the generated PC prompt layer: live Melee/Ranged text and the six known input atlases.
#pragma once
#include <stdbool.h>
#include <stdint.h>

struct NvnTexture;
void keyboard_mouse_prompts_init(void);
const uint8_t *keyboard_mouse_prompts_texture(struct NvnTexture *texture, const uint8_t *original,
                                             uint64_t size, bool *changed);
void keyboard_mouse_prompts_texture_destroyed(struct NvnTexture *texture);
