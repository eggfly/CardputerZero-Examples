# CardputerZero-Examples

Multi-stack "Hello World" + small demos for **M5 CardputerZero** —
Raspberry Pi CM Zero + 1.9" ST7789V **320×170** TFT, keyboard, audio, no touchscreen.

完整的规划、设备参数、技术栈对比和进度清单见 [CHECKLIST.md](./CHECKLIST.md)。

## Hardware

| | |
| --- | --- |
| SoC | RP3A0 quad-core Cortex-A53 @ 1 GHz (same die as Pi Zero 2W) |
| RAM | 512 MB LPDDR2 |
| LCD | 1.9" ST7789V — **320×170** RGB565, no touch |
| Framebuffer | `/dev/fb0` (read `FBIOGET_VSCREENINFO` — do not hardcode) |
| Input | `/dev/input/event*` via evdev |
| Audio | ALSA `default` (aplay / SDL2_mixer / libasound) |
| Arch / OS | aarch64 Debian-like Linux |
| SSH | `pi@<device-ip>` (password `pi`) |

Driver overlays: <https://github.com/m5stack/m5stack-linux-dtoverlays/tree/main/modules/CardputerZero>

## Quick start: get a `.deb` onto the device

新手最快的路径是让 GitHub Actions 替你编译 —— 本地什么工具链都不用装，
push 到 `ci/**` 分支即可，产物是每个示例的 `.deb`。

```bash
# 1) push your branch (CI triggers on any ci/** branch)
git checkout -b ci/try-it
git push -u origin ci/try-it

# 2) wait for the run, then download every .deb
gh run watch -R <owner>/CardputerZero-Examples
gh run download -R <owner>/CardputerZero-Examples -n '*-arm64-deb' --dir dist/

# 3) install on the device and restart APPLaunch
scp dist/framebuffer-hello_0.1-m5stack1_arm64.deb pi@<device-ip>:/tmp/
ssh pi@<device-ip> \
    'sudo dpkg -i /tmp/framebuffer-hello_0.1-m5stack1_arm64.deb \
      && sudo systemctl restart APPLaunch.service'
```

