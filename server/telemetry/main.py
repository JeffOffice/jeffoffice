"""JeffOffice's anonymous usage statistics: the collector and its stats page.

JeffPub and JeffDeck each send at most one ping a day (see libs/shell/app/telemetry.cpp),
unless the person using them turned this off (one choice, for the whole suite):

    POST /v1/ping  {"app": "jeffdeck", "install": "<random UUID>", "version": "0.8.0",
                    "os": "windows", "osVersion": "10.0.22631", "lang": "en-US",
                    "launches": 2, "counts": {"file.open.pptx": 3, ...}}

app is the program's file name, "jeffpub" or "jeffdeck"; a ping without it is JeffPub's
(what JeffPub sent before JeffDeck did). The collector keeps one ping a day for each
install and app, with that app's counts, and nothing is added up across apps.

Nothing else is kept: no IP address (none is logged either), no file names,
no text. The install id is made up by the apps and says nothing about the
person or the computer; both apps send the same one.

The same choice also sends crash reports: on the next start after a crash, the
app sends where the program stopped, as file names and numeric positions:

    POST /v1/crash {"app": "jeffdeck", "install": "<the same UUID>", "version": "0.8.0",
                    "os": "windows", "kind": "exception", "code": 3221225477,
                    "frames": [{"module": "JeffDeck.exe", "offset": 6699}, ...],
                    "when": "2026-10-11"}

kind is "signal" (code is the signal number) or "exception" (the Windows
exception code); at most 64 frames; when is the UTC date of the crash, within
the last 30 days; app is optional, as in a ping. Every field is checked and nothing
else is accepted, so no free text is ever stored. 204 means stored; 202 means this
install's app already sent 10 reports today and this one was dropped; 400 or 413
means refused.

    GET /                      what's collected (public)
    GET /stats[?app=jeffpub]   the numbers, for one app (HTTP basic auth, password STATS_PASSWORD)
    GET /crashes[?app=...]     that app's 100 newest crash reports, identical ones grouped (same auth)
    GET /health                "ok"

Without ?app= the pages show JeffPub, as they did before there was another app. An
app has to be named in APPS before the collector takes its pings.

Settings come from the environment: PORT, DATA_DIR (where the SQLite file
goes; a Railway volume), STATS_PASSWORD. Standard library only.
"""
import base64
import datetime as dt
import hmac
import html
import json
import os
import re
import sqlite3
import threading
import urllib.parse
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer

DATA_DIR = os.environ.get("DATA_DIR", os.path.dirname(os.path.abspath(__file__)))
DB_PATH = os.path.join(DATA_DIR, "telemetry.db")
PASSWORD = os.environ.get("STATS_PASSWORD", "")
MAX_BODY = 16 * 1024

UUID = re.compile(r"^[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}$")
VERSION = re.compile(r"^[0-9A-Za-z.+-]{1,20}$")
OS = re.compile(r"^[a-z]{1,16}$")
APP = re.compile(r"[a-z]{1,16}")
OS_VERSION = re.compile(r"^[0-9A-Za-z .()_+-]{0,40}$")
LANG = re.compile(r"^[A-Za-z_-]{0,20}$")
FEATURE = re.compile(r"^[a-z0-9][a-z0-9._-]{0,47}$")
MODULE = re.compile(r"[A-Za-z0-9._+-]{1,64}")
DATE = re.compile(r"[0-9]{4}-[0-9]{2}-[0-9]{2}")
CRASH_FIELDS = {"install", "version", "os", "kind", "code", "frames", "when"}   # and "app", if the sender says
CRASH_KINDS = ("signal", "exception")
MAX_FRAMES = 64
MAX_CRASH_AGE = 30  # days back that a crash's date may be
MAX_CRASHES_PER_DAY = 10  # stored per install and app
APPS = {"jeffpub": "JeffPub", "jeffdeck": "JeffDeck"}  # file name -> name shown; add an app here before it sends
DEFAULT_APP = "jeffpub"  # whose a ping or report is when it doesn't say

_lock = threading.Lock()


def db():
    con = sqlite3.connect(DB_PATH, timeout=10)
    con.execute("PRAGMA journal_mode=WAL")
    return con


PINGS = """CREATE TABLE {} (
    day TEXT NOT NULL, install TEXT NOT NULL, app TEXT NOT NULL DEFAULT 'jeffpub', version TEXT, os TEXT,
    os_version TEXT, lang TEXT, launches INTEGER NOT NULL DEFAULT 0,
    PRIMARY KEY (day, install, app))"""
