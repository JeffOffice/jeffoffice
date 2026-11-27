# Grafieken, sparklines, afbeeldingen en vormen
<!-- keywords: ingesloten object, object invoegen, object uit bereik, object uit grafiek, object uit pagina, plakken als object, objectenlijst, object bewerken, veldknoppen, draaigrafiekknoppen, groeperen, groep opheffen, uitlijnen, links uitlijnen, verdelen, objecten selecteren, meerdere vormen, meerdere kiezen, selectiekader, grafieklfilters, reeks verbergen, aanbevolen grafieken, grafiek suggesties, grafiek, diagram, kolom, staaf, lijn, cirkel, vlak, spreiding, combinatie, radar, gevulde radar, spinnenwebgrafiek, legenda, titel, rasterlijnen, gegevenslabels, grafiekgegevens, grafiektype, stijlen, indelingen, verplaatsen, formaat wijzigen, afbeelding, vorm, object, reeks, trendlijn, vergelijking, R-kwadraat, secundaire as, sparkline, sparklines, datumas, winst/verlies, minigrafiek, markeringen, naam, alternatieve tekst, logaritmisch, logaritmische schaal, opslaan als afbeelding, sjabloon, crtx, titeltekst, legenda-sleutel, selectiedeelvenster, vulling van vorm, omtrek van vorm, draaien, spiegelen, bijsnijden, tekst bewerken, randen van afbeelding, afbeelding invoegen, pictogrammen, wmf, emf, vormstijlen, stijlen, vorm invoegen, tekstvak, vormen, pijl, ovaal, rechthoek, ster, diagram, diagrammen, online afbeeldingen, gratis afbeeldingen, openverse, wikimedia commons, licentie, bronvermelding, tekstopmaak, wordart, vervorming, afbeeldingen comprimeren, kleiner maken van afbeeldingen, bestandsgrootte, ppi, bijgesneden gebieden verwijderen, streepjescode, QR-code, vorm koppelen aan cel, vorm toont cel, tekstvak toont cel, celwaarde in vorm -->

Een grafiek is een beeld van getallen dat aan die getallen gekoppeld blijft: als u een getal wijzigt, verandert de grafiek mee. Afbeeldingen en vormen liggen op dezelfde manier over de cellen, maar ze zijn niet aan getallen gekoppeld.

## Een grafiek maken

1. Selecteer de getallen die u in een grafiek wilt tonen. Als de eerste rij namen bevat en de eerste kolom labels, selecteer die dan ook. JeffCrunch kijkt naar de eerste rij en kolom en gebruikt ze als namen en labels wanneer het woorden zijn (of datums, voor de labels).
2. Kies op het tabblad **Invoegen**, in **Grafieken**, een type: **Kolom**, **Staaf**, **Lijn**, **Cirkel**, **Vlak**, **Spreiding**, **Combinatie**, **Radar** of **Koers**. De kleine pijl naast een type toont de varianten, zoals gestapelde kolommen, een lijn met markeringen, of een spreidingsdiagram met punten, met verbonden punten of met vloeiende lijnen. Een radardiagram tekent elk label als een spaak vanuit het midden, en elke reeks als een gesloten vorm door de getallen op de spaken. Zo ziet u meerdere maten van één onderwerp naast elkaar.
   Een **koersgrafiek** toont koersen: voor elke categorie een lijn van de hoogste tot de laagste koers, met een klein bolletje voor de slotkoers, of, als er een openingskoers is, een balk tussen de openingskoers en de slotkoers (licht bij een stijging, donker bij een daling). De grafiek wordt gemaakt uit uw kolommen precies zoals ze staan, van links naar rechts, en er wordt niet gecontroleerd of de getallen op hun plek staan. Zet ze daarom in deze volgorde: hoog, laag en slot; open, hoog, laag en slot; volume, hoog, laag en slot; of volume, open, hoog, laag en slot. Staan de kolommen in een andere volgorde, dan wordt de grafiek toch getekend, maar betekenen de markeringen iets anders (het bolletje voor de slotkoers komt op de derde koerskolom, en de balk loopt van de eerste tot de laatste koerskolom). Bij een volume worden de volumes op de linkeras afgelezen en de koersen op een tweede as rechts. Past het aantal kolommen niet bij het type, dan meldt JeffOffice hoeveel er nodig zijn en wordt er geen grafiek gemaakt.
3. Wilt u eerst alle typen bekijken, kies dan **Meer grafieken**. De lijst links toont elk type, getekend uit uw cellen. **Rijen en kolommen wisselen** keert de reeksen om.

Weet u niet welk type u moet kiezen? Selecteer uw cellen en kies **Aanbevolen grafieken** op het tabblad **Invoegen**. JeffCrunch bekijkt de cellen en noemt een paar types die erbij passen, het beste eerst. Datums onder elkaar leiden bijvoorbeeld tot een lijn, en één kolom met een paar positieve getallen en labels tot een cirkel. Elk type wordt getekend uit uw cellen. Kies er een en klik op **OK**.

Selecteert u maar één cel, dan maakt JeffCrunch een grafiek van het blok cellen rond die cel dat iets bevat.

Reeksen lopen door de kolommen als de selectie minstens zoveel rijen als kolommen heeft, en langs de rijen als er meer kolommen dan rijen zijn. Een cirkel heeft één reeks. Een spreidingsdiagram neemt zijn horizontale waarden uit de eerste kolom.

Voorbeeld: u selecteert A1:C4, waarbij A2:A4 Q1, Q2 en Q3 bevatten, B1 en C1 Oost en West, en de rest getallen. De grafiek heeft twee reeksen, Oost en West, en drie groepen kolommen, Q1 tot en met Q3.

## Een reeks voorlopig verbergen

