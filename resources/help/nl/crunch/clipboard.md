# Knippen, kopiëren en plakken
<!-- keywords: knippen, kopiëren, plakken, klembord, verplaatsen, dupliceren, speciaal plakken, waarden, formules, opmaak, transponeren, lege cellen overslaan, optellen, aftrekken, vermenigvuldigen, delen, bewerking, samengevoegd, tekst, tabblad, html, webpagina, ander programma, Escape, bewegende stippellijn, rand, kolombreedten, notities, opmerkingen, getalnotaties, alle behalve randen, afbeelding, bitmap, kopiëren als afbeelding, afgedrukte afbeelding, zoals afgedrukt, koppeling plakken, hyperlink, tekst importeren, tekst met scheidingstekens, tekst slepen, komma, puntkomma, csv, scheidingsteken, standaard plakken instellen, standaard plakken, vaste breedte, plakken als tekst, delen splitsen -->

**Kopiëren** (**{key:Copy}**) zet een kopie van de geselecteerde cellen op het klembord, een plek waar de computer één ding tegelijk bewaart. **Plakken** (**{key:Paste}**) zet wat op het klembord staat in de cellen, beginnend bij de geselecteerde cel. **Knippen** (**{key:Cut}**) werkt als kopiëren, maar bij het plakken worden de cellen verplaatst en wordt de oude plek leeggemaakt.

Een bewegende stippellijn toont de cellen die zijn gekopieerd of geknipt. Druk op **Esc** om de rand weg te halen; de cellen zijn dan niet meer klaar om te worden geplakt. Geknipte cellen worden ook losgelaten wanneer u vóór het plakken een andere wijziging maakt (typen, invoegen, ongedaan maken): knip ze opnieuw als u ze nog wilt verplaatsen.

## Plakken binnen JeffCrunch

Alles komt mee: de waarden, de formules, de opmaak en de samengevoegde cellen. Een formule verwijst naar dezelfde plaatsen *ten opzichte van de plek waar hij staat*. Voorbeeld: kopieer **=A1*2** uit B1 en plak in B5, dan wordt het **=A5*2**. Een deel met een dollarteken, zoals **$A$1**, blijft zoals het is. Als u een blok selecteert dat een geheel aantal keer zo groot is als wat u hebt gekopieerd, wordt de kopie herhaald om het te vullen: kopieer één cel, selecteer twintig cellen, plak, en alle twintig krijgen de waarde.

Een geplakt getal dat te breed is voor een kolom met nog de oorspronkelijke breedte, maakt de kolom breder.

## Knippen verplaatst cellen

Wanneer u cellen knipt en plakt, worden de cellen echt verplaatst. Elke formule die naar hen verwees, verwijst nu naar hun nieuwe plek. Voorbeeld: B1 bevat **=A1+A2**. Knip A1:A2 en plak in E4, dan wordt B1 **=E4+E5**. Wat er stond in de cellen waarover u plakt, is weg (**Ongedaan maken** haalt het terug), en een formule die een van die cellen las, toont **#VERW!**, omdat de cel die hij las er niet meer is. Als de geknipte cellen maar een deel van een samengevoegde cel zijn, of u plakt over maar een deel ervan, wordt niets verplaatst: selecteer de hele samengevoegde cel. Wanneer u cellen naar een ander blad knipt, blijven de formules lezen uit de cellen die ze eerder lazen (ze vermelden het blad). Het klembord wordt na een knip- en plakactie leeggemaakt.

## Speciaal plakken

**Ctrl+Alt+V**, of de pijl onder **Plakken** op het tabblad Start, laat u kiezen wat u wilt plakken:

- **Alles**, **Formules**, **Waarden** of **Opmaak**. *Waarden* zet een formule om in het getal of de tekst die hij toont.
- **Alle behalve randen**: alles, maar de cellen behouden de randen die ze hadden.
- **Formules en getalnotaties** of **Waarden en getalnotaties**: de formules (of de waarden) komen mee met de manier waarop de getallen worden weergegeven, zoals procent of datum, maar niet de kleuren, lettertypen of randen. De cellen behouden de uitstraling die ze hadden.
- **Kolombreedten**: de kolommen waarin u plakt, krijgen de breedten van de gekopieerde kolommen.
- **Opmerkingen en notities**: de notities van de gekopieerde cellen worden op de cellen gezet waarin u plakt. Geen celwaarde verandert.
- Een **bewerking**: **Optellen**, **Aftrekken**, **Vermenigvuldigen** of **Delen**. De gekopieerde getallen worden gecombineerd met de getallen die al in de cellen staan. Voorbeeld: de cellen bevatten 10 en 20, u hebt 5 gekopieerd en kiest *Vermenigvuldigen*: ze worden 50 en 100.
- **Lege cellen overslaan**: lege cellen in de kopie laten de cellen waarop ze terechtkomen zoals ze waren.
- **Transponeren**: rijen worden kolommen en kolommen worden rijen.

## De knop Plakopties

Direct nadat u cellen hebt geplakt die u in JeffCrunch hebt gekopieerd, verschijnt een kleine knop **Plakopties** naast de laatste geplakte cel. Klik erop om een andere manier te kiezen waarop dezelfde cellen worden geplakt: **Bronopmaak behouden** (alles), **Waarden**, **Formules**, **Opmaak**, **Geen randen**, **Formules en getalnotaties**, **Waarden en getalnotaties**, **Bronkolombreedten behouden** of **Transponeren**. JeffCrunch haalt de plak-actie terug en voert hem opnieuw uit op die manier. De wijziging telt als één stap: één **Ongedaan maken** haalt de hele plak-actie terug, hoe vaak u ook van gedachten bent veranderd. Voorbeeld: kopieer A1:B2, waarbij B1 **=A1*2** is, en plak in F5. Kies **Waarden**, dan bevat G5 het getal 2 in plaats van **=F5*2**; kiest u **Bronopmaak behouden**, dan komt de formule terug.

Met het toetsenbord tikt u na het plakken op de **Ctrl**-toets (op een Mac de **Control**-toets) om het menu van de knop te openen, en kiest u met de pijltoetsen en **Enter**. De knop verdwijnt wanneer u een cel of iets anders in de werkmap wijzigt, iets anders kopieert, op **Esc** drukt of de laatst geplakte cel uit beeld scrolt (hij komt terug als u terugscrolt). Na knippen en plakken, of bij tekst uit een ander programma, wordt de knop niet getoond.

## Koppeling plakken, Plakken als hyperlink en Tekst importeren bij plakken

Drie andere keuzes staan in de pijl onder **Plakken**. Ze werken op cellen die u in dezelfde werkmap hebt gekopieerd, behalve de laatste.

- **Koppeling plakken** zet in elke cel een formule die naar de gekopieerde cel verwijst. Voorbeeld: kopieer A1:B2, selecteer D4 en gebruik **Koppeling plakken**: D4 wordt **=Blad1!$A$1**. Wanneer A1 verandert, verandert D4 mee. Een gekopieerde lege cel toont 0.
- **Plakken als hyperlink** zet in elke cel dezelfde woorden die de gekopieerde cel toont, en maakt elk ervan een koppeling naar de gekopieerde cel. Houd **Ctrl** ingedrukt en klik erop om naar die cel te gaan. Het werkt op maximaal 2.000 cellen tegelijk.
- **Tekst importeren bij plakken** is bedoeld voor tekst uit een ander programma. Het opent hetzelfde venster als het openen van een tekstbestand: u geeft aan wat de onderdelen scheidt (een komma, een tabteken, een spatie, een puntkomma, een verticale streep of iets anders), wat een onderdeel tussen aanhalingstekens zet, vanaf welke rij u wilt beginnen en hoe elke kolom moet worden gelezen (zoals getypt, als tekst, als datum, of weggelaten). Sommige tekst zet onderdelen in kolommen van dezelfde breedte, met spaties ertussen. Vink dan **De kolommen hebben een vaste breedte** aan en geef aan waar elke kolom begint: elke regel wordt op die plaatsen gesneden, en de spaties rond elk onderdeel worden verwijderd. Kies *Tekst* voor een kolom met codes zoals 007, zodat de nullen blijven staan. De onderdelen komen in cellen, beginnend bij de geselecteerde cel, als één stap die u ongedaan kunt maken.

## Tekst met komma's, puntkomma's of kolommen

