# Samenvoegen, enveloppen en etiketten
<!-- keywords: afdruk samenvoegen, samenvoegen, verzendlijsten, geadresseerden, adressenlijst, standaardbrief, etiketten, enveloppen, samenvoegveld, adresblok, begroetingsregel, regels, als dan anders, record overslaan, volgend record, voorbeeld van resultaten, voltooien en samenvoegen, catalogus, e-mailberichten, csv, xlsx, ods, vcard, streepjescode, barcode -->

Met afdruk samenvoegen maakt u veel kopieën van één brief, elk met de gegevens van een andere persoon. U schrijft de brief één keer. De gegevens van de personen bewaart u in een **lijst**. JeffWrite zet de naam en het adres van de eerste persoon in de eerste kopie, die van de tweede persoon in de tweede kopie, enzovoort.

De hulpmiddelen vindt u op het tabblad **Verzendlijsten**. Voor een stapsgewijze aanpak klikt u op **Verzendlijsten > Afdrukvoorbereiding starten > Stapsgewijze wizard Afdruk samenvoegen**.

## Kies wat u maakt

Klik op **Verzendlijsten > Afdrukvoorbereiding starten** en kies:

- **Brieven**: één kopie van de pagina voor elke persoon. Elke kopie begint op een nieuwe pagina.
- **E-mailberichten**: één bericht voor elke persoon. Zie "E-mailberichten verzenden" hieronder.
- **Enveloppen** en **Etiketten**: een pagina die daarvoor wordt gemaakt. Zie de onderdelen hieronder.
- **Catalogus**: één lijst met iedereen, zonder paginawisseling tussen de personen. Gebruik die voor een telefoonlijst of een catalogus.
- **Samenvoegtype wissen** maakt het document weer een gewoon document.

JeffWrite bewaart de soort samenvoeging en de naam van de lijst in het bestand wanneer u het opslaat. Wanneer u het bestand weer opent, is de samenvoeging er nog.

## Kies uw geadresseerden

Een **geadresseerde** is een persoon die een kopie krijgt. Klik op **Verzendlijsten > Adressen selecteren**:

- **Een nieuwe lijst typen**: typ de personen in het venster dat opent. Klik voor elke persoon op **Nieuwe vermelding**. JeffWrite biedt daarna aan de lijst op te slaan als csv-bestand.
- **Een bestaande lijst gebruiken**: kies een bestand dat al een lijst is. Dat kan een csv-bestand zijn of een tekstbestand met tabs (één rij voor elke persoon, en de eerste rij bevat de kolomnamen), een spreadsheet (.xlsx, .xls of .ods, van JeffCrunch of een ander programma), of een contactpersonenbestand (.vcf).
- **Selecteren uit contactpersonen**: kies een contactpersonenbestand (.vcf). De meeste adresboeken en telefoons kunnen hun contactpersonen als zo'n bestand opslaan.

Klik daarna op **Verzendlijsten > Adressenlijst bewerken** om de lijst te controleren:

- Haal het vinkje weg bij een rij om die persoon over te slaan.
- Klik op een kolomkop om te sorteren.
- Typ in **Geadresseerden filteren** om alleen de rijen te tonen die uw woorden bevatten.
- **Dubbele vermeldingen zoeken** haalt het vinkje weg bij herhalingen.
- **Adressen valideren** kleurt de rijen die geen adres hebben of een postcode die er onjuist uitziet.

## Zet de gegevens in de brief

Een **samenvoegveld** is een plaatshouder. Het toont de naam van een kolom tussen dubbele punthaken, zoals «Voornaam». In elke kopie wordt het vervangen door de gegevens van die persoon.

Klik waar de gegevens moeten komen en gebruik daarna **Verzendlijsten > Velden schrijven en invoegen**:

