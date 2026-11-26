# Gegevenshulpmiddelen
<!-- keywords: prognoseblad, prognose, toekomstige waarden, betrouwbaarheidsinterval, seasonality, seizoensinvloed, trend, voortschrijdend gemiddelde, formulier, gegevensformulier, gegevensinvoer, record, records, nieuw record, volgend zoeken, vorige zoeken, criteria, scenariobeheer, scenario's, wat-als, wat als, scenariosamenvatting, gegevens, tabblad gegevens, tekst naar kolommen, splitsen, scheidingsteken, komma, tab, vaste breedte, dubbele waarden verwijderen, doelzoeken, subtotaal, totalen, geavanceerd filter, voorwaarden, gegevensvalidatie, ongeldige gegevens cirkelen, rode ovaal, validatiemarkeringen wissen, consolideren, bereiken combineren, toestaan, lijst, keuzelijst, invoerbericht, foutmelding, herhaalde rijen, uniek, tekstbestand, csv, uit tekst, kolommen, scheidingsteken, aanhalingstekens, koppelingen bewerken, waarden bijwerken, bron wijzigen, koppelingen, extern, bron, koppeling verbreken, status controleren, bron openen -->

Het tabblad **Gegevens** heeft de hulpmiddelen die met een lijst van waarden werken: tekst uit een bestand halen, een lijst sorteren, tekst in kolommen splitsen en herhaalde rijen verwijderen.

## Uit tekst/CSV

**Uit tekst/CSV** opent een tekstbestand (.csv, .tsv of .txt). Het is hetzelfde als **Tekstbestand openen** op het tabblad Bestand. JeffCrunch vraagt hoe het bestand in kolommen is verdeeld en wat elke kolom bevat, en toont het daarna als een nieuwe werkmap. Hetzelfde venster verschijnt wanneer u een tekstbestand opent of **Tekstimport plakken** gebruikt.

Het venster raadt wat elke kolom bevat op basis van de eerste 200 rijen van het bestand, en toont die raadzaam in het vak boven de kolom. Wijzig elke kolom die u wilt aanpassen voordat u het bestand opent. Een soort die u wijzigt, wordt nooit vervangen door een latere raadzaam, ook niet als u daarna het scheidingsteken wijzigt. Een raadzaam doet bij een kolom maar drie dingen: ze laat de kolom **Zoals getypt**, wat de manier is waarop een bestand wordt gelezen wanneer niets wordt geraden; ze maakt de kolom **Tekst** om die te beschermen; of ze maakt de kolom datums in één volgorde die de waarden bewijzen. Dit zijn de regels:

- Een kolom blijft **Zoals getypt** wanneer Zoals getypt de waarden goed leest: getallen, waar en onwaar, tijden, datums met maandnamen, getallen met teken zoals -5 en +3, woorden en versienummers zoals 1.13.2. Wijzigen zou ze alleen slechter maken.
- Een kolom is een **datum** alleen wanneer elke waarde een jaartal met vier cijfers heeft (of een volledige datum is, zoals 2024-05-13) en in één volgorde en geen andere als datum leest (maand, dag, jaar; dag, maand, jaar; of jaar, maand, dag). De datums worden getoond zoals u datums typt. Wanneer meer dan één volgorde op elke waarde past, zoals 05/06/2024, blijft de kolom **Tekst**, staat ze in het venster vermeld, en kiest u de volgorde in het vak.
- Waarden met drie getallen en geen jaartal met vier cijfers, zoals 1.13.2 of 13.05.24, worden door de raadzaam nooit tot datums gemaakt. Als Zoals getypt ze als tekst zou houden, blijft de kolom Zoals getypt; als Zoals getypt ze tot datums zou maken, is de kolom **Tekst**. In beide gevallen staat de kolom in het venster vermeld als mogelijke datums met tweecijferige jaartallen, zodat u een volgorde kunt kiezen.
- Een kolom is **Tekst** wanneer een waarde een nul voor meer cijfers heeft of meer dan 15 cijfers heeft, wanneer een pluteken voor zeven of meer cijfers staat (een telefoonnummer), of wanneer een exponent zoals 1E5 naast andere waarden staat.
- Een waarde die met een gelijkteken begint, wordt overgelaten aan het vak **Formules lezen**. Als het vak is aangevinkt, blijft de kolom Zoals getypt zodat de waarde als formule wordt gelezen. Als het vak niet is aangevinkt, is de kolom **Tekst**.
- Vink **De eerste rij bevat kolomnamen** aan, dan wordt de eerste rij niet voor de raadzaam gebruikt. Het venster vinkt dit aan wanneer de eerste rij tekst boven getallen of datums is.
- De kolommen na de eerste 100 worden als tekst ingelezen.