Selecteer een grafiek. Naast de grafiek verschijnen drie kleine knoppen. De knop met de trechter is **Grafiekfilters**. Die toont de reeksen van de grafiek, elk met een vinkje. Haal het vinkje weg om een reeks uit de grafiek te laten. Zet het vinkje weer aan om haar op dezelfde plek terug te laten keren. **Ongedaan maken** werkt ook. Een grafiek houdt altijd minstens één reeks. Een reeks die u hebt weggelaten, blijft in het bestand staan als u opslaat, en blijft verborgen wanneer u het bestand opent, zodat er niets verloren gaat. Zet het vinkje weer aan om haar terug te halen. (Een donutgrafiek toont bij het openen altijd alle reeksen.)

## Verplaatsen en formaat wijzigen

Klik op een grafiek, afbeelding of vorm om die te selecteren. Sleep om het te verplaatsen, of sleep een van de acht vierkantjes aan de rand om het formaat te wijzigen.

- Houd **Alt** ingedrukt terwijl u sleept, zodat een rand uitlijnt op de rand van een cel.
- **Uitlijnen op raster** en **Uitlijnen op vorm** staan op het tabblad **Vormopmaak** en worden voor elke werkmap onthouden. Met **Uitlijnen op raster** aan gaat een rand die u sleept naar de dichtstbijzijnde rand van een cel. Met **Uitlijnen op vorm** aan gaat een rand naar de rand van een ander object als die er binnen 6 pixels van ligt. Alleen de eerste 2.000 andere objecten van een blad tellen mee voor **Uitlijnen op vorm**. Staan beide aan, dan winnen de randen van cellen, omdat een object aan cellen is vastgemaakt, en worden de randen van de andere objecten niet bekeken. Een wijziging werkt meteen in elk geopend venster.
- **Alt** keert het uitlijnen om dat aan staat, voor één slepen: met **Uitlijnen op raster** of **Uitlijnen op vorm** aan, schakelt **Alt** het uitlijnen voor die slepen uit (het verandert nooit in het andere type), en met beide uit maakt **Alt** dat de randen naar cellen gaan, zoals hierboven.
- Een gedraaid object wordt uitgelijnd op de omtrek die u ziet (het kleinste kader dat het omvat), en niet op het kader van vóór het draaien. Dat geldt ook voor de objecten waarop het uitlijnt.
- Houd **Shift** ingedrukt terwijl u een hoek sleept om de verhoudingen te behouden.
- Met de pijltoetsen verplaatst u het object een paar pixels tegelijk. **Ctrl** met een pijl verplaatst het één pixel, **Shift** met een pijl maakt het groter of kleiner, en **Alt** met een pijl gaat naar de volgende rand van een cel.
- **Esc** tijdens het slepen zet het terug.

Elke verplaatsing of formaatwijziging is één stap die u ongedaan kunt maken. Klik met de rechtermuisknop op het object en kies **Eigenschappen** om de **Naam** in te typen (elk object op een blad heeft een eigen naam nodig) en de **Alternatieve tekst** (een korte zin die iemand die het object niet kan zien, vertelt wat het toont), om de grootte in te typen en om te kiezen wat het doet wanneer u cellen invoegt, verwijdert of het formaat wijzigt: verplaatsen en van formaat veranderen met de cellen, verplaatsen met de cellen maar de grootte behouden, of op zijn plek blijven. Die keuze wordt in het bestand opgeslagen. **Dit object afdrukken** in hetzelfde vak schakelt het object uit op de afgedrukte pagina's (het blijft op het scherm zichtbaar); het bestand bewaart ook die keuze.

## Wijzigen wat een grafiek toont

Wanneer een grafiek is geselecteerd, verschijnen er twee knoppen naast en wordt op het lint het tabblad **Grafiek** getoond.

