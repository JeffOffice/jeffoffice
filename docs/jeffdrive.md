# JeffDrive

JeffDrive keeps a folder the same on all of your computers, using storage you rent or run yourself. It works with any storage service that speaks the S3 protocol (the common language for storing files in "buckets," first used by Amazon and now offered by many companies and by programs you can run at home). There is no JeffDrive server and no JeffDrive account: the only things involved are your computers and your bucket.

JeffDrive is part of the JeffOffice family, but it is a separate download with its own version number. It works with no JeffOffice app installed.

This note is the design: what version 1 does, how it stores files, the order it is built in, and how each step is measured. It is a draft; nothing here is built yet.

## The decisions (November 27, 2026)

- **For one person with several computers.** Version 1 does not share folders between people. The bucket layout leaves room for that later without moving anything already stored.
- **Windows, macOS, and Linux.** The same three as the JeffOffice apps. Phones and a web page come later.
- **Files are stored in pieces.** Every file is cut into pieces, and each piece is stored under a name made from its contents (its "fingerprint"). Changing one page of a 500 MB file uploads only the pieces that changed, and two copies of the same file cost the space of one. The price is that the bucket cannot be read with other tools, so JeffDrive always includes an export tool (below), and this layout is published.
- **Encryption is on unless you turn it off.** Setup asks for a passphrase and scrambles every piece on your computer before it is uploaded. The storage company cannot read file contents, file names, or folder names. You may choose "no encryption" at setup instead.
- **Online-only files on Windows and macOS.** A file can show in the folder without taking disk space and download when you open it. Linux has no system feature for this, so on Linux you pick which folders to keep on the computer and browse the rest in a JeffDrive window.
- **History is kept 30 days, and you can change that.** Old versions of changed files and copies of deleted files can be brought back. Settings offers 7 days, 30 days, 1 year, or forever, and shows how much bucket space history uses.
- **Share links make a separate copy.** Sending someone a link uploads a whole, readable copy of that one file to a "shared" area of the bucket and gives you a link that stops working after a time you choose (7 days at most on most services). The person needs nothing installed. That copy is not encrypted, and JeffDrive says so before you share.
- **Six storage services are tested every release:** Amazon S3, Cloudflare R2, Backblaze B2, Wasabi, MinIO, and Garage (the last two are programs you run on your own server). Any other S3 service can be entered by hand.
- **Light ties to the JeffOffice apps.** When JeffDrive is installed, JeffWrite, JeffCrunch, JeffDeck, and JeffPub show a JeffDrive shortcut in Open and Save, a sync mark in the title bar, and "Version History" and "Share Link" in the File menu.
- **Free and open source,** under the same license as the JeffOffice apps. You pay only your storage company.

### Smaller choices made while writing this note

These were not asked one by one. Any of them can be overruled.

1. **Two computers change the same file before either hears of the other:** both versions are kept. The later one is saved beside the first as `Budget (changed on Laptop, 2026-11-27).xlsx`, and JeffDrive tells you. Nothing is ever merged or thrown away silently.
2. **A lost passphrase:** setup makes a recovery key (a long code) and offers to print it or save it as a file. The passphrase or the recovery key opens the drive. If both are lost, the files cannot be recovered by anyone, and setup says this in plain words before it continues.
3. **Changing the passphrase** does not upload anything again (see "Encryption" for why).
4. **The export tool** writes every file, or the whole drive as of a chosen day, to an ordinary folder. It is in the app and is also a command-line program, so it can be used years from now even if the app's window no longer runs.
5. **The bucket's access keys and the passphrase** are kept in the computer's own password store (the Windows Credential Manager, the macOS Keychain, the Linux Secret Service), never in a plain settings file.
6. **More than one drive** can be set up on a computer (for example "Home" and "Work"), each with its own folder and its own bucket or its own area of a bucket.
7. **The code lives in the JeffOffice repository** (`libs/drive` for the engine, `apps/jeffdrive` for the app) so it shares the build machines, the translations, and the look of the other apps, and is written in C++ with Qt like them. It has its own installer and its own release schedule.

## What version 1 does

