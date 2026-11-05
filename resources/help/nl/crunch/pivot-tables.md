# Draaitabellen
<!-- keywords: lijst met formules, berekend veld, formulevelden, rapportfilterpagina's, draaitabel, omlaag doorklikken, omhoog doorklikken, draaitabel invoegen, maken, selecteren, draaitabel verplaatsen, GETPIVOTDATA, GETPIVOTDATA genereren, vernieuwen, alles vernieuwen, gegevensbron wijzigen, lijst met velden, velden, rijen, kolommen, waarden, filters, rapportfilter, totaal, subtotaal, indeling, compact, overzicht, tabelvorm, wissen, waardeveldinstellingen, som, aantal, gemiddelde, waarden weergeven als, percentage van totaal, verschil, lopend totaal, getalnotatie, veldinstellingen, sorteren, filteren, bovenste, groeperen, datums, maanden, jaren, uitvouwen, samenvouwen, opties, stijl, gestreepte rijen, lege rijen, labels herhalen, vernieuwen bij openen van bestand, draaigrafiek, segmentering, tijdlijn, details weergeven, omlaag doorklikken, omhoog doorklikken, rijen achter een waarde, draaitabel maken, pivot -->

Een draaitabel is een klein rapport dat is gemaakt van een lange lijst. De lijst heeft bijvoorbeeld één rij voor elke verkoop; het rapport telt de verkopen op per regio en maand. U schrijft geen formules. U geeft aan welke kolommen van de lijst onder elkaar komen te staan, welke er bovenaan komen, en welke wordt opgeteld, en het rapport vult zichzelf in.

## Een voorbeeld

Hier is een lijst in de cellen A1 tot en met C7. Elke rij is één verkoop.

| Regio | Maand | Bedrag |
|---|---|---|
| Oost | jan | 10 |
| Oost | jan | 5 |
| Oost | feb | 20 |
| West | jan | 7 |
| West | feb | 3 |
| West | feb | 4 |

Een draaitabel met **Regio** aan de zijkant, **Maand** bovenaan en de som van **Bedrag** in het midden ziet er zo uit.

| Som van Bedrag | Maand | | |
|---|---|---|---|
| Regio | jan | feb | Eindtotaal |
| Oost | 15 | 20 | 35 |
| West | 7 | 7 | 14 |
| Eindtotaal | 22 | 27 | 49 |

Oost in januari is 15, omdat de twee rijen van Oost voor januari, 10 en 5, worden opgeteld. De kolom Eindtotaal telt elke rij op, de rij Eindtotaal telt elke kolom op, en de cel in de hoek telt alles op.

De gebruikte woorden:

- **Veld** is een kolom van de lijst. Regio, Maand en Bedrag zijn velden.
- **Rijen**, **Kolommen** en **Waarden** zijn de drie plaatsen waar een veld kan staan. Een veld in Rijen geeft één regel voor elk van zijn verschillende items; een veld in Kolommen geeft één kolom voor elk item; een veld in Waarden is het veld dat wordt opgeteld. Een vierde plaats, **Filters**, bevat een veld dat de rijen weglaat die u niet kiest.
- Een **cache** (de draaitabelcache) is een kopie van uw lijst die de draaitabel binnen het bestand bewaart, zodat ze totalen kan tonen zonder naar de cellen te kijken. Het rapport wordt berekend uit de cache, niet uit de cellen, en verandert dus niet wanneer de cellen veranderen. **Vernieuwen** kopieert de cellen opnieuw naar de cache.

## Een draaitabel maken

1. Klik op een cel in de lijst, of selecteer de cellen die u wilt gebruiken. De lijst heeft een rij met namen boven de gegevens nodig.
2. Kies **Invoegen** en daarna **Draaitabel**.
3. Het vak toont de cellen waarin de gegevens staan. Wijzig ze als ze niet kloppen. U kunt ook de naam van een tabel typen.
4. Kies waar de draaitabel komt: **Nieuw blad** (de standaard) of een cel op dit blad of een ander blad. Kies een plek waar niets anders staat, omdat de lege tabel ongeveer achttien rijen en drie kolommen beslaat.
5. Kies **OK**. De tabel begint leeg en het deelvenster **Draaitabelvelden** wordt geopend. Vink de velden aan die u wilt: een veld met getallen wordt opgeteld, en elk ander veld wordt een rij. Verplaats daarna velden naar de vakken zoals in de volgende paragrafen wordt beschreven.

