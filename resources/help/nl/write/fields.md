# Velden

<!-- keywords: veld, velden, INDEX, TOA, CITATION, BIBLIOGRAPHY, XE, TA, TC, veld bijwerken, F9, koppeling verbreken, vergrendelen, veldcodes, paginanummer, datum, NUMPAGES, SEQ, REF, PAGEREF, formule, Invoegen veld -->

Een **veld** is een stuk tekst dat het programma voor u berekent, zoals een paginanummer, de datum van vandaag of het nummer van een figuur. Het bestaat uit twee delen: een **code**, die aangeeft wat er berekend moet worden, en een **resultaat**, de tekst die u ziet. De code voor een paginanummer is `PAGE`, en het resultaat op de derde pagina is `3`.

## Een veld in de tekst plaatsen

1. Klik op de plaats waar het veld moet komen.
2. Klik op **Invoegen > Veld**.
3. Kies een veld uit de lijst. De code verschijnt in het vak eronder, en u kunt die wijzigen.
4. Klik op **OK**.

Kies bijvoorbeeld **Datum**, wijzig de code in `DATE \@ "MMMM d, yyyy"` en klik op **OK**. De tekst toont de datum als `November 2, 2026`.

## Velden bijwerken

Een veld toont het resultaat dat het had toen het laatst werd berekend. Druk op **F9** om het veld waarin de invoegmarkering staat opnieuw te berekenen. Om veel velden tegelijk bij te werken, selecteert u de tekst met die velden en drukt u op **F9**. Druk op **Ctrl+A** en daarna op **F9** om elk veld in het document bij te werken.

Bijwerken is één stap: met **Ongedaan maken** zet u alle resultaten terug. Bijwerken is geen bijgehouden wijziging, ook niet wanneer Wijzigingen bijhouden is ingeschakeld, omdat het resultaat niet iets is wat u hebt getypt.

Een veld in een koptekst of voettekst dat het paginanummer, het aantal pagina's of een kop weergeeft, is altijd juist en hoeft niet te worden bijgewerkt.

## De codes weergeven

Druk op **Shift+F9** om de code van het veld waarin de invoegmarkering staat weer te geven, tussen accolades zoals `{ PAGE }`, in plaats van het resultaat. Druk op **Alt+F9** om de codes van alle velden weer te geven, en druk er nogmaals op om de resultaten weer te geven. Dit verandert alleen wat u ziet: het bestand wordt opgeslagen zoals het was, en het document wordt niet als gewijzigd gemarkeerd.

## Een veld loskoppelen of vergrendelen

- **Ctrl+Shift+F9** zet het veld om in gewone tekst. Het resultaat blijft staan, maar het is geen veld meer.
- **Ctrl+F11** vergrendelt een veld, zodat **F9** het ongemoeid laat. **Ctrl+Shift+F11** ontgrendelt het.

## Opmaak

Voeg schakelaars toe aan de code om te wijzigen hoe een resultaat eruitziet:

- `\* roman`, `\* ROMAN`, `\* alphabetic`, `\* Ordinal`, `\* CardText`, `\* Upper`, `\* Lower` en `\* Caps` wijzigen de hoofdletters of het soort getal. `{ = 21 \* CardText }` toont `twenty-one`.
- `\# "0.00"` bepaalt hoe een getal eruitziet. `{ = 1234.5 \# "#,##0.00" }` toont `1,234.50`.
- `\@ "dddd, MMMM d"` bepaalt hoe een datum eruitziet. `M` is de maand en `m` is de minuut.

## Velden die JeffWrite berekent

PAGE, NUMPAGES, SECTIONPAGES, SECTION, PAGEREF, REF, NOTEREF, STYLEREF, SEQ, DATE, TIME, CREATEDATE, SAVEDATE, PRINTDATE, AUTHOR, TITLE, SUBJECT, FILENAME, FILESIZE, DOCPROPERTY, NUMWORDS, NUMCHARS, IF, COMPARE, QUOTE, een formule die begint met `=`, TOC (ook een illustratielijst), INDEX, TOA, CITATION en BIBLIOGRAPHY. De verborgen markeringen TC, XE en TA hebben geen resultaat om te berekenen: ze vertellen een tabel, een register of een lijst met autoriteiten wat erin moet worden opgenomen. Een veld van een ander soort behoudt het resultaat dat het heeft, en wordt opgeslagen zoals het was.

DATE, TIME, FILENAME en AUTHOR zijn afhankelijk van de computer en het moment, dus ze kunnen bij elk bijwerken veranderen.
