# Grafieken, sparklines, afbeeldingen en vormen

<!-- keywords: veldknoppen, knoppen draaigrafiek, groeperen, groep opheffen, uitlijnen, links uitlijnen, verdelen, objecten selecteren, meerdere vormen, meerdere kiezen, selectievak, grafiekfilters, reeksen verbergen, aanbevolen grafieken, grafiekvoorstellen, grafiek, diagram, kolom, staaf, lijn, cirkel, vlak, spreiding, combinatie, legenda, titel, rasterlijnen, gegevenslabels, grafiekgegevens, grafiektype, weergaven, indelingen, verplaatsen, formaat wijzigen, afbeelding, vorm, object, reeks, trendlijn, trendlijn, vergelijking, R-kwadraat, tweede as, sparkline, sparklines, datumas, winst/verlies, minigrafiek, markeringen, naam, alternatieve tekst, alt-tekst, logaritmisch, logaritmische schaal, opslaan als afbeelding, sjabloon, crtx, titeltekst, legenda-sleutel, selectiedeelvenster, vulling van vorm, omtrek van vorm, draaien, spiegelen, bijsnijden, tekst bewerken, randen van afbeelding, afbeelding invoegen, pictogrammen, pictogram, wmf, emf, vormstijlen, stijlen, vorm invoegen, tekstvak, vormen, pijl, ovaal, rechthoek, ster, diagram, diagrammen -->

Een grafiek is een beeld van getallen dat aan die getallen gekoppeld blijft: als u een getal wijzigt, verandert de grafiek mee. Afbeeldingen en vormen liggen op dezelfde manier over de cellen, maar ze zijn niet aan getallen gekoppeld.

## Een grafiek maken

1. Selecteer de getallen die u in de grafiek wilt tonen. Als de eerste rij namen bevat en de eerste kolom labels, selecteer die dan ook. JeffCrunch kijkt naar de eerste rij en kolom en gebruikt ze als namen en labels wanneer het woorden zijn (of datums, voor de labels).
2. Kies op het tabblad **Invoegen**, in **Grafieken**, een soort: **Kolom**, **Staaf**, **Lijn**, **Cirkel**, **Vlak**, **Spreiding** of **Combinatie**. De kleine pijl naast een soort toont de varianten, zoals gestapelde kolommen, een lijn met markeringen, of een spreidingsdiagram met punten, met verbonden punten of met vloeiende lijnen.
3. Om eerst alle soorten te bekijken, kiest u **Meer grafieken**. De lijst links toont elke soort getekend met uw cellen. Met **Rijen en kolommen wisselen** draait u de reeksen om.

Weet u niet welke soort u moet kiezen? Selecteer uw cellen en kies **Aanbevolen grafieken** op het tabblad **Invoegen**. JeffCrunch bekijkt de cellen en geeft een paar soorten die erbij passen, de beste eerst. Datums onder elkaar leiden bijvoorbeeld tot een lijn, en één kolom met een paar positieve getallen en labels tot een cirkel. Elke soort wordt getekend met uw cellen. Kies er een en klik op **OK**.

Als u maar één cel selecteert, maakt JeffCrunch een grafiek van het cellenblok eromheen dat gegevens bevat.

Reeksen lopen langs de kolommen als de selectie minstens zoveel rijen als kolommen heeft, en langs de rijen als de selectie meer kolommen dan rijen heeft. Een cirkel bevat één reeks. Een spreidingsdiagram neemt zijn horizontale waarden uit de eerste kolom.

Voorbeeld: u selecteert A1:C4, waarin A2:A4 Q1, Q2 en Q3 bevatten, B1 en C1 Oost en West, en de rest getallen. De grafiek heeft twee reeksen, Oost en West, en drie groepen kolommen, Q1 tot en met Q3.

## Een reeks voorlopig verbergen

Kies een grafiek. Naast de grafiek verschijnen drie kleine knoppen. De knop met de trechter is **Grafiekfilters**. Die toont de reeksen van de grafiek, elk met een vinkje. Haal het vinkje weg om een reeks uit de grafiek te laten. Zet het vinkje weer aan om haar op dezelfde plek terug te zetten. **Ongedaan maken** werkt ook. Een grafiek houdt altijd minstens één reeks. Een verborgen reeks blijft in het bestand staan wanneer u opslaat en blijft verborgen wanneer u het bestand opent, zodat er niets verloren gaat. Zet het vinkje weer aan om haar terug te zetten. (Een ringdiagram toont bij het openen alle reeksen.)

