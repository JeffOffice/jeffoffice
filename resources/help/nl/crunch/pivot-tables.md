# Draaitabellen

<!-- keywords: ander bestand, andere werkmap, externe bron, formules weergeven, berekend veld, berekend item, berekeningsvolgorde, formulevelden, rapportfilterpagina's, draaitabel, omlaag doorklikken, omhoog doorklikken, draaitabel invoegen, maken, selecteren, draaitabel verplaatsen, gegevens ophalen, GETPIVOTDATA, GETPIVOTDATA genereren, vernieuwen, alles vernieuwen, gegevensbron wijzigen, veldenlijst, velden, rijen, kolommen, waarden, filters, rapportfilter, totaal, subtotaal, indeling, compact, overzicht, tabelvorm, wissen, instellingen waardeveld, som, aantal, gemiddelde, waarden weergeven als, percentage van totaal, verschil, lopend totaal, getalnotatie, veldinstellingen, sorteren, filteren, boven, groeperen, opheffen, datums, maanden, jaren, uitvouwen, samenvouwen, opties, stijl, gestreepte rijen, lege rijen, labels herhalen, vernieuwen bij openen van bestand, draaigrafiek, segmentering, tijdlijn, details weergeven, doorklikken, rijen achter een waarde -->

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
- **Rijen**, **Kolommen** en **Waarden** zijn de drie plaatsen waar een veld kan komen. Een veld in Rijen geeft één regel voor elk van zijn verschillende items; een veld in Kolommen geeft één kolom voor elk item; een veld in Waarden is het veld dat wordt opgeteld. Een vierde plaats, **Filters**, bevat een veld dat de rijen weglaat die u niet kiest.
- Een **cache** (de draaitabelcache) is een kopie van uw lijst die de draaitabel in het bestand bewaart, zodat ze totalen kan tonen zonder naar de cellen te kijken. Het rapport wordt berekend op basis van de cache, niet op basis van de cellen, en verandert dus niet wanneer de cellen veranderen. **Vernieuwen** kopieert de cellen opnieuw naar de cache.

## Een draaitabel maken

1. Klik op een cel in de lijst, of selecteer de cellen die u wilt gebruiken. De lijst heeft boven de gegevens een rij met namen nodig.
2. Kies **Invoegen** en daarna **Draaitabel**.
3. Het vak toont de cellen waarin de gegevens staan. Pas ze aan als ze niet kloppen. U kunt ook de naam van een tabel typen. Om een lijst te gebruiken die in een ander bestand staat, kiest u **Een bereik in een ander bestand**, kiest u het bestand met **Bladeren** en typt u het bereik, of de naam van een tabel of een naam die u in dat bestand hebt gemaakt, zoals `Blad1!A1:D100`. JeffCrunch leest werkmappen (`.xlsx`, `.xlsm`, `.xls` en `.ods`) en `.csv`-bestanden.
4. Kies waar de draaitabel komt: **Nieuw blad** (de standaard), of een cel op dit blad of een ander blad. Kies een plek waar niets anders staat, omdat de lege tabel ongeveer achttien rijen en drie kolommen beslaat.
5. Kies **OK**. De tabel begint leeg en het deelvenster **Draaitabelvelden** wordt geopend. Vink de velden aan die u wilt: een veld met getallen wordt opgeteld, en elk ander veld wordt een rij. Verplaats daarna de velden naar de vakken zoals in de volgende paragrafen staat.

Wanneer de gegevens in een ander bestand staan, bewaart de draaitabel een kopie ervan, zodat de tabel werkt en wordt opgeslagen zonder dat bestand. Zie Gegevens in een ander bestand hieronder.

Een nieuw blad is een eigen stap van Ongedaan maken, en de tabel ook. Wanneer u opslaat als .xlsx, wordt de tabel geschreven zoals het oorspronkelijke programma dat doet, zodat hij daar als draaitabel opent. Een kopie die als .xls of .ods is opgeslagen, bewaart de getallen die de tabel toont in de cellen, maar niet de tabel zelf, en het opslaan meldt dat.

## GETPIVOTDATA: één getal uit een draaitabel lezen

`GETPIVOTDATA` geeft het getal dat een draaitabel toont voor de waarden en de rijen en kolommen die u opgeeft. Voor het voorbeeld hierboven:

`=GETPIVOTDATA("Som van Bedrag"; $E$3; "Regio"; "Oost"; "Maand"; "Jan")` geeft 15.

- Het eerste argument is de naam van de waarde, zoals de tabel die toont (bijvoorbeeld Som van Bedrag). De naam van het veld dat wordt opgeteld (Bedrag) werkt ook.
- Het tweede argument is elke cel van de draaitabel.
- Daarna volgen paren van een veld en een van zijn items. Geeft u alleen Regio en Oost op, dan krijgt u het totaal van Oost, 35. Geeft u niets op, dan krijgt u het eindtotaal, 49. De velden die u opgeeft moeten de eerste velden in de rijen zijn, of de eerste velden in de kolommen, in volgorde.
- Toont de tabel het getal niet, dan toont de cel `#VERW!`.

Wanneer u tijdens het typen van een formule op een waarde van een draaitabel klikt, schrijft JeffCrunch deze formule voor u, zodat hij na het herschikken van de tabel nog steeds het juiste getal leest. Om dit uit te schakelen, kiest u **Opties** op het tabblad **Draaitabel** en haalt u het vinkje weg bij het selectievakje over GETPIVOTDATA.

## Wat JeffCrunch met een draaitabel doet

Een draaitabel in een werkmap wordt getoond zoals hij is opgeslagen. Als u er niets aan verandert, wordt het bestand opgeslagen met de draaitabel precies zoals hij was. Wanneer u de tabel vernieuwt of wijzigt, berekent JeffCrunch het rapport opnieuw, zet de nieuwe getallen in de cellen en schrijft de draaitabel terug, zodat het programma dat hem heeft gemaakt hem zonder problemen opent.

JeffCrunch berekent draaitabellen die optellen, tellen, gemiddelden berekenen enzovoort, die datums en getallen groeperen, en die waarden als percentage, verschil of lopend totaal tonen. Een draaitabel die een waarde als rang of als deel van een bovenliggend item toont, groepen items bevat die iemand met de hand heeft gemaakt, of een gegevensmodel gebruikt, wordt getoond en bewaard zoals hij is opgeslagen, en de opdrachten melden dat ze hem niet kunnen wijzigen.

Klik op een willekeurige cel van een draaitabel. Het tabblad **Draaitabel** verschijnt.

## Vernieuwen

**Vernieuwen** leest de cellen waarvan de draaitabel is gemaakt en berekent het rapport opnieuw. Nieuwe items komen na de items die er al zijn; items die zijn verdwenen, vervallen. Kolombreedten en de indeling die u hebt gekozen, blijven behouden. **Alles vernieuwen**, op het tabblad **Gegevens** en op het tabblad **Draaitabel**, doet hetzelfde voor elke draaitabel in de werkmap.

Als het rapport groter zou worden en over cellen heen zou lopen die iets bevatten, vernieuwt JeffCrunch niet. Het meldt welke cellen in de weg staan. Wis ze of verplaats de draaitabel, en vernieuw opnieuw.

**Ongedaan maken** neemt een vernieuwing in één stap terug.

## Gegevensbron wijzigen

**Gegevensbron wijzigen** vraagt om de cellen waarin de lijst staat, inclusief de eerste rij met namen. Typ ze als `Blad1!A1:D100`, of typ de naam van een tabel of een naam die u hebt gemaakt. JeffCrunch leest ze en berekent het rapport opnieuw. Als een veld van de lijst in de nieuwe cellen dezelfde naam heeft, blijft de plaats ervan in het rapport behouden.

U kunt de tabel ook naar een bereik in een ander bestand laten verwijzen. Kies in het vak **Een bereik in een ander bestand**, kies het bestand en typ het bereik erin. Kiest u **Een bereik in deze werkmap**, dan verwijst de tabel weer naar cellen hier. Elke van deze wijzigingen is één stap van Ongedaan maken.

## Gegevens in een ander bestand

Een draaitabel die is gemaakt uit een ander bestand, bewaart de gegevens waaruit hij is gemaakt binnen deze werkmap, zodat u de werkmap kunt openen, wijzigen en opslaan wanneer het andere bestand er niet meer is. **Vernieuwen** leest het andere bestand opnieuw. Is het bestand verplaatst, ontbreekt het, is het te groot, is het geen werkmap, of heeft het het bereik niet meer, dan houdt de tabel de gegevens die hij had, en een bericht meldt dat het bestand niet kon worden gelezen. Een bestand dat op een andere computer in uw netwerk staat, wordt pas gelezen wanneer u het zelf kiest in **Gegevensbron wijzigen**. Eén vernieuwing leest elk bestand één keer, en in totaal ten hoogste 32 bestanden, 1 GB en 30 seconden; een tabel die daarover heen gaat, behoudt zijn gegevens.

