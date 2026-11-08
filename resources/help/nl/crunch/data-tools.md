# Gegevenshulpmiddelen
<!-- keywords: scenario manager, scenarios, wat-als, wat als, scenariobeheer, scenario's, scenariosamenvatting, gegevens, tabblad gegevens, tekst naar kolommen, splitsen, scheidingsteken, komma, tab, vaste breedte, duplicaten verwijderen, doelzoeken, subtotaal, totalen, geavanceerd filter, voorwaarden, gegevensvalidatie, ongeldige gegevens markeren, rode ovaal, validatiemarkeringen wissen, consolideren, bereiken combineren, toestaan, lijst, vervolgkeuzelijst, invoerbericht, foutmelding, herhaalde rijen, uniek, tekstbestand, csv, uit tekst, kolommen, scheidingsteken, aanhalingstekens, koppelingen bewerken, waarden bijwerken, bron wijzigen, koppelingen, extern, bron, koppeling verbreken, status controleren, bron openen -->

Het tabblad **Gegevens** bevat de hulpmiddelen die met een lijst met waarden werken: tekst uit een bestand halen, een lijst sorteren, tekst in kolommen splitsen en herhaalde rijen verwijderen.

## Uit tekst/CSV

Met **Uit tekst/CSV** opent u een tekstbestand (.csv, .tsv of .txt). Dit is hetzelfde als **Tekstbestand openen** op het tabblad Bestand. JeffCrunch vraagt hoe het bestand in kolommen is verdeeld en wat voor soort tekst erin staat, en toont het daarna als nieuwe werkmap.

## Koppelingen bewerken

Als een werkmap formules heeft die een andere werkmap lezen (zoals ='[Prijzen.xlsx]Blad1'!A1), somt **Koppelingen bewerken** in de groep **Gegevens ophalen** die andere werkmappen op. Die worden bronnen genoemd. Elke rij toont waar de bron staat en wat de status is. Totdat de werkmap opnieuw met zijn bronnen wordt geopend, toont hij de getallen die zijn bestand daarvan heeft opgeslagen.

- **Status controleren** zoekt op deze computer naar elke bron en meldt **Gevonden** of dat de bron niet is gevonden. Een bron op het web wordt niet opgehaald; de status zegt dan dat er niet is gecontroleerd.
- **Waarden bijwerken** leest de gekozen bron van deze computer en geeft elke cel die alleen een verwijzing naar een van zijn cellen is de waarde die die cel nu heeft. Een cel die met de waarde meer doet, zoals `=[1]Prijzen!A1*2`, behoudt het getal dat het bestand ervan heeft opgeslagen. Het bericht onder de lijst zegt hoeveel cellen zijn bijgewerkt en hoeveel niet. Waarden bijwerken is één stap, dus **Ongedaan maken** zet de oude waarden terug. De nieuwe waarden worden ook in het eigen deel van de koppeling in het bestand geschreven wanneer u opslaat, zodat de werkmap ze de volgende keer dat hij wordt geopend toont, ook als de bron er niet is.
- **Bron wijzigen** laat u een ander bestand kiezen voor de gekozen koppeling, bijvoorbeeld wanneer de werkmap is verplaatst. De formules blijven zoals ze zijn. De nieuwe locatie wordt bij het opslaan in het bestand geschreven, en **Ongedaan maken** neemt de wijziging terug.
- **Bron openen** opent de gekozen bron in een nieuw venster, als die kan worden gevonden.
- **Koppeling verbreken** zet elke cel die de gekozen bron leest om in de waarde die die nu toont, en een naam die voor een plaats in die bron stond, in **#VERW!**. JeffCrunch vraagt eerst om bevestiging. Koppeling verbreken is één stap, dus **Ongedaan maken** brengt de formules, de namen en de koppelingen terug, totdat u de werkmap opslaat. Wanneer er geen koppeling meer over is, wordt het bestand opgeslagen zonder de onderdelen die ze bevatten.

## Sorteren en filteren

De groep **Sorteren en filteren** heeft dezelfde opdrachten als het menu **Sorteren en filteren** op het tabblad Start. Zie [Sorteren en filteren](sort-filter).

## Tekst naar kolommen

Gebruik dit wanneer één kolom meerdere stukken tekst bevat die u in afzonderlijke kolommen wilt hebben, zoals "Jansen, Ann, 31".

