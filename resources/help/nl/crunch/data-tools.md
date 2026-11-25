# Gegevenshulpmiddelen
<!-- keywords: prognoseblad, prognose, toekomstige waarden, betrouwbaarheidsinterval, seizoensinvloed, trend, voortschrijdend, formulier, gegevensformulier, gegevens invoeren, record, records, nieuw record, volgende zoeken, vorige zoeken, criteria, scenariobeheer, scenario's, wat-als, wat als, scenariosamenvatting, gegevens, tabblad gegevens, tekst naar kolommen, splitsen, scheidingsteken, komma, tab, vaste breedte, duplicaten verwijderen, doelzoeken, subtotaal, totalen, geavanceerd filter, voorwaarden, gegevensvalidatie, ongeldige gegevens cirkelen, rode ovaal, validatiemarkeringen wissen, consolideren, bereiken combineren, toestaan, lijst, vervolgkeuzelijst, invoerbericht, foutmelding, herhaalde rijen, uniek, tekstbestand, csv, uit tekst, kolommen, scheidingsteken, aanhalingstekens, koppelingen bewerken, waarden bijwerken, bron wijzigen, koppelingen, extern, bron, koppeling verbreken, status controleren, bron openen -->

Het tabblad **Gegevens** bevat de hulpmiddelen die werken op een lijst met waarden: tekst uit een bestand halen, een lijst sorteren, tekst in kolommen splitsen en herhaalde rijen verwijderen.

## Uit tekst/CSV

**Uit tekst/CSV** opent een tekstbestand (.csv, .tsv of .txt). Het is hetzelfde als **Tekstbestand openen** op het tabblad Bestand. JeffCrunch vraagt hoe het bestand in kolommen is verdeeld en welk soort tekst het bevat, en toont het daarna als nieuwe werkmap.

## Uit bestand

**Uit bestand** in de groep **Gegevens ophalen** laadt de rijen van een JSON-bestand, een XML-bestand of een andere werkmap als tabel. Alleen een bestand op deze computer kan worden geladen. Een webadres of een bestand op een andere computer wordt geweigerd.

1. Klik op **Uit bestand**, dan op **Bladeren** en kies het bestand. De eerste rijen worden meteen getoond, met per kolom het soort dat JeffCrunch uit die eerste rijen heeft herkend: getal, datum, waar of onwaar, of tekst. Het hele bestand wordt bij het laden gecontroleerd, dus een kolom kan uiteindelijk als tekst eindigen. Er verandert niets op het blad totdat u op **Laden** klikt.
2. Als het bestand meer dan één lijst bevat, kiest u de lijst die u wilt laden. In een JSON-bestand is dat een lijst met items met benoemde waarden, zoals `$.items`. In een XML-bestand is dat een item dat zich herhaalt, zoals `/feed/entry`. In een werkmap is dat een blad, een tabel of een benoemd bereik.
3. Een JSON-item binnen een item wordt een kolom met de naam `buiten.binnen`, tot vier niveaus diep. Een lijst met losse waarden wordt samengevoegd in één cel (de eerste 10.000 waarden), of verdeeld over kolommen (de eerste 16), naar keuze. In XML worden de kenmerken en de onderliggende items kolommen. Twee verschillende waarden delen nooit een kolom: als twee dezelfde naam zouden krijgen, krijgt de tweede een getal achter de naam of, in XML, zijn voorvoegsel, en een opmerking meldt dat. Een naam die binnen één JSON-item meerdere keren voorkomt, behoudt de laatste waarde, en een opmerking meldt dat. Een getal dat te groot of te klein is voor een cel, wordt als tekst bewaard, zoals het is geschreven, en een teken dat niet gelezen kan worden, wordt getoond als vervangteken; beide worden vermeld.
4. Vink **Alle kolommen als tekst laden** aan om elke waarde te bewaren zoals ze is geschreven. Een kolom met getallen die met een nul begint, zoals 007, wordt altijd als tekst bewaard.
5. Typ de cel waar de tabel moet komen en klik op **Laden**. Als de tabel cellen zou overlappen die iets bevatten, vraagt JeffCrunch of die mogen worden vervangen. Als er geen ruimte is, of het blad is beveiligd, krijgt u een melding en verandert er niets.