Alleen de eerste 200 rijen worden bekeken, dus een waarde verderop kan een verrassing zijn. In een datumkolom komt een waarde verderop die geen datum is in de gekozen volgorde als tekst binnen, nooit als een verkeerde datum. In een kolom Tekst wordt elke waarde ingelezen zoals ze is geschreven. In een kolom Zoals getypt wordt een waarde verderop gelezen zoals Zoals getypt die overal anders leest.

**Tekstimport plakken** voegt de tekst in alsof u die had getypt, met uw eigen decimaal- en duizendtalteken, dus het venster heeft voor die tekens geen vakken, en de raadzaam leest de tekst op dezelfde manier als het plakken. Het venster voor een bestand heeft die vakken wel.

## Uit bestand

**Uit bestand** in de groep **Gegevens ophalen** laadt de rijen van een JSON-bestand, een XML-bestand of een andere werkmap als tabel. Alleen een bestand op deze computer kan worden geladen. Een webadres of een bestand op een andere computer wordt geweigerd.

1. Klik op **Uit bestand**, dan op **Bladeren** en kies het bestand. De eerste rijen worden meteen getoond, met de soort die JeffCrunch voor elke kolom uit die eerste rijen vond: getal, datum, waar of onwaar, of tekst. Het hele bestand wordt bij het laden gecontroleerd, dus een kolom kan alsnog tekst worden. Op het blad verandert niets totdat u op **Laden** klikt.
2. Wanneer het bestand meer dan één lijst bevat, kiest u de lijst die u wilt laden. In een JSON-bestand is dat een lijst met items met benoemde waarden, zoals `$.items`. In een XML-bestand is dat een item dat zich herhaalt, zoals `/feed/entry`. In een werkmap is dat een blad, een tabel of een benoemd bereik.
3. Een JSON-item binnen een item wordt een kolom met de naam `buiten.binnen`, tot vier niveaus diep. Een lijst met losse waarden wordt samengevoegd in één cel (de eerste 10.000 waarden), of over kolommen verdeeld (de eerste 16), naar uw keuze. In XML worden de kenmerken en de onderliggende items kolommen. Twee verschillende waarden delen nooit een kolom: wanneer twee dezelfde naam zouden krijgen, krijgt de tweede een getal erachter of, in XML, zijn voorvoegsel, en een opmerking zegt dat. Een naam die binnen één JSON-item herhaald wordt, behoudt de laatste waarde, en een opmerking zegt dat. Een getal dat te groot of te klein is voor een cel, blijft als tekst zoals geschreven, en een teken dat niet kan worden gelezen, wordt getoond als vervangingsteken; beide worden genoteerd.
4. Vink **Alle kolommen als tekst laden** aan om elke waarde te behouden zoals ze is geschreven. Een kolom met getallen met een voorloopnul, zoals 007, blijft altijd als tekst.
5. Typ de cel waar de tabel moet komen en klik op **Laden**. Als de tabel cellen zou bedekken die iets bevatten, vraagt JeffCrunch of u die wilt vervangen. Als er geen ruimte is, of het blad is beveiligd, krijgt u een melding en verandert er niets.

De tabel is één stap van **Ongedaan maken**. Hij onthoudt het bestand en uw keuzes. Om het bestand opnieuw in te lezen, klikt u in de tabel op een cel en klikt u op **Vernieuwen** op het tabblad Gegevens. **Alles vernieuwen** is de grote knop ernaast: die leest elke tabel die met Gegevens ophalen is gemaakt en vernieuwt daarna elke draaitabel, allemaal als één stap van Ongedaan maken. De tabel groeit of krimpt mee met de rijen van het bestand. Kolommen die u naast de tabel hebt toegevoegd, blijven staan: een formule die u omlaag hebt doorgevoerd, wordt in nieuwe rijen doorgevoerd, en de rijen die uit het bestand zijn verdwenen, gaan ook uit de tabel weg. Als het bestand weg is of niet gelezen kan worden, krijgt u een melding en verandert er niets. Als de kolommen van het bestand zelf zijn gewijzigd, wordt u verteld welke ontbreken of nieuw zijn, en verandert er niets. Maak de tabel opnieuw met Gegevens ophalen, of zet de kolommen terug in het bestand.

