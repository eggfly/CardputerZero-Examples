#!/usr/bin/env bash
# Path A: Xvfb + ffmpeg mirror to /dev/fb0
set -e

HERE="$(cd "$(dirname "$0")" && pwd)"
cd "$HERE"

Xvfb :1 -screen 0 320x170x16 -nolisten tcp &
XVFB_PID=$!
cleanup() {
  kill "$XVFB_PID" 2>/dev/null || true
  if [ -n "${FFMPEG_PID:-}" ]; then
    kill "$FFMPEG_PID" 2>/dev/null || true
  fi
}
trap cleanup EXIT INT TERM

export DISPLAY=:1
sleep 0.5

sudo ffmpeg -loglevel error -f x11grab -video_size 320x170 -framerate 15 -i :1 \
  -pix_fmt rgb565le -f fbdev /dev/fb0 &
FFMPEG_PID=$!

python3 hello_tk.py