De tabel is één stap van **Ongedaan maken**. Ze onthoudt het bestand en uw keuzes. Om het bestand opnieuw in te lezen, klikt u op een cel in de tabel en klikt u op **Vernieuwen** op het tabblad Gegevens. **Alles vernieuwen** is de grote knop ernaast: die leest elke tabel die met Gegevens ophalen is gemaakt en vernieuwt daarna elke draaitabel, allemaal als één stap van Ongedaan maken. De tabel groeit of krimpt mee met de rijen van het bestand. Kolommen die u naast de tabel hebt toegevoegd, blijven behouden: een formule die u naar beneden hebt doorgevoerd, wordt in nieuwe rijen ingevuld, en de rijen die uit het bestand zijn verdwenen, verdwijnen ook uit de tabel. Als het bestand weg is of niet gelezen kan worden, krijgt u een melding en verandert er niets. Als de kolommen van het bestand zelf zijn veranderd, krijgt u te horen welke kolommen ontbreken of nieuw zijn, en verandert er niets. Maak de tabel dan opnieuw met Gegevens ophalen, of zet de kolommen terug in het bestand.

De koppeling naar het bestand wordt in de tabel bewaard, in een opgeslagen `.xlsx`-werkmap, in een onderdeel dat andere programma's negeren. De koppeling bevat het volledige pad van het bestand op deze computer, dus iedereen die de werkmap krijgt, kan dat pad lezen. Om de koppeling te verwijderen voordat u de werkmap deelt, gebruikt u **Controleren**, daarna **Koppelingen naar gegevensbestanden**, en daarna **Verwijderen**. Een kopie die u in een ander bestandstype opslaat, bewaart de koppeling niet, en **Compatibiliteit controleren** meldt dat. **Controleren** toont de koppelingen onder **Koppelingen naar gegevensbestanden** en kan ze verwijderen. De tabellen en hun cellen blijven behouden.

Bestanden worden met grenzen ingelezen: een bestand tot 64 MB, 200.000 rijen, 256 kolommen en 32.767 tekens per cel. XML met een documenttype of entiteiten wordt geweigerd. Alles wat door een grens wordt afgekapt, wordt gemeld.

## Koppelingen bewerken

Als een werkmap formules bevat die een andere werkmap lezen (zoals ='[Prijzen.xlsx]Blad1'!A1), somt **Koppelingen bewerken** in de groep **Gegevens ophalen** die andere werkmappen op; die heten bronnen. Elke rij toont waar de bron staat en wat de status is. Totdat de werkmap opnieuw met zijn bronnen wordt geopend, toont die de getallen die het bestand uit die bronnen heeft opgeslagen.

