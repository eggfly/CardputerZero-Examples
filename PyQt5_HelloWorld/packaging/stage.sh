#!/usr/bin/env bash
set -euo pipefail

install -D -m 0644 hello.py "$STAGE$APP_INSTALL_DIR/hello.py"

tmp=$(mktemp)
cat >"$tmp" <<'EOF'
#!/bin/sh
export QT_QPA_PLATFORM=${QT_QPA_PLATFORM:-linuxfb:fb=/dev/fb0,size=320x170}
export QT_QPA_EVDEV_KEYBOARD_PARAMETERS=${QT_QPA_EVDEV_KEYBOARD_PARAMETERS:-/dev/input/event0}
exec python3 /usr/share/APPLaunch/apps/py-qt-hello/hello.py "$@"
EOF
install -D -m 0755 "$tmp" "$STAGE$INSTALL_PREFIX/bin/py-qt-hello"
rm -f "$tmp"
chmod 0755 "$STAGE$INSTALL_PREFIX/bin/py-qt-hello"
