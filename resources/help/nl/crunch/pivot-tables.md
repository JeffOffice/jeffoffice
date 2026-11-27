# Draaitabellen
<!-- keywords: ander bestand, andere werkmap, externe bron, formules weergeven, berekend veld, berekend item, berekeningsvolgorde, formulevelden, rapportfilterpagina's, draaitabel, omlaag doorklikken, omhoog doorklikken, draaitabel invoegen, maken, selecteren, draaitabel verplaatsen, GETPIVOTDATA, GETPIVOTDATA genereren, vernieuwen, alles vernieuwen, gegevensbron wijzigen, veldenlijst, velden, rijen, kolommen, waarden, filters, rapportfilter, totaalsom, subtotaal, indeling, compact, overzicht, tabelvorm, wissen, waardeveldinstellingen, som, aantal, gemiddelde, weergeven als, percentage van totaal, verschil, lopend totaal, getalnotatie, veldinstellingen, sorteren, filteren, boven, groeperen, groepering opheffen, datums, maanden, jaren, uitvouwen, samenvouwen, opties, stijl, gestreepte rijen, lege rijen, labels herhalen, vernieuwen bij openen van bestand, draaigrafiek, segmentering, tijdlijn, details weergeven, omlaag doorklikken, omhoog doorklikken, rijen achter een waarde -->

Een draaitabel is een klein rapport dat is gemaakt van een lange lijst. De lijst heeft bijvoorbeeld één rij voor elke verkoop; het rapport telt de verkopen op per regio en per maand. U schrijft geen formules. U geeft aan welke kolommen van de lijst aan de zijkant komen, welke bovenaan, en welke wordt opgeteld, en het rapport vult zichzelf in.

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
- **Rijen**, **Kolommen** en **Waarden** zijn de drie plaatsen waar een veld kan staan. Een veld in Rijen geeft één regel voor elk van zijn verschillende waarden; een veld in Kolommen geeft één kolom voor elk; een veld in Waarden is het veld dat wordt opgeteld. Een vierde plaats, **Filters**, bevat een veld dat de rijen weglaat die u niet kiest.
- Een **cache** (de draaitabelcache) is een kopie van uw lijst die de draaitabel in het bestand bewaart, zodat ze totalen kan tonen zonder naar de cellen te kijken. Het rapport wordt berekend op basis van de cache, niet op basis van de cellen, en verandert dus niet wanneer de cellen veranderen. **Vernieuwen** kopieert de cellen opnieuw naar de cache.

## Een draaitabel maken

1. Klik op een cel in de lijst, of selecteer de cellen die u wilt gebruiken. De lijst heeft boven de gegevens een rij met namen nodig.
2. Kies **Invoegen** en daarna **Draaitabel**.
3. Het vak toont de cellen waarin de gegevens staan. Pas ze aan als ze niet kloppen. U kunt ook de naam van een tabel typen. Om een lijst te gebruiken die in een ander bestand staat, kiest u **Een bereik in een ander bestand**, kiest u het bestand met **Bladeren**, en typt u het bereik, of de naam van een tabel of een naam die u in dat bestand hebt gemaakt, zoals `Blad1!A1:D100`. JeffCrunch leest werkmappen (`.xlsx`, `.xlsm`, `.xls` en `.ods`) en `.csv`-bestanden.
4. Kies waar de draaitabel komt: **Een nieuw blad** (de standaard), of een cel op dit blad of een ander blad. Kies een plek waar niets anders staat, want de lege tabel beslaat ongeveer achttien rijen en drie kolommen.
5. Kies **OK**. De tabel begint leeg, en het deelvenster **Draaitabelvelden** wordt geopend. Vink de velden aan die u wilt hebben: een veld met getallen wordt opgeteld, en elk ander veld wordt een rij. Verplaats daarna de velden naar de vakken zoals in de volgende paragrafen wordt beschreven.

Wanneer de gegevens in een ander bestand staan, bewaart de draaitabel een kopie ervan, zodat de tabel werkt en wordt opgeslagen zonder dat bestand. Zie Gegevens in een ander bestand hieronder.

Een nieuw blad is een eigen stap van Ongedaan maken, en de tabel ook. Wanneer u opslaat als .xlsx, wordt de tabel geschreven zoals het oorspronkelijke programma er een schrijft, zodat hij daar als draaitabel opent. Een kopie die als .xls of .ods wordt opgeslagen, bewaart de getallen die de tabel toont in de cellen, maar niet de tabel zelf, en het opslaan laat u dat weten.