- **Status controleren** zoekt elke bron op deze computer en meldt **Gevonden**, of dat de bron niet is gevonden. Een bron op het web wordt niet opgehaald; de status meldt dan dat er niet is gecontroleerd.
- **Waarden bijwerken** leest de gekozen bron van deze computer en geeft elke cel die alleen naar een van zijn cellen verwijst de waarde die die cel nu heeft. Een cel die meer met de waarde doet, zoals `=[1]Prijzen!A1*2`, behoudt het getal dat het bestand heeft opgeslagen, en de melding onder de lijst zegt hoeveel cellen zijn bijgewerkt en hoeveel niet. Waarden bijwerken is één stap, dus **Ongedaan maken** zet de oude waarden terug. De nieuwe waarden worden ook in het eigen onderdeel van de koppeling in het bestand geschreven bij het opslaan, zodat de werkmap ze de volgende keer dat ze wordt geopend toont, ook als de bron er niet is.
- **Bron wijzigen** laat u een ander bestand kiezen voor de gekozen koppeling, bijvoorbeeld wanneer de werkmap is verplaatst. De formules blijven zoals ze zijn. De nieuwe plaats wordt bij het opslaan in het bestand geschreven, en **Ongedaan maken** neemt de wijziging terug.
- **Bron openen** opent de gekozen bron in een nieuw venster, als die gevonden kan worden.
- **Koppeling verbreken** zet elke cel die de gekozen bron leest om in de waarde die die nu toont, en een naam die in die bron naar een plaats verwees, wordt **#REF!**. JeffCrunch vraagt eerst om bevestiging. Koppeling verbreken is één stap, dus **Ongedaan maken** brengt de formules, de namen en de koppelingen terug, totdat u de werkmap opslaat. Als er geen koppeling meer over is, wordt het bestand opgeslagen zonder de onderdelen die ze bevatten.

## Formulier

**Formulier** (in de groep **Gegevenshulpmiddelen** en in het zoekvak) toont één rij van een lijst tegelijk, als klein formulier. Een lijst is een tabel, of een celbereik met een rij namen bovenaan en onder elke naam een rij gegevens. Klik eerst op een willekeurige cel van de lijst.

- Elk vak is één kolom, met als label de naam uit de bovenste rij. Typ in een vak om de waarde te wijzigen. Een vak dat grijs is, bevat een formule: het formulier toont wat die oplevert, en u wijzigt de formule in het blad.
- **Volgende zoeken** en **Vorige zoeken** gaan naar de volgende en de vorige rij. Naar een andere rij gaan, of op **Enter** drukken, schrijft wat u in deze rij hebt getypt.
- **Nieuw** toont een leeg formulier. Vul het in en druk daarna opnieuw op **Nieuw**, op **Volgende zoeken** of op **Sluiten**. De rij wordt toegevoegd onder de laatste rij van de lijst. Een tabel groeit met één rij. Als een cel onder de lijst niet leeg is, meldt het formulier dat en voegt het niets toe.
- **Verwijderen** haalt de rij weg waarnaar u kijkt, nadat het daarom heeft gevraagd. De rijen eronder schuiven omhoog. Een tabel houdt altijd minstens één rij met gegevens. Een rij die verborgen is, of die door een filter wordt verborgen, wordt niet in het formulier getoond, niet meegeteld en nooit door het formulier verwijderd. Als de lijst te lang is om in zijn geheel te worden gevonden, meldt het formulier dat het alleen de eerste rijen toont.
- **Herstellen** neemt terug wat u in deze rij hebt getypt, voordat u de rij verlaat.
- **Criteria** laat u rijen zoeken. Typ een voorwaarde in een of meer vakken en druk daarna op **Volgende zoeken** of **Vorige zoeken**. Een voorwaarde is een waarde, zoals **Ann** (tekst die overeenkomt met het begin van het woord; **?** en ***** staan voor letters), of een waarde na een teken: **>100**, **<=5**, **<>0**, **=Oslo**. Een rij moet aan elke voorwaarde voldoen die u typt. Druk op **Formulier** om terug te gaan, of op **Wissen** om opnieuw te beginnen. Zolang er criteria zijn ingesteld, slaan **Volgende zoeken** en **Vorige zoeken** de rijen over die er niet aan voldoen.
- **Sluiten** schrijft de rij weg en sluit het formulier.

Elke rij die u wijzigt, toevoegt of verwijdert, is één stap, dus **Ongedaan maken** in het blad neemt die in één keer terug.

## Sorteren en filteren

De groep **Sorteren en filteren** heeft dezelfde opdrachten als het menu **Sorteren en rijen selecteren** op het tabblad Start. Zie [Sorteren en filteren](sort-filter).

## Tekst naar kolommen

Gebruik dit wanneer één kolom meerdere stukjes tekst bevat die u in aparte kolommen wilt, zoals "Smith, Ann, 31".

