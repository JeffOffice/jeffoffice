# Grafieken, sparklines, afbeeldingen en vormen
<!-- keywords: ingesloten object, object invoegen, object uit een bereik, object uit een grafiek, object uit een pagina, plakken als object, lijst met objecten, object bewerken, veldknoppen, veldknoppen draaigrafiek, groeperen, groep opheffen, uitlijnen, links uitlijnen, verdelen, objecten selecteren, meerdere vormen, meerdere selecteren, selectiekader, grafiekfilters, reeks verbergen, aanbevolen grafieken, grafiekvoorstellen, grafiek, diagram, kolom, staaf, lijn, cirkel, vlak, spreiding, combinatie, radar, gevulde radar, spinnenwebdiagram, legenda, titel, rasterlijnen, gegevenslabels, grafiekgegevens, grafiektype, look, indelingen, verplaatsen, formaat wijzigen, afbeelding, vorm, object, reeks, trendlijn, vergelijking, R-kwadraat, secundaire as, sparkline, sparklines, datumas, winst/verlies, minigrafiek, markeringen, naam, alternatieve tekst, logaritmisch, logaritmische schaal, opslaan als afbeelding, sjabloon, crtx, titeltekst, legenda-sleutel, selectiedeelvenster, vulling van vorm, omtrek van vorm, draaien, spiegelen, bijsnijden, tekst bewerken, randen van afbeelding, afbeelding invoegen, pictogrammen, icoon, wmf, emf, vormstijlen, stijlen, vorm invoegen, tekstvak, vormen, pijl, ovaal, rechthoek, ster, diagram, diagrammen, online afbeeldingen, gratis afbeeldingen, openverse, wikimedia commons, licentie, bronvermelding, tekstopmaak, wordart, vervormen, afbeeldingen comprimeren, afbeeldingen kleiner maken, bestandsgrootte, ppi, bijgesneden gebieden verwijderen, streepjescode, QR-code, vorm koppelen aan cel, vorm toont cel, tekstvak toont cel, celwaarde in vorm -->

Een grafiek is een beeld van getallen dat aan die getallen gekoppeld blijft: verandert u een getal, dan verandert de grafiek mee. Afbeeldingen en vormen liggen op dezelfde manier over de cellen, maar ze zijn niet aan getallen gekoppeld.

## Een grafiek maken

1. Selecteer de getallen die u in een grafiek wilt weergeven. Heeft de eerste rij namen en de eerste kolom labels, selecteer die dan ook. JeffCrunch kijkt naar de eerste rij en kolom en gebruikt ze als namen en labels wanneer het woorden zijn (of datums, voor de labels).
2. Kies op het tabblad **Invoegen**, in de groep **Grafieken**, een type: **Kolom**, **Staaf**, **Lijn**, **Cirkel**, **Vlak**, **Spreiding**, **Combinatie** of **Radar**. De kleine pijl naast een type toont de varianten, zoals gestapelde kolommen, een lijn met markeringen, of een spreidingsdiagram met punten, met verbonden punten of met vloeiende lijnen. Een radardiagram tekent elk label als een spaak vanuit het midden, en elke reeks als een gesloten vorm door haar getallen op de spaken. Zo ziet u meerdere maatstaven van één zaak naast elkaar.
3. Kies voor een overzicht van alle typen eerst **Meer grafieken**. De lijst links toont elk type, getekend uit uw cellen. Met **Rijen en kolommen omwisselen** draait u de reeksen om.

Weet u niet welk type u moet kiezen? Selecteer uw cellen en kies **Aanbevolen grafieken** op het tabblad **Invoegen**. JeffCrunch bekijkt de cellen en geeft een paar types die erbij passen, het beste eerst. Datums onder elkaar leiden bijvoorbeeld tot een lijn, en één kolom met een paar positieve getallen en labels tot een cirkel. Elk type wordt getekend uit uw cellen. Kies er een en klik op **OK**.

Selecteert u maar één cel, dan maakt JeffCrunch een grafiek van het blok cellen eromheen dat gegevens bevat.

Reeksen lopen door de kolommen als de selectie minstens zoveel rijen als kolommen heeft, en langs de rijen als er meer kolommen dan rijen zijn. Een cirkel heeft één reeks. Een spreidingsdiagram neemt zijn horizontale waarden uit de eerste kolom.

Voorbeeld: u selecteert A1:C4, waarin A2:A4 Q1, Q2 en Q3 bevatten, B1 en C1 Oost en West, en de rest getallen. De grafiek heeft twee reeksen, Oost en West, en drie groepen kolommen, Q1 tot en met Q3.

## Een reeks tijdelijk verbergen

Selecteer een grafiek. Naast de grafiek verschijnen drie kleine knoppen. De knop met de trechter is **Grafiekfilters**. Die toont de reeksen van de grafiek, elk met een vinkje. Haal het vinkje weg om een reeks uit de grafiek te laten. Zet het vinkje weer terug om haar op dezelfde plek terug te laten komen. **Ongedaan maken** werkt ook. Een grafiek houdt altijd minstens één reeks. Een reeks die u hebt weggelaten, blijft in het bestand staan wanneer u opslaat, en blijft verborgen wanneer u het bestand opent, zodat er niets verloren gaat. Zet het vinkje terug om haar weer te tonen. (Een donutgrafiek toont bij het openen alle reeksen.)

## Verplaatsen en formaat wijzigen

Klik op een grafiek, afbeelding of vorm om die te selecteren. Sleep om te verplaatsen, of sleep aan een van de acht vierkantjes op de rand om het formaat te wijzigen.

