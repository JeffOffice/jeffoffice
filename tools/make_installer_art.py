#!/usr/bin/env python3
"""Draws the Windows installer artwork in the JeffOffice brand.

    tools/make_installer_art.py

Writes packaging/windows/welcome.bmp (the side panel of the Welcome and
Finish pages) and packaging/windows/header.bmp (the banner on the other
pages), both at twice NSIS's size so they stay sharp on high-DPI screens.
The mark is the one in resources/brand: a pinwheel of the six apps' colors on
a white tile. Needs Python 3 with Pillow.
"""
import math
import os
from PIL import Image, ImageDraw, ImageFilter, ImageFont

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
FONTS = os.path.join(ROOT, "resources", "fonts")
OUT = os.path.join(ROOT, "packaging", "windows")
# The apps in suite order, with their colors and letters.
APPS = [("JP", (0x9E, 0x1F, 0x63)), ("JD", (0xB6, 0x42, 0x0F)), ("JW", (0x1D, 0x55, 0xC4)),
        ("JC", (0x13, 0x77, 0x3A)), ("JM", (0x0A, 0x6C, 0x8C)), ("JB", (0x5E, 0x2C, 0xA5))]
INK = (0x1F, 0x29, 0x37)
DEEP = (0x11, 0x18, 0x27)
EDGE = (0xD1, 0xD5, 0xDB)
SCALE = 4  # draw large, then shrink for smooth edges


def font(name, size):
    return ImageFont.truetype(os.path.join(FONTS, name), size * SCALE)


def mark(size):
    """The JeffOffice mark: six blades on a white rounded tile."""
    s = size * SCALE
    im = Image.new("RGBA", (s, s), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.rounded_rectangle((s * 0.01, s * 0.01, s * 0.99, s * 0.99), radius=s * 0.235, fill=(255, 255, 255),
                        outline=EDGE, width=max(1, round(s * 0.012)))
    cx = cy = s / 2
    r = s * 0.36
    for i, (_, color) in enumerate(APPS):
        # A blade: from the center up, then a curve out to the right (the
        # SVG's quadratic Bezier, sampled), turned 60 degrees for each app.
        p0, c, p1 = (0, -r), (r * 0.62, -r * 0.86), (r * 0.62, -r * 0.36)
        pts = [(0, 0), p0]
        for k in range(1, 33):
            t = k / 32
            pts.append(((1 - t) ** 2 * p0[0] + 2 * (1 - t) * t * c[0] + t * t * p1[0],
                        (1 - t) ** 2 * p0[1] + 2 * (1 - t) * t * c[1] + t * t * p1[1]))
        a = math.radians(i * 60)
        d.polygon([(cx + x * math.cos(a) - y * math.sin(a), cy + x * math.sin(a) + y * math.cos(a)) for x, y in pts], fill=color)
    hole = s * 0.04
    d.ellipse((cx - hole, cy - hole, cx + hole, cy + hole), fill=(255, 255, 255))
    return im


def tile(size, letters, color, alpha=255):
    """An app's tile, as on File > About."""
    s = size * SCALE
    im = Image.new("RGBA", (s, s), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.rounded_rectangle((0, 0, s - 1, s - 1), radius=int(s * 0.24), fill=color + (alpha,))
    f = font("Jost-Medium.ttf", int(size * 0.42))
    box = d.textbbox((0, 0), letters, font=f)
    d.text(((s - (box[2] - box[0])) / 2 - box[0], (s - (box[3] - box[1])) / 2 - box[1]), letters, font=f,
           fill=(255, 255, 255, alpha))
    return im


def shadowed(im, layer, x, y, blur, offset, alpha):
    shadow = Image.new("RGBA", im.size, (0, 0, 0, 0))
    sd = ImageDraw.Draw(shadow)
    sd.rounded_rectangle((x, y + offset, x + layer.width, y + layer.height + offset), radius=int(layer.width * 0.235),
                         fill=(0, 0, 0, alpha))
    im = Image.alpha_composite(im, shadow.filter(ImageFilter.GaussianBlur(blur)))
    im.alpha_composite(layer, (x, y))
    return im


def welcome():
    w, h = 328, 628
    W, H = w * SCALE, h * SCALE
    im = Image.new("RGB", (W, H))
    px = im.load()
    for y in range(H):
        t = y / (H - 1)
        c = tuple(int(INK[i] + (DEEP[i] - INK[i]) * t) for i in range(3))
        for x in range(W):
            px[x, y] = c
    im = im.convert("RGBA")
    m = mark(112)
    im = shadowed(im, m, (W - m.width) // 2, 92 * SCALE, 12 * SCALE, 8 * SCALE, 90)
    d = ImageDraw.Draw(im)

    def centered(text, f, y, fill):
        box = d.textbbox((0, 0), text, font=f)
        d.text(((W - (box[2] - box[0])) / 2 - box[0], y * SCALE), text, font=f, fill=fill)

    centered("JeffOffice", font("Jost-Medium.ttf", 38), 232, (249, 250, 251))
    centered("An office suite,", font("Lato-Regular.ttf", 20), 294, (255, 255, 255, 215))
    centered("free and open.", font("Lato-Regular.ttf", 20), 320, (255, 255, 255, 215))
    # The apps: JeffPub is ready; the others, dimmed, are coming.
    size, gap = 34, 9
    x0 = (w - (len(APPS) * size + (len(APPS) - 1) * gap)) // 2
    for i, (letters, color) in enumerate(APPS):
        im.alpha_composite(tile(size, letters, color, 255 if i == 0 else 110), ((x0 + i * (size + gap)) * SCALE, 470 * SCALE))
    d = ImageDraw.Draw(im)
    centered("JeffPub is ready.", font("Lato-Regular.ttf", 15), 520, (255, 255, 255, 190))
    centered("More apps are coming.", font("Lato-Regular.ttf", 15), 541, (255, 255, 255, 190))
    im.convert("RGB").resize((w, h), Image.LANCZOS).save(os.path.join(OUT, "welcome.bmp"))


def header():
    w, h = 300, 114
    W, H = w * SCALE, h * SCALE
    im = Image.new("RGBA", (W, H), (255, 255, 255, 255))
    m = mark(66)
    im.alpha_composite(m, (W - m.width - 24 * SCALE, (H - m.height) // 2))
    im.convert("RGB").resize((w, h), Image.LANCZOS).save(os.path.join(OUT, "header.bmp"))


if __name__ == "__main__":
    welcome()
    header()
