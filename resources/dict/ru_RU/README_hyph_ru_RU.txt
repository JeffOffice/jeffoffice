hyph_ru_RU.dic: Russian hyphenation patterns, CONVERTED from hyph-ru.tex

THIS FILE IS A CONVERTED, CHANGED FORM OF THE ORIGINAL. It is not hyph-ru.tex and
is not the original author's file. It was made by JeffOffice LLC with the script
tools/dict/tex_to_hyph.py (in the JeffOffice source), which rewrites the TeX
\patterns{} and \hyphenation{} lists as a libhyphen .dic file (UTF-8, with the
file's own minimums of 2 letters before and 2 after a hyphen). The file name
hyph_ru_RU.dic is distinct from the original name, hyph-ru.tex, as the LaTeX
Project Public License requires of a changed file.

What the conversion changes:
* every start of every pattern is written out with the digits of the patterns
  that end the same way, as libhyphen's substrings.pl does, so the loader needs
  no TeX-style matching;
* the patterns that contain a hyphen are left out (they only keep TeX from
  breaking beside a hyphen; the loader already never does);
* each word of the \hyphenation{} list becomes one pattern for that word.
The patterns themselves are Lebedev's, unchanged.

Original
--------
Title:     Hyphenation patterns for Russian (hyph-ru.tex)
Authors:   Alexander I. Lebedev <swan (at) scon155.phys.msu.su>
Copyright: Copyright (C) 1999-2003 Alexander I. Lebedev
Version:   2003/03/10
Source:    the hyph-utf8 package, https://github.com/hyphenation/tex-hyphen
           (hyph-utf8/tex/generic/hyph-utf8/patterns/tex/hyph-ru.tex),
           also on CTAN, https://ctan.org/pkg/hyph-utf8
           and http://www.hyphenation.org/tex
Notes:     "Patterns were generated with patgen from a 990,000-word list and
           then manually corrected."

License
-------
The original states: licence LPPL, version 1.2, or any later version
(https://latex-project.org/lppl/). This converted file is distributed under the
same terms: the LaTeX Project Public License, version 1.2 or (at your option)
any later version (https://latex-project.org/lppl/). It is a modified version
of the original, it has a different name, and this notice says what changed.
JeffOffice's GPL does not cover it.