Een nieuw blad is een eigen stap van Ongedaan maken, en de tabel ook. Wanneer u opslaat als .xlsx, wordt de tabel geschreven zoals het oorspronkelijke programma er een schrijft, zodat hij daar als draaitabel opent. Een kopie die u opslaat als .xls of .ods behoudt de getallen die de tabel toont in zijn cellen, maar niet de tabel zelf, en het opslaan laat u dat weten.

## GETPIVOTDATA: één getal uit een draaitabel lezen

`GETPIVOTDATA` geeft het getal dat een draaitabel toont voor de waarden en de rijen en kolommen die u noemt. Voor het voorbeeld hierboven:

`=GETPIVOTDATA("Som van Bedrag"; $E$3; "Regio"; "Oost"; "Maand"; "jan")` geeft 15.

- Het eerste argument is de naam van de waarde, zoals de tabel die toont (bijvoorbeeld Som van Bedrag). De naam van het veld dat wordt opgeteld (Bedrag) werkt ook.
- Het tweede argument is elke cel van de draaitabel.
- Daarna volgen paren van een veld en een van zijn items. Noemt u alleen Regio en Oost, dan krijgt u het totaal van Oost, 35. Noemt u niets, dan krijgt u het eindtotaal, 49. De velden die u noemt moeten de eerste velden in de rijen zijn, of de eerste velden in de kolommen, in die volgorde.
- Als de tabel het getal niet toont, toont de cel `#REF!`.

Wanneer u tijdens het typen van een formule op een waarde van een draaitabel klikt, schrijft JeffCrunch deze formule voor u, zodat die nog steeds het juiste getal leest nadat de tabel anders is gerangschikt. Om dit uit te schakelen, kiest u **Opties** op het tabblad **Draaitabel** en schakelt u het selectievakje over GETPIVOTDATA uit.

## Wat JeffCrunch met een draaitabel doet

Een draaitabel in een werkmap wordt getoond zoals hij is opgeslagen. Als u er niets aan verandert, wordt het bestand opgeslagen met de draaitabel precies zoals hij was. Wanneer u hem vernieuwt of wijzigt, berekent JeffCrunch het rapport opnieuw, zet de nieuwe getallen in de cellen en schrijft de draaitabel terug, zodat het programma dat hem heeft gemaakt hem zonder problemen opent.

JeffCrunch berekent draaitabellen die optellen, tellen, gemiddelden berekenen enzovoort, die datums en getallen groeperen, en die waarden tonen als percentage, verschil of lopend totaal. Een draaitabel met een berekend veld of item, die een waarde toont als rangschikking of als aandeel van een bovenliggend item, met groepen items die iemand met de hand heeft gemaakt, of die een gegevensmodel gebruikt, wordt getoond en bewaard zoals hij is opgeslagen, en de opdrachten laten u weten dat ze hem niet kunnen wijzigen.

Klik op een willekeurige cel van een draaitabel. Het tabblad **Draaitabel** verschijnt.

## Vernieuwen

**Vernieuwen** leest de cellen waaruit de draaitabel is gemaakt en berekent het rapport opnieuw. Nieuwe items komen na de items die er al zijn; items die zijn verdwenen, vallen weg. Kolombreedten en de indeling die u hebt gekozen, blijven behouden. **Alles vernieuwen**, op het tabblad **Gegevens** en op het tabblad **Draaitabel**, doet hetzelfde voor elke draaitabel in de werkmap.

Als het rapport zou groeien over cellen die iets bevatten, vernieuwt JeffCrunch niet. Het laat u weten welke cellen in de weg staan. Maak ze leeg of verplaats de draaitabel, en vernieuw opnieuw.

**Ongedaan maken** neemt een vernieuwing in één stap terug.

## Gegevensbron wijzigen

**Gegevensbron wijzigen** vraagt om de cellen waarin de lijst staat, inclusief de eerste rij met namen. Typ ze zoals `Blad1!A1:D100`, of typ de naam van een tabel of een naam die u zelf hebt gemaakt. JeffCrunch leest ze en berekent het rapport opnieuw. Als een veld van de lijst in de nieuwe cellen dezelfde naam heeft, blijft zijn plaats in het rapport behouden.

## Wissen