Als een werkmap aangeeft dat de tabel opnieuw moet worden gelezen wanneer hij wordt geopend, en de gegevens van de tabel staan in een ander bestand, vraagt JeffCrunch eerst, op dezelfde manier als bij de koppelingen naar andere werkmappen. Totdat u ja zegt, wordt geen enkel bestand gelezen.

Een werkmap die als .xlsx is opgeslagen, bewaart waar het bestand staat. Als u het bestand in het vak kiest, wordt het volledige adres opgeslagen. Een werkmap die door een ander programma is gemaakt, waarvan de tabel naar een bestand verwijst met een korte pad zoals `Prijzen.xlsx`, bewaart dat pad, en JeffCrunch zoekt het bestand in dezelfde map als de werkmap. Wanneer u met **Opslaan als** de werkmap in een andere map opslaat, wordt zo'n pad herschreven zodat het nog steeds naar hetzelfde bestand verwijst. Vernieuwen volgt geen koppelingen die het andere bestand zelf naar nog andere bestanden heeft.

## Wissen

**Wissen** heeft twee keuzes. **Filters wissen** verwijdert alle filters en toont alle items weer. **Alles wissen** haalt elk veld uit het rapport en laat het leeg, klaar om opnieuw op te bouwen.

## Omlaag en omhoog doorklikken

## Rapportfilterpagina's

Een filter staat boven een draaitabel en laat u één item tegelijk bekijken, bijvoorbeeld één maand. **Rapportfilterpagina's weergeven** op het tabblad **Draaitabel** maakt tegelijk een pagina voor elk item. Elke pagina is een nieuw blad, genoemd naar het item, met een eigen kopie van de tabel. Het filter van elke kopie staat op dat item. Heeft de tabel meer dan één filter, dan kiest u het filter dat u wilt gebruiken. Eén **Ongedaan maken** verwijdert alle pagina's.

Wanneer een rapport meer dan één veld in de rijen heeft, kan een item van het eerste veld worden geopend of gesloten. Klik op de cel met de naam van het item en kies **Omhoog doorklikken** om het te sluiten: de regels van het volgende veld eronder worden verborgen en alleen het totaal blijft staan. **Omlaag doorklikken** opent het weer. Een item van het laatste veld heeft niets eronder. Elke handeling is één **Ongedaan maken**.

## Selecteren en verplaatsen

**Selecteren** op het tabblad **Draaitabel** selecteert een deel van het rapport: **Volledige draaitabel**, **Waarden** (de getallen) of **Labels** (de namen aan de zijkant; heeft het rapport geen namen aan de zijkant, dan de namen bovenaan). **Draaitabel verplaatsen…** vraagt om een cel op hetzelfde blad en tekent het rapport opnieuw met de eerste cel daar. De cellen die het achterlaat, worden leeggemaakt. Formules die naar de cellen van het rapport verwijzen, volgen ze, zoals bij knippen en plakken. Staat er iets op de nieuwe plek, dan verplaatst JeffCrunch het rapport niet en meldt het dat. Eén **Ongedaan maken** zet het terug.

## Het deelvenster met velden

**Veldenlijst** op het tabblad **Draaitabel** opent het deelvenster **Draaitabelvelden**. Het toont de velden van de lijst met een vakje ernaast, en daaronder de vier plaatsen: **Filters**, **Kolommen**, **Rijen** en **Waarden**.

