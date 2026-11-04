# Grafieken, sparklines, afbeeldingen en vormen
<!-- keywords: grafiek, diagram, kolom, staaf, lijn, taart, vlak, spreiding, combinatie, legenda, titel, rasterlijnen, gegevenslabels, grafiekgegevens, grafiektype, weergave, indelingen, verplaatsen, formaat wijzigen, afbeelding, vorm, object, reeks, trendlijn, trend lijn, vergelijking, R-kwadraat, tweede as, sparkline, sparklines, datumas, win/verlies, minigrafiek, markeringen -->

Een grafiek is een beeld van getallen dat aan die getallen gekoppeld blijft: wanneer u een getal wijzigt, verandert de grafiek mee. Afbeeldingen en vormen liggen op dezelfde manier over de cellen, maar zijn niet aan getallen gekoppeld.

## Een grafiek maken

1. Selecteer de getallen die u in de grafiek wilt weergeven. Als de eerste rij namen bevat en de eerste kolom labels, selecteer die dan ook. JeffCrunch kijkt naar de eerste rij en kolom en gebruikt ze als namen en labels wanneer het woorden zijn (of datums, voor de labels).
2. Kies op het tabblad **Invoegen** in **Grafieken** een soort: **Kolom**, **Staaf**, **Lijn**, **Taart**, **Vlak**, **Spreiding** of **Combinatie**. De kleine pijl naast een soort toont de varianten, zoals gestapelde kolommen, een lijn met markeringen, of een spreidingsdiagram met punten, verbonden punten of vloeiende lijnen.
3. Wilt u eerst alle soorten bekijken, kies dan **Meer grafieken**. De lijst links toont elke soort, getekend met uw cellen. Met **Rijen en kolommen omwisselen** draait u de reeksen om.

Als u slechts één cel selecteert, maakt JeffCrunch een grafiek van het cellenblok rond die cel dat gegevens bevat.

Reeksen lopen langs de kolommen wanneer de selectie minstens zoveel rijen heeft als kolommen, en langs de rijen wanneer er meer kolommen dan rijen zijn. Een taartdiagram heeft één reeks. Een spreidingsdiagram neemt de horizontale waarden uit de eerste kolom.

Voorbeeld: u selecteert A1:C4, waarin A2:A4 Q1, Q2 en Q3 bevatten, B1 en C1 Oost en West bevatten, en de rest getallen zijn. De grafiek heeft twee reeksen, Oost en West, en drie groepen kolommen, Q1 tot en met Q3.

## Verplaatsen en formaat wijzigen

Klik op een grafiek, afbeelding of vorm om die te selecteren. Sleep het object om het te verplaatsen, of sleep een van de acht vierkantjes aan de rand om het formaat te wijzigen.

- Houd **Alt** ingedrukt terwijl u sleept om een rand uit te lijnen met de rand van een cel.
- Houd **Shift** ingedrukt terwijl u een hoek sleept om de verhoudingen te behouden.
- Met de pijltoetsen verplaatst u het object enkele pixels tegelijk. **Ctrl** met een pijltoets verplaatst het één pixel, **Shift** met een pijltoets maakt het groter of kleiner, en **Alt** met een pijltoets gaat naar de volgende rand van een cel.
- **Esc** terwijl u sleept zet het terug op de oorspronkelijke plaats.

Elke verplaatsing of wijziging van het formaat is één stap die u ongedaan kunt maken. Klik met de rechtermuisknop op het object en kies **Eigenschappen** om de grootte te typen, en om te kiezen wat er gebeurt wanneer u cellen invoegt, verwijdert of het formaat ervan wijzigt: verplaatsen en het formaat aanpassen met de cellen, meeverplaatsen maar de grootte behouden, of op zijn plaats blijven. Deze keuze wordt in het bestand opgeslagen. **Dit object afdrukken** in hetzelfde vak schakelt het object uit op de afgedrukte pagina's (het blijft wel op het scherm zichtbaar); ook die keuze wordt in het bestand bewaard.

## Wijzigen wat een grafiek toont

Wanneer een grafiek is geselecteerd, verschijnen er twee knoppen naast en verschijnt het tabblad **Grafiek** op het lint.