1. Selecteer de cellen van **één** kolom.
2. Kies **Tekst naar kolommen** op het tabblad **Gegevens**.
3. Kies hoe de tekst wordt gesplitst:
   - **Splitsen op een scheidingsteken**: vink de tekens aan die tussen de stukken staan: **Tab**, **Puntkomma**, **Komma**, **Spatie** of **Ander**, waar u uw eigen teken typt. **Opeenvolgende scheidingstekens als één behandelen** zorgt dat twee komma's achter elkaar geen leeg stuk maken. **Tekst tussen aanhalingstekens** noemt het teken dat een stuk omsluit dat niet mag worden gesplitst, zoals "Lee, Bo". Binnen de aanhalingstekens staan twee tekens achter elkaar voor één teken.
   - **Splitsen op vaste posities**: typ de posities waar een nieuwe kolom begint, waarbij tekens vanaf 1 worden geteld. Bijvoorbeeld: 5, 12 begint een nieuwe kolom bij het vijfde en bij het twaalfde teken.
4. Kies wat elk stuk is: **Algemeen** leest getallen en datums zoals u ze hebt getypt, en **Tekst** bewaart elk stuk zoals het is, zodat "007" "007" blijft.
   Om één stuk anders te behandelen, typt u het nummer ervan bij **Stuk** en kiest u hoe het wordt gelezen: **Zoals hierboven**, **Zoals getypt**, **Tekst**, een datum geschreven als **maand, dag, jaar**, **dag, maand, jaar** of **jaar, maand, dag**, of **Overslaan**, wat dat stuk nergens neerzet. Het eerste stuk is 1. Een datumstuk mag de maand als naam hebben (**5 mrt 2024**, **5 maart 2024**) in het Engels of in uw taal, mag gescheiden zijn door /, -, een punt, een komma of spaties, en mag worden gevolgd door een tijd (**10:30**, **10:30:45** of **10:30 PM**). Een stuk dat in de gekozen volgorde geen datum is (een maand 13, een dag die niet bestaat, een jaar dat geen twee of vier cijfers heeft, of een tijd die niet klopt), blijft als tekst bewaard, precies zoals het is getypt; het wordt nooit in een andere volgorde gelezen.
5. Typ bij **Stukken plaatsen vanaf** de eerste cel voor de stukken. Die begint als de eerste cel die u hebt geselecteerd.
6. Het vak onder de keuzes toont de eerste rijen gesplitst zoals u hebt gekozen. Klik op **OK**.

De stukken komen in de cellen rechts van de eerste cel, en vervangen wat er in die cellen stond. Tekst naar kolommen is één stap die u ongedaan kunt maken.

## Duplicaten verwijderen

Gebruik dit om van elke groep herhaalde rijen alleen de eerste te bewaren.

1. Selecteer de cellen van de lijst, of één cel erin. JeffCrunch neemt het hele gegevensblok rond één cel.
2. Kies **Duplicaten verwijderen**.
3. Vink **Mijn gegevens hebben een kopregel** aan als de eerste rij namen bevat. Die rij blijft staan en wordt niet vergeleken.
4. Vink de kolommen aan die moeten overeenkomen, wil twee rijen als herhaling gelden. Twee rijen zijn alleen herhalingen als elke aangevinkte kolom hetzelfde is. Hoofdletters en kleine letters tellen als gelijk, en een formule telt als de waarde die hij oplevert.
5. Klik op **OK**. Het dialoogvenster meldt hoeveel rijen zijn verwijderd en hoeveel er over zijn.

De rijen onder een verwijderde rij schuiven binnen de geselecteerde cellen omhoog, met hun opmaak en formules. De cellen onder de selectie verschuiven niet. De lege rijen die onderaan de selectie overblijven, worden leeggemaakt. Duplicaten verwijderen is één stap die u ongedaan kunt maken.