COUNTS = """CREATE TABLE {} (
    day TEXT NOT NULL, install TEXT NOT NULL, app TEXT NOT NULL DEFAULT 'jeffpub', feature TEXT NOT NULL,
    n INTEGER NOT NULL, PRIMARY KEY (day, install, app, feature))"""
CRASHES = """CREATE TABLE {} (
    id INTEGER PRIMARY KEY AUTOINCREMENT, received TEXT NOT NULL, happened TEXT NOT NULL,
    install TEXT NOT NULL, version TEXT NOT NULL, os TEXT NOT NULL, kind TEXT NOT NULL,
    code INTEGER NOT NULL, frames TEXT NOT NULL, app TEXT NOT NULL DEFAULT 'jeffpub')"""
INDEXES = """
CREATE INDEX IF NOT EXISTS counts_day ON counts (day);
CREATE INDEX IF NOT EXISTS crashes_received ON crashes (received, install);
"""


def init():
    os.makedirs(DATA_DIR, exist_ok=True)
    with db() as con:
        for create, name in ((PINGS, "pings"), (COUNTS, "counts"), (CRASHES, "crashes")):
            con.execute(create.format("IF NOT EXISTS " + name))
    migrate()
    with db() as con:
        con.executescript(INDEXES)


def has_app(con, table):
    return any(column[1] == "app" for column in con.execute("PRAGMA table_info(" + table + ")"))


def migrate():
    """Gives a database from before there were two apps its "app" columns, every row
    JeffPub's, in one transaction: all of it or none. Running it again changes nothing.
    SQLite can't change a primary key in place, so the two tables whose key gains the
    app are rebuilt and the old ones dropped."""
    con = sqlite3.connect(DB_PATH, timeout=10, isolation_level=None)   # no implicit transactions: BEGIN and COMMIT are ours
    try:
        con.execute("BEGIN IMMEDIATE")
        try:
            migrate_pings(con)
            migrate_counts(con)
            migrate_crashes(con)
        except BaseException:
            con.execute("ROLLBACK")
            raise
        con.execute("COMMIT")
    finally:
        con.close()


def migrate_pings(con):
    if has_app(con, "pings"):
        return
    con.execute("DROP TABLE IF EXISTS pings_new")
    con.execute(PINGS.format("pings_new"))
    con.execute("INSERT INTO pings_new (day, install, app, version, os, os_version, lang, launches) "
                "SELECT day, install, ?, version, os, os_version, lang, launches FROM pings", (DEFAULT_APP,))
    con.execute("DROP TABLE pings")
    con.execute("ALTER TABLE pings_new RENAME TO pings")


def migrate_counts(con):
    if has_app(con, "counts"):
        return
    con.execute("DROP TABLE IF EXISTS counts_new")
    con.execute(COUNTS.format("counts_new"))
    con.execute("INSERT INTO counts_new (day, install, app, feature, n) SELECT day, install, ?, feature, n FROM counts", (DEFAULT_APP,))
    con.execute("DROP TABLE counts")   # and its index, made again by init()
    con.execute("ALTER TABLE counts_new RENAME TO counts")


def migrate_crashes(con):
    if not has_app(con, "crashes"):
        con.execute("ALTER TABLE crashes ADD COLUMN app TEXT NOT NULL DEFAULT 'jeffpub'")


def parse_ping(body):
    """The ping's fields, checked; None when anything is off."""
    try:
        p = json.loads(body)
    except (ValueError, RecursionError):   # UnicodeDecodeError is a ValueError
        return None
    if not isinstance(p, dict):
        return None
    install, version, app = p.get("install"), p.get("version"), p.get("app", DEFAULT_APP)
    os_name, os_version, lang = p.get("os", ""), p.get("osVersion", ""), p.get("lang", "")
    launches, counts = p.get("launches", 0), p.get("counts", {})
    if not (isinstance(install, str) and UUID.match(install)):
        return None
    if not (isinstance(version, str) and VERSION.match(version)):
        return None
    if not (isinstance(app, str) and APP.fullmatch(app) and app in APPS):
        return None
    if not (isinstance(os_name, str) and OS.match(os_name)):
        return None
    if not (isinstance(os_version, str) and OS_VERSION.match(os_version)):
        return None
    if not (isinstance(lang, str) and LANG.match(lang)):
        return None
    if not (isinstance(launches, int) and 0 <= launches <= 1000):
        return None
    if not (isinstance(counts, dict) and len(counts) <= 400):
        return None
    for k, v in counts.items():
        if not (FEATURE.match(k) and isinstance(v, int) and 0 <= v <= 100000):
            return None
    return install, version, os_name, os_version, lang, launches, counts, app


