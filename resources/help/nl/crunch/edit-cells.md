# Cellen invullen

<!-- keywords: vetgedrukt woord, deel van een cel, opmaak in tekst, deel van tekst opmaken, woord kleuren, invoeren, typen, type, bewerken, wijzigen, cel, invoer, formulebalk, F2, dubbelklikken, Tab, Escape, annuleren, ongedaan maken, opnieuw, verwijderen, wissen, procent, valuta, geld, datum, tijd, getal, tekst, apostrof, formule, is gelijk aan, vervangen, limiet, tekens -->

Om iets in een cel te zetten, selecteert u de cel en begint u met typen. Wat u typt, vervangt wat de cel eerder bevatte. Druk op **Enter** om het te bewaren, of op **Esc** om het op te geven en de cel te laten zoals hij was.

## Drie manieren om te beginnen

- **Typen over de cel.** Selecteer een cel en typ. Het eerste teken dat u typt, begint de invoer, en de cel toont wat u typt terwijl u doorgaat.
- **Wat in de cel staat, bewerken.** Druk op **F2** of dubbelklik op de cel. De cel opent met zijn inhoud, en de invoegpositie staat aan het eind. Gebruik de pijltoetsen om in de tekst te bewegen.
- **De formulebalk gebruiken.** Klik in de formulebalk, naast het vak Naam, en typ daar. De cel toont ook wat u typt.

Wanneer u over een cel typt, rondt een pijltoets de invoer af en verplaatst de selectie. Wanneer u bewerkt wat in de cel staat, verplaatsen de pijltoetsen de invoegpositie in de tekst. Druk op **F2** om van de ene manier naar de andere te gaan.

## De invoer afronden

| Toetsen | Wat ze doen |
| --- | --- |
| Enter | De invoer bewaren en omlaag gaan (of waarheen **Bestand > Opties > Bewerken** aangeeft) |
| Shift+Enter | De invoer bewaren en de andere kant op gaan |
| Ctrl+Enter | De invoer in alle geselecteerde cellen zetten en op uw plek blijven |
| Tab | De invoer bewaren en naar rechts gaan |
| Shift+Tab | De invoer bewaren en naar links gaan |
| Pijltoetsen | De invoer bewaren en één cel verplaatsen (alleen wanneer u over de cel typte) |
| Esc | De invoer opgeven |
| Alt+Enter | Een nieuwe regel in de cel beginnen |
| Ctrl+; | De datum van vandaag invoegen (in de invoer, wanneer u er een typt) |
| Ctrl+Shift+; | De huidige tijd invoegen |
| Alt+omlaag | De woorden die al in de kolom staan weergeven, en er een kiezen |

**Ctrl+Enter** met meerdere geselecteerde cellen zet wat u typte in alle cellen. Een formule wordt geschreven voor de cel waarin u typte, en in elk van de andere cellen leest hij als een kopie ervan zou lezen: als u **=A1** typt terwijl het blok E6:F7 is geselecteerd, staat in F7 **=B2**. Het is één stap voor Ongedaan maken.

**Alt+omlaag** op een cel toont, in volgorde en elk woord één keer, de woorden in de cellen erboven en eronder die niet door een lege cel zijn gescheiden. Wanneer u een woord kiest, wordt het getypt. Een cel met een eigen keuzelijst toont die lijst.

In **Bestand > Opties > Bewerken** kiest u of **Enter** omlaag, naar rechts, omhoog of naar links gaat, of helemaal niet verplaatst.

Als u met **Tab** in meerdere cellen langs een rij typt en daarna op **Enter** drukt, gaat de selectie naar de volgende rij, in de kolom waar u begon. Als u eerst een blok cellen selecteert, lopen **Enter** en **Tab** door het blok.

## Een deel van de tekst opmaken

Sommige woorden in een cel kunnen er anders uitzien dan de rest. Een woord kan bijvoorbeeld vet of rood zijn.

