# Knippen, kopiëren en plakken

<!-- keywords: knippen, kopiëren, plakken, klembord, verplaatsen, dupliceren, speciaal, waarden, formules, opmaak, transponeren, lege cellen overslaan, optellen, aftrekken, vermenigvuldigen, delen, bewerking, samengevoegd, tekst, tabel, html, webpagina, ander programma, Escape, marcherende mieren, rand, kolombreedten, notities, opmerkingen, getalnotaties, alles behalve randen, afbeelding, bitmap, kopiëren als afbeelding, afgedrukte afbeelding, zoals afgedrukt, koppeling plakken, hyperlink, tekstimport, tekst splitsen, scheidingsteken, standaard plakken instellen, standaardplakken, vaste breedte -->

**Kopiëren** (**{key:Copy}**) zet een kopie van de geselecteerde cellen op het klembord, een plek waar de computer één ding tegelijk bewaart. **Plakken** (**{key:Paste}**) zet wat op het klembord staat in de cellen, beginnend bij de geselecteerde cel. **Knippen** (**{key:Cut}**) werkt als kopiëren, maar bij het plakken worden de cellen verplaatst en wordt de oude plek leeggemaakt.

Een bewegende gestreepte rand toont de cellen die zijn gekopieerd of geknipt. Druk op **Esc** om de rand te verwijderen; de cellen zijn dan niet meer klaar om te worden geplakt. Geknipte cellen worden ook losgelaten wanneer u vóór het plakken een andere wijziging aanbrengt (typen, invoegen, ongedaan maken). Knip ze dan opnieuw als u ze nog wilt verplaatsen.

## Plakken binnen JeffCrunch

Alles komt mee: de waarden, de formules, de opmaak en de samengevoegde cellen. Een formule blijft verwijzen naar dezelfde plaatsen *ten opzichte van waar hij staat*. Voorbeeld: kopieer **=A1*2** uit B1 en plak het in B5, dan wordt het **=A5*2**. Een deel met een dollarteken, zoals **$A$1**, blijft zoals het is. Als u een bereik selecteert dat een geheel aantal keer zo groot is als wat u hebt gekopieerd, wordt de kopie herhaald om het te vullen: kopieer één cel, selecteer twintig cellen, plak, en alle twintig krijgen de inhoud.

Een geplakt getal dat te breed is voor een kolom met nog de oorspronkelijke breedte, verbreedt de kolom.

## Knippen verplaatst cellen

Wanneer u cellen knipt en plakt, worden de cellen werkelijk verplaatst. Elke formule die naar hen verwees, verwijst naar hun nieuwe plaats. Voorbeeld: B1 bevat **=A1+A2**. Knip A1:A2 en plak in E4, dan wordt B1 **=E4+E5**. Wat in de cellen stond waar u overheen plakt, is weg (**Ongedaan maken** haalt het terug), en een formule die een van die cellen las, toont **#VERW!**, omdat de cel die hij las niet meer bestaat. Als de cellen die u knipt maar een deel van een samengevoegde cel zijn, of u plakt over maar een deel van een samengevoegde cel, wordt niets verplaatst: selecteer de hele samengevoegde cel. Wanneer u cellen naar een ander blad knipt, blijven de formules lezen uit de cellen die ze eerder lazen (ze vermelden het blad). Het klembord wordt geleegd na een knippen en plakken.

## Plakken speciaal

**Ctrl+Alt+V**, of de pijl onder **Plakken** op het tabblad Start, laat u kiezen wat er wordt geplakt:

- **Alles**, **Formules**, **Waarden** of **Opmaak**. *Waarden* verandert een formule in het getal of de tekst die hij toont.
- **Alles behalve randen**: alles, maar de cellen behouden de randen die ze hadden.
- **Formules en getalnotaties** of **Waarden en getalnotaties**: de formules (of de waarden) komen mee met de manier waarop de getallen worden weergegeven, zoals procent of datum, maar niet de kleuren, lettertypen of randen. De cellen behouden hun eigen uiterlijk.
- **Kolombreedten**: de kolommen waarin u plakt, krijgen de breedten van de gekopieerde kolommen.
- **Opmerkingen en notities**: de notities van de gekopieerde cellen worden op de cellen gezet waarin u plakt. Geen celwaarde verandert.
- Een **bewerking**: **Optellen**, **Aftrekken**, **Vermenigvuldigen** of **Delen**. De gekopieerde getallen worden gecombineerd met de getallen die al in de cellen staan. Voorbeeld: de cellen bevatten 10 en 20, u kopieert 5 en kiest *Vermenigvuldigen*: ze worden 50 en 100.
- **Lege cellen overslaan**: lege cellen in de kopie laten de cellen waarop ze terechtkomen zoals ze waren.
- **Transponeren**: rijen worden kolommen en kolommen worden rijen.