That's the whole beginner loop. For details (cross-building from your
laptop, native CM0 builds, publishing to M5's firmware store, trilingual
docs), jump to [Packaging &amp; CI](#packaging--ci) below.

## Examples index &amp; device status

One merged inventory. Columns:

- **Framework / Language** — UI toolkit and source language.
- **Audio** — sound path (ALSA / mixer lib, `—` if silent).
- **Screen** — how pixels reach the 320×170 LCD.
- **Keys** — how the app reads the TCA8418 keypad.
- **Launch** — reaches first frame under APPLaunch.
- **ESC** — a single short-press exits in ≤ 1 s.
- **Notes** — nav-key issues, regressions, pending work.

Legend: 🟢 works · 🔴 broken · — n/a.

Last measured on device: **2026-05-01** (fresh CI `.deb` install).

| # | Example | Framework | Language | Audio | Screen | Keys | Launch | ESC | Notes |
|---|---|---|---|---|---|---|:---:|:---:|---|
| 1 | [FrameBuffer_HelloWorld](./FrameBuffer_HelloWorld) | raw /dev/fb0 | C11 | — | mmap RGB565 + 32bpp fallback | evdev raw | 🟢 | 🟢 | |
| 2 | [FrameBuffer_Game](./FrameBuffer_Game) (Snake) | raw /dev/fb0 | C11 | ALSA `aplay` beep | mmap RGB565 | evdev raw | 🟢 | 🟢 | |
| 3 | [SDL2_HelloWorld](./SDL2_HelloWorld) | SDL2+ttf+mixer | C | SDL2_mixer | SDL2 Wayland | SDL2 KeyboardEvent | 🔴 | — | regression: doesn't launch after latest CI deploy |
| 4 | [SDL2_Game](./SDL2_Game) (Breakout) | SDL2+mixer | C | SDL2_mixer (PCM) | SDL2 Wayland | SDL2 KeyboardEvent | 🔴 | — | regression: doesn't launch |
| 5 | [LVGL_HelloWorld](./LVGL_HelloWorld) | LVGL v9.2 | C | — | `lv_linux_fbdev` → /dev/fb0 | `lv_linux_evdev` keypad | 🟢 | 🟢 | Enter not routed to button (`lv_group` wiring) |
| 6 | [Rust_FrameBuffer_HelloWorld](./Rust_FrameBuffer_HelloWorld) | embedded-graphics | Rust | — | `framebuffer` crate RGB565 | `evdev-rs` | 🟢 | 🟢 | |
| 7 | [Python_FrameBuffer_HelloWorld](./Python_FrameBuffer_HelloWorld) | Pillow + numpy | Python 3 | — | `mmap` + vectorized RGB→RGB565 | python-evdev | 🟢 | 🟢 | |
| 8 | [Python_Tkinter_HelloWorld](./Python_Tkinter_HelloWorld) | Tkinter + Xvfb | Python 3 | — | Xvfb → ffmpeg / PIL mirror → /dev/fb0 | Tk key bindings | 🟢 | 🟢 | residue on exit: last frame lingers until launcher redraws |
| 9 | [Qt_HelloWorld](./Qt_HelloWorld) | Qt5 Widgets | C++ | — | Qt `-platform linuxfb` | Qt QKeyEvent | 🟢 | 🟢 | |
| 10 | [PyQt5_HelloWorld](./PyQt5_HelloWorld) | Qt5 via PyQt5 | Python 3 | — | Qt `-platform linuxfb` | Qt QKeyEvent | 🟢 | 🟢 | residue on exit: last frame lingers until launcher redraws |
| 11 | [Slint_HelloWorld](./Slint_HelloWorld) | Slint 1.9 | Rust | — | `backend-linuxkms-noseat` | Slint KeyEvent | 🟢 | 🔴 | short-press ESC not caught; only long-press exits |
| 12 | [Demo_Clock](./Demo_Clock) | raw /dev/fb0 | C | — | mmap RGB565 | evdev raw | 🟢 | 🟢 | |
| 13 | [Demo_SysDashboard](./Demo_SysDashboard) | Pillow | Python 3 | — | mmap RGB565 | python-evdev | 🟢 | 🟢 | |
| 14 | [Demo_WifiScan](./Demo_WifiScan) | Pillow + `iw` | Python 3 | — | mmap RGB565 | python-evdev | 🟢 | 🔴 | short-press ESC ignored; long-press exits |
| 15 | [Demo_ImageViewer](./Demo_ImageViewer) | Pillow | Python 3 | — | mmap RGB565 | python-evdev | 🔴 | — | regression: doesn't launch |
| 16 | [Demo_2048](./Demo_2048) | raw /dev/fb0 | C | — | mmap RGB565 | evdev raw | 🔴 | — | crashes on launch |
| 17 | [Demo_Matrix](./Demo_Matrix) | raw /dev/fb0 | C | — | mmap RGB565 | evdev raw | 🔴 | — | launches then exits |
| 18 | [Demo_MarkdownReader](./Demo_MarkdownReader) | Pillow | Python 3 | — | mmap RGB565 | python-evdev | 🟢 | 🔴 | displays but no keys reach the app (still reads stdin) |
| 19 | [Demo_MusicSpectrum](./Demo_MusicSpectrum) | raw /dev/fb0 + ALSA | C | ALSA capture (any `plughw:*`) | mmap RGB565 | evdev raw | 🔴 | — | regression: launches then exits immediately |
| 20 | [Game_Tetris](./Game_Tetris) | raw /dev/fb0 | C | — | mmap RGB565 | evdev raw | 🟢 | 🟢 | |
| 21 | [Game_Minesweeper](./Game_Minesweeper) | raw /dev/fb0 | C | — | mmap RGB565 | evdev raw | 🟢 | 🟢 | |
| 22 | [Game_Sokoban](./Game_Sokoban) | raw /dev/fb0 | C | — | mmap RGB565 | evdev raw | 🟢 | 🟢 | |
| 23 | [Game_Pong](./Game_Pong) | raw /dev/fb0 | C | — | mmap RGB565 | evdev raw | 🟢 | 🟢 | |
| 24 | [Game_Flappy](./Game_Flappy) | raw /dev/fb0 | C | — | mmap RGB565 | evdev raw | 🟢 | 🟢 | |
| 25 | [Game_LunarLander](./Game_LunarLander) | raw /dev/fb0 | C | — | mmap RGB565 | evdev raw | 🟢 | 🟢 | |
| 26 | [Game_Invaders](./Game_Invaders) | SDL2 | C | — | SDL2 Wayland | SDL2 KeyboardEvent | 🔴 | — | regression: doesn't launch |
| 27 | [Game_Asteroids](./Game_Asteroids) | SDL2 | C | — | SDL2 Wayland | SDL2 KeyboardEvent | 🔴 | — | regression: doesn't launch |
| 28 | [Game_TicTacToe](./Game_TicTacToe) | SDL2 | C | — | SDL2 Wayland | SDL2 KeyboardEvent | 🔴 | — | regression: doesn't launch |
| 29 | [NES_Emulator](./NES_Emulator) | SDL2 + LaiNES | C++ | SDL2 audio | SDL2 Wayland (`scaletoheight` / `cropheight`, `V` toggles) | SDL2 KeyboardEvent | 🔴 | — | regression: doesn't launch |

**Summary.** Still-broken (🔴 Launch): all 6 SDL2 apps (SDL2_Hello,
SDL2_Breakout, Invaders, Asteroids, TicTacToe, NES), plus
Demo_ImageViewer, Demo_MusicSpectrum, Demo_2048, Demo_Matrix. ESC
short-press regressions: Slint_HelloWorld, Demo_WifiScan,
Demo_MarkdownReader. Nav-key regressions: LVGL Enter not routed.
Exit-residue regressions: Python_Tkinter_HelloWorld,
PyQt5_HelloWorld.

> **SDL2 backend note.** The board runs `labwc` as a Wayland compositor
> that holds DRM master, so SDL2's default `KMSDRM` driver fails with
> `KMSDRM not available`. Every SDL2 example wrapper exports
> `SDL_VIDEODRIVER=wayland` (plus `WAYLAND_DISPLAY` / `XDG_RUNTIME_DIR`);
> the Wayland surface is then mirrored onto `/dev/fb0` by the system's
> `grim | ppm2fb` loop.
>
> **Keyboard device.** `/dev/input/by-path/platform-3f804000.i2c-event`
> (TCA8418 keypad). User `pi` is in the `input` and `video` groups, so
> no `sudo` is needed to read evdev or write `/dev/fb0`.

## Packaging &amp; CI

Every runnable example ships a `packaging/` directory that produces an
APPLaunch-compatible `.deb`. See [`docs/PACKAGING.md`](./docs/PACKAGING.md) for
the full contract and [`.github/workflows/build-debs.yml`](./.github/workflows/build-debs.yml)
for the workflow.

Three ways to get a `.deb`, in increasing order of "for day-to-day dev":

| Flow | When to use | Host requirement |
| --- | --- | --- |
| 1. **CI on GitHub Actions (recommended)** | Always reachable, reproducible, produces release assets | nothing — just push |
| 2. **Native build on the CM0 (aarch64)** | Small single-file fb / SDL demos | device only |
| 3. **Cross-build from macOS / x86_64 Linux** | Iterating locally on heavier examples | Docker |

> ⚠️ **CM0 is not suitable for host-compiling large projects.** The board has
> 512 MB RAM and a single SD-card backed swap. `make -j2` on anything
> non-trivial (Qt, LVGL + deps, NES_Emulator, Slint, Flutter) has been observed
> to **OOM-kill even with swap enabled**. Use native CM0 build only for the
> tiny C / fb / SDL demos; anything bigger should go through flow 1 or 3.

### 1) CI: online build on `ubuntu-24.04-arm`

The workflow runs on a native aarch64 GitHub runner, so no emulation or
sysroot is involved. It discovers every `*/packaging/meta.env`, builds each
`.deb` in parallel, and:

- On every push / PR → uploads each `.deb` as a **workflow artifact**.
- On a published **Release** → attaches every `.deb` to the release.

```bash
# trigger a build by pushing to a CI branch
git checkout -b ci/my-change
git push -u origin ci/my-change

# watch the run
gh run watch -R <owner>/CardputerZero-Examples

# grab all .debs from the latest successful run on your branch
gh run download -R <owner>/CardputerZero-Examples \
    -n '*-arm64-deb' --dir dist/
```

To cut a release that publishes every example's `.deb`:

```bash
gh release create v0.1.0 --prerelease --notes "Batch build"
# CI's `release: [published]` trigger then attaches dist/*.deb
```

### 2) Native build on the CM0 (small demos only)

Works for pure-C / fb / small SDL examples. No sysroot needed.

```bash
# On your laptop
rsync -av --exclude target --exclude build ./ pi@<device-ip>:~/CardputerZero-Examples/

# On the device
ssh pi@<device-ip>
cd ~/CardputerZero-Examples/<Example_Dir>
# follow that example's README (Makefile / CMake / cargo / python)

# Or build a .deb directly on-device (same script as CI uses):
cd ~/CardputerZero-Examples
./scripts/pack-deb.sh FrameBuffer_HelloWorld
```

For the fb demos you'll usually need sudo (or `usermod -aG video pi`).

**Do not** try this for `NES_Emulator`, `Qt_HelloWorld`, `Slint_HelloWorld`,
or anything pulling LVGL via FetchContent — the link step will OOM.

### 3) Cross-build from macOS / x86_64 Linux (via Docker)

Use a Debian arm64 container under `docker buildx` / QEMU user-mode so the
build toolchain matches the device's Debian-like root.

```bash
# one-time: enable arm64 emulation on the host
docker run --rm --privileged tonistiigi/binfmt --install arm64

# build one example's .deb (mirrors CI)
docker run --rm --platform linux/arm64 \
    -v "$PWD":/src -w /src debian:bookworm bash -lc '
      apt-get update &amp;&amp; apt-get install -y \
        build-essential cmake pkg-config fakeroot dpkg-dev \
        libsdl2-dev libsdl2-ttf-dev libsdl2-mixer-dev \
        python3-pil fonts-wqy-microhei
      ./scripts/pack-deb.sh FrameBuffer_HelloWorld
    '
```

The `.deb` lands in `dist/`. Deploy exactly like flow 2.

> Apple Silicon Macs emulate aarch64 natively (no QEMU translation for the
> binary, only for syscall ABI), so this path is actually faster than the CM0.
> On x86_64 hosts it uses QEMU user-mode and is ~5–10× slower than CI — fine
> for iteration, not for batch builds.

### Install / deploy (all flows)

```bash
scp dist/framebuffer-hello_0.1-m5stack1_arm64.deb pi@<device-ip>:/tmp/
ssh pi@<device-ip> 'sudo dpkg -i /tmp/framebuffer-hello_0.1-m5stack1_arm64.deb \
    &amp;&amp; sudo systemctl restart APPLaunch.service'
```

### Publishing to the M5 firmware store

Runnable `.deb`s can also be pushed to M5's FirmwareManagementV3 so they show
up in the online store. The CLI lives in
`M5CardputerZero-UserDemo-Features/doc/firmware_manager.py`.

```bash
# one-time: log in (token saved next to the script as .fw_manager_token)
python3 firmware_manager.py login --email <you> --password <pwd>

# upload a CI-built .deb
HTTPS_PROXY= HTTP_PROXY= ALL_PROXY= no_proxy='*' \
python3 firmware_manager.py upload \
    --file dist/framebuffer-hello_0.1-m5stack1_arm64.deb \
    --avatar FrameBuffer_HelloWorld/packaging/icon.png \
    --name "FbHello" --sku M5Stack-Zero --version v1.0.5 \
    --description "FrameBuffer hello" --class firmware
```

Gotchas (current M5 backend behavior, subject to change):

- Version is validated **per-SKU globally**, not per-firmwareName: the new
  version must be strictly greater than the highest existing version under
  the same SKU. Check current state with
  `python3 firmware_manager.py list --sku M5Stack-Zero` and bump accordingly.

---

<details>
<summary><b>简体中文</b></summary>

### 打包与 CI —— 中文说明

每个可运行的示例都带一个 `packaging/` 目录，产出符合 APPLaunch 规范的
`.deb` 包。完整规则见 [`docs/PACKAGING.md`](./docs/PACKAGING.md)，工作流定义见
[`.github/workflows/build-debs.yml`](./.github/workflows/build-debs.yml)。

生成 `.deb` 的三种途径：

| 方式 | 适用场景 | 主机要求 |
| --- | --- | --- |
| 1. **GitHub Actions 在线编译（推荐）** | 稳定可复现，并可自动发布到 Release | 无需本地工具链，push 即可 |
| 2. **CM0 设备本机编译** | 纯 C / fb / 轻量级 SDL 示例 | 仅设备 |
| 3. **macOS / x86_64 Linux 交叉编译** | 本地快速迭代大型示例 | Docker |

> ⚠️ **CM0 不适合编译大型项目。** 整板只有 512 MB 内存 + SD 卡 swap，
> `make -j2` 编译 Qt、LVGL 及依赖、NES_Emulator、Slint、Flutter 时
> **即使开了 swap 也会被 OOM 杀掉**。设备本机编译只建议用于小型 C / fb / SDL demo，
> 其他统一走方式 1 或方式 3。

**1) CI 在 `ubuntu-24.04-arm` 上原生 aarch64 构建**