1. Druk op F2 om de cel te bewerken (of dubbelklik erop).
2. Selecteer de woorden die u wilt wijzigen, met Shift en de pijltoetsen of met de muis.
3. Gebruik **Vet**, **Cursief**, **Onderstreept**, **Doorhalen**, **Lettertype**, **Lettergrootte** of **Lettertypekleur** op het tabblad **Start**, of Ctrl+B, Ctrl+I en Ctrl+U.
4. Druk op Enter.

Alleen de woorden die u selecteerde, veranderen. De rest van de cel behoudt de eigen opmaak van de cel. In het invoervak zien de woorden tijdens het bewerken één opmaak, en de cel toont elk deel in zijn eigen opmaak wanneer u op Enter drukt. Als u later woorden typt of verwijdert, verschuift de opmaak met de woorden mee. Een lettertype of lettergrootte kiezen zonder woorden te selecteren, wijzigt de hele cel, zoals altijd.

Cellen die opmaak binnen de tekst hebben van een ander programma, tonen die opmaak, behouden die en slaan die weer op. Een cel die u wijzigt, behoudt zijn opmaak. Opmaak binnen een tekst geldt voor tekst, niet voor getallen of formules. Een kopie die is opgeslagen als een oudere .xls-bestand of als .ods, behoudt de woorden, en bij het opslaan wordt gemeld dat de opmaak binnen de woorden niet wordt behouden.

## Wat u typt, wordt

JeffCrunch leest wat u typt op de manier die de instellingen van uw computer aangeven. Het decimaalteken, het duizendtalteken, het valutateken en de volgorde van dag en maand in een datum komen uit die instellingen. Een getal dat u typt in een cel zonder eigen notatie, krijgt de notatie die het zo toont als u het typte.

| U typt | De cel bevat | En toont |
| --- | --- | --- |
| 1234,5 | het getal 1234,5 | 1234,5 |
| 5% | het getal 0,05 | 5% |
| € 12,50 | het getal 12,5 | € 12,50 |
| 1 1/2 | het getal 1,5 | 1 1/2 |
| 3-4-2025 | de datum, als aantal dagen | 3-4-2025 |
| 22:30 | de tijd, als deel van een dag | 22:30 |
| WAAR | de logische waarde waar | WAAR |
| #N/B | de fout #N/B | #N/B |
| =B2*2 | een formule | het resultaat van de formule |
| al het andere | tekst | de tekst |

Een cel die al een eigen notatie heeft, zoals een percentage of een datum, behoudt die. Een cel waarvan de notatie **Tekst** is, behoudt alles wat u typt als tekst, behalve een formule.

Om iets dat op een getal lijkt als tekst te bewaren, zoals een code **007**, typt u eerst een apostrof: **'007**. De apostrof hoort niet bij de tekst. Wanneer u de cel bewerkt, ziet u hem weer.

## Formules

Een formule begint met een is-gelijkteken. Bijvoorbeeld **=B2*2** verdubbelt het getal in B2. Namen van functies mag u met hoofdletters of kleine letters typen; JeffCrunch schrijft ze met hoofdletters.

De namen van functies zijn altijd de Engelse, in elke taal: typ **SUM**, niet de naam die uw taal eraan geeft. De scheidingstekens ertussen volgen de instellingen van uw computer. Waar het decimaalteken een komma is, zoals in het Duits en Frans, scheidt u de argumenten van een functie met een puntkomma en schrijft u in een getal een komma: **=SUM(A1;A2)*0,5**. De formulebalk en **F2** tonen formules op dezelfde manier. Waar het decimaalteken een punt is, scheidt u argumenten met een komma: **=SUM(A1,A2)*0.5**. Het bestand bewaart altijd de vorm die elk programma leest, met komma's tussen argumenten en punten in getallen, zodat het overal hetzelfde opent.

Als een formule een fout bevat, zoals een haakje dat nooit wordt gesloten, vertelt JeffCrunch wat er mis is. De invoer blijft zoals u hem typte, met de invoegpositie in de buurt van de fout, zodat u hem kunt herstellen. Er wordt niets gewijzigd totdat de formule klopt.

