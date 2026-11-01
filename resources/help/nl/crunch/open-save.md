# Werkmappen openen en opslaan
<!-- keywords: nieuw, leeg, map1, naamloos, openen, opslaan, opslaan als, bestand, werkmap, xlsx, xlsm, xls, ods, csv, tsv, tekstbestand, gescheiden, scheidingsteken, importeren, exporteren, recent, vastgemaakt, vastmaken, slepen, neerzetten, laden, groot, grote bestanden, macro's, niet opgeslagen, wijzigingen, sluiten, kopie, berekeningsketen, sjabloon, xltx, xltm, kan niet openen, beschadigd, versleuteld, wachtwoord -->

Als JeffCrunch start zonder bestand om te openen, bevat het venster een nieuwe, lege werkmap met één blad, met de naam Map1. Elke nieuwe werkmap krijgt het volgende nummer (Map2, Map3, enzovoort), en een nummer wordt niet opnieuw gebruikt nadat het venster is gesloten. Als u nog een werkmap wilt beginnen, drukt u op **{key:New}**, of klikt u op **Bestand**, dan op **Nieuw** en kiest u **Lege werkmap**; deze wordt in een eigen venster geopend. Een nieuwe werkmap die u niet hebt gewijzigd, maakt plaats als u een bestand in het venster ervan opent; een gewijzigde werkmap blijft staan en het bestand wordt in een ander venster geopend. **Opslaan** vraagt bij een nieuwe werkmap om een naam, net als **Opslaan als**, en slaat de werkmap op als .xlsx-bestand. Als u een nieuwe werkmap sluit die u niet hebt gewijzigd, wordt er niets gevraagd.

Als u een werkmap wilt openen, drukt u op **{key:Open}**, of klikt u op **Bestand** en dan op **Openen**, en kiest u **Bladeren**. U kunt een werkmap ook vanuit uw bestandsbeheer naar het venster slepen. Een werkmap die al in een venster geopend is, wordt niet nog een keer geopend: dat venster komt naar voren.

JeffCrunch opent **.xlsx**-werkmappen en **.xlsm**-werkmappen (werkmappen met macro's), en de sjablonen **.xltx** en **.xltm** (een sjabloon met macro's). JeffCrunch voert geen macro's uit en laat ze ongemoeid: het macroproject, de handtekening ervan en de onderdelen die ernaar verwijzen, worden precies teruggeschreven zoals ze waren. Een sjabloon wordt opgeslagen als sjabloon en een werkmap met macro's als werkmap met macro's, omdat het soort bestand deel uitmaakt van het bestand.

## Andere soorten bestanden

JeffCrunch opent ook oudere **.xls**-werkmappen, OpenDocument-spreadsheets (**.ods** en de platte variant **.fods**) en tekstbestanden met waarden (**.csv**, **.tsv** en **.txt**). Een werkmap die uit een van deze bestanden is geopend, wordt opgeslagen als een nieuw **.xlsx**-bestand: als u op **{key:Save}** drukt, meldt JeffCrunch dit en vraagt om een naam, en het bestand dat u hebt geopend, wordt nooit gewijzigd.

Van een OpenDocument-spreadsheet worden de bladen, waarden, formules (met de resultaten die in het bestand stonden), getalnotaties, lettertypen, opvullingen, randen, samengevoegde cellen, kolombreedten en rijhoogten, verborgen rijen en kolommen, geblokkeerde titels en namen gelezen. Grafieken, afbeeldingen en andere tekeningen, opmerkingen, draaitabellen, macro's, voorwaardelijke opmaak en gegevensvalidatie worden nog niet gelezen; een bericht bovenaan vertelt u welke daarvan het bestand bevat, en een kopie die u hier opslaat, bevat ze niet. Een spreadsheet die met een wachtwoord is beveiligd, wordt niet geopend: open deze in het programma waarmee hij is gemaakt en sla een kopie zonder wachtwoord op.

