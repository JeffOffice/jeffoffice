#!/usr/bin/env python3
"""Draws the apps' icons: the app's two letters in white on its color, a
rounded square (JeffPub: "JP" on magenta; JeffDeck: "JD" on orange; JeffWrite: "JW" on blue).

    tools/make_app_icon.py

Writes each app's app.png (256 px; the Mac .icns and the Linux icon are made
from it) and app.ico (16 to 256 px, for Windows): resources/ for JeffPub,
apps/deck/resources/ for JeffDeck, apps/write/resources/ for JeffWrite. Needs Python 3 with Pillow.
"""
import os
from PIL import Image, ImageDraw, ImageFont

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
RES = os.path.join(ROOT, "resources")
SCALE = 4  # draw large, then shrink for smooth edges
# Each app: its letters, its color (AppInfo's accent), and where its icons go.
APPS = (
    ("JP", (158, 31, 99), RES),
    ("JD", (182, 66, 15), os.path.join(ROOT, "apps", "deck", "resources")),
    ("JW", (29, 85, 196), os.path.join(ROOT, "apps", "write", "resources")),
)


def icon(size, TEXT, ACCENT):
    s = size * SCALE
    im = Image.new("RGBA", (s, s), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.rounded_rectangle((0, 0, s - 1, s - 1), radius=int(s * 0.22), fill=ACCENT)
    f = ImageFont.truetype(os.path.join(RES, "fonts", "Montserrat-Bold.ttf"), int(s * 0.52))
    box = d.textbbox((0, 0), TEXT, font=f)
    w, h = box[2] - box[0], box[3] - box[1]
    d.text(((s - w) / 2 - box[0], (s - h) / 2 - box[1]), TEXT, font=f, fill=(255, 255, 255))
    return im.resize((size, size), Image.LANCZOS)


sizes = [16, 20, 24, 32, 40, 48, 64, 128, 256]
for text, accent, out in APPS:
    big = icon(256, text, accent)
    big.save(os.path.join(out, "app.png"))
    big.save(os.path.join(out, "app.ico"), sizes=[(n, n) for n in sizes], append_images=[icon(n, text, accent) for n in sizes[:-1]])
