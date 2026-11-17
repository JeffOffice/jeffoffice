# Gegevenshulpmiddelen
<!-- keywords: formulier, gegevensformulier, gegevens invoeren, invoer, record, records, nieuw record, volgende zoeken, vorige zoeken, criteria, scenariobeheer, scenario's, wat-als, wat als, scenariosamenvatting, gegevens, tabblad gegevens, tekst naar kolommen, splitsen, scheidingsteken, komma, tab, vaste breedte, duplicaten verwijderen, doelzoeken, subtotaal, totalen, geavanceerd filter, voorwaarden, gegevensvalidatie, ongeldige gegevens markeren, rode ovaal, validatiemarkeringen wissen, consolideren, bereiken combineren, toestaan, lijst, keuzelijst, invoerbericht, foutmelding, herhaalde rijen, uniek, tekstbestand, csv, uit tekst, kolommen, scheidingsteken, aanhalingstekens, koppelingen bewerken, waarden bijwerken, bron wijzigen, koppelingen, extern, bron, koppeling verbreken, status controleren, bron openen -->

Op het tabblad **Gegevens** staan de hulpmiddelen die werken op een lijst met waarden: tekst uit een bestand halen, een lijst sorteren, tekst in kolommen splitsen en herhaalde rijen verwijderen.

## Uit tekst/CSV

Met **Uit tekst/CSV** opent u een tekstbestand (.csv, .tsv of .txt). Dat is hetzelfde als **Tekstbestand openen** op het tabblad Bestand. JeffCrunch vraagt hoe het bestand in kolommen is verdeeld en wat voor soort tekst het bevat, en toont het daarna als een nieuwe werkmap.

## Koppelingen bewerken

Als een werkmap formules heeft die naar een andere werkmap verwijzen (zoals ='[Prijzen.xlsx]Blad1'!A1), toont **Koppelingen bewerken** in de groep **Gegevens ophalen** die andere werkmappen. Die worden bronnen genoemd. Elke rij toont waar de bron staat en wat de status is. Totdat de werkmap opnieuw wordt geopend met zijn bronnen, toont hij de getallen die het bestand van die bronnen heeft opgeslagen.

- **Status controleren** zoekt voor elke bron op deze computer en meldt **Gevonden** of dat de bron niet is gevonden. Een bron op het web wordt niet opgehaald; de status zegt dan dat er niet is gecontroleerd.
- **Waarden bijwerken** leest de gekozen bron van deze computer en geeft elke cel die alleen een verwijzing naar een van zijn cellen is, de waarde die die cel nu heeft. Een cel die meer met de waarde doet, zoals `=[1]Prijzen!A1*2`, behoudt het getal dat het bestand heeft opgeslagen. Het bericht onder de lijst geeft aan hoeveel cellen zijn bijgewerkt en hoeveel niet. Waarden bijwerken is één stap, dus met **Ongedaan maken** zet u de oude waarden terug. De nieuwe waarden worden ook in het eigen deel van de koppeling in het bestand geschreven wanneer u opslaat, zodat de werkmap ze de volgende keer dat hij wordt geopend toont, ook als de bron er niet is.
- **Bron wijzigen** laat u een ander bestand kiezen voor de gekozen koppeling, bijvoorbeeld wanneer de werkmap is verplaatst. De formules blijven zoals ze zijn. De nieuwe plaats wordt bij het opslaan in het bestand geschreven, en **Ongedaan maken** neemt de wijziging terug.
- **Bron openen** opent de gekozen bron in een nieuw venster, als die kan worden gevonden.
- **Koppeling verbreken** zet elke cel die naar de gekozen bron verwijst om in de waarde die die nu toont, en een naam die een plaats in die bron aanduidde, wordt **#REF!**. JeffCrunch vraagt eerst om bevestiging. Koppeling verbreken is één stap, dus met **Ongedaan maken** brengt u de formules, de namen en de koppelingen terug, totdat u de werkmap opslaat. Wanneer er geen koppeling meer over is, wordt het bestand opgeslagen zonder de onderdelen die ze bevatten.

## Formulier