- **Samenvoegveld invoegen** toont de kolommen van uw lijst. Kies er een.
  - **Streepjescode uit veld**, in dezelfde lijst, zet een streepjescode in waarvan de waarde een kolom is, zoals een artikelnummer of het 13-cijferige ISBN van een boek. Kies de kolom, kies daarna het type en kijk in het venster **Streepjescode invoegen** (wat u daar typt is alleen een voorbeeld). Het venster biedt alleen wat een veld kan dragen: het type, de grootte, de hoogte van de streepjes, de cijfers en het QR-niveau. Het laat een prijs van een boek, een toevoeging, een controleteken en de transparante achtergrond weg, omdat een veld dat niet kan aangeven; een ISBN van een boek is een EAN-13. In het document toont het veld de kolomnaam tussen guillemets. Bij samenvoegen krijgt de kopie van elke persoon een streepjescode met de eigen waarde van die persoon, als afbeelding in de regel getekend, zodat die net als elke andere afbeelding wordt afgedrukt en naar pdf wordt opgeslagen. De waarde is alleen gegevens: aanhalingstekens of een backslash erin veranderen niets. Is een waarde leeg, dan komt er geen streepjescode. Kan een code de waarde niet bevatten (een letter in een EAN-13, of meer dan 500 tekens in Code 128), dan toont de kopie op die plaats een korte melding tussen vierkante haken; de andere kopieën worden daardoor niet beïnvloed. Eén samenvoeging maakt hoogstens 2.000 streepjescodes, waarbij ook elke waarde telt die niet past; voeg minder personen samen als er staat dat het er te veel zijn. In een opgeslagen `.docx` is het veld een DISPLAYBARCODE-veld met de kolom erin, zodat het behouden blijft wanneer u het bestand opslaat en weer opent. Een veld met een waarde die u zelf hebt getypt, of van een type dat JeffWrite niet tekent (UPC-E, bijvoorbeeld), blijft zoals het is. Bij samenvoegen per e-mail wordt de waarde als tekst ingevoegd.
- **Adresblok** zet een volledig postadres in: de naam, het bedrijf, de straat en de plaats. Kies de regels die u wilt. Een regel die leeg is, wordt weggelaten. U kunt ook een land opgeven dat moet worden weggelaten, zoals uw eigen land. Met **Voorbeeld** ziet u het adres van de eerste persoon.
- **Begroetingsregel** zet een regel in zoals "Beste dr. Fenwick,". U kiest de woorden. De eerste keuze, "Beste dr. Fenwick,", gebruikt de titel van de persoon. Heeft een persoon geen titel, dan gebruikt de regel de voor- en achternaam, zoals in "Beste Nora Pell,". Heeft een persoon geen naam, dan zegt de begroeting iets anders, zoals "Geachte heer/mevrouw,", wat u kunt wijzigen.
- **Velden koppelen** vertelt het adresblok en de begroetingsregel welke kolom welk onderdeel bevat, wanneer uw kolomnamen ongewoon zijn.

## Regels

Een **regel** maakt een kopie voor sommige personen anders. Klik op **Regels** en kies er een:

- **Als…Dan…Anders**: toon de ene tekst wanneer een kolom een waarde heeft, en een andere tekst wanneer dat niet zo is. Bijvoorbeeld: als Provincie "CA" is, schrijf "Welkom thuis".
- **Record overslaan als**: laat een persoon weg wanneer een kolom een waarde heeft, zoals wanneer de achternaam leeg is.
- **Volgend record** en **Volgend record als**: gebruik de gegevens van de volgende persoon op dezelfde plaats. Etiketten gebruiken dit om op elk etiket een andere persoon te zetten.
- **Record-nr. samenvoegen** zet het nummer van de rij van de persoon in de lijst. **Volgnr. samenvoegen** telt de tot dan toe gemaakte kopieën.
- **Vragen** en **Invulveld**: stellen u een vraag wanneer u het samenvoegen voltooit. Vragen bewaart het antwoord onder een naam. Invulveld zet het antwoord in de tekst.
- **Bladwijzer instellen**: geeft een naam een waarde. Een kruisverwijzing naar de naam toont de waarde.

## Bekijk het resultaat

Klik op **Verzendlijsten > Voorbeeld van resultaten**. Er opent een deelvenster dat de pagina voor één persoon toont. Gebruik **Eerste record**, **Vorige record**, **Volgend record** en **Laatste record** om door de personen te gaan, of typ een nummer. **Ontvanger zoeken** springt naar de volgende persoon die uw woorden in een gegeven bevat.

**Samenvoegvelden markeren** arceert de samenvoegvelden op uw pagina grijs, zodat u ziet waar ze staan. Andere velden, zoals paginanummers, worden niet gearceerd. De arcering is op het scherm zichtbaar en wordt nooit afgedrukt.

Uw eigen document wordt niet gewijzigd. **Controleren op fouten** laat u weten wanneer de lijst geen kolom heeft voor een veld, of wanneer niemand is opgenomen.

## Voltooi het samenvoegen

Klik op **Verzendlijsten > Voltooien en samenvoegen**:

