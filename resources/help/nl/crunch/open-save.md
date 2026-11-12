# Werkmappen openen en opslaan
<!-- keywords: herstellen niet-opgeslagen werk, automatisch herstel, vastloper, kopie, oudere indeling, opendocument, compatibiliteit, nieuw, leeg, boek1, naamloos, openen, opslaan, opslaan als, bestand, werkmap, xlsx, xlsm, xls, ods, csv, tsv, tekstbestand, scheidingsteken, importeren, exporteren, recent, vastmaken, slepen, neerzetten, laden, groot, macro's, niet-opgeslagen, wijzigingen, sluiten, kopie, berekeningsketen, sjabloon, xltx, xltm, kan niet worden geopend, beschadigd, versleuteld, wachtwoord, kopie opslaan, alleen-lezen, openen als kopie, nieuw venster, reservekopie, mijn sjablonen, opslaan als sjabloon, ots, repareren, openen en repareren, ingebouwde sjablonen, budget, factuur, urenstaat, lening, agenda, cijferlijst, vaste breedte, vaste kolommen, decimaalteken, duizendtalteken, min achteraan, html, webpagina, htm, tabellen uit een pagina -->

Wanneer JeffCrunch start zonder bestand om te openen, bevat het venster een nieuwe, lege werkmap met één blad, genaamd Boek1. Elke nieuwe werkmap krijgt het volgende nummer (Boek2, Boek3, enzovoort), en een nummer wordt niet opnieuw gebruikt nadat het venster is gesloten. Om een andere werkmap te starten, drukt u op **{key:New}**, of klikt u op **Bestand**, daarna op **Nieuw**, en kiest u **Lege werkmap**; die opent in een eigen venster. Een nieuwe werkmap die u niet hebt gewijzigd, maakt plaats wanneer u een bestand opent in het venster; een die u wel hebt gewijzigd, blijft staan, en het bestand opent in een ander venster. **Opslaan** bij een nieuwe werkmap vraagt om een naam, net als **Opslaan als**, en slaat de werkmap op als .xlsx-bestand. Een nieuwe werkmap die u niet hebt gewijzigd, sluiten levert geen vraag op.

Om een werkmap te openen, drukt u op **{key:Open}**, of klikt u op **Bestand** en daarna op **Openen**, en kiest u **Bladeren**. U kunt ook een werkmap vanuit uw bestandsbeheer naar het venster slepen. Een werkmap die al in een venster open is, wordt niet nogmaals geopend: dat venster komt dan naar voren.

JeffCrunch opent **.xlsx**-werkmappen en **.xlsm**-werkmappen (die macro's bevatten), en de sjablonen **.xltx** en **.xltm** (een sjabloon met macro's). Het voert geen macro's uit, en laat ze ongemoeid: het macroproject, de handtekening ervan en de onderdelen die ernaar verwijzen, worden precies zoals ze waren teruggeschreven. Een sjabloon wordt als sjabloon opgeslagen, en een werkmap met macro's als werkmap met macro's, omdat de soort deel van het bestand is.

## Niet-opgeslagen werk herstellen

Als JeffCrunch onverwacht sluit, bijvoorbeeld bij een stroomstoring, toont de volgende start een lijst van de kopieën die het van uw niet-opgeslagen werk heeft bewaard. Kies welke u wilt openen en klik op **Openen**; elke kopie opent als nieuwe werkmap zonder bestand, dus klik op **Opslaan** en kies een naam. **Verwijderen** gooit de aangevinkte kopieën weg, en **Niet nu** laat ze voor later staan. U kunt de lijst op elk moment opvragen via **Bestand > Openen > Niet-opgeslagen werk herstellen**. Zie [JeffCrunch-instellingen wijzigen](options) voor hoe vaak de kopieën worden gemaakt.

## Andere soorten bestanden

JeffCrunch opent ook oudere **.xls**-werkmappen, OpenDocument-spreadsheets (**.ods** en de platte **.fods**), en tekstbestanden met waarden (**.csv**, **.tsv** en **.txt**). Een werkmap die uit een van deze bestanden is geopend, wordt opgeslagen als nieuw **.xlsx**-bestand (of als **.xls**- of **.ods**-kopie: zie hieronder). Wanneer u op **{key:Save}** drukt, meldt JeffCrunch dat en vraagt om een naam, en het bestand dat u hebt geopend, wordt nooit gewijzigd. De afbeeldingen, vormen, tekstvakken, groepen, lijnen en tekstopmaak van een **.ods**-bestand gaan met hun plaats, grootte, draaiing, volgorde, namen, alternatieve tekst, opvullingen en omtrekken naar het nieuwe bestand; de grafieken gaan mee, getekend uit de cellen die ze lezen.

