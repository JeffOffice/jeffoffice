# Grafieken, sparklines, afbeeldingen en vormen
<!-- keywords: grafiek, diagram, kolom, staaf, lijn, cirkel, vlak, spreiding, combinatie, legenda, titel, rasterlijnen, gegevenslabels, grafiekgegevens, grafiektype, weergave, indelingen, verplaatsen, formaat wijzigen, afbeelding, vorm, object, reeks, trendlijn, trend lijn, vergelijking, R-kwadraat, tweede as, sparkline, sparklines, datumas, winst/verlies, minigrafiek, markeringen, naam, alternatieve tekst, logaritmisch, logaritmische schaal, opslaan als afbeelding, sjabloon, crtx, titeltekst, legenda-sleutel -->

Een grafiek is een beeld van getallen dat aan die getallen gekoppeld blijft: wijzigt u een getal, dan verandert de grafiek mee. Afbeeldingen en vormen liggen op dezelfde manier over de cellen, maar ze zijn niet aan getallen gekoppeld.

## Een grafiek maken

1. Selecteer de getallen die u in een grafiek wilt weergeven. Staan er namen in de eerste rij en labels in de eerste kolom, selecteer die dan ook. JeffCrunch kijkt naar de eerste rij en kolom en gebruikt die als namen en labels wanneer het woorden zijn (of datums, voor de labels).
2. Kies op het tabblad **Invoegen**, in de groep **Grafieken**, een soort: **Kolom**, **Staaf**, **Lijn**, **Cirkel**, **Vlak**, **Spreiding** of **Combinatie**. Met de kleine pijl naast een soort kiest u de varianten, zoals gestapelde kolommen, een lijn met markeringen, of een spreidingsgrafiek met punten, met verbonden punten of met vloeiende lijnen.
3. Wilt u eerst alle soorten bekijken, kies dan **Meer grafieken**. De lijst links toont elke soort, getekend met uw cellen. Met **Rijen en kolommen omwisselen** draait u de reeksen om.

Selecteert u maar één cel, dan maakt JeffCrunch een grafiek van het cellenblok eromheen dat gegevens bevat.

Reeksen lopen langs de kolommen wanneer de selectie minstens zoveel rijen als kolommen heeft, en langs de rijen wanneer er meer kolommen dan rijen zijn. Een cirkel bevat één reeks. Een spreidingsgrafiek neemt de horizontale waarden uit de eerste kolom.

Voorbeeld: u selecteert A1:C4, waarbij A2:A4 Q1, Q2 en Q3 bevatten, B1 en C1 Oost en West bevatten, en de rest getallen zijn. De grafiek heeft twee reeksen, Oost en West, en drie groepen kolommen, Q1 tot en met Q3.

## Verplaatsen en formaat wijzigen

Klik op een grafiek, afbeelding of vorm om die te selecteren. Sleep om te verplaatsen, of sleep een van de acht vierkantjes op de rand om het formaat te wijzigen.

- Houd **Alt** ingedrukt terwijl u sleept om een rand uit te lijnen op de rand van een cel.
- Houd **Shift** ingedrukt terwijl u een hoek sleept om de verhoudingen te behouden.
- Met de pijltoetsen verplaatst u het object een paar pixels tegelijk. **Ctrl** met een pijltoets verplaatst het één pixel, **Shift** met een pijltoets maakt het groter of kleiner, en **Alt** met een pijltoets gaat naar de volgende rand van een cel.
- **Esc** terwijl u sleept, zet het object terug.

Elke verplaatsing of formaatwijziging is één stap die u ongedaan kunt maken. Klik met de rechtermuisknop op het object en kies **Eigenschappen** om de **Naam** in te typen (elk object op een blad moet een eigen naam hebben) en de **Alternatieve tekst** (een korte zin die voor iemand die het object niet kan zien, beschrijft wat het toont), om de grootte in te typen en om te kiezen wat het object doet wanneer u cellen invoegt, verwijdert of het formaat ervan wijzigt: meeverplaatsen en meeschalen met de cellen, meeverplaatsen maar de grootte behouden, of op zijn plaats blijven. De keuze wordt in het bestand opgeslagen. **Dit object afdrukken** in hetzelfde vak zet het object uit op de afgedrukte pagina's (op het scherm blijft het zichtbaar); het bestand bewaart die keuze ook.

## Wijzigen wat een grafiek toont

Wanneer een grafiek is geselecteerd, verschijnen er twee knoppen naast en verschijnt het tabblad **Grafiek** op het lint.