def utc_today():
    return dt.datetime.now(dt.timezone.utc).date()


def record(ping, today=None):
    install, version, os_name, os_version, lang, launches, counts, app = ping
    day = (today or utc_today()).isoformat()
    with _lock, db() as con:
        con.execute(
            """INSERT INTO pings (day, install, app, version, os, os_version, lang, launches)
               VALUES (?, ?, ?, ?, ?, ?, ?, ?)
               ON CONFLICT (day, install, app) DO UPDATE SET version = excluded.version,
                 os = excluded.os, os_version = excluded.os_version, lang = excluded.lang,
                 launches = launches + excluded.launches""",
            (day, install, app, version, os_name, os_version, lang, launches),
        )
        con.executemany(
            """INSERT INTO counts (day, install, app, feature, n) VALUES (?, ?, ?, ?, ?)
               ON CONFLICT (day, install, app, feature) DO UPDATE SET n = n + excluded.n""",
            [(day, install, app, k, v) for k, v in counts.items() if v > 0],
        )


def is_int(v):
    return isinstance(v, int) and not isinstance(v, bool)


def parse_crash(body, today=None):
    """A crash report's fields, checked; None when anything is off. The frames come
    back as compact JSON built here, never the sender's own text."""
    try:
        c = json.loads(body)
    except (ValueError, RecursionError):
        return None
    if not (isinstance(c, dict) and CRASH_FIELDS <= set(c) <= CRASH_FIELDS | {"app"}):
        return None
    install, version, os_name, kind = c["install"], c["version"], c["os"], c["kind"]
    code, frames, when, app = c["code"], c["frames"], c["when"], c.get("app", DEFAULT_APP)
    if not (isinstance(install, str) and UUID.fullmatch(install)):
        return None
    if not (isinstance(version, str) and VERSION.fullmatch(version)):
        return None
    if not (isinstance(os_name, str) and OS.fullmatch(os_name)):
        return None
    if not (isinstance(app, str) and APP.fullmatch(app) and app in APPS):
        return None
    if kind not in CRASH_KINDS:
        return None
    if not (is_int(code) and -2 ** 31 <= code < 2 ** 32):
        return None
    if not (isinstance(frames, list) and len(frames) <= MAX_FRAMES):
        return None
    clean = []
    for f in frames:
        if not (isinstance(f, dict) and set(f) == {"module", "offset"}):
            return None
        module, offset = f["module"], f["offset"]
        if not (isinstance(module, str) and MODULE.fullmatch(module)):
            return None
        if not (is_int(offset) and 0 <= offset < 2 ** 48):
            return None
        clean.append({"module": module, "offset": offset})
    if not (isinstance(when, str) and DATE.fullmatch(when)):
        return None
    try:
        day = dt.date.fromisoformat(when)
    except ValueError:
        return None
    if not 0 <= ((today or utc_today()) - day).days <= MAX_CRASH_AGE:
        return None
    return install, version, os_name, kind, code, json.dumps(clean, separators=(",", ":")), day.isoformat(), app


def record_crash(crash, today=None):
    """Store a checked report; False (and nothing stored) when the install has
    already sent its share for the day from that app."""
    install, version, os_name, kind, code, frames, happened, app = crash
    received = (today or utc_today()).isoformat()
    with _lock, db() as con:
        n = con.execute("SELECT COUNT(*) FROM crashes WHERE received = ? AND install = ? AND app = ?",
                        (received, install, app)).fetchone()[0]
        if n >= MAX_CRASHES_PER_DAY:
            return False
        con.execute(
            """INSERT INTO crashes (received, happened, install, version, os, kind, code, frames, app)
               VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)""",
            (received, happened, install, version, os_name, kind, code, frames, app),
        )
    return True


