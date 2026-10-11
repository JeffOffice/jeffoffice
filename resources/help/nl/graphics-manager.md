# Afbeeldingenbeheer
<!-- keywords: afbeeldingenbeheer, afbeeldingen, foto's, plaatjes, gekoppelde afbeelding, koppelen aan bestand, koppeling bijwerken, koppeling wijzigen, afbeelding insluiten, ingesloten, ontbreekt, ontbrekende afbeelding, gewijzigd, niet bijgewerkt, buiten map, opnieuw koppelen, afbeeldingsbestand zoeken, status afbeelding, resolutie -->

**Afbeeldingenbeheer** toont alle afbeeldingen in de publicatie en geeft aan waar elke afbeelding vandaan komt. Dit is vooral belangrijk voor afbeeldingen die u aan hun bestand hebt gekoppeld, omdat een gekoppelde afbeelding afhankelijk is van een bestand buiten de publicatie. Zie [Afbeeldingen toevoegen en aanpassen](pictures) voor informatie over het koppelen van een afbeelding.

## Openen

Klik op **Beeld > Weergeven > Afbeeldingenbeheer**. Het deelvenster wordt rechts geopend. Klik nogmaals op dezelfde knop of op de **X** boven in het deelvenster om het te sluiten.

## De lijst lezen

Elke rij toont een kleine afbeelding, de bestandsnaam, de pagina, het bestandstype, de grootte in kB en de **status**. Met het vak boven de lijst sorteert u de rijen op pagina, naam, grootte of type. Onder de lijst staan de details van de geselecteerde afbeelding: de status, het bestand, of de publicatie de volledige afbeelding bewaart, de grootte in pixels en de **effectieve resolutie**: hoeveel pixels van de afbeelding er op elke inch van de pagina vallen. Ongeveer 300 is goed voor drukwerk.

| Status | Betekenis |
|---|---|
| Ingesloten | De afbeelding is in de publicatie opgeslagen. Er is niets buiten de publicatie nodig. |
| Gekoppeld | De afbeelding is aan een bestand gekoppeld en het bestand staat ongewijzigd op de plek waar JeffPub het verwacht. |
| Ontbreekt | De afbeelding is gekoppeld aan een bestand dat er niet meer is. Als de publicatie geen kopie bewaart, toont de pagina het kleine voorbeeld, dat er wazig uitziet, en dat wordt ook afgedrukt. |
| Gewijzigd | Het bestand is gewijzigd sinds de koppeling is gemaakt of voor het laatst is bijgewerkt. Klik op **Koppeling bijwerken** om de wijzigingen over te nemen. |
| Niet bijgewerkt | De afbeelding is gekoppeld aan een bestand buiten de map van de publicatie, dus JeffPub heeft dat bestand niet geopend. De pagina toont wat de publicatie heeft opgeslagen. Klik op **Koppeling bijwerken** om het bestand te gebruiken. Zie 'Afbeeldingen van buiten de map van de publicatie' hieronder. |

## Wat de knoppen doen

- **Ga naar** selecteert de afbeelding op de pagina.
- **Opslaan als…** slaat een kopie van de afbeelding op in een bestand.
- **Vervangen…** plaatst een ander bestand in hetzelfde kader.
- **Koppeling bijwerken** leest het bestand opnieuw in. Gebruik deze knop voor een afbeelding met de status **Gewijzigd**, nadat u een ontbrekend bestand hebt teruggezet, of voor een afbeelding met de status **Niet bijgewerkt**. Alle afbeeldingen met dezelfde koppeling worden bijgewerkt, en de afbeelding behoudt haar plaats en breedte.
- **Koppeling wijzigen…** laat de afbeelding naar een ander bestand verwijzen, bijvoorbeeld als u de bestanden naar een andere map hebt verplaatst.
- **Afbeelding insluiten** zet de koppeling om in een kopie die in de publicatie wordt opgeslagen. De afbeelding krijgt dan de status **Ingesloten**. Als het bestand ontbreekt, kan alleen het kleine voorbeeld worden ingesloten; de statusbalk meldt dit.

De drie koppelingsknoppen zijn grijs, tenzij de geselecteerde afbeelding gekoppeld is. Elke knop voert één stap uit, dus met **Ctrl+Z** maakt u die ongedaan.

## Afbeeldingen van buiten de map van de publicatie

Wanneer u een publicatie opent, leest JeffPub de bestanden van gekoppelde afbeeldingen die in dezelfde map als de publicatie staan, of in een submap daarvan. Bestanden op andere plaatsen, op uw computer of in uw netwerk, worden niet bekeken.

De reden: een publicatie die iemand u stuurt, kan naar elk willekeurig bestand verwijzen. Als JeffPub alles opende waarnaar de publicatie verwijst, zou de publicatie van een onbekende een van uw privéafbeeldingen op haar pagina's kunnen halen, die u vervolgens ongemerkt zou kunnen opslaan of terugsturen. In een netwerk kan alleen al controleren of een bestand bestaat, uw Windows-aanmeldgegevens naar een andere computer sturen.

Een afbeelding die aan een bestand op een andere plek is gekoppeld, toont daarom wat de publicatie heeft opgeslagen: de volledige afbeelding als de publicatie die bewaart, en anders het kleine voorbeeld, dat er wazig uitziet. De status is **Niet bijgewerkt** en de details vermelden dat het bestand buiten de map staat.

Als u het bestand wilt gebruiken, selecteert u de afbeelding en klikt u op **Koppeling bijwerken**, of klikt u op **Koppeling wijzigen…** en kiest u het bestand zelf. Daarmee geeft u toestemming voor die ene afbeelding, en die toestemming geldt tot u de publicatie sluit. Andere afbeeldingen blijven **Niet bijgewerkt** totdat u voor elk hetzelfde doet.

Bij opslaan blijft de koppeling zoals ze was, dus de volgende keer dat u de publicatie opent, is de afbeelding weer **Niet bijgewerkt**. Wilt u dat afbeeldingen automatisch worden geladen, bewaar ze dan in de map van de publicatie of in een submap daarvan.

Afdrukken, opslaan als PDF en opslaan als e-book gebruiken wat de pagina toont. Klik bij een afbeelding met de status **Niet bijgewerkt** dus op **Koppeling bijwerken** voordat u afdrukt. De [Ontwerpcontrole](design-checker) geeft deze afbeeldingen weer.

## Een ontbrekende afbeelding herstellen

1. Selecteer de afbeelding met de status **Ontbreekt** in de lijst.
2. Klik op **Koppeling wijzigen…** en kies het bestand. U kunt het bestand ook terugzetten op de oorspronkelijke plek en op **Koppeling bijwerken** klikken.
3. Als het bestand definitief weg is, klikt u op **Afbeelding insluiten** om het voorbeeld te behouden, of op **Vervangen…** om een andere afbeelding te gebruiken.

De [Ontwerpcontrole](design-checker) geeft ook ontbrekende gekoppelde afbeeldingen weer.

## Verwante onderwerpen

- [Afbeeldingen toevoegen en aanpassen](pictures)
- [Uw ontwerp controleren](design-checker)
- [Opslaan als afbeelding, webpagina of e-book](export)
