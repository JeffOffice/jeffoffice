# Gegevenshulpmiddelen
<!-- keywords: gegevens, tabblad gegevens, tekst naar kolommen, splitsen, scheidingsteken, komma, tab, vaste breedte, dubbele waarden verwijderen, doelzoeken, subtotaal, totalen, geavanceerd filter, voorwaarden, gegevensvalidatie, ongeldige gegevens markeren, rode ovaal, validatiemarkeringen wissen, consolideren, bereiken combineren, toestaan, lijst, vervolgkeuzelijst, invoerbericht, foutmelding, herhaalde rijen, uniek, tekstbestand, csv, uit tekst, kolommen, scheidingsteken, aanhalingstekens -->

Op het tabblad **Gegevens** staan de hulpmiddelen die werken op een lijst met waarden: tekst uit een bestand halen, een lijst sorteren, tekst in kolommen splitsen en herhaalde rijen verwijderen.

## Uit tekst/CSV

**Uit tekst/CSV** opent een tekstbestand (.csv, .tsv of .txt). Het is hetzelfde als **Tekstbestand openen** op het tabblad Bestand. JeffCrunch vraagt hoe het bestand in kolommen is verdeeld en wat voor soort tekst het bevat, en toont het daarna als een nieuwe werkmap.

## Sorteren en filteren

De groep **Sorteren en filteren** bevat dezelfde opdrachten als het menu **Sorteren en rijen filteren** op het tabblad Start. Zie [Sorteren en filteren](sort-filter).

## Tekst naar kolommen

Gebruik dit wanneer één kolom meerdere stukjes tekst bevat die u in afzonderlijke kolommen wilt hebben, zoals "Jansen, Ann, 31".

1. Selecteer de cellen van **één** kolom.
2. Kies **Tekst naar kolommen** op het tabblad **Gegevens**.
3. Kies hoe de tekst wordt gesplitst:
   - **Splitsen op een scheidingsteken**: vink de tekens aan die tussen de stukjes staan: **Tab**, **Puntkomma**, **Komma**, **Spatie** of **Andere**, waarbij u zelf het teken typt. **Scheidingstekens achter elkaar als één behandelen** voorkomt dat twee komma's achter elkaar een leeg stukje opleveren. **Tekst tussen aanhalingstekens** geeft het teken aan dat een stukje omsluit dat niet mag worden gesplitst, zoals "Lee, Bo". Binnen de aanhalingstekens tellen twee tekens achter elkaar als één.
   - **Splitsen op vaste posities**: typ de posities waar een nieuwe kolom begint, waarbij tekens vanaf 1 worden geteld. Bij 5, 12 begint bijvoorbeeld een nieuwe kolom bij het vijfde en bij het twaalfde teken.
4. Kies wat elk stukje is: **Algemeen** herkent getallen en datums zoals u ze hebt getypt, en **Tekst** behoudt elk stukje zoals het is, zodat "007" "007" blijft.
5. Typ bij **Delen plaatsen vanaf** de eerste cel voor de delen. Die staat standaard op de eerste cel die u hebt geselecteerd.
6. Het vak onder de keuzes toont de eerste rijen zoals u ze hebt gesplitst. Klik op **OK**.

De delen komen in de cellen rechts van de eerste cel, en ze vervangen wat er in die cellen stond. Tekst naar kolommen is één stap die u ongedaan kunt maken.

## Dubbele waarden verwijderen

Gebruik dit om van elke groep herhaalde rijen alleen de eerste te bewaren.

1. Selecteer de cellen van de lijst, of één cel erin. JeffCrunch neemt het hele gegevensblok rond één cel.
2. Kies **Dubbele waarden verwijderen**.
3. Vink **Mijn gegevens hebben koppen** aan als de eerste rij namen bevat. Die rij blijft staan en wordt niet vergeleken.
4. Vink de kolommen aan die met elkaar moeten overeenkomen, wil twee rijen als herhaling gelden. Twee rijen zijn alleen herhalingen wanneer alle aangevinkte kolommen gelijk zijn. Hoofdletters en kleine letters tellen als gelijk, en een formule telt als de waarde die hij oplevert.
5. Klik op **OK**. Het dialoogvenster meldt hoeveel rijen zijn verwijderd en hoeveel er zijn overgebleven.

De rijen onder een verwijderde rij schuiven omhoog binnen de geselecteerde cellen, met hun opmaak en formules. De cellen onder de selectie verschuiven niet. De lege rijen die onderaan de selectie overblijven, worden leeggemaakt. Dubbele waarden verwijderen is één stap die u ongedaan kunt maken.

