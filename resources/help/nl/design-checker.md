# Ontwerp controleren

<!-- keywords: ontwerpcontrole, controleren, fouten, problemen, lage resolutie, overloop, tekst past niet, leeg tekstvak, buiten de pagina, preflight, eindcontrole, proef, gekoppelde afbeelding, ontbrekend lettertype -->

De **Ontwerpcontrole** zoekt in uw publicatie naar dingen die vaak misgaan. Hij vindt tekst die niet past, lege vakken, objecten die buiten de pagina steken en afbeeldingen die slecht zullen afdrukken. Voer hem uit voordat u afdrukt of een bestand naar een drukkerij stuurt.

## De ontwerpcontrole uitvoeren

Klik op **Beoordelen > Controleren > Ontwerpcontrole uitvoeren**. Het deelvenster **Ontwerpcontrole** opent rechts. U kunt ook op **Bestand > Info** klikken en daarna op **Ontwerpcontrole uitvoeren**.

Het deelvenster heeft twee selectievakjes, die beide in eerste instantie aan staan:

- **Algemene ontwerpcontroles uitvoeren** zoekt naar indelingsproblemen.
- **Laatste publicatiecontroles uitvoeren** zoekt naar zaken die belangrijk zijn bij het afdrukken.

Laat beide aan staan om elk soort probleem te zien. De lijst wordt bijgewerkt terwijl u werkt. Als er niets te repareren valt, staat er "Geen problemen gevonden."

## Een probleem oplossen

1. Klik op een item in de lijst onder **Selecteer een item om te herstellen**.
2. Klik op **Naar item**. Of dubbelklik op het item.

JeffPub gaat naar de pagina en selecteert het object. Los het probleem op en het item verdwijnt uit de lijst.

## Wat er wordt gevonden

| Melding | Wat het betekent | Wat u moet doen |
|---|---|---|
| Pagina heeft geen inhoud | De pagina is leeg. | Voeg iets toe of verwijder de pagina. |
| Object valt gedeeltelijk buiten de pagina | Een deel van een object ligt buiten de pagina. | Verplaats het, of laat het staan als het een afloop is. |
| Tekstvak is leeg | Een tekstvak bevat geen tekst. | Typ er tekst in, of verwijder het. |
| Artikel met overlooptekst | Een artikel is de tekst die door gekoppelde vakken loopt. Dit artikel heeft meer tekst dan er past. | Vergroot een vak, kort de tekst in, of [koppel een ander vak](linked-text). |
| Tekst is sterk verkleind om te passen | Het verkleinen van tekst om in het vak te passen heeft de tekst kleiner dan de helft van de oorspronkelijke grootte gemaakt. | Vergroot het vak of gebruik minder tekst. |
| Tijdelijke aanduiding voor afbeelding is leeg | Een afbeeldingskader heeft geen afbeelding. | Klik erop om er een toe te voegen, of verwijder het. |
| Afbeelding is niet proportioneel geschaald | De afbeelding is uitgerekt of samengedrukt. | Wijzig het formaat om de vorm te herstellen. |
| Afbeelding is gekoppeld, niet ingesloten | De afbeelding is aan een bestand gekoppeld en alleen een kleine voorvertoning is in de publicatie opgeslagen. Een afbeelding die is ingevoegd met **Invoegen en koppelen** wordt volledig opgeslagen en staat niet in de lijst. | Bewaar het oorspronkelijke bestand bij uw publicatie, of klik in het [Afbeeldingenbeheer](graphics-manager) op **Afbeelding insluiten**. |
| Gekoppelde afbeelding ontbreekt | Het bestand waarnaar een gekoppelde afbeelding verwijst, is verdwenen. De pagina toont daardoor alleen de kleine voorvertoning, en die zou worden afgedrukt. | Klik in het Afbeeldingenbeheer op **Koppeling wijzigen…** om het bestand te zoeken, of zet het terug en klik op **Koppeling bijwerken**. |
| Gekoppelde afbeelding toont alleen de voorvertoning tot Koppeling bijwerken | De afbeelding is gekoppeld aan een bestand buiten de map van de publicatie, dat JeffPub niet zelf opent. De pagina toont de kleine voorvertoning, en die zou worden afgedrukt. | Selecteer in het Afbeeldingenbeheer de afbeelding en klik op **Koppeling bijwerken**. |
| Afbeelding heeft lage resolutie | De afbeelding heeft minder dan 150 ppi (pixels per inch) op het afgedrukte formaat. | Gebruik een grotere afbeelding, of druk hem kleiner af. |
| Object heeft transparantie of effecten (wordt mogelijk anders afgedrukt) | Doorzichtige gebieden en effecten kunnen op papier er anders uitzien. | Druk een proefpagina af. |
| Lettertype "naam" is niet geïnstalleerd | Een lettertype in uw tekst staat niet op deze computer, dus een ander neemt de plaats in. | Installeer het lettertype, of kies een ander. |

Elke melding eindigt met het paginanummer, zoals "(Pagina 2)".

## Voor een bestand voor een drukkerij

Wanneer u een PDF/X-bestand maakt, waarschuwt JeffPub u voor afbeeldingen met lage resolutie en ontbrekende lettertypen, en biedt hij aan de Ontwerpcontrole te openen. Zie [PDF-bestanden en drukkerijen](pdf).

## Zie ook

- [PDF-bestanden en drukkerijen](pdf)
- [Een publicatie afdrukken](printing)
- [Tekst laten doorlopen tussen tekstvakken](linked-text)
- [Afbeeldingen toevoegen en aanpassen](pictures)
- [Afbeeldingenbeheer](graphics-manager)