ABOUT = """<!doctype html><meta charset="utf-8"><title>JeffOffice usage statistics</title>
<style>body{font:16px/1.5 system-ui,sans-serif;max-width:40em;margin:2em auto;padding:0 1em}</style>
<h1>JeffOffice usage statistics</h1>
<p>JeffPub and JeffDeck share one choice about this. Unless you turn it off (in setup, when JeffPub or JeffDeck
first starts, or under File &gt; Options in either), each of them sends the following once a day while it's in
use, and once more the day it's updated:</p>
<ul><li>a random number made up when JeffOffice was installed, which says nothing about you or your computer
(JeffPub and JeffDeck send the same number, so someone who uses both is one person here);</li>
<li>which app it comes from, JeffPub or JeffDeck, its version, your operating system and its version,
and the language it's set to;</li>
<li>how many times it was started, and how many times each of its commands was used
(for example "opened a .pub file" in JeffPub, "opened a .pptx file" in JeffDeck, or "exported a PDF").</li></ul>
<p>Never your files, their names, or anything in them. Your IP address isn't kept.
The numbers show how many people use JeffPub and JeffDeck and which parts matter most to them.</p>
<h2>Crash reports</h2>
<p>If usage statistics are on and JeffPub or JeffDeck crashes, it sends a crash report the next time it starts.
The report says where in the program it stopped (the names of program files, and numbers that give
a position in each), what kind of failure it was (a number), the day it happened, which app it was and its
version, your operating system, and the same random number as above. It never includes your documents, their
names or contents, anything you typed, or anything that was in your computer's memory. If you turn
usage statistics off, no crash reports are sent either.</p>
<p>The program that collects all of this is part of JeffOffice's source:
<a href="https://github.com/JeffOffice/jeffoffice/tree/main/server/telemetry">server/telemetry</a>.</p>
"""


STYLE = """body{font:15px/1.45 system-ui,sans-serif;max-width:56em;margin:1.5em auto;padding:0 1em;color:#1E2430}
table{border-collapse:collapse;margin-bottom:1em}td,th{padding:3px 10px;border-bottom:1px solid #dde;text-align:left}
.apps a{display:inline-block;padding:.2em .8em;margin-right:.4em;border-radius:6px;background:#F2F5F9;color:#1F5FAD;text-decoration:none}
.apps a[aria-current]{background:#1F5FAD;color:#fff}
"""


def app_choice(page, app):
    """The links that pick which app a page shows."""
    current = ' aria-current="page"'
    links = "".join(f'<a href="/{page}?app={a}"{current if a == app else ""}>{html.escape(name)}</a>' for a, name in APPS.items())
    return f'<p class="apps">{links}</p>'


