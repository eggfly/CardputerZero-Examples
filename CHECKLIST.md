# CardputerZero 320×170 Example 清单

这个仓库的目标是为 **M5 CardputerZero**（CM Zero + 1.9" ST7789V 320×170 TFT，键盘 + 音频，**无触摸**）
收集/改写一批"小屏 app"示例，覆盖 FrameBuffer、SDL2、LVGL、Rust、Python、Qt、PyQt、Slint 等多种技术栈。

## 设备速查（给所有 example 作者的上下文）

| 项目 | 值 |
| --- | --- |
| SoC | RP3A0 quad Cortex-A53 @ 1 GHz（同 Pi Zero 2W） |
| RAM | 512 MB LPDDR2 |
| LCD | 1.9" ST7789V，**320×170**，RGB565，无触摸 |
| FB 设备 | `/dev/fb0`（单缓冲，`FBIOGET_VSCREENINFO` / `FBIOGET_FSCREENINFO`） |
| 键盘 | `/dev/input/event*`（evdev） |
| 音频 | ALSA `default`；用 `aplay` / `SDL_mixer` / `alsa-lib` 均可 |
| SSH | `pi@192.168.50.150`，密码 `pi` |
| 架构 | `aarch64`（原生在设备上编译即可，驱动见 m5stack-linux-dtoverlays） |
| 典型工具链 | 原生 gcc/g++、Python3、Qt5、SDL2、LVGL（build from source） |

> **原则**：UI 一律按 320×170 设计；字体默认 12–14 px；按键导航（无触摸，不要依赖指点设备）。

## 主清单（Checklist）

对应 issue 或 PR 时勾选即可。

### A. 基础示例（已存在空目录，需要填充）

- [x] **A1. FrameBuffer_HelloWorld**（C / `/dev/fb0` mmap + evdev，最小依赖）
- [x] **A2. FrameBuffer_Game**（Snake，fb + evdev，可选 `aplay` 蜂鸣）
- [x] **A3. SDL2_HelloWorld**（SDL2 软件渲染 + TTF + mixer）
- [x] **A4. SDL2_Game**（SDL2 Breakout，60 Hz 定时步进）
- [x] **A5. LVGL_HelloWorld**（LVGL v9.2.2 fbdev + evdev，按钮 + 计数）

### B. 其他语言的 Framebuffer Hello

- [x] **B1. Rust_FrameBuffer_HelloWorld**（`framebuffer` + `embedded-graphics` + `evdev`）
- [x] **B2. Python_FrameBuffer_HelloWorld**（`mmap` + `Pillow` + `numpy` RGB565）
- [x] **B3. Python_Tkinter_HelloWorld**（Xvfb + ffmpeg / PIL.ImageGrab 镜像到 `/dev/fb0`）
- [ ] **B4. Go_FrameBuffer_HelloWorld**（可选，`gofb` / syscall mmap）
- [ ] **B5. Node_FrameBuffer_HelloWorld**（可选，`node-framebuffer` / raw fs）

### C. GUI Toolkit Hello

- [x] **C1. Qt_HelloWorld**（C++ Qt5 Widgets，`linuxfb`，qmake + cmake 双通道）
- [x] **C2. PyQt5_HelloWorld**（PyQt5 fixed 320×170 + `run.sh`）
- [ ] **C3. GTK_HelloWorld**（可选，GTK3/4 + Cairo on fbdev——注意 GTK4 需要 GL）
- [ ] **C4. ImGui_HelloWorld**（可选，Dear ImGui + SDL2 backend）

### D. 较重型栈（CM0 可能跑不动完整版）

- [x] **D4. Slint_HelloWorld**（Rust Slint 1.9 `backend-linuxkms-noseat` + 软件渲染）

> Flutter（flutter-pi / flutter-elinux）与 GPUI 在 CM0 上硬件不足（无 GPU）；
> Electron / WebView（`cog`/`wpe-webkit`）资源开销过大。已从仓库中移除相关 stub，
> 不再追踪。

### E. 创意 Demo（320×170 天然小屏氛围）

