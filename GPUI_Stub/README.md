# GPUI on CardputerZero (CM Zero) - Feasibility Stub

> Status: Not feasible today. This directory is documentation only.

## What is GPUI

GPUI is the Rust UI framework developed for the Zed editor.
Upstream: <https://github.com/zed-industries/zed> (see `crates/gpui/`).

It is designed around hardware-accelerated backends:

- Metal on macOS
- DirectX on Windows
- Vulkan (via `blade-graphics`) on Linux

## Why it will not run on CM Zero

CM Zero (RP3A0, 512 MB) has:

- No GPU
- No Vulkan driver, no GLES
- 320x170 ST7789V panel on `/dev/fb0`

GPUI does not have a software rasterizer backend. There is no supported
path to render GPUI without a Vulkan/Metal/DirectX device. Even if a
software Vulkan implementation (Mesa `lavapipe` / `llvmpipe`) were
available, the CPU budget on a single Cortex-A class core at ~1 GHz is
far below what is needed for 60 FPS GPU-style compositing.

## Recommended alternatives for CM Zero

| Framework | Backend on CM Zero | Notes |
|---|---|---|
| **Slint** | `backend-linuxkms-noseat` + `renderer-software` | See `../Slint_HelloWorld/`. Best fit. |
| **LVGL** | direct `/dev/fb0` | See `../LVGL_HelloWorld/`. Smallest footprint. |
| **egui** | `eframe` on SDL2 + swrast | Works but heavier. See `../SDL2_HelloWorld/` for SDL2 base. |

## Reopen this when

- Mesa `lavapipe` (software Vulkan) is known to run on armv8 with
  acceptable throughput for the target framerate, AND
- GPUI ships a configuration that tolerates a software-only Vulkan ICD.

Until then: please do not attempt a GPUI port on CM Zero. Open an issue
on this repository if the landscape changes.

## 中文摘要

GPUI 是 Zed 编辑器的 GPU UI 框架,依赖 Metal / DirectX / Vulkan。
CM Zero 没有 GPU,没有 Vulkan 驱动,因此无法运行 GPUI。
推荐改用 Slint(见 `../Slint_HelloWorld/`)或 LVGL。