## Doelzoeken

Gebruik dit wanneer u het antwoord weet dat u wilt, maar niet het getal dat het oplevert. Een betalingscel bevat bijvoorbeeld een formule met een rentepercentage, en u wilt dat de betaling 500 is. Doelzoeken probeert getallen in de rentecel tot de betaling 500 is.

1. Kies **Doelzoeken** op het tabblad **Gegevens**.
2. Typ bij **Cel met de formule** de cel die de formule bevat, zoals B3. Die begint als de cel die u hebt geselecteerd.
3. Typ bij **Gewenste waarde** het antwoord dat de formule moet geven.
4. Typ bij **Cel om te wijzigen** de cel die Doelzoeken mag wijzigen, zoals B1. Die moet een getal bevatten, geen formule.
5. Klik op **OK**.

Als Doelzoeken een getal vindt, blijft dat in de cel om te wijzigen staan, en de statusbalk zegt welk getal het is. Als het er geen vindt, verandert er niets en krijgt u een melding. Probeer een ander startgetal in de cel om te wijzigen, en controleer of de formule die cel echt gebruikt. Doelzoeken is één stap die u ongedaan kunt maken.

## Scenariobeheer

Een scenario is een benoemde reeks waarden voor sommige cellen. Gebruik scenario's om "wat als" op één blad te proberen. Bewaar bijvoorbeeld een scenario "Lage verkoop" en een scenario "Hoge verkoop" voor dezelfde invoercellen, en toon het scenario dat u wilt bekijken.

1. Kies **Scenariobeheer** op het tabblad **Gegevens**.
2. Klik op **Toevoegen**. Typ een naam, zoals Hoge verkoop. Typ bij **Te wijzigen cellen** de cellen die het scenario instelt, zoals B1:B3 of D5. Een scenario kan maximaal 32 cellen wijzigen. De cellen die u hebt geselecteerd, worden voor u ingevuld. Elke cel begint met de waarde die hij nu heeft. Typ in de tabel de waarde die dit scenario moet gebruiken, en klik op **OK**.
3. Kies een scenario in de lijst en klik op **Weergeven** om de waarden in de cellen te zetten. Dubbelklikken op een naam doet hetzelfde. Klik op **Bewerken** om de naam, de opmerking of de waarden te wijzigen. Klik op **Verwijderen** om het weg te halen.

Weergeven, Toevoegen, Bewerken en Verwijderen zijn elk één stap die u ongedaan kunt maken.

**Samenvatting** zet alle scenario's naast elkaar op een nieuw blad met de naam Scenariosamenvatting. Het vraagt om de resultaatcellen, dat zijn de cellen die u wilt vergelijken, zoals het totaal in B10. Voor elk scenario toont de samenvatting de waarden van de wijzigende cellen en wat de resultaatcellen opleveren, berekend met de waarden van dat scenario. Een kolom met de naam Huidige waarden toont hoe de cellen waren toen u de samenvatting maakte. Uw eigen cellen blijven zoals ze waren.

De scenario's worden in de werkmap opgeslagen. Een werkmap die in een ander programma is gemaakt, behoudt zijn scenario's, en u ziet ze hier. Een kopie die u opslaat als een oudere .xls-bestand of als .ods, bewaart ze niet, en het opslaan meldt dat.

## Subtotaal

Gebruik dit om onder elke groep rijen met dezelfde waarde in één kolom een rij met totalen te zetten, en onderaan een eindtotaal. De lijst heeft een kopregel nodig en moet worden gesorteerd op de kolom waarop u groepeert, zodat gelijke waarden naast elkaar staan.