## Koppeling plakken, plakken als hyperlink en tekstimport plakken

Drie andere keuzes staan in de pijl onder **Plakken**. Ze werken op cellen die u in dezelfde werkmap hebt gekopieerd, behalve de laatste.

- **Koppeling plakken** zet in elke cel een formule die verwijst naar de gekopieerde cel. Voorbeeld: kopieer A1:B2, selecteer D4 en gebruik **Koppeling plakken**: D4 wordt **=Blad1!$A$1**. Wanneer A1 verandert, verandert D4 mee. Een gekopieerde lege cel toont 0.
- **Plakken als hyperlink** zet in elke cel dezelfde woorden als de gekopieerde cel toont, en maakt elk ervan een koppeling naar de gekopieerde cel. Houd **Ctrl** ingedrukt en klik erop om naar die cel te gaan. Het werkt op maximaal 2.000 cellen tegelijk.
- **Tekstimport plakken** is bedoeld voor tekst uit een ander programma. Het opent hetzelfde venster als het openen van een tekstbestand: u geeft aan wat de onderdelen scheidt (een komma, een tabteken, een spatie, een puntkomma, een verticale streep of iets anders), wat een onderdeel omsluit, de eerste rij die moet worden gebruikt, en hoe elke kolom moet worden gelezen (zoals getypt, als tekst, als datum, of overslaan). Sommige tekst zet de onderdelen in kolommen van gelijke breedte, met spaties ertussen. Vink **De kolommen hebben een vaste breedte** aan en geef aan waar elke kolom begint: elke regel wordt op die plaatsen geknipt en de spaties rond elk onderdeel worden verwijderd. Kies *Tekst* voor een kolom met codes zoals 007, zodat de nullen behouden blijven. De onderdelen komen in cellen, beginnend bij de geselecteerde cel, als één stap die u ongedaan kunt maken.

## Standaard plakken instellen

**Standaardplakken instellen** is de laatste keuze in de pijl onder **Plakken**. Die bepaalt wat de knop **Plakken** en **{key:Paste}** plakken wanneer u cellen hebt gekopieerd: alles (in het begin), of alleen de waarden, alleen de formules, alleen de opmaak, enzovoort. Voorbeeld: kies *Waarden*, dan komt een gekopieerde formule terecht als het getal dat hij toont. De keuze blijft bewaard voor de volgende keer. Ze verandert niet **Plakken speciaal** of de keuzes in het menu, en tekst uit andere programma's wordt zoals die is geplakt. Kies *Alles* om terug te gaan.

## Het menu Plakken en afbeeldingen

De pijl onder **Plakken** bevat de gebruikelijke keuzes, en ook **Geen randen**, **Formules en getalopmaak**, **Waarden en getalopmaak**, **Brontkolombreedten behouden** en **Opmerkingen en notities**. Ze zijn gelijk aan de keuzes in Plakken speciaal, met één klik per keuze.

De pijl naast **Kopiëren** bevat **Kopiëren als afbeelding** en **Kopiëren als bitmap**. Elk zet een afbeelding van de geselecteerde cellen op het klembord, getekend zoals u ze ziet, met de rasterlijnen. Een bitmap heeft de grootte van het scherm. Een afbeelding heeft twee keer zoveel punten, dus blijft scherp wanneer u ze vergroot. **Kopiëren als afgedrukte afbeelding** tekent de cellen zoals het papier ze zou tonen: de rasterlijnen staan er alleen als de paginainstelling ze afdrukt. Plak het in JeffWrite of JeffDeck als afbeelding. Als de cellen niet in één afbeelding passen, selecteer dan minder cellen.

## Tussen programma's

Wanneer u kopieert, zet JeffCrunch de cellen ook als tekst op het klembord (cellen gescheiden door tabtekens, één rij per regel) en als tabel voor webpagina's, zodat andere programma's ze kunnen gebruiken. Wanneer u iets plakt dat niet uit JeffCrunch komt, tekst met tabtekens, of een tabel van een webpagina of een ander rekenblad, komt elk onderdeel in een eigen cel en wordt het gelezen alsof u het hebt getypt: getallen, datums en formules die beginnen met een gelijkteken. Kopiëren uit een andere werkmap die in JeffCrunch open is, brengt waarden, formules en getalnotaties mee, maar niet de overige opmaak.

## Wat niet kan worden geplakt

JeffCrunch meldt het en verandert niets wanneer het plakken zou reiken tot in een deel van een **matrixformule**, over de rij met kolomnamen van een **tabel**, in vergrendelde cellen van een **beveiligd blad**, voorbij het einde van het blad, of in meer dan 100.000 cellen. Een volledige **matrixformule** kan worden geknipt en verplaatst (selecteer al zijn cellen); een deel ervan, een gegevenstabel, of de rij met namen van een tabel kan dat niet.