- Houd **Alt** ingedrukt terwijl u sleept om een rand uit te lijnen op de rand van een cel.
- **Uitlijnen op raster** en **Uitlijnen op vorm** staan op het tabblad **Vormopmaak**, en worden voor elke werkmap onthouden. Met **Uitlijnen op raster** aan, gaat een rand die u sleept naar de dichtstbijzijnde rand van een cel. Met **Uitlijnen op vorm** aan, gaat een rand naar de rand van een ander object wanneer die er binnen 6 pixels van ligt. Alleen de eerste 2.000 andere objecten van een blad tellen mee voor **Uitlijnen op vorm**. Staan beide aan, dan winnen de randen van cellen, omdat een object aan cellen is verankerd, en worden de randen van de andere objecten niet bekeken. Een wijziging werkt meteen in elk geopend venster.
- **Alt** keert het uitlijnen om dat voor één sleepbeweging geldt: staat **Uitlijnen op raster** of **Uitlijnen op vorm** aan, dan schakelt **Alt** het uitlijnen voor die sleepbeweging uit (het wisselt nooit naar de andere soort), en staan beide uit, dan zorgt **Alt** dat de randen naar cellen gaan, zoals hierboven.
- Een gedraaid object wordt uitgelijnd op de omtrek die u ziet (het kleinste kader dat het omvat), en niet op het kader van vóór het draaien. Dat geldt ook voor de objecten waarop het uitlijnt.
- Houd **Shift** ingedrukt terwijl u een hoek sleept om de verhoudingen te behouden.
- De pijltoetsen verschuiven het object een paar pixels per keer. **Ctrl** met een pijltoets verschuift het één pixel, **Shift** met een pijltoets maakt het groter of kleiner, en **Alt** met een pijltoets gaat naar de volgende rand van een cel.
- **Esc** terwijl u sleept, zet het object terug.

Elke verplaatsing of wijziging van het formaat is één stap die u ongedaan kunt maken. Klik met de rechtermuisknop op het object en kies **Eigenschappen** om de **Naam** te typen (elk object op een blad heeft een eigen naam nodig) en de **Alternatieve tekst** (een korte zin die aan iemand die het object niet kan zien, vertelt wat het toont), om het formaat in te typen, en om te kiezen wat het doet wanneer u cellen invoegt, verwijdert of het formaat ervan wijzigt: meeverplaatsen en het formaat meewijzigen, meeverplaatsen maar het formaat behouden, of op zijn plaats blijven. Die keuze wordt in het bestand opgeslagen. **Dit object afdrukken** in hetzelfde vak schakelt het object op de afgedrukte pagina's uit (het blijft op het scherm zichtbaar); het bestand bewaart die keuze ook.

## Wijzigen wat een grafiek toont

Wanneer een grafiek is geselecteerd, verschijnen er twee knoppen naast en komt het tabblad **Grafiek** op het lint.

