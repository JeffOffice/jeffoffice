"""Screenshots of the apps by driving them on a private Xephyr display (no window manager).

Start the display first (parent display needed), for example:
  Xephyr :98 -ac -noreset -nolisten tcp -screen 3200x2000
Then import this module: launch() an app, place() its window, key()/typ()/click() through XTest,
and grab() a downscaled crisp PNG (the apps run at QT_SCALE_FACTOR=2). Local use only.
"""
import ctypes, os, subprocess, sys, time
from ctypes import c_void_p, c_int, c_uint, c_ulong, c_char_p, POINTER, byref, Structure
from PIL import Image
X = ctypes.CDLL('libX11.so.6'); T = ctypes.CDLL('libXtst.so.6')
X.XOpenDisplay.restype = c_void_p; X.XOpenDisplay.argtypes = [c_char_p]
X.XDefaultRootWindow.restype = c_ulong; X.XDefaultRootWindow.argtypes = [c_void_p]
X.XKeysymToKeycode.restype = c_uint; X.XKeysymToKeycode.argtypes = [c_void_p, c_ulong]
X.XStringToKeysym.restype = c_ulong; X.XStringToKeysym.argtypes = [c_char_p]
X.XFlush.argtypes = [c_void_p]; X.XSync.argtypes = [c_void_p, c_int]
T.XTestFakeKeyEvent.argtypes = [c_void_p, c_uint, c_int, c_ulong]
T.XTestFakeButtonEvent.argtypes = [c_void_p, c_uint, c_int, c_ulong]
T.XTestFakeMotionEvent.argtypes = [c_void_p, c_int, c_int, c_int, c_ulong]
class XImage(Structure):
    _fields_ = [('width', c_int), ('height', c_int), ('xoffset', c_int), ('format', c_int), ('data', c_void_p),
                ('byte_order', c_int), ('bitmap_unit', c_int), ('bitmap_bit_order', c_int), ('bitmap_pad', c_int),
                ('depth', c_int), ('bytes_per_line', c_int), ('bits_per_pixel', c_int)]
X.XGetImage.restype = POINTER(XImage)
X.XGetImage.argtypes = [c_void_p, c_ulong, c_int, c_int, c_uint, c_uint, c_ulong, c_int]
X.XMoveResizeWindow.argtypes = [c_void_p, c_ulong, c_int, c_int, c_uint, c_uint]
X.XSetInputFocus.argtypes = [c_void_p, c_ulong, c_int, c_ulong]
X.XQueryTree.argtypes = [c_void_p, c_ulong, POINTER(c_ulong), POINTER(c_ulong), POINTER(POINTER(c_ulong)), POINTER(c_uint)]
D = X.XOpenDisplay(os.environ.get('SHOT_DISPLAY', ':98').encode())
ROOT = X.XDefaultRootWindow(D)
SHIFT, CTRL, ALT = 'Shift_L', 'Control_L', 'Alt_L'
def _kc(name): return X.XKeysymToKeycode(D, X.XStringToKeysym(name.encode()))
def key(combo, delay=0.08):
    """key('ctrl+shift+f'), key('Return'), key('Down')"""
    parts = combo.split('+'); mods = [{'ctrl': CTRL, 'shift': SHIFT, 'alt': ALT}[p.lower()] for p in parts[:-1]]
    k = parts[-1]
    if len(k) == 1 and k.isupper(): mods.append(SHIFT); k = k.lower()
    names = {' ': 'space', '.': 'period', ',': 'comma', '-': 'minus', '\n': 'Return', '\t': 'Tab', '/': 'slash', ':': 'colon', '=': 'equal', '\'': 'apostrophe', '(': 'parenleft', ')': 'parenright', '$': 'dollar', '%': 'percent', '!': 'exclam', '?': 'question', '&': 'ampersand', '#': 'numbersign', '"': 'quotedbl', ';': 'semicolon', '+': 'plus', '*': 'asterisk', '@': 'at', '_': 'underscore'}
    k = names.get(k, k)
    for m in mods: T.XTestFakeKeyEvent(D, _kc(m), 1, 0)
    T.XTestFakeKeyEvent(D, _kc(k), 1, 0); T.XTestFakeKeyEvent(D, _kc(k), 0, 0)
    for m in reversed(mods): T.XTestFakeKeyEvent(D, _kc(m), 0, 0)
    X.XFlush(D); time.sleep(delay)
