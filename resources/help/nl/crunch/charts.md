# Grafieken, sparklines, afbeeldingen en vormen
<!-- keywords: grafiek, diagram, kolom, staaf, lijn, cirkel, vlak, spreiding, combinatie, legenda, titel, rasterlijnen, gegevenslabels, grafiekgegevens, grafiektype, stijlen, indelingen, verplaatsen, formaat wijzigen, afbeelding, vorm, object, reeks, trendlijn, trend lijn, vergelijking, R-kwadraat, tweede as, sparkline, sparklines, datumas, winst/verlies, minigrafiek, markeringen -->

Een grafiek is een beeld van getallen dat aan die getallen gekoppeld blijft: wijzigt u een getal, dan verandert de grafiek mee. Afbeeldingen en vormen liggen op dezelfde manier over de cellen, maar ze zijn niet aan getallen gekoppeld.

## Een grafiek maken

1. Selecteer de getallen die u in de grafiek wilt tonen. Als de eerste rij namen bevat en de eerste kolom labels, selecteer die dan ook. JeffCrunch kijkt naar de eerste rij en kolom en gebruikt ze als namen en labels wanneer het woorden (of, voor de labels, datums) zijn.
2. Kies op het tabblad **Invoegen** in **Grafieken** een type: **Kolom**, **Staaf**, **Lijn**, **Cirkel**, **Vlak**, **Spreiding** of **Combinatie**. De kleine pijl naast een type toont de varianten, zoals gestapelde kolommen, een lijn met markeringen, of een spreidingsdiagram met punten, verbonden punten of vloeiende lijnen.
3. Wilt u eerst alle typen bekijken, kies dan **Meer grafieken**. De lijst links toont elk type met uw cellen. **Rijen en kolommen wisselen** keert de reeksen om.

Als u slechts één cel selecteert, neemt JeffCrunch het celbereik rond die cel dat gegevens bevat.

Reeksen lopen per kolom als de selectie minstens zoveel rijen als kolommen heeft, en per rij als er meer kolommen dan rijen zijn. Een cirkel heeft één reeks. Een spreidingsdiagram neemt de horizontale waarden uit de eerste kolom.

Voorbeeld: u selecteert A1:C4, waarbij A2:A4 Q1, Q2 en Q3 bevatten, B1 en C1 Oost en West bevatten, en de rest getallen zijn. De grafiek heeft twee reeksen, Oost en West, en drie groepen kolommen, Q1 tot en met Q3.

## Verplaatsen en formaat wijzigen

Klik op een grafiek, afbeelding of vorm om deze te selecteren. Sleep om te verplaatsen, of sleep een van de acht vierkantjes op de rand om het formaat te wijzigen.

- Houd **Alt** ingedrukt tijdens het slepen om een rand uit te lijnen op de rand van een cel.
- Houd **Shift** ingedrukt tijdens het slepen van een hoek om de verhoudingen te behouden.
- Met de pijltoetsen verplaatst u het object een paar pixels tegelijk. **Ctrl** met een pijltoets verplaatst het één pixel, **Shift** met een pijltoets maakt het groter of kleiner, en **Alt** met een pijltoets gaat naar de volgende rand van een cel.
- **Esc** tijdens het slepen zet het object terug.

Elke verplaatsing of wijziging van het formaat is één stap die u ongedaan kunt maken. Klik met de rechtermuisknop op het object en kies **Eigenschappen** om het formaat in te typen, en om te kiezen wat er gebeurt wanneer u cellen invoegt, verwijdert of het formaat ervan wijzigt: meeverplaatsen en meeschalen met de cellen, meeverplaatsen maar het formaat behouden, of op zijn plaats blijven. De keuze wordt in het bestand opgeslagen. **Dit object afdrukken** in hetzelfde vak schakelt het object uit op de afgedrukte pagina's (het blijft op het scherm zichtbaar); ook die keuze wordt in het bestand bewaard.

## Wat een grafiek toont wijzigen

Wanneer een grafiek is geselecteerd, verschijnen naast de grafiek twee knoppen en verschijnt op het lint het tabblad **Grafiek**.

