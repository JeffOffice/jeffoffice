# Grafieken, sparklines, afbeeldingen en vormen
<!-- keywords: veldknoppen, knoppen draaigrafiek, groeperen, groep opheffen, uitlijnen, links uitlijnen, verdelen, objecten selecteren, meerdere vormen, meerdere kiezen, selectiekader, grafiekfilters, reeks verbergen, aanbevolen grafieken, grafiekuggesties, grafiek, diagram, kolom, staaf, lijn, cirkel, vlak, spreiding, combinatie, legenda, titel, rasterlijnen, gegevenslabels, grafiekgegevens, grafiektype, weergave, indelingen, verplaatsen, formaat wijzigen, afbeelding, vorm, object, reeks, trendlijn, trendlijn, vergelijking, R-kwadraat, tweede as, sparkline, sparklines, datumas, winst/verlies, minigrafiek, markeringen, naam, alternatieve tekst, alt-tekst, logaritmisch, logaritmische schaal, opslaan als afbeelding, sjabloon, crtx, titeltekst, legenda-sleutel, selectievenster, vulling van vorm, omtrek van vorm, draaien, spiegelen, bijsnijden, tekst bewerken, rand van afbeelding, afbeelding invoegen, pictogrammen, icoon, wmf, emf, vormstijlen, stijlen, vorm invoegen, tekstvak, vormen, pijl, ovaal, rechthoek, ster -->

Een grafiek is een beeld van getallen dat aan die getallen gekoppeld blijft: als u een getal wijzigt, verandert de grafiek mee. Afbeeldingen en vormen liggen op dezelfde manier over de cellen, maar ze zijn niet aan getallen gekoppeld.

## Een grafiek maken

1. Selecteer de getallen die u in een grafiek wilt weergeven. Als de eerste rij namen bevat en de eerste kolom labels, selecteert u die ook. JeffCrunch kijkt naar de eerste rij en kolom en gebruikt ze als namen en labels wanneer het woorden zijn (of datums, voor de labels).
2. Kies op het tabblad **Invoegen**, in de groep **Grafieken**, een soort: **Kolom**, **Staaf**, **Lijn**, **Cirkel**, **Vlak**, **Spreiding** of **Combinatie**. Het kleine pijltje naast een soort toont de varianten, zoals gestapelde kolommen, een lijn met markeringen, of een spreidingsdiagram met punten, met verbonden punten of met vloeiende lijnen.
3. Wilt u eerst alle soorten bekijken, kies dan **Meer grafieken**. De lijst links toont elke soort getekend met uw cellen. Met **Rijen en kolommen omwisselen** draait u de reeksen om.

Weet u niet welke soort u moet kiezen? Selecteer uw cellen en kies **Aanbevolen grafieken** op het tabblad **Invoegen**. JeffCrunch bekijkt de cellen en toont een paar soorten die erbij passen, de beste eerst. Datums in de eerste kolom leveren bijvoorbeeld een lijn op, en één kolom met enkele positieve getallen en labels een cirkel. Elke soort wordt getekend met uw cellen. Kies er een en kies **OK**.

Als u maar één cel selecteert, neemt JeffCrunch het cellenblok eromheen op dat gegevens bevat.

Reeksen lopen door de kolommen als de selectie minstens zoveel rijen heeft als kolommen, en langs de rijen als de selectie meer kolommen dan rijen heeft. Een cirkel bevat één reeks. Een spreidingsdiagram neemt de horizontale waarden uit de eerste kolom.

Voorbeeld: u selecteert A1:C4, waarbij A2:A4 Q1, Q2 en Q3 bevatten, B1 en C1 Oost en West, en de rest getallen is. De grafiek heeft twee reeksen, Oost en West, en drie groepen kolommen, Q1 tot en met Q3.

## Een reeks voorlopig verbergen

Kies een grafiek. Naast de grafiek verschijnen drie kleine knoppen. De knop met de trechter is **Grafiekfilters**. Die toont de reeksen van de grafiek, elk met een vinkje. Haal het vinkje weg om die reeks uit de grafiek te laten. Zet het vinkje weer aan om de reeks op dezelfde plek terug te zetten. **Ongedaan maken** werkt ook. Een grafiek houdt altijd minstens één reeks. Een reeks die u hebt weggelaten, blijft in het bestand staan wanneer u opslaat, en blijft verborgen wanneer u het bestand opent, zodat er niets verloren gaat. Zet het vinkje weer aan om haar terug te halen. (Een ringdiagram toont bij het openen alle reeksen.)