De koppeling naar het bestand wordt in de tabel bewaard in een opgeslagen `.xlsx`-werkmap, in een onderdeel dat andere programma's negeren. De koppeling bevat het volledige pad van het bestand op deze computer, dus iemand die de werkmap krijgt, kan dat pad lezen. Om het te verwijderen voordat u de werkmap deelt, gebruikt u **Controleren**, dan **Koppelingen naar gegevensbestanden**, en daarna **Verwijderen**. Een kopie die als ander bestandstype wordt opgeslagen, bewaart de koppeling niet, en **Compatibiliteit controleren** meldt dat. **Controleren** toont de koppelingen onder **Koppelingen naar gegevensbestanden** en kan ze verwijderen. De tabellen en hun cellen blijven staan.

Bestanden worden met grenzen ingelezen: een bestand tot 64 MB, 200.000 rijen, 256 kolommen en 32.767 tekens in een cel. XML met een documenttype of entiteiten wordt geweigerd. Alles wat door een grens wordt afgekapt, wordt vermeld.

## Koppelingen bewerken

Als een werkmap formules heeft die een andere werkmap lezen (zoals ='[Prijzen.xlsx]Blad1'!A1), toont **Koppelingen bewerken** in de groep **Gegevens ophalen** die andere werkmappen, die bronnen worden genoemd. Elke rij toont waar de bron is en de status ervan. Totdat de werkmap opnieuw met zijn bronnen wordt geopend, toont die de getallen die het bestand van die bronnen heeft opgeslagen.

- **Status controleren** zoekt elke bron op deze computer en meldt **Gevonden**, of dat de bron niet is gevonden. Een bron op het web wordt niet opgehaald, dus de status zegt dat die niet is gecontroleerd.
- **Waarden bijwerken** leest de gekozen bron van deze computer en geeft elke cel die alleen een verwijzing naar een van zijn cellen is, de waarde die die cel nu heeft. Een cel die meer met de waarde doet, zoals `=[1]Prijzen!A1*2`, behoudt het getal dat het bestand had opgeslagen, en de melding onder de lijst zegt hoeveel cellen zijn bijgewerkt en hoeveel niet. Waarden bijwerken is één stap, dus **Ongedaan maken** zet de oude waarden terug. De nieuwe waarden worden ook in het eigen onderdeel van de koppeling in het bestand geschreven wanneer u opslaat, zodat de werkmap ze de volgende keer dat hij wordt geopend toont, zelfs als de bron er niet is.
- **Bron wijzigen** laat u een ander bestand kiezen voor de gekozen koppeling, bijvoorbeeld wanneer de werkmap is verplaatst. De formules blijven zoals ze zijn. De nieuwe plaats wordt bij het opslaan in het bestand geschreven, en **Ongedaan maken** neemt de wijziging terug.
- **Bron openen** opent de gekozen bron in een nieuw venster, wanneer die kan worden gevonden.
- **Koppeling verbreken** zet elke cel die de gekozen bron leest om in de waarde die die nu toont, en een naam die voor een plaats in die bron stond, wordt **#REF!**. JeffCrunch vraagt eerst. Koppeling verbreken is één stap, dus **Ongedaan maken** brengt de formules, de namen en de koppelingen terug, tot u de werkmap opslaat. Wanneer er geen koppeling meer over is, wordt het bestand opgeslagen zonder de onderdelen die ze bevatten.

## Formulier

**Formulier** (in de groep **Gegevenshulpmiddelen**, en in het zoekvak) toont één rij van een lijst tegelijk, als een klein formulier. Een lijst is een tabel, of een celbereik met een rij namen bovenaan en een rij gegevens onder elke naam. Klik eerst op een willekeurige cel van de lijst.

