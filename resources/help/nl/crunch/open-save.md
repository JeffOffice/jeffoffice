# Werkmappen openen en opslaan
<!-- keywords: kopiëren, oudere indeling, opendocument, compatibiliteit, nieuw, leeg, boek1, naamloos, openen, opslaan, opslaan als, bestand, werkmap, xlsx, xlsm, xls, ods, csv, tsv, tekstbestand, gescheiden, importeren, exporteren, recent, vastmaken, slepen, neerzetten, laden, groot, omvangrijk, macro's, niet-opgeslagen, wijzigingen, sluiten, kopie, berekeningsketen, sjabloon, xltx, xltm, kan niet openen, beschadigd, versleuteld, wachtwoord, kopie opslaan, alleen-lezen, openen als kopie, nieuw venster, reservekopie, mijn sjablonen, sjabloon opslaan, ots, herstellen, openen en herstellen -->

Wanneer JeffCrunch start zonder bestand om te openen, bevat het venster een nieuwe, lege werkmap met één blad, genaamd Boek1. Elke nieuwe werkmap krijgt het volgende nummer (Boek2, Boek3, enzovoort), en een nummer wordt niet opnieuw gebruikt nadat het venster is gesloten. Om een andere werkmap te starten, drukt u op **{key:New}**, of klikt u op **Bestand**, dan op **Nieuw**, en kiest u **Lege werkmap**; deze wordt in een eigen venster geopend. Een nieuwe werkmap die u niet hebt gewijzigd, maakt plaats wanneer u een bestand opent in het venster; een werkmap die u wel hebt gewijzigd, blijft staan, en het bestand wordt in een ander venster geopend. **Opslaan** bij een nieuwe werkmap vraagt om een naam, net als **Opslaan als**, en slaat de werkmap op als een .xlsx-bestand. Wanneer u een nieuwe werkmap sluit die u niet hebt gewijzigd, wordt er niets gevraagd.

Om een werkmap te openen, drukt u op **{key:Open}**, of klikt u op **Bestand** en daarna op **Openen**, en kiest u **Bladeren**. U kunt een werkmap ook vanuit uw bestandsbeheer naar het venster slepen. Een werkmap die al in een venster geopend is, wordt niet nogmaals geopend: dat venster komt naar voren.

JeffCrunch opent **.xlsx**-werkmappen en **.xlsm**-werkmappen (die macro's bevatten), en de sjablonen **.xltx** en **.xltm** (een sjabloon die macro's bevat). Het voert macro's niet uit en laat ze ongemoeid: het macroproject, de handtekening ervan en de onderdelen die ernaar verwijzen, worden precies zoals ze waren teruggeschreven. Een sjabloon wordt opgeslagen als sjabloon, en een werkmap met macro's als werkmap met macro's, omdat het type deel uitmaakt van het bestand.

## Andere soorten bestanden

JeffCrunch opent ook oudere **.xls**-werkmappen, OpenDocument-spreadsheets (**.ods** en de platte **.fods**), en tekstbestanden met waarden (**.csv**, **.tsv** en **.txt**). Een werkmap die uit een van deze bestanden is geopend, wordt opgeslagen als een nieuw **.xlsx**-bestand (of als een kopie in **.xls**- of **.ods**-indeling: zie hieronder). Wanneer u op **{key:Save}** drukt, meldt JeffCrunch dat en vraagt om een naam; het geopende bestand wordt nooit gewijzigd.

Een OpenDocument-spreadsheet wordt gelezen met de bladen, waarden, formules (met de resultaten die het bestand bewaarde), getalnotaties, lettertypen, opvullingen, randen, samengevoegde cellen, kolombreedten en rijhoogten, verborgen rijen en kolommen, vastgezette rijen en kolommen, en namen. Grafieken, afbeeldingen en andere tekeningen, opmerkingen, draaitabellen, macro's, voorwaardelijke opmaak en gegevensvalidatie worden nog niet gelezen; een melding bovenaan vertelt welke van deze het bestand heeft, en een kopie die van hier wordt opgeslagen, bevat ze niet. Een spreadsheet die met een wachtwoord is beveiligd, vraagt bij het openen om het wachtwoord: zie [Met een wachtwoord beveiligde werkmappen openen](open-password).

