# Werkmappen openen en opslaan
<!-- keywords: nieuw, leeg, map1, naamloos, openen, opslaan, opslaan als, bestand, werkmap, xlsx, xlsm, xls, ods, csv, tsv, tekstbestand, gescheiden, scheidingsteken, importeren, exporteren, recent, vastgemaakt, vastmaken, slepen, neerzetten, laden, groot, grote bestanden, macro's, niet opgeslagen, wijzigingen, sluiten, kopie, berekeningsketen -->

Als JeffCrunch wordt gestart zonder een bestand om te openen, bevat het venster een nieuwe, lege werkmap met één blad, met de naam Map1. Elke nieuwe werkmap krijgt het volgende nummer (Map2, Map3, enzovoort), en een nummer wordt niet opnieuw gebruikt nadat het venster ervan is gesloten. Als u nog een werkmap wilt beginnen, drukt u op **{key:New}**, of klikt u op **Bestand**, daarna op **Nieuw**, en kiest u **Lege werkmap**; deze wordt in een eigen venster geopend. Een nieuwe werkmap die u niet hebt gewijzigd, maakt plaats als u in het venster ervan een bestand opent; een werkmap die u wel hebt gewijzigd, blijft staan en het bestand wordt in een ander venster geopend. Met **Opslaan** wordt voor een nieuwe werkmap om een naam gevraagd, net als bij **Opslaan als**, en wordt de werkmap opgeslagen als .xlsx-bestand. Als u een nieuwe werkmap sluit die u niet hebt gewijzigd, wordt er niets gevraagd.

Als u een werkmap wilt openen, drukt u op **{key:Open}**, of klikt u op **Bestand** en daarna op **Openen**, en kiest u **Bladeren**. U kunt een werkmap ook vanuit uw bestandsbeheer naar het venster slepen. Een werkmap die al in een venster geopend is, wordt niet nog een keer geopend: dat venster komt dan naar voren.