- Elk vak is één kolom, met als label de naam uit de bovenste rij. Typ in een vak om de waarde te wijzigen. Een vak dat grijs is, bevat een formule: het formulier toont wat die oplevert, en u wijzigt de formule op het blad.
- **Volgend zoeken** en **Vorig zoeken** gaan naar de volgende en de vorige rij. Naar een andere rij gaan, of op **Enter** drukken, schrijft wat u in deze rij hebt getypt.
- **Nieuw** toont een leeg formulier. Vul het in en druk daarna opnieuw op **Nieuw**, op **Volgend zoeken** of op **Sluiten**. De rij wordt toegevoegd onder de laatste rij van de lijst. Een tabel groeit met één rij. Als een cel onder de lijst niet leeg is, meldt het formulier dat en voegt het niets toe.
- **Verwijderen** haalt de rij weg waar u naar kijkt, nadat het vraagt of u dat wilt. De rijen eronder schuiven omhoog. Een tabel houdt altijd minstens één rij met gegevens. Een rij die verborgen is, of die door een filter wordt verborgen, wordt niet in het formulier getoond, niet meegeteld en nooit door het formulier verwijderd. Als de lijst te lang is om in zijn geheel te worden gevonden, meldt het formulier dat het alleen de eerste rijen toont.
- **Herstellen** neemt terug wat u in deze rij hebt getypt, voordat u die verlaat.
- **Criteria** laat u naar rijen zoeken. Typ een voorwaarde in een of meer vakken en druk daarna op **Volgend zoeken** of **Vorig zoeken**. Een voorwaarde is een waarde, zoals **Ann** (tekst die begint met het woord, en **?** en ***** staan voor letters), of een waarde na een teken: **>100**, **<=5**, **<>0**, **=Oslo**. Een rij moet aan elke voorwaarde voldoen die u typt. Druk op **Formulier** om terug te gaan, of op **Wissen** om opnieuw te beginnen. Terwijl criteria zijn ingesteld, slaan **Volgend zoeken** en **Vorig zoeken** de rijen over die er niet aan voldoen.
- **Sluiten** schrijft de rij weg en sluit het formulier.

Elke rij die u wijzigt, toevoegt of verwijdert, is één stap, dus **Ongedaan maken** op het blad neemt die in één keer terug.

## Sorteren en filteren

De groep **Sorteren en filteren** heeft dezelfde opdrachten als het menu **Sorteren en rijen selecteren** op het tabblad Start. Zie [Sorteren en filteren](sort-filter).

## Tekst naar kolommen

Gebruik dit wanneer één kolom meerdere stukken tekst bevat die u in aparte kolommen wilt hebben, zoals "Smith, Ann, 31".

1. Selecteer de cellen van **één** kolom.
2. Kies **Tekst naar kolommen** op het tabblad **Gegevens**.
3. Kies hoe de tekst wordt gesplitst:
   - **Splitsen op een scheidingsteken**: vink de tekens aan die tussen de stukken staan: **Tab**, **Puntkomma**, **Komma**, **Spatie**, of **Andere**, waar u uw eigen teken typt. **Scheidingstekens achter elkaar als één behandelen** zorgt dat twee komma's achter elkaar geen leeg stuk maken. **Tekst tussen aanhalingstekens** noemt het teken dat een stuk omsluit dat niet mag worden gesplitst, zoals "Lee, Bo". Binnen de aanhalingstekens staan twee tekens achter elkaar voor één teken.
   - **Splitsen op vaste posities**: typ de posities waar een nieuwe kolom begint, waarbij tekens vanaf 1 worden geteld. Bijvoorbeeld 5, 12 begint een nieuwe kolom bij het vijfde en bij het twaalfde teken.
4. Kies wat elk stuk is: **Algemeen** leest getallen en datums zoals u ze typt, en **Tekst** behoudt elk stuk zoals het is, zodat "007" "007" blijft.
   Om één stuk anders te behandelen, typt u het nummer ervan bij **Stuk** en kiest u hoe het wordt gelezen: **Zoals hierboven**, **Zoals getypt**, **Tekst**, een datum geschreven als **maand, dag, jaar**, **dag, maand, jaar**, of **jaar, maand, dag**, of **Weglaten**, wat dat stuk nergens plaatst. Het eerste stuk is 1. Een datumstuk mag de maand als naam hebben (**5 mrt 2024**, **5 maart 2024**), in het Engels of in uw taal, mag gescheiden zijn door /, -, een punt, een komma of spaties, en mag worden gevolgd door een tijd (**10:30**, **10:30:45**, of **10:30 PM**). Een stuk dat geen datum is in de volgorde die u koos (een maand 13, een dag die niet bestaat, een jaar dat geen twee of vier cijfers is, of een tijd die niet klopt), blijft als tekst staan zoals het is getypt; het wordt nooit in een andere volgorde gelezen.
