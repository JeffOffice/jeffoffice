# Knippen, kopiëren en plakken

<!-- keywords: knippen, kopiëren, plakken, klembord, verplaatsen, dupliceren, speciaal, waarden, formules, opmaak, transponeren, lege cellen overslaan, optellen, aftrekken, vermenigvuldigen, delen, bewerking, samengevoegd, tekst, tabblad, html, webpagina, ander programma, Escape, lopende stippellijn, rand, kolombreedten, notities, opmerkingen, getalnotatie, alles behalve randen, afbeelding, bitmap, kopiëren als afbeelding -->

**Kopiëren** (**{key:Copy}**) zet een kopie van de geselecteerde cellen op het klembord, een plek waar de computer één ding tegelijk bewaart. **Plakken** (**{key:Paste}**) zet wat op het klembord staat in de cellen, beginnend bij de geselecteerde cel. **Knippen** (**{key:Cut}**) werkt als kopiëren, maar bij het plakken worden de cellen verplaatst en wordt de oude plek leeg.

Een bewegende stippellijn toont de cellen die zijn gekopieerd of geknipt. Druk op **Esc** om de stippellijn te verwijderen; de cellen zijn dan niet meer klaar om te worden geplakt. Geknipte cellen worden ook losgelaten als u voor het plakken een andere wijziging maakt (typen, invoegen, ongedaan maken). Knip de cellen dan opnieuw als u ze nog wilt verplaatsen.

## Plakken binnen JeffCrunch

Alles wordt meegenomen: de waarden, de formules, de opmaak en de samengevoegde cellen. Een formule blijft verwijzen naar dezelfde plaatsen *ten opzichte van waar de formule staat*. Voorbeeld: kopieer **=A1*2** uit B1 en plak deze in B5, dan wordt het **=A5*2**. Een deel met een dollarteken, zoals **$A$1**, blijft ongewijzigd. Als u een blok selecteert dat een geheel aantal keer zo groot is als wat u hebt gekopieerd, wordt de kopie herhaald om het blok te vullen. Kopieert u één cel en selecteert u twintig cellen, dan krijgen alle twintig de inhoud na het plakken.

Een geplakt getal dat te breed is voor een kolom die nog de oorspronkelijke breedte heeft, maakt de kolom breder.

## Knippen verplaatst cellen

Wanneer u cellen knipt en plakt, worden de cellen echt verplaatst. Elke formule die naar deze cellen verwees, verwijst na het verplaatsen naar de nieuwe plaats. Voorbeeld: B1 bevat **=A1+A2**. Knip A1:A2 en plak op E4, dan wordt B1 **=E4+E5**. Wat er stond in de cellen waarover u plakt, is verdwenen (met **Ongedaan maken** haalt u het terug), en een formule die een van die cellen gebruikte, toont **#REF!**, omdat de cel die werd gebruikt niet meer bestaat. Als de geknipte cellen maar een deel van een samengevoegde cel zijn, of u plakt maar over een deel van een samengevoegde cel, dan wordt er niets verplaatst: selecteer de hele samengevoegde cel. Wanneer u cellen naar een ander blad knipt, blijven de formules de cellen gebruiken die ze eerder gebruikten (er staat dan bij naar welk blad ze verwijzen). Het klembord wordt geleegd na een knippen en plakken.

## Plakken speciaal

**Ctrl+Alt+V**, of de pijl onder **Plakken** op het tabblad Start, laat u kiezen wat u wilt plakken:

- **Alles**, **Formules**, **Waarden** of **Opmaak**. *Waarden* verandert een formule in het getal of de tekst die hij toont.
- **Alles behalve randen**: alles, maar de cellen behouden de randen die ze hadden.
- **Formules en getalnotatie** of **Waarden en getalnotatie**: de formules (of de waarden) komen mee met de manier waarop de getallen worden weergegeven, zoals procent of datum, maar niet met de kleuren, lettertypen of randen. De cellen behouden de uitstraling die ze hadden.
- **Kolombreedten**: de kolommen waarin u plakt, krijgen de breedten van de gekopieerde kolommen.
- **Opmerkingen en notities**: de notities van de gekopieerde cellen worden op de cellen gezet waarin u plakt. Geen celwaarde verandert.
- Een **bewerking**: **Optellen**, **Aftrekken**, **Vermenigvuldigen** of **Delen**. De gekopieerde getallen worden gecombineerd met de getallen die al in de cellen staan. Voorbeeld: de cellen bevatten 10 en 20, u hebt 5 gekopieerd en u kiest *Vermenigvuldigen*: ze worden 50 en 100.
- **Lege cellen overslaan**: lege cellen in de kopie laten de cellen waarop ze terechtkomen zoals ze waren.
- **Transponeren**: rijen worden kolommen en kolommen worden rijen.

## Het menu Plakken en afbeeldingen

De pijl onder **Plakken** toont de gebruikelijke keuzes, en ook **Geen randen**, **Formules en getalnotatie**, **Waarden en getalnotatie**, **Bronkolombreedten behouden** en **Opmerkingen en notities**. Dat zijn dezelfde keuzes als bij Plakken speciaal, elk met één klik.

De pijl naast **Kopiëren** bevat **Kopiëren als afbeelding** en **Kopiëren als bitmap**. Elk zet een afbeelding van de geselecteerde cellen op het klembord, getekend zoals u ze ziet, met de rasterlijnen. Een bitmap heeft de grootte van het scherm. Een afbeelding heeft twee keer zoveel punten, dus die blijft scherp als u hem vergroot. Plak deze in JeffWrite of JeffDeck als afbeelding. Passen de cellen niet in één afbeelding, selecteer dan minder cellen.

## Tussen programma's

Wanneer u kopieert, zet JeffCrunch de cellen ook als tekst op het klembord (cellen gescheiden door tabs, één rij per regel) en als tabel voor webpagina's, zodat andere programma's ze kunnen gebruiken. Wanneer u iets plakt dat niet uit JeffCrunch komt, zoals tekst met tabs, of een tabel van een webpagina of een ander spreadsheet, dan komt elk onderdeel in een eigen cel en wordt het gelezen alsof u het hebt getypt: getallen, datums en formules die beginnen met een gelijkteken. Kopiëren uit een andere werkmap die open is in JeffCrunch neemt waarden, formules en getalnotaties mee, maar niet de overige opmaak.

## Wat niet kan worden geplakt

JeffCrunch laat u dat weten en verandert niets als plakken zou reiken in een deel van een **matrixformule**, over de rij met kolomnamen van een **tabel**, in vergrendelde cellen van een **beveiligd blad**, voorbij het einde van het blad, of in meer dan 100.000 cellen. Een volledige **matrixformule** kan worden geknipt en verplaatst (selecteer al zijn cellen); een deel ervan, een gegevenstabel of de rij met namen van een tabel kan dat niet.