- De knop **+** (en **Grafiekonderdelen** op het tabblad) zet de titel, assen, astitels, rasterlijnen, legenda en gegevenslabels van de grafiek aan of uit, en bepaalt waar de legenda en de labels komen.
- De kwastknop (en **Kleuren** en **Stijlen** op het tabblad) geeft de grafiek een van de kleurenset­ten en een van de stijlen van JeffCrunch. **Indelingen** stelt meerdere onderdelen tegelijk in.
- **Grafiekgegevens** toont de reeksen van de grafiek. U kunt ze toevoegen, bewerken, verwijderen en opnieuw ordenen, de cellen wijzigen die elke reeks leest, de cellen met de labels instellen, de rijen en kolommen wisselen, en kiezen hoe lege cellen en verborgen rijen en kolommen in de grafiek worden weergegeven.
- **Trendlijn** (in het menu **+**) tekent een lijn die op de getallen van elke reeks is aangepast, zodat u ziet waarheen ze gaan. Kies **Lineair** voor een rechte lijn, **Exponentieel** voor getallen die met een vast percentage groeien, **Logaritmisch** voor getallen die snel stijgen en daarna vlakker worden, **Polynoom** voor een kromme met bochten, **Macht**, of **Voortschrijdend gemiddelde**, dat het gemiddelde van de laatste paar getallen tekent. **Vergelijking weergeven** schrijft de formule van de lijn op de grafiek, en **R-kwadraat weergeven** schrijft hoe goed de lijn past (1 is een perfecte pasvorm; 0 is helemaal geen pasvorm). De trendlijn krijgt een eigen regel in de legenda.
- Wanneer een reeks tegen een tweede verticale as wordt getekend, hebben de menu's **Assen** en **Astitels** de opties **Tweede horizontale as** en **Tweede verticale as** voor die reeks. Het menu **Legenda** heeft **Over de grafiek**, waarmee de legenda over het diagram wordt gelegd in plaats van ernaast.
- **Gegevenslabels** heeft ook **Label bevat** (de waarde, de naam van de categorie, de naam van de reeks en het percentage, in elke combinatie), **Scheidingsteken** (wat ertussen staat: een komma, puntkomma, punt, nieuwe regel of spatie) en **Notatie getal…** (een notatiecode zoals 0,0 voor de getallen in de labels; laat dit leeg om de getallen te tonen zoals ze zijn). Schakelt u alle inhoud uit, dan verdwijnen de labels.
- **Horizontale as opmaken…** en **Verticale as opmaken…** op het tabblad **Grafiek** openen een vak voor één as. Hier stelt u de laagste en hoogste waarde en de hoofd- en subintervallen in (elk automatisch of een getal dat u typt), of de waarden in omgekeerde volgorde lopen, waar de andere as deze snijdt, de notatie van de labels, de schaalstreepjes, waar de labels staan en hoeveel ze worden gedraaid. Alleen een getalas heeft grenzen en intervallen.
- **Grafiek verplaatsen…** op het tabblad **Grafiek** vraagt om een ander werkblad en zet de grafiek daar, in dezelfde cellen. De grafiek leest nog steeds dezelfde cellen en behoudt zijn formaat en zijn plek op het blad, ook als de rijen en kolommen daar hoger of breder zijn, en hij verplaatst en schaalt mee met de cellen daar zoals voorheen. Verborgen bladen worden niet aangeboden. Het is één stap **Ongedaan maken**, ook nadat u hebt opgeslagen. Een grafiekblad (een blad dat alleen een grafiek is) kan nog niet worden gemaakt, en een grafiek die daarop staat, kan niet worden verplaatst.
- **Grafiektype** wijzigt het type voor de hele grafiek, of voor elke reeks afzonderlijk: kolommen met een lijn erover, of een reeks tegen een tweede verticale as.

Een verwijzing naar cellen wordt geschreven als **Blad1!$B$2:$B$9**, waarbij de naam van het blad tussen enkele aanhalingstekens staat als die spaties bevat.

## Sparklines

Een sparkline is een kleine grafiek die binnen één cel past. Hij toont de trend van een rij of kolom met getallen, direct naast die getallen.

