#!/usr/bin/env bash
set -euo pipefail

install -D -m 0755 breakout "$STAGE$APP_INSTALL_DIR/breakout"

if [[ -d assets ]]; then
    mkdir -p "$STAGE$APP_INSTALL_DIR/assets"
    cp -r assets/. "$STAGE$APP_INSTALL_DIR/assets/"
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
cd /usr/share/APPLaunch/apps/sdl2-breakout
exec /usr/share/APPLaunch/apps/sdl2-breakout/breakout "$@"
EOF
chmod 0755 "$STAGE$INSTALL_PREFIX/bin/$PKG_NAME"