- **Afzonderlijke documenten bewerken** maakt een nieuw document met de kopie van elke persoon. U kunt het wijzigen voordat u afdrukt.
- **Documenten afdrukken** maakt dat document en opent de pagina **Afdrukken**.
- **Opslaan als PDF** slaat alle kopieën op in één pdf-bestand.
- **E-mailberichten verzenden** maakt voor elke persoon een .eml-berichtbestand, in een map die u kiest. Kies de kolom met de e-mailadressen en typ een onderwerp. JeffWrite verzendt de berichten niet. Open de bestanden in uw e-mailprogramma.

De personen waarbij u het vinkje hebt weggehaald in de lijst, worden weggelaten.

Er wordt alleen een berichtbestand gemaakt voor een persoon wiens adres één e-mailadres bevat. Bevat een adres er twee (met een komma of puntkomma ertussen), staat er een naam bij, of ontbreekt het domein, dan krijgt die persoon geen bericht, en JeffWrite meldt hoeveel dat er zijn. Een lang onderwerp wordt in stukken opgeslagen die e-mailprogramma's weer samenvoegen.

Bij een samenvoeging ziet u een voortgangsvenster, en met **Annuleren** stopt u het. Als een regel een vraag stelt (Vragen of Invulveld), wordt u elke verschillende vraag één keer gesteld, tot 20 vragen. Daarna gebruiken de rest de standaardantwoorden. Als u een vraag annuleert, stopt de hele samenvoeging. Een samenvoeging stopt en meldt dat, als er meer dan 10.000 pagina's zouden ontstaan, zodat een heel lang document met een heel lange lijst niet uren duurt. Voeg dan een deel van de lijst tegelijk samen.

## Enveloppen

Klik op **Verzendlijsten > Enveloppen**. Typ het bezorgadres en uw retouradres, en kies het formaat van de envelop uit de lijst. De formaten worden aangeduid met hun breedte en hoogte. Klik op **Aan document toevoegen** om de envelop op een eigen pagina aan het begin van uw document te zetten, of op **Afdrukken** om hem af te drukken. De pagina is zo breed als de lange zijde van de envelop, zoals de meeste printers een envelop aannemen.

Om enveloppen voor een samenvoeging te maken, klikt u op **Afdrukvoorbereiding starten > Enveloppen**. Elke envelop heeft een adresblok.

## Etiketten

Klik op **Verzendlijsten > Etiketten**. Typ het adres en kies het vel uit de lijst. Een vel wordt aangeduid met het aantal etiketten dat het bevat en het formaat van elk etiket. Staat uw vel niet in de lijst, kies dan **Eigen vel** en typ de afmetingen: hoeveel etiketten er naast elkaar en onder elkaar staan, hoe groot een etiket is, de ruimte boven en naast het eerste etiket, en hoe ver de etiketten van elkaar af staan.

- **Volledige pagina met hetzelfde etiket** zet hetzelfde adres op elk etiket van het vel.
- **Enkel etiket** zet het adres op één etiket, zodat u een gedeeltelijk gebruikt vel kunt opmaken. Geef aan welke rij en kolom.

Klik op **Nieuw document** om een document te maken dat u kunt bekijken en wijzigen, of op **Afdrukken** om het af te drukken.

Om etiketten voor een samenvoeging te maken, klikt u op **Afdrukvoorbereiding starten > Etiketten**. Het eerste etiket heeft een adresblok, en elk ander etiket begint met een regel **Volgend record**. Als u het eerste etiket wijzigt, klikt u op **Etiketten bijwerken** om de wijziging naar alle andere te kopiëren.

## Wat het bestand bevat

De soort samenvoeging, de naam en plaats van de lijst, het onderwerp en de e-mailkolom worden opgeslagen in uw .docx-bestand. Welke personen u het vinkje hebt laten verliezen, en uw keuzes bij Velden koppelen, worden niet opgeslagen. Die worden na het openen van het bestand opnieuw gevraagd. Een lijst wordt elke keer opnieuw gelezen uit het bestand. Staat het bestand niet meer op de plaats waar het stond, dan zoekt JeffWrite naast uw document en vraagt u daarna het bestand te kiezen.

Een lijst kan in totaal tot 1.000 kolommen en 4.000.000 cellen hebben, en een samenvoeging gebruikt de eerste 100.000 personen. Heeft een bestand meer, dan gebruikt JeffWrite wat past en meldt dat. Rijen die leeg zijn, worden uit de lijst weggelaten.

## Zie ook

- [Velden gebruiken](fields)
- [Afdrukken](print)
- [Tabellen maken en wijzigen](tables)