def stats_page(app=DEFAULT_APP):
    today = dt.datetime.now(dt.timezone.utc).date()
    d30 = (today - dt.timedelta(days=29)).isoformat()
    d60 = (today - dt.timedelta(days=59)).isoformat()
    d7 = (today - dt.timedelta(days=6)).isoformat()
    with db() as con:
        q = lambda sql, *a: con.execute(sql, a).fetchall()
        one = lambda sql, *a: con.execute(sql, a).fetchone()[0]
        periods = (("Today", today.isoformat(), "="), ("Last 7 days", d7, ">="), ("Last 30 days", d30, ">="), ("Ever", "", ">="))
        # Installs that sent a ping in each period, for each app and for either.
        overview = [(name, *(one("SELECT COUNT(DISTINCT install) FROM pings WHERE app = ? AND day " + op + " ?", a, since)
                             for _, since, op in periods)) for a, name in APPS.items()]
        overview.append(("Either app", *(one("SELECT COUNT(DISTINCT install) FROM pings WHERE day " + op + " ?", since)
                                         for _, since, op in periods)))
        totals = [(label, one("SELECT COUNT(DISTINCT install) FROM pings WHERE app = ? AND day " + op + " ?", app, since))
                  for label, since, op in periods]
        daily = dict(q("SELECT day, COUNT(*) FROM pings WHERE app = ? AND day >= ? GROUP BY day", app, d60))
        firsts = dict(q("SELECT first, COUNT(*) FROM (SELECT install, MIN(day) AS first FROM pings WHERE app = ? GROUP BY install) "
                        "WHERE first >= ? GROUP BY first", app, d60))
        newest = "(SELECT install, MAX(day) AS d FROM pings WHERE app = ? AND day >= ? GROUP BY install) l " \
                 "ON p.install = l.install AND p.day = l.d AND p.app = ?"
        latest = "SELECT p.{0}, COUNT(*) FROM pings p JOIN " + newest + " GROUP BY p.{0} ORDER BY 2 DESC"
        versions = q(latest.format("version"), app, d30, app)
        systems = q(latest.format("os"), app, d30, app)
        os_versions = q("SELECT p.os || ' ' || p.os_version, COUNT(*) FROM pings p JOIN " + newest + " GROUP BY 1 ORDER BY 2 DESC LIMIT 15",
                        app, d30, app)
        langs = q(latest.format("lang"), app, d30, app)
        features = q("SELECT feature, SUM(n), COUNT(DISTINCT install) FROM counts WHERE app = ? AND day >= ? GROUP BY feature "
                     "ORDER BY 3 DESC, 2 DESC LIMIT 100", app, d30)
        crashes = q("SELECT version, COUNT(*), COUNT(DISTINCT install) FROM crashes WHERE app = ? AND received >= ? "
                    "GROUP BY version ORDER BY 2 DESC, 1", app, d30)
    e = html.escape
    name = APPS[app]

    def table(title, head, rows):
        body = "".join("<tr>" + "".join(f"<td>{e(str(c))}</td>" for c in r) + "</tr>" for r in rows)
        return f"<h2>{e(title)}</h2><table><tr>{''.join(f'<th>{e(h)}</th>' for h in head)}</tr>{body or '<tr><td>none yet</td></tr>'}</table>"

    days = [(today - dt.timedelta(days=i)).isoformat() for i in range(59, -1, -1)]
    peak = max([1] + list(daily.values()))
    w = 10
    bars = "".join(
        f'<rect x="{i * w}" y="{120 - 110 * daily.get(d, 0) / peak:.1f}" width="{w - 2}" height="{110 * daily.get(d, 0) / peak:.1f}" fill="#1F5FAD">'
        f"<title>{d}: {daily.get(d, 0)} active, {firsts.get(d, 0)} new</title></rect>"
        for i, d in enumerate(days))
    chart = f'<svg viewBox="0 0 {w * 60} 125" width="100%" role="img" aria-label="Active installs per day">{bars}</svg>'
    return f"""<!doctype html><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>{e(name)} usage</title>
<style>{STYLE}td:nth-child(n+2){{text-align:right}}.cards{{display:flex;gap:1em;flex-wrap:wrap}}
.card{{background:#F2F5F9;border-radius:8px;padding:.6em 1em}}.card b{{display:block;font-size:1.6em}}</style>
<h1>{e(name)} usage</h1>
{app_choice("stats", app)}
<p>Installs that sent statistics (each counted once per period). Times are UTC. Each app is counted by itself; the last row
counts an install that uses both once.</p>
{table("All apps", ["App", *[label for label, _, _ in periods]], overview)}
<h2>{e(name)}</h2>
<div class="cards">{''.join(f'<div class="card"><b>{n}</b>{e(label)}</div>' for label, n in totals)}</div>
<h2>Active installs, last 60 days</h2>{chart}
{table("Version (last 30 days)", ["Version", "Installs"], versions)}
{table("Operating system (last 30 days)", ["System", "Installs"], systems)}
{table("System versions (last 30 days)", ["System", "Installs"], os_versions)}
{table("Language (last 30 days)", ["Language", "Installs"], langs)}
{table("Commands used (last 30 days)", ["Command", "Times", "Installs"], features)}
{table("Crashes by version (last 30 days)", ["Version", "Reports", "Installs"], crashes)}
<p><a href="/crashes?app={app}">The newest {e(name)} crash reports</a></p>
"""