- De knop **+** (en **Grafiekonderdelen** op het tabblad) zet de titel, assen, astitels, rasterlijnen, legenda en gegevenslabels van de grafiek aan of uit, en geeft aan waar de legenda en de labels staan.
- De kwastknop (en **Kleuren** en **Look** op het tabblad) geeft de grafiek een van de kleurensets van JeffCrunch en een van de looks. **Indelingen** stelt meerdere onderdelen tegelijk in.
- **Grafiekgegevens** toont de reeksen van de grafiek. U kunt ze toevoegen, bewerken, verwijderen en opnieuw ordenen, de cellen wijzigen die elke reeks leest, de cellen met de labels instellen, rijen en kolommen omwisselen, en kiezen hoe lege cellen en verborgen rijen en kolommen in de grafiek worden weergegeven.
- **Veldknoppen** (in het menu **+**, alleen voor een draaigrafiek) toont kleine knoppen met de namen van de velden van de draaitabel die de grafiek volgt: de rapportfilters onder de titel, en de rijen, waarden en kolommen langs de onderkant. Zet elke soort aan of uit, of kies **Alles tonen** of **Alles verbergen**. Een nieuwe draaigrafiek toont ze.
- **Foutbalken** (in het menu **+**, voor een kolom-, staaf-, lijn-, vlak- of spreidingsgrafiek) tekenen een kleine lijn boven en onder elk punt om aan te geven hoeveel de werkelijke waarde kan afwijken. Kies **Standaardfout**, **Percentage (5%)**, **Standaarddeviatie** of **Vaste waarde (1)**. Kies **Aangepast** om eigen bedragen te gebruiken: typ de cellen met de bedragen boven elk punt (zoals `=Blad1!$D$2:$D$4`), en daarna de cellen voor de bedragen eronder. Laat er één leeg om de balken maar aan één kant te tekenen. De balken veranderen wanneer die cellen veranderen. Een bestand dat eigen bedragen per punt heeft, behoudt ze.
- **Lijnen** (in het menu **+**, voor een lijngrafiek) heeft **Druppellijnen**, die van het hoogste punt van elke categorie naar de as lopen, **Hoog-laag-lijnen**, die het hoogste en laagste punt van elke categorie verbinden, en **Op-neerstaven**, die vakjes zijn tussen de eerste en de laatste reeks van elke categorie. Een opstaaf laat zien dat de laatste reeks hoger is. Hoog-laag-lijnen en op-neerstaven hebben twee of meer reeksen nodig. Vlakgrafieken hebben druppellijnen.
- **Gegevenstabel** (in het menu **+**, voor een kolom-, lijn- of vlakgrafiek) zet een raster met de getallen onder de grafiek, met een kolom voor elke categorie en een rij voor elke reeks. De lijst ervan zet de tabel aan en uit, en stelt de lijnen tussen rijen, de lijnen tussen kolommen, de omtrek en de legenda-sleutels in (de kleur van elke reeks naast haar naam).
- **Trendlijn** (in het menu **+**) tekent een lijn die op de getallen van elke reeks is gepast, zodat u ziet waar ze naartoe gaan. Kies **Lineair** voor een rechte lijn, **Exponentieel** voor getallen die met een vast percentage groeien, **Logaritmisch** voor getallen die snel stijgen en dan vlakker worden, **Polynoom** voor een kromme met bochten, **Macht**, of **Voortschrijdend gemiddelde**, dat het gemiddelde van de laatste paar getallen tekent. **Vergelijking tonen** zet de formule van de lijn op de grafiek, en **R-kwadraat tonen** zet erbij hoe goed de lijn past (1 is een perfecte pasvorm; 0 is helemaal geen pasvorm). De trendlijn krijgt een eigen item in de legenda.
- Wanneer een reeks tegen een tweede verticale as is getekend, hebben de menu's **Assen** en **Astitels** de items **Tweede horizontale as** en **Tweede verticale as** voor die reeks. Het menu **Legenda** heeft **Over de grafiek**, dat de legenda op het vlak legt in plaats van ernaast.
- **Gegevenslabels** heeft ook **Label bevat** (de waarde, de naam van de categorie, de naam van de reeks, het percentage en de legenda-sleutel, een klein vierkantje in de kleur van de reeks, in elke combinatie), **Scheidingsteken** (wat ertussen staat: een komma, puntkomma, punt, nieuwe regel of spatie) en **Getalnotatie…** (een notatiecode zoals 0,0 voor de getallen in de labels; laat leeg om de getallen zoals ze zijn te tonen). **Waarde uit cellen…** toont de woorden van sommige cellen als labels, één cel voor elk punt. Typ de cellen, zoals `=Blad1!$D$2:$D$4`. De labels veranderen wanneer de cellen veranderen. Laat leeg om ze weg te halen. Zet alle inhoud uit en de labels verdwijnen.
- **Horizontale as opmaken…** en **Verticale as opmaken…** op het tabblad **Grafiek** openen een vak voor één as. Daarin stelt u de laagste en hoogste waarde en de hoofd- en subintervallen in (elk automatisch of een getal dat u typt), of de waarden in omgekeerde volgorde lopen, de weergave-eenheden (de labels tonen de getallen gedeeld, bijvoorbeeld door 1.000 voor duizendtallen, zodat 30.000 als 30 wordt gelezen; vink **Naam van de eenheden op de grafiek tonen** aan om de naam, zoals Duizendtallen, met de astitel af te drukken), of de schaal logaritmisch is (elke streep is een grondtal maal de vorige, zoals 1, 10, 100 en 1000, wat past bij getallen die sterk in omvang verschillen en allemaal groter dan 0 moeten zijn), waar de andere as deze snijdt, de getalnotatie van de labels, de streepjes, waar de labels staan en hoe ver de labels zijn gedraaid. Alleen een getalas heeft grenzen en eenheden.
- **Onderdeel van grafiek opmaken…** op het tabblad **Grafiek** (en in het snelmenu met de rechtermuisknop) stelt de vulling en de lijn in van de pagina van de grafiek, van het vlak (het kader waarin de markeringen staan), of van één reeks. Bij een lijn stelt het ook in of die vloeiend is en welke markering bij elk punt staat. Het snelmenu van een grafiek heeft ook **Rijen en kolommen omwisselen** en de twee asopdrachten.
- **Huidige selectie** toont de onderdelen van de grafiek: het grafiekgebied, het plotgebied en elke reeks. Kies er een, en **Selectie opmaken** opent het opmaakvenster bij dat onderdeel. **Terugzetten naar stijl** haalt weg wat op het gekozen onderdeel is ingesteld en geeft terug wat de stijl van de grafiek het geeft. Voor het grafiekgebied is dat de vulling, de lijn en de tekstkleur die een look heeft gezet (de lichte tekst van de donkere look gaat terug naar de tekstkleur van het thema). Voor het plotgebied zijn dat de vulling en de lijn. Voor een reeks is dat de kleur (die de keuze Kleuren of de stijl van de grafiek geeft), de omtrek, de markeringen zoals de soort grafiek die heeft, en de kleuren van haar punten; een vloeiende lijn blijft behouden wanneer het type grafiek een vloeiend type is. Titels, de legenda, labels, assen, rasterlijnen en de gegevens worden niet gewijzigd, en een onderdeel dat al overeenkomt, blijft ongewijzigd. Het is één stap van Ongedaan maken.
- **Grafiek verplaatsen…** op het tabblad **Grafiek** vraagt waar de grafiek heen moet. Kies **Nieuw blad** en typ een naam, dan krijgt de grafiek een eigen grafiekblad: een blad dat alleen een grafiek is, zonder cellen. Kies **Object in** en selecteer een werkblad, dan komt de grafiek op dat werkblad. Een grafiek op een grafiekblad kan alleen naar een werkblad worden verplaatst, en het grafiekblad dat achterblijft, wordt verwijderd. De grafiek leest nog steeds de cellen die hij las. Op een werkblad behoudt hij zijn formaat en zijn plaats op het blad, ook waar de rijen en kolommen hoger of breder zijn, en hij verplaatst en wijzigt het formaat mee met de cellen daar, zoals voorheen. Verborgen bladen worden niet aangeboden. Het is één **Ongedaan maken**, ook na het opslaan, tenzij door de verplaatsing een blad is verwijderd. Een bestand in de vorm `.ods` kan geen grafiekblad bewaren, dus sla een grafiekblad op in de vorm `.xlsx`.
- **Grafiektitel** en **Astitels** (in het menu **+**) hebben ook **Titeltekst…**. Typ de woorden die u wilt. Of typ een gelijkteken en een cel, zoals **=Blad1!A1**, dan toont de titel wat die cel bevat. Wanneer de cel verandert, verandert ook de titel. Het bestand bewaart de koppeling.
- **Opslaan als afbeelding…** op het tabblad **Grafiek** (en in het snelmenu met de rechtermuisknop) slaat de geselecteerde grafiek op als afbeeldingsbestand **PNG** of **JPEG**, twee keer zo groot als op het blad, zodat hij scherp blijft.
- **Opslaan als sjabloon…** en **Sjabloon toepassen…** (op het tabblad **Grafiek**, in de groep **Type**) bewaren de look van een grafiek in een **.crtx**-bestand en geven die aan een andere grafiek. De look omvat het type, de kleuren, de instellingen voor titel en legenda, de assen en de lettertypen. De grafiek waarop u hem toepast, behoudt zijn eigen getallen en de cellen die hij leest. Eén **Ongedaan maken** haalt de look weer weg.
- **Grafiektype wijzigen** verandert het type voor de hele grafiek, of voor elke reeks afzonderlijk: kolommen met een lijn erboven, of een reeks tegen een tweede verticale as.

