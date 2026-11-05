# Grafieken, sparklines, afbeeldingen en vormen
<!-- keywords: grafiekfilters, reeks verbergen, aanbevolen grafieken, grafiekvoorstellen, grafiek, diagram, kolom, staaf, lijn, cirkel, vlak, spreiding, combinatie, legenda, titel, rasterlijnen, gegevenslabels, grafiekgegevens, grafiektype, stijlen, indelingen, verplaatsen, formaat wijzigen, afbeelding, vorm, object, reeks, trendlijn, trend lijn, vergelijking, R-kwadraat, tweede as, sparkline, sparklines, datumas, winst/verlies, minigrafiek, markeringen, naam, alternatieve tekst, logaritmisch, logschaal, opslaan als afbeelding, sjabloon, crtx, titeltekst, legendasleutel, selectievenster, vormopvulling, vormomtrek, draaien, spiegelen, bijsnijden, tekst bewerken, afbeeldingsrand, afbeelding invoegen, vorm invoegen, tekstvak, vormen, pijl, ovaal, rechthoek, ster -->

Een grafiek is een afbeelding van getallen die aan die getallen gekoppeld blijft: past u een getal aan, dan past de grafiek zich aan. Afbeeldingen en vormen liggen op dezelfde manier over de cellen, maar ze zijn niet aan getallen gekoppeld.

## Een grafiek maken

1. Selecteer de getallen die u in de grafiek wilt tonen. Heeft de eerste rij namen en de eerste kolom labels, selecteer die dan ook. JeffCrunch kijkt naar de eerste rij en kolom en gebruikt ze als namen en labels wanneer het woorden zijn (of datums, voor de labels).
2. Kies op het tabblad **Invoegen**, in de groep **Grafieken**, een type: **Kolom**, **Staaf**, **Lijn**, **Cirkel**, **Vlak**, **Spreiding** of **Combinatie**. De kleine pijl naast een type toont de varianten, zoals gestapelde kolommen, een lijn met markeringen, of een spreidingsdiagram met punten, met verbonden punten of met vloeiende lijnen.
3. Wilt u eerst alle typen bekijken, kies dan **Meer grafieken**. De lijst links toont elk type getekend met uw cellen. Met **Rijen en kolommen omwisselen** draait u de reeksen om.

Weet u niet welk type u moet kiezen? Selecteer uw cellen en kies **Aanbevolen grafieken** op het tabblad **Invoegen**. JeffCrunch bekijkt de cellen en geeft een paar types die erbij passen, het beste eerst. Staan datums in de eerste kolom, dan wordt bijvoorbeeld een lijn voorgesteld; staat één kolom met enkele positieve getallen en labels, dan een cirkel. Elk type wordt getekend met uw cellen. Kies er een en selecteer **OK**.

Selecteert u maar één cel, dan maakt JeffCrunch een grafiek van het cellenblok eromheen dat gegevens bevat.

Reeksen lopen langs de kolommen als de selectie minstens zoveel rijen als kolommen heeft, en langs de rijen als de selectie meer kolommen dan rijen heeft. Een cirkel bevat één reeks. Een spreidingsdiagram neemt de horizontale waarden uit de eerste kolom.

Voorbeeld: u selecteert A1:C4, waarin A2:A4 Q1, Q2 en Q3 bevatten, B1 en C1 Oost en West bevatten, en de rest getallen zijn. De grafiek heeft twee reeksen, Oost en West, en drie groepen kolommen, Q1 tot en met Q3.

## Een reeks tijdelijk verbergen

Selecteer een grafiek. Naast de grafiek verschijnen drie kleine knoppen. De knop met de trechter is **Grafiekfilters**. Die toont de reeksen van de grafiek, elk met een vinkje. Haal het vinkje weg om die reeks uit de grafiek te laten. Zet het vinkje weer aan om de reeks op dezelfde plek terug te zetten. **Ongedaan maken** werkt ook. Een grafiek houdt altijd minstens één reeks. Een verborgen reeks wordt niet bewaard wanneer u het bestand opslaat en sluit, dus zet het vinkje eerst weer aan als u de reeks wilt behouden.