1. Selecteer een cel in de lijst.
2. Kies **Subtotaal** in de groep **Overzicht** op het tabblad **Gegevens**.
3. Kies bij **Bij elke wijziging in** de kolom die de rijen groepeert, zoals Regio.
4. Kies bij **Functie gebruiken** wat u wilt berekenen: **Som**, **Aantal**, **Gemiddelde**, **Max**, **Min** en andere.
5. Vink bij **Subtotaal toevoegen aan** elke kolom aan die u wilt optellen.
6. Laat **Huidige subtotalen vervangen** aangevinkt om subtotalen die er al zijn te vervangen.
7. **Samenvatting onder gegevens** staat zoals het overzicht van het blad die heeft. Laat het aangevinkt om elke totaalrij onder zijn groep te zetten, of vink het uit om elke totaalrij boven zijn groep te zetten, met het eindtotaal bovenaan. Klik op **OK**.

Elke totaalrij heet "Oost-totaal" (of de waarde waarvoor hij is) en bevat een SUBTOTAAL-formule, zodat hij de andere totaalrijen niet meetelt. De rijen van elke groep zijn gegroepeerd, zodat u met de knoppen 1, 2 en 3 naast de rijen alleen de totalen kunt tonen. Om de totalen weg te halen, opent u **Subtotaal** opnieuw en klikt u op **Alles verwijderen**. Als de totalen naar de andere kant van een blad zijn gegaan dat al eigen groepen had, meldt de statusbalk dat die groepen nu hun samenvattingsrijen aan die kant hebben, en zet Alles verwijderen de kant terug zoals die was vóór het eerste subtotaal, tenzij u die sinds die tijd zelf hebt gewijzigd. Wanneer de totalen boven staan, zitten de overzichtsknoppen bij de totaalrijen, zodat een plus- of minknop naast "Oost-totaal" de Oost-rijen eronder toont of verbergt. Als u **Huidige subtotalen vervangen** uitschakelt, moeten de nieuwe totalen aan dezelfde kant komen als de bestaande. Anders wordt er niets gewijzigd en krijgt u een melding. Subtotalen toevoegen of verwijderen is één stap die u ongedaan kunt maken.

## Geavanceerd filter

De filterknoppen kiezen rijen per keer op één kolom. **Geavanceerd** (in de groep **Sorteren en filteren**) laat u de voorwaarden op het blad schrijven, zodat ze zo lang kunnen zijn als nodig.

1. Kopieer naast de lijst de koppen van de kolommen die u wilt testen naar een rij lege cellen.
2. Schrijf onder elke kop een voorwaarde. Voorwaarden in **dezelfde rij** moeten allemaal waar zijn (dit is "en"). Voorwaarden in **verschillende rijen** zijn alternatieven: een rij van de lijst hoeft aan maar één van die rijen te voldoen (dit is "of").
3. Kies **Geavanceerd**. Controleer of **Lijstbereik** de lijst met zijn kopregel is, en typ de cellen van uw koppen en voorwaarden bij **Voorwaardenbereik**.
4. Kies **Andere rijen in de lijst verbergen**, of **Rijen naar een andere plaats kopiëren** en typ de eerste cel van de plaats, zoals H1. Vink **Rijen overslaan die een eerdere rij herhalen** aan om alleen één van elke rij te kopiëren.
5. Klik op **OK**.

Zo schrijft u een voorwaarde:

- Een woord, zoals **An**, kiest tekst die ermee begint. Hoofdletters en kleine letters tellen als gelijk. **?** staat voor elk willekeurig teken en ***** voor elke reeks tekens.
- Een getal kiest cellen die er gelijk aan zijn.
- **=Ann** kiest tekst die precies Ann is. **=** alleen kiest lege cellen.
- **>**, **>=**, **<**, **<=** en **<>** (niet gelijk) voor een getal of woord vergelijken ermee, zoals **>25**. **<>** alleen kiest cellen die niet leeg zijn.

Een rij voorwaarden zonder inhoud kiest elke rij. Om de verborgen rijen weer te tonen, kiest u **Wissen**. Een kopie wordt meteen gemaakt en verandert niet mee als de lijst verandert. Geavanceerd filter is één stap die u ongedaan kunt maken.

