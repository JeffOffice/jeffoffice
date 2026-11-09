# Werkmappen openen en opslaan
<!-- keywords: onherstelbaar werk herstellen, automatisch herstel, vastlopen, kopie, oudere indeling, opendocument, compatibiliteit, nieuw, leeg, boek1, naamloos, openen, opslaan, opslaan als, bestand, werkmap, xlsx, xlsm, xls, ods, csv, tsv, tekstbestand, gescheiden, importeren, exporteren, recent, vastzetten, slepen, neerzetten, laden, groot, macro's, niet-opgeslagen, wijzigingen, sluiten, kopie, berekeningsketen, sjabloon, xltx, xltm, kan niet worden geopend, beschadigd, versleuteld, wachtwoord, kopie opslaan, alleen-lezen, alleen lezen, openen als kopie, nieuw venster, reservekopie, mijn sjablonen, sjabloon opslaan, ots, herstellen, openen en herstellen, ingebouwde sjablonen, budget, factuur, urenstaat, lening, agenda, cijferlijst, vaste breedte, vaste kolommen, decimaalteken, duizendtalteken, minteken achteraan, html, webpagina, htm, tabellen uit een pagina -->

Wanneer JeffCrunch start zonder bestand om te openen, bevat het venster een nieuwe, lege werkmap met één blad, met de naam Boek1. Elke nieuwe werkmap krijgt het volgende nummer (Boek2, Boek3, enzovoort), en een nummer wordt niet opnieuw gebruikt nadat het venster is gesloten. Om een andere werkmap te starten, drukt u op **{key:New}** of klikt u op **Bestand**, daarna op **Nieuw**, en kiest u **Lege werkmap**; die wordt in een eigen venster geopend. Een nieuwe werkmap die u niet hebt gewijzigd, maakt plaats wanneer u een bestand opent in het venster; een werkmap die u wel hebt gewijzigd, blijft staan, en het bestand wordt in een ander venster geopend. **Opslaan** bij een nieuwe werkmap vraagt om een naam, net als **Opslaan als**, en slaat de werkmap op als .xlsx-bestand. Een nieuwe werkmap die u niet hebt gewijzigd, sluiten zonder dat er iets wordt gevraagd.

Om een werkmap te openen, drukt u op **{key:Open}** of klikt u op **Bestand** en daarna op **Openen**, en kiest u **Bladeren**. U kunt een werkmap ook vanuit uw bestandsbeheer naar het venster slepen. Een werkmap die al in een venster is geopend, wordt niet opnieuw geopend: dat venster komt naar voren.

JeffCrunch opent **.xlsx**-werkmappen en **.xlsm**-werkmappen (die macro's bevatten), en de sjablonen **.xltx** en **.xltm** (een sjabloon met macro's). JeffCrunch voert macro's niet uit en laat ze ongemoeid: het macroproject, de handtekening ervan en de onderdelen die ernaar verwijzen, worden precies zoals ze waren teruggeschreven. Een sjabloon wordt als sjabloon opgeslagen, en een werkmap met macro's als werkmap met macro's, omdat het soort deel is van het bestand.

## Niet-opgeslagen werk herstellen

Als JeffCrunch onverwacht wordt afgesloten, bijvoorbeeld door een stroomstoring, toont de volgende start een lijst met de kopieën die het van uw niet-opgeslagen werk heeft bewaard. Kies welke u wilt openen en klik op **Openen**; elke kopie wordt geopend als nieuwe werkmap zonder bestand, dus klik op **Opslaan** en kies een naam. **Verwijderen** gooit de aangevinkte kopieën weg, en **Niet nu** laat ze staan voor later. U kunt de lijst op elk moment opvragen via **Bestand > Openen > Niet-opgeslagen werk herstellen**. Zie [JeffCrunch-instellingen wijzigen](options) voor hoe vaak de kopieën worden gemaakt.

## Andere soorten bestanden