## GETPIVOTDATA: één getal uit een draaitabel lezen

`GETPIVOTDATA` geeft het getal dat een draaitabel toont voor de waarden en de rijen en kolommen die u opgeeft. Voor het voorbeeld hierboven:

`=GETPIVOTDATA("Som van Bedrag"; $E$3; "Regio"; "Oost"; "Maand"; "Jan")` geeft 15.

- Het eerste argument is de naam van de waarde, zoals de tabel die toont (bijvoorbeeld Som van Bedrag). De naam van het veld dat wordt opgeteld (Bedrag) werkt ook.
- Het tweede argument is elke cel van de draaitabel.
- Daarna volgen paren van een veld en een van zijn items. Noemt u alleen Regio en Oost, dan krijgt u het totaal van Oost, 35. Noemt u niets, dan krijgt u het eindtotaal, 49. De velden die u noemt, moeten de eerste velden in de rijen zijn, of de eerste velden in de kolommen, in volgorde.
- Als de tabel het getal niet toont, toont de cel `#REF!`.

Wanneer u tijdens het typen van een formule op een waarde van een draaitabel klikt, schrijft JeffCrunch deze formule voor u, zodat die nog steeds het juiste getal leest nadat de tabel is herschikt. Om dit uit te schakelen, kiest u **Opties** op het tabblad **Draaitabel** en haalt u het vinkje weg bij het selectievakje over GETPIVOTDATA.

## Wat JeffCrunch met een draaitabel doet

Een draaitabel in een werkmap wordt getoond zoals hij is opgeslagen. Als u er niets aan verandert, wordt het bestand opgeslagen met de draaitabel precies zoals hij was. Wanneer u de tabel vernieuwt of wijzigt, berekent JeffCrunch het rapport opnieuw, zet de nieuwe getallen in de cellen en schrijft de draaitabel terug, zodat het programma dat hem heeft gemaakt, hem zonder foutmelding opent.

JeffCrunch berekent draaitabellen die optellen, tellen, gemiddelden berekenen en zo meer, die datums en getallen groeperen, en die waarden tonen als een aandeel, een verschil of een lopend totaal. Een draaitabel die een waarde toont als een rangorde of als een aandeel van een bovenliggend item, die groepen van items bevat die iemand met de hand heeft gemaakt, of die een gegevensmodel gebruikt, wordt getoond en bewaard zoals hij is opgeslagen, en de opdrachten laten u weten dat ze hem niet kunnen wijzigen.

Klik op een willekeurige cel van een draaitabel. Het tabblad **Draaitabel** verschijnt.

## Vernieuwen

**Vernieuwen** leest de cellen waaruit de draaitabel is gemaakt en berekent het rapport opnieuw. Nieuwe waarden komen na de waarden die er al staan; waarden die zijn verdwenen, worden verwijderd. Kolombreedten en de indeling die u hebt gekozen, blijven behouden. **Alles vernieuwen**, op het tabblad **Gegevens** en op het tabblad **Draaitabel**, doet hetzelfde voor elke draaitabel in de werkmap.

Als het rapport zou groeien over cellen die iets bevatten, vernieuwt JeffCrunch niet. Het laat u weten welke cellen in de weg staan. Wis ze of verplaats de draaitabel, en vernieuw opnieuw.

**Ongedaan maken** neemt een vernieuwing in één stap terug.

## Gegevensbron wijzigen

**Gegevensbron wijzigen** vraagt om de cellen waarin de lijst staat, inclusief de eerste rij met namen. Typ ze zoals `Blad1!A1:D100`, of typ de naam van een tabel of een naam die u hebt gemaakt. JeffCrunch leest ze en berekent het rapport opnieuw. Als een veld van de lijst in de nieuwe cellen dezelfde naam heeft, blijft de plek ervan in het rapport behouden.

U kunt de tabel ook naar een bereik in een ander bestand laten wijzen. Kies in het vak **Een bereik in een ander bestand**, kies het bestand en typ het bereik erin. Met **Een bereik in deze werkmap** wijst u de tabel weer naar cellen hier. Beide wijzigingen zijn één stap van Ongedaan maken.

## Gegevens in een ander bestand

