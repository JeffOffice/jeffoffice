# Draaitabellen
<!-- keywords: draaitabel, draaitabellen, pivottabel, draaitabel invoegen, maken, aanmaken, getpivotdata, draaitabel.ophalen, getpivotdata genereren, vernieuwen, alles vernieuwen, gegevensbron wijzigen, andere gegevensbron, lijst met velden, velden, rijen, kolommen, waarden, filters, rapportfilter, eindtotaal, subtotaal, indeling, compact, overzicht, tabelvorm, wissen, waardeveldinstellingen, som, aantal, gemiddelde -->

Een draaitabel is een klein rapport dat wordt gemaakt van een lange lijst. De lijst heeft bijvoorbeeld één rij voor elke verkoop; het rapport telt de verkopen op per regio en per maand. U schrijft geen formules. U geeft aan welke kolommen van de lijst langs de zijkant komen, welke bovenaan, en welke wordt opgeteld, en het rapport vult zichzelf in.

## Een voorbeeld

Hier staat een lijst in de cellen A1 tot en met C7. Elke rij is één verkoop.

| Regio | Maand | Bedrag |
|---|---|---|
| Oost | jan | 10 |
| Oost | jan | 5 |
| Oost | feb | 20 |
| West | jan | 7 |
| West | feb | 3 |
| West | feb | 4 |

Een draaitabel met **Regio** langs de zijkant, **Maand** bovenaan en de som van **Bedrag** in het midden ziet er zo uit.

| Som van Bedrag | Maand | | |
|---|---|---|---|
| Regio | jan | feb | Eindtotaal |
| Oost | 15 | 20 | 35 |
| West | 7 | 7 | 14 |
| Eindtotaal | 22 | 27 | 49 |

Oost in januari is 15, omdat de twee rijen van Oost voor januari, 10 en 5, worden opgeteld. De kolom Eindtotaal telt elke rij op, de rij Eindtotaal telt elke kolom op, en de cel in de hoek telt alles op.

De gebruikte begrippen:

- Een **veld** is een kolom van de lijst. Regio, Maand en Bedrag zijn de velden.
- **Rijen**, **Kolommen** en **Waarden** zijn de drie plaatsen waar een veld kan staan. Een veld in Rijen geeft één regel voor elke verschillende vermelding; een veld in Kolommen geeft één kolom voor elke vermelding; een veld in Waarden is het veld dat wordt opgeteld. Een vierde plaats, **Filters**, bevat een veld dat de rijen weglaat die u niet kiest.
- Een **cache** (de draaitabelcache) is een kopie van uw lijst die de draaitabel in het bestand bewaart, zodat de tabel totalen kan weergeven zonder naar de cellen te kijken. Het rapport wordt berekend op basis van de cache, niet van de cellen, en verandert dus niet als de cellen veranderen. Met **Vernieuwen** worden de cellen opnieuw naar de cache gekopieerd.

## Een draaitabel maken

1. Klik op een cel in de lijst of selecteer de cellen die u wilt gebruiken. Boven de gegevens van de lijst moet een rij met namen staan.
2. Kies **Invoegen** en dan **Draaitabel**.
3. Het vak toont de cellen waarin de gegevens staan. Wijzig ze als ze niet kloppen. U kunt ook de naam van een tabel typen.
4. Kies waar de draaitabel komt: **Een nieuw blad** (de standaard), of een cel op dit blad of een ander blad. Kies een plek waar verder niets staat, want de lege tabel beslaat ongeveer achttien rijen en drie kolommen.
5. Kies **OK**. De tabel is eerst leeg en het deelvenster **Draaitabelvelden** wordt geopend. Schakel de velden in die u wilt gebruiken: een veld met getallen wordt opgeteld en elk ander veld wordt een rij. Verplaats daarna velden naar de vakken zoals in de volgende secties staat beschreven.

Een nieuw blad is een aparte stap bij Ongedaan maken, en de tabel ook. Als u opslaat als .xlsx, wordt de tabel geschreven zoals het oorspronkelijke programma dat doet, zodat hij daar als draaitabel wordt geopend. Een kopie die is opgeslagen als .xls of .ods behoudt de getallen die de tabel in de cellen toont, maar niet de tabel zelf; dat wordt bij het opslaan gemeld.

## GETPIVOTDATA: één getal uit een draaitabel lezen

`GETPIVOTDATA` geeft het getal dat een draaitabel toont voor de waarden en de rijen en kolommen die u opgeeft. Voor het voorbeeld hierboven:

`=GETPIVOTDATA("Som van Bedrag", $E$3, "Regio", "Oost", "Maand", "jan")` geeft 15.

- Het eerste argument is de naam van de waarde, zoals de tabel die toont (bijvoorbeeld Som van Bedrag). De naam van het veld dat wordt opgeteld (Bedrag) werkt ook.
- Het tweede argument is een willekeurige cel van de draaitabel.
- Daarna volgen paren van een veld en een van de items ervan. Geeft u alleen Regio, Oost op, dan krijgt u het totaal van Oost, 35. Geeft u niets op, dan krijgt u het eindtotaal, 49. De velden die u opgeeft, moeten de eerste velden in de rijen zijn, of de eerste velden in de kolommen, in volgorde.
- Als de tabel het getal niet toont, toont de cel `#REF!`.

Als u tijdens het typen van een formule op een waarde van een draaitabel klikt, schrijft JeffCrunch deze formule voor u, zodat de formule ook na het herschikken van de tabel het juiste getal leest. Wilt u dit uitschakelen, kies dan **Opties** op het tabblad **Draaitabel** en schakel het selectievakje voor GETPIVOTDATA uit.

