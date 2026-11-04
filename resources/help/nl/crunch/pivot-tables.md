# Draaitabellen

<!-- keywords: draaitabel, draaitabellen, uitklappen, inklappen, draaitabel invoegen, maken, selecteren, draaitabel verplaatsen, GETPIVOTDATA, GEEF.DRAAITABEL.GEGEVENS, vernieuwen, alles vernieuwen, gegevensbron wijzigen, veldenlijst, velden, rijen, kolommen, waarden, filters, rapportfilter, totaal, subtotaal, indeling, compact, overzicht, tabelvorm, wissen, waardeveldinstellingen, som, aantal, gemiddelde, waarden weergeven als, percentage van totaal, verschil, lopend totaal, getalnotatie, veldinstellingen, sorteren, filteren, bovenste, groeperen, datums, maanden, jaren, uitvouwen, samenvouwen, opties, stijl, gestreepte rijen, lege rijen, labels herhalen, vernieuwen bij openen van bestand, draaigrafiek, segmentering, tijdlijn, details weergeven, uitklappen, rijen achter een waarde -->

Een draaitabel is een klein rapport dat is gemaakt van een lange lijst. De lijst heeft bijvoorbeeld één rij voor elke verkoop; het rapport telt de verkopen op per regio en per maand. U schrijft geen formules. U geeft aan welke kolommen van de lijst aan de zijkant komen, welke bovenaan, en welke wordt opgeteld. Het rapport vult zichzelf in.

## Een voorbeeld

Hier is een lijst in de cellen A1 tot en met C7. Elke rij is één verkoop.

| Regio | Maand | Bedrag |
|---|---|---|
| Oost | Jan | 10 |
| Oost | Jan | 5 |
| Oost | Feb | 20 |
| West | Jan | 7 |
| West | Feb | 3 |
| West | Feb | 4 |

Een draaitabel met **Regio** aan de zijkant, **Maand** bovenaan en de som van **Bedrag** in het midden ziet er zo uit.

| Som van Bedrag | Maand | | |
|---|---|---|---|
| Regio | Jan | Feb | Eindtotaal |
| Oost | 15 | 20 | 35 |
| West | 7 | 7 | 14 |
| Eindtotaal | 22 | 27 | 49 |

Oost in januari is 15, omdat de twee rijen van Oost voor januari, 10 en 5, worden opgeteld. De kolom Eindtotaal telt elke rij op, de rij Eindtotaal telt elke kolom op, en de cel in de hoek telt alles op.

De gebruikte woorden:

- **Veld** is een kolom van de lijst. Regio, Maand en Bedrag zijn velden.
- **Rijen**, **Kolommen** en **Waarden** zijn de drie plaatsen waar een veld kan staan. Een veld in Rijen geeft één regel voor elk van zijn verschillende items; een veld in Kolommen geeft één kolom voor elk item; een veld in Waarden is het veld dat wordt opgeteld. Een vierde plaats, **Filters**, bevat een veld dat de rijen weglaat die u niet kiest.
- Een **cache** (de draaitabelcache) is een kopie van uw lijst die de draaitabel in het bestand bewaart, zodat ze totalen kan tonen zonder naar de cellen te kijken. Het rapport wordt berekend uit de cache, niet uit de cellen, en verandert daarom niet wanneer de cellen veranderen. **Vernieuwen** kopieert de cellen opnieuw naar de cache.

## Een draaitabel maken

1. Klik op een cel in de lijst, of selecteer de cellen die u wilt gebruiken. De lijst heeft een rij met namen boven de gegevens nodig.
2. Kies **Invoegen** en daarna **Draaitabel**.
3. Het vak toont de cellen waarin de gegevens staan. Wijzig ze als ze niet kloppen. U kunt ook de naam van een tabel typen.
4. Kies waar de draaitabel komt: **Nieuw blad** (de standaard), of een cel op dit blad of een ander blad. Kies een plaats waar niets anders staat, want de lege tabel beslaat ongeveer achttien rijen en drie kolommen.
5. Kies **OK**. De tabel begint leeg, en het deelvenster **Draaitabelvelden** wordt geopend. Vink de velden aan die u wilt: een veld met getallen wordt opgeteld, en elk ander veld wordt een rij. Verplaats daarna de velden naar de vakken zoals in de volgende paragrafen staat.

Een nieuw blad is een eigen stap van Ongedaan maken, en de tabel ook. Wanneer u opslaat als .xlsx, wordt de tabel geschreven zoals het oorspronkelijke programma dat doet, zodat ze daar als draaitabel wordt geopend. Een kopie die u opslaat als .xls of .ods behoudt de getallen die de tabel toont in de cellen, maar niet de tabel zelf, en het opslaan laat u dat weten.

