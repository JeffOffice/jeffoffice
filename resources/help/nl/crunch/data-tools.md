# Gegevenshulpmiddelen

<!-- keywords: prognoseblad, prognose, toekomstige waarden, betrouwbaarheidsinterval, seizoensinvloed, trend, voortschrijdend, formulier, gegevensformulier, gegevensinvoer, record, records, nieuw record, volgende zoeken, vorige zoeken, criteria, scenariobeheer, scenario's, wat-als, wat als, scenariosamenvatting, gegevens, tabblad gegevens, tekst naar kolommen, splitsen, scheidingsteken, komma, tab, vaste breedte, duplicaten verwijderen, doelzoeken, subtotaal, totalen, geavanceerd filter, voorwaarden, gegevensvalidatie, ongeldige gegevens cirkelen, rode ovaal, validatiemarkeringen wissen, consolideren, bereiken combineren, toestaan, lijst, keuzelijst, invoerbericht, foutmelding, herhaalde rijen, uniek, tekstbestand, csv, uit tekst, kolommen, scheidingsteken, aanhalingstekens, koppelingen bewerken, waarden bijwerken, bron wijzigen, koppelingen, extern, bron, koppeling verbreken, status controleren, bron openen -->

Op het tabblad **Gegevens** staan de hulpmiddelen die werken op een lijst met waarden: tekst uit een bestand halen, een lijst sorteren, tekst in kolommen splitsen en herhaalde rijen verwijderen.

## Uit Tekst/CSV

Met **Uit tekst/CSV** opent u een tekstbestand (.csv, .tsv of .txt). Dit is hetzelfde als **Tekstbestand openen** op het tabblad Bestand. JeffCrunch vraagt hoe het bestand in kolommen is verdeeld en wat voor soort tekst het bevat, en toont het daarna als een nieuwe werkmap.

## Koppelingen bewerken

Als een werkmap formules heeft die een andere werkmap lezen (zoals ='[Prijzen.xlsx]Blad1'!A1), somt **Koppelingen bewerken** in de groep **Gegevens ophalen** die andere werkmappen op. Die heten bronnen. Elke rij toont waar de bron staat en wat de status is. Totdat de werkmap opnieuw wordt geopend met zijn bronnen, toont hij de getallen die het bestand van die bronnen heeft opgeslagen.

- **Status controleren** zoekt voor elke bron op deze computer en meldt **Gevonden**, of dat de bron niet is gevonden. Een bron op het web wordt niet opgehaald; de status zegt dan dat die niet is gecontroleerd.
- **Waarden bijwerken** leest de gekozen bron van deze computer en geeft elke cel die alleen een verwijzing is naar een van zijn cellen de waarde die die cel nu heeft. Een cel die meer doet met de waarde, zoals `=[1]Prijzen!A1*2`, behoudt het getal dat het bestand heeft opgeslagen. Het bericht onder de lijst zegt hoeveel cellen zijn bijgewerkt en hoeveel niet. Waarden bijwerken is één stap, dus **Ongedaan maken** zet de oude waarden terug. De nieuwe waarden worden ook in het eigen deel van de koppeling in het bestand geschreven wanneer u opslaat, zodat de werkmap ze de volgende keer dat u hem opent toont, ook als de bron er niet is.
- **Bron wijzigen** laat u een ander bestand kiezen voor de gekozen koppeling, bijvoorbeeld wanneer de werkmap is verplaatst. De formules blijven zoals ze zijn. De nieuwe locatie wordt bij het opslaan in het bestand geschreven, en **Ongedaan maken** neemt de wijziging terug.
- **Bron openen** opent de gekozen bron in een nieuw venster, als die kan worden gevonden.
- **Koppeling verbreken** zet elke cel die de gekozen bron leest om in de waarde die die nu toont, en een naam die in de koppeling voor een plaats stond, wordt **#REF!**. JeffCrunch vraagt eerst om bevestiging. Koppeling verbreken is één stap, dus **Ongedaan maken** brengt de formules, de namen en de koppelingen terug, tot u de werkmap opslaat. Wanneer er geen koppeling meer over is, wordt het bestand opgeslagen zonder de onderdelen die ze bevatten.