5. Typ bij **De stukjes plaatsen vanaf** de eerste cel voor de stukjes. Die begint bij de eerste cel die u hebt geselecteerd.
6. Het vak onder de keuzes toont de eerste rijen gesplitst zoals u hebt gekozen. Druk op **OK**.

De stukjes gaan in de cellen rechts van de eerste cel, en ze vervangen wat er in die cellen stond. Tekst naar kolommen is één stap die u ongedaan kunt maken.

## Dubbele waarden verwijderen

Gebruik dit om alleen de eerste van elke groep herhaalde rijen te behouden.

1. Selecteer de cellen van de lijst, of één cel erin. JeffCrunch neemt het hele blok met gegevens rond één enkele cel.
2. Kies **Dubbele waarden verwijderen**.
3. Vink **Mijn gegevens hebben een kopregel** aan als de eerste rij namen bevat. Die rij blijft staan en wordt niet vergeleken.
4. Vink de kolommen aan die moeten overeenkomen, wil twee rijen als herhaling gelden. Twee rijen zijn alleen herhalingen wanneer elke aangevinkte kolom gelijk is. Hoofd- en kleine letters tellen als gelijk, en een formule telt als de waarde die die oplevert.
5. Druk op **OK**. Het dialoogvenster meldt hoeveel rijen zijn verwijderd en hoeveel er over zijn.

De rijen onder een verwijderde rij schuiven omhoog binnen de geselecteerde cellen, met hun opmaak en formules. De cellen onder de selectie verschuiven niet. De lege rijen die onderaan de selectie overblijven, worden gewist. Dubbele waarden verwijderen is één stap die u ongedaan kunt maken.

## Doelzoeken

Gebruik dit wanneer u het antwoord kent dat u wilt, maar niet het getal dat het oplevert. Bijvoorbeeld: een betalingscel bevat een formule die een rentevoet gebruikt, en u wilt dat de betaling 500 is. Doelzoeken probeert getallen in de rentecel tot de betaling 500 is.

1. Kies **Doelzoeken** op het tabblad **Gegevens**.
2. Typ bij **Cel met de formule** de cel die de formule bevat, zoals B3. Die begint als de cel die u hebt geselecteerd.
3. Typ bij **Gewenste waarde** het antwoord dat de formule moet geven.
4. Typ bij **Te wijzigen cel** de cel die Doelzoeken mag wijzigen, zoals B1. Die moet een getal bevatten, geen formule.
5. Druk op **OK**.

Als Doelzoeken een getal vindt, blijft het in de te wijzigen cel staan, en de statusbalk zegt welk getal het is. Als het er geen vindt, verandert er niets en krijgt u een melding. Probeer een ander startgetal in de te wijzigen cel, en controleer of de formule die cel echt gebruikt. Doelzoeken is één stap die u ongedaan kunt maken.

## Scenariobeheer

Een scenario is een benoemde set waarden voor een aantal cellen. Gebruik scenario's om "wat als" op één blad uit te proberen. Bijvoorbeeld: houd een scenario "Lage verkoop" en een scenario "Hoge verkoop" voor dezelfde invoercellen, en toon het scenario dat u wilt bekijken.

1. Kies **Scenariobeheer** op het tabblad **Gegevens**.
2. Druk op **Toevoegen**. Typ een naam, zoals Hoge verkoop. Typ bij **Te wijzigen cellen** de cellen die het scenario instelt, zoals B1:B3 of D5. Een scenario kan maximaal 32 cellen wijzigen. De cellen die u hebt geselecteerd, worden voor u ingevuld. Elke cel begint met de waarde die hij nu heeft. Typ in de tabel de waarde die dit scenario moet gebruiken, en druk op **OK**.
3. Kies een scenario in de lijst en druk op **Tonen** om de waarden in de cellen te zetten. Dubbelklikken op een naam doet hetzelfde. Druk op **Bewerken** om de naam, de opmerking of de waarden te wijzigen. Druk op **Verwijderen** om het weg te halen.

Tonen, Toevoegen, Bewerken en Verwijderen zijn elk één stap die u ongedaan kunt maken.