- De knop **+** (en **Grafiekonderdelen** op het tabblad) zet de titel, assen, astitels, rasterlijnen, legenda en gegevenslabels van de grafiek aan of uit, en bepaalt waar de legenda en de labels komen.
- De kwastknop (en **Kleuren** en **Stijlen** op het tabblad) geeft de grafiek een van de kleurenschema's en een van de stijlen van JeffCrunch. **Indelingen** stelt meerdere onderdelen tegelijk in.
- **Grafiekgegevens** toont de reeksen van de grafiek. U kunt ze toevoegen, bewerken, verwijderen en opnieuw ordenen, de cellen wijzigen die elke reeks leest, de cellen met de labels instellen, rijen en kolommen omwisselen, en kiezen hoe lege cellen en verborgen rijen en kolommen in de grafiek worden weergegeven.
- **Trendlijn** (in het menu **+**) tekent een lijn die op de getallen van elke reeks is gepast, zodat u ziet waar ze naartoe gaan. Kies **Lineair** voor een rechte lijn, **Exponentieel** voor getallen die met een vast percentage groeien, **Logaritmisch** voor getallen die snel stijgen en daarna afvlakken, **Polynoom** voor een kromme met bochten, **Macht**, of **Voortschrijdend gemiddelde**, dat het gemiddelde van de laatste enkele getallen tekent. **Vergelijking weergeven** schrijft de formule van de lijn op de grafiek, en **R-kwadraat weergeven** schrijft hoe goed de lijn past (1 is een perfecte pasvorm; 0 is helemaal geen pasvorm). De trendlijn krijgt een eigen item in de legenda.
- Wanneer een reeks tegen een tweede verticale as wordt getekend, hebben de menu's **Assen** en **Astitels** de items **Tweede horizontaal** en **Tweede verticaal** voor die reeks. Het menu **Legenda** heeft **Over de grafiek**, waarmee de legenda over het diagram wordt gelegd in plaats van ernaast.
- **Gegevenslabels** heeft ook **Labels bevatten** (de waarde, de naam van de categorie, de naam van de reeks, het percentage en de legenda-sleutel, een klein vierkantje in de kleur van de reeks, in elke gewenste combinatie), **Scheidingsteken** (wat ertussen staat: een komma, puntkomma, punt, nieuwe regel of spatie) en **Getalnotatie…** (een notatiecode zoals 0,0 voor de getallen in de labels; laat het leeg om de getallen zoals ze zijn te tonen). Schakelt u alle inhoud uit, dan verdwijnen de labels.
- **Horizontale as opmaken…** en **Verticale as opmaken…** op het tabblad **Grafiek** openen een vak voor één as. U stelt er de laagste en hoogste waarde in, en de hoofd- en subintervallen (elk automatisch of een getal dat u typt), of de waarden in omgekeerde volgorde lopen, of de schaal logaritmisch is (elke streep is een grondtal maal de vorige, zoals 1, 10, 100 en 1000; dit past bij getallen die sterk in grootte verschillen en allemaal groter dan 0 moeten zijn), waar de andere as de as snijdt, de notatie van de labels, de schaalstreepjes, waar de labels staan en hoe ver de labels zijn gedraaid. Alleen een getalas heeft grenzen en intervallen.
- **Grafiek verplaatsen…** op het tabblad **Grafiek** vraagt om een ander werkblad en zet de grafiek daar, in dezelfde cellen. De grafiek leest nog steeds de cellen die hij las, en behoudt zijn grootte en zijn plaats op het blad, ook waar de rijen en kolommen hoger of breder zijn, en hij verplaatst en schaalt met de cellen daar net zo als voorheen. Verborgen bladen worden niet aangeboden. Het is één stap **Ongedaan maken**, ook nadat u hebt opgeslagen. Een grafiekblad (een blad dat alleen een grafiek is) kan nog niet worden gemaakt, en een grafiek die erop staat, kan niet worden verplaatst.
- **Grafiektitel** en **Astitels** (in het menu **+**) hebben ook **Titeltekst…**. Typ de tekst die u wilt. Of typ een gelijkteken en een cel, zoals **=Blad1!A1**, dan toont de titel wat die cel bevat. Wanneer de cel verandert, verandert ook de titel. Het bestand bewaart de koppeling.
- **Opslaan als afbeelding…** op het tabblad **Grafiek** (en in het menu met de rechtermuisknop) slaat de geselecteerde grafiek op als afbeeldingsbestand in de notatie **PNG** of **JPEG**, twee keer zo groot als op het blad, zodat hij scherp blijft.
- **Opslaan als sjabloon…** en **Sjabloon toepassen…** (op het tabblad **Grafiek**, in de groep **Type**) bewaren de stijl van een grafiek in een **.crtx**-bestand en geven die aan een andere grafiek. De stijl omvat het soort, de kleuren, de instellingen voor titel en legenda, de assen en de lettertypen. De grafiek waarop u de stijl toepast, behoudt zijn eigen getallen en de cellen die hij leest. Eén stap **Ongedaan maken** haalt de stijl weer weg.
- **Grafiektype** wijzigt het soort voor de hele grafiek, of voor elke reeks afzonderlijk: kolommen met een lijn erover, of een reeks tegen een tweede verticale as.

Een verwijzing naar cellen schrijft u als **Blad1!$B$2:$B$9**, waarbij de naam van het blad tussen enkele aanhalingstekens staat wanneer die spaties bevat.

