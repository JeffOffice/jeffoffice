# Afdruk samenvoegen en catalogus samenvoegen
<!-- keywords: afdruk samenvoegen, samenvoegen, geadresseerden, adressenlijst, etiketten, standaardbrieven, csv, xlsx, vcard, samenvoegveld, adresblok, catalogus, productlijst, e-mail samenvoegen, voorbeeld -->

Afdruk samenvoegen vult één publicatie met gegevens uit een lijst. U schrijft een brief één keer, en JeffPub maakt er voor elke persoon een kopie van, met de naam en het adres van die persoon op de juiste plek. Met catalogus samenvoegen herhaalt u één indeling voor elk item in een lijst.

De hulpmiddelen staan op het tabblad **Verzendlijsten**. Voor een begeleide route klikt u op **Verzendlijsten > Starten > Afdrukvoorbereiding starten > Stapsgewijze wizard Afdruk samenvoegen** om het deelvenster **Afdruk samenvoegen** te openen.

## Kies uw geadresseerden

Klik op **Verzendlijsten > Starten > Geadresseerden selecteren**:

- **Bestaande lijst gebruiken…**: kies een csv-bestand, een tekstbestand met tabs als scheidingsteken, een .xlsx-spreadsheet (het eerste blad) of een .vcf-bestand met contactpersonen. Bij een csv-, tekst- of spreadsheetbestand moet de eerste rij de kolomnamen bevatten.
- **Selecteren uit contactpersonen…**: kies een bestand met contactpersonen (.vcf of .vcard). De meeste adresboeken en telefoons kunnen contactpersonen zo opslaan; elk contact wordt een geadresseerde, met kolommen voor naam, adres, telefoonnummer en e-mailadres.
- **Nieuwe lijst typen…**: vul het venster **Nieuwe adressenlijst** in en gebruik **Nieuw item** voor elke persoon. JeffPub biedt daarna aan de lijst als csv-bestand op te slaan.

Om de lijst te controleren, klikt u op **Verzendlijsten > Starten > Adressenlijst bewerken**. Verwijder in het venster **Geadresseerden voor afdruk samenvoegen** het vinkje bij een rij om die persoon over te slaan, klik op een kop om te sorteren, en typ in **Geadresseerden filteren** om de lijst te beperken. Met **Ontvangers onderscheiden op basis van** kiest u een kolom waarmee personen van elkaar te onderscheiden zijn. De lijst wordt met de niet-aangevinkte rijen in uw publicatie bewaard. Wanneer u hetzelfde lijstbestand opnieuw kiest, worden dezelfde personen weer overgeslagen, ook als u de rijen daarvoor hebt gesorteerd. Met **Dubbele vermeldingen zoeken** vinkt u herhalingen uit, en met **Adressen valideren** markeert u rijen met een ontbrekend adres of een afwijkende postcode.

## Samenvoegvelden toevoegen

Een samenvoegveld is een tijdelijke aanduiding, zoals «Voornaam», die voor elke persoon een echte waarde wordt. Klik in een tekstvak (JeffPub voegt er een toe als er geen is) en gebruik daarna **Verzendlijsten > Velden schrijven en invoegen**:

- **Adresblok**: een postadres dat wordt opgebouwd uit kolommen met namen als Voornaam, Achternaam en Adresregel 1.
- **Begroetingsregel**: "Geachte" en de naam van de persoon, of "Beste vriend," als er geen naam is.
- **Samenvoegveld invoegen**: kies een willekeurige kolom.
- **Afbeeldingsveld**: kies een kolom met bestandsnamen van afbeeldingen. JeffPub voegt een leeg kader toe en zoekt de bestanden naast uw lijst. Nadat u de publicatie hebt opgeslagen en opnieuw hebt geopend, wordt een afbeelding waarvan het bestand buiten de map van de publicatie staat, niet getoond totdat u de adressenlijst opnieuw kiest (**Verzendlijsten > Geadresseerden selecteren > Bestaande lijst gebruiken…**). Daarna worden de afbeeldingen in de map van de lijst, en in de mappen daarbinnen, getoond tot u de publicatie sluit. JeffPub onthoudt dit niet voor de volgende keer. Of bewaar de afbeeldingsbestanden in de map van de publicatie, of in een map daarbinnen; dan worden ze getoond zodra de publicatie wordt geopend. Zo kan een publicatie die iemand u stuurt niet zelfstandig afbeeldingen van uw computer lezen. De afbeelding van elke record wordt getoond wanneer u het resultaat bekijkt, afdrukt, een pdf opslaat of samenvoegt naar een nieuwe publicatie of catalogus.

## Een getal, prijs, datum of tijd naar wens weergeven