Een draaitabel die van een ander bestand is gemaakt, bewaart de gegevens waarop hij is gebaseerd in deze werkmap, zodat u de werkmap kunt openen, wijzigen en opslaan wanneer het andere bestand er niet meer is. **Vernieuwen** leest het andere bestand opnieuw. Als het bestand is verplaatst, ontbreekt, te groot is, geen werkmap is, of het bereik niet meer heeft, behoudt de tabel de gegevens die hij had, en een bericht meldt dat het bestand niet kon worden gelezen. Een bestand dat op een andere computer in uw netwerk staat, wordt pas gelezen wanneer u het zelf kiest in **Gegevensbron wijzigen**. Eén vernieuwing leest elk bestand één keer, en in totaal ten hoogste 32 bestanden, 1 GB en 30 seconden; een tabel die daarboven uitkomt, behoudt zijn gegevens.

Als een werkmap zegt dat de tabel bij het openen opnieuw moet worden gelezen, en de gegevens van de tabel staan in een ander bestand, vraagt JeffCrunch eerst om toestemming, op dezelfde manier als bij de koppelingen naar andere werkmappen. Totdat u ja zegt, wordt geen enkel bestand gelezen.

Een werkmap die als .xlsx is opgeslagen, bewaart waar het bestand staat. Wanneer u het bestand in het vak kiest, wordt het volledige adres opgeslagen. Een werkmap die door een ander programma is gemaakt, waarvan de tabel naar een bestand verwijst met een korte padnaam zoals `Prijzen.xlsx`, bewaart dat pad, en JeffCrunch zoekt het bestand in dezelfde map als de werkmap. Wanneer u **Opslaan als** gebruikt om de werkmap in een andere map op te slaan, wordt zo'n pad herschreven zodat het nog steeds hetzelfde bestand aanduidt. Vernieuwen volgt geen koppelingen die het andere bestand naar nog andere bestanden heeft.

## Wissen

**Wissen** heeft twee keuzes. **Filters wissen** verwijdert alle filters en toont alle waarden weer. **Alles wissen** haalt elk veld uit het rapport en laat het leeg, klaar om opnieuw op te bouwen.

## Omlaag doorklikken en omhoog doorklikken

Wanneer een rapport meer dan één veld in de rijen heeft, kan een item van het eerste veld worden geopend of gesloten. Klik op de cel met de naam van het item en kies **Omhoog doorklikken** om het te sluiten: de regels van het volgende veld eronder worden verborgen en alleen het totaal blijft staan. **Omlaag doorklikken** opent het weer. Een item van het laatste veld heeft niets eronder. Elke handeling is één stap van **Ongedaan maken**.

## Rapportfilterpagina's

Een filter staat boven een draaitabel en laat u één item tegelijk bekijken, zoals één maand. **Rapportfilterpagina's weergeven** op het tabblad **Draaitabel** maakt in één keer een pagina voor elk item. Elke pagina is een nieuw blad, genoemd naar het item, met een eigen kopie van de tabel. Het filter van elke kopie is ingesteld op dat item. Heeft de tabel meer dan één filter, dan kiest u welk u wilt gebruiken. Eén **Ongedaan maken** haalt alle pagina's weg.

## Selecteren en verplaatsen

**Selecteren** op het tabblad **Draaitabel** selecteert een deel van het rapport: **Hele draaitabel**, **Waarden** (de getallen), of **Labels** (de namen aan de zijkant; als het rapport er geen aan de zijkant heeft, de namen bovenaan). **Draaitabel verplaatsen…** vraagt om een cel op hetzelfde blad en tekent het rapport opnieuw met de eerste cel daar. De cellen die worden achtergelaten, worden leeggemaakt. Formules die naar de cellen van het rapport verwijzen, volgen ze, zoals bij knippen en plakken. Als de nieuwe plek iets bevat, verplaatst JeffCrunch het rapport niet en laat het u dat weten. Eén **Ongedaan maken** zet het terug.

## Het deelvenster met velden

**Veldenlijst** op het tabblad **Draaitabel** opent het deelvenster **Draaitabelvelden**. Het toont de velden van de lijst met een vakje ernaast, en daaronder de vier plaatsen: **Filters**, **Kolommen**, **Rijen** en **Waarden**.

