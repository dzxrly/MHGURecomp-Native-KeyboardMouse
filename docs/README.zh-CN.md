<h1 align="center">MHGURecomp 原生键鼠输入</h1>

<p align="center">
  <a href="../README.md">English</a> · <strong>简体中文</strong> · <a href="README.zh-TW.md">繁體中文</a>
</p>

这是 **Monster Hunter Generations Ultimate** 的原生 PC 移植，针对 Nintendo Switch **1.4.0** 版本。游戏的 ARM 代码静态重编译为 C，由宿主运行时实现 Switch 服务，并通过 Vulkan 渲染。

本仓库 fork 自 [morerokk/MHGURecomp](https://github.com/morerokk/MHGURecomp)，以**原生键鼠输入优化**为主要方向独立开发，并定期手动同步原仓库的部分改动。

**仓库不包含原始游戏导出、重编译生成的游戏源码或密钥。** 仓库包含预生成的 PC 提示 mod；运行游戏仍需使用自己的游戏导出。

## 功能

- 普通任务镜头、弩／弓瞄准和望远镜使用直接鼠标角度输入，保留细微移动。
- 根据实际装备自动切换近战／远程键位及提示文字、按钮图标；猎猫使用近战预设。
- 游戏菜单的视角移动速度、准星移动速度直接调节对应鼠标灵敏度，即时生效。
- 保留射击硬直、入瞄准延迟、招式转向窗口、俯仰范围及原有镜头行为。
- 按键提示覆盖游戏的七种语言：英语、简体中文、繁体中文、法语、德语、意大利语、西班牙语。
- 分辨率、宽高比、MSAA／FXAA、FSR、VSync、30～240FPS 设置，手柄支持和 F1 设置面板。
- 保留原项目的本地联机、可选怪物血量倍率、村任务联机、模组和金手指支持。

原生鼠标实现只针对上述 Switch 可执行文件；其他游戏构建及 3DS 版不在当前适配范围内。

## 运行要求

所有启动入口都会在加载游戏代码前，核对支持的 Switch MHGU 1.4.0 的 main／sdk 完整 Build ID，以及部分基础 romfs 资源指纹。不匹配时提示具体文件并停止启动。资源指纹不等于整套 ROM 的完整性校验。保留原始合并 romfs，修改放在 `mods/`；`mhgurecomp --check-rom <游戏目录>` 只检查，不启动游戏。

在村庄或任务中按 `/` 打开键盘聊天，Enter 发送、Esc 取消；可在 F1 设置修改 `Controls.ChatKey`，已被键鼠操作占用的按键会标出。`Anti-aliasing.FXAAMode = 3_11` 可选择 FXAA 3.11。

- Windows 10／11 64位。**本项目当前没有 Linux 构建与测试环境，因此暂不兼容 Linux，也不提供 Linux 发布包。**
- 支持 AVX2 的处理器、支持 Vulkan 1.2 的显卡，至少1GB显存、4GB内存。
- 自己导出的完整游戏文件，**将1.4.0更新合并到本体资源中**。
- Title ID：`0100770008DD8000`；`main` 的 Build ID 以 `FB08F1D20FD1204F` 开头。

本项目仅针对 Windows，原生输入的实际游戏测试也在 Windows 下进行。

## 安装

预编译程序以[本仓库的 Releases](https://github.com/dzxrly/MHGURecomp-Native-KeyboardMouse/releases)为准。没有安装包时，请[从源码构建](#从源码构建)。原仓库的程序不包含本仓库的原生输入改动。

1. 将程序解压到自选目录。
2. 把游戏的 `exefs`、`romfs` 放入 `data`：

   ```text
   MHGURecomp/
     mhgurecomp.exe
     mhgugame.dll
     （其他运行库）
     data/
       exefs/
         main
         main.npdm
         rtld
         sdk
         subsdk0
       romfs/
         nativeNX/
         ...
   ```

3. 启动 `mhgurecomp.exe`，首次启动会生成 `mhgurecomp.ini`。
4. 按下文启用键鼠模式并安装自动提示包。

`exefs` 必须来自更新；`romfs` 必须是本体叠加更新后的完整资源，约35,500个文件、12.3GB。缺失、版本不符或资源不完整时，程序会提示错误。

## 键鼠配置

### 启用与基础设置

修改 `mhgurecomp.ini`，然后重启一次：

   ```ini
   [Controls]
   InputMode = Keyboard_Mouse_Input

   [Mouse]
   MouseSensitivity = 1.0
   ```

源码默认仍为 `InputMode = Original`。两种模式均可使用手柄。键鼠模式下，轻弩、重弩、弓自动使用**远程 Ranged**，其余猎人武器和猎猫自动使用**近战 Melee**。更换装备后按键和提示自动切换，无需重启；旧的 `KeyboardLayout` 不再生效。

### 安装自动提示包

Release 包已包含 `mods/Keyboard_Mouse_Input_Auto`。保留该目录，在 F1 设置的 **Mods** 页启用后重启即可，无需另行下载或生成资源。

   ```text
   mods/
     Keyboard_Mouse_Input_Auto/
       romfs/
         keyboard_mouse_input.flag
         keyboard_mouse_prompts.bin
         nativeNX/
   ```

保留两个标记／数据文件。一份 Auto 包包含两套预设，旧的 Melee／Ranged 单预设包会跳过；Original 模式也会跳过 Auto 包。文字和图标跟随装备与游戏语言切换；在 Mods 页禁用的包不会自行启用。

### 按键表

| 输入 | 近战 Melee | 远程 Ranged |
| --- | --- | --- |
| WASD | 左摇杆：移动 | 左摇杆：移动 |
| 鼠标移动 | 任务视角 | 任务视角／武器瞄准 |
| 鼠标左键 | X：主攻击／拔刀 | A：射击、确认／交互 |
| 鼠标右键 | A：副攻击、确认／交互 | R：瞄准 |
| R | R：防御／武器特殊动作 | X：装填／拔刀 |
| 鼠标侧键5 | R：防御／武器特殊动作 | 未绑定 |
| F／鼠标侧键4 | Y：收刀／使用道具 | Y：收刀／使用道具 |
| E | Y：收刀／使用道具 | R：瞄准 |
| 左 Shift | R：通用奔跑 | R：通用奔跑 |
| Space | B：闪避、取消／返回 | B：闪避、取消／返回 |
| C | X+A：组合攻击 | X+A：组合动作 |
| 滚轮上／下 | L+Y／L+A：选择道具 | L+X／L+B：选择弹药或瓶 |
| Q | L：道具栏／镜头操作 | L：道具栏／镜头操作 |
| Esc／Enter／小键盘 Enter | ＋：游戏菜单 | ＋：游戏菜单 |
| Backspace | －：减号菜单 | －：减号菜单 |
| Tab／鼠标中键 | ZL：切换目标（操作类型1） | ZL：切换目标（操作类型1） |
| T | ZR | ZR |
| 1／2／3 | 方向上／右／下：狩技（操作类型1） | 方向上／右／下：狩技（操作类型1） |
| 方向键 | 十字方向键 | 十字方向键 |
| Ctrl／Alt | 按下左／右摇杆 | 按下左／右摇杆 |
| F1／F2 | PC 设置／释放鼠标 | PC 设置／释放鼠标 |

按键按 QWERTY 键盘的物理位置读取。点击窗口可重新捕获鼠标。输入文字时 Enter 确认、Esc 取消、Ctrl+V 粘贴。游戏菜单中近战用右键确认，远程用左键确认，Space 返回。预设切换会释放按住状态并清空滚轮队列。

### 弓与猎猫

弓自动使用远程预设。收弓时左键保留 A 确认／交互，按 R（游戏 X）拔弓；拔弓后：

| 输入 | 弓操作类型1 | 弓操作类型2 |
| --- | --- | --- |
| 鼠标左键 | X：蓄力，松开发射 | 游戏设置对应的 R：蓄力，松开发射 |
| 鼠标右键 | 游戏设置对应的 R：按住瞄准 | X：点击切换瞄准 |
| 鼠标侧键5 | A：弓副动作 | A：弓副动作 |

游戏的 R/ZR 互换设置参与鼠标映射，键盘仍按基础表工作。打开菜单后左键恢复为 A；松开鼠标会释放按下时对应的按钮。目前弓的两种操作类型仍共用基础远程提示，鼠标操作以上表为准。

猎猫使用近战预设：左键 X 近战，右键 A 回旋镖；按住右键蓄力时，再按住 R／侧键5可精瞄。拔武器时游戏 R 防御、R+X 上击；收武器时 R+X+A 防御。滚轮选择支援行动，收武器时 F 使用，拔武器时 R+F 使用。原有资格、消耗和动作限制继续生效。

### 鼠标灵敏度与镜头选项

`MouseSensitivity` 范围为0.1～10，作为基础灵敏度倍率。直接输入使用鼠标相对位移计数，而非屏幕像素；鼠标 DPI 会影响同一物理移动产生的计数。程序不额外加入鼠标加速曲线。

| 游戏菜单选项 | 对应直接输入 | 缓慢 | 标准 | 快速 |
| --- | --- | --- | --- | --- |
| 视角移动速度 | 普通任务镜头 | 0.5倍 | 1倍 | 1.5倍 |
| 准星移动速度 | 弩／弓瞄准及望远镜 | 0.5倍 | 1倍 | 1.5倍 |

两项独立即时生效。`MouseSensitivity = 1` 时，水平移动1000计数约对应20°／40°／60°。普通视角、快速瞄准和第一人称／望远镜分别遵循各自的反转设置；五种瞄准视角类型、初始镜头位置、目标视角和地形调整保留原有行为。

动作屏蔽期间的鼠标位移会丢弃，恢复后不会补转。转动镜头也不会强制让正在出招的近战角色超出游戏允许窗口转向。

村庄、固定弩炮和猎猫特殊瞄准保留鼠标转摇杆兼容路径，仅这些场景使用 `[MouseConversion]`：

   ```ini
   [MouseConversion]
   MinWindowMs = 8.0
   FullStickSpeed = 1500.0
   DeadzoneCompensation = 0.40
   AimDeadzoneCompensation = 0.50
   LowSpeedCutoff = 0.0
   FrameTimeSmoothing = 0.25
   ```

修改这些参数后重启。`FullStickSpeed` 表示灵敏度为1时达到满摇杆所需的鼠标速度，单位为计数／秒；两项 Compensation 分别补偿普通／瞄准死区；`MinWindowMs` 控制最小采样窗口；`LowSpeedCutoff` 过滤极慢移动；`FrameTimeSmoothing` 调整帧间隔估计。兼容镜头仍受游戏摇杆转速限制。

### 已验证与待完善

实际游戏验证包括弩／弓瞄准和弹道方向、弓的两种操作类型、镜头菜单选项、自动换装预设、猎猫回旋镖精瞄／防御、操虫棍瞄准／发出猎虫和固定弩炮兼容瞄准。另检查了太刀移动／拔刀方向、长枪防御／突刺、盾斧防御；这不代表全部武器、风格和动作均已验证。

仍需完善弓专属提示、装备支援行动后的猎猫实际施放、可变倍率，以及更多武器／风格／弹种和帧率组合。不同帧率的鼠标采样单元测试不能代替原生角度实测。游戏手册中的整页手柄插图尚未替换。

0.11.3 的 Windows 补测已覆盖实际鼠标按钮触发的大剑拔刀／攻击／防御，以及片手剑攻击、X+A 和防御；大剑蓄力等级和完整连段仍待验证。普通镜头在所选 30／120／240FPS 设置下，1000 计数的角度变化一致；重弩在 30 和 240 设置下另检查了直接瞄准、实际弹方向及射击屏蔽期间的输入丢弃。这些设置不等于持续达到对应帧率，也不代表全部武器／风格均已覆盖。

`InputMode = Original` 保留原映射：WASD 移动，鼠标转镜头，左右键 X／A，Space／R 为 B／Y，Q／E 为 L／R，Tab／T 为 ZL／ZR，Enter／Backspace 为＋／－；该模式下 Esc 释放鼠标。

## 设置与文件位置

F1 打开 PC 设置；Alt+Enter 或 F11 切换全屏，F12 截图，F7 切换 MSAA。PC 设置界面目前支持英语／西班牙语，与游戏的七种语言独立。

`mhgurecomp.ini` 内附各项说明，包括显示／渲染分辨率、MSAA／FXAA、FSR 3.1／4及质量、绘制距离、FPS／VSync、控制、鼠标、网络、怪物血量、村任务、额外内容和音频。启用 FSR 时由它接管抗锯齿。详见[设置说明](technical/SETTINGS_UI.md)、[FSR 3.1](technical/FSR-PORT.md)、[FSR 4](technical/FSR4-PORT.md)。

| 内容 | 位置 |
| --- | --- |
| 配置／日志 | 程序旁的 `mhgurecomp.ini`／`log.txt` |
| Windows 存档 | `%APPDATA%\MHGURecomp\save\save_0\` |
| 模组／金手指 | 程序旁的 `mods/`／`cheats/` |
| 截图／着色器缓存 | 程序旁的 `shots/`／`cache/` |

导入自己的 Switch 存档时，将 `system`、`system_backup` 复制到 `save_0`，覆盖前备份已有存档。GitHub 仓库改名不改变程序文件名或存档位置。

## 联机、模组与金手指

使用游戏的**本地无线通信**选项。局域网／虚拟局域网可直接发现房间；互联网联机需要兼容的程序版本，在 `[Multiplayer] Server` 设置房间服务器，用相同 `Key` 限定好友发现。这里不保证原项目公共服务器持续可用，也不保证不同构建间互通。自建方法见 [server/README.md](../server/README.md)。不使用中继时，主机需开放端口（默认 TCP／UDP 39500），客户端配置 `Join` 地址。

每个模组放入独立的 `mods/` 子目录，在 F1 → Mods 启用、排序并重启。支持 RomFS 替换、ARC 条目合并和 Forge 插件；冲突条目以后加载者为准。Insert 打开 Forge；不支持的代码补丁会写入日志。运行时已有的分辨率、帧率功能无需另装修复模组。

将适用于1.4.0的金手指 `.txt` 放入 `cheats/`，每项以 `[名称]` 开头，启动时加载；文件名前加 `_` 可禁用。`extras/cheats/` 的可选代码说明见[技能点](technical/research/skill-points.md)和[勇气收刀](technical/research/valor-sheathe.md)。

若发行包提供 `bonus/system` 且 `BonusContent = On`，程序可在未存在时导入免费的活动任务／道具包／随从内容；设为 Off 可关闭。

## 更新与原仓库同步

从**本仓库**下载新版，替换程序文件并保留游戏数据、设置、模组和存档。本仓库构建的更新器也只查询本仓库发布，不安装原项目版本；没有已发布版本或网络不可用时会启动当前游戏。`0.11.2-13-g0641c27` 这类开发版本不会被自动替换成标签发行版。

本项目会定期查看[原仓库](https://github.com/morerokk/MHGURecomp)，手动选择并整合部分改动，功能和版本号不要求同步。可保留指向原仓库的 `upstream` 远端，用于这项工作。

## 从源码构建

克隆本仓库，安装编译工具和 Python 依赖，然后使用自己的游戏文件构建：

   ```sh
   git clone https://github.com/dzxrly/MHGURecomp-Native-KeyboardMouse.git
   cd MHGURecomp-Native-KeyboardMouse
   make gen GAME=/path/to/game
   make -j4 GAME=/path/to/game
   ```

完整的 [Windows 构建及打包说明](technical/BUILDING.md)包含 MSYS2 软件包、Python、FFmpeg 和输出目录。程序仍使用 `mhgurecomp`、`mhgugame` 文件名。

现有输入与配置检查：

   ```sh
   make test-settings test-keyboard-mouse test-settings-locale test-keyboard-mouse-assets
   ```

## 源码结构与许可

| 目录 | 用途 |
| --- | --- |
| `runtime/` | 宿主运行时、原生鼠标、自动预设、图形、网络和设置 |
| `tools/` | 重编译器和本地提示包生成工具 |
| `updater/`／`server/` | 本仓库更新器／可选房间服务器 |
| `docs/` | 其他语言 README；`technical/` 保存实现和构建说明 |
| `extras/` | PC 提示素材、资源工具说明和可选金手指 |

项目代码采用 MIT 许可，保留原项目署名和第三方声明，见 [LICENSE](../LICENSE) 与 [THIRD_PARTY_NOTICES](../THIRD_PARTY_NOTICES)。许可不覆盖游戏或生成的游戏代码。本项目与 Capcom、Nintendo 无关联。
