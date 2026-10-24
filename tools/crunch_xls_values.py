#!/usr/bin/env python3
"""Rewrites old binary workbooks so that every formula is the value the file stored for it, record by record, with no program calculating anything.

    python3 -I tools/crunch_xls_values.py list.txt out-dir [jobs]

For each workbook in the list (one path to a line) it writes out-dir/NNNNN.xls (the number is the place in the list): the workbook stream of the file with each FORMULA record
made a NUMBER, a BOOLERR, or a LABEL (the text of the STRING record that follows it) of the same cell, and the records that only formulas need (shared formulas, arrays, data
tables, the STRING records) taken out, in a compound file of its own (BIFF5 and BIFF8) or as the bare stream (BIFF3 and BIFF4). The places of the sheets are put right. A program
that opens such a file has nothing to calculate, so what it writes as the workbook's values is what the file stored: this is how the comparison with another program's .xlsx keeps
the stored results (LibreOffice calculates the formulas of an .xls when it opens it, whatever the result stored). out-dir/index.tsv has the number, the path, and what was done
(or why the file was not rewritten).
"""
import struct
import sys
import os
from concurrent.futures import ProcessPoolExecutor

SIG = bytes.fromhex("D0CF11E0A1B11AE1")
END, FREE, NOSTREAM = 0xFFFFFFFE, 0xFFFFFFFF, 0xFFFFFFFF


