# JeffMail feature list

JeffMail is the email, calendar, contacts, and tasks program of JeffOffice, modeled on Outlook in Office 2021. It keeps your mail where your provider keeps it, so any other program can read the same mailbox, and it opens the files other programs make (`.eml`, `.mbox`, `.msg`, `.pst`, `.ics`, `.vcf`). It is not built yet: nothing exists except what the suite already does for every app, so `[x]` and `[~]` count only that and the rules that are already decided. This list is the whole job, laid out the way the original is (File, then each tab of the main window and of the windows for a message, an appointment, a contact, and a task, then what sits under the ribbon), so the work can be planned and tracked.

Last reviewed October 16, 2026.

Legend: `[x]` done · `[~]` partly there (the note says what's missing, or which shared JeffOffice part it builds on) · `[ ]` not yet · **(alt)** the usual way depends on a proprietary online service, so the app offers an open replacement.

Every item carries a tier:

- **first release**: needed in JeffMail's first public release.
- **1.0**: needed before JeffMail is called finished (version 1.0).
- **later**: wanted, but not before 1.0.

## How the tiers were chosen

**First release** is the first public build (a preview, as JeffDeck 0.7.0 is). A person can add an account from any standard provider, read, search, write, send, and file mail, keep a calendar and an address book in sync over the open standards, and keep a task list, and nothing in a message reaches out until the person allows it. **1.0** is what someone coming from the mail program in Office 2021 expects to find day to day. Version 1.0 is ready to ship when a person can move a working mailbox, calendar, and address book over (from the provider, or from a `.pst` file) and lose nothing they use. An item is **later** when it is used by few people, needs a piece the suite doesn't have yet, or depends on something outside the program (a company's registration with Microsoft, a server's support).

Every command in the original's ribbons was read from Microsoft's published control lists for the main window and for the five item windows (a message being written, a message being read, an appointment or meeting, a contact, and a task). The lists overlap heavily, so each feature appears once here, under the place a person looks for it first. There are no usage counts for mail, so the tiers follow what a person moving from the original needs on day one, then what a full replacement needs.

## Beyond Outlook

What JeffMail offers that the original doesn't, then the plans that go past it.

- [x] **first release** Free and open source (GPL-3.0): no license fee, no subscription, and no account with JeffOffice. JeffMail talks to your providers and to nobody else; the suite's update check and its opt-in statistics never carry the contents of a message, an event, or a contact
- [~] **first release** Runs on Linux and Windows, where the original ran only on Windows and Mac, and on macOS as the rest of the suite does *(shared: the suite's builds, installers, and AppImage; the macOS app is still a test build)*
- [~] **first release** Installs without administrator rights, or runs with no installation (Windows `.zip`, Linux AppImage) *(shared: the suite's packaging)*
- [ ] **first release** Open standards and any provider: IMAP and SMTP for mail, CalDAV for calendars and tasks, and CardDAV for contacts, so a Fastmail, iCloud, Nextcloud, Posteo, or company server works the same way, and leaving JeffMail costs nothing
- [ ] **first release** Your mailbox stays yours: for mail on a server, the server's copy is the real one and JeffMail's copy is a cache that can be thrown away and rebuilt; every message can be saved out as a plain `.eml` or `.mbox` file at any time
- [ ] **first release** Private by default: no remote picture, tracking pixel, or linked content loads until you allow it, and attachments never run (details under Safety and limits)
- [ ] **first release** Search that runs on your own computer over everything downloaded, offline and without a service (details under Search and folders)
- [ ] **first release** A "Proton Mail" account preset that finds Proton Mail Bridge on this computer (details under Accounts and sign-in)
- [~] **first release** Spelling works offline in 12 languages, with no cloud service *(shared: the bundled dictionaries in libs/base/text)*
- [ ] **first release** **(alt)** Grammar and style checking that runs on your own computer, in place of Microsoft's online Editor: Harper, English first *(shared with JeffWrite)*
- [ ] **1.0** One inbox for all accounts: a combined Inbox, Sent, Drafts, and Flagged view across every account, which the original doesn't have
- [ ] **1.0** One-click Unsubscribe on mailing-list messages, using the sender's own unsubscribe header, with a note of what will be sent before it is sent
- [ ] **1.0** Undo Send: a message waits a few seconds in the Outbox after Send (you choose how long) and can be pulled back
- [ ] **1.0** Safer sending: a warning before sending when the text says something is attached and nothing is, when the subject is empty, or when a reply would go to a very large list
- [ ] **1.0** OpenPGP as well as S/MIME for signing and encrypting, with keys found through Web Key Directory and Autocrypt headers (details under Safety and limits)
- [ ] **1.0** A Mail Merge hand-off: JeffWrite and JeffPub send their email merges through JeffMail's accounts instead of writing one `.eml` file per recipient
- [~] **1.0** Help that works offline, with a search that also finds commands and runs them *(shared: libs/shell help)*
- [ ] **1.0** `jmailtool`, a command-line tool that converts and checks mail files (`.eml`, `.mbox`, `.msg`, `.pst`, `.ics`, `.vcf`), verifies the local cache against the server, and runs the fidelity and hostile-file tests, as `jpubtool` and `jdecktool` do for the other apps *(shared: the pattern of those tools)*
- [ ] **later** Snooze: hide a message until a day and time you pick, then bring it back to the top of the Inbox
- [ ] **later** **(alt)** A local replacement for Focused Inbox: an "Other" view that sorts mail by who you write to, your contacts, and mailing-list headers, learned on your computer with nothing sent anywhere
- [ ] **later** JMAP (the newer open mail protocol that Fastmail and some others offer) as a faster alternative to IMAP

## File (backstage)

### Info
- [ ] **first release** Account Information: each account with its kind, servers, and status, with Add Account and Account Settings *(shared: the backstage page of libs/shell)*
- [ ] **1.0** Cleanup Tools: the size of each folder and of the whole local copy, the account's quota where the server reports it, a list of the largest messages, a way to empty Deleted and Junk, and archiving of old mail
- [ ] **1.0** Rules and Alerts opens the rules (details under Rules, filters, and junk mail)
- [ ] **1.0** Automatic Replies opens the out-of-office settings (details under Rules, filters, and junk mail)

### Open and export
- [ ] **first release** Open a single `.eml`, `.ics`, or `.vcf` file (double-click, drag, or File > Open) to read it without importing it: a message in its own window, an event or contact with Add to Calendar or Add to Contacts
- [ ] **first release** Import and Export in one place, with a preview of what will be added and a count when it finishes: `.eml` (one or many files), `.mbox` and Maildir folders (Thunderbird, Apple Mail, and Unix mail), `.vcf`, `.csv` contacts with column matching, and `.ics`
- [ ] **1.0** Import `.msg` files (the original's single-item files) and export an item as `.msg` *(shared: the compound-file reader and writer in libs/base/io)*
- [ ] **1.0** Import `.pst` and `.ost` data files read-only: mail with folders, flags, categories, and attachments; appointments and recurrences; contacts and groups; tasks; and notes, from old and new versions, including password-protected files. JeffMail never writes a `.pst`
- [ ] **1.0** Export a whole account or a chosen set of folders to a folder of `.eml` or `.mbox` files plus `.ics` and `.vcf`, so a person can always take everything away
- [ ] **1.0** Export the settings (accounts without passwords, rules, signatures, categories, and views) and import them on another computer
- [ ] **later** Import Apple Mail (`.emlx`) mailboxes and the original Mac program's archive (`.olm`), and RSS/Atom feed lists (OPML)
- [ ] **later** Open another person's shared mailbox when the server offers one (IMAP shared folders)

### Save As and Print
- [ ] **first release** Save a message as `.eml` (with every header and attachment as received), `.txt`, or `.html`; Save All Attachments; Save a calendar or a single event as `.ics`
- [~] **first release** Print a message, event, contact, or task with Print Preview, paper, margins, and orientation *(shared: JeffPub's printing and libs/shell)*
- [ ] **1.0** Save a calendar for a chosen range of dates, with a choice of detail (availability only, limited details, or full details) and whether private items are included
- [~] **1.0** Print styles for mail and tasks: a table of the list as shown, and a memo with the header and body, optionally printing attachments and starting each item on a new page *(shared: libs/page and the PDF writer)*
- [ ] **1.0** Print styles for the calendar: day, week, and month, with a small month beside them and a choice of hours and details
- [~] **1.0** Save as PDF from any print *(shared: JeffPub's PDF export)*
- [ ] **later** More print styles: tri-fold and a plain list of calendar details, and for contacts card, booklet, and phone-directory styles; and a way to define your own

### Options
- [~] **first release** General: user name, display language, light or dark interface, starting module, usage statistics (opt in), and update checks *(shared: libs/shell settings)*
- [ ] **first release** Mail, Calendar, People, and Tasks, each with its own page: format for new messages (HTML or plain text), fonts, signatures, reply and forward layout, when a message counts as read, and how often to check; work hours and work week, first day of the week, week numbers, default reminder, time zones, and holidays; how names are filed, the default address book, and the autocomplete list; and default task reminders and overdue colors
- [ ] **first release** Security and privacy in one page, with every default visible (remote content, tracker blocking, attachments, links, read receipts), so no setting is hidden (details under Safety and limits)
- [~] **first release** Language: editing and proofing languages and the display language, in seven languages for the program and its Help *(shared: libs/shell i18n and the dictionaries)*
- [~] **first release** Proofing: spelling as you type, ignored words, custom dictionaries, and AutoCorrect, shared with the editor *(shared: JeffPub's Proofing options)*
- [ ] **1.0** Search: which accounts and folders are indexed, and the status of the index
- [ ] **1.0** Accessibility: high contrast, text size, a screen-reader-friendly list layout, and no motion
- [ ] **1.0** Customize the Ribbon and the Quick Access Toolbar, and customize the keyboard (assign keys to commands, Quick Steps, and signatures)
- [ ] **1.0** Advanced: where the local copy is kept and how large it may grow, the sound and look of reminders, the proxy to use, and the log of what the program sent and received (with passwords hidden)

## Accounts and sign-in

### Adding an account
- [ ] **first release** Add an account from an email address: JeffMail finds the servers by its own list of providers, then the domain's published settings file (the Mozilla autoconfig format), then DNS service records for mail submission and IMAP, then Microsoft's autodiscover for the domain, then a guess from the domain's mail servers; if all fail, it asks for the settings
- [ ] **first release** Manual setup: incoming and outgoing server names, ports, connection security (encrypted from the start, or upgraded with STARTTLS), user name, and sign-in method, with separate sign-in for outgoing mail if the server needs it
- [ ] **first release** Test the settings: tries to connect, sign in, and send to the account's own address, and says in plain words which step failed and why (wrong password, certificate problem, port blocked, IMAP turned off by an administrator)
- [ ] **first release** Provider presets that need no technical settings: Gmail, Outlook.com and Microsoft 365, Yahoo, iCloud, Fastmail, AOL, GMX and mail.com, Zoho, mailbox.org, Posteo, and Proton Mail (a starting list; more are added from the open provider database)
- [ ] **first release** The same step finds the account's calendars and address books on the open standards (a well-known address or DNS records for CalDAV and CardDAV) and offers to add them
- [ ] **first release** Several accounts, each with its own name and color; several identities per account (aliases, with their own names and signatures); a default account for new messages; replies go out from the address the message was sent to
- [ ] **first release** Remove an account: asks whether to keep the local copy, and deletes the saved passwords and tokens
- [ ] **1.0** Account Settings: change the password, servers, ports, names, signature, which folders mean Sent, Drafts, Deleted, Junk, and Archive, how often to check, and how much to keep on this computer, with Test again
- [ ] **1.0** Reorder accounts in the folder pane, and move an account's data to another folder on disk
- [ ] **later** Bring accounts in from other mail programs: servers, names, and signatures from Thunderbird and from the original program on Windows (passwords are never copied; each is asked for once)

### Signing in
- [ ] **first release** OAuth 2.0 sign-in in the person's own web browser (loopback address and PKCE, as RFC 8252 describes), never an embedded browser, for providers that offer it: Google, Microsoft, Yahoo, and AOL to start; the resulting token signs in to IMAP and SMTP
- [ ] **first release** Outlook.com and Microsoft 365 mail over IMAP and SMTP with Microsoft's OAuth sign-in, with a clear message when a Microsoft 365 organization has turned IMAP off or wants its administrator to approve the sign-in
- [ ] **first release** App passwords where a provider requires them (iCloud, Fastmail, and any account with two-step verification): the setup page says in plain words where to create one for this provider
- [ ] **first release** Password sign-in for ordinary servers over an encrypted connection, with the strongest method the server offers negotiated automatically
- [ ] **first release** Passwords and tokens are kept in the system's secret store (Windows Credential Manager, macOS Keychain, or the Secret Service on Linux), never in a plain file; tokens refresh without asking, and a sign-in that has expired or been revoked asks again at once and says so
- [ ] **first release** A certificate the system doesn't trust (self-signed or expired) shows its details and fingerprint, and the person may trust that exact certificate for that exact server; nothing is ever trusted silently
- [ ] **first release** The app registrations at Google and at Microsoft that OAuth sign-in needs (publisher verification, and Google's review for mail access) *(outside the program; see the open questions)*
- [ ] **1.0** Sign in with a client certificate for company servers
- [ ] **later** Kerberos and NTLM sign-in (GSSAPI) for company servers

### Protocols
- [ ] **first release** IMAP, with push (IDLE), fast resync of changes (CONDSTORE and QRESYNC), MOVE, SPECIAL-USE folder names, and the Gmail extensions (labels shown as folders without duplicating messages); each is used only if the server offers it, with polling as the fallback
- [ ] **first release** SMTP submission with sign-in, large-message limits, international addresses, and delivery notices; a copy goes to Sent through IMAP unless the server files it itself (as Gmail does), so there are never two
- [ ] **first release** CalDAV for calendars and tasks and CardDAV for contacts: discovery, change tokens and ETags for quick sync, edits made offline queued, and a conflict never overwrites silently
- [ ] **first release** Proton Mail through Proton Mail Bridge (Proton's own local IMAP and SMTP program, on paid plans): the "Proton Mail" preset looks for Bridge on this computer, asks for Bridge's own user name and password (not the Proton password), trusts Bridge's local certificate for this computer only, and explains in plain words when Bridge isn't running or isn't installed
- [ ] **first release** Proton calendars and contacts can't be synced (Proton offers no open standard for them), and the preset says so; a one-time import of the `.ics` and `.vcf` files Proton exports is offered instead. JeffMail never uses Proton's unofficial API
- [ ] **1.0** POP3, with options to leave messages on the server for a number of days or until deleted here, for older providers and archive accounts
- [ ] **later** Microsoft Graph, the only way to reach Outlook.com and Microsoft 365 calendars, contacts, and tasks: after JeffOffice's app registration and publisher verification (details under Calendar, People, and Tasks)
- [ ] **later** Exchange Web Services, for Exchange servers a company runs itself (Microsoft has announced its retirement from Exchange Online, so it would serve only those)
- [ ] **later** LDAP directories, for looking up addresses in a company's address book
- [ ] **later** ManageSieve, to read and edit a server's own mail filters (details under Rules, filters, and junk mail)

## Mail

The main window, then reading a message, then writing one. Everything that file, filter, or find mail across the whole program is under Search and folders and Rules, filters, and junk mail.

### The main window: Home tab
- [ ] **first release** New Email, and the New Items menu (email, appointment, meeting, contact, contact group, task, and note) from every module
- [ ] **first release** Delete (to the account's Deleted folder, with Undo), and Delete forever (Shift+Delete, after a confirmation); Archive in one key, moving the message to the account's Archive folder
- [ ] **first release** Reply, Reply All, and Forward, with the replied and forwarded marks kept on the server so every program sees them
- [ ] **first release** Mark as read and as unread; Mark All as Read for a folder
- [ ] **first release** Move to a folder (a list of recent folders, a search over all folders, and Other), Copy to a folder, and drag and drop between folders and between accounts
- [ ] **first release** Flag a message for follow up; the server keeps the flag too (dates and reminders come with Follow Up below)
- [ ] **first release** Filter Email: Unread, Has Attachments, Flagged, Important, Sent To Me, Categorized, and a date (today, this week, and so on), above the list
- [ ] **1.0** Follow Up flags with a date: Today, Tomorrow, This Week, Next Week, No Date, and Custom; Add Reminder; Mark Complete; and Clear. A flagged message appears in the To-Do list with its dates *(the dates stay on this computer if the server can't keep them)*
- [ ] **1.0** Categorize: color categories (the original's six, renamable, with more added), a shortcut key for each, and a quick-click category; the same categories apply to mail, events, contacts, and tasks, and travel with the item as far as the provider allows (IMAP keywords, iCalendar and vCard categories)
- [ ] **1.0** Quick Steps: one-click sets of actions that you build (move to a folder, categorize, flag, mark read, delete, archive, reply with a template, forward to someone, create a task or meeting), with a shortcut key for each and a manage dialog
- [ ] **1.0** Ignore a conversation: this thread and its future messages go straight to Deleted
- [ ] **1.0** Clean Up a conversation, a folder, or a folder and its subfolders: removes messages whose text is fully quoted in a later reply, after showing how many and which, with a choice of where they go
- [ ] **1.0** Always Move Messages in This Conversation, and Create Rule from a message (always file mail from this sender or to this list in a folder)
- [ ] **1.0** Reply with Meeting: a new invitation to the people on the message, with its text in the notes
- [ ] **1.0** Reply All with the original attachments, and Forward as Attachment (the whole message as an `.eml` inside a new one)
- [ ] **1.0** Resend a message you sent, and Edit Message to change a received message's text in your own copy
- [ ] **1.0** Junk menu: block the sender, never block the sender or the domain, mark as not junk (details under Rules, filters, and junk mail)
- [ ] **1.0** Night mode for reading: a message shows light text on a dark background, separately from the dark interface
- [ ] **later** Read Aloud with the system's voices *(shared with JeffWrite and JeffDeck)*
- [ ] **later** Move a message to a note-taking program: not offered, because the original sends it only to OneNote

### Send/Receive tab
- [ ] **first release** Send/Receive All (F9), Update Folder, Send All for the Outbox, Cancel All, and Show Progress (a list of every account and folder with its state, speed, and errors)
- [ ] **first release** Work Offline: reading what is downloaded, writing, and queuing, with the Outbox sending when the connection returns
- [ ] **1.0** Send/Receive Groups: which accounts and folders are checked together, how often (every few minutes, at start, at exit, or only by hand), and whether a group is checked while offline
- [ ] **1.0** Download Preferences: headers only, headers and then full messages, or full messages; per folder, for how long back; and Download Headers, mark a message or folder to download, and Process Marked Items
- [ ] **1.0** Update Folder also refreshes calendars and address books, and a Download Address Book button forces a full CardDAV resync
- [ ] **1.0** Push for the Inbox and chosen folders through IDLE, with polling for folders and servers that can't, so new mail appears at once

### Folder tab
- [ ] **first release** New Folder anywhere in an account's tree; Rename, Move, Copy, and Delete a folder; Sort folders A to Z; Favorites, to pin any folder (or search) at the top of the pane
- [ ] **first release** Empty Folder and Delete All for Deleted and Junk
- [ ] **1.0** Folder Properties: name, what it holds, unread and total counts, whether it is kept on this computer, and its retention settings
- [ ] **1.0** IMAP Folders: choose which of the server's folders are shown (subscribe and unsubscribe), and update the list
- [ ] **1.0** Run Rules Now, with the choice of rules and folder (details under Rules, filters, and junk mail), and Recover Deleted Items (messages the server still holds, and the account's recovery folder where there is one)
- [ ] **1.0** AutoArchive: move mail older than a number of months to an archive folder, on the server or on this computer, with the policy set per folder
- [ ] **1.0** New Search Folder (details under Search and folders)
- [ ] **1.0** Open a folder in a new window, and Go to Folder (Ctrl+Y) with a quick list of recent ones
- [ ] **later** Share a folder with other people and open shared mailboxes, where the server supports sharing (IMAP access rights)
- [ ] **later** Folder colors and icons

### View tab and the layout of the window
- [ ] **first release** Layouts for the message list: Compact (one line), Single (two lines), and Preview (a few lines of each message), with the preview off, one, two, or three lines
- [ ] **first release** Reading Pane on the right, at the bottom, or off; a message marked read when selected, after a delay you set, or never; and the space bar to page through a message and move to the next
- [ ] **first release** Folder pane normal, minimized, or off; Favorites on or off; and the navigation bar at the bottom or the side with Mail, Calendar, People, and Tasks, with Ctrl+1 to 4 to switch
- [ ] **first release** Show as Conversations: messages of one thread grouped by their reply headers (with the subject and date as a fallback), expandable, with options to show senders above subjects, to always expand, to include the thread's messages from other folders, and the old indented look
- [ ] **first release** Arrange By: date, from, to, category, flag status, size, subject, type, attachments, account, and importance, ascending or descending, with grouping headers (Today, Yesterday, Last Week, and so on) that expand and collapse
- [ ] **first release** Light and dark interface, following the system *(shared: libs/shell theme)*
- [ ] **1.0** View Settings: choose, order, and size the columns (Add Columns and a field chooser), Group By, Sort, Filter, fonts and grid lines, Conditional Formatting (color a message by a rule, such as unread from a person), and Format Columns
- [ ] **1.0** Save a view, Apply a view to other folders, Reset a view, and Manage Views (define, rename, and delete)
- [ ] **1.0** To-Do Bar on the side with the next appointments, tasks, and a small month; and Peeks that show the calendar, a contact, or tasks when the pointer rests on the navigation bar
- [ ] **1.0** Quick actions that appear when the pointer rests on a message (delete, archive, flag, mark read, and move), set to the ones you choose
- [ ] **1.0** Reminders window, Open in New Window, and Close All Items
- [ ] **1.0** Text size and density (cozy, compact) for the lists, apart from the message text size
- [ ] **later** People pane under a message: the sender's card with recent mail and events, from your own data only

### Reading a message
- [ ] **first release** A header block with the sender's name and address both visible, To, Cc, date, subject, and the full list of recipients one click away
- [ ] **first release** The body shown from the message's own formatting after cleaning (no scripts, no forms, no frames, no plug-ins, no auto-redirects, no outside style sheets), or as plain text when the person prefers
- [ ] **first release** Pictures in the message held back (a bar says so, with Load pictures for this message) until allowed; pictures inside the message itself show at once
- [ ] **first release** Links show their real destination when the pointer rests on them, and a link whose text names one address and whose target is another says so before it opens in the system browser; a mailto: link opens a new message
- [ ] **first release** Attachments listed with icons, names, and sizes; Open, Save As, Save All, and drag to the desktop (details under Safety and limits for what Open means)
- [ ] **first release** Messages attached to messages open inside the program; a delivery-failure notice and a read-receipt notice display in plain words
- [ ] **first release** A meeting invitation shows as a card with the time, place, attendees, and conflicts with the calendar, and Accept, Tentative, Decline, and Propose New Time buttons
- [ ] **first release** A vCard attachment shows as a contact card with Add to Contacts
- [ ] **first release** Next and Previous message arrows, and Open in its own window with the Message and Help tabs
- [ ] **first release** Message Header and View Source in the Actions menu: every header and the raw text of the message, for troubleshooting
- [ ] **first release** Zoom a message and choose proportional or fixed-width text
- [ ] **1.0** Find in the message, and Find Related: all messages from this sender, and all in this conversation
- [ ] **1.0** Change the message's character set when a sender's program labeled it wrongly, and Unwrap Text to repair plain-text lines broken in the middle
- [ ] **1.0** Hovering over a name shows a contact card (address, phone, recent messages, and next shared event from your own data), with Add to Contacts in one click
- [ ] **1.0** When a sender asked for a read receipt, JeffMail asks before sending one (always, never, or ask), and says who asked
- [ ] **1.0** Select text and copy it with its formatting, and Select All
- [ ] **later** View in Browser: a cleaned copy of the message opens in the system browser, only when you ask
- [ ] **later** **(alt)** Translate a message or a selection with a model that runs on your own computer, if a small enough open one exists; no cloud translation

### Writing a message: Message tab
- [ ] **first release** Send, with the From picker for the account and identity; a draft saved automatically every few minutes in the account's Drafts folder; and Close asking to keep or discard *(shared: AutoRecover keeps unsent text after a crash)*
- [ ] **first release** To, Cc, and Bcc boxes with completion from contacts, recent recipients, and the autocomplete list; Check Names (resolves each name, and asks when a name matches two people); Address Book to pick from; and contact groups expanded, or kept as one name until Send
- [ ] **first release** Show Bcc and Show From toggles
- [ ] **first release** Attach File by button, drag, or paste, with a list of recent files, a remove button, and the size shown against the account's limit
- [ ] **first release** Signatures: several per account, one default for new messages and another for replies and forwards, text or formatted with pictures, placed above or below the quoted text, and edited in the same editor
- [~] **first release** Basic text from JeffWrite's editor: font, size, bold, italic, underline, strikethrough, color, highlight, bulleted and numbered lists, alignment, indents, text direction, Clear Formatting, Format Painter, and the paste choices (keep source formatting, merge, or text only) *(shared: JeffWrite's editor)*
- [~] **first release** Spelling as you type, the Spelling and Grammar dialog (F7), and the language of the selection, with Harper for English grammar *(shared: libs/shell spelling; Harper shared with JeffWrite)*
- [ ] **first release** The message body is written as clean HTML that other mail programs show the same way (tables and plain styles, not floating objects), with a plain-text twin sent beside it, or as plain text only; plain text wraps lines in the format other programs repair
- [ ] **first release** Reply and forward layout: reply above or below the quote, an attribution line in the message's language, the quote marked by indent or by a prefix, and the choice to include, attach, or leave out the original
- [ ] **first release** Reply inline in the reading pane, and Pop Out into a window of its own
- [ ] **1.0** Attach Item: another message (as `.eml`), a contact (as a vCard), or an event (as `.ics`); Business Card (a contact's vCard plus a short text); and Insert Calendar, a summary of free and busy times for a range of dates
- [ ] **1.0** Importance High and Low, and Sensitivity (Normal, Personal, Private, Confidential) as the headers other programs show
- [ ] **1.0** Styles, tables, and a shared Format Text tab: Format as HTML or Plain Text, and Paste Special *(shared: JeffWrite's editor)*
- [ ] **later** A Dictate button, if JeffWrite settles on an offline speech model *(shared with JeffWrite)*

### Writing a message: Insert tab
- [~] **first release** Insert pictures from a file, which travel inside the message and are scaled down to a size you choose; a link (Ctrl+K); symbols; a horizontal line; and the date and time *(shared: JeffWrite's editor)*
- [~] **1.0** Tables: insert, draw, quick tables, and convert text to a table, in a form other mail programs show *(shared: the table engine)*
- [~] **1.0** **(alt)** Online Pictures from Openverse and Wikimedia Commons, keeping the credit, in place of Bing *(shared: JeffPub's online pictures)*
- [~] **1.0** Icons from the suite's open icon set *(shared: JeffPub's icon picker)*
- [ ] **1.0** Quick Parts and AutoText: reusable pieces of text and layout, saved from a selection *(shared: the building-blocks gallery, to be built once)*
- [ ] **1.0** Strip location and camera data from attached pictures, and hidden data from attached Office files, before sending, after asking
- [ ] **later** Shapes, charts, SmartArt, equations, and 3-D models are inserted as pictures that look the same everywhere, not as live objects
- [ ] **later** Screenshot and Screen Clipping, where the system allows the program to capture the screen
- [ ] **later** Text boxes inserted as a bordered table cell, which is how other mail programs show them
- [ ] **later** Draw tab: pens and highlighters, kept as a picture *(shared: the ink layer, to be built)*

### Writing a message: Options tab
- [ ] **first release** Request a delivery notice (the server's own, where it supports one) and a read receipt (a request in the headers that the recipient's program may ignore), with a note that neither is certain
- [ ] **1.0** Sign and Encrypt buttons for S/MIME and OpenPGP, with a "can everyone receive this encrypted" check *(details under Safety and limits)*
- [ ] **1.0** Delay Delivery: hold the message in the Outbox until a date and time (works only while JeffMail is open, and says so)
- [ ] **1.0** Direct Replies To: another address for replies; and Save Sent Item To: a folder of your choice, or not at all
- [ ] **1.0** Character set for outgoing mail (UTF-8 by default, with the older sets when a recipient's program needs one)
- [ ] **1.0** Large attachments: a warning when the message passes the account's size limit, an offer to shrink pictures or to compress the files into one, and no cloud storage link
- [ ] **1.0** Themes, colors, fonts, and a page color for the message body, written as plain styles
- [ ] **later** Per-recipient choices remembered: send plain text only, always sign, or always encrypt
- [ ] **later** Send Later through a server that holds the message (JMAP or SMTP future release), so the program needn't stay open

### Writing a message: Review and other tabs
- [~] **1.0** Thesaurus offline *(shared: JeffPub's thesaurus)*
- [~] **1.0** Word Count *(shared: JeffWrite's count)*
- [ ] **1.0** Accessibility Checker for a message: missing picture descriptions, tables without header rows, and low contrast *(shared: JeffWrite's checker)*
- [ ] **1.0** Set Language and language preferences for the message *(shared: libs/base/text)*
- [ ] **later** Read Aloud for the message being written *(shared with JeffWrite and JeffDeck)*
- [ ] **later** Contextual Picture, Table, Shape, and Text Box tabs on the editor's formats; the picture and table tabs come with the editor, and the others only when shapes are inserted as live objects *(shared: JeffWrite's editor)*

### Keyboard and menus
- [~] **first release** KeyTips and the ribbon by keyboard, with a visible focus ring *(shared: libs/shell)*
- [ ] **first release** The original's keyboard shortcuts for the common commands (new message, reply, reply all, forward, send, mark read and unread, move, flag, search, go to a module, new appointment, contact, and task, and send/receive)
- [ ] **1.0** Quick Access Toolbar *(shared: libs/shell)*

## Calendar

Calendars come from CalDAV accounts, from files, from internet addresses, and from this computer. Microsoft's own calendars (Outlook.com and Microsoft 365) can only be reached through Microsoft Graph, which is a later release; until then a published `.ics` address from such a calendar can be followed read-only.

### Looking at the calendar
- [ ] **first release** Day, Work Week, Week, and Month views, with Today, Next 7 Days, Go to Date, forward and back, and a small month on the side for jumping
- [ ] **first release** All-day and multi-day events across the top of the day and week views; events that cross midnight; and a time scale of 5 to 60 minutes
- [ ] **first release** Work hours and work week shaded, the first day of the week, and week numbers, all from Options
- [ ] **first release** Several calendars at once: show or hide each, one color for each, a default calendar for new items, and each shown from its own account
- [ ] **1.0** Schedule View (days side by side as a horizontal timeline), and a list of upcoming events
- [ ] **1.0** Overlay: lay several calendars over each other in one view and see which is which by color
- [ ] **1.0** Two time zones side by side, and an event's own time zone shown when it differs from yours
- [ ] **1.0** Peek at the calendar from anywhere by resting the pointer on its button, and the calendar in the To-Do Bar
- [ ] **1.0** Holidays and special days for a country, from built-in open data, with no download from a service
- [ ] **1.0** Birthdays and anniversaries from the contacts, shown as events that never need editing
- [ ] **later** Calendar groups: a named set of calendars shown together

### Appointments and events
- [ ] **first release** Create an appointment: subject, place, start and end (with an all-day choice and the time zone), notes written in the shared editor, a reminder, how the time shows (free, tentative, busy, out of office, or working elsewhere), categories, private, and importance
- [ ] **first release** Create by clicking a time slot and typing, or by dragging across a span; move and resize by dragging; copy by dragging with Ctrl held; and open by double-click
- [ ] **first release** Recurrence: daily, weekly, monthly (a date or the nth weekday), and yearly, every so many, ending after a count, by a date, or never; change just one occurrence or the whole series, delete one occurrence, and open either from a right-click; patterns and exceptions from other programs are kept exactly
- [ ] **first release** Reminders as alarms at a set time before: a window or notification, a sound, snooze for 5 minutes to several days, and dismiss; the alarm travels with the event on CalDAV
- [ ] **first release** Time zones done correctly: the standard time-zone database inside the program (and updated with it), the zone names from files made on Windows understood, and events keep their meaning across daylight saving changes and travel
- [ ] **first release** Subscribe to an internet calendar by its address (`webcal:` or `https:` `.ics`), read-only, refreshed on a schedule you set, including the published address that Outlook.com and Microsoft 365 calendars can offer
- [ ] **first release** Import and export `.ics` files, and open one from an email to add it
- [ ] **first release** CalDAV calendars: add, discover, sync, create, edit, and delete, with offline changes queued and conflicts shown (details under Accounts and sign-in)
- [ ] **1.0** Attachments to an event, kept with it where the server supports it
- [ ] **1.0** Drag a message onto the Calendar button to start an appointment with the message's text and attachments, and drag onto Tasks or People for a task or a contact
- [ ] **1.0** Send Calendar by email: a chosen range as an `.ics` file in a new message
- [ ] **1.0** Copy an event from a read-only calendar into one of yours (Add to My Calendar)
- [ ] **1.0** Print the day, week, or month (details under File)

### Meetings and invitations
- [ ] **first release** Receive an invitation as a card in the message and in the calendar as tentative, with Accept, Tentative, and Decline, each with or without a comment and with or without sending a reply; for a series, answer one occurrence or all
- [ ] **1.0** Conflicts: an overlapping event is flagged as you accept or create an event
- [ ] **1.0** Create a meeting: add required, optional, and resource attendees from contacts, send the invitation (as an email carrying the invitation, or through the CalDAV server's own scheduling where it has one), and send updates when the time, place, or notes change
- [ ] **1.0** Scheduling Assistant: attendees' free and busy times in a grid, suggestions for a time everyone is free, and a plain "no availability published" for people whose server shares none
- [ ] **1.0** Tracking: who accepted, who is tentative, who declined, and who hasn't answered, as a table and as a count in the header; Request Responses on or off
- [ ] **1.0** Propose New Time (a counter-proposal), Allow New Time Proposals, and Allow Forwarding; the organizer accepts a proposal with one click
- [ ] **1.0** Cancel a meeting, one occurrence, or the series, notifying attendees; Invite More People to one occurrence or the series; and Reply to All Attendees with a message
- [ ] **1.0** Forward an invitation as an `.ics` file to someone outside the meeting
- [ ] **1.0** **(alt)** Online meeting links without a vendor: Add a Video Link inserts a link from the service the person names (Jitsi, Zoom, Google Meet, or Teams), puts it in the place and notes, and shows a Join button when the event is near; JeffMail doesn't host meetings
- [ ] **1.0** Resource calendars (rooms and equipment) where a CalDAV server offers them, as attendees of the "resource" kind
- [ ] **later** Room Finder: a list of free rooms for a time, where the server publishes room lists
- [ ] **later** Linked meeting notes: Take Meeting Notes opens a JeffWrite document tied to the event **(alt)** in place of OneNote

### Sharing and other services
- [ ] **later** Share a calendar with other people and set their permissions, where the CalDAV server offers sharing (each server does this its own way)
- [ ] **later** Publish a calendar as a read-only `.ics` address on a WebDAV or CalDAV server you control
- [ ] **later** Microsoft calendars for Outlook.com and Microsoft 365 through Microsoft Graph, after JeffOffice's app registration and publisher verification *(decided; see Decided)*

## People (contacts)

### Address books and the list
- [ ] **first release** Address books from CardDAV accounts, from files, and on this computer, each shown on its own and searchable together; the default for new contacts is set in Options
- [ ] **first release** Views of the list: Cards, Business Cards (a table), Phone, and List, sorted by file-as name, with an alphabet index, and a filter by category or company
- [ ] **first release** Find a contact by typing in the box at the top of every module, searching name, company, address, phone, and notes across all address books
- [ ] **first release** CardDAV sync: discover, add, change, delete, with offline changes queued and a conflict never overwriting silently (details under Accounts and sign-in)
- [ ] **first release** Autocomplete when addressing: contacts first, then people you have written to (remembered, ranked by how often and how recently), with a way to forget a suggestion

### The contact
- [ ] **first release** A contact holds: title, first, middle, and last name, suffix, nickname, file-as; company, job title, and department; several email addresses and phone numbers each with a label; home, work, and other postal addresses, one marked as the mailing address; web pages; instant-message addresses; birthday and anniversary; spouse, manager, and assistant; notes; categories; a private mark; and a picture
- [ ] **first release** All fields shown on one page (Details), so nothing the original keeps is hidden, and fields the program doesn't know are kept untouched when saved
- [ ] **first release** Save and Close, Save and New, Delete, and New Contact from the Same Company
- [ ] **first release** Add the sender of a message to contacts in one click; Add to Contacts from a vCard attachment
- [ ] **first release** Send a contact by email as a vCard (Forward Contact)
- [~] **first release** vCard 3.0 and 4.0 read and written, and 2.1 read, with inline photos and extension fields round-tripping *(shared: JeffPub's vCard reader, to be moved into a shared library)*
- [ ] **1.0** Communicate: New Message, New Meeting Request, and New Task for a contact, from the contact or its right-click menu

### Groups and tools
- [ ] **1.0** Contact groups (the original's distribution lists): create, add members from contacts or by typing an address, remove, update, notes, send to the group, and expand in the To box; stored on the server as vCard groups where the server supports them, and kept on this computer, with a note, where it doesn't
- [ ] **1.0** Duplicates: adding a contact that looks like one you have offers to merge or update it, and a Merge Duplicates tool covers the whole address book
- [ ] **1.0** Import `.csv` contacts with column matching (including the column names the original uses), and export to `.csv` and `.vcf`
- [ ] **1.0** Categories and Follow Up flags for contacts
- [ ] **1.0** Contact picture add, change, and remove; and shown in the list and on message cards
- [ ] **1.0** Show an address on a map in the system browser when you click it (the click is the request; nothing is sent before)
- [ ] **1.0** Use contacts as the source of a mail merge in JeffWrite and JeffPub: Select from Contacts *(shared: the merge-source library, to be built once)*
- [ ] **1.0** Call opens a `tel:` link in the program the system uses for calls; JeffMail dials nothing itself
- [ ] **later** Business Card editor: a layout, a picture, and fields for a contact's electronic business card
- [ ] **later** Contact certificates: S/MIME certificates and OpenPGP keys kept on a contact and used when sending encrypted mail
- [ ] **later** LDAP directory lookup (company address books) in completion and the Address Book
- [ ] **later** Link contacts: show two entries from different address books as one person
- [ ] **later** Print contacts as cards, booklets, or a phone directory
- [ ] **later** Microsoft contacts for Outlook.com and Microsoft 365 through Microsoft Graph *(decided; see Decided)*

## Tasks and To Do

### Tasks
- [ ] **first release** Create, edit, complete, and delete tasks: subject, start date, due date, status (not started, in progress, completed, waiting, or deferred), priority, percent complete, a reminder, categories, private, and notes in the shared editor; mark complete with a click, with a completed task struck through or hidden as you choose, and an overdue task in a color
- [ ] **first release** Task lists on this computer, with the To-Do List view (tasks and flagged messages together), Simple List, Today, Next 7 Days, Overdue, and Completed, grouped and sorted by due date, start date, category, or priority
- [ ] **first release** Flagged messages appear in the To-Do List with their dates, and completing one clears or completes the flag on the message
- [ ] **1.0** Tasks kept on CalDAV servers that support them (iCalendar to-do items), with the same sync and offline rules as events
- [ ] **1.0** Recurring tasks: every so often, or a new one when the last is completed; skip an occurrence
- [ ] **1.0** To-Do Bar and Daily Task List: today's tasks and agenda beside the mail list and under the day and week calendar views, sorted by start or due date, with completed ones shown or hidden
- [ ] **1.0** Create a task from a message by dragging it to Tasks or by flagging; the task links back to its message so Open Original works
- [ ] **1.0** Task reminders in the Reminders window, with the same snooze as events
- [ ] **1.0** Task details: total work, actual work, mileage, billing information, and companies, which the original keeps and some people use
- [ ] **later** Several task lists on a server, and sub-tasks under a task *(beyond the original)*
- [ ] **later** Assign a task to another person and track its status reports, as the original does with task requests; works only where both ends support it
- [ ] **later** Microsoft To Do and Microsoft 365 tasks through Microsoft Graph *(decided; see Decided)*

### Notes
- [ ] **later** Notes: short colored notes with categories, kept on this computer and, if the person chooses, in a Notes folder on the mail server as the other programs do
- [ ] **later** Notes can be forwarded by email, moved between folders, and shown as icons, a list, or a table

## Contextual tabs

JeffMail has one tab that appears only while something is selected: Attachments, for a message with an attachment. The windows for a message, an appointment, a contact, and a task have their own tabs, which are listed under their modules.

### Attachments tab (a message with an attachment selected)
- [ ] **first release** Open, Save As, Save All Attachments, Select All, and Copy
- [~] **first release** Preview in place: pictures, PDF files, and plain text, shown by the suite's own readers within memory and time limits *(shared: libs/base PDF and picture readers)*
- [~] **1.0** Preview Word, Excel, and PowerPoint files and the suite's own files in place, by the suite's own readers *(shared: libs/office and the apps' file readers)*
- [ ] **1.0** Quick Print of an attachment, and Remove Attachment, which writes the message again without the file to free space after a confirmation that says it changes the server's copy
- [ ] **1.0** Show Message returns to the message the attachment came from
- [ ] **1.0** Open an attachment in the matching JeffOffice app when there is one, from a private temporary copy that is deleted when the app closes
- [ ] **later** Save an attachment to a cloud storage service: not offered, because the original does it only for OneDrive and SharePoint

## Shortcut menus (right-click)

What a right-click offers depends on what is under the pointer; the commands are the ones the ribbon already holds, put where the original puts them.

- [ ] **1.0** Right-click menus on messages, a selection of several, a group heading, a column heading, folders, accounts, attachments, calendar items, contacts, and tasks, with the same commands in the same places as the original
- [ ] **first release** On a message in the list: Open, Reply, Reply All, Forward, Mark as Read or Unread, Flag, Move, and Delete
- [ ] **first release** On a misspelled word while writing: suggestions, Ignore All, and Add to Dictionary, with Cut, Copy, and Paste
- [ ] **1.0** On a folder or an account: New Folder, Rename, Delete, Mark All as Read, Empty (for Deleted Items), and Properties
- [ ] **1.0** On an attachment: Open (only for a safe kind of file, from a private copy), Save As, Save All, and Remove
- [ ] **later** The mini toolbar that floats by selected text while writing, with the font, size, bold, italic, color, and list buttons

## Under the ribbon: search and folders

### Search
- [ ] **first release** Search box at the top of every module (Ctrl+E); results appear as you type; scope is the current folder, with a choice of its subfolders, the whole account, every account, or every kind of item (mail, events, contacts, and tasks)
- [ ] **first release** A search index built in the background on this computer from message headers, bodies, and attachment names, searched offline, with no service and no copy leaving the computer; work throttled so typing and scrolling stay smooth
- [ ] **first release** Search words that narrow a search: `from:`, `to:`, `cc:`, `subject:`, `body:`, `hasattachments:`, `category:`, `received:`, `size:`, `unread:`, and `flagged:`, with AND, OR, NOT, quoted phrases, and a `*` wildcard
- [ ] **first release** The Search tab while a search is active: refine by sender, subject, attachments, categories, date (today through this year, or a range), unread, flagged, important, and sent to me or copied to me; More for any field; Recent Searches; and Close Search
- [ ] **1.0** Matches highlighted in the list and in the open message
- [ ] **1.0** Search the text inside attachments (PDF files, Office files, and plain text) by the suite's readers within limits *(shared: libs/office and libs/base readers)*
- [ ] **1.0** Search the server for messages not downloaded (IMAP search, and Gmail's own search syntax), with those results labeled and merged with the local ones
- [ ] **1.0** Search Folders: saved searches shown as folders that follow their rule (Unread, Flagged, Large Mail, Important, Categorized, Mail from a person, and your own), kept on this computer and never changing the server
- [ ] **1.0** Advanced Find (Ctrl+Shift+F): build a query on any field across item types, with saved queries
- [ ] **1.0** Search the people: the Find a Contact box, with completion from address books and directories
- [ ] **1.0** Sort results by date or by how well they match
- [ ] **1.0** Index Status and Options: which accounts and folders are indexed, a rebuild button, and limits on disk and CPU use

### Folders
- [ ] **first release** The folder pane: each account with its folders, Favorites on top, unread counts, a folder's special role (Inbox, Drafts, Sent, Deleted, Junk, Archive) found from the server's own labels or by name, a local Outbox, and local folders on this computer
- [ ] **first release** Move, copy, and drag folders and messages; a drag between two accounts copies the message and then removes the original after the copy is confirmed
- [ ] **1.0** Go to Folder (Ctrl+Y) and the recent folders list; Folders List and Shortcuts views
- [ ] **later** A "Today" start page: the day's agenda, tasks, and unread counts in one view, as Outlook Today does
- [ ] **later** Shared and public folders on servers that offer them (IMAP shared namespaces)

## Under the ribbon: rules, filters, and junk mail

### Rules
- [ ] **first release** Rules that run when mail arrives: conditions on sender, recipient, subject, words in the body or headers, size, attachments, importance, category, and account; actions to move or copy to a folder, delete, mark read, flag, categorize, and stop processing more rules; with a wizard that builds a rule in plain sentences
- [ ] **first release** Create Rule from a message: always move mail from this sender, to this address, or with this subject into a folder
- [ ] **1.0** Rules and Alerts: the list of rules with turn on and off, order, copy, rename, delete, and an editor with exceptions, and export and import as a file
- [ ] **1.0** More conditions and actions: sent only to me, from a list of people, in a date range, and with a header; actions to forward, redirect, reply with a template, show a desktop alert, play a sound, set importance, and move a conversation
- [ ] **1.0** Run Rules Now on a folder, and rules for sent messages as well as received ones
- [ ] **1.0** Rules say plainly that they run only while JeffMail is open, unless they are saved on the server
- [ ] **later** Rules saved on the server as Sieve scripts (ManageSieve), so they run when JeffMail is closed, with a view of the script for people who want it

### Automatic replies
- [ ] **1.0** Automatic Replies (out of office): a message and a date range, one reply per sender, sent only while JeffMail is open, and the screen says so
- [ ] **later** Automatic Replies saved on the server (Sieve's vacation action) so they work with JeffMail closed

### Junk mail
- [ ] **first release** The provider's junk filter is respected: messages the server puts in Junk are shown there and don't raise a new-mail alert; Not Junk moves a message back to the Inbox and tells the server it was a mistake
- [ ] **first release** Report as junk moves a message to Junk and trains the provider's filter by the keyword and folder the server uses
- [ ] **1.0** A junk filter on your computer that learns from what you mark as Junk and Not Junk, works offline, moves rather than deletes, and has levels (off, low, high, and safe senders only)
- [ ] **1.0** Blocked Senders, Safe Senders, and Safe Recipients lists by address or domain; contacts counted as safe if you choose
- [ ] **1.0** Phishing handling for suspected messages (details under Safety and limits)
- [ ] **1.0** Unsubscribe in one click on mailing-list messages (see Beyond Outlook)
- [ ] **later** Block by country code, domain ending, or character set
- [ ] **later** Report phishing and spam to the provider's abuse address, with the message attached as the provider asks

## Under the ribbon: notifications and offline

### Notifications
- [ ] **first release** A notification for new mail from the system's notification area, with a choice of what it shows (sender and subject, sender only, or nothing), per account and per folder, and Open, Mark as Read, and Delete on the notification
- [ ] **first release** An unread count on the taskbar or dock icon, and an optional tray icon
- [ ] **first release** The Reminders window for events and tasks that are due, with Snooze, Dismiss, Dismiss All, and Open Item; reminders sound, and they show on top when the window is closed to the tray
- [ ] **first release** A status bar that says Connected, Offline, or Trying again, and which account has a problem, with a click to see why
- [ ] **1.0** Run in the background after the window is closed (a tray icon, optional start at login) so mail, notifications, and reminders keep working
- [ ] **1.0** Quiet hours: notifications and sounds muted on a schedule, or while the system is in do-not-disturb
- [ ] **1.0** Sounds: choose a sound for new mail and for reminders, or none

### Working offline
- [ ] **first release** Read everything already downloaded without a connection; write, reply, and delete, with changes queued and sent in order when the connection returns; the Outbox shows what's waiting and why
- [ ] **first release** How much to keep on this computer: per account and per folder, headers only or full messages, and for how long back (one month to all); attachments fetched when asked; a cap on disk use with cleanup that never deletes the only copy of anything
- [ ] **first release** Retries with growing waits; a message that cannot be sent stays in the Outbox with the reason, and is never lost
- [ ] **1.0** Large messages downloaded in pieces that resume after a dropped connection
- [ ] **1.0** Offline use of calendars, contacts, and tasks: changes queue and are merged on reconnect, with conflicts shown
- [ ] **1.0** Proxy settings (system, HTTP, and SOCKS), and optional pausing of sync on a metered connection
- [ ] **1.0** A log of what was sent and received, viewable and copyable, with passwords and tokens hidden
- [ ] **1.0** Default mail program: registers for `mailto:`, `.eml`, `.ics`, `.vcf`, and `webcal:` *(shared: the suite's file associations and installers)*

## Fidelity and testing

JeffMail will be checked against Outlook and against the servers people use. The Windows test machine has Office 2021 (Word, Excel, PowerPoint, Outlook, Access, and Publisher), so Outlook makes the reference pictures of messages and opens the files JeffMail saves. Reference pictures and real messages stay on the build machine and never go in the repository.

- [ ] **first release** Every file JeffMail saves that Outlook reads (`.eml`, `.msg`, `.ics`, and `.vcf`) opens in Outlook on the Windows test machine, and every message, appointment, and contact Outlook saves opens in JeffMail, with nothing lost that a person can see
- [ ] **first release** Sample messages (HTML mail with tables and inline pictures, long quoted replies, and right-to-left and East Asian text) are drawn by JeffMail and by Outlook and compared as pictures, on the scale JeffPub's comparison uses
- [ ] **first release** Messages and calendars held out from tuning are scored apart from the ones used for tuning, so the numbers are not flattered by the files the reader was fitted to
- [ ] **1.0** Large public collections of real messages run through the message reader and the save-and-reopen check, each file that does not open or does not save back listed with its cause *(the Enron email collection is one candidate; it is not downloaded yet)*
- [ ] **1.0** Tested against the servers people use: Dovecot, Cyrus, Gmail, Outlook.com, Yahoo, iCloud, Fastmail, Proton Mail Bridge, and Nextcloud, Radicale, and Baikal for CalDAV and CardDAV, with a list of what was checked in the repository

## Safety and limits

The three rules decided for the whole suite apply here first: nothing in a message reaches out until the person allows it, attachments never run, and every file and message reader is fuzzed and held to limits on memory and time. The items below are how JeffMail keeps them.

### Nothing reaches out
- [ ] **first release** Remote content is blocked in every message: pictures, style sheets, fonts, frames, linked media, and any other address a message could fetch. A bar names what was held back and offers Load for this message, Always for this sender, and Always for this domain (the last two are remembered and can be reviewed and removed in Options); messages from contacts get no exemption unless the person turns that on
- [ ] **first release** Tracking pixels and other tracking requests are never fetched while content is blocked, so a sender cannot tell that, when, where, or on what device a message was opened; the bar says how many were stopped
- [ ] **first release** No scripts, forms, plug-ins, frames, auto-redirects, auto-play, or outside style sheets run in a message; the HTML is cleaned and drawn by a renderer that has no network access and no scripting
- [ ] **first release** Read as plain text, for one message or for all, with links still shown and usable
- [ ] **first release** Links show their real destination; a link whose visible text is an address that differs from its target is flagged before it opens; addresses with look-alike letters from other alphabets, raw numeric addresses, and well-known link shorteners get a note
- [ ] **first release** A read receipt is never sent, and an automatic reply never goes out, without the person's choice
- [ ] **1.0** The tracker bar names the trackers it recognized, using an open list shipped with the program and updated with it, and explains what each would have learned
- [ ] **1.0** Links can have common tracking parameters removed as they open, if the person turns that on
- [ ] **1.0** Optional: strip location and camera data from attached pictures and hidden data from attached Office files (details under Writing a message)

### Attachments never run
- [ ] **first release** JeffMail never starts a program, script, installer, shortcut, or macro from an attachment, and offers no way to. A type that can run code can only be saved to disk, after a warning, and never opened from JeffMail
- [ ] **first release** Open on a safe kind of file (documents, pictures, sound, video, and archives) hands a private temporary copy to the matching JeffOffice app or the system's viewer, only when the person asks, and the copy is removed afterward
- [ ] **first release** The real kind of a file is judged from its contents as well as its name; double extensions, right-to-left override characters, and long names that hide an ending are shown plainly
- [ ] **first release** Saved attachments are marked as coming from the internet where the system supports it (the Windows download mark, the macOS quarantine flag), so the system's own warnings still work
- [ ] **1.0** A list of file kinds always treated as dangerous (the way the original blocks some), editable, and showing a plain reason for each
- [ ] **1.0** Archives are opened for preview with limits on size, depth, and the number of files, so a zip bomb is refused with a message
- [ ] **later** A scan hook: hand a saved attachment to an anti-virus program on the computer before opening, if the person has one

### Readers held to limits
- [ ] **first release** Every reader that touches untrusted data is fuzzed and run within limits on time and memory: the message parser (MIME, character sets, encoded words), the HTML cleaner and renderer, picture and PDF previews, vCard and iCalendar, `.mbox`, `.msg`, `.pst`, TNEF, S/MIME and OpenPGP, and the parsers for IMAP, SMTP, and DAV replies *(shared: the hostile-file sweep of the suite)*
- [ ] **first release** Message rendering and attachment previews run in a separate process with no access to the person's files, the network, or other messages, and a crash there shows an error in place of the message, not a crash of the program
- [ ] **first release** A damaged or hostile message, folder, or data file is opened within limits, or refused with a clear message, and the rest of the mailbox keeps working
- [ ] **first release** A sweep of hostile messages, calendars, and address books runs before each release, alongside tests on large public collections of real messages

### Connections and storage
- [ ] **first release** Encrypted connections by default (TLS from the start, or STARTTLS that is required, not optional); a connection that can't be encrypted is refused unless the person turns on unencrypted access for that one server and accepts a warning that stays visible
- [ ] **first release** The server's certificate is checked every time, and a change in a certificate the person had trusted is reported before anything is sent
- [ ] **first release** Mail, calendars, and contacts are sent to the account's own servers and nowhere else; nothing goes to JeffOffice but the suite's update check and opt-in statistics, which carry no addresses, subjects, or contents
- [ ] **1.0** The local copy encrypted with a passphrase the person sets, so a lost laptop doesn't expose mail (otherwise the system's disk encryption is the answer, and Options says so)
- [ ] **later** A program lock after a time of inactivity, asking for the system password
- [ ] **first release** Opening a file never changes it; Import copies, and Export writes new files beside the old

### Telling real from fake
- [ ] **1.0** Sender checks: shows the result of the receiving server's own sender checks (SPF, DKIM, and DMARC) from the message header; marks a name that matches a contact but an address that doesn't; and marks a Reply-To that differs from the sender
- [ ] **1.0** Phishing warnings: a clear banner on messages that look like impersonation or credential theft, with links turned off until the person clicks to turn them on, and a Report Phishing button that forwards the message as an attachment to the provider's reporting address
- [ ] **1.0** A note on first-time senders, and on senders whose domain nearly matches one you often write to
- [ ] **later** Verify a message's DKIM signature on this computer rather than trusting the server's header

### Signing and encrypting
- [ ] **1.0** S/MIME: sign and encrypt, verify and decrypt, import a certificate from a `.p12` or `.pfx` file, keep a person's certificate from a signed message, check the chain and its revocation, encrypt to yourself too, and a default of signing for chosen accounts
- [ ] **1.0** OpenPGP: sign, encrypt, verify, and decrypt (PGP/MIME and the older inline form); make, import, export, expire, and revoke keys; find a person's key through Web Key Directory and the keys that Autocrypt headers carry; and a simple "trust the first key you see, and warn on a change" model
- [ ] **1.0** A sign-and-encrypt indicator in the header of a message (valid, invalid, unknown key, and expired) and a lock on the Send button for a message that will be encrypted
- [ ] **1.0** A check before sending an encrypted message that every recipient has a key, with a choice to send it unencrypted, remove the person, or cancel
- [ ] **later** Contact certificates and keys managed on the contact and in a key manager of their own
- [ ] **later** Hardware keys and smart cards for S/MIME and OpenPGP

## File formats

### Messages
- [~] **first release** `.eml` and the message format underneath (RFC 5322 and MIME): every header kept and shown; multipart/alternative, related, mixed, signed, and encrypted; quoted-printable, base64, and 8-bit; encoded words and long header values; and character sets from UTF-8 and the Windows and ISO sets to the East Asian and Cyrillic ones. Messages inside messages, delivery-failure and read-receipt notices, and calendar and vCard parts are read, and attachment names in every convention seen. Damaged messages that other programs show are shown here too. *(shared: JeffPub writes a simple `.eml` today)*
- [ ] **first release** `.mbox` read and written (the common variants), and Maildir folders read and written
- [~] **1.0** `.msg` read and written: messages, appointments, contacts, and tasks, with attachments and embedded messages *(shared: libs/base/io compound file reader and writer)*
- [ ] **1.0** `.pst` and `.ost` read-only (details under File), from the open specification of the format, in a reader held to the same limits as the others
- [ ] **1.0** TNEF (the `winmail.dat` attachment the original sometimes sends): unpacked into its files, its text, and its calendar and contact parts
- [ ] **later** Apple Mail `.emlx` and the Mac program's `.olm`, read-only
- [ ] **later** HTML single-file save of a message

### Calendars and contacts
- [ ] **first release** iCalendar (`.ics`, RFC 5545) read and written for events, to-dos, time zones, alarms, recurrences, exceptions, attendees, and attachments; unknown properties kept unchanged; the Microsoft extensions read (Windows time-zone names, busy status, and the formatted description)
- [ ] **first release** Invitation messages (the iCalendar methods request, reply, cancel, counter, and refresh, carried in mail) read and written so that Outlook, Google Calendar, Apple Calendar, and Thunderbird accept them
- [~] **first release** vCard `.vcf` (2.1, 3.0, and 4.0) read, and 3.0 and 4.0 written *(shared: JeffPub's vCard reader)*
- [ ] **1.0** `.csv` contacts import and export with the original's column set
- [ ] **later** Journal items from a `.pst` imported as notes in a folder

### The local copy
- [ ] **first release** A local store for the cache of server mail, drafts, the Outbox, local folders, the search index, and settings; its format is an engineering choice made when building starts (see Decided); the requirements are that a damaged cache is rebuilt from the server, that local-only folders can always be exported as `.mbox`, that a crash never loses a draft, and that it holds 200,000 messages without slowing the lists
- [ ] **first release** Large folders stay quick: lists load by the screenful, bodies and attachments come when asked, and scrolling a folder of a hundred thousand messages stays smooth
- [ ] **1.0** Rebuild the local copy and the search index from the server, from Cleanup Tools
- [ ] **1.0** Compact the local store and report how much space it freed

## Shared with other JeffOffice apps

What JeffMail builds on, and the pieces it should build so that JeffWrite, JeffCrunch, JeffDeck, JeffPub, and JeffBase can use them.

### Already there
- [~] **first release** libs/shell: the window, ribbon, KeyTips, file pages, Help, translations, AutoRecover, settings, spelling, updates, statistics, and crash reports
- [~] **first release** libs/text and libs/page: the text engine and editor that JeffWrite's flowing editor grows from, and the one JeffMail's composer uses
- [~] **first release** libs/office: Open XML packages and readers, used for attachment previews of Word, Excel, and PowerPoint files
- [~] **first release** libs/base: zip and compound files (`.msg`), PDF reading and writing, fonts, barcodes, and the 12-language dictionaries
- [~] **first release** JeffPub's earlier mail pieces: a simple `.eml` writer, a vCard reader, mail merge, email-as-attachment sharing, online pictures, the icon picker, and the thesaurus
- [~] **first release** Qt Network and its TLS support are already linked (the update check and statistics use them); mail needs sockets, IMAP, SMTP, and WebDAV on top

### To build once, for several apps
- [ ] **first release** A mail profile of the flow-document editor: only the features email can carry, a clean HTML export with a plain-text twin, and an import that cleans received HTML for display *(needed by: JeffWrite, JeffMail)*
- [ ] **first release** A safe HTML and style renderer for untrusted content: no network, no scripts, bounded memory and time *(needed by: JeffMail, JeffWrite for HTML import)*
- [ ] **first release** A MIME library: parse and build messages, headers, character sets, and encoded words, so every app's "send by email" writes a correct message *(needed by: JeffMail, JeffPub, JeffWrite, JeffDeck, JeffCrunch)*
- [ ] **first release** The grammar and style check behind the spelling service (Harper) *(needed by: JeffWrite, JeffMail)*
- [ ] **first release** The fuzzing harness and the resource limits for readers, extended to the mail readers *(needed by: all)*
- [ ] **first release** A field engine on the field model of libs/text (dates, merge fields, and properties) *(needed by: all)*
- [ ] **first release** An iCalendar and vCard library, with a round trip that keeps what it doesn't understand, moved out of JeffPub's merge code *(needed by: JeffMail, JeffPub, JeffWrite, JeffBase)*
- [ ] **1.0** Share by email from every app: File > Share > Email opens a JeffMail message with the file attached or the text in the body, and falls back to a ready-to-send `.eml` file when JeffMail isn't installed *(needed by: all)*
- [ ] **1.0** A merge-source library (CSV, vCard, spreadsheets) with JeffMail's address books as a source, and Merge to Email sent through JeffMail's accounts *(needed by: JeffPub, JeffWrite, JeffMail, JeffBase)*
- [ ] **1.0** Date, time, recurrence, reminder, and time-zone controls, with the time-zone database, for events, tasks, and also date cells and fields *(needed by: JeffMail, JeffCrunch, JeffBase)*
- [ ] **1.0** A secret store and OAuth sign-in service (the system keychain, and the browser sign-in with a loopback address), in case another app ever reads an online source *(needed by: JeffMail, JeffCrunch)*
- [ ] **1.0** A table engine with merged cells and styles *(needed by: JeffWrite, JeffMail, JeffCrunch)*
- [ ] **1.0** Building blocks in one format and gallery (Quick Parts, signatures, page parts) *(needed by: all)*
- [ ] **1.0** An accessibility checker and its pane *(needed by: JeffWrite, JeffDeck, JeffCrunch, JeffMail)*
- [ ] **later** Text to speech for Read Aloud *(needed by: JeffWrite, JeffDeck, JeffMail)*
- [ ] **later** An ink layer (pens, highlighter, eraser) *(needed by: JeffWrite, JeffDeck, JeffMail)*

## Left out on purpose

Not planned, because they only work with an online service, a product JeffOffice doesn't have, or a protocol that is closed or being retired: Exchange ActiveSync and MAPI over HTTP, Microsoft 365 Groups and Teams, Skype, instant messaging and presence, phone dialing and call logs, Office add-ins and the Store, Outlook on the web, the switch between the original's classic and new-look windows, OneDrive and SharePoint attachments and links, voting buttons and approval requests, information rights management and sensitivity labels, Exchange retention policies, Clutter, public folders and Exchange delegates, the Journal, InfoPath and custom forms, Visual Basic macros and COM add-ins (pending the macro question in JeffWrite), the Business Contact Manager, fax, Windows CE sync, Microsoft Translator and Insights, the weather bar, `.pst` writing, `.rwz` rule files, sending in Rich Text format, the Gmail API, and Proton's unofficial API (never used).

## Decided

Decided October 11, 2026, except where noted.

- First release, mail: mail from any standard provider over IMAP and SMTP, with OAuth sign-in where the provider offers it and app passwords otherwise, plus Outlook.com and Microsoft 365 mail over IMAP with Microsoft's OAuth sign-in.
- First release, calendars and contacts: over the open standards CalDAV and CardDAV from the start. Many providers, such as Fastmail, offer them.
- Microsoft calendars and contacts (Outlook.com and Microsoft 365) come in a later release: they can only be reached through Microsoft Graph, which needs an app registration and publisher verification.
- Proton Mail: through Proton Mail Bridge (Proton's local IMAP and SMTP program, on paid plans). JeffMail offers a "Proton Mail" account preset that finds Bridge. Proton calendars and contacts can't sync, since Proton offers no CalDAV or CardDAV, and Proton's unofficial API is never used.
- Security, for the whole suite: nothing in a message reaches out until the person allows it (remote pictures, tracking pixels, and linked content); attachments never run; and every file and message reader is fuzzed and held to limits on memory and time.
- Composing uses JeffWrite's editor, shared, so the formatting features of the message body come from that editor.
- The suite saves in Office's formats by default. For mail, the format of the local store is an engineering choice, made when building starts.
- Grammar checking: Harper, English first, shared with JeffWrite.

## Open questions

1. Who registers JeffMail with Google and with Microsoft for OAuth sign-in, and what does a Gmail or Microsoft 365 user get (app passwords only?) until each provider's review or approval passes?
2. Does Microsoft Graph (calendars, contacts, and To Do for Outlook.com and Microsoft 365) stay at later, or move to 1.0, since most people leaving the original program use Microsoft 365?
3. Should the first release only receive and answer meeting invitations, or also create meetings with attendees?
4. Should `.pst` import move to the first release, as the way a person carries years of old mail out of the original program?
5. Should S/MIME and OpenPGP move to the first release, or stay at 1.0?
6. Is POP3 worth 1.0, or does it move to later, since the providers that remain on it are few?
7. Exchange Web Services for company-run Exchange servers: keep at later, or leave out because Microsoft is retiring it from Exchange Online?
8. Automatic Replies that work only while JeffMail is open: acceptable at 1.0, or offered only where the server can run them (Sieve)?
9. Should new messages default to HTML or to plain text?
10. Macros and add-ins: follow whatever JeffWrite decides, or decide separately for mail?
11. Which providers get a preset in the first release beyond Gmail, Outlook.com, Yahoo, iCloud, Fastmail, and Proton Mail?
12. Notes: keep at later, or leave out with the Journal?
