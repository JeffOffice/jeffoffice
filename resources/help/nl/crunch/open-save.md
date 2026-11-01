# Werkmappen openen en opslaan
<!-- keywords: kopie, ouder bestandsformaat, opendocument, compatibiliteit, nieuw, leeg, map1, naamloos, openen, opslaan, opslaan als, bestand, werkmap, xlsx, xlsm, xls, ods, csv, tsv, tekstbestand, gescheiden, scheidingsteken, importeren, exporteren, recent, vastmaken, slepen, neerzetten, laden, groot, macro's, niet opgeslagen, wijzigingen, sluiten, berekeningsketen, sjabloon, xltx, xltm, kan niet openen, beschadigd, versleuteld, wachtwoord -->

Als JeffCrunch start zonder bestand om te openen, bevat het venster een nieuwe, lege werkmap met één blad, met de naam Map1. Elke nieuwe werkmap krijgt het volgende nummer (Map2, Map3, enzovoort), en een nummer wordt niet opnieuw gebruikt nadat het venster ervan is gesloten. Druk op **{key:New}** om er nog een te beginnen, of klik op **Bestand**, dan op **Nieuw** en kies **Lege werkmap**; deze wordt in een eigen venster geopend. Een nieuwe werkmap die u niet hebt gewijzigd, maakt plaats als u een bestand in het venster ervan opent; een gewijzigde werkmap blijft staan en het bestand wordt in een ander venster geopend. **Opslaan** vraagt bij een nieuwe werkmap om een naam, net als **Opslaan als**, en slaat de werkmap op als .xlsx-bestand. Bij het sluiten van een nieuwe werkmap die u niet hebt gewijzigd, wordt niets gevraagd.

Druk op **{key:Open}** om een werkmap te openen, of klik op **Bestand** en dan op **Openen** en kies **Bladeren**. U kunt een werkmap ook vanuit uw bestandsbeheer naar het venster slepen. Een werkmap die al in een venster geopend is, wordt niet een tweede keer geopend: dat venster komt naar voren.

JeffCrunch opent **.xlsx**-werkmappen en **.xlsm**-werkmappen (werkmappen met macro's), en de sjablonen **.xltx** en **.xltm** (een sjabloon met macro's). Macro's worden niet uitgevoerd en blijven onaangeroerd: het macroproject, de handtekening ervan en de onderdelen die ernaar verwijzen, worden precies zo teruggeschreven als ze waren. Een sjabloon wordt als sjabloon opgeslagen en een werkmap met macro's als werkmap met macro's, omdat het type deel uitmaakt van het bestand.

## Andere soorten bestanden

JeffCrunch opent ook oudere **.xls**-werkmappen, OpenDocument-spreadsheets (**.ods** en het platte **.fods**) en tekstbestanden met waarden (**.csv**, **.tsv** en **.txt**). Een werkmap die uit zo'n bestand is geopend, wordt opgeslagen als nieuw **.xlsx**-bestand (of als **.xls**- of **.ods**-kopie: zie hieronder): als u op **{key:Save}** drukt, meldt JeffCrunch dit en vraagt om een naam. Het bestand dat u hebt geopend, wordt nooit gewijzigd.

Van een OpenDocument-spreadsheet worden de bladen, waarden, formules (met de resultaten die in het bestand stonden), getalnotaties, lettertypen, opvullingen, randen, samengevoegde cellen, kolombreedten en rijhoogten, verborgen rijen en kolommen, geblokkeerde titels en namen gelezen. Grafieken, afbeeldingen en andere tekeningen, opmerkingen, draaitabellen, macro's, voorwaardelijke opmaak en gegevensvalidatie worden nog niet gelezen; een bericht bovenaan vermeldt welke daarvan in het bestand staan, en een kopie die u vanuit JeffCrunch opslaat, bevat ze niet. Een spreadsheet die met een wachtwoord is beveiligd, wordt niet geopend: open het bestand in het programma waarmee het is gemaakt en sla een kopie op zonder wachtwoord.

