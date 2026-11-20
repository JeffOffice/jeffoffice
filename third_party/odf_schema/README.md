# OpenDocument schema files (test data)

The RELAX NG schemas of the OpenDocument standard, version 1.2 (OASIS Standard, 29 September 2011): the document schema, the manifest schema, and the digital-signature schema. They come from the standards body's own site, https://docs.oasis-open.org/office/v1.2/os/, and are byte for byte the files published there.

| File | sha256 |
|---|---|
| `OpenDocument-v1.2-os-schema.rng` | `ca5d2510858c6f922b20cd27dc76d34959c1a976b9dfc6b5caeadbebfc83dbbb` |
| `OpenDocument-v1.2-os-manifest-schema.rng` | `75f972902bc785c15e51fb4e112e2f1b24e2327829eb54786cf8687ee39c7631` |
| `OpenDocument-v1.2-os-dsig-schema.rng` | `52fbf196dc11fcaac41ff4b320f005daf419794c2f343a26ecab05778d7e7f6f` |

Test data, never shipped. Only the tests use them: the OpenDocument writers' output is checked against them (`tests/office/odfvalidate.cpp`). They are not compiled into any program, installed, or packaged, and `tools/check_third_party.py` fails if one of them differs from the hash above.

## Notice

Each file's own header carries its notice and terms, unchanged, and they must be read there, not here; the copyright line is "Copyright (c) OASIS Open 2002-2011. All Rights Reserved." The files are kept with their headers unchanged, and `.gitattributes` keeps their line endings as they are.

## Never modify these files

Not a line, not a space, not the line endings. A change in what a test needs is made in the test, not here. To use a later version, replace the file with the published one and update the hash here.

The version 1.3 schemas are not here: their headers carry a copyright line but not the permission to copy that the version 1.2 headers state, so they are left out until that is settled.
