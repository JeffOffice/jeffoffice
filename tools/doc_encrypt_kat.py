#!/usr/bin/env python3
"""Makes the known-answer streams of the encrypted .doc tests, from the published rules alone (MS-OFFCRYPTO 2.3.6 and MS-DOC 2.2.6).

Three forms are made: RC4 with a 40-bit key (version 1.1), RC4 with the cryptographic provider's key (CryptoAPI, version 4.2, 128 bits, SHA-1, header flags 0x04: no AES), and the XOR obfuscation (with a password of the Western Windows coding, euro sign included; the layout, offset 68 for the document stream and 0 for the others, is the one the published rules give for the bytes counted from the start of each stream).
The three streams of a document are made from patterned plain bytes (byte i is (i * 7 + 3) & 255, the first bytes of the document stream being
the signature and flags a reader needs): the document stream is enciphered from its byte 68 on, the table stream after its description of the
protection, and the data stream whole, in blocks of 512 bytes, the key of each made from the block's number. Nothing here shares code with the
program. Run it and paste what it prints into tests/office/tst_doc.cpp (the kDocKnownAnswer strings)."""
import base64, hashlib, struct

PASSWORD = "Paßwörd1"
XOR_PASSWORD = "\u20acuro \u2019s"   # (the euro sign and a curly quote have places in the Western Windows coding)
SALT = bytes((i * 13 + 5) & 255 for i in range(16))
VERIFIER = bytes((i * 29 + 11) & 255 for i in range(16))

def rc4(key, data):
    s = list(range(256)); j = 0
    for i in range(256):
        j = (j + s[i] + key[i % len(key)]) & 255
        s[i], s[j] = s[j], s[i]
    i = j = 0; out = bytearray()
    for b in data:
        i = (i + 1) & 255; j = (j + s[i]) & 255
        s[i], s[j] = s[j], s[i]
        out.append(b ^ s[(s[i] + s[j]) & 255])
    return bytes(out)

def pattern(n): return bytes((i * 7 + 3) & 255 for i in range(n))

def encipher(stream, start, keyfor):
    out = bytearray(stream)
    for block in range(start // 512, (len(stream) + 511) // 512):
        lo, hi = block * 512, min(len(stream), block * 512 + 512)
        ks = rc4(keyfor(block), bytes(512))        # the keystream of the block, from its first byte
        for p in range(max(lo, start), hi): out[p] ^= ks[p - lo]
    return bytes(out)

pw = PASSWORD.encode("utf-16-le")
# ---- RC4, 40 bits (1.1)
h0 = hashlib.md5(pw).digest()
base40 = hashlib.md5((h0[:5] + SALT) * 16).digest()[:5]
key40 = lambda block: hashlib.md5(base40 + struct.pack("<I", block)).digest()
enc_ver = rc4(key40(0), VERIFIER + hashlib.md5(VERIFIER).digest())
desc11 = struct.pack("<HH", 1, 1) + SALT + enc_ver
# ---- CryptoAPI RC4, 128 bits
baseCA = hashlib.sha1(SALT + pw).digest()
keyCA = lambda block: hashlib.sha1(baseCA + struct.pack("<I", block)).digest()[:16]
enc = rc4(keyCA(0), VERIFIER + hashlib.sha1(VERIFIER).digest())
csp = "Enhanced Cryptographic Provider, version 1.0.0".encode("utf-16-le") + b"\0\0"
header = struct.pack("<IIIIIIII", 0x04, 0, 0x6801, 0x8004, 128, 1, 0, 0) + csp
descCA = struct.pack("<HHII", 4, 2, 0x04, len(header)) + header + struct.pack("<I", 16) + SALT + enc[:16] + struct.pack("<I", 20) + enc[16:36]

def make(desc, keyfor):
    wd = bytearray(pattern(1100)); wd[0:2] = b"\xec\xa5"; flags = 0x0100 | 0x0200
    wd[10:12] = struct.pack("<H", flags); wd[14:18] = struct.pack("<I", len(desc))
    table = desc + pattern(900)
    data = pattern(700)
    return (encipher(bytes(wd), 68, keyfor), encipher(table, len(desc), keyfor), encipher(data, 0, keyfor))

for name, desc, keyfor in (("RC4_40", desc11, key40), ("CRYPTOAPI", descCA, keyCA)):
    w, t, d = make(desc, keyfor)
    print(name, "desc", len(desc))
    for label, s in (("wd", w), ("table", t), ("data", d)):
        print(name, label, base64.b64encode(s).decode())

# ---- XOR obfuscation (MS-OFFCRYPTO 2.3.7): the password as bytes of the Western Windows coding (15 at most), a 16-bit key and verifier made from it, and a 16-byte table.
def xor_forms(password):
    pw = password.encode("cp1252")[:15]
    initial = [0xE1F0, 0x1D0F, 0xCC9C, 0x84C0, 0x110C, 0x0E10, 0xF1CE, 0x313E, 0x1872, 0xE139, 0xD40F, 0x84F9, 0x280C, 0xA96A, 0x4EC3]
    pad = [0xBB, 0xFF, 0xFF, 0xBA, 0xFF, 0xFF, 0xB9, 0x80, 0x00, 0xBE, 0x0F, 0x00, 0xBF, 0x0F, 0x00]
    def step(x): return ((x << 1) & 0xFFFF) ^ (0x1021 if x & 0x8000 else 0)
    # the matrix of 105 numbers: row r (the r-th place of a 15-letter password) holds seven, each a step of the check value from the one before
    matrix = []
    for row in range(15):
        x = 1
        for _ in range(16 + 8 * (14 - row)): x = step(x)
        for _ in range(7): matrix.append(x); x = step(x)
    key = initial[len(pw) - 1]; element = 104
    for ch in reversed(pw):
        for _ in range(7):
            if ch & 0x40: key ^= matrix[element]
            ch = (ch << 1) & 0xFF; element -= 1
    ver = 0
    for b in reversed(bytes([len(pw)]) + pw):
        ver = ((((ver & 0x4000) != 0) | ((ver * 2) & 0x7FFF)) ^ b)
    ver ^= 0xCE4B
    table = []
    for i in range(16):
        k = (key >> 8) if i & 1 else (key & 0xFF)
        src = pw[i] if i < len(pw) else pad[i - len(pw)]
        v = src ^ k
        table.append(((v >> 1) | (v << 7)) & 0xFF)
    return key, ver, table

def xor_stream(stream, start, table):
    out = bytearray(stream)
    for q in range(start, len(out)):
        r = out[q]
        out[q] = (((r >> 3) | (r << 5)) & 0xFF) ^ table[q % 16]    # (the inverse of: XOR with the table, then rotate left by 3)
    return bytes(out)

key, ver, table = xor_forms(XOR_PASSWORD)
wd = bytearray(pattern(1100)); wd[0:2] = b"\xec\xa5"; wd[10:12] = struct.pack("<H", 0x8000 | 0x0100 | 0x0200); wd[14:18] = struct.pack("<HH", key, ver)
print("XOR key", key, "verifier", ver)
print("XOR wd", base64.b64encode(xor_stream(bytes(wd), 68, table)).decode())
print("XOR table", base64.b64encode(xor_stream(pattern(900), 0, table)).decode())
print("XOR data", base64.b64encode(xor_stream(pattern(700), 0, table)).decode())