**Samenvatting** zet alle scenario's naast elkaar op een nieuw blad met de naam Scenariosamenvatting. Het vraagt om de resultaatcellen, dat zijn de cellen die u wilt vergelijken, zoals het totaal in B10. Voor elk scenario toont de samenvatting de waarden van de te wijzigen cellen en wat de resultaatcellen opleveren, berekend met de waarden van dat scenario. Een kolom Huidige waarden toont hoe de cellen waren toen u de samenvatting maakte. Uw eigen cellen blijven zoals ze waren.

De scenario's worden in de werkmap opgeslagen. Een werkmap die met een ander programma is gemaakt, behoudt zijn scenario's, en u ziet ze hier. Een kopie die als oudere .xls-file of als .ods wordt opgeslagen, bewaart ze niet, en het opslaan meldt dat.

## Subtotaal

Gebruik dit om onder elke groep rijen die dezelfde waarde in één kolom hebben een rij met totalen te zetten, en onderaan een eindtotaal. De lijst heeft een kopregel nodig, en moet gesorteerd zijn op de kolom waarop u groepeert, zodat gelijke waarden naast elkaar staan.

1. Selecteer een cel in de lijst.
2. Kies **Subtotaal** in de groep **Overzicht** op het tabblad **Gegevens**.
3. Kies bij **Bij elke wijziging in** de kolom die de rijen groepeert, zoals Regio.
4. Kies bij **Functie gebruiken** wat u wilt berekenen: **Som**, **Aantal**, **Gemiddelde**, **Max**, **Min**, en andere.
5. Vink bij **Subtotaal toevoegen aan** elke kolom aan die u wilt optellen.
6. Laat **Huidige subtotalen vervangen** aangevinkt om subtotalen die er al zijn te vervangen.
7. **Overzichtsrijen onder gegevens** begint zoals het overzicht van het blad is ingesteld. Laat het aangevinkt om elke totaalrij onder zijn groep te zetten, of vink het uit om elke totaalrij boven zijn groep te zetten, met het eindtotaal bovenaan. Druk op **OK**.

Elke totaalrij zegt "Oost totaal" (of de waarde waarvoor die bedoeld is) en bevat een SUBTOTAAL-formule, zodat die de andere totaalrijen overslaat. De rijen van elke groep worden gegroepeerd, zodat u met de knoppen 1, 2 en 3 naast de rijen alleen de totalen kunt tonen. Om de totalen weg te halen, opent u **Subtotaal** opnieuw en drukt u op **Alles verwijderen**. Als de totalen naar de andere kant van een blad zijn gegaan dat al eigen groepen had, meldt de statusbalk dat die groepen hun samenvattingsrijen nu aan die kant hebben, en zet Alles verwijderen de kant terug zoals die was voor het eerste subtotaal, tenzij u die sindsdien zelf hebt gewijzigd. Wanneer de totalen boven staan, staan de overzichtsknoppen bij de totaalrijen, dus een plus- of minknop naast "Oost totaal" toont of verbergt de Oost-rijen eronder. Als u **Huidige subtotalen vervangen** uitschakelt, moeten de nieuwe totalen aan dezelfde kant komen als de bestaande. Anders wordt niets gewijzigd, en een melding zegt dat. Subtotalen toevoegen of verwijderen is één stap die u ongedaan kunt maken.

## Geavanceerd filter

De filterknoppen kiezen rijen per kolom. **Geavanceerd** (in de groep **Sorteren en filteren**) laat u de voorwaarden op het blad schrijven, zodat ze zo lang kunnen zijn als nodig.

1. Kopieer naast de lijst de koppen van de kolommen die u wilt testen naar een rij met lege cellen.
2. Schrijf onder elke kop een voorwaarde. Voorwaarden in **dezelfde rij** moeten allemaal waar zijn (dit is "en"). Voorwaarden in **verschillende rijen** zijn alternatieven: een rij van de lijst hoeft aan slechts één van die rijen te voldoen (dit is "of").
3. Kies **Geavanceerd**. Controleer dat **Lijstbereik** de lijst met zijn kopregel is, en typ de cellen van uw koppen en voorwaarden bij **Voorwaardenbereik**.
4. Kies **De overige rijen in de lijst verbergen**, of **De rijen naar een andere plaats kopiëren** en typ de eerste cel van de plaats, zoals H1. Vink **Rijen die een eerdere rij herhalen overslaan** aan om er maar één van elk te kopiëren.
5. Druk op **OK**.

