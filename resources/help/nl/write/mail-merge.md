# Afdrukken samenvoegen, enveloppen en etiketten
<!-- keywords: afdruk samenvoegen, samenvoegen, verzendlijsten, geadresseerden, adressenlijst, standaardbrief, etiketten, enveloppen, samenvoegveld, adresblok, begroetingsregel, regels, als dan anders, record overslaan, volgende record, voorbeeld van resultaten, voltooien en samenvoegen, adreslijst, e-mail, csv, xlsx, ods, vcard -->

Met **afdrukken samenvoegen** maakt u veel kopieën van één brief, elk met de gegevens van een andere persoon erin. U schrijft de brief één keer. U bewaart de gegevens van de personen in een **lijst**. JeffWrite zet de naam en het adres van de eerste persoon in de eerste kopie, die van de tweede persoon in de tweede kopie, en zo verder.

De hulpmiddelen staan op het tabblad **Verzendlijsten**. Voor een stapsgewijze route klikt u op **Verzendlijsten > Afdrukvoorbereiding starten > Stapsgewijze wizard Afdruk samenvoegen**.

## Kies wat u gaat maken

Klik op **Verzendlijsten > Afdrukvoorbereiding starten** en kies er één:

- **Brieven**: één kopie van de pagina voor elke persoon. Elke kopie begint op een nieuwe pagina.
- **E-mailberichten**: één bericht voor elke persoon. Zie "E-mailberichten verzenden" hieronder.
- **Enveloppen** en **Etiketten**: een pagina die daarvoor is gemaakt. Zie de onderdelen hieronder.
- **Adreslijst**: één lijst van iedereen, zonder paginawisseling tussen de personen. Gebruik dit voor een telefoonlijst of een catalogus.
- **Samenvoegtype wissen** maakt van het document weer een gewoon document.

JeffWrite bewaart de soort samenvoeging en de naam van de lijst in het bestand wanneer u het opslaat. Wanneer u het bestand later opent, is de samenvoeging nog aanwezig.

## Kies uw geadresseerden

Een **geadresseerde** is een persoon die een kopie krijgt. Klik op **Verzendlijsten > Geadresseerden selecteren**:

- **Een nieuwe lijst typen**: typ de personen in het venster dat opent. Klik voor elke persoon op **Nieuwe vermelding**. JeffWrite biedt daarna aan de lijst op te slaan als csv-bestand.
- **Bestaande lijst gebruiken**: kies een bestand dat al een lijst is. Dat kan een csv-bestand zijn of een tekstbestand met tabs (één rij voor elke persoon, en de eerste rij bevat de kolomnamen), een spreadsheet (.xlsx, .xls of .ods, van JeffCrunch of een ander programma), of een contactenbestand (.vcf).
- **Selecteren uit contactpersonen**: kies een contactenbestand (.vcf). De meeste adresboeken en telefoons kunnen hun contactpersonen als zodanig opslaan.

Klik vervolgens op **Verzendlijsten > Adressenlijst bewerken** om de lijst te controleren:

- Schakel een rij uit om die persoon weg te laten.
- Klik op een kolomkop om te sorteren.
- Typ in **Geadresseerden filteren** om alleen de rijen te tonen die uw woorden bevatten.
- **Dubbele vermeldingen zoeken** schakelt herhalingen uit.
- **Adressen valideren** kleurt de rijen die geen adres hebben of een postcode die onjuist lijkt.

## Zet de gegevens in de brief

Een **samenvoegveld** is een tijdelijke aanduiding. Het toont de naam van een kolom tussen dubbele punthaken, zoals «Voornaam». In elke kopie wordt het vervangen door het gegeven van die persoon.

Klik waar het gegeven moet komen en gebruik daarna **Verzendlijsten > Velden schrijven en invoegen**:

