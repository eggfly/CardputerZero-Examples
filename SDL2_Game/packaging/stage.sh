#!/usr/bin/env bash
set -euo pipefail

install -D -m 0755 breakout "$APP_INSTALL_DIR/breakout"

if [[ -d assets ]]; then
    mkdir -p "$APP_INSTALL_DIR/assets"
    cp -r assets/. "$APP_INSTALL_DIR/assets/"
fi

cat >"$STAGE$INSTALL_PREFIX/bin/$PKG_NAME" <<'EOF'
#!/bin/sh
export SDL_VIDEODRIVER=${SDL_VIDEODRIVER:-KMSDRM}
cd /usr/share/APPLaunch/apps/sdl2-breakout
exec /usr/share/APPLaunch/apps/sdl2-breakout/breakout "$@"
EOF
chmod 0755 "$STAGE$INSTALL_PREFIX/bin/$PKG_NAME"
