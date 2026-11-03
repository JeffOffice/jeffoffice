# Werkmappen openen en opslaan
<!-- keywords: kopie, kopiëren, ouder formaat, oudere indeling, opendocument, compatibiliteit, nieuw, leeg, lege werkmap, map1, naamloos, openen, opslaan, opslaan als, bestand, werkmap, xlsx, xlsm, xls, ods, csv, tsv, tekstbestand, gescheiden, scheidingsteken, importeren, exporteren, recent, vastmaken, vastgemaakt, slepen, neerzetten, laden, groot, grote bestanden, macro's, niet opgeslagen, wijzigingen, sluiten, berekeningsketen, sjabloon, xltx, xltm, kan niet openen, beschadigd, versleuteld, wachtwoord -->

Als JeffCrunch start zonder een bestand om te openen, staat er in het venster een nieuwe, lege werkmap met één blad, met de naam Map1. Elke nieuwe werkmap krijgt het volgende nummer (Map2, Map3 enzovoort), en een nummer wordt niet opnieuw gebruikt nadat het venster ervan is gesloten. Om nog een werkmap te beginnen, drukt u op **{key:New}**, of klikt u op **Bestand**, dan op **Nieuw**, en kiest u **Lege werkmap**; deze wordt in een eigen venster geopend. Een nieuwe werkmap die u niet hebt gewijzigd, maakt plaats als u een bestand in het venster ervan opent; een werkmap die u wel hebt gewijzigd, blijft staan en het bestand wordt in een ander venster geopend. **Opslaan** vraagt bij een nieuwe werkmap om een naam, net als **Opslaan als**, en slaat de werkmap op als .xlsx-bestand. Als u een nieuwe werkmap sluit die u niet hebt gewijzigd, wordt er niets gevraagd.

Om een werkmap te openen, drukt u op **{key:Open}**, of klikt u op **Bestand** en dan op **Openen**, en kiest u **Bladeren**. U kunt een werkmap ook vanuit uw bestandsbeheer naar het venster slepen. Een werkmap die al in een venster geopend is, wordt niet nog een keer geopend: dat venster komt naar voren.

JeffCrunch opent **.xlsx**-werkmappen en **.xlsm**-werkmappen (werkmappen met macro's), en de sjablonen **.xltx** en **.xltm** (een sjabloon met macro's). Macro's worden niet uitgevoerd en blijven onaangeroerd: het macroproject, de handtekening ervan en de onderdelen die ernaar verwijzen, worden precies zo teruggeschreven als ze waren. Een sjabloon wordt opgeslagen als sjabloon en een werkmap met macro's als werkmap met macro's, omdat het soort bestand bij het bestand hoort.

## Andere soorten bestanden

JeffCrunch opent ook oudere **.xls**-werkmappen, OpenDocument-spreadsheets (**.ods** en het platte **.fods**) en tekstbestanden met waarden (**.csv**, **.tsv** en **.txt**). Een werkmap die uit een van deze bestanden is geopend, wordt opgeslagen als een nieuw **.xlsx**-bestand (of als kopie in **.xls** of **.ods**: zie hieronder): als u op **{key:Save}** drukt, meldt JeffCrunch dit en vraagt om een naam. Het bestand dat u hebt geopend, wordt nooit gewijzigd.

Van een OpenDocument-spreadsheet worden de bladen, waarden, formules (met de resultaten die in het bestand stonden), getalnotaties, lettertypen, opvullingen, randen, samengevoegde cellen, kolombreedten en rijhoogten, verborgen rijen en kolommen, geblokkeerde titels en namen gelezen. Grafieken, afbeeldingen en andere tekeningen, opmerkingen, draaitabellen, macro's, voorwaardelijke opmaak en gegevensvalidatie worden nog niet gelezen; een melding bovenaan vertelt welke daarvan in het bestand staan, en een kopie die u vanuit JeffCrunch opslaat, bevat ze niet. Bij een spreadsheet die met een wachtwoord is beveiligd, wordt bij het openen om het wachtwoord gevraagd: zie [Werkmappen met een wachtwoord openen](open-password).

