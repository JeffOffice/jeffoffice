# Afdruk samenvoegen, enveloppen en etiketten
<!-- keywords: afdruk samenvoegen, samenvoegen, verzendlijsten, geadresseerden, adressenlijst, standaardbrief, etiketten, enveloppen, samenvoegveld, adresblok, begroetingsregel, regels, als dan anders, record overslaan, volgend record, voorbeeld van resultaten, voltooien en samenvoegen, adreslijst, e-mail, csv, xlsx, ods, vcard, envelop, etiket, samenvoegen, mailing, brief, verzenden naar, contactpersonen, lijst, adreslijst maken, vervolgbrief -->

Met **afdruk samenvoegen** maakt u veel kopieën van één brief, elk met de gegevens van een andere persoon. U schrijft de brief één keer. U bewaart de gegevens van de personen in een **lijst**. JeffWrite zet de naam en het adres van de eerste persoon in de eerste kopie, die van de tweede persoon in de tweede kopie, en zo verder.

De hulpmiddelen staan op het tabblad **Verzendlijsten**. Voor een begeleide weg klikt u op **Verzendlijsten > Afdrukvoorbereiding starten > Stapsgewijze wizard Afdruk samenvoegen**.

## Kies wat u wilt maken

Klik op **Verzendlijsten > Afdrukvoorbereiding starten** en kies een van de volgende opties:

- **Brieven**: één kopie van de pagina voor elke persoon. Elke kopie begint op een nieuwe pagina.
- **E-mailberichten**: één bericht voor elke persoon. Zie "E-mailberichten verzenden" hieronder.
- **Enveloppen** en **Etiketten**: een pagina die hiervoor is gemaakt. Zie de onderstaande secties.
- **Catalogus**: één lijst van iedereen, zonder paginawissel tussen de personen. Gebruik dit voor een telefoonlijst of een catalogus.
- **Samenvoegtype wissen** maakt het document weer een gewoon document.

JeffWrite bewaart de soort samenvoeging en de naam van de lijst in het bestand wanneer u het opslaat. Wanneer u het bestand weer opent, is de samenvoeging er nog.

## Kies uw geadresseerden

Een **geadresseerde** is een persoon die een kopie krijgt. Klik op **Verzendlijsten > Geadresseerden selecteren**:

- **Nieuwe lijst typen**: typ de personen in het venster dat opent. Klik voor elk persoon op **Nieuwe vermelding**. JeffWrite biedt daarna aan de lijst op te slaan als csv-bestand.
- **Bestaande lijst gebruiken**: kies een bestand dat al een lijst is. Dat kan een csv-bestand zijn of een tekstbestand met tabs (één rij per persoon, en de eerste rij bevat de kolomnamen), een spreadsheet (.xlsx, .xls of .ods, uit JeffCrunch of een ander programma), of een bestand met contactpersonen (.vcf).
- **Selecteren uit contactpersonen**: kies een bestand met contactpersonen (.vcf). De meeste adresboeken en telefoons kunnen hun contactpersonen als zodanig bestand opslaan.

Klik daarna op **Verzendlijsten > Adressenlijst bewerken** om de lijst te controleren:

- Schakel een rij uit om die persoon over te slaan.
- Klik op een kolomkop om te sorteren.
- Typ in **Geadresseerden filteren** om alleen de rijen te tonen die uw woorden bevatten.
- **Ontvangers onderscheiden op basis van** kiest de kolom die personen van elkaar onderscheidt, zoals een e-mailadres. Wanneer u het document opnieuw opent en de lijst wordt gelezen, blijven de personen die u hebt uitgeschakeld weer buiten de samenvoeging, ook als de lijst een andere volgorde heeft of er nieuwe personen zijn. Met **Plaatsen in de lijst** worden ze onthouden aan de hand van hun rijnummer in het lijstbestand, ook als u de rijen in dit venster hebt gesorteerd of gefilterd. Als de lijst zo is veranderd dat dit niet meer klopt, meldt JeffWrite dat en neemt iedereen op die het niet kon koppelen. Het document bewaart een gecodeerde vorm van elk adres, niet het adres zelf, maar iemand die het document heeft en een adres raadt, kan die raad nog wel controleren. **Document controleren** (Bestand > Info) verwijdert de registratie van wie is uitgeschakeld.
- **Dubbele vermeldingen zoeken** schakelt herhalingen uit.
- **Adressen valideren** kleurt de rijen die geen adres hebben of een postcode die onjuist lijkt.