## Formulier

Met **Formulier** (in de groep **Gegevenshulpmiddelen** en in het zoekvak) ziet u per keer één rij van een lijst, als een klein formulier. Een lijst is een tabel, of een celbereik met een rij namen bovenaan en onder elke naam een rij met gegevens. Klik eerst op een willekeurige cel van de lijst.

- Elk vak is één kolom, met als label de naam uit de bovenste rij. Typ in een vak om de waarde te wijzigen. Een vak dat grijs is, bevat een formule: het formulier toont het resultaat, en u wijzigt de formule in het blad.
- **Volgende zoeken** en **Vorige zoeken** gaan naar de volgende en de vorige rij. Naar een andere rij gaan, of op **Enter** drukken, schrijft wat u in deze rij hebt getypt.
- **Nieuw** toont een leeg formulier. Vul het in en druk daarna opnieuw op **Nieuw**, op **Volgende zoeken** of op **Sluiten**. De rij wordt toegevoegd onder de laatste rij van de lijst. Een tabel wordt met één rij uitgebreid. Als een cel onder de lijst niet leeg is, meldt het formulier dat en voegt het niets toe.
- **Verwijderen** haalt de rij weg waarnaar u kijkt, nadat het om bevestiging heeft gevraagd. De rijen eronder schuiven omhoog. Een tabel houdt altijd minstens één rij met gegevens. Een rij die verborgen is, of die door een filter wordt verborgen, wordt niet in het formulier getoond, niet meegeteld en nooit door het formulier verwijderd. Als de lijst te lang is om in zijn geheel te worden gevonden, meldt het formulier dat het alleen de eerste rijen toont.
- **Herstellen** neemt terug wat u in deze rij hebt getypt, voordat u de rij verlaat.
- **Criteria** laat u naar rijen zoeken. Typ een voorwaarde in een of meer vakken en druk daarna op **Volgende zoeken** of **Vorige zoeken**. Een voorwaarde is een waarde, zoals **Ann** (tekst die begint met het gezochte woord; **?** en ***** staan voor letters), of een waarde na een teken: **>100**, **<=5**, **<>0**, **=Oslo**. Een rij moet aan elke voorwaarde voldoen die u typt. Druk op **Formulier** om terug te gaan, of op **Wissen** om opnieuw te beginnen. Zolang er criteria zijn ingesteld, slaan **Volgende zoeken** en **Vorige zoeken** de rijen over die er niet aan voldoen.
- **Sluiten** schrijft de rij weg en sluit het formulier.

Elke rij die u wijzigt, toevoegt of verwijdert, is één stap, dus **Ongedaan maken** in het blad neemt die in één keer terug.

## Sorteren en filteren

De groep **Sorteren en filteren** heeft dezelfde opdrachten als het menu **Sorteren en filteren** op het tabblad Start. Zie [Sorteren en filteren](sort-filter).

## Tekst naar kolommen

Gebruik dit wanneer één kolom meerdere stukjes tekst bevat die u in aparte kolommen wilt hebben, zoals "Smith, Ann, 31".

1. Selecteer de cellen van **één** kolom.
2. Kies **Tekst naar kolommen** op het tabblad **Gegevens**.
3. Kies hoe de tekst moet worden gesplitst:
   - **Splitsen bij een scheidingsteken**: vink de tekens aan die tussen de stukjes staan: **Tab**, **Puntkomma**, **Komma**, **Spatie** of **Andere**, waarbij u zelf een teken typt. **Scheidingstekens achter elkaar als één behandelen** zorgt ervoor dat twee komma's achter elkaar geen leeg stukje maken. **Tekst tussen aanhalingstekens** geeft het teken aan dat een stukje omsluit dat niet mag worden gesplitst, zoals "Lee, Bo". Binnen de aanhalingstekens staan twee tekens achter elkaar voor één teken.
   - **Splitsen op vaste posities**: typ de posities waar een nieuwe kolom begint, waarbij tekens vanaf 1 worden geteld. Bijvoorbeeld 5, 12 begint een nieuwe kolom bij het vijfde en bij het twaalfde teken.
