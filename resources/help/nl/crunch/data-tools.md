# Gegevenshulpmiddelen
<!-- keywords: scenariobeheer, scenario's, wat-als, wat als, scenariosamenvatting, gegevens, tabblad gegevens, tekst naar kolommen, splitsen, scheidingsteken, komma, tab, vaste breedte, duplicaten verwijderen, doelzoeken, subtotaal, totalen, geavanceerd filter, voorwaarden, gegevensvalidatie, ongeldige gegevens markeren, rode ovaal, validatiemarkeringen wissen, consolideren, bereiken combineren, toestaan, lijst, vervolgkeuzelijst, invoerbericht, foutmelding, herhaalde rijen, uniek, tekstbestand, csv, uit tekst, kolommen, scheidingsteken, aanhalingstekens, koppelingen bewerken, waarden bijwerken, bron wijzigen, koppelingen, extern, bron, koppeling verbreken, status controleren, bron openen -->

Op het tabblad **Gegevens** staan de hulpmiddelen die met een lijst van waarden werken: tekst uit een bestand halen, een lijst sorteren, tekst in kolommen splitsen en herhaalde rijen verwijderen.

## Uit tekst/CSV

Met **Uit tekst/CSV** opent u een tekstbestand (.csv, .tsv of .txt). Dit is hetzelfde als **Tekstbestand openen** op het tabblad Bestand. JeffCrunch vraagt hoe het bestand in kolommen is gesplitst en wat voor soort tekst het bevat, en toont het daarna als een nieuwe werkmap.

## Koppelingen bewerken

Als een werkmap formules heeft die naar een andere werkmap verwijzen (zoals ='[Prijzen.xlsx]Blad1'!A1), toont **Koppelingen bewerken** in de groep **Gegevens ophalen** die andere werkmappen. Die heten bronnen. Elke rij toont waar de bron staat en wat de status is. Totdat de werkmap opnieuw met zijn bronnen wordt geopend, toont hij de getallen die het bestand van die bronnen heeft opgeslagen.

- **Status controleren** zoekt op deze computer naar elke bron en meldt **Gevonden** of dat de bron niet is gevonden. Een bron op het web wordt niet opgehaald; de status zegt dan dat de bron niet is gecontroleerd.
- **Waarden bijwerken** leest de gekozen bron van deze computer en geeft elke cel die alleen naar een van zijn cellen verwijst, de waarde die die cel nu heeft. Een cel die meer met de waarde doet, zoals `=[1]Prijzen!A1*2`, behoudt het getal dat het bestand had opgeslagen. Het bericht onder de lijst zegt hoeveel cellen zijn bijgewerkt en hoeveel niet. Waarden bijwerken is één stap, dus met **Ongedaan maken** zet u de oude waarden terug.
- **Bron wijzigen** laat u een ander bestand kiezen voor de gekozen koppeling, bijvoorbeeld wanneer de werkmap is verplaatst. De formules blijven ongewijzigd. De nieuwe locatie wordt bij het opslaan in het bestand geschreven, en **Ongedaan maken** neemt de wijziging terug.
- **Bron openen** opent de gekozen bron in een nieuw venster, als deze kan worden gevonden.
- **Koppeling verbreken** zet elke cel die naar de gekozen bron verwijst om in de waarde die hij nu toont, en een naam die in de werkmap voor een plaats stond, in **#VERW!**. Dit kan niet ongedaan worden gemaakt, dus JeffCrunch vraagt eerst om bevestiging. Als er geen koppeling meer over is, wordt het bestand opgeslagen zonder de onderdelen die ze bevatten.

## Sorteren en filteren

De groep **Sorteren en filteren** heeft dezelfde opdrachten als het menu **Sorteren en filteren** op het tabblad Start. Zie [Sorteren en filteren](sort-filter).

## Tekst naar kolommen

Gebruik dit wanneer één kolom meerdere stukjes tekst bevat die u in afzonderlijke kolommen wilt hebben, zoals "Jansen, Ann, 31".

1. Selecteer de cellen van **één** kolom.
2. Kies **Tekst naar kolommen** op het tabblad **Gegevens**.
3. Kies hoe de tekst wordt gesplitst:
   - **Splitsen op scheidingsteken**: vink de tekens aan die tussen de stukjes staan: **Tab**, **Puntkomma**, **Komma**, **Spatie** of **Andere**, waarin u zelf een teken typt. **Scheidingstekens achter elkaar als één behandelen** voorkomt dat twee komma's na elkaar een leeg stukje maken. **Tekst tussen aanhalingstekens** geeft het teken aan dat een stukje omsluit dat niet mag worden gesplitst, zoals "Lee, Bo". Binnen de aanhalingstekens staan twee tekens achter elkaar voor één teken.
   - **Splitsen op vaste posities**: typ de posities waar een nieuwe kolom begint, waarbij tekens vanaf 1 worden geteld. Met 5, 12 begint een nieuwe kolom bij het vijfde en bij het twaalfde teken.
