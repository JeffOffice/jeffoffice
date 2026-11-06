# Grafieken, sparklines, afbeeldingen en vormen
<!-- keywords: groeperen, groep opheffen, uitlijnen, links uitlijnen, verdelen, objecten selecteren, meerdere vormen, meerdere selecteren, selectiekader, grafiekfilters, reeksen verbergen, aanbevolen grafieken, grafieksuggesties, grafiek, diagram, kolom, staaf, lijn, cirkel, vlak, spreiding, combinatie, legenda, titel, rasterlijnen, gegevenslabels, grafiekgegevens, grafiektype, weergave, indelingen, verplaatsen, formaat wijzigen, afbeelding, vorm, object, reeks, trendlijn, vergelijking, R-kwadraat, tweede as, sparkline, sparklines, datumas, winst/verlies, minigrafiek, markeringen, naam, alternatieve tekst, logaritmisch, logaritmische schaal, opslaan als afbeelding, sjabloon, crtx, titeltekst, legenda-sleutel, selectievenster, vulling van vorm, omtrek van vorm, draaien, spiegelen, bijsnijden, tekst bewerken, randen van afbeelding, afbeelding invoegen, pictogrammen, pictogram, wmf, emf, vormstijlen, stijlen, vorm invoegen, tekstvak, vormen, pijl, ovaal, rechthoek, ster -->

Een grafiek is een afbeelding van getallen die aan die getallen gekoppeld blijft: wijzigt u een getal, dan verandert de grafiek mee. Afbeeldingen en vormen liggen op dezelfde manier over de cellen, maar ze zijn aan geen enkel getal gekoppeld.

## Een grafiek maken

1. Selecteer de getallen die u in een grafiek wilt weergeven. Heeft de eerste rij namen en de eerste kolom labels, selecteer die dan ook. JeffCrunch kijkt naar de eerste rij en kolom en gebruikt ze als namen en labels wanneer het woorden zijn (of datums, voor de labels).
2. Kies op het tabblad **Invoegen**, in **Grafieken**, een type: **Kolom**, **Staaf**, **Lijn**, **Cirkel**, **Vlak**, **Spreiding** of **Combinatie**. Het kleine pijltje naast een type geeft de varianten, zoals gestapelde kolommen, een lijn met markeringen, of een spreidingsdiagram met punten, met verbonden punten of met vloeiende lijnen.
3. Wilt u eerst alle typen zien, kies dan **Meer grafieken**. De lijst links toont elk type, getekend met uw cellen. Met **Rijen en kolommen omwisselen** draait u de reeksen om.

Weet u niet welk type u moet kiezen? Selecteer uw cellen en kies **Aanbevolen grafieken** op het tabblad **Invoegen**. JeffCrunch bekijkt de cellen en geeft een paar typen die erbij passen, het beste eerst. Staan bijvoorbeeld datums onder elkaar, dan wordt een lijn voorgesteld, en een kolom met enkele positieve getallen en labels levert een cirkel op. Elk type is getekend met uw cellen. Kies er een en klik op **OK**.

Selecteert u maar één cel, dan maakt JeffCrunch een grafiek van het cellenblok eromheen dat gegevens bevat.

Reeksen lopen langs de kolommen wanneer de selectie minstens zoveel rijen als kolommen heeft, en langs de rijen wanneer er meer kolommen dan rijen zijn. Een cirkel heeft één reeks. Een spreidingsdiagram neemt de horizontale waarden uit de eerste kolom.

Voorbeeld: u selecteert A1:C4, waarin A2:A4 Q1, Q2 en Q3 bevatten, B1 en C1 Oost en West, en de rest getallen. De grafiek heeft twee reeksen, Oost en West, en drie groepen kolommen, Q1 tot en met Q3.

## Een reeks voorlopig verbergen