## Verplaatsen en formaat wijzigen

Klik op een grafiek, afbeelding of vorm om het te selecteren. Sleep het om het te verplaatsen, of sleep een van de acht vierkantjes op de rand om het formaat te wijzigen.

- Houd **Alt** ingedrukt terwijl u sleept om een rand uit te lijnen op de rand van een cel.
- Houd **Shift** ingedrukt terwijl u een hoek sleept om de verhoudingen te behouden.
- De pijltoetsen verplaatsen het object een paar pixels per keer. **Ctrl** met een pijltoets verplaatst het één pixel, **Shift** met een pijltoets maakt het groter of kleiner, en **Alt** met een pijltoets gaat naar de volgende rand van een cel.
- **Esc** terwijl u sleept, zet het object terug op de oorspronkelijke plaats.

Elke verplaatsing of wijziging van het formaat is één stap die u ongedaan kunt maken. Klik met de rechtermuisknop op het object en kies **Eigenschappen** om een **Naam** in te typen (elk object op een blad heeft een eigen naam nodig) en de **Alternatieve tekst** (een korte zin die voor iemand die het object niet kan zien, beschrijft wat het toont), om het formaat in te typen, en om te kiezen wat het object doet wanneer u cellen invoegt, verwijdert of het formaat ervan wijzigt: verplaatsen en het formaat meewijzigen met de cellen, meeverplaatsen maar het formaat behouden, of op zijn plaats blijven. Die keuze wordt in het bestand opgeslagen. **Dit object afdrukken** in hetzelfde venster schakelt het object uit op de afgedrukte pagina's (het blijft op het scherm zichtbaar); ook die keuze bewaart het bestand.

## Wijzigen wat een grafiek toont

Wanneer een grafiek is geselecteerd, verschijnen er twee knoppen naast de grafiek en verschijnt het tabblad **Grafiek** op het lint.

