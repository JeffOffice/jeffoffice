# Werkmappen openen en opslaan
<!-- keywords: openen, opslaan, opslaan als, bestand, werkmap, xlsx, xlsm, xls, ods, csv, tsv, tekstbestand, gescheiden, scheidingsteken, importeren, exporteren, recent, vastgemaakt, vastmaken, slepen, neerzetten, laden, groot, grote bestanden, macro's, niet opgeslagen, wijzigingen, sluiten, kopie, berekeningsketen -->

Als u een werkmap wilt openen, drukt u op **{key:Open}**, of klikt u op **Bestand** en vervolgens op **Openen**, en kiest u **Bladeren**. U kunt een werkmap ook vanuit uw bestandsbeheer naar het venster slepen. Een werkmap die al in een venster geopend is, wordt niet nog een keer geopend: dat venster komt dan naar voren.

JeffCrunch opent **.xlsx**-werkmappen en **.xlsm**-werkmappen (werkmappen met macro's). JeffCrunch voert geen macro's uit en laat ze ongewijzigd.

## Andere soorten bestanden

JeffCrunch opent ook oudere **.xls**-werkmappen, OpenDocument-spreadsheets (**.ods** en het platte **.fods**) en tekstbestanden met waarden (**.csv**, **.tsv** en **.txt**). Een werkmap die uit een van deze bestanden is geopend, wordt opgeslagen als een nieuw **.xlsx**-bestand: wanneer u op **{key:Save}** drukt, meldt JeffCrunch dat en vraagt het om een naam. Het bestand dat u hebt geopend, wordt nooit gewijzigd.

Van een OpenDocument-spreadsheet worden de bladen, waarden, formules (met de resultaten die in het bestand zijn bewaard), getalnotaties, lettertypen, opvullingen, randen, samengevoegde cellen, kolombreedten en rijhoogten, verborgen rijen en kolommen, geblokkeerde titels en namen gelezen. Grafieken, afbeeldingen en andere tekeningen, opmerkingen, draaitabellen, macro's, voorwaardelijke opmaak en gegevensvalidatie worden nog niet gelezen; een melding bovenaan vertelt welke daarvan in het bestand staan, en een kopie die u vanuit JeffCrunch opslaat, bevat ze niet. Een spreadsheet die met een wachtwoord is beveiligd, wordt niet geopend: open deze in het programma waarmee hij is gemaakt en sla een kopie zonder wachtwoord op.

Een tekstbestand vormt één blad. JeffCrunch zoekt uit hoe de waarden zijn gescheiden (door komma's, puntkomma's, tabs of verticale strepen) en hoe de tekens zijn gecodeerd (UTF-8 of UTF-16, of anders de codetabel van uw taal), en leest elke waarde zoals u die in uw taal zou typen: een getal zoals 1.234,5 in het Duits, een percentage, een geldbedrag, een datum met jaartal (zoals 3/15/2024, of 15.03.2024 in het Duits), een tijd, TRUE of FALSE (in uw taal, zoals WAHR en FALSCH in het Duits, en in het Engels). Een waarde die met een gelijkteken begint, is een formule. Cijfers die met een nul beginnen (zoals 007) en getallen van meer dan 15 cijfers blijven tekst, zodat een postcode of rekeningnummer niet wordt gewijzigd. Hetzelfde geldt voor een dag en maand zonder jaartal, in elke volgorde en in elke taal (zoals 3/4, 12.5, 1-2, March 1 of Dec 25), omdat het jaar dan verzonnen zou moeten worden, en voor teksten zoals de gennaam MARCH1. Een blad bevat 1.048.576 rijen en 16.384 kolommen; als het bestand meer bevat, opent JeffCrunch wat past en meldt het dat.

Wanneer u een tekstbestand opent, toont een venster eerst de eerste rijen en vraagt het hoe het bestand moet worden gelezen: wat de velden scheidt (een komma, puntkomma, tab, spatie, verticale streep of een teken dat u typt, of een combinatie daarvan, waarbij u desgewenst opeenvolgende scheidingstekens als één kunt laten tellen), welk teken een veld met een scheidingsteken erin omsluit, de tekstcodering en de rij waarbij moet worden begonnen. Boven elke kolom kiest u hoe die wordt gelezen: **Zoals getypt**, **Tekst** (om een onderdeelnummer zoals 007 of een lang rekeningnummer ongewijzigd te laten), een **Datum** in de volgorde die u opgeeft, of **Overslaan**. Het voorbeeld past zich aan bij elke wijziging en toont maximaal de eerste 200 rijen en 100 kolommen, maar het hele bestand wordt geopend. Met **Bestand > Openen > Een tekstbestand openen** wordt het altijd gevraagd, ook als de naam van het bestand ongebruikelijk is.

Als u alleen de waarden van het blad dat u bekijkt wilt bewaren, klikt u op **Bestand**, vervolgens op **Opslaan als**, en kiest u het bestandstype **.csv**. JeffCrunch schrijft de waarden zoals de cellen ze weergeven, in UTF-8, met een puntkomma ertussen als uw taal een komma in getallen gebruikt en anders een komma, en met tekst tussen aanhalingstekens waar dat nodig is. De andere bladen, de formules, de opmaak en de instellingen komen niet in een .csv-bestand terecht, en uw werkmap verandert niet door het schrijven ervan.

## Grote werkmappen

JeffCrunch leest een werkmap op de achtergrond. De eerste rijen van het blad dat geopend was toen de werkmap werd opgeslagen, verschijnen vrijwel meteen, ook als de werkmap heel groot is. De statusbalk linksonder vertelt wat JeffCrunch doet: **Openen**, **Lezen** van een blad met het aantal tot nu toe gelezen cellen, **Berekenen** met een percentage en daarna **Gereed**.

Tijdens het lezen kunt u schuiven, selecteren en kopiëren. De tabbladen van de andere bladen worden beschikbaar zodra ze zijn gelezen. Een beschadigde werkmap wordt geopend voor zover die kan worden gelezen, en een melding vertelt wat is weggelaten.

## Recente werkmappen

De pagina **Openen** van het scherm Bestand toont de werkmappen die u onlangs hebt geopend. Klik met de rechtermuisknop op een werkmap om deze bovenaan de lijst **vast te maken** of om deze uit de lijst te verwijderen.

## Opslaan

Druk op **{key:Save}**, of klik op **Bestand** en vervolgens op **Opslaan**, om uw wijzigingen op te slaan in de werkmap die u hebt geopend. Als u het origineel wilt behouden en een nieuw bestand wilt maken, klikt u op **Bestand** en vervolgens op **Opslaan als**, en kiest u een naam. Een werkmap met macro's wordt opgeslagen als **.xlsm**-bestand, een werkmap zonder macro's als **.xlsx**-bestand.

JeffCrunch slaat alleen op wat u hebt gewijzigd. Al het andere in het bestand, zoals de andere bladen, de grafieken, de afbeeldingen, de macro's en de instellingen die JeffCrunch niet weergeeft, wordt byte voor byte ongewijzigd overgenomen. In een blad dat u hebt gewijzigd, worden alleen de gewijzigde cellen opnieuw geschreven, samen met de formules die ervan afhangen en die hun nieuwe waarden krijgen. Als u geen enkele cel hebt gewijzigd, blijft het bestand ongemoeid.

Als een ander programma het bestand heeft gewijzigd nadat u het hebt geopend, overschrijft JeffCrunch het niet zonder te vragen, omdat de wijzigingen van dat andere programma dan verloren zouden gaan. U wordt gevraagd of u **Kopie opslaan**, het bestand **Overschrijven** of **Annuleren** wilt kiezen.

Het nieuwe bestand wordt eerst naast het oude geschreven en neemt pas de plaats van het oude in als het volledig is. Als er iets misgaat, ziet u een melding en blijft het bestaande bestand zoals het was.

Als de formules nog worden berekend wanneer u opslaat, wacht JeffCrunch tot dat klaar is, zodat het bestand de resultaten bevat. Als u intussen op **Esc** drukt om de berekening te stoppen, wordt het opslaan geannuleerd en meldt de statusbalk dat; sla opnieuw op wanneer u wilt. JeffCrunch laat één ding uit het bestand weg: de lijst met de volgorde waarin een programma de formules het laatst heeft berekend (de berekeningsketen). Het volgende programma dat de werkmap opent, maakt die opnieuw. Als JeffCrunch een formule niet kon berekenen, of als u de berekening hebt gestopt, vraagt het bestand dat programma om bij het openen van de werkmap alle formules opnieuw te berekenen.

## Sluiten met wijzigingen

Als u een werkmap sluit met wijzigingen die niet zijn opgeslagen, vraagt JeffCrunch of u ze wilt **Opslaan**, **Niet opslaan**, of dat u wilt **Annuleren** en verder wilt werken. Een sterretje (*) in de titel van het venster geeft aan dat er niet-opgeslagen wijzigingen zijn.