## GEEF.DRAAITABEL.GEGEVENS: één getal uit een draaitabel lezen

`GEEF.DRAAITABEL.GEGEVENS` geeft het getal dat een draaitabel toont voor de waarden en de rijen en kolommen die u opgeeft. Voor het voorbeeld hierboven:

`=GEEF.DRAAITABEL.GEGEVENS("Som van Bedrag"; $E$3; "Regio"; "Oost"; "Maand"; "Jan")` geeft 15.

- Het eerste argument is de naam van de waarde, zoals de tabel die toont (bijvoorbeeld Som van Bedrag). De naam van het veld dat wordt opgeteld (Bedrag) werkt ook.
- Het tweede argument is een willekeurige cel van de draaitabel.
- Daarna volgen paren van een veld en een van zijn items. Geeft u alleen Regio en Oost op, dan krijgt u het totaal van Oost, 35. Geeft u niets op, dan krijgt u het eindtotaal, 49. De velden die u opgeeft moeten de eerste velden in de rijen zijn, of de eerste velden in de kolommen, in volgorde.
- Als de tabel het getal niet toont, toont de cel `#VERW!`.

Wanneer u tijdens het typen van een formule op een waarde van een draaitabel klikt, schrijft JeffCrunch deze formule voor u, zodat die het juiste getal blijft lezen nadat de tabel opnieuw is ingedeeld. Om dit uit te schakelen, kiest u **Opties** op het tabblad **Draaitabel** en haalt u het vinkje weg bij GEEF.DRAAITABEL.GEGEVENS.

## Wat JeffCrunch met een draaitabel doet

Een draaitabel in een werkmap wordt getoond zoals hij is opgeslagen. Als u er niets aan verandert, wordt het bestand opgeslagen met de draaitabel precies zoals hij was. Wanneer u de tabel vernieuwt of wijzigt, berekent JeffCrunch het rapport opnieuw, zet de nieuwe getallen in de cellen en schrijft de draaitabel terug, zodat het programma dat hem heeft gemaakt hem zonder problemen opent.

JeffCrunch berekent draaitabellen die optellen, tellen, gemiddelden nemen en dergelijke, die datums en getallen groeperen, en die waarden tonen als percentage, verschil of lopend totaal. Een draaitabel met een berekend veld of berekend item, die een waarde toont als rangorde of als percentage van een bovenliggend item, groepen items heeft die iemand met de hand heeft gemaakt, of een gegevensmodel gebruikt, wordt getoond en bewaard zoals hij is opgeslagen, en de opdrachten laten u weten dat ze hem niet kunnen wijzigen.

Klik op een willekeurige cel van een draaitabel. Het tabblad **Draaitabel** verschijnt.

## Vernieuwen

**Vernieuwen** leest de cellen waaruit de draaitabel is gemaakt en berekent het rapport opnieuw. Nieuwe items komen na de items die er al waren; items die zijn verdwenen, vervallen. Kolombreedten en de indeling die u hebt gekozen, blijven behouden. **Alles vernieuwen**, op het tabblad **Gegevens** en op het tabblad **Draaitabel**, doet hetzelfde voor elke draaitabel in de werkmap.

Als het rapport zou uitgroeien over cellen die iets bevatten, vernieuwt JeffCrunch niet. Het laat u weten welke cellen in de weg staan. Maak die leeg of verplaats de draaitabel en vernieuw opnieuw.

**Ongedaan maken** neemt een vernieuwing in één stap terug.

## Gegevensbron wijzigen

**Gegevensbron wijzigen** vraagt om de cellen waarin de lijst staat, inclusief de eerste rij met namen. Typ ze zoals `Blad1!A1:D100`, of typ de naam van een tabel of een naam die u zelf hebt gemaakt. JeffCrunch leest ze en berekent het rapport opnieuw. Als een veld van de lijst in de nieuwe cellen dezelfde naam heeft, blijft de plaats ervan in het rapport behouden.

## Wissen

**Wissen** heeft twee keuzes. **Filters wissen** verwijdert elk filter en toont alle items weer. **Alles wissen** haalt elk veld uit het rapport, zodat het leeg is en opnieuw kan worden opgebouwd.

## Uitklappen en inklappen

