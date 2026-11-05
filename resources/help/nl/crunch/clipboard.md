# Knippen, kopiëren en plakken

<!-- keywords: knippen, kopiëren, plakken, klembord, verplaatsen, dupliceren, speciaal, waarden, formules, opmaak, transponeren, lege cellen overslaan, optellen, aftrekken, vermenigvuldigen, delen, bewerking, samengevoegd, tekst, tabel, html, webpagina, ander programma, Escape, zwevende rand, rand, kolombreedten, notities, opmerkingen, getalnotaties, alle behalve randen, afbeelding, bitmap, kopiëren als afbeelding, afgedrukte afbeelding, als afgedrukt, koppeling plakken, hyperlink, tekstimport, tekst splitsen, scheidingsteken -->

**Kopiëren** (**{key:Ctrl+C}**) zet een kopie van de geselecteerde cellen op het klembord, een plek waar de computer één ding tegelijk bewaart. **Plakken** (**{key:Ctrl+V}**) zet wat op het klembord staat in de cellen, beginnend bij de geselecteerde cel. **Knippen** (**{key:Ctrl+X}**) werkt als kopiëren, maar bij het plakken worden de cellen verplaatst en wordt de oude plek leeg.

Een bewegende gestippelde rand toont de cellen die zijn gekopieerd of geknipt. Druk op **Esc** om die rand weg te halen; de cellen zijn dan niet meer klaar om te worden geplakt. Geknipte cellen worden ook losgelaten wanneer u vóór het plakken een andere wijziging maakt (typen, invoegen, ongedaan maken). Knip ze dan opnieuw als u ze nog wilt verplaatsen.

## Plakken binnen JeffCrunch

Alles komt mee: de waarden, de formules, de opmaak en de samengevoegde cellen. Een formule blijft verwijzen naar dezelfde plaatsen *ten opzichte van de plek waar hij staat*. Voorbeeld: kopieer **=A1*2** uit B1 en plak het in B5, dan wordt het **=A5*2**. Een deel met een dollarteken, zoals **$A$1**, blijft zoals het is. Als u een blok selecteert dat een geheel aantal keer zo groot is als wat u hebt gekopieerd, wordt de kopie herhaald om het te vullen: kopieer één cel, selecteer twintig cellen, plak, en alle twintig krijgen de waarde.

Een geplakt getal dat te breed is voor een kolom met nog de oorspronkelijke breedte, maakt de kolom breder.

## Knippen verplaatst cellen

Wanneer u cellen knipt en plakt, worden de cellen echt verplaatst. Elke formule die naar hen verwees, verwijst naar hun nieuwe plek. Voorbeeld: B1 bevat **=A1+A2**. Knip A1:A2 en plak op E4, dan wordt B1 **=E4+E5**. Wat in de cellen stond waar u overheen plakt, is weg (**Ongedaan maken** haalt het terug), en een formule die een van die cellen las, toont **#VERW!**, omdat de cel die hij las er niet meer is. Als de geknipte cellen slechts een deel van een samengevoegde cel zijn, of u plakt over slechts een deel ervan, wordt niets verplaatst: selecteer de hele samengevoegde cel. Wanneer u cellen naar een ander blad knipt, blijven de formules lezen uit de cellen die ze vóór het knippen lazen (ze vermelden het blad). Het klembord wordt geleegd na een knip- en plakactie.

## Speciaal plakken

**Ctrl+Alt+V**, of de pijl onder **Plakken** op het tabblad Start, laat u kiezen wat u plakt:

- **Alles**, **Formules**, **Waarden** of **Opmaak**. *Waarden* verandert een formule in het getal of de tekst die hij toont.
- **Alles behalve randen**: alles, maar de cellen behouden de randen die ze hadden.
- **Formules en getalnotatie**, of **Waarden en getalnotatie**: de formules (of de waarden) komen mee met de manier waarop de getallen worden weergegeven, zoals procent of een datum, maar niet de kleuren, lettertypen of randen. De cellen behouden de uitstraling die ze hadden.
- **Kolombreedten**: de kolommen waarin u plakt, krijgen de breedten van de gekopieerde kolommen.
- **Opmerkingen en notities**: de notities van de gekopieerde cellen worden op de cellen gezet waarin u plakt. Geen celwaarde verandert.
- Een **bewerking**: **Optellen**, **Aftrekken**, **Vermenigvuldigen** of **Delen**. De gekopieerde getallen worden gecombineerd met de getallen die al in de cellen staan. Voorbeeld: de cellen bevatten 10 en 20, u hebt 5 gekopieerd en koos *Vermenigvuldigen*: ze worden 50 en 100.
- **Lege cellen overslaan**: lege cellen in de kopie laten de cellen waarop ze landen zoals ze waren.
- **Transponeren**: rijen worden kolommen en kolommen worden rijen.