Een verwijzing naar cellen wordt geschreven als **Blad1!$B$2:$B$9**, met de naam van het blad tussen enkele aanhalingstekens wanneer die spaties bevat.

## Sparklines

Een sparkline is een minigrafiek die in één cel past. Hij toont naast de getallen de trend van een rij of een kolom met getallen.

1. Selecteer de getallen. Voor één sparkline per rij selecteert u meerdere rijen met getallen.
2. Kies op het tabblad **Invoegen**, in de groep **Sparklines**, **Lijn**, **Kolom** of **Winst/verlies**. Een **Lijn** verbindt de getallen met een lijn. Een **Kolom** tekent een staaf voor elk getal. Een **Winst/verlies** tekent een staaf omhoog voor elk getal boven nul en een staaf omlaag voor elk getal eronder, allemaal even groot, wat past bij resultaten die alleen goed of slecht zijn.
3. Controleer in het vak **Sparklines maken** het **Gegevensbereik** (de cellen die de sparklines lezen) en typ het **Locatiebereik** (de cellen waarin ze komen). Kies één locatiecel voor elke rij gegevens, of voor elke kolom gegevens. Kiest u één cel, dan leest de sparkline alle gegevens.

De sparklines die u in één stap maakt, vormen een groep en delen één look. Klik op een cel met een sparkline, dan verschijnt het tabblad **Sparkline**.

- **Gegevens bewerken** wijzigt de gegevens en de locatie van de hele groep, of de gegevens van degene die u hebt aangeklikt. Ook bepaalt het wat er met lege cellen gebeurt (een lege ruimte laten, ze als nul behandelen, of de punten erover heen verbinden) en of cellen in verborgen rijen en kolommen worden gelezen.
- **Type** wisselt tussen lijn, kolom en winst/verlies.
- **Tonen** markeert punten: het **Hoogste punt**, het **Laagste punt**, **Negatieve punten**, het **Eerste punt**, het **Laatste punt** en, voor een lijn, **Markeringen** bij elk punt.
- **Stijlen** geeft de groep een set kleuren. **Sparklinekleur** stelt de kleur van de lijn of de staven in, en de **Dikte** (hoe dik de lijn is, in punten). **Markeringskleur** stelt de kleur van elke soort gemarkeerd punt in.
- **As** toont een lijn bij nul, zet de getallen van rechts naar links uit, en stelt de laagste en hoogste waarde van de verticale as in. **Automatisch voor elke sparkline** past elke sparkline aan op zijn eigen getallen. **Dezelfde voor alle sparklines** geeft de hele groep één schaal, zodat u ze kunt vergelijken. **Aangepaste waarde** laat u een getal typen. **Datumas…** verdeelt de punten naar datum: typ de cellen met de datum van elk punt, en een punt met een latere datum staat verder naar rechts. Laat leeg om de punten weer gelijkmatig te verdelen.
- **Groeperen** zet de geselecteerde sparklines in één groep. **Groep opheffen** geeft elk zijn eigen look. **Wissen** haalt de geselecteerde sparklines weg, of hun hele groepen.

Een sparkline volgt zijn cellen: voegt u een rij boven de gegevens in, dan leest hij nog steeds dezelfde getallen. Elke wijziging is één stap die u ongedaan kunt maken. Sparklines worden in het bestand opgeslagen in de vorm die andere spreadsheetprogramma's lezen.

## Een afbeelding, vorm of tekstvak invoegen