def typ(text, delay=0.03):
    shifted = '~!@#$%^&*()_+{}|:"<>?'
    for ch in text:
        if ch.isupper() or ch in shifted:
            base = {'!': '1', '@': '2', '#': '3', '$': '4', '%': '5', '^': '6', '&': '7', '*': '8', '(': '9', ')': '0', '_': 'minus', '+': 'equal', ':': 'semicolon', '"': 'apostrophe', '?': 'slash', '<': 'comma', '>': 'period'}.get(ch, ch.lower())
            for m in [SHIFT]: T.XTestFakeKeyEvent(D, _kc(m), 1, 0)
            names = {'1': '1'}
            nm = base
            nm = {' ': 'space'}.get(nm, nm)
            T.XTestFakeKeyEvent(D, _kc(nm), 1, 0); T.XTestFakeKeyEvent(D, _kc(nm), 0, 0)
            T.XTestFakeKeyEvent(D, _kc(SHIFT), 0, 0); X.XFlush(D); time.sleep(delay)
        else: key(ch, delay)
def move(x, y):
    T.XTestFakeMotionEvent(D, 0, x * SCALE, y * SCALE, 0); X.XFlush(D); time.sleep(0.05)
def click(x, y, button=1, double=False):
    move(x, y)
    for _ in range(2 if double else 1):
        T.XTestFakeButtonEvent(D, button, 1, 0); T.XTestFakeButtonEvent(D, button, 0, 0); X.XFlush(D); time.sleep(0.06)
    time.sleep(0.25)
def toplevels():
    r, p, ch, n = c_ulong(), c_ulong(), POINTER(c_ulong)(), c_uint()
    X.XQueryTree(D, ROOT, byref(r), byref(p), byref(ch), byref(n))
    return [ch[i] for i in range(n.value)]
SCALE = 2
X.XGetWindowAttributes.argtypes = [c_void_p, c_ulong, c_void_p]
class Attr(Structure):
    _fields_ = [('x', c_int), ('y', c_int), ('w', c_int), ('h', c_int), ('bw', c_int), ('depth', c_int), ('visual', c_void_p), ('root', c_ulong),
                ('cls', c_int), ('bg', c_int), ('wg', c_int), ('bs', c_int), ('bp', c_ulong), ('bpx', c_ulong), ('su', c_int), ('cm', c_ulong),
                ('mi', c_int), ('map_state', c_int), ('aem', ctypes.c_long), ('yem', ctypes.c_long), ('dnp', ctypes.c_long), ('ro', c_int), ('scr', c_void_p)]
def geom(win):
    a = Attr(); X.XGetWindowAttributes(D, win, byref(a)); return a.x, a.y, a.w, a.h, a.map_state == 2
def place(w=1600, h=1000):
    """Put the app's largest mapped top-level window at 0,0, sized w x h logical pixels."""
    wins = [(geom(t)[2] * geom(t)[3], t) for t in toplevels() if geom(t)[4]]
    if wins: X.XMoveResizeWindow(D, max(wins)[1], 0, 0, w * SCALE, h * SCALE)
    X.XFlush(D)
def focus_top():
    for win in toplevels()[-1:]: X.XSetInputFocus(D, win, 2, 0)
    X.XFlush(D)
def grab(path, w=1600, h=1000):
    X.XSync(D, 0)
    W, H = w * SCALE, h * SCALE
    im = X.XGetImage(D, ROOT, 0, 0, W, H, 0xFFFFFFFF, 2)[0]
    buf = ctypes.string_at(im.data, im.bytes_per_line * H)
    img = Image.frombuffer('RGBA', (W, H), buf, 'raw', 'BGRA', im.bytes_per_line, 1).convert('RGB')
    if SCALE != 1: img = img.resize((w, h), Image.LANCZOS)
    img.save(path)
def launch(argv, home, extra_env=None):
    env = dict(os.environ, DISPLAY=':98', QT_QPA_PLATFORM='xcb', JP_NO_TELEMETRY='1', HOME=home, XDG_CONFIG_HOME=home + '/.config',
               XDG_DATA_HOME=home + '/.local/share', XDG_CACHE_HOME=home + '/.cache', QT_SCALE_FACTOR=str(SCALE), CUPS_SERVER='/nonexistent', LANG='en_US.UTF-8', LC_ALL='en_US.UTF-8')
    env.update(extra_env or {})
    os.makedirs(home, exist_ok=True)
    return subprocess.Popen(argv, env=env, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