- [x] **E1. Demo_Clock**（C / fb，大字号 HH:MM）
- [x] **E2. Demo_MusicSpectrum**（skeleton：ALSA capture + fb，FFT TODO）
- [x] **E3. Demo_SysDashboard**（Python / CPU/Mem/Temp/IP）
- [x] **E4. Demo_WifiScan**（Python + `iw scan` 解析）
- [ ] **E5. BLE 扫描器**（BlueZ / `bluetoothctl`，简易列表）
- [x] **E6. Demo_2048**（C / fb，4×4 键盘）
- [x] **E7. Demo_MarkdownReader**（Python 分页）
- [ ] **E8. 示波器**（ADS1115 或 ALSA 输入，LVGL chart）
- [x] **E9. Demo_ImageViewer**（Python + Pillow）
- [x] **E10. Demo_Matrix**（C 屏保 Matrix rain）

### E+ 经典游戏移植（320×170 + 键盘友好）

- [x] **G1. Game_Tetris**（C / fb，10×17 网格）
- [x] **G2. Game_Minesweeper**（C / fb，20×14 + HUD）
- [x] **G3. Game_Sokoban**（C / fb，3 关内建关卡 + Undo）
- [x] **G4. Game_Pong**（C / fb，人 vs CPU）
- [x] **G5. Game_Flappy**（C / fb，Flappy Bird 克隆）
- [x] **G6. Game_LunarLander**（C / fb，推力+旋转+重力）
- [x] **G7. Game_Invaders**（SDL2 软渲，简化 Space Invaders）
- [x] **G8. Game_Asteroids**（SDL2 软渲，向量风格）
- [x] **G9. Game_TicTacToe**（SDL2 软渲，Minimax CPU）
- [x] **G10. NES_Emulator**（SDL2 + LaiNES；`scaletoheight` 181×170 信箱 / `cropheight` 320×170 拉伸裁切；`V` 键切换）

### F. 构建与部署基础设施

- [x] **F1. 根 `README.md` 索引**（按表格列出每个 example 的状态 / 技术栈 / 依赖）
- [x] **F2. APPLaunch `.deb` 打包**（`scripts/pack-deb.sh` + 每个 example `packaging/meta.env|build.sh|stage.sh`）
- [x] **F3. GitHub Actions CI**（`.github/workflows/build-debs.yml`，`ubuntu-24.04-arm` 原生矩阵，release 自动附 `.deb`）
- [x] **F4. 打包约定文档**（`docs/PACKAGING.md`）
- [x] **F5. `scripts/deploy.sh`**（`scp .deb + dpkg -i + restart APPLaunch`，支持 `--uninstall` / `--dry-run`）
- [x] **F6. `scripts/dev-on-mac/`**（`run-sdl.sh` macOS 原生；`setup-docker.sh` + `run-in-docker.sh` + `smoke-deb.sh` aarch64 容器）

## 技术路线对比（写在这儿方便将来挑选）

| 栈 | 在 CM0 上 | 内存占用 | 延迟 | 成本 | 备注 |
| --- | --- | --- | --- | --- | --- |
| Framebuffer (C) | ✅ 原生 | 极小 | 1 frame | 很低 | 最硬核，自己管理 RGB565 |
| SDL2 + KMSDRM | ✅ | 小 | 1 frame | 低 | CM0 无 GPU 时走 `software` |
| LVGL fbdev | ✅ | 小 | 1 frame | 中 | UserDemo 已采用，API 成熟 |
| Rust + embedded-graphics | ✅ | 极小 | 1 frame | 低 | Cross build 简单 |
| Python + mmap | ✅ | 中 | 多 frame | 很低 | 原型最快，但帧率低 |
| Qt5 Widgets linuxfb | ⚠️ | 大 | 多 frame | 中 | 需要大量 Qt 库 |
| PyQt5 linuxfb | ⚠️ | 很大 | 多 frame | 中 | 可行但重 |
| Slint linuxfb | ✅ | 小 | 1 frame | 中 | 对 320×170 友好，值得试 |

> 硬件限制：Flutter（flutter-pi/flutter-elinux）、GPUI、GTK4、Webview/Electron
> 在 CM0 上均不可行（无 GPU / 资源不足），已从本仓库移除。

## 进度追踪约定

- 每完成一个 example：勾上对应行，并在 example 目录里写 `README.md`（中英任一即可）。
- 目录命名：`<Stack>_<Purpose>`，例如 `Rust_FrameBuffer_HelloWorld`、`PyQt5_HelloWorld`。
- 每个 example 至少包含：源码 + 构建脚本（Makefile / CMakeLists / Cargo.toml / requirements.txt）+ `README.md`（含"为什么 320×170"）。
- 纯文档 stub 也要有目录和 `README.md`，说明为何当前不实现（"CM0 无 GPU" 等）。
