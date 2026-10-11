# Niet-opgeslagen werk terughalen
<!-- keywords: herstellen, herstel, autoherstel, automatisch herstellen, automatisch opslaan, vastgelopen, crash, verloren werk, niet opgeslagen, stroomstoring, terugzetten, bevroren, per ongeluk gesloten, reservekopie, back-up -->

Als JeffPub onverwacht wordt afgesloten of de stroom uitvalt, is uw laatste werk mogelijk nog veilig. Terwijl u werkt, bewaart JeffPub een reservekopie van elke publicatie met wijzigingen die u nog niet hebt opgeslagen. Dit heet **AutoHerstel**.

## Zo werkt AutoHerstel

- Om de paar minuten slaat JeffPub een reservekopie op van elke publicatie met niet-opgeslagen wijzigingen. Standaard gebeurt dit elke 10 minuten.
- JeffPub bewaart de kopieën in een AutoHerstel-map, in de map waarin het programma zijn eigen gegevens bewaart. U hoeft daar zelf nooit te kijken.
- De reservekopie vervangt nooit uw eigenlijke bestand.
- De kopie verdwijnt wanneer u de publicatie opslaat of sluit, of JeffPub op de normale manier afsluit.

AutoHerstel is geen vervanging voor opslaan. Als JeffPub stopt, kunt u het werk van de laatste paar minuten kwijtraken. Druk regelmatig op Ctrl+S.

## Uw werk herstellen

Wanneer u JeffPub na een crash opnieuw start, wordt het venster **Niet-opgeslagen werk herstellen** automatisch geopend. Elke regel toont de naam van de publicatie, waar het bestand stond (of "nooit opgeslagen") en wanneer de kopie is gemaakt.

1. Laat het vinkje staan bij elke publicatie die u terug wilt. Haal het vinkje weg bij de andere.
2. Klik op **Openen**.
3. Controleer het werk en druk op Ctrl+S.

In de titelbalk van de herstelde publicatie staat "(Hersteld)" totdat u deze opslaat. Bij Ctrl+S wordt om een naam gevraagd; u begint in de map van het oorspronkelijke bestand, met de oorspronkelijke naam. Kies de oorspronkelijke naam om het bestand te vervangen, of kies een nieuwe naam om beide te bewaren.

Afbeeldingen die aan bestanden zijn gekoppeld, worden gevonden alsof de publicatie geopend is in de map van het oorspronkelijke bestand: afbeeldingen in die map, of in een map daarbinnen, worden gewoon geladen. Een afbeelding die gekoppeld is aan een bestand op een andere plek, toont het opgeslagen voorbeeld en wordt gemarkeerd als **Niet bijgewerkt**. Selecteer de afbeelding en klik op **Koppeling bijwerken** in [Afbeeldingenbeheer](graphics-manager). Een publicatie die nooit is opgeslagen, heeft geen map; voor al haar gekoppelde afbeeldingen moet u dus **Koppeling bijwerken** gebruiken.

De andere knoppen in het venster zijn:

- **Verwijderen** verwijdert de aangevinkte kopieën definitief, nadat u dit hebt bevestigd.
- **Niet nu** sluit het venster en laat de kopieën staan. JeffPub biedt ze opnieuw aan wanneer het programma de volgende keer start.

JeffPub biedt alleen kopieën aan van een sessie die onverwacht is beëindigd. Kopieën van een JeffPub-venster dat nog geopend is, worden niet aangeboden.

## Hersteld werk later terugvinden

Als u het venster hebt gesloten en uw kopieën toch terug wilt, klikt u op **Bestand > Openen** en vervolgens op **Niet-opgeslagen werk herstellen**. Als er niets te herstellen is, meldt JeffPub dat.

## Instellen hoe vaak wordt opgeslagen

Klik op **Bestand > Opties** en vervolgens op het tabblad **Opslaan**. Wijzig **AutoHerstel-gegevens opslaan elke** in een getal van 1 tot 120 minuten. Hoe kleiner het getal, hoe minder u kunt kwijtraken. De nieuwe tijd is volledig van kracht wanneer u JeffPub de volgende keer start.

## Verwante onderwerpen

- [Publicaties openen en opslaan](open-save)
- [De instellingen van JeffPub wijzigen](options)
- [Hulp krijgen en problemen melden](support)