## Doelzoeken

Gebruik dit wanneer u het gewenste antwoord kent, maar niet het getal dat het oplevert. Een betaalcel bevat bijvoorbeeld een formule met een rentevoet, en u wilt dat de betaling 500 is. Doelzoeken probeert getallen in de rentecel totdat de betaling 500 is.

1. Kies **Doelzoeken** op het tabblad **Gegevens**.
2. Typ bij **Cel met de formule** de cel met de formule, zoals B3. Die staat standaard op de cel die u hebt geselecteerd.
3. Typ bij **Gewenste waarde** het antwoord dat de formule moet geven.
4. Typ bij **Cel om te wijzigen** de cel die Doelzoeken mag wijzigen, zoals B1. Die moet een getal bevatten, geen formule.
5. Klik op **OK**.

Als Doelzoeken een getal vindt, blijft het in de cel om te wijzigen staan, en de statusbalk vertelt wat het is. Lukt het niet, dan verandert er niets en krijgt u daarvan bericht. Probeer een ander beginnummer in de cel om te wijzigen en controleer of de formule echt die cel gebruikt. Doelzoeken is één stap die u ongedaan kunt maken.

## Subtotaal

Gebruik dit om onder elke groep rijen die dezelfde waarde in één kolom hebben een rij met totalen te zetten, en onderaan een eindtotaal. De lijst heeft een koprij nodig en moet gesorteerd zijn op de kolom waarop u groepeert, zodat gelijke waarden naast elkaar staan.

1. Selecteer een cel in de lijst.
2. Kies **Subtotaal** in de groep **Overzicht** op het tabblad **Gegevens**.
3. Kies bij **Bij elke wijziging in** de kolom die de rijen groepeert, zoals Regio.
4. Kies bij **Functie gebruiken** wat moet worden berekend: **Som**, **Aantal**, **Gemiddelde**, **Max**, **Min** en andere.
5. Vink bij **Subtotaal toevoegen aan** elke kolom aan die moet worden opgeteld.
6. Laat **Huidige subtotalen vervangen** aangevinkt om bestaande subtotalen te vervangen. Klik op **OK**.

Elke totaalrij heet "Totaal oost" (of de waarde waarvoor hij geldt) en bevat een SUBTOTAAL-formule, zodat de andere totaalrijen worden overgeslagen. De rijen van elke groep worden gegroepeerd, zodat u met de knoppen 1, 2 en 3 naast de rijen alleen de totalen kunt tonen. Om de totalen weer weg te halen, opent u **Subtotaal** opnieuw en klikt u op **Alles verwijderen**. Subtotalen toevoegen of verwijderen is één stap die u ongedaan kunt maken.

## Geavanceerd filter

Met de filterknoppen kiest u rijen per kolom. Met **Geavanceerd** (in de groep **Sorteren en filteren**) schrijft u de voorwaarden op het blad zelf, zodat ze zo lang kunnen zijn als nodig.

1. Kopieer naast de lijst de koppen van de kolommen die u wilt controleren naar een rij lege cellen.
2. Schrijf onder elke kop een voorwaarde. Voorwaarden in **dezelfde rij** moeten alle waar zijn (dit is "en"). Voorwaarden in **verschillende rijen** zijn alternatieven: een rij van de lijst hoeft aan slechts één van die rijen te voldoen (dit is "of").
3. Kies **Geavanceerd**. Controleer of **Lijstbereik** de lijst met de koprij is, en typ bij **Voorwaardenbereik** de cellen van uw koppen en voorwaarden.
4. Kies **Andere rijen in de lijst verbergen**, of **Rijen naar een andere plaats kopiëren** en typ de eerste cel van die plaats, zoals H1. Vink **Rijen overslaan die een eerdere rij herhalen** aan om er maar één van elk te kopiëren.
5. Klik op **OK**.

Zo schrijft u een voorwaarde:

- Een woord, zoals **An**, selecteert tekst die ermee begint. Hoofdletters en kleine letters tellen als gelijk. **?** staat voor elk willekeurig teken en ***** voor een willekeurige reeks tekens.
- Een getal selecteert cellen die er gelijk aan zijn.
- **=Ann** selecteert tekst die precies Ann is. **=** alleen selecteert lege cellen.
- **>**, **>=**, **<**, **<=** en **<>** (niet gelijk aan) voor een getal of woord vergelijken ermee, zoals **>25**. **<>** alleen selecteert cellen die niet leeg zijn.