Een tekstbestand is één blad. JeffCrunch stelt vast hoe de waarden zijn gescheiden (door komma's, puntkomma's, tabs of verticale strepen) en hoe de tekens zijn gecodeerd (UTF-8 of UTF-16, of anders de codetabel van uw taal), en leest elke waarde zoals u die in uw taal zou typen: een getal zoals 1.234,5 in het Nederlands, een percentage, een geldbedrag, een datum met jaartal (zoals 15-3-2024, of 3/15/2024 in het Engels), een tijd, WAAR of ONWAAR (in uw taal, en in het Engels TRUE en FALSE). Een waarde die met een gelijkteken begint, is een formule. Cijfers die met een nul beginnen (zoals 007) en getallen van meer dan 15 cijfers blijven tekst, zodat een postcode of rekeningnummer niet wordt gewijzigd. Dat geldt ook voor een dag en een maand zonder jaartal, in elke volgorde en in elke taal (zoals 3/4, 12.5, 1-2, 1 maart of 25 dec), omdat het jaartal dan verzonnen zou moeten worden, en voor teksten zoals de gennaam MARCH1. Een blad bevat 1.048.576 rijen en 16.384 kolommen; als het bestand er meer heeft, opent JeffCrunch wat past en meldt dit.

Als u een tekstbestand opent, toont een venster eerst de eerste rijen en vraagt hoe het bestand moet worden gelezen: wat de velden scheidt (een of meer van komma, puntkomma, tab, spatie, verticale streep of een teken dat u typt, waarbij u opeenvolgende scheidingstekens desgewenst als één kunt laten behandelen), welk teken een veld met een scheidingsteken omsluit, de tekstcodering (Unicode, de Windows-codetabellen, de ISO 8859-tabellen, Mac Roman en de Oost-Aziatische) en de rij waarbij moet worden begonnen. Boven elke kolom kiest u hoe die wordt gelezen: **Zoals getypt**, **Tekst** (om een onderdeelnummer zoals 007 of een lang rekeningnummer ongewijzigd te houden), een **Datum** in de volgorde die u opgeeft, of **Overslaan**. Het voorbeeld volgt elke wijziging en toont hooguit de eerste 200 rijen en 100 kolommen, maar het hele bestand wordt geopend. **Bestand > Openen > Een tekstbestand openen** vraagt het ook als de bestandsnaam ongebruikelijk is.