- Vink een veld aan om het toe te voegen. Een veld dat alleen getallen bevat, gaat naar **Waarden** en wordt opgeteld; elk ander veld gaat naar **Rijen**.
- Haal het vinkje weg bij een veld om het overal te verwijderen.
- Klik op een veld in een van de vier plaatsen en kies **Naar** om het naar een andere plaats te brengen. De woorden **Omhoog verplaatsen** en **Omlaag verplaatsen** wijzigen de volgorde op zijn plaats, en **Veld verwijderen** haalt het weg.
- Met meer dan één waarde verschijnt in **Kolommen** een regel met de naam **Waarden**. Verplaats die naar **Rijen** om de waarden aan de zijkant te zetten.
- **Waardeveldinstellingen** (dubbelklik op een waarde, of gebruik de knop) stelt de naam in die het rapport voor de waarde toont en hoe ze wordt opgeteld: **Som**, **Aantal**, **Aantal getallen**, **Gemiddelde**, **Max**, **Min**, **Product**, **StDev** en **StDevp** (de standaarddeviatie van een steekproef, of van alle waarden), en **Var** en **Varp** (de variantie van hetzelfde). Ook stelt het in hoe de waarde wordt weergegeven en welke getalnotatie ze krijgt. Zie de volgende paragraaf.
- **Veldinstellingen** (dubbelklik op een veld in Filters, Kolommen of Rijen, of gebruik de knop) stelt één veld in. Zie "Veldinstellingen".

Elke wijziging is één stap van **Ongedaan maken**.

## Waarden weergeven als, en de getalnotatie van een waarde

In **Waardeveldinstellingen** verandert **Waarden weergeven als** wat een cel van het rapport toont.

- **% van totaal**: de cel gedeeld door het totaal van het hele rapport. In het voorbeeld is Oost in januari 15 van 49, dat is 30,6%.
- **% van kolomtotaal** en **% van rijtotaal**: de cel gedeeld door het totaal van zijn kolom, of van zijn rij.
- **Index**: hoe de cel zich verhoudt tot wat zijn rij en kolom zouden geven als de gegevens gelijkmatig verdeeld waren. Een getal boven 1 betekent meer dan het gelijke aandeel.
- **Verschil met**, **% van** en **% verschil met**: de cel vergeleken met een ander item van een veld. Kies het **Basisveld** (een veld in Rijen of Kolommen) en het **Basisitem** (een van zijn items, of het item ervoor of erna). Met Maand als basisveld en Jan als basisitem laat februari zien hoeveel meer of minder het is dan januari. Het basisitem zelf blijft leeg bij een verschil, en is 100% bij **% van**.
- **Lopend totaal in**: elke cel telt de cellen ervoor op langs het basisveld.
- **Rangorde van klein naar groot** en **Rangorde van groot naar klein**: elke cel toont zijn plaats, 1, 2, 3, enzovoort, onder de cellen van het basisveld in dezelfde rij of kolom. Rang 1 is het kleinste getal, of het grootste.
- **% van bovenliggend rijtotaal** en **% van bovenliggend kolomtotaal**: elke cel toont welk deel van zijn bovenliggend item het is. Het bovenliggende item van een item is het item één niveau hoger, zoals het jaar dat een maand bevat. Een item zonder bovenliggend item wordt getoond als een deel van het eindtotaal.

**Getalnotatie** neemt een notatiecode, zoals `0,00`, `#.##0` of `0,0%`. Laat het leeg om de notatie te behouden die de cellen hebben. Een waarde die als aandeel wordt getoond, krijgt `0,00%`, tenzij u een andere notatie kiest. De cellen van het rapport nemen de notatie meteen over. **Ongedaan maken** neemt haar terug samen met de getallen.

## Veldinstellingen

Selecteer een veld in het vak **Filters**, **Kolommen** of **Rijen** en kies **Veldinstellingen** op het tabblad **Draaitabel** (of dubbelklik erop).