## Verplaatsen en formaat wijzigen

Klik op een grafiek, afbeelding of vorm om die te selecteren. Sleep om te verplaatsen, of sleep een van de acht vierkantjes aan de rand om het formaat te wijzigen.

- Houd **Alt** ingedrukt terwijl u sleept om een rand uit te lijnen op de rand van een cel.
- Houd **Shift** ingedrukt terwijl u een hoek sleept om de verhoudingen te behouden.
- Met de pijltjestoetsen verplaatst u het object een paar pixels per keer. **Ctrl** met een pijltoets verplaatst het één pixel, **Shift** met een pijltoets maakt het groter of kleiner, en **Alt** met een pijltoets gaat naar de volgende rand van een cel.
- **Esc** terwijl u sleept, zet het object terug op de oorspronkelijke plaats.

Elke verplaatsing of wijziging van het formaat is één stap die u ongedaan kunt maken. Klik met de rechtermuisknop op het object en kies **Eigenschappen** om de **Naam** in te typen (elk object op een blad heeft een eigen naam nodig) en de **Alternatieve tekst** (een korte zin die een persoon die het object niet kan zien, vertelt wat het toont), om de grootte in te typen en om te kiezen wat er gebeurt wanneer u cellen invoegt, verwijdert of het formaat ervan wijzigt: meeverplaatsen en het formaat meewijzigen met de cellen, meeverplaatsen maar de grootte behouden, of op zijn plaats blijven. De keuze wordt in het bestand opgeslagen. **Dit object afdrukken** in hetzelfde venster schakelt het object uit op de afgedrukte pagina's (het blijft op het scherm zichtbaar); het bestand bewaart die keuze ook.

## Wijzigen wat een grafiek toont

Wanneer een grafiek is geselecteerd, verschijnen er twee knoppen naast en wordt het tabblad **Grafiek** op het lint getoond.