在 `ci/**` 分支 push 即可触发，每条示例的 `.deb` 并行产出：push / PR 上传为
workflow artifact，published Release 上自动挂到 Release Assets。

```bash
git checkout -b ci/my-change
git push -u origin ci/my-change
gh run watch -R <owner>/CardputerZero-Examples
gh run download -R <owner>/CardputerZero-Examples -n '*-arm64-deb' --dir dist/
```

**2) CM0 本机编译（仅限小示例）**

```bash
rsync -av --exclude target --exclude build ./ pi@<device-ip>:~/CardputerZero-Examples/
ssh pi@<device-ip>
cd ~/CardputerZero-Examples && ./scripts/pack-deb.sh FrameBuffer_HelloWorld
```

`NES_Emulator`、`Qt_HelloWorld`、`Slint_HelloWorld`
以及任何通过 FetchContent 拉 LVGL 的示例**不要**在设备上编译，链接阶段会 OOM。

**3) macOS / x86_64 交叉编译（Docker + QEMU）**

```bash
docker run --rm --privileged tonistiigi/binfmt --install arm64
docker run --rm --platform linux/arm64 -v "$PWD":/src -w /src debian:bookworm \
    bash -lc 'apt-get update && apt-get install -y build-essential cmake pkg-config \
      fakeroot dpkg-dev libsdl2-dev libsdl2-ttf-dev libsdl2-mixer-dev \
      python3-pil fonts-wqy-microhei && ./scripts/pack-deb.sh FrameBuffer_HelloWorld'
```

