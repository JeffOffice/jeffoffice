# Knippen, kopiëren en plakken

<!-- keywords: knippen, kopiëren, plakken, klembord, verplaatsen, dupliceren, speciaal plakken, waarden, formules, opmaak, transponeren, lege cellen overslaan, optellen, aftrekken, vermenigvuldigen, delen, bewerking, samengevoegd, tekst, tabblad, html, webpagina, ander programma, Escape, lopende mieren, rand, kolombreedten, notities, opmerkingen, getalnotaties, alles behalve randen, afbeelding, bitmap, kopiëren als afbeelding, afgedrukte afbeelding, zoals afgedrukt, koppeling plakken, hyperlink, tekst importeren, tekst met scheidingstekens, tekst neerzetten, komma, puntkomma, csv, scheidingsteken, standaardplakken instellen, standaard plakken, vaste breedte -->

**Kopiëren** (**{key:Copy}**) zet een kopie van de geselecteerde cellen op het klembord, een plek waar de computer één ding tegelijk bewaart. **Plakken** (**{key:Paste}**) zet wat op het klembord staat in de cellen, beginnend bij de geselecteerde cel. **Knippen** (**{key:Cut}**) werkt als kopiëren, maar bij het plakken worden de cellen verplaatst en wordt de oude plek leeggemaakt.

Een bewegende gestreepte rand toont de cellen die zijn gekopieerd of geknipt. Druk op **Esc** om die rand weg te halen; de cellen zijn dan niet meer klaar om te worden geplakt. Geknipte cellen worden ook losgelaten wanneer u vóór het plakken een andere wijziging maakt (typen, invoegen, ongedaan maken). Knip ze dan opnieuw als u ze nog wilt verplaatsen.

## Plakken binnen JeffCrunch

Alles komt mee: de waarden, de formules, de opmaak en de samengevoegde cellen. Een formule blijft verwijzen naar dezelfde plaatsen *ten opzichte van de plek waar hij staat*. Voorbeeld: kopieer **=A1*2** uit B1 en plak die in B5, dan wordt het **=A5*2**. Een deel met een dollarteken, zoals **$A$1**, blijft zoals het is. Als u een blok selecteert dat een geheel aantal keer zo groot is als wat u hebt gekopieerd, wordt de kopie herhaald om het te vullen: kopieer één cel, selecteer twintig cellen, plak, en alle twintig krijgen de inhoud.

Een geplakt getal dat te breed is voor een kolom die nog zijn oorspronkelijke breedte heeft, maakt de kolom breder.

## Knippen verplaatst cellen

Wanneer u cellen knipt en plakt, worden de cellen echt verplaatst. Elke formule die naar die cellen verwees, verwijst nu naar hun nieuwe plek. Voorbeeld: B1 bevat **=A1+A2**. Knip A1:A2 en plak in E4, dan wordt B1 **=E4+E5**. Wat in de cellen stond waarop u plakt, is weg (**Ongedaan maken** haalt het terug), en een formule die een van die cellen las, toont **#VERW!**, omdat de cel die hij las er niet meer is. Als de geknipte cellen maar een deel van een samengevoegde cel zijn, of u plakt maar over een deel van een samengevoegde cel, wordt niets verplaatst: selecteer de hele samengevoegde cel. Wanneer u cellen naar een ander blad knipt, blijven de formules lezen uit de cellen die ze eerder lazen (ze vermelden welk blad dat is). Het klembord wordt geleegd na een knip- en plakactie.

## Speciaal plakken

**Ctrl+Alt+V**, of de pijl onder **Plakken** op het tabblad Start, laat u kiezen wat er wordt geplakt:

- **Alles**, **Formules**, **Waarden** of **Opmaak**. *Waarden* zet een formule om in het getal of de tekst die hij toont.
- **Alles behalve randen**: alles, maar de cellen houden de randen die ze hadden.
- **Formules en getalnotaties**, of **Waarden en getalnotaties**: de formules (of de waarden) komen met de manier waarop de getallen worden weergegeven, zoals procent of datum, maar niet met de kleuren, lettertypen of randen. De cellen behouden het uiterlijk dat ze hadden.
- **Kolombreedten**: de kolommen waarin u plakt, krijgen de breedten van de gekopieerde kolommen.
- **Opmerkingen en notities**: de notities van de gekopieerde cellen worden op de cellen gezet waarin u plakt. Geen celwaarde verandert.
- Een **bewerking**: **Optellen**, **Aftrekken**, **Vermenigvuldigen** of **Delen**. De gekopieerde getallen worden gecombineerd met de getallen die al in de cellen staan. Voorbeeld: de cellen bevatten 10 en 20, u hebt 5 gekopieerd en *Vermenigvuldigen* gekozen: ze worden 50 en 100.
- **Lege cellen overslaan**: lege cellen in de kopie laten de cellen waarop ze terechtkomen zoals ze waren.
- **Transponeren**: rijen worden kolommen en kolommen worden rijen.