- De knop **+** (en **Grafiekonderdelen** op het tabblad) zet de titel, assen, astitels, rasterlijnen, legenda en gegevenslabels van de grafiek aan of uit, en geeft aan waar de legenda en de labels komen.
- De kwastknop (en **Kleuren** en **Stijlen** op het tabblad) geeft de grafiek een van de kleurensets van JeffCrunch en een van de uiterlijken. **Indelingen** stelt meerdere onderdelen tegelijk in.
- **Grafiekgegevens** toont de reeksen van de grafiek. U kunt ze toevoegen, bewerken, verwijderen en opnieuw ordenen, de cellen wijzigen die elke reeks leest, de cellen met de labels instellen, de rijen en kolommen omwisselen, en kiezen hoe lege cellen en verborgen rijen en kolommen in de grafiek worden weergegeven.
- **Veldknoppen** (in het menu van de **+**, alleen voor een draaigrafiek) toont kleine knoppen met de namen van de velden van de draaitabel die de grafiek volgt: de rapportfilters onder de titel, en de rijen, waarden en kolommen langs de onderkant. Schakel elke soort aan of uit, of kies **Alles tonen** of **Alles verbergen**. Een nieuwe draaigrafiek toont ze.
- **Foutbalken** (in het menu van de **+**, voor een kolom-, staaf-, lijn-, vlak- of spreidingsdiagram) tekenen een kleine lijn boven en onder elk punt om aan te geven hoeveel de werkelijke waarde kan afwijken. Kies **Standaardfout**, **Percentage (5%)**, **Standaarddeviatie** of **Vaste waarde (1)**. Kies **Aangepast** om eigen bedragen te gebruiken: typ de cellen met de bedragen boven elk punt (zoals `=Blad1!$D$2:$D$4`), en daarna de cellen voor de bedragen onder het punt. Laat er één leeg om de balken maar aan één kant te tekenen. De balken veranderen wanneer die cellen veranderen. Een bestand dat eigen bedragen per punt heeft, behoudt die.
- **Lijnen** (in het menu van de **+**, voor een lijndiagram) bevat **Daallijnen**, die van het hoogste punt van elke categorie naar beneden lopen tot de as, **Hoog-laag-lijnen**, die het hoogste en laagste punt van elke categorie verbinden, en **Omhoog/omlaag-staven**, dat zijn vakjes tussen de eerste en de laatste reeks van elke categorie. Een opwaartse staaf laat zien dat de laatste reeks hoger ligt. Hoog-laag-lijnen en omhoog/omlaag-staven hebben twee of meer reeksen nodig. Vlakdiagrammen hebben daallijnen.
- **Trendlijn** (in het menu van de **+**) tekent een lijn die op de getallen van elke reeks is aangepast, zodat u ziet waarheen ze gaan. Kies **Lineair** voor een rechte lijn, **Exponentieel** voor getallen die met een vast percentage groeien, **Logaritmisch** voor getallen die snel stijgen en daarna vlakker worden, **Polynoom** voor een kromme met bochten, **Macht**, of **Voortschrijdend gemiddelde**, dat het gemiddelde van de laatste paar getallen tekent. **Vergelijking tonen** schrijft de formule van de lijn op de grafiek, en **R-kwadraat tonen** schrijft hoe goed de lijn past (1 is een perfecte aansluiting; 0 is helemaal geen aansluiting). De trendlijn krijgt een eigen regel in de legenda.
- Wanneer een reeks tegen een tweede verticale as wordt getekend, hebben de menu's **Assen** en **Astitels** de items **Tweede horizontaal** en **Tweede verticaal** voor die reeks. Het menu **Legenda** heeft **Over de grafiek**, waarmee de legenda over het diagram wordt gelegd in plaats van ernaast.
- **Gegevenslabels** heeft ook **Label bevat** (de waarde, de naam van de categorie, de naam van de reeks, het percentage en de legenda-sleutel, dat is een klein vierkantje in de kleur van de reeks, in elke combinatie), **Scheidingsteken** (wat ertussen staat: een komma, puntkomma, punt, nieuwe regel of spatie) en **Getalnotatie…** (een notatiecode zoals 0,0 voor de getallen in de labels; laat dit leeg om de getallen zoals ze zijn te tonen). **Waarde uit cellen…** toont de woorden van sommige cellen als labels, één cel per punt. Typ de cellen, zoals `=Blad1!$D$2:$D$4`. De labels veranderen wanneer de cellen veranderen. Laat het leeg om ze weg te halen. Schakelt u alle onderdelen van de inhoud uit, dan verdwijnen de labels.
- **Horizontale as opmaken…** en **Verticale as opmaken…** op het tabblad **Grafiek** openen een venster voor één as. Hier stelt u de laagste en hoogste waarde en de hoofd- en subintervallen in (elk automatisch of een getal dat u typt), of de waarden in omgekeerde volgorde lopen, de weergave-eenheden (de labels tonen de getallen gedeeld, bijvoorbeeld door 1.000 voor duizendtallen, zodat 30.000 als 30 wordt weergegeven; vink **Naam van de eenheden op de grafiek weergeven** aan om de naam, zoals Duizendtallen, bij de astitel af te drukken), of de schaal logaritmisch is (elke streep is een grondtal maal de vorige, zoals 1, 10, 100 en 1000, wat geschikt is voor getallen die sterk in omvang verschillen en allemaal groter dan 0 moeten zijn), waar de andere as deze snijdt, de notatie van de labels, de streepjes, waar de labels staan en hoe ver de labels gedraaid zijn. Alleen een getalas heeft grenzen en intervallen.
- **Grafiek verplaatsen…** op het tabblad **Grafiek** vraagt om een ander werkblad en zet de grafiek daar, in dezelfde cellen. Ze leest nog steeds de cellen die ze las, en behoudt haar grootte en haar plek op het blad, ook als de rijen en kolommen daar hoger of breder zijn; ze verplaatst en wijzigt het formaat mee met de cellen daar, zoals voorheen. Verborgen bladen worden niet aangeboden. Het is één stap **Ongedaan maken**, ook nadat u hebt opgeslagen. Een grafiekblad (een blad dat alleen een grafiek is) kan nog niet worden gemaakt, en een grafiek die erop staat kan niet worden verplaatst.
- **Grafiektitel** en **Astitels** (in het menu van de **+**) hebben ook **Titeltekst…**. Typ de woorden die u wilt. Of typ een gelijkteken en een cel, zoals **=Blad1!A1**, dan toont de titel wat die cel bevat. Wanneer de cel verandert, verandert ook de titel. Het bestand bewaart de koppeling.
- **Opslaan als afbeelding…** op het tabblad **Grafiek** (en in het snelmenu met de rechtermuisknop) slaat de geselecteerde grafiek op als afbeeldingsbestand in **PNG**- of **JPEG**-indeling, tweemaal zo groot als op het blad, zodat hij scherp blijft.
- **Opslaan als sjabloon…** en **Sjabloon toepassen…** (op het tabblad **Grafiek**, in de groep **Type**) bewaren het uiterlijk van een grafiek in een **.crtx**-bestand en geven het aan een andere grafiek. Het uiterlijk omvat de soort, de kleuren, de instellingen van titel en legenda, de assen en de lettertypen. De grafiek waarop u het toepast, behoudt haar eigen getallen en de cellen die ze leest. Eén **Ongedaan maken** haalt het uiterlijk weer weg.
- **Grafiektype wijzigen** verandert de soort voor de hele grafiek, of voor elke reeks afzonderlijk: kolommen met een lijn erover, of een reeks tegen een tweede verticale as.