Apple Silicon Mac 原生跑 aarch64，比 CM0 还快；x86_64 主机走 QEMU user-mode，
比 CI 慢约 5–10 倍，但本地迭代足够。

**部署（三种方式通用）**

```bash
scp dist/xxx_arm64.deb pi@<device-ip>:/tmp/
ssh pi@<device-ip> 'sudo dpkg -i /tmp/xxx_arm64.deb && sudo systemctl restart APPLaunch.service'
```

**发布到 M5 固件商店**

用 `M5CardputerZero-UserDemo-Features/doc/firmware_manager.py`：

```bash
python3 firmware_manager.py login --email <你> --password <密码>
python3 firmware_manager.py upload \
    --file dist/framebuffer-hello_0.1-m5stack1_arm64.deb \
    --avatar FrameBuffer_HelloWorld/packaging/icon.png \
    --name "FbHello" --sku M5Stack-Zero --version v1.0.5 \
    --description "FrameBuffer hello" --class firmware
```

⚠️ 版本号校验是**按 SKU 全局**的（不是按 firmwareName），新版本必须严格大于
同 SKU 下现有最高版本。先用 `firmware_manager.py list --sku M5Stack-Zero`
查询当前最高版本再递增。

</details>

<details>
<summary><b>繁體中文</b></summary>

