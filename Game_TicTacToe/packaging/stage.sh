#!/usr/bin/env bash
set -euo pipefail

install -D -m 0755 tictactoe "$STAGE$APP_INSTALL_DIR/tictactoe"

cat >"$STAGE$INSTALL_PREFIX/bin/$PKG_NAME" <<'EOF'
#!/bin/sh
export SDL_VIDEODRIVER=${SDL_VIDEODRIVER:-KMSDRM}
cd /usr/share/APPLaunch/apps/game-tictactoe
exec /usr/share/APPLaunch/apps/game-tictactoe/tictactoe "$@"
EOF
chmod 0755 "$STAGE$INSTALL_PREFIX/bin/$PKG_NAME"