- De knop **+** (en **Grafiekonderdelen** op het tabblad) zet de titel, assen, astitels, rasterlijnen, legenda en gegevenslabels van de grafiek aan of uit, en bepaalt waar de legenda en de labels komen.
- De kwastknop (en **Kleuren** en **Stijlen** op het tabblad) geeft de grafiek een van de kleurenschema's van JeffCrunch en een van de stijlen. **Indelingen** stelt meerdere onderdelen tegelijk in.
- **Grafiekgegevens** toont de reeksen van de grafiek. U kunt ze toevoegen, bewerken, verwijderen en opnieuw ordenen, de cellen wijzigen die elke reeks leest, de cellen met de labels instellen, de rijen en kolommen omwisselen, en kiezen hoe lege cellen en verborgen rijen en kolommen in de grafiek worden weergegeven.
- **Veldknoppen** (in het menu **+**, alleen voor een draaigrafiek) toont kleine knoppen met de namen van de velden van de draaitabel die de grafiek volgt: de rapportfilters onder de titel, en de rijen, waarden en kolommen langs de onderkant. Zet elke soort aan of uit, of kies **Alles weergeven** of **Alles verbergen**. Een nieuwe draaigrafiek toont ze.
- **Foutbalken** (in het menu **+**, voor een kolom-, staaf-, lijn-, vlak- of spreidingsgrafiek) tekent boven en onder elk punt een kleine lijn die aangeeft hoeveel de werkelijke waarde kan afwijken. Kies **Standaardfout**, **Percentage (5%)**, **Standaarddeviatie** of **Vaste waarde (1)**. Kies **Aangepast** om eigen bedragen te gebruiken: typ de cellen met de bedragen boven elk punt (zoals `=Blad1!$D$2:$D$4`), en daarna de cellen voor de bedragen eronder. Laat er één leeg om de balken maar aan één kant te tekenen. De balken veranderen wanneer die cellen veranderen. Een bestand dat eigen bedragen per punt heeft, behoudt die.
- **Lijnen** (in het menu **+**, voor een lijngrafiek) heeft **Druppellijnen**, die van het hoogste punt van elke categorie naar de as lopen, **Hoog-laaglijnen**, die het hoogste en laagste punt van elke categorie verbinden, en **Op-neerstaven**, die vakjes zijn tussen de eerste en de laatste reeks van elke categorie. Een opstaaf toont dat de laatste reeks hoger is. Hoog-laaglijnen en Op-neerstaven hebben twee reeksen of meer nodig. Vlakgrafieken hebben druppellijnen.
- **Gegevenstabel** (in het menu **+**, voor een kolom-, lijn- of vlakgrafiek) zet een raster met de getallen onder de grafiek, met een kolom voor elke categorie en een rij voor elke reeks. De lijst daarvan zet de tabel aan en uit, en bepaalt de lijnen tussen rijen, de lijnen tussen kolommen, de omtrek en de legenda-sleutels (de kleur van elke reeks naast haar naam).
- **Trendlijn** (in het menu **+**) tekent een lijn die op de getallen van elke reeks is aangepast, zodat u ziet waarheen ze gaan. Kies **Lineair** voor een rechte lijn, **Exponentieel** voor getallen die met een vast percentage groeien, **Logaritmisch** voor getallen die snel stijgen en dan vlakker worden, **Polynoom** voor een kromme met bochten, **Macht**, of **Voortschrijdend gemiddelde**, dat het gemiddelde van de laatste paar getallen tekent. **Vergelijking tonen** schrijft de formule van de lijn op de grafiek, en **R-kwadraat tonen** schrijft hoe goed de lijn past (1 is een perfecte aansluiting; 0 is helemaal geen aansluiting). De trendlijn krijgt een eigen regel in de legenda.
- Wordt een reeks getekend tegen een tweede verticale as, dan hebben de menu's **Assen** en **Astitels** de keuzes **Tweede horizontale** en **Tweede verticale** voor die reeks. Het menu **Legenda** heeft **Over de grafiek**, dat de legenda over het vlak legt in plaats van ernaast.
- **Gegevenslabels** heeft ook **Label bevat** (de waarde, de naam van de categorie, de naam van de reeks, het percentage en de legenda-sleutel, dat is een klein vierkantje in de kleur van de reeks, in elke combinatie), **Scheidingsteken** (wat ertussen staat: een komma, puntkomma, punt, nieuwe regel of spatie) en **Getalnotatie…** (een notatiecode zoals 0,0 voor de getallen in de labels; laat dit leeg om de getallen zoals ze zijn te tonen). **Waarde uit cellen…** toont de woorden van sommige cellen als labels, één cel voor elk punt. Typ de cellen, zoals `=Blad1!$D$2:$D$4`. De labels veranderen wanneer de cellen veranderen. Laat het leeg om ze weg te halen. Zet alle inhoudsonderdelen uit en de labels verdwijnen.
- **Horizontale as opmaken…** en **Verticale as opmaken…** op het tabblad **Grafiek** openen een vak voor één as. Daarin stelt u de laagste en hoogste waarde in en de hoofd- en subintervallen (elk automatisch of een getal dat u typt), of de waarden in omgekeerde volgorde lopen, de weergave-eenheden (de labels tonen de getallen gedeeld, bijvoorbeeld door 1.000 voor duizendtallen, zodat 30.000 als 30 wordt gelezen; vink **De naam van de eenheden op de grafiek weergeven** aan om de naam, zoals Duizendtallen, met de astitel af te drukken), of de schaal logaritmisch is (elke streep is een grondtal maal de vorige, zoals 1, 10, 100 en 1000, wat past bij getallen die sterk in grootte verschillen en allemaal groter dan 0 moeten zijn), waar de andere as deze as snijdt, de getalnotatie van de labels, de streepjes, waar de labels staan en hoe ver de labels gedraaid zijn. Alleen een getalas heeft grenzen en eenheden.
- **Grafiekonderdeel opmaken…** op het tabblad **Grafiek** (en in het rechtermuismenu) stelt de vulling en de lijn in van de pagina van de grafiek, van het vlak (het vak waarin de markeringen staan) of van één reeks. Voor een lijn stelt het ook in of die vloeiend is en welke markering bij elk punt staat. Het rechtermuismenu van een grafiek heeft ook **Rijen en kolommen wisselen** en de twee commando's voor de assen.
- **Huidige selectie** toont de onderdelen van de grafiek: het grafiekgebied, het plotgebied en elke reeks. Kies er een, en **Selectie opmaken** opent het opmaakvenster op dat onderdeel. **Terugzetten naar stijl** verwijdert wat op het gekozen onderdeel is ingesteld en geeft terug wat de stijl van de grafiek het geeft. Voor het grafiekgebied is dat de vulling, de lijn en de tekstkleur die een stijl instelde (de lichte tekst van de donkere stijl gaat terug naar de tekstkleur van het thema). Voor het plotgebied zijn dat de vulling en de lijn. Voor een reeks is dat de kleur (die het kleurenschema of de stijl van de grafiek haar geeft), de omtrek, de markeringen zoals het type grafiek die heeft, en de kleuren van haar punten; een vloeiende lijn blijft vloeiend als het type grafiek dat is. Titels, de legenda, labels, assen, rasterlijnen en de gegevens worden niet veranderd, en een onderdeel dat al past, blijft ongewijzigd. Het is één stap van Ongedaan maken.
- **Grafiek verplaatsen…** op het tabblad **Grafiek** vraagt waar de grafiek heen moet. Kies **Nieuw blad** en typ een naam, dan krijgt de grafiek een eigen grafiekblad: een blad dat alleen uit een grafiek bestaat, zonder cellen. Kies **Object in** en selecteer een werkblad, dan komt de grafiek op dat werkblad. Een grafiek op een grafiekblad kan alleen naar een werkblad worden verplaatst, en het grafiekblad dat wordt verlaten, wordt verwijderd. De grafiek leest nog steeds de cellen die hij las. Op een werkblad behoudt hij zijn grootte en zijn plek op het blad, ook waar de rijen en kolommen hoger of breder zijn, en hij verplaatst en verandert van formaat met de cellen daar, zoals voorheen. Verborgen bladen worden niet aangeboden. Het is één **Ongedaan maken**, zelfs nadat u had opgeslagen, tenzij door de verplaatsing een blad is verwijderd. Een bestand in de vorm `.ods` kan geen grafiekblad bevatten, dus sla een grafiekblad op in de vorm `.xlsx`.
- **Grafiektitel** en **Astitels** (in het menu **+**) hebben ook **Titeltekst…**. Typ de woorden die u wilt. Of typ een gelijkteken en een cel, zoals **=Blad1!A1**, dan toont de titel wat die cel bevat. Wanneer de cel verandert, verandert de titel mee. Het bestand bewaart de koppeling.
- **Opslaan als afbeelding…** op het tabblad **Grafiek** (en in het rechtermuismenu) slaat de geselecteerde grafiek op als een **PNG**- of **JPEG**-afbeeldingsbestand, tweemaal zo groot als op het blad, zodat hij scherp blijft.
- **Opslaan als sjabloon…** en **Sjabloon toepassen…** (op het tabblad **Grafiek**, in de groep **Type**) bewaren de stijl van een grafiek in een **.crtx**-bestand en geven die aan een andere grafiek. De stijl is het type, de kleuren, de instellingen van titel en legenda, de assen en de lettertypen. De grafiek waarop u hem toepast, behoudt zijn eigen getallen en de cellen die hij leest. Met één **Ongedaan maken** verdwijnt de stijl weer.
- **Grafiektype** wijzigt het type voor de hele grafiek, of voor elke reeks afzonderlijk: kolommen met een lijn erover, of een reeks tegen een tweede verticale as.