## Zet de gegevens in de brief

Een **samenvoegveld** is een tijdelijke aanduiding. Het toont de naam van een kolom tussen dubbele punthaken, zoals «Voornaam». In elke kopie wordt het vervangen door de gegevens van die persoon.

Klik waar het gegeven moet komen en gebruik daarna **Verzendlijsten > Velden schrijven en invoegen**:

- **Samenvoegveld invoegen** toont de kolommen van uw lijst. Kies er een.
  - **Streepjescode uit veld** staat in dezelfde lijst. Dit zet een streepjescode in waarvan de waarde een kolom is, zoals een artikelnummer of het 13-cijferige ISBN van een boek. Kies de kolom, kies daarna het type en kijk in het venster **Streepjescode invoegen** (wat u daar typt is alleen een voorbeeld). Het venster biedt alleen wat een veld kan bevatten: het type, de grootte, de staafhoogte, de cijfers en het QR-niveau. Het laat de prijs van een boek, een toevoeging, een controleteken en de transparante achtergrond weg, omdat een veld dat niet kan aangeven; het ISBN van een boek is een EAN-13. In het document wordt het veld als streepjescode getekend, als voorbeeld van het type totdat u samenvoegt. Om het type of de vormgeving te wijzigen, dubbelklikt u erop of klikt u er met de rechtermuisknop op en kiest u **Streepjescode bewerken**. Wanneer u samenvoegt, krijgt de kopie van elke persoon een streepjescode met de eigen waarde van die persoon, als afbeelding in de regel. Zo wordt hij mee afgedrukt en opgeslagen als pdf, net als elke andere afbeelding. De waarde is alleen gegevens: aanhalingstekens of een backslash erin veranderen niets. Is een waarde leeg, dan is er geen streepjescode. Kan een code de waarde niet bevatten (een letter in een EAN-13, of meer dan 500 tekens in Code 128), dan toont de kopie in de plaats daarvan een korte melding tussen vierkante haken; de andere kopieën worden daar niet door beïnvloed. Eén samenvoeging maakt hoogstens 2.000 streepjescodes, waarbij ook de waarden worden geteld die niet kunnen worden weergegeven. Als er meldt dat het er te veel zijn, voegt u minder personen samen. In een opgeslagen `.docx`-bestand is het veld een DISPLAYBARCODE-veld met de kolom erin, zodat het behouden blijft wanneer u het bestand opslaat en opnieuw opent. Een veld met een handmatig ingevoerde waarde, of van een type dat JeffWrite niet tekent (UPC-E bijvoorbeeld), blijft zoals het is. Bij samenvoegingen van e-mail wordt de waarde als tekst ingevoegd.
- **Adresblok** zet een volledig postadres in: de naam, het bedrijf, de straat en de plaats. Kies de regels die u wilt. Een regel die leeg is, wordt weggelaten. U kunt ook een land opgeven dat moet worden weggelaten, bijvoorbeeld uw eigen land. Met **Voorbeeld** ziet u het adres van de eerste persoon.
- **Begroetingsregel** zet een regel in zoals "Geachte dr. Fenwick,". U kiest de woorden. De eerste keuze, "Geachte dr. Fenwick,", gebruikt de titel van de persoon. Heeft iemand geen titel, dan gebruikt het de voor- en achternaam, zoals in "Geachte Nora Pell,". Heeft iemand geen naam, dan staat er iets anders in de begroeting, zoals "Geachte heer/mevrouw,", wat u kunt wijzigen.
- **Samenvoegvelden koppelen** vertelt het adresblok en de begroetingsregel welke kolom welk onderdeel bevat, wanneer uw kolomnamen ongebruikelijk zijn.