4. Kies wat elk stukje is: **Algemeen** leest getallen en datums zoals u ze hebt getypt, en **Tekst** behoudt elk stukje zoals het is, zodat "007" "007" blijft.
5. Typ bij **Gegevens plaatsen vanaf** de eerste cel voor de stukjes. Die is in het begin de eerste cel die u hebt geselecteerd.
6. In het vak onder de keuzes ziet u hoe de eerste rijen gesplitst worden zoals u hebt gekozen. Klik op **OK**.

De stukjes komen in de cellen rechts van de eerste cel, en ze vervangen wat daarin stond. Tekst naar kolommen is één stap die u ongedaan kunt maken.

## Duplicaten verwijderen

Gebruik dit om van elke groep herhaalde rijen alleen de eerste te bewaren.

1. Selecteer de cellen van de lijst, of één cel erin. JeffCrunch neemt het hele gegevensblok rond één cel.
2. Kies **Duplicaten verwijderen**.
3. Vink **Mijn gegevens hebben een kopregel** aan als de eerste rij namen bevat. Die rij blijft staan en wordt niet vergeleken.
4. Vink de kolommen aan die met elkaar moeten overeenkomen, wil twee rijen als herhaling gelden. Twee rijen zijn alleen herhalingen als elke aangevinkte kolom gelijk is. Hoofdletters en kleine letters tellen als hetzelfde, en een formule telt als de waarde die hij oplevert.
5. Klik op **OK**. Het dialoogvenster meldt hoeveel rijen zijn verwijderd en hoeveel er over zijn.

De rijen onder een verwijderde rij schuiven omhoog binnen de geselecteerde cellen, met hun opmaak en formules. De cellen onder de selectie verschuiven niet. De lege rijen die onderaan de selectie overblijven, worden leeggemaakt. Duplicaten verwijderen is één stap die u ongedaan kunt maken.

## Doelzoeken

Gebruik dit wanneer u het antwoord kent dat u wilt, maar niet het getal dat het oplevert. Een betalingscel bevat bijvoorbeeld een formule met een rentepercentage, en u wilt dat de betaling 500 is. Doelzoeken probeert getallen in de rentecel tot de betaling 500 is.

1. Kies **Doelzoeken** op het tabblad **Gegevens**.
2. Typ bij **Cel met de formule** de cel die de formule bevat, zoals B3. Die is in het begin de cel die u hebt geselecteerd.
3. Typ bij **Gewenste waarde** het antwoord dat de formule moet geven.
4. Typ bij **Cel om te wijzigen** de cel die Doelzoeken mag wijzigen, zoals B1. Die moet een getal bevatten, geen formule.
5. Klik op **OK**.

Als Doelzoeken een getal vindt, blijft dat in de cel om te wijzigen staan, en de statusbalk zegt welk getal het is. Als het niets vindt, verandert er niets en krijgt u dat te horen. Probeer een ander startgetal in de cel om te wijzigen, en controleer of de formule die cel werkelijk gebruikt. Doelzoeken is één stap die u ongedaan kunt maken.

## Scenariobeheer

Een scenario is een benoemde reeks waarden voor een aantal cellen. Gebruik scenario's om "wat als" uit te proberen op één blad. Houd bijvoorbeeld een scenario "Lage verkoop" en een scenario "Hoge verkoop" aan voor dezelfde invoercellen, en toon het scenario dat u wilt bekijken.

1. Kies **Scenariobeheer** op het tabblad **Gegevens**.
2. Klik op **Toevoegen**. Typ een naam, zoals Hoge verkoop. Typ bij **Wijzigende cellen** de cellen die het scenario instelt, zoals B1:B3 of D5. Een scenario kan maximaal 32 cellen wijzigen. De cellen die u hebt geselecteerd, worden voor u ingevuld. Elke cel begint met de waarde die hij nu heeft. Typ in de tabel de waarde die dit scenario moet gebruiken en klik op **OK**.
3. Kies een scenario in de lijst en klik op **Weergeven** om de waarden ervan in de cellen te zetten. Dubbelklikken op een naam doet hetzelfde. Klik op **Bewerken** om de naam, de opmerking of de waarden te wijzigen. Klik op **Verwijderen** om het weg te halen.