- **Aangepaste naam** wijzigt de naam die het rapport voor het veld toont.
- **Subtotalen**: **Automatisch** toont een totaal voor elke groep van het veld, **Geen** doet dat niet.
- **Itemlabels weergeven in**: compacte, overzichts- of tabelvorm, zoals bij Rapportindeling.
- **Na elk item een lege regel invoegen** zet een lege regel na elke groep.
- **Itemlabels herhalen** toont de naam van een buitenste item op elke regel van zijn groep (overzichts- en tabelvorm).
- **Sorteren**: de volgorde van de items behouden (handmatig), of sorteren van A naar Z of van Z naar A op de labels. **Van klein naar groot op een waarde** en **Van groot naar klein op een waarde** sorteren de items op de getallen van een waardeveld. Het rapport behoudt de sortering wanneer de gegevens veranderen.
- **Filter** laat items weg op hun labels (begint met, eindigt op, bevat, is gelijk aan) of op een waarde (groter dan, kleiner dan, tussen, en zo meer), of houdt alleen de bovenste of onderste paar items over op basis van een waarde. Het filter wordt berekend wanneer u OK kiest. Het verbergt de items die het weglaat; de vinkjes hieronder laten zien welke dat zijn, en u kunt een item weer zichtbaar maken door het aan te vinken.
- **Deze items weergeven** is een lijst van de items van het veld, elk met een vinkje. Haal een vinkje weg om het item te verbergen; de totalen laten het dan weg.

## Datums en getallen groeperen

Selecteer een veld met datums of getallen in **Rijen** of **Kolommen** en kies **Veld groeperen** op het tabblad **Draaitabel**. Groepen worden uit de gegevens gemaakt, dus de gegevens moeten in die kolom alleen getallen of datums bevatten, zonder tekst en zonder lege cellen.

- **Datums**: vink aan waarop u wilt groeperen: **Seconden**, **Minuten**, **Uren**, **Dagen**, **Maanden**, **Kwartalen** of **Jaren**. Het fijnste deel dat u aanvinkt, blijft bij het veld. Voor elk grovere deel voegt JeffCrunch een veld toe, genoemd naar dat deel (bijvoorbeeld **Jaren**), en zet het vóór het datumveld op dezelfde plek. **Beginnen bij** en **Eindigen bij** zijn de eerste en laatste datum; laat ze staan om de vroegste en de laatste datum uit de gegevens te gebruiken. Datums vóór het begin of na het einde gaan in twee extra groepen, die beginnen met `<` en `>`.
- **Getallen**: **Beginnen bij**, **Eindigen bij** en **Met stappen van** maken groepen zoals 1-10, 11-20, enzovoort.

**Groepering opheffen** verwijdert de groepen en de velden die daarvoor zijn toegevoegd. Vernieuwen maakt de groepen opnieuw op basis van de nieuwe gegevens. Wanneer twee draaitabellen op dezelfde gegevens zijn gebaseerd, kunnen er geen groepen worden gemaakt, omdat een groep een deel is van de gegevens die ze delen.

## Berekende velden

Een berekend veld is een veld dat u maakt met een formule. Het gebruikt andere velden van de lijst. Een bonus van een tiende van het bedrag is bijvoorbeeld `=Bedrag*0,1`.

Kies **Berekend veld** op het tabblad **Draaitabel**. Typ een naam en een formule. Schrijf de naam van een veld in de formule, of dubbelklik erop in de lijst **Velden** om het erin te zetten. Zet een naam met spaties tussen enkele aanhalingstekens, zoals `='Eenheidsprijs'*Aantal`. U kunt getallen gebruiken, `+ - * / ^ %`, vergelijkingen en een paar functies zoals `ALS`, `AFRONDEN`, `SOM`, `MIN`, `MAX`, `GEMIDDELDE` en `ABS`.

Kies **Toevoegen**. Het nieuwe veld wordt in het vak **Waarden** gezet. Het verschijnt ook in de veldenlijst, waar u het net als elk ander veld kunt aan- of uitvinken. Het kan niet in de rijen, kolommen of filters.

Een berekend veld wordt in elke cel van het rapport berekend. De naam van een veld staat voor het totaal van dat veld in de cel, dus `=Bedrag*Bedrag` kwadrateert het totaal van de cel. Het kwadrateert niet elke rij afzonderlijk en telt die dan op. Een eindtotaal gebruikt de totalen van de hele lijst.

**Lijstformules** (naast **Berekend veld**) maakt een nieuw blad dat elk berekend veld en zijn formule noemt, en elk berekend item met zijn formule, zijn veld en zijn berekeningsvolgorde. **Ongedaan maken** verwijdert de cellen, en nog een **Ongedaan maken** verwijdert het blad.

