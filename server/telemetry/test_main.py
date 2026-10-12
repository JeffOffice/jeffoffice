"""Tests for the usage statistics collector: python3 -m unittest test_main"""
import base64
import datetime as dt
import json
import os
import tempfile
import threading
import unittest
import urllib.error
import urllib.request
import uuid

os.environ["DATA_DIR"] = tempfile.mkdtemp()
os.environ["STATS_PASSWORD"] = "secret"
import main  # noqa: E402  (reads the environment above)

INSTALL = "0f8fad5b-d9cb-469f-a165-70867728950e"


def utc_today():
    return dt.datetime.now(dt.timezone.utc).date()


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

    def test_bad_pings_are_refused(self):
        self.assertEqual(self.post(self.ping(install="not-a-uuid")), 400)
        self.assertEqual(self.post(self.ping(counts={"C:\\Users\\jeff\\letter.pub": 1})), 400)
        self.assertEqual(self.post(self.ping(counts={"x": -1})), 400)
        self.assertEqual(self.post(self.ping(lang="<script>")), 400)
        self.assertEqual(self.post(b"{not json"), 400)
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

    def test_signals_and_signed_windows_codes_are_accepted(self):
        self.assertEqual(self.send(kind="signal", code=11, os="linux", frames=[]), 204)
        self.assertEqual(self.send(code=-1073741819), 204)
        self.assertEqual(self.send(code=0), 204)
        self.assertEqual([r[6] for r in self.rows()], [11, -1073741819, 0])

    def test_the_table_holds_no_free_text(self):
        with main.db() as con:
            columns = [r[1] for r in con.execute("PRAGMA table_info(crashes)")]
        self.assertEqual(columns, ["id", "received", "happened", "install", "version", "os", "kind", "code", "frames"])

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
        self.assertIn('href="/crashes"', page)

    def test_stats_say_so_when_nothing_crashed(self):
        code, page = self.get("/stats", "secret")
        self.assertEqual(code, 200)
        self.assertIn("Crashes", page)

    def test_crash_path_takes_only_post(self):
        self.assertEqual(self.get("/v1/crash")[0], 404)
        self.assertEqual(self.post(self.crash(), "/v1/crashes"), 404)


if __name__ == "__main__":
    unittest.main()