Weergeven, Toevoegen, Bewerken en Verwijderen zijn elk één stap die u ongedaan kunt maken.

**Samenvatting** zet alle scenario's naast elkaar op een nieuw blad met de naam Scenariosamenvatting. Het vraagt om de resultaatcellen, dat zijn de cellen die u wilt vergelijken, zoals het totaal in B10. Voor elk scenario toont de samenvatting de waarden van de wijzigende cellen en wat de resultaatcellen worden, berekend met de waarden van dat scenario. Een kolom met de naam Huidige waarden toont hoe de cellen waren toen u de samenvatting maakte. Uw eigen cellen blijven zoals ze waren.

De scenario's worden in de werkmap opgeslagen. Een werkmap die door een ander programma is gemaakt, behoudt zijn scenario's, en u ziet ze hier. Een kopie die u opslaat als een oud .xls-bestand of als .ods, bevat ze niet, en het opslaan meldt dat.

## Subtotaal

Gebruik dit om na elke groep rijen met dezelfde waarde in één kolom een rij met totalen te zetten, en onderaan een eindtotaal. De lijst heeft een kopregel nodig, en hij moet gesorteerd zijn op de kolom waarop u groepeert, zodat gelijke waarden naast elkaar staan.

1. Selecteer een cel in de lijst.
2. Kies **Subtotaal** in de groep **Overzicht** op het tabblad **Gegevens**.
3. Kies bij **Bij elke wijziging in** de kolom die de rijen groepeert, zoals Regio.
4. Kies bij **Functie gebruiken** wat moet worden berekend: **Som**, **Aantal**, **Gemiddelde**, **Max**, **Min** en andere.
5. Vink bij **Subtotaal toevoegen aan** elke kolom aan die moet worden opgeteld.
6. Laat **Huidige subtotalen vervangen** aangevinkt om subtotalen die er al staan te vervangen. Klik op **OK**.

Elke totaalrij zegt "Totaal Oost" (of de waarde waarvoor hij geldt) en bevat een SUBTOTAAL-formule, zodat hij de andere totaalrijen niet meetelt. De rijen van elke groep worden gegroepeerd, zodat u met de knoppen 1, 2 en 3 naast de rijen alleen de totalen kunt tonen. Om de totalen weg te halen, opent u **Subtotaal** opnieuw en klikt u op **Alles verwijderen**. Subtotalen toevoegen of verwijderen is één stap die u ongedaan kunt maken.

## Geavanceerd filter

Met de filterknoppen kiest u rijen per keer één kolom. Met **Geavanceerd** (in de groep **Sorteren en filteren**) schrijft u de voorwaarden in het blad, zodat ze zo lang kunnen zijn als nodig.

1. Kopieer naast de lijst de koppen van de kolommen die u wilt testen naar een rij lege cellen.
2. Schrijf onder elke kop een voorwaarde. Voorwaarden in **dezelfde rij** moeten alle waar zijn (dit is "en"). Voorwaarden in **verschillende rijen** zijn alternatieven: een rij van de lijst hoeft aan maar één van die rijen te voldoen (dit is "of").
3. Kies **Geavanceerd**. Controleer of **Lijstbereik** de lijst met zijn kopregel is, en typ de cellen van uw koppen en voorwaarden bij **Voorwaardenbereik**.
4. Kies **De andere rijen in de lijst verbergen**, of **De rijen naar een andere plaats kopiëren** en typ de eerste cel van de plaats, zoals H1. Vink **Rijen overslaan die een eerdere rij herhalen** aan om alleen één van elk te kopiëren.
5. Klik op **OK**.

Zo schrijft u een voorwaarde:

- Een woord, zoals **An**, selecteert tekst die ermee begint. Hoofdletters en kleine letters tellen als hetzelfde. **?** staat voor één willekeurig teken en ***** voor een willekeurige reeks tekens.
- Een getal selecteert cellen die er gelijk aan zijn.
- **=Ann** selecteert tekst die precies Ann is. **=** alleen selecteert lege cellen.
- **>**, **>=**, **<**, **<=** en **<>** (niet gelijk aan) voor een getal of woord vergelijken ermee, zoals **>25**. **<>** alleen selecteert cellen die niet leeg zijn.