**Formulier** (in de groep **Gegevenshulpmiddelen** en in het zoekvak) toont één rij van een lijst tegelijk, als klein formulier. Een lijst is een tabel, of een celbereik met een rij namen bovenaan en onder elke naam een rij gegevens. Klik eerst op een willekeurige cel van de lijst.

- Elk vak is één kolom en heeft als label de naam uit de bovenste rij. Typ in een vak om de waarde te wijzigen. Een vak dat grijs is, bevat een formule: het formulier toont wat die oplevert, en u wijzigt de formule in het blad.
- **Volgende zoeken** en **Vorige zoeken** gaan naar de volgende en de vorige rij. Naar een andere rij gaan, of op **Enter** drukken, schrijft wat u in deze rij hebt getypt.
- **Nieuw** toont een leeg formulier. Vul het in en druk daarna opnieuw op **Nieuw**, op **Volgende zoeken** of op **Sluiten**. De rij wordt toegevoegd onder de laatste rij van de lijst. Een tabel wordt met één rij groter. Als een cel onder de lijst niet leeg is, meldt het formulier dat en voegt het niets toe.
- **Verwijderen** haalt de rij weg waar u naar kijkt, nadat u bent gevraagd dit te bevestigen. De rijen eronder schuiven omhoog. Een tabel houdt altijd minstens één rij met gegevens. Een rij die verborgen is, of die door een filter wordt verborgen, wordt niet in het formulier getoond, niet geteld en door het formulier nooit verwijderd. Als de lijst te lang is om in zijn geheel te worden doorzocht, meldt het formulier dat het alleen de eerste rijen toont.
- **Herstellen** neemt terug wat u in deze rij hebt getypt, voordat u de rij verlaat.
- **Criteria** laat u rijen zoeken. Typ een voorwaarde in een of meer vakken en druk daarna op **Volgende zoeken** of **Vorige zoeken**. Een voorwaarde is een waarde, zoals **Ann** (tekst die begint met die letters; **?** en ***** staan voor letters), of een waarde na een teken: **>100**, **<=5**, **<>0**, **=Oslo**. Een rij moet aan elke voorwaarde voldoen die u hebt getypt. Druk op **Formulier** om terug te gaan, of op **Wissen** om opnieuw te beginnen. Zolang er criteria zijn ingesteld, slaan **Volgende zoeken** en **Vorige zoeken** de rijen over die er niet aan voldoen.
- **Sluiten** schrijft de rij weg en sluit het formulier.

Elke rij die u wijzigt, toevoegt of verwijdert, is één stap, dus met **Ongedaan maken** in het blad neemt u die in één keer terug.

## Sorteren en filteren

De groep **Sorteren en filteren** bevat dezelfde opdrachten als het menu **Sorteren en rijen selecteren** op het tabblad Start. Zie [Sorteren en filteren](sort-filter).

## Tekst naar kolommen

Gebruik dit wanneer één kolom meerdere stukjes tekst bevat die u in aparte kolommen wilt, zoals "Smith, Ann, 31".

1. Selecteer de cellen van **één** kolom.
2. Kies **Tekst naar kolommen** op het tabblad **Gegevens**.
3. Kies hoe de tekst wordt gesplitst:
   - **Splitsen bij een scheidingsteken**: vink de tekens aan die tussen de stukjes staan: **Tab**, **Puntkomma**, **Komma**, **Spatie** of **Andere**, waar u zelf het teken typt. **Opeenvolgende scheidingstekens als één behandelen** zorgt ervoor dat twee komma's achter elkaar geen leeg stukje maken. **Tekst tussen aanhalingstekens** geeft het teken aan dat een stuk omsluit dat niet mag worden gesplitst, zoals "Lee, Bo". Binnen de aanhalingstekens staan twee tekens achter elkaar voor één teken.
   - **Splitsen op vaste posities**: typ de posities waar een nieuwe kolom begint, waarbij tekens vanaf 1 worden geteld. Bijvoorbeeld: 5, 12 begint een nieuwe kolom bij het vijfde en bij het twaalfde teken.
4. Kies wat elk stukje is: **Algemeen** leest getallen en datums zoals u ze hebt getypt, en **Tekst** houdt elk stukje zoals het is, zodat "007" "007" blijft.
5. Typ bij **Stukjes plaatsen vanaf** de eerste cel voor de stukjes. Die staat standaard op de eerste cel die u hebt geselecteerd.
6. Het vak onder de keuzes toont de eerste rijen gesplitst zoals u hebt gekozen. Druk op **OK**.

