#!/usr/bin/env bash
set -euo pipefail

install -D -m 0644 viewer.py "$STAGE$APP_INSTALL_DIR/viewer.py"
install -d -m 0755 "$STAGE$APP_INSTALL_DIR/samples"

tmp=$(mktemp)
cat >"$tmp" <<'EOF'
#!/bin/sh
DIR="${1:-/usr/share/APPLaunch/apps/demo-imgview/samples}"
exec sudo -E python3 /usr/share/APPLaunch/apps/demo-imgview/viewer.py "$DIR"
EOF
install -D -m 0755 "$tmp" "$STAGE$INSTALL_PREFIX/bin/demo-imgview"
rm -f "$tmp"
chmod 0755 "$STAGE$INSTALL_PREFIX/bin/demo-imgview"
