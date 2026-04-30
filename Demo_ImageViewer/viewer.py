#!/usr/bin/env python3
"""Demo_ImageViewer: browse JPG/PNG from a directory on /dev/fb0.

Usage: viewer.py <dir>
Keys: Left/Right prev/next, Esc quit.
"""

import mmap
import os
import select
import sys

from PIL import Image

FB_DEV = "/dev/fb0"
W, H = 320, 170
BPP = 2
EXTS = (".jpg", ".jpeg", ".png", ".bmp", ".gif")


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


def list_images(d):
    try:
        files = sorted(f for f in os.listdir(d) if f.lower().endswith(EXTS))
    except OSError:
        files = []
    return [os.path.join(d, f) for f in files]


def load_and_fit(path):
    canvas = Image.new("RGB", (W, H), (0, 0, 0))
    try:
        im = Image.open(path).convert("RGB")
    except OSError:
        return canvas
    im.thumbnail((W, H), Image.LANCZOS)
    x = (W - im.width) // 2
    y = (H - im.height) // 2
    canvas.paste(im, (x, y))
    return canvas


def read_key():
    dr, _, _ = select.select([sys.stdin], [], [], 0.2)
    if not dr:
        return ""
    return sys.stdin.read(1)


def main():
    if len(sys.argv) < 2:
        print("usage: viewer.py <dir>", file=sys.stderr)
        return 2
    files = list_images(sys.argv[1])
    if not files:
        print("no images found", file=sys.stderr)
        return 1

    fd = os.open(FB_DEV, os.O_RDWR)
    mm = mmap.mmap(fd, W * H * BPP, mmap.MAP_SHARED, mmap.PROT_WRITE | mmap.PROT_READ)

    idx = 0
    try:
        while True:
            img = load_and_fit(files[idx])
            mm.seek(0)
            mm.write(image_to_rgb565(img))
            k = ""
            while not k:
                k = read_key()
            if k == "\x1b":
                break
            if k in ("n", "l", "d"):
                idx = (idx + 1) % len(files)
            elif k in ("p", "h", "a"):
                idx = (idx - 1) % len(files)
    finally:
        mm.close()
        os.close(fd)
    return 0


if __name__ == "__main__":
    sys.exit(main())