## Sparklines

Een sparkline is een minigrafiek die binnen één cel past. Hij toont de trend van een rij of kolom met getallen, direct naast die getallen.

1. Selecteer de getallen. Wilt u één sparkline per rij, selecteer dan meerdere rijen met getallen.
2. Kies op het tabblad **Invoegen**, in de groep **Sparklines**, **Lijn**, **Kolom** of **Winst/verlies**. Een **lijn** verbindt de getallen met een lijn. Een **kolom** tekent een staaf voor elk getal. **Winst/verlies** tekent een staaf omhoog voor elk getal boven nul en een staaf omlaag voor elk getal eronder, allemaal even groot; dit past bij resultaten die alleen goed of slecht zijn.
3. Controleer in het vak **Sparklines maken** het **Gegevensbereik** (de cellen die de sparklines lezen) en typ het **Locatiebereik** (de cellen waarin ze komen). Kies één locatiecel voor elke rij met gegevens, of voor elke kolom met gegevens. Kiest u één cel, dan leest de sparkline alle gegevens.

De sparklines die u in één stap maakt, vormen een groep en delen één stijl. Klik op een cel met een sparkline en het tabblad **Sparkline** verschijnt.

- **Gegevens bewerken** wijzigt de gegevens en de locatie van de hele groep, of de gegevens van de sparkline waarop u klikte. Ook bepaalt het wat er met lege cellen gebeurt (een lege ruimte laten, ze als nul behandelen, of de punten erover verbinden) en of cellen in verborgen rijen en kolommen worden gelezen.
- **Type** schakelt tussen lijn, kolom en winst/verlies.
- **Weergeven** markeert punten: het **Hoogste punt**, het **Laagste punt**, **Negatieve punten**, het **Eerste punt**, het **Laatste punt** en, voor een lijn, **Markeringen** op elk punt.
- **Stijlen** geeft de groep een set kleuren. **Sparklinekleur** stelt de kleur van de lijn of staven in en de **Dikte** (hoe dik de lijn is, in punten). **Markeringskleur** stelt de kleur van elk soort gemarkeerd punt in.
- **As** toont een lijn bij nul, zet de getallen van rechts naar links uit, en stelt de laagste en hoogste waarde van de verticale as in. **Automatisch voor elke sparkline** past elke sparkline aan op zijn eigen getallen. **Hetzelfde voor alle sparklines** geeft de hele groep één schaal, zodat u ze kunt vergelijken. **Aangepaste waarde** laat u een getal typen. **Datumas…** spatieert de punten op basis van datums: typ de cellen met de datum van elk punt, en een punt met een latere datum staat verder naar rechts. Laat het leeg om de punten weer gelijkmatig te spatiëren.
- **Groeperen** zet de geselecteerde sparklines in één groep. **Groep opheffen** geeft elke sparkline een eigen stijl. **Wissen** verwijdert de geselecteerde sparklines of hun hele groepen.

Een sparkline volgt zijn cellen: voegt u een rij boven de gegevens in, dan leest hij nog steeds dezelfde getallen. Elke wijziging is één stap die u ongedaan kunt maken. Sparklines worden in het bestand opgeslagen in de vorm die andere spreadsheetprogramma's lezen.

## Een object knippen, kopiëren en plakken

Een afbeelding of vorm wordt op dezelfde manier gekopieerd en geplakt als een grafiek. Een vorm die iets anders gebruikt dan een afbeelding uit de werkmap, zoals een koppeling naar een bestand, kan niet worden gekopieerd; JeffCrunch vertelt u waarom.

Kopieert u een grafiek en plakt u die in dezelfde werkmap, op een willekeurig blad, dan leest de kopie dezelfde cellen als het origineel. Plakt u hem in een andere werkmap, dan behoudt hij de getallen die hij toonde, maar leest hij geen cellen meer, omdat de cellen die hij daar zou lezen niet de cellen zijn waarop hij is gebaseerd.

## Een object een naam geven

Wanneer een object is geselecteerd, toont het naamvak de naam ervan. Typ daar een nieuwe naam en druk op **Enter**. Ongedaan maken zet de oude naam terug, en de nieuwe naam wordt in het bestand opgeslagen.

## Een object verwijderen

Druk op **Delete** terwijl het object is geselecteerd. Ongedaan maken haalt het weer terug.

## Wat wordt opgeslagen

Alles hierboven wordt in het bestand opgeslagen in de vorm die andere spreadsheetprogramma's lezen, zodat een grafiek die u hier maakt, daar op dezelfde manier opent. Een grafiek die van een ander programma komt, met iets wat JeffCrunch niet kan behouden wanneer het een grafiek opnieuw schrijft (foutbalken, een driedimensionale grafiek), kan nog wel worden verplaatst, van formaat worden veranderd en verwijderd, maar de onderdelen en gegevens ervan worden hier niet gewijzigd.