## Verplaatsen en het formaat wijzigen

Klik op een grafiek, afbeelding of vorm om die te selecteren. Sleep om te verplaatsen, of sleep een van de acht vierkantjes aan de rand om het formaat te wijzigen.

- Houd **Alt** ingedrukt tijdens het slepen om een rand uit te lijnen op de rand van een cel.
- Houd **Shift** ingedrukt terwijl u een hoek sleept om de verhoudingen te behouden.
- Met de pijltoetsen verplaatst u het object een paar pixels tegelijk. **Ctrl** met een pijltoets verplaatst het één pixel, **Shift** met een pijltoets maakt het groter of kleiner, en **Alt** met een pijltoets gaat naar de volgende rand van een cel.
- **Esc** tijdens het slepen zet het object terug.

Elke verplaatsing of wijziging van het formaat is één stap die u ongedaan kunt maken. Klik met de rechtermuisknop op het object en kies **Eigenschappen** om de **Naam** in te typen (elk object op een blad heeft een eigen naam nodig) en de **Alternatieve tekst** (een korte zin die voor iemand die het object niet kan zien, beschrijft wat het toont), om de grootte in te typen en om te kiezen wat er gebeurt wanneer u cellen invoegt, verwijdert of het formaat ervan wijzigt: meeverplaatsen en meeschalen met de cellen, meeverplaatsen maar de grootte behouden, of op zijn plaats blijven. De keuze wordt in het bestand opgeslagen. **Dit object afdrukken** in hetzelfde venster schakelt het object uit op de afgedrukte pagina's (het blijft op het scherm zichtbaar); ook die keuze bewaart het bestand.

## Wijzigen wat een grafiek toont

Wanneer een grafiek is geselecteerd, verschijnen er twee knoppen naast en verschijnt het tabblad **Grafiek** op het lint.