Een verwijzing naar cellen is van de vorm **Blad1!$B$2:$B$9**, met de naam van het blad tussen enkele aanhalingstekens als die spaties bevat.

## Sparklines

Een sparkline is een kleine grafiek die binnen één cel past. Hij toont naast de getallen de trend van een rij of kolom met getallen.

1. Selecteer de getallen. Voor één sparkline per rij selecteert u meerdere rijen met getallen.
2. Kies op het tabblad **Invoegen**, in **Sparklines**, **Lijn**, **Kolom** of **Winst/verlies**. Een **Lijn** verbindt de getallen met een lijn. Een **Kolom** tekent een balk voor elk getal. **Winst/verlies** tekent een balk omhoog voor elk getal boven nul en een balk omlaag voor elk getal eronder, allemaal even groot. Dat past bij resultaten die alleen goed of slecht zijn.
3. Controleer in het vak **Sparklines maken** het **Gegevensbereik** (de cellen die de sparklines lezen) en typ het **Locatiebereik** (de cellen waarin ze komen). Kies één locatiecel voor elke rij gegevens, of voor elke kolom gegevens. Kiest u één cel, dan leest de sparkline alle gegevens.

De sparklines die u in één stap maakt, vormen een groep en delen één stijl. Klik op een cel met een sparkline, dan verschijnt het tabblad **Sparkline**.

- **Gegevens bewerken** wijzigt de gegevens en de locatie van de hele groep, of de gegevens van de ene die u hebt aangeklikt. Ook bepaalt het wat er met lege cellen gebeurt (een lege ruimte laten, ze als nul behandelen, of de punten erover verbinden) en of cellen in verborgen rijen en kolommen worden gelezen.
- **Type** schakelt tussen lijn, kolom en winst/verlies.
- **Weergeven** markeert punten: het **Hoogste punt**, het **Laagste punt**, **Negatieve punten**, het **Eerste punt**, het **Laatste punt** en, voor een lijn, **Markeringen** op elk punt.
- **Stijlen** geeft de groep een reeks kleuren. **Sparklinekleur** stelt de kleur van de lijn of de balken in, en de **Dikte** (hoe dik de lijn is, in punten). **Markeringskleur** stelt de kleur van elke soort gemarkeerd punt in.
- **As** toont een lijn bij nul, zet de getallen van rechts naar links uit, en stelt de laagste en hoogste waarde van de verticale as in. **Automatisch voor elke sparkline** past elke sparkline aan op de eigen getallen. **Dezelfde voor alle sparklines** geeft de hele groep één schaal, zodat u ze kunt vergelijken. **Aangepaste waarde** laat u een getal typen. **Datumas…** verdeelt de punten op datum: typ de cellen met de datum van elk punt, en een punt met een latere datum staat verder naar rechts. Laat het leeg om de punten weer gelijkmatig te verdelen.
- **Groeperen** zet de geselecteerde sparklines in één groep. **Groep opheffen** geeft elke sparkline een eigen stijl. **Wissen** verwijdert de geselecteerde sparklines, of hun hele groepen.

Een sparkline volgt zijn cellen: voegt u een rij boven de gegevens in, dan leest hij nog steeds dezelfde getallen. Elke wijziging is één stap die u ongedaan kunt maken. Sparklines worden in het bestand opgeslagen in de vorm die andere spreadsheetprogramma's lezen.

## Een afbeelding, vorm of tekstvak invoegen

