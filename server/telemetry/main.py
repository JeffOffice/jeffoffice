"""JeffOffice's anonymous usage statistics: the collector and its stats page.

Each JeffOffice app sends at most one ping a day (see libs/shell/app/telemetry.cpp),
unless the person using them turned this off (one choice, for the whole suite):

    POST /v1/ping  {"app": "jeffdeck", "install": "<random UUID>", "version": "0.8.0",
                    "os": "windows", "osVersion": "10.0.22631", "lang": "en-US",
                    "launches": 2, "counts": {"file.open.pptx": 3, ...}}

app is the program's file name, "jeffpub", "jeffdeck", "jeffwrite", or "jeffcrunch"; a ping without it is JeffPub's
(what JeffPub sent before JeffDeck did). The collector keeps one ping a day for each
install and app, with that app's counts, and nothing is added up across apps.

The collector also works out which country a ping came from. When the ping arrives, it looks the
connection's address up in an offline database (DB-IP's "IP to Country Lite", fetched into DATA_DIR/geo
and refreshed monthly), keeps only the two-letter country code, and drops the address on the spot: the
address is never stored, never logged, and never sent anywhere. With no database, the country is empty.

Nothing else is kept: no IP address, no file names,
no text. The install id is made up by the apps and says nothing about the
person or the computer; every app sends the same one.

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
    GET /stats[?days=7|30|90]  the dashboard: every app side by side (HTTP basic auth, password STATS_PASSWORD)
    GET /crashes[?app=...]     the 100 newest crash reports, identical ones grouped (same auth)
    GET /health                "ok"

Without ?app= the pages cover all the apps. An app has to be named in APPS before the collector takes its pings.

    The stats page also has a Downloads section, filled by download_daily() once a day from GitHub's release
    counts, Cloudflare's analytics for apt.jeffoffice.com (env CF_ANALYTICS_TOKEN, CF_ZONE_ID), and the Snap Store's
    metrics (env SNAP_METRICS_CREDENTIALS, SNAP_ID). Those are public counters; they hold nothing about a person.

Everything is kept for 24 months (KEEP_MONTHS), then deleted: purge() runs at start and once a day.
`python main.py --forget <install id>` deletes one install's pings, counts, and crash reports, for a
person who asks (the privacy notice at https://www.jeffoffice.com/privacy says how).
Settings come from the environment: PORT, DATA_DIR (where the SQLite file
goes; a Railway volume), STATS_PASSWORD. Standard library only.
"""
import base64
import bisect
import datetime as dt
import gzip
import hmac
import html
import ipaddress
import json
import os
import re
import socket
import sqlite3
import sys
import threading
import time
import urllib.parse
import urllib.request
from array import array
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
APPS = {"jeffpub": "JeffPub", "jeffdeck": "JeffDeck", "jeffwrite": "JeffWrite", "jeffcrunch": "JeffCrunch"}  # file name -> name shown; add an app here before it sends
DEFAULT_APP = "jeffpub"  # whose a ping or report is when it doesn't say

_lock = threading.Lock()


def db():
    con = sqlite3.connect(DB_PATH, timeout=10)
    con.execute("PRAGMA journal_mode=WAL")
    return con


PINGS = """CREATE TABLE {} (
    day TEXT NOT NULL, install TEXT NOT NULL, app TEXT NOT NULL DEFAULT 'jeffpub', version TEXT, os TEXT,
    os_version TEXT, lang TEXT, launches INTEGER NOT NULL DEFAULT 0, country TEXT,
    PRIMARY KEY (day, install, app))"""
COUNTS = """CREATE TABLE {} (
    day TEXT NOT NULL, install TEXT NOT NULL, app TEXT NOT NULL DEFAULT 'jeffpub', feature TEXT NOT NULL,
    n INTEGER NOT NULL, PRIMARY KEY (day, install, app, feature))"""
CRASHES = """CREATE TABLE {} (
    id INTEGER PRIMARY KEY AUTOINCREMENT, received TEXT NOT NULL, happened TEXT NOT NULL,
    install TEXT NOT NULL, version TEXT NOT NULL, os TEXT NOT NULL, kind TEXT NOT NULL,
    code INTEGER NOT NULL, frames TEXT NOT NULL, app TEXT NOT NULL DEFAULT 'jeffpub')"""
DOWNLOADS = """CREATE TABLE {} (
    day TEXT NOT NULL, source TEXT NOT NULL, item TEXT NOT NULL, n INTEGER NOT NULL, PRIMARY KEY (day, source, item))"""
INDEXES = """
CREATE INDEX IF NOT EXISTS counts_day ON counts (day);
CREATE INDEX IF NOT EXISTS crashes_received ON crashes (received, install);
"""


def init():
    os.makedirs(DATA_DIR, exist_ok=True)
    with db() as con:
        for create, name in ((PINGS, "pings"), (COUNTS, "counts"), (CRASHES, "crashes"), (DOWNLOADS, "downloads")):
            con.execute(create.format("IF NOT EXISTS " + name))
    migrate()
    with db() as con:
        con.executescript(INDEXES)


def has_column(con, table, column):
    return any(c[1] == column for c in con.execute("PRAGMA table_info(" + table + ")"))


def has_app(con, table):
    return has_column(con, table, "app")


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
            if not has_column(con, "pings", "country"):
                con.execute("ALTER TABLE pings ADD COLUMN country TEXT")   # old rows stay NULL
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


# ---------------------------------------------------------------- country from the connection's address
# The address is looked up in an offline database the moment a ping arrives; only the two-letter country code
# goes on, and the address itself is never stored, logged, or passed on.

GEO_DIR = os.path.join(DATA_DIR, "geo")
GEO_URL = "https://download.db-ip.com/free/dbip-country-lite-{:04d}-{:02d}.csv.gz"   # DB-IP "IP to Country Lite", CC BY 4.0
GEO_NAME = re.compile(r"^dbip-country-lite-\d{4}-\d{2}\.csv\.gz$")
GEO_MAX_AGE = 7 * 24 * 3600    # seconds before the file is fetched again: weekly (DB-IP publishes monthly, so a new month shows up within a week)
GEO_STALE_MONTHS = 2           # the stats page flags a list older than this
GEO_MIN_ROWS = 50000           # a download with fewer rows than this is refused
GEO_MAX_BYTES = 64 * 1024 * 1024
_geo = None                    # the loaded Geo, or None: then every country is NULL
_geo_path = None               # the file _geo came from


class Geo:
    """Sorted ranges in compact arrays. IPv4 is looked up whole; IPv6 by its first 64 bits (the
    ranges are never finer than that in practice, and it halves the memory)."""

    def __init__(self, lines):
        v4, v6 = (array("I"), array("I"), array("H")), (array("Q"), array("Q"), array("H"))
        for line in lines:
            parts = line.strip().split(",")
            if len(parts) < 3:
                continue
            first, last, code = (p.strip().strip('"') for p in parts[:3])
            if not (len(code) == 2 and code.isalpha() and code.isupper()) or code == "ZZ":
                continue
            try:
                if ":" in first:
                    lo = int.from_bytes(socket.inet_pton(socket.AF_INET6, first)[:8], "big")
                    hi = int.from_bytes(socket.inet_pton(socket.AF_INET6, last)[:8], "big")
                    target = v6
                else:
                    lo, hi = int.from_bytes(socket.inet_aton(first), "big"), int.from_bytes(socket.inet_aton(last), "big")
                    target = v4
            except (OSError, ValueError, OverflowError):
                continue
            target[0].append(lo)   # straight into arrays: the file is sorted, so there's no big temporary list
            target[1].append(hi)
            target[2].append(ord(code[0]) << 8 | ord(code[1]))
        self.v4, self.v6 = self.sorted(v4), self.sorted(v6)
        self.rows = len(v4[0]) + len(v6[0])

    @staticmethod
    def sorted(cols):
        """The three arrays, in order of start; they only get rebuilt if the file wasn't sorted."""
        starts = cols[0]
        if all(starts[i] <= starts[i + 1] for i in range(len(starts) - 1)):
            return cols
        order = sorted(range(len(starts)), key=starts.__getitem__)
        return tuple(array(c.typecode, (c[i] for i in order)) for c in cols)

    def lookup(self, ip):
        """Two-letter code for an ipaddress object, or None."""
        starts, ends, codes = self.v4 if ip.version == 4 else self.v6
        n = int(ip) if ip.version == 4 else int(ip) >> 64
        i = bisect.bisect_right(starts, n) - 1
        if i >= 0 and n <= ends[i]:
            return chr(codes[i] >> 8) + chr(codes[i] & 255)
        return None


def load_geo(path):
    """A Geo from a DB-IP csv.gz file; raises when the file isn't readable gzip."""
    with gzip.open(path, "rt", encoding="utf-8", errors="replace") as f:
        return Geo(f)