Om een veld te wijzigen, kiest u zijn naam in het vak **Naam**. Zijn formule verschijnt. Wijzig hem en kies **Wijzigen**. **Verwijderen** haalt het veld en zijn waarde weg. Elke handeling is één stap van **Ongedaan maken**. Een berekend veld wordt in het bestand bewaard en blijft behouden wanneer het rapport wordt vernieuwd. Twee draaitabellen die op dezelfde gegevens zijn gebaseerd, kunnen er geen hebben, omdat een berekend veld een deel is van de gegevens die ze delen.

## Berekende items

Een berekend item is een item dat u maakt uit andere items van hetzelfde veld. Het veld Regio heeft bijvoorbeeld de items Oost en West. Een nieuw item met de naam Beide kan `=Oost+West` zijn.

Klik op een cel in de draaitabel en kies **Berekend item** op het tabblad **Draaitabel**. Kies het **Veld**. Het moet in de rijen of de kolommen staan. Typ een **Naam** en een **Formule**. Schrijf de naam van een item in de formule, of dubbelklik erop in de lijst **Items** om het erin te zetten. Zet een naam met spaties tussen enkele aanhalingstekens, zoals `='Groot Oost'+West`. U kunt getallen gebruiken, `+ - * / ^ %` en haakjes.

Het nieuwe item krijgt een regel in het rapport. In elke kolom toont het de formule berekend uit de regels van de items die erin worden genoemd. Het nieuwe item telt mee in de totalen. Een totaal van sommen, zoals een eindtotaal of een subtotaal, telt de regel van het nieuwe item op bij de regels van de andere items. Dus met Oost op 35, West op 14, en Beide (Oost plus West) op 49, is het eindtotaal 98. Als een item in de formule verborgen is, toont de cel **#REF!**. Als de formule een item noemt dat er niet is, toont de cel **#NAAM?**.

Om een item te wijzigen, kiest u zijn naam in het vak **Naam**. Zijn formule verschijnt. Wijzig hem en kies **Wijzigen**. **Verwijderen** haalt het item en zijn regel weg. Elke handeling is één stap van **Ongedaan maken**. Een berekend item wordt in het bestand bewaard en blijft behouden wanneer het rapport wordt vernieuwd. Twee draaitabellen die op dezelfde gegevens zijn gebaseerd, kunnen er geen hebben, omdat een berekend item een deel is van de gegevens die ze delen.

### Berekeningsvolgorde

Soms heeft één cel twee berekende items. Het ene staat in zijn rij en het andere in zijn kolom. De volgorde waarin ze worden berekend, kan het antwoord veranderen. Stel dat het rijitem `=Oost-West` is en het kolomitem `=Jan+1`. Berekend in de ene volgorde is de cel (Oost - West) + 1. In de andere volgorde is het (Oost + 1) - (West + 1), wat een ander getal is.

**Berekeningsvolgorde** op het tabblad **Draaitabel** toont de berekende items in de volgorde waarin ze worden berekend. Het item dat lager in de lijst staat, wordt als laatste berekend. Kies een item en klik op **Omhoog verplaatsen** of **Omlaag verplaatsen**. De volgorde wordt samen met het bestand opgeslagen als de volgorde van de items, en andere programma's lezen die op dezelfde manier. **Lijstformules** toont de plaats van elk item erin.

## De rijen achter een waarde tonen

Dubbelklik op een waarde van een draaitabel, of kies **Details weergeven** op het tabblad **Draaitabel**. JeffCrunch maakt een nieuw blad, genaamd Detail 1, 2, enzovoort, met de rijen van de lijst die het getal vormen. De namen van de velden staan in de eerste rij. F2 bewerkt de cel nog steeds, als de cel kan worden bewerkt. **Ongedaan maken** verwijdert de rijen, en nog een **Ongedaan maken** verwijdert het blad.

## Uitvouwen en samenvouwen

Een veld waarvan de items andere items eronder hebben, kan de regels onder zijn items tonen of verbergen. **Hele veld uitvouwen** en **Hele veld samenvouwen** doen dat voor elk item van het veld dat u in het deelvenster selecteert.

## Opties voor draaitabel

**Opties** op het tabblad **Draaitabel** opent een vak met de instellingen van de tabel.

