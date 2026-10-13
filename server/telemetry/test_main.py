"""Tests for the usage statistics collector: python3 -m unittest test_main"""
import base64
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
        for bad in ["", "JeffDeck", "jeffdeck1", "jeff-deck", "jeff deck", "jeffdeck\n", "jeffwrite", "x", "a" * 17,
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

    def test_public_page_names_both_apps(self):
        code, page = self.get("/")
        self.assertEqual(code, 200)
        text = " ".join(page.split())
        self.assertIn("<title>JeffOffice usage statistics</title>", text)
        self.assertEqual(page.count("JeffDeck") > 3, True)
        self.assertIn("JeffPub and JeffDeck", text)
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
        for bad in ["", "JeffDeck", "jeffdeck1", "jeff deck", "jeffdeck\n", "jeffwrite", "x", "a" * 17, "../jeffpub", 5, None, True, ["jeffdeck"]]:
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
        self.assertIn("<td>9.9.9</td><td>4</td><td>3</td>", page)
        self.assertIn("<td>9.9.8</td><td>1</td><td>1</td>", page)
        self.assertNotIn("8.8.8", page)
        self.assertIn('href="/crashes?app=jeffpub"', page)

    def test_stats_are_split_by_app(self):
        self.post(self.ping(app="jeffpub", version="7.7.1", counts={"pub.only": 2}, lang="de-DE"))
        self.post(self.ping(app="jeffdeck", version="8.8.1", counts={"deck.only": 3, "file.open.pptx": 1}, os="macos", lang="fr-FR"))
        code, pub = self.get("/stats", "secret")                     # JeffPub's, as the page always was
        self.assertEqual(code, 200)
        _, pub_too = self.get("/stats?app=jeffpub", "secret")
        code, deck = self.get("/stats?app=jeffdeck", "secret")
        self.assertEqual(code, 200)
        self.assertEqual(pub, pub_too)
        for shown, hidden in ((pub, ("8.8.1", "deck.only", "fr-FR", "file.open.pptx")), (deck, ("7.7.1", "pub.only", "de-DE", "export.pdf"))):
            for text in hidden:
                self.assertNotIn(text, shown)
        for text in ("7.7.1", "pub.only", "de-DE"):
            self.assertIn(text, pub)
        for text in ("8.8.1", "deck.only", "fr-FR", "file.open.pptx", "macos"):
            self.assertIn(text, deck)
        for page in (pub, deck):
            self.assertIn('href="/stats?app=jeffpub"', page)
            self.assertIn('href="/stats?app=jeffdeck"', page)
        self.assertIn("<h1>JeffPub usage</h1>", pub)
        self.assertIn("<h1>JeffDeck usage</h1>", deck)
        self.assertIn('href="/crashes?app=jeffdeck"', deck)
        self.assertIn('href="/crashes?app=jeffpub"', pub)

    def test_stats_overview_counts_each_app_and_the_suite(self):
        a, b, c = (str(uuid.uuid4()) for _ in range(3))
        with main.db() as con:
            con.execute("DELETE FROM pings WHERE install IN (?, ?, ?)", (a, b, c))
        for install, app in ((a, "jeffpub"), (a, "jeffdeck"), (b, "jeffdeck"), (c, "jeffpub")):
            self.post(self.ping(install=install, app=app))
        with main.db() as con:
            ever = {app: con.execute("SELECT COUNT(DISTINCT install) FROM pings WHERE app = ?", (app,)).fetchone()[0] for app in main.APPS}
            suite = con.execute("SELECT COUNT(DISTINCT install) FROM pings").fetchone()[0]
        _, page = self.get("/stats?app=jeffdeck", "secret")
        self.assertIn("Installs that sent statistics", page)
        for app, name in (("jeffpub", "JeffPub"), ("jeffdeck", "JeffDeck")):
            self.assertRegex(page, r"<tr><td>%s</td>(<td>\d+</td>){3}<td>%d</td></tr>" % (name, ever[app]))
        self.assertRegex(page, r"<tr><td>Either app</td>(<td>\d+</td>){3}<td>%d</td></tr>" % suite)
        self.assertGreater(suite, 0)
        self.assertLess(suite, sum(ever.values()))   # a uses both apps: counted once in the suite row

    def test_an_app_that_is_not_known_has_no_page(self):
        for path in ("/stats?app=jeffwrite", "/stats?app=", "/stats?app=JeffDeck", "/stats?app=jeffdeck&app=x", "/crashes?app=jeffwrite", "/crashes?app=%00"):
            with self.subTest(path=path):
                self.assertEqual(self.get(path, "secret")[0], 404)
        self.assertEqual(self.get("/stats?app=jeffdeck")[0], 401)   # the password still comes first
        self.assertEqual(self.get("/stats?other=1", "secret")[0], 200)

    def test_crashes_are_split_by_app(self):
        self.send(app="jeffpub", frames=[{"module": "JeffPub.exe", "offset": 0x1A2B}])
        self.send(app="jeffdeck", frames=[{"module": "JeffDeck.exe", "offset": 0x3C4D}], version="8.8.2")
        self.send(frames=[{"module": "JeffPub.exe", "offset": 0x1A2B}])   # no app: JeffPub's
        _, pub = self.get("/crashes", "secret")
        _, deck = self.get("/crashes?app=jeffdeck", "secret")
        self.assertIn("JeffPub.exe+0x1a2b", pub)
        self.assertNotIn("JeffDeck.exe", pub)
        self.assertIn("<td>2</td>", pub)                               # the two JeffPub reports share a row
        self.assertIn("JeffDeck.exe+0x3c4d", deck)
        self.assertNotIn("JeffPub.exe", deck)
        self.assertIn("<h1>JeffDeck crashes</h1>", deck)
        for page in (pub, deck):
            self.assertIn('href="/crashes?app=jeffpub"', page)
            self.assertIn('href="/crashes?app=jeffdeck"', page)

    def test_stats_count_each_apps_crashes_apart(self):
        self.send(app="jeffdeck", version="6.6.6")
        self.send(app="jeffdeck", version="6.6.6")
        self.send(app="jeffpub", version="6.6.5")
        _, deck = self.get("/stats?app=jeffdeck", "secret")
        _, pub = self.get("/stats", "secret")
        self.assertIn("<td>6.6.6</td><td>2</td><td>2</td>", deck)
        self.assertNotIn("6.6.5", deck)
        self.assertIn("<td>6.6.5</td><td>1</td><td>1</td>", pub)
        self.assertNotIn("6.6.6", pub)

    def test_stats_say_so_when_nothing_crashed(self):
        code, page = self.get("/stats", "secret")
        self.assertEqual(code, 200)
        self.assertIn("Crashes", page)

    def test_crash_path_takes_only_post(self):
        self.assertEqual(self.get("/v1/crash")[0], 404)
        self.assertEqual(self.post(self.crash(), "/v1/crashes"), 404)


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


if __name__ == "__main__":
    unittest.main()
