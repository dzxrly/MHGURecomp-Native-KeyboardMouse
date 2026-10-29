# Keyboard_Mouse_Input：近战与远程键鼠布局

> `InputMode` 选择原始输入或通用键鼠输入；`KeyboardLayout` 选择近战（`Melee`）或远程（`Ranged`）的固定键位和对应图标。表中的“游戏按钮”是发送给 MHGU 的虚拟 Switch 按钮。LMB／RMB／MMB 分别表示鼠标左／右／中键。本功能基于 MHGURecomp 0.10.8。

[English guide](README.en.md)

在游戏目录的 `mhgurecomp.ini` 中设置，完全退出并重启游戏后生效：

```ini
[Controls]
InputMode = Keyboard_Mouse_Input
KeyboardLayout = Ranged
```

- `InputMode = Original`：原始键鼠映射、原始手柄提示与原鼠标转换逻辑。这是源码默认值；手柄输入始终可用。
- `InputMode = Keyboard_Mouse_Input`：启用下面的键鼠布局、对应图标／说明和改进的鼠标转换。
- `KeyboardLayout = Melee`：近战；`KeyboardLayout = Ranged`：远程。默认远程。本项不自动检测武器，更换类别后需要手动修改并重启。

此前版本的两项配置已由这两项替代。安装新版资源时，将旧提示包移出 `mods`，避免旧资源覆盖当前布局。

## 按键映射

| 输入 | 近战 Melee | 远程 Ranged |
| --- | --- | --- |
| WASD | 左摇杆移动 | 左摇杆移动 |
| 鼠标移动 | 右摇杆视角 | 右摇杆视角／瞄准 |
| 鼠标左键 | X：主攻击／拔刀 | A：射击、确认和交互 |
| 鼠标右键 | A：副攻击、确认和交互 | R：瞄准 |
| R | R：防御／武器特殊动作 | X：装填／拔刀 |
| 鼠标侧键5 | R：防御／武器特殊动作 | 不绑定，保留现有布局 |
| F／鼠标侧键4 | Y：收刀／使用道具 | Y：收刀／使用道具 |
| E | Y：收刀／使用道具的别名 | R：保留此前绑定 |
| 左 Shift | R：通用奔跑 | R：通用奔跑 |
| Space | B：闪避、取消、菜单返回 | B：闪避、取消、菜单返回 |
| C | X+A：组合攻击 | X+A：蹲射等组合动作 |
| 滚轮上／下 | L+Y／L+A：选择道具 | L+X／L+B：选择弹药或瓶 |
| Q | L：道具栏／原游戏镜头操作 | 同左 |
| Esc／Enter／小键盘 Enter | ＋：游戏菜单 | 同左 |
| Backspace | －：原游戏减号菜单 | 同左 |
| Tab／鼠标中键 | ZL：操作类型1下切换目标 | 同左 |
| T | ZR：原游戏对应动作 | 同左 |
| 1／2／3 | 方向上／右／下：操作类型1下狩技 | 同左 |
| 方向键 | 十字方向键 | 同左 |
| Ctrl／Alt | 按下左／右摇杆 | 同左 |
| F1／F2 | 主机设置／释放鼠标捕获 | 同左 |

远程表中的射击／装填动作以轻弩和重弩为基准。MHGU 的弓有独立的操作类型：类型1用 X 拉弓、R 瞄准，因此本布局对应 R 键拉弓、鼠标右键瞄准；类型2对调 X/R。这里没有自动识别弓并重映射，不能将弩炮的左键射击说明直接套用到弓。

布局借鉴世界／冰原与荒野的左键主攻击、右键副攻击或瞄准、Shift 奔跑、Space 闪避、R 装填等习惯。近战 R 防御／特殊动作也便于从荒野过渡；远程保留此前已经使用的 F 收刀、C 组合输入。图标源素材直接复用，没有重新生成。