Wanneer een rapport meer dan één veld in de rijen heeft, kan een item van het eerste veld worden uitgeklapt of ingeklapt. Klik op de cel met de naam van het item en kies **Inklappen** om het in te klappen: de regels van het volgende veld eronder worden verborgen en alleen het totaal blijft staan. **Uitklappen** klapt het weer open. Een item van het laatste veld heeft niets eronder. Elke stap is één keer **Ongedaan maken**.

## Selecteren en verplaatsen

**Selecteren** op het tabblad **Draaitabel** selecteert een deel van het rapport: **Hele draaitabel**, **Waarden** (de getallen), of **Labels** (de namen aan de zijkant; als het rapport er aan de zijkant geen heeft, de namen bovenaan). **Draaitabel verplaatsen…** vraagt om een cel op hetzelfde blad en tekent het rapport opnieuw met de eerste cel daar. De cellen die worden verlaten, worden leeggemaakt. Formules die naar de cellen van het rapport verwijzen, volgen ze, zoals bij knippen en plakken. Als de nieuwe plaats iets bevat, verplaatst JeffCrunch het rapport niet en laat u dat weten. Eén keer **Ongedaan maken** zet het terug.

## Het deelvenster met velden

**Veldenlijst** op het tabblad **Draaitabel** opent het deelvenster **Draaitabelvelden**. Het toont de velden van de lijst, met een vakje ernaast, en daaronder de vier plaatsen: **Filters**, **Kolommen**, **Rijen** en **Waarden**.

- Vink een veld aan om het toe te voegen. Een veld dat alleen getallen bevat, gaat naar **Waarden** en wordt opgeteld; elk ander veld gaat naar **Rijen**.
- Haal het vinkje weg bij een veld om het overal te verwijderen.
- Klik op een veld in een van de vier plaatsen en kies **Verplaatsen naar** om het in een andere plaats te zetten. Met **Omhoog verplaatsen** en **Omlaag verplaatsen** verandert u de volgorde binnen de plaats, en **Veld verwijderen** haalt het weg.
- Met meer dan één waarde verschijnt een regel met de naam **Waarden** in **Kolommen**. Verplaats die naar **Rijen** om de waarden aan de zijkant te zetten.
- **Waardeveldinstellingen** (dubbelklik op een waarde, of gebruik de knop) stelt de naam in die het rapport voor de waarde toont en hoe ze wordt opgeteld: **Som**, **Aantal**, **Aantal getallen**, **Gemiddelde**, **Max**, **Min**, **Product**, **StDev** en **StDevp** (de standaarddeviatie van een steekproef, of van alle waarden), en **Var** en **Varp** (de variantie van hetzelfde). Ook stelt het in hoe de waarde wordt weergegeven en welke getalnotatie ze krijgt. Zie de volgende paragraaf.
- **Veldinstellingen** (dubbelklik op een veld in Filters, Kolommen of Rijen, of gebruik de knop) stelt één veld in. Zie "Veldinstellingen".

Elke wijziging is één stap van **Ongedaan maken**.

## Waarden weergeven als, en de getalnotatie van een waarde

In **Waardeveldinstellingen** verandert **Waarden weergeven als** wat een cel van het rapport toont.

- **% van eindtotaal**: de cel gedeeld door het totaal van het hele rapport. In het voorbeeld is Oost in januari 15 van 49, dat is 30,6%.
- **% van kolomtotaal** en **% van rijtotaal**: de cel gedeeld door het totaal van zijn kolom, of van zijn rij.
- **Index**: hoe de cel zich verhoudt tot wat zijn rij en kolom zouden geven als de gegevens gelijkmatig verdeeld waren. Een getal boven 1 betekent meer dan het gelijke aandeel.
- **Verschil van**, **% van** en **% verschil van**: de cel vergeleken met een ander item van een veld. Kies het **Basisveld** (een veld in Rijen of Kolommen) en het **Basisitem** (een van zijn items, of het vorige of volgende). Met Maand als basisveld en Jan als basisitem laat februari zien hoeveel meer of minder het is dan januari. Het basisitem zelf blijft leeg bij een verschil, en is 100% bij **% van**.
- **Lopend totaal in**: elke cel telt de cellen ervoor op langs het basisveld.

**Getalnotatie** neemt een notatiecode, zoals `0,00`, `#.##0` of `0,0%`. Laat het leeg om de notatie van de cellen te behouden. Een waarde die als percentage wordt getoond, krijgt `0,00%`, tenzij u een notatie kiest. De cellen van het rapport krijgen de notatie meteen. **Ongedaan maken** neemt die terug, samen met de getallen.

## Veldinstellingen

Selecteer een veld in het vak **Filters**, **Kolommen** of **Rijen** en kies **Veldinstellingen** op het tabblad **Draaitabel** (of dubbelklik erop).