- De knop **+** (en **Grafiekonderdelen** op het tabblad) zet de titel, assen, astitels, rasterlijnen, legenda en gegevenslabels van de grafiek aan of uit, en geeft aan waar de legenda en de labels komen.
- De kwastknop (en **Kleuren** en **Stijlen** op het tabblad) geeft de grafiek een van de kleurensets en een van de stijlen van JeffCrunch. **Indelingen** stelt meerdere onderdelen tegelijk in.
- **Grafiekgegevens** toont de reeksen van de grafiek. U kunt ze toevoegen, bewerken, verwijderen en opnieuw ordenen, de cellen wijzigen die elke reeks leest, de cellen met de labels instellen, de rijen en kolommen wisselen, en kiezen hoe lege cellen en verborgen rijen en kolommen in de grafiek worden weergegeven.
- **Veldknoppen** (in het menu **+**, alleen voor een draaigrafiek) toont kleine knoppen met de namen van de velden van de draaitabel die de grafiek volgt: de rapportfilters onder de titel, en de rijen, waarden en kolommen langs de onderkant. Zet elke soort aan of uit, of kies **Alles weergeven** of **Alles verbergen**. Een nieuwe draaigrafiek toont ze.
- **Foutbalken** (in het menu **+**, voor een kolom-, staaf-, lijn-, vlak- of spreidingsgrafiek) tekenen boven en onder elk punt een kleine lijn die aangeeft hoeveel de werkelijke waarde kan afwijken. Kies **Standaardfout**, **Percentage (5%)**, **Standaarddeviatie** of **Vaste waarde (1)**. Kies **Aangepast** om eigen bedragen te gebruiken: typ de cellen met de bedragen boven elk punt (zoals `=Blad1!$D$2:$D$4`) en daarna de cellen voor de bedragen eronder. Laat er één leeg om de balken maar aan één kant te tekenen. De balken veranderen wanneer die cellen veranderen. Een bestand dat eigen bedragen per punt heeft, behoudt die.
- **Lijnen** (in het menu **+**, voor een lijngrafiek) heeft **Verticale lijnen**, die van het hoogste punt van elke categorie naar beneden lopen tot de as, **Hoog-laag-lijnen**, die het hoogste en laagste punt van elke categorie verbinden, en **Op-/neerwaartse staven**, dat zijn vakjes tussen de eerste en de laatste reeks van elke categorie. Een opwaartse staaf laat zien dat de laatste reeks hoger ligt. Hoog-laag-lijnen en op-/neerwaartse staven vereisen twee of meer reeksen. Vlakgrafieken hebben verticale lijnen.
- **Trendlijn** (in het menu **+**) tekent een lijn die op de getallen van elke reeks is gelegd, zodat u ziet waar ze naartoe gaan. Kies **Lineair** voor een rechte lijn, **Exponentieel** voor getallen die met een vast percentage groeien, **Logaritmisch** voor getallen die snel stijgen en dan vlakker worden, **Polynoom** voor een kromme met bochten, **Macht**, of **Voortschrijdend gemiddelde**, dat het gemiddelde van de laatste paar getallen tekent. **Vergelijking weergeven** schrijft de formule van de lijn op de grafiek, en **R-kwadraat weergeven** schrijft hoe goed de lijn past (1 is een perfecte aansluiting; 0 betekent helemaal geen aansluiting). De trendlijn krijgt een eigen regel in de legenda.
- Wanneer een reeks tegen een tweede verticale as wordt getekend, hebben de menu's **Assen** en **Astitels** de opties **Tweede horizontale as** en **Tweede verticale as** voor die reeks. Het menu **Legenda** heeft **Over de grafiek**, dat de legenda over het diagram legt in plaats van ernaast.
- **Gegevenslabels** heeft ook **Labelinhoud** (de waarde, de naam van de categorie, de naam van de reeks, het percentage en de legenda-sleutel, een klein vierkantje in de kleur van de reeks, in elke combinatie), **Scheidingsteken** (wat ertussen staat: een komma, puntkomma, punt, nieuwe regel of spatie) en **Getalnotatie…** (een notatiecode zoals 0,0 voor de getallen in de labels; laat dit leeg om de getallen te tonen zoals ze zijn). **Waarde uit cellen…** toont de woorden van enkele cellen als labels, één cel voor elk punt. Typ de cellen, zoals =Blad1!$D$2:$D$4. De labels veranderen wanneer de cellen veranderen. Laat het leeg om ze te verwijderen. Schakel alle inhoudsopties uit en de labels verdwijnen.
- **Horizontale as opmaken…** en **Verticale as opmaken…** op het tabblad **Grafiek** openen een venster voor één as. Hier stelt u de laagste en hoogste waarde in en de hoofd- en subintervallen (elk automatisch of een getal dat u typt), of de waarden in omgekeerde volgorde lopen, de weergave-eenheden (de labels tonen de getallen gedeeld, bijvoorbeeld door 1.000 voor duizendtallen, zodat 30.000 als 30 wordt gelezen; vink **De naam van de eenheden op de grafiek weergeven** aan om de naam, zoals Duizendtallen, samen met de astitel af te drukken), of de schaal logaritmisch is (elke streep is een grondtal maal de vorige, zoals 1, 10, 100 en 1000, wat geschikt is voor getallen die sterk in grootte verschillen en allemaal groter dan 0 moeten zijn), waar de andere as deze snijdt, de getalnotatie van de labels, de streepjes van de as, waar de labels staan en hoe ver de labels zijn gedraaid. Alleen een getalas heeft grenzen en intervallen.
- **Grafiek verplaatsen…** op het tabblad **Grafiek** vraagt waar de grafiek naartoe moet. Kies **Nieuw blad** en typ een naam, dan krijgt de grafiek een eigen grafiekblad: een blad dat alleen een grafiek is, zonder cellen. Kies **Object in** en selecteer een werkblad, dan komt de grafiek op dat werkblad. Een grafiek op een grafiekblad kan alleen naar een werkblad worden verplaatst, en het grafiekblad dat hierbij achterblijft, wordt verwijderd. De grafiek leest nog steeds de cellen die hij las. Op een werkblad behoudt hij zijn grootte en plek op het blad, ook waar rijen en kolommen hoger of breder zijn, en hij verplaatst en schaalt mee met de cellen daar. Verborgen bladen worden niet aangeboden. Het is één stap **Ongedaan maken**, ook nadat u hebt opgeslagen, tenzij er door deze handeling een blad is verwijderd. Een bestand in de vorm `.ods` kan geen grafiekblad bevatten; sla een grafiekblad daarom op in de vorm `.xlsx`.
- **Grafiektitel** en **Astitels** (in het menu **+**) hebben ook **Titeltekst…**. Typ de gewenste woorden. Of typ een gelijkteken en een cel, zoals **=Blad1!A1**, dan toont de titel wat die cel bevat. Wanneer de cel verandert, verandert ook de titel. Het bestand bewaart de koppeling.
- **Opslaan als afbeelding…** op het tabblad **Grafiek** (en in het rechtermuisknopmenu) slaat de geselecteerde grafiek op als afbeeldingsbestand in de vorm **PNG** of **JPEG**, twee keer zo groot als op het blad, zodat hij scherp blijft.
- **Opslaan als sjabloon…** en **Sjabloon toepassen…** (op het tabblad **Grafiek**, in de groep **Type**) bewaren de stijl van een grafiek in een **.crtx**-bestand en geven die aan een andere grafiek. De stijl omvat de soort, de kleuren, de instellingen voor titel en legenda, de assen en de lettertypen. De grafiek waarop u hem toepast, behoudt zijn eigen getallen en de cellen die hij leest. Eén **Ongedaan maken** haalt de stijl weer weg.
- **Grafiektype** wijzigt de soort voor de hele grafiek of voor elke reeks: kolommen met een lijn erboven, of een reeks tegen een tweede verticale as.