- **Naam** is de naam van de draaitabel. Geen andere draaitabel van de werkmap mag die hebben.
- **Voor foutwaarden tonen** en **Voor lege cellen tonen** zetten een eigen tekst in die cellen.
- **Cellen met labels samenvoegen en centreren**, **Kolombreedten aanpassen wanneer de tabel wordt vernieuwd** en **Opmaak van cellen behouden wanneer de tabel wordt vernieuwd** worden bij de tabel opgeslagen.
- **Totaalsommen voor rijen weergeven** en **voor kolommen weergeven** zijn hetzelfde als de keuzes voor Eindtotalen.
- **Gegevens vernieuwen wanneer het bestand wordt geopend** leest de cellen elke keer opnieuw wanneer de werkmap wordt geopend. Het is één stap van Ongedaan maken, net als elke vernieuwing.

## Stijl: koppen, banden en de galerie

De **Stijlopties** op het tabblad **Draaitabel** zetten de kopregel, de eerste kolom (rijkoppen) en de banden van rijen en van kolommen aan of uit. Met **Stijlen** heeft u een galerie met uitstralingen, en **Stijl wissen** haalt de uitstraling weg. Deze uitstralingen worden op dezelfde manier getekend als die van tabellen. **Lege rijen** en **Labels herhalen** stellen de lege regel en de herhaalde labels voor elk veld van de rijen tegelijk in.

## Ontwerp: subtotalen, eindtotalen en indeling

- **Subtotalen** toont of verbergt het totaal van elke groep, boven of onder de groep. Wanneer er meer dan één veld in Rijen staat, is het totaal van elk buitenste veld een subtotaal.
- **Eindtotalen** heeft vier keuzes: uit voor rijen en kolommen, aan voor beide, alleen aan voor rijen (een kolom met totalen rechts, één voor elke rij), en alleen aan voor kolommen (een rij met totalen onderaan).
- **Rapportindeling** is de vorm van de labels. In **compacte** vorm delen alle velden van Rijen één kolom, en een nieuwe groep wordt een beetje rechts getoond. In **overzichtsvorm** heeft elk veld een eigen kolom, en elke groep heeft een eigen regel boven zijn items. In **tabelvorm** heeft elk veld een eigen kolom en staat de naam van de groep op de eerste regel, met het subtotaal eronder.

## Dezelfde lijst, een filter

Verplaats **Maand** van **Kolommen** naar **Filters**. Het rapport verliest zijn kolommen, en een regel erboven toont **Maand** en **(Alle)**. De getallen zijn nu het totaal van elke regio over alle maanden.

## Draaigrafiek

Klik op een cel in een draaitabel en kies **Invoegen**, en daarna **Draaigrafiek**. JeffCrunch maakt een kolomdiagram van de waarden van de tabel. Het laat de eindtotalen weg. De grafiek is in het bestand als de grafiek van de draaitabel gemarkeerd. Hij leest de cellen van de tabel, dus zijn getallen veranderen wanneer u de tabel vernieuwt of de indeling ervan wijzigt. Als de tabel meer rijen of kolommen krijgt, maak de grafiek dan opnieuw, want de grafiek houdt de cellen vast waaruit hij is gemaakt. Een draaigrafiek die uit een bestand komt, wordt getoond zoals hij is opgeslagen.

## Segmenteringen en tijdlijnen

Een segmentering is een reeks knoppen die een draaitabel filtert, en een tijdlijn is een schuifbalk die hetzelfde doet voor datums.

Klik op een cel in een draaitabel en klik vervolgens op **Segmenteringen** op het tabblad Draaitabel. Het deelvenster Segmenteringen wordt geopend en toont de segmenteringen en tijdlijnen van de werkmap. Een segmentering heeft één knop voor elk item van zijn veld. De knoppen die blauw zijn, zijn de items die de tabel toont.

Klik op een knop om alleen dat item te tonen. Klik er nogmaals op om alle items weer te tonen. Houd **Ctrl** ingedrukt en klik om een item aan de getoonde items toe te voegen, of om er een weg te halen. Elke klik is één stap van Ongedaan maken, en Ongedaan maken zet de tabel terug. Wanneer u opslaat, bewaart het bestand de keuze van de segmentering.