Kies een grafiek. Naast de grafiek verschijnen drie kleine knoppen. De knop met de trechter is **Grafiekfilters**. Die toont de reeksen van de grafiek, elk met een vinkje. Haal het vinkje weg om een reeks uit de grafiek te laten. Zet het vinkje weer aan om de reeks op dezelfde plaats terug te zetten. **Ongedaan maken** werkt ook. Een grafiek houdt altijd minstens één reeks. Een verborgen reeks blijft in het bestand staan wanneer u opslaat en blijft verborgen wanneer u het bestand opent, zodat er niets verloren gaat. Zet het vinkje weer aan om de reeks terug te zetten. (Een ringdiagram toont bij het openen alle reeksen.)

## Verplaatsen en formaat wijzigen

Klik op een grafiek, afbeelding of vorm om die te selecteren. Sleep om te verplaatsen, of sleep een van de acht vierkantjes aan de rand om het formaat te wijzigen.

- Houd **Alt** ingedrukt terwijl u sleept om een rand uit te lijnen op de rand van een cel.
- Houd **Shift** ingedrukt terwijl u een hoek sleept om de verhoudingen te behouden.
- Met de pijltoetsen verplaatst u het object een paar pixels tegelijk. **Ctrl** met een pijltoets verplaatst het één pixel, **Shift** met een pijltoets maakt het groter of kleiner, en **Alt** met een pijltoets gaat naar de volgende rand van een cel.
- **Esc** tijdens het slepen zet het object terug op de oude plaats.

Elke verplaatsing of wijziging van het formaat is één stap die u ongedaan kunt maken. Klik met de rechtermuisknop op het object en kies **Eigenschappen** om de **Naam** in te typen (elk object op een blad heeft een eigen naam nodig), en de **Alternatieve tekst** (een korte zin die aan iemand die het object niet kan zien, vertelt wat het toont). In hetzelfde venster typt u ook het formaat en kiest u wat het object doet wanneer u cellen invoegt, verwijdert of het formaat ervan wijzigt: meeverplaatsen en meeschalen met de cellen, meeverplaatsen maar het formaat behouden, of op zijn plaats blijven. Die keuze wordt in het bestand opgeslagen. **Dit object afdrukken** in hetzelfde venster schakelt het object op de afgedrukte pagina's uit (op het scherm blijft het zichtbaar); ook die keuze wordt in het bestand bewaard.

## Wat een grafiek toont wijzigen

Wanneer een grafiek is geselecteerd, verschijnen er twee knoppen naast en wordt het tabblad **Grafiek** op het lint getoond.