## Wat JeffCrunch met een draaitabel doet

Een draaitabel in een werkmap wordt weergegeven zoals hij is opgeslagen. Als u er niets aan wijzigt, wordt het bestand opgeslagen met de draaitabel precies zoals hij was. Als u hem vernieuwt of wijzigt, berekent JeffCrunch het rapport opnieuw, zet de nieuwe getallen in de cellen en schrijft de draaitabel terug, zodat het programma waarmee hij is gemaakt hem zonder problemen opent.

JeffCrunch berekent draaitabellen die optellen, tellen, het gemiddelde berekenen enzovoort. Een draaitabel die datums of getallen groepeert, een berekend veld of item heeft, waarden weergeeft als percentage, verschil of rangorde, of een gegevensmodel gebruikt, wordt weergegeven en bewaard zoals hij is opgeslagen; de opdrachten melden dan dat ze hem niet kunnen wijzigen.

Klik op een willekeurige cel van een draaitabel. Het tabblad **Draaitabel** verschijnt.

## Vernieuwen

**Vernieuwen** leest de cellen waarop de draaitabel is gebaseerd en berekent het rapport opnieuw. Nieuwe vermeldingen komen na de bestaande; vermeldingen die verdwenen zijn, vervallen. Kolombreedten en de gekozen indeling blijven behouden. **Alles vernieuwen**, op het tabblad **Gegevens** en op het tabblad **Draaitabel**, doet hetzelfde voor elke draaitabel in de werkmap.

Als het rapport zou uitgroeien over cellen waarin iets staat, vernieuwt JeffCrunch niet. U ziet dan welke cellen in de weg staan. Wis ze of verplaats de draaitabel en vernieuw opnieuw.

Met **Ongedaan maken** draait u een vernieuwing in één stap terug.

## Andere gegevensbron

**Andere gegevensbron** vraagt naar de cellen waarin de lijst staat, inclusief de eerste rij met namen. Typ ze zoals `Blad1!A1:D100`, of typ de naam van een tabel of een naam die u hebt gemaakt. JeffCrunch leest ze en berekent het rapport opnieuw. Als een veld van de lijst in de nieuwe cellen dezelfde naam heeft, blijft de plaats ervan in het rapport behouden.

## Wissen

**Wissen** heeft twee opties. **Filters wissen** verwijdert alle filters en toont alle vermeldingen weer. **Alles wissen** haalt alle velden uit het rapport en laat het leeg achter, klaar om opnieuw op te bouwen.

## Het deelvenster met velden

**Lijst met velden** op het tabblad **Draaitabel** opent het deelvenster **Draaitabelvelden**. Het toont de velden van de lijst met naast elk veld een selectievakje, en daaronder de vier plaatsen: **Filters**, **Kolommen**, **Rijen** en **Waarden**.

- Schakel een veld in om het toe te voegen. Een veld met alleen getallen gaat naar **Waarden** en wordt opgeteld; elk ander veld gaat naar **Rijen**.
- Schakel een veld uit om het overal te verwijderen.
- Klik op een veld in een van de vier plaatsen en kies **Verplaatsen naar** om het naar een andere plaats te zetten. Met **Omhoog** en **Omlaag** wijzigt u de volgorde binnen die plaats, en met **Veld verwijderen** haalt u het weg.
- Bij meer dan één waarde verschijnt in **Kolommen** een regel met de naam **Waarden**. Verplaats die naar **Rijen** om de waarden langs de zijkant te zetten.
- **Waardeveldinstellingen** (dubbelklik op een waarde, of gebruik de knop) bepaalt de naam die het rapport voor de waarde toont en hoe het totaal wordt berekend: **Som**, **Aantal**, **Aantal getallen**, **Gemiddelde**, **Max**, **Min**, **Product**, **Stdev** en **Stdevp** (de standaarddeviatie van een steekproef of van alle waarden), en **Var** en **Varp** (de variantie daarvan).

Elke wijziging is één stap bij **Ongedaan maken**.

## Ontwerp: subtotalen, eindtotalen en indeling

- **Subtotalen** toont of verbergt het totaal van elke groep, boven of onder aan de groep. Als er meer dan één veld in Rijen staat, is het totaal van elk buitenste veld een subtotaal.
- **Eindtotalen** heeft vier opties: uit voor rijen en kolommen, aan voor beide, alleen aan voor rijen (een kolom met totalen rechts, één voor elke rij) en alleen aan voor kolommen (een rij met totalen onderaan).
- **Rapportindeling** bepaalt de vorm van de labels. In **compacte** vorm delen alle velden van Rijen één kolom, en wordt een nieuwe groep iets naar rechts weergegeven. In **overzichtsvorm** heeft elk veld een eigen kolom, en heeft elke groep een eigen regel boven de items. In **tabelvorm** heeft elk veld een eigen kolom en staat de naam van de groep op de eerste regel, met het subtotaal onder de groep.

## Dezelfde lijst, met een filter

Verplaats **Maand** van **Kolommen** naar **Filters**. Het rapport verliest zijn kolommen en een regel erboven toont **Maand** en **(Alles)**. De getallen zijn nu het totaal van elke regio over alle maanden.

## Beperkingen

Een draaitabel kan maximaal 2.000 velden hebben, een lijst van maximaal vijf miljoen rijen en een rapport van maximaal vier miljoen cellen. Een bestand dat deze grenzen overschrijdt, wordt weergegeven zoals het is opgeslagen en kan niet worden gewijzigd. Het opnieuw lezen van honderdduizend rijen van twintig kolommen en het berekenen van het rapport duurde ongeveer een halve seconde op de computer waarop dit is gemeten; op een tragere computer duurt het langer.