**Wissen** heeft twee keuzes. **Filters wissen** verwijdert elk filter en toont alle items weer. **Alles wissen** haalt elk veld uit het rapport en laat het leeg, klaar om opnieuw op te bouwen.

## Omlaag en omhoog doorklikken

## Rapportfilterpagina's

Een filter staat boven een draaitabel en laat u één item per keer bekijken, bijvoorbeeld één maand. **Rapportfilterpagina's weergeven** op het tabblad **Draaitabel** maakt in één keer een pagina voor elk item. Elke pagina is een nieuw blad, genoemd naar het item, met een eigen kopie van de tabel. Het filter van elke kopie is ingesteld op dat item. Heeft de tabel meer dan één filter, dan kiest u welk filter wordt gebruikt. Eén **Ongedaan maken** verwijdert alle pagina's.

Wanneer een rapport meer dan één veld in de rijen heeft, kan een item van het eerste veld worden uitgevouwen of samengevouwen. Klik op de cel met de naam van het item en kies **Omhoog doorklikken** om het samen te vouwen: de regels van het volgende veld eronder worden verborgen en alleen het totaal blijft staan. **Omlaag doorklikken** vouwt het weer open. Een item van het laatste veld heeft niets eronder. Elke actie is één **Ongedaan maken**.

## Selecteren en verplaatsen

**Selecteren** op het tabblad **Draaitabel** selecteert een deel van het rapport: **Hele draaitabel**, **Waarden** (de getallen), of **Labels** (de namen aan de zijkant; heeft het rapport geen namen aan de zijkant, dan de namen bovenaan). **Draaitabel verplaatsen…** vraagt om een cel op hetzelfde blad en tekent het rapport opnieuw met zijn eerste cel daar. De cellen die het achterlaat, worden leeggemaakt. Formules die naar cellen van het rapport verwijzen, volgen ze, net zoals bij knippen en plakken. Staat er iets op de nieuwe plek, dan verplaatst JeffCrunch het rapport niet en laat dat weten. Eén **Ongedaan maken** zet het terug.

## Het deelvenster met velden

**Lijst met velden** op het tabblad **Draaitabel** opent het deelvenster **Draaitabelvelden**. Het toont de velden van de lijst met een vakje ernaast, en daaronder de vier plaatsen: **Filters**, **Kolommen**, **Rijen** en **Waarden**.

- Vink een veld aan om het toe te voegen. Een veld dat alleen getallen bevat, gaat naar **Waarden** en wordt opgeteld; elk ander veld gaat naar **Rijen**.
- Haal het vinkje weg om een veld overal te verwijderen.
- Klik op een veld in een van de vier plaatsen en kies **Verplaatsen naar** om het naar een andere plaats te brengen. De woorden **Omhoog** en **Omlaag** wijzigen de volgorde op zijn plaats, en **Veld verwijderen** haalt het weg.
- Bij meer dan één waarde verschijnt er een regel met de naam **Waarden** in **Kolommen**. Verplaats hem naar **Rijen** om de waarden aan de zijkant te zetten.
- **Waardeveldinstellingen** (dubbelklik op een waarde, of gebruik de knop) stelt de naam in die het rapport voor de waarde toont en hoe ze wordt opgeteld: **Som**, **Aantal**, **Aantal getallen**, **Gemiddelde**, **Max**, **Min**, **Product**, **StDev** en **StDevp** (de standaarddeviatie van een steekproef, of van alle waarden), en **Var** en **Varp** (de variantie van hetzelfde). Ook stelt het in hoe de waarde wordt getoond en de getalnotatie. Zie de volgende paragraaf.
- **Veldinstellingen** (dubbelklik op een veld in Filters, Kolommen of Rijen, of gebruik de knop) stelt één veld in. Zie "Veldinstellingen".

Elke wijziging is één stap van **Ongedaan maken**.

## Waarden weergeven als, en de getalnotatie van een waarde

In **Waardeveldinstellingen** verandert **Waarden weergeven als** wat een cel van het rapport toont.