- A JeffDrive folder on each computer. What you put in it appears on your other computers.
- A tray icon that says what JeffDrive is doing: up to date, syncing (with a count and a time left), paused, or a problem that needs you.
- Marks on files and folders in the system's file manager: synced, syncing, online-only, or a problem.
- Pause and resume. Limits on upload and download speed. An option not to sync on a metered connection or on battery.
- A list of names to leave out (for example `*.tmp`, `node_modules`, `Thumbs.db`), with sensible defaults.
- Version History for any file, and Deleted Files for any folder, each with Restore.
- "Restore the whole drive to how it was on a day," which is the remedy if a program (or ransomware) damages many files at once.
- Share Link and Stop Sharing.
- Online-only files (Windows, macOS) or a choice of folders (Linux), and a Free Up Space command.
- A "Test This Bucket" check at setup that proves the service does what JeffDrive needs before any file is uploaded.
- A running estimate of what the bucket costs per month at the service's listed prices, for the six tested services.
- Export, as described above.

### Not in version 1

Folders shared between people, phone apps, a web page for reaching files from any browser, online-only files on Linux, and copying directly between two computers on the same network. The layout below is chosen so none of these needs a new bucket format.

## How files are stored

The design rests on one rule: **no two computers ever write to the same object in the bucket.** A storage bucket has no referee. If two computers both replace the same object, one silently wins. Some services can refuse a write when the object changed since you last looked, but not all do, and they differ in the details. JeffDrive does not depend on that. Instead, everything in the bucket is one of three kinds, and each kind is safe by its nature:

| Kind | What it holds | Why two computers cannot collide |
|---|---|---|
| **Packs** | The pieces of files. Small pieces are bundled into packs of about 16 MB so that 100,000 small files are a few thousand uploads, not 100,000. | A pack's name is made from its contents. Two computers that upload the same pack upload the same bytes under the same name. |
| **Journals** | Each computer's own running list of what it did: "added this file, made of these pieces," "renamed," "deleted." | Each computer writes only under its own name (`journal/<computer>/<number>`). |
| **Shared copies** | The readable copies behind share links. | Each has a random name used once. |

A computer learns what the others did by reading their journals. Putting the journals together in order gives the current state of the drive, and also its state on any past day, which is how Version History and "restore to a day" work without any extra copies.

**Finding out that something changed.** A bucket cannot tell a computer that something is new, so each computer asks, by default every 30 seconds while it is in use and less often when it is idle. The question is one small request per other computer ("anything after entry number 4,812?"). Storage services charge for each request, so this is designed to a budget (see the bar below).

**Cutting files into pieces.** Cut points are chosen from the file's own contents, not at fixed distances, so inserting a paragraph near the start of a document shifts one piece and leaves the rest matching what is already stored. Pieces average about 1 MB.

**Cleaning up.** When history older than the limit is dropped, pieces that nothing refers to any more can be deleted. This is the one dangerous step in the design: one computer might delete a piece at the moment another computer starts using it for a new file. So cleanup takes two passes at least a week apart. The first pass only writes a list of pieces that look unused. The second pass deletes a listed piece only if every computer's journal has been read since and still none refers to it. A computer that has been off for longer than the wait checks its pieces again before it trusts them.

**Opening an online-only file** fetches only the pieces needed, in order, so a video starts playing before it has all arrived.

### Encryption

- Setup makes one random **drive key**. Every piece and every journal entry is scrambled with it, using a standard method that also detects any tampering. The scrambling code comes from a widely reviewed open-source library (libsodium); JeffDrive writes none of its own.
- The drive key itself is stored in the bucket, locked twice: once by your passphrase and once by the recovery key. That is why changing the passphrase is instant: only the small locked copy of the drive key is replaced, by a new object with a new name.
- Turning a passphrase into a lock uses a deliberately slow method (Argon2id), so that guessing passphrases by machine takes years even for short ones.
- With encryption on, a piece's name is made from its contents **and** the drive key. Otherwise the storage company could check whether you hold a known file by computing its fingerprint.
- **What the storage company can still see:** how much you store, roughly how many pieces, when you sync, and from which internet address. It cannot see names, contents, folder structure, or which pieces belong to which file.
- With encryption off, pieces and journals are stored as they are. The layout is otherwise the same, and a drive can be converted either way by uploading it again.

## Online-only files

