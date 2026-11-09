# PDF-bestanden en drukkerijen
<!-- keywords: pdf, pdf maken, opslaan als pdf, pdf exporteren, pdf/x, pdf/a, archiveren, drukkerij, commerciële drukker, cmyk, afloop, snijtekens, kleurprofiel, steunkleur, overdrukken, wachtwoord, versleutelen, beveiligen met een wachtwoord -->

Een **PDF** is een bestand dat op elke computer hetzelfde eruitziet. Gebruik het om een publicatie per e-mail te versturen, online te plaatsen of naar een drukkerij te sturen.

## Een PDF maken

1. Klik op **Bestand > Exporteren** en vervolgens op **PDF maken**.
2. Kies onder **Optimaliseren voor** een kwaliteit. **dpi** betekent dots per inch (stippen per inch). Meer stippen geven scherpere afbeeldingen en grotere bestanden.
3. Kies onder **Pagina's** de optie **Alle pagina's**, **Huidige pagina** of **Pagina's van** de ene pagina **tot** de andere.
4. Klik op **PDF opslaan…**, typ een naam en klik op **Opslaan**.

| Keuze | Afbeeldingen | Geschikt voor |
|---|---|---|
| Minimale grootte | 96 dpi | Weergave op het scherm |
| Standaard | 150 dpi | Online delen |
| Hoge kwaliteit afdrukken | 300 dpi | Bureauprinters |
| Commerciële drukpers | Volledige resolutie | Een drukkerij. Voegt snijtekens, afloopmarkeringen, pasmerken, kleurbalken en opdrachtgegevens toe. |

Andere opties:

- **Documenteigenschappen opnemen** zet de titel, auteur, onderwerp en trefwoorden in de pdf.
- **PDF openen na opslaan** toont de pdf meteen in uw viewer.
- **Boekjesvellen** verschijnt bij boekjes. Hiermee komen twee pagina's per zijde in vouwvolgorde. Schakel dit uit voor één pagina per pdf-pagina.

## PDF/A voor het bewaren van gegevens

**PDF/A** is een pdf die jaren meegaat. Alle lettertypen zijn ingesloten en er worden standaardkleuren gebruikt, zonder doorzichtige effecten. Vink **PDF/A voor langdurige archivering** aan, of klik op **Bestand > Exporteren** en vervolgens op **PDF/A maken voor archivering**.

## PDF/X voor een drukkerij

**PDF/X** is een strengere pdf die drukkerijen vragen. Kleuren worden opgegeven als inkthoeveelheden, lettertypen zijn ingesloten en het formaat en de afloop zijn aangegeven. De **afloop** is extra afbeelding of kleur die over de paginarand heen loopt, zodat er na het bijsnijden geen witte rand overblijft.

1. Klik op **Bestand > Exporteren** en vervolgens op **PDF/X maken voor een commerciële drukkerij**. Of vink **PDF/X voor een commerciële drukkerij** aan in het venster **PDF maken**. U kunt dit niet gebruiken met PDF/A.
2. Kies onder **Afdrukconditie** de conditie die uw drukkerij noemt.
3. Wil de drukkerij snijtekens en afloop, kies dan ook **Commerciële drukpers** onder **Optimaliseren voor**.
4. Klik op **PDF opslaan…**.

De condities zijn:

- **PDF/X-1a** voor Amerikaans rotatieoffset (SWOP) of Europees offset (FOGRA39). Doorzichtige effecten worden afgevlakt.
- **PDF/X-4** voor premium gestreken papier (FOGRA51, PSO Coated v3), of met het kleurprofiel van uw eigen drukker (een .icc-bestand dat u kiest). Doorzichtige effecten blijven behouden.

Het kleurprofiel PSO Coated v3 wordt niet met JeffPub meegeleverd. De eerste keer dat u het gebruikt, vraagt JeffPub toestemming voor het downloaden van het profiel, eenmalig, van het European Color Initiative.

Voordat het bestand wordt gemaakt, controleert JeffPub afbeeldingen die op hun afdrukformaat minder dan 150 ppi (pixels per inch) hebben en lettertypen die op deze computer ontbreken. Klik op **Toch PDF maken**, **Ontwerpcontrole openen** of **Annuleren**.

## Een pdf met een wachtwoord beveiligen

Klik in het venster **PDF maken** op **Versleutelen met een wachtwoord…**. Typ het wachtwoord twee keer. Wie de pdf opent, moet dan het wachtwoord invoeren. Het wachtwoord beschermt het bestand echt: de pagina's worden met sterke versleuteling (AES-256) door elkaar gehaald en niemand kan ze zonder wachtwoord lezen.

U kunt ook **Afdrukken, kopiëren en bewerken beperken** aanvinken, een tweede wachtwoord opgeven en kiezen wat is toegestaan. Deze beperkingen zijn alleen een verzoek. De meeste pdf-lezers houden zich eraan, maar wie ze wil omzeilen, wordt er niet door tegengehouden. Reken er dus niet op om een bestand privé te houden.

Een vergeten wachtwoord kan niet worden hersteld, dus bewaar het op een veilige plek. Het wachtwoord geldt voor die ene pdf en wordt nooit opgeslagen. Een pdf met wachtwoord kan geen PDF/A of PDF/X zijn, dus die twee opties worden uitgeschakeld zolang er een wachtwoord is ingesteld. Op **Bestand > Delen** staat **Versleutelen met een wachtwoord…** boven **Verzenden als PDF**; daarmee krijgt de pdf in een e-mail een wachtwoord.

Wanneer u een pdf met wachtwoord als afbeelding plaatst, vraagt JeffPub om het wachtwoord. Is het onjuist, dan vraagt JeffPub opnieuw; klik op **Annuleren** om te stoppen. De pagina in uw publicatie is een kopie en heeft geen wachtwoord nodig. Vraagt de eigenaar van de pdf dat er niet van wordt gekopieerd, dan plaatst JeffPub hem toch en toont een melding daarover; dat verzoek is geen slot.

Een afbeelding die is gekoppeld aan een pdf met wachtwoord, wordt bij het openen van de publicatie als **Niet bijgewerkt** getoond, en JeffPub vraagt nooit zelf om het wachtwoord. Klik op **Koppeling bijwerken** om er wel om gevraagd te worden.

## Kleuren instellen voor een drukkerij

Klik op **Bestand > Info** en vervolgens op de kaart **Instellingen commercieel drukken**:

- **Kleurmodel** is **RGB**, **Eén kleur (steunkleur)**, **Steunkleuren**, **Proceskleuren (CMYK)** of **Proceskleuren plus steunkleuren**. Proceskleuren zijn cyaan-, magenta-, gele en zwarte inkt. Een steunkleur is een speciale inkt die de drukkerij mengt. Voeg elke steunkleur toe onder **Steunkleuren**.
- **Zwart overdrukken in bestanden voor een drukkerij (PDF/X)** laat zwart over de kleuren eronder drukken, zodat een verschoven plaat geen witte rand oplevert. U kiest wat het omvat: tekst kleiner dan een bepaald formaat, lijnen of vullingen.

## Zie ook

- [Een publicatie afdrukken](printing)
- [Opslaan als afbeelding, webpagina of e-book](export)
- [Uw ontwerp controleren](design-checker)
