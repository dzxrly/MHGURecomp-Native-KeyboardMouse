#pragma once
#include "vk.h"

// Render-thread copy. Guest allocations retain their original dimensions;
// host render targets and viewports are resized together at a frame boundary.
extern Config g_graphics;
extern uint32_t g_graphics_render[2], g_graphics_output[2];
void vk_graphics_init(void);
void vk_graphics_apply(void);
void vk_graphics_target_size(const NvnTexture *t, uint32_t *w, uint32_t *h);
void vk_prewarm_apply(void);