Een segmentering die bij het bestand hoort, wordt ook over de cellen getekend, op de plek die het bestand aangeeft. Ze toont haar titel en één knop voor elk item. De knoppen die blauw zijn, zijn de items die de tabel toont. Klik op een knop om alleen dat item te tonen, houd **Ctrl** ingedrukt en klik om een item toe te voegen of weg te halen, en klik op de trechter rechtsboven om weer alle items te tonen. Een klik is één stap van Ongedaan maken. Wanneer de tabel op een andere manier is gefilterd, of u op Ongedaan maken klikt, volgen de knoppen dat. Wanneer een segmentering te klein is om al haar knoppen te tonen, verschijnt er rechts een schuifbalk: draai het muiswiel boven de segmentering, klik boven of onder de schuifknop, of sleep de schuifknop om bij de rest te komen. Houd **Shift** ingedrukt en klik op een tweede knop om het hele bereik van items te kiezen, van de eerste knop waarop u klikte tot deze; houd **Shift** en **Ctrl** ingedrukt om het bereik aan de al gekozen items toe te voegen. Een afgedrukte segmentering kan niet scrollen, dus drukt ze de knoppen af die passen, en haar laatste regel zegt hoeveel items er nog meer zijn. Een segmentering die een tabel filtert in plaats van een draaitabel, of waarvan de draaitabel niet in het bestand staat, wordt getekend met haar titel en een zin die zegt dat JeffCrunch haar nog niet kan gebruiken; ze wordt in het bestand bewaard zoals ze was. In een werkmap die alleen-lezen is geopend, en op een blad dat is beveiligd tegen wijzigingen aan draaitabellen, kan een klik de tabel niet filteren. Een segmentering wordt met het blad afgedrukt, tenzij het bestand zegt dat dat niet moet. Als u op een knop van een segmentering klikt, wordt er gefilterd. Als u op de titel of de rand klikt, wordt de segmentering als object geselecteerd, net als een afbeelding. Een geselecteerde segmentering heeft greepjes: sleep haar om haar te verplaatsen, sleep een greepje om haar van grootte te veranderen, of druk op een pijltoets om haar te verplaatsen. Houd **Ctrl** of **Shift** ingedrukt en klik op de titel om haar samen met andere objecten te selecteren. Klik met de rechtermuisknop op een segmentering voor een menu met **Verwijderen** en het deelvenster **Segmenteringen**. Druk op **Delete** om een geselecteerde segmentering te verwijderen: het bestand heeft dan geen segmentering meer, en het filter dat ze op de draaitabel zette, blijft staan totdat u het wijzigt. Verplaatsen, van grootte veranderen en verwijderen zijn elk één stap van Ongedaan maken, en een werkmap die alleen-lezen is geopend, of een blad dat is beveiligd tegen wijzigen van objecten, staat dat niet toe.

Om een segmentering met het toetsenbord te bedienen, drukt u op **F6** om naar de eerste segmentering op het blad te gaan (**Shift+F6** gaat naar de laatste). Gebruik daarna de pijltoetsen, **Pos1**, **Einde**, **Page Up** en **Page Down** om tussen de knoppen te bewegen, **Spatie** of **Enter** om de knop te kiezen, **Ctrl+Spatie** om hem aan de gekozen items toe te voegen of weg te halen, **Shift+Spatie** om het bereik van de laatst gekozen knop tot nu te kiezen, en **Ctrl+Alt+C** om weer alle items te tonen. Druk op **Esc** om de knoppen los te laten (de pijltoetsen verplaatsen dan de segmentering, en **Delete** verwijdert haar; terwijl u tussen de knoppen bent, doet **Delete** niets), en nogmaals op **Esc** om terug te gaan naar de cellen. **F6** gaat door naar de volgende segmentering, en na de laatste weer naar de cellen.

**Rapportverbindingen** zegt welke draaitabellen een segmentering filtert. Een tijdlijn toont de periode van datums die ze heeft gekozen. JeffCrunch tekent een tijdlijn nog niet over de cellen en wijzigt er nog geen. Beide worden in het bestand bewaard zoals ze zijn opgeslagen.

## Limieten

Een draaitabel kan maximaal 2.000 velden hebben, een lijst van maximaal vijf miljoen rijen, en een rapport van maximaal vier miljoen cellen. Een bestand dat hier overheen gaat, wordt getoond zoals het is opgeslagen en kan niet worden gewijzigd. Het opnieuw inlezen van honderdduizend rijen met twintig kolommen en het berekenen van het rapport duurde ongeveer een halve seconde op de computer waarop dit is gemeten; een tragere computer doet er langer over.
