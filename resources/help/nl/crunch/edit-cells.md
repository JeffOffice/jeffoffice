# Typen in cellen
<!-- keywords: typen, invoeren, invoer, bewerken, wijzigen, cel, celinhoud, formulebalk, F2, dubbelklikken, Tab, Esc, annuleren, ongedaan maken, opnieuw, verwijderen, wissen, procent, percentage, valuta, geld, datum, tijd, getal, tekst, apostrof, formule, isgelijkteken, vervangen, limiet, tekens -->

Als u iets in een cel wilt zetten, selecteert u de cel en begint u te typen. Wat u typt, vervangt wat er in de cel stond. Druk op **Enter** om het te behouden, of op **Esc** om het te annuleren en de cel te laten zoals hij was.

## Drie manieren om te beginnen

- **Over de cel heen typen.** Selecteer een cel en typ. Het eerste teken dat u typt, begint de invoer, en de cel geeft weer wat u typt terwijl u typt.
- **De inhoud van de cel bewerken.** Druk op **F2** of dubbelklik op de cel. De cel wordt geopend met de inhoud, en de cursor staat aan het einde. Gebruik de pijltoetsen om door de tekst te gaan.
- **De formulebalk gebruiken.** Klik in de formulebalk, naast het naamvak, en typ daar. De cel geeft ook weer wat u typt.

Als u over een cel heen typt, voltooit een pijltoets de invoer en verplaatst die de selectie. Als u de inhoud van de cel bewerkt, gaan de pijltoetsen door de tekst. Druk op **F2** om tussen deze twee te wisselen.

## De invoer voltooien

| Toetsen | Wat ze doen |
| --- | --- |
| Enter | De invoer behouden en omlaag gaan (of in de richting die is ingesteld bij **Bestand > Opties > Bewerken**) |
| Shift+Enter | De invoer behouden en de andere kant op gaan |
| Ctrl+Enter | De invoer in elke geselecteerde cel zetten en blijven waar u bent |
| Tab | De invoer behouden en naar rechts gaan |
| Shift+Tab | De invoer behouden en naar links gaan |
| Pijltoetsen | De invoer behouden en één cel verder gaan (alleen als u over de cel heen hebt getypt) |
| Esc | De invoer annuleren |
| Alt+Enter | Een nieuwe regel in de cel beginnen |
| Ctrl+; | De datum van vandaag invoegen (in de invoer, als u aan het typen bent) |
| Ctrl+Shift+; | De huidige tijd invoegen |
| Alt+Pijl-omlaag | De woorden weergeven die al in de kolom staan, en er een kiezen |

Met **Ctrl+Enter** zet u wat u hebt getypt in alle geselecteerde cellen. Een formule wordt geschreven voor de cel waarin u typte, en in elk van de andere cellen staat die zoals een kopie ervan zou luiden: als u **=A1** typt terwijl het blok E6:F7 is geselecteerd, staat er in F7 **=B2**. Dit is één stap voor Ongedaan maken.

Met **Alt+Pijl-omlaag** op een cel worden de woorden weergegeven, op volgorde en elk één keer, die in de cellen erboven en eronder staan zolang er geen lege cel tussen zit. Als u er een kiest, wordt het getypt. Een cel met een eigen lijst geeft in plaats daarvan die lijst weer.

Kies bij **Bestand > Opties > Bewerken** of **Enter** omlaag, naar rechts, omhoog of naar links gaat, of niet verplaatst.

Als u met **Tab** in meerdere cellen van een rij typt en daarna op **Enter** drukt, gaat de selectie naar de volgende rij, in de kolom waar u begon. Als u eerst een blok cellen selecteert, gaan **Enter** en **Tab** door het blok.

## Wat er van uw invoer wordt

JeffCrunch leest wat u typt volgens de instellingen van uw computer. Het decimaalteken, het scheidingsteken voor duizendtallen, het valutasymbool en de volgorde van dag en maand in een datum komen allemaal uit die instellingen. Een getal dat u typt in een cel zonder eigen opmaak, krijgt de opmaak die het weergeeft zoals u het typte.

| U typt | De cel bevat | En geeft weer |
| --- | --- | --- |
| 1234,5 | het getal 1234,5 | 1234,5 |
| 5% | het getal 0,05 | 5% |
| € 12,50 | het getal 12,5 | € 12,50 |
| 1 1/2 | het getal 1,5 | 1 1/2 |
| 3-4-2025 | de datum, als een aantal dagen | 3-4-2025 |
| 22:30 | de tijd, als deel van een dag | 22:30 |
| WAAR | de logische waarde waar | WAAR |
| #N/B | de fout #N/B | #N/B |
| =B2*2 | een formule | het resultaat van de formule |
| iets anders | tekst | de tekst |

