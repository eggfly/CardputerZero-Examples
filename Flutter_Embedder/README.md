# Flutter Embedder on CardputerZero (CM Zero)

> Status: Feasibility / planning stub. This directory does NOT compile.

## Target hardware

- CM Zero (aarch64, RP3A0, 512 MB RAM)
- Display: 320x170 ST7789V via `/dev/fb0`
- No GPU (no GLES / Vulkan / Metal)
- Input: evdev keyboard, no touchscreen

## English

Flutter upstream does not ship an official framebuffer (`/dev/fb0`) embedder.
The two community embedders usually referenced are:

- **flutter-pi** - <https://github.com/ardera/flutter-pi>
  - Requires GBM/DRM and at least GLES2.
  - CM Zero has NO GPU, so flutter-pi is not usable here.
- **flutter-elinux** (Sony) - <https://github.com/sony/flutter-elinux>
  - Supports `--target-backend-type=wayland` or `x11`.
  - A software rendering path exists but is slow on a 1 GHz Cortex-A class SoC.

### Feasibility verdict

Not practical on CM Zero for real applications. A demo at low FPS
(single-digit) is possible via flutter-elinux + swrast, but the Flutter
engine binary size (tens of MB) and RAM footprint (well above 100 MB)
already consume a large fraction of the 512 MB budget.

### Minimal steps IF you still want to try

1. On a dev machine:
   `flutter create --platforms=linux tiny_app`
2. Edit `linux/my_application.cc` and force the window to 320x170.
3. Cross-compile using the flutter-elinux instructions
   (<https://github.com/sony/flutter-elinux>).
4. Copy the engine bundle to the device and run:
   `flutter-elinux-runner --bundle-path=/opt/tiny_app/bundle`.

### Toy alternative

Run Flutter on a desktop, render to an offscreen surface, and push frames
over SSH as raw RGB images to `/dev/fb0`. This is a demo trick, not a
product path.

## 中文

Flutter 官方上游没有提供基于 framebuffer (`/dev/fb0`) 的 embedder。
常被提及的两个社区 embedder:

- **flutter-pi** - <https://github.com/ardera/flutter-pi>
  - 依赖 GBM/DRM,并至少需要 GLES2。
  - CM Zero 没有 GPU,不可用。
- **flutter-elinux** - <https://github.com/sony/flutter-elinux>
  - 支持 wayland / x11 后端,有软件渲染路径,但速度很慢。

结论:在 CM Zero 上不实用,只能做低帧率 demo。Flutter 引擎体积
(几十 MB)和运行时内存(>100 MB)对 512 MB 的板子压力极大。

如仍要尝试,流程同上英文部分。更现实的方案是用 Slint / LVGL / egui。

## See also

- `../Slint_HelloWorld/` - recommended replacement (works on fbdev).
- `./plan.md` - detailed porting checklist.