Een tekstbestand wordt één blad. JeffCrunch bepaalt hoe de waarden zijn gescheiden (door komma's, puntkomma's, tabs of verticale strepen) en hoe de tekens zijn gecodeerd (UTF-8 of UTF-16, of anders de codetabel van uw taal), en leest elke waarde zoals u die in uw taal zou typen: een getal zoals 1.234,5, een percentage, een geldbedrag, een datum met jaartal (zoals 15-3-2024), een tijd, WAAR of ONWAAR (in uw taal, en in het Engels als TRUE en FALSE). Een waarde die met een gelijkteken begint, is een formule. Cijfers die met een nul beginnen (zoals 007) en getallen van meer dan 15 cijfers blijven tekst, zodat een postcode of rekeningnummer niet wordt gewijzigd. Dat geldt ook voor een dag en maand zonder jaartal, in elke volgorde en in elke taal (zoals 3/4, 12.5, 1-2, 1 maart of 25 dec), omdat het jaar dan verzonnen zou moeten worden, en voor teksten zoals de gennaam MARCH1. Een blad bevat 1.048.576 rijen en 16.384 kolommen; als het bestand er meer heeft, opent JeffCrunch wat past en meldt dit.

Als u een tekstbestand opent, toont een venster eerst de eerste rijen en vraagt hoe het bestand moet worden gelezen: wat de velden scheidt (komma, puntkomma, tab, spatie, verticale streep of een teken dat u typt, eventueel met opeenvolgende scheidingstekens als één behandeld), welk teken een veld omsluit dat een scheidingsteken bevat, de tekstcodering (Unicode, de Windows-codetabellen, de ISO 8859-tabellen, Mac Roman en de Oost-Aziatische) en de rij waarbij moet worden begonnen. Boven elke kolom kiest u hoe deze wordt gelezen: **Zoals getypt**, **Tekst** (om een onderdeelnummer zoals 007 of een lang rekeningnummer ongewijzigd te houden), een **Datum** in de volgorde die u opgeeft, of **Overslaan**. Het voorbeeld volgt elke wijziging en toont maximaal de eerste 200 rijen en 100 kolommen, maar het hele bestand wordt geopend. **Bestand > Openen > Een tekstbestand openen** vraagt dit ook als de bestandsnaam ongebruikelijk is.