Een cel bevat maximaal 32.767 tekens, en een formule maximaal 8.192. JeffCrunch meldt wanneer een invoer langer is.

## Cellen wissen

Selecteer de cellen en druk op **Delete**. Dit wist wat de cellen bevatten en behoudt hun notatie, en het is één stap die u ongedaan kunt maken. JeffCrunch wist maximaal 100.000 cellen tegelijk.

Op het tabblad **Start** heeft de knop **Wissen** een menu. **Alles wissen** maakt de cellen leeg, en wist ook hun opmaak, notities en koppelingen. **Opmaak wissen** behoudt wat de cellen bevatten en zet de gewone opmaak terug. **Inhoud wissen** is hetzelfde als **Delete**. **Opmerkingen en notities wissen** haalt alleen de notities van de geselecteerde cellen weg. **Hyperlinks wissen** haalt alleen de koppelingen weg, en de cellen behouden hun tekst. Een keuze is grijs wanneer de geselecteerde cellen niets hebben om te wissen.

## Ongedaan maken en Opnieuw

**{key:Undo}** maakt de laatste wijziging ongedaan, en **{key:Redo}** voert haar opnieuw uit. Ze staan ook op de snelle werkbalk bovenaan. Elke cel die u invult, is één stap, en elk wissen ook. Ongedaan maken zet de cel terug zoals hij was: de waarde, de formule, de notatie en de formules die ervan afhangen. JeffCrunch onthoudt maximaal 1.000 stappen, minder wanneer ze heel veel cellen wijzigden, zodat er nooit veel geheugen nodig is.

## Terwijl JeffCrunch de formules berekent

Wanneer u een cel wijzigt, berekent JeffCrunch de formules die ervan afhangen op de achtergrond. De cel die u wijzigde, toont meteen zijn nieuwe inhoud, en de andere cellen worden even later bijgewerkt; de statusbalk toont **Berekenen** totdat dat klaar is. Intussen kunt u gewoon doorgaan met typen.

## Wat niet kan worden gewijzigd

- Een cel die deel uitmaakt van een **matrixformule** of een gegevenstabel, kan niet alleen worden gewijzigd.
- Op een **beveiligd blad** kan een vergrendelde cel niet worden gewijzigd. Elke cel is vergrendeld, tenzij de werkmap aangeeft dat hij dat niet is. JeffCrunch meldt het wanneer u het probeert. Om de cel te wijzigen, kiest u op het tabblad **Controleren** de optie **Beveiliging blad opheffen**.
- De eerste rij van een **tabel** bevat de namen van de kolommen. Typ een nieuwe naam in een van die cellen om de kolom te hernoemen: de formules die de oude naam gebruiken, volgen, en de tabel in het bestand blijft geldig. Een naam die een andere kolom al heeft, krijgt er een getal achter, een naam mag niet leeg zijn, en **Delete** wist die rij niet.
- Een werkmap die niet in zijn geheel kon worden gelezen, kan wel worden bekeken, maar niet worden gewijzigd of opgeslagen.
- Een functie die JeffCrunch nog niet kent, laat een nieuwe formule zonder waarde in de cel achter; de werkmap vraagt het programma dat hem opent om de formule uit te rekenen.
- Opmaak, sorteren, en zoeken en vervangen komen in een latere versie. Invoegen en verwijderen, knippen en plakken, doorvoeren, en de afmetingen van rijen en kolommen staan in [Rijen, kolommen en cellen invoegen en verwijderen](insert-delete), [Knippen, kopiëren en plakken](clipboard), [Cellen doorvoeren en optellen](fill-sum), en [De breedte van kolommen en de hoogte van rijen wijzigen](sizes).

## Nieuwe werkmappen

In **Bestand > Opties > Nieuwe werkmappen** kiest u het lettertype, de lettergrootte en het aantal bladen waarmee elke nieuwe werkmap begint. Werkmappen die u al hebt, worden niet gewijzigd.