Een verwijzing naar cellen wordt geschreven als **Blad1!$B$2:$B$9**, met de naam van het blad tussen enkele aanhalingstekens als die spaties bevat.

## Sparklines

Een sparkline is een minigrafiek die in één cel past. Hij toont de trend van een rij of kolom getallen, direct naast die getallen.

1. Selecteer de getallen. Wilt u één sparkline per rij, selecteer dan meerdere rijen met getallen.
2. Kies op het tabblad **Invoegen**, in de groep **Sparklines**, **Lijn**, **Kolom** of **Winst/verlies**. Een **Lijn** verbindt de getallen met een lijn. Een **Kolom** tekent een staaf voor elk getal. Een **Winst/verlies** tekent een staaf omhoog voor elk getal boven nul en een staaf omlaag voor elk getal eronder, allemaal even groot; dat past bij resultaten die alleen goed of slecht zijn.
3. Controleer in het venster **Sparklines maken** het **Gegevensbereik** (de cellen die de sparklines lezen) en typ het **Locatiebereik** (de cellen waarin ze komen). Kies één locatiecel voor elke rij gegevens, of voor elke kolom gegevens. Kiest u één cel, dan leest de sparkline alle gegevens.

De sparklines die u in één stap maakt, vormen een groep en delen één uiterlijk. Klik op een cel met een sparkline, dan verschijnt het tabblad **Sparkline**.

- **Gegevens bewerken** wijzigt de gegevens en de locatie van de hele groep, of de gegevens van de sparkline waarop u klikte. Ook bepaalt het wat er gebeurt met lege cellen (een opening laten, ze als nul behandelen, of de punten erover heen verbinden) en of cellen in verborgen rijen en kolommen worden gelezen.
- **Type** schakelt tussen lijn, kolom en winst/verlies.
- **Weergeven** markeert punten: het **Hoogste punt**, het **Laagste punt**, **Negatieve punten**, het **Eerste punt**, het **Laatste punt** en, voor een lijn, **Markeringen** op elk punt.
- **Stijlen** geeft de groep een set kleuren. **Sparklinekleur** stelt de kleur van de lijn of staven in en de **Dikte** (hoe dik de lijn is, in punten). **Markeringskleur** stelt de kleur in van elke soort gemarkeerd punt.
- **As** toont een lijn bij nul, zet de getallen van rechts naar links uit, en stelt de laagste en hoogste waarde van de verticale as in. **Automatisch voor elke sparkline** past elke sparkline aan op zijn eigen getallen. **Hetzelfde voor alle sparklines** geeft de hele groep één schaal, zodat u ze kunt vergelijken. **Aangepaste waarde** laat u een getal typen. **Datumas…** verdeelt de punten op datum: typ de cellen met de datum van elk punt, en een punt met een latere datum staat verder naar rechts. Laat het leeg om de punten weer gelijkmatig te verdelen.
- **Groeperen** zet de geselecteerde sparklines in één groep. **Groep opheffen** geeft elke sparkline een eigen uiterlijk. **Wissen** verwijdert de geselecteerde sparklines, of hun hele groepen.