Zo schrijft u een voorwaarde:

- Een woord, zoals **An**, selecteert tekst die ermee begint. Hoofd- en kleine letters tellen als gelijk. **?** staat voor een willekeurig teken en ***** voor een willekeurige reeks tekens.
- Een getal selecteert cellen die er gelijk aan zijn.
- **=Ann** selecteert tekst die precies Ann is. **=** alleen selecteert lege cellen.
- **>**, **>=**, **<**, **<=** en **<>** (niet gelijk aan) voor een getal of woord vergelijken ermee, zoals **>25**. **<>** alleen selecteert cellen die niet leeg zijn.

Een rij voorwaarden zonder iets erin selecteert elke rij. Om de verborgen rijen weer te tonen, kiest u **Wissen**. Een kopie wordt meteen gemaakt en verandert niet wanneer de lijst verandert. Geavanceerd filter is één stap die u ongedaan kunt maken.

## Gegevensvalidatie

Gebruik dit om vast te leggen wat in sommige cellen mag worden getypt, zodat een verkeerde invoer wordt tegengehouden of in twijfel getrokken.

1. Selecteer de cellen.
2. Kies **Gegevensvalidatie** in de groep **Gegevenshulpmiddelen** op het tabblad **Gegevens**.
3. Kies op het tabblad **Instellingen** wat u wilt **Toestaan**: **Elke waarde** (geen regel), **Geheel getal**, **Decimaal getal**, **Lijst**, **Datum**, **Tijd**, **Tekstlengte**, of **Aangepast**.
4. Kies bij een getal, datum, tijd of tekstlengte **Gegevens** (**tussen**, **niet tussen**, **gelijk aan**, enzovoort) en typ de grenzen. Een grens kan een waarde zijn of een formule die met = begint.
5. Voor een **Lijst** typt u de keuzes met komma's ertussen, zoals Rood,Groen,Blauw, of typt u na = een celbereik, zoals =$A$1:$A$5. De cel toont dan een pijltje met de keuzes. Schakel **Keuzelijst in cel** uit om het pijltje te verbergen.
6. Voor **Aangepast** typt u een formule die met = begint. De invoer is toegestaan wanneer de formule waar is.
7. Met **Lege cellen negeren** mag een lege cel passeren.

Vink op het tabblad **Invoerbericht** het vak aan en schrijf een titel en een bericht. Ze worden naast de cel getoond wanneer die is geselecteerd.

Kies op het tabblad **Foutmelding** wat er gebeurt met een invoer die niet is toegestaan. **Stoppen** weigert die. **Waarschuwing** vraagt of u die wilt behouden. **Informatie** vertelt het de gebruiker en houdt de invoer. Schrijf een titel en een bericht voor de melding.

**Ongeldige gegevens cirkelen** (naast **Gegevensvalidatie**) zet een rode ovaal rond elke cel op het blad waarvan de waarde de regel niet toestaat, zoals een getal dat er was voordat de regel bestond, of het resultaat van een formule. Een bericht laat u weten of alle cellen in orde zijn. Een cel die u verbetert, verliest zijn ovaal. **Validatiemarkeringen wissen** haalt alle ovalen weg.

**Wissen** haalt de regels weg van de geselecteerde cellen. De nieuwe regel vervangt elke regel die de geselecteerde cellen hadden; andere cellen houden de hunne. Het wijzigen van regels is één stap die u ongedaan kunt maken, en de regels worden in het bestand opgeslagen.

## Consolideren

Gebruik dit om bereiken die op elkaar lijken, zoals dezelfde tabel uit verschillende bladen, in één tabel te combineren.

1. Kies **Consolideren** in de groep **Gegevenshulpmiddelen** op het tabblad **Gegevens**.
2. Kies de **Functie** om mee te combineren, zoals **Som** of **Gemiddelde**.
3. Typ een bereik in het vak en druk op **Toevoegen**. Gebruik **Blad2!A1:C10** voor een bereik op een ander blad. Voeg elk bereik toe dat u wilt combineren. Selecteer een bereik in de lijst en druk op **Verwijderen** om het weg te halen.
4. Als de bereiken namen in hun eerste rij hebben, vinkt u **Bovenste rij bevat de kolomnamen** aan. Als ze namen in hun eerste kolom hebben, vinkt u **Linker kolom bevat de rijnamen** aan.
5. Typ bij **Resultaat plaatsen op** de eerste cel voor het resultaat.
6. Vink **Koppelingen maken naar de brongegevens** aan als elk resultaat een formule moet zijn die zijn bronnen leest, zodat het verandert wanneer zij veranderen. Zonder dit zijn de resultaten gewone getallen.
7. Druk op **OK**.

