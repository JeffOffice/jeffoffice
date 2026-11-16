#!/usr/bin/env python3
"""Makes the known-answer file of the OpenDocument password tests, from the standard alone.

The package is built here, with Python's own zlib, hashlib, and zipfile and the `cryptography` package's AES, following the ODF 1.2 / 1.3
encryption rules (part 3, "Encryption"): the part is deflated, then enciphered with AES-256 in CBC mode (PKCS#7 padding); the key is
PBKDF2-HMAC-SHA1 of the SHA-256 of the UTF-8 password, 32 bytes, with a salt; the manifest keeps the size before deflating and the SHA-256
of the first 1,024 bytes of the deflated part. Nothing here shares code with the program, so a mistake that the program's encoder and
decoder make the same way cannot pass the test that reads this file. The salts and ivs are fixed so that the output is always the same.
Run it, and paste the printed base64 (first the AES form, then the older Blowfish form) into tests/sheet/tst_sheet_crypt.cpp (kOdfKnownAnswer, kOdfKnownBlowfish)."""
import base64, hashlib, io, zlib, zipfile
import warnings
warnings.simplefilter("ignore")
from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes
try:
    from cryptography.hazmat.decrepit.ciphers.algorithms import Blowfish
except ImportError:
    from cryptography.hazmat.primitives.ciphers.algorithms import Blowfish

PASSWORD = "Pässword ☃"
ROUNDS = 1000
# (text that does not deflate to less than 1,024 bytes, so that the checksum of the first 1,024 bytes of the deflated part is of real data)
NOISE = ''.join(base64.b64encode(hashlib.sha256(bytes([i % 256, i // 256])).digest()).decode() for i in range(120))
CONTENT = ('<?xml version="1.0" encoding="UTF-8"?><office:document-content xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0">'
           '<office:body><office:text>Known answer text. ' + NOISE + '</office:text></office:body></office:document-content>').encode()
STYLES = b'<?xml version="1.0" encoding="UTF-8"?><office:document-styles xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0"/>'
MIME = b"application/vnd.oasis.opendocument.text"

def seal(name, data, n, blowfish=False):
    salt = bytes((n * 16 + i) & 255 for i in range(16))
    iv = bytes((200 - n * 16 - i) & 255 for i in range(8 if blowfish else 16))
    start = hashlib.sha1(PASSWORD.encode("utf-8")).digest() if blowfish else hashlib.sha256(PASSWORD.encode("utf-8")).digest()
    key = hashlib.pbkdf2_hmac("sha1", start, salt, ROUNDS, 16 if blowfish else 32)
    comp = zlib.compressobj(9, zlib.DEFLATED, -15)
    packed = comp.compress(data) + comp.flush()
    if blowfish:
        # the older form: Blowfish in cipher feedback mode (whole 8-byte blocks fed back), no padding, a SHA-1 of the first 1,024 bytes
        enc = Cipher(Blowfish(key), modes.CFB(iv)).encryptor()
        body = enc.update(packed) + enc.finalize()
        check = base64.b64encode(hashlib.sha1(packed[:1024]).digest()).decode()
        algo, ctype, skey, sname = "Blowfish CFB", "SHA1/1K", 20, "http://www.w3.org/2000/09/xmldsig#sha1"
        ksize = 16
    else:
        pad = 16 - len(packed) % 16
        enc = Cipher(algorithms.AES(key), modes.CBC(iv)).encryptor()
        body = enc.update(packed + bytes([pad]) * pad) + enc.finalize()
        check = base64.b64encode(hashlib.sha256(packed[:1024]).digest()).decode()
        algo, ctype, skey, sname = "http://www.w3.org/2001/04/xmlenc#aes256-cbc", "urn:oasis:names:tc:opendocument:xmlns:manifest:1.0#sha256-1k", 32, "http://www.w3.org/2000/09/xmldsig#sha256"
        ksize = 32
    entry = ('<manifest:file-entry manifest:full-path="%s" manifest:media-type="text/xml" manifest:size="%d">'
             '<manifest:encryption-data manifest:checksum-type="%s" manifest:checksum="%s">'
             '<manifest:algorithm manifest:algorithm-name="%s" manifest:initialisation-vector="%s"/>'
             '<manifest:key-derivation manifest:key-derivation-name="PBKDF2" manifest:key-size="%d" manifest:iteration-count="%d" manifest:salt="%s"/>'
             '<manifest:start-key-generation manifest:start-key-generation-name="%s" manifest:key-size="%d"/>'
             '</manifest:encryption-data></manifest:file-entry>') % (name, len(data), ctype, check, algo, base64.b64encode(iv).decode(), ksize, ROUNDS, base64.b64encode(salt).decode(), sname, skey)
    return body, entry

def package(blowfish):
    c_body, c_entry = seal("content.xml", CONTENT, 1, blowfish)
    s_body, s_entry = seal("styles.xml", STYLES, 2, blowfish)
    manifest = ('<?xml version="1.0" encoding="UTF-8"?><manifest:manifest xmlns:manifest="urn:oasis:names:tc:opendocument:xmlns:manifest:1.0" manifest:version="1.2">'
                '<manifest:file-entry manifest:full-path="/" manifest:media-type="%s"/>%s%s</manifest:manifest>') % (MIME.decode(), c_entry, s_entry)
    buf = io.BytesIO()
    with zipfile.ZipFile(buf, "w") as z:
        z.writestr(zipfile.ZipInfo("mimetype", (2026, 11, 16, 0, 0, 0)), MIME, zipfile.ZIP_STORED)
        z.writestr(zipfile.ZipInfo("content.xml", (2026, 11, 16, 0, 0, 0)), c_body, zipfile.ZIP_STORED)
        z.writestr(zipfile.ZipInfo("styles.xml", (2026, 11, 16, 0, 0, 0)), s_body, zipfile.ZIP_STORED)
        z.writestr(zipfile.ZipInfo("META-INF/manifest.xml", (2026, 11, 16, 0, 0, 0)), manifest.encode(), zipfile.ZIP_DEFLATED)
    return base64.b64encode(buf.getvalue()).decode()

print(package(False))
print(package(True))
print("content.xml sha256:", hashlib.sha256(CONTENT).hexdigest())