Als u alleen de waarden van het blad wilt bewaren dat u bekijkt, klikt u op **Bestand**, dan op **Opslaan als** en kiest u het bestandstype **.csv** (of **.tsv** en **.txt**, gescheiden door tabs). Voordat het bestand wordt geschreven, meldt JeffCrunch wat uw werkmap bevat dat niet in het bestand past (bijvoorbeeld de andere bladen, formules, afbeeldingen, grafieken, notities en macro's), vraagt hoe het moet worden geschreven en of u **Alleen waarden opslaan** wilt kiezen, in plaats daarvan **Opslaan als .xlsx** om alles te behouden, of **Annuleren**:

- **Scheidingsteken.** Een komma, puntkomma, tab, spatie, verticale streep of een teken dat u typt. Een **.csv** begint met een puntkomma als uw taal in getallen een komma gebruikt, en anders met een komma; een **.tsv** of **.txt** met een tab.
- **Tekstcodering.** UTF-8 met BOM (zodat andere programma's letters met accenten goed lezen), UTF-8 zonder BOM, UTF-16, of een codetabel zoals West-Europees (1252), de ISO 8859-tabellen of Mac Roman. Een teken waarvoor de codetabel geen byte heeft, wordt als vraagteken geschreven, en de statusbalk vermeldt hoeveel.
- **Regeleinden.** Windows (regelterugloop en regelinvoer), Linux en Mac (regelinvoer) of Oude Mac (regelterugloop).
- **Cellen.** **Zoals weergegeven in de cellen** schrijft wat u ziet. **Zoals opgeslagen** schrijft elk cijfer van een getal (0.1 en niet 10%) en een datum als ISO-tekst zoals 2026-03-04.

Tekst komt tussen aanhalingstekens als die het scheidingsteken, een aanhalingsteken, een regeleinde of een spatie aan het begin of einde bevat. De andere bladen, de formules, de opmaak en de instellingen staan niet in het bestand, en uw werkmap wordt niet gewijzigd door het schrijven ervan.

Een werkmap die uit een tekstbestand komt, wordt teruggeschreven in de vorm die het bestand had: hetzelfde scheidingsteken, dezelfde regeleinden en dezelfde codering. Als u de werkmap als kopie opslaat zonder een cel te wijzigen, is de kopie byte voor byte hetzelfde bestand en wordt niets gevraagd.

## Een kopie opslaan als .xls of .ods

Als u een kopie wilt maken die oudere programma's of programma's die OpenDocument lezen kunnen openen, klikt u op **Bestand**, dan op **Opslaan als** en kiest u het bestandstype **.xls** of **.ods**. De kopie wordt gemaakt van uw werkmap zoals die nu is, met de wijzigingen die u nog niet hebt opgeslagen, en bevat:

- de waarden, met hun teksten, getallen, logische waarden en fouten;
- de formules, met hun huidige resultaten (een matrixformule blijft een matrixformule);
- de getalnotaties, lettertypen, opvullingen, randen, uitlijning en of een cel vergrendeld is;
- samengevoegde cellen, kolombreedten en rijhoogten, verborgen rijen en kolommen, overzichten van rijen en kolommen, en geblokkeerde titels;
- de bladen, in hun volgorde, met hun namen en welke verborgen zijn, en de namen die u hebt gedefinieerd (en de afdrukbereiken);
- de koppelingen, en hoe elk blad wordt afgedrukt (het papier, de marges, de afdrukstand, de schaal, kop- en voetteksten en pagina-einden).

Een **.xls**-bestand bevat ook gegevenstabellen en behoudt de berekeningsinstellingen van de werkmap. Een kopie kan niet alles bevatten. Voordat JeffCrunch het bestand schrijft, meldt het wat uw werkmap bevat dat niet in het bestand past: grafieken, afbeeldingen en andere tekeningen, notities, draaitabellen, tabellen, voorwaardelijke opmaak, gegevensvalidatie, filters, bladbeveiliging, formulierbesturingselementen, macro's en koppelingen naar andere werkmappen (de formules die ze gebruiken, behouden de waarden die ze hadden). Een formule die het oudere formaat niet kan weergeven (een functie die het niet kent, of een cel na kolom IV of rij 65.536), wordt geschreven als de waarde die eruit kwam, en het bericht telt deze formules. Het oudere formaat houdt ook op bij 65.536 rijen, 256 kolommen, 255 bladen en 32.767 tekens in een cel, en bij ongeveer 4.000 celopmaken en 56 kleuren: wat niet past, wordt afgekapt, en het bericht vermeldt hoeveel. U kunt dan kiezen voor **Kopie opslaan**, **Opslaan als .xlsx** om alles te behouden, of **Annuleren**.

Uw werkmap wordt niet gewijzigd door de kopie en wordt niet als opgeslagen gemarkeerd. Het bestand dat u hebt geopend, wordt nooit overschreven: geef de kopie een andere naam. Van een geopend **.xls**- of **.ods**-bestand kunt u een kopie opslaan in het andere formaat.

## Grote werkmappen

JeffCrunch leest een werkmap op de achtergrond. De eerste rijen van het blad dat geopend was toen de werkmap werd opgeslagen, verschijnen vrijwel direct, ook als de werkmap heel groot is. De statusbalk linksonder vermeldt waarmee JeffCrunch bezig is: **Openen**, **Lezen** van een blad met het aantal tot nu toe gelezen cellen, **Berekenen** met een percentage, en daarna **Gereed**.

U kunt schuiven, selecteren en kopiëren terwijl er wordt gelezen. De tabbladen van de andere bladen worden actief zodra ze zijn gelezen. Een beschadigde werkmap wordt geopend voor zover die kan worden gelezen, en een bericht vermeldt wat is weggelaten.

## Als een bestand niet wordt geopend

Als JeffCrunch een bestand niet kan openen, wordt in gewone woorden uitgelegd waarom, en blijven de werkmappen die u open hebt ongewijzigd. JeffCrunch onderscheidt een leeg bestand van een bestand dat helemaal geen werkmap is (bijvoorbeeld een PDF-document, een afbeelding, een tekstdocument of een presentatie); een zip-pakket dat onvolledig of beschadigd is, wat meestal betekent dat een download of kopie is onderbroken, zodat u het opnieuw moet kopiëren vanaf het origineel; een werkmap die met een wachtwoord is versleuteld, die JeffCrunch nog niet kan openen; en een samengesteld bestand dat geen werkmap bevat.

## Recente werkmappen

De pagina **Openen** van het scherm Bestand toont de werkmappen die u onlangs hebt geopend. Klik met de rechtermuisknop op een werkmap om deze **vast te maken** bovenaan de lijst of om deze uit de lijst te verwijderen.

## Opslaan

Druk op **{key:Save}**, of klik op **Bestand** en dan op **Opslaan**, om uw wijzigingen in de geopende werkmap op te slaan. Als u het origineel wilt bewaren en een nieuw bestand wilt maken, klikt u op **Bestand** en dan op **Opslaan als** en kiest u een naam. Een werkmap met macro's wordt opgeslagen als **.xlsm**-bestand, een sjabloon als **.xltx**- of **.xltm**-bestand en elke andere werkmap als **.xlsx**-bestand. Een naam die eindigt op een ander werkmaptype wordt niet geaccepteerd, en het bericht vermeldt welke extensie u moet gebruiken; namen die eindigen op **.xls**, **.ods**, **.csv**, **.tsv** en **.txt** maken kopieën, zoals hierboven beschreven.

JeffCrunch slaat alleen op wat u hebt gewijzigd. Al het andere in het bestand, zoals de andere bladen, de grafieken, de afbeeldingen, de macro's en de instellingen die JeffCrunch niet weergeeft, wordt byte voor byte overgenomen zoals het was. In een blad dat u hebt gewijzigd, worden alleen de gewijzigde cellen opnieuw geschreven, samen met de formules die ervan afhangen en die hun nieuwe waarden krijgen. Als u geen cel hebt gewijzigd, blijft het bestand ongemoeid.

Als een ander programma het bestand heeft gewijzigd nadat u het hebt geopend, overschrijft JeffCrunch het niet zonder te vragen, omdat de wijzigingen van het andere programma dan verloren zouden gaan. U kunt kiezen voor **Kopie opslaan…**, het bestand **Overschrijven** of **Annuleren**.

Het nieuwe bestand wordt eerst naast het oude geschreven en neemt pas de plaats van het oude in als het volledig is. Als er iets misgaat, ziet u een bericht en blijft het bestaande bestand zoals het was.

Als de formules nog worden berekend wanneer u opslaat, wacht JeffCrunch tot dat klaar is, zodat het bestand de resultaten bevat. Als u intussen op **Esc** drukt om de berekening te stoppen, wordt het opslaan geannuleerd en vermeldt de statusbalk dit; sla opnieuw op wanneer u wilt. JeffCrunch laat één ding weg uit het bestand: de lijst met de volgorde waarin een programma de formules het laatst heeft berekend (de berekeningsketen). Het programma dat de werkmap daarna opent, maakt die opnieuw. Als JeffCrunch een formule niet kon berekenen of als u de berekening hebt gestopt, vraagt het bestand dat programma om bij het openen van de werkmap alle formules opnieuw te berekenen.

## Sluiten met wijzigingen

Als u een werkmap sluit met wijzigingen die niet zijn opgeslagen, vraagt JeffCrunch of u ze wilt **Opslaan**, **Niet opslaan** wilt kiezen, of met **Annuleren** wilt doorwerken. Een sterretje (*) in de titel van het venster geeft aan dat er niet-opgeslagen wijzigingen zijn.