Een rij voorwaarden zonder inhoud selecteert elke rij. Om de verborgen rijen weer te tonen, kiest u **Wissen**. Een kopie wordt meteen gemaakt en verandert niet wanneer de lijst verandert. Geavanceerd filter is één stap die u ongedaan kunt maken.

## Gegevensvalidatie

Gebruik dit om vast te leggen wat in sommige cellen mag worden getypt, zodat een verkeerde invoer wordt tegengehouden of ter discussie gesteld.

1. Selecteer de cellen.
2. Kies **Gegevensvalidatie** in de groep **Gegevenshulpmiddelen** op het tabblad **Gegevens**.
3. Kies op het tabblad **Instellingen** wat u wilt **Toestaan**: **Elke waarde** (geen regel), **Geheel getal**, **Decimaal getal**, **Lijst**, **Datum**, **Tijd**, **Tekstlengte** of **Aangepast**.
4. Kies bij een getal, datum, tijd of tekstlengte bij **Gegevens** (**tussen**, **niet tussen**, **gelijk aan** enzovoort) en typ de grenzen. Een grens kan een waarde zijn of een formule die begint met =.
5. Voor een **Lijst** typt u de keuzes met komma's ertussen, zoals Rood,Groen,Blauw, of typt u na = een celbereik, zoals =$A$1:$A$5. De cel toont dan een pijl met de keuzes. Schakel **Keuzelijst in cel** uit om de pijl te verbergen.
6. Voor **Aangepast** typt u een formule die begint met =. De invoer is toegestaan wanneer de formule waar is.
7. Met **Lege cellen negeren** kan een lege cel doorgaan.

Vink op het tabblad **Invoerbericht** het vakje aan en schrijf een titel en een bericht. Ze worden naast de cel getoond wanneer die is geselecteerd.

Kies op het tabblad **Foutmelding** wat er gebeurt met een invoer die niet is toegestaan. **Stop** weigert hem. **Waarschuwing** vraagt of hij moet worden behouden. **Informatie** meldt het aan de gebruiker en behoudt de invoer. Schrijf een titel en een bericht voor de melding.

**Ongeldige gegevens cirkelen** (naast **Gegevensvalidatie**) zet een rode ovaal om elke cel in het blad waarvan de waarde de regel niet toestaat, zoals een getal dat er al stond voordat de regel bestond, of het resultaat van een formule. Een bericht laat u weten als elke cel in orde is. Een cel die u corrigeert, verliest zijn ovaal. **Validatiemarkeringen wissen** haalt alle ovalen weg.

**Alles wissen** haalt de regels van de geselecteerde cellen. De nieuwe regel vervangt elke regel die de geselecteerde cellen hadden; andere cellen behouden de hunne. Het wijzigen van regels is één stap die u ongedaan kunt maken, en de regels worden in het bestand opgeslagen.

## Consolideren

Gebruik dit om bereiken die op elkaar lijken, zoals dezelfde tabel uit meerdere bladen, samen te voegen tot één tabel.

1. Kies **Consolideren** in de groep **Gegevenshulpmiddelen** op het tabblad **Gegevens**.
2. Kies de **Functie** waarmee moet worden gecombineerd, zoals **Som** of **Gemiddelde**.
3. Typ een bereik in het vak en klik op **Toevoegen**. Gebruik **Blad2!A1:C10** voor een bereik op een ander blad. Voeg elk bereik toe dat u wilt combineren. Selecteer een bereik in de lijst en klik op **Verwijderen** om het weg te halen.
4. Als de bereiken namen in hun eerste rij hebben, vink dan **Bovenste rij bevat de namen van kolommen** aan. Als ze namen in hun eerste kolom hebben, vink dan **Linker kolom bevat de namen van rijen** aan.
5. Typ bij **Resultaat plaatsen op** de eerste cel voor het resultaat.
6. Vink **Koppelingen naar de brongegevens maken** aan als elk resultaat een formule moet zijn die zijn brongegevens leest, zodat het verandert wanneer zij veranderen. Zonder dit zijn de resultaten gewone getallen.
7. Klik op **OK**.