- **Samenvoegveld invoegen** toont de kolommen van uw lijst. Kies er één.
  - **Streepjescode uit veld** staat in dezelfde lijst en zet een streepjescode in waarvan de waarde een kolom is, zoals een artikelnummer of het 13-cijferige ISBN van een boek. Kies de kolom, kies daarna het type en controleer het venster **Streepjescode invoegen** (wat u daar typt, is alleen een voorbeeld). Het venster biedt alleen wat een veld kan bevatten: het type, de grootte, de streephoogte, de cijfers en het QR-niveau. Het laat de prijs van een boek, een toevoeging, een controleteken en de transparante achtergrond weg, omdat een veld die niet kan aangeven; het ISBN van een boek is een EAN-13. In het document wordt het veld als streepjescode getekend, als voorbeeld van het type totdat u samenvoegt. Om het type of de weergave te wijzigen, dubbelklikt u erop of klikt u er met de rechtermuisknop op en kiest u **Streepjescode bewerken**. Wanneer u samenvoegt, krijgt de kopie van elke persoon een streepjescode met de eigen waarde van die persoon, als afbeelding in de regel getekend. Die wordt dus mee afgedrukt en opgeslagen in pdf, net als elke andere afbeelding. De waarde is alleen gegevens: aanhalingstekens of een backslash erin veranderen niets. Is een waarde leeg, dan is er geen streepjescode. Kan een code de waarde niet bevatten (een letter in een EAN-13, of meer dan 500 tekens in Code 128), dan toont de kopie in de plaats daarvan een korte melding tussen vierkante haken; de andere kopieën worden daar niet door beïnvloed. Eén samenvoeging maakt hoogstens 2.000 streepjescodes, waarbij elke waarde die niet kan worden verwerkt meetelt; voeg minder personen samen als er wordt gemeld dat het er te veel zijn. In een opgeslagen .docx-bestand is het veld een DISPLAYBARCODE-veld met de kolom erin, zodat het bewaard blijft wanneer u het bestand opslaat en opnieuw opent. Een veld met een ingetypte waarde, of van een type dat JeffWrite niet tekent (bijvoorbeeld UPC-E), blijft zoals het is. Bij e-mailsamenvoegingen wordt de waarde als tekst ingevoegd.
- **Adresblok** zet een volledig postadres in: de naam, het bedrijf, de straat en de plaats. Kies de regels die u wilt. Een regel die leeg is, wordt weggelaten. U kunt ook een land opgeven dat moet worden weggelaten, zoals uw eigen land. Met **Voorbeeld** ziet u het adres van de eerste persoon.
- **Begroetingsregel** zet een regel in zoals "Geachte dr. Fenwick,". U kiest de woorden. De eerste keuze, "Geachte dr. Fenwick,", gebruikt de titel van de persoon. Heeft een persoon geen titel, dan gebruikt de regel de voor- en achternaam, zoals in "Geachte Nora Pell,". Heeft een persoon geen naam, dan zegt de begroeting iets anders, zoals "Geachte heer/mevrouw,", wat u kunt wijzigen.
- **Velden koppelen** vertelt het adresblok en de begroetingsregel welke kolom welk onderdeel bevat, wanneer uw kolomnamen ongewoon zijn.

## Regels

Een **regel** maakt een kopie voor sommige personen anders. Klik op **Regels** en kies er één:

- **Als…Dan…Anders**: toon één tekst wanneer een kolom een waarde heeft, en een andere tekst wanneer dat niet zo is. Bijvoorbeeld: als Provincie "CA" is, schrijf "Welkom thuis".
- **Record overslaan als**: laat een persoon weg wanneer een kolom een waarde heeft, bijvoorbeeld wanneer de achternaam leeg is.
- **Volgende record** en **Volgend record als**: gebruik de gegevens van de volgende persoon op dezelfde plaats. Etiketten gebruiken dit om op elk etiket een andere persoon te zetten.
- **Samenvoegrecord nr.** zet het nummer van de rij van de persoon in de lijst. **Samenvoegvolgnummer #** telt de kopieën die tot nu toe zijn gemaakt.
- **Vragen** en **Invulveld**: vragen u iets wanneer u het samenvoegen voltooit. Vragen bewaart het antwoord onder een naam. Invulveld zet het antwoord in de tekst.
- **Bladwijzer instellen**: geeft een naam een waarde. Een kruisverwijzing naar de naam toont de waarde.

## Bekijk de resultaten

Klik op **Verzendlijsten > Voorbeeld van resultaten**. Er opent een deelvenster dat de pagina voor één persoon toont. Gebruik **Eerste record**, **Vorige record**, **Volgende record** en **Laatste record** om door de personen te bladeren, of typ een nummer. **Ontvanger zoeken** springt naar de volgende persoon die uw woorden in een gegeven bevat.

**Samenvoegvelden markeren** arceert de samenvoegvelden in uw pagina grijs, zodat u ziet waar ze staan. Andere velden, zoals paginanummers, worden niet gearceerd. De arcering is op het scherm zichtbaar en wordt nooit afgedrukt.

Uw eigen document wordt niet gewijzigd. **Controleren op fouten** meldt wanneer de lijst geen kolom heeft voor een veld, of wanneer niemand is opgenomen.

## Voltooi het samenvoegen

Klik op **Verzendlijsten > Voltooien en samenvoegen**:

- **Afzonderlijke documenten bewerken** maakt een nieuw document met de kopie van elke persoon. U kunt het wijzigen voordat u afdrukt.
- **Documenten afdrukken** maakt dat document en opent de pagina **Afdrukken**.
- **Opslaan als pdf** slaat alle kopieën op in één pdf-bestand.
- **E-mailberichten verzenden** maakt voor elke persoon een berichtbestand (.eml) in een map die u kiest. Kies de kolom met de e-mailadressen en typ een onderwerp. JeffWrite verstuurt de berichten niet. Open de bestanden in uw e-mailprogramma.

De personen die u in de lijst hebt uitgeschakeld, worden weggelaten.

Een berichtbestand wordt alleen gemaakt voor een persoon wiens adres één e-mailadres is. Bevat een adres er twee (met een komma of puntkomma ertussen), staat er een naam bij, of ontbreekt het domein, dan krijgt die persoon geen bericht, en JeffWrite meldt hoeveel het er zijn. Een lang onderwerp wordt in stukken opgeslagen die e-mailprogramma's weer samenvoegen.

Bij het samenvoegen verschijnt een voortgangsvenster en met **Annuleren** stopt u het. Als een regel een vraag stelt (Vragen of Invulveld), wordt u elke verschillende vraag één keer gesteld, tot 20 vragen. Daarna gebruiken de rest hun standaardantwoorden. Als u een vraag annuleert, stopt het hele samenvoegen. Het samenvoegen stopt, en dat wordt gemeld, als er meer dan 10.000 pagina's zouden ontstaan, zodat een heel lang document met een heel lange lijst niet uren duurt. Voeg dan een deel van de lijst tegelijk samen.

## Enveloppen

Klik op **Verzendlijsten > Enveloppen**. Typ het bezorgadres en uw retouradres, en kies het formaat van de envelop uit de lijst. De formaten staan vermeld met hun breedte en hoogte. Klik op **Aan document toevoegen** om de envelop op een eigen pagina aan het begin van uw document te zetten, of op **Afdrukken** om hem af te drukken. De pagina is zo breed als de lange zijde van de envelop, zoals de meeste printers een envelop aannemen.

Om enveloppen voor een samenvoeging te maken, klikt u op **Afdrukvoorbereiding starten > Enveloppen**. Elke envelop heeft een adresblok.

## Etiketten

Klik op **Verzendlijsten > Etiketten**. Typ het adres en kies het vel uit de lijst. Een vel wordt benoemd naar het aantal etiketten erop en de grootte van elk etiket. Staat uw vel niet in de lijst, kies dan **Eigen vel** en typ de afmetingen: hoeveel etiketten naast elkaar en onder elkaar staan, hoe groot een etiket is, de ruimte boven en naast het eerste etiket, en hoe ver de etiketten van elkaar staan.

- **Volledige pagina met hetzelfde etiket** zet hetzelfde adres op elk etiket van het vel.
- **Enkel etiket** zet het adres op één etiket, zodat u een vel dat al deels is gebruikt, kunt opmaken. Geef de rij en kolom op.

Klik op **Nieuw document** om een document te maken dat u kunt bekijken en wijzigen, of op **Afdrukken** om het af te drukken.

Om etiketten voor een samenvoeging te maken, klikt u op **Afdrukvoorbereiding starten > Etiketten**. Het eerste etiket heeft een adresblok, en elk ander etiket begint met een regel **Volgende record**. Als u het eerste etiket wijzigt, klikt u op **Etiketten bijwerken** om de wijziging op alle andere te zetten.

## Wat het bestand bevat

De soort samenvoeging, de naam en plaats van de lijst, het onderwerp en de e-mailkolom worden opgeslagen in uw .docx-bestand. Welke personen u hebt uitgeschakeld, en uw keuzes bij Velden koppelen, worden niet opgeslagen. Die worden opnieuw gevraagd nadat u het bestand opent. Een lijst wordt elke keer opnieuw uit zijn bestand gelezen. Staat het bestand niet meer op de plek waar het stond, dan zoekt JeffWrite naast uw document, en vraagt daarna om het bestand te kiezen.

Een lijst kan in totaal maximaal 1.000 kolommen en 4.000.000 cellen hebben, en een samenvoeging gebruikt de eerste 100.000 personen. Heeft een bestand meer, dan gebruikt JeffWrite wat erin past en meldt dat. Rijen zonder inhoud worden uit de lijst weggelaten.

## Zie ook

- [Velden gebruiken](fields)
- [Afdrukken](print)
- [Tabellen maken en wijzigen](tables)