Om alleen de waarden van het blad dat u bekijkt te bewaren, klikt u op **Bestand**, dan op **Opslaan als**, en kiest u het bestandstype **.csv** (of **.tsv** en **.txt**, gescheiden door tabs). Voordat JeffCrunch het bestand schrijft, wordt gemeld wat uw werkmap bevat dat niet in het bestand past (bijvoorbeeld de andere bladen, formules, afbeeldingen, grafieken, notities en macro's) en wordt gevraagd hoe het bestand moet worden geschreven, en of u **Alleen waarden opslaan** kiest, in plaats daarvan **Opslaan als .xlsx…** om alles te behouden, of **Annuleren**:

- **Scheidingsteken.** Een komma, puntkomma, tab, spatie, verticale streep of een teken dat u typt. Een **.csv**-bestand gebruikt standaard een puntkomma als uw taal een komma in getallen gebruikt, en anders een komma; een **.tsv**- of **.txt**-bestand gebruikt een tab.
- **Tekstcodering.** UTF-8 met BOM (zodat andere programma's letters met accenten goed lezen), UTF-8 zonder BOM, UTF-16 of een codetabel zoals West-Europees (1252), de ISO 8859-tabellen of Mac Roman. Een teken waarvoor de codetabel geen byte heeft, wordt als vraagteken geschreven, en de statusbalk meldt hoeveel.
- **Regeleinden.** Windows (regelterugloop en regelinvoer), Linux en Mac (regelinvoer) of Oude Mac (regelterugloop).
- **Cellen.** **Zoals weergegeven in de cellen** schrijft wat u ziet. **Zoals opgeslagen** schrijft alle cijfers van een getal (0.1 en niet 10%) en een datum als ISO-tekst, zoals 2026-03-04.

Tekst wordt tussen aanhalingstekens gezet als deze het scheidingsteken, een aanhalingsteken, een regeleinde of een spatie aan het begin of eind bevat. De andere bladen, de formules, de opmaak en de instellingen staan niet in het bestand, en uw werkmap verandert niet als u zo'n bestand schrijft.

Een werkmap die uit een tekstbestand afkomstig is, wordt teruggeschreven in de vorm die het bestand had: hetzelfde scheidingsteken, dezelfde regeleinden en dezelfde codering. Als u de werkmap als kopie opslaat zonder een cel te wijzigen, is de kopie byte voor byte hetzelfde bestand, en wordt er niets gevraagd.

## Een kopie opslaan als .xls of .ods

Om een kopie te maken die oudere programma's of programma's die OpenDocument lezen kunnen openen, klikt u op **Bestand**, dan op **Opslaan als**, en kiest u het bestandstype **.xls** of **.ods**. De kopie wordt gemaakt van uw werkmap zoals deze nu is, inclusief wijzigingen die u nog niet hebt opgeslagen, en bevat:

- de waarden, met hun teksten, getallen, logische waarden en fouten;
- de formules, met hun huidige resultaten (een matrixformule blijft een matrixformule);
- de getalnotaties, lettertypen, opvullingen, randen, uitlijning en of een cel is vergrendeld;
- samengevoegde cellen, kolombreedten en rijhoogten, verborgen rijen en kolommen, overzichten van rijen en kolommen, en geblokkeerde titels;
- de bladen, in hun volgorde, met hun namen en welke verborgen zijn, en de namen die u hebt gedefinieerd (en de afdrukbereiken);
- de koppelingen, en hoe elk blad wordt afgedrukt (papier, marges, afdrukstand, schaal, kop- en voetteksten en pagina-einden).

Een **.xls**-bestand bevat ook gegevenstabellen en behoudt de berekeningsinstellingen van de werkmap. Een kopie kan niet alles bevatten. Voordat JeffCrunch het bestand schrijft, wordt gemeld wat uw werkmap bevat dat niet in het bestand past: grafieken, afbeeldingen en andere tekeningen, notities, draaitabellen, tabellen, voorwaardelijke opmaak, gegevensvalidatie, filters, bladbeveiliging, formulierbesturingselementen, macro's en koppelingen naar andere werkmappen (formules die deze gebruiken, behouden de waarden die ze hadden). Een formule die de oudere indeling niet kan weergeven (een functie die daarin niet bestaat, of een cel voorbij kolom IV of rij 65.536), wordt geschreven als de waarde die eruit voortkwam, en de melding telt hoeveel. De oudere indeling heeft bovendien een maximum van 65.536 rijen, 256 kolommen, 255 bladen en 32.767 tekens per cel, en van ongeveer 4.000 celopmaken en 56 kleuren: wat niet past, wordt afgekapt, en de melding zegt hoeveel. U kunt dan kiezen voor **Kopie opslaan**, **Opslaan als .xlsx…** om alles te behouden, of **Annuleren**.

Uw werkmap verandert niet door de kopie en wordt niet als opgeslagen gemarkeerd. Het bestand dat u hebt geopend, wordt nooit overschreven: geef de kopie een andere naam. Van een **.xls**- of **.ods**-bestand dat u hebt geopend, kunt u een kopie in het andere bestandstype opslaan.

## Grote werkmappen

JeffCrunch leest een werkmap op de achtergrond. De eerste rijen van het blad dat geopend was toen de werkmap werd opgeslagen, verschijnen vrijwel direct, ook bij een zeer grote werkmap. De statusbalk linksonder toont waarmee JeffCrunch bezig is: **Openen**, **Lezen** van een blad met het aantal tot nu toe gelezen cellen, **Berekenen** met een percentage, en daarna **Gereed**.

Tijdens het lezen kunt u schuiven, selecteren en kopiëren. De tabbladen van de andere bladen worden actief zodra ze zijn gelezen. Een beschadigde werkmap wordt geopend voor zover deze kan worden gelezen, en een melding geeft aan wat is weggelaten.

## Als een bestand niet wordt geopend

Als JeffCrunch een bestand niet kan openen, wordt in gewone woorden uitgelegd waarom, en blijven de geopende werkmappen zoals ze zijn. JeffCrunch maakt onderscheid tussen een bestand dat leeg is en een bestand dat helemaal geen werkmap is (bijvoorbeeld een PDF-document, een afbeelding, een tekstdocument of een presentatie); een zip-pakket dat is afgekapt of beschadigd, wat meestal betekent dat een download of kopieerbewerking is onderbroken (kopieer het dan opnieuw vanaf het origineel); een werkmap die met een wachtwoord is beveiligd, waarbij om het wachtwoord wordt gevraagd (zie [Werkmappen met een wachtwoord openen](open-password)); en een samengesteld bestand dat geen werkmap bevat.

## Recente werkmappen

De pagina **Openen** van het scherm Bestand toont de werkmappen die u onlangs hebt geopend. Klik met de rechtermuisknop op een werkmap om deze **vast te maken** bovenaan de lijst of om deze uit de lijst te verwijderen.

## Opslaan

Druk op **{key:Save}**, of klik op **Bestand** en dan op **Opslaan**, om uw wijzigingen op te slaan in de werkmap die u hebt geopend. Om het origineel te behouden en een nieuw bestand te maken, klikt u op **Bestand** en dan op **Opslaan als**, en kiest u een naam. Een werkmap met macro's wordt opgeslagen als **.xlsm**-bestand, een sjabloon als **.xltx**- of **.xltm**-bestand, en elke andere werkmap als **.xlsx**-bestand. Een naam die eindigt op de extensie van een ander soort werkmap wordt niet geaccepteerd, en de melding zegt welke extensie u moet gebruiken; namen die eindigen op **.xls**, **.ods**, **.csv**, **.tsv** en **.txt** maken kopieën, zoals hierboven beschreven.

JeffCrunch slaat alleen op wat u hebt gewijzigd. Al het andere in het bestand, zoals de andere bladen, de grafieken, de afbeeldingen, de macro's en de instellingen die JeffCrunch niet weergeeft, wordt byte voor byte gekopieerd zoals het was. In een blad dat u hebt gewijzigd, worden alleen de gewijzigde cellen opnieuw geschreven, samen met de formules die ervan afhangen en die hun nieuwe waarden krijgen. Als u geen enkele cel hebt gewijzigd, blijft het bestand ongemoeid.

Als een ander programma het bestand heeft gewijzigd nadat u het opende, slaat JeffCrunch het niet zonder te vragen op, omdat de wijzigingen van dat andere programma dan verloren zouden gaan. U kunt kiezen voor **Kopie opslaan…**, **Overschrijven** of **Annuleren**.

Het nieuwe bestand wordt eerst naast het oude geschreven en neemt pas de plaats van het oude in als het volledig is. Als er iets misgaat, ziet u een melding en blijft het bestaande bestand zoals het was.

Als de formules nog worden berekend wanneer u opslaat, wacht JeffCrunch tot dat klaar is, zodat het bestand de resultaten bevat. Als u intussen op **Esc** drukt om de berekening te stoppen, wordt het opslaan geannuleerd en meldt de statusbalk dit; sla opnieuw op wanneer u wilt. JeffCrunch laat één ding weg uit het bestand: de lijst met de volgorde waarin een programma de formules het laatst heeft berekend (de berekeningsketen). Het programma dat de werkmap daarna opent, maakt deze opnieuw. Als JeffCrunch een formule niet kon berekenen of als u de berekening hebt gestopt, vraagt het bestand dat programma om bij het openen van de werkmap alle formules opnieuw te berekenen.

## Sluiten met wijzigingen

Als u een werkmap sluit met wijzigingen die niet zijn opgeslagen, vraagt JeffCrunch of u deze wilt opslaan (**Opslaan**), niet wilt opslaan (**Niet opslaan**), of wilt **Annuleren** en verder wilt werken. Een sterretje (*) in de titel van het venster geeft aan dat er niet-opgeslagen wijzigingen zijn.
