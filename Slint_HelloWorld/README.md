# Slint Hello World on CardputerZero (CM Zero)

A minimal Slint 1.9 app targeting the 320x170 panel on CM Zero via the
`linuxkms` backend with the software renderer.

Upstream Slint: <https://github.com/slint-ui/slint>

## English

### Why Slint here

- Pure-Rust UI framework with a small footprint.
- Has a `backend-linuxkms-noseat` backend that drives `/dev/dri/card0`
  directly without a desktop, plus a CPU software renderer
  (`renderer-software`) so no GPU is required.
- Input via `libinput` (keyboard through evdev is automatic).

CM Zero has no GPU, but it does expose `/dev/dri/card0` (DRM node) and
`/dev/fb0`. The linuxkms backend uses DRM dumb buffers, not GL, so it
runs on SoCs without a GPU driver.

### Layout

```
Slint_HelloWorld/
  Cargo.toml
  build.rs
  ui/appwindow.slint
  src/main.rs
```

### Build (host / desktop dev)

```
cargo build --release
```

This will pull Slint 1.9 and build with the linuxkms backend. On a
Linux host with a DRM node you can also run it directly.

### Cross-compile for CM Zero (aarch64)

Using [`cross`](https://github.com/cross-rs/cross):

```
cross build --target aarch64-unknown-linux-gnu --release
```

The resulting binary is at
`target/aarch64-unknown-linux-gnu/release/slint_hello`.

### Run on device

```
scp target/aarch64-unknown-linux-gnu/release/slint_hello pi@<ip>:/tmp/
ssh pi@<ip>
sudo /tmp/slint_hello
```

`sudo` is usually required because DRM master access and libinput both
want elevated privileges. Alternatively add the user to `video` and
`input` groups and set appropriate udev rules.

### Runtime requirements on the device

- Kernel with DRM (`/dev/dri/card0` exists).
- `libinput` and `libudev` present (most Debian/Raspbian images have
  these by default).
- Framebuffer / panel driver bound to the ST7789V so DRM has a CRTC to
  drive.

### Trade-offs

- `backend-winit` + `renderer-software` also works on a desktop but
  requires an X11 / Wayland server, which is usually not present on a
  headless CM Zero.
- `backend-linuxkms-noseat` is the right choice for bare-metal-ish Linux.
- The software renderer uses CPU; a 320x170 window is tiny so frame
  rates are fine for typical UI.

## 中文

### 为什么选 Slint

- 纯 Rust,体积小。
- 提供 `backend-linuxkms-noseat`,直接驱动 `/dev/dri/card0`,配合
  `renderer-software` 软件渲染,不需要 GPU。
- 输入通过 `libinput`(evdev 键盘自动可用)。

### 构建

主机开发:

```
cargo build --release
```

交叉编译到 aarch64:

```
cross build --target aarch64-unknown-linux-gnu --release
```

### 部署到设备

```
scp target/aarch64-unknown-linux-gnu/release/slint_hello pi@<ip>:/tmp/
ssh pi@<ip>
sudo /tmp/slint_hello
```

DRM 通常需要 root 权限,或者把用户加入 `video` / `input` 组。

### 依赖

- 内核提供 DRM 节点 `/dev/dri/card0`。
- 用户空间有 `libinput` / `libudev`。
- ST7789V 有对应的 DRM 驱动(如 `panel-mipi-dbi` 或 `fbtft` + DRM shim)。

按 ESC 触发窗口关闭请求以退出。
