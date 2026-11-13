#!/usr/bin/env python3
"""Kept so that the translation scripts that call it still work.

Help's topics are Markdown files in resources/help/<language>/ (JeffDeck's in
its deck/ folder). They are no longer listed in resources/resources.qrc and
built into the programs: they are installed in the data folder beside the
programs (help/), and a development build reads them from resources/help.
Adding, renaming, or removing a topic needs no list to update.
"""


def main():
    print("nothing to list: Help is read from resources/help")


if __name__ == "__main__":
    main()