- De knop **+** (en **Grafiekonderdelen** op het tabblad) zet de titel, assen, astitels, rasterlijnen, legenda en gegevenslabels van de grafiek aan of uit, en bepaalt waar de legenda en de labels komen.
- De kwastknop (en **Kleuren** en **Stijlen** op het tabblad) geeft de grafiek een van de kleurensets en een van de stijlen van JeffCrunch. **Indelingen** stelt meerdere onderdelen tegelijk in.
- **Grafiekgegevens** toont de reeksen van de grafiek. U kunt ze toevoegen, bewerken, verwijderen en opnieuw ordenen, de cellen wijzigen die elke reeks leest, de cellen met de labels instellen, de rijen en kolommen omwisselen en kiezen hoe lege cellen en verborgen rijen en kolommen in de grafiek worden weergegeven.
- **Foutbalken** (in het menu **+**, voor een kolom-, staaf-, lijn- of spreidingsgrafiek) tekenen een kleine lijn boven en onder elk punt, die laat zien hoeveel de werkelijke waarde kan afwijken. Kies **Standaardfout**, **Percentage (5%)**, **Standaarddeviatie** of **Vaste waarde (1)**. Een bestand dat eigen bedragen per punt heeft, behoudt die bedragen.
- **Lijnen** (in het menu **+**, voor een lijngrafiek) heeft **Daallijnen**, die van het hoogste punt van elke categorie naar beneden lopen tot de as, **Hoog-laag-lijnen**, die het hoogste en laagste punt van elke categorie verbinden, en **Omhoog/omlaag-staven**, die vakjes zijn tussen de eerste en de laatste reeks van elke categorie. Een omhoog-staaf toont dat de laatste reeks hoger ligt. Hoog-laag-lijnen en omhoog/omlaag-staven hebben minstens twee reeksen nodig. Vlakgrafieken hebben daallijnen.
- **Trendlijn** (in het menu **+**) tekent een lijn die op de getallen van elke reeks is aangepast, zodat u ziet waar ze naartoe gaan. Kies **Lineair** voor een rechte lijn, **Exponentieel** voor getallen die met een vast percentage groeien, **Logaritmisch** voor getallen die snel stijgen en daarna vlakker worden, **Polynoom** voor een kromme met bochten, **Macht**, of **Voortschrijdend gemiddelde**, dat het gemiddelde van de laatste paar getallen tekent. **Vergelijking weergeven** schrijft de formule van de lijn op de grafiek, en **R-kwadraat weergeven** schrijft hoe goed de lijn past (1 is een perfecte aansluiting; 0 is helemaal geen aansluiting). De trendlijn krijgt een eigen item in de legenda.
- Wanneer een reeks tegen een tweede verticale as wordt getekend, hebben de menu's **Assen** en **Astitels** de items **Tweede horizontaal** en **Tweede verticaal** voor die reeks. In het menu **Legenda** staat **Over de grafiek**, dat de legenda op het diagram zelf legt in plaats van ernaast.
- **Gegevenslabels** heeft ook **Label bevat** (de waarde, de naam van de categorie, de naam van de reeks, het percentage en de legenda-sleutel, een klein vierkantje in de kleur van de reeks, in elke combinatie), **Scheidingsteken** (wat ertussen staat: een komma, puntkomma, punt, nieuwe regel of spatie) en **Getalnotatie…** (een notatiecode zoals 0,0 voor de getallen in de labels; laat het veld leeg om de getallen zoals ze zijn te tonen). **Waarde uit cellen…** toont de woorden van sommige cellen als labels, één cel per punt. Typ de cellen, zoals =Blad1!$D$2:$D$4. De labels veranderen wanneer de cellen veranderen. Laat het veld leeg om ze weg te halen. Schakelt u alle inhoudsopties uit, dan verdwijnen de labels.
- **Horizontale as opmaken…** en **Verticale as opmaken…** op het tabblad **Grafiek** openen een venster voor één as. Hier stelt u de laagste en hoogste waarde in, en de hoofd- en subintervallen (elk automatisch of een getal dat u typt). Ook kiest u of de waarden in omgekeerde volgorde lopen, de weergave-eenheden (de labels tonen de getallen gedeeld, bijvoorbeeld door 1.000 voor duizendtallen, dus 30.000 wordt 30; vink **Naam van de eenheden op de grafiek weergeven** aan om de naam, zoals Duizendtallen, met de astitel af te drukken), of de schaal logaritmisch is (elke streep is een grondtal maal de vorige, zoals 1, 10, 100 en 1000, wat geschikt is voor getallen die sterk in grootte verschillen en allemaal groter dan 0 moeten zijn), waar de andere as deze as kruist, de getalnotatie van de labels, de streepjes en waar de labels staan, en hoeveel de labels zijn gedraaid. Alleen een getalas heeft grenzen en intervallen.
- **Grafiek verplaatsen…** op het tabblad **Grafiek** vraagt om een ander werkblad en zet de grafiek daar neer, in dezelfde cellen. De grafiek leest nog steeds dezelfde cellen en behoudt zijn formaat en zijn plek op het blad, ook wanneer de rijen en kolommen daar hoger of breder zijn; hij verplaatst en schaalt daar mee met de cellen zoals voorheen. Verborgen bladen worden niet aangeboden. Het is één stap **Ongedaan maken**, ook nadat u hebt opgeslagen. Een grafiekblad (een blad dat alleen uit een grafiek bestaat) kan nog niet worden gemaakt, en een grafiek die op zo'n blad staat, kan niet worden verplaatst.
- **Grafiektitel** en **Astitels** (in het menu **+**) hebben ook **Titeltekst…**. Typ de gewenste woorden. Of typ een gelijkteken en een cel, zoals **=Blad1!A1**, dan toont de titel wat die cel bevat. Verandert de cel, dan verandert de titel mee. Het bestand bewaart de koppeling.
- **Opslaan als afbeelding…** op het tabblad **Grafiek** (en in het rechtermuisknopmenu) slaat de geselecteerde grafiek op als afbeeldingsbestand in **PNG** of **JPEG**, twee keer zo groot als op het blad, zodat hij scherp blijft.
- **Opslaan als sjabloon…** en **Sjabloon toepassen…** (op het tabblad **Grafiek**, in de groep **Type**) bewaren de stijl van een grafiek in een **.crtx**-bestand en geven die aan een andere grafiek. De stijl omvat het type, de kleuren, de instellingen voor titel en legenda, de assen en de lettertypen. De grafiek waarop u de stijl toepast, behoudt zijn eigen getallen en de cellen die hij leest. Eén stap **Ongedaan maken** haalt de stijl weer weg.
- **Grafiektype** wijzigt het type voor de hele grafiek, of voor elke reeks afzonderlijk: kolommen met een lijn erboven, of een reeks tegen een tweede verticale as.

