#!/usr/bin/env bash
set -euo pipefail
install -D -m 0755 target/release/slint_hello "$STAGE$INSTALL_PREFIX/bin/$PKG_NAME"