Een rij voorwaarden zonder inhoud selecteert alle rijen. Om de verborgen rijen weer te tonen, kiest u **Wissen**. Een kopie wordt meteen gemaakt en verandert niet wanneer de lijst verandert. Geavanceerd filter is één stap die u ongedaan kunt maken.

## Gegevensvalidatie

Gebruik dit om vast te leggen wat in sommige cellen mag worden getypt, zodat een verkeerde invoer wordt tegengehouden of in twijfel getrokken.

1. Selecteer de cellen.
2. Kies **Gegevensvalidatie** in de groep **Hulpmiddelen voor gegevens** op het tabblad **Gegevens**.
3. Kies op het tabblad **Instellingen** bij **Toestaan**: **Elke waarde** (geen regel), **Geheel getal**, **Decimaal getal**, **Lijst**, **Datum**, **Tijd**, **Tekstlengte** of **Aangepast**.
4. Kies voor een getal, datum, tijd of tekstlengte bij **Gegevens** (**tussen**, **niet tussen**, **gelijk aan**, enzovoort) en typ de grenzen. Een grens kan een waarde zijn of een formule die begint met =.
5. Typ voor een **Lijst** de keuzes met komma's ertussen, zoals Rood,Groen,Blauw, of typ na = een celbereik, zoals =$A$1:$A$5. De cel toont dan een pijl met de keuzes. Schakel **Vervolgkeuzelijst in cel** uit om de pijl te verbergen.
6. Typ bij **Aangepast** een formule die begint met =. De invoer is toegestaan wanneer de formule waar is.
7. Met **Lege cellen negeren** kan een lege cel zonder controle door.

Vink op het tabblad **Invoerbericht** het vakje aan en schrijf een titel en een bericht. Ze worden naast de cel getoond zodra die is geselecteerd.

Kies op het tabblad **Foutmelding** wat er gebeurt met een invoer die niet is toegestaan. **Stoppen** weigert de invoer. **Waarschuwing** vraagt of de invoer moet worden bewaard. **Informatie** laat de persoon weten dat de invoer niet klopt en bewaart die. Schrijf een titel en een bericht voor de melding.

**Ongeldige gegevens markeren** (naast **Gegevensvalidatie**) zet een rode ovaal om elke cel op het blad waarvan de waarde niet door de regel wordt toegestaan, zoals een getal dat er al stond voordat de regel bestond, of het resultaat van een formule. Een bericht laat u weten of alle cellen in orde zijn. Een cel die u hebt gecorrigeerd, verliest zijn ovaal. **Validatiemarkeringen wissen** haalt alle ovalen weg.

**Alles wissen** haalt de regels van de geselecteerde cellen af. De nieuwe regel vervangt alle regels die de geselecteerde cellen al hadden; andere cellen houden de hunne. Het wijzigen van regels is één stap die u ongedaan kunt maken, en de regels worden in het bestand opgeslagen.

## Consolideren

Gebruik dit om bereiken die op elkaar lijken, zoals dezelfde tabel op verschillende bladen, in één tabel te combineren.

1. Kies **Consolideren** in de groep **Hulpmiddelen voor gegevens** op het tabblad **Gegevens**.
2. Kies de **Functie** waarmee u combineert, zoals **Som** of **Gemiddelde**.
3. Typ een bereik in het vak en klik op **Toevoegen**. Gebruik **Blad2!A1:C10** voor een bereik op een ander blad. Voeg elk bereik toe dat u wilt combineren. Selecteer een bereik in de lijst en druk op **Verwijderen** om het weg te halen.
4. Als de bereiken namen in hun eerste rij hebben, vink dan **Bovenste rij bevat de kolomnamen** aan. Als ze namen in hun eerste kolom hebben, vink dan **Linker kolom bevat de rijnamen** aan.
5. Typ bij **Resultaat plaatsen op** de eerste cel voor het resultaat.
6. Vink **Koppelingen naar de brongegevens maken** aan als elk resultaat een formule moet zijn die de bronnen leest, zodat het verandert wanneer zij veranderen. Zonder dit vinkje zijn de resultaten gewone getallen.
7. Klik op **OK**.

Zonder aangevinkte namen wordt de cel op dezelfde plaats in elk bereik gecombineerd (eerst de eerste cel van elk bereik, dan de tweede, enzovoort). Met aangevinkte namen worden cellen met dezelfde namen gecombineerd, waar ze ook in elk bereik staan, en een naam die maar in sommige bereiken voorkomt, krijgt toch een rij of kolom. Consolideren is één stap die u ongedaan kunt maken.
