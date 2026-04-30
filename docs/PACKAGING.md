# Packaging & CI — APPLaunch .deb for every example

This repo ships each example as an APPLaunch-compatible Debian package so it can
be installed on the M5 CardputerZero and shown as an entry in APPLaunch's main
list. The format and install paths follow the official spec at
`M5CardputerZero-UserDemo/doc/APPLaunch-App-打包指南.md`.

## Runtime layout on the device (APPLaunch convention)

```
/usr/share/APPLaunch/
├── applications/
│   └── <pkg>.desktop        # entry APPLaunch scans on startup
├── bin/                     # wrapper/launcher scripts that APPLaunch Exec=... points at
├── apps/<pkg>/              # private files (binary, assets, venv, etc.)
│   ├── <binary>             # actual executable (can also live directly in bin/)
│   └── assets/…
└── share/
    ├── images/<pkg>.png     # icon referenced by Icon=share/images/<pkg>.png
    └── font/*.ttf           # (shared, managed by APPLaunch itself)
```

- **`.desktop` Exec** should be an absolute path. For Python / multi-file apps
  we install a tiny wrapper at `/usr/share/APPLaunch/bin/<pkg>` that sets up env
  (DISPLAY for Tkinter/Qt, etc.) and execs the real program under
  `/usr/share/APPLaunch/apps/<pkg>/`.
- **Icon path** is relative to `/usr/share/APPLaunch/` (APPLaunch's working
  directory). Keep icons 48×48 PNG.
- APPLaunch restarts scan on `systemctl restart APPLaunch.service`.

## Per-example packaging contract

Each example that opts in creates `packaging/` with:

| File | Required | Purpose |
| --- | --- | --- |
| `meta.env` | ✅ | Shell-sourced. Declares `PKG_NAME`, `PKG_VERSION`, `PKG_REVISION`, `PKG_DESC`, `PKG_DEPENDS`, `APP_NAME`, `APP_EXEC`, `APP_TERMINAL`, `APP_ICON`, optional `APP_SYSPLAUSE`, `PKG_MAINTAINER`, `PKG_HOMEPAGE`. |
| `build.sh` | ✅ | Executable. Builds the example in-place (idempotent). Runs from the example dir. |
| `stage.sh` | ✅ | Executable. Copies artifacts into `$STAGE$INSTALL_PREFIX/bin/`, `$STAGE$APP_INSTALL_DIR/`, etc. Env provided: `STAGE`, `INSTALL_PREFIX=/usr/share/APPLaunch`, `APP_INSTALL_DIR=/usr/share/APPLaunch/apps/<pkg>`, `PKG_NAME`. |
| `app.desktop.in` | optional | If present, substituted with `{{APP_NAME}}`, `{{APP_EXEC}}`, `{{APP_TERMINAL}}`, `{{APP_SYSPLAUSE}}`, `{{APP_ICON}}`. Otherwise `pack-deb.sh` renders a minimal one from meta.env. |
| `icon.png` | optional | 48×48 PNG for the launcher; auto-renamed to `<pkg>.png`. |
| `postinst`, `prerm` | optional | Standard dpkg maintainer scripts. |
| `ci-deps.sh` | optional | Run with `sudo` in CI before build. Must be `apt-get install -y …`. |

### Naming

- `PKG_NAME` must be lowercase, no spaces, match `[a-z0-9][a-z0-9.+-]+`.
- Output: `dist/<PKG_NAME>_<PKG_VERSION>-<PKG_REVISION>_arm64.deb`.

### Declaring runtime dependencies

`PKG_DEPENDS="libsdl2-2.0-0, libsdl2-ttf-2.0-0"` — these are **runtime** deps
the device must have. Do **not** put `build-essential` etc here; that's
`ci-deps.sh`'s job.

## Building locally

```bash
# one example
./scripts/pack-deb.sh FrameBuffer_HelloWorld

# everything
./scripts/pack-all.sh

# with output elsewhere / skip rebuild
./scripts/pack-all.sh --output-dir /tmp/debs --skip-build
```

Install on device:

```bash
scp dist/framebuffer_hello_0.1-m5stack1_arm64.deb pi@192.168.50.150:/tmp/
ssh pi@192.168.50.150 'sudo dpkg -i /tmp/framebuffer_hello_0.1-m5stack1_arm64.deb \
    && sudo systemctl restart APPLaunch.service'
```

Uninstall: `sudo dpkg -r <pkg_name>`.

## CI (GitHub Actions)

`.github/workflows/build-debs.yml`:

1. **discover** job — walks the repo for `*/packaging/meta.env` and emits a
   JSON matrix.
2. **build** job — runs on `ubuntu-24.04-arm` (native aarch64 runner, so no
   cross-compile), installs per-example deps via `packaging/ci-deps.sh`, runs
   `scripts/pack-deb.sh`, uploads `dist/*.deb` as an artifact.
3. **release** job — on a published Release, downloads all artifacts and
   attaches the `.deb` files to the release.

Only examples that have a `packaging/meta.env` appear in the matrix. Stubs
(`Flutter_Embedder`, `GPUI_Stub`) intentionally have none and are skipped.

## Adding a new example to CI

1. Create `<example>/packaging/meta.env` + `build.sh` + `stage.sh`.
2. `chmod +x packaging/build.sh packaging/stage.sh`.
3. Test locally: `./scripts/pack-deb.sh <example>`.
4. Test on device via `dpkg -i`, confirm it shows up in APPLaunch after
   `systemctl restart APPLaunch.service`.
5. Commit. CI will pick it up automatically.

## Why apps-under-`apps/<pkg>/` and not loose files?

APPLaunch's official doc shows binaries directly in `bin/`. That works for a
single app but gets messy when 20 examples all drop files there. We preserve
that pattern for `.desktop` and `bin/` launchers, but put private payload under
`apps/<pkg>/` so uninstall is clean (`dpkg -r` removes exactly that dir).