def client_ip(headers, peer):
    """The visitor's address as an ipaddress object, or None when it isn't a public one. Railway's edge
    proxy puts the visitor's address in X-Real-IP (docs.railway.com/networking/public-networking/specs-and-limits);
    without that header (or with junk in it), the socket's peer is used. X-Forwarded-For isn't read: a visitor can write it."""
    for text in ((headers.get("X-Real-IP") or "").strip(), peer):
        try:
            ip = ipaddress.ip_address(text)
        except ValueError:
            continue
        if ip.version == 6 and ip.ipv4_mapped:
            ip = ip.ipv4_mapped
        return ip if ip.is_global else None
    return None


def country_of(headers, peer):
    """The two-letter country for this request, or None (no public address, or no database loaded)."""
    geo, ip = _geo, client_ip(headers, peer)
    return geo.lookup(ip) if geo is not None and ip is not None else None


def geo_files():
    try:
        return sorted(os.path.join(GEO_DIR, n) for n in os.listdir(GEO_DIR) if GEO_NAME.match(n))
    except OSError:
        return []


def geo_download(today=None):
    """Fetch this month's file (else last month's), check it loads and is big enough; returns (path, Geo) or None."""
    today = today or utc_today()
    os.makedirs(GEO_DIR, exist_ok=True)
    year, month = today.year, today.month
    for _ in range(2):
        url = GEO_URL.format(year, month)
        dest = os.path.join(GEO_DIR, url.rsplit("/", 1)[1])
        tmp = dest + ".part"
        try:
            req = urllib.request.Request(url, headers={"User-Agent": "JeffOffice-telemetry/1.0 (+https://telemetry.jeffoffice.com)"})
            with urllib.request.urlopen(req, timeout=60) as r, open(tmp, "wb") as f:
                size = 0
                while chunk := r.read(1 << 16):
                    size += len(chunk)
                    if size > GEO_MAX_BYTES:
                        raise ValueError("too big")
                    f.write(chunk)
            geo = load_geo(tmp)
            if geo.rows < GEO_MIN_ROWS:
                raise ValueError("too few rows")
            os.replace(tmp, dest)
            return dest, geo
        except Exception as e:
            print(f"geo database {year:04d}-{month:02d} not used: {type(e).__name__}", flush=True)
            try:
                os.remove(tmp)
            except OSError:
                pass
        year, month = (year, month - 1) if month > 1 else (year - 1, 12)
    return None


def ensure_geo(today=None):
    """Load the newest file on disk if nothing is loaded, and fetch a newer one when it's missing or old.
    The previous file stays until the new one has loaded."""
    global _geo, _geo_path
    files = geo_files()
    if _geo is None:
        for path in reversed(files):
            try:
                _geo = load_geo(path)
                _geo_path = path
                break
            except Exception as e:
                print(f"geo file unreadable: {type(e).__name__}", flush=True)
    if not files or time.time() - os.path.getmtime(files[-1]) > GEO_MAX_AGE:
        got = geo_download(today)
        if got:
            _geo, _geo_path = got[1], got[0]
            for old in geo_files():
                if old != got[0]:
                    os.remove(old)


def geo_daily():
    while True:
        try:
            ensure_geo()
            print(f"country database: {_geo.rows if _geo else 0} ranges", flush=True)
        except Exception as e:   # no database just means no countries; try again tomorrow
            print(f"country database failed: {type(e).__name__}", flush=True)
        time.sleep(24 * 3600)


# ---------------------------------------------------------------- download counts (GitHub, apt, Snap Store)
# Once a day (download_daily) each source is asked for its counts and the totals go in the downloads table:
#   downloads(day, source, item, n), one row per day, source, and item. Items, by source:
#     github  "release:<tag>:<platform>"  the CUMULATIVE count GitHub shows for that release's installers
#             (platform: windows, macos, linux), as of day; downloads in a period = differences between snapshots.
#     apt     "kind:deb" / "kind:update-check"  requests that day answered 200, 206, or 304 for a .deb or for
#             dists/*/(In)Release; "version:<v>" .deb requests by version; "country:<CC>" update checks by country.
#     snap    "kind:total", "version:<v>", "country:<CC>"  the weekly installed base in the bucket dated day.
# The HTTP call is a parameter (http(url, headers=None, body=None) -> parsed JSON) so tests run offline.
# A fetcher returns the number of rows it stored, or None when its credentials aren't set; errors propagate
# to run_download_fetchers(), which logs the exception's type and nothing else (never a URL or a secret).

HTTP_UA = "JeffOffice-telemetry/1.0 (+https://telemetry.jeffoffice.com)"
GITHUB_RELEASES = "https://api.github.com/repos/JeffOffice/jeffoffice/releases?per_page=100"
CF_GRAPHQL = "https://api.cloudflare.com/client/v4/graphql"
CF_DEFAULT_ZONE = "1ca02c63d10f2fe1b656aa2bdb7e1196"
APT_HOST = "apt.jeffoffice.com"
APT_BACKFILL_DAYS = 7          # earlier days fetched when missing; Cloudflare keeps adaptive data at least 31 days on every plan
APT_OK_STATUS = (200, 206, 304)
SNAP_NAME = "jeffoffice"
SNAP_API = "https://dashboard.snapcraft.io/dev/api"
SNAP_METRICS = ("weekly_installed_base_by_version", "weekly_installed_base_by_country")
SNAP_WINDOW_DAYS = 14
DOWNLOAD_START_DELAY = 60      # seconds after start before the first fetch

PLATFORMS = (("windows", "Windows installer"), ("macos", "Mac disk image"), ("linux", "Linux .deb, AppImage"))   # all from GitHub
ASSET_PLATFORM = ((re.compile(r"^JeffOffice-Setup.*\.exe$"), "windows"), (re.compile(r"\.dmg$"), "macos"),
                  (re.compile(r"\.(deb|AppImage)$"), "linux"))
DEB_VERSION = re.compile(r"_([0-9][0-9A-Za-z.+~-]*?)_[a-z0-9]+\.deb$")
APT_RELEASE_PATH = re.compile(r"^/dists/[^/]+/(InRelease|Release)$")


def http_json(url, headers=None, body=None, timeout=30):
    """GET (no body) or POST (a JSON body) and parse the JSON answer; raises on any failure."""
    data = None if body is None else json.dumps(body).encode()
    req = urllib.request.Request(url, data=data, headers={"User-Agent": HTTP_UA, **(headers or {})})
    with urllib.request.urlopen(req, timeout=timeout) as r:
        return json.loads(r.read(16 * 1024 * 1024))


def store_downloads(source, by_day):
    """by_day: {day: {item: n}}. Each (day, source) is replaced whole. Returns the rows stored."""
    rows = 0
    with _lock, db() as con:
        for day, items in by_day.items():
            con.execute("DELETE FROM downloads WHERE day = ? AND source = ?", (day, source))
            con.executemany("INSERT INTO downloads (day, source, item, n) VALUES (?, ?, ?, ?)",
                            [(day, source, k, int(v)) for k, v in items.items()])
            rows += len(items)
    return rows


def asset_platform(name):
    return next((p for rx, p in ASSET_PLATFORM if rx.search(name)), None)


def fetch_github(http=None, today=None):
    """A snapshot of the release download counters, stored under today's date (no credentials needed).
    Only the first page (100 releases) is read."""
    http = http or http_json
    today = (today or utc_today()).isoformat()
    releases = http(GITHUB_RELEASES, {"Accept": "application/vnd.github+json"})
    items = {}
    for rel in releases:
        tag = rel.get("tag_name")
        if rel.get("draft") or not isinstance(tag, str) or not VERSION.match(tag.lstrip("v")):
            continue
        for a in rel.get("assets", []):
            plat = asset_platform(a.get("name", ""))
            if plat and is_int(a.get("download_count")):
                key = f"release:{tag}:{plat}"
                items[key] = items.get(key, 0) + a["download_count"]
    return store_downloads("github", {today: items})


APT_QUERY = ("query($zone:string!,$day:Date!,$host:string!){viewer{zones(filter:{zoneTag:$zone}){"
             "httpRequestsAdaptiveGroups(limit:10000,filter:{date:$day,clientRequestHTTPHost:$host},orderBy:[count_DESC]){"
             "count dimensions{clientRequestPath clientCountryName edgeResponseStatus}}}}}")


def apt_day_items(groups):
    """The downloads items for one day's Cloudflare groups. clientCountryName is Cloudflare's two-letter code."""
    items = {"kind:deb": 0, "kind:update-check": 0}
    for g in groups:
        dim, n = g.get("dimensions") or {}, g.get("count") or 0
        if dim.get("edgeResponseStatus") not in APT_OK_STATUS or not is_int(n):
            continue
        path = dim.get("clientRequestPath") or ""
        if path.endswith(".deb"):
            items["kind:deb"] += n
            m = DEB_VERSION.search(path)
            if m and VERSION.match(m.group(1)):
                items["version:" + m.group(1)] = items.get("version:" + m.group(1), 0) + n
        elif APT_RELEASE_PATH.match(path):
            items["kind:update-check"] += n
            cc = dim.get("clientCountryName") or ""
            if len(cc) == 2 and cc.isalpha():
                items["country:" + cc.upper()] = items.get("country:" + cc.upper(), 0) + n
    return items