## Gegevensvalidatie

Gebruik dit om vast te leggen wat in sommige cellen mag worden getypt, zodat een verkeerde invoer wordt tegengehouden of in twijfel getrokken.

1. Selecteer de cellen.
2. Kies **Gegevensvalidatie** in de groep **Gegevenshulpmiddelen** op het tabblad **Gegevens**.
3. Kies op het tabblad **Instellingen** wat u wilt **Toestaan**: **Elke waarde** (geen regel), **Geheel getal**, **Decimaal getal**, **Lijst**, **Datum**, **Tijd**, **Tekstlengte** of **Aangepast**.
4. Kies bij een getal, datum, tijd of tekstlengte bij **Gegevens** (**tussen**, **niet tussen**, **gelijk aan**, enzovoort) en typ de grenzen. Een grens kan een waarde zijn of een formule die met = begint.
5. Bij een **Lijst** typt u de keuzes met komma's ertussen, zoals Rood,Groen,Blauw, of typt u na = een celbereik, zoals =$A$1:$A$5. De cel toont dan een pijltje met de keuzes. Schakel **Vervolgkeuzelijst in cel** uit om het pijltje te verbergen.
6. Bij **Aangepast** typt u een formule die met = begint. De invoer is toegestaan wanneer de formule waar is.
7. Met **Lege cellen negeren** kan een lege cel worden overgeslagen.

Op het tabblad **Invoerbericht** vinkt u het vakje aan en schrijft u een titel en een bericht. Ze verschijnen naast de cel zodra die is geselecteerd.

Op het tabblad **Foutmelding** kiest u wat er gebeurt met een invoer die niet is toegestaan. **Stop** weigert die. **Waarschuwing** vraagt of die moet worden behouden. **Informatie** laat de gebruiker het weten en behoudt de invoer. Schrijf een titel en een bericht voor de melding.

**Ongeldige gegevens cirkelen** (naast **Gegevensvalidatie**) zet een rode ovaal rond elke cel op het blad waarvan de waarde niet is toegestaan door de regel, zoals een getal dat er al stond vóór de regel, of het resultaat van een formule. Een bericht laat weten als alle cellen in orde zijn. Een cel die u corrigeert, verliest zijn ovaal. **Validatiemarkeringen wissen** haalt alle ovalen weg.

**Alles wissen** haalt de regels van de geselecteerde cellen af. De nieuwe regel vervangt elke regel die de geselecteerde cellen hadden; andere cellen behouden de hunne. Regels wijzigen is één stap die u ongedaan kunt maken, en de regels worden in het bestand opgeslagen.

## Consolideren

Gebruik dit om bereiken die op elkaar lijken, zoals dezelfde tabel uit meerdere bladen, samen te voegen tot één tabel.

1. Kies **Consolideren** in de groep **Gegevenshulpmiddelen** op het tabblad **Gegevens**.
2. Kies de **Functie** waarmee u wilt combineren, zoals **Som** of **Gemiddelde**.
3. Typ een bereik in het vak en klik op **Toevoegen**. Gebruik **Blad2!A1:C10** voor een bereik op een ander blad. Voeg elk bereik toe dat u wilt combineren. Selecteer een bereik in de lijst en klik op **Verwijderen** om het weg te halen.
4. Als de bereiken in hun eerste rij namen hebben, vinkt u **Bovenste rij bevat de namen van kolommen** aan. Als ze namen in hun eerste kolom hebben, vinkt u **Linker kolom bevat de namen van rijen** aan.
5. Typ bij **Resultaat plaatsen op** de eerste cel voor het resultaat.
6. Vink **Koppelingen naar de brongegevens maken** aan als elk resultaat een formule moet zijn die de bronnen leest, zodat het verandert wanneer zij veranderen. Zonder dat zijn de resultaten gewone getallen.
7. Klik op **OK**.

