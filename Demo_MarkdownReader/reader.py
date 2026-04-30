#!/usr/bin/env python3
"""Demo_MarkdownReader: paged text reader for .md files on /dev/fb0.

Usage: reader.py <file.md>
Keys: Space next page, b previous, Esc quit.
Rendering is monospaced/mono-color; markdown formatting is not parsed.
"""

import mmap
import os
import select
import sys

from PIL import Image, ImageDraw, ImageFont

FB_DEV = "/dev/fb0"
W, H = 320, 170
BPP = 2
FONT_PATH = "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf"
FONT_SIZE = 11
LINE_H = 13
MARGIN = 4
LINES_PER_PAGE = (H - MARGIN * 2 - 14) // LINE_H  # reserve footer
MAX_COLS = 48


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


def wrap(text):
    out = []
    for raw in text.splitlines():
        if not raw:
            out.append("")
            continue
        while len(raw) > MAX_COLS:
            cut = raw.rfind(" ", 0, MAX_COLS)
            if cut <= 0:
                cut = MAX_COLS
            out.append(raw[:cut])
            raw = raw[cut:].lstrip()
        out.append(raw)
    return out


def paginate(lines):
    pages = []
    for i in range(0, len(lines), LINES_PER_PAGE):
        pages.append(lines[i: i + LINES_PER_PAGE])
    return pages or [[""]]


def render(page, idx, total, font, font_sm):
    img = Image.new("RGB", (W, H), (0, 0, 0))
    d = ImageDraw.Draw(img)
    y = MARGIN
    for line in page:
        d.text((MARGIN, y), line, font=font, fill=(230, 230, 230))
        y += LINE_H
    d.text((MARGIN, H - 14),
           f"Page {idx + 1}/{total}   Space=next  b=prev  Esc=quit",
           font=font_sm, fill=(120, 120, 120))
    return img


def read_key():
    dr, _, _ = select.select([sys.stdin], [], [], 0.2)
    if not dr:
        return ""
    return sys.stdin.read(1)


def main():
    if len(sys.argv) < 2:
        print("usage: reader.py <file.md>", file=sys.stderr)
        return 2
    with open(sys.argv[1], encoding="utf-8", errors="replace") as f:
        text = f.read()

    font = ImageFont.truetype(FONT_PATH, FONT_SIZE)
    font_sm = ImageFont.truetype(FONT_PATH, 9)

    pages = paginate(wrap(text))
    idx = 0

    fd = os.open(FB_DEV, os.O_RDWR)
    mm = mmap.mmap(fd, W * H * BPP, mmap.MAP_SHARED, mmap.PROT_WRITE | mmap.PROT_READ)

    try:
        while True:
            img = render(pages[idx], idx, len(pages), font, font_sm)
            mm.seek(0)
            mm.write(image_to_rgb565(img))
            k = ""
            while not k:
                k = read_key()
            if k == "\x1b":
                break
            elif k == " ":
                if idx + 1 < len(pages):
                    idx += 1
            elif k in ("b", "B"):
                if idx > 0:
                    idx -= 1
    finally:
        mm.close()
        os.close(fd)
    return 0


if __name__ == "__main__":
    sys.exit(main())