- De knop **+** (en **Grafiekonderdelen** op het tabblad) zet de titel, assen, astitels, rasterlijnen, legenda en gegevenslabels van de grafiek aan of uit, en bepaalt waar de legenda en de labels komen.
- De kwastknop (en **Kleuren** en **Stijlen** op het tabblad) geeft de grafiek een van de kleurenschema's en een van de stijlen van JeffCrunch. Met **Indelingen** stelt u meerdere onderdelen tegelijk in.
- **Grafiekgegevens** toont de reeksen van de grafiek. U kunt ze toevoegen, bewerken, verwijderen en opnieuw ordenen, de cellen wijzigen die elke reeks leest, de cellen met de labels instellen, rijen en kolommen omwisselen, en kiezen hoe lege cellen en verborgen rijen en kolommen in de grafiek worden weergegeven.
- **Trendlijn** (in het menu **+**) tekent een lijn die op de getallen van elke reeks is aangepast, zodat u ziet waar ze naartoe gaan. Kies **Lineair** voor een rechte lijn, **Exponentieel** voor getallen die met een vast percentage groeien, **Logaritmisch** voor getallen die snel stijgen en daarna vlakker worden, **Polynomiaal** voor een kromme met bochten, **Macht**, of **Voortschrijdend gemiddelde**, dat het gemiddelde van de laatste paar getallen tekent. Met **Vergelijking weergeven** zet u de formule van de lijn in de grafiek, en met **R-kwadraat weergeven** zet u erin hoe goed de lijn past (1 is een perfecte aansluiting; 0 is helemaal geen aansluiting). De trendlijn krijgt een eigen regel in de legenda.
- Wanneer een reeks is getekend tegen een tweede verticale as, hebben de menu's **Assen** en **Astitels** de opties **Tweede horizontaal** en **Tweede verticaal** voor die reeks. Het menu **Legenda** heeft **Over de grafiek**, waarmee de legenda over het diagram wordt geplaatst in plaats van ernaast.
- **Gegevenslabels** heeft ook **Label bevat** (de waarde, de naam van de categorie, de naam van de reeks en het percentage, in elke combinatie), **Scheidingsteken** (wat ertussen komt: een komma, puntkomma, punt, nieuwe regel of spatie) en **Getalnotatie…** (een notatiecode zoals 0,0 voor de getallen in de labels; laat dit leeg om de getallen zoals ze zijn te tonen). Schakelt u alle inhoud uit, dan verdwijnen de labels.
- **Horizontale as opmaken…** en **Verticale as opmaken…** op het tabblad **Grafiek** openen een vak voor één as. Hiermee stelt u de laagste en hoogste waarden in, en de hoofd- en subintervallen (elk automatisch of een getal dat u typt), of de waarden in omgekeerde volgorde lopen, waar de andere as deze snijdt, de notatie van de labels, de schaalstreepjes, waar de labels staan en hoe ver de labels zijn gedraaid. Alleen een getalas heeft grenzen en intervallen.
- **Grafiektype** wijzigt het type voor de hele grafiek of voor elke reeks afzonderlijk: kolommen met een lijn erover, of een reeks tegen een tweede verticale as.

Een verwijzing naar cellen wordt geschreven als **Blad1!$B$2:$B$9**, waarbij de naam van het blad tussen enkele aanhalingstekens staat wanneer die spaties bevat.

## Sparklines

Een sparkline is een minigrafiek die in één cel past. Hij toont de trend van een rij of kolom met getallen, direct naast die getallen.

1. Selecteer de getallen. Wilt u één sparkline per rij, selecteer dan meerdere rijen met getallen.
2. Kies op het tabblad **Invoegen** in **Sparklines** de optie **Lijn**, **Kolom** of **Win/verlies**. Een **Lijn** verbindt de getallen met een lijn. Een **Kolom** tekent een staafje voor elk getal. Een **Win/verlies** tekent een staafje omhoog voor elk getal boven nul en een staafje omlaag voor elk getal eronder, allemaal even groot; dat past bij resultaten die alleen goed of slecht zijn.
3. Controleer in het vak **Sparklines maken** het **Gegevensbereik** (de cellen die de sparklines lezen) en typ het **Locatiebereik** (de cellen waarin ze komen). Kies één locatiecel voor elke rij gegevens, of voor elke kolom gegevens. Kiest u één cel, dan leest de sparkline alle gegevens.