Op het tabblad **Invoegen**, in **Illustraties**, opent **Afbeelding** een bestandskiezer. Kies een afbeeldingsbestand (PNG, JPEG, GIF, BMP, TIFF of SVG), en het komt op het blad bij de geselecteerde cel. Een afbeelding die breder is dan 480 punten wordt verkleind om te passen en behoudt haar vorm. De werkmap bewaart de afbeelding zelf. Een afbeelding van een soort die de werkmap niet zo kan bewaren (een SVG, bijvoorbeeld) wordt bewaard als PNG, een gewone afbeelding van stippen. Een bestand dat geen afbeelding is, of groter is dan 16 MB, wordt geweigerd met een melding. Een WMF- of EMF-bestand (de afbeeldingssoorten die sommige oudere programma's maken van lijnen en vormen) wordt ook getekend en als PNG bewaard.

**Pictogrammen** toont hetzelfde venster met kleine lijnpictogrammen dat JeffPub en JeffDeck hebben, met een vak om ze op naam te zoeken. Kies er een of meer, en elk komt op het blad als afbeelding, 48 punten breed en hoog. Het wordt in donkergrijs getekend, op een formaat dat scherp blijft wanneer u het vergroot. Elk is één stap die u ongedaan kunt maken.

**Streepjescode** opent een kort menu met twee keuzes. **Streepjescode** opent een venster om één streepjescode te maken (**Streepjescodes uit cellen**, hieronder, maakt er veel): boek-ISBN's (met of zonder prijs), EAN-13, UPC-A, EAN-8, Code 128, Code 39, of een QR-code, die elke tekst kan bevatten, zoals een webadres. Kies een **Type**, typ de tekst of de cijfers, en het venster toont de streepjescode terwijl u typt. Als die niet kan worden gemaakt, zegt het in rood wat er niet klopt. Klik op **Invoegen**, en de code komt op het blad als afbeelding, één stap die u ongedaan kunt maken. Verplaats en verander het formaat zoals bij elke afbeelding. Om hem te wijzigen, selecteert u hem en klikt u weer op **Streepjescode**, past u de tekst aan en klikt u op **Bijwerken**; de nieuwe streepjescode neemt de plaats en de breedte van de oude over. Wanneer u opslaat, wordt hij als afbeelding opgeslagen met een SVG die scherp blijft, zodat elk programma hem toont. JeffCrunch bewaart zijn instellingen in het bestand, zodat het een streepjescode blijft die u kunt bewerken wanneer u de werkmap weer in JeffCrunch opent.

**Streepjescode > Streepjescodes uit cellen** (in hetzelfde menu **Streepjescode**) maakt een streepjescode voor elke gevulde cel van de geselecteerde cellen, voor een vel etiketten. Selecteer de cellen, klik erop, en kies dan het type en de vormgeving in hetzelfde venster. Wat u daar typt is alleen een voorbeeld, omdat de eigen tekst van elke cel zijn streepjescode wordt. De streepjescodes komen in een kolom direct rechts van de geselecteerde cellen, de een onder de ander, als één stap die u ongedaan kunt maken. Een cel waarvan de tekst de code niet kan bevatten (bijvoorbeeld een letter in een EAN-13) wordt in een melding genoemd en krijgt geen streepjescode. Er worden tegelijk ten hoogste 500 cellen verwerkt; selecteer er minder als het meldt dat er te veel zijn. Elke streepjescode onthoudt de cel waaruit hij komt en volgt die: wanneer u die cel wijzigt, wordt de streepjescode opnieuw gemaakt uit de nieuwe tekst, op dezelfde plek en in dezelfde grootte, als een eigen stap die u ongedaan kunt maken. De koppeling verschuift mee met de cel wanneer u rijen of kolommen invoegt of verwijdert. Verwijdert u de cel, dan behoudt de streepjescode de laatste tekst die hij had. Wanneer u opslaat, wordt de afbeelding in het bestand gemaakt uit de tekst van de cel zoals die op dat moment is, zodat andere programma's ook de actuele waarde tonen.

**Vormen** opent een lijst: rechthoek, afgeronde rechthoek, ovaal, driehoek, ruit, pijlen, een ster, een hart, een wolk, een spraakballon en een lijn. Kies er een en hij komt op het blad in lichtblauw met een donkerdere rand. **Tekstvak**, in de groep **Tekst**, zet een leeg vak neer zonder vulling en zonder rand. Klik op **Tekstvak tekenen** (de hoofdknop, of de eerste keuze in het menu), en sleep dan op het blad om het vak te tekenen. Een slepen maakt een vak van de grootte die u sleept, maar nooit kleiner dan 18 punten (0,64 cm) aan een zijde, en een klik van minder dan 4 punten geeft het gebruikelijke formaat. Houd **Shift** ingedrukt om het vak vierkant te houden. De aanwijzer is een kruis tot u loslaat; **Esc**, of een ander commando, legt het hulpmiddel weer neer. Het vak is één stap die u ongedaan kunt maken.

Om de tekst van een tekstvak te draaien, selecteert u het en klikt u op **Tekstrichting** op het tabblad **Vormopmaak**: **Horizontaal**, **Alle tekst 90° draaien** of **Alle tekst 270° draaien**. De woorden worden gedraaid getypt, zoals ze worden getoond, en een werkmap die als .xlsx- of .ods-bestand is opgeslagen, behoudt de richting.

Met een vorm geselecteerd geeft **Stijlen** op het tabblad **Vormopmaak** er een kant-en-klare look aan: een vulling en een omtrek die bij elkaar passen, zoals lichtblauw met een donkerblauwe rand. De laatste twee houden alleen de omtrek of alleen de vulling. Eén klik is één stap die u ongedaan kunt maken. **Vormvulling** en **Vormomtrek** wijzigen elk onderdeel afzonderlijk.

## Gratis afbeeldingen online zoeken

**Online afbeeldingen**, in de groep **Illustraties** van het tabblad **Invoegen**, zoekt gratis afbeeldingen. Ze komen uit twee open bibliotheken: Openverse, dat in veel collecties zoekt, en Wikimedia Commons, de bibliotheek met de afbeeldingen van Wikipedia.

1. Klik op **Online afbeeldingen…**. Er opent een venster rechts.
2. Kies een bibliotheek, typ waarnaar u zoekt, en klik op **Zoeken** (of druk op Enter).
3. Klik op een afbeelding om te zien wie hem heeft gemaakt en welke licentie hij heeft. Een **licentie** zegt hoe u een afbeelding mag gebruiken. Klik op de naam van de licentie om hem te lezen.
4. Klik op **Invoegen** (of dubbelklik op de afbeelding).

**Wat JeffCrunch verstuurt.** Uw zoekwoorden gaan naar de bibliotheek die u kiest, en alleen wanneer u op **Zoeken** klikt of op Enter drukt. JeffCrunch vraagt de bibliotheek daarna om de kleine voorvertoningen van wat is gevonden, en om de ene afbeelding die u invoegt. Zoals bij elke webpagina kan de bibliotheek het internetadres van uw computer zien, en wordt zij verteld dat JeffCrunch het vraagt. Niets uit uw werkmap wordt verstuurd.

De afbeelding komt op het blad zoals een afbeelding uit een bestand, en de werkmap bewaart hem. De titel wordt opgeslagen als de **alternatieve tekst**. De meeste licenties vragen u de maker te noemen. Wanneer **Bronvermelding onder de afbeelding toevoegen** is aangevinkt, voegt JeffCrunch onder de afbeelding een klein tekstvak toe met de titel, de maker en de licentie. U kunt het verplaatsen, wijzigen of verwijderen. De afbeelding en zijn bronvermelding gaan in één stap, dus **Ongedaan maken** haalt beide terug. Controleer of de licentie past bij hoe u de afbeelding gaat gebruiken.

## Tekstopmaak toevoegen

**Tekstopmaak** is opvallende tekst: letters met kleur, een omtrek, een schaduw of een gebogen vorm.

1. Klik op **Invoegen > Tekst > Tekstopmaak**. Er opent een galerie met stijlen.
2. Klik op een stijl. De woorden "Hier uw tekst" komen op het blad in die stijl.
3. Wilt u de woorden wijzigen, selecteer de tekstopmaak en klik op **Tekst bewerken…** op het tabblad **Vormopmaak**. Een vak vraagt om de woorden, het lettertype, de grootte, vet en cursief. Een grotere grootte maakt de tekstopmaak groter.
4. Op het tabblad **Vormopmaak** geeft **Stijlen voor tekstopmaak** de letters een andere stijl, en **Tekstvervorming** buigt de woorden in een vorm, zoals een golf of een boog. "Eenvoudig" houdt ze recht.

Tekstopmaak is een vorm in de tekening van het blad, met haar vervorming, vulling en omtrek, dus ze behoudt haar uiterlijk wanneer u de werkmap als .xlsx-bestand opslaat en weer opent, in JeffCrunch of in een ander programma. Elke wijziging is één stap die u ongedaan kunt maken. Een werkmap die als .ods-bestand is opgeslagen, behoudt ook zijn afbeeldingen, vormen, tekstvakken, groepen, lijnen en tekstopmaak. Tekstopmaak wordt opgeslagen als de fontwork-vorm die OpenDocument heeft, met haar woorden, lettertype, vulling, omtrek en vervorming, en keert terug als tekstopmaak wanneer u het .ods-bestand weer opent. Een paar vervormingen waarvoor OpenDocument geen vorm heeft (Golf 2, Golf 4, en de onder- en bovenvormen van Opblazen en Leeglopen) worden opgeslagen als rechte woorden met dezelfde letters en stijl. Grafieken worden ook in een .ods-bestand bewaard: zie hieronder.

## Afbeeldingen kleiner maken

Een afbeelding van een camera kan veel groter zijn dan hij op het blad wordt getoond, en dan wordt het bestand groot. Klik op een afbeelding en klik dan op **Afbeeldingen comprimeren…** in de groep **Aanpassen** van het tabblad **Afbeeldingsopmaak**. Kies waar de werkmap voor wordt gebruikt: **Commercieel drukken (300 ppi)**, **Desktopdrukken (220 ppi)** of **Web (96 ppi)**. Laat **Toepassen op alle afbeeldingen op dit blad** aangevinkt om elke afbeelding op het blad kleiner te maken, of haal het vinkje weg om alleen de geselecteerde afbeelding kleiner te maken. **Bijgesneden gebieden van afbeeldingen verwijderen** haalt ook de verborgen delen weg die u had bijgesneden; het bijsnijden is dan weg. Een afbeelding wordt alleen vervangen als de nieuwe kleiner is. Grootte, plek, naam en alternatieve tekst blijven zoals ze waren. Ongedaan maken (Ctrl+Z) geeft de afbeeldingen terug, en de opgeslagen werkmap bevat alleen de kleinere afbeeldingen. Afbeeldingen op andere bladen worden niet gewijzigd: ga naar elk blad en doe het opnieuw. Een afbeelding die een tekening is (SVG, EMF of WMF), of een animatie, blijft zoals ze is.

Elke compressie is één stap die u ongedaan kunt maken. Het nieuwe object is geselecteerd, zodat u het kunt slepen, van formaat veranderen, naar voren of naar achteren verplaatsen, een naam en alternatieve tekst geven in **Eigenschappen**, of op **Delete** drukken.

## De tekst van een cel in een vorm of tekstvak tonen

Een vorm of tekstvak kan tonen wat een cel toont, en die volgen. Selecteer de vorm, klik in de **formulebalk**, typ een gelijkteken en een cel, en druk op **Enter**.

- `=B2` koppelt de vorm aan B2 op dit blad. `=Blad2!B3` koppelt hem aan een cel op een ander blad. `=Totaal` koppelt hem aan een naam die voor één cel staat.
- De vorm toont dan de tekst van de cel zoals de cel die toont, met de getalnotatie. Wanneer u de cel wijzigt, de notatie ervan, of een formule waarvan die afhangt, verandert de vorm mee. Tekst die langer is dan een cel kan bevatten (32.767 tekens) wordt afgekapt. Alle gekoppelde vormen van een werkmap samen tonen ten hoogste 4 MB tekst: daarboven toont een vorm afgekapte tekst.
- Met de vorm geselecteerd toont de formulebalk de koppeling, zoals `=Blad1!$B$2`. Wis de formulebalk en druk op **Enter** om de koppeling weg te halen. De vorm behoudt dan de woorden die hij toont.
- Het intypen van woorden in de vorm haalt de koppeling ook weg.
- Een koppeling is één cel. Een bereik zoals `=A1:B2`, een lijst, een berekening of een naam die geen cel is, wordt geweigerd met een melding, en de vorm behoudt zijn koppeling.
- Wordt de rij of kolom van de cel verwijderd, of het blad ervan, dan toont de vorm `#VERW!` en behoudt hij de koppeling, net als een formule. Ongedaan maken brengt de cel terug.
- Elke koppeling, en elke keer dat u er een wegneemt, is één stap die u ongedaan kunt maken.

De koppeling wordt in een .xlsx-bestand opgeslagen met de tekst die de cel toonde, zodat een programma dat koppelingen niet volgt toch de woorden toont, en JeffCrunch opent het bestand met de koppeling werkend. OpenDocument kent geen manier om een tekstvak aan een cel te koppelen, dus een werkmap die als .ods-bestand is opgeslagen, bewaart de woorden die de vorm op dat moment toont, en niet de koppeling.

## Het uiterlijk van een afbeelding, vorm of tekstvak wijzigen

Selecteer een afbeelding en het tabblad **Afbeeldingsopmaak** verschijnt. Selecteer een vorm, een lijn of een tekstvak en het tabblad **Vormopmaak** verschijnt. Elke wijziging is één stap die u ongedaan kunt maken, en het object behoudt zijn naam, zijn alternatieve tekst, zijn plek en zijn volgorde ten opzichte van de andere objecten.

- **Vormvulling** en **Vormomtrek** kiezen een kleur, of geen. **Lijnstijl** stelt de dikte (hoe dik de lijn is) en de streepjes in (doorgetrokken, gestreept of gestippeld). Een afbeelding heeft **Afbeeldingsrand**, dat zijn omtrek is.
- **Draaien** draait het object een kwartslag naar rechts of naar links, of spiegelt het van links naar rechts of van boven naar beneden.
- **Bijsnijden**, voor een afbeelding, knipt deze bij tot een venstervorm: vierkant, 4 breed bij 3 hoog, 3 breed bij 4 hoog, 16 bij 9, of 9 bij 16. Het bijknippen gebeurt gelijkmatig vanuit het midden van wat zichtbaar is, en de afbeelding wordt zo groot gemaakt als wat overblijft. **De hele afbeelding tonen** brengt terug wat is weggeknipt. **Vullen** houdt het kader en knipt de afbeelding zo bij dat ze het hele kader bedekt. **Passend** toont de hele afbeelding, en het kader krijgt de vorm van de afbeelding binnen het oude kader. **Bijsnijden tot vorm** (de galerie naast **Bijsnijden**) knipt de afbeelding bij tot een vorm, zoals een ovaal, een ster of een afgeronde rechthoek; de vorm wordt met de afbeelding opgeslagen. Het afbeeldingsbestand wordt niet gewijzigd: het weggeknipte deel is alleen verborgen.
- **Tekst bewerken** is voor vormen en tekstvakken. Typ de woorden en kies hoe ze over de breedte en de hoogte staan, de grootte, vet of normaal, en of ze naar de vorm terugvloeien of op één regel blijven. Voor tekstopmaak vraagt het in plaats daarvan om de woorden, het lettertype, de grootte, vet en cursief.
- **Selectiedeelvenster** opent een lijst van elk object op het blad bij naam, het voorste eerst. Klik op een naam om dat object te selecteren, wat helpt wanneer objecten elkaar verbergen. **Naar voren** en **Naar achteren** verplaatsen het in de lijst.
- **Eigenschappen** stelt de grootte, de naam, de alternatieve tekst en wat het object doet wanneer cellen worden verplaatst of groter gemaakt, in.

## Een object knippen, kopiëren en plakken

Een afbeelding of vorm wordt op dezelfde manier gekopieerd en geplakt als een grafiek. Een vorm die iets anders gebruikt dan een afbeelding uit de werkmap, zoals een koppeling naar een bestand, kan niet worden gekopieerd, en JeffCrunch laat u weten waarom.

Kopieer een grafiek en plak hem in dezelfde werkmap, op elk blad: de kopie leest dezelfde cellen als de eerste. Plakt u hem in een andere werkmap, dan behoudt hij de getallen die hij toonde, maar hij leest geen cellen meer, omdat de cellen die hij daar zou lezen niet de cellen zijn waarvan hij is gemaakt.

## Meerdere objecten selecteren, groeperen en uitlijnen

Klik op één object en houd vervolgens Shift of Ctrl ingedrukt terwijl u op een ander klikt om dat ook te selecteren. Klik op een geselecteerd object met Shift of Ctrl om het los te laten. Geselecteerde objecten krijgen elk een gestreept kader. U kunt ook **Objecten selecteren** kiezen op het tabblad **Vormopmaak** en over lege cellen een vak slepen: elk object dat helemaal in het vak past, wordt geselecteerd. Druk op Esc, of kies **Objecten selecteren** opnieuw, om weer cellen te selecteren. Druk op Delete om alle geselecteerde objecten in één stap te verwijderen.

Wanneer er twee of meer zijn geselecteerd, heeft het tabblad **Vormopmaak** deze opdrachten in de groep **Schikken**:

- **Groeperen** voegt de geselecteerde vormen, afbeeldingen en lijnen samen tot één object. Het verplaatst en verandert van formaat als één geheel, en het bestand bewaart het als groep. Een grafiek kan niet in een groep worden geplaatst.
- **Groep opheffen** haalt een geselecteerde groep uit elkaar. Elk onderdeel komt terug op zijn plek op het blad.
- **Uitlijnen** lijnt de geselecteerde objecten uit: **Links uitlijnen**, **Centreren**, **Rechts uitlijnen**, **Boven uitlijnen**, **Midden uitlijnen** en **Onder uitlijnen**. Ze lijnen uit op de rand of het midden van de hele set. **Horizontaal verdelen** en **Verticaal verdelen** verdelen drie of meer objecten gelijkmatig tussen de twee aan de uiteinden.

Groeperen, Groep opheffen en elke uitlijning zijn één stap die u ongedaan kunt maken.

## Een naam geven

Met een object geselecteerd toont het naamvak de naam ervan. Typ er een nieuwe naam en druk op **Enter**. Ongedaan maken zet de oude naam terug, en de nieuwe naam wordt in het bestand opgeslagen.

## Verwijderen

Druk op **Delete** met het object geselecteerd. Ongedaan maken brengt het terug.

## Wat wordt opgeslagen

Alles hierboven wordt in het bestand opgeslagen in de vorm die andere spreadsheetprogramma's lezen, zodat een grafiek die u hier maakt, daar op dezelfde manier opent. Een grafiek die uit een ander programma komt met iets wat JeffCrunch niet kan behouden wanneer het een grafiek herschrijft (foutbalken, een driedimensionale grafiek), kan nog wel worden verplaatst, van formaat veranderd en verwijderd, maar de onderdelen en gegevens ervan worden hier niet gewijzigd.

## Een object invoegen

Een **object** bevat zijn eigen bestand binnen de werkmap, zodat de app die het heeft gemaakt het weer kan openen. Het toont een beeld van zichzelf, maar het is geen afbeelding: het blijft een object wanneer u het verplaatst, van formaat verandert, kopieert of de werkmap opslaat.

1. Klik op **Invoegen**, dan op **Object**, en kies **Uit JeffCrunch-bereik**, **Uit JeffCrunch-grafiek** of **Uit JeffWrite-pagina**.
2. Kies het bestand, en het bereik of het paginanummer als het vak daarom vraagt. Hebt u er één gekopieerd in de andere app, dan biedt het vak aan die te gebruiken.
3. Het object komt in de geselecteerde cel. Sleep het om het te verplaatsen, en sleep een handvat om het van formaat te veranderen. Een bereik, grafiek of pagina die u van formaat verandert, wordt opnieuw getekend op zijn nieuwe grootte, zodat hij scherp blijft. Druk op **Delete** om het te verwijderen. Elke stap is één stap van Ongedaan maken.

Om iets in te voegen dat u in een andere app hebt gekopieerd, klikt u op de pijl onder **Plakken** en kiest u **Als object**.

**Dubbelklik** op een object om het in de app te openen waarin het is gemaakt. Sla het daar op, dan toont het object hier de wijziging als één stap van Ongedaan maken. Een cel waarin u typte, blijft zoals ze is. Een kopie die u plakt is een object op zich: het wijzigen van de ene verandert de andere niet. Hetzelfde geldt voor een blad dat u kopieert: de objecten erop zijn kopieën, en de kopie van het blad behoudt ze.

**Bestand**, dan **Info**, dan **Objecten** toont elk object in de werkmap. Selecteer er een om het te openen, om het in een gewone afbeelding om te zetten, of om het te verwijderen.

Een object dat in een ander programma is gemaakt, toont de afbeelding die het bestand ervan bewaart, welke soort afbeelding dat ook is. U kunt het selecteren, verplaatsen, van formaat veranderen, uitlijnen met andere objecten en verwijderen, en het verschuift met de cellen zoals uw eigen objecten dat doen. U kunt het hier niet bewerken, niet in een groep plaatsen, en de plaats in de volgorde van de objecten niet wijzigen. Het wordt opgeslagen zoals het bestand het had, met alleen de plaats gewijzigd als u het hebt verplaatst. Een object dat een .ods-bestand bevatte, werkt op dezelfde manier. Opgeslagen als .xlsx- of .ods-bestand, staan uw objecten in de standaardvorm die andere programma's lezen. Wanneer er een .ods-bestand wordt geschreven voor een object waarvan de afbeelding niet kan worden getekend, wordt het object zonder afbeelding bewaard.

## Diagrammen uit andere programma's

Een **diagram** is een afbeelding van vakjes en pijlen, zoals een proces, die door een ander programma is gemaakt. JeffCrunch toont een diagram aan de hand van de afbeelding ervan die het bestand ernaast bewaart. Klik erop om het te selecteren, en sleep het om het te verplaatsen. Wanneer u het bestand opslaat, wordt het diagram precies zo teruggeschreven als het was, zodat het programma dat het heeft gemaakt het nog steeds kan wijzigen.

JeffCrunch kan wat er in een diagram staat nog niet wijzigen, en een diagram kan niet worden gekopieerd of in een groep geplaatst. Bevat het bestand geen afbeelding van een diagram, dan toont JeffCrunch een eenvoudig grijs vak dat zegt dat het een diagram is.

## Grafieken in .ods-bestanden

Een werkmap die als .ods-bestand is opgeslagen, behoudt zijn grafieken: kolom-, staaf-, lijn-, vlak-, cirkel-, donut-, spreidings-, radar- en koersgrafieken van hoog, laag en slot of van open, hoog, laag en slot, elk met de reeksen en de cellen die ze lezen (op dit blad of op een ander), categorieën, titel, legenda, assen (titels, laagste en hoogste waarde, getalnotatie), gegevenslabels, kleuren en plek. Een .ods-bestand uit een ander programma wordt geopend met de grafieken, getekend uit de cellen die ze lezen. Wat een .ods-grafiek niet kan bevatten, wordt weggelaten en bij het opslaan benoemd: trendlijnen, foutbalken, secundaire assen (dus een koersgrafiek met een volume wordt opgeslagen met het volume op de eerste as, en opent als kolommen), en de extra stijlen van één enkel punt. Grafieken van typen die JeffCrunch niet tekent, openen als kolommen met hun waarden.