Een tekstbestand is één blad. JeffCrunch zoekt uit hoe de waarden zijn gescheiden (door komma's, puntkomma's, tabs of verticale strepen) en hoe de tekens zijn gecodeerd (UTF-8 of UTF-16, en anders de codetabel van uw taal), en leest elke waarde zoals u die in uw taal zou typen: een getal zoals 1.234,5 in het Nederlands, een percentage, een geldbedrag, een datum met jaar (zoals 15-3-2024, of 3/15/2024 in het Engels), een tijd, WAAR of ONWAAR (in uw taal, en in het Engels als TRUE en FALSE). Een waarde die met een isgelijkteken begint, is een formule. Cijfers die met een nul beginnen (zoals 007) en getallen van meer dan 15 cijfers blijven tekst, zodat een postcode of rekeningnummer niet wordt gewijzigd. Hetzelfde geldt voor een dag en een maand zonder jaar, in elke volgorde en in elke taal (zoals 3/4, 12.5, 1-2, 1 maart of 25 dec), omdat het jaar dan verzonnen zou moeten worden, en voor teksten zoals de gennaam MARCH1. Een blad bevat 1.048.576 rijen en 16.384 kolommen; als het bestand meer bevat, opent JeffCrunch wat past en meldt dit.

Als u een tekstbestand opent, toont een venster eerst de eerste rijen en vraagt hoe het bestand moet worden gelezen: wat de velden scheidt (een komma, puntkomma, tab, spatie, verticale streep of een teken dat u typt, waarbij u desgewenst opeenvolgende scheidingstekens als één kunt laten behandelen), welk teken om een veld met een scheidingsteken staat, de tekstcodering (Unicode, de Windows-codetabellen, de ISO 8859-tabellen, Mac Roman en de Oost-Aziatische codetabellen) en de rij waarmee moet worden begonnen. Boven elke kolom kiest u hoe die wordt gelezen: **Zoals getypt**, **Tekst** (om een onderdeelnummer zoals 007 of een lang rekeningnummer ongewijzigd te houden), een **Datum** in de volgorde die u opgeeft, of **Overslaan**. Het voorbeeld volgt elke wijziging en toont maximaal de eerste 200 rijen en 100 kolommen, maar het hele bestand wordt geopend. **Bestand > Openen > Een tekstbestand openen** vraagt het ook als de naam van het bestand ongebruikelijk is.