De stukjes komen in de cellen rechts van de eerste cel, en ze vervangen wat er in die cellen stond. Tekst naar kolommen is één stap die u ongedaan kunt maken.

## Duplicaten verwijderen

Gebruik dit om van elke groep herhaalde rijen alleen de eerste te bewaren.

1. Selecteer de cellen van de lijst, of één cel erin. JeffCrunch neemt het hele gegevensblok rond één cel.
2. Kies **Duplicaten verwijderen**.
3. Vink **Mijn gegevens hebben een kopregel** aan als de eerste rij namen bevat. Die rij blijft staan en wordt niet vergeleken.
4. Vink de kolommen aan die moeten overeenkomen, wil twee rijen als herhaling gelden. Twee rijen zijn alleen herhalingen wanneer elke aangevinkte kolom gelijk is. Hoofdletters en kleine letters tellen als gelijk, en een formule telt als de waarde die hij oplevert.
5. Druk op **OK**. Het dialoogvenster meldt hoeveel rijen zijn verwijderd en hoeveel er over zijn.

De rijen onder een verwijderde rij schuiven omhoog binnen de geselecteerde cellen, met hun opmaak en formules. De cellen onder de selectie verschuiven niet. De lege rijen die onderaan de selectie overblijven, worden leeggemaakt. Duplicaten verwijderen is één stap die u ongedaan kunt maken.

## Doelzoeken

Gebruik dit wanneer u het antwoord weet dat u wilt, maar niet het getal dat het oplevert. Bijvoorbeeld: een cel met een betaling bevat een formule die een rentepercentage gebruikt, en u wilt dat de betaling 500 is. Doelzoeken probeert getallen in de rentecel tot de betaling 500 is.

1. Kies **Doelzoeken** op het tabblad **Gegevens**.
2. Typ bij **Cel met de formule** de cel met de formule, zoals B3. Die staat standaard op de cel die u hebt geselecteerd.
3. Typ bij **Gewenste waarde** het antwoord dat de formule moet geven.
4. Typ bij **Te wijzigen cel** de cel die Doelzoeken mag wijzigen, zoals B1. Die moet een getal bevatten, geen formule.
5. Druk op **OK**.

Als Doelzoeken een getal vindt, blijft het in de te wijzigen cel staan, en de statusbalk zegt wat het is. Als dat niet lukt, verandert er niets en krijgt u daarvan bericht. Probeer een ander startgetal in de te wijzigen cel en controleer of de formule echt die cel gebruikt. Doelzoeken is één stap die u ongedaan kunt maken.

## Scenariobeheer

Een scenario is een benoemde reeks waarden voor een aantal cellen. Gebruik scenario's om "wat als" op één blad uit te proberen. Bewaar bijvoorbeeld een scenario "Lage verkoop" en een scenario "Hoge verkoop" voor dezelfde invoercellen, en toon het scenario dat u wilt bekijken.

1. Kies **Scenariobeheer** op het tabblad **Gegevens**.
2. Druk op **Toevoegen**. Typ een naam, zoals Hoge verkoop. Typ bij **Te wijzigen cellen** de cellen die het scenario instelt, zoals B1:B3 of D5. Een scenario kan maximaal 32 cellen wijzigen. De cellen die u hebt geselecteerd, worden voor u ingevuld. Elke cel begint met de waarde die hij nu heeft. Typ in de tabel de waarde die dit scenario moet gebruiken, en druk op **OK**.
3. Kies een scenario in de lijst en druk op **Weergeven** om de waarden in de cellen te zetten. Dubbelklikken op een naam doet hetzelfde. Druk op **Bewerken** om de naam, de opmerking of de waarden te wijzigen. Druk op **Verwijderen** om het weg te halen.

Weergeven, Toevoegen, Bewerken en Verwijderen zijn elk één stap die u ongedaan kunt maken.

