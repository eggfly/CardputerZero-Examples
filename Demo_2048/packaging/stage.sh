#!/usr/bin/env bash
set -euo pipefail
install -D -m 0755 demo2048 "$STAGE$INSTALL_PREFIX/bin/$PKG_NAME"