- Vink een veld aan om het toe te voegen. Een veld dat alleen getallen bevat, gaat naar **Waarden** en wordt opgeteld; elk ander veld gaat naar **Rijen**.
- Haal het vinkje weg bij een veld om het overal te verwijderen.
- Klik op een veld in een van de vier plaatsen en kies **Verplaatsen naar** om het naar een andere plaats te brengen. De woorden **Omhoog verplaatsen** en **Omlaag verplaatsen** wijzigen de volgorde ervan op zijn plaats, en **Veld verwijderen** haalt het eruit.
- Met meer dan één waarde verschijnt in **Kolommen** een regel met de naam **Waarden**. Verplaats die naar **Rijen** om de waarden aan de zijkant te zetten.
- **Instellingen waardeveld** (dubbelklik op een waarde, of gebruik de knop) bepaalt de naam die het rapport voor de waarde toont en hoe ze wordt opgeteld: **Som**, **Aantal**, **Aantal getallen**, **Gemiddelde**, **Max**, **Min**, **Product**, **StDev** en **StDevp** (de standaarddeviatie van een steekproef, of van alle waarden), en **Var** en **Varp** (de variantie van hetzelfde). Ook bepaalt het hoe de waarde wordt weergegeven en de getalnotatie ervan. Zie de volgende paragraaf.
- **Veldinstellingen** (dubbelklik op een veld in Filters, Kolommen of Rijen, of gebruik de knop) stelt één veld in. Zie "Veldinstellingen".

Elke wijziging is één stap van **Ongedaan maken**.

## Waarden weergeven als, en de getalnotatie van een waarde

In **Instellingen waardeveld** bepaalt **Waarden weergeven als** wat een cel van het rapport toont.

- **% van eindtotaal**: de cel gedeeld door het totaal van het hele rapport. In het voorbeeld is Oost in januari 15 van 49, dat is 30,6%.
- **% van kolomtotaal** en **% van rijtotaal**: de cel gedeeld door het totaal van zijn kolom, of van zijn rij.
- **Index**: hoe de cel zich verhoudt tot wat zijn rij en kolom zouden geven als de gegevens gelijkmatig verdeeld waren. Een getal boven 1 betekent meer dan het gelijke deel.
- **Verschil met**, **% van** en **% verschil met**: de cel vergeleken met een ander item van een veld. Kies het **Basisveld** (een veld in Rijen of Kolommen) en het **Basisitem** (een van zijn items, of het vorige of volgende). Met Maand als basisveld en Jan als basisitem laat februari zien hoeveel meer of minder het is dan januari. Het basisitem zelf blijft leeg bij een verschil, en is 100% bij **% van**.
- **Lopend totaal in**: elke cel telt de cellen ervoor op langs het basisveld.
- **Rang van klein naar groot** en **Rang van groot naar klein**: elke cel toont zijn plaats, 1, 2, 3 enzovoort, onder de cellen van het basisveld in dezelfde rij of kolom. Rang 1 is het kleinste getal, of het grootste.
- **% van bovenliggend rijtotaal** en **% van bovenliggend kolomtotaal**: elke cel toont welk deel van zijn bovenliggend item hij is. Het bovenliggende item van een item is het item één niveau hoger, zoals het jaar dat een maand bevat. Een item zonder bovenliggend item wordt weergegeven als deel van het eindtotaal.

**Getalnotatie** neemt een notatiecode, zoals `0,00`, `#.##0` of `0,0%`. Laat het leeg om de notatie van de cellen te behouden. Een waarde die als percentage wordt weergegeven, krijgt `0,00%` tenzij u een andere notatie kiest. De cellen van het rapport krijgen de notatie meteen. **Ongedaan maken** neemt die terug samen met de getallen.

## Veldinstellingen

Selecteer een veld in het vak **Filters**, **Kolommen** of **Rijen** en kies **Veldinstellingen** op het tabblad **Draaitabel** (of dubbelklik erop).

- **Aangepaste naam** wijzigt de naam die het rapport voor het veld toont.
- **Subtotalen**: **Automatisch** toont een totaal voor elke groep van het veld, **Geen** toont er geen.
- **Itemlabels weergeven in**: compacte, overzichts- of tabelvorm, zoals bij Rapportindeling.
- **Na elk item een lege regel invoegen** zet een lege regel na elke groep.
- **Itemlabels herhalen** toont de naam van een buitenste item op elke regel van zijn groep (overzichts- en tabelvorm).
- **Sorteren**: houd de volgorde van de items aan (handmatig), of sorteer van A naar Z of van Z naar A op de labels. **Van klein naar groot op een waarde** en **Van groot naar klein op een waarde** sorteren de items op de getallen in een waardeveld. Het rapport behoudt de sortering wanneer de gegevens veranderen.
- **Filter** laat items weg op hun labels (begint met, eindigt op, bevat, is gelijk aan) of op een waarde (groter dan, kleiner dan, tussen, enzovoort), of houdt alleen de bovenste of onderste paar items over op basis van een waarde. Het filter wordt berekend wanneer u op OK kiest. Het verbergt de items die het weglaat; de vakjes hieronder laten zien welke dat zijn, en u kunt een item weer zichtbaar maken door het aan te vinken.
- **Deze items weergeven** is een lijst van de items van het veld met een vakje bij elk. Haal een vinkje weg om het item te verbergen; de totalen laten het dan weg.