Een verwijzing naar cellen schrijft u als **Blad1!$B$2:$B$9**, met de naam van het blad tussen enkele aanhalingstekens wanneer die spaties bevat.

## Sparklines

Een sparkline is een minigrafiek die in één cel past. Hij toont de trend van een rij of kolom met getallen, direct naast die getallen.

1. Selecteer de getallen. Wilt u één sparkline per rij, selecteer dan meerdere rijen met getallen.
2. Kies op het tabblad **Invoegen**, in **Sparklines**, **Lijn**, **Kolom** of **Winst/verlies**. Een **Lijn** verbindt de getallen met een lijn. Een **Kolom** tekent een staaf voor elk getal. Een **Winst/verlies** tekent een staaf omhoog voor elk getal boven nul en een staaf omlaag voor elk getal eronder, allemaal even groot; dat is geschikt voor resultaten die alleen goed of slecht zijn.
3. Controleer in het venster **Sparklines maken** het **Gegevensbereik** (de cellen die de sparklines lezen) en typ het **Locatiebereik** (de cellen waarin ze komen). Kies één locatiecel voor elke rij gegevens, of voor elke kolom gegevens. Kiest u één cel, dan leest de sparkline alle gegevens.

De sparklines die u in één stap maakt, vormen een groep en delen één stijl. Klik op een cel met een sparkline en het tabblad **Sparkline** verschijnt.

- **Gegevens bewerken** wijzigt de gegevens en de locatie van de hele groep, of de gegevens van de sparkline die u hebt aangeklikt. Ook bepaalt u wat er met lege cellen gebeurt (een lege ruimte laten, ze als nul behandelen, of de punten erover heen verbinden) en of cellen in verborgen rijen en kolommen worden gelezen.
- **Type** schakelt tussen lijn, kolom en winst/verlies.
- **Tonen** markeert punten: het **Hoogste punt**, het **Laagste punt**, **Negatieve punten**, het **Eerste punt**, het **Laatste punt** en, voor een lijn, **Markeringen** op elk punt.
- **Stijlen** geeft de groep een set kleuren. **Sparklinekleur** stelt de kleur van de lijn of de staven in en de **Dikte** (hoe dik de lijn is, in punten). **Markeringskleur** stelt de kleur van elk soort gemarkeerd punt in.
- **As** toont een lijn bij nul, zet de getallen van rechts naar links en stelt de laagste en hoogste waarde van de verticale as in. **Automatisch voor elke sparkline** past elke sparkline aan op zijn eigen getallen. **Hetzelfde voor alle sparklines** geeft de hele groep één schaal, zodat u ze kunt vergelijken. **Aangepaste waarde** laat u een getal typen. **Datumas…** verdeelt de punten op datum: typ de cellen met de datum van elk punt, en een punt met een latere datum staat verder naar rechts. Laat het veld leeg om de punten weer gelijkmatig te verdelen.
- **Groeperen** zet de geselecteerde sparklines in één groep. **Groep opheffen** geeft elke sparkline zijn eigen stijl. **Wissen** haalt de geselecteerde sparklines weg, of hun hele groepen.

Een sparkline volgt zijn cellen: voegt u een rij boven de gegevens in, dan leest hij nog steeds dezelfde getallen. Elke wijziging is één stap die u ongedaan kunt maken. Sparklines worden in het bestand opgeslagen in een vorm die andere spreadsheetprogramma's lezen.