## De knop Plakopties

Direct nadat u cellen hebt geplakt die u in JeffCrunch hebt gekopieerd, verschijnt bij de laatst geplakte cel een kleine knop **Plakopties**. Klik erop om een andere manier te kiezen waarop dezelfde cellen worden ingevoegd: **Bronopmaak behouden** (alles), **Waarden**, **Formules**, **Opmaak**, **Geen randen**, **Formules en getalnotaties**, **Waarden en getalnotaties**, **Bronkolombreedten behouden** of **Transponeren**. JeffCrunch haalt het plakken terug en voert het opnieuw uit op die manier. De wijziging is één stap: met één keer **Ongedaan maken** haalt u het hele plakken terug, hoe vaak u ook van gedachten bent veranderd. Voorbeeld: kopieer A1:B2, waar B1 **=A1*2** is, en plak in F5. Kies **Waarden** en G5 bevat het getal 2 in plaats van **=F5*2**; kies **Bronopmaak behouden** en de formule is er weer.

Vanaf het toetsenbord tikt u na een plakactie op de toets **Ctrl** (de toets **Control** op een Mac) om het menu van de knop te openen, en kiest u daarna met de pijltoetsen en **Enter**. De knop verdwijnt wanneer u een cel of iets anders in de werkmap wijzigt, iets anders kopieert, op **Esc** drukt, of de laatst geplakte cel uit het zicht scrolt (hij komt terug wanneer u terugscrolt). Hij wordt niet aangeboden na knippen en plakken, of voor tekst uit een ander programma.

## Koppeling plakken, Plakken als hyperlink en Tekst importeren plakken

Er zijn nog drie keuzes in de pijl onder **Plakken**. Ze werken op cellen die u in dezelfde werkmap hebt gekopieerd, behalve de laatste.

- **Koppeling plakken** zet in elke cel een formule die verwijst naar de gekopieerde cel. Voorbeeld: kopieer A1:B2, selecteer D4 en gebruik **Koppeling plakken**: D4 wordt **=Blad1!$A$1**. Wanneer A1 verandert, verandert D4 mee. Een gekopieerde cel die leeg is, toont 0.
- **Plakken als hyperlink** zet in elke cel dezelfde woorden die de gekopieerde cel toont, en maakt van elk ervan een koppeling naar de gekopieerde cel. Houd **Ctrl** ingedrukt en klik erop om naar die cel te gaan. Het werkt op maximaal 2.000 cellen tegelijk.
- **Tekst importeren plakken** is bedoeld voor tekst uit een ander programma. Het opent hetzelfde venster als het openen van een tekstbestand: u geeft aan wat de delen scheidt (een komma, een tab, een spatie, een puntkomma, een verticale streep of iets anders), wat een deel tussen aanhalingstekens zet, de eerste rij die moet worden gebruikt, en hoe elke kolom moet worden gelezen (zoals getypt, als tekst, als datum, of overslaan). Sommige tekst zet de delen in kolommen van dezelfde breedte, met spaties ertussen. Vink **De kolommen hebben een vaste breedte** aan en geef op waar elke kolom begint: elke regel wordt op die plaatsen geknipt, en de spaties rond elk deel worden verwijderd. Kies *Tekst* voor een kolom met codes zoals 007, zodat de nullen behouden blijven. De delen gaan in de cellen, beginnend bij de geselecteerde cel, als één stap die u ongedaan kunt maken.

## Tekst met komma's, puntkomma's of kolommen

Wanneer de tekst die u uit een ander programma plakt duidelijk een tabel is, opent JeffCrunch hetzelfde venster als bij het openen van een tekstbestand, zodat u kunt controleren hoe de tekst wordt gesplitst voordat er iets in de cellen komt. Dit gebeurt alleen bij een gewone **Plakken** (**{key:Paste}**), nooit bij **Speciaal plakken** of een van de keuzes daarin. Het gebeurt wanneer de tekst minstens twee regels heeft en geen tab bevat, en wanneer óf elke regel evenveel delen heeft, minstens drie, gescheiden door komma's, puntkomma's of verticale streepjes, met in elke kolom iets; (een deel tussen aanhalingstekens mag een komma bevatten) óf de delen staan in kolommen van dezelfde breedte, met op elke regel dezelfde spatiegaten (minstens drie regels en minstens twee gaten). Getallen met duizendtalkomma's, zoals 1,234, en regels code of lijsten die op een komma of puntkomma eindigen, worden niet als tabel gezien. Het venster begint met de splitsing die voor de tekst is gevonden. Als de getallen komma's bevatten en de delen door puntkomma's zijn gescheiden, is de komma het decimaalteken. Klik op **OK** om de delen in te voegen, of op **Annuleren** om niets in te voegen.

