# Afdrukken samenvoegen en cataloguszoeken

<!-- keywords: afdruk samenvoegen, samenvoegen, geadresseerden, adressenlijst, etiketten, standaardbrieven, csv, xlsx, vcard, samenvoegveld, adresblok, catalogus, productlijst, e-mail samenvoegen, voorbeeld -->

Afdruk samenvoegen vult één publicatie met gegevens uit een lijst. U schrijft een brief één keer, en JeffPub maakt voor elke persoon een kopie met de naam en het adres van die persoon op de juiste plaats. Bij catalogus samenvoegen wordt één indeling herhaald voor elk item in een lijst.

De hulpmiddelen staan op het tabblad **Verzendlijsten**. Voor een stapsgewijze begeleiding klikt u op **Verzendlijsten > Starten > Afdrukvoorbereiding starten > Stapsgewijze wizard Afdruk samenvoegen** om het taakvenster **Afdruk samenvoegen** te openen.

## Geadresseerden kiezen

Klik op **Verzendlijsten > Starten > Geadresseerden selecteren**:

- **Bestaande lijst gebruiken…**: kies een csv-bestand, een tekstbestand met tabs, een xlsx-spreadsheet (het eerste blad) of een vcf-bestand met contactpersonen. In een csv-, tekst- of spreadsheetbestand moet de eerste rij de kolomnamen bevatten.
- **Selecteren uit contactpersonen…**: kies een bestand met contactpersonen (.vcf of .vcard). De meeste adresboeken en telefoons kunnen hun contactpersonen in zo'n bestand opslaan. Elke contactpersoon wordt een geadresseerde, met kolommen voor naam, adres, telefoonnummer en e-mailadres.
- **Nieuwe lijst typen…**: vul het venster **Nieuwe adressenlijst** in en gebruik **Nieuwe vermelding** voor elke persoon. JeffPub biedt daarna aan de lijst op te slaan als csv-bestand.

Om de lijst te controleren, klikt u op **Verzendlijsten > Starten > Adressenlijst bewerken**. Verwijder in het venster **Geadresseerden voor afdruk samenvoegen** het vinkje bij een rij om die persoon over te slaan, klik op een kopje om te sorteren, en typ in **Geadresseerden filteren** om de lijst te beperken. Met **Dubbele vermeldingen zoeken** worden herhalingen uitgevinkt, en met **Adressen valideren** worden rijen gemarkeerd met een ontbrekend adres of een vreemde postcode.

## Samenvoegvelden toevoegen

Een samenvoegveld is een tijdelijke aanduiding, zoals «Voornaam», die voor elke persoon een echte waarde wordt. Klik in een tekstvak (JeffPub maakt er een aan als u dat niet doet) en gebruik daarna **Verzendlijsten > Velden schrijven en invoegen**:

- **Adresblok**: een postadres dat is samengesteld uit kolommen met namen als Voornaam, Achternaam en Adresregel 1.
- **Begroetingsregel**: "Geachte" en de naam van de persoon, of "Beste vriend," als er geen naam is.
- **Samenvoegveld invoegen**: kies een willekeurige kolom.
- **Afbeeldingsveld**: kies een kolom met bestandsnamen van afbeeldingen. JeffPub voegt een leeg kader toe en zoekt de bestanden naast uw lijst. Nadat u de publicatie hebt opgeslagen en opnieuw hebt geopend, wordt een afbeelding waarvan het bestand buiten de map van de publicatie staat, pas getoond nadat u de geadresseerdenlijst opnieuw hebt gekozen (**Verzendlijsten > Geadresseerden selecteren > Bestaande lijst gebruiken…**). Daarna worden de afbeeldingen in de map van de lijst, en in de mappen daarbinnen, getoond tot u de publicatie sluit. JeffPub onthoudt dit niet voor de volgende keer. U kunt de afbeeldingsbestanden ook in de map van de publicatie zelf, of in een map daarbinnen, zetten. Dan worden ze getoond zodra de publicatie wordt geopend. Zo kan een publicatie die iemand u stuurt niet op eigen initiatief afbeeldingen van uw computer lezen. De afbeelding van elke record wordt getoond wanneer u het resultaat voorbeeld bekijkt, afdrukt, een pdf opslaat of samenvoegt naar een nieuwe publicatie of catalogus.

## Een getal, prijs, datum of tijd op uw manier tonen