Als u alleen de waarden van het blad dat u bekijkt wilt bewaren, klikt u op **Bestand**, dan op **Opslaan als**, en kiest u het bestandstype **.csv** (of **.tsv** en **.txt**, gescheiden door tabs). Voordat JeffCrunch het bestand schrijft, meldt het wat uw werkmap bevat dat het bestand niet kan bevatten (bijvoorbeeld de andere bladen, formules, afbeeldingen, grafieken, notities en macro's), en vraagt het hoe het bestand moet worden geschreven en of u **Alleen waarden opslaan**, in plaats daarvan **Opslaan als .xlsx** om alles te behouden, of **Annuleren** kiest:

- **Scheidingsteken.** Een komma, puntkomma, tab, spatie, verticale streep of een teken dat u typt. Een **.csv** begint met een puntkomma als uw taal een komma in getallen gebruikt, en anders met een komma; een **.tsv** of **.txt** met een tab.
- **Tekstcodering.** UTF-8 met BOM (zodat andere programma's letters met accenten goed lezen), UTF-8 zonder BOM, UTF-16, of een codetabel zoals West-Europees (1252), de ISO 8859-tabellen of Mac Roman. Een teken waarvoor de codetabel geen byte heeft, wordt als vraagteken geschreven, en de statusbalk meldt hoeveel.
- **Regeleinden.** Windows (regelterugloop en regelinvoer), Linux en Mac (regelinvoer) of Oude Mac (regelterugloop).
- **Cellen.** **Zoals weergegeven in de cellen** schrijft wat u ziet. **Zoals opgeslagen** schrijft elk cijfer van een getal (0,1 en niet 10%) en een datum als ISO-tekst, zoals 2026-03-04.

Tekst wordt tussen aanhalingstekens gezet als deze het scheidingsteken, een aanhalingsteken, een regeleinde of een spatie aan het begin of eind bevat. De andere bladen, de formules, de opmaak en de instellingen staan niet in het bestand, en uw werkmap verandert niet doordat u zo'n bestand schrijft.

Een werkmap die uit een tekstbestand afkomstig is, wordt teruggeschreven in de vorm die het bestand had: met hetzelfde scheidingsteken, dezelfde regeleinden en dezelfde codering. Als u het als kopie hebt opgeslagen zonder een cel te wijzigen, is de kopie byte voor byte hetzelfde bestand en wordt er niets gevraagd.

## Grote werkmappen

JeffCrunch leest een werkmap op de achtergrond. De eerste rijen van het blad dat open was toen de werkmap werd opgeslagen, verschijnen vrijwel meteen, ook als de werkmap heel groot is. De statusbalk linksonder laat zien waarmee JeffCrunch bezig is: **Openen**, **Lezen** van een blad met het aantal tot dan toe gelezen cellen, **Berekenen** met een percentage, en dan **Gereed**.

U kunt schuiven, selecteren en kopiëren terwijl het bestand wordt gelezen. De tabbladen van de andere bladen worden actief zodra ze zijn gelezen. Een beschadigde werkmap wordt geopend voor zover deze kan worden gelezen, en een bericht meldt wat is weggelaten.

## Als een bestand niet wordt geopend

Als JeffCrunch een bestand niet kan openen, wordt in gewone woorden uitgelegd waarom, en blijven de werkmappen die u open hebt ongewijzigd. JeffCrunch onderscheidt een leeg bestand van een bestand dat helemaal geen werkmap is (bijvoorbeeld een PDF-document, een afbeelding, een tekstdocument of een presentatie); een zip-pakket dat is afgekapt of beschadigd, wat meestal betekent dat een download of kopie is onderbroken (kopieer het dan opnieuw van het origineel); een werkmap die met een wachtwoord is versleuteld, die JeffCrunch nog niet kan openen; en een samengesteld bestand dat geen werkmap bevat.

## Recente werkmappen

De pagina **Openen** van het scherm Bestand toont de werkmappen die u onlangs hebt geopend. Klik met de rechtermuisknop op een werkmap om deze bovenaan de lijst **vast te maken** of om deze uit de lijst te verwijderen.

## Opslaan

Druk op **{key:Save}**, of klik op **Bestand** en dan op **Opslaan**, om uw wijzigingen op te slaan in de werkmap die u hebt geopend. Als u het origineel wilt behouden en een nieuw bestand wilt maken, klikt u op **Bestand** en dan op **Opslaan als**, en kiest u een naam. Een werkmap met macro's wordt opgeslagen als **.xlsm**-bestand, een sjabloon als **.xltx**- of **.xltm**-bestand en elke andere werkmap als **.xlsx**-bestand. Een naam die op een andere extensie eindigt, wordt niet geaccepteerd, en het bericht meldt welke extensie u moet gebruiken.

JeffCrunch slaat alleen op wat u hebt gewijzigd. Al het andere in het bestand, zoals de andere bladen, de grafieken, de afbeeldingen, de macro's en de instellingen die JeffCrunch niet weergeeft, wordt byte voor byte gekopieerd zoals het was. In een blad dat u hebt gewijzigd, worden alleen de gewijzigde cellen opnieuw geschreven, samen met de formules die ervan afhangen en die hun nieuwe waarden krijgen. Als u geen cel hebt gewijzigd, blijft het bestand ongemoeid.

Als een ander programma het bestand heeft gewijzigd nadat u het hebt geopend, overschrijft JeffCrunch het niet zonder te vragen, omdat dan de wijzigingen van het andere programma verloren zouden gaan. U kunt kiezen uit **Kopie opslaan**, het bestand **Overschrijven** of **Annuleren**.

Het nieuwe bestand wordt eerst naast het oude geschreven en neemt pas de plaats van het oude in als het volledig is. Als er iets misgaat, ziet u een bericht en blijft het bestaande bestand zoals het was.

Als de formules nog worden berekend wanneer u opslaat, wacht JeffCrunch tot dat klaar is, zodat het bestand de resultaten bevat. Als u intussen op **Esc** drukt om de berekening te stoppen, wordt het opslaan geannuleerd en meldt de statusbalk dit; sla opnieuw op wanneer u wilt. JeffCrunch laat één ding weg uit het bestand: de lijst met de volgorde waarin een programma de formules het laatst heeft berekend (de berekeningsketen). Het programma dat de werkmap daarna opent, maakt deze opnieuw. Als JeffCrunch een formule niet kon berekenen of als u de berekening hebt gestopt, vraagt het bestand dat programma om bij het openen van de werkmap alle formules opnieuw te berekenen.

## Sluiten met wijzigingen

Als u een werkmap sluit met wijzigingen die niet zijn opgeslagen, vraagt JeffCrunch of u ze wilt **Opslaan**, **Niet opslaan** of **Annuleren** om verder te werken. Een sterretje (*) in de titel van het venster geeft aan dat er niet-opgeslagen wijzigingen zijn.