MHGU 的一个虚拟按钮在战斗和菜单中有不同含义，因此近战的菜单确认／交互是右键，远程是左键；两者均用 Space 返回。这里没有额外检测场景来切换菜单绑定，也没有实现世界／荒野的投射器、集中模式、自动收刀冲刺等新动作。游戏内“操作类型”仍决定虚拟按钮的行为，建议从类型1核对。

Shift 始终只发送 R，不发送 B，也不自动衔接强力冲刺。武器拔出时 R 仍可能被游戏解释为防御或瞄准。英勇收刀、装填、蹲射等动作的条件与时机仍由游戏处理。输入文字时 Enter 确认、Esc 取消；F2 释放鼠标后点击窗口可重新捕获。

滚轮使用 L 按下 → L+选择键 → 释放选择键 → 释放 L 的序列，每阶段至少40毫秒；多格滚动排队处理，失焦、文本输入或主机菜单切换会清空队列。它是固定游戏按钮序列，未对所有菜单做场景识别。

## 图标与七语言资源

生成两份独立提示包，使用当前 `KeyboardLayout` 对应的一份。七种游戏语言均有适配：简体中文、繁体中文、英语、法语、德语、意大利语、西班牙语。两套包都包含共享按钮字体、标题、菜单翻页、HUD、目标选择、狩技方向和大厅的图标，以及各语言的显式按键说明。近战将 X/A/R 提示映射为鼠标左键／鼠标右键／R；远程对应 R／鼠标左键／鼠标右键。

用 Python 3.10+、Pillow 12.2+，从仓库根目录执行；替换路径，两个输出目录必须为空，且位于原始 romfs 之外：

```sh
python -m pip install "Pillow>=12.2"
python tools/build_keyboard_mouse_assets.py /path/to/game/romfs /path/to/mods/Keyboard_Mouse_Input_Melee --layout Melee
python tools/build_keyboard_mouse_assets.py /path/to/game/romfs /path/to/mods/Keyboard_Mouse_Input_Ranged --layout Ranged
```

两份生成目录放入游戏的 `mods`，保留各自的 `romfs/keyboard_mouse_input.flag`（内容分别为 `Melee` 或 `Ranged`）。程序按配置只加载匹配的资源层，重命名／嵌套 mod 文件夹不影响识别。Mods 列表中应启用两份包；配置不会覆盖 `load_order.txt` 的禁用状态。`Original` 会跳过两份包，其他 mod 不受影响。不要把其他功能的资源混入这两份专用资源层。

图标是固定主要绑定，别名不逐一绘制：例如奔跑文字写 Shift，R 图标按当前布局显示 R 或鼠标右键；狩技方向图标显示方向键，1/2/3 仍可用。键帽按 QWERTY 物理位置标示，不自动适配 AZERTY、QWERTZ。操作手册整张插画中的手柄图示尚未重绘。F1 主机菜单保留上游英／西语。

工具保留 GMD 的标识、条目顺序、颜色及分页标记；包内有差异的文本从其自身原文修改。只重编码属于图标的 BC3 区块，其他纹理区块和 ARC 中未修改的压缩资源保留原字节。生成的 `manifest.json` 记录每种语言的修改，预览用于本地检查。游戏资源、生成后的 mod、预览和存档不进入公开仓库；PC 图标素材的生成提示词见 `imagegen-prompts.txt`。

## 鼠标转换与检查

`[Mouse] MouseSensitivity` 仍控制总体灵敏度。专门的 `[MouseConversion]` 配置控制鼠标到右摇杆的转换，只对 `InputMode = Keyboard_Mouse_Input` 生效，近战／远程共用，修改后需要完全退出并重启游戏。以下默认值保留当前转换效果：

```ini
[MouseConversion]
MinWindowMs = 8.0
FullStickSpeed = 1500.0
DeadzoneCompensation = 0.40
LowSpeedCutoff = 0.002
FrameTimeSmoothing = 0.25
```