- **% van eindtotaal**: de cel gedeeld door het totaal van het hele rapport. In het voorbeeld is Oost in januari 15 van 49, dat is 30,6%.
- **% van kolomtotaal** en **% van rijtotaal**: de cel gedeeld door het totaal van zijn kolom, of van zijn rij.
- **Index**: hoe de cel zich verhoudt tot wat zijn rij en kolom zouden geven als de gegevens gelijkmatig verdeeld waren. Een getal boven 1 betekent meer dan het gelijke deel.
- **Verschil met**, **% van** en **% verschil met**: de cel vergeleken met een ander item van een veld. Kies het **Basisveld** (een veld in Rijen of Kolommen) en het **Basisitem** (een van zijn items, of het vorige of volgende). Met Maand als basisveld en jan als basisitem laat februari zien hoeveel meer of minder het is dan januari. Het basisitem zelf blijft leeg bij een verschil en is 100% bij **% van**.
- **Lopend totaal in**: elke cel telt de cellen ervoor op langs het basisveld.
- **Rangschikken van klein naar groot** en **Rangschikken van groot naar klein**: elke cel toont zijn plaats, 1, 2, 3 enzovoort, onder de cellen van het basisveld in dezelfde rij of kolom. Rang 1 is het kleinste getal, of het grootste.
- **% van bovenliggend rijtotaal** en **% van bovenliggend kolomtotaal**: elke cel toont welk deel van zijn bovenliggend item hij is. Het bovenliggend item van een item is het item één niveau hoger, zoals het jaar dat een maand bevat. Een item zonder bovenliggend item wordt getoond als deel van het eindtotaal.

**Getalnotatie** neemt een notatiecode, zoals `0,00`, `#.##0` of `0,0%`. Laat het leeg om de notatie te behouden die de cellen hebben. Een waarde die als percentage wordt getoond, krijgt `0,00%`, tenzij u een notatie kiest. De cellen van het rapport nemen de notatie meteen over. **Ongedaan maken** neemt die terug, samen met de getallen.

## Veldinstellingen

Selecteer een veld in het vak **Filters**, **Kolommen** of **Rijen** en kies **Veldinstellingen** op het tabblad **Draaitabel** (of dubbelklik erop).

- **Aangepaste naam** wijzigt de naam die het rapport voor het veld toont.
- **Subtotalen**: **Automatisch** toont een totaal voor elke groep van het veld, **Geen** toont er geen.
- **Itemlabels weergeven in**: compacte, overzichts- of tabelvorm, zoals bij Rapportindeling.
- **Een lege regel invoegen na elk item** plaatst na elke groep een lege regel.
- **Itemlabels herhalen** toont de naam van een buitenste item op elke regel van zijn groep (overzichts- en tabelvorm).
- **Sorteren**: de volgorde van de items behouden (handmatig), of sorteren van A naar Z of van Z naar A op de labels. **Van klein naar groot op waarde** en **Van groot naar klein op waarde** sorteren de items op de getallen van een waardeveld. Het rapport behoudt de sortering wanneer de gegevens veranderen.
- **Filter** laat items weg op hun labels (begint met, eindigt met, bevat, is gelijk aan) of op een waarde (groter dan, kleiner dan, tussen, enzovoort), of houdt alleen de bovenste of onderste paar items over op basis van een waarde. Het filter wordt berekend wanneer u OK kiest. Het verbergt de items die het weglaat; de selectievakjes hieronder laten zien welke dat zijn, en u kunt een item weer tonen door het aan te vinken.
- **Deze items weergeven** is een lijst van de items van het veld met elk een vinkje. Haal het vinkje weg om een item te verbergen; de totalen laten het dan weg.

## Datums en getallen groeperen

Selecteer een veld met datums of getallen in **Rijen** of **Kolommen** en kies **Veld groeperen** op het tabblad **Draaitabel**. Groepen worden uit de gegevens gemaakt, dus de gegevens in die kolom moeten alleen getallen of datums bevatten, zonder tekst en zonder lege cellen.

- **Datums**: vink aan waarop u wilt groeperen: **Seconden**, **Minuten**, **Uren**, **Dagen**, **Maanden**, **Kwartalen** of **Jaren**. Het fijnste onderdeel dat u aanvinkt, blijft bij het veld. Voor elk grover onderdeel voegt JeffCrunch een veld toe, met de naam ervan (bijvoorbeeld **Jaren**), en plaatst het vóór het datumveld op dezelfde plek. **Beginnen bij** en **Eindigen bij** zijn de eerste en laatste datum; laat ze staan om de vroegste en laatste datum uit de gegevens te gebruiken. Datums vóór het begin of na het einde komen in twee extra groepen, die beginnen met `<` en `>`.
- **Getallen**: **Beginnen bij**, **Eindigen bij** en **Per** maken groepen zoals 1-10, 11-20, enzovoort.

