# Velden

<!-- keywords: veld, velden, INDEX, TOA, CITATION, BIBLIOGRAPHY, XE, TA, TC, veld bijwerken, F9, koppeling verbreken, vergrendelen, veldcodes, paginanummer, datum, NUMPAGES, SEQ, REF, PAGEREF, formule, Veld invoegen, veld bewerken, klikken met rechtermuisknop, veld, velden, pagina, datum, invoegen, veldcode, veldcodes -->

Een **veld** is een stuk tekst dat het programma voor u berekent, zoals een paginanummer, de datum van vandaag of het nummer van een figuur. Een veld heeft twee delen: een **code**, die aangeeft wat er moet worden berekend, en een **resultaat**, de tekst die u ziet. De code voor een paginanummer is `PAGE`, en het resultaat op de derde pagina is `3`.

## Een veld in de tekst plaatsen

1. Klik op de plek waar het veld moet komen.
2. Klik op **Invoegen > Veld**.
3. Kies een veld in de lijst. De code verschijnt in het vak eronder, en u kunt die wijzigen.
4. Klik op **OK**.

Kies bijvoorbeeld **Datum**, wijzig de code in `DATE \@ "MMMM d, yyyy"` en klik op **OK**. De tekst toont de datum als `2 november 2026`.

## De code van een veld wijzigen

Klik met de rechtermuisknop op een veld en kies **Veld bewerken**. Er verschijnt een vak met de code. Wijzig die en klik op **OK**, dan wordt het veld opnieuw berekend. **Ongedaan maken** zet het oude veld terug. Een vergrendeld veld kunt u pas bewerken als u het ontgrendelt. Het snelmenu met de rechtermuisknop bevat ook **Veld bijwerken**, **Veldcode in-/uitschakelen**, **Veld vergrendelen**, **Veld ontgrendelen** en **Veldkoppeling verbreken**.

## Velden bijwerken

Een veld toont het resultaat dat het had toen het voor het laatst werd berekend. Druk op **F9** om het veld waarin de invoegpositie staat opnieuw te berekenen. Om veel velden tegelijk bij te werken, selecteert u de tekst met die velden en drukt u op **F9**. Druk op **Ctrl+A** en daarna op **F9** om elk veld in het document bij te werken.

Bijwerken is één stap: **Ongedaan maken** zet alle resultaten terug. Bijwerken telt niet als een bijgehouden wijziging, ook niet wanneer Wijzigingen bijhouden is ingeschakeld, omdat het resultaat niet iets is wat u hebt getypt.

Een veld in een koptekst of voettekst dat het paginanummer, het aantal pagina's of een kop toont, is altijd juist en hoeft niet te worden bijgewerkt.

## De codes tonen

Druk op **Shift+F9** om de code van het veld waarin de invoegpositie staat te tonen, tussen accolades zoals `{ PAGE }`, in plaats van het resultaat. Druk op **Alt+F9** om de codes van alle velden te tonen; druk nogmaals op de toets om de resultaten weer te geven. Dit verandert alleen wat u ziet: het bestand wordt opgeslagen zoals het was, en het document wordt niet als gewijzigd gemarkeerd.

## Een veld loskoppelen of vergrendelen

- **Ctrl+Shift+F9** zet het veld om in gewone tekst. Het resultaat blijft staan, maar het is geen veld meer.
- **Ctrl+F11** vergrendelt een veld, zodat **F9** het niet verandert. **Ctrl+Shift+F11** ontgrendelt het.

## Opmaak

Voeg schakelaars toe aan de code om te wijzigen hoe een resultaat eruitziet:

- `\* roman`, `\* ROMAN`, `\* alphabetic`, `\* Ordinal`, `\* CardText`, `\* Upper`, `\* Lower` en `\* Caps` wijzigen de hoofdletters of de soort getal. `{ = 21 \* CardText }` toont `eenentwintig`.
- `\# "0.00"` bepaalt hoe een getal eruitziet. `{ = 1234.5 \# "#,##0.00" }` toont `1.234,50`.
- `\@ "dddd, MMMM d"` bepaalt hoe een datum eruitziet. `M` is de maand en `m` is de minuut.

## Velden die JeffWrite berekent

PAGE, NUMPAGES, SECTIONPAGES, SECTION, PAGEREF, REF, NOTEREF, STYLEREF, SEQ, DATE, TIME, CREATEDATE, SAVEDATE, PRINTDATE, AUTHOR, TITLE, SUBJECT, FILENAME, FILESIZE, DOCPROPERTY, NUMWORDS, NUMCHARS, IF, COMPARE, QUOTE, een formule die begint met `=`, TOC (ook een illustratielijst), INDEX, TOA, CITATION en BIBLIOGRAPHY. De verborgen markeringen TC, XE en TA hebben geen resultaat om te berekenen: ze vertellen een inhoudsopgave, een register of een lijst met autoriteiten wat er moet worden opgenomen. Een veld van een ander soort behoudt het resultaat dat het heeft, en wordt opgeslagen zoals het was.

DATE, TIME, FILENAME en AUTHOR hangen af van de computer en het moment, dus ze kunnen bij elke update veranderen.