| 参数 | 有效范围／单位 | 调整作用 |
| --- | --- | --- |
| `MinWindowMs` | 1～50 毫秒 | 采样窗口下限。增大会更平滑，停手后也会保留更久；实际窗口仍按完整帧间隔取整，最高100毫秒。 |
| `FullStickSpeed` | 100～100000，相对位移计数／秒 | 灵敏度为1时达到满摇杆所需的鼠标速度。增大后，同样移动速度对应的摇杆幅度减小。 |
| `DeadzoneCompensation` | 0～0.95，满摇杆幅度的比例 | 补偿游戏自身死区。增大会加强起步响应；设得过低可能使微小移动重新落入游戏死区。 |
| `LowSpeedCutoff` | 0～0.1，归一化速度比例 | 低于此值时不输出。减小允许更细微的移动；0只关闭这道额外截止，静止时仍输出0。 |
| `FrameTimeSmoothing` | 0.01～1，新帧间隔的权重 | 增大可更快跟随实际帧率变化；减小可让采样窗口更稳定。它平滑的是帧间隔估计，不是鼠标速度。 |

归一化速度为 `相对位移计数/秒 × MouseSensitivity ÷ FullStickSpeed`。因此默认截止对应3个计数／秒；改变灵敏度或满幅速度后，这个对应值也会改变。实际满摇杆速度为 `FullStickSpeed ÷ MouseSensitivity`。这些计数不是屏幕像素，鼠标 DPI 也会影响同一物理移动产生的计数。

生成的 INI 包含同样的单位、范围和默认值注释。启动日志的 `[input] mouse conversion:` 行会打印实际加载的五个值。已有配置缺少此段时，程序会补入默认段；超范围、NaN 或无穷值按项目配置规则拒绝，不会作为有效转换参数使用。`Original` 模式不使用这些参数。

通用键鼠模式使用有限时间窗平均鼠标相对位移，替换原25毫秒指数平滑。使用默认参数时，窗口至少8毫秒，并取一个或多个游戏帧间隔；稳定30／60／120／240 FPS时约为33.3／16.7／8.3／8.3毫秒，实际帧率下降会加长窗口。低速截止从默认灵敏度下30降为3个相对位移计数／秒。失焦、释放捕获、文本输入和主机菜单切换会清空历史。

方向、圆形死区补偿和满摇杆限幅保留，`MouseSensitivity` 仍控制速度。它仍是鼠标到右摇杆的转换，不能保证现代原生鼠标的固定距离转角，也不突破游戏最大转速。有限窗使停手后的输入在窗口过期时归零；实际画面延迟还包括游戏更新与显示。原始模式保留上游转换。

字体图块回归需要 Pillow，并使用空白合成图集和本项目 PC 素材，不依赖游戏文件。

在构建环境运行 `make test-settings test-keyboard-mouse test-settings-locale test-keyboard-mouse-assets`。检查包含两套映射、滚轮在30～240 FPS下的序列、七语言说明、配置持久化和重启语义，以及鼠标重采样在125～8000 Hz输入和30～240 FPS采样下的累计量、停止、方向及清空行为。累计量测试验证输入采样，不代表实际相机转角误差或玩家手感测试。

参考：[Capcom Generations 大剑操作](https://game.capcom.com/manual/MH_Gen/en/page-101.html)、[Capcom Generations 重弩操作](https://game.capcom.com/manual/MH_Gen/en/page-122.html)。MHGU具体动作另核对本机1.4.0的猎人笔记；风格差异由游戏处理。[世界／冰原 PC 键位与游戏截图](https://www.shacknews.com/article/106612/pc-keyboard-controls-and-key-bindings-monster-hunter-world)、[荒野 PC／手柄键位与实机截图](https://dotesports.com/monster-hunter/news/all-controls-and-keybinds-in-monster-hunter-wilds-pc-and-console)用于比较操作习惯；本布局不宣称逐项复刻这些游戏的默认设置。