## Datums en getallen groeperen

Selecteer een veld met datums of getallen in **Rijen** of **Kolommen** en kies **Veld groeperen** op het tabblad **Draaitabel**. Groepen worden uit de gegevens gemaakt, dus de kolom moet alleen getallen of alleen datums bevatten, zonder tekst en zonder lege cellen.

- **Datums**: vink aan waarop u wilt groeperen: **Seconden**, **Minuten**, **Uren**, **Dagen**, **Maanden**, **Kwartalen** of **Jaren**. Het fijnste deel dat u aanvinkt, blijft op het veld staan. Voor elk grover deel voegt JeffCrunch een veld toe, genoemd naar dat deel (bijvoorbeeld **Jaren**), en zet het vóór het datumveld op dezelfde plek. **Beginnen bij** en **Eindigen bij** zijn de eerste en de laatste datum; laat ze staan om de vroegste en de laatste datum uit de gegevens te gebruiken. Datums vóór het begin of na het einde gaan in twee extra groepen, die beginnen met `<` en `>`.
- **Getallen**: **Beginnen bij**, **Eindigen bij** en **Met stappen van** maken groepen zoals 1-10, 11-20, enzovoort.

**Groepering opheffen** haalt de groepen weg en de velden die ervoor zijn toegevoegd. Vernieuwen maakt de groepen opnieuw op basis van de nieuwe gegevens. Wanneer twee draaitabellen uit dezelfde gegevens zijn gemaakt, kunnen er geen groepen worden gemaakt, omdat een groep een deel van de gegevens is die ze delen.

## Berekende velden

Een berekend veld is een veld dat u maakt met een formule. Het gebruikt andere velden van de lijst. Een bonus van een tiende van het bedrag is bijvoorbeeld `=Bedrag*0,1`.

Kies **Berekend veld** op het tabblad **Draaitabel**. Typ een naam en een formule. Schrijf de naam van een veld in de formule, of dubbelklik erop in de lijst **Velden** om hem erin te zetten. Zet een naam met spaties tussen enkele aanhalingstekens, zoals `='Eenheidsprijs'*Aantal`. U kunt getallen gebruiken, `+ - * / ^ %`, vergelijkingen en een paar functies zoals `ALS`, `AFRONDEN`, `SOM`, `MIN`, `MAX`, `GEMIDDELDE` en `ABS`.

Kies **Toevoegen**. Het nieuwe veld wordt in het vak **Waarden** gezet. Het verschijnt ook in de veldenlijst, waar u het net als elk ander veld kunt aanvinken of uitvinken. Het kan niet in de rijen, kolommen of filters.

Een berekend veld wordt in elke cel van het rapport berekend. De naam van een veld staat voor het totaal van dat veld in de cel, dus `=Bedrag*Bedrag` kwadrateert het totaal van de cel. Het kwadrateert niet elke rij en telt die dan op. Een eindtotaal gebruikt de totalen van de hele lijst.

**Lijstformules** (naast **Berekend veld**) maakt een nieuw blad dat elk berekend veld en zijn formule vermeldt, en elk berekend item met zijn formule, zijn veld en zijn berekeningsvolgorde. **Ongedaan maken** verwijdert de cellen, en een nieuwe **Ongedaan maken** verwijdert het blad.

Om een veld te wijzigen, kiest u de naam ervan in het vak **Naam**. De formule verschijnt. Wijzig hem en kies **Wijzigen**. **Verwijderen** haalt het veld en zijn waarde weg. Elke handeling is één stap van **Ongedaan maken**. Een berekend veld wordt met het bestand opgeslagen en blijft behouden wanneer het rapport wordt vernieuwd. Twee draaitabellen die uit dezelfde gegevens zijn gemaakt, kunnen er geen hebben, omdat een berekend veld een deel is van de gegevens die ze delen.

## Berekende items