1. Selecteer de cellen van **één** kolom.
2. Kies **Tekst naar kolommen** op het tabblad **Gegevens**.
3. Kies hoe de tekst wordt gesplitst:
   - **Splitsen op scheidingsteken**: vink de tekens aan die tussen de stukken staan: **Tab**, **Puntkomma**, **Komma**, **Spatie** of **Ander**, waarbij u zelf het teken typt. **Opeenvolgende scheidingstekens als één behandelen** voorkomt dat twee komma's achter elkaar een leeg stuk opleveren. **Tekst tussen aanhalingstekens** geeft het teken aan dat een stuk omsluit dat niet mag worden gesplitst, zoals "Lee, Bo". Binnen de aanhalingstekens staan twee tekens achter elkaar voor één teken.
   - **Splitsen op vaste posities**: typ de posities waar een nieuwe kolom begint, waarbij tekens vanaf 1 worden geteld. Met 5, 12 begint een nieuwe kolom bij het vijfde en bij het twaalfde teken.
4. Kies wat elk stuk is: **Algemeen** herkent getallen en datums zoals u ze hebt getypt, en **Tekst** houdt elk stuk zoals het is, zodat "007" "007" blijft.
5. Typ bij **Gegevens plaatsen vanaf** de eerste cel voor de stukken. Die staat eerst op de eerste cel die u hebt geselecteerd.
6. Het vak onder de keuzes toont de eerste rijen zoals u ze hebt gesplitst. Klik op **OK**.

De stukken komen in de cellen rechts van de eerste cel, en ze vervangen wat er in die cellen stond. Tekst naar kolommen is één stap die u ongedaan kunt maken.

## Duplicaten verwijderen

Gebruik dit om van elke groep herhaalde rijen alleen de eerste te bewaren.

1. Selecteer de cellen van de lijst, of één cel erin. JeffCrunch neemt het hele gegevensblok rond één cel.
2. Kies **Dubbele waarden verwijderen**.
3. Vink **Mijn gegevens hebben koppen** aan als de eerste rij namen bevat. Die rij blijft staan en wordt niet vergeleken.
4. Vink de kolommen aan die moeten overeenkomen, wil twee rijen als herhaling gelden. Twee rijen zijn alleen herhalingen wanneer elke aangevinkte kolom gelijk is. Hoofdletters en kleine letters tellen als hetzelfde, en een formule telt als de waarde die hij oplevert.
5. Klik op **OK**. Het dialoogvenster meldt hoeveel rijen zijn verwijderd en hoeveel er over zijn.

De rijen onder een verwijderde rij schuiven omhoog binnen de geselecteerde cellen, met hun opmaak en formules. De cellen onder de selectie verschuiven niet. De lege rijen die onderaan de selectie overblijven, worden leeggemaakt. Duplicaten verwijderen is één stap die u ongedaan kunt maken.

## Doelzoeken

Gebruik dit wanneer u het antwoord weet dat u wilt, maar niet het getal dat het oplevert. Bijvoorbeeld: een cel met een betaling bevat een formule met een rentepercentage, en u wilt dat de betaling 500 is. Doelzoeken probeert getallen in de rentecel tot de betaling 500 is.

1. Kies **Doelzoeken** op het tabblad **Gegevens**.
2. Typ bij **Cel met de formule** de cel met de formule, zoals B3. Die staat eerst op de cel die u hebt geselecteerd.
3. Typ bij **Gewenste waarde** het antwoord dat de formule moet geven.
4. Typ bij **Te wijzigen cel** de cel die Doelzoeken mag wijzigen, zoals B1. Die moet een getal bevatten, geen formule.
5. Klik op **OK**.

Als Doelzoeken een getal vindt, blijft het in de te wijzigen cel staan, en de statusbalk zegt welk getal het is. Als het er geen vindt, verandert er niets en krijgt u een melding. Probeer een ander startgetal in de te wijzigen cel, en controleer of de formule die cel echt gebruikt. Doelzoeken is één stap die u ongedaan kunt maken.

## Scenariobeheer

Een scenario is een benoemde set waarden voor sommige cellen. Gebruik scenario's om "wat-als" op één blad uit te proberen. Bewaar bijvoorbeeld een scenario "Lage verkoop" en een scenario "Hoge verkoop" voor dezelfde invoercellen, en toon het scenario dat u wilt bekijken.

1. Kies **Scenariobeheer** op het tabblad **Gegevens**.
2. Klik op **Toevoegen**. Typ een naam, zoals Hoge verkoop. Typ bij **Te wijzigen cellen** de cellen die het scenario instelt, zoals B1:B3 of D5. Een scenario kan maximaal 32 cellen wijzigen. De cellen die u hebt geselecteerd, worden automatisch ingevuld. Elke cel begint met de waarde die hij nu heeft. Typ in de tabel de waarde die dit scenario moet gebruiken, en klik op **OK**.
3. Kies een scenario in de lijst en klik op **Weergeven** om de waarden ervan in de cellen te zetten. Dubbelklikken op een naam doet hetzelfde. Klik op **Bewerken** om de naam, de opmerking of de waarden te wijzigen. Klik op **Verwijderen** om het weg te halen.

