#!/usr/bin/env bash
set -euo pipefail

install -D -m 0755 asteroids "$STAGE$APP_INSTALL_DIR/asteroids"

cat >"$STAGE$INSTALL_PREFIX/bin/$PKG_NAME" <<'EOF'
#!/bin/sh
export SDL_VIDEODRIVER=${SDL_VIDEODRIVER:-KMSDRM}
cd /usr/share/APPLaunch/apps/game-asteroids
exec /usr/share/APPLaunch/apps/game-asteroids/asteroids "$@"
EOF
chmod 0755 "$STAGE$INSTALL_PREFIX/bin/$PKG_NAME"