Een verwijzing naar cellen schrijft u zoals **Blad1!$B$2:$B$9**, waarbij de naam van het blad tussen enkele aanhalingstekens staat als er spaties in zitten.

## Sparklines

Een sparkline is een minigrafiek die in één cel past. Hij toont de trend van een rij of kolom met getallen, direct naast die getallen.

1. Selecteer de getallen. Voor één sparkline per rij selecteert u meerdere rijen met getallen.
2. Kies op het tabblad **Invoegen**, in **Sparklines**, **Lijn**, **Kolom** of **Winst/verlies**. Een **Lijn** verbindt de getallen met een lijn. Een **Kolom** tekent een staaf voor elk getal. **Win/verlies** tekent een staaf omhoog voor elk getal boven nul en een staaf omlaag voor elk getal eronder, allemaal even groot, wat geschikt is voor resultaten die alleen goed of slecht zijn.
3. Controleer in het venster **Sparklines maken** het **Gegevensbereik** (de cellen die de sparklines lezen) en typ het **Locatiebereik** (de cellen waarin ze komen). Kies één locatiecel voor elke rij met gegevens, of voor elke kolom met gegevens. Kiest u één cel, dan leest de sparkline alle gegevens.

De sparklines die u in één stap maakt, vormen een groep en delen één stijl. Klik op een cel met een sparkline en het tabblad **Sparkline** verschijnt.

- **Gegevens bewerken** wijzigt de gegevens en de locatie van de hele groep, of de gegevens van de ene die u hebt aangeklikt. Hier kiest u ook wat er met lege cellen gebeurt (een lege ruimte laten, ze als nul behandelen, of de punten erover heen verbinden) en of cellen in verborgen rijen en kolommen worden gelezen.
- **Type** schakelt tussen lijn, kolom en winst/verlies.
- **Weergeven** markeert punten: het **Hoogste punt**, het **Laagste punt**, **Negatieve punten**, het **Eerste punt**, het **Laatste punt** en, voor een lijn, **Markeringen** op elk punt.
- **Stijlen** geeft de groep een set kleuren. **Sparklinekleur** stelt de kleur van de lijn of de staven in en de **Dikte** (hoe dik de lijn is, in punten). **Markeringskleur** stelt de kleur in van elke soort gemarkeerd punt.
- **As** toont een lijn bij nul, zet de getallen van rechts naar links uit, en stelt de laagste en hoogste waarde van de verticale as in. **Automatisch voor elke sparkline** past elke sparkline aan op zijn eigen getallen. **Hetzelfde voor alle sparklines** geeft de hele groep één schaal, zodat u ze kunt vergelijken. **Aangepaste waarde** laat u een getal typen. **Datumas…** verdeelt de punten op datum: typ de cellen met de datum van elk punt, en een punt met een latere datum staat verder naar rechts. Laat het leeg om de punten weer gelijkmatig te verdelen.
- **Groeperen** zet de geselecteerde sparklines in één groep. **Groep opheffen** geeft elke sparkline een eigen stijl. **Wissen** verwijdert de geselecteerde sparklines of hun hele groepen.

Een sparkline volgt zijn cellen: voegt u een rij boven de gegevens in, dan leest hij nog steeds dezelfde getallen. Elke wijziging is één stap die u ongedaan kunt maken. Sparklines worden in het bestand opgeslagen in de vorm die andere spreadsheetprogramma's lezen.