Een tekstbestand is één blad. JeffCrunch bepaalt hoe de waarden zijn gescheiden (door komma's, puntkomma's, tabs of verticale strepen) en hoe de letters zijn gecodeerd (UTF-8 of UTF-16, anders de codepagina van uw taal), en leest elke waarde zoals u die in uw taal zou typen: een getal zoals 1.234,5 in het Duits, een percentage, een bedrag, een datum met jaartal (zoals 3/15/2024, of 15.03.2024 in het Duits), een tijd, WAAR of ONWAAR (in uw taal, zoals WAHR en FALSCH in het Duits, en in het Engels). Een waarde die begint met een is-gelijkteken, is een formule. Cijfers die beginnen met een nul (zoals 007) en getallen van meer dan 15 cijfers blijven tekst, zodat een postcode of een rekeningnummer niet wordt gewijzigd, en dat geldt ook voor een dag en een maand zonder jaar, in welke volgorde en in welke taal ook (zoals 3/4, 12.5, 1-2, 1 maart of 25 dec), omdat het jaar dan zou moeten worden verzonnen, en voor teksten zoals het gennaam MARCH1. Een blad bevat 1.048.576 rijen en 16.384 kolommen; als het bestand meer heeft, opent JeffCrunch wat past en laat het u weten.

Wanneer u een tekstbestand opent, toont een venster eerst de eerste rijen en vraagt hoe het wordt gelezen: wat de velden scheidt (een komma, puntkomma, tab, spatie, verticale streep of een teken dat u typt, met eventueel meerdere scheidingstekens achter elkaar als één geteld), wat een veld met een scheidingsteken tussen aanhalingstekens zet, de tekstcodering (Unicode, de Windows-codepagina's, de ISO 8859-pagina's, Mac Roman en de Oost-Aziatische), en de rij waarop wordt begonnen. Boven elke kolom kiest u hoe die wordt gelezen: **Zoals getypt**, **Tekst** (om een onderdeelnummer zoals 007 of een lang rekeningnummer ongewijzigd te houden), een **Datum** in de volgorde die u aangeeft, of **Overslaan**. De voorbeeldweergave volgt elke wijziging en toont hooguit de eerste 200 rijen en 100 kolommen, maar het hele bestand wordt geopend. **Bestand > Openen > Tekstbestand openen** vraagt het ook wanneer de naam van het bestand ongebruikelijk is.

