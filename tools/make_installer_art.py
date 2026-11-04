#!/usr/bin/env python3
"""Draws the Windows installer artwork in the JeffOffice brand.

    tools/make_installer_art.py

Writes packaging/windows/art/welcome-WxH.png (the side panel of the Welcome and
Finish pages) and art/small-N.png (the mark in the corner of the other pages),
at exactly the pixel sizes Inno Setup 6.6 and later asks for at each display
scale from 100% to 250%, so Windows never resizes them (the sizes are in the
Inno Setup help, under WizardImageFile and WizardSmallImageFile; they assume
the default wizard size). Also writes resources/brand/jeffoffice-icon.png
(512 px, the suite's own icon: the snap's in the Snap Store).
The mark is the one in resources/brand: a pinwheel of the six apps' colors on
a white tile. Needs Python 3 with Pillow.
"""
import math
import os
from PIL import Image, ImageDraw, ImageFilter, ImageFont

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
FONTS = os.path.join(ROOT, "resources", "fonts")
OUT = os.path.join(ROOT, "packaging", "windows", "art")
# Display scale -> side panel (w, h) and corner image size, from the Inno Setup help.
WELCOME = {100: (202, 386), 125: (269, 515), 150: (336, 643), 175: (403, 772), 200: (430, 824), 225: (498, 953), 250: (534, 1022)}
SMALL = {100: 58, 125: 77, 150: 97, 175: 116, 200: 124, 225: 143, 250: 159}
# The apps in suite order, with their colors and letters.
APPS = [("JP", (0x9E, 0x1F, 0x63)), ("JD", (0xB6, 0x42, 0x0F)), ("JW", (0x1D, 0x55, 0xC4)),
        ("JC", (0x13, 0x77, 0x3A)), ("JM", (0x0A, 0x6C, 0x8C)), ("JB", (0x5E, 0x2C, 0xA5))]
INK = (0x1F, 0x29, 0x37)
DEEP = (0x11, 0x18, 0x27)
EDGE = (0xD1, 0xD5, 0xDB)
SS = 4  # draw this many times larger, then shrink once for smooth edges


def font(name, size):
    return ImageFont.truetype(os.path.join(FONTS, name), max(1, round(size * SS)))


def mark(size, tiled=True):
    """The JeffOffice mark: six blades on a white rounded tile, or, untiled, the blades alone with a clear center."""
    s = round(size * SS)
    im = Image.new("RGBA", (s, s), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    if tiled:
        d.rounded_rectangle((s * 0.01, s * 0.01, s * 0.99, s * 0.99), radius=s * 0.235, fill=(255, 255, 255),
                            outline=EDGE, width=max(1, round(s * 0.012)))
    cx = cy = s / 2
    r = s * (0.36 if tiled else 0.34)
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
    d.ellipse((cx - hole, cy - hole, cx + hole, cy + hole), fill=(255, 255, 255) if tiled else (0, 0, 0, 0))
    return im


def tile(size, letters, color, alpha=255):
    """An app's tile, as on File > About."""
    s = round(size * SS)
    im = Image.new("RGBA", (s, s), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.rounded_rectangle((0, 0, s - 1, s - 1), radius=int(s * 0.24), fill=color + (alpha,))
    f = font("Jost-Medium.ttf", size * 0.42)
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


def welcome(w, h, name):
    """The side panel at w x h pixels. The layout is the original 328 x 628 one, scaled by k."""
    k = min(w / 328, h / 628)
    top = (h - 628 * k) / 2
    W, H = w * SS, h * SS
    im = Image.new("RGB", (W, H))
    px = im.load()
    for y in range(H):
        t = y / (H - 1)
        c = tuple(int(INK[i] + (DEEP[i] - INK[i]) * t) for i in range(3))
        for x in range(W):
            px[x, y] = c
    im = im.convert("RGBA")

    def Y(v):
        return round((top + v * k) * SS)

    m = mark(112 * k)
    im = shadowed(im, m, (W - m.width) // 2, Y(92), 12 * k * SS, 8 * k * SS, 90)
    d = ImageDraw.Draw(im)

    def centered(text, f, y, fill):
        box = d.textbbox((0, 0), text, font=f)
        d.text(((W - (box[2] - box[0])) / 2 - box[0], Y(y)), text, font=f, fill=fill)

    centered("JeffOffice", font("Jost-Medium.ttf", 44 * k), 226, (255, 255, 255))
    centered("An office suite,", font("Lato-Bold.ttf", 25 * k), 292, (255, 255, 255))
    centered("free and open.", font("Lato-Bold.ttf", 25 * k), 326, (255, 255, 255))
    # The apps: JeffPub and JeffDeck are ready, JeffWrite and JeffCrunch are
    # betas (a little dimmer), JeffMail and JeffBase are coming.
    size, gap = 34 * k, 9 * k
    x0 = (w - (len(APPS) * size + (len(APPS) - 1) * gap)) / 2
    for i, (letters, color) in enumerate(APPS):
        alpha = 255 if i < 2 else 190 if i < 4 else 110
        im.alpha_composite(tile(size, letters, color, alpha), (round((x0 + i * (size + gap)) * SS), Y(462)))
    d = ImageDraw.Draw(im)
    centered("2 ready, 2 betas,", font("Lato-Bold.ttf", 18 * k), 515, (255, 255, 255))
    centered("2 coming soon.", font("Lato-Bold.ttf", 18 * k), 541, (255, 255, 255))
    im.convert("RGB").resize((w, h), Image.LANCZOS).save(os.path.join(OUT, name), optimize=True)


def small(n, name):
    """The blades alone on a transparent square, with a margin, so they sit on a light or a dark page: Setup puts the
    corner image flush against the window's right edge."""
    mark(n, tiled=False).resize((n, n), Image.LANCZOS).save(os.path.join(OUT, name), optimize=True)


def suite_icon():
    mark(512).resize((512, 512), Image.LANCZOS).save(os.path.join(ROOT, "resources", "brand", "jeffoffice-icon.png"))


if __name__ == "__main__":
    os.makedirs(OUT, exist_ok=True)
    for pct, (w, h) in WELCOME.items():
        welcome(w, h, "welcome-%dx%d.png" % (w, h))
        small(SMALL[pct], "small-%d.png" % SMALL[pct])
    suite_icon()