4. Kies wat elk stukje is: **Algemeen** leest getallen en datums zoals u ze hebt getypt, en **Tekst** houdt elk stukje zoals het is, zodat "007" "007" blijft.
5. Typ bij **Delen plaatsen vanaf** de eerste cel voor de delen. Die begint met de eerste cel die u hebt geselecteerd.
6. Het vak onder de keuzes toont de eerste rijen zoals u ze hebt gesplitst. Klik op **OK**.

De delen komen in de cellen rechts van de eerste cel en vervangen wat erin stond. Tekst naar kolommen is één stap die u ongedaan kunt maken.

## Duplicaten verwijderen

Gebruik dit om van elke groep herhaalde rijen alleen de eerste te bewaren.

1. Selecteer de cellen van de lijst, of één cel erin. JeffCrunch neemt het hele gegevensblok rond een enkele cel.
2. Kies **Duplicaten verwijderen**.
3. Vink **Mijn gegevens hebben een kopregel** aan als de eerste rij namen bevat. Die rij blijft staan en wordt niet vergeleken.
4. Vink de kolommen aan die met elkaar moeten overeenkomen, wil twee rijen als herhaling gelden. Twee rijen zijn alleen herhalingen wanneer elke aangevinkte kolom gelijk is. Hoofdletters en kleine letters tellen als gelijk, en een formule telt als de waarde die ze oplevert.
5. Klik op **OK**. Het dialoogvenster meldt hoeveel rijen zijn verwijderd en hoeveel er over zijn.

De rijen onder een verwijderde rij schuiven omhoog binnen de geselecteerde cellen, met hun opmaak en formules. De cellen onder de selectie verschuiven niet. De lege rijen die onderaan de selectie overblijven, worden leeggemaakt. Duplicaten verwijderen is één stap die u ongedaan kunt maken.

## Doelzoeken

Gebruik dit wanneer u het antwoord weet dat u wilt, maar niet het getal dat het oplevert. Een betalingscel bevat bijvoorbeeld een formule met een rentepercentage, en u wilt dat de betaling 500 is. Doelzoeken probeert getallen in de rentecel tot de betaling 500 is.

1. Kies **Doelzoeken** op het tabblad **Gegevens**.
2. Typ bij **Cel met de formule** de cel met de formule, zoals B3. Die begint met de cel die u hebt geselecteerd.
3. Typ bij **Gewenste waarde** het antwoord dat de formule moet geven.
4. Typ bij **Te wijzigen cel** de cel die Doelzoeken mag wijzigen, zoals B1. Die moet een getal bevatten, geen formule.
5. Klik op **OK**.

Als Doelzoeken een getal vindt, blijft het in de te wijzigen cel staan, en de statusbalk zegt welk getal het is. Als dat niet lukt, verandert er niets en krijgt u daarvan bericht. Probeer een ander startgetal in de te wijzigen cel en controleer of de formule die cel echt gebruikt. Doelzoeken is één stap die u ongedaan kunt maken.

## Scenariobeheer

Een scenario is een benoemde set waarden voor een aantal cellen. Gebruik scenario's om "wat als" op één blad uit te proberen. Bewaar bijvoorbeeld een scenario "Lage verkoop" en een scenario "Hoge verkoop" voor dezelfde invoercellen, en toon het scenario dat u wilt bekijken.

1. Kies **Scenariobeheer** op het tabblad **Gegevens**.
2. Klik op **Toevoegen**. Typ een naam, zoals Hoge verkoop. Typ bij **Te wijzigen cellen** de cellen die het scenario instelt, zoals B1:B3 of D5. Een scenario kan maximaal 32 cellen wijzigen. De cellen die u hebt geselecteerd, worden alvast ingevuld. Elke cel begint met de waarde die hij nu heeft. Typ in de tabel de waarde die dit scenario moet gebruiken, en klik op **OK**.
3. Kies een scenario in de lijst en klik op **Weergeven** om de waarden in de cellen te zetten. Dubbelklikken op een naam doet hetzelfde. Klik op **Bewerken** om de naam, de opmerking of de waarden te wijzigen. Klik op **Verwijderen** om het weg te halen.

