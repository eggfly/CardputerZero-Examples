#!/usr/bin/env bash
set -euo pipefail

install -D -m 0644 hello_tk.py "$STAGE$APP_INSTALL_DIR/hello_tk.py"
install -D -m 0644 fb_blit.py  "$STAGE$APP_INSTALL_DIR/fb_blit.py"
install -D -m 0755 run_xvfb.sh "$STAGE$APP_INSTALL_DIR/run_xvfb.sh"

tmp=$(mktemp)
cat >"$tmp" <<'EOF'
#!/bin/sh
set -e
cd /usr/share/APPLaunch/apps/py-tk-hello
exec sh ./run_xvfb.sh "$@"
EOF
install -D -m 0755 "$tmp" "$STAGE$INSTALL_PREFIX/bin/py-tk-hello"
rm -f "$tmp"
chmod 0755 "$STAGE$INSTALL_PREFIX/bin/py-tk-hello"
