# Typen in cellen
<!-- keywords: typen, invoeren, invoer, bewerken, wijzigen, cel, celinhoud, formulebalk, F2, dubbelklikken, Tab, Esc, annuleren, ongedaan maken, opnieuw, verwijderen, wissen, procent, percentage, valuta, geld, datum, tijd, getal, tekst, apostrof, formule, gelijkteken, vervangen, limiet, maximum, tekens -->

Als u iets in een cel wilt zetten, selecteert u de cel en begint u te typen. Wat u typt, vervangt wat er in de cel stond. Druk op **Enter** om het te behouden, of op **Esc** om het te annuleren en de cel te laten zoals hij was.

## Drie manieren om te beginnen

- **Over de cel heen typen.** Selecteer een cel en typ. Het eerste teken dat u typt, begint de invoer, en de cel toont wat u typt terwijl u typt.
- **De inhoud van de cel bewerken.** Druk op **F2** of dubbelklik op de cel. De cel wordt geopend met de inhoud, en de cursor staat aan het einde. Gebruik de pijltoetsen om door de tekst te gaan.
- **De formulebalk gebruiken.** Klik in de formulebalk, naast het naamvak, en typ daar. De cel toont ook wat u typt.

Als u over een cel heen typt, voltooit een pijltoets de invoer en verplaatst die de selectie. Als u de inhoud van de cel bewerkt, gaan de pijltoetsen door de tekst. Druk op **F2** om tussen deze twee te wisselen.

## De invoer voltooien

| Toetsen | Wat ze doen |
| --- | --- |
| Enter | Invoer behouden en omlaag gaan |
| Shift+Enter | Invoer behouden en omhoog gaan |
| Tab | Invoer behouden en naar rechts gaan |
| Shift+Tab | Invoer behouden en naar links gaan |
| Pijltoetsen | Invoer behouden en één cel verdergaan (alleen als u over de cel heen hebt getypt) |
| Esc | Invoer annuleren |
| Alt+Enter | Nieuwe regel beginnen in de cel |

Als u met **Tab** in meerdere cellen van een rij typt en daarna op **Enter** drukt, gaat de selectie naar de volgende rij, in de kolom waar u begon. Als u eerst een blok cellen selecteert, gaan **Enter** en **Tab** door dat blok.

## Wat er van uw invoer wordt

JeffCrunch leest wat u typt volgens de instellingen van uw computer. Het decimaalteken, het scheidingsteken voor duizendtallen, het valutasymbool en de volgorde van dag en maand in een datum komen allemaal uit die instellingen. Een getal dat u typt in een cel zonder eigen opmaak, krijgt de opmaak die het weergeeft zoals u het hebt getypt.

In de voorbeelden hieronder zijn Nederlandse instellingen gebruikt.

| U typt | De cel bevat | En toont |
| --- | --- | --- |
| 1234,5 | het getal 1234,5 | 1234,5 |
| 5% | het getal 0,05 | 5% |
| € 12,50 | het getal 12,5 | € 12,50 |
| 3-4-2025 | de datum, als een aantal dagen | 3-4-2025 |
| 22:30 | de tijd, als een deel van een dag | 22:30 |
| TRUE | de logische waarde waar | TRUE |
| #N/A | de fout #N/A | #N/A |
| =B2*2 | een formule | het resultaat van de formule |
| iets anders | tekst | de tekst |

Een cel die al een eigen opmaak heeft, zoals procent of datum, behoudt die. Een cel met de opmaak **Tekst** bewaart alles wat u typt als tekst, behalve een formule.

Als u iets dat op een getal lijkt als tekst wilt bewaren, zoals de code **007**, typt u eerst een apostrof: **'007**. De apostrof hoort niet bij de tekst. Wanneer u de cel bewerkt, ziet u hem weer.

## Formules

Een formule begint met een gelijkteken. Zo verdubbelt **=B2*2** het getal in B2. U kunt namen van functies in hoofdletters of kleine letters typen; JeffCrunch schrijft ze in hoofdletters.