Een OpenDocument-spreadsheet wordt gelezen met de bladen, waarden, formules (met de resultaten die het bestand heeft bewaard), getalnotaties, lettertypen, opvullingen, randen, samengevoegde cellen, kolombreedten en rijhoogten, verborgen rijen en kolommen, vastgezette rijen en kolommen en namen. Grafieken, afbeeldingen en andere tekeningen, opmerkingen, draaitabellen, macro's, voorwaardelijke opmaak en gegevensvalidatie worden nog niet gelezen; een melding bovenaan vertelt welke het bestand bevat, en een kopie die vanaf hier wordt opgeslagen, heeft ze niet. Een spreadsheet die met een wachtwoord is beveiligd, vraagt erom bij het openen: zie [Met een wachtwoord beveiligde werkmappen openen](open-password).

Een tekstbestand is één blad. JeffCrunch bepaalt hoe de waarden zijn gescheiden (door komma's, puntkomma's, tabs of verticale strepen) en hoe de letters zijn gecodeerd (UTF-8 of UTF-16, anders de codepagina van uw taal), en leest elke waarde zoals u haar in uw taal zou typen: een getal zoals 1.234,5 in het Duits, een percentage, een bedrag, een datum met het jaar (zoals 15-3-2024 of 15.03.2024 in het Duits), een tijd, WAAR of ONWAAR (in uw taal, zoals WAHR en FALSCH in het Duits, en TRUE en FALSE in het Engels). Een waarde die met een is-gelijkteken begint, is een formule. Cijfers die met een nul beginnen (zoals 007) en getallen van meer dan 15 cijfers blijven tekst, zodat een postcode of een rekeningnummer niet wordt veranderd; hetzelfde geldt voor een dag en een maand zonder jaar, in welke volgorde en in welke taal ook (zoals 3/4, 12.5, 1-2, 1 maart of 25 dec), omdat het jaar dan zou moeten worden verzonnen, en voor teksten zoals de gennaam MARCH1. Een blad bevat 1.048.576 rijen en 16.384 kolommen; als het bestand meer heeft, opent JeffCrunch wat past en meldt dat.

Wanneer u een tekstbestand opent, toont een venster eerst de eerste rijen en vraagt hoe het wordt gelezen: wat de velden scheidt (een komma, puntkomma, tab, spatie, verticale streep of een teken dat u typt, met eventueel meerdere scheidingstekens achter elkaar als één gerekend), welk teken een veld met een scheidingsteken omsluit, de tekencodering (Unicode, de Windows-codepagina's, de ISO 8859-pagina's, Mac Roman en de Oost-Aziatische), en de rij waarop wordt begonnen. Boven elke kolom kiest u hoe die wordt gelezen: **Zoals getypt**, **Tekst** (om een artikelnummer zoals 007 of een lang rekeningnummer ongewijzigd te houden), een **Datum** in de volgorde die u opgeeft, of **Weglaten**. Sommige bestanden zetten hun waarden in kolommen van gelijke breedte, met spaties ertussen en zonder scheidingsteken. Vink **De kolommen hebben een vaste breedte** aan, dan verschijnen de eerste regels van het bestand in een vak. Klik tussen twee tekens om daar een kolom te laten beginnen, en klik nogmaals om die weg te halen; of typ de posities bij **Kolommen beginnen bij**, zoals 10, 25, 40. Elk stuk wordt bij die posities afgesneden, en de spaties eromheen worden weggehaald. Met **Decimaalteken** en **Duizendtalteken** vertelt u JeffCrunch hoe getallen in dit bestand zijn geschreven, ongeacht de taal die u zelf gebruikt; zo is 1.234,50 duizend tweehonderd vierendertig en een halve wanneer het bestand een komma als decimaalteken gebruikt. Vink **Een minteken kan na het getal staan** aan voor bestanden die -12 als 12- schrijven. De voorbeeldweergave volgt elke wijziging en toont hooguit de eerste 200 rijen en 100 kolommen, maar het hele bestand wordt geopend. **Bestand > Openen > Tekstbestand openen** vraagt het altijd, ook als de naam van het bestand ongewoon is.