Een berekend item is een item dat u maakt uit andere items van hetzelfde veld. Het veld Regio heeft bijvoorbeeld de items Oost en West. Een nieuw item met de naam Beide kan `=Oost+West` zijn.

Klik op een cel in de draaitabel en kies **Berekend item** op het tabblad **Draaitabel**. Kies het **Veld**. Het moet in de rijen of de kolommen staan. Typ een **Naam** en een **Formule**. Schrijf de naam van een item in de formule, of dubbelklik erop in de lijst **Items** om hem erin te zetten. Zet een naam met spaties tussen enkele aanhalingstekens, zoals `='Groot Oost'+West`. U kunt getallen gebruiken, `+ - * / ^ %`, en haakjes.

Het nieuwe item krijgt een regel in het rapport. In elke kolom toont het de formule, berekend uit de regels van de items die erin worden genoemd. Het nieuwe item telt mee in de totalen. Een totaal van sommen, zoals een eindtotaal of een subtotaal, telt de regel van het nieuwe item op bij de regels van de andere items. Met Oost op 35, West op 14 en Beide (Oost plus West) op 49, is het eindtotaal dus 98. Als een item dat in de formule staat verborgen is, toont de cel **#VERW!**. Als de formule een item noemt dat er niet is, toont de cel **#NAAM?**.

Om een item te wijzigen, kiest u de naam ervan in het vak **Naam**. De formule verschijnt. Wijzig hem en kies **Wijzigen**. **Verwijderen** haalt het item en zijn regel weg. Elke handeling is één stap van **Ongedaan maken**. Een berekend item wordt met het bestand opgeslagen en blijft behouden wanneer het rapport wordt vernieuwd. Twee draaitabellen die uit dezelfde gegevens zijn gemaakt, kunnen er geen hebben, omdat een berekend item een deel is van de gegevens die ze delen.

### Berekeningsvolgorde

Soms heeft één cel twee berekende items. Het ene staat in zijn rij en het andere in zijn kolom. De volgorde waarin ze worden berekend, kan het antwoord veranderen. Stel dat het rijitem `=Oost-West` is en het kolomitem `=Jan+1`. Berekend in de ene volgorde, is de cel (Oost - West) + 1. In de andere volgorde is hij (Oost + 1) - (West + 1), wat een ander getal is.

**Berekeningsvolgorde** op het tabblad **Draaitabel** toont de berekende items in de volgorde waarin ze worden berekend. Het item lager in de lijst wordt als laatste berekend. Kies een item en klik op **Omhoog verplaatsen** of **Omlaag verplaatsen**. De volgorde wordt met het bestand opgeslagen als de volgorde van de items, en andere programma's lezen hem op dezelfde manier. **Lijstformules** toont de plaats van elk item erin.

## De rijen achter een waarde tonen

Dubbelklik op een waarde van een draaitabel, of kies **Details weergeven** op het tabblad **Draaitabel**. JeffCrunch maakt een nieuw blad, genoemd Detail 1, 2 enzovoort, met de rijen van de lijst die het getal vormen. De namen van de velden staan in de eerste rij. F2 bewerkt de cel nog steeds, als de cel kan worden bewerkt. **Ongedaan maken** verwijdert de rijen, en een nieuwe **Ongedaan maken** verwijdert het blad.

## Uitvouwen en samenvouwen

Een veld waarvan de items andere items eronder hebben, kan de regels onder zijn items tonen of verbergen. **Volledig veld uitvouwen** en **Volledig veld samenvouwen** doen dat voor elk item van het veld dat u in het deelvenster selecteert.

## Opties voor draaitabel

**Opties** op het tabblad **Draaitabel** opent een vak met de keuzes van de tabel.

- **Naam** is de naam van de draaitabel. Geen andere draaitabel van de werkmap mag die naam hebben.
- **Voor foutwaarden tonen** en **Voor lege cellen tonen** zetten een eigen tekst in die cellen.
- **Cellen met labels samenvoegen en centreren**, **Kolombreedten aanpassen wanneer de tabel wordt bijgewerkt** en **Opmaak van cellen behouden wanneer de tabel wordt bijgewerkt** worden met de tabel opgeslagen.
- **Eindtotalen voor rijen tonen** en **voor kolommen** zijn dezelfde als de keuzes voor Eindtotalen.
- **Gegevens vernieuwen wanneer het bestand wordt geopend** leest de cellen elke keer opnieuw wanneer de werkmap wordt geopend. Het is één stap van Ongedaan maken, zoals elke vernieuwing.