### 打包與 CI —— 繁體中文說明

每個可執行的範例都附帶 `packaging/` 目錄，產出符合 APPLaunch 規範的 `.deb`。
完整規則見 [`docs/PACKAGING.md`](./docs/PACKAGING.md)，工作流定義見
[`.github/workflows/build-debs.yml`](./.github/workflows/build-debs.yml)。

產生 `.deb` 的三種方式：

| 方式 | 適用情境 | 主機需求 |
| --- | --- | --- |
| 1. **GitHub Actions 線上編譯（推薦）** | 穩定可重現，可自動發佈至 Release | 無需本機工具鏈，push 即可 |
| 2. **CM0 裝置本機編譯** | 純 C / fb / 輕量 SDL 範例 | 僅裝置 |
| 3. **macOS / x86_64 Linux 交叉編譯** | 本機快速迭代較大範例 | Docker |

> ⚠️ **CM0 不適合編譯大型專案。** 主機板僅 512 MB 記憶體 + SD 卡 swap，
> `make -j2` 編譯 Qt、LVGL 及其依賴、NES_Emulator、Slint、Flutter 時
> **即便啟用 swap 亦會被 OOM 殺掉**。裝置本機編譯僅建議用於小型 C / fb / SDL
> demo，其餘一律走方式 1 或方式 3。