**Groep opheffen** haalt de groepen weg, en de velden die ervoor zijn toegevoegd. Vernieuwen maakt de groepen opnieuw op basis van de nieuwe gegevens. Wanneer twee draaitabellen uit dezelfde gegevens zijn gemaakt, kunnen er geen groepen worden gemaakt, omdat een groep een deel is van de gegevens die ze delen.

## Berekende velden

Een berekend veld is een veld dat u maakt met een formule. Het gebruikt andere velden van de lijst. Een bonus van een tiende van het bedrag is bijvoorbeeld `=Bedrag*0,1`.

Kies **Berekend veld** op het tabblad **Draaitabel**. Typ een naam en een formule. Schrijf de naam van een veld in de formule, of dubbelklik erop in de lijst **Velden** om het erin te zetten. Zet een naam met spaties tussen enkele aanhalingstekens, zoals `='Prijs per stuk'*Aantal`. U kunt getallen gebruiken, `+ - * / ^ %`, vergelijkingen en een paar functies, zoals `ALS`, `AFRONDEN`, `SOM`, `MIN`, `MAX`, `GEMIDDELDE` en `ABS`.

Kies **Toevoegen**. Het nieuwe veld wordt in het vak **Waarden** gezet. Het verschijnt ook in de veldenlijst, waar u het net als elk ander veld kunt aan- of uitvinken. Het kan niet in de rijen, kolommen of filters.

Een berekend veld wordt in elke cel van het rapport berekend. De naam van een veld staat voor het totaal van dat veld in de cel, dus `=Bedrag*Bedrag` kwadrateert het totaal van de cel. Het kwadrateert niet elke rij en telt die daarna op. Een eindtotaal gebruikt de totalen van de hele lijst.

**Formules weergeven** (naast **Berekend veld**) maakt een nieuw blad dat elk berekend veld en zijn formule benoemt. **Ongedaan maken** verwijdert de cellen, en nog een **Ongedaan maken** verwijdert het blad.

Om een veld te wijzigen, kiest u de naam ervan in het vak **Naam**. De formule wordt getoond. Wijzig hem en kies **Wijzigen**. **Verwijderen** haalt het veld en zijn waarde weg. Elke actie is één stap van **Ongedaan maken**. Een berekend veld wordt met het bestand opgeslagen en blijft behouden wanneer het rapport wordt vernieuwd. Twee draaitabellen die uit dezelfde gegevens zijn gemaakt, kunnen er geen hebben, omdat een berekend veld een deel is van de gegevens die ze delen.

## De rijen achter een waarde tonen

Dubbelklik op een waarde van een draaitabel, of kies **Details weergeven** op het tabblad **Draaitabel**. JeffCrunch maakt een nieuw blad, genoemd Detail 1, 2, enzovoort, met de rijen van de lijst waaruit het getal bestaat. De namen van de velden staan in de eerste rij. F2 bewerkt de cel nog steeds, als de cel kan worden bewerkt. **Ongedaan maken** verwijdert de rijen, en nog een **Ongedaan maken** verwijdert het blad.

## Uitvouwen en samenvouwen

Een veld waarvan de items andere items onder zich hebben, kan de regels onder zijn items tonen of verbergen. **Hele veld uitvouwen** en **Hele veld samenvouwen** doen dit voor elk item van het veld dat u in het deelvenster selecteert.

## Opties voor draaitabel

**Opties** op het tabblad **Draaitabel** opent een vak met de keuzes voor de tabel.

- **Naam** is de naam van de draaitabel. Geen andere draaitabel in de werkmap mag die naam hebben.
- **Voor foutwaarden tonen** en **Voor lege cellen tonen** zetten een eigen tekst in die cellen.
- **Cellen met labels samenvoegen en centreren**, **Kolombreedten aanpassen wanneer de tabel wordt bijgewerkt**, en **Opmaak van cellen behouden wanneer de tabel wordt bijgewerkt** worden samen met de tabel opgeslagen.
- **Totaalsommen voor rijen weergeven** en **voor kolommen weergeven** zijn hetzelfde als de keuzes voor eindtotalen.
- **Gegevens vernieuwen wanneer het bestand wordt geopend** leest de cellen elke keer opnieuw wanneer de werkmap wordt geopend. Het is één stap van Ongedaan maken, net als elke vernieuwing.

