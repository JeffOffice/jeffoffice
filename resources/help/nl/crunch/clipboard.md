# Knippen, kopiëren en plakken

<!-- keywords: knippen, kopiëren, plakken, klembord, verplaatsen, dupliceren, speciaal, waarden, formules, opmaak, transponeren, lege cellen overslaan, optellen, aftrekken, vermenigvuldigen, delen, bewerking, samengevoegd, tekst, tabblad, html, webpagina, ander programma, Escape, lopende mieren, rand, kolombreedten, notities, opmerkingen, getalnotaties, alles behalve randen, afbeelding, bitmap, kopiëren als afbeelding, afgedrukte afbeelding, zoals afgedrukt, koppeling plakken, hyperlink, tekst importeren, gescheiden tekst, tekst neerzetten, komma, puntkomma, csv, scheidingsteken, standaardplakken instellen, standaard plakken, vaste breedte -->

**Kopiëren** (**{key:Copy}**) zet een kopie van de geselecteerde cellen op het klembord, een plek die de computer voor één ding tegelijk bewaart. **Plakken** (**{key:Paste}**) zet wat op het klembord staat in de cellen, beginnend bij de geselecteerde cel. **Knippen** (**{key:Cut}**) werkt als kopiëren, maar bij het plakken worden de cellen verplaatst en wordt de oude plek leeg gemaakt.

Een bewegende gestreepte rand toont de cellen die zijn gekopieerd of geknipt. Druk op **Esc** om de rand weg te halen; de cellen zijn dan niet meer klaar om te worden geplakt. Geknipte cellen worden ook losgelaten als u vóór het plakken een andere wijziging maakt (typen, invoegen, ongedaan maken). Knip ze dan opnieuw als u ze nog wilt verplaatsen.

## Plakken binnen JeffCrunch

Alles komt mee: de waarden, de formules, de opmaak en de samengevoegde cellen. Een formule blijft verwijzen naar dezelfde plaatsen *ten opzichte van waar hij staat*. Voorbeeld: kopieer **=A1*2** uit B1 en plak het in B5, dan wordt het **=A5*2**. Een deel met een dollarteken, zoals **$A$1**, blijft ongewijzigd. Als u een blok selecteert dat een geheel aantal keer zo groot is als wat u hebt gekopieerd, wordt de kopie herhaald om het te vullen: kopieer één cel, selecteer twintig cellen, plak, en alle twintig krijgen de inhoud.

Een geplakt getal dat te breed is voor een kolom die nog zijn oorspronkelijke breedte heeft, maakt de kolom breder.

## Knippen verplaatst cellen

Wanneer u cellen knipt en plakt, worden de cellen echt verplaatst. Elke formule die naar hen verwees, verwijst nu naar hun nieuwe plek. Voorbeeld: B1 bevat **=A1+A2**. Knip A1:A2 en plak in E4, dan wordt B1 **=E4+E5**. Wat er stond in de cellen waarover u plakt, is weg (**Ongedaan maken** haalt het terug), en een formule die een van die cellen las, toont **#REF!**, omdat de cel die hij las er niet meer is. Als de geknipte cellen maar een deel van een samengevoegde cel zijn, of u plakt over maar een deel van een samengevoegde cel, wordt niets verplaatst: selecteer de hele samengevoegde cel. Wanneer u cellen naar een ander blad knipt, blijven de formules de cellen lezen die ze eerder lazen (ze geven aan welk blad dat is). Het klembord wordt na een knip- en plakactie leeggemaakt.

## Plakken speciaal

**Ctrl+Alt+V**, of de pijl onder **Plakken** op het tabblad Start, laat u kiezen wat er wordt geplakt:

