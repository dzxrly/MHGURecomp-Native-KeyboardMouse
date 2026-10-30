# 键鼠操作

[English](README.en.md)

提供近战和远程两套键位，并替换游戏中的按钮图标和按键说明。支持简体中文、繁体中文、英语、法语、德语、意大利语和西班牙语。

## 启用

编辑游戏目录下的 `mhgurecomp.ini`：

```ini
[Controls]
InputMode = Keyboard_Mouse_Input
KeyboardLayout = Ranged
```

近战武器选择 `Melee`，远程武器选择 `Ranged`。修改后退出并重启游戏。切换布局需要手动修改配置；程序不会根据装备自动切换。

源码默认使用 `InputMode = Original`，保留上游的键鼠映射、手柄图标和鼠标转换。手柄在两种模式下都能使用。

键鼠图标需要按下一节生成并安装。

## 安装图标和按键说明

需要 Python 3.10 或更新版本，以及 Pillow 12.2 或更新版本。在仓库根目录执行以下命令，将示例路径换成自己的游戏资源目录和游戏目录下的 `mods`：

```sh
python -m pip install "Pillow>=12.2"
python tools/build_keyboard_mouse_assets.py /path/to/game/romfs /path/to/mods/Keyboard_Mouse_Input_Melee --layout Melee
python tools/build_keyboard_mouse_assets.py /path/to/game/romfs /path/to/mods/Keyboard_Mouse_Input_Ranged --layout Ranged
```

输入目录应为原始游戏资源。两个输出目录必须为空，并放在输入目录之外。生成后的结构如下：

```text
mods/
  Keyboard_Mouse_Input_Melee/
    romfs/
      keyboard_mouse_input.flag
      nativeNX/
  Keyboard_Mouse_Input_Ranged/
    romfs/
      keyboard_mouse_input.flag
      nativeNX/
```

在 F1 设置菜单的 Mods 页启用两份资源包。程序会按 `KeyboardLayout` 加载其中一份；`Original` 模式会跳过两份。保留各自的 `keyboard_mouse_input.flag`，其中记录了资源包对应的布局。在 Mods 页被禁用的资源包不会因修改输入配置而自动启用。

图标覆盖按钮字体、标题、菜单翻页、战斗界面、目标选择、狩技方向和大厅；按键说明随游戏语言切换。图标显示主要绑定，不会列出所有备用按键。游戏手册中的整页手柄插图尚未替换，F1 设置菜单仍使用上游的英文或西班牙语。

## 按键表

| 输入 | 近战 Melee | 远程 Ranged |
| --- | --- | --- |
| WASD | 左摇杆：移动 | 左摇杆：移动 |
| 鼠标移动 | 右摇杆：视角 | 右摇杆：视角／瞄准 |
| 鼠标左键 | X：主攻击／拔刀 | A：射击、确认／交互 |
| 鼠标右键 | A：副攻击、确认／交互 | R：瞄准 |
| R | R：防御／武器特殊动作 | X：装填／拔刀 |
| 鼠标侧键5 | R：防御／武器特殊动作 | 未绑定 |
| F／鼠标侧键4 | Y：收刀／使用道具 | Y：收刀／使用道具 |
| E | Y：收刀／使用道具 | R：瞄准 |
| 左 Shift | R：通用奔跑 | R：通用奔跑 |
| Space | B：闪避、取消／返回 | B：闪避、取消／返回 |
| C | X+A：组合攻击 | X+A：蹲射等组合动作 |
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

菜单确认／交互在近战布局中是右键，在远程布局中是左键；两者都用 Space 返回。输入文字时，Enter 确认、Esc 取消。按 F2 释放鼠标后，点击游戏窗口可重新捕获。

Shift 对应游戏的通用奔跑，与强力冲刺分开。拔刀时，同一个游戏按钮 R 也可能用于防御或瞄准，具体动作取决于武器和操作类型。

远程表中的射击和装填按轻弩、重弩填写。弓在操作类型1下用 R 键拉弓、鼠标右键瞄准；类型2下这两个动作对调。

按键按 QWERTY 键盘的物理位置读取，其他键盘布局也使用相同位置。

## 鼠标设置

`[Mouse]` 下的 `MouseSensitivity` 控制总体灵敏度。`[MouseConversion]` 调整鼠标到右摇杆的转换，只在 `Keyboard_Mouse_Input` 模式下生效。修改这些参数后需要重启游戏。

