# Uw ontwerp controleren
<!-- keywords: ontwerpcontrole, controleren, fouten, problemen, lage resolutie, overloop, overlooptekst, tekst past niet, leeg tekstvak, buiten de pagina, preflight, eindcontrole, drukproef, gekoppelde afbeelding, ontbrekend lettertype -->

De **Ontwerpcontrole** doorzoekt uw publicatie op dingen die vaak misgaan. De controle vindt tekst die niet past, lege vakken, objecten die buiten de pagina uitsteken en afbeeldingen die slecht worden afgedrukt. Voer de controle uit voordat u afdrukt of een bestand naar een drukkerij stuurt.

## De Ontwerpcontrole uitvoeren

Klik op **Controleren > Controleren > Ontwerpcontrole uitvoeren**. Rechts wordt het taakvenster **Ontwerpcontrole** geopend. U kunt ook op **Bestand > Eigenschappen** klikken en daarna op **Ontwerpcontrole uitvoeren**.

Het taakvenster heeft twee selectievakjes, die standaard allebei zijn ingeschakeld:

- **Algemene ontwerpcontroles uitvoeren** zoekt naar problemen in de indeling.
- **Laatste publicatiecontroles uitvoeren** zoekt naar dingen die belangrijk zijn bij het afdrukken.

Laat beide ingeschakeld om alle soorten problemen te zien. De lijst wordt bijgewerkt terwijl u werkt. Als er niets te herstellen is, staat er "Geen problemen gevonden."

## Een probleem herstellen

1. Klik op een item in de lijst onder **Selecteer een item om te herstellen**.
2. Klik op **Naar item**. U kunt ook dubbelklikken op het item.

JeffPub gaat naar de pagina en selecteert het object. Herstel het probleem, waarna het item uit de lijst verdwijnt.

## Wat de controle vindt

| Melding | Wat het betekent | Wat u kunt doen |
|---|---|---|
| Pagina heeft geen inhoud | De pagina is leeg. | Voeg iets toe of verwijder de pagina. |
| Object valt gedeeltelijk buiten de pagina | Een deel van een object ligt buiten de pagina. | Verplaats het, of laat het staan als het om afloop gaat. |
| Tekstvak is leeg | Een tekstvak bevat geen tekst. | Typ er tekst in of verwijder het. |
| Artikel met overlooptekst | Een artikel is de tekst die door gekoppelde tekstvakken loopt. Dit artikel bevat meer tekst dan erin past. | Vergroot een tekstvak, kort de tekst in of [koppel een ander tekstvak](linked-text). |
| Tekst is sterk verkleind om te passen | Door de tekst te verkleinen zodat die in het vak past, is de tekst minder dan half zo groot geworden. | Vergroot het tekstvak of gebruik minder tekst. |
| Tijdelijke aanduiding voor afbeelding is leeg | Een afbeeldingskader bevat geen afbeelding. | Klik erop om een afbeelding toe te voegen of verwijder het. |
| Afbeelding is niet proportioneel geschaald | De afbeelding is uitgerekt of samengedrukt. | Wijzig de grootte zodat de oorspronkelijke vorm terugkomt. |
| Afbeelding is gekoppeld, niet ingesloten | De afbeelding is gekoppeld aan een bestand en in de publicatie is alleen een klein voorbeeld opgeslagen. Een afbeelding die met **Invoegen en koppelen** is ingevoegd, wordt volledig opgeslagen en staat niet in de lijst. | Bewaar het oorspronkelijke bestand bij uw publicatie, of klik op **Afbeelding insluiten** in [Afbeeldingenbeheer](graphics-manager). |
| Gekoppelde afbeelding ontbreekt | Het bestand waarnaar een gekoppelde afbeelding verwijst, bestaat niet meer. De pagina toont daardoor alleen het kleine voorbeeld, en dat wordt ook afgedrukt. | Klik in Afbeeldingenbeheer op **Koppeling wijzigen…** om het bestand te zoeken, of zet het bestand terug en klik op **Koppeling bijwerken**. |
| Gekoppelde afbeelding toont alleen het voorbeeld tot Koppeling bijwerken | De afbeelding is gekoppeld aan een bestand buiten de map van de publicatie, dat JeffPub niet uit zichzelf opent. De pagina toont het kleine voorbeeld, en dat wordt ook afgedrukt. | Selecteer de afbeelding in Afbeeldingenbeheer en klik op **Koppeling bijwerken**. |
| Afbeelding heeft een lage resolutie | De afbeelding heeft minder dan 150 ppi (pixels per inch) op het afgedrukte formaat. | Gebruik een grotere afbeelding of druk de afbeelding kleiner af. |
| Object heeft transparantie of effecten (wordt mogelijk anders afgedrukt) | Doorzichtige delen en effecten kunnen er op papier anders uitzien. | Druk een testpagina af. |
| Lettertype "naam" is niet geïnstalleerd | Een lettertype in uw tekst staat niet op deze computer, dus wordt een ander lettertype gebruikt. | Installeer het lettertype of kies een ander lettertype. |

Elke melding eindigt met het paginanummer, bijvoorbeeld "(Pagina 2)".

## Voordat u een bestand voor een drukkerij maakt

Wanneer u een PDF/X-bestand maakt, waarschuwt JeffPub u voor afbeeldingen met een lage resolutie en ontbrekende lettertypen, en biedt het aan de Ontwerpcontrole te openen. Zie [PDF-bestanden en drukkerijen](pdf).

## Verwante onderwerpen

- [PDF-bestanden en drukkerijen](pdf)
- [Een publicatie afdrukken](printing)
- [Tekst laten doorlopen tussen tekstvakken](linked-text)
- [Afbeeldingen toevoegen en aanpassen](pictures)
- [Afbeeldingenbeheer](graphics-manager)
