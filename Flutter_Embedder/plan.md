# Flutter on CM Zero - Porting Plan

Detailed checklist if a full port is ever attempted. Each item is a
non-trivial task; the list is intentionally ordered.

## 1. Toolchain

- [ ] Install `flutter` SDK on host (Linux x86_64 recommended).
- [ ] Install aarch64 cross toolchain
      (`gcc-aarch64-linux-gnu`, `g++-aarch64-linux-gnu`).
- [ ] Install `cmake`, `ninja`, `pkg-config`, `clang`.
- [ ] Clone flutter-elinux:
      `git clone https://github.com/sony/flutter-elinux`.

## 2. Engine

- [ ] Fetch matching Flutter engine sources (`depot_tools` + `gclient sync`).
- [ ] Configure engine for `linux_arm64` with software renderer only:
      `--no-enable-vulkan --no-enable-gl --enable-software`.
- [ ] Build `libflutter_engine.so` (release).
- [ ] Strip symbols, measure size (expect > 15 MB).

## 3. Embedder

- [ ] Choose backend: `eglstream`, `wayland`, or custom fbdev.
- [ ] For fbdev: write a minimal embedder that:
  - [ ] Opens `/dev/fb0`, reads `fb_var_screeninfo` (320x170, RGB565).
  - [ ] Registers `FlutterEngineRun` with a software rasterizer callback.
  - [ ] On each `present`, `memcpy` the rendered RGBA buffer, convert to
        RGB565, write to mapped framebuffer.
- [ ] Wire evdev keyboard events into
      `FlutterEngineSendKeyEvent` / text input.

## 4. App

- [ ] `flutter create --platforms=linux tiny_app`.
- [ ] Lock `MediaQuery.size` to 320x170 logical pixels.
- [ ] Disable animations where possible (`MediaQuery.disableAnimations`).
- [ ] Use `RepaintBoundary` aggressively to reduce repaint cost.
- [ ] Bundle assets minimally; avoid large fonts (prefer a single 10-14 px
      bitmap-ish font).

## 5. Deployment

- [ ] Copy `libflutter_engine.so`, `icudtl.dat`, bundle assets, embedder
      binary to `/opt/tiny_app/` on device.
- [ ] systemd unit or rc script to launch on boot.
- [ ] Confirm `/dev/fb0` permissions (add user to `video` group).

## 6. Known blockers

- RAM pressure: Dart VM + engine typically > 100 MB.
- No GPU: every frame is CPU-rasterized; expect < 10 FPS for non-trivial
  UI at 320x170.
- Binary footprint: total payload > 25 MB vs. a Slint binary in single-digit MB.

## 7. Exit criteria for "works"

- [ ] Launches in under 10 s cold.
- [ ] Renders a static screen with text and one animated widget at >= 15 FPS.
- [ ] Keyboard input reaches a `TextField`.
- [ ] Memory residency < 200 MB RSS.

## References

- flutter-pi: <https://github.com/ardera/flutter-pi>
- flutter-elinux: <https://github.com/sony/flutter-elinux>
- Flutter embedder API: <https://github.com/flutter/engine/tree/main/shell/platform/embedder>