De namen van functies zijn altijd de Engelse, in elke taal: typ **SUM**, niet de Nederlandse naam. De tekens ertussen volgen de instellingen van uw computer. Waar het decimaalteken een komma is, zoals in het Nederlands, scheidt u de argumenten van een functie met een puntkomma en schrijft u een komma in een getal: **=SUM(A1;A2)*0,5**. De formulebalk en **F2** tonen formules op dezelfde manier. Waar het decimaalteken een punt is, scheidt u argumenten met een komma: **=SUM(A1,A2)*0.5**. Het bestand bewaart altijd de vorm die elk programma kan lezen, met komma's tussen argumenten en punten in getallen, zodat het overal hetzelfde wordt geopend.

Als een formule een fout bevat, zoals een haakje dat nooit wordt gesloten, meldt JeffCrunch wat er mis is. De invoer blijft staan zoals u hem typte, met de cursor bij de fout, zodat u die kunt herstellen. Er wordt niets gewijzigd totdat de formule klopt.

Een cel kan maximaal 32.767 tekens bevatten, en een formule maximaal 8.192. JeffCrunch meldt het wanneer een invoer langer is.

## Cellen wissen

Selecteer de cellen en druk op **Delete**. Hiermee wist u de inhoud van de cellen en blijft hun opmaak behouden; dit is één stap die u ongedaan kunt maken. JeffCrunch wist maximaal 100.000 cellen tegelijk.

## Ongedaan maken en Opnieuw

Met **{key:Undo}** maakt u de laatste wijziging ongedaan, en met **{key:Redo}** voert u die opnieuw uit. Deze opdrachten staan ook op de werkbalk Snelle toegang bovenaan. Elke cel waarin u typt, is één stap, en dat geldt ook voor elke keer dat u wist. Ongedaan maken zet de cel terug zoals hij was: de waarde, de formule, de opmaak en de formules die ervan afhangen. JeffCrunch onthoudt maximaal 1.000 stappen, minder als ze heel veel cellen hebben gewijzigd, zodat het nooit veel geheugen gebruikt.

## Terwijl JeffCrunch de formules berekent

Wanneer u een cel wijzigt, berekent JeffCrunch op de achtergrond de formules die ervan afhangen. De gewijzigde cel toont meteen de nieuwe inhoud, en de andere cellen worden even later bijgewerkt; tot dan staat **Berekenen** op de statusbalk. U kunt intussen gewoon doortypen.

## Wat niet kan worden gewijzigd

- Een cel die deel uitmaakt van een **matrixformule** of een gegevenstabel, kan niet afzonderlijk worden gewijzigd.
- Op een **beveiligd blad** kan een vergrendelde cel niet worden gewijzigd. Elke cel is vergrendeld, tenzij de werkmap aangeeft dat dat niet zo is. JeffCrunch meldt het wanneer u het probeert. Als u de cel wilt wijzigen, kiest u Beveiliging blad opheffen op het tabblad Controleren.
- De eerste rij van een **tabel** bevat de namen van de kolommen. Typ een nieuwe naam in een van die cellen om de kolom een andere naam te geven: de formules die de oude naam gebruiken, worden aangepast, en de tabel in het bestand blijft geldig. Een naam die een andere kolom al heeft, krijgt er een nummer achter, een naam mag niet leeg zijn, en **Delete** wist die rij niet.
- Een werkmap die niet helemaal kon worden gelezen, kunt u bekijken maar niet wijzigen of opslaan.
- Een functie die JeffCrunch nog niet kent, laat een nieuwe formule zonder waarde in de cel staan; de werkmap vraagt het programma dat hem daarna opent, de formule te berekenen.
- Opmaak, sorteren en zoeken en vervangen komen in een latere versie. Invoegen en verwijderen, knippen en plakken, doorvoeren en de grootte van rijen en kolommen worden beschreven in [Rijen, kolommen en cellen invoegen en verwijderen](insert-delete), [Knippen, kopiëren en plakken](clipboard), [Cellen doorvoeren en optellen](fill-sum) en [De breedte van kolommen en de hoogte van rijen wijzigen](sizes).