## Regels

Een **regel** maakt een kopie anders voor sommige personen. Klik op **Regels** en kies er een:

- **Als…Dan…Anders**: toon de ene tekst wanneer een kolom een waarde heeft, en de andere tekst wanneer dat niet zo is. Bijvoorbeeld: als Provincie "Utrecht" is, schrijf dan "Welkom thuis".
- **Record overslaan als**: laat een persoon weg wanneer een kolom voldoet aan een test die u opgeeft, zoals wanneer de achternaam leeg is.
- **Volgend record** en **Volgend record als**: gebruik de gegevens van de volgende persoon op dezelfde plaats. Etiketten gebruiken dit om op elk etiket een andere persoon te zetten.
- **Nummer geadresseerde** zet het rijnummer van de persoon in de lijst. **Volgnummer samenvoeging** telt de tot dan toe gemaakte kopieën.
- **Vraag** en **Invulveld**: vragen u iets wanneer u de samenvoeging voltooit. Vraag bewaart het antwoord onder een naam. Invulveld zet het antwoord in de tekst.
- **Bladwijzer instellen**: geeft een naam een waarde. Een kruisverwijzing naar de naam toont de waarde.

## Voorbeeld van resultaten

Klik op **Verzendlijsten > Voorbeeld van resultaten**. Er opent een deelvenster dat de pagina voor één persoon toont. Gebruik **Eerste record**, **Vorig record**, **Volgend record** en **Laatste record** om door de personen te bladeren, of typ een nummer. Met **Geadresseerde zoeken** springt u naar de volgende persoon die uw woorden in een gegeven heeft.

Met **Samenvoegvelden markeren** worden de samenvoegvelden in uw pagina grijs gearceerd, zodat u ziet waar ze staan. Andere velden, zoals paginanummers, worden niet gearceerd. De arcering is op het scherm zichtbaar en wordt nooit afgedrukt.

Uw eigen document wordt niet gewijzigd. **Fouten controleren** meldt wanneer de lijst geen kolom heeft voor een veld, of wanneer niemand is opgenomen.

## De samenvoeging voltooien

Klik op **Verzendlijsten > Voltooien en samenvoegen**:

- **Afzonderlijke documenten bewerken** maakt een nieuw document met de kopie van elke persoon. U kunt het wijzigen voordat u afdrukt.
- **Documenten afdrukken** maakt dat document en opent de pagina **Afdrukken**.
- **Opslaan als pdf** slaat elke kopie op in één pdf-bestand.
- **E-mailberichten verzenden** maakt voor elke persoon één berichtbestand (.eml) in een map die u kiest. Kies de kolom met de e-mailadressen en typ een onderwerp. JeffWrite verstuurt de berichten niet. Open de bestanden in uw e-mailprogramma.
- **Elk opslaan als bestand** maakt voor elke persoon een bestand in een map die u kiest, als .docx-bestand of als pdf. Geef de bestanden namen met kolommen tussen accolades, zoals {Achternaam}, {Voornaam}; een naam die twee keer voorkomt, krijgt een nummer. Een vak toont de voornamen en hoeveel bestanden er worden gemaakt. U wordt gevraagd voordat het meer dan 200 bestanden zijn, en eenmaal als er al bestanden met dezelfde namen in de map staan. Klik op **Annuleren** om tussen bestanden te stoppen: de al gemaakte bestanden blijven bewaard; het venster sluiten heeft hetzelfde effect. Een bestand dat een koppeling is naar een ander bestand, wordt vervangen door een nieuw bestand, en het bestand waarnaar het verwees, wordt niet aangeraakt. De bestanden bevatten alleen de brief voor één persoon: ze bevatten niet de lijst, de locatie ervan of wie is overgeslagen. Eén samenvoeging maakt hoogstens 100.000 bestanden.

De personen die u in de lijst hebt uitgeschakeld, worden overgeslagen.

