#!/usr/bin/env bash
set -euo pipefail

install -D -m 0755 invaders "$STAGE$APP_INSTALL_DIR/invaders"

cat >"$STAGE$INSTALL_PREFIX/bin/$PKG_NAME" <<'EOF'
#!/bin/sh
export SDL_VIDEODRIVER=${SDL_VIDEODRIVER:-KMSDRM}
cd /usr/share/APPLaunch/apps/game-invaders
exec /usr/share/APPLaunch/apps/game-invaders/invaders "$@"
EOF
chmod 0755 "$STAGE$INSTALL_PREFIX/bin/$PKG_NAME"
