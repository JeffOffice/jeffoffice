# Velden
<!-- keywords: veld, velden, veld bijwerken, F9, koppeling verbreken, veldkoppeling verbreken, vergrendelen, veldcodes, paginanummer, datum, NUMPAGES, SEQ, REF, PAGEREF, formule, Veld invoegen -->

Een **veld** is een stukje tekst dat het programma voor u berekent, zoals een paginanummer, de datum van vandaag of het nummer van een figuur. Een veld bestaat uit twee delen: een **code**, die aangeeft wat er berekend moet worden, en een **resultaat**, de tekst die u ziet. De code voor een paginanummer is `PAGE`, en het resultaat daarvan op de derde pagina is `3`.

## Een veld in de tekst invoegen

1. Klik op de plek waar het veld moet komen.
2. Klik op **Invoegen > Veld**.
3. Kies een veld in de lijst. De code ervan verschijnt in het vak eronder, en u kunt deze wijzigen.
4. Klik op **OK**.

Kies bijvoorbeeld **Datum**, wijzig de code in `DATE \@ "d MMMM yyyy"` en klik op **OK**. De tekst toont de datum dan als `2 november 2026`.

## Velden bijwerken

Een veld toont het resultaat van de laatste keer dat het werd berekend. Druk op **F9** om het veld waarin de cursor staat opnieuw te berekenen. Als u veel velden tegelijk wilt bijwerken, selecteert u de tekst waarin ze staan en drukt u op **F9**. Druk op **Ctrl+A** en daarna op **F9** om alle velden in het document bij te werken.

Bijwerken is één stap: met **Ongedaan maken** worden alle resultaten teruggezet. Bijwerken wordt niet als wijziging bijgehouden, ook niet als Wijzigingen bijhouden is ingeschakeld, omdat het resultaat niet iets is wat u hebt getypt.

Een veld in een koptekst of voettekst dat het paginanummer, het aantal pagina's of een kop toont, is altijd juist en hoeft niet te worden bijgewerkt.

## De codes weergeven

Druk op **Shift+F9** om de code van het veld waarin de cursor staat weer te geven tussen accolades, zoals `{ PAGE }`, in plaats van het resultaat. Druk op **Alt+F9** om de codes van alle velden weer te geven, en druk er nogmaals op om de resultaten weer te geven. Dit verandert alleen wat u ziet: het bestand wordt opgeslagen zoals het was, en het document wordt niet als gewijzigd gemarkeerd.

## De koppeling van een veld verbreken of een veld vergrendelen

- Met **Ctrl+Shift+F9** maakt u van het veld gewone tekst. Het resultaat blijft staan, maar het is geen veld meer.
- Met **Ctrl+F11** vergrendelt u een veld, zodat **F9** het ongemoeid laat. Met **Ctrl+Shift+F11** ontgrendelt u het.

## Notaties

Voeg schakelopties aan de code toe om te wijzigen hoe een resultaat eruitziet:

- `\* roman`, `\* ROMAN`, `\* alphabetic`, `\* Ordinal`, `\* CardText`, `\* Upper`, `\* Lower` en `\* Caps` wijzigen het hoofdlettergebruik of het soort getal. `{ = 21 \* CardText }` toont `eenentwintig`.
- `\# "0,00"` bepaalt hoe een getal eruitziet. `{ = 1234,5 \# "#.##0,00" }` toont `1.234,50`.
- `\@ "dddd d MMMM"` bepaalt hoe een datum eruitziet. `M` is de maand en `m` is de minuut.

## Velden die JeffWrite berekent

PAGE, NUMPAGES, SECTIONPAGES, SECTION, PAGEREF, REF, NOTEREF, STYLEREF, SEQ, DATE, TIME, CREATEDATE, SAVEDATE, PRINTDATE, AUTHOR, TITLE, SUBJECT, FILENAME, FILESIZE, DOCPROPERTY, NUMWORDS, NUMCHARS, IF, QUOTE, een formule die begint met `=`, en TOC. Een veld van een ander soort behoudt het resultaat dat het heeft en wordt opgeslagen zoals het was.

DATE, TIME, FILENAME en AUTHOR zijn afhankelijk van de computer en het moment, dus ze kunnen bij elke keer bijwerken veranderen.