Een sparkline volgt zijn cellen: voegt u een rij boven de gegevens in, dan leest hij nog steeds dezelfde getallen. Elke wijziging is één stap die u ongedaan kunt maken. Sparklines worden in het bestand opgeslagen in een vorm die andere spreadsheetprogramma's lezen.

## Een afbeelding, vorm of tekstvak invoegen

Op het tabblad **Invoegen**, in de groep **Illustraties**, opent **Afbeelding** een bestandskiezer. Kies een afbeeldingsbestand (PNG, JPEG, GIF, BMP, TIFF of SVG) en het komt op het blad, bij de geselecteerde cel. Een afbeelding die breder is dan 480 pixels, wordt verkleind om te passen en behoudt haar vorm. De werkmap bewaart de afbeelding zelf. Een afbeelding van een soort die de werkmap niet zoals die is kan bewaren (een SVG, bijvoorbeeld), wordt bewaard als PNG, een gewone afbeelding van pixels. Een bestand dat geen afbeelding is, of groter is dan 16 MB, wordt geweigerd met een melding. Een WMF- of EMF-bestand (de afbeeldingssoorten die sommige oudere programma's maken van lijnen en vormen) wordt ook getekend en als PNG bewaard.

**Pictogrammen** toont hetzelfde venster met kleine lijnafbeeldingen dat JeffPub en JeffDeck hebben, met een vak om ze op naam te zoeken. Kies er een of meer, en elk komt op het blad als afbeelding, 48 punten breed en hoog. Ze worden donkergrijs getekend, op een formaat dat scherp blijft wanneer u ze vergroot. Elk is één stap die u ongedaan kunt maken.

**Vormen** opent een lijst: rechthoek, afgeronde rechthoek, ovaal, driehoek, ruit, pijlen, een ster, een hart, een wolk, een spraakballon en een lijn. Kies er een en hij komt op het blad in lichtblauw met een donkerdere rand. **Tekstvak**, in de groep **Tekst**, zet een leeg vak neer zonder vulling en zonder rand.

Wanneer een vorm is geselecteerd, geeft **Stijlen** op het tabblad **Vormopmaak** haar een kant-en-klare stijl: een vulling en een omtrek die bij elkaar passen, zoals lichtblauw met een donkerblauwe rand. De laatste twee houden alleen de omtrek of alleen de vulling. Eén klik is één stap die u ongedaan kunt maken. **Vormvulling** en **Vormomtrek** wijzigen elk onderdeel afzonderlijk.

Elke stap is één handeling die u ongedaan kunt maken. Het nieuwe object is geselecteerd, zodat u het kunt slepen, het formaat kunt wijzigen, het naar voren of naar achteren kunt verplaatsen, een naam en alternatieve tekst kunt geven in **Eigenschappen**, of op **Delete** kunt drukken.

## Wijzigen hoe een afbeelding, vorm of tekstvak eruitziet

Selecteert u een afbeelding, dan verschijnt het tabblad **Afbeeldingsopmaak**. Selecteert u een vorm, een lijn of een tekstvak, dan verschijnt het tabblad **Vormopmaak**. Elke wijziging is één stap die u ongedaan kunt maken, en het object behoudt zijn naam, zijn alternatieve tekst, zijn plaats en zijn volgorde ten opzichte van de andere objecten.

- **Vormvulling** en **Vormomtrek** kiezen een kleur, of geen. **Lijnstijl** stelt de dikte (hoe dik de lijn is) en de streepjes in (doorgetrokken, gestreept of gestippeld). Een afbeelding heeft **Afbeeldingsrand**, dat is de omtrek.
- **Draaien** draait het object een kwartslag naar rechts of links, of spiegelt het van links naar rechts of van boven naar beneden.
- **Bijsnijden** snijdt een afbeelding bij tot een venstervorm: vierkant, 4 breed bij 3 hoog, 3 breed bij 4 hoog, 16 bij 9 of 9 bij 16. Het bijsnijden gebeurt gelijkmatig vanuit het midden van wat zichtbaar is, en de afbeelding wordt zo groot gemaakt als wat overblijft. **De hele afbeelding tonen** haalt weer op wat was weggesneden. Het afbeeldingsbestand wordt niet gewijzigd: het weggesneden deel wordt alleen verborgen.
- **Tekst bewerken** is voor vormen en tekstvakken. Typ de woorden en kies hoe ze horizontaal en verticaal staan, de grootte, vet of normaal, en of ze om de vorm heen lopen of op één regel blijven.
- **Selectievenster** opent een lijst van elk object op het blad bij naam, het object vooraan bovenaan. Klik op een naam om dat object te selecteren; dat helpt wanneer objecten elkaar verbergen. **Naar voren** en **Naar achteren** verplaatsen het object in de lijst.
- **Eigenschappen** stelt de grootte, de naam, de alternatieve tekst en wat het object doet wanneer cellen worden verplaatst of vergroot, in.

## Een object knippen, kopiëren en plakken

Een afbeelding of vorm kopieert en plakt u op dezelfde manier als een grafiek. Een vorm die iets anders gebruikt dan een afbeelding uit de werkmap, zoals een koppeling naar een bestand, kan niet worden gekopieerd, en JeffCrunch laat u weten waarom.

Kopieert u een grafiek en plakt u die in dezelfde werkmap, op elk blad, dan leest de kopie dezelfde cellen als het origineel. Plakt u haar in een andere werkmap, dan behoudt ze de getallen die ze toonde, maar leest ze geen cellen meer, omdat de cellen die ze daar zou lezen niet dezelfde zijn als die waaruit ze is gemaakt.

## Meerdere objecten selecteren, groeperen en uitlijnen

Klik op één object en houd vervolgens **Shift** of **Ctrl** ingedrukt terwijl u op een ander klikt om dat ook te selecteren. Klik nogmaals op een geselecteerd object terwijl u **Shift** of **Ctrl** ingedrukt houdt om de selectie op te heffen. Geselecteerde objecten krijgen elk een gestreept kader. U kunt ook **Objecten selecteren** kiezen op het tabblad **Vormopmaak** en een vak slepen over lege cellen: elk object dat helemaal binnen het vak past, wordt geselecteerd. Druk op Esc, of kies **Objecten selecteren** opnieuw, om weer cellen te selecteren. Druk op **Delete** om alle geselecteerde objecten in één stap te verwijderen.

Wanneer twee of meer objecten zijn geselecteerd, staan op het tabblad **Vormopmaak**, in de groep **Schikken**, deze opdrachten:

- **Groeperen** voegt de geselecteerde vormen, afbeeldingen en lijnen samen tot één object. Het verplaatst en wijzigt het formaat als één geheel, en het bestand bewaart het als groep. Een grafiek kan niet in een groep worden geplaatst.
- **Groep opheffen** haalt een geselecteerde groep uit elkaar. Elk onderdeel komt terug op zijn plek op het blad.
- **Uitlijnen** lijnt de geselecteerde objecten uit: **Links uitlijnen**, **Centreren**, **Rechts uitlijnen**, **Boven uitlijnen**, **Midden uitlijnen** en **Onder uitlijnen**. Ze lijnen uit op de rand of het midden van de hele reeks. **Horizontaal verdelen** en **Verticaal verdelen** verdelen drie of meer objecten gelijkmatig tussen de twee aan de uiteinden.

Groeperen, Groep opheffen en elke uitlijningskeuze zijn één stap die u ongedaan kunt maken.

## Een naam geven

Wanneer een object is geselecteerd, toont het naamvak de naam ervan. Typ daar een nieuwe naam en druk op **Enter**. **Ongedaan maken** zet de oude naam terug, en de nieuwe naam wordt in het bestand opgeslagen.

## Verwijderen

Druk op **Delete** terwijl het object geselecteerd is. **Ongedaan maken** haalt het terug.

## Wat wordt opgeslagen

Alles hierboven wordt in het bestand opgeslagen in een vorm die andere spreadsheetprogramma's lezen, zodat een grafiek die u hier maakt, daar op dezelfde manier opent. Een grafiek die uit een ander programma komt met iets wat JeffCrunch niet kan behouden wanneer het een grafiek opnieuw wegschrijft (foutbalken, een driedimensionale grafiek), kan nog wel worden verplaatst, van formaat worden gewijzigd en verwijderd, maar de onderdelen en gegevens ervan worden hier niet gewijzigd.