**1) CI 於 `ubuntu-24.04-arm` 原生 aarch64 建置**

於 `ci/**` 分支 push 即可觸發，每個範例的 `.deb` 會平行產出；push / PR 上傳為
workflow artifact，published Release 會自動附加到 Release Assets。

```bash
git checkout -b ci/my-change
git push -u origin ci/my-change
gh run watch -R <owner>/CardputerZero-Examples
gh run download -R <owner>/CardputerZero-Examples -n '*-arm64-deb' --dir dist/
```

**2) CM0 本機編譯（僅限小範例）**

```bash
rsync -av --exclude target --exclude build ./ pi@<device-ip>:~/CardputerZero-Examples/
ssh pi@<device-ip>
cd ~/CardputerZero-Examples && ./scripts/pack-deb.sh FrameBuffer_HelloWorld
```

`NES_Emulator`、`Qt_HelloWorld`、`Slint_HelloWorld`
以及任何透過 FetchContent 拉 LVGL 的範例**請勿**在裝置上編譯,連結階段會 OOM。

**3) macOS / x86_64 交叉編譯（Docker + QEMU）**

```bash
docker run --rm --privileged tonistiigi/binfmt --install arm64
docker run --rm --platform linux/arm64 -v "$PWD":/src -w /src debian:bookworm \
    bash -lc 'apt-get update && apt-get install -y build-essential cmake pkg-config \
      fakeroot dpkg-dev libsdl2-dev libsdl2-ttf-dev libsdl2-mixer-dev \
      python3-pil fonts-wqy-microhei && ./scripts/pack-deb.sh FrameBuffer_HelloWorld'
```

Apple Silicon Mac 原生執行 aarch64,比 CM0 還快;x86_64 主機走 QEMU user-mode,
較 CI 慢約 5–10 倍,但供本機迭代已足夠。

**部署（三種方式通用）**

```bash
scp dist/xxx_arm64.deb pi@<device-ip>:/tmp/
ssh pi@<device-ip> 'sudo dpkg -i /tmp/xxx_arm64.deb && sudo systemctl restart APPLaunch.service'
```

**發佈至 M5 韌體商店**

使用 `M5CardputerZero-UserDemo-Features/doc/firmware_manager.py`：

```bash
python3 firmware_manager.py login --email <您> --password <密碼>
python3 firmware_manager.py upload \
    --file dist/framebuffer-hello_0.1-m5stack1_arm64.deb \
    --avatar FrameBuffer_HelloWorld/packaging/icon.png \
    --name "FbHello" --sku M5Stack-Zero --version v1.0.5 \
    --description "FrameBuffer hello" --class firmware
```

⚠️ 版本號校驗為**按 SKU 全域**（並非按 firmwareName），新版本必須嚴格大於
同 SKU 下現有最高版本。可先用 `firmware_manager.py list --sku M5Stack-Zero`
查詢目前最高版本再遞增。

</details>

<details>
<summary><b>日本語</b></summary>

### パッケージング & CI — 日本語説明

実行可能なすべてのサンプルには `packaging/` ディレクトリが付属し、APPLaunch
準拠の `.deb` を生成します。仕様の全文は
[`docs/PACKAGING.md`](./docs/PACKAGING.md)、ワークフロー定義は
[`.github/workflows/build-debs.yml`](./.github/workflows/build-debs.yml) を参照。

`.deb` を得る 3 つの方法：

| 方法 | 適した用途 | ホスト要件 |
| --- | --- | --- |
| 1. **GitHub Actions によるオンラインビルド（推奨）** | 再現性が高く、Release に自動添付可能 | ローカルツール不要、push するだけ |
| 2. **CM0 実機ネイティブビルド** | 純 C / fb / 軽量 SDL サンプル | 実機のみ |
| 3. **macOS / x86_64 Linux クロスビルド** | 大きめのサンプルを手元で反復開発 | Docker |