Een samenvoegveld toont de waarde precies zoals uw lijst die bevat. Om dat te wijzigen, klikt u naast het veld (of selecteert u het, of meerdere velden) en klikt u op **Verzendlijsten > Velden schrijven en invoegen > Samenvoegveld opmaken…**. Kies onder **Waarde weergeven als**:

- **Getal**, met 0 tot 4 **Decimalen**. Bijvoorbeeld: 1234,5 met 1 decimaal wordt weergegeven als 1.234,5.
- **Valuta**, met het valutateken van uw regio: 1234,5 wordt weergegeven als € 1.234,50, als dat het teken is.
- **Percentage**, waarbij 0,256 met 1 decimaal wordt weergegeven als 25,6%.
- **Datum**, in een **Stijl** zoals Kort, Lang, Jaar, Maand, dag, Maand en dag, of Maand en jaar.
- **Tijd**, in een Kort, 12-uurs of 24-uurs **Stijl**.

De regel **Voorbeeld** toont wat uw keuze oplevert. De waarden worden gelezen en getoond in de taal van de tekst op het veld (Amerikaans Engels als er geen taal is ingesteld), niet in de taal van uw computer. In Amerikaans Engels is 03/04/2026 4 maart, en in een taal waarin de dag voorop staat, zoals Duits of Brits Engels, is het 3 april. In het Duits is 1.234,5 een getal met een decimaalkomma. Een jaartal van twee cijfers wordt gelezen als 2000 tot 2029 voor 00 tot 29, en 1930 tot 1999 voor 30 tot 99. Een getal tussen haakjes, zoals (500), is negatief. Een waarde die geen getal, datum of tijd van de gekozen soort is, wordt precies getoond zoals ze in uw lijst staat: een woord, "nan" of "inf" (een persoon kan Nan heten), een lege waarde, een tijd met een datumnotatie, of een datum met een tijdnotatie. Zowel het voorbeeld als het voltooide samenvoegen tonen de opgemaakte waarde. De keuze is één stap van **Ongedaan maken** en wordt met de publicatie opgeslagen.

## Het resultaat bekijken

Klik op **Verzendlijsten > Voorbeeld van resultaten > Voorbeeld van resultaten** om echte gegevens in plaats van de velden te zien. Gebruik **Eerste record**, **Vorig record**, **Volgend record** en **Laatste record** om door de personen te bladeren, of **Ontvanger zoeken** om naar een overeenkomst te springen.

## Het samenvoegen voltooien

Klik op **Verzendlijsten > Voltooien > Voltooien en samenvoegen**:

- **Samenvoegen naar printer…** opent de pagina **Afdrukken**. Vink **Alle records van afdruk samenvoegen afdrukken** aan en druk daarna af.
- **Samenvoegen in nieuwe publicatie** maakt een nieuwe publicatie met een kopie van uw pagina's voor elke persoon.
- **Samenvoegen naar pdf…** slaat één pdf op met de pagina's van alle personen.
- **Samenvoegen naar e-mail…** maakt per persoon één e-mailbestand (.eml), met de eerste pagina als afbeelding. Kies de e-mailkolom, typ een onderwerp en kies een map. JeffPub verstuurt de berichten niet. Open de bestanden in uw e-mailprogramma.
- **Adressenlijst exporteren…** slaat de lijst op als csv-bestand.

## Catalogus samenvoegen

Klik op **Invoegen > Pagina's > Cataloguspagina's** om het taakvenster **Catalogussamenvoegen** te openen.

1. Kies de productlijst, zoals hierboven beschreven.
2. Klik op **Cataloguszone invoegen**. Er verschijnt een kader met het label "Cataloguszone". Stel de **Rijen** en **Kolommen** in, en de positie en grootte. Zet de velden voor één item in de eerste cel door ze in de lijst te dubbelklikken. Ze worden in de andere cellen herhaald. Kies voor een afbeelding de versie van het veld met "(afbeelding)".
3. Bekijk een voorbeeld met de pijlen. Elke pagina toont zoveel items als de zone cellen heeft. Voeg daarna samen naar een nieuwe publicatie, een pdf of de printer.

Met **Cataloguszone verwijderen** verwijdert u de zone.

## Zie ook

- [Een publicatie afdrukken](printing)
- [Tekstvakken toevoegen en opmaken](text-boxes)
- [Het tabblad Verzendlijsten](tab-mailings)