- **Windows:** the system's cloud files feature, the one File Explorer already understands. Files show a cloud mark, download when opened, and can be set to "Always keep on this device."
- **macOS:** the system's File Provider feature. The drive appears under Locations in Finder. This needs the app to be signed and approved by Apple, which the JeffOffice apps already are.
- **Linux:** choose folders to keep on the computer. Everything else is listed in the JeffDrive window, where a double-click downloads and opens a file.

This is the most expensive part of version 1 and the hardest to test, because each system's feature has its own rules and its own failure modes. It is built last (step 6), after sync is proven without it.

## Storage services

Each of the six tested services gets a setup preset (so you paste two keys and pick a region, not a server address) and runs the full test suite before every release. For MinIO and Garage, the suite runs against a copy on our own build machine. For the four companies, it runs against a small test bucket at each.

"Test This Bucket" checks, in under a minute, that the service can: store and fetch an object, fetch part of an object, list by name, upload a large object in parts, make a time-limited link, and delete. If something is missing it says which, in plain words, and what will not work.

Before any service is named as supported on the website, its behavior is checked by test, not taken from its documentation.

## The version 1 bar

JeffDrive ships as a preview when it meets every line below. Each is measured by a program.

| Measure | Bar |
|---|---|
| **Never loses a file.** A test drives 2 to 5 pretend computers through 1,000,000 random steps (add, change, rename, move, delete, go offline, come back, lose the connection mid-upload, lose power mid-write). | Every run ends with all computers holding the same files, and every version that ever finished saving is in the folder or in history |
| **Stopped at any moment.** The program is killed at each of its write points in turn, then started again. | No damaged file, no lost file, and sync resumes without uploading finished pieces again |
| **Cleanup is safe.** The random test above, with cleanup running throughout and a computer that stays off past the waiting period. | No file ever refers to a missing piece |
| **Encryption.** Every object in a test bucket is examined. | No file name, folder name, or run of file contents appears anywhere; a changed byte in any object is detected and reported |
| **First upload:** 100,000 files, 50 GB. | Limited by the connection, not by JeffDrive: at least 90% of the measured line speed; under 20,000 requests |
| **Small change to a big file:** 1 MB changed in the middle of a 1 GB file. | Under 4 MB uploaded |
| **Idle cost.** Three computers, nothing changing, for a simulated month. | Under 10 cents a month in requests at each tested company's listed prices (to be confirmed against current price lists) |
| **Light on the computer.** A drive of 500,000 files, idle and then syncing. | Idle: no measurable processor use and under 150 MB of memory. Syncing: never more than half the processor cores unless you ask |
| **Noticing changes.** A saved file on one computer. | Starts uploading within 2 seconds; appears on another running computer within 60 seconds |
| **Online-only (Windows, macOS).** A 2 GB video opened from online-only. | Starts playing within 3 seconds on a 100 Mbit line; Free Up Space returns the disk space at once |
| **All six services.** | The whole suite passes against each |

## The order it is built in

1. **The engine with no network:** cutting, packs, journals, and putting journals together, with a folder on disk standing in for the bucket. The million-step test runs here first, because it is fastest here.
2. **Encryption,** including the recovery key and the passphrase change.
3. **Real buckets:** the six services, "Test This Bucket," resuming interrupted uploads, the request budget, and speed limits.
4. **The app:** setup, the tray icon, Settings, Version History, Deleted Files, restore to a day, Export, and cleanup.
5. **Share links,** file manager marks, and the ties in the four JeffOffice apps.
6. **Online-only files** on Windows, then macOS, and folder choice on Linux.
7. **Preview release.**

## Open questions

- **The name.** "JeffDrive" needs the same trademark check the other app names had.
- **Which S3 features each of the six services really has** (parts of an object, large uploads in parts, link lifetimes). Checked by test in step 3; the design above deliberately needs only the most basic ones.
- **Very large drives.** Putting journals together is fast for hundreds of thousands of files. For many millions, computers will need to write an occasional summary so a new computer does not read years of journals. The format reserves a place for this; it is not built in version 1 unless the tests show the need.
- **Files that are open and changing constantly** (a mail program's database, a virtual machine's disk). Version 1 waits until such a file has been still for a few seconds, and the default leave-out list covers the common cases.