## Stijl: koppen, banden en de galerie

De **Stijlopties** op het tabblad **Draaitabel** schakelen de kopregel, de eerste kolom (rijkoppen) en de banden van rijen en van kolommen in of uit. **Stijlen** heeft een galerie met uitstraling, en **Stijl wissen** haalt de uitstraling weg. Deze uitstralingen worden op dezelfde manier getekend als die van tabellen. **Lege rijen** en **Labels herhalen** stellen de lege regel en de herhaalde labels in voor elk veld van de rijen tegelijk.

## Ontwerp: subtotalen, eindtotalen en indeling

- **Subtotalen** toont of verbergt het totaal van elke groep, boven of onder de groep. Wanneer er meer dan één veld in Rijen staat, is het totaal van elk buitenste veld een subtotaal.
- **Eindtotalen** heeft vier keuzes: uit voor rijen en kolommen, aan voor beide, alleen aan voor rijen (een kolom met totalen rechts, één voor elke rij), en alleen aan voor kolommen (een rij met totalen onderaan).
- **Rapportindeling** is de vorm van de labels. In **compacte** vorm delen alle velden van Rijen één kolom, en een nieuwe groep wordt een beetje naar rechts getoond. In **overzichtsvorm** heeft elk veld een eigen kolom, en elke groep heeft een eigen regel boven zijn items. In **tabelvorm** heeft elk veld een eigen kolom en staat de naam van de groep op de eerste regel, met het subtotaal onder de groep.

## Dezelfde lijst, een filter

Verplaats **Maand** van **Kolommen** naar **Filters**. Het rapport verliest zijn kolommen, en er verschijnt een regel erboven met **Maand** en **(Alles)**. De getallen zijn nu het totaal van elke regio over alle maanden.

## Draaigrafiek

Klik op een cel in een draaitabel en kies **Invoegen**, en daarna **Draaigrafiek**. JeffCrunch maakt een kolomdiagram van de waarden van de tabel. Het laat de eindtotalen weg. De grafiek wordt in het bestand gemarkeerd als de grafiek van de draaitabel. Hij leest de cellen van de tabel, dus zijn getallen veranderen wanneer u de tabel vernieuwt of de indeling ervan wijzigt. Komen er meer rijen of kolommen bij in de tabel, maak de grafiek dan opnieuw, omdat de grafiek de cellen behoudt waarmee hij is gemaakt. Een draaigrafiek die uit een bestand komt, wordt getoond zoals hij is opgeslagen.

## Segmenteringen en tijdlijnen

Een segmentering is een reeks knoppen die een draaitabel filtert, en een tijdlijn is een schuifregelaar die hetzelfde voor datums doet.

Klik op een cel in een draaitabel en klik daarna op **Segmenteringen** op het tabblad Draaitabel. Het deelvenster Segmenteringen wordt geopend en somt de segmenteringen en tijdlijnen van de werkmap op. Een segmentering heeft één knop voor elk item van zijn veld. De blauwe knoppen zijn de items die de tabel toont.

Klik op een knop om alleen dat item te tonen. Klik er nogmaals op om alle items weer te tonen. Houd **Ctrl** ingedrukt en klik om een item aan de getoonde items toe te voegen, of om er één weg te halen. Elke klik is één stap van Ongedaan maken, en Ongedaan maken zet de tabel terug. Wanneer u opslaat, bewaart het bestand de keuze van de segmentering.

**Rapportverbindingen** zegt welke draaitabellen een segmentering filtert. Een tijdlijn toont de periode die hij heeft gekozen. JeffCrunch toont de segmenteringen in dit deelvenster, niet over de cellen, en het wijzigt een tijdlijn nog niet. Beide worden in het bestand bewaard zoals ze zijn opgeslagen.

## Beperkingen

Een draaitabel kan tot 2.000 velden hebben, een lijst van maximaal vijf miljoen rijen en een rapport van maximaal vier miljoen cellen. Een bestand dat hier overheen gaat, wordt getoond zoals het is opgeslagen en kan niet worden gewijzigd. Het opnieuw inlezen van honderdduizend rijen met twintig kolommen en het berekenen van het rapport kostte ongeveer een halve seconde op de computer waarop dit is gemeten; een tragere computer doet er langer over.