Met geen namen aangevinkt wordt de cel op dezelfde plaats in elk bereik gecombineerd (de eerste cel van elk bereik, dan de tweede, enzovoort). Met namen aangevinkt worden cellen met dezelfde namen gecombineerd, waar ze ook in elk bereik staan, en een naam die alleen sommige bereiken hebben, krijgt toch een rij of kolom. Consolideren is één stap die u ongedaan kunt maken.

## Prognoseblad

Prognoseblad maakt een nieuw blad met een tabel en een grafiek van toekomstige waarden, berekend uit een tijdlijn en een reeks waarden. Het vindt de trend en het zich herhalende seizoen van de waarden, en voegt een band toe waarbinnen de toekomstige waarden waarschijnlijk blijven.

1. Selecteer de tijdlijn en de waarden, twee kolommen naast elkaar (eerst de tijdlijn). De tijdlijn bestaat uit datums of getallen die even ver uit elkaar liggen, zoals elke maand of elke 5 eenheden. Een eerste rij met namen wordt overgeslagen.
2. Kies **Prognoseblad** in de groep **Prognose** op het tabblad **Gegevens**.
3. Een afbeelding toont de prognose, en verandert wanneer u de keuzes wijzigt. Kies **Lijndiagram** of **Kolomdiagram**.
4. Stel de keuzes in:
   - **Tijdlijnbereik** en **Waardenbereik** zijn de cellen die worden gelezen. Typ hier andere cellen als de selectie verkeerd was.
   - **Begin prognose** is leeg om na het laatste punt te beginnen. Een datum of getal vóór het laatste punt begint daar en gebruikt alleen de punten ervoor, zodat u de prognose kunt vergelijken met wat er werkelijk gebeurde.
   - **Einde prognose** is het laatste tijdstip waarvoor u een prognose maakt. Er worden maximaal 10.000 punten voorspeld.
   - **Betrouwbaarheidsinterval** is hoe zeker de band is, eerst 95%.
   - **Seizoensinvloed** wordt bepaald door naar de waarden te kijken, of u stelt de lengte van het seizoen in punten in, of u kiest geen. De waarden moeten twee volledige seizoenen bevatten.
   - **Ontbrekende punten aanvullen met** bepaalt wat een tijdstip zonder waarde krijgt: een rechte lijn tussen de buren, of nul.
   - **Dubbele waarden samenvoegen met** bepaalt hoe waarden op hetzelfde tijdstip worden gecombineerd: Gemiddelde, Aantal, AantalA, Max, Mediaan, Min of Som.
   - **Prognosestatistieken opnemen** voegt een tabel toe met Alfa, Bèta, Gamma, MASE, SMAPE, MAE en RMSE, die laten zien hoe goed de prognose bij de waarden past.
5. Als er iets niet klopt, meldt het dialoogvenster in gewone woorden wat (stappen die niet gelijk zijn, te weinig punten, tekst in de waarden), en **Maken** wacht.
6. Klik op **Maken**.

Het nieuwe blad wordt vóór het blad geplaatst waarop u stond. De tabel heeft de kolommen Tijdlijn, Waarden, Prognose, Ondergrens betrouwbaarheidsinterval en Bovengrens betrouwbaarheidsinterval. De prognose en de grenzen zijn formules die `FORECAST.ETS` en `FORECAST.ETS.CONFINT` gebruiken op de eigen kolommen Tijdlijn en Waarden van de tabel, dus ze worden opnieuw berekend wanneer u een waarde wijzigt. De grafiek toont de waarden, de prognose en de band. Prognoseblad is één stap die u ongedaan kunt maken.

De getallen kunnen enkele procenten afwijken van die van andere spreadsheetprogramma's, die hun eigen manier gebruiken om de afvlakking te passen. Een prognose die al in een bestand staat, behoudt de waarden die het bestand heeft opgeslagen, totdat de gegevens die het leest, worden gewijzigd.

Een bestand met meer dan 1.000.000 punten wordt niet gebruikt.