- **Aangepaste naam** wijzigt de naam die het rapport voor het veld toont.
- **Subtotalen**: **Automatisch** toont een totaal voor elke groep van het veld, **Geen** doet dat niet.
- **Itemlabels weergeven in**: compacte, overzichts- of tabelvorm, zoals bij Rapportindeling.
- **Na elk item een lege regel invoegen** zet een lege regel na elke groep.
- **Itemlabels herhalen** toont de naam van een buitenste item op elke regel van zijn groep (overzichts- en tabelvorm).
- **Sorteren**: de volgorde van de items behouden (handmatig), of van A naar Z of van Z naar A sorteren op de labels. **Van kleinste naar grootste op basis van een waarde** en **Van grootste naar kleinste op basis van een waarde** sorteren de items op de getallen van een waardeveld. Het rapport behoudt de sortering wanneer de gegevens veranderen.
- **Filter** laat items weg op hun labels (begint met, eindigt op, bevat, is gelijk aan) of op een waarde (groter dan, kleiner dan, tussen, enzovoort), of houdt alleen de bovenste of onderste paar items over op basis van een waarde. Het filter wordt berekend wanneer u op OK kiest. Het verbergt de items die het weglaat; de vakjes hieronder laten zien welke dat zijn, en u kunt een item weer tonen door het aan te vinken.
- **Deze items weergeven** is een lijst van de items van het veld, met elk een vakje. Haal het vinkje weg om het item te verbergen; de totalen laten het dan weg.

## Datums en getallen groeperen

Selecteer een veld met datums of getallen in **Rijen** of **Kolommen** en kies **Veld groeperen** op het tabblad **Draaitabel**. Groepen worden uit de gegevens gemaakt, dus de gegevens in die kolom moeten alleen getallen of datums bevatten, zonder tekst en zonder lege cellen.

- **Datums**: vink aan waarop u wilt groeperen: **Seconden**, **Minuten**, **Uren**, **Dagen**, **Maanden**, **Kwartalen** of **Jaren**. Het fijnste deel dat u aanvinkt, blijft bij het veld. Voor elk grover deel voegt JeffCrunch een veld toe, genoemd naar dat deel (bijvoorbeeld **Jaren**), en zet het vóór het datumveld op dezelfde plaats. **Beginnen bij** en **Eindigen bij** zijn de eerste en laatste datum; laat ze staan om de kleinste en grootste datum uit de gegevens te gebruiken. Datums vóór het begin of na het einde komen in twee extra groepen, die beginnen met `<` en `>`.
- **Getallen**: **Beginnen bij**, **Eindigen bij** en **Per** maken groepen zoals 1-10, 11-20, enzovoort.

**Groepering opheffen** haalt de groepen weg en de velden die ervoor zijn toegevoegd. Vernieuwen maakt de groepen opnieuw uit de nieuwe gegevens. Wanneer twee draaitabellen zijn gemaakt uit dezelfde gegevens, kunnen er geen groepen worden gemaakt, omdat een groep een deel van de gegevens is die ze delen.

## De rijen achter een waarde tonen

Dubbelklik op een waarde van een draaitabel, of kies **Details weergeven** op het tabblad **Draaitabel**. JeffCrunch maakt een nieuw blad, genaamd Detail 1, 2, enzovoort, met de rijen van de lijst waaruit het getal bestaat. De namen van de velden staan in de eerste rij. F2 bewerkt de cel nog steeds, als de cel kan worden bewerkt. **Ongedaan maken** verwijdert de rijen, en nog een **Ongedaan maken** verwijdert het blad.

## Uitvouwen en samenvouwen

Een veld waarvan de items andere items eronder hebben, kan de regels onder zijn items tonen of verbergen. **Hele veld uitvouwen** en **Hele veld samenvouwen** doen dat voor elk item van het veld dat u in het deelvenster selecteert.

## Opties voor draaitabel

**Opties** op het tabblad **Draaitabel** opent een vak met de keuzes van de tabel.

- **Naam** is de naam van de draaitabel. Geen andere draaitabel in de werkmap mag die naam hebben.
- **Voor foutwaarden tonen** en **Voor lege cellen tonen** zetten een eigen tekst in die cellen.
- **Cellen met labels samenvoegen en centreren**, **Kolombreedten aanpassen wanneer de tabel wordt bijgewerkt** en **Opmaak van cellen behouden wanneer de tabel wordt bijgewerkt** worden samen met de tabel opgeslagen.
- **Totalen voor rijen weergeven** en **voor kolommen** zijn hetzelfde als de keuzes voor Eindtotalen.
- **Gegevens vernieuwen wanneer het bestand wordt geopend** leest de cellen elke keer opnieuw wanneer de werkmap wordt geopend. Het is één stap van Ongedaan maken, zoals elke vernieuwing.