Weergeven, Toevoegen, Bewerken en Verwijderen zijn elk één stap die u ongedaan kunt maken.

Met **Samenvatting** worden alle scenario's naast elkaar gezet op een nieuw blad met de naam Scenariosamenvatting. Er wordt om de resultaatcellen gevraagd: de cellen die u wilt vergelijken, zoals het totaal in B10. Voor elk scenario toont de samenvatting de waarden van de te wijzigen cellen en wat de resultaatcellen worden, berekend met de waarden van dat scenario. Een kolom met de naam Huidige waarden toont hoe de cellen waren toen u de samenvatting maakte. Uw eigen cellen blijven zoals ze waren.

De scenario's worden in de werkmap opgeslagen. Een werkmap die door een ander programma is gemaakt, behoudt zijn scenario's, en u ziet ze hier. Een kopie die is opgeslagen als een oudere .xls-bestand of als .ods bevat ze niet, en het opslaan meldt dat.

## Subtotaal

Gebruik dit om onder elke groep rijen met dezelfde invoer in één kolom een rij met totalen te zetten, en onderaan een eindtotaal. De lijst heeft een kopregel nodig en moet worden gesorteerd op de kolom waarop u groepeert, zodat gelijke invoer naast elkaar staat.

1. Selecteer een cel in de lijst.
2. Kies **Subtotaal** in de groep **Overzicht** op het tabblad **Gegevens**.
3. Kies bij **Bij elke wijziging in** de kolom die de rijen groepeert, zoals Regio.
4. Kies bij **Functie gebruiken** wat u wilt berekenen: **Som**, **Aantal**, **Gemiddelde**, **Max**, **Min** en andere.
5. Vink bij **Subtotaal toevoegen aan** elke kolom aan die u wilt totaliseren.
6. Laat **Huidige subtotalen vervangen** aangevinkt om subtotalen die er al staan te vervangen. Klik op **OK**.

Elke totaalrij zegt "Oost totaal" (of de invoer waarvoor hij geldt) en bevat een SUBTOTAAL-formule, zodat de andere totaalrijen worden overgeslagen. De rijen van elke groep worden gegroepeerd, zodat u met de knoppen 1, 2 en 3 naast de rijen alleen de totalen kunt tonen. Om de totalen weg te halen, opent u **Subtotaal** opnieuw en klikt u op **Alles verwijderen**. Subtotalen toevoegen of verwijderen is één stap die u ongedaan kunt maken.

## Geavanceerd filter

Met de filterknoppen kiest u rijen één kolom per keer. **Geavanceerd** (in de groep **Sorteren en filteren**) laat u de voorwaarden op het blad schrijven, zodat ze zo lang kunnen zijn als nodig.

1. Kopieer naast de lijst de koppen van de kolommen die u wilt testen naar een rij lege cellen.
2. Schrijf onder elke kop een voorwaarde. Voorwaarden in **dezelfde rij** moeten allemaal waar zijn (dit is "en"). Voorwaarden in **verschillende rijen** zijn alternatieven: een rij van de lijst hoeft aan slechts één van die rijen te voldoen (dit is "of").
3. Kies **Geavanceerd**. Controleer of **Lijstbereik** de lijst met zijn kopregel is, en typ de cellen van uw koppen en voorwaarden bij **Voorwaardenbereik**.
4. Kies **De overige rijen verbergen, in de lijst**, of **Rijen naar een andere plaats kopiëren** en typ de eerste cel van de plek, zoals H1. Vink **Rijen overslaan die een eerdere rij herhalen** aan om er maar één van elk te kopiëren.
5. Klik op **OK**.

Zo schrijft u een voorwaarde:

- Een woord, zoals **An**, selecteert tekst die ermee begint. Hoofdletters en kleine letters tellen als hetzelfde. **?** staat voor elk willekeurig teken en ***** voor elke willekeurige reeks tekens.
- Een getal selecteert cellen die er gelijk aan zijn.
- **=Ann** selecteert tekst die precies Ann is. **=** alleen selecteert lege cellen.
- **>**, **>=**, **<**, **<=** en **<>** (niet gelijk aan) voor een getal of woord vergelijken ermee, zoals **>25**. **<>** alleen selecteert cellen die niet leeg zijn.

