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
| SSH | `pi@192.168.50.150` (password `pi`) |

Driver overlays: <https://github.com/m5stack/m5stack-linux-dtoverlays/tree/main/modules/CardputerZero>

## Examples index

| Directory | Stack | Status | Notes |
| --- | --- | --- | --- |
| [FrameBuffer_HelloWorld](./FrameBuffer_HelloWorld) | C / `/dev/fb0` + evdev | ✅ | Pure C11, no deps. Reads vinfo, supports 16/32 bpp. ESC/Q quits. |
| [FrameBuffer_Game](./FrameBuffer_Game) | C / fb + evdev | ✅ | Snake, 10 fps, optional `aplay` beep. |
| [SDL2_HelloWorld](./SDL2_HelloWorld) | C / SDL2 + SDL2_ttf + SDL2_mixer | ✅ | Software renderer, DejaVuSans 14 pt, FPS counter. |
| [SDL2_Game](./SDL2_Game) | C / SDL2 + mixer | ✅ | Breakout, 60 Hz fixed timestep. |
| [LVGL_HelloWorld](./LVGL_HelloWorld) | C / LVGL v9.2 | ✅ | CMake FetchContent; `lv_linux_fbdev` + `lv_linux_evdev`. |
| [Rust_FrameBuffer_HelloWorld](./Rust_FrameBuffer_HelloWorld) | Rust / `framebuffer` + `embedded-graphics` | ✅ | RGB565 + 32 bpp fallback; cross build docs. |
| [Python_FrameBuffer_HelloWorld](./Python_FrameBuffer_HelloWorld) | Python 3 / mmap + Pillow + numpy | ✅ | Vectorized RGB→RGB565, evdev quit, SIGINT clean. |
| [Python_Tkinter_HelloWorld](./Python_Tkinter_HelloWorld) | Python 3 / Tkinter + Xvfb | ✅ | Two paths: ffmpeg mirror, or PIL.ImageGrab self-mirror. |
| [Qt_HelloWorld](./Qt_HelloWorld) | C++ Qt5 Widgets | ✅ | `-platform linuxfb`; qmake or CMake. |
| [PyQt5_HelloWorld](./PyQt5_HelloWorld) | Python PyQt5 | ✅ | `run.sh` sets linuxfb + evdev env. |
| [Slint_HelloWorld](./Slint_HelloWorld) | Rust Slint 1.9 | ✅ | `backend-linuxkms-noseat` + software renderer. |
| [Flutter_Embedder](./Flutter_Embedder) | Flutter (flutter-pi / flutter-elinux) | 📄 stub | No GPU on CM0 → feasibility doc + plan only. |
| [GPUI_Stub](./GPUI_Stub) | Zed GPUI (Rust) | 📄 stub | Requires Vulkan/Metal; CM0 cannot run GPUI. |
| [Demo_Clock](./Demo_Clock) | C / fb | ✅ | Big-font HH:MM:SS + date. |
| [Demo_SysDashboard](./Demo_SysDashboard) | Python / fb | ✅ | CPU/mem/temp/IP. |
| [Demo_WifiScan](./Demo_WifiScan) | Python + `iw` | ✅ | Scrollable SSID list. |
| [Demo_ImageViewer](./Demo_ImageViewer) | Python + Pillow | ✅ | ←/→ browse a folder. |
| [Demo_2048](./Demo_2048) | C / fb | ✅ | 4×4 keyboard 2048. |
| [Demo_Matrix](./Demo_Matrix) | C / fb | ✅ | Matrix-rain screensaver. |
| [Demo_MarkdownReader](./Demo_MarkdownReader) | Python + Pillow | ✅ | Paged plain-text reader. |
| [Demo_MusicSpectrum](./Demo_MusicSpectrum) | C / ALSA + fb | 📄 skeleton | ALSA capture loop done; FFT + render TODO. |

Legend: ✅ runnable · 📄 doc / skeleton only

## Packaging &amp; CI

Every runnable example ships a `packaging/` directory that produces an
APPLaunch-compatible `.deb`. See [`docs/PACKAGING.md`](./docs/PACKAGING.md) for
the full contract and [`.github/workflows/build-debs.yml`](./.github/workflows/build-debs.yml)
for CI.

```bash
# build one example's deb locally
./scripts/pack-deb.sh FrameBuffer_HelloWorld

# build every example's deb in parallel (into dist/)
./scripts/pack-all.sh

# install on device
scp dist/framebuffer-hello_0.1-m5stack1_arm64.deb pi@192.168.50.150:/tmp/
ssh pi@192.168.50.150 'sudo dpkg -i /tmp/framebuffer-hello_0.1-m5stack1_arm64.deb \
    &amp;&amp; sudo systemctl restart APPLaunch.service'
```

CI runs on `ubuntu-24.04-arm` (native aarch64), discovers every `*/packaging/meta.env`,
builds each `.deb` in parallel, uploads as artifacts, and on a published Release
attaches them all to the release.

## Build &amp; run

Everything builds **natively on the CM0** — no sysroot needed. Typical flow:

```bash
# On your laptop
rsync -av --exclude target --exclude build ./ pi@192.168.50.150:~/CardputerZero-Examples/

# On the device
ssh pi@192.168.50.150
cd ~/CardputerZero-Examples/<Example_Dir>
# follow that example's README (Makefile / CMake / cargo / python)
```

For the fb demos you'll usually need sudo (or `usermod -aG video pi`). Every
README covers install prerequisites, build command, run command, and the SSH
deploy command.

## Conventions for new examples

1. **One directory per example**, named `<Stack>_<Purpose>` (e.g. `Go_FrameBuffer_HelloWorld`).
2. **Never hardcode 320×170** in the fb write path — always read vinfo; warn if mismatched.
3. **Keyboard only** — no pointer / touch assumptions.
4. **Keep deps minimal** and document install commands in the README.
5. **No binary assets committed** — provide `sox` / ffmpeg commands to generate them.
6. README sections: 目的/Purpose · 依赖/Deps · 构建/Build · 运行/Run · 部署/Deploy.

See [CHECKLIST.md](./CHECKLIST.md) for what's planned, what's done, and the full
feasibility table across Framebuffer / SDL2 / LVGL / Rust / Python / Qt / PyQt /
Flutter / GPUI / Slint on this target.