def fetch_apt(http=None, today=None):
    """Yesterday's (UTC) apt traffic from Cloudflare's GraphQL Analytics API, and any of the 7 days before that not
    stored yet. One query per day: that stays inside every plan's one-day query window, and adaptive data is kept at
    least 31 days (developers.cloudflare.com/analytics/graphql-api/limits). Counts are Cloudflare's sampled estimates."""
    token = os.environ.get("CF_ANALYTICS_TOKEN", "")
    if not token:
        return None
    http = http or http_json
    zone = os.environ.get("CF_ZONE_ID") or CF_DEFAULT_ZONE
    today = today or utc_today()
    yesterday = today - dt.timedelta(days=1)
    with db() as con:
        have = {r[0] for r in con.execute("SELECT day FROM downloads WHERE source = 'apt' AND item = 'kind:update-check'")}
    days = [yesterday] + [yesterday - dt.timedelta(days=i) for i in range(1, APT_BACKFILL_DAYS + 1) if (yesterday - dt.timedelta(days=i)).isoformat() not in have]
    rows = 0
    for day in days:
        answer = http(CF_GRAPHQL, {"Authorization": "Bearer " + token, "Content-Type": "application/json"},
                      {"query": APT_QUERY, "variables": {"zone": zone, "day": day.isoformat(), "host": APT_HOST}})
        if answer.get("errors"):
            raise ValueError("graphql errors")
        zones = ((answer.get("data") or {}).get("viewer") or {}).get("zones") or []
        if not zones:
            raise ValueError("no zone")
        rows += store_downloads("apt", {day.isoformat(): apt_day_items(zones[0].get("httpRequestsAdaptiveGroups") or [])})
    return rows


def _b64(text):
    return base64.urlsafe_b64decode(text.replace("+", "-").replace("/", "_") + "=" * (-len(text) % 4))


def _bind_discharge(root, discharge):
    """The discharge macaroon bound to the root, serialized as received (v1 or v2 binary, base64): the same bytes
    with the 32-byte signature replaced by HMAC-SHA256(key, HMAC(key, root sig) + HMAC(key, discharge sig)), key = 32
    zero bytes. This is pymacaroons' Macaroon.prepare_for_request (HashSignaturesBinder), which craft-store calls."""
    r, d = _b64(root), _b64(discharge)

    def sig_at(raw):   # the signature is the last field: v1 packet "NNNNsignature <32 bytes>\n", v2 [6, 32, <32 bytes>]
        return len(raw) - (33 if raw[:1] != b"\x02" else 32)

    def sig(raw):
        i = sig_at(raw)
        return raw[i:i + 32]
    key = bytes(32)
    mac = lambda data: hmac.new(key, data, "sha256").digest()
    bound = mac(mac(sig(r)) + mac(sig(d)))
    i = sig_at(d)
    out = d[:i] + bound + d[i + 32:]
    return base64.urlsafe_b64encode(out).decode().rstrip("=")


def snap_auth_header(credentials):
    """The Authorization header for dashboard.snapcraft.io from `snapcraft export-login --acls package_metrics`
    output (base64 of JSON holding the root and discharge macaroons: {"t": "u1-macaroon", "v": {"r": ..., "d": ...}} or
    just {"r": ..., "d": ...}). Source: canonical/craft-store, craft_store/ubuntu_one_store_client.py,
    UbuntuOneStoreClient._get_authorization_header: "Macaroon root=<root>, discharge=<bound discharge>", where the
    discharge is bound with root_macaroon.prepare_for_request(discharge) (see _bind_discharge)."""
    text = credentials.strip()
    try:
        data = json.loads(base64.b64decode(text + "=" * (-len(text) % 4)).decode())
    except ValueError:
        data = json.loads(text)
    if isinstance(data, dict) and "v" in data:
        data = json.loads(data["v"]) if isinstance(data["v"], str) else data["v"]
    root, discharge = data.get("r") or data.get("root"), data.get("d") or data.get("discharge")
    if not (isinstance(root, str) and isinstance(discharge, str)):
        raise ValueError("credentials")
    return f"Macaroon root={root}, discharge={_bind_discharge(root, discharge)}"


def fetch_snap(http=None, today=None):
    """The Snap Store's weekly installed base by version and by country (canonical/snapcraft, snapcraft/commands/metrics.py
    and store/client.py: POST /dev/api/snaps/metrics {"filters": [{"snap_id", "metric_name", "start", "end"}]} answers
    {"metrics": [{"status", "buckets": [dates], "series": [{"name", "values": [...]}]}]})."""
    creds = os.environ.get("SNAP_METRICS_CREDENTIALS", "")
    if not creds.strip():
        return None
    http = http or http_json
    headers = {"Authorization": snap_auth_header(creds), "Accept": "application/json", "Content-Type": "application/json"}
    snap_id = os.environ.get("SNAP_ID", "")
    if not snap_id:
        account = http(SNAP_API + "/account", headers)   # {"snaps": {"16": {"jeffoffice": {"snap-id": ...}}}}
        snap_id = account["snaps"]["16"][SNAP_NAME]["snap-id"]
    end = (today or utc_today()) - dt.timedelta(days=1)
    start = end - dt.timedelta(days=SNAP_WINDOW_DAYS)
    answer = http(SNAP_API + "/snaps/metrics", headers, {"filters": [
        {"snap_id": snap_id, "metric_name": m, "start": start.isoformat(), "end": end.isoformat()} for m in SNAP_METRICS]})
    by_day = {}
    for metric in answer["metrics"]:
        if metric.get("status") != "OK":
            continue
        kind = "version" if metric["metric_name"].endswith("version") else "country"
        for series in metric["series"]:
            name = series["name"]
            if kind == "country":
                name = name.upper()
                if not (len(name) == 2 and name.isalpha()):
                    continue
            elif not VERSION.match(name):
                continue
            for bucket, v in zip(metric["buckets"], series["values"]):
                if is_int(v) and DATE.fullmatch(bucket):
                    items = by_day.setdefault(bucket, {})
                    items[f"{kind}:{name}"] = v
                    if kind == "version":
                        items["kind:total"] = items.get("kind:total", 0) + v
    return store_downloads("snap", by_day) if by_day else 0


FETCHERS = (("github", fetch_github), ("apt", fetch_apt), ("snap", fetch_snap))


def run_download_fetchers(today=None, http=None, fetchers=None):
    """Runs each fetcher; a failure is logged as one short line (the exception's type only) and the others still run."""
    done = {}
    for name, fetch in fetchers or FETCHERS:
        try:
            done[name] = fetch(http, today)
            print(f"downloads {name}: {'skipped, not connected' if done[name] is None else str(done[name]) + ' rows'}", flush=True)
        except Exception as e:
            done[name] = None
            print(f"downloads {name} failed: {type(e).__name__}", flush=True)
    return done


def download_daily():
    time.sleep(DOWNLOAD_START_DELAY)
    while True:
        run_download_fetchers()
        time.sleep(24 * 3600)


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


KEEP_MONTHS = 24


def cutoff(today):
    """The first day still kept: KEEP_MONTHS before today (Feb 29 falls back to Feb 28)."""
    months = today.year * 12 + today.month - 1 - KEEP_MONTHS
    year, month = divmod(months, 12)
    month += 1
    while True:
        try:
            return dt.date(year, month, today.day)
        except ValueError:
            today = today - dt.timedelta(days=1)


def purge(today=None):
    """Delete pings, counts, and crash reports older than KEEP_MONTHS; returns how many rows went."""
    first = cutoff(today or utc_today()).isoformat()
    with _lock, db() as con:
        n = con.execute("DELETE FROM pings WHERE day < ?", (first,)).rowcount
        n += con.execute("DELETE FROM counts WHERE day < ?", (first,)).rowcount
        n += con.execute("DELETE FROM crashes WHERE received < ?", (first,)).rowcount
        n += con.execute("DELETE FROM downloads WHERE day < ?", (first,)).rowcount
    return n


def forget(install):
    """Delete everything kept for one install id; returns how many rows went."""
    with _lock, db() as con:
        return sum(con.execute(f"DELETE FROM {t} WHERE install = ?", (install,)).rowcount
                   for t in ("pings", "counts", "crashes"))


def purge_daily():
    while True:
        try:
            print(f"purged {purge()} rows older than {KEEP_MONTHS} months", flush=True)
        except Exception as e:   # a failed purge must not stop the collector; the next day tries again
            print(f"purge failed: {e}", flush=True)
        time.sleep(24 * 3600)


def utc_today():
    return dt.datetime.now(dt.timezone.utc).date()