## Een afbeelding, een vorm of een tekstvak invoegen

Op het tabblad **Invoegen**, in **Illustraties**, opent **Afbeelding** een bestandskiezer. Kies een afbeeldingsbestand (PNG, JPEG, GIF, BMP, TIFF of SVG) en het komt op het blad, bij de geselecteerde cel. Is een afbeelding breder dan 480 pixels, dan wordt ze verkleind om te passen en behoudt ze haar vorm. De werkmap bewaart de afbeelding zelf. Een afbeelding van een soort dat de werkmap niet zoals het is kan bewaren (een SVG, bijvoorbeeld), wordt als PNG opgeslagen, een gewone afbeelding van punten. Een bestand dat geen afbeelding is, of groter is dan 16 MB, wordt geweigerd met een melding. Een WMF- of EMF-bestand (de afbeeldingssoorten die sommige oudere programma's van lijnen en vormen maken) wordt ook als PNG getekend en bewaard.

**Pictogrammen** toont hetzelfde venster met kleine lijnafbeeldingen dat JeffPub en JeffDeck hebben, met een vak om ze op naam te zoeken. Kies er een of meer en elk komt als afbeelding op het blad, 48 punten breed en hoog. Het wordt in donkergrijs getekend, op een formaat dat scherp blijft wanneer u het groter maakt. Elk pictogram is één stap die u ongedaan kunt maken.

**Vormen** opent een lijst: rechthoek, afgeronde rechthoek, ovaal, driehoek, ruit, pijlen, een ster, een hart, een wolk, een spraakballon en een lijn. Kies er een en hij komt op het blad in lichtblauw met een donkerdere rand. **Tekstvak**, in de groep **Tekst**, zet een leeg vak neer zonder vulling en zonder rand.

Wanneer een vorm is geselecteerd, geeft **Stijlen** op het tabblad **Vormopmaak** een kant-en-klare stijl: een vulling en een omtrek die bij elkaar passen, zoals lichtblauw met een donkerblauwe rand. De twee laatste behouden alleen de omtrek of alleen de vulling. Eén klik is één stap die u ongedaan kunt maken. **Vormvulling** en **Vormomtrek** wijzigen elk onderdeel afzonderlijk.

Elke handeling is één stap die u ongedaan kunt maken. Het nieuwe object is geselecteerd, zodat u het kunt slepen, het formaat ervan kunt wijzigen, het naar voren of naar achteren kunt verplaatsen, het een naam en alternatieve tekst kunt geven in **Eigenschappen**, of op **Delete** kunt drukken.

## Een afbeelding, vorm of tekstvak anders laten eruitzien

Selecteert u een afbeelding, dan verschijnt het tabblad **Afbeeldingsopmaak**. Selecteert u een vorm, een lijn of een tekstvak, dan verschijnt het tabblad **Vormopmaak**. Elke wijziging is één stap die u ongedaan kunt maken, en het object behoudt zijn naam, zijn alternatieve tekst, zijn plaats en zijn volgorde ten opzichte van de andere objecten.

- **Vormvulling** en **Vormomtrek** kiezen een kleur, of geen. **Lijnstijl** stelt de dikte (hoe dik de lijn is) en de streepjes in (doorgetrokken, gestreept of gestippeld). Een afbeelding heeft **Afbeeldingsrand**, dat de omtrek is.
- **Draaien** draait het object een kwartslag naar rechts of links, of spiegelt het van links naar rechts of van boven naar beneden.
- **Bijsnijden** snijdt een afbeelding bij tot een venstervorm: vierkant, 4 breed bij 3 hoog, 3 breed bij 4 hoog, 16 bij 9, of 9 bij 16. Het bijsnijden gebeurt gelijkmatig vanuit het midden van wat zichtbaar is, en de afbeelding wordt zo groot gemaakt als wat overblijft. **Hele afbeelding tonen** haalt het weggesneden deel weer terug. Het afbeeldingsbestand wordt niet gewijzigd: het weggesneden deel wordt alleen verborgen.
- **Tekst bewerken** is voor vormen en tekstvakken. Typ de woorden en kies hoe ze horizontaal en verticaal staan, de grootte, vet of normaal, en of ze binnen de vorm terugvloeien of op één regel blijven staan.
- **Selectiedeelvenster** opent een lijst van elk object op het blad bij naam, met het object dat vooraan staat bovenaan. Klik op een naam om dat object te selecteren; dat helpt wanneer objecten elkaar verbergen. **Naar voren** en **Naar achteren** verplaatsen het in de lijst.
- **Eigenschappen** stelt het formaat, de naam, de alternatieve tekst en wat het object doet wanneer cellen worden verplaatst of vergroot.

## Een object knippen, kopiëren en plakken

Een afbeelding of vorm wordt op dezelfde manier gekopieerd en geplakt als een grafiek. Een vorm die iets anders gebruikt dan een afbeelding uit de werkmap, zoals een koppeling naar een bestand, kan niet worden gekopieerd; JeffCrunch laat u weten waarom.

Kopieert u een grafiek en plakt u die in dezelfde werkmap, op elk blad, dan leest de kopie dezelfde cellen als het origineel. Plakt u die in een andere werkmap, dan behoudt de grafiek de getallen die hij toonde, maar leest hij geen cellen meer, omdat de cellen die hij daar zou lezen niet de cellen zijn waarop hij is gebaseerd.

## Meerdere objecten selecteren, groeperen en uitlijnen

Klik op een object en houd daarna Shift of Ctrl ingedrukt terwijl u op een ander klikt om dat ook te selecteren. Klik nogmaals op een geselecteerd object met Shift of Ctrl om het weer los te laten. Geselecteerde objecten krijgen elk een gestreept kader. U kunt ook **Objecten selecteren** kiezen op het tabblad **Vormopmaak** en een vak slepen over lege cellen: elk object dat volledig binnen het vak past, wordt geselecteerd. Druk op Esc, of kies **Objecten selecteren** nogmaals, om weer cellen te kunnen selecteren. Druk op Delete om alle geselecteerde objecten in één stap te verwijderen.

Wanneer er twee of meer objecten zijn geselecteerd, heeft het tabblad **Vormopmaak** deze opties in de groep **Schikken**:

- **Groeperen** voegt de geselecteerde vormen, afbeeldingen en lijnen samen tot één object. Het verplaatst en schaalt als één geheel, en het bestand bewaart het als groep. Een grafiek kan niet in een groep worden geplaatst.
- **Groep opheffen** haalt een geselecteerde groep uit elkaar. Elk onderdeel komt terug op zijn plaats op het blad.
- **Uitlijnen** lijnt de geselecteerde objecten uit: **Links uitlijnen**, **Centreren**, **Rechts uitlijnen**, **Boven uitlijnen**, **Midden uitlijnen** en **Onder uitlijnen**. Ze lijnen uit op de rand of het midden van de hele selectie. **Horizontaal verdelen** en **Verticaal verdelen** verdelen drie of meer objecten gelijkmatig tussen de twee uiterste.

Groeperen, Groep opheffen en elke uitlijningskeuze zijn elk één stap die u ongedaan kunt maken.

## Een naam geven

Wanneer een object is geselecteerd, toont het naamvak de naam ervan. Typ daar een nieuwe naam en druk op **Enter**. Ongedaan maken zet de oude naam terug, en de nieuwe naam wordt in het bestand opgeslagen.

## Verwijderen

Druk op **Delete** terwijl het object is geselecteerd. Ongedaan maken haalt het weer terug.

## Wat wordt opgeslagen

Alles hierboven wordt in het bestand opgeslagen in een vorm die andere spreadsheetprogramma's lezen, zodat een grafiek die u hier maakt, daar op dezelfde manier wordt geopend. Een grafiek die uit een ander programma komt met iets wat JeffCrunch niet kan behouden wanneer het een grafiek herschrijft (foutbalken, een driedimensionale grafiek), kan wel worden verplaatst, van formaat worden veranderd en worden verwijderd, maar de onderdelen en gegevens ervan worden hier niet gewijzigd.