JeffCrunch opent **.xlsx**-werkmappen en **.xlsm**-werkmappen (werkmappen met macro's). Macro's worden niet uitgevoerd en blijven ongewijzigd behouden.

## Andere soorten bestanden

JeffCrunch opent ook oudere **.xls**-werkmappen, OpenDocument-spreadsheets (**.ods** en de platte variant **.fods**) en tekstbestanden met waarden (**.csv**, **.tsv** en **.txt**). Een werkmap die uit een van deze bestanden is geopend, wordt opgeslagen als een nieuw **.xlsx**-bestand: als u op **{key:Save}** drukt, meldt JeffCrunch dit en wordt om een naam gevraagd. Het bestand dat u hebt geopend, wordt nooit gewijzigd.

Van een OpenDocument-spreadsheet worden de bladen, waarden, formules (met de resultaten die in het bestand zijn bewaard), getalnotaties, lettertypen, opvullingen, randen, samengevoegde cellen, kolombreedten en rijhoogten, verborgen rijen en kolommen, geblokkeerde titels en namen gelezen. Grafieken, afbeeldingen en andere tekeningen, opmerkingen, draaitabellen, macro's, voorwaardelijke opmaak en gegevensvalidatie worden nog niet gelezen; een bericht bovenaan vertelt welke daarvan in het bestand staan, en een kopie die u vanuit JeffCrunch opslaat, bevat ze niet. Een spreadsheet die met een wachtwoord is beveiligd, wordt niet geopend: open het bestand in het programma waarmee het is gemaakt en sla een kopie op zonder wachtwoord.

Een tekstbestand vormt één blad. JeffCrunch zoekt uit hoe de waarden van elkaar zijn gescheiden (door komma's, puntkomma's, tabs of verticale strepen) en hoe de tekens zijn gecodeerd (UTF-8 of UTF-16, of anders de codetabel van uw taal), en leest elke waarde zoals u die in uw taal zou typen: een getal zoals 1.234,5 in het Duits, een percentage, een geldbedrag, een datum met jaartal (zoals 3/15/2024, of 15.03.2024 in het Duits), een tijd, of TRUE of FALSE (in uw taal, zoals WAHR en FALSCH in het Duits, en in het Engels). Een waarde die begint met een isgelijkteken is een formule. Cijfers die met een nul beginnen (zoals 007) en getallen van meer dan 15 cijfers blijven tekst, zodat een postcode of rekeningnummer niet verandert. Hetzelfde geldt voor een dag en een maand zonder jaartal, in elke volgorde en in elke taal (zoals 3/4, 12.5, 1-2, March 1 of Dec 25), omdat het jaartal dan verzonnen zou moeten worden, en voor teksten zoals de gennaam MARCH1. Een blad bevat 1.048.576 rijen en 16.384 kolommen; als het bestand meer bevat, opent JeffCrunch wat past en krijgt u daarvan een melding.

Als u een tekstbestand opent, toont een venster eerst de eerste rijen en wordt gevraagd hoe het bestand moet worden gelezen: wat de velden scheidt (een of meer van komma, puntkomma, tab, spatie, verticale streep of een teken dat u zelf typt, waarbij u opeenvolgende scheidingstekens desgewenst als één kunt laten tellen), waartussen een veld met een scheidingsteken staat, de tekstcodering en de rij waarbij moet worden begonnen. Boven elke kolom kiest u hoe die wordt gelezen: **Zoals getypt**, **Tekst** (om een onderdeelnummer zoals 007 of een lang rekeningnummer ongewijzigd te houden), een **Datum** in de volgorde die u opgeeft, of **Overslaan**. Het voorbeeld past zich aan bij elke wijziging en toont maximaal de eerste 200 rijen en 100 kolommen, maar het hele bestand wordt geopend. Met **Bestand > Openen > Tekstbestand openen** wordt dit ook gevraagd als het bestand een ongebruikelijke naam heeft.

Als u alleen de waarden wilt bewaren van het blad dat u bekijkt, klikt u op **Bestand**, daarna op **Opslaan als**, en kiest u het bestandstype **.csv**. JeffCrunch schrijft de waarden zoals de cellen ze weergeven, in UTF-8, met een puntkomma ertussen als uw taal in getallen een komma gebruikt en anders een komma, en met tekst tussen aanhalingstekens waar dat nodig is. De andere bladen, de formules, de opmaak en de instellingen komen niet in een .csv-bestand terecht, en uw werkmap verandert niet als u er een schrijft.

## Grote werkmappen

JeffCrunch leest een werkmap op de achtergrond. De eerste rijen van het blad dat geopend was toen de werkmap werd opgeslagen, verschijnen vrijwel meteen, ook als de werkmap heel groot is. De statusbalk linksonder laat zien waarmee JeffCrunch bezig is: **Openen**, **Lezen** van een blad met het aantal cellen dat tot nu toe is gelezen, **Berekenen** met een percentage, en daarna **Gereed**.

U kunt schuiven, selecteren en kopiëren terwijl de werkmap wordt gelezen. De tabbladen van de andere bladen worden beschikbaar zodra ze zijn gelezen. Een beschadigde werkmap wordt geopend voor zover die kan worden gelezen, en een bericht vertelt wat is weggelaten.

## Recente werkmappen

Op de pagina **Openen** van het scherm Bestand staan de werkmappen die u onlangs hebt geopend. Klik met de rechtermuisknop op een werkmap om deze bovenaan de lijst **vast te maken** of om deze uit de lijst te verwijderen.

## Opslaan

Druk op **{key:Save}**, of klik op **Bestand** en daarna op **Opslaan**, om uw wijzigingen op te slaan in de werkmap die u hebt geopend. Als u het origineel wilt behouden en een nieuw bestand wilt maken, klikt u op **Bestand** en daarna op **Opslaan als**, en kiest u een naam. Een werkmap met macro's wordt opgeslagen als **.xlsm**-bestand en een werkmap zonder macro's als **.xlsx**-bestand.

JeffCrunch slaat alleen op wat u hebt gewijzigd. Al het andere in het bestand, zoals de andere bladen, de grafieken, de afbeeldingen, de macro's en de instellingen die JeffCrunch niet weergeeft, wordt byte voor byte ongewijzigd gekopieerd. In een blad dat u hebt gewijzigd, worden alleen de cellen die u hebt gewijzigd opnieuw geschreven, samen met de formules die ervan afhangen en die hun nieuwe waarden krijgen. Als u geen enkele cel hebt gewijzigd, blijft het bestand ongemoeid.

Als een ander programma het bestand heeft gewijzigd nadat u het hebt geopend, overschrijft JeffCrunch het niet zonder te vragen, omdat de wijzigingen van dat andere programma anders verloren zouden gaan. U wordt gevraagd of u een **Kopie opslaan**, het bestand **Overschrijven** of **Annuleren** wilt.

Het nieuwe bestand wordt eerst naast het oude geschreven en neemt pas de plaats van het oude in als het volledig is. Als er iets misgaat, krijgt u een bericht en blijft het bestaande bestand zoals het was.

Als de formules nog worden berekend wanneer u opslaat, wacht JeffCrunch totdat dat klaar is, zodat het bestand de resultaten bevat. Als u intussen op **Esc** drukt om de berekening te stoppen, wordt het opslaan geannuleerd en meldt de statusbalk dat; sla opnieuw op wanneer u wilt. Eén ding laat JeffCrunch weg uit het bestand: de lijst met de volgorde waarin een programma de formules het laatst heeft berekend (de berekeningsketen). Het programma dat de werkmap daarna opent, maakt die opnieuw. Als JeffCrunch een formule niet kon berekenen of als u de berekening hebt gestopt, vraagt het bestand dat programma om bij het openen van de werkmap alle formules opnieuw te berekenen.

## Sluiten met wijzigingen

Als u een werkmap sluit met wijzigingen die niet zijn opgeslagen, vraagt JeffCrunch of u ze wilt **Opslaan**, **Niet opslaan**, of dat u wilt **Annuleren** om verder te werken. Een sterretje (*) in de titel van het venster geeft aan dat er niet-opgeslagen wijzigingen zijn.