```ini
[MouseConversion]
MinWindowMs = 8.0
FullStickSpeed = 1500.0
DeadzoneCompensation = 0.40
AimDeadzoneCompensation = 0.50
LowSpeedCutoff = 0.0
FrameTimeSmoothing = 0.25
```

> 相对位移计数是鼠标报告的移动量，不是屏幕像素；鼠标 DPI（每英寸计数）会影响同一物理移动产生的计数。归一化速度按 `计数/秒 × MouseSensitivity ÷ FullStickSpeed` 计算。

| 参数 | 范围／单位 | 作用 |
| --- | --- | --- |
| `MinWindowMs` | 1～50 毫秒 | 鼠标位移的平均窗口下限。增大后更平滑，停手后归零也更慢。实际窗口按完整帧间隔取整，最长100毫秒。 |
| `FullStickSpeed` | 100～100000 计数／秒 | 灵敏度为1时，达到满摇杆所需的鼠标速度。数值越大，同样鼠标速度对应的摇杆幅度越小。 |
| `DeadzoneCompensation` | 0～0.95，满摇杆幅度的比例 | 补偿游戏的摇杆死区。增大可加强小幅移动的响应，过低时小幅移动可能无法带动镜头。 |
| `AimDeadzoneCompensation` | 0～0.95，每个轴的满幅比例 | 远程布局按住鼠标右键或 E 时，对有移动的轴分别补偿。默认0.50使慢速水平、垂直和斜向输入都能触发右摇杆方向按键；设为0时使用普通视角的圆形补偿。 |
| `LowSpeedCutoff` | 0～0.1，归一化速度 | 忽略低于此值的移动。减小可保留更细微的输入；设为0时关闭这道过滤。 |
| `FrameTimeSmoothing` | 0.01～1，新帧间隔的权重 | 调整帧间隔估计。数值大时更快跟随帧率变化，小则更稳定；它不直接平滑鼠标速度。 |

先用 `MouseSensitivity` 调整总体速度。需要进一步调整时，再改上表中的参数。普通视角达到满摇杆所需的实际速度为 `FullStickSpeed ÷ MouseSensitivity`。默认不额外过滤低速移动；已有配置中的 `LowSpeedCutoff = 0.002` 仍会过滤低于每秒3个计数的输入（灵敏度为1时），需要保留这些输入时将其改为0。

普通视角使用圆形死区补偿。远程布局按住右键或 E 时使用按轴补偿，输出仍限制在摇杆圆内；仅按 Shift 奔跑不会启用瞄准补偿。该配置跟随按键，不检测武器或游戏当前是否进入瞄准；弓的操作类型和动作仍以游戏内设置为准。按轴补偿会改变低速斜向输入的比例，若感觉斜向过强，可设为0关闭。

鼠标输入仍会转换成右摇杆输入，受游戏最大转速限制，同样的鼠标移动距离不一定产生相同转角。转换设置只影响鼠标输入，游戏的射击动作、后坐力及瞄准限制保持原样。失焦、释放鼠标、输入文字或打开 PC 菜单时，会清空之前的鼠标位移。

各参数的默认值和范围也写在 INI 注释中。启动日志的 `[input] mouse conversion:` 行可查看实际加载的数值。

## 开发与检查

在正常构建环境中运行：

```sh
make test-settings test-keyboard-mouse test-settings-locale test-keyboard-mouse-assets
```

检查覆盖两套键位、道具／弹药滚轮、七种语言的说明、配置读写和鼠标采样。图标测试使用合成纹理和仓库中的 PC 素材，不需要游戏文件；鼠标测试检查输入采样，不测量实际镜头转角。

滚轮每次发送“按住 L → 按选择键 → 松开选择键 → 松开 L”，每阶段至少40毫秒。连续滚动会排队处理，失焦、输入文字或切换 PC 菜单时清空队列。

资源工具只修改按键文本和图标，保留其他资源。输出目录内的 `manifest.json` 记录修改项，并附有图标预览；生成的游戏资源留在本地。PC 图标素材的生成提示词见 [imagegen-prompts.txt](imagegen-prompts.txt)。

武器和风格的具体动作可查游戏内猎人笔记。相关按钮说明也可参考 Capcom 的 Generations 手册：[大剑](https://game.capcom.com/manual/MH_Gen/en/page-101.html)、[重弩](https://game.capcom.com/manual/MH_Gen/en/page-122.html)。