Met **Samenvatting** zet u alle scenario's naast elkaar op een nieuw blad met de naam Scenariosamenvatting. Er wordt gevraagd naar de resultaatcellen, dat zijn de cellen die u wilt vergelijken, zoals het totaal in B10. Voor elk scenario toont de samenvatting de waarden van de te wijzigen cellen en wat de resultaatcellen opleveren, berekend met de waarden van dat scenario. Een kolom met de naam Huidige waarden toont hoe de cellen waren toen u de samenvatting maakte. Uw eigen cellen blijven zoals ze waren.

De scenario's worden in de werkmap opgeslagen. Een werkmap die met een ander programma is gemaakt, behoudt zijn scenario's, en u ziet ze hier. Een kopie die als oudere .xls-bestand of als .ods is opgeslagen, bevat ze niet, en het opslaan meldt dat.

## Subtotaal

Gebruik dit om onder elke groep rijen die dezelfde waarde in één kolom hebben een rij met totalen te zetten, en onderaan een eindtotaal. De lijst heeft een kopregel nodig en moet worden gesorteerd op de kolom waarop u groepeert, zodat gelijke waarden naast elkaar staan.

1. Selecteer een cel in de lijst.
2. Kies **Subtotaal** in de groep **Overzicht** op het tabblad **Gegevens**.
3. Kies bij **Bij elke wijziging in** de kolom die de rijen groepeert, zoals Regio.
4. Kies bij **Functie gebruiken** wat u wilt berekenen: **Som**, **Aantal**, **Gemiddelde**, **Max**, **Min** en andere.
5. Vink bij **Subtotaal toevoegen aan** elke kolom aan die u wilt optellen.
6. Laat **Huidige subtotalen vervangen** aangevinkt om subtotalen die er al staan te vervangen. Druk op **OK**.

Elke totaalrij zegt "Totaal Oost" (of de waarde waarvoor het is) en bevat een SUBTOTAAL-formule, zodat die de andere totaalrijen overslaat. De rijen van elke groep worden gegroepeerd, zodat u met de knoppen 1, 2 en 3 naast de rijen alleen de totalen kunt tonen. Om de totalen weg te halen, opent u **Subtotaal** opnieuw en drukt u op **Alles verwijderen**. Subtotalen toevoegen of verwijderen is één stap die u ongedaan kunt maken.

## Geavanceerd filter

Met de filterknoppen selecteert u rijen per keer één kolom. Met **Geavanceerd** (in de groep **Sorteren en filteren**) schrijft u de voorwaarden in het blad, zodat ze zo lang kunnen zijn als nodig.

1. Kopieer naast de lijst de koppen van de kolommen die u wilt controleren naar een rij lege cellen.
2. Schrijf onder elke kop een voorwaarde. Voorwaarden in **dezelfde rij** moeten allemaal waar zijn (dit is "en"). Voorwaarden in **verschillende rijen** zijn alternatieven: een rij van de lijst moet aan slechts één van die rijen voldoen (dit is "of").
3. Kies **Geavanceerd**. Controleer dat **Lijstbereik** de lijst met zijn kopregel is, en typ de cellen van uw koppen en voorwaarden bij **Voorwaardenbereik**.
4. Kies **De andere rijen in de lijst verbergen**, of **De rijen naar een andere plaats kopiëren** en typ de eerste cel van de plaats, zoals H1. Vink **Rijen die een eerdere rij herhalen overslaan** aan om er maar één van elk te kopiëren.
5. Druk op **OK**.

Zo schrijft u een voorwaarde:

- Een woord, zoals **An**, selecteert tekst die ermee begint. Hoofdletters en kleine letters tellen als gelijk. **?** staat voor één willekeurig teken en ***** voor een willekeurige reeks tekens.
- Een getal selecteert cellen die er gelijk aan zijn.
- **=Ann** selecteert tekst die precies Ann is. **=** alleen selecteert lege cellen.
- **>**, **>=**, **<**, **<=** en **<>** (niet gelijk aan) vóór een getal of woord vergelijken ermee, zoals **>25**. **<>** alleen selecteert cellen die niet leeg zijn.

Een rij voorwaarden zonder inhoud selecteert elke rij. Om de verborgen rijen weer te tonen, kiest u **Wissen**. Een kopie wordt meteen gemaakt en verandert niet wanneer de lijst verandert. Geavanceerd filter is één stap die u ongedaan kunt maken.

## Gegevensvalidatie