- **Alles**, **Formules**, **Waarden** of **Opmaak**. *Waarden* zet een formule om in het getal of de tekst die hij toont.
- **Alles behalve randen**: alles, maar de cellen behouden de randen die ze hadden.
- **Formules en getalnotaties**, of **Waarden en getalnotaties**: de formules (of de waarden) komen mee met de manier waarop de getallen worden getoond, zoals procent of datum, maar niet de kleuren, lettertypen of randen. De cellen behouden de uitstraling die ze hadden.
- **Kolombreedten**: de kolommen waarop u plakt, krijgen de breedten van de gekopieerde kolommen.
- **Opmerkingen en notities**: de notities van de gekopieerde cellen worden op de cellen gezet waarop u plakt. Geen celwaarde verandert.
- Een **bewerking**: **Optellen**, **Aftrekken**, **Vermenigvuldigen** of **Delen**. De gekopieerde getallen worden gecombineerd met de getallen die al in de cellen staan. Voorbeeld: de cellen bevatten 10 en 20, u hebt 5 gekopieerd en u kiest *Vermenigvuldigen*: ze worden 50 en 100.
- **Lege cellen overslaan**: lege cellen in de kopie laten de cellen waarop ze terechtkomen zoals ze waren.
- **Transponeren**: rijen worden kolommen en kolommen worden rijen.

## Koppeling plakken, Als hyperlink plakken en Tekst importeren plakken

Drie andere keuzes staan in de pijl onder **Plakken**. Ze werken op cellen die u in dezelfde werkmap hebt gekopieerd, behalve de laatste.

- **Koppeling plakken** zet in elke cel een formule die verwijst naar de gekopieerde cel. Voorbeeld: kopieer A1:B2, selecteer D4 en gebruik **Koppeling plakken**: D4 wordt **=Blad1!$A$1**. Wanneer A1 verandert, verandert D4 mee. Een gekopieerde lege cel toont 0.
- **Als hyperlink plakken** zet in elke cel dezelfde woorden die de gekopieerde cel toont, en maakt van elk ervan een koppeling naar de gekopieerde cel. Houd **Ctrl** ingedrukt en klik erop om naar die cel te gaan. Het werkt op maximaal 2.000 cellen tegelijk.
- **Tekst importeren plakken** is bedoeld voor tekst uit een ander programma. Het opent hetzelfde venster als het openen van een tekstbestand: u geeft aan wat de delen scheidt (een komma, een tab, een spatie, een puntkomma, een verticale streep of iets anders), welke aanhalingstekens een deel omsluiten, de eerste rij die moet worden gebruikt, en hoe elke kolom moet worden gelezen (zoals getypt, als tekst, als datum, of overslaan). Sommige tekst zet de delen in kolommen van gelijke breedte, met spaties ertussen. Vink **De kolommen hebben een vaste breedte** aan en geef aan waar elke kolom begint: elke regel wordt op die plaatsen gesplitst, en de spaties rond elk deel worden verwijderd. Kies *Tekst* voor een kolom met codes zoals 007, zodat de nullen blijven staan. De delen komen in de cellen, beginnend bij de geselecteerde cel, als één stap die u ongedaan kunt maken.

## Tekst met komma's, puntkomma's of kolommen

Wanneer de tekst die u uit een ander programma plakt duidelijk een tabel is, opent JeffCrunch hetzelfde venster als het openen van een tekstbestand, zodat u kunt controleren hoe de tekst wordt gesplitst voordat er iets in de cellen komt. Dit gebeurt alleen bij een gewoon **Plakken** (**{key:Paste}**), nooit bij **Plakken speciaal** of een van de keuzes daarin. Het gebeurt wanneer de tekst minstens twee regels heeft en geen tab, en wanneer óf elke regel evenveel delen heeft, minstens drie, gescheiden door komma's, puntkomma's of verticale streepjes, met in elke kolom iets (een deel tussen aanhalingstekens mag een komma bevatten); óf de delen staan in kolommen van gelijke breedte, met dezelfde spaties ertussen op elke regel (minstens drie regels en minstens twee tussenruimtes). Getallen met duizendtalkomma's, zoals 1,234, en regels code of lijsten die eindigen op een komma of puntkomma worden niet als tabel gezien. Het venster begint met de splitsing die werd gevonden. Als de getallen komma's als decimaalteken hebben en de delen door puntkomma's zijn gescheiden, is de komma het decimaalteken. Klik op **OK** om de delen erin te zetten, of op **Annuleren** om niets in te voegen.

