<h1 align="center">键鼠提示资源（开发）</h1>

[English](README.en.md) · [开发文档](../../docs/technical/README.md)

用户只需在发布包中启用 Auto 提示 mod，操作见[用户指南](../../docs/README.zh-CN.md#键鼠配置)。

本目录供维护者使用，保存开发素材、映射，以及 `prebuilt/Keyboard_Mouse_Input_Auto` 中随发布包分发的固定资源。打包直接使用该预生成目录；修改资源时保留 `romfs/keyboard_mouse_input.flag` 和 `romfs/keyboard_mouse_prompts.bin`。

实现说明：[原生输入与自动提示](../../docs/technical/ARCHITECTURE.md#input-flow)。素材提示词：[imagegen-prompts.txt](imagegen-prompts.txt)。