- De knop **+** (en **Grafiekonderdelen** op het tabblad) schakelt de titel, assen, astitels, rasterlijnen, legenda en gegevenslabels van de grafiek aan of uit, en bepaalt waar de legenda en de labels komen.
- De kwastknop (en **Kleuren** en **Stijlen** op het tabblad) geeft de grafiek een van de kleurenset en een van de stijlen van JeffCrunch. **Indelingen** stelt meerdere onderdelen tegelijk in.
- **Grafiekgegevens** toont de reeksen van de grafiek. U kunt ze toevoegen, bewerken, verwijderen en opnieuw ordenen, de cellen wijzigen die elke reeks leest, de cellen met de labels instellen, rijen en kolommen omwisselen, en kiezen hoe lege cellen en verborgen rijen en kolommen in de grafiek worden weergegeven.
- **Trendlijn** (in het menu **+**) tekent een lijn die op de getallen van elke reeks is gefit, zodat u ziet waar ze naartoe gaan. Kies **Lineair** voor een rechte lijn, **Exponentieel** voor getallen die met een vast percentage groeien, **Logaritmisch** voor getallen die snel stijgen en daarna vlakker worden, **Polynoom** voor een kromme met bochten, **Macht**, of **Voortschrijdend gemiddelde**, dat het gemiddelde van de laatste paar getallen tekent. **Vergelijking weergeven** schrijft de formule van de lijn in de grafiek, en **R-kwadraat weergeven** schrijft hoe goed de lijn past (1 is een perfecte pasvorm; 0 is helemaal geen pasvorm). De trendlijn krijgt een eigen regel in de legenda.
- Wordt een reeks getekend tegen een tweede verticale as, dan hebben de menu's **Assen** en **Astitels** de opties **Tweede horizontale as** en **Tweede verticale as** voor die reeks. In het menu **Legenda** staat **Over de grafiek**, waarmee de legenda over het diagram wordt gelegd in plaats van ernaast.
- **Gegevenslabels** heeft ook **Label bevat** (de waarde, de naam van de categorie, de naam van de reeks, het percentage en de legendasleutel, een klein vierkantje in de kleur van de reeks, in elke combinatie), **Scheidingsteken** (wat ertussen staat: een komma, puntkomma, punt, nieuwe regel of spatie) en **Getalnotatie…** (een notatiecode, zoals 0,0, voor de getallen in de labels; laat dit leeg om de getallen zoals ze zijn te tonen). Schakelt u alle inhoudsopties uit, dan verdwijnen de labels.
- **Horizontale as opmaken…** en **Verticale as opmaken…** op het tabblad **Grafiek** openen een venster voor één as. Hier stelt u de laagste en hoogste waarde in, en de hoofd- en subintervallen (elk automatisch of een getal dat u typt), of de waarden in omgekeerde volgorde lopen, of de schaal logaritmisch is (elke streep is een grondtal maal de vorige, zoals 1, 10, 100 en 1000; dat past bij getallen die sterk in grootte verschillen en allemaal groter dan 0 moeten zijn), waar de andere as de as snijdt, de notatie van de labels, de streepjes, waar de labels staan en hoeveel de labels zijn gedraaid. Alleen een getalas heeft grenzen en intervallen.
- **Grafiek verplaatsen…** op het tabblad **Grafiek** vraagt om een ander werkblad en zet de grafiek daar neer, op dezelfde cellen. De grafiek leest nog steeds de cellen die hij las en behoudt zijn formaat en zijn plek op het blad, ook als de rijen en kolommen daar hoger of breder zijn; hij verplaatst en wijzigt het formaat met de cellen daar net zo als voorheen. Verborgen bladen worden niet aangeboden. Het is één stap **Ongedaan maken**, ook nadat u hebt opgeslagen. Een grafiekblad (een blad dat alleen een grafiek is) kan nog niet worden gemaakt, en een grafiek die erop staat, kan niet worden verplaatst.
- **Grafiektitel** en **Astitels** (in het menu **+**) hebben ook **Titeltekst…**. Typ de gewenste tekst. Of typ een gelijkteken en een cel, zoals **=Blad1!A1**, dan toont de titel wat die cel bevat. Verandert de cel, dan verandert ook de titel. Het bestand bewaart de koppeling.
- **Opslaan als afbeelding…** op het tabblad **Grafiek** (en in het contextmenu) slaat de geselecteerde grafiek op als een afbeeldingsbestand in **PNG** of **JPEG**, twee keer zo groot als op het blad, zodat hij scherp blijft.
- **Opslaan als sjabloon…** en **Sjabloon toepassen…** (op het tabblad **Grafiek**, in de groep **Type**) bewaren de stijl van een grafiek in een **.crtx**-bestand en passen die toe op een andere grafiek. De stijl omvat het type, de kleuren, de instellingen voor titel en legenda, de assen en de lettertypen. De grafiek waarop u de stijl toepast, behoudt zijn eigen getallen en de cellen die hij leest. Eén stap **Ongedaan maken** haalt de stijl weer weg.
- **Grafiektype** wijzigt het type voor de hele grafiek of voor elke reeks: kolommen met een lijn erboven, of een reeks tegen een tweede verticale as.

Een verwijzing naar cellen schrijft u als **Blad1!$B$2:$B$9**; de naam van het blad staat tussen enkele aanhalingstekens als die spaties bevat.

## Sparklines

Een sparkline is een kleine grafiek die binnen één cel past. Hij toont de trend van een rij of kolom met getallen, vlak naast die getallen.