Een rij voorwaarden zonder inhoud selecteert elke rij. Om de verborgen rijen weer te tonen, kiest u **Wissen**. Een kopie wordt meteen gemaakt en verandert niet wanneer de lijst verandert. Geavanceerd filter is één stap die u ongedaan kunt maken.

## Gegevensvalidatie

Gebruik dit om vast te leggen wat in sommige cellen mag worden getypt, zodat een verkeerde invoer wordt tegengehouden of in twijfel getrokken.

1. Selecteer de cellen.
2. Kies **Gegevensvalidatie** in de groep **Hulpmiddelen voor gegevens** op het tabblad **Gegevens**.
3. Kies op het tabblad **Instellingen** wat u wilt **Toestaan**: **Elke waarde** (geen regel), **Geheel getal**, **Decimaal getal**, **Lijst**, **Datum**, **Tijd**, **Tekstlengte** of **Aangepast**.
4. Kies voor een getal, datum, tijd of tekstlengte bij **Gegevens** (**tussen**, **niet tussen**, **gelijk aan**, enzovoort) en typ de grenzen. Een grens kan een waarde zijn of een formule die met = begint.
5. Voor een **Lijst** typt u de keuzes met komma's ertussen, zoals Rood,Groen,Blauw, of typt u na = een celbereik, zoals =$A$1:$A$5. De cel toont dan een pijl met de keuzes. Schakel **Vervolgkeuzelijst in cel** uit om de pijl te verbergen.
6. Voor **Aangepast** typt u een formule die met = begint. De invoer is toegestaan wanneer de formule waar is.
7. **Lege cellen negeren** laat een lege cel door.

Vink op het tabblad **Invoerbericht** het vakje aan en schrijf een titel en een bericht. Ze worden naast de cel getoond wanneer die is geselecteerd.

Kies op het tabblad **Foutmelding** wat er gebeurt met een invoer die niet is toegestaan. **Stop** weigert de invoer. **Waarschuwing** vraagt of de invoer moet worden bewaard. **Informatie** informeert de gebruiker en bewaart de invoer. Schrijf een titel en een bericht voor de melding.

Met **Ongeldige gegevens markeren** (naast **Gegevensvalidatie**) zet u een rode ovaal om elke cel op het blad waarvan de waarde zijn regel niet toestaat, zoals een getal dat er al stond voordat de regel bestond, of het resultaat van een formule. Een bericht laat zien of alle cellen in orde zijn. Een cel die u hebt gecorrigeerd, verliest zijn ovaal. **Validatiemarkeringen wissen** haalt alle ovalen weg.

Met **Alles wissen** verwijdert u de regels van de geselecteerde cellen. De nieuwe regel vervangt elke regel die de geselecteerde cellen hadden; andere cellen houden de hunne. Regels wijzigen is één stap die u ongedaan kunt maken, en de regels worden in het bestand opgeslagen.

## Consolideren

Gebruik dit om bereiken die op elkaar lijken, zoals dezelfde tabel op meerdere bladen, tot één tabel te combineren.

1. Kies **Consolideren** in de groep **Hulpmiddelen voor gegevens** op het tabblad **Gegevens**.
2. Kies de **Functie** waarmee u wilt combineren, zoals **Som** of **Gemiddelde**.
3. Typ een bereik in het vak en klik op **Toevoegen**. Gebruik **Blad2!A1:C10** voor een bereik op een ander blad. Voeg elk bereik toe dat u wilt combineren. Selecteer een bereik in de lijst en klik op **Verwijderen** om het weg te halen.
4. Als de bereiken in hun eerste rij namen hebben, vinkt u **Bovenste rij bevat de namen van kolommen** aan. Als ze in hun eerste kolom namen hebben, vinkt u **Linker kolom bevat de namen van rijen** aan.
5. Typ bij **Resultaat plaatsen op** de eerste cel voor het resultaat.
6. Vink **Koppelingen naar de brongegevens maken** aan als elk resultaat een formule moet zijn die zijn bronnen leest, zodat het verandert wanneer zij veranderen. Zonder dit zijn de resultaten gewone getallen.
7. Klik op **OK**.

Zonder aangevinkte namen wordt de cel op dezelfde plek in elk bereik gecombineerd (de eerste cel van elk bereik, dan de tweede, enzovoort). Met aangevinkte namen worden cellen met dezelfde namen gecombineerd, waar ze in elk bereik ook staan, en een naam die alleen sommige bereiken hebben, krijgt nog steeds een rij of kolom. Consolideren is één stap die u ongedaan kunt maken.
