#!/usr/bin/env bash
set -euo pipefail

install -D -m 0755 build/nes-emulator "$STAGE$APP_INSTALL_DIR/nes-emulator"

if [[ -d roms ]]; then
    mkdir -p "$STAGE$APP_INSTALL_DIR/roms"
    # Copy only the README; do not ship ROM files.
    if [[ -f roms/README.md ]]; then
        cp roms/README.md "$STAGE$APP_INSTALL_DIR/roms/README.md"
    fi
fi

cat >"$STAGE$INSTALL_PREFIX/bin/$PKG_NAME" <<'EOF'
#!/bin/sh
# CM0 runs a labwc Wayland compositor that owns /dev/dri/card0, so
# SDL2's KMSDRM backend cannot acquire DRM master. Use the Wayland
# backend instead; labwc output is mirrored to /dev/fb0 by the
# system's grim + ppm2fb bridge.
export WAYLAND_DISPLAY=${WAYLAND_DISPLAY:-wayland-0}
export XDG_RUNTIME_DIR=${XDG_RUNTIME_DIR:-/run/user/$(id -u)}
export SDL_VIDEODRIVER=${SDL_VIDEODRIVER:-wayland}
APP_DIR=/usr/share/APPLaunch/apps/nes-emulator
ROM_DIR=${NES_ROM_DIR:-$APP_DIR/roms}
ROM=${NES_ROM:-$(ls -1 "$ROM_DIR"/*.nes 2>/dev/null | head -1)}
if [ -n "$ROM" ]; then
    exec "$APP_DIR/nes-emulator" "$ROM"
fi
exec "$APP_DIR/nes-emulator"
EOF
chmod 0755 "$STAGE$INSTALL_PREFIX/bin/$PKG_NAME"