1. Selecteer de getallen. Wilt u één sparkline per rij, selecteer dan meerdere rijen met getallen.
2. Kies op het tabblad **Invoegen** in **Sparklines** de optie **Lijn**, **Kolom** of **Winst/verlies**. Een **Lijn** verbindt de getallen met een lijn. Een **Kolom** tekent een staafje voor elk getal. Een **Winst/verlies** tekent een staafje omhoog voor elk getal boven nul en een staafje omlaag voor elk getal eronder, allemaal even groot; dat past bij resultaten die alleen goed of slecht zijn.
3. Controleer in het vak **Sparklines maken** het **Gegevensbereik** (de cellen die de sparklines lezen) en typ het **Locatiebereik** (de cellen waarin ze komen). Kies één locatiecel voor elke rij gegevens, of voor elke kolom gegevens. Kiest u één cel, dan leest de sparkline alle gegevens.

De sparklines die u in één keer maakt, vormen een groep en delen één stijl. Klik op een cel met een sparkline en het tabblad **Sparkline** verschijnt.

- **Gegevens bewerken** wijzigt de gegevens en de locatie van de hele groep, of alleen de gegevens van de sparkline waarop u klikte. Ook bepaalt u wat er met lege cellen gebeurt (een lege ruimte laten, ze als nul behandelen, of de punten erover heen verbinden) en of cellen in verborgen rijen en kolommen worden gelezen.
- **Type** schakelt tussen lijn, kolom en winst/verlies.
- **Weergeven** markeert punten: het **Hoogste punt**, het **Laagste punt**, **Negatieve punten**, het **Eerste punt**, het **Laatste punt** en, bij een lijn, **Markeringen** op elk punt.
- **Stijlen** geeft de groep een set kleuren. **Sparklinekleur** stelt de kleur van de lijn of de staven in, en de **Dikte** (hoe dik de lijn is, in punten). **Markeringskleur** stelt de kleur in van elk type gemarkeerd punt.
- **As** toont een lijn bij nul, zet de getallen van rechts naar links uit en stelt de laagste en hoogste waarde van de verticale as in. **Automatisch voor elke sparkline** past elke sparkline aan op zijn eigen getallen. **Hetzelfde voor alle sparklines** geeft de hele groep één schaal, zodat u ze kunt vergelijken. **Aangepaste waarde** laat u een getal typen. **Datumas…** plaatst de punten op basis van datums: typ de cellen met de datum van elk punt, en een punt met een latere datum komt verder naar rechts te staan. Laat dit leeg om de punten weer gelijkmatig te verdelen.
- **Groep** zet de geselecteerde sparklines in één groep. **Groep opheffen** geeft elke sparkline een eigen stijl. **Wissen** verwijdert de geselecteerde sparklines, of hun hele groepen.

Een sparkline volgt zijn cellen: voegt u een rij boven de gegevens in, dan leest de sparkline nog steeds dezelfde getallen. Elke wijziging is één stap die u ongedaan kunt maken. Sparklines worden in het bestand opgeslagen in een vorm die andere spreadsheetprogramma's lezen.

## Een object knippen, kopiëren en plakken

Een afbeelding of vorm wordt op dezelfde manier gekopieerd en geplakt als een grafiek. Een vorm die iets anders gebruikt dan een afbeelding uit de werkmap, zoals een koppeling naar een bestand, kan niet worden gekopieerd; JeffCrunch geeft aan waarom.

Kopieert u een grafiek en plakt u die in dezelfde werkmap, op een willekeurig blad, dan leest de kopie dezelfde cellen als het origineel. Plakt u hem in een andere werkmap, dan behoudt hij de getallen die hij toonde, maar leest hij geen cellen meer, omdat de cellen die hij daar zou lezen niet de cellen zijn waarop hij was gebaseerd.

## Een naam geven

Wanneer een object is geselecteerd, toont het naamvak de naam ervan. Typ daar een nieuwe naam en druk op **Enter**. Ongedaan maken zet de oude naam terug, en de nieuwe naam wordt in het bestand opgeslagen.

## Verwijderen

Druk op **Delete** terwijl het object is geselecteerd. Ongedaan maken haalt het terug.

## Wat wordt opgeslagen

Alles hierboven wordt in het bestand opgeslagen in een vorm die andere spreadsheetprogramma's lezen, zodat een grafiek die u hier maakt daar op dezelfde manier opent. Een grafiek die uit een ander programma komt met iets wat JeffCrunch niet kan behouden wanneer het een grafiek herschrijft (foutbalken, een driedimensionale grafiek), kan wel worden verplaatst, van formaat veranderd en verwijderd, maar de onderdelen en gegevens ervan worden hier niet gewijzigd.