Wanneer de tekst die u uit een ander programma plakt duidelijk een tabel is, opent JeffCrunch hetzelfde venster als bij het openen van een tekstbestand, zodat u kunt controleren hoe de tekst is gesplitst voordat er iets in de cellen komt. Dit gebeurt alleen bij gewoon **Plakken** (**{key:Paste}**), nooit bij **Speciaal plakken** of een van de keuzes daarvan. Het gebeurt wanneer de tekst minstens twee regels heeft en geen tabteken, en wanneer ofwel elke regel hetzelfde aantal onderdelen heeft, minstens drie, gescheiden door komma's, puntkomma's of verticale streepjes, waarbij in elke kolom iets staat (een onderdeel tussen aanhalingstekens mag een komma bevatten); ofwel de onderdelen in kolommen van dezelfde breedte staan, met op elke regel dezelfde spaties ertussen (minstens drie regels en minstens twee tussenruimten). Getallen met duizendtalkomma's, zoals 1.234, en regels code of lijsten die eindigen op een komma of puntkomma worden niet als tabel gezien. Het venster begint met de splitsing die is gevonden. Als de getallen komma's als decimaalteken hebben en de onderdelen door puntkomma's zijn gescheiden, is de komma het decimaalteken. Klik op **OK** om de onderdelen te plakken, of op **Annuleren** om niets te plakken.

Voorbeeld: kopieer deze regels uit een e-mail.

    naam,aantal,prijs
    Appel,12,1.50
    Peer,3,0.25

Plakt u ze met een derde kolom, zoals een prijs, dan toont het venster drie kolommen, met **Komma** aangevinkt. Eén zin op één regel, of regels met een verschillend aantal komma's, of regels met maar twee onderdelen, worden als woorden geplakt, zonder venster. Tekst met tabs gaat rechtstreeks in de cellen, zoals altijd. Om het venster voor elke willekeurige tekst te openen, kiest u **Tekst importeren bij plakken** (hierboven).

Hetzelfde gebeurt wanneer u tekst uit een ander programma naar het raster slepen: ze komt in de geselecteerde cel. Een neerzetten op een andere plek in het venster doet niets.

## Standaard plakken instellen

**Standaard plakken instellen** is de laatste keuze in de pijl onder **Plakken**. Die bepaalt wat de knop **Plakken** en **{key:Paste}** plakken wanneer u cellen hebt gekopieerd: alles (in het begin), of alleen de waarden, alleen de formules, alleen de opmaak, enzovoort. Voorbeeld: kies *Waarden*, dan komt een gekopieerde formule terecht als het getal dat hij toont. De keuze blijft bewaard voor de volgende keer. Ze verandert niets aan **Speciaal plakken** of aan de keuzes in het menu, en tekst uit andere programma's wordt zoals die is geplakt. Kies *Alles* om terug te keren.

Dezelfde keuze staat op de pagina **Bewerken** van **Opties**, in twee vakken: één voor cellen die u in deze werkmap hebt gekopieerd, en één voor cellen die u in een andere werkmap hebt gekopieerd. Totdat u voor een andere werkmap een keuze maakt, volgt die de eerste.

## Het menu Plakken en afbeeldingen

De pijl onder **Plakken** toont de gebruikelijke keuzes, en ook **Geen randen**, **Formules en getalnotaties**, **Waarden en getalnotaties**, **Bronkolombreedten behouden** en **Opmerkingen en notities**. Ze zijn dezelfde als de keuzes bij Speciaal plakken, met één klik per keuze.

De pijl naast **Kopiëren** heeft **Kopiëren als afbeelding** en **Kopiëren als bitmap**. Elk zet een afbeelding van de geselecteerde cellen op het klembord, getekend zoals u ze ziet, met de rasterlijnen. Een bitmap heeft de grootte van het scherm. Een afbeelding heeft twee keer zoveel stippen, zodat ze scherp blijft als ze groter wordt gemaakt. **Kopiëren als afgedrukte afbeelding** tekent de cellen zoals het papier ze zou tonen: de rasterlijnen staan er alleen in als de pagina-instelling ze afdrukt. Plak die als afbeelding in JeffWrite of JeffDeck. Passen de cellen niet in één afbeelding, selecteer dan minder cellen.

## Tussen programma's