Op het tabblad **Invoegen**, in de groep **Illustraties**, opent **Afbeelding** een bestandskiezer. Kies een afbeeldingsbestand (PNG, JPEG, GIF, BMP, TIFF of SVG), dan komt het op het blad bij de geselecteerde cel. Een afbeelding die breder is dan 480 pixels, wordt verkleind om erin te passen, en behoudt haar verhoudingen. De werkmap bewaart de afbeelding zelf. Een afbeelding van een soort die de werkmap niet zoals die is kan bewaren (een SVG, bijvoorbeeld), wordt als PNG bewaard, een gewone afbeelding van pixels. Een bestand dat geen afbeelding is, of groter is dan 16 MB, wordt geweigerd met een melding. Een WMF- of EMF-bestand (de afbeeldingssoorten die sommige oudere programma's maken van lijnen en vormen) wordt ook getekend en als PNG bewaard.

**Pictogrammen** toont hetzelfde venster met kleine lijnafbeeldingen dat JeffPub en JeffDeck hebben, met een vak om ze op naam te zoeken. Kies er een of meer, en elk komt als afbeelding op het blad, 48 punten breed en hoog. Hij is donkergrijs getekend, op een formaat dat scherp blijft wanneer u hem vergroot. Elk is één stap die u ongedaan kunt maken.

**Streepjescode** opent een kort menu met twee keuzes. **Streepjescode** opent een venster om één streepjescode te maken (**Streepjescodes uit cellen**, hieronder, maakt er veel): boek-ISBN's (met of zonder prijs), EAN-13, UPC-A, EAN-8, Code 128, Code 39, of een QR-code, die elke tekst kan bevatten, zoals een webadres. Kies een **Type**, typ de tekst of de cijfers, en het venster toont de streepjescode terwijl u typt, en zegt in rood wat er niet klopt als hij niet gemaakt kan worden. Klik op **Invoegen**, dan komt hij als afbeelding op het blad, één stap die u ongedaan kunt maken. Verplaats hem en wijzig zijn formaat zoals bij elke afbeelding. Om hem te wijzigen, selecteert u hem en klikt u opnieuw op **Streepjescode**, past u de tekst aan en klikt u op **Bijwerken**; de nieuwe streepjescode neemt de plaats en de breedte van de oude over. Wanneer u opslaat, wordt hij als afbeelding opgeslagen met een SVG die scherp blijft, zodat elk programma hem toont, en JeffCrunch bewaart zijn instellingen in het bestand, zodat het een streepjescode blijft die u kunt bewerken wanneer u de werkmap opnieuw in JeffCrunch opent.

**Streepjescode > Streepjescodes uit cellen** (in hetzelfde menu **Streepjescode**) maakt een streepjescode voor elke gevulde cel van de geselecteerde cellen, voor een vel etiketten. Selecteer de cellen, klik erop en kies daarna het type en de weergave in hetzelfde venster. Wat u daar typt, is alleen een voorbeeld, want de eigen tekst van elke cel wordt de streepjescode. De streepjescodes komen in een kolom direct rechts van de geselecteerde cellen, de ene onder de andere, als één stap die u ongedaan kunt maken. Een cel waarvan de tekst de code niet kan bevatten (een letter in een EAN-13, bijvoorbeeld), wordt in een melding genoemd en krijgt geen streepjescode. Er worden tegelijk ten hoogste 500 cellen verwerkt; selecteer er minder als er staat dat het er te veel zijn. Elke streepjescode onthoudt de cel waaruit hij komt en volgt die: wanneer u die cel wijzigt, wordt de streepjescode opnieuw gemaakt uit de nieuwe tekst, op dezelfde plek en in dezelfde grootte, als een eigen stap die u ongedaan kunt maken. De koppeling verschuift mee met de cel wanneer u rijen of kolommen invoegt of verwijdert. Als u de cel verwijdert, behoudt de streepjescode de laatste tekst die hij had. Wanneer u opslaat, wordt de afbeelding in het bestand gemaakt uit de tekst van de cel zoals die op dat moment is, zodat ook andere programma's de actuele waarde tonen.

**Vormen** opent een lijst: rechthoek, afgeronde rechthoek, ovaal, driehoek, ruit, pijlen, een ster, een hart, een wolk, een spraakballon en een lijn. Kies er een, en hij komt op het blad in lichtblauw met een donkerdere rand. **Tekstvak**, in de groep **Tekst**, zet een leeg vak neer zonder vulling en zonder rand. Klik op **Tekstvak tekenen** (de hoofdknop, of de eerste keuze in het menu), en sleep daarna op het blad om het vak te tekenen. Een slepende beweging maakt een vak van het formaat dat u sleept, maar nooit kleiner dan 18 punten (0,6 cm) aan een zijde, en een klik van minder dan 4 punten geeft het gebruikelijke formaat. Houd **Shift** ingedrukt om het vak vierkant te houden. De aanwijzer is een kruis tot u loslaat; **Esc**, of een andere opdracht, legt het hulpmiddel weer neer. Het vak is één stap die u ongedaan kunt maken.

Om de tekst van een tekstvak te draaien, selecteert u het en klikt u op **Tekstrichting** op het tabblad **Vormopmaak**: **Horizontaal**, **Alle tekst 90° draaien**, of **Alle tekst 270° draaien**. De tekst wordt gedraaid getypt, zoals hij wordt getoond, en een werkmap die als .xlsx- of .ods-bestand is opgeslagen, behoudt de richting.

Met een vorm geselecteerd geeft **Stijlen** op het tabblad **Vormopmaak** er een kant-en-klare look aan: een vulling en een omtrek die bij elkaar passen, zoals lichtblauw met een donkerblauwe rand. De laatste twee houden alleen de omtrek of alleen de vulling. Eén klik is één stap die u ongedaan kunt maken. **Vulling van vorm** en **Omtrek van vorm** wijzigen elk onderdeel apart.

## Gratis afbeeldingen online zoeken

**Online afbeeldingen**, in de groep **Illustraties** op het tabblad **Invoegen**, zoekt gratis afbeeldingen. Ze komen uit twee open bibliotheken: Openverse, dat in veel collecties zoekt, en Wikimedia Commons, de bibliotheek met de afbeeldingen van Wikipedia.

1. Klik op **Online afbeeldingen…**. Rechts opent een deelvenster.
2. Kies een bibliotheek, typ waar u naar zoekt, en klik op **Zoeken** (of druk op Enter).
3. Klik op een afbeelding om te zien wie hem heeft gemaakt en welke licentie hij heeft. Een **licentie** zegt hoe u een afbeelding mag gebruiken. Klik op de naam van de licentie om haar te lezen.
4. Klik op **Invoegen** (of dubbelklik op de afbeelding).

**Wat JeffCrunch verstuurt.** Uw zoekwoorden gaan naar de bibliotheek die u kiest, en alleen wanneer u op **Zoeken** klikt of op Enter drukt. JeffCrunch vraagt de bibliotheek daarna om de kleine voorbeelden van wat is gevonden, en om de ene afbeelding die u invoegt. Zoals elke webpagina kan de bibliotheek het internetadres van uw computer zien, en wordt zij verteld dat JeffCrunch het vraagt. Niets uit uw werkmap wordt verstuurd.

De afbeelding komt op het blad zoals een afbeelding uit een bestand, en de werkmap bewaart haar. Haar titel wordt opgeslagen als haar **alternatieve tekst**. De meeste licenties vragen u de maker te noemen. Wanneer **Een bronvermelding onder de afbeelding toevoegen** is aangevinkt, voegt JeffCrunch onder de afbeelding een klein tekstvak toe met de titel, de maker en de licentie. U kunt het verplaatsen, wijzigen of verwijderen. De afbeelding en de bronvermelding komen in één stap, zodat **Ongedaan maken** beide terugneemt. Controleer of de licentie past bij hoe u de afbeelding gaat gebruiken.

## Tekstopmaak toevoegen

**Tekstopmaak** zijn opvallende woorden: letters met kleur, een omtrek, een schaduw of een gebogen vorm.

1. Klik op **Invoegen > Tekst > Tekstopmaak**. Een galerie met stijlen opent.
2. Klik op een stijl. De woorden "Uw tekst hier" komen op het blad in die stijl.
3. Wilt u de woorden wijzigen, selecteer dan de tekstopmaak en klik op **Tekst bewerken…** op het tabblad **Vormopmaak**. Een vak vraagt om de woorden, het lettertype, de grootte, vet en cursief. Een grotere grootte maakt de tekstopmaak groter.
4. Op het tabblad **Vormopmaak** geeft **Stijlen voor tekstopmaak** de letters een andere stijl, en **Tekst vervormen** buigt de woorden in een vorm, zoals een golf of een boog. "Vlak" houdt ze recht.

Tekstopmaak is een vorm in de tekening van het blad, met haar vervorming, vulling en omtrek, dus ze behoudt haar uiterlijk wanneer u de werkmap als .xlsx-bestand opslaat en weer opent, in JeffCrunch of in een ander programma. Elke wijziging is één stap die u ongedaan kunt maken. Een werkmap die als .ods-bestand is opgeslagen, behoudt ook haar afbeeldingen, vormen, tekstvakken, groepen, lijnen en tekstopmaak. Tekstopmaak wordt opgeslagen als de fontwork-vorm die OpenDocument heeft, met haar woorden, lettertype, vulling, omtrek en vervorming, en komt weer als tekstopmaak tevoorschijn wanneer u het .ods-bestand opnieuw opent. Een paar vervormingen waarvoor OpenDocument geen vorm heeft (Golf 2, Golf 4, en de onderste en bovenste vormen van Opblazen en Leeglopen) worden opgeslagen als rechte woorden met dezelfde letters en stijl. Grafieken worden ook in een .ods-bestand bewaard: zie hieronder.

## Afbeeldingen kleiner maken

Een foto van een camera kan veel groter zijn dan hij op het blad wordt getoond, en dan maakt hij het bestand groot. Klik op een afbeelding en klik daarna op **Afbeeldingen comprimeren…** in de groep **Aanpassen** op het tabblad **Afbeeldingsopmaak**. Kies waarvoor de werkmap wordt gebruikt: **Commercieel drukken (300 ppi)**, **Desktopdrukken (220 ppi)** of **Web (96 ppi)**. Laat **Toepassen op alle afbeeldingen op dit blad** aangevinkt om elke afbeelding op het blad kleiner te maken, of haal het vinkje weg om alleen de geselecteerde afbeelding kleiner te maken. **Bijgesneden gebieden van afbeeldingen verwijderen** haalt ook de verborgen delen weg die u hebt bijgesneden; het bijsnijden is dan weg. Een afbeelding wordt alleen vervangen wanneer de nieuwe kleiner is. Haar formaat, plaats, naam en alternatieve tekst blijven zoals ze waren. Ongedaan maken (Ctrl+Z) geeft de afbeeldingen terug, en de opgeslagen werkmap bevat alleen de kleinere afbeeldingen. Afbeeldingen op andere bladen worden niet gewijzigd: ga naar elk blad en doe het opnieuw. Een afbeelding die een tekening is (SVG, EMF of WMF), of een animatie, blijft zoals ze is.

Elke compressie is één stap die u ongedaan kunt maken. Het nieuwe object is geselecteerd, zodat u het kunt slepen, het formaat kunt wijzigen, het naar voren of naar achteren kunt zetten, het een naam en alternatieve tekst kunt geven in **Eigenschappen**, of op **Delete** kunt drukken.

## De tekst van een cel tonen in een vorm of tekstvak

Een vorm of tekstvak kan tonen wat een cel toont, en die cel volgen. Selecteer de vorm, klik in de **formulebalk**, typ een gelijkteken en een cel, en druk op **Enter**.

- `=B2` koppelt de vorm aan B2 op dit blad. `=Blad2!B3` koppelt hem aan een cel op een ander blad. `=Totaal` koppelt hem aan een naam die voor één cel staat.
- De vorm toont dan de tekst van de cel zoals de cel die toont, met haar getalnotatie. Wanneer u de cel wijzigt, de opmaak ervan, of een formule waarvan die afhangt, verandert ook de vorm. Tekst die langer is dan een cel kan bevatten (32.767 tekens), wordt afgekapt. Alle gekoppelde vormen van een werkmap tonen samen hoogstens 4 MB aan tekst: daarboven toont een vorm een afgekapte tekst.
- Met de vorm geselecteerd toont de formulebalk de koppeling, zoals `=Blad1!$B$2`. Wis de formulebalk en druk op **Enter** om de koppeling weg te halen. De vorm behoudt dan de woorden die hij toont.
- Woorden typen in de vorm haalt de koppeling ook weg.
- Een koppeling is één cel. Een bereik zoals `=A1:B2`, een lijst, een berekening, of een naam die geen cel is, wordt geweigerd met een melding, en de vorm behoudt zijn koppeling.
- Wordt de rij of kolom van de cel verwijderd, of het blad ervan, dan toont de vorm `#VERW!` en behoudt hij de koppeling, zoals een formule zou doen. Ongedaan maken brengt de cel terug.
- Elke koppeling, en elke keer dat u er een weghaalt, is één stap die u ongedaan kunt maken.

De koppeling wordt in een .xlsx-bestand opgeslagen met de tekst die de cel toonde, zodat een programma dat koppelingen niet volgt, de woorden nog steeds toont, en JeffCrunch opent het bestand met de koppeling werkend. OpenDocument kent geen manier om een tekstvak aan een cel te koppelen, dus een werkmap die als .ods-bestand is opgeslagen, bewaart de woorden die de vorm op dat moment toont, en niet de koppeling.

## De look van een afbeelding, vorm of tekstvak wijzigen

Selecteer een afbeelding en het tabblad **Afbeeldingsopmaak** verschijnt. Selecteer een vorm, lijn of tekstvak en het tabblad **Vormopmaak** verschijnt. Elke wijziging is één stap die u ongedaan kunt maken, en het object behoudt zijn naam, alternatieve tekst, plaats en volgorde ten opzichte van de andere objecten.

- **Vulling van vorm** en **Omtrek van vorm** kiezen een kleur, of geen. **Lijnstijl** stelt de dikte (hoe dik de lijn is) en de streepjes in (doorgetrokken, gestreept of gestippeld). Een afbeelding heeft **Randen van afbeelding**, dat is haar omtrek.
- **Draaien** draait het object een kwartslag naar rechts of links, of spiegelt het van links naar rechts of van boven naar beneden.
- **Bijsnijden**, voor een afbeelding, snijdt haar bij tot een venstervorm: vierkant, 4 breed bij 3 hoog, 3 breed bij 4 hoog, 16 bij 9, of 9 bij 16. De snede wordt gelijkmatig vanuit het midden van wat zichtbaar is gemaakt, en de afbeelding wordt zo groot gemaakt als wat overblijft. **Volledige afbeelding tonen** brengt terug wat is bijgesneden. **Vullen** houdt het kader en snijdt de afbeelding zo bij dat ze het hele kader bedekt. **Passend** toont de hele afbeelding, en het kader krijgt de vorm van de afbeelding binnen het oude kader. **Bijsnijden tot vorm** (de galerie naast **Bijsnijden**) snijdt de afbeelding bij tot een vorm zoals een ovaal, een ster of een afgeronde rechthoek; de vorm wordt met de afbeelding opgeslagen. Het afbeeldingsbestand wordt niet gewijzigd: het afgesneden deel wordt alleen verborgen.
- **Tekst bewerken** is voor vormen en tekstvakken. Typ de woorden, en kies hoe ze horizontaal en verticaal staan, de grootte, vet of normaal, en of ze binnen de vorm terugvloeien of op één regel blijven. Voor tekstopmaak vraagt het in plaats daarvan om de woorden, het lettertype, de grootte, vet en cursief.
- **Selectiedeelvenster** opent een lijst van elk object op het blad bij naam, het voorste eerst. Klik op een naam om dat object te selecteren; dat helpt wanneer objecten elkaar verbergen. **Naar voren** en **Naar achteren** verplaatsen het in de lijst.
- **Eigenschappen** stelt het formaat, de naam, de alternatieve tekst en wat het object doet wanneer cellen worden verplaatst of groter gemaakt, in.

## Een object knippen, kopiëren en plakken

Een afbeelding of vorm wordt op dezelfde manier gekopieerd en geplakt als een grafiek. Een vorm die iets anders gebruikt dan een afbeelding uit de werkmap, zoals een koppeling naar een bestand, kan niet worden gekopieerd, en JeffCrunch vertelt u waarom.

Kopieer een grafiek en plak hem in dezelfde werkmap, op elk blad: de kopie leest dezelfde cellen als het origineel. Plakt u hem in een andere werkmap, dan behoudt hij de getallen die hij toonde, maar leest hij geen cellen meer, omdat de cellen die hij daar zou lezen niet de cellen zijn waaruit hij is gemaakt.

## Meerdere objecten selecteren, groeperen en uitlijnen

Klik op één object en houd daarna Shift of Ctrl ingedrukt terwijl u op een ander klikt om dat ook te selecteren. Klik met Shift of Ctrl nogmaals op een geselecteerd object om het los te laten. Geselecteerde objecten krijgen elk een gestreept kader. U kunt ook **Objecten selecteren** kiezen op het tabblad **Vormopmaak** en over lege cellen een vak slepen: elk object dat helemaal binnen het vak past, wordt geselecteerd. Druk op Esc, of kies **Objecten selecteren** opnieuw, om weer cellen te selecteren. Druk op Delete om alle geselecteerde objecten in één stap te verwijderen.

Wanneer er twee of meer zijn geselecteerd, heeft het tabblad **Vormopmaak** deze opdrachten in de groep **Schikken**:

- **Groeperen** voegt de geselecteerde vormen, afbeeldingen en lijnen samen tot één object. Het verplaatst en wijzigt het formaat als één geheel, en het bestand bewaart het als groep. Een grafiek kan niet in een groep worden geplaatst.
- **Groep opheffen** haalt een geselecteerde groep uit elkaar. Elk onderdeel komt terug op zijn plaats op het blad.
- **Uitlijnen** lijnt de geselecteerde objecten uit: **Links uitlijnen**, **Centreren**, **Rechts uitlijnen**, **Boven uitlijnen**, **Midden uitlijnen** en **Onder uitlijnen**. Ze lijnen uit op de rand of het midden van de hele set. **Horizontaal verdelen** en **Verticaal verdelen** verdelen drie of meer objecten gelijkmatig tussen de twee aan de uiteinden.

Groeperen, Groep opheffen en elke uitlijnkeuze zijn één stap die u ongedaan kunt maken.

## Een naam geven

Met een object geselecteerd toont het naamvak zijn naam. Typ daar een nieuwe naam en druk op **Enter**. Ongedaan maken zet de oude naam terug, en de nieuwe naam wordt in het bestand opgeslagen.

## Verwijderen

Druk op **Delete** met het object geselecteerd. Ongedaan maken brengt het terug.

## Wat wordt opgeslagen

Alles hierboven wordt in het bestand opgeslagen in de vorm die andere spreadsheetprogramma's lezen, zodat een grafiek die u hier maakt, daar op dezelfde manier opent. Een grafiek die uit een ander programma komt, met iets wat JeffCrunch niet kan bewaren wanneer het een grafiek herschrijft (foutbalken, een driedimensionale grafiek), kan nog steeds worden verplaatst, van formaat veranderd en verwijderd, maar de onderdelen en gegevens ervan worden hier niet gewijzigd.

## Een object invoegen

Een **object** bevat zijn eigen bestand in de werkmap, zodat de app die het heeft gemaakt, het weer kan openen. Het toont een beeld van zichzelf, maar het is geen afbeelding: het blijft een object wanneer u het verplaatst, van formaat verandert, kopieert of de werkmap opslaat.

1. Klik op **Invoegen**, dan op **Object**, en kies **Uit JeffCrunch-bereik**, **Uit JeffCrunch-grafiek** of **Uit JeffWrite-pagina**.
2. Kies het bestand, en het bereik of het paginanummer als het vak daarom vraagt. Hebt u er één in de andere app gekopieerd, dan biedt het vak aan die te gebruiken.
3. Het object komt in de geselecteerde cel. Sleep om het te verplaatsen, en sleep een greep om het formaat te wijzigen. Een bereik, grafiek of pagina die u van formaat wijzigt, wordt opnieuw getekend op het nieuwe formaat, zodat hij scherp blijft. Druk op **Delete** om het weg te halen. Elk is één stap van Ongedaan maken.

Om wat u in een andere app hebt gekopieerd in te voegen, klikt u op de pijl onder **Plakken** en kiest u **Als object**.

**Dubbelklik** op een object om het in de app te openen die het heeft gemaakt. Sla het daar op, dan toont het object hier de wijziging als één stap van Ongedaan maken. Een cel waarin u typte, blijft zoals hij is. Een kopie die u plakt, is een eigen object: als u er één wijzigt, verandert de andere niet. Hetzelfde geldt voor een blad dat u kopieert: de objecten erop zijn kopieën, en de kopie van het blad behoudt ze.

**Bestand**, dan **Info**, dan **Objecten** toont elk object in de werkmap. Selecteer er één om het te openen, om het in een gewone afbeelding om te zetten, of om het te verwijderen.

Een object dat in een ander programma is gemaakt, toont de afbeelding die zijn bestand bewaart, wat voor soort afbeelding dat ook is. U kunt het selecteren, verplaatsen, van formaat laten veranderen, uitlijnen met andere objecten en verwijderen, en het verschuift mee met de cellen, net als uw eigen objecten. U kunt het hier niet bewerken, in een groep plaatsen, of de volgorde van de objecten wijzigen. Het wordt opgeslagen zoals het bestand het had, met alleen de plaats gewijzigd als u het hebt verplaatst. Een object dat in een .ods-bestand zat, werkt op dezelfde manier. Opgeslagen als .xlsx- of .ods-bestand staan uw objecten in de standaardvorm die andere programma's lezen. Wanneer een .ods-bestand wordt geschreven voor een object waarvan de afbeelding niet getekend kan worden, blijft het object zonder afbeelding bewaard.

## Diagrammen uit andere programma's

Een **diagram** is een beeld van vakjes en pijlen, zoals een proces, dat door een ander programma is gemaakt. JeffCrunch toont een diagram aan de hand van de afbeelding ervan die naast het bestand is bewaard. Klik erop om het te selecteren, en sleep het om het te verplaatsen. Wanneer u het bestand opslaat, wordt het diagram precies zoals het was teruggeschreven, zodat het programma dat het heeft gemaakt het nog steeds kan wijzigen.

JeffCrunch kan de inhoud van een diagram nog niet wijzigen, en een diagram kan niet worden gekopieerd of in een groep geplaatst. Als het bestand geen afbeelding van een diagram bewaart, toont JeffCrunch een gewoon grijs vak dat zegt dat het een diagram is.

## Grafieken in .ods-bestanden

Een werkmap die als .ods-bestand is opgeslagen, bewaart haar grafieken: kolommen, staven, lijnen, vlakken, cirkels, donuts, spreidingsdiagrammen en radardiagrammen, elk met haar reeksen en de cellen die ze lezen (op dit blad of een ander), categorieën, titel, legenda, assen (titels, laagste en hoogste waarde, getalnotatie), gegevenslabels, kleuren en plaats. Een .ods-bestand dat door een ander programma is gemaakt, wordt geopend met zijn grafieken, getekend uit de cellen die ze lezen. Wat een .ods-grafiek niet kan bewaren, wordt weggelaten en bij het opslaan genoemd: trendlijnen, foutbalken, secundaire assen en de extra looks van één enkel punt. Grafieken van typen die JeffCrunch niet tekent (aandelen- en radargrafieken) worden geopend als kolommen met hun waarden.

Een recensent heeft de volgende problemen in uw vorige vertaling gevonden; vermijd ze:
- "Een afbeelding die breder is dan 480 punten wordt verkleind om te passen en behoudt zijn vorm." (voor "A picture wider than 480 dots is made smaller to fit, and keeps its shape."): De Engelse maat is dots (pixels), maar het Nederlands zegt punten (1/72 inch), een andere maat, waardoor de genoemde grens in grootte een andere betekenis krijgt.