Als geen namen zijn aangevinkt, wordt de cel op dezelfde plek in elk bereik gecombineerd (de eerste cel van elk bereik, dan de tweede, enzovoort). Met aangevinkte namen worden cellen met dezelfde namen gecombineerd, waar ze in elk bereik ook staan, en een naam die maar in sommige bereiken voorkomt, krijgt nog steeds een rij of kolom. Consolideren is één stap die u ongedaan kunt maken.

## Prognoseblad

Prognoseblad maakt een nieuw blad met een tabel en een diagram van toekomstige waarden, berekend uit een tijdlijn en een reeks waarden. Het vindt de trend en het zich herhalende seizoen van de waarden, en voegt een band toe waarbinnen de toekomstige waarden waarschijnlijk blijven.

1. Selecteer de tijdlijn en de waarden, twee kolommen naast elkaar (de tijdlijn eerst). De tijdlijn bestaat uit datums of getallen die even ver uit elkaar liggen, zoals elke maand of elke 5 eenheden. Een eerste rij met namen wordt overgeslagen.
2. Kies **Prognoseblad** in de groep **Prognose** op het tabblad **Gegevens**.
3. Een afbeelding toont de prognose, en verandert wanneer u de keuzes wijzigt. Kies **Lijndiagram** of **Kolomdiagram**.
4. Stel de keuzes in:
   - **Tijdlijnbereik** en **Waardenbereik** zijn de cellen die worden gelezen. Typ hier andere cellen als de selectie verkeerd was.
   - **Begin prognose** is leeg om na het laatste punt te beginnen. Een datum of getal vóór het laatste punt begint daar en gebruikt alleen de punten ervoor, zodat u de prognose kunt vergelijken met wat er is gebeurd.
   - **Einde prognose** is het laatste tijdstip waarvoor u voorspelt. Er worden hoogstens 10.000 punten voorspeld.
   - **Betrouwbaarheidsinterval** is hoe zeker de band is, in het begin 95%.
   - **Seizoensinvloed** wordt bepaald door naar de waarden te kijken, of u stelt de lengte van het seizoen in punten in, of kiest geen. De waarden moeten twee volledige seizoenen bevatten.
   - **Ontbrekende punten aanvullen met** bepaalt wat een tijdstip zonder waarde krijgt: een rechte lijn tussen zijn buren, of nul.
   - **Dubbele waarden samenvoegen met** bepaalt hoe waarden op hetzelfde tijdstip worden gecombineerd: Gemiddelde, Aantal, AantalA, Max, Mediaan, Min, of Som.
   - **Prognosestatistieken opnemen** voegt een tabel toe met Alfa, Bèta, Gamma, MASE, SMAPE, MAE en RMSE, die laten zien hoe goed de prognose op de waarden past.
5. Als er iets niet klopt, zegt het dialoogvenster in gewone woorden wat (stappen die niet gelijk zijn, te weinig punten, tekst in de waarden), en **Maken** wacht.
6. Druk op **Maken**.

Het nieuwe blad wordt vóór het blad geplaatst waarop u was. De tabel heeft de kolommen Tijdlijn, Waarden, Prognose, Ondergrens betrouwbaarheidsinterval en Bovengrens betrouwbaarheidsinterval. De kolom Prognose en de grenzen zijn formules die `FORECAST.ETS` en `FORECAST.ETS.CONFINT` gebruiken op de eigen kolommen Tijdlijn en Waarden van de tabel, dus ze worden opnieuw berekend wanneer u een waarde wijzigt. Het diagram toont de waarden, de prognose en de band. Prognoseblad is één stap die u ongedaan kunt maken.

De getallen kunnen enkele procenten afwijken van die van andere spreadsheetprogramma's, die hun eigen manier gebruiken om de afvlakking te passen. Een prognose die een bestand al bevat, behoudt de waarden die het bestand heeft opgeslagen, totdat de gegevens die ze lezen worden gewijzigd.

Een bestand met meer dan 1.000.000 punten wordt niet gebruikt.