Gebruik dit om vast te leggen wat in sommige cellen mag worden getypt, zodat een verkeerde invoer wordt tegengehouden of in twijfel getrokken.

1. Selecteer de cellen.
2. Kies **Gegevensvalidatie** in de groep **Gegevenshulpmiddelen** op het tabblad **Gegevens**.
3. Kies op het tabblad **Instellingen** wat u wilt **Toestaan**: **Elke waarde** (geen regel), **Geheel getal**, **Decimaal getal**, **Lijst**, **Datum**, **Tijd**, **Tekstlengte** of **Aangepast**.
4. Kies bij een getal, datum, tijd of tekstlengte **Gegevens** (**tussen**, **niet tussen**, **gelijk aan**, enzovoort) en typ de grenzen. Een grens kan een waarde zijn of een formule die begint met =.
5. Voor een **Lijst** typt u de keuzes met komma's ertussen, zoals Rood,Groen,Blauw, of typt u na = een celbereik, zoals =$A$1:$A$5. De cel toont dan een pijl met de keuzes. Schakel **Keuzelijst in cel** uit om de pijl te verbergen.
6. Bij **Aangepast** typt u een formule die begint met =. De invoer is toegestaan wanneer de formule waar is.
7. Met **Lege cellen negeren** mag een lege cel doorgaan.

Vink op het tabblad **Invoerbericht** het vakje aan en schrijf een titel en een bericht. Ze worden naast de cel getoond wanneer die is geselecteerd.

Kies op het tabblad **Foutmelding** wat er gebeurt met een invoer die niet is toegestaan. **Stop** weigert de invoer. **Waarschuwing** vraagt of u hem wilt houden. **Informatie** laat de gebruiker weten dat, en houdt de invoer. Schrijf een titel en een bericht voor de melding.

**Ongeldige gegevens markeren** (naast **Gegevensvalidatie**) zet een rode ovaal om elke cel op het blad waarvan de waarde niet is toegestaan door de regel, zoals een getal dat er stond voordat de regel bestond, of het resultaat van een formule. Een bericht laat weten of alle cellen in orde zijn. Een cel die u verbetert, verliest zijn ovaal. **Validatiemarkeringen wissen** haalt alle ovalen weg.

Met **Alles wissen** haalt u de regels weg van de geselecteerde cellen. De nieuwe regel vervangt elke regel die de geselecteerde cellen hadden; andere cellen behouden de hunne. Regels wijzigen is één stap die u ongedaan kunt maken, en de regels worden in het bestand opgeslagen.

## Consolideren

Gebruik dit om bereiken die op elkaar lijken, zoals dezelfde tabel op meerdere bladen, samen te voegen tot één tabel.

1. Kies **Consolideren** in de groep **Gegevenshulpmiddelen** op het tabblad **Gegevens**.
2. Kies de **Functie** waarmee u wilt combineren, zoals **Som** of **Gemiddelde**.
3. Typ een bereik in het vak en druk op **Toevoegen**. Gebruik **Blad2!A1:C10** voor een bereik op een ander blad. Voeg elk bereik toe dat u wilt combineren. Selecteer een bereik in de lijst en druk op **Verwijderen** om het weg te halen.
4. Als de bereiken namen in hun eerste rij hebben, vink dan **Bovenste rij bevat de namen van kolommen** aan. Als ze namen in hun eerste kolom hebben, vink dan **Linkerkolom bevat de namen van rijen** aan.
5. Typ bij **Resultaat plaatsen op** de eerste cel voor het resultaat.
6. Vink **Koppelingen naar de brongegevens maken** aan als elk resultaat een formule moet zijn die zijn broncellen leest, zodat het verandert wanneer die veranderen. Zonder dit vakje zijn de resultaten gewone getallen.
7. Druk op **OK**.

Als er geen namen zijn aangevinkt, wordt de cel op dezelfde plaats in elk bereik gecombineerd (de eerste cel van elk bereik, dan de tweede, enzovoort). Met aangevinkte namen worden cellen met dezelfde namen gecombineerd, waar ze ook in elk bereik staan, en een naam die maar in sommige bereiken voorkomt, krijgt nog steeds een rij of kolom. Consolideren is één stap die u ongedaan kunt maken.