## Een afbeelding, vorm of tekstvak invoegen

Op het tabblad **Invoegen**, in **Illustraties**, opent **Afbeelding** een bestandskiezer. Kies een afbeeldingsbestand (PNG, JPEG, GIF, BMP, TIFF of SVG) en het komt op het blad, bij de geselecteerde cel. Een afbeelding die breder is dan 480 punten wordt kleiner gemaakt om te passen en behoudt zijn vorm. De werkmap bewaart de afbeelding zelf. Een afbeelding van een soort die de werkmap niet zoals die is kan bewaren (een SVG, bijvoorbeeld) wordt bewaard als PNG, een gewone afbeelding van puntjes. Een bestand dat geen afbeelding is, of groter is dan 16 MB, wordt geweigerd met een melding. Een WMF- of EMF-bestand (de afbeeldingsvormen die sommige oudere programma's maken van lijnen en vormen) wordt ook getekend en als PNG bewaard.

**Pictogrammen** toont hetzelfde venster met kleine lijnafbeeldingen dat JeffPub en JeffDeck hebben, met een vak om ze op naam te zoeken. Kies er een of meer, dan komt elk op het blad als afbeelding van 48 punten breed en hoog. Ze worden donkergrijs getekend, op een formaat dat scherp blijft wanneer u ze vergroot. Elk is één stap die u ongedaan kunt maken.

**Vormen** opent een lijst: rechthoek, afgeronde rechthoek, ovaal, driehoek, ruit, pijlen, een ster, een hart, een wolk, een spraakballon en een lijn. Kies er een en hij komt op het blad in lichtblauw met een donkerdere rand. **Tekstvak**, in de groep **Tekst**, zet een leeg vak neer zonder vulling en zonder rand.

Wanneer een vorm is geselecteerd, geeft **Stijlen** op het tabblad **Vormopmaak** de vorm een kant-en-klare stijl: een vulling en een omtrek die bij elkaar passen, zoals lichtblauw met een donkerblauwe rand. De twee laatste houden alleen de omtrek of alleen de vulling over. Eén klik is één stap die u ongedaan kunt maken. **Vulling van vorm** en **Omtrek van vorm** wijzigen elk onderdeel afzonderlijk.

Elk is één stap die u ongedaan kunt maken. Het nieuwe object is geselecteerd, zodat u het kunt slepen, het formaat kunt wijzigen, naar voren of naar achteren kunt verplaatsen, een naam en alternatieve tekst kunt geven in **Eigenschappen**, of op **Delete** kunt drukken.

## Wijzigen hoe een afbeelding, vorm of tekstvak eruitziet

Selecteer een afbeelding en het tabblad **Afbeeldingsopmaak** verschijnt. Selecteer een vorm, een lijn of een tekstvak en het tabblad **Vormopmaak** verschijnt. Elke wijziging is één stap die u ongedaan kunt maken, en het object behoudt zijn naam, alternatieve tekst, plek en volgorde ten opzichte van de andere objecten.

- **Vulling van vorm** en **Omtrek van vorm** kiezen een kleur, of geen. **Lijnstijl** stelt de dikte (hoe dik de lijn is) en de streepjes in (doorgetrokken, gestreept of gestippeld). Een afbeelding heeft **Randen van afbeelding**, dat is de omtrek.
- **Draaien** draait het object een kwartslag naar rechts of links, of spiegelt het van links naar rechts of van boven naar beneden.
- **Bijsnijden** snijdt een afbeelding bij tot een venstervorm: vierkant, 4 breed bij 3 hoog, 3 breed bij 4 hoog, 16 bij 9 of 9 bij 16. De snede wordt gelijkmatig vanuit het midden van wat zichtbaar is gemaakt, en de afbeelding wordt zo groot gemaakt als wat overblijft. **De hele afbeelding tonen** haalt weer op wat is afgesneden. Het afbeeldingsbestand wordt niet gewijzigd: het afgesneden deel wordt alleen verborgen.
- **Tekst bewerken** is voor vormen en tekstvakken. Typ de woorden en kies hoe ze horizontaal en verticaal staan, de grootte, vet of normaal, en of ze binnen de vorm terugvloeien of op één regel blijven.
- **Selectiedeelvenster** opent een lijst van alle objecten op het blad met hun naam, het object vooraan het eerst. Klik op een naam om dat object te selecteren, wat handig is wanneer objecten elkaar verbergen. **Naar voren** en **Naar achteren** verplaatsen het object in de lijst.
- **Eigenschappen** stelt de grootte, de naam, de alternatieve tekst en wat het object doet wanneer cellen worden verplaatst of vergroot.

## Een object knippen, kopiëren en plakken

Een afbeelding of vorm wordt op dezelfde manier gekopieerd en geplakt als een grafiek. Een vorm die iets anders gebruikt dan een afbeelding uit de werkmap, zoals een koppeling naar een bestand, kan niet worden gekopieerd, en JeffCrunch vertelt u waarom.

Kopieer een grafiek en plak hem in dezelfde werkmap, op een willekeurig blad: de kopie leest dezelfde cellen als het origineel. Plakt u hem in een andere werkmap, dan behoudt hij de getallen die hij toonde, maar leest hij geen cellen meer, omdat de cellen die hij daar zou lezen niet die zijn waarvan hij is gemaakt.

## Meerdere objecten selecteren, groeperen en uitlijnen

Klik op een object en houd daarna Shift of Ctrl ingedrukt terwijl u op een ander klikt om dat ook te selecteren. Klik nog eens op een geselecteerd object met Shift of Ctrl om het los te laten. Geselecteerde objecten krijgen elk een gestreept kader. U kunt ook op het tabblad **Vormopmaak** **Objecten selecteren** kiezen en een vak slepen over lege cellen: elk object dat volledig binnen het vak past, wordt geselecteerd. Druk op Esc, of kies **Objecten selecteren** nogmaals, om weer cellen te kunnen selecteren. Druk op Delete om alle geselecteerde objecten in één stap te verwijderen.

Wanneer er twee of meer objecten zijn geselecteerd, heeft het tabblad **Vormopmaak** deze opties in de groep **Schikken**:

- **Groeperen** voegt de geselecteerde vormen, afbeeldingen en lijnen samen tot één object. Het verplaatst en schaalt als één geheel, en het bestand bewaart het als groep. Een grafiek kan niet in een groep worden geplaatst.
- **Groep opheffen** haalt een geselecteerde groep uit elkaar. Elk onderdeel gaat terug naar zijn plek op het blad.
- **Uitlijnen** lijnt de geselecteerde objecten uit: **Links uitlijnen**, **Centreren**, **Rechts uitlijnen**, **Boven uitlijnen**, **Midden uitlijnen** en **Onder uitlijnen**. Ze lijnen uit op de rand of het midden van de hele set. **Horizontaal verdelen** en **Verticaal verdelen** verdelen drie of meer objecten gelijkmatig tussen de twee aan de uiteinden.

Groeperen, Groep opheffen en elke uitlijningskeuze zijn één stap die u ongedaan kunt maken.

## Een naam geven

Wanneer een object is geselecteerd, toont het naamvak de naam ervan. Typ daar een nieuwe naam en druk op **Enter**. Ongedaan maken zet de oude naam terug, en de nieuwe naam wordt in het bestand opgeslagen.

## Verwijderen

Druk op **Delete** terwijl het object is geselecteerd. Ongedaan maken haalt het weer terug.

## Wat wordt opgeslagen

Alles hierboven wordt in het bestand opgeslagen in de vorm die andere spreadsheetprogramma's lezen, zodat een grafiek die u hier maakt, daar op dezelfde manier opent. Een grafiek die uit een ander programma komt met iets wat JeffCrunch niet kan bewaren wanneer het een grafiek opnieuw schrijft (foutbalken, een driedimensionale grafiek), kan wel worden verplaatst, van formaat veranderd en verwijderd, maar de onderdelen en gegevens ervan worden hier niet gewijzigd.

## Diagrammen uit andere programma's

Een **diagram** is een beeld van vakjes en pijlen, zoals een proces, dat door een ander programma is gemaakt. JeffCrunch toont een diagram aan de hand van de afbeelding ervan die het bestand naast het diagram bewaart. Klik erop om het te selecteren en sleep het om het te verplaatsen. Wanneer u het bestand opslaat, wordt het diagram precies zoals het was teruggeschreven, zodat het programma dat het maakte het nog steeds kan wijzigen.

JeffCrunch kan de inhoud van een diagram nog niet wijzigen, en een diagram kan niet worden gekopieerd of in een groep worden geplaatst. Als het bestand geen afbeelding van een diagram bewaart, toont JeffCrunch een eenvoudig grijs vak dat zegt dat het een diagram is.
