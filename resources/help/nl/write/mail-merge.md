# Samenvoegen, enveloppen en etiketten
<!-- keywords: afdruk samenvoegen, samenvoegen, verzendlijsten, geadresseerden, adressenlijst, standaardbrief, etiketten, enveloppen, samenvoegveld, adresblok, begroetingsregel, regels, als dan anders, record overslaan, volgend record, voorbeeld van resultaten, samenvoegen voltooien, catalogus, e-mailberichten, csv, xlsx, ods, vcard, mailing, adressen, brieven, voorbeeld -->

Met **afdruk samenvoegen** maakt u veel kopieën van één brief, elk met de gegevens van een andere persoon. U schrijft de brief één keer. U bewaart de gegevens van de personen in een **lijst**. JeffWrite zet de naam en het adres van de eerste persoon in de eerste kopie, die van de tweede persoon in de tweede kopie, enzovoort.

De hulpmiddelen staan op het tabblad **Verzendlijsten**. Voor een begeleide route klikt u op **Verzendlijsten > Afdrukvoorbereiding starten > Stapsgewijze wizard Afdruk samenvoegen**.

## Kies wat u maakt

Klik op **Verzendlijsten > Afdrukvoorbereiding starten** en kies één van de volgende opties:

- **Brieven**: één kopie van de pagina voor elke persoon. Elke kopie begint op een nieuwe pagina.
- **E-mailberichten**: één bericht voor elke persoon. Zie "E-mailberichten verzenden" hieronder.
- **Enveloppen** en **Etiketten**: een pagina die daarvoor is gemaakt. Zie de onderstaande secties.
- **Catalogus**: één lijst met iedereen, zonder paginaeinde tussen de personen. Gebruik dit voor een telefoonlijst of een catalogus.
- **Samenvoegen wissen** maakt van het document weer een gewoon document.

JeffWrite bewaart de soort samenvoeging en de naam van de lijst in het bestand wanneer u opslaat. Wanneer u het bestand opnieuw opent, is de samenvoeging er nog.

## Kies uw geadresseerden

Een **geadresseerde** is een persoon die een kopie krijgt. Klik op **Verzendlijsten > Geadresseerden selecteren**:

- **Nieuwe lijst typen**: typ de personen in het venster dat opent. Klik voor elke persoon op **Nieuwe vermelding**. JeffWrite biedt daarna aan de lijst op te slaan als .csv-bestand.
- **Bestaande lijst gebruiken**: kies een bestand dat al een lijst is. Dit kan een .csv-bestand zijn of een tekstbestand met tabs (één rij per persoon, en de eerste rij bevat de kolomnamen), een spreadsheet (.xlsx, .xls of .ods, van JeffCrunch of een ander programma), of een contactpersonenbestand (.vcf).
- **Selecteren uit contactpersonen**: kies een contactpersonenbestand (.vcf). De meeste adresboeken en telefoons kunnen hun contactpersonen als zodanig bestand opslaan.

Klik daarna op **Verzendlijsten > Lijst met geadresseerden bewerken** om de lijst te controleren:

- Haal het vinkje weg bij een rij om die persoon weg te laten.
- Klik op een kolomkop om te sorteren.
- Typ in **Geadresseerden filteren** om alleen de rijen te tonen waarin uw woorden staan.
- **Dubbele vermeldingen zoeken** haalt het vinkje weg bij herhalingen.
- **Adressen valideren** kleurt de rijen die geen adres hebben of een postcode die onjuist lijkt.

## Zet de gegevens in de brief

Een **samenvoegveld** is een tijdelijke aanduiding. Het toont de naam van een kolom tussen dubbele punthaken, zoals «Voornaam». In elke kopie wordt het vervangen door de gegevens van die persoon.

Klik waar de gegevens moeten komen en gebruik daarna **Verzendlijsten > Velden schrijven en invoegen**:

- **Samenvoegveld invoegen** toont de kolommen van uw lijst. Kies er één.
  - **Streepjescode uit veld** zet in dezelfde lijst een streepjescode in waarvan de waarde een kolom is, zoals een artikelnummer of het 13-cijferige ISBN van een boek. Kies de kolom en kies daarna het type. Controleer het venster **Streepjescode invoegen** (wat u daar typt, is alleen een voorbeeld). Het venster biedt alleen wat een veld kan bevatten: het type, de grootte, de balkhoogte, de cijfers en het QR-niveau. Het laat weg wat een veld niet kan aangeven: de prijs van een boek, een add-on, een controleteken en de transparante achtergrond. Het ISBN van een boek is een EAN-13. In het document wordt het veld als streepjescode getekend, als voorbeeld van het type tot u samenvoegt. Om het type of de weergave te wijzigen, dubbelklikt u erop, of klikt u er met de rechtermuisknop op en kiest u **Streepjescode bewerken**. Bij het samenvoegen krijgt de kopie van elke persoon een streepjescode met de eigen waarde van die persoon, als afbeelding in de regel. Zo wordt hij mee afgedrukt en opgeslagen in pdf, zoals elke andere afbeelding. De waarde is alleen gegevens: aanhalingstekens of een backslash erin veranderen niets. Is een waarde leeg, dan is er geen streepjescode. Kan een code de waarde niet bevatten (een letter in een EAN-13, of meer dan 500 tekens in Code 128), dan toont de kopie in plaats daarvan een korte melding tussen vierkante haken. De andere kopieën worden daardoor niet beïnvloed. Eén samenvoeging maakt hoogstens 2.000 streepjescodes, waarbij elke waarde die niet kan worden weergegeven meetelt. Als er te veel zijn, past u het aantal personen aan. In een opgeslagen .docx-bestand is het veld een DISPLAYBARCODE-veld met de kolom erin. Zo blijft het bewaard wanneer u het bestand opslaat en opnieuw opent. Een veld met een ingetypte waarde, of van een type dat JeffWrite niet tekent (zoals UPC-E), blijft zoals het is. Bij samenvoegingen per e-mail zet u de waarde als tekst in het bericht.
- **Adresblok** zet een volledig postadres in: de naam, het bedrijf, de straat en de plaats. Kies de regels die u wilt. Een regel zonder inhoud wordt weggelaten. U kunt ook een land opgeven dat moet worden weggelaten, zoals uw eigen land. Met **Voorbeeld** ziet u het adres van de eerste persoon.
- **Begroetingsregel** zet een regel in zoals "Geachte dr. Fenwick,". U kiest de woorden. De eerste keuze, "Geachte dr. Fenwick,", gebruikt de titel van de persoon. Heeft een persoon geen titel, dan gebruikt de regel de voor- en achternaam, zoals in "Geachte Nora Pell,". Heeft een persoon geen naam, dan zegt de begroeting iets anders, zoals "Geachte heer/mevrouw,", wat u kunt wijzigen.
- **Velden koppelen** vertelt het adresblok en de begroetingsregel welke kolom welk onderdeel bevat, wanneer uw kolomnamen ongewoon zijn.

## Regels

Een **regel** maakt een kopie anders voor sommige personen. Klik op **Regels** en kies er één:

- **Als…Dan…Anders…**: toon de ene tekst wanneer een kolom een waarde heeft, en de andere tekst wanneer dat niet zo is. Bijvoorbeeld: als Provincie "NH" is, schrijf "Welkom thuis".
- **Record overslaan als**: laat een persoon weg wanneer een kolom voldoet aan een test die u instelt, zoals wanneer de achternaam leeg is.
- **Volgend record** en **Volgend record als**: gebruik de gegevens van de volgende persoon op dezelfde plaats. Etiketten gebruiken dit om op elk etiket een andere persoon te zetten.
- **Samenvoegrecord nr.** zet het nummer van de rij van de persoon in de lijst. **Samenvoegvolgnummer #** telt de tot nu toe gemaakte kopieën.
- **Vragen** en **Invulveld**: stel een vraag wanneer u het samenvoegen voltooit. Vragen bewaart het antwoord onder een naam. Invulveld zet het antwoord in de tekst.
- **Bladwijzer instellen**: geeft een naam een waarde. Een kruisverwijzing naar de naam toont de waarde.

## Voorbeeld van resultaten

Klik op **Verzendlijsten > Voorbeeld van resultaten**. Er opent een deelvenster dat de pagina voor één persoon toont. Gebruik **Eerste record**, **Vorig record**, **Volgend record** en **Laatste record** om door de personen te gaan, of typ een nummer. Met **Geadresseerde zoeken** springt u naar de volgende persoon die uw woorden in een gegeven heeft.

Met **Samenvoegvelden markeren** worden de samenvoegvelden in uw pagina grijs gearceerd, zodat u ziet waar ze staan. Andere velden, zoals paginanummers, worden niet gearceerd. De arcering is alleen op het scherm zichtbaar en wordt nooit afgedrukt.

Uw eigen document wordt niet gewijzigd. **Controleren op fouten** meldt wanneer de lijst geen kolom heeft voor een veld, of wanneer niemand is opgenomen.

## Samenvoegen voltooien

Klik op **Verzendlijsten > Samenvoegen voltooien**:

- **Afzonderlijke documenten bewerken** maakt een nieuw document met de kopie van elke persoon. U kunt het wijzigen voordat u afdrukt.
- **Documenten afdrukken** maakt dat document en opent de pagina **Afdrukken**.
- **Opslaan als PDF** slaat alle kopieën op in één pdf-bestand.
- **E-mailberichten verzenden** maakt voor elke persoon één berichtbestand (.eml), in een map die u kiest. Kies de kolom met de e-mailadressen en typ een onderwerp. JeffWrite verstuurt de berichten niet. Open de bestanden in uw e-mailprogramma.

De personen waarvan u het vinkje in de lijst hebt weggehaald, worden weggelaten.

Er wordt alleen een berichtbestand gemaakt voor een persoon wiens adres één e-mailadres bevat. Bevat een adres er twee (met een komma of puntkomma ertussen), staat er een naam bij, of ontbreekt het domein, dan krijgt die persoon geen bericht. JeffWrite meldt hoeveel dat er zijn. Een lang onderwerp wordt in stukken opgeslagen, die e-mailprogramma's weer samenvoegen.

Tijdens een samenvoeging ziet u een voortgangsvenster, en met **Annuleren** stopt u het. Als een regel een vraag stelt (Vragen of Invulveld), wordt u voor elke verschillende vraag één keer gevraagd, tot 20 vragen. Daarna gebruiken de overige vragen hun standaardantwoorden. Als u een vraag annuleert, stopt de hele samenvoeging. Een samenvoeging stopt en meldt dat ook als die meer dan 10.000 pagina's zou maken. Zo duurt een zeer lang document met een zeer lange lijst niet uren. Voeg in dat geval een deel van de lijst tegelijk samen.

## Enveloppen

Klik op **Verzendlijsten > Enveloppen**. Typ het afleveradres en uw retouradres, en kies het formaat van de envelop uit de lijst. De formaten staan vermeld met hun breedte en hoogte. Klik op **Toevoegen aan document** om de envelop op een eigen pagina aan het begin van uw document te zetten, of op **Afdrukken** om hem af te drukken. De pagina is zo breed als de lange zijde van de envelop, zoals de meeste printers een envelop aannemen.

Om enveloppen voor een samenvoeging te maken, klikt u op **Afdrukvoorbereiding starten > Enveloppen**. Elke envelop heeft een adresblok.

## Etiketten

Klik op **Verzendlijsten > Etiketten**. Typ het adres en kies het vel uit de lijst. Een vel wordt genoemd naar het aantal etiketten erop en het formaat van elk etiket. Staat uw vel niet in de lijst, kies dan **Eigen vel** en typ de afmetingen: hoeveel etiketten er naast elkaar en onder elkaar staan, hoe groot een etiket is, de ruimte boven en naast het eerste etiket, en hoe ver de etiketten van elkaar staan.

- **Volledige pagina met hetzelfde etiket** zet hetzelfde adres op elk etiket van het vel.
- **Enkel etiket** zet het adres op één etiket, zodat u een gedeeltelijk gebruikt vel kunt opmaken. Geef de rij en kolom op.

Klik op **Nieuw document** om een document te maken dat u kunt bekijken en wijzigen, of op **Afdrukken** om het direct af te drukken.

Om etiketten voor een samenvoeging te maken, klikt u op **Afdrukvoorbereiding starten > Etiketten**. Het eerste etiket heeft een adresblok, en elk ander etiket begint met een regel **Volgend record**. Wijzigt u het eerste etiket, klik dan op **Etiketten bijwerken** om de wijziging naar alle andere te kopiëren.

## Wat het bestand bevat

De soort samenvoeging, de naam en plaats van de lijst, het onderwerp en de e-mailkolom worden opgeslagen in uw .docx-bestand. Welke personen u hebt uitgeschakeld en uw keuzes bij Velden koppelen worden niet opgeslagen. Die worden na het openen van het bestand opnieuw gevraagd. Een lijst wordt elke keer opnieuw uit het bestand gelezen. Staat het bestand niet meer op de oorspronkelijke plek, dan zoekt JeffWrite naast uw document. Daarna vraagt het u het bestand aan te wijzen.

Een lijst kan in totaal tot 1.000 kolommen en 4.000.000 cellen hebben, en een samenvoeging gebruikt de eerste 100.000 personen. Bevat een bestand meer, dan gebruikt JeffWrite wat erin past en meldt dat. Rijen zonder inhoud worden uit de lijst weggelaten.

## Zie ook

- [Velden gebruiken](fields)
- [Afdrukken](print)
- [Tabellen maken en wijzigen](tables)