def record(ping, today=None, country=None):
    install, version, os_name, os_version, lang, launches, counts, app = ping
    day = (today or utc_today()).isoformat()
    with _lock, db() as con:
        con.execute(
            """INSERT INTO pings (day, install, app, version, os, os_version, lang, launches, country)
               VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
               ON CONFLICT (day, install, app) DO UPDATE SET version = excluded.version,
                 os = excluded.os, os_version = excluded.os_version, lang = excluded.lang,
                 launches = launches + excluded.launches,
                 country = COALESCE(excluded.country, country)""",
            (day, install, app, version, os_name, os_version, lang, launches, country),
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


FOOTER_LINKS = " · ".join(f'<a href="{u}">{t}</a>' for t, u in (
    ("Privacy notice", "https://www.jeffoffice.com/privacy/"),
    ("Terms of service", "https://www.jeffoffice.com/terms/"),
    ("License (GNU GPL version 3)", "https://github.com/JeffOffice/jeffoffice/blob/main/LICENSE"),
    ("Third-party licenses", "https://www.jeffoffice.com/third-party/"),
    ("This service's source code", "https://github.com/JeffOffice/jeffoffice/tree/main/server/telemetry")))


ABOUT = """<!doctype html><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<meta name="color-scheme" content="light dark"><title>JeffOffice usage statistics</title>
<style>:root{--bg:#f6f7f9;--text:#1c1d1f;--muted:#5f6368;--link:#1f63c6;--card:#fff;--border:rgba(0,0,0,.12)}
@media (prefers-color-scheme:dark){:root{--bg:#161718;--text:#ececec;--muted:#a3a7ad;--link:#7fb0f5;--card:#1f2022;--border:rgba(255,255,255,.12)}}
body{margin:0;background:var(--bg);color:var(--text);font:400 16px/1.6 system-ui,-apple-system,"Segoe UI",Roboto,sans-serif}
main{max-width:42rem;margin:0 auto;padding:1.5rem 1rem 2rem;overflow-wrap:anywhere}
h1{font-size:1.5rem;font-weight:500;line-height:1.25;margin:0 0 1rem}h2{font-size:1.1rem;font-weight:500;margin:1.6rem 0 .5rem}
a{color:var(--link)}code{font-size:.9em}ul{padding-left:1.2rem}li{margin:.3rem 0}
footer{color:var(--muted);font-size:.85rem;margin-top:2rem;padding-top:1rem;border-top:.5px solid var(--border)}</style>
<main>
<h1>JeffOffice usage statistics</h1>
<h2>Why we collect this</h2>
<p>JeffOffice is made by a small team, and these numbers are how we learn what matters to the people who use it. They show us which features get used, so we can improve them first and keep them working well. They show us which versions and systems are in use, so we know what to test and when it's safe to stop supporting an old one. Crash reports show us where a program stopped, so we can fix the problem, often before anyone writes to us. The country count tells us which languages to add next.</p>
<p>We don't sell this data, share it, or use it for advertising. It's optional: you can turn it off at any time in File &gt; Options.</p>
<h2>What's sent</h2>
<p>The JeffOffice apps (JeffPub, JeffDeck, JeffWrite, and JeffCrunch) share one choice about this, asked in setup or when one of them first starts, and changed
under File &gt; Options in any of them. When it's on, each of them sends the following once a day while it's in
use, and once more the day it's updated:</p>
<ul><li>a random number made up when JeffOffice was installed, which says nothing about you or your computer
(every app sends the same number, so someone who uses several is one person here);</li>
<li>which app it comes from, its version, your operating system and its version,
and your computer's language;</li>
<li>how many times it was started, and how many times each of its commands was used
(for example "opened a .pub file" in JeffPub, "opened a .pptx file" in JeffDeck, "saved a .xlsx file" in JeffCrunch, or "exported a PDF").</li></ul>
<p>Never your files, their names, or anything in them. When a statistics message arrives, this program works out
which country it came from, using the address of the connection and a database that lives on the server itself,
and keeps only the country (for example "DE"). It doesn't keep or log the address, and doesn't send it anywhere.
Everything is deleted after 24 months.
The numbers show how many people use each app and which parts matter most to them.</p>
<h2>Crash reports</h2>
<p>If usage statistics are on and one of the apps crashes, it sends a crash report the next time it starts.
The report says where in the program it stopped (the names of program files, and numbers that give
a position in each), what kind of failure it was (a number), the day it happened, which app it was and its
version, your operating system, and the same random number as above. It never includes your documents, their
names or contents, anything you typed, or anything that was in your computer's memory. If you turn
usage statistics off, no crash reports are sent either.</p>
<p>More, including how to have your records deleted: the <a href="https://www.jeffoffice.com/privacy">JeffOffice privacy notice</a>.</p>
<p>The program that collects all of this is part of JeffOffice's source:
<a href="https://github.com/JeffOffice/jeffoffice/tree/main/server/telemetry">server/telemetry</a>.</p>
<footer>""" + FOOTER_LINKS + "</footer></main>\n"


PERIODS = (7, 30, 90)
DEFAULT_DAYS = 30

# ISO 3166-1 alpha-2 : numeric id used by world-atlas countries-110m : name shown
COUNTRIES = {}
for _row in """AD:020:Andorra|AE:784:United Arab Emirates|AF:004:Afghanistan|AG:028:Antigua and Barbuda|AI:660:Anguilla|AL:008:Albania|AM:051:Armenia|AO:024:Angola|AQ:010:Antarctica|AR:032:Argentina|AS:016:American Samoa|AT:040:Austria|AU:036:Australia|AW:533:Aruba|AX:248:Åland Islands|AZ:031:Azerbaijan|BA:070:Bosnia and Herzegovina|BB:052:Barbados|BD:050:Bangladesh|BE:056:Belgium|BF:854:Burkina Faso|BG:100:Bulgaria|BH:048:Bahrain|BI:108:Burundi|BJ:204:Benin|BL:652:Saint Barthélemy|BM:060:Bermuda|BN:096:Brunei|BO:068:Bolivia|BQ:535:Caribbean Netherlands|BR:076:Brazil|BS:044:Bahamas|BT:064:Bhutan|BV:074:Bouvet Island|BW:072:Botswana|BY:112:Belarus|BZ:084:Belize|CA:124:Canada|CC:166:Cocos Islands|CD:180:DR Congo|CF:140:Central African Republic|CG:178:Congo|CH:756:Switzerland|CI:384:Côte d'Ivoire|CK:184:Cook Islands|CL:152:Chile|CM:120:Cameroon|CN:156:China|CO:170:Colombia|CR:188:Costa Rica|CU:192:Cuba|CV:132:Cape Verde|CW:531:Curaçao|CX:162:Christmas Island|CY:196:Cyprus|CZ:203:Czechia|DE:276:Germany|DJ:262:Djibouti|DK:208:Denmark|DM:212:Dominica|DO:214:Dominican Republic|DZ:012:Algeria|EC:218:Ecuador|EE:233:Estonia|EG:818:Egypt|EH:732:Western Sahara|ER:232:Eritrea|ES:724:Spain|ET:231:Ethiopia|FI:246:Finland|FJ:242:Fiji|FK:238:Falkland Islands|FM:583:Micronesia|FO:234:Faroe Islands|FR:250:France|GA:266:Gabon|GB:826:United Kingdom|GD:308:Grenada|GE:268:Georgia|GF:254:French Guiana|GG:831:Guernsey|GH:288:Ghana|GI:292:Gibraltar|GL:304:Greenland|GM:270:Gambia|GN:324:Guinea|GP:312:Guadeloupe|GQ:226:Equatorial Guinea|GR:300:Greece|GS:239:South Georgia|GT:320:Guatemala|GU:316:Guam|GW:624:Guinea-Bissau|GY:328:Guyana|HK:344:Hong Kong|HM:334:Heard Island|HN:340:Honduras|HR:191:Croatia|HT:332:Haiti|HU:348:Hungary|ID:360:Indonesia|IE:372:Ireland|IL:376:Israel|IM:833:Isle of Man|IN:356:India|IO:086:British Indian Ocean Territory|IQ:368:Iraq|IR:364:Iran|IS:352:Iceland|IT:380:Italy|JE:832:Jersey|JM:388:Jamaica|JO:400:Jordan|JP:392:Japan|KE:404:Kenya|KG:417:Kyrgyzstan|KH:116:Cambodia|KI:296:Kiribati|KM:174:Comoros|KN:659:Saint Kitts and Nevis|KP:408:North Korea|KR:410:South Korea|KW:414:Kuwait|KY:136:Cayman Islands|KZ:398:Kazakhstan|LA:418:Laos|LB:422:Lebanon|LC:662:Saint Lucia|LI:438:Liechtenstein|LK:144:Sri Lanka|LR:430:Liberia|LS:426:Lesotho|LT:440:Lithuania|LU:442:Luxembourg|LV:428:Latvia|LY:434:Libya|MA:504:Morocco|MC:492:Monaco|MD:498:Moldova|ME:499:Montenegro|MF:663:Saint Martin|MG:450:Madagascar|MH:584:Marshall Islands|MK:807:North Macedonia|ML:466:Mali|MM:104:Myanmar|MN:496:Mongolia|MO:446:Macao|MP:580:Northern Mariana Islands|MQ:474:Martinique|MR:478:Mauritania|MS:500:Montserrat|MT:470:Malta|MU:480:Mauritius|MV:462:Maldives|MW:454:Malawi|MX:484:Mexico|MY:458:Malaysia|MZ:508:Mozambique|NA:516:Namibia|NC:540:New Caledonia|NE:562:Niger|NF:574:Norfolk Island|NG:566:Nigeria|NI:558:Nicaragua|NL:528:Netherlands|NO:578:Norway|NP:524:Nepal|NR:520:Nauru|NU:570:Niue|NZ:554:New Zealand|OM:512:Oman|PA:591:Panama|PE:604:Peru|PF:258:French Polynesia|PG:598:Papua New Guinea|PH:608:Philippines|PK:586:Pakistan|PL:616:Poland|PM:666:Saint Pierre and Miquelon|PN:612:Pitcairn|PR:630:Puerto Rico|PS:275:Palestine|PT:620:Portugal|PW:585:Palau|PY:600:Paraguay|QA:634:Qatar|RE:638:Réunion|RO:642:Romania|RS:688:Serbia|RU:643:Russia|RW:646:Rwanda|SA:682:Saudi Arabia|SB:090:Solomon Islands|SC:690:Seychelles|SD:729:Sudan|SE:752:Sweden|SG:702:Singapore|SH:654:Saint Helena|SI:705:Slovenia|SJ:744:Svalbard and Jan Mayen|SK:703:Slovakia|SL:694:Sierra Leone|SM:674:San Marino|SN:686:Senegal|SO:706:Somalia|SR:740:Suriname|SS:728:South Sudan|ST:678:São Tomé and Príncipe|SV:222:El Salvador|SX:534:Sint Maarten|SY:760:Syria|SZ:748:Eswatini|TC:796:Turks and Caicos Islands|TD:148:Chad|TF:260:French Southern Territories|TG:768:Togo|TH:764:Thailand|TJ:762:Tajikistan|TK:772:Tokelau|TL:626:Timor-Leste|TM:795:Turkmenistan|TN:788:Tunisia|TO:776:Tonga|TR:792:Türkiye|TT:780:Trinidad and Tobago|TV:798:Tuvalu|TW:158:Taiwan|TZ:834:Tanzania|UA:804:Ukraine|UG:800:Uganda|UM:581:US Minor Outlying Islands|US:840:United States|UY:858:Uruguay|UZ:860:Uzbekistan|VA:336:Vatican City|VC:670:Saint Vincent and the Grenadines|VE:862:Venezuela|VG:092:British Virgin Islands|VI:850:US Virgin Islands|VN:704:Vietnam|VU:548:Vanuatu|WF:876:Wallis and Futuna|WS:882:Samoa|YE:887:Yemen|YT:175:Mayotte|ZA:710:South Africa|ZM:894:Zambia|ZW:716:Zimbabwe""".split("|"):
    _code, _num, _name = _row.split(":")
    COUNTRIES[_code] = (int(_num), _name)

ATLAS_SCRIPTS = ("https://cdn.jsdelivr.net/npm/d3@7.9.0/dist/d3.min.js",
                 "https://cdn.jsdelivr.net/npm/topojson-client@3.1.0/dist/topojson-client.min.js")
ATLAS_DATA = "https://cdn.jsdelivr.net/npm/world-atlas@2.0.2/countries-110m.json"


# ---------------------------------------------------------------- data layer
# gather_stats() runs the queries and returns plain data; render_stats() (below) turns it
# into HTML and knows nothing about SQL, so the page's design can be swapped on its own.

def geo_status(today=None):
    """Which country list is in use: {"month": date of its month, "fetched": date of the download, "stale": bool}, or None."""
    path = _geo_path
    if _geo is None or not path:
        return None
    m = re.search(r"(\d{4})-(\d{2})\.csv\.gz$", path)
    month = dt.date(int(m.group(1)), int(m.group(2)), 1)
    try:
        fetched = dt.datetime.fromtimestamp(os.path.getmtime(path), dt.timezone.utc).date()
    except OSError:
        fetched = None
    today = today or utc_today()
    age = (today.year - month.year) * 12 + today.month - month.month
    return {"month": month, "fetched": fetched, "stale": age > GEO_STALE_MONTHS}


def geo_line(g):
    """The footer's sentence about the country list."""
    if g is None:
        return '<span class="down">Country list: none loaded, so countries aren\'t being recorded.</span>'
    text = f"Country list: DB-IP's {g['month']:%B %Y} list"
    if g["fetched"]:
        text += f", downloaded {g['fetched']:%b} {g['fetched'].day}, {g['fetched'].year}"
    text += ". It's checked for a new one every week."
    return f'<span class="down">{html.escape(text)} It\'s more than {GEO_STALE_MONTHS} months old, so downloads may be failing.</span>' if g["stale"] else html.escape(text)


def github_rollup(rows, dates):
    """rows: (day, item, n) of the github source, oldest first. Returns {"since": first snapshot day, "platforms":
    {platform: {"total": all-time, "series": downloads per day for dates, "period": their sum}}, "versions": {version: downloads
    in dates}}. A release's downloads on a day are its count minus its previous snapshot; the first snapshot day of all is the
    baseline (nothing counted); a release first seen later counts whole."""
    since = rows[0][0] if rows else None
    last, per_day, versions, total = {}, {}, {}, {}
    for day, item, n in rows:
        _, tag, plat = item.split(":", 2)
        prev = last.get((tag, plat))
        gain = max(n - prev, 0) if prev is not None else (0 if day == since else n)
        last[(tag, plat)] = n
        if day in dates:
            per_day[(plat, day)] = per_day.get((plat, day), 0) + gain
            versions[tag.lstrip("v")] = versions.get(tag.lstrip("v"), 0) + gain
    for (tag, plat), n in last.items():
        total[plat] = total.get(plat, 0) + n
    plats = {}
    for plat, _ in PLATFORMS:
        series = [per_day.get((plat, day), 0) for day in dates]
        plats[plat] = {"total": total.get(plat, 0), "series": series, "period": sum(series)}
    return {"since": since, "platforms": plats, "versions": {v: n for v, n in versions.items() if n}}


def gather_downloads(days, today=None):
    """The Downloads section's numbers: github (see github_rollup), apt, and snap, each None-valued fields when it has no rows."""
    today = today or utc_today()
    dates = [(today - dt.timedelta(days=days - 1 - i)).isoformat() for i in range(days)]
    with db() as con:
        rows = lambda source, lo=None: con.execute("SELECT day, item, n FROM downloads WHERE source = ?" + (" AND day >= ?" if lo else "")
                                                     + " ORDER BY day", (source, lo) if lo else (source,)).fetchall()
        gh = github_rollup(rows("github"), set(dates))
        apt_rows = rows("apt", dates[0])
        snap_all = rows("snap")
    deb, checks, versions, countries = [0] * days, [0] * days, {}, {}
    for day, item, n in apt_rows:
        i = dates.index(day) if day in dates else None
        if i is None:
            continue
        if item == "kind:deb":
            deb[i] += n
        elif item == "kind:update-check":
            checks[i] += n
        elif item.startswith("version:"):
            versions[item[8:]] = versions.get(item[8:], 0) + n
        elif item.startswith("country:"):
            countries[item[8:]] = countries.get(item[8:], 0) + n
    apt = {"connected": bool(os.environ.get("CF_ANALYTICS_TOKEN")), "has_data": bool(apt_rows), "deb_series": deb, "deb": sum(deb),
           "check_series": checks, "checks": sum(checks), "versions": versions, "countries": countries}
    totals, latest = {}, {}   # snap: kind:total per bucket day, and the newest value of each item
    for day, item, n in snap_all:
        if item == "kind:total":
            totals[day] = n
        latest[item] = (day, n)
    newest = max((d for d, _ in latest.values()), default=None)
    snap = {"connected": bool(os.environ.get("SNAP_METRICS_CREDENTIALS", "").strip()), "has_data": bool(snap_all), "day": newest,
            "now": totals.get(newest, 0) if newest else 0,
            "series": [totals[d] for d in sorted(totals)[-26:]],   # weekly buckets, so the newest half year
            "countries": {k[8:]: v[1] for k, v in latest.items() if k.startswith("country:") and v[0] == newest}}
    combined = dict(gh["versions"])
    for v, n in versions.items():
        combined[v] = combined.get(v, 0) + n
    return {"github": gh, "apt": apt, "snap": snap, "versions": combined}


def gather_stats(days=DEFAULT_DAYS, today=None):
    """Everything the dashboard shows, as a dict: one row per app plus one for all apps, the
    country, version, system, language, and command lists, and the crash counts by version."""
    today = today or utc_today()
    start = today - dt.timedelta(days=days - 1)
    prev = start - dt.timedelta(days=days)
    t, s, p = today.isoformat(), start.isoformat(), prev.isoformat()
    pend = (start - dt.timedelta(days=1)).isoformat()   # the previous period is p..pend
    dates = [(start + dt.timedelta(days=i)).isoformat() for i in range(days)]
    with db() as con:
        q = lambda sql, *a: con.execute(sql, a).fetchall()
        one = lambda sql, *a: con.execute(sql, a).fetchone()[0]

        def by_app(sql, *a):
            return {app: n for app, n in q(sql, *a)}

        def block(select, group_extra, *a):
            """{app or None: n}: the same count per app and, under None, over all of them."""
            out = {app: n for app, n in q(select.format(cols="app, ", group=" GROUP BY app"), *a)}
            out[None] = q(select.format(cols="NULL, ", group=""), *a)[0][1]
            return out

        active = lambda lo, hi: block("SELECT {cols}COUNT(DISTINCT install) FROM pings WHERE day >= ? AND day <= ?{group}", None, lo, hi)
        new = lambda lo, hi: block("SELECT {cols}COUNT(*) FROM (SELECT app, MIN(day) AS f FROM pings GROUP BY install, app) "
                                   "WHERE f >= ? AND f <= ?{group}", None, lo, hi)
        new_all = lambda lo, hi: one("SELECT COUNT(*) FROM (SELECT MIN(day) AS f FROM pings GROUP BY install) WHERE f >= ? AND f <= ?", lo, hi)
        crashed = lambda lo, hi: block("SELECT {cols}COUNT(*) FROM crashes WHERE received >= ? AND received <= ?{group}", None, lo, hi)
        a_now, a_prev, c_now, c_prev = active(s, t), active(p, pend), crashed(s, t), crashed(p, pend)
        n_now, n_prev = new(s, t), new(p, pend)
        n_now[None], n_prev[None] = new_all(s, t), new_all(p, pend)
        series = {}
        for day, app, n in q("SELECT day, app, COUNT(*) FROM pings WHERE day >= ? GROUP BY day, app", s):
            series.setdefault(app, {})[day] = n
        series[None] = dict(q("SELECT day, COUNT(DISTINCT install) FROM pings WHERE day >= ? GROUP BY day", s))
        rows = [{"app": a, "name": APPS[a] if a else "All apps", "series": [series.get(a, {}).get(day, 0) for day in dates],
                 "active": a_now.get(a, 0), "active_prev": a_prev.get(a, 0), "new": n_now.get(a, 0), "new_prev": n_prev.get(a, 0),
                 "crashes": c_now.get(a, 0), "crashes_prev": c_prev.get(a, 0)} for a in (*APPS, None)]
        # One row per install (its newest ping in the period) so a person isn't counted twice.
        latest = lambda expr, group="install": q(
            f"SELECT c, COUNT(*) FROM (SELECT {expr} AS c, MAX(day) FROM pings WHERE day >= ? GROUP BY {group}) "
            "GROUP BY c ORDER BY 2 DESC, 1", s)
        per = {}   # feature -> app -> [times, installs]
        for feature, a, times, installs in q("SELECT feature, app, SUM(n), COUNT(DISTINCT install) FROM counts WHERE day >= ? "
                                             "GROUP BY feature, app", s):
            per.setdefault(feature, {})[a] = [times, installs]
        commands = sorted(({"feature": f, "times": tm, "installs": ins, "apps": per[f]} for f, tm, ins in
                           q("SELECT feature, SUM(n), COUNT(DISTINCT install) FROM counts WHERE day >= ? GROUP BY feature", s)),
                          key=lambda c: (-c["installs"], -c["times"], c["feature"]))
        crash_versions = q("SELECT version, COUNT(*), COUNT(DISTINCT install) FROM crashes WHERE received >= ? "
                           "GROUP BY version ORDER BY 2 DESC, 1", s)
        data = {
            "days": days, "today": t, "geo": geo_status(today), "start": s, "dates": dates, "rows": rows,
            "countries": latest("country"), "countries_since": one("SELECT MIN(day) FROM pings WHERE country IS NOT NULL"),
            "versions": latest("version", "install, app"), "systems": latest("os"), "langs": latest("lang"),
            "commands": commands, "crash_versions": crash_versions,
        }
    data["downloads"] = gather_downloads(days, today)
    data["kpi"] = next(r for r in rows if r["app"] is None)   # all apps, for callers that want one set of numbers
    return data


# ---------------------------------------------------------------- render layer (HTML and CSS only)

STYLE = """
:root{color-scheme:light dark;--bg:#F5F6F8;--card:#fff;--text:#1C2230;--muted:#5A6477;--border:#D9DEE7;--bar:#2a78d6;--track:#EBEFF5;
--up:#17703A;--down:#B3261E;--chip:#fff;--chip-on:#2a78d6;--chip-on-text:#fff;--link:#1F63B8;--nodata:#E3E7ED;--line:#fff;--gray:#8C95A6;
--r0:#d6e6f9;--r1:#a3c6f0;--r2:#5a9ae6;--r3:#2a78d6;--r4:#174a85;
--c-jeffpub:#2a78d6;--c-jeffdeck:#eb6834;--c-jeffwrite:#1baf7a;--c-jeffcrunch:#eda100}
@media (prefers-color-scheme:dark){:root:not([data-theme=light]){--bg:#0F141C;--card:#171E2B;--text:#E6EAF2;--muted:#9DA8BC;--border:#2C3649;
--bar:#3987e5;--track:#232D40;--up:#5CD08A;--down:#FF8A80;--chip:#171E2B;--chip-on:#3987e5;--chip-on-text:#0C1118;--link:#7FB0F5;--nodata:#222B3C;--line:#171E2B;--gray:#7C879A;
--r0:#0c2a4d;--r1:#174a85;--r2:#2a78d6;--r3:#5a9ae6;--r4:#9cc3f0;
--c-jeffpub:#3987e5;--c-jeffdeck:#d95926;--c-jeffwrite:#199e70;--c-jeffcrunch:#c98500}}
*{box-sizing:border-box}
body{margin:0;background:var(--bg);color:var(--text);font:400 15px/1.45 system-ui,-apple-system,"Segoe UI",Roboto,sans-serif}
main{max-width:68rem;margin:0 auto;padding:1rem}
h1{font-size:1.4rem;font-weight:500;margin:0}h2{font-size:.95rem;font-weight:500;margin:0 0 .6rem}
a{color:var(--link)}
.head{display:flex;flex-wrap:wrap;gap:.5rem 1rem;align-items:center;justify-content:space-between;margin-bottom:1rem}
.head .right{display:flex;flex-wrap:wrap;gap:.5rem 1rem;align-items:center}
.chips{display:flex;flex-wrap:wrap;gap:.35rem}
.chips a{padding:.2rem .75rem;border-radius:999px;border:.5px solid var(--border);background:var(--chip);color:var(--text);text-decoration:none}
.chips a[aria-current]{background:var(--chip-on);border-color:var(--chip-on);color:var(--chip-on-text);font-weight:500}
.muted{color:var(--muted)}.small{font-size:.8rem}
.card{background:var(--card);border:.5px solid var(--border);border-radius:12px;padding:.9rem 1rem;margin-bottom:1rem;min-width:0}
.two{display:grid;grid-template-columns:minmax(0,2fr) minmax(0,1fr);gap:1rem;margin-bottom:1rem}.two .card{margin:0}
@media (max-width:42rem){.two{grid-template-columns:1fr}}
.cards{display:grid;grid-template-columns:repeat(auto-fit,minmax(10.5rem,1fr));gap:1rem;margin-bottom:1rem}.cards .card{margin:0}
.tablewrap{overflow-x:auto}table{border-collapse:collapse;width:100%}
td,th{padding:.45rem .5rem;border-bottom:.5px solid var(--border);text-align:left;vertical-align:middle}
th{font-weight:500;color:var(--muted);font-size:.8rem}tr:last-child td{border-bottom:0}
td.r,th.r{text-align:right;font-variant-numeric:tabular-nums}
.app{white-space:nowrap}.sw{display:inline-block;width:10px;height:10px;border-radius:3px;margin-right:.5rem}
.delta{display:block;font-size:.75rem;color:var(--muted)}.up{color:var(--up)}.down{color:var(--down)}
table.dl .app{white-space:normal}table.dl .delta{white-space:nowrap}
td.spark svg{width:160px;height:30px}
@media (max-width:32.5rem){td.spark,th.spark{padding-left:.2rem;padding-right:.2rem}td.spark svg{width:64px;height:24px}td.spark circle{display:none}td,th{padding-left:.3rem;padding-right:.3rem}}
svg{display:block;max-width:100%}
.mapbox{position:relative;aspect-ratio:2/1;width:100%;margin-bottom:.4rem}.mapbox svg{position:absolute;inset:0;width:100%;height:100%}
.mapbox p{position:absolute;inset:0;margin:0;display:flex;align-items:center;justify-content:center;text-align:center;padding:1rem}.mapbox p[hidden]{display:none}.sphere{fill:none}.c{fill:var(--nodata);stroke:var(--line);stroke-width:.4}
ul.rows{list-style:none;margin:0;padding:0}
ul.rows li{display:grid;grid-template-columns:minmax(2.5rem,40%) minmax(0,1fr) auto;gap:.5rem;align-items:center;padding:.18rem 0}
ul.rows.wide li{grid-template-columns:minmax(2.5rem,62%) minmax(0,1fr) auto}
ul.rows .l{overflow:hidden;text-overflow:ellipsis;white-space:nowrap}
ul.rows .n{text-align:right;font-variant-numeric:tabular-nums;color:var(--muted)}
ul.rows .t{height:8px;background:var(--track);border-radius:0 4px 4px 0}
ul.rows .t span{display:block;height:100%;background:var(--bar);border-radius:0 4px 4px 0}
code{font-size:.9em}summary{cursor:pointer;color:var(--link)}details{margin-bottom:1rem}details .card{margin-top:.6rem}
footer{color:var(--muted);font-size:.8rem;margin:1.2rem 0 .5rem}
"""

MAP_JS = """
document.addEventListener('DOMContentLoaded',function(){
 var el=document.getElementById('map'),status=document.getElementById('map-status');
 if(!el)return;
 function fail(){if(status&&!status.hidden)status.textContent='The map could not load (it needs scripts from cdn.jsdelivr.net). The list beside it shows the same numbers.'}
 if(!window.d3||!window.topojson){fail();return}
 var timer=setTimeout(fail,15000);
 var data=JSON.parse(document.getElementById('country-data').textContent);
 fetch(%s).then(function(r){return r.json()}).then(function(w){
  var feats=topojson.feature(w,w.objects.countries).features.filter(function(f){return +f.id!==10});
  function draw(){
   var cs=getComputedStyle(document.documentElement),ramp=[0,1,2,3,4].map(function(i){return cs.getPropertyValue('--r'+i).trim()});
   var max=d3.max(Object.values(data),function(d){return d[0]})||1;
   function color(n){return ramp[Math.min(4,Math.floor(5*Math.sqrt(n/max)))]}
   var land={type:'FeatureCollection',features:feats};
   var path=d3.geoPath(d3.geoNaturalEarth1().fitSize([960,480],land));
   var svg=d3.select(el);svg.selectAll('*').remove();svg.attr('viewBox','0 0 960 480');
   svg.selectAll('path.c').data(feats).join('path').attr('class','c').attr('d',path)
    .style('fill',function(f){var d=data[+f.id];return d?color(d[0]):null})
    .append('title').text(function(f){var d=data[+f.id];return f.properties.name+': '+(d?d[0]:0)});
  }
  draw();
  matchMedia('(prefers-color-scheme: dark)').addEventListener('change',draw);
  clearTimeout(timer);if(status)status.hidden=true;
 }).catch(function(){clearTimeout(timer);fail()});
});
""" % json.dumps(ATLAS_DATA)


def page_url(page, app=None, days=None):
    query = [] if app is None else [("app", app)]
    if days is not None and days != DEFAULT_DAYS:
        query.append(("days", days))
    return "/" + page + ("?" + urllib.parse.urlencode(query) if query else "")


def chips(options, current, label):
    """options: (text, url, value) triples; the one equal to current is marked."""
    links = "".join(f'<a href="{html.escape(u)}"{" aria-current=page" if v == current else ""}>{html.escape(t)}</a>' for t, u, v in options)
    return f'<nav class="chips" aria-label="{html.escape(label)}">{links}</nav>'


def app_chips(page, app, days=None):
    opts = [("All", page_url(page, None, days), None)] + [(name, page_url(page, a, days), a) for a, name in APPS.items()]
    return chips(opts, app, "App")


def pct(n, total):
    return f"{100 * n / total:.0f}%" if total else "0%"


OS_NAMES = {"windows": "Windows", "macos": "macOS", "linux": "Linux"}


def bar_list(rows, empty="none yet", label=lambda v: v, wide=False):
    """rows: (value, count): label, a bar relative to the biggest, and the count (the share is in the hover text)."""
    total = sum(n for _, n in rows)
    if not rows:
        return f'<p class="muted">{html.escape(empty)}</p>'
    top = max(n for _, n in rows)
    e = html.escape
    items = "".join(f'<li title="{e(str(label(v)))}: {n} ({pct(n, total)})"><span class="l">{e(str(label(v)))}</span>'
                    f'<span class="t"><span style="width:{100 * n / top:.1f}%"></span></span><span class="n">{n}</span></li>' for v, n in rows)
    return f'<ul class="rows{" wide" if wide else ""}">{items}</ul>'


def delta(cur, prev):
    """The change against the previous equal period, as a small line under a number."""
    if prev == 0:
        return '<span class="delta">no earlier data</span>' if cur == 0 else f'<span class="delta">was 0</span>'
    change = 100 * (cur - prev) / prev
    cls = "up" if change > 0 else "down" if change < 0 else ""
    return f'<span class="delta {cls}">{"+" if change > 0 else ""}{change:.0f}%</span>'


def sparkline(values, color, label):
    w, h, pad = 160, 30, 5
    peak = max([1] + values)
    n = len(values)
    pts = [(pad + (w - 2 * pad) * (i / max(n - 1, 1)), h - pad - (h - 2 * pad) * v / peak) for i, v in enumerate(values)]
    line = " ".join(f"{x:.1f},{y:.1f}" for x, y in pts)
    area = f"{pts[0][0]:.1f},{h - pad} {line} {pts[-1][0]:.1f},{h - pad}"
    x, y = pts[-1]
    return (f'<svg width="{w}" height="{h}" viewBox="0 0 {w} {h}" preserveAspectRatio="none" role="img" aria-label="{html.escape(label)}">'
            f'<title>{html.escape(label)}</title><polygon points="{area}" fill="{color}" fill-opacity=".12"/>'
            f'<polyline points="{line}" fill="none" stroke="{color}" stroke-width="2" stroke-linejoin="round" stroke-linecap="round"/>'
            f'<circle cx="{x:.1f}" cy="{y:.1f}" r="3.5" fill="{color}" stroke="var(--card)" stroke-width="2"/></svg>')


def render_map(d):
    e = html.escape
    known, unknown = {}, 0
    for code, n in d["countries"]:
        if code in COUNTRIES:
            known[code] = n
        else:
            unknown += n
    data = {str(COUNTRIES[c][0]): [n, COUNTRIES[c][1]] for c, n in known.items()}
    payload = json.dumps(data).replace("<", "\\u003c")
    top = sorted(known.items(), key=lambda kv: (-kv[1], kv[0]))[:8]
    since = d["countries_since"]
    note = f"Countries are known for pings since {e(since)}." if since else "No country is known yet: it is recorded for pings from now on."
    rows = bar_list(top, "no countries yet", lambda c: COUNTRIES[c][1])
    return (f'<div class="two"><section class="card"><h2>Installs by country</h2>'
            f'<div class="mapbox"><svg id="map" viewBox="0 0 960 480" role="img" aria-label="World map of installs by country"></svg>'
            f'<p id="map-status" class="muted small">Loading the map…</p></div>'
            f'<script type="application/json" id="country-data">{payload}</script>'
            f'<p class="muted small">{note} Installs active in the period, by the country of their newest ping.</p></section>'
            f'<section class="card"><h2>Top countries</h2>{rows}<p>Unknown: {unknown}</p></section></div>')


def render_downloads(d):
    """The Downloads section: a table like the per-app one, then small cards when there is something to list."""
    dl, days, e = d["downloads"], d["days"], html.escape
    gh, apt, snap = dl["github"], dl["apt"], dl["snap"]

    def named(name, source):
        return f'{e(name)}<span class="delta">{e(source)}</span>'

    def row(name, source, series, label, main, sub="", total="", color="var(--bar)"):
        spark = sparkline(series, color, label) if series else ""
        return (f'<tr><td class="app">{named(name, source)}</td><td class="spark">{spark}</td><td class="r">{main}{sub}</td>'
                f'<td class="r">{total}</td></tr>')

    def off(name, source, what):
        return f'<tr><td class="app">{named(name, source)}</td><td class="muted" colspan="3">{e(what)}: not connected yet</td></tr>'

    body = ""
    for plat, name in PLATFORMS:
        p = gh["platforms"][plat]
        body += row(name, "GitHub releases", p["series"], f"{name}: {p['period']} in {days} days", p["period"], total=p["total"])
    if not apt["connected"]:
        body += off("apt", "apt.jeffoffice.com", "apt")
    else:
        sub = f'<span class="delta">{apt["checks"]} checks</span>'
        body += row("apt (.deb)", "apt.jeffoffice.com", apt["check_series"], f"apt update checks per day: {apt['checks']} in {days} days",
                    apt["deb"], sub, "")
    if not snap["connected"]:
        body += off("Snap Store", "snapcraft.io", "Snap Store")
    elif snap["day"]:
        body += row("Snap Store", "snapcraft.io", snap["series"], f"Snap installed base, week of {snap['day']}: {snap['now']}", snap["now"],
                    '<span class="delta">installed now</span>', "")
    else:
        body += f'<tr><td class="app">{named("Snap Store", "snapcraft.io")}</td><td class="muted" colspan="3">no data yet</td></tr>'
    since = gh["since"]
    notes = (f"GitHub counts downloads of each installer; the period is the change in GitHub's running total, "
             f"counting since {e(since)}. All time is GitHub's own total. " if since else
             "GitHub's counts start with the first daily snapshot, which hasn't been taken yet. ")
    notes += ("apt shows .deb downloads in the period; its graph and the checks figure count update checks, which are requests for the "
              "repository's Release files, not separate computers. Snap Store shows the weekly installed base (its newest week), not downloads.") if apt["connected"] else ""
    table = (f'<section class="card"><h2>Downloads</h2><div class="tablewrap"><table class="dl"><tr><th></th><th class="spark">Per day</th>'
             f'<th class="r">Last {days} days</th><th class="r">All time</th></tr>{body}</table></div>'
             f'<p class="muted small">{notes}</p></section>')
    top = lambda m: sorted(m.items(), key=lambda kv: (-kv[1], kv[0]))[:8]
    country = lambda c: COUNTRIES[c][1] if c in COUNTRIES else c
    cards = "".join(f'<section class="card"><h2>{e(title)}</h2>{bar_list(rows, label=label)}</section>' for title, rows, label in (
        (f"Downloads by version, {days} days", top(dl["versions"]), lambda v: v),
        ("Snap installs by country", top(snap["countries"]), country),
        (f"apt update checks by country, {days} days", top(apt["countries"]), country)) if rows)
    return table + (f'<div class="cards">{cards}</div>' if cards else "")


def render_stats(d):
    e = html.escape
    days = d["days"]
    body = ""
    for r in d["rows"]:
        color = f"var(--c-{r['app']})" if r["app"] else "var(--gray)"
        swatch = f'<span class="sw" style="background:{color}"></span>' if r["app"] else ""
        last = r["series"][-1] if r["series"] else 0
        spark = sparkline(r["series"], color, f"{r['name']}: {last} active on {d['today']}")
        body += (f'<tr><td class="app">{swatch}{e(r["name"])}</td><td class="spark">{spark}</td>'
                 f'<td class="r">{r["active"]}{delta(r["active"], r["active_prev"])}</td>'
                 f'<td class="r">{r["new"]}{delta(r["new"], r["new_prev"])}</td>'
                 f'<td class="r">{r["crashes"]}{delta(r["crashes"], r["crashes_prev"])}</td></tr>')
    table = (f'<section class="card"><div class="tablewrap"><table><tr><th>App</th><th class="spark">Active per day</th>'
             f'<th class="r">Active, {days} days</th><th class="r">New</th><th class="r">Crashes</th></tr>{body}</table></div>'
             f'<p class="muted small">Active and new count installs; each is compared with the {days} days before. '
             f'An install that uses several apps is counted once in each app and once in All apps.</p></section>')
    cards = "".join(f'<section class="card"><h2>{e(title)}</h2>{rows}</section>' for title, rows in (
        ("Version", bar_list(d["versions"], label=lambda v: v or "(unknown)")),
        ("Operating system", bar_list(d["systems"], label=lambda v: OS_NAMES.get(v, v) or "(unknown)")),
        ("Language", bar_list(d["langs"][:8], label=lambda v: v or "(unknown)")),
        ("Top commands", bar_list([(c["feature"], c["installs"]) for c in d["commands"][:8]], wide=True))))

    cmds = d["commands"]
    crows = "".join(f'<tr><td>{e(c["feature"])}</td><td>{e(", ".join(f"{APPS[a]} {v[1]}" for a, v in c["apps"].items() if a in APPS))}</td>'
                    f'<td class="r">{c["times"]}</td><td class="r">{c["installs"]}</td></tr>' for c in cmds)
    allcmds = (f'<details><summary>Show all {len(cmds)} commands</summary><div class="card"><div class="tablewrap"><table>'
               f'<tr><th>Command</th><th>Installs by app</th><th class="r">Times</th><th class="r">Installs</th></tr>{crows}</table></div></div></details>'
               if cmds else "")
    crash_rows = "".join(f'<tr><td>{e(v)}</td><td class="r">{n}</td><td class="r">{i}</td></tr>' for v, n, i in d["crash_versions"])
    crashes = (f'<div class="tablewrap"><table><tr><th>Version</th><th class="r">Reports</th><th class="r">Installs</th></tr>{crash_rows}</table></div>'
               if crash_rows else '<p class="muted">No crashes in this period.</p>')
    period = chips([(f"{n} days", page_url("stats", None, n), n) for n in PERIODS], days, "Period")
    return f"""<!doctype html><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>JeffOffice usage</title>
<style>{STYLE}</style>
<main>
<div class="head"><h1>JeffOffice usage</h1><div class="right"><span class="muted small">Times are UTC</span>{period}</div></div>
{table}
{render_downloads(d)}
{render_map(d)}
<div class="cards">{cards}</div>
{allcmds}
<section class="card"><h2>Crashes by version, last {days} days</h2>{crashes}<p><a href="/crashes">The newest crash reports</a></p></section>
<footer>IP geolocation by <a href="https://db-ip.com">DB-IP</a>. Download counts from GitHub, Cloudflare, and the Snap Store. {geo_line(d.get("geo"))}<br>{FOOTER_LINKS}</footer>
</main>
{"".join(f'<script defer src="{u}"></script>' for u in ATLAS_SCRIPTS)}
<script>{MAP_JS}</script>
"""


def crashes_page(app=None):
    """The 100 newest reports (of one app, or of all); the ones that stopped in the same place share a row."""
    with db() as con:
        rows = con.execute("SELECT happened, version, os, kind, code, frames, app FROM crashes" + (" WHERE app = ?" if app else "") +
                           " ORDER BY id DESC LIMIT 100", (app,) if app else ()).fetchall()
    groups = {}
    for happened, version, os_name, kind, code, frames, a in rows:
        g = groups.setdefault((kind, code, frames, a), {"n": 0, "latest": happened, "versions": [], "systems": []})
        g["n"] += 1
        g["latest"] = max(g["latest"], happened)
        for key, value in (("versions", version), ("systems", os_name)):
            if value not in g[key]:
                g[key].append(value)
    e = html.escape
    name = APPS[app] if app else "All apps"

    def row(key, g):
        kind, code, frames, a = key
        shown = f"0x{code & 0xFFFFFFFF:08X}" if kind == "exception" else str(code)
        stack = "<br>".join(e(f"{f['module']}+0x{f['offset']:x}") for f in json.loads(frames))
        cells = [g["n"]] + ([] if app else [APPS.get(a, a)]) + [g["latest"], ", ".join(g["versions"]), ", ".join(OS_NAMES.get(x, x) for x in g["systems"]), kind, shown]
        where = f"<code>{stack}</code>" if stack else "<i>no frames</i>"
        return "<tr>" + "".join(f"<td>{e(str(c))}</td>" for c in cells) + f"<td>{where}</td></tr>"

    heads = ["Reports"] + ([] if app else ["App"]) + ["Latest", "Version", "System", "Kind", "Code", "Where it stopped"]
    body = "".join(row(k, g) for k, g in groups.items()) or f'<tr><td colspan="{len(heads)}">none yet</td></tr>'
    head = "".join(f"<th>{h}</th>" for h in heads)
    return f"""<!doctype html><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>JeffOffice crashes</title>
<style>{STYLE}</style>
<main>
<div class="head"><h1>{e(name)}: crashes</h1><span class="muted small">Dates are UTC. <a href="/stats">Back to the numbers</a></span></div>
<div style="margin-bottom:1rem">{app_chips("crashes", app)}</div>
<section class="card"><p class="muted">The newest 100 crash reports, with the ones that stopped in the same place on one row.</p>
<div class="tablewrap"><table><tr>{head}</tr>{body}</table></div></section>
<footer>{FOOTER_LINKS}</footer>
</main>
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
                self.send(401, b"Password needed", extra=[("WWW-Authenticate", 'Basic realm="JeffOffice usage"')])
                return
            # /crashes?app= names one app the collector knows (else all of them); /stats?days= is one of PERIODS (else 30).
            query = urllib.parse.parse_qs(url.query, keep_blank_values=True)
            asked, span = query.get("app", [None]), query.get("days", [str(DEFAULT_DAYS)])
            if len(asked) != 1 or (asked[0] is not None and asked[0] not in APPS) or len(span) != 1 or span[0] not in map(str, PERIODS) \
                    or (path == "/stats" and "app" in query):   # /stats shows every app; only /crashes takes ?app=
                self.send(404, b"Not found")
                return
            page = render_stats(gather_stats(int(span[0]))) if path == "/stats" else crashes_page(asked[0])
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
        record(ping, country=country_of(self.headers, self.client_address[0]))
        self.send(204)


def main():
    init()
    if len(sys.argv) == 3 and sys.argv[1] == "--forget":
        print(f"deleted {forget(sys.argv[2])} rows", flush=True)
        return
    threading.Thread(target=purge_daily, daemon=True).start()
    threading.Thread(target=geo_daily, daemon=True).start()
    threading.Thread(target=download_daily, daemon=True).start()
    port = int(os.environ.get("PORT", "8080"))
    server = ThreadingHTTPServer(("0.0.0.0", port), Handler)
    server.daemon_threads = True
    print(f"listening on {port}", flush=True)
    server.serve_forever()


if __name__ == "__main__":
    main()
