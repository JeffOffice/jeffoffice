# Bundled dictionaries

JeffPub checks spelling and hyphenates with these dictionaries from the [LibreOffice dictionaries project](https://github.com/LibreOffice/dictionaries), one folder per language as LibreOffice lays them out. The files are unchanged, except the Russian hyphenation, which is converted (see its row). Each remains under its own license, recorded in the README and license files beside it. None is covered by JeffPub's GPL.

| Folder | Contents | License |
|---|---|---|
| `en` | English spelling for the United States, Canada, and Australia (`en_US`, `en_CA`, `en_AU`, from [SCOWL](http://wordlist.aspell.net/)) | SCOWL license, permissive (`README_en_US.txt`, `README_en_CA.txt`, `README_en_AU.txt`) |
| `en` | English spelling for the United Kingdom (`en_GB`, Marco A. G. Pinto and others) | LGPL (`README_en_GB.txt`) |
| `en` | American and British hyphenation (`hyph_en_US.dic`, `hyph_en_GB.dic`) | BSD-style (`README_hyph_en_US.txt`, `README_hyph_en_GB.txt`) |
| `en` | English thesaurus (`th_en_US_v2.dat`, from [WordNet](https://wordnet.princeton.edu/)) | WordNet license (`WordNet_license.txt`) |
| `es` | Spanish spelling for 21 regions (`es_AR` to `es_VE`, from the [RLA-ES project](https://github.com/sbosio/rla-es) release 2.9, by Santiago Bosio and others) and hyphenation (`hyph_es.dic`) | GPL-3.0+, LGPL-3.0+, or MPL-1.1+, your choice (`LICENSE.md`, `README_hunspell_es.txt`, `README_hyph_es.txt`) |
| `fr_FR` | French spelling (`fr`, from [Grammalecte](https://grammalecte.net/)) | MPL-2.0 (`README_dict_fr.txt`) |
| `fr_FR` | French hyphenation (`hyph_fr.dic`) | LGPL (`README_hyph_fr.txt`) |
| `de` | German spelling (`de_DE_frami`, from [igerman98](https://www.j3e.de/ispell/igerman98/)) | GPL-2.0 or GPL-3.0 (`README_de_DE_frami.txt`, `COPYING_GPLv2`, `COPYING_GPLv3`) |
| `de` | German hyphenation (`hyph_de_DE.dic`) | LGPL-2.0+ (`README_hyph_de.txt`, `COPYING_LGPL_v2.0.txt`) |
| `it_IT` | Italian spelling (`it_IT`) | GPL-3.0 (`README_it_IT.txt`) |
| `it_IT` | Italian hyphenation (`hyph_it_IT.dic`) | LGPL (`README_hyph_it_IT.txt`) |
| `nl_NL` | Dutch spelling and hyphenation (from [OpenTaal](https://www.opentaal.org/)) | BSD (revised) or CC BY 3.0 (`LICENSE.txt`, `README.md`) |
| `pt_BR` | Brazilian Portuguese spelling and hyphenation (from [VERO](http://pt-br.libreoffice.org/projetos/projeto-vero-verificador-ortografico/)) | LGPL-3.0 or MPL (`README_pt_BR.txt`, `README_hyph_pt_BR.txt`, `README_en.txt`) |
| `pt_PT` | European Portuguese spelling and hyphenation | GPL-2.0+ (`LICENSES.txt`, `README_pt_PT.txt`, `README_hyph_pt_PT.txt`) |
| `ru_RU` | Russian spelling (`ru_RU`, Alexander I. Lebedev). The folder's own hyphenation patterns, thesaurus, and grammar checker are not bundled: the patterns state no license | BSD-style (`README_ru_RU.txt`) |
| `ru_RU` | Russian hyphenation (`hyph_ru_RU.dic`), **converted** by `tools/dict/tex_to_hyph.py` from Lebedev's `hyph-ru.tex` (version 2003/03/10, from [hyph-utf8](https://github.com/hyphenation/tex-hyphen)), so this one file is not unchanged | LPPL 1.2 or later (`README_hyph_ru_RU.txt`) |
| `id` | Indonesian spelling and hyphenation (`id_ID`, from [hunspell-id](https://github.com/shuLhan/hunspell-id)). The thesaurus is not bundled | LGPL-3.0 (`LICENSE-dict`, `README-dict.adoc`). The upstream project's license file (`.reuse/dep5`) lists `hyph_id_ID.dic` under LGPL-3.0-only, though the file has no header of its own. The comments at the top of `id_ID.aff` (lines 5-8) still say GPL; the project's license file and the SPDX tag in that file say LGPL-3.0 |
| `uk_UA` | Ukrainian spelling (`uk_UA`, from [dict_uk](https://github.com/brown-uk/dict_uk), by Andriy Rysin and others) | MPL-1.1 (`README_uk_UA.txt`) |
| `uk_UA` | Ukrainian hyphenation (`hyph_uk_UA.dic`, Maksym Polyakov) | GPL-2.0+ (`README_hyph_uk_UA.txt`) |

Which dictionary checks a piece of text follows the language set on it (Review > Language). Text in a language without a dictionary here isn't checked or hyphenated. The table of languages and files is in `libs/base/text/dictionaries.cpp`.

The Spanish regions share most of their words, so `es/` keeps them once: `es_common.dic` holds the words every region has with the same flags, each `es_XX.dic` only that region's other words (loaded on top with Hunspell's `add_dic`), and the regions share eight `.aff` files (`es_BO.aff` is also Chile's, Costa Rica's, and nine others'; `es_MX.aff` also Peru's and Venezuela's). The words and flags a region accepts are those of release 2.9; `tools/split_spanish_dicts.py` makes these files from a release's `es_XX.dic` and `es_XX.aff` (see its header), and the table in `libs/base/text/dictionaries.cpp` names the `.aff` each region uses. `es_ES` and `es_MX` carry three small additions to the release (`cifrar`, `cifrador`, `encriptar`).