Een cel die al een eigen opmaak heeft, zoals een percentage of een datum, behoudt die. Een cel met de opmaak **Tekst** bewaart alles wat u typt als tekst, behalve een formule.

Als u iets dat op een getal lijkt als tekst wilt bewaren, zoals de code **007**, typt u eerst een apostrof: **'007**. De apostrof hoort niet bij de tekst. Als u de cel bewerkt, ziet u hem weer.

## Formules

Een formule begint met een isgelijkteken. Zo verdubbelt **=B2*2** het getal in B2. U kunt de namen van functies in hoofdletters of kleine letters typen; JeffCrunch schrijft ze in hoofdletters.

De namen van functies zijn altijd de Engelse, in elke taal: typ **SUM**, niet de Nederlandse naam. De tekens ertussen volgen de instellingen van uw computer. Waar het decimaalteken een komma is, zoals in het Nederlands, scheidt u de argumenten van een functie met een puntkomma en schrijft u een komma in een getal: **=SUM(A1;A2)*0,5**. De formulebalk en **F2** geven formules op dezelfde manier weer. Waar het decimaalteken een punt is, scheidt u argumenten met een komma: **=SUM(A1,A2)*0.5**. Het bestand bewaart altijd de vorm die elk programma leest, met komma's tussen argumenten en punten in getallen, zodat het overal hetzelfde wordt geopend.

Als een formule een fout bevat, zoals een haakje dat nooit wordt gesloten, meldt JeffCrunch wat er mis is. De invoer blijft staan zoals u die typte, met de cursor bij de fout, zodat u die kunt herstellen. Er wordt niets gewijzigd totdat de formule klopt.

Een cel kan maximaal 32.767 tekens bevatten en een formule maximaal 8.192. JeffCrunch meldt het als een invoer langer is.

## Cellen wissen

Selecteer de cellen en druk op **Delete**. Hiermee wist u de inhoud van de cellen; de opmaak blijft behouden. Dit is één stap die u ongedaan kunt maken. JeffCrunch wist maximaal 100.000 cellen tegelijk.

## Ongedaan maken en Opnieuw

Met **{key:Undo}** maakt u de laatste wijziging ongedaan, en met **{key:Redo}** voert u die opnieuw uit. Deze opdrachten staan ook op de werkbalk Snelle toegang bovenaan. Elke cel waarin u typt is één stap, en dat geldt ook voor elke keer dat u cellen wist. Ongedaan maken zet de cel terug zoals hij was: de waarde, de formule, de opmaak en de formules die ervan afhangen. JeffCrunch onthoudt maximaal 1.000 stappen, minder als ze heel veel cellen wijzigden, zodat het nooit veel geheugen gebruikt.

## Terwijl JeffCrunch de formules berekent

Als u een cel wijzigt, berekent JeffCrunch op de achtergrond de formules die ervan afhangen. De cel die u wijzigde, geeft meteen de nieuwe inhoud weer, en de andere cellen worden even later bijgewerkt; in de statusbalk staat **Berekenen** totdat dat is gebeurd. U kunt intussen gewoon doortypen.

## Wat niet kan worden gewijzigd

- Een cel die deel uitmaakt van een **matrixformule** of een gegevenstabel, kan niet afzonderlijk worden gewijzigd.
- Op een **beveiligd blad** kan een vergrendelde cel niet worden gewijzigd. Elke cel is vergrendeld, tenzij de werkmap aangeeft dat dit niet zo is. JeffCrunch meldt het als u het probeert. Als u de cel wilt wijzigen, kiest u Beveiliging blad opheffen op het tabblad Controleren.
- De eerste rij van een **tabel** bevat de namen van de kolommen. Typ een nieuwe naam in een van die cellen om de kolom een andere naam te geven: de formules die de oude naam gebruiken, worden aangepast, en de tabel in het bestand blijft geldig. Een naam die een andere kolom al heeft, krijgt een nummer erachter, een naam mag niet leeg zijn, en **Delete** wist die rij niet.
- Een werkmap die niet volledig kon worden gelezen, kan worden bekeken, maar niet worden gewijzigd of opgeslagen.
- Een functie die JeffCrunch nog niet kent, laat een nieuwe formule zonder waarde in de cel achter; de werkmap vraagt het programma dat hem daarna opent om de formule te berekenen.
- Opmaak, sorteren en zoeken en vervangen komen in een latere versie. Invoegen en verwijderen, knippen en plakken, doorvoeren en de grootte van rijen en kolommen worden beschreven in [Rijen, kolommen en cellen invoegen en verwijderen](insert-delete), [Knippen, kopiëren en plakken](clipboard), [Cellen doorvoeren en optellen](fill-sum) en [De breedte van kolommen en de hoogte van rijen wijzigen](sizes).