Voorbeeld: kopieer deze regels uit een e-mail.

    naam,aantal,prijs
    Appel,12,1.50
    Peer,3,0.25

Plak ze met een derde kolom, zoals een prijs, dan toont het venster drie kolommen, met **Komma** aangevinkt. Eén zin op één regel, of regels met een verschillend aantal komma's, of slechts twee delen per regel, worden als woorden geplakt, zonder venster. Tekst met tabs gaat rechtstreeks in de cellen, zoals altijd. Om het venster voor elke tekst te openen, kiest u **Tekst importeren plakken** (zie hierboven).

Hetzelfde gebeurt wanneer u tekst uit een ander programma sleept en op het raster neerzet: die gaat in de geselecteerde cel. Een neerzetting ergens anders in het venster doet niets.

## Standaardplakken instellen

**Standaardplakken instellen** is de laatste keuze in de pijl onder **Plakken**. Die bepaalt wat de knop **Plakken** en **{key:Paste}** invoegen wanneer u cellen hebt gekopieerd: alles (in het begin), of alleen de waarden, alleen de formules, alleen de opmaak, enzovoort. Voorbeeld: kies *Waarden*, dan wordt een gekopieerde formule het getal dat hij toont. De keuze blijft bewaard voor de volgende keer. Ze verandert niets aan **Speciaal plakken** of aan de keuzes in het menu, en tekst uit andere programma's wordt zoals die is geplakt. Kies *Alles* om terug te gaan.

Dezelfde keuze staat op de pagina **Bewerken** van **Opties**, in twee vakken: één voor cellen die u in deze werkmap hebt gekopieerd, en één voor cellen die u in een andere werkmap hebt gekopieerd. Totdat u voor een andere werkmap een keuze maakt, volgt die de eerste.

## Het menu Plakken en afbeeldingen

De pijl onder **Plakken** toont de gebruikelijke keuzes, en daarnaast **Geen randen**, **Formules en getalnotaties**, **Waarden en getalnotaties**, **Bronkolombreedten behouden** en **Opmerkingen en notities**. Ze zijn gelijk aan de keuzes in Speciaal plakken, met één klik per keuze.

De pijl naast **Kopiëren** bevat **Kopiëren als afbeelding** en **Kopiëren als bitmap**. Elk zet een afbeelding van de geselecteerde cellen op het klembord, getekend zoals u ze ziet, met de rasterlijnen. Een bitmap heeft de grootte van het scherm. Een afbeelding heeft twee keer zoveel punten, zodat hij scherp blijft wanneer hij wordt vergroot. **Kopiëren als afgedrukte afbeelding** tekent de cellen zoals het papier ze zou tonen: de rasterlijnen staan er alleen op als de paginainstelling ze afdrukt. Plak die in JeffWrite of JeffDeck als afbeelding. Als de cellen niet in één afbeelding passen, selecteert u er minder.

## Tussen programma's

Wanneer u kopieert, zet JeffCrunch de cellen ook als tekst op het klembord (cellen gescheiden door tabs, één rij per regel) en als tabel voor webpagina's, zodat andere programma's ze kunnen gebruiken. Wanneer u iets plakt dat niet uit JeffCrunch komt, tekst met tabs, of een tabel van een webpagina of een ander spreadsheet, dan komt elk deel in een eigen cel en wordt het gelezen alsof u het hebt getypt: getallen, datums en formules die met een is-gelijkteken beginnen. Kopiëren uit een andere werkmap die in JeffCrunch open is, levert waarden, formules en getalnotaties op, maar niet de overige opmaak. Plakt u ze in JeffDeck of JeffPub, dan komen ze als tabel die de lettertypen, arceringen, randen en de getallen zoals ze worden weergegeven behoudt (zie **Cellen als tabel plakken** in de Help daarvan).

## Wat niet kan worden geplakt

JeffCrunch laat het weten en wijzigt niets wanneer het plakken zou reiken tot in een deel van een **matrixformule**, over de rij met kolomnamen van een **tabel**, in vergrendelde cellen van een **beveiligd blad**, voorbij het einde van het blad, of in meer dan 100.000 cellen. Een hele **matrixformule** kan worden geknipt en verplaatst (selecteer al zijn cellen); een deel ervan, een gegevenstabel, of de rij met namen van een tabel kan dat niet.