## Stijl: koppen, banden en de galerie

Met de **Stijlopties** op het tabblad **Draaitabel** schakelt u de kopregel, de eerste kolom (rijkoppen) en de banden van rijen en kolommen in of uit. **Stijlen** heeft een galerie met uitstralingen, en **Stijl wissen** haalt de uitstraling weg. Deze uitstralingen worden op dezelfde manier getekend als de stijlen van tabellen. **Lege rijen** en **Labels herhalen** stellen de lege regel en de herhaalde labels in voor elk veld van de rijen tegelijk.

## Ontwerp: subtotalen, eindtotalen en indeling

- **Subtotalen** toont of verbergt het totaal van elke groep, bovenaan of onderaan de groep. Wanneer er meer dan één veld in Rijen staat, is het totaal van elk buitenste veld een subtotaal.
- **Eindtotalen** heeft vier keuzes: uit voor rijen en kolommen, aan voor beide, alleen aan voor rijen (een kolom met totalen rechts, één voor elke rij), en alleen aan voor kolommen (een rij met totalen onderaan).
- **Rapportindeling** is de vorm van de labels. In **compacte** vorm delen alle velden van Rijen één kolom, en wordt een nieuwe groep een stukje naar rechts getoond. In **overzichtsvorm** heeft elk veld zijn eigen kolom, en heeft elke groep een eigen regel boven zijn items. In **tabelvorm** heeft elk veld zijn eigen kolom en staat de naam van de groep op de eerste regel, met het subtotaal eronder.

## Dezelfde lijst, met een filter

Verplaats **Maand** van **Kolommen** naar **Filters**. Het rapport verliest zijn kolommen, en een regel erboven toont **Maand** en **(Alle)**. De getallen zijn nu het totaal van elke regio over alle maanden.

## Draaigrafiek

Klik op een cel in een draaitabel en kies **Invoegen** en daarna **Draaigrafiek**. JeffCrunch maakt een kolomgrafiek van de waarden van de tabel. Die laat de eindtotalen weg. De grafiek is in het bestand gemarkeerd als de grafiek van de draaitabel. Ze leest de cellen van de tabel, dus haar getallen veranderen wanneer u de tabel vernieuwt of de indeling ervan wijzigt. Als de tabel meer rijen of kolommen krijgt, maak de grafiek dan opnieuw, omdat de grafiek de cellen bewaart waaruit ze is gemaakt. Een draaigrafiek die uit een bestand komt, wordt getoond zoals hij is opgeslagen.

## Segmenteringen en tijdlijnen

Een segmentering is een reeks knoppen die een draaitabel filtert, en een tijdlijn is een schuifregelaar die hetzelfde doet voor datums.

Klik op een cel in een draaitabel en klik daarna op **Segmenteringen** op het tabblad Draaitabel. Het deelvenster Segmenteringen wordt geopend en toont de segmenteringen en tijdlijnen in de werkmap. Een segmentering heeft één knop per item van zijn veld. De knoppen die blauw zijn, zijn de items die de tabel toont.

Klik op een knop om alleen dat item te tonen. Klik er nogmaals op om alle items weer te tonen. Houd **Ctrl** ingedrukt en klik om een item toe te voegen aan de getoonde items, of om er een weg te halen. Elke klik is één stap van Ongedaan maken, en Ongedaan maken zet de tabel terug. Wanneer u opslaat, bewaart het bestand de keuze van de segmentering.

**Rapportverbindingen** laat zien welke draaitabellen een segmentering filtert. Een tijdlijn toont de periode die ze heeft gekozen. JeffCrunch toont de segmenteringen in dit deelvenster, niet over de cellen, en het wijzigt een tijdlijn nog niet. Beide blijven in het bestand bewaard zoals ze zijn opgeslagen.

## Grenzen

Een draaitabel kan maximaal 2.000 velden hebben, een lijst van maximaal vijf miljoen rijen, en een rapport van maximaal vier miljoen cellen. Een bestand dat hierboven uitkomt, wordt getoond zoals het is opgeslagen en kan niet worden gewijzigd. Het opnieuw inlezen van honderdduizend rijen met twintig kolommen en het berekenen van het rapport duurde ongeveer een halve seconde op de computer waarop dit is gemeten; een tragere computer heeft er langer over.