def cfb_stream(data, names):
    if data[:8] != SIG:
        return None
    shift = struct.unpack_from("<H", data, 30)[0]
    ss = 1 << shift
    mshift = struct.unpack_from("<H", data, 32)[0]
    mss = 1 << mshift
    nfat = struct.unpack_from("<I", data, 44)[0]
    firstdir = struct.unpack_from("<I", data, 48)[0]
    cutoff = struct.unpack_from("<I", data, 56)[0]
    firstminifat = struct.unpack_from("<I", data, 60)[0]
    firstdifat = struct.unpack_from("<I", data, 68)[0]
    ndifat = struct.unpack_from("<I", data, 72)[0]

    def sec(n):
        return data[(n + 1) * ss:(n + 2) * ss]

    difat = list(struct.unpack_from("<109I", data, 76))
    d, k = firstdifat, 0
    while d < 0xFFFFFFF0 and k < ndifat and k < 10000:
        s = sec(d)
        if len(s) < ss:
            break
        difat += list(struct.unpack_from("<%dI" % (ss // 4), s)[:-1])
        d = struct.unpack_from("<I", s, ss - 4)[0]
        k += 1
    fat = []
    for x in difat[:nfat]:
        s = sec(x)
        if len(s) == ss:
            fat += list(struct.unpack_from("<%dI" % (ss // 4), s))

    def chain(start, table, limit=1 << 24):
        out, seen, c = [], set(), start
        while c < len(table) and c not in seen and len(out) < limit:
            seen.add(c)
            out.append(c)
            c = table[c]
        return out

    dirdata = b"".join(sec(n) for n in chain(firstdir, fat))
    ents = []
    for o in range(0, len(dirdata) - 127, 128):
        nl = struct.unpack_from("<H", dirdata, o + 64)[0]
        name = dirdata[o:o + max(0, nl - 2)].decode("utf-16le", "replace")
        ents.append((name, dirdata[o + 66], struct.unpack_from("<I", dirdata, o + 116)[0], struct.unpack_from("<Q" if ss == 4096 else "<I", dirdata, o + 120)[0]))
    found = None
    for want in names:   # (a file saved for both the old and the new program holds two: the first name asked for is the one wanted)
        for e in ents:
            if e[1] == 2 and e[0] == want:
                found = e
                break
        if found:
            break
    if not found:
        return None
    _, _, start, size = found
    if size >= cutoff:
        return b"".join(sec(n) for n in chain(start, fat))[:size]
    mf = []
    for n in chain(firstminifat, fat):
        mf += list(struct.unpack_from("<%dI" % (ss // 4), sec(n)))
    root = b"".join(sec(n) for n in chain(ents[0][2], fat))
    return b"".join(root[m * mss:(m + 1) * mss] for m in chain(start, mf))[:size]


def cfb_write(stream, name):
    """A compound file of version 3 (sectors of 512 bytes) with the one stream, with the sectors of the allocation table listed in the header and, past 109 of them, in sectors of their own."""
    ss = 512
    if len(stream) < 4096:
        stream = stream + b"\0" * (4096 - len(stream))
    nstream = (len(stream) + ss - 1) // ss
    nfat, ndifat = 1, 0
    while True:
        ndifat = (nfat - 109 + 126) // 127 if nfat > 109 else 0
        if nfat * 128 >= nfat + ndifat + 1 + nstream:
            break
        nfat += 1
    # the sectors: the allocation tables, then the sectors that list them, the directory, the stream
    fat = [FREE] * (nfat * 128)
    for i in range(nfat):
        fat[i] = 0xFFFFFFFD
    for i in range(ndifat):
        fat[nfat + i] = 0xFFFFFFFC
    dirsec = nfat + ndifat
    fat[dirsec] = END
    first = dirsec + 1
    for i in range(nstream):
        fat[first + i] = first + i + 1 if i + 1 < nstream else END
    hdr = bytearray(ss)
    hdr[0:8] = SIG
    struct.pack_into("<HHHHH", hdr, 24, 0x3E, 3, 0xFFFE, 9, 6)
    struct.pack_into("<IIIIIIIII", hdr, 40, 0, nfat, dirsec, 0, 4096, END, 0, END if ndifat == 0 else nfat, ndifat)
    for i in range(109):
        struct.pack_into("<I", hdr, 76 + 4 * i, i if i < nfat else FREE)
    difat = bytearray()
    for k in range(ndifat):
        sec = [FREE] * 127
        for m in range(127):
            idx = 109 + k * 127 + m
            if idx < nfat:
                sec[m] = idx
        difat += struct.pack("<127I", *sec) + struct.pack("<I", (nfat + k + 1) if k + 1 < ndifat else END)
    d = bytearray(ss)
    for o in range(0, ss, 128):
        struct.pack_into("<III", d, o + 68, NOSTREAM, NOSTREAM, NOSTREAM)

    def ent(o, nm, typ, child, start, size):
        u = nm.encode("utf-16le")
        d[o:o + len(u)] = u
        struct.pack_into("<H", d, o + 64, len(u) + 2)
        d[o + 66] = typ
        d[o + 67] = 1
        struct.pack_into("<III", d, o + 68, NOSTREAM, NOSTREAM, child)
        struct.pack_into("<I", d, o + 116, start)
        struct.pack_into("<I", d, o + 120, size)

    ent(0, "Root Entry", 5, 1, END, 0)
    ent(128, name, 2, NOSTREAM, first, len(stream))
    padded = stream + b"\0" * (nstream * ss - len(stream))
    return bytes(hdr) + struct.pack("<%dI" % len(fat), *fat) + bytes(difat) + bytes(d) + padded


def records(st):
    o = 0
    while o + 4 <= len(st):
        i, l = struct.unpack_from("<HH", st, o)
        yield o, i, st[o + 4:o + 4 + l]
        o += 4 + l


def rec(i, p):
    return struct.pack("<HH", i, len(p)) + p


def rewrite(stream, biff):
    """The stream with every FORMULA the cell's stored value. biff: 8, 5, or 4 (BIFF3 and BIFF4, with the numbers of their records)."""
    formula_ids = (0x0206, 0x0406) if biff == 4 else (0x0006,)
    bof_ids = (0x0809, 0x0209, 0x0409)
    recs = list(records(stream))
    out = []            # (old offset, id, payload)
    i = 0
    n = len(recs)
    while i < n:
        o, rid, p = recs[i]
        if rid in formula_ids and len(p) >= 14:
            row, col, xf = struct.unpack_from("<HHH", p, 0)
            res = p[6:14]
            new = None
            if res[6:8] == b"\xff\xff":
                kind = res[0]
                if kind == 1:
                    new = (0x0205, struct.pack("<HHHBB", row, col, xf, res[2], 0))
                elif kind == 2:
                    new = (0x0205, struct.pack("<HHHBB", row, col, xf, res[2], 1))
                elif kind == 3:
                    new = (0x0204, struct.pack("<HHH", row, col, xf) + (b"\0\0\0" if biff == 8 else b"\0\0"))
                elif kind == 0:
                    # the text is in the STRING record that follows, perhaps after the record of a shared formula, an array, or a table
                    j = i + 1
                    while j < n and recs[j][1] in (0x04BC, 0x0221, 0x0236):
                        j += 1
                    if j < n and recs[j][1] == 0x0207:
                        new = (0x0204, struct.pack("<HHH", row, col, xf) + recs[j][2])
                        out.append((o, new[0], new[1]))
                        i = j + 1
                        continue
                    new = (0x0204, struct.pack("<HHH", row, col, xf) + (b"\0\0\0" if biff == 8 else b"\0\0"))
                else:
                    new = (0x0201, struct.pack("<HHH", row, col, xf))
            else:
                new = (0x0203, struct.pack("<HHH", row, col, xf) + res)
            out.append((o, new[0], new[1]))
            i += 1
            continue
        if rid in (0x04BC, 0x0221, 0x0236, 0x0207):
            i += 1
            continue
        out.append((o, rid, p))
        i += 1
    # write, and note where each start of a substream went
    pieces = []
    at = 0
    moved = {}
    bound_at = []
    for o, rid, p in out:
        if rid in bof_ids:
            moved[o] = at
        if rid == 0x0085:
            bound_at.append(len(pieces))
        pieces.append(rec(rid, p))
        at += len(pieces[-1])
    for k in bound_at:
        rid, p = struct.unpack_from("<HH", pieces[k], 0)[0], pieces[k][4:]
        old = struct.unpack_from("<I", p, 0)[0]
        if old in moved:
            p = struct.pack("<I", moved[old]) + p[4:]
            pieces[k] = rec(0x0085, p)
    return b"".join(pieces)


def convert(args):
    n, path, out_dir = args
    try:
        data = open(path, "rb").read()
    except Exception as e:
        return n, path, "not read: %s" % e
    out = os.path.join(out_dir, "%05d.xls" % n)
    try:
        if data[:8] == SIG:
            st = cfb_stream(data, ("Workbook", "Book"))
            if st is None:
                return n, path, "no workbook stream"
            vers = struct.unpack_from("<H", st, 4)[0] if len(st) >= 6 else 0
            biff = 8 if vers >= 0x600 else 5
            if len(st) < 4 or struct.unpack_from("<H", st, 0)[0] != 0x0809:
                return n, path, "not a workbook"
            # (a workbook that is encrypted is not rewritten)
            for _, rid, _ in records(st):
                if rid == 0x002F:
                    return n, path, "encrypted"
                if rid == 0x000A:
                    break
            new = rewrite(st, biff)
            open(out, "wb").write(cfb_write(new, "Workbook" if biff == 8 else "Book"))
            return n, path, "rewritten, biff%d" % biff
        if data[:2] in (b"\x09\x02", b"\x09\x04"):
            open(out, "wb").write(rewrite(data, 4))
            return n, path, "rewritten, biff4"
        return n, path, "not a workbook of the old form"
    except Exception as e:
        return n, path, "failed: %s" % e


def main():
    if len(sys.argv) < 3:
        print(__doc__)
        return 2
    lst, out_dir = sys.argv[1:3]
    jobs = int(sys.argv[3]) if len(sys.argv) > 3 else 8
    os.makedirs(out_dir, exist_ok=True)
    paths = [l.rstrip("\n") for l in open(lst) if l.strip()]
    with ProcessPoolExecutor(jobs) as ex, open(os.path.join(out_dir, "index.tsv"), "w") as ix:
        for n, path, what in ex.map(convert, [(i, p, out_dir) for i, p in enumerate(paths)], chunksize=8):
            ix.write("%05d\t%s\t%s\n" % (n, path, what))
    return 0


if __name__ == "__main__":
    sys.exit(main())