Weergeven, Toevoegen, Bewerken en Verwijderen zijn elk één stap die u ongedaan kunt maken.

Met **Samenvatting** zet u alle scenario's naast elkaar op een nieuw blad met de naam Scenariosamenvatting. Er wordt gevraagd naar de resultaatcellen, dat zijn de cellen die u wilt vergelijken, zoals het totaal in B10. Voor elk scenario toont de samenvatting de waarden van de te wijzigen cellen en wat de resultaatcellen worden, berekend met de waarden van dat scenario. Een kolom met de naam Huidige waarden laat zien hoe de cellen stonden toen u de samenvatting maakte. Uw eigen cellen blijven zoals ze waren.

De scenario's worden in de werkmap opgeslagen. Een werkmap die met een ander programma is gemaakt, behoudt zijn scenario's, en u ziet ze hier. Een kopie die als oudere .xls-bestand of als .ods wordt opgeslagen, bevat ze niet, en het opslaan meldt dat.

## Subtotaal

Gebruik dit om onder elke groep rijen met dezelfde waarde in één kolom een rij met totalen te zetten, en onderaan een eindtotaal. De lijst heeft een kopregel nodig en moet gesorteerd zijn op de kolom waarop u groepeert, zodat gelijke waarden naast elkaar staan.

1. Selecteer een cel in de lijst.
2. Kies **Subtotaal** in de groep **Overzicht** op het tabblad **Gegevens**.
3. Kies bij **Bij elke wijziging in** de kolom die de rijen groepeert, zoals Regio.
4. Kies bij **Functie gebruiken** wat u wilt berekenen: **Som**, **Aantal**, **Gemiddelde**, **Max**, **Min** en andere.
5. Vink bij **Subtotaal toevoegen aan** elke kolom aan die u wilt totaliseren.
6. Laat **Huidige subtotalen vervangen** aangevinkt om subtotalen die er al staan te vervangen. Klik op **OK**.

Elke totaalrij krijgt de tekst "Oost totaal" (of de waarde waarvoor het is) en bevat een SUBTOTAAL-formule, zodat die de andere totaalrijen overslaat. De rijen van elke groep worden gegroepeerd, zodat u met de knoppen 1, 2 en 3 naast de rijen alleen de totalen kunt tonen. Om de totalen weg te halen, opent u **Subtotaal** opnieuw en klikt u op **Alles verwijderen**. Subtotalen toevoegen of verwijderen is één stap die u ongedaan kunt maken.

## Geavanceerd filter

Met de filterknoppen kiest u rijen op basis van één kolom tegelijk. Met **Geavanceerd** (in de groep **Sorteren en filteren**) schrijft u de voorwaarden op het blad, zodat ze zo lang kunnen zijn als nodig.

1. Kopieer naast de lijst de koppen van de kolommen die u wilt testen naar een rij lege cellen.
2. Schrijf onder elke kop een voorwaarde. Voorwaarden in **dezelfde rij** moeten allemaal waar zijn (dit is "en"). Voorwaarden in **verschillende rijen** zijn alternatieven: een rij van de lijst hoeft aan maar één van die rijen te voldoen (dit is "of").
3. Kies **Geavanceerd**. Controleer of **Lijstbereik** de lijst met zijn kopregel is, en typ de cellen van uw koppen en voorwaarden bij **Voorwaardenbereik**.
4. Kies **Overige rijen in de lijst verbergen**, of **Rijen naar een andere plaats kopiëren** en typ de eerste cel van de plaats, zoals H1. Vink **Rijen overslaan die een eerdere rij herhalen** aan om er maar één van elke soort te kopiëren.
5. Klik op **OK**.

Zo schrijft u een voorwaarde:

- Een woord, zoals **An**, selecteert tekst die ermee begint. Hoofdletters en kleine letters tellen als gelijk. **?** staat voor één willekeurig teken en ***** voor een willekeurige reeks tekens.
- Een getal selecteert cellen die er gelijk aan zijn.
- **=Ann** selecteert tekst die precies Ann is. **=** alleen selecteert lege cellen.
- **>**, **>=**, **<**, **<=** en **<>** (niet gelijk) vóór een getal of woord vergelijken ermee, zoals **>25**. **<>** alleen selecteert cellen die niet leeg zijn.