Om alleen de waarden van het blad te bewaren dat u bekijkt, klikt u op **Bestand**, daarna op **Opslaan als**, en kiest u het bestandstype **.csv** (of **.tsv** en **.txt**, met tabs als scheidingsteken). Voordat JeffCrunch het bestand schrijft, meldt het wat uw werkmap heeft dat het bestand niet bevat (de andere bladen, formules, afbeeldingen, grafieken, notities en macro's, bijvoorbeeld) en vraagt hoe het moet worden geschreven, en of u **Alleen waarden opslaan** wilt, **Opslaan als .xlsx** om alles te bewaren, of **Annuleren**:

- **Scheidingsteken.** Een komma, puntkomma, tab, spatie, verticale streep of een teken dat u typt. Een **.csv** begint met een puntkomma waar uw taal een komma in getallen schrijft, en anders met een komma; een **.tsv** of **.txt** met een tab.
- **Tekstcodering.** UTF-8 met markering (zodat andere programma's accenten goed overnemen), UTF-8 zonder markering, UTF-16, of een codepagina zoals West-Europees (1252), de ISO 8859-pagina's of Mac Roman. Een letter waarvoor de codepagina geen byte heeft, wordt geschreven als een vraagteken, en de statusbalk vertelt hoeveel het er zijn.
- **Regeleinden.** Windows (wagenterugloop en regelinvoer), Linux en Mac (regelinvoer), of oude Mac (wagenterugloop).
- **Cellen.** **Zoals weergegeven in de cellen** schrijft wat u ziet. **Zoals opgeslagen** schrijft elk cijfer van een getal (0,1 en niet 10%), en een datum als ISO-tekst zoals 2026-03-04.

Tekst komt tussen aanhalingstekens wanneer ze het scheidingsteken, een aanhalingsteken, een regeleinde of een spatie aan een van de uiteinden bevat. De andere bladen, de formules, de opmaak en de instellingen komen niet in het bestand, en uw werkmap wordt niet gewijzigd door het schrijven van een bestand.

Een werkmap die uit een tekstbestand komt, wordt teruggeschreven in de vorm die het bestand had: met hetzelfde scheidingsteken, dezelfde regeleinden en dezelfde codering. Als u die als kopie hebt opgeslagen zonder een cel te wijzigen, is de kopie hetzelfde bestand, byte voor byte, en er wordt niets gevraagd.

## Een kopie opslaan als .xls of .ods

Om een kopie te maken die oudere programma's, of programma's die OpenDocument lezen, kunnen openen, klikt u op **Bestand**, daarna op **Opslaan als**, en kiest u het bestandstype **.xls** of **.ods**. De kopie wordt gemaakt van uw werkmap zoals die nu is, met de wijzigingen die u nog niet hebt opgeslagen, en bevat:

- de waarden, met hun teksten, getallen, waarheidswaarden en fouten;
- de formules, met de resultaten die ze nu hebben (een matrixformule blijft een matrixformule);
- de getalnotaties, lettertypen, opvullingen, randen, uitlijning, en of een cel is vergrendeld;
- samengevoegde cellen, kolombreedten en rijhoogten, verborgen rijen en kolommen, rij- en kolomoverzichten, en vastgezette rijen en kolommen;
- de bladen, in hun volgorde, met hun namen en welke verborgen zijn, en de namen die u hebt gedefinieerd (en de afdrukbereiken);
- de koppelingen, en hoe elk blad wordt afgedrukt (het papier, de marges, de afdrukstand, de schaal, koptekst en voettekst, en de pagina-einden).

Een **.xls**-bestand bevat ook gegevenstabellen en bewaart de rekeninstellingen van de werkmap. Een kopie kan niet alles bevatten. Voordat JeffCrunch het bestand schrijft, meldt het wat uw werkmap heeft dat het bestand niet bevat: grafieken, afbeeldingen en andere tekeningen, notities, draaitabellen, tabellen, voorwaardelijke opmaak, gegevensvalidatie, filters, bladbeveiliging, formulierbesturingselementen, macro's en koppelingen naar andere werkmappen (de formules die ze gebruiken, behouden de waarden die ze hadden). Een formule die de oudere indeling niet kan weergeven (een functie die het niet kent, of een cel na kolom IV of rij 65.536), wordt geschreven als de waarde die eruit kwam, en de melding telt ze. De oudere indeling eindigt ook bij 65.536 rijen, 256 kolommen, 255 bladen en 32.767 tekens in een cel, en bij ongeveer 4.000 celopmaken en 56 kleuren: wat niet past, wordt afgesneden, en de melding zegt hoeveel. U kunt daarna kiezen voor **Kopie opslaan**, voor **Opslaan als .xlsx** om alles te bewaren, of voor **Annuleren**.

Uw werkmap wordt door de kopie niet gewijzigd, en wordt niet als opgeslagen gemarkeerd. Het bestand dat u hebt geopend, wordt nooit geschreven: geef de kopie een andere naam. Een kopie van een **.xls**- of **.ods**-bestand dat u hebt geopend, kan in het andere type worden opgeslagen.

## Een kopie opslaan

Klik op **Bestand > Kopie opslaan** en kies een naam. JeffCrunch schrijft de werkmap naar dat bestand, met de wijzigingen die u nog niet hebt opgeslagen, en u werkt verder in de werkmap die u had. De naam en de niet-opgeslagen wijzigingen blijven zoals ze waren. U kunt elk bestandstype kiezen dat **Opslaan als** aanbiedt. Een kopie wordt nooit over het eigen bestand van de werkmap geschreven.

## Een reservekopie bewaren

Klik op **Bestand > Opties** en zet op de pagina **Algemeen** de optie **Altijd een reservekopie maken** aan. Elke keer dat u opslaat, kopieert JeffCrunch eerst het oude bestand naar een bestand met de naam "Reservekopie van" plus de naam van het bestand, in dezelfde map. Als de kopie niet kan worden gemaakt, vraagt het programma of u zonder kopie wilt opslaan. Het antwoord is Nee, tenzij u Ja kiest, en het bestand blijft zoals het was.

## Opslaan als sjabloon

Een sjabloon is een werkmap die u als uitgangspunt gebruikt. Ze kan formules, stijlen en bladen bevatten. Klik op **Bestand > Opslaan als sjabloon** en typ een naam. De sjabloon (een **.xltx**-bestand) komt in **Mijn sjablonen**, en uw werkmap wordt niet gewijzigd. In **Opslaan als** kunt u ook **Sjablonen (*.xltx)** of **OpenDocument-spreadsheetsjablonen (*.ots)** kiezen, om een sjabloonbestand op de gewenste plek op te slaan.

Om een werkmap te starten vanuit uw sjabloon, klikt u op **Bestand > Nieuw**. Uw sjablonen staan onder **Mijn sjablonen**. Klik op een sjabloon. Er wordt een nieuwe werkmap geopend met de cellen en bladen van de sjabloon. Die heeft nog geen eigen bestand, dus **Opslaan** vraagt om een naam, en de sjabloon wordt niet gewijzigd. Om een sjabloon uit de lijst te verwijderen, klikt u er met de rechtermuisknop op en kiest u **Verwijderen uit Mijn sjablonen**.

## Andere manieren om een bestand te openen

Op de pagina **Openen** klikt u op **Meer manieren om te openen**. U kunt ook met de rechtermuisknop op een naam in de lijst Recent klikken.

- **Openen als alleen-lezen** opent de werkmap alleen om te lezen. U kunt bekijken, scrollen, zoeken en kopiëren, maar geen cel wijzigen. De titelbalk zegt "Alleen-lezen". Als u op **Opslaan** drukt, vraagt JeffCrunch om een nieuwe naam, zodat het bestand dat u hebt geopend niet wordt gewijzigd.
- **Openen als kopie** opent de werkmap zonder eigen bestand. **Opslaan** vraagt waar het moet worden opgeslagen. Het bestand dat u hebt geopend, wordt nooit gewijzigd.
- **Openen in nieuw venster** opent de werkmap in een eigen venster, ook wanneer het venster waarin u zit leeg is.
- **Openen en herstellen** is voor een beschadigd .xlsx-bestand. Zie hieronder.

## Openen en herstellen

Soms is een .xlsx-bestand beschadigd. Een download is halverwege gestopt, of een schijf heeft fouten gemaakt. Een .xlsx-bestand is een bundel van kleinere onderdelen, zoals de bladen, de stijlen en de gedeelde tekst. **Openen en herstellen** leest elk onderdeel dat het kan lezen. Een onderdeel dat halverwege stopte, wordt bewaard tot de plaats waar het stopte, zodat u mogelijk alleen de eerste rijen van een blad krijgt. Een onderdeel dat niet kan worden gelezen, wordt weggelaten. Als de lijst van onderdelen of de verbindingen ertussen verloren zijn, maakt JeffCrunch ze opnieuw.

De hersteld werkmap wordt geopend zonder eigen bestand, met een naam zoals "Hersteld budget". Een melding vertelt wat tot een onderbreking is bewaard en wat is weggelaten. Het beschadigde bestand wordt niet gewijzigd. Sla de hersteld werkmap op onder een nieuwe naam, en controleer de getallen voordat u ze vertrouwt.

## Grote werkmappen

JeffCrunch leest een werkmap op de achtergrond. De eerste rijen van het blad dat open was toen de werkmap werd opgeslagen, verschijnen bijna meteen, ook wanneer de werkmap erg groot is. De statusbalk linksonder vertelt wat het programma doet: **Openen**, **Lezen** van een blad met het aantal tot nu toe gelezen cellen, **Berekenen** met een percentage, en dan **Gereed**.

U kunt scrollen, selecteren en kopiëren terwijl het leest. De tabbladen van de andere bladen worden actief zodra ze zijn gelezen. Een werkmap die beschadigd is, wordt geopend zover als hij kan worden gelezen, en een melding zegt wat er is weggelaten.

## Als een bestand niet opent

Wanneer JeffCrunch een bestand niet kan openen, zegt het in duidelijke taal waarom, en blijven de werkmappen die u open hebt, zoals ze zijn. Het onderscheidt een bestand dat leeg is van een bestand dat helemaal geen werkmap is (een PDF-document, een afbeelding, een tekstdocument of een presentatie, bijvoorbeeld); een zip-pakket dat is afgebroken of beschadigd, wat meestal betekent dat een download of kopie is onderbroken, dus kopieer het opnieuw vanaf het origineel; een werkmap die met een wachtwoord is beveiligd, waarvoor het programma om het wachtwoord vraagt (zie [Met een wachtwoord beveiligde werkmappen openen](open-password)); en een samengesteld bestand dat geen werkmap bevat.

## Recente werkmappen

De pagina **Openen** van het scherm Bestand toont de werkmappen die u onlangs hebt geopend. Klik met de rechtermuisknop op een werkmap om die **vast te maken** aan de bovenkant van de lijst, of om hem uit de lijst te verwijderen.

## Opslaan

Druk op **{key:Save}**, of klik op **Bestand** en daarna op **Opslaan**, om uw wijzigingen in de werkmap die u hebt geopend op te slaan. Om het origineel te bewaren en een nieuw bestand te maken, klikt u op **Bestand** en daarna op **Opslaan als**, en kiest u een naam. Een werkmap met macro's wordt opgeslagen als een **.xlsm**-bestand, een sjabloon als een **.xltx**- of **.xltm**-bestand, en elke andere werkmap als een **.xlsx**-bestand. Een naam die eindigt op een ander type werkmap wordt niet geaccepteerd, en de melding zegt welke extensie u moet gebruiken; de namen die eindigen op **.xls**, **.ods**, **.csv**, **.tsv** en **.txt** maken kopieën, zoals hierboven beschreven.

JeffCrunch slaat alleen op wat u hebt gewijzigd. Al het overige in het bestand, zoals de andere bladen, de grafieken, de afbeeldingen, de macro's en de instellingen die JeffCrunch niet toont, wordt byte voor byte overgenomen zoals het was. In een blad dat u hebt gewijzigd, worden alleen de cellen die u hebt gewijzigd opnieuw geschreven, samen met de formules die ervan afhangen; die krijgen hun nieuwe waarden. Als u geen cel hebt gewijzigd, blijft het bestand ongemoeid.

Als een ander programma het bestand heeft gewijzigd nadat u het hebt geopend, slaat JeffCrunch niet over het bestand heen zonder te vragen, omdat de wijzigingen van dat andere programma dan verloren zouden gaan. Het vraagt of u **Een kopie opslaan**, het bestand **Overschrijven**, of **Annuleren** wilt kiezen.

Het nieuwe bestand wordt eerst naast het oude geschreven, en neemt de plaats van het oude alleen in wanneer het compleet is. Als er iets misgaat, ziet u een melding, en het bestand dat er stond, blijft zoals het was.

Als de formules nog worden berekend wanneer u opslaat, wacht JeffCrunch tot de berekening klaar is, zodat het bestand de resultaten bevat. Als u ondertussen op **Esc** drukt om de berekening te stoppen, wordt het opslaan geannuleerd en de statusbalk meldt dat; sla opnieuw op wanneer u dat wilt. JeffCrunch laat één ding weg uit het bestand: de lijst van de volgorde waarin een programma de formules het laatst heeft berekend (de berekeningsketen). Een programma dat de werkmap daarna opent, maakt die opnieuw. Als JeffCrunch sommige formules niet heeft kunnen berekenen, of u hebt de berekening gestopt, vraagt het bestand aan dat programma om alle formules opnieuw te berekenen wanneer het de werkmap opent.

## Sluiten met wijzigingen

Als u een werkmap sluit die niet-opgeslagen wijzigingen heeft, vraagt JeffCrunch of u ze wilt **Opslaan**, **Niet opslaan**, of **Annuleren** en doorgaan met werken. Een asterisk (*) in de titel van het venster laat zien dat er niet-opgeslagen wijzigingen zijn.