Wanneer u kopieert, zet JeffCrunch de cellen ook als tekst op het klembord (cellen gescheiden door tabs, één rij per regel) en als tabel voor webpagina's, zodat andere programma's ze kunnen gebruiken. Wanneer u iets plakt dat niet uit JeffCrunch komt, zoals tekst met tabs of een tabel van een webpagina of een ander spreadsheet, komt elk onderdeel in een eigen cel en wordt het gelezen alsof u het had getypt: getallen, datums en formules die beginnen met een is-gelijk-teken. Kopiëren uit een andere werkmap die in JeffCrunch is geopend, brengt waarden, formules en getalnotaties mee, maar niet de overige opmaak. Plakt u ze in JeffDeck of JeffPub, dan komen ze als tabel die de lettertypen, arceringen, randen en de getallen zoals ze worden weergegeven, behoudt (zie **Cellen als tabel plakken** in de Help daarvan).

## Wat niet kan worden geplakt

JeffCrunch laat het u weten en wijzigt niets wanneer het plakken zou reiken tot in een deel van een **matrixformule**, over de rij met kolomnamen van een **tabel**, in vergrendelde cellen van een **beveiligd blad**, voorbij het einde van het blad, of in meer dan 100.000 cellen. Een hele **matrixformule** kan worden geknipt en verplaatst (selecteer al zijn cellen); een deel ervan, een gegevenstabel, of de rij met namen van een tabel kan dat niet.

## Het deelvenster Klembord

Het deelvenster Klembord onthoudt de laatste 24 dingen die u in het programma van dit venster hebt gekopieerd of geknipt, zodat u een oudere kunt plakken zonder ze opnieuw te kopiëren. Klik op het kleine pijltje in de hoek van de groep **Klembord** op het tabblad **Start** (of druk op **Alt**, dan **H**, dan **FO**) om het te openen. Er verschijnt een paneel naast de pagina.

Elk item toont een klein voorbeeld en wat voor soort ding het is: tekst, opgemaakte tekst, cellen, een afbeelding, vormen of dia's. Het nieuwste staat bovenaan. Klik op een item om het te plakken waar de cursor of de selectie staat. Het wordt geplakt zoals elk ander plakken: één stap van **Ongedaan maken** haalt het terug, en de knop Plakopties verschijnt zoals gewoonlijk.

- **Alles plakken** plakt elk item, het oudste eerst, zodat ze in de volgorde staan waarin u ze hebt gekopieerd.
- **Verwijderen** haalt het gekozen item weg. **Alles wissen** haalt ze allemaal weg.
- Het deelvenster verzamelt alleen zolang het open is. Om na het sluiten door te blijven verzamelen, vinkt u **Blijven verzamelen wanneer dit deelvenster is gesloten** aan. De keuze wordt onthouden, en het verzamelen begint wanneer het programma start, vóór u het deelvenster opent.
- Om ook te verzamelen wat u in andere programma's kopieert, vinkt u **Ook verzamelen wat in andere programma's wordt gekopieerd** aan. Alleen platte tekst en afbeeldingen worden bewaard, en bij elke start van het programma staat dit weer uit. Met deze optie aan wordt een kopie uit een ander programma volledig gelezen voordat dit programma beslist of het wordt bewaard, zodat een zeer grote kopie het programma even kan laten pauzeren en evenveel geheugen kan gebruiken als de kopie groot is. Er wordt maar één vorm gelezen: de tekst als die er is, anders één afbeelding. Staat deze optie uit, dan wordt niets gelezen wat in andere programma's is gekopieerd.

De items worden alleen in het geheugen van het programma bewaard. Niets wordt in een bestand opgeslagen, en de lijst wordt leeggemaakt wanneer u het laatste venster van het programma sluit. Een item blijft in de lijst nadat het document waaruit het komt, is gesloten. Een item groter dan 8 MB wordt niet bewaard, en het deelvenster laat u weten wanneer dat gebeurt; een afbeelding telt 4 bytes per stip (pixel), dus een afbeelding van meer dan ongeveer 2 miljoen pixels is te groot. Alle items samen worden beperkt tot 64 MB, en de oudste gaan eerst weg. Alles wat een wachtwoordbeheerder als geheim markeert, wordt nooit bewaard.
