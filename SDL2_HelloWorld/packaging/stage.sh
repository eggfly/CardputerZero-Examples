#!/usr/bin/env bash
set -euo pipefail

install -D -m 0755 hello_sdl2 "$APP_INSTALL_DIR/hello_sdl2"

cat >"$STAGE$INSTALL_PREFIX/bin/$PKG_NAME" <<'EOF'
#!/bin/sh
export SDL_VIDEODRIVER=${SDL_VIDEODRIVER:-KMSDRM}
exec /usr/share/APPLaunch/apps/sdl2-hello/hello_sdl2 "$@"
EOF
chmod 0755 "$STAGE$INSTALL_PREFIX/bin/$PKG_NAME"
