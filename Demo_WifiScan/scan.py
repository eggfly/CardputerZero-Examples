#!/usr/bin/env python3
"""Demo_WifiScan: scan nearby WiFi networks and render top 10 on fb0.

Keys:
  Up/Down  - scroll
  R        - rescan
  Esc/Q    - quit
"""

import mmap
import os
import re
import select
import subprocess
import sys

from PIL import Image, ImageDraw, ImageFont

FB_DEV = "/dev/fb0"
W, H = 320, 170
BPP = 2
FONT_PATH = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"
VISIBLE_ROWS = 10
ROW_H = 14


def rgb565(r, g, b):
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)


def image_to_rgb565(img):
    px = img.load()
    buf = bytearray(W * H * 2)
    i = 0
    for y in range(H):
        for x in range(W):
            r, g, b = px[x, y][:3]
            v = rgb565(r, g, b)
            buf[i] = v & 0xFF
            buf[i + 1] = (v >> 8) & 0xFF
            i += 2
    return bytes(buf)


def scan():
    """Return list of (ssid, signal_dbm). Requires sudo for iw scan."""
    try:
        out = subprocess.check_output(
            ["sudo", "iw", "dev", "wlan0", "scan"],
            text=True, stderr=subprocess.DEVNULL, timeout=15,
        )
    except (OSError, subprocess.CalledProcessError, subprocess.TimeoutExpired):
        return []

    nets = []
    cur_ssid = None
    cur_sig = None
    for line in out.splitlines():
        if line.startswith("BSS "):
            if cur_ssid:
                nets.append((cur_ssid, cur_sig if cur_sig is not None else -100))
            cur_ssid, cur_sig = None, None
        m = re.match(r"\s*signal:\s*(-?\d+\.?\d*)", line)
        if m:
            cur_sig = float(m.group(1))
        m = re.match(r"\s*SSID:\s*(.*)", line)
        if m:
            cur_ssid = m.group(1).strip() or "<hidden>"
    if cur_ssid:
        nets.append((cur_ssid, cur_sig if cur_sig is not None else -100))

    nets.sort(key=lambda n: n[1], reverse=True)
    return nets


def render(nets, offset, font, font_sm):
    img = Image.new("RGB", (W, H), (0, 0, 0))
    d = ImageDraw.Draw(img)
    d.text((6, 2), f"WiFi Scan ({len(nets)})", font=font, fill=(255, 255, 0))
    visible = nets[offset: offset + VISIBLE_ROWS]
    y = 20
    for ssid, sig in visible:
        bar = max(0, min(100, int((sig + 90) * 100 / 60)))
        color = (0, 255, 100) if bar > 60 else (255, 200, 0) if bar > 30 else (255, 80, 80)
        d.text((6, y), ssid[:24], font=font_sm, fill=(220, 220, 220))
        d.text((220, y), f"{int(sig):>4} dBm", font=font_sm, fill=color)
        y += ROW_H
    d.text((6, H - 12), "R=rescan  Up/Down=scroll  Esc=quit",
           font=font_sm, fill=(100, 100, 100))
    return img


def write_fb(img, mm):
    mm.seek(0)
    mm.write(image_to_rgb565(img))


def read_key_nonblock():
    """Minimal stdin reader; for production use evdev."""
    dr, _, _ = select.select([sys.stdin], [], [], 0.1)
    if not dr:
        return ""
    return sys.stdin.read(1)


def main():
    font = ImageFont.truetype(FONT_PATH, 14)
    font_sm = ImageFont.truetype(FONT_PATH, 11)

    fd = os.open(FB_DEV, os.O_RDWR)
    mm = mmap.mmap(fd, W * H * BPP, mmap.MAP_SHARED, mmap.PROT_WRITE | mmap.PROT_READ)

    nets = scan()
    offset = 0
    write_fb(render(nets, offset, font, font_sm), mm)

    try:
        while True:
            k = read_key_nonblock()
            if k in ("q", "\x1b"):
                break
            if k in ("r", "R"):
                nets = scan()
                offset = 0
            elif k == "j":
                if offset + VISIBLE_ROWS < len(nets):
                    offset += 1
            elif k == "k":
                if offset > 0:
                    offset -= 1
            write_fb(render(nets, offset, font, font_sm), mm)
    finally:
        mm.close()
        os.close(fd)


if __name__ == "__main__":
    main()