1. Selecteer de getallen. Wilt u één sparkline per rij, selecteer dan meerdere rijen met getallen.
2. Kies op het tabblad **Invoegen**, in de groep **Sparklines**, **Lijn**, **Kolom** of **Winst/verlies**. Een **Lijn** verbindt de getallen met een lijn. Een **Kolom** tekent een staaf voor elk getal. **Winst/verlies** tekent voor elk getal boven nul een staaf omhoog en voor elk getal onder nul een staaf omlaag, allemaal even hoog, wat past bij resultaten die alleen goed of slecht zijn.
3. Controleer in het venster **Sparklines maken** het **Gegevensbereik** (de cellen die de sparklines lezen) en typ het **Locatiebereik** (de cellen waarin ze komen). Kies één locatiecel voor elke rij met gegevens, of voor elke kolom met gegevens. Kiest u één cel, dan leest de sparkline alle gegevens.

De sparklines die u in één stap maakt, vormen een groep en delen één stijl. Klik op een cel met een sparkline, dan verschijnt het tabblad **Sparkline**.

- **Gegevens bewerken** wijzigt de gegevens en de locatie van de hele groep, of alleen de gegevens van degene waarop u klikte. Hier kiest u ook wat er met lege cellen gebeurt (een lege ruimte laten, ze als nul behandelen, of de punten erover heen verbinden) en of cellen in verborgen rijen en kolommen worden gelezen.
- **Type** schakelt tussen lijn, kolom en winst/verlies.
- **Weergeven** markeert punten: het **Hoogste punt**, het **Laagste punt**, **Negatieve punten**, het **Eerste punt**, het **Laatste punt** en, bij een lijn, **Markeringen** op elk punt.
- **Stijlen** geeft de groep een set kleuren. **Sparklinekleur** stelt de kleur van de lijn of de staven in, en de **Dikte** (hoe dik de lijn is, in punten). **Markeringskleur** stelt de kleur in van elk soort gemarkeerd punt.
- **As** toont een lijn bij nul, zet de getallen van rechts naar links uit en stelt de laagste en hoogste waarde van de verticale as in. **Automatisch voor elke sparkline** past elke sparkline aan op zijn eigen getallen. **Hetzelfde voor alle sparklines** geeft de hele groep één schaal, zodat u ze kunt vergelijken. **Aangepaste waarde** laat u een getal typen. **Datumas…** verdeelt de punten op datum: typ de cellen met de datum van elk punt, en een punt met een latere datum staat verder naar rechts. Laat dit leeg om de punten weer gelijkmatig te verdelen.
- **Groeperen** zet de geselecteerde sparklines in één groep. **Groepering opheffen** geeft elke sparkline een eigen stijl. **Wissen** verwijdert de geselecteerde sparklines of hun hele groepen.

Een sparkline volgt zijn cellen: voegt u een rij boven de gegevens in, dan leest hij nog steeds dezelfde getallen. Elke wijziging is één stap die u ongedaan kunt maken. Sparklines worden in het bestand opgeslagen in een vorm die andere spreadsheetprogramma's lezen.

## Een afbeelding, vorm of tekstvak invoegen

Op het tabblad **Invoegen**, in de groep **Illustraties**, opent **Afbeelding** een bestandskiezer. Kies een afbeeldingsbestand (PNG, JPEG, GIF, BMP, TIFF of SVG) en het komt op het blad, bij de geselecteerde cel. Een afbeelding die breder is dan 480 punten, wordt verkleind om te passen en behoudt zijn vorm. De werkmap bewaart de afbeelding zelf. Een afbeelding van een soort die de werkmap niet zoals die is kan bewaren (een SVG, bijvoorbeeld), wordt opgeslagen als PNG, een gewone afbeelding van pixels. Een bestand dat geen afbeelding is, of groter is dan 16 MB, wordt geweigerd met een melding.

**Vormen** opent een lijst: rechthoek, afgeronde rechthoek, ovaal, driehoek, ruit, pijlen, een ster, een hart, een wolk, een spraakballon en een lijn. Kies er een en hij komt op het blad, lichtblauw met een donkerdere rand. **Tekstvak**, in de groep **Tekst**, zet een leeg vak neer zonder vulling en zonder rand.