## Koppeling plakken, Plakken als hyperlink en Tekstimport plakken

Drie andere keuzes staan in de pijl onder **Plakken**. Ze werken op cellen die u in dezelfde werkmap hebt gekopieerd, behalve de laatste.

- **Koppeling plakken** zet in elke cel een formule die naar de gekopieerde cel verwijst. Voorbeeld: kopieer A1:B2, selecteer D4 en gebruik **Koppeling plakken**: D4 wordt **=Blad1!$A$1**. Wanneer A1 verandert, verandert D4 mee. Een gekopieerde lege cel toont 0.
- **Plakken als hyperlink** zet in elke cel dezelfde woorden die de gekopieerde cel toont, en maakt elk ervan een koppeling naar de gekopieerde cel. Houd **Ctrl** ingedrukt en klik erop om naar die cel te gaan. Het werkt op maximaal 2.000 cellen tegelijk.
- **Tekstimport plakken** is bedoeld voor tekst uit een ander programma. Het opent hetzelfde venster als het openen van een tekstbestand: u geeft aan wat de onderdelen scheidt (een komma, een tab, een spatie, een puntkomma, een verticale streep of iets anders), wat een onderdeel omsluit, de eerste rij die moet worden gebruikt en hoe elke kolom wordt gelezen (zoals getypt, als tekst, als datum, of weggelaten). Kies *Tekst* voor een kolom met codes zoals 007, zodat de nullen blijven staan. De onderdelen komen in cellen, beginnend bij de geselecteerde cel, als één stap die u ongedaan kunt maken.

## Het menu Plakken en afbeeldingen

De pijl onder **Plakken** toont de gebruikelijke keuzes, en ook **Geen randen**, **Formules en getalnotatie**, **Waarden en getalnotatie**, **Bronkolombreedten behouden** en **Opmerkingen en notities**. Ze zijn gelijk aan de keuzes in Speciaal plakken, met één klik per keuze.

De pijl naast **Kopiëren** bevat **Kopiëren als afbeelding** en **Kopiëren als bitmap**. Elk zet een afbeelding van de geselecteerde cellen op het klembord, getekend zoals u ze ziet, met de rasterlijnen. Een bitmap heeft het formaat van het scherm. Een afbeelding heeft twee keer zoveel punten, zodat ze scherp blijft wanneer u ze vergroot. **Kopiëren als afgedrukte afbeelding** tekent de cellen zoals het papier ze zou tonen: de rasterlijnen staan er alleen op als de pagina-instelling ze afdrukt. Plak ze in JeffWrite of JeffDeck als afbeelding. Als de cellen niet in één afbeelding passen, selecteert u er minder.

## Tussen programma's

Wanneer u kopieert, zet JeffCrunch de cellen ook als tekst op het klembord (cellen gescheiden door tabs, één rij per regel) en als tabel voor webpagina's, zodat andere programma's ze kunnen gebruiken. Wanneer u iets plakt dat niet uit JeffCrunch komt, zoals tekst met tabs, of een tabel van een webpagina of een ander spreadsheet, komt elk onderdeel in een eigen cel en wordt het gelezen alsof u het hebt getypt: getallen, datums en formules die beginnen met een is-gelijk-teken. Kopiëren uit een andere werkmap die in JeffCrunch open is, brengt waarden, formules en getalnotaties mee, maar niet de overige opmaak.

## Wat niet kan worden geplakt

JeffCrunch laat het weten en verandert niets wanneer het plakken zou reiken in een deel van een **matrixformule**, over de rij met kolomnamen van een **tabel**, in vergrendelde cellen van een **beveiligd blad**, voorbij het einde van het blad, of in meer dan 100.000 cellen. Een hele **matrixformule** kan worden geknipt en verplaatst (selecteer al zijn cellen); een deel ervan, een gegevenstabel of de rij met namen van een tabel niet.