Een rij voorwaarden zonder inhoud selecteert elke rij. Om de verborgen rijen weer te tonen, kiest u **Wissen**. Een kopie wordt meteen gemaakt en verandert niet wanneer de lijst verandert. Geavanceerd filter is één stap die u ongedaan kunt maken.

## Gegevensvalidatie

Gebruik dit om vast te leggen wat in sommige cellen mag worden getypt, zodat een verkeerde invoer wordt tegengehouden of in twijfel getrokken.

1. Selecteer de cellen.
2. Kies **Gegevensvalidatie** in de groep **Hulpmiddelen voor gegevens** op het tabblad **Gegevens**.
3. Kies op het tabblad **Instellingen** bij **Toestaan** wat mag: **Elke waarde** (geen regel), **Geheel getal**, **Decimaal getal**, **Lijst**, **Datum**, **Tijd**, **Tekstlengte** of **Aangepast**.
4. Kies bij een getal, datum, tijd of tekstlengte bij **Gegevens** (**tussen**, **niet tussen**, **gelijk aan**, enzovoort) en typ de grenzen. Een grens kan een waarde zijn of een formule die met = begint.
5. Voor een **Lijst** typt u de keuzes met komma's ertussen, zoals Rood,Groen,Blauw, of typt u na = een celbereik, zoals =$A$1:$A$5. De cel toont dan een pijltje met de keuzes. Schakel **Vervolgkeuzelijst in cel** uit om het pijltje te verbergen.
6. Voor **Aangepast** typt u een formule die met = begint. De invoer is toegestaan wanneer de formule waar is.
7. Met **Lege cellen negeren** laat u een lege cel door.

Vink op het tabblad **Invoerbericht** het vakje aan en schrijf een titel en een bericht. Ze worden naast de cel getoond zodra die is geselecteerd.

Kies op het tabblad **Foutmelding** wat er gebeurt met een invoer die niet is toegestaan. **Stop** weigert de invoer. **Waarschuwing** vraagt of de invoer moet blijven staan. **Informatie** laat de gebruiker weten dat en houdt de invoer. Schrijf een titel en een bericht voor de melding.

**Ongeldige gegevens markeren** (naast **Gegevensvalidatie**) zet een rode ovaal om elke cel op het blad waarvan de waarde niet door de regel is toegestaan, bijvoorbeeld een getal dat er al stond voordat de regel werd ingesteld, of het resultaat van een formule. Een bericht laat zien als elke cel in orde is. Een cel die u hebt gecorrigeerd, verliest zijn ovaal. **Validatiemarkeringen wissen** haalt alle ovalen weg.

Met **Alles wissen** haalt u de regels van de geselecteerde cellen weg. De nieuwe regel vervangt elke regel die de geselecteerde cellen al hadden; andere cellen houden de hunne. Regels wijzigen is één stap die u ongedaan kunt maken, en de regels worden in het bestand opgeslagen.

## Consolideren

Gebruik dit om bereiken die op elkaar lijken, zoals dezelfde tabel op meerdere bladen, samen te voegen tot één tabel.

1. Kies **Consolideren** in de groep **Hulpmiddelen voor gegevens** op het tabblad **Gegevens**.
2. Kies de **Functie** waarmee u wilt combineren, zoals **Som** of **Gemiddelde**.
3. Typ een bereik in het vak en klik op **Toevoegen**. Gebruik **Blad2!A1:C10** voor een bereik op een ander blad. Voeg elk bereik toe dat u wilt combineren. Selecteer een bereik in de lijst en klik op **Verwijderen** om het eruit te halen.
4. Als de bereiken in hun eerste rij namen hebben, vink dan **Bovenste rij bevat de namen van kolommen** aan. Als ze namen in hun eerste kolom hebben, vink dan **Linker kolom bevat de namen van rijen** aan.
5. Typ bij **Resultaat plaatsen op** de eerste cel voor het resultaat.
6. Vink **Koppelingen naar de brongegevens maken** aan als elk resultaat een formule moet zijn die de brongegevens leest, zodat het meeverandert wanneer zij veranderen. Zonder dit vinkje zijn de resultaten gewone getallen.
7. Klik op **OK**.

Zonder aangevinkte namen wordt de cel op dezelfde plek in elk bereik gecombineerd (de eerste cel van elk bereik, dan de tweede, enzovoort). Met aangevinkte namen worden cellen met dezelfde namen gecombineerd, waar ze ook in elk bereik staan, en een naam die alleen in sommige bereiken voorkomt, krijgt nog steeds een rij of kolom. Consolideren is één stap die u ongedaan kunt maken.