Voorbeeld: kopieer deze regels uit een e-mail.

    name,qty,price
    Apple,12,1.50
    Pear,3,0.25

Plakt u ze met een derde kolom, zoals een prijs, dan toont het venster drie kolommen, met **Komma** aangevinkt. Een zin op één regel, of regels met een verschillend aantal komma's, of regels met maar twee delen, worden als woorden geplakt, zonder venster. Tekst met tabs gaat rechtstreeks in de cellen, zoals altijd. Wilt u het venster voor elke tekst openen, kies dan **Tekst importeren plakken** (hierboven).

Hetzelfde gebeurt wanneer u tekst uit een ander programma sleept en op het raster neerzet: die komt in de geselecteerde cel. Neerzetten op een andere plek in het venster doet niets.

## Standaardplakken instellen

**Standaardplakken instellen** is de laatste keuze in de pijl onder **Plakken**. Het bepaalt wat de knop **Plakken** en **{key:Paste}** invoegen wanneer u cellen hebt gekopieerd: alles (in het begin), of alleen de waarden, alleen de formules, alleen de opmaak, enzovoort. Voorbeeld: kies *Waarden*, en een gekopieerde formule komt terecht als het getal dat hij toont. De keuze blijft bewaard voor de volgende keer. Het verandert niets aan **Plakken speciaal** of aan de keuzes in het menu, en tekst uit andere programma's wordt zoals die is geplakt. Kies *Alles* om terug te gaan.

## Het plakmenu en afbeeldingen

De pijl onder **Plakken** toont de gebruikelijke keuzes, en ook **Zonder randen**, **Formules en getalnotaties**, **Waarden en getalnotaties**, **Bronkolombreedten behouden** en **Opmerkingen en notities**. Het zijn dezelfde keuzes als in Plakken speciaal, met één klik per keuze.

De pijl naast **Kopiëren** bevat **Kopiëren als afbeelding** en **Kopiëren als bitmap**. Elk zet een afbeelding van de geselecteerde cellen op het klembord, getekend zoals u ze ziet, met de rasterlijnen. Een bitmap heeft de grootte van het scherm. Een afbeelding heeft twee keer zoveel punten, zodat ze scherp blijft wanneer u haar vergroot. **Kopiëren als afgedrukte afbeelding** tekent de cellen zoals het papier ze zou tonen: de rasterlijnen staan er alleen op als de pagina-instelling ze afdrukt. Plak haar in JeffWrite of JeffDeck als afbeelding. Past de selectie niet in één afbeelding, selecteer dan minder cellen.

## Tussen programma's

Wanneer u kopieert, zet JeffCrunch de cellen ook op het klembord als tekst (cellen gescheiden door tabs, één rij per regel) en als tabel voor webpagina's, zodat andere programma's ze kunnen gebruiken. Wanneer u iets plakt dat niet uit JeffCrunch komt, tekst met tabs, of een tabel van een webpagina of een ander spreadsheet, komt elk deel in een eigen cel en wordt het gelezen alsof u het zelf had getypt: getallen, datums en formules die beginnen met een is-gelijk-teken. Kopiëren uit een andere werkmap die in JeffCrunch open staat, brengt waarden, formules en getalnotaties mee, maar niet de overige opmaak. Plakt u ze in JeffDeck of JeffPub, dan komen ze als tabel terecht die de lettertypen, arceringen, randen en de getallen zoals ze worden getoond behoudt (zie **Cellen als tabel plakken** in de Help van die programma's).

## Wat niet kan worden geplakt

JeffCrunch laat u dit weten en verandert niets wanneer het plakken zou reiken in een deel van een **matrixformule**, over de rij met kolomnamen van een **tabel**, in vergrendelde cellen van een **beveiligd blad**, voorbij het einde van het blad, of in meer dan 100.000 cellen. Een hele **matrixformule** kan worden geknipt en verplaatst (selecteer al zijn cellen); een deel ervan, een gegevenstabel, of de rij met namen van een tabel kan dat niet.