def crashes_page(app=DEFAULT_APP):
    """The app's 100 newest reports; the ones that stopped in the same place share a row."""
    with db() as con:
        rows = con.execute("SELECT happened, version, os, kind, code, frames FROM crashes WHERE app = ? "
                           "ORDER BY id DESC LIMIT 100", (app,)).fetchall()
    groups = {}
    for happened, version, os_name, kind, code, frames in rows:
        g = groups.setdefault((kind, code, frames), {"n": 0, "latest": happened, "versions": [], "systems": []})
        g["n"] += 1
        g["latest"] = max(g["latest"], happened)
        for key, value in (("versions", version), ("systems", os_name)):
            if value not in g[key]:
                g[key].append(value)
    e = html.escape
    name = APPS[app]

    def row(key, g):
        kind, code, frames = key
        shown = f"0x{code & 0xFFFFFFFF:08X}" if kind == "exception" else str(code)
        stack = "<br>".join(e(f"{f['module']}+0x{f['offset']:x}") for f in json.loads(frames))
        cells = [g["n"], g["latest"], ", ".join(g["versions"]), ", ".join(g["systems"]), kind, shown]
        where = f"<code>{stack}</code>" if stack else "<i>no frames</i>"
        return "<tr>" + "".join(f"<td>{e(str(c))}</td>" for c in cells) + f"<td>{where}</td></tr>"

    body = "".join(row(k, g) for k, g in groups.items()) or '<tr><td colspan="7">none yet</td></tr>'
    head = "".join(f"<th>{h}</th>" for h in ("Reports", "Latest", "Version", "System", "Kind", "Code", "Where it stopped"))
    return f"""<!doctype html><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>{e(name)} crashes</title>
<style>{STYLE}td{{vertical-align:top}}code{{font-size:.9em}}</style>
<h1>{e(name)} crashes</h1>
{app_choice("crashes", app)}
<p>The newest 100 crash reports, with the ones that stopped in the same place on one row.
Dates are UTC. <a href="/stats?app={app}">Back to the numbers</a></p>
<table><tr>{head}</tr>{body}</table>
"""


class Handler(BaseHTTPRequestHandler):
    server_version = "jeffpub-telemetry"
    sys_version = ""

    def log_message(self, fmt, *args):
        # No client address: only the request line and status.
        print(f"{self.command} {self.path.split('?')[0]} {args[1] if len(args) > 1 else ''}", flush=True)

    def send(self, code, body=b"", ctype="text/plain; charset=utf-8", extra=()):
        self.send_response(code)
        for k, v in extra:
            self.send_header(k, v)
        if body:
            self.send_header("Content-Type", ctype)
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        if body and self.command != "HEAD":
            self.wfile.write(body)

    def authorized(self):
        if not PASSWORD:
            return False
        h = self.headers.get("Authorization", "")
        if not h.startswith("Basic "):
            return False
        try:
            user_pass = base64.b64decode(h[6:]).decode("utf-8")
        except (ValueError, UnicodeDecodeError):
            return False
        return hmac.compare_digest(user_pass.partition(":")[2].encode(), PASSWORD.encode())

    def do_GET(self):
        url = urllib.parse.urlsplit(self.path)
        path = url.path
        if path == "/health":
            self.send(200, b"ok")
        elif path == "/":
            self.send(200, ABOUT.encode(), "text/html; charset=utf-8")
        elif path in ("/stats", "/crashes"):
            if not self.authorized():
                self.send(401, b"Password needed", extra=[("WWW-Authenticate", 'Basic realm="JeffPub usage"')])
                return
            # Which app: one ?app= naming one the collector knows, else JeffPub's.
            asked = urllib.parse.parse_qs(url.query, keep_blank_values=True).get("app", [DEFAULT_APP])
            if len(asked) != 1 or asked[0] not in APPS:
                self.send(404, b"Not found")
                return
            page = stats_page(asked[0]) if path == "/stats" else crashes_page(asked[0])
            self.send(200, page.encode(), "text/html; charset=utf-8", extra=[("Cache-Control", "no-store")])
        else:
            self.send(404, b"Not found")

    do_HEAD = do_GET

    def do_POST(self):
        path = self.path.split("?")[0]
        if path not in ("/v1/ping", "/v1/crash"):
            self.send(404, b"Not found")
            return
        try:
            length = int(self.headers.get("Content-Length", "0"))
        except ValueError:
            length = -1
        if length <= 0 or length > MAX_BODY:
            self.send(413 if length > MAX_BODY else 400, b"Bad request")
            return
        body = self.rfile.read(length)
        if path == "/v1/crash":
            crash = parse_crash(body)
            if crash is None:
                self.send(400, b"Bad request")
                return
            self.send(204 if record_crash(crash) else 202)
            return
        ping = parse_ping(body)
        if ping is None:
            self.send(400, b"Bad request")
            return
        record(ping)
        self.send(204)


def main():
    init()
    port = int(os.environ.get("PORT", "8080"))
    server = ThreadingHTTPServer(("0.0.0.0", port), Handler)
    server.daemon_threads = True
    print(f"listening on {port}", flush=True)
    server.serve_forever()


if __name__ == "__main__":
    main()