Een samenvoegveld toont de waarde precies zoals uw lijst die bevat. Om dat te wijzigen, klikt u naast het veld (of selecteert u het, of meerdere velden) en klikt u op **Verzendlijsten > Velden schrijven en invoegen > Samenvoegveld opmaken…**. Kies onder **De waarde weergeven als**:

- **Getal**, met 0 tot 4 **Decimalen**. Bijvoorbeeld: 1234,5 met 1 decimaal wordt weergegeven als 1.234,5.
- **Valuta**, met het valutateken van uw regio: 1234,5 wordt weergegeven als € 1.234,50, waar dat het teken is.
- **Percentage**, waarbij 0,256 met 1 decimaal wordt weergegeven als 25,6%.
- **Datum**, in een **Stijl** zoals Kort, Lang, Jaar, maand, dag, Maand en dag, of Maand en jaar.
- **Tijd**, in een Kort, 12-uurs of 24-uurs **Stijl**.

De regel **Voorbeeld** toont wat uw keuze oplevert. De waarden worden gelezen en weergegeven in de taal van de tekst bij het veld (Engels (VS) als er geen taal is ingesteld), niet in de taal van uw computer. In het Engels (VS) is 03/04/2026 4 maart, en in een taal waarin de dag eerst komt, zoals Duits of Brits Engels, is het 3 april; in het Duits is 1.234,5 een getal met een decimaal komma. Een jaar met twee cijfers wordt gelezen als 2000 tot 2029 voor 00 tot 29, en als 1930 tot 1999 voor 30 tot 99. Een getal tussen haakjes, zoals (500), is negatief. Een waarde die geen getal, datum of tijd is van het soort dat u hebt gekozen, wordt precies getoond zoals in uw lijst: een woord, "nan" of "inf" (een persoon kan Nan heten), een lege waarde, een tijd met een datumnotatie, en een datum met een tijdnotatie. De voorbeeldweergave en het uiteindelijke samenvoegen tonen beide de opgemaakte waarde. De keuze is één stap die u ongedaan kunt maken, en wordt met de publicatie opgeslagen.

## Het resultaat bekijken

Klik op **Verzendlijsten > Voorbeeld van resultaten > Voorbeeld van resultaten** om echte gegevens in plaats van de velden te zien. Gebruik **Eerste record**, **Vorig record**, **Volgend record** en **Laatste record** om door de personen te bladeren, of **Geadresseerde zoeken** om naar een overeenkomst te springen.

## Het samenvoegen voltooien

Klik op **Verzendlijsten > Voltooien > Voltooien en samenvoegen**:

- **Naar printer samenvoegen…** opent de pagina **Afdrukken**. Vink **Alle records van afdruk samenvoegen afdrukken** aan en druk af.
- **Naar nieuwe publicatie samenvoegen** maakt een nieuwe publicatie met een kopie van uw pagina's voor elke persoon.
- **Naar pdf samenvoegen…** slaat één pdf op met de pagina's van alle personen.
- **Naar e-mail samenvoegen…** maakt per persoon één e-mailbestand (.eml), met de eerste pagina als afbeelding. Kies de kolom met het e-mailadres, typ een onderwerp en kies een map. JeffPub verzendt de berichten niet. Open de bestanden in uw e-mailprogramma.
- **Elk opslaan als bestand…** maakt voor elke persoon één publicatie (.jpub), in een map die u kiest. Geef de bestanden namen met kolommen tussen accolades, zoals {Achternaam}, {Voornaam}. U wordt gevraagd om bevestiging bij meer dan 200 bestanden, en eenmalig als er al bestanden met dezelfde namen staan. Klik op **Annuleren** om tussen twee bestanden te stoppen.
- **Adressenlijst exporteren…** slaat de lijst op als csv-bestand.

## Catalogus samenvoegen

Klik op **Invoegen > Pagina's > Cataloguspagina's** om het deelvenster **Catalogusamenvoeging** te openen.

1. Kies de productlijst, zoals hierboven beschreven.
2. Klik op **Catalogusgebied invoegen**. Er verschijnt een vak met de tag "Catalogusgebied". Stel de **Rijen** en **Kolommen** in, en de positie en grootte ervan. Zet de velden voor één item in de eerste cel door ze in de lijst te dubbelklikken. Ze worden in de andere cellen herhaald. Kies de versie "(afbeelding)" van een veld voor een afbeelding.
3. Bekijk een voorbeeld met de pijlen. Elke pagina toont zoveel items als het gebied cellen heeft. Voeg daarna samen naar een nieuwe publicatie, een pdf of de printer.

Met **Catalogusgebied verwijderen** verwijdert u het gebied.

## Zie ook

- [Een publicatie afdrukken](printing)
- [Tekstvakken toevoegen en opmaken](text-boxes)
- [Het tabblad Verzendlijsten](tab-mailings)