> ⚠️ **CM0 では大きなプロジェクトのホストコンパイルは不可。** 本体は 512 MB RAM
> + SD カード swap のみで、Qt・LVGL と依存関係・NES_Emulator・Slint・Flutter
> を `make -j2` すると **swap があっても OOM で kill される** ことが確認されています。
> 実機ビルドは小さな C / fb / SDL デモに限定し、それ以外は方法 1 または方法 3 を使用してください。

**1) CI: `ubuntu-24.04-arm` でネイティブ aarch64 ビルド**

`ci/**` ブランチへ push すると起動し、サンプルごとに `.deb` を並列ビルドします。
push / PR 時は workflow artifact として、published Release 時は Release Assets
として自動添付されます。

```bash
git checkout -b ci/my-change
git push -u origin ci/my-change
gh run watch -R <owner>/CardputerZero-Examples
gh run download -R <owner>/CardputerZero-Examples -n '*-arm64-deb' --dir dist/
```

**2) CM0 実機ビルド（小サンプル限定）**

```bash
rsync -av --exclude target --exclude build ./ pi@<device-ip>:~/CardputerZero-Examples/
ssh pi@<device-ip>
cd ~/CardputerZero-Examples && ./scripts/pack-deb.sh FrameBuffer_HelloWorld
```

`NES_Emulator`、`Qt_HelloWorld`、`Slint_HelloWorld`、および
FetchContent 経由で LVGL を取得するサンプルは **実機でビルドしないでください** —
リンク段階で OOM になります。

**3) macOS / x86_64 クロスビルド（Docker + QEMU）**

```bash
docker run --rm --privileged tonistiigi/binfmt --install arm64
docker run --rm --platform linux/arm64 -v "$PWD":/src -w /src debian:bookworm \
    bash -lc 'apt-get update && apt-get install -y build-essential cmake pkg-config \
      fakeroot dpkg-dev libsdl2-dev libsdl2-ttf-dev libsdl2-mixer-dev \
      python3-pil fonts-wqy-microhei && ./scripts/pack-deb.sh FrameBuffer_HelloWorld'
```

Apple Silicon Mac では aarch64 がネイティブ動作するため CM0 より高速です。
x86_64 ホストでは QEMU user-mode のため CI より 5–10 倍遅いものの、
ローカル反復には十分です。

**デプロイ（3 方式共通）**

```bash
scp dist/xxx_arm64.deb pi@<device-ip>:/tmp/
ssh pi@<device-ip> 'sudo dpkg -i /tmp/xxx_arm64.deb && sudo systemctl restart APPLaunch.service'
```

**M5 ファームウェアストアへの公開**

`M5CardputerZero-UserDemo-Features/doc/firmware_manager.py` を使用：

```bash
python3 firmware_manager.py login --email <あなた> --password <パスワード>
python3 firmware_manager.py upload \
    --file dist/framebuffer-hello_0.1-m5stack1_arm64.deb \
    --avatar FrameBuffer_HelloWorld/packaging/icon.png \
    --name "FbHello" --sku M5Stack-Zero --version v1.0.5 \
    --description "FrameBuffer hello" --class firmware
```

⚠️ バージョン番号は **SKU 単位でグローバル** に検証されます（firmwareName 別ではない）。
新バージョンは同一 SKU 内の現在の最高バージョンより厳密に大きい必要があります。
`firmware_manager.py list --sku M5Stack-Zero` で現在の最高値を確認してから
インクリメントしてください。

</details>

## Conventions for new examples

1. **One directory per example**, named `<Stack>_<Purpose>` (e.g. `Go_FrameBuffer_HelloWorld`).
2. **Never hardcode 320×170** in the fb write path — always read vinfo; warn if mismatched.
3. **Keyboard only** — no pointer / touch assumptions.
4. **Keep deps minimal** and document install commands in the README.
5. **No binary assets committed** — provide `sox` / ffmpeg commands to generate them.
6. README sections: 目的/Purpose · 依赖/Deps · 构建/Build · 运行/Run · 部署/Deploy.

See [CHECKLIST.md](./CHECKLIST.md) for what's planned, what's done, and the full
feasibility table across Framebuffer / SDL2 / LVGL / Rust / Python / Qt / PyQt /
Slint on this target.