Een berichtbestand wordt alleen gemaakt voor een persoon wiens adres één e-mailadres is. Bevat een adres er twee (met een komma of puntkomma ertussen), staat er een naam bij, of ontbreekt het domein, dan krijgt die persoon geen bericht, en JeffWrite meldt hoeveel dat er zijn. Een lang onderwerp wordt in stukken opgeslagen die e-mailprogramma's weer samenvoegen.

Een samenvoeging toont een voortgangsvenster, en met **Annuleren** stopt u het. Als een regel een vraag stelt (Vraag of Invulveld), wordt u elke verschillende vraag één keer gesteld, tot 20 vragen. Daarna gebruiken de rest de standaardantwoorden. Als u een vraag annuleert, stopt de hele samenvoeging. Een samenvoeging stopt, en meldt dat, als ze meer dan 10.000 pagina's zou maken, zodat een zeer lang document met een zeer lange lijst niet uren duurt. Voeg dan een deel van de lijst tegelijk samen.

## Enveloppen

Klik op **Verzendlijsten > Enveloppen**. Typ het bezorgadres en uw retouradres, en kies het formaat van de envelop uit de lijst. De formaten staan vermeld met hun breedte en hoogte. Klik op **Aan document toevoegen** om de envelop op een eigen pagina aan het begin van uw document te zetten, of op **Afdrukken** om hem af te drukken. De pagina is zo breed als de lange zijde van de envelop, zoals de meeste printers een envelop aannemen.

Om enveloppen voor een samenvoeging te maken, klikt u op **Afdrukvoorbereiding starten > Enveloppen**. Elke envelop krijgt een adresblok.

## Etiketten

Klik op **Verzendlijsten > Etiketten**. Typ het adres en kies het vel uit de lijst. Een vel wordt genoemd naar hoeveel etiketten het bevat en hoe groot elk etiket is. Staat uw vel niet in de lijst, kies dan **Eigen vel** en typ de afmetingen: hoeveel etiketten er naast en onder elkaar staan, hoe groot een etiket is, de ruimte boven en naast het eerste etiket, en hoe ver de etiketten van elkaar af staan.

- **Volledige pagina met hetzelfde etiket** zet hetzelfde adres op elk etiket van het vel.
- **Enkel etiket** zet het adres op één etiket, zodat u een gedeeltelijk gebruikt vel kunt opmaken. Geef de rij en kolom op.

Klik op **Nieuw document** om een document te maken dat u kunt bekijken en wijzigen, of op **Afdrukken** om het af te drukken.

Om etiketten voor een samenvoeging te maken, klikt u op **Afdrukvoorbereiding starten > Etiketten**. Het eerste etiket heeft een adresblok, en elk ander etiket begint met een regel **Volgend record**. Als u het eerste etiket wijzigt, klikt u op **Etiketten bijwerken** om de wijziging naar alle andere te kopiëren.

## Wat het bestand bevat

De soort samenvoeging, de naam en plaats van de lijst, het onderwerp en de e-mailkolom worden in uw .docx-bestand opgeslagen. Welke personen u hebt uitgeschakeld, wordt ook opgeslagen, als korte codes van de kolom die u koos, niet als hun namen of adressen (alleen een .docx-bestand bewaart dat; Compatibiliteit controleren zegt dat voor de andere soorten). Uw keuzes bij Samenvoegvelden koppelen worden eveneens opgeslagen. Een lijst wordt elke keer opnieuw gelezen uit zijn bestand. Staat het bestand niet meer op de plek waar het stond, dan zoekt JeffWrite naast uw document, en vraagt daarna of u het zelf wilt kiezen.

Een lijst kan in totaal tot 1.000 kolommen en 4.000.000 cellen hebben, en een samenvoeging gebruikt de eerste 100.000 personen. Heeft een bestand er meer, dan gebruikt JeffWrite wat erin past en meldt dat. Rijen die leeg zijn, worden uit de lijst weggelaten.

## Zie ook

- [Velden gebruiken](fields)
- [Afdrukken](print)
- [Tabellen maken en wijzigen](tables)