Elk van deze is één stap die u ongedaan kunt maken. Het nieuwe object is geselecteerd, zodat u het kunt slepen, het formaat kunt wijzigen, het naar voren of naar achteren kunt halen, het in **Eigenschappen** een naam en alternatieve tekst kunt geven, of op **Delete** kunt drukken.

## Het uiterlijk van een afbeelding, vorm of tekstvak wijzigen

Selecteert u een afbeelding, dan verschijnt het tabblad **Afbeeldingsopmaak**. Selecteert u een vorm, een lijn of een tekstvak, dan verschijnt het tabblad **Vormopmaak**. Elke wijziging is één stap die u ongedaan kunt maken, en het object behoudt zijn naam, zijn alternatieve tekst, zijn plek en zijn volgorde ten opzichte van de andere objecten.

- **Vormvulling** en **Vormomtrek** kiezen een kleur, of geen. **Lijnstijl** stelt de dikte (hoe dik de lijn is) en de streepjes in (doorgetrokken, gestreept of gestippeld). Een afbeelding heeft **Afbeeldingsrand**, de omtrek ervan.
- **Draaien** draait het object een kwartslag naar rechts of links, of spiegelt het van links naar rechts of van boven naar beneden.
- **Bijsnijden**, voor een afbeelding, snijdt die bij tot een venstervorm: vierkant, 4 breed bij 3 hoog, 3 breed bij 4 hoog, 16 bij 9, of 9 bij 16. Het bijsnijden gebeurt gelijkmatig vanuit het midden van wat zichtbaar is, en de afbeelding wordt zo groot gemaakt als wat overblijft. **Hele afbeelding tonen** brengt weer wat is bijgesneden. Het afbeeldingsbestand verandert niet: het bijgesneden deel wordt alleen verborgen.
- **Tekst bewerken** is voor vormen en tekstvakken. Typ de woorden en kies hoe ze horizontaal en verticaal staan, de grootte, vet of normaal, en of ze binnen de vorm teruglopen of op één regel blijven.
- **Selectievenster** opent een lijst van alle objecten op het blad met hun naam, het object vooraan het eerst. Klik op een naam om dat object te selecteren; dat helpt wanneer objecten elkaar verbergen. **Naar voren** en **Naar achteren** verplaatsen het object in de lijst.
- **Eigenschappen** stelt het formaat, de naam, de alternatieve tekst en wat het object doet wanneer cellen worden verplaatst of vergroot, in.

## Een object knippen, kopiëren en plakken

Een afbeelding of vorm wordt op dezelfde manier gekopieerd en geplakt als een grafiek. Een vorm die iets gebruikt wat niet uit de werkmap komt, zoals een koppeling naar een bestand, kan niet worden gekopieerd; JeffCrunch laat u weten waarom.

Kopieert u een grafiek en plakt u die in dezelfde werkmap, op een willekeurig blad, dan leest de kopie dezelfde cellen als het origineel. Plakt u hem in een andere werkmap, dan behoudt hij de getallen die hij toonde, maar leest hij geen cellen meer, omdat de cellen die hij daar zou lezen niet de cellen zijn waaruit hij is gemaakt.

## De naam wijzigen

Wanneer een object is geselecteerd, toont het naamvak de naam ervan. Typ er een nieuwe naam en druk op **Enter**. **Ongedaan maken** zet de oude naam terug, en de nieuwe naam wordt in het bestand opgeslagen.

## Verwijderen

Druk op **Delete** terwijl het object is geselecteerd. **Ongedaan maken** brengt het terug.

## Wat wordt opgeslagen

Alles hierboven wordt in het bestand opgeslagen in een vorm die andere spreadsheetprogramma's lezen, zodat een grafiek die u hier maakt, daar op dezelfde manier opent. Een grafiek die uit een ander programma komt met iets wat JeffCrunch bij het herschrijven van een grafiek niet kan bewaren (foutbalken, een driedimensionale grafiek), kan hier wel worden verplaatst, van formaat veranderd en verwijderd, maar de elementen en gegevens ervan worden hier niet gewijzigd.
