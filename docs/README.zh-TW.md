<h1 align="center">MHGURecomp 原生鍵鼠輸入</h1>

<p align="center">
  <a href="../README.md">English</a> · <a href="README.zh-CN.md">简体中文</a> · <strong>繁體中文</strong>
</p>

這是 **Monster Hunter Generations Ultimate** 的原生 PC 移植，針對 Nintendo Switch **1.4.0** 版本。遊戲的 ARM 程式碼靜態重編譯為 C，由宿主執行階段實現 Switch 服務，並通過 Vulkan 渲染。

本倉庫 fork 自 [morerokk/MHGURecomp](https://github.com/morerokk/MHGURecomp)，以**原生鍵鼠輸入優化**為主要方向獨立開發，並定期手動同步原倉庫的部分改動。

**倉庫不包含原始遊戲匯出、重編譯生成的遊戲原始碼或金鑰。** 倉庫包含預先產生的 PC 提示 mod；執行遊戲仍需使用自己的遊戲匯出。

## 功能

- 普通任務鏡頭、弩／弓瞄準和望遠鏡使用直接滑鼠角度輸入，保留細微移動。
- 根據實際裝備自動切換近戰／遠程鍵位及提示文字、按鈕圖示；獵貓使用近戰預設。
- 遊戲菜單的視角移動速度、準星移動速度直接調節對應滑鼠靈敏度，即時生效。
- 保留射擊硬直、入瞄準延遲、招式轉向視窗、俯仰範圍及原有鏡頭行為。
- 按鍵提示覆蓋遊戲的七種語言：英語、簡體中文、繁體中文、法語、德語、意大利語、西班牙語。
- 解析度、寬高比、MSAA／FXAA、FSR、VSync、30～240FPS 設定，手柄支援和 F1 設定面板。
- 保留原專案的本地連線、可選怪物血量倍率、村任務連線、模組和金手指支援。

原生滑鼠實現只針對上述 Switch 可執行檔案；其他遊戲建置及 3DS 版不在當前適配範圍內。

## 執行要求

所有啟動入口都會在載入遊戲程式碼前，核對支援的 Switch MHGU 1.4.0 的 main／sdk 完整 Build ID，以及部分基礎 romfs 資源指紋。不符合時提示具體檔案並停止啟動。資源指紋不等於整套 ROM 的完整性驗證。保留原始合併 romfs，修改放在 `mods/`；`mhgurecomp --check-rom <遊戲目錄>` 只檢查，不啟動遊戲。

在村莊或任務中按 `/` 開啟鍵盤聊天，Enter 傳送、Esc 取消；可在 F1 設定修改 `Controls.ChatKey`，已被鍵鼠操作占用的按鍵會標示。`Anti-aliasing.FXAAMode = 3_11` 可選擇 FXAA 3.11。

- Windows 10／11 64位或 Linux。
- 支援 AVX2 的處理器、支援 Vulkan 1.2 的顯示卡，至少1GB顯示記憶體、4GB記憶體。
- 自己匯出的完整遊戲檔案，**將1.4.0更新合併到本體資源中**。
- Title ID：`0100770008DD8000`；`main` 的 Build ID 以 `FB08F1D20FD1204F` 開頭。

目前原生輸入的實際遊戲測試在 Windows、60FPS 下進行。專案保留 Linux 執行階段，但尚未在 Linux 重復同一組鍵鼠實測。

## 安裝

預編譯程式以[本倉庫的 Releases](https://github.com/dzxrly/MHGURecomp-Native-KeyboardMouse/releases)為準。沒有安裝包時，請[從源碼建置](#從源碼建置)。原倉庫的程式不包含本倉庫的原生輸入改動。

1. 將程式解壓到自選目錄。
2. 把遊戲的 `exefs`、`romfs` 放入 `data`：

   ```text
   MHGURecomp/
     mhgurecomp.exe
     mhgugame.dll
     （其他執行庫）
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

3. 啟動 `mhgurecomp.exe`，首次啟動會生成 `mhgurecomp.ini`。
4. 按下文啟用鍵鼠模式並安裝自動提示包。

`exefs` 必須來自更新；`romfs` 必須是本體疊加更新後的完整資源，約35,500個檔案、12.3GB。缺失、版本不符或資源不完整時，程式會提示錯誤。

## 鍵鼠設定

### 啟用與基礎設定

修改 `mhgurecomp.ini`，然後重啟一次：

   ```ini
   [Controls]
   InputMode = Keyboard_Mouse_Input

   [Mouse]
   MouseSensitivity = 1.0
   ```

源碼預設仍為 `InputMode = Original`。兩種模式均可使用手柄。鍵鼠模式下，輕弩、重弩、弓自動使用**遠程 Ranged**，其余獵人武器和獵貓自動使用**近戰 Melee**。更換裝備後按鍵和提示自動切換，無需重啟；舊的 `KeyboardLayout` 不再生效。

### 安裝自動提示包

本倉庫的 Release 套件已包含 `mods/Keyboard_Mouse_Input_Auto`，可直接使用隨附提示層。下方產生命令僅用於從原始碼建置，或在本機重新產生提示套件。

使用 Python 3.10或更新版本，在源碼倉庫根目錄執行：

   ```sh
   python -m pip install "Pillow>=12.2"
   python tools/build_keyboard_mouse_assets.py /path/to/game/romfs /path/to/game/mods/Keyboard_Mouse_Input_Auto --layout Auto
   ```

輸入應為未修改的遊戲 `romfs`；輸出目錄應為空，並位于輸入目錄之外。生成後，在 F1 設定的 **Mods** 頁啟用 `Keyboard_Mouse_Input_Auto` 並重啟。

   ```text
   mods/
     Keyboard_Mouse_Input_Auto/
       romfs/
         keyboard_mouse_input.flag
         keyboard_mouse_prompts.bin
         nativeNX/
   ```

保留兩個標記／數據檔案。一份 Auto 包包含兩套預設，舊的 Melee／Ranged 單預設包會跳過；Original 模式也會跳過 Auto 包。文字和圖示跟隨裝備與遊戲語言切換；在 Mods 頁禁用的包不會自行啟用。

### 按鍵表

| 輸入 | 近戰 Melee | 遠程 Ranged |
| --- | --- | --- |
| WASD | 左搖桿：移動 | 左搖桿：移動 |
| 滑鼠移動 | 任務視角 | 任務視角／武器瞄準 |
| 滑鼠左鍵 | X：主攻擊／拔刀 | A：射擊、確認／互動 |
| 滑鼠右鍵 | A：副攻擊、確認／互動 | R：瞄準 |
| R | R：防禦／武器特殊動作 | X：裝填／拔刀 |
| 滑鼠側鍵5 | R：防禦／武器特殊動作 | 未綁定 |
| F／滑鼠側鍵4 | Y：收刀／使用道具 | Y：收刀／使用道具 |
| E | Y：收刀／使用道具 | R：瞄準 |
| 左 Shift | R：通用奔跑 | R：通用奔跑 |
| Space | B：閃避、取消／返回 | B：閃避、取消／返回 |
| C | X+A：組合攻擊 | X+A：組合動作 |
| 滾輪上／下 | L+Y／L+A：選擇道具 | L+X／L+B：選擇彈藥或瓶 |
| Q | L：道具欄／鏡頭操作 | L：道具欄／鏡頭操作 |
| Esc／Enter／小鍵盤 Enter | ＋：遊戲菜單 | ＋：遊戲菜單 |
| Backspace | －：減號菜單 | －：減號菜單 |
| Tab／滑鼠中鍵 | ZL：切換目標（操作類型1） | ZL：切換目標（操作類型1） |
| T | ZR | ZR |
| 1／2／3 | 方向上／右／下：狩技（操作類型1） | 方向上／右／下：狩技（操作類型1） |
| 方向鍵 | 十字方向鍵 | 十字方向鍵 |
| Ctrl／Alt | 按下左／右搖桿 | 按下左／右搖桿 |
| F1／F2 | PC 設定／釋放滑鼠 | PC 設定／釋放滑鼠 |

按鍵按 QWERTY 鍵盤的物理位置讀取。點擊視窗可重新捕獲滑鼠。輸入文字時 Enter 確認、Esc 取消、Ctrl+V 貼上。遊戲菜單中近戰用右鍵確認，遠程用左鍵確認，Space 返回。預設切換會釋放按住狀態並清空滾輪隊列。

### 弓與獵貓

弓自動使用遠程預設。收弓時左鍵保留 A 確認／互動，按 R（遊戲 X）拔弓；拔弓後：

| 輸入 | 弓操作類型1 | 弓操作類型2 |
| --- | --- | --- |
| 滑鼠左鍵 | X：蓄力，松開發射 | 遊戲設定對應的 R：蓄力，松開發射 |
| 滑鼠右鍵 | 遊戲設定對應的 R：按住瞄準 | X：點擊切換瞄準 |
| 滑鼠側鍵5 | A：弓副動作 | A：弓副動作 |

遊戲的 R/ZR 互換設定參與滑鼠映射，鍵盤仍按基礎表工作。打開菜單後左鍵恢復為 A；松開滑鼠會釋放按下時對應的按鈕。目前弓的兩種操作類型仍共用基礎遠程提示，滑鼠操作以上表為準。

獵貓使用近戰預設：左鍵 X 近戰，右鍵 A 迴旋鏢；按住右鍵蓄力時，再按住 R／側鍵5可精瞄。拔武器時遊戲 R 防禦、R+X 上擊；收武器時 R+X+A 防禦。滾輪選擇支援行動，收武器時 F 使用，拔武器時 R+F 使用。原有資格、消耗和動作限制繼續生效。

### 滑鼠靈敏度與鏡頭選項

`MouseSensitivity` 範圍為0.1～10，作為基礎靈敏度倍率。直接輸入使用滑鼠相對位移計數，而非螢幕像素；滑鼠 DPI 會影響同一物理移動產生的計數。程式不額外加入滑鼠加速曲線。

| 遊戲菜單選項 | 對應直接輸入 | 緩慢 | 標準 | 快速 |
| --- | --- | --- | --- | --- |
| 視角移動速度 | 普通任務鏡頭 | 0.5倍 | 1倍 | 1.5倍 |
| 準星移動速度 | 弩／弓瞄準及望遠鏡 | 0.5倍 | 1倍 | 1.5倍 |

兩項獨立即時生效。`MouseSensitivity = 1` 時，水平移動1000計數約對應20°／40°／60°。普通視角、快速瞄準和第一人稱／望遠鏡分別遵循各自的反轉設定；五種瞄準視角類型、初始鏡頭位置、目標視角和地形調整保留原有行為。

動作屏蔽期間的滑鼠位移會丟棄，恢復後不會補轉。轉動鏡頭也不會強制讓正在出招的近戰角色超出遊戲允許視窗轉向。

村莊、固定弩炮和獵貓特殊瞄準保留滑鼠轉搖桿相容路徑，僅這些場景使用 `[MouseConversion]`：

   ```ini
   [MouseConversion]
   MinWindowMs = 8.0
   FullStickSpeed = 1500.0
   DeadzoneCompensation = 0.40
   AimDeadzoneCompensation = 0.50
   LowSpeedCutoff = 0.0
   FrameTimeSmoothing = 0.25
   ```

修改這些參數後重啟。`FullStickSpeed` 表示靈敏度為1時達到滿搖桿所需的滑鼠速度，單位為計數／秒；兩項 Compensation 分別補償普通／瞄準死區；`MinWindowMs` 控制最小取樣視窗；`LowSpeedCutoff` 過濾極慢移動；`FrameTimeSmoothing` 調整幀間隔估計。相容鏡頭仍受遊戲搖桿轉速限制。

### 已驗證與待完善

實際遊戲驗證包括弩／弓瞄準和彈道方向、弓的兩種操作類型、鏡頭菜單選項、自動換裝預設、獵貓迴旋鏢精瞄／防禦、操蟲棍瞄準／發出獵蟲和固定弩炮相容瞄準。另檢查了太刀移動／拔刀方向、長槍防禦／突刺、盾斧防禦；這不代表全部武器、風格和動作均已驗證。

仍需完善弓專屬提示、裝備支援行動後的獵貓實際施放、可變倍率，以及更多武器／風格／彈種和幀率組合。不同幀率的滑鼠取樣單元測試不能代替原生角度實測。遊戲手冊中的整頁手柄插圖尚未替換。

`InputMode = Original` 保留原映射：WASD 移動，滑鼠轉鏡頭，左右鍵 X／A，Space／R 為 B／Y，Q／E 為 L／R，Tab／T 為 ZL／ZR，Enter／Backspace 為＋／－；該模式下 Esc 釋放滑鼠。

## 設定與檔案位置

F1 打開 PC 設定；Alt+Enter 或 F11 切換全螢幕，F12 截圖，F7 切換 MSAA。PC 設定界面目前支援英語／西班牙語，與遊戲的七種語言獨立。

`mhgurecomp.ini` 內附各項說明，包括顯示／渲染解析度、MSAA／FXAA、FSR 3.1／4及品質、繪製距離、FPS／VSync、控制、滑鼠、網路、怪物血量、村任務、額外內容和音訊。啟用 FSR 時由它接管抗鋸齒。詳見[設定說明](technical/SETTINGS_UI.md)、[FSR 3.1](technical/FSR-PORT.md)、[FSR 4](technical/FSR4-PORT.md)。

| 內容 | 位置 |
| --- | --- |
| 設定／記錄 | 程式旁的 `mhgurecomp.ini`／`log.txt` |
| Windows 存檔 | `%APPDATA%\MHGURecomp\save\save_0\` |
| Linux 存檔 | `$XDG_DATA_HOME/MHGURecomp/save/save_0/`，預設 `~/.local/share/MHGURecomp/save/save_0/` |
| 模組／金手指 | 程式旁的 `mods/`／`cheats/` |
| 截圖／著色器快取 | 程式旁的 `shots/`／`cache/` |

匯入自己的 Switch 存檔時，將 `system`、`system_backup` 複製到 `save_0`，覆蓋前備份已有存檔。GitHub 倉庫改名不改變程式檔案名稱或存檔位置。

## 連線、模組與金手指

使用遊戲的**本地無線通信**選項。區域網路／虛擬區域網路可直接發現房間；網際網路連線需要相容的程式版本，在 `[Multiplayer] Server` 設定房間伺服器，用相同 `Key` 限定好友發現。這裡不保證原專案公共伺服器持續可用，也不保證不同建置間互通。自建方法見 [server/README.md](../server/README.md)。不使用中繼時，主機需開放連接埠（預設 TCP／UDP 39500），用戶端設定 `Join` 地址。

每個模組放入獨立的 `mods/` 子目錄，在 F1 → Mods 啟用、排序並重啟。支援 RomFS 替換、ARC 條目合併和 Forge 外掛程式；衝突條目以後載入者為準。Insert 打開 Forge；不支援的程式碼補丁會寫入記錄。執行階段已有的解析度、幀率功能無需另裝修復模組。

將適用於1.4.0的金手指 `.txt` 放入 `cheats/`，每項以 `[名稱]` 開頭，啟動時載入；檔案名稱前加 `_` 可禁用。`extras/cheats/` 的可選程式碼說明見[技能點](technical/research/skill-points.md)和[勇氣收刀](technical/research/valor-sheathe.md)。

若發行包提供 `bonus/system` 且 `BonusContent = On`，程式可在未存在時匯入免費的活動任務／道具包／隨從內容；設為 Off 可關閉。

## 更新與原倉庫同步

從**本倉庫**下載新版，替換程式檔案並保留遊戲數據、設定、模組和存檔。本倉庫建置的更新器也只查詢本倉庫發佈，不安裝原專案版本；沒有已發佈版本或網路不可用時會啟動當前遊戲。`0.11.2-13-g0641c27` 這類開發版本不會被自動替換成標籤發行版。

本專案會定期查看[原倉庫](https://github.com/morerokk/MHGURecomp)，手動選擇並整合部分改動，功能和版本號不要求同步。可保留指向原倉庫的 `upstream` 遠端，用於這項工作。

## 從源碼建置

克隆本倉庫，安裝編譯工具和 Python 依賴，然後使用自己的遊戲檔案建置：

   ```sh
   git clone https://github.com/dzxrly/MHGURecomp-Native-KeyboardMouse.git
   cd MHGURecomp-Native-KeyboardMouse
   make gen GAME=/path/to/game
   make -j4 GAME=/path/to/game
   ```

完整的 [Windows／Linux 建置及打包說明](technical/BUILDING.md)包含 MSYS2 軟體包、Python、FFmpeg 和輸出目錄。程式仍使用 `mhgurecomp`、`mhgugame` 檔案名稱。

現有輸入與設定檢查：

   ```sh
   make test-settings test-keyboard-mouse test-settings-locale test-keyboard-mouse-assets
   ```

## 源碼結構與許可

| 目錄 | 用途 |
| --- | --- |
| `runtime/` | 宿主執行階段、原生滑鼠、自動預設、圖形、網路和設定 |
| `tools/` | 重編譯器和本地提示包生成工具 |
| `updater/`／`server/` | 本倉庫更新器／可選房間伺服器 |
| `docs/` | 其他語言 README；`technical/` 保存實現和建置說明 |
| `extras/` | PC 提示素材、資源工具說明和可選金手指 |

專案程式碼采用 MIT 許可，保留原專案署名和第三方聲明，見 [LICENSE](../LICENSE) 與 [THIRD_PARTY_NOTICES](../THIRD_PARTY_NOTICES)。許可不覆蓋遊戲或生成的遊戲程式碼。本專案與 Capcom、Nintendo 無關聯。