JeffCrunch opent ook oudere **.xls**-werkmappen, OpenDocument-spreadsheets (**.ods** en de platte **.fods**) en tekstbestanden met waarden (**.csv**, **.tsv** en **.txt**). Een werkmap die uit een van deze bestanden is geopend, wordt opgeslagen als nieuw **.xlsx**-bestand (of als **.xls**- of **.ods**-kopie: zie hieronder). Wanneer u op **{key:Save}** drukt, meldt JeffCrunch dat en vraagt om een naam; het bestand dat u hebt geopend, wordt nooit gewijzigd.

Een OpenDocument-spreadsheet wordt gelezen met de bladen, waarden, formules (met de resultaten die het bestand bewaarde), getalnotaties, lettertypen, opvullingen, randen, samengevoegde cellen, kolombreedten en rijhoogten, verborgen rijen en kolommen, vastgezette rijen en kolommen, en namen. Grafieken, afbeeldingen en andere tekeningen, opmerkingen, draaitabellen, macro's, voorwaardelijke opmaak en gegevensvalidatie worden nog niet gelezen; een bericht bovenaan vertelt u welke het bestand heeft, en een kopie die vanaf hier wordt opgeslagen, bevat ze niet. Een spreadsheet die met een wachtwoord is beveiligd, vraagt erom wanneer u het opent: zie [Met een wachtwoord beveiligde werkmappen openen](open-password).

Een tekstbestand is één blad. JeffCrunch bepaalt hoe de waarden zijn gescheiden (door komma's, puntkomma's, tabs of verticale strepen) en hoe de letters zijn gecodeerd (UTF-8 of UTF-16, anders de codepagina van uw taal), en leest elke waarde zoals u die in uw taal zou typen: een getal zoals 1.234,5 in het Duits, een percentage, een bedrag, een datum met jaar (zoals 15-3-2024 of 15.03.2024 in het Duits), een tijd, WAAR of ONWAAR (in uw taal, zoals WAHR en FALSCH in het Duits, en TRUE en FALSE in het Engels). Een waarde die begint met een gelijkteken is een formule. Cijfers die beginnen met een nul (zoals 007) en getallen met meer dan 15 cijfers blijven tekst, zodat een postcode of een rekeningnummer niet wordt gewijzigd. Dat geldt ook voor een dag en een maand zonder jaar, in welke volgorde en welke taal ook (zoals 3/4, 12,5, 1-2, 1 maart of 25 dec), omdat het jaar dan verzonnen zou moeten worden, en voor teksten zoals de gennaam MARCH1. Een blad bevat 1.048.576 rijen en 16.384 kolommen; heeft het bestand er meer, dan opent JeffCrunch wat past en laat u dat weten.

Wanneer u een tekstbestand opent, toont een venster eerst de eerste rijen en vraagt het hoe het wordt gelezen: wat de velden scheidt (een komma, puntkomma, tab, spatie, verticale streep of een teken dat u typt, met de optie om meerdere scheidingstekens achter elkaar als één te tellen), wat een veld omsluit dat een scheidingsteken bevat, de tekstcodering (Unicode, de Windows-codepagina's, de ISO 8859-pagina's, Mac Roman en de Oost-Aziatische), en de rij waarop wordt begonnen. Boven elke kolom kiest u hoe die wordt gelezen: **Zoals getypt**, **Tekst** (om een onderdeelnummer zoals 007 of een lang rekeningnummer ongewijzigd te houden), een **Datum** in de volgorde die u aangeeft, of **Overslaan**. Sommige bestanden zetten waarden in kolommen van dezelfde breedte, met spaties ertussen en zonder scheidingsteken. Vink **De kolommen hebben een vaste breedte** aan, en de eerste regels van het bestand verschijnen in een vak. Klik tussen twee tekens om daar een kolom te laten beginnen, en klik opnieuw om die weg te halen; of typ de posities bij **Kolommen beginnen bij**, zoals 10, 25, 40. Elk deel wordt op die posities geknipt en de spaties eromheen worden weggehaald. Met **Decimaalteken** en **Duizendtalteken** geeft u aan hoe getallen in dit bestand zijn geschreven, ongeacht uw eigen taal: 1.234,50 is dus duizend tweehonderd vierendertig en een half wanneer het bestand een komma als decimaalteken gebruikt. Vink **Een minteken kan na het getal staan** aan voor bestanden die -12 als 12- schrijven. De voorbeeldweergave volgt elke wijziging en toont hooguit de eerste 200 rijen en 100 kolommen, maar het hele bestand wordt geopend. **Bestand > Openen > Tekstbestand openen** vraagt het ook wanneer de naam van het bestand ongebruikelijk is.

