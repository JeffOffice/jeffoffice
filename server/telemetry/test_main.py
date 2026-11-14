"""Tests for the usage statistics collector: python3 -m unittest test_main"""
import base64
import contextlib
import gzip
import io
import ipaddress
import datetime as dt
import json
import os
import sqlite3
import tempfile
import threading
import unittest
from unittest import mock
import urllib.error
import urllib.request
import uuid

os.environ["DATA_DIR"] = tempfile.mkdtemp()
os.environ["STATS_PASSWORD"] = "secret"
import main  # noqa: E402  (reads the environment above)

INSTALL = "0f8fad5b-d9cb-469f-a165-70867728950e"


def utc_today():
    return dt.datetime.now(dt.timezone.utc).date()


def row_index(table, column):
    with main.db() as con:
        return [r[1] for r in con.execute("PRAGMA table_info(" + table + ")")].index(column)


class Collector(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        main.init()
        cls.server = main.ThreadingHTTPServer(("127.0.0.1", 0), main.Handler)
        cls.url = "http://127.0.0.1:%d" % cls.server.server_address[1]
        threading.Thread(target=cls.server.serve_forever, daemon=True).start()

    @classmethod
    def tearDownClass(cls):
        cls.server.shutdown()

    def post(self, body, path="/v1/ping"):
        data = body if isinstance(body, bytes) else json.dumps(body).encode()
        req = urllib.request.Request(self.url + path, data=data, headers={"Content-Type": "application/json"})
        try:
            return urllib.request.urlopen(req).status
        except urllib.error.HTTPError as e:
            return e.code

    def get(self, path, password=None):
        req = urllib.request.Request(self.url + path)
        if password is not None:
            req.add_header("Authorization", "Basic " + base64.b64encode(b"jeff:" + password.encode()).decode())
        try:
            r = urllib.request.urlopen(req)
            return r.status, r.read().decode()
        except urllib.error.HTTPError as e:
            return e.code, ""

    def ping(self, **over):
        p = {"install": INSTALL, "version": "0.1.37", "os": "windows", "osVersion": "10.0.22631", "lang": "en-US",
             "launches": 1, "counts": {"file.open.pub": 2, "export.pdf": 1}}
        p.update(over)
        return p

    def test_pings_add_up_by_day(self):
        self.assertEqual(self.post(self.ping()), 204)
        self.assertEqual(self.post(self.ping(counts={"file.open.pub": 3})), 204)
        with main.db() as con:
            launches = con.execute("SELECT launches FROM pings WHERE install = ?", (INSTALL,)).fetchone()[0]
            n = con.execute("SELECT SUM(n) FROM counts WHERE install = ? AND feature = 'file.open.pub'", (INSTALL,)).fetchone()[0]
        self.assertEqual(launches, 2)
        self.assertEqual(n, 5)

    def app_rows(self, table, install, app):
        with main.db() as con:
            return con.execute("SELECT * FROM " + table + " WHERE install = ? AND app = ?", (install, app)).fetchall()

    def test_records_older_than_24_months_are_purged(self):
        old, kept = str(uuid.uuid4()), str(uuid.uuid4())
        today = dt.date(2028, 11, 3)
        main.record(main.parse_ping(json.dumps(self.ping(install=old)).encode()), today=dt.date(2026, 11, 2))
        main.record(main.parse_ping(json.dumps(self.ping(install=kept)).encode()), today=dt.date(2026, 11, 3))
        main.purge(today)
        self.assertEqual(self.app_rows("pings", old, "jeffpub"), [])
        self.assertEqual(self.app_rows("counts", old, "jeffpub"), [])
        self.assertEqual(len(self.app_rows("pings", kept, "jeffpub")), 1)

    def test_the_cutoff_is_24_months_back(self):
        self.assertEqual(main.cutoff(dt.date(2028, 11, 3)), dt.date(2026, 11, 3))
        self.assertEqual(main.cutoff(dt.date(2028, 2, 29)), dt.date(2026, 2, 28))
        self.assertEqual(main.cutoff(dt.date(2028, 3, 31)), dt.date(2026, 3, 31))

    def test_forget_deletes_one_install(self):
        gone, kept = str(uuid.uuid4()), str(uuid.uuid4())
        for i in (gone, kept):
            self.assertEqual(self.post(self.ping(install=i)), 204)
        self.assertGreater(main.forget(gone), 0)
        self.assertEqual(self.app_rows("pings", gone, "jeffpub"), [])
        self.assertEqual(self.app_rows("counts", gone, "jeffpub"), [])
        self.assertEqual(len(self.app_rows("pings", kept, "jeffpub")), 1)

    def test_a_ping_with_no_app_is_jeffpubs(self):
        # What JeffPub sent before JeffDeck did: no "app" in it.
        install = str(uuid.uuid4())
        self.assertNotIn("app", self.ping())
        self.assertEqual(self.post(self.ping(install=install)), 204)
        self.assertEqual(len(self.app_rows("pings", install, "jeffpub")), 1)
        self.assertEqual(self.app_rows("pings", install, "jeffdeck"), [])
        self.assertEqual(self.post(self.ping(install=install, app="jeffpub", counts={"file.open.pub": 1})), 204)
        (row,) = self.app_rows("pings", install, "jeffpub")
        self.assertEqual(row[row_index("pings", "launches")], 2)

    def test_each_app_has_its_own_ping_and_counts_for_the_day(self):
        install = str(uuid.uuid4())
        self.assertEqual(self.post(self.ping(install=install, app="jeffpub", launches=2, counts={"file.open.pub": 3})), 204)
        self.assertEqual(self.post(self.ping(install=install, app="jeffdeck", launches=1, counts={"file.open.pptx": 4, "file.save": 1})), 204)
        self.assertEqual(self.post(self.ping(install=install, app="jeffdeck", launches=1, counts={"file.open.pptx": 1})), 204)
        with main.db() as con:
            pings = con.execute("SELECT app, launches FROM pings WHERE install = ? ORDER BY app", (install,)).fetchall()
            counts = con.execute("SELECT app, feature, n FROM counts WHERE install = ? ORDER BY app, feature", (install,)).fetchall()
        self.assertEqual(pings, [("jeffdeck", 2), ("jeffpub", 2)])
        self.assertEqual(counts, [("jeffdeck", "file.open.pptx", 5), ("jeffdeck", "file.save", 1), ("jeffpub", "file.open.pub", 3)])

    def test_the_app_is_checked(self):
        install = str(uuid.uuid4())
        for bad in ["", "JeffDeck", "jeffdeck1", "jeff-deck", "jeff deck", "jeffdeck\n", "jeffmail", "x", "a" * 17,
                    "../jeffpub", 5, None, True, ["jeffdeck"], {"jeffdeck": 1}]:
            with self.subTest(app=bad):
                self.assertEqual(self.post(self.ping(install=install, app=bad)), 400)
        with main.db() as con:
            self.assertEqual(con.execute("SELECT COUNT(*) FROM pings WHERE install = ?", (install,)).fetchone()[0], 0)

    def test_bad_pings_are_refused(self):
        self.assertEqual(self.post(self.ping(install="not-a-uuid")), 400)
        self.assertEqual(self.post(self.ping(counts={"C:\\Users\\jeff\\letter.pub": 1})), 400)
        self.assertEqual(self.post(self.ping(counts={"x": -1})), 400)
        self.assertEqual(self.post(self.ping(lang="<script>")), 400)
        self.assertEqual(self.post(b"{not json"), 400)
        self.assertEqual(self.post(b"[" * 16000), 400)   # nested too deep for the parser
        self.assertEqual(self.post(b"x" * (main.MAX_BODY + 1)), 413)

    def test_stats_need_the_password(self):
        self.post(self.ping())
        self.assertEqual(self.get("/stats")[0], 401)
        self.assertEqual(self.get("/stats", "wrong")[0], 401)
        code, page = self.get("/stats", "secret")
        self.assertEqual(code, 200)
        self.assertIn("0.1.37", page)
        self.assertIn("export.pdf", page)

    def test_public_pages(self):
        self.assertEqual(self.get("/health"), (200, "ok"))
        code, page = self.get("/")
        self.assertEqual(code, 200)
        self.assertIn("Never your files", page)

    def test_public_page_names_every_app(self):
        code, page = self.get("/")
        self.assertEqual(code, 200)
        text = " ".join(page.split())
        self.assertIn("<title>JeffOffice usage statistics</title>", text)
        self.assertIn("JeffPub, JeffDeck, JeffWrite, and JeffCrunch", text)
        self.assertIn("which app", text)
        self.assertNotIn("JeffDeck sends no", text)

    def test_public_page_explains_crash_reports(self):
        code, page = self.get("/")
        self.assertEqual(code, 200)
        text = " ".join(page.lower().split())
        self.assertIn("crash report", text)
        self.assertIn("next time it starts", text)
        self.assertIn("usage statistics are on", text)
        for never in ("documents", "names", "contents", "typed", "memory"):
            self.assertIn(never, text)

    def setUp(self):
        with main.db() as con:
            con.execute("DELETE FROM crashes")

    def crash(self, **over):
        r = {"install": str(uuid.uuid4()), "version": "0.6.5", "os": "windows", "kind": "exception",
             "code": 0xC0000005,
             "frames": [{"module": "JeffPub.exe", "offset": 0x1A2B}, {"module": "ntdll.dll", "offset": 0x9F3A1}],
             "when": utc_today().isoformat()}
        r.update(over)
        return r

    def send(self, report=None, **over):
        return self.post(self.crash(**over) if report is None else report, "/v1/crash")

    def rows(self, where="1", *args):
        with main.db() as con:
            return con.execute(
                "SELECT received, happened, install, version, os, kind, code, frames FROM crashes WHERE " + where
                + " ORDER BY id", args).fetchall()

    def test_a_good_report_is_stored(self):
        install = str(uuid.uuid4())
        self.assertEqual(self.send(install=install), 204)
        (row,) = self.rows("install = ?", install)
        received, happened, _, version, os_name, kind, code, frames = row
        self.assertEqual(received, utc_today().isoformat())
        self.assertEqual(happened, utc_today().isoformat())
        self.assertEqual((version, os_name, kind, code), ("0.6.5", "windows", "exception", 0xC0000005))
        self.assertEqual(frames, '[{"module":"JeffPub.exe","offset":6699},{"module":"ntdll.dll","offset":652193}]')

    def test_a_report_says_which_app_crashed(self):
        a, b, c = (str(uuid.uuid4()) for _ in range(3))
        self.assertEqual(self.send(install=a), 204)                     # no app: JeffPub's, as before
        self.assertEqual(self.send(install=b, app="jeffpub"), 204)
        self.assertEqual(self.send(install=c, app="jeffdeck"), 204)
        with main.db() as con:
            apps = dict(con.execute("SELECT install, app FROM crashes").fetchall())
        self.assertEqual(apps, {a: "jeffpub", b: "jeffpub", c: "jeffdeck"})

    def test_a_report_with_a_bad_app_is_refused(self):
        for bad in ["", "JeffDeck", "jeffdeck1", "jeff deck", "jeffdeck\n", "jeffmail", "x", "a" * 17, "../jeffpub", 5, None, True, ["jeffdeck"]]:
            with self.subTest(app=bad):
                self.assertEqual(self.send(app=bad), 400)
        self.assertEqual(self.rows(), [])

    def test_signals_and_signed_windows_codes_are_accepted(self):
        self.assertEqual(self.send(kind="signal", code=11, os="linux", frames=[]), 204)
        self.assertEqual(self.send(code=-1073741819), 204)
        self.assertEqual(self.send(code=0), 204)
        self.assertEqual([r[6] for r in self.rows()], [11, -1073741819, 0])

    def test_the_table_holds_no_free_text(self):
        with main.db() as con:
            columns = [r[1] for r in con.execute("PRAGMA table_info(crashes)")]
        self.assertEqual(columns, ["id", "received", "happened", "install", "version", "os", "kind", "code", "frames", "app"])

    def test_init_adds_the_table_to_an_older_database(self):
        with main.db() as con:
            con.execute("DROP TABLE crashes")
        main.init()
        main.init()
        self.assertEqual(self.send(), 204)
        self.assertEqual(len(self.rows()), 1)

    def test_bad_crash_reports_are_refused(self):
        bad = {
            "install": ["not-a-uuid", "", 5, None, "0F8FAD5B-D9CB-469F-A165-70867728950E", INSTALL + "x", INSTALL + "\n"],
            "version": ["", "1" * 21, "1.0 beta", "../1", "0.6.5\n", 5, None, ["0.6.5"]],
            "os": ["", "Windows", "win dows", "windows10", "windows\n", "a" * 17, 5, None],
            "kind": ["", "Signal", "crash", "assert", 1, None, ["signal"]],
            "code": ["11", 1.5, 11.0, True, None, [11], 2 ** 32, -2 ** 31 - 1, 10 ** 40],
            "frames": ["ntdll.dll+0x1", {"module": "a.dll", "offset": 1}, None, 5,
                       [{"module": "a.dll", "offset": 1}] * 65, ["ntdll.dll+0x1"], [None], [[]],
                       [{"module": "a.dll"}], [{"offset": 1}], [{}],
                       [{"module": "a.dll", "offset": 1, "path": "C:\\Users\\jeff"}],
                       [{"module": "a.dll", "offset": 1, "name": "x"}]],
            "when": ["", "2026-13-01", "2026-02-30", "20261011", "2026-10-11T10:00:00", " " + utc_today().isoformat(),
                     utc_today().isoformat() + "\n", "Oct 11 2026", 20261011, None,
                     (utc_today() + dt.timedelta(days=1)).isoformat(),
                     (utc_today() + dt.timedelta(days=400)).isoformat(),
                     (utc_today() - dt.timedelta(days=31)).isoformat(), "0001-01-01", "\u0662\u0660\u0662\u0666-\u0661\u0660-\u0661\u0661"],
        }
        modules = ["a/b.dll", "a\\b.dll", "C:\\Windows\\ntdll.dll", "/usr/lib/libc.so.6", "..\\x.dll", "../x.so", "",
                   "x" * 65, "ntdll dll", "n\u00fcll.dll", "a\nb", "<b>.dll", "a:b", "a,b", 5, None, ["a.dll"]]
        offsets = [-1, 2 ** 48, 10 ** 30, 1.5, 1.0, "16", True, None, [1]]
        for field, values in bad.items():
            for value in values:
                with self.subTest(field=field, value=value):
                    self.assertEqual(self.send(**{field: value}), 400)
        for module in modules:
            with self.subTest(module=module):
                self.assertEqual(self.send(frames=[{"module": module, "offset": 1}]), 400)
        for offset in offsets:
            with self.subTest(offset=offset):
                self.assertEqual(self.send(frames=[{"module": "a.dll", "offset": offset}]), 400)
        self.assertEqual(self.rows(), [])

    def test_missing_and_unknown_fields_are_refused(self):
        for field in ("install", "version", "os", "kind", "code", "frames", "when"):
            with self.subTest(missing=field):
                report = self.crash()
                del report[field]
                self.assertEqual(self.send(report), 400)
        for extra in ("message", "text", "note", "path", "osVersion", "lang", "counts", "Install"):
            with self.subTest(extra=extra):
                self.assertEqual(self.send(**{extra: "hello"}), 400)
        self.assertEqual(self.rows(), [])

    def test_whole_bodies_that_are_not_reports_are_refused(self):
        for body in (b"", b"{not json", b"null", b"[]", b'"text"', b"5", b"{}", b'[{"install": 1}]',
                     b"\xff\xfe", b"[" * 10000, b"1" * 5000):
            with self.subTest(body=body[:20]):
                self.assertEqual(self.post(body, "/v1/crash"), 400)
        self.assertEqual(self.post(b"x" * (main.MAX_BODY + 1), "/v1/crash"), 413)
        self.assertEqual(self.rows(), [])

    def test_the_edges_of_every_limit_are_accepted(self):
        today = dt.date(2026, 10, 11)
        edge = self.crash(when="2026-09-11", code=2 ** 32 - 1,
                          frames=[{"module": "A" * 64, "offset": 2 ** 48 - 1}] + [{"module": "a.b-c_d+e", "offset": 0}] * 63)
        self.assertIsNotNone(main.parse_crash(json.dumps(edge).encode(), today))
        self.assertIsNotNone(main.parse_crash(json.dumps(self.crash(when="2026-10-11", code=-2 ** 31, frames=[])).encode(), today))
        self.assertIsNone(main.parse_crash(json.dumps(self.crash(when="2026-09-10")).encode(), today))
        self.assertIsNone(main.parse_crash(json.dumps(self.crash(when="2026-10-12")).encode(), today))

    def test_the_biggest_good_report_fits_the_body_limit(self):
        frames = [{"module": "m" * 64, "offset": 2 ** 48 - 1}] * 64
        body = json.dumps(self.crash(frames=frames), indent=2).encode()
        self.assertLess(len(body), main.MAX_BODY)
        self.assertEqual(self.post(body, "/v1/crash"), 204)

    def test_ten_reports_a_day_are_stored_per_install(self):
        install = str(uuid.uuid4())
        self.assertEqual([self.send(install=install, code=i) for i in range(12)], [204] * 10 + [202] * 2)
        self.assertEqual(len(self.rows("install = ?", install)), 10)
        self.assertEqual(self.send(), 204)
        self.assertEqual(self.send(install=install), 202)
        self.assertEqual(len(self.rows("install = ?", install)), 10)

    def test_the_limit_is_for_each_app_of_an_install(self):
        install = str(uuid.uuid4())
        self.assertEqual([self.send(install=install, app="jeffdeck", code=i) for i in range(11)], [204] * 10 + [202])
        # JeffPub's reports from the same install still get in.
        self.assertEqual(self.send(install=install, app="jeffpub"), 204)
        self.assertEqual(len(self.rows("install = ?", install)), 11)

    def test_the_limit_starts_over_each_day(self):
        install = str(uuid.uuid4())
        report = main.parse_crash(json.dumps(self.crash(install=install)).encode())
        yesterday = utc_today() - dt.timedelta(days=1)
        self.assertEqual([main.record_crash(report, yesterday) for _ in range(11)], [True] * 10 + [False])
        self.assertTrue(main.record_crash(report, utc_today()))
        self.assertEqual(len(self.rows("install = ?", install)), 11)

    def test_crashes_page_needs_the_password(self):
        self.send()
        self.assertEqual(self.get("/crashes")[0], 401)
        self.assertEqual(self.get("/crashes", "wrong")[0], 401)
        code, page = self.get("/crashes", "secret")
        self.assertEqual(code, 200)
        self.assertIn("ntdll.dll+0x9f3a1", page)

    def test_crashes_page_groups_identical_frames(self):
        same = [{"module": "JeffPub.exe", "offset": 0x1A2B}, {"module": "Qt6Core.dll", "offset": 0x4D2}]
        other = [{"module": "libfoo.so.6", "offset": 0xBEEF}]
        installs = [str(uuid.uuid4()) for _ in range(4)]
        self.send(install=installs[0], frames=same, version="0.6.5")
        self.send(install=installs[1], frames=same, version="0.6.6", os="macos")
        self.send(install=installs[2], frames=other, kind="signal", code=11, os="linux")
        self.send(install=installs[3], frames=same, version="0.6.5")
        code, page = self.get("/crashes", "secret")
        self.assertEqual(code, 200)
        self.assertEqual(page.count("JeffPub.exe+0x1a2b"), 1)
        self.assertEqual(page.count("Qt6Core.dll+0x4d2"), 1)
        self.assertEqual(page.count("libfoo.so.6+0xbeef"), 1)
        self.assertIn("<td>3</td>", page)
        self.assertIn("<td>1</td>", page)
        self.assertIn("0.6.5, 0.6.6", page)
        self.assertIn("macos", page)
        self.assertIn("0xC0000005", page)
        self.assertIn("<td>signal</td><td>11</td>", page)
        for install in installs:
            self.assertNotIn(install, page)

    def test_crashes_page_lists_the_newest_hundred(self):
        for i in range(105):
            self.send(frames=[{"module": "a.dll", "offset": i}])
        code, page = self.get("/crashes", "secret")
        self.assertEqual(code, 200)
        self.assertEqual(page.count("a.dll+0x"), 100)
        self.assertIn("a.dll+0x68<", page)
        self.assertNotIn("a.dll+0x4<", page)
        self.assertIn("a.dll+0x5<", page)

    def test_crashes_page_with_no_reports(self):
        code, page = self.get("/crashes", "secret")
        self.assertEqual(code, 200)
        self.assertIn("none yet", page)

    def test_a_report_with_no_frames_shows_so(self):
        self.send(frames=[])
        code, page = self.get("/crashes", "secret")
        self.assertEqual(code, 200)
        self.assertIn("<td>1</td>", page)
        self.assertIn("<td><i>no frames</i></td></tr>", page)

    def test_stats_count_crashes_per_version_for_30_days(self):
        self.send(version="9.9.9")
        self.send(version="9.9.9")
        self.send(version="9.9.9", install=INSTALL)
        self.send(version="9.9.9", install=INSTALL)
        self.send(version="9.9.8")
        old = main.parse_crash(json.dumps(self.crash(version="8.8.8")).encode())
        main.record_crash(old, utc_today() - dt.timedelta(days=30))
        code, page = self.get("/stats", "secret")
        self.assertEqual(code, 200)
        self.assertIn("Crashes", page)
        self.assertIn('<td>9.9.9</td><td class="r">4</td><td class="r">3</td>', page)
        self.assertIn('<td>9.9.8</td><td class="r">1</td><td class="r">1</td>', page)
        self.assertNotIn("8.8.8", page)
        self.assertIn('href="/crashes"', page)

    def test_stats_are_one_dashboard_with_every_section(self):
        self.post(self.ping(app="jeffdeck", version="8.8.1", counts={"deck.only": 3}, os="macos", lang="fr-FR"))
        code, page = self.get("/stats", "secret")
        self.assertEqual(code, 200)
        for text in ("<h1>JeffOffice usage</h1>", "Times are UTC", "Active per day", "Active, 30 days", "All apps",
                     "Installs by country", "Top countries", "Unknown:", "<h2>Version</h2>", "<h2>Operating system</h2>",
                     "<h2>Language</h2>", "<h2>Top commands</h2>", "Show all", "Crashes by version", "IP geolocation by",
                     'href="https://db-ip.com"', "d3@7.9.0", "topojson-client@3.1.0", "world-atlas@2.0.2",
                     'href="/stats?days=7"', 'href="/stats?days=90"', 'href="/crashes"', "var(--c-jeffdeck)", "8.8.1", "deck.only", "fr-FR"):
            self.assertIn(text, page)
        for name in main.APPS.values():
            self.assertIn(name, page)
        self.assertEqual(self.get("/stats?days=7", "secret")[0], 200)
        self.assertIn("Active, 90 days", self.get("/stats?days=90", "secret")[1])

    def test_stats_take_no_app_filter(self):
        self.assertEqual(self.get("/stats?app=jeffdeck", "secret")[0], 404)

    def test_stats_count_each_apps_crashes_apart(self):
        self.send(app="jeffdeck", version="6.6.6")
        self.send(app="jeffdeck", version="6.6.6")
        self.send(app="jeffpub", version="6.6.5")
        rows = {r["app"]: r for r in main.gather_stats()["rows"]}
        self.assertEqual((rows["jeffdeck"]["crashes"], rows["jeffpub"]["crashes"], rows[None]["crashes"]), (2, 1, 3))
        _, page = self.get("/stats", "secret")
        self.assertIn('<td>6.6.6</td><td class="r">2</td><td class="r">2</td>', page)

    def test_the_rows_count_active_new_and_previous_period(self):
        today = utc_today()
        a, b = str(uuid.uuid4()), str(uuid.uuid4())
        with main.db() as con:
            for t in ("pings", "counts", "crashes"):
                con.execute("DELETE FROM " + t)
        for install, app, ago in ((a, "jeffpub", 3), (a, "jeffdeck", 0), (b, "jeffpub", 45)):
            main.record(main.parse_ping(json.dumps(self.ping(install=install, app=app)).encode()), today - dt.timedelta(days=ago))
        d = main.gather_stats(30, today)
        rows = {r["app"]: r for r in d["rows"]}
        key = ("active", "active_prev", "new", "new_prev")
        self.assertEqual([rows[None][k] for k in key], [1, 1, 1, 1])
        self.assertEqual([rows["jeffpub"][k] for k in key], [1, 1, 1, 1])
        self.assertEqual([rows["jeffdeck"][k] for k in key], [1, 0, 1, 0])
        self.assertEqual(len(rows["jeffpub"]["series"]), 30)
        self.assertEqual(rows["jeffdeck"]["series"][-1], 1)
        self.assertEqual(rows["jeffpub"]["series"][-4], 1)

    def test_the_public_page_says_why(self):
        _, page = self.get("/")
        self.assertIn("Why we collect this", page)
        self.assertLess(page.index("Why we collect this"), page.index("<ul>"))
        self.assertIn("works out", page)
        for url in ("https://www.jeffoffice.com/privacy/", "https://www.jeffoffice.com/terms/",
                    "https://github.com/JeffOffice/jeffoffice/blob/main/LICENSE", "https://www.jeffoffice.com/third-party/",
                    "https://github.com/JeffOffice/jeffoffice/tree/main/server/telemetry"):
            self.assertIn('href="%s"' % url, page)
        self.assertIn("Third-party licenses", page)

    def test_an_app_that_is_not_known_has_no_page(self):
        for path in ("/stats?app=jeffmail", "/stats?app=", "/stats?app=JeffDeck", "/stats?app=jeffdeck&app=x", "/crashes?app=jeffmail", "/crashes?app=%00", "/stats?days=10", "/stats?days=", "/stats?days=7&days=30"):
            with self.subTest(path=path):
                self.assertEqual(self.get(path, "secret")[0], 404)
        self.assertEqual(self.get("/stats?app=jeffdeck")[0], 401)   # the password still comes first
        self.assertEqual(self.get("/stats?other=1", "secret")[0], 200)

    def test_crashes_are_split_by_app(self):
        self.send(app="jeffpub", frames=[{"module": "JeffPub.exe", "offset": 0x1A2B}])
        self.send(app="jeffdeck", frames=[{"module": "JeffDeck.exe", "offset": 0x3C4D}], version="8.8.2")
        self.send(frames=[{"module": "JeffPub.exe", "offset": 0x1A2B}])   # no app: JeffPub's
        _, pub = self.get("/crashes?app=jeffpub", "secret")
        _, deck = self.get("/crashes?app=jeffdeck", "secret")
        _, everyone = self.get("/crashes", "secret")
        self.assertIn("JeffPub.exe", everyone)
        self.assertIn("JeffDeck.exe", everyone)
        self.assertIn("JeffPub.exe+0x1a2b", pub)
        self.assertNotIn("JeffDeck.exe", pub)
        self.assertIn("<td>2</td>", pub)                               # the two JeffPub reports share a row
        self.assertIn("JeffDeck.exe+0x3c4d", deck)
        self.assertNotIn("JeffPub.exe", deck)
        self.assertIn("<h1>JeffDeck: crashes</h1>", deck)
        for page in (pub, deck):
            self.assertIn('href="/crashes?app=jeffpub"', page)
            self.assertIn('href="/crashes?app=jeffdeck"', page)

    def test_stats_say_so_when_nothing_crashed(self):
        code, page = self.get("/stats", "secret")
        self.assertEqual(code, 200)
        self.assertIn("No crashes in this period", page)

    def test_crash_path_takes_only_post(self):
        self.assertEqual(self.get("/v1/crash")[0], 404)
        self.assertEqual(self.post(self.crash(), "/v1/crashes"), 404)


GEO_CSV = """1.0.0.0,1.0.0.255,AU
1.0.1.0,1.0.3.255,CN
8.8.8.0,8.8.8.255,US
2001:db8::,2001:db8:0:ffff:ffff:ffff:ffff:ffff,DE
2a00::,2a00:0:0:1:ffff:ffff:ffff:ffff,FR
9.9.9.0,9.9.9.255,ZZ
"""


def fixture_geo():
    return main.Geo(GEO_CSV.splitlines())


class Country(unittest.TestCase):
    def look(self, geo, text):
        return geo.lookup(ipaddress.ip_address(text))

    def test_lookup_finds_ranges_and_their_edges(self):
        g = fixture_geo()
        for text, want in (("1.0.0.0", "AU"), ("1.0.0.255", "AU"), ("1.0.1.0", "CN"), ("1.0.3.255", "CN"), ("1.0.4.0", None),
                           ("0.255.255.255", None), ("8.8.8.8", "US"), ("9.9.9.9", None),      # ZZ means unknown
                           ("2001:db8::1", "DE"), ("2001:db8:0:ffff::1", "DE"), ("2001:db8:0:1:0:0:0:0", "DE"), ("2001:db8:1::", None),
                           ("2a00::", "FR"), ("2a00:0:0:1::5", "FR"), ("2a00:0:0:2::", None), ("::1", None)):
            self.assertEqual(self.look(g, text), want, text)

    def test_lookup_reads_a_gzip_file_and_unsorted_rows(self):
        path = os.path.join(tempfile.mkdtemp(), "x.csv.gz")
        with gzip.open(path, "wt") as f:
            f.write('"8.8.8.0","8.8.8.255","US"\n1.0.0.0,1.0.0.255,AU\nnot,a,row\n')
        g = main.load_geo(path)
        self.assertEqual((g.rows, self.look(g, "1.0.0.7"), self.look(g, "8.8.8.8")), (2, "AU", "US"))

    def test_the_visitor_address_comes_from_x_real_ip_then_the_socket(self):
        ip = lambda headers, peer: str(main.client_ip(headers, peer))
        self.assertEqual(ip({"X-Real-IP": "8.8.8.8"}, "10.0.0.1"), "8.8.8.8")
        self.assertEqual(ip({"X-Real-IP": " 2a00::1 "}, "10.0.0.1"), "2a00::1")
        self.assertEqual(ip({"X-Real-IP": "nonsense"}, "1.0.0.9"), "1.0.0.9")
        self.assertEqual(ip({}, "1.0.0.9"), "1.0.0.9")
        self.assertEqual(ip({"X-Real-IP": "8.8.8.8", "X-Forwarded-For": "1.0.0.9"}, "10.0.0.1"), "8.8.8.8")   # XFF is never read
        self.assertEqual(ip({"X-Real-IP": "::ffff:8.8.8.8"}, "10.0.0.1"), "8.8.8.8")

    def test_private_loopback_and_invalid_addresses_have_no_country(self):
        with mock.patch.object(main, "_geo", fixture_geo()):
            for header in ("10.1.2.3", "192.168.0.1", "127.0.0.1", "::1", "fe80::1", "100.64.0.1", "garbage", ""):
                self.assertIsNone(main.country_of({"X-Real-IP": header}, "127.0.0.1"), header)
            self.assertEqual(main.country_of({"X-Real-IP": "8.8.8.8"}, "127.0.0.1"), "US")

    def test_no_database_means_no_country(self):
        with mock.patch.object(main, "_geo", None):
            self.assertIsNone(main.country_of({"X-Real-IP": "8.8.8.8"}, "127.0.0.1"))

    def test_a_download_that_is_too_small_is_refused_and_the_old_file_stays(self):
        d = tempfile.mkdtemp()
        old = os.path.join(d, "dbip-country-lite-2026-09.csv.gz")
        with gzip.open(old, "wt") as f:
            f.write(GEO_CSV)
        os.utime(old, (1, 1))   # ancient
        small = io.BytesIO(gzip.compress(GEO_CSV.encode()))
        with mock.patch.object(main, "GEO_DIR", d), mock.patch.object(main, "_geo", None), \
             mock.patch.object(main.urllib.request, "urlopen", lambda *a, **k: contextlib.nullcontext(small)), \
             contextlib.redirect_stdout(io.StringIO()):
            main.ensure_geo(dt.date(2026, 11, 14))
            self.assertEqual(os.listdir(d), ["dbip-country-lite-2026-09.csv.gz"])
            self.assertEqual(self.look(main._geo, "8.8.8.8"), "US")   # loaded from the old file

    def test_a_good_download_replaces_the_old_file(self):
        d = tempfile.mkdtemp()
        old = os.path.join(d, "dbip-country-lite-2026-09.csv.gz")
        with gzip.open(old, "wt") as f:
            f.write(GEO_CSV)
        os.utime(old, (1, 1))
        new = gzip.compress(GEO_CSV.replace("8.8.8.0,8.8.8.255,US", "8.8.8.0,8.8.8.255,CA").encode())
        with mock.patch.object(main, "GEO_DIR", d), mock.patch.object(main, "_geo", None), mock.patch.object(main, "GEO_MIN_ROWS", 3), \
             mock.patch.object(main.urllib.request, "urlopen", lambda *a, **k: contextlib.nullcontext(io.BytesIO(new))), \
             contextlib.redirect_stdout(io.StringIO()):
            main.ensure_geo(dt.date(2026, 11, 14))
            self.assertEqual(os.listdir(d), ["dbip-country-lite-2026-11.csv.gz"])
            self.assertEqual(self.look(main._geo, "8.8.8.8"), "CA")

    def test_the_address_is_never_stored_or_logged(self):
        main.init()
        server = main.ThreadingHTTPServer(("127.0.0.1", 0), main.Handler)
        threading.Thread(target=server.serve_forever, daemon=True).start()
        out = io.StringIO()
        try:
            with mock.patch.object(main, "_geo", fixture_geo()), contextlib.redirect_stdout(out):
                for ip in ("8.8.8.8", "2a00::77"):
                    body = json.dumps({"install": str(uuid.uuid4()), "version": "0.1.37", "os": "linux", "launches": 1}).encode()
                    req = urllib.request.Request("http://127.0.0.1:%d/v1/ping?x=%s" % (server.server_address[1], ip), data=body,
                                                 headers={"X-Real-IP": ip, "X-Forwarded-For": ip})
                    self.assertEqual(urllib.request.urlopen(req).status, 204)
                with main.db() as con:
                    con.execute("PRAGMA wal_checkpoint(FULL)")
        finally:
            server.shutdown()
        with main.db() as con:
            self.assertEqual(sorted(r[0] for r in con.execute("SELECT country FROM pings WHERE country IS NOT NULL")).count("US"), 1)
        stored = b"".join(open(main.DB_PATH + suffix, "rb").read() for suffix in ("", "-wal") if os.path.exists(main.DB_PATH + suffix))
        for ip in ("8.8.8.8", "2a00::77", "127.0.0.1"):
            self.assertNotIn(ip.encode(), stored)
            self.assertNotIn(ip, out.getvalue())

    def test_a_same_day_ping_keeps_the_newest_known_country(self):
        main.init()
        install = str(uuid.uuid4())
        ping = main.parse_ping(json.dumps({"install": install, "version": "1.0", "os": "linux"}).encode())
        for country in ("US", None, "CA", None):
            main.record(ping, country=country)
        with main.db() as con:
            self.assertEqual(con.execute("SELECT country FROM pings WHERE install = ?", (install,)).fetchall(), [("CA",)])


OLD_SCHEMA = """
CREATE TABLE pings (
    day TEXT NOT NULL, install TEXT NOT NULL, version TEXT, os TEXT,
    os_version TEXT, lang TEXT, launches INTEGER NOT NULL DEFAULT 0,
    PRIMARY KEY (day, install));
CREATE TABLE counts (
    day TEXT NOT NULL, install TEXT NOT NULL, feature TEXT NOT NULL,
    n INTEGER NOT NULL, PRIMARY KEY (day, install, feature));
CREATE INDEX counts_day ON counts (day);
CREATE TABLE crashes (
    id INTEGER PRIMARY KEY AUTOINCREMENT, received TEXT NOT NULL, happened TEXT NOT NULL,
    install TEXT NOT NULL, version TEXT NOT NULL, os TEXT NOT NULL, kind TEXT NOT NULL,
    code INTEGER NOT NULL, frames TEXT NOT NULL);
CREATE INDEX crashes_received ON crashes (received, install);
"""

OLD_ROWS = [
    ("INSERT INTO pings VALUES ('2026-10-01', 'aaaaaaaa-0000-4000-8000-000000000001', '0.7.0', 'windows', '10.0.22631', 'en-US', 2)",),
    ("INSERT INTO pings VALUES ('2026-10-02', 'aaaaaaaa-0000-4000-8000-000000000001', '0.7.0', 'windows', '10.0.22631', 'en-US', 1)",),
    ("INSERT INTO pings VALUES ('2026-10-02', 'aaaaaaaa-0000-4000-8000-000000000002', '0.6.11', 'macos', '14.5', 'de-DE', 3)",),
    ("INSERT INTO counts VALUES ('2026-10-01', 'aaaaaaaa-0000-4000-8000-000000000001', 'file.open.pub', 4)",),
    ("INSERT INTO counts VALUES ('2026-10-01', 'aaaaaaaa-0000-4000-8000-000000000001', 'export.pdf', 1)",),
    ("INSERT INTO counts VALUES ('2026-10-02', 'aaaaaaaa-0000-4000-8000-000000000002', 'file.open.pub', 7)",),
    ("INSERT INTO crashes VALUES (7, '2026-10-02', '2026-10-01', 'aaaaaaaa-0000-4000-8000-000000000001', '0.7.0', 'windows', 'exception', 3221225477, '[{\"module\":\"JeffPub.exe\",\"offset\":6699}]')",),
    ("INSERT INTO crashes VALUES (9, '2026-10-03', '2026-10-03', 'aaaaaaaa-0000-4000-8000-000000000002', '0.6.11', 'macos', 'signal', 11, '[]')",),
]


class Migration(unittest.TestCase):
    """The database from before there were two apps."""

    def setUp(self):
        self.path = os.path.join(tempfile.mkdtemp(), "old.db")
        patch = mock.patch.object(main, "DB_PATH", self.path)
        patch.start()
        self.addCleanup(patch.stop)

    def old_database(self, schema=OLD_SCHEMA):
        con = sqlite3.connect(self.path)
        con.executescript(schema)
        for (sql,) in OLD_ROWS:
            try:
                con.execute(sql)
            except sqlite3.OperationalError:
                pass   # a table this older shape doesn't have
        con.commit()
        con.close()

    def dump(self):
        with main.db() as con:
            tables = [r[0] for r in con.execute("SELECT name FROM sqlite_master WHERE type = 'table' AND name NOT LIKE 'sqlite_%' ORDER BY name")]
            return {t: con.execute("SELECT * FROM " + t + " ORDER BY 1, 2, 3").fetchall() for t in tables}

    def shape(self):
        with main.db() as con:
            tables = {t: [(r[1], r[2], r[3], r[5]) for r in con.execute("PRAGMA table_info(" + t + ")")]
                      for (t,) in con.execute("SELECT name FROM sqlite_master WHERE type = 'table' AND name NOT LIKE 'sqlite_%'").fetchall()}
            indexes = sorted(r[0] for r in con.execute("SELECT name FROM sqlite_master WHERE type = 'index' AND name NOT LIKE 'sqlite_%'"))
        return tables, indexes

    def test_every_old_row_becomes_jeffpubs(self):
        self.old_database()
        before = self.dump()
        main.init()
        after = self.dump()
        self.assertEqual(len(after["pings"]), 3)
        self.assertEqual(len(after["counts"]), 3)
        self.assertEqual(len(after["crashes"]), 2)
        with main.db() as con:
            for table in ("pings", "counts", "crashes"):
                self.assertEqual(con.execute("SELECT DISTINCT app FROM " + table).fetchall(), [("jeffpub",)], table)
            # Nothing else changed in any row.
            self.assertEqual(con.execute("SELECT day, install, version, os, os_version, lang, launches FROM pings ORDER BY 1, 2").fetchall(),
                             sorted(before["pings"]))
            self.assertEqual(con.execute("SELECT day, install, feature, n FROM counts ORDER BY 1, 2, 3").fetchall(), sorted(before["counts"]))
            self.assertEqual(con.execute("SELECT id, received, happened, install, version, os, kind, code, frames FROM crashes ORDER BY id").fetchall(),
                             sorted(before["crashes"]))

    def test_the_second_app_can_ping_on_the_same_day(self):
        self.old_database()
        main.init()
        install = "aaaaaaaa-0000-4000-8000-000000000001"
        ping = (install, "0.8.0", "windows", "10.0.22631", "en-US", 1, {"file.open.pptx": 1}, "jeffdeck")
        main.record(ping, dt.date(2026, 10, 2))
        main.record(ping[:-1] + ("jeffpub",), dt.date(2026, 10, 2))
        with main.db() as con:
            rows = con.execute("SELECT app, launches FROM pings WHERE install = ? AND day = '2026-10-02' ORDER BY app", (install,)).fetchall()
        self.assertEqual(rows, [("jeffdeck", 1), ("jeffpub", 2)])   # JeffPub's old row of 1 plus today's 1
        # Crash numbers go on from where they were.
        main.record_crash(main.parse_crash(json.dumps({"install": install, "version": "0.8.0", "os": "windows", "kind": "signal", "code": 11,
                                                       "frames": [], "when": utc_today().isoformat(), "app": "jeffdeck"}).encode()))
        with main.db() as con:
            self.assertEqual(con.execute("SELECT MAX(id) FROM crashes").fetchone()[0], 10)

    def test_running_it_again_changes_nothing(self):
        self.old_database()
        main.init()
        once = self.dump(), self.shape()
        main.init()
        main.init()
        self.assertEqual((self.dump(), self.shape()), once)

    def test_an_old_database_ends_up_shaped_like_a_new_one(self):
        self.old_database()
        main.init()
        migrated = self.shape()
        os.remove(self.path)
        main.init()
        self.assertEqual(self.shape(), migrated)
        self.assertEqual(self.dump(), {"counts": [], "crashes": [], "pings": []})

    def test_a_database_with_no_crashes_table_gets_one(self):
        self.old_database(OLD_SCHEMA.split("CREATE TABLE crashes")[0])
        main.init()
        self.assertEqual(len(self.dump()["pings"]), 3)
        with main.db() as con:
            self.assertEqual([r[1] for r in con.execute("PRAGMA table_info(crashes)")][-1], "app")

    def test_a_migration_that_fails_leaves_the_old_database_whole(self):
        self.old_database()
        before = self.dump(), self.shape()
        with mock.patch.object(main, "migrate_crashes", side_effect=RuntimeError("disk full")):
            with self.assertRaises(RuntimeError):
                main.init()
        self.assertEqual((self.dump(), self.shape()), before)   # pings and counts, already rebuilt inside it, are back as they were
        main.init()                                             # and the next start migrates as usual
        with main.db() as con:
            self.assertEqual(con.execute("SELECT DISTINCT app FROM pings").fetchall(), [("jeffpub",)])
        self.assertEqual(len(self.dump()["pings"]), 3)

    def test_a_migrated_database_still_refuses_a_second_row_for_one_app(self):
        self.old_database()
        main.init()
        with main.db() as con:
            with self.assertRaises(sqlite3.IntegrityError):
                con.execute("INSERT INTO pings (day, install, app, version) VALUES ('2026-10-01', 'aaaaaaaa-0000-4000-8000-000000000001', 'jeffpub', '1')")
            con.execute("INSERT INTO pings (day, install, app, version) VALUES ('2026-10-01', 'aaaaaaaa-0000-4000-8000-000000000001', 'jeffdeck', '1')")


    def test_old_rows_get_a_null_country_and_keep_everything_else(self):
        self.old_database()
        main.init()
        with main.db() as con:
            self.assertEqual(con.execute("SELECT DISTINCT country FROM pings").fetchall(), [(None,)])
            self.assertEqual(con.execute("SELECT COUNT(*) FROM pings").fetchone()[0], 3)
            self.assertIn("country", [r[1] for r in con.execute("PRAGMA table_info(pings)")])
        main.init()   # again: nothing changes
        with main.db() as con:
            self.assertEqual(con.execute("SELECT COUNT(*) FROM pings").fetchone()[0], 3)

    def test_a_database_with_apps_but_no_country_gets_the_column(self):
        con = sqlite3.connect(self.path)
        con.executescript(main.PINGS.format("pings").replace(", country TEXT", "") + ";" + main.COUNTS.format("counts") + ";" + main.CRASHES.format("crashes"))
        con.execute("INSERT INTO pings (day, install, app, version) VALUES ('2026-10-01', 'x', 'jeffdeck', '1')")
        con.commit()
        con.close()
        main.init()
        with main.db() as con:
            self.assertEqual(con.execute("SELECT app, version, country FROM pings").fetchall(), [("jeffdeck", "1", None)])


if __name__ == "__main__":
    unittest.main()
