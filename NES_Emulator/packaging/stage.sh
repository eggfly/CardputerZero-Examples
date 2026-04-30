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
export SDL_VIDEODRIVER=${SDL_VIDEODRIVER:-KMSDRM}
APP_DIR=/usr/share/APPLaunch/apps/nes-emulator
ROM_DIR=${NES_ROM_DIR:-$APP_DIR/roms}
ROM=${NES_ROM:-$(ls -1 "$ROM_DIR"/*.nes 2>/dev/null | head -1)}
if [ -n "$ROM" ]; then
    exec "$APP_DIR/nes-emulator" "$ROM"
fi
exec "$APP_DIR/nes-emulator"
EOF
chmod 0755 "$STAGE$INSTALL_PREFIX/bin/$PKG_NAME"