Zonder aangevinkte namen wordt de cel op dezelfde plaats in elk bereik gecombineerd (de eerste cel van elk bereik, dan de tweede, enzovoort). Met aangevinkte namen worden cellen met dezelfde namen gecombineerd, waar ze ook in elk bereik staan, en een naam die alleen sommige bereiken hebben, krijgt toch een rij of kolom. Consolideren is één stap die u ongedaan kunt maken.

## Prognoseblad

Met het prognoseblad maakt u een nieuw blad met een tabel en een grafiek van toekomstige waarden, berekend uit een tijdlijn en een reeks waarden. Het vindt de trend en het zich herhalende seizoen van de waarden, en voegt een band toe waarbinnen de toekomstige waarden waarschijnlijk blijven.

1. Selecteer de tijdlijn en de waarden, twee kolommen naast elkaar (de tijdlijn eerst). De tijdlijn bestaat uit datums of getallen die even ver uit elkaar liggen, zoals elke maand of elke 5 eenheden. Een eerste rij met namen wordt niet meegerekend.
2. Kies **Prognoseblad** in de groep **Prognose** op het tabblad **Gegevens**.
3. Een afbeelding toont de prognose en verandert wanneer u de keuzes wijzigt. Kies **Lijndiagram** of **Kolomdiagram**.
4. Stel de keuzes in:
   - **Bereik tijdlijn** en **Bereik waarden** zijn de cellen die worden gelezen. Typ hier andere cellen als de selectie verkeerd was.
   - **Begin prognose** is leeg om na het laatste punt te beginnen. Een datum of getal vóór het laatste punt begint daar en gebruikt alleen de punten die ervoor liggen, zodat u de prognose kunt vergelijken met wat er gebeurd is.
   - **Einde prognose** is het laatste tijdstip waarvoor een prognose wordt gemaakt. Er worden maximaal 10.000 punten voorspeld.
   - **Betrouwbaarheidsinterval** is hoe zeker de band is, in het begin 95%.
   - **Seizoensinvloed** wordt bepaald door naar de waarden te kijken, of u stelt de lengte van het seizoen in punten in, of kiest geen. De waarden moeten twee volledige seizoenen bevatten.
   - **Ontbrekende punten aanvullen met** bepaalt wat een tijdstip zonder waarde krijgt: een rechte lijn tussen de buren, of nul.
   - **Dubbele waarden samenvoegen met** bepaalt hoe waarden op hetzelfde tijdstip worden gecombineerd: Gemiddelde, Aantal, AantalA, Max, Mediaan, Min of Som.
   - **Prognosestatistieken opnemen** voegt een tabel toe met Alfa, Bèta, Gamma, MASE, SMAPE, MAE en RMSE, die aangeven hoe goed de prognose bij de waarden past.
5. Als er iets niet klopt, zegt het dialoogvenster in gewone woorden wat dat is (stappen die niet gelijk zijn, te weinig punten, tekst in de waarden), en **Maken** wacht.
6. Klik op **Maken**.

Het nieuwe blad komt vóór het blad waarop u stond. De tabel heeft de kolommen Tijdlijn, Waarden, Prognose, Ondergrens betrouwbaarheidsinterval en Bovengrens betrouwbaarheidsinterval. De prognose en de grenzen zijn formules die **FORECAST.ETS** en **FORECAST.ETS.CONFINT** gebruiken op de eigen kolommen Tijdlijn en Waarden van de tabel, dus ze worden opnieuw berekend wanneer u een waarde wijzigt. De grafiek toont de waarden, de prognose en de band. Het prognoseblad is één stap die u ongedaan kunt maken.

De getallen kunnen enkele procenten verschillen van die in andere spreadsheetprogramma's, die hun eigen manier gebruiken om de vlakke lijnen aan te passen. Een prognose die een bestand al bevat, behoudt de waarden die het bestand heeft opgeslagen totdat de gegevens die hij leest, worden gewijzigd.

Een bestand met meer dan 1.000.000 punten wordt niet gebruikt.