De sparklines die u in één stap maakt, vormen een groep en delen één uiterlijk. Klik op een cel met een sparkline om het tabblad **Sparkline** te zien.

- **Gegevens bewerken** wijzigt de gegevens en de locatie van de hele groep, of alleen de gegevens van de sparkline waarop u klikte. Ook bepaalt u wat er met lege cellen gebeurt (een lege ruimte laten, als nul behandelen, of de punten erover heen verbinden) en of cellen in verborgen rijen en kolommen worden gelezen.
- **Type** wisselt tussen lijn, kolom en win/verlies.
- **Weergeven** markeert punten: het **Hoogste punt**, het **Laagste punt**, **Negatieve punten**, het **Eerste punt**, het **Laatste punt** en, bij een lijn, **Markeringen** op elk punt.
- **Stijlen** geeft de groep een kleurenschema. **Sparklinekleur** stelt de kleur van de lijn of de staafjes in en de **Dikte** (hoe dik de lijn is, in punten). **Markeringskleur** stelt de kleur in van elk soort gemarkeerd punt.
- **As** toont een lijn bij nul, zet de getallen van rechts naar links uit, en stelt de laagste en hoogste waarden van de verticale as in. **Automatisch voor elke sparkline** past elke sparkline aan op zijn eigen getallen. **Hetzelfde voor alle sparklines** geeft de hele groep één schaal, zodat u ze kunt vergelijken. **Aangepaste waarde** laat u een getal typen. Met **Datumas…** plaatst u de punten op datum: typ de cellen met de datum van elk punt, en een punt met een latere datum staat verder naar rechts. Laat dit leeg om de punten weer gelijkmatig te verdelen.
- **Groeperen** zet de geselecteerde sparklines in één groep. **Groep opheffen** geeft elke sparkline een eigen uiterlijk. **Wissen** verwijdert de geselecteerde sparklines of hun hele groepen.

Een sparkline volgt zijn cellen: voegt u een rij boven de gegevens in, dan leest hij nog steeds dezelfde getallen. Elke wijziging is één stap die u ongedaan kunt maken. Sparklines worden in het bestand opgeslagen in een vorm die andere spreadsheetprogramma's lezen.

## Een object knippen, kopiëren en plakken

Een afbeelding of vorm wordt op dezelfde manier gekopieerd en geplakt als een grafiek. Een vorm die iets anders gebruikt dan een afbeelding uit de werkmap, zoals een koppeling naar een bestand, kan niet worden gekopieerd, en JeffCrunch laat u weten waarom.

Kopieert u een grafiek en plakt u die in dezelfde werkmap, op een willekeurig blad, dan leest de kopie dezelfde cellen als het origineel. Plakt u de grafiek in een andere werkmap, dan behoudt hij de getallen die hij toonde, maar leest hij geen cellen meer, omdat de cellen die hij daar zou lezen niet dezelfde zijn als die waarvan hij is gemaakt.

## Een naam geven

Wanneer een object is geselecteerd, toont het naamvak de naam ervan. Typ daar een nieuwe naam en druk op **Enter**. Ongedaan maken zet de oude naam terug, en de nieuwe naam wordt in het bestand opgeslagen.

## Verwijderen

Druk op **Delete** terwijl het object is geselecteerd. Ongedaan maken haalt het terug.

## Wat wordt opgeslagen

Alles hierboven wordt in het bestand opgeslagen in een vorm die andere spreadsheetprogramma's lezen, zodat een grafiek die u hier maakt daar op dezelfde manier wordt geopend. Een grafiek die uit een ander programma komt, met iets wat JeffCrunch niet kan behouden wanneer het een grafiek opnieuw schrijft (foutbalken, een driedimensionale grafiek), kan wel worden verplaatst, van formaat worden veranderd en worden verwijderd, maar de onderdelen en gegevens ervan worden hier niet gewijzigd.