JeffCrunch opent ook **webpagina's** (**.html** en **.htm**), en een pagina die met de naam van een werkmap is opgeslagen. Elke tabel op de pagina wordt een blad, genoemd **Tabel1**, **Tabel2**, enzovoort. De cellen worden gelezen alsof u ze had getypt: een getal is een getal, en een datum is een datum. Een cel die over meerdere kolommen of rijen is samengevoegd, laat lege cellen achter, zodat de andere cellen in hun kolommen blijven. De woorden buiten de tabellen, de afbeeldingen en de opmaak van de pagina worden niet gelezen, en een pagina zonder tabel wordt niet geopend. Niets wat de pagina noemt, wordt van het internet opgehaald.

Om alleen de waarden van het blad waarnaar u kijkt te bewaren, klikt u op **Bestand**, daarna op **Opslaan als**, en kiest u het **.csv**-bestandstype (of **.tsv** en **.txt**, met tabs gescheiden). Voordat JeffCrunch het bestand schrijft, meldt het wat uw werkmap heeft dat het bestand niet bevat (de andere bladen, formules, afbeeldingen, grafieken, notities en macro's, bijvoorbeeld), en vraagt hoe het moet worden geschreven: of alleen de **Waarden opslaan**, of **Opslaan als .xlsx** om alles te behouden, of **Annuleren**:

- **Scheidingsteken.** Een komma, puntkomma, tab, spatie, verticale streep, of een teken dat u typt. Een **.csv** begint met een puntkomma waar uw taal een komma in getallen schrijft, en elders met een komma; een **.tsv** of **.txt** met een tab.
- **Tekstcodering.** UTF-8 met markering (zodat andere programma's accentletters goed overnemen), UTF-8 zonder markering, UTF-16, of een codepagina zoals West-Europees (1252), de ISO 8859-pagina's, Mac Roman, of de Japanse, Chinese, Koreaanse, Hebreeuwse, Arabische, Baltische, Vietnamese en Thaise pagina's. Een letter waarvoor de codepagina geen byte heeft, wordt als vraagteken geschreven, en de statusbalk zegt hoeveel dat er zijn.
- **Regeleinden.** Windows (regelterugloop en regelinvoer), Linux en Mac (regelinvoer), of oude Mac (regelterugloop).
- **Cellen.** **Zoals weergegeven in de cellen** schrijft wat u ziet. **Zoals opgeslagen** schrijft elk cijfer van een getal (0,1 en niet 10%), en een datum als ISO-tekst zoals 2026-03-04.

Tekst komt tussen aanhalingstekens wanneer ze het scheidingsteken, een aanhalingsteken, een regeleinde of een spatie aan een van beide kanten bevat. De andere bladen, de formules, de opmaak en de instellingen staan niet in het bestand, en uw werkmap wordt niet gewijzigd door het schrijven ervan.

Een werkmap die uit een tekstbestand komt, wordt teruggeschreven in de vorm die het bestand had: met hetzelfde scheidingsteken, dezelfde regeleinden en dezelfde codering. Als u die als kopie hebt opgeslagen zonder een cel te wijzigen, is de kopie hetzelfde bestand, byte voor byte, en wordt er niets gevraagd.

## Een kopie als .xls of .ods opslaan

Om een kopie te maken die oudere programma's, of programma's die OpenDocument lezen, kunnen openen, klikt u op **Bestand**, daarna op **Opslaan als**, en kiest u het **.xls**- of **.ods**-bestandstype. De kopie wordt gemaakt van uw werkmap zoals die nu is, met de wijzigingen die u nog niet hebt opgeslagen, en bevat:

- de waarden, met hun teksten, getallen, waarheidswaarden en fouten;
- de formules, met de resultaten die ze nu hebben (een matrixformule blijft een matrixformule);
- de getalnotaties, lettertypen, opvullingen, randen, uitlijning, en of een cel is vergrendeld;
- samengevoegde cellen, kolombreedten en rijhoogten, verborgen rijen en kolommen, rij- en kolomoverzichten, en vastgezette rijen en kolommen;
- de bladen, in hun volgorde, met hun namen en welke verborgen zijn, en de namen die u hebt gedefinieerd (en de afdrukbereiken);
- de grafieken van elk blad (alleen in een **.ods**-kopie): kolom-, staaf-, lijn-, vlak-, cirkel-, ring- en spreidingsgrafieken, met hun reeksen en de cellen die ze lezen, categorieën, titels, legenda, assen (titels, schaal, getalnotatie), gegevenslabels, kleuren en plaats (wat een **.ods**-grafiek niet kan weergeven, zoals trendlijnen, foutbalken en secundaire assen, wordt in de melding genoemd);
- de afbeeldingen, vormen, tekstvakken, groepen, lijnen en tekstopmaak van elk blad (alleen in een **.ods**-kopie), met hun plaats, grootte, draaiing, volgorde, namen, alternatieve tekst, opvullingen, omtrekken, woorden, en de afbeeldingen zelf met hun bijsnijding (tekstopmaak wordt opgeslagen als de fontwork-vorm die OpenDocument heeft);
- de koppelingen, en hoe elk blad wordt afgedrukt (het papier, de marges, de afdrukstand, de schaal, koptekst en voettekst, en de pagina-einden).

Een **.xls**-bestand bevat ook gegevenstabellen, en bewaart de berekeningsinstellingen van de werkmap. Een kopie kan niet alles bevatten. Voordat JeffCrunch het bestand schrijft, zegt het wat uw werkmap heeft dat het bestand niet zal bevatten: grafieken, afbeeldingen en andere tekeningen (een **.xls**-kopie; een **.ods**-kopie bewaart ze, grafieken inbegrepen, behalve diagrammen), notities, draaitabellen, tabellen, voorwaardelijke opmaak, gegevensvalidatie, filters, bladbeveiliging, formulierbesturingselementen, macro's en koppelingen naar andere werkmappen (de formules die ze gebruiken, houden de waarden die ze hadden). Een formule die de oudere vorm niet kan uitdrukken (een functie die die niet kent, of een cel na kolom IV of rij 65.536), wordt geschreven als de waarde die eruit kwam, en de melding telt ze. De oudere vorm stopt ook bij 65.536 rijen, 256 kolommen, 255 bladen en 32.767 tekens in een cel, en bij ongeveer 4.000 celopmaken en 56 kleuren: wat niet past, wordt afgekapt, en de melding zegt hoeveel. U kunt dan kiezen om de **Kopie opslaan**, om **Opslaan als .xlsx** te kiezen en alles te behouden, of om **Annuleren** te kiezen.

Uw werkmap wordt door de kopie niet gewijzigd, en ze wordt niet als opgeslagen gemarkeerd. Het bestand dat u hebt geopend, wordt nooit geschreven: geef de kopie een andere naam. Een kopie van een **.xls**- of **.ods**-bestand dat u hebt geopend, kan in het andere soort worden opgeslagen.

## Een kopie opslaan

Klik op **Bestand > Kopie opslaan** en kies een naam. JeffCrunch schrijft de werkmap naar dat bestand, met de wijzigingen die u nog niet hebt opgeslagen, en u blijft werken in de werkmap die u had. De naam en de niet-opgeslagen wijzigingen blijven zoals ze waren. U kunt elk soort bestand kiezen dat **Opslaan als** aanbiedt. Een kopie wordt nooit over het eigen bestand van de werkmap geschreven.

## Een reservekopie bewaren

Klik op **Bestand > Opties**, en zet op de pagina **Algemeen** de optie **Altijd een reservekopie maken** aan. Telkens wanneer u opslaat, kopieert JeffCrunch eerst het oude bestand naar een bestand met de naam "Reservekopie van" plus de naam ervan, in dezelfde map. Als de kopie niet kan worden gemaakt, vraagt het programma of u zonder wilt opslaan. Het antwoord is Nee, tenzij u Ja zegt, en het bestand blijft zoals het was.

## Opslaan als sjabloon

Een sjabloon is een werkmap die u als beginpunt gebruikt. Ze kan formules, stijlen en bladen bevatten. Klik op **Bestand > Opslaan als sjabloon** en typ een naam. Het sjabloon (een .xltx-bestand) komt in **Mijn sjablonen**, en uw werkmap wordt niet gewijzigd. In **Opslaan als** kunt u ook **Sjablonen (*.xltx)** kiezen, of **OpenDocument-spreadsheetsjablonen (*.ots)**, om een sjabloonbestand op een plek naar keuze op te slaan.

Om een werkmap uit uw sjabloon te starten, klikt u op **Bestand > Nieuw**. Uw sjablonen staan onder **Mijn sjablonen**. Klik op een sjabloon. Er opent een nieuwe werkmap met de cellen en bladen van het sjabloon. Die heeft nog geen eigen bestand, dus **Opslaan** vraagt om een naam, en het sjabloon wordt niet gewijzigd. Om een sjabloon uit de lijst te verwijderen, klikt u er met de rechtermuisknop op en kiest u **Verwijderen uit Mijn sjablonen**.

## Andere manieren om een bestand te openen

Klik op de pagina **Openen** op **Meer manieren om te openen**. U kunt ook met de rechtermuisknop op een naam in de lijst Recent klikken.

- **Openen als alleen-lezen** opent de werkmap alleen om te lezen. U kunt bekijken, scrollen, zoeken en kopiëren, maar geen cel wijzigen. De titelbalk toont "Alleen-lezen". Als u op **Opslaan** drukt, vraagt JeffCrunch om een nieuwe naam, zodat het bestand dat u hebt geopend niet wordt gewijzigd.
- **Openen als kopie** opent de werkmap zonder eigen bestand. **Opslaan** vraagt waar het moet komen. Het bestand dat u hebt geopend, wordt nooit gewijzigd.
- **Openen in nieuw venster** opent de werkmap in een eigen venster, ook wanneer het venster waarin u zit leeg is.
- **Openen en repareren** is voor een .xlsx- of .xls-bestand dat beschadigd is. Zie hieronder.

## Openen en repareren

Soms is een .xlsx-bestand beschadigd. Een download is halverwege gestopt, of een schijf heeft fouten gemaakt. Een .xlsx-bestand is een bundel van kleinere onderdelen, zoals de bladen, de stijlen en de gedeelde tekst. **Openen en repareren** leest elk onderdeel dat het kan lezen. Een onderdeel dat midden in de gegevens is gestopt, wordt bewaard tot de plaats waar het stopte, dus u kunt de eerste rijen van een blad krijgen. Een onderdeel dat niet kan worden gelezen, wordt weggelaten. Als de lijst met onderdelen of de verbindingen ertussen ontbreken, maakt JeffCrunch ze opnieuw.

De gerepareerde werkmap opent zonder eigen bestand, met een naam als "Gerepareerd budget". Een melding vertelt wat tot een breuk is bewaard en wat is weggelaten. Het beschadigde bestand wordt niet gewijzigd. Sla de gerepareerde werkmap onder een nieuwe naam op, en controleer de getallen voordat u ze vertrouwt.

### Een .xls-bestand

Ook een .xls-bestand kan beschadigd zijn. Als de buitenste omhulling is afgebroken, zoekt **Openen en repareren** door het bestand naar het begin van de werkmap en leest vanaf daar. Een blad dat is afgebroken, behoudt de cellen voor de breuk. Wat na de breuk kwam, is verloren, en u wordt dat meegedeeld. De gerepareerde werkmap opent als kopie, dus sla die onder een nieuwe naam op.

## Grote werkmappen

JeffCrunch leest een werkmap op de achtergrond. De eerste rijen van het blad dat open was toen de werkmap werd opgeslagen, verschijnen bijna meteen, ook wanneer de werkmap erg groot is. De statusbalk linksonder vertelt wat het doet: **Openen**, **Lezen** van een blad met het aantal tot dan toe gelezen cellen, **Berekenen** met een percentage, en daarna **Gereed**.

U kunt scrollen, selecteren en kopiëren terwijl het leest. De tabbladen van de andere bladen worden actief zodra ze gelezen zijn. Een beschadigde werkmap wordt geopend voor zover ze kan worden gelezen, en een melding zegt wat er is weggelaten.

## Als een bestand niet opent

Wanneer JeffCrunch een bestand niet kan openen, zegt het in gewone woorden waarom, en blijven de werkmappen die u open hebt zoals ze zijn. Het onderscheidt een bestand dat leeg is van een bestand dat helemaal geen werkmap is (een pdf-document, een afbeelding, een tekstdocument of een presentatie, bijvoorbeeld); een zippakket dat is afgebroken of beschadigd, wat meestal betekent dat een download of kopie is onderbroken, dus kopieer het opnieuw van het origineel; een werkmap die met een wachtwoord is beveiligd, waarom het vraagt (zie [Met een wachtwoord beveiligde werkmappen openen](open-password)); en een samengesteld bestand dat geen werkmap bevat.

## Recente werkmappen

De pagina **Openen** van het scherm Bestand toont de werkmappen die u onlangs hebt geopend. Klik er met de rechtermuisknop op om die **vast te maken** boven aan de lijst, of om hem uit de lijst te verwijderen.

## Opslaan

Druk op **{key:Save}**, of klik op **Bestand** en daarna op **Opslaan**, om uw wijzigingen op te slaan in de werkmap die u hebt geopend. Om het origineel te bewaren en een nieuw bestand te maken, klikt u op **Bestand** en daarna op **Opslaan als**, en kiest u een naam. Een werkmap met macro's wordt opgeslagen als **.xlsm**-bestand, een sjabloon als **.xltx**- of **.xltm**-bestand, en elke andere als **.xlsx**-bestand. Een naam die op een ander soort werkmap eindigt, wordt niet geaccepteerd, en de melding zegt welke uitgang u moet gebruiken; de namen die eindigen op **.xls**, **.ods**, **.csv**, **.tsv** en **.txt** maken kopieën, zoals hierboven beschreven.

JeffCrunch slaat alleen op wat u hebt gewijzigd. Al het andere in het bestand, zoals de andere bladen, de grafieken, de afbeeldingen, de macro's en de instellingen die JeffCrunch niet toont, wordt byte voor byte gekopieerd zoals het was. In een blad dat u hebt gewijzigd, worden alleen de cellen die u hebt gewijzigd opnieuw geschreven, samen met de formules die ervan afhangen, die hun nieuwe waarden krijgen. Als u geen cel hebt gewijzigd, blijft het bestand ongemoeid.

Als een ander programma het bestand heeft gewijzigd nadat u het hebt geopend, slaat JeffCrunch niet over het bestand op zonder te vragen, omdat dat de wijzigingen van het andere programma zou laten verloren gaan. Het vraagt of u een **Kopie opslaan** wilt, het bestand wilt **Overschrijven**, of **Annuleren** wilt kiezen.

Het nieuwe bestand wordt eerst naast het oude geschreven, en het neemt de plaats van het oude alleen in wanneer het compleet is. Als er iets misgaat, ziet u een melding, en het bestand dat er stond, blijft zoals het was.

Als de formules nog worden berekend wanneer u opslaat, wacht JeffCrunch tot ze klaar zijn, zodat het bestand hun resultaten bevat. Als u ondertussen op **Esc** drukt om de berekening te stoppen, wordt het opslaan geannuleerd, en de statusbalk zegt dat; sla dan opnieuw op wanneer u wilt. JeffCrunch laat één ding weg uit het bestand: de lijst met de volgorde waarin een programma de formules het laatst heeft berekend (de berekeningsketen). Een programma dat de werkmap daarna opent, maakt die opnieuw. Als JeffCrunch sommige formules niet kon berekenen, of u de berekening hebt gestopt, vraagt het bestand aan het programma dat het later opent om elke formule opnieuw te berekenen.

## Sluiten met wijzigingen

Als u een werkmap sluit met wijzigingen die niet zijn opgeslagen, vraagt JeffCrunch of u ze wilt **Opslaan**, **Niet opslaan**, of **Annuleren** kiezen om door te werken. Een asterisk (*) in de titel van het venster laat zien dat er niet-opgeslagen wijzigingen zijn.

## Ingebouwde sjablonen

Onder de lege werkmap in **Bestand > Nieuw** staan kant-en-klare werkmappen in groepen: **Financiën** (een maandbudget, een leningschema en een kasboek), **Zakelijk** (een factuur, een onkostendeclaratie, een urenstaat, een voorraadlijst, een kilometerregistratie en een verkooprapport), **Planning** (een agenda en een projectplan) en **Onderwijs** (een cijferlijst). Elk toont een klein beeld van de bovenkant. Klik op een ervan om een nieuwe werkmap ervan te starten. Typ in de gele vakken. De andere getallen worden voor u berekend. De werkmap heeft pas een eigen bestand wanneer u ze opslaat.