JeffCrunch opent ook **webpagina's** (**.html** en **.htm**), en een pagina die met de naam van een werkmap is opgeslagen. Elke tabel op de pagina wordt een blad, genoemd **Tabel1**, **Tabel2**, enzovoort. De cellen worden gelezen alsof u ze had getypt: een getal is een getal en een datum is een datum. Een cel die over meerdere kolommen of rijen is samengevoegd, laat lege cellen achter, zodat de andere cellen in hun kolommen blijven. De woorden buiten de tabellen, de afbeeldingen en de opmaak van de pagina worden niet gelezen, en een pagina zonder tabel wordt niet geopend. Niets wat de pagina noemt, wordt van internet gehaald.

Om alleen de waarden van het blad waarin u kijkt op te slaan, klikt u op **Bestand**, daarna op **Opslaan als**, en kiest u het **.csv**-bestandstype (of **.tsv** en **.txt**, met tabs als scheidingsteken). Voordat JeffCrunch het bestand schrijft, meldt het wat uw werkmap heeft dat het bestand niet zal bevatten (bijvoorbeeld de andere bladen, formules, afbeeldingen, grafieken, notities en macro's), en vraagt het hoe het moet worden geschreven: met **Alleen waarden opslaan**, met **Opslaan als .xlsx** om alles te behouden, of met **Annuleren**:

- **Scheidingsteken.** Een komma, puntkomma, tab, spatie, verticale streep of een teken dat u typt. Een **.csv** begint met een puntkomma waar uw taal een komma gebruikt in getallen, en anders met een komma; een **.tsv** of **.txt** met een tab.
- **Tekstcodering.** UTF-8 met markering (zodat andere programma's accenten goed overnemen), UTF-8 zonder markering, UTF-16, of een codepagina zoals West-Europees (1252), de ISO 8859-pagina's, Mac Roman, of de Japanse, Chinese, Koreaanse, Hebreeuwse, Arabische, Baltische, Vietnamese en Thaise pagina's. Een letter waarvoor de codepagina geen byte heeft, wordt als vraagteken geschreven, en de statusbalk zegt hoeveel het er zijn.
- **Regeleinden.** Windows (carriage return en line feed), Linux en Mac (line feed), of oude Mac (carriage return).
- **Cellen.** **Zoals weergegeven in de cellen** schrijft wat u ziet. **Zoals opgeslagen** schrijft elk cijfer van een getal (0,1 en niet 10%), en een datum als ISO-tekst zoals 2026-03-04.

Tekst komt tussen aanhalingstekens wanneer die het scheidingsteken, een aanhalingsteken, een regeleinde of een spatie aan het begin of einde bevat. De andere bladen, de formules, de opmaak en de instellingen staan niet in het bestand, en uw werkmap wordt niet gewijzigd door het schrijven ervan.

Een werkmap die uit een tekstbestand komt, wordt teruggeschreven in de vorm die het bestand had: met hetzelfde scheidingsteken, dezelfde regeleinden en dezelfde codering. Als u hem als kopie hebt opgeslagen zonder een cel te wijzigen, is de kopie hetzelfde bestand, byte voor byte, en wordt er niets gevraagd.

## Een kopie opslaan als .xls of .ods

Om een kopie te maken die oudere programma's, of programma's die OpenDocument lezen, kunnen openen, klikt u op **Bestand**, daarna op **Opslaan als**, en kiest u het bestandstype **.xls** of **.ods**. De kopie wordt gemaakt van uw werkmap zoals die nu is, met de wijzigingen die u nog niet hebt opgeslagen, en bevat:

- de waarden, met hun teksten, getallen, waarheidswaarden en fouten;
- de formules, met de resultaten die ze nu hebben (een matrixformule blijft een matrixformule);
- de getalnotaties, lettertypen, opvullingen, randen, uitlijning, en of een cel is vergrendeld;
- samengevoegde cellen, kolombreedten en rijhoogten, verborgen rijen en kolommen, rij- en kolomgroepen, en vastgezette rijen en kolommen;
- de bladen, in hun volgorde, met hun namen en welke verborgen zijn, en de namen die u hebt gedefinieerd (en de afdrukbereiken);
- de koppelingen, en hoe elk blad wordt afgedrukt (het papier, de marges, de afdrukstand, de schaal, de koptekst en voettekst, en de pagina-einden).

Een **.xls**-bestand bevat ook gegevenstabellen en bewaart de rekeninstellingen van de werkmap. Een kopie kan niet alles bevatten. Voordat JeffCrunch het bestand schrijft, meldt het wat uw werkmap heeft dat het bestand niet zal bevatten: grafieken, afbeeldingen en andere tekeningen, notities, draaitabellen, tabellen, voorwaardelijke opmaak, gegevensvalidatie, filters, bladbeveiliging, formulierbesturingselementen, macro's en koppelingen naar andere werkmappen (de formules die ze gebruiken, houden de waarden die ze hadden). Een formule die de oudere vorm niet kan weergeven (een functie die die vorm niet kent, of een cel na kolom IV of na rij 65.536), wordt geschreven als de waarde die eruit kwam, en het bericht telt ze. De oudere vorm stopt ook bij 65.536 rijen, 256 kolommen, 255 bladen en 32.767 tekens in een cel, en bij ongeveer 4.000 celopmaken en 56 kleuren: wat niet past, wordt afgekapt, en het bericht zegt hoeveel. U kunt dan kiezen om **Kopie opslaan** te kiezen, om **Opslaan als .xlsx** te kiezen en alles te behouden, of om **Annuleren** te kiezen.

Uw werkmap wordt door de kopie niet gewijzigd, en hij wordt niet als opgeslagen gemarkeerd. Het bestand dat u hebt geopend, wordt nooit geschreven: geef de kopie een andere naam. Een kopie van een **.xls**- of **.ods**-bestand dat u hebt geopend, kan in het andere soort worden opgeslagen.

## Een kopie opslaan

Klik op **Bestand > Kopie opslaan** en kies een naam. JeffCrunch schrijft de werkmap naar dat bestand, met de wijzigingen die u nog niet hebt opgeslagen, en u werkt verder in de werkmap die u had. De naam en de niet-opgeslagen wijzigingen blijven zoals ze waren. U kunt elk bestandstype kiezen dat **Opslaan als** aanbiedt. Een kopie wordt nooit over het eigen bestand van de werkmap geschreven.

## Een reservekopie bewaren

Klik op **Bestand > Opties**, en zet op de pagina **Algemeen** de optie **Altijd een reservekopie maken** aan. Elke keer dat u opslaat, kopieert JeffCrunch eerst het oude bestand naar een bestand met de naam "Reservekopie van" plus de naam van het bestand, in dezelfde map. Als de kopie niet kan worden gemaakt, vraagt de app of u zonder wilt opslaan. Het antwoord is Nee, tenzij u Ja kiest; het bestand blijft dan zoals het was.

## Opslaan als sjabloon

Een sjabloon is een werkmap die u als uitgangspunt gebruikt. Hij kan formules, stijlen en bladen bevatten. Klik op **Bestand > Opslaan als sjabloon** en typ een naam. De sjabloon (een **.xltx**-bestand) komt in **Mijn sjablonen**, en uw werkmap wordt niet gewijzigd. In **Opslaan als** kunt u ook **Sjablonen (*.xltx)** of **OpenDocument-spreadsheetsjablonen (*.ots)** kiezen, om een sjabloonbestand op de gewenste plek op te slaan.

Om een werkmap van uw sjabloon te starten, klikt u op **Bestand > Nieuw**. Uw sjablonen staan onder **Mijn sjablonen**. Klik op een sjabloon. Er opent een nieuwe werkmap met de cellen en bladen van de sjabloon. Die heeft nog geen eigen bestand, dus **Opslaan** vraagt om een naam, en de sjabloon wordt niet gewijzigd. Om een sjabloon uit de lijst te verwijderen, klikt u er met de rechtermuisknop op en kiest u **Verwijderen uit Mijn sjablonen**.

## Andere manieren om een bestand te openen

Klik op de pagina **Openen** op **Meer manieren om te openen**. U kunt ook met de rechtermuisknop op een naam in de lijst Recent klikken.

- **Openen als alleen-lezen** opent de werkmap alleen om te lezen. U kunt kijken, scrollen, zoeken en kopiëren, maar geen cel wijzigen. In de titelbalk staat "Alleen-lezen". Als u op **Opslaan** drukt, vraagt JeffCrunch om een nieuwe naam, zodat het bestand dat u hebt geopend niet wordt gewijzigd.
- **Openen als kopie** opent de werkmap zonder eigen bestand. **Opslaan** vraagt waar het moet komen. Het bestand dat u hebt geopend, wordt nooit gewijzigd.
- **Openen in nieuw venster** opent de werkmap in een eigen venster, ook wanneer het venster waarin u zit leeg is.
- **Openen en herstellen** is voor een beschadigd .xlsx- of .xls-bestand. Zie hieronder.

## Openen en herstellen

Soms is een .xlsx-bestand beschadigd. Een download is halverwege gestopt, of een schijf heeft fouten gemaakt. Een .xlsx-bestand is een bundel van kleinere onderdelen, zoals de bladen, de stijlen en de gedeelde tekst. **Openen en herstellen** leest elk onderdeel dat het kan lezen. Een onderdeel dat halverwege stopte, wordt bewaard tot de plek waar het stopte, dus u kunt de eerste rijen van een blad krijgen. Een onderdeel dat niet kan worden gelezen, wordt weggelaten. Als de lijst van de onderdelen of de verbindingen ertussen weg zijn, maakt JeffCrunch ze opnieuw.

De hersteld werkmap wordt geopend zonder eigen bestand, met een naam zoals "Hersteld budget". Een bericht vertelt wat tot een onderbreking is bewaard en wat is weggelaten. Het beschadigde bestand wordt niet gewijzigd. Sla de hersteld werkmap op onder een nieuwe naam, en controleer de getallen voordat u erop vertrouwt.

### Een .xls-bestand

Ook een .xls-bestand kan beschadigd zijn. Als de buitenste omhulling is afgekapt, zoekt **Openen en herstellen** door het bestand naar het begin van de werkmap en leest vanaf daar. Een blad dat is afgekapt, bewaart de cellen voor de afkapping. Wat na de afkapping kwam, is verloren, en u wordt dat verteld. De hersteld werkmap wordt als kopie geopend, dus sla hem op onder een nieuwe naam.

## Grote werkmappen

JeffCrunch leest een werkmap op de achtergrond. De eerste rijen van het blad dat open was toen de werkmap werd opgeslagen, verschijnen bijna meteen, ook wanneer de werkmap erg groot is. De statusbalk linksonder laat zien wat er gebeurt: **Openen**, **Lezen** van een blad met het aantal tot dan gelezen cellen, **Berekenen** met een percentage, en daarna **Gereed**.

U kunt scrollen, selecteren en kopiëren terwijl er wordt gelezen. De tabbladen van de andere bladen worden actief zodra ze gelezen zijn. Een beschadigde werkmap wordt zo ver geopend als er kan worden gelezen, en een bericht zegt wat is weggelaten.

## Als een bestand niet opent

Wanneer JeffCrunch een bestand niet kan openen, zegt het met gewone woorden waarom, en blijven de werkmappen die u open hebt, zoals ze zijn. Het onderscheidt een bestand dat leeg is van een bestand dat helemaal geen werkmap is (bijvoorbeeld een pdf-document, een afbeelding, een tekstdocument of een presentatie); een zip-pakket dat is afgekapt of beschadigd, wat meestal betekent dat een download of kopie is onderbroken, dus kopieer het opnieuw vanaf het origineel; een werkmap die met een wachtwoord is beveiligd, waar het om vraagt (zie [Met een wachtwoord beveiligde werkmappen openen](open-password)); en een samengesteld bestand dat geen werkmap bevat.

## Recente werkmappen

De pagina **Openen** van het scherm Bestand toont de werkmappen die u onlangs hebt geopend. Klik met de rechtermuisknop op een werkmap om hem **vast te zetten** bovenaan de lijst, of om hem uit de lijst te verwijderen.

## Opslaan

Druk op **{key:Save}**, of klik op **Bestand** en daarna op **Opslaan**, om uw wijzigingen op te slaan in de werkmap die u hebt geopend. Om het origineel te bewaren en een nieuw bestand te maken, klikt u op **Bestand** en daarna op **Opslaan als**, en kiest u een naam. Een werkmap met macro's wordt opgeslagen als **.xlsm**-bestand, een sjabloon als **.xltx**- of **.xltm**-bestand, en elke andere werkmap als **.xlsx**-bestand. Een naam die eindigt op een ander soort werkmap wordt niet geaccepteerd, en het bericht zegt welke extensie u moet gebruiken; de namen die eindigen op **.xls**, **.ods**, **.csv**, **.tsv** en **.txt** maken kopieën, zoals hierboven beschreven.

JeffCrunch slaat alleen op wat u hebt gewijzigd. Al het andere in het bestand, zoals de andere bladen, de grafieken, de afbeeldingen, de macro's en de instellingen die JeffCrunch niet toont, wordt gekopieerd zoals het was, byte voor byte. In een blad dat u hebt gewijzigd, worden alleen de cellen die u hebt gewijzigd opnieuw geschreven, samen met de formules die ervan afhangen, die hun nieuwe waarden krijgen. Als u geen cel hebt gewijzigd, wordt het bestand met rust gelaten.

Als een ander programma het bestand heeft gewijzigd nadat u het opende, slaat JeffCrunch niet over het bestand op zonder te vragen, omdat de wijzigingen van dat andere programma dan verloren zouden gaan. Het vraagt of u **Kopie opslaan**, **Overschrijven** van het bestand, of **Annuleren** wilt kiezen.

Het nieuwe bestand wordt eerst naast het oude geschreven, en neemt pas de plaats van het oude in wanneer het compleet is. Gaat er iets mis, dan ziet u een bericht, en het bestand dat er stond, is ongewijzigd.

Als de formules nog worden berekend wanneer u opslaat, wacht JeffCrunch tot de berekening klaar is, zodat het bestand de resultaten bevat. Als u in de tussentijd op **Esc** drukt om de berekening te stoppen, wordt het opslaan geannuleerd, en de statusbalk meldt dat; sla opnieuw op wanneer u dat wilt. JeffCrunch laat één ding uit het bestand weg: de lijst met de volgorde waarin een programma de formules het laatst heeft berekend (de berekeningsketen). Een programma dat de werkmap daarna opent, maakt die opnieuw. Als JeffCrunch sommige formules niet kon berekenen, of u hebt de berekening gestopt, dan vraagt het bestand dat het programma om alle formules opnieuw te berekenen wanneer het de werkmap opent.

## Sluiten met wijzigingen

Als u een werkmap sluit die wijzigingen heeft die niet zijn opgeslagen, vraagt JeffCrunch of u ze wilt **Opslaan**, **Niet opslaan**, of **Annuleren** en verder wilt werken. Een asterisk (*) in de titel van het venster laat zien dat er niet-opgeslagen wijzigingen zijn.

## Ingebouwde sjablonen

Op **Bestand > Nieuw** staan, onder de lege werkmap, kant-en-klare werkmappen in groepen: **Financiën** (een maandelijks budget, een leningschema en een kasboek), **Zakelijk** (een factuur, een onkostendeclaratie, een urenstaat, een voorraadlijst, een kilometerregistratie en een verkooprapport), **Planning** (een agenda en een projectplan) en **Onderwijs** (een cijferlijst). Elk toont een kleine afbeelding van de bovenkant. Klik op een sjabloon om een nieuwe werkmap ervan te starten. Typ in de gele vakjes. De andere getallen worden voor u berekend. De werkmap heeft pas een eigen bestand wanneer u hem opslaat.