## Stijl: koppen, banden en de galerie

De **Stijlopties** op het tabblad **Draaitabel** zetten de kopregel, de eerste kolom (rijkoppen) en de banden van rijen en van kolommen aan of uit. **Stijlen** heeft een galerie met looks, en **Stijl wissen** haalt de look weg. Deze looks worden op dezelfde manier getekend als de looks van tabellen. **Lege rijen** en **Labels herhalen** stellen de lege regel en de herhaalde labels in voor elk veld van de rijen tegelijk.

## Ontwerp: subtotalen, eindtotalen en indeling

- **Subtotalen** toont of verbergt het totaal van elke groep, boven of onder de groep. Wanneer er meer dan één veld in Rijen staat, is het totaal van elk buitenste veld een subtotaal.
- **Eindtotalen** heeft vier keuzes: uit voor rijen en kolommen, aan voor beide, alleen voor rijen (een kolom met totalen rechts, één per rij), en alleen voor kolommen (een rij met totalen onderaan).
- **Rapportindeling** is de vorm van de labels. In **compacte** vorm delen alle velden van Rijen één kolom, en een nieuwe groep wordt een beetje naar rechts getoond. In **overzichtsvorm** heeft elk veld zijn eigen kolom, en elke groep heeft een eigen regel boven zijn items. In **tabelvorm** heeft elk veld zijn eigen kolom en staat de naam van de groep op de eerste regel, met het subtotaal eronder.

## Dezelfde lijst, een filter

Verplaats **Maand** van **Kolommen** naar **Filters**. Het rapport verliest zijn kolommen, en er verschijnt een regel erboven met **Maand** en **(Alles)**. De getallen zijn nu het totaal per regio over alle maanden.

## Draaigrafiek

Klik op een cel van een draaitabel en kies **Invoegen** en daarna **Draaigrafiek**. JeffCrunch maakt een kolomdiagram van de waarden van de tabel. Het laat de eindtotalen weg. De grafiek is in het bestand gemarkeerd als de grafiek van de draaitabel. Hij leest de cellen van de tabel, dus zijn getallen veranderen wanneer u de tabel vernieuwt of zijn indeling wijzigt. Komen er in de tabel meer rijen of kolommen bij, maak de grafiek dan opnieuw, omdat de grafiek de cellen houdt waaruit hij is gemaakt. Een draaigrafiek uit een bestand wordt getoond zoals hij is opgeslagen.

## Segmenteringen en tijdlijnen

Een segmentering is een reeks knoppen die een draaitabel filtert, en een tijdlijn is een schuifregelaar die hetzelfde doet voor datums.

Klik op een cel van een draaitabel en klik daarna op **Segmenteringen** op het tabblad Draaitabel. Het deelvenster Segmenteringen wordt geopend en toont de segmenteringen en tijdlijnen in de werkmap. Een segmentering heeft één knop voor elk item van zijn veld. De knoppen die blauw zijn, zijn de items die de tabel toont.

Klik op een knop om alleen dat item te tonen. Klik er nogmaals op om alle items weer te tonen. Houd **Ctrl** ingedrukt en klik om een item toe te voegen aan de getoonde items, of om er een weg te halen. Elke klik is één stap van Ongedaan maken, en Ongedaan maken zet de tabel terug. Wanneer u opslaat, bewaart het bestand de keuze van de segmentering.

**Rapportverbindingen** laat zien welke draaitabellen een segmentering filtert. Een tijdlijn toont het datumbereik dat hij heeft gekozen. JeffCrunch toont de segmenteringen in dit deelvenster, niet over de cellen, en past een tijdlijn nog niet aan. Beide worden in het bestand bewaard zoals ze zijn opgeslagen.

## Grenzen

Een draaitabel kan maximaal 2.000 velden hebben, een lijst van maximaal vijf miljoen rijen, en een rapport van maximaal vier miljoen cellen. Een bestand dat hierboven uitkomt, wordt getoond zoals het is opgeslagen en kan niet worden gewijzigd. Het opnieuw lezen van honderdduizend rijen met twintig kolommen en het berekenen van het rapport duurde ongeveer een halve seconde op de computer waarop dit is gemeten; een tragere computer heeft er langer over.
