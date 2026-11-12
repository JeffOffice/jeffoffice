# Cellen invoeren en bewerken
<!-- keywords: symbool, symboollettertype, speciale tekens, sneltoets voor een symbool, automatische correctie, slimme aanhalingstekens, streepje, breuk, koppeling, adres, vetgedrukt woord, deel van een cel, opmaak in tekst, opmaak van een deel van de tekst, kleur een woord, tekst, typen, invoeren, bewerken, wijzigen, cel, invoer, formulebalk, F2, dubbelklikken, Tab, Esc, annuleren, ongedaan maken, opnieuw, verwijderen, wissen, procent, valuta, geld, datum, tijd, getal, tekst, apostrof, formule, is gelijk aan, vervangen, limiet, tekens -->

Om iets in een cel te zetten, selecteert u de cel en begint u met typen. Wat u typt, vervangt wat de cel bevatte. Druk op **Enter** om het te bewaren, of op **Esc** om het op te geven en de cel te laten zoals hij was.

## Drie manieren om te beginnen

- **Typen over de cel.** Selecteer een cel en typ. Het eerste teken dat u typt, begint de invoer, en de cel toont wat u typt terwijl u dat doet.
- **De inhoud van de cel bewerken.** Druk op **F2** of dubbelklik op de cel. De cel opent met zijn inhoud en de cursor staat aan het eind. Gebruik de pijltoetsen om in de tekst te bewegen.
- **De formulebalk gebruiken.** Klik in de formulebalk, naast het Naamvak, en typ daar. De cel toont ook wat u typt.

Wanneer u over een cel typt, rondt een pijltoets de invoer af en verplaatst de selectie. Wanneer u de inhoud van de cel bewerkt, verplaatsen de pijltoetsen de cursor in de tekst. Druk op **F2** om van de ene naar de andere modus te wisselen.

## De invoer afronden

| Toetsen | Wat ze doen |
| --- | --- |
| Enter | De invoer bewaren en omlaag gaan (of waarheen **Bestand > Opties > Bewerken** aangeeft) |
| Shift+Enter | De invoer bewaren en de andere kant op gaan |
| Ctrl+Enter | De invoer in alle geselecteerde cellen zetten en op de plek blijven |
| Tab | De invoer bewaren en naar rechts gaan |
| Shift+Tab | De invoer bewaren en naar links gaan |
| Pijltoetsen | De invoer bewaren en één cel verplaatsen (alleen wanneer u over de cel typte) |
| Esc | De invoer opgeven |
| Alt+Enter | Een nieuwe regel binnen de cel beginnen |
| Ctrl+; | De datum van vandaag invoegen (in de invoer, wanneer u er een typt) |
| Ctrl+Shift+; | De huidige tijd invoegen |
| Alt+Omlaag | De woorden die al in de kolom staan weergeven, en er één kiezen |

**Ctrl+Enter** met meerdere geselecteerde cellen zet wat u typte in al die cellen. Een formule is geschreven voor de cel waarin u typte, en in elk van de andere cellen leest hij zoals een kopie ervan zou lezen: als u **=A1** typt terwijl het blok E6:F7 is geselecteerd, staat in F7 **=B2**. Het is één stap van Ongedaan maken.

**Alt+Omlaag** op een cel toont, in volgorde en elk één keer, de woorden in de cellen erboven en eronder die niet door een lege cel zijn gescheiden. Kiest u een woord, dan wordt het getypt. Heeft een cel een eigen lijst, dan toont die cel die lijst.

In **Bestand > Opties > Bewerken** kiest u of **Enter** naar beneden, naar rechts, naar boven of naar links gaat, of helemaal niet verplaatst.

Als u met **Tab** in meerdere cellen langs een rij typt en daarna op **Enter** drukt, gaat de selectie naar de volgende rij, in de kolom waar u begon. Hebt u eerst een blok cellen geselecteerd, dan verplaatsen **Enter** en **Tab** de selectie door het blok.

## Automatische correctie tijdens het typen

Wanneer u een spatie of een leesteken zoals een punt na een woord typt, kan JeffCrunch het woord verbeteren terwijl u het afrondt. Het corrigeert veelvoorkomende typefouten (zoals "teh" in plaats van "the"), twee hoofdletters aan het begin van een woord, een kleine letter die een zin of een dag van de week begint, en een recht aanhalingsteken (dat wordt een krullend teken). Ook zet het twee koppeltekens tussen woorden om in een lang streepje, en 1/2, 1/4 en 3/4 in ½, ¼ en ¾ wanneer de invoer woorden bevat. Een getal zoals 1/2 dat u alleen typt, blijft zoals altijd een datum of een breuk.

Elke correctie is een stap op zich. Druk direct na een correctie op **Ctrl+Z** om alleen die correctie ongedaan te maken en wat u typte te behouden. In een formule wordt niets veranderd: een invoer die begint met een is-gelijk-teken, of met een plus- of minteken gevolgd door een formule, blijft zoals u hem typte. Een webadres (zoals https://example.org) of een e-mailadres dat de hele invoer vormt, wordt bij het afronden van de invoer een koppeling in de cel, wanneer **Webadressen direct omzetten in hyperlinks tijdens het typen** aan staat.

U schakelt deze opties in of uit via **Bestand > Opties > Controle**. Een verkeerd gespeld woord wordt tijdens het typen gemarkeerd: zie [Spelling](notes-links).

## Een deel van de tekst opmaken

Sommige woorden in een cel kunnen er anders uitzien dan de rest. Een woord kan bijvoorbeeld vet of rood zijn.

1. Druk op F2 om de cel te bewerken (of dubbelklik erop).
2. Selecteer de woorden die u wilt wijzigen, met Shift en de pijltoetsen of met de muis.
3. Gebruik **Vet**, **Cursief**, **Onderstrepen**, **Doorhalen**, **Lettertype**, **Tekengrootte** of **Tekstkleur** op het tabblad **Start**, of Ctrl+B, Ctrl+I en Ctrl+U.
4. Druk op Enter.

Alleen de woorden die u selecteerde, veranderen. De rest van de cel behoudt de eigen opmaak van de cel. Het invoervak toont de woorden tijdens het bewerken in één opmaak, en de cel toont elk deel in zijn eigen opmaak wanneer u op Enter drukt. Als u later woorden typt of verwijdert, verschuift de opmaak mee met de woorden. Een lettertype of grootte kiezen zonder woorden te selecteren, verandert zoals altijd de hele cel.

Cellen die opmaak binnen de tekst hebben uit een ander programma, tonen die opmaak, behouden die en slaan die weer op. Een cel die u wijzigt, behoudt zijn opmaak. Opmaak binnen een tekst geldt voor tekst, niet voor getallen of formules. Een kopie die is opgeslagen als een oudere .xls-bestand of als .ods, behoudt de woorden, en bij het opslaan wordt gemeld dat de opmaak binnen die woorden niet wordt behouden.

## Wat u typt, wordt

JeffCrunch leest wat u typt op de manier die is ingesteld op uw computer. Het decimaalteken, het duizendtalteken, het valutateken en de volgorde van dag en maand in een datum komen uit die instellingen. Een getal dat u typt in een cel zonder eigen opmaak, krijgt de opmaak die het laat zien zoals u het typte.

| U typt | De cel bevat | En toont |
| --- | --- | --- |
| 1234,5 | het getal 1234,5 | 1234,5 |
| 5% | het getal 0,05 | 5% |
| € 12,50 | het getal 12,5 | € 12,50 |
| 1 1/2 | het getal 1,5 | 1 1/2 |
| 4-3-2025 | de datum, als aantal dagen | 4-3-2025 |
| 22:30 | de tijd, als deel van een dag | 22:30 |
| WAAR | de logische waarde waar | WAAR |
| #N/B | de fout #N/B | #N/B |
| =B2*2 | een formule | het resultaat van de formule |
| al het andere | tekst | de tekst |

Een cel die al een eigen opmaak heeft, zoals een percentage of een datum, behoudt die. Een cel waarvan de opmaak **Tekst** is, houdt alles wat u typt als tekst, behalve een formule.

Wilt u iets dat op een getal lijkt als tekst bewaren, zoals een code **007**, typ dan eerst een apostrof: **'007**. De apostrof hoort niet bij de tekst. Wanneer u de cel bewerkt, ziet u hem weer.

## Formules

Een formule begint met een is-gelijk-teken. **=B2*2** verdubbelt bijvoorbeeld het getal in B2. U mag de namen van functies met hoofdletters of kleine letters typen; JeffCrunch schrijft ze met hoofdletters.

De namen van functies zijn altijd de Engelse, in elke taal: typ **SUM**, niet de naam die uw taal eraan geeft. De tekens ertussen volgen de instellingen van uw computer. Waar het decimaalteken een komma is, zoals in het Duits en Frans, scheidt u de argumenten van een functie met een puntkomma en schrijft u een komma in een getal: **=SOM(A1;A2)*0,5**. De formulebalk en **F2** tonen formules op dezelfde manier. Waar het decimaalteken een punt is, scheidt u argumenten met een komma: **=SOM(A1,A2)*0.5**. Het bestand bewaart altijd de vorm die elk programma leest, met komma's tussen argumenten en punten in getallen, zodat het overal hetzelfde opent.

Heeft een formule een fout, zoals een haakje dat nooit wordt gesloten, dan vertelt JeffCrunch wat er mis is. De invoer blijft zoals u hem typte, met de cursor bij de fout, zodat u die kunt herstellen. Er verandert niets totdat de formule klopt.

Een cel bevat maximaal 32.767 tekens, en een formule maximaal 8.192. JeffCrunch laat u weten wanneer een invoer langer is.

## Cellen wissen

Selecteer de cellen en druk op **Delete**. Dit wist wat de cellen bevatten en behoudt hun opmaak, en het is één stap die u ongedaan kunt maken. JeffCrunch wist maximaal 100.000 cellen tegelijk.

Op het tabblad **Start** heeft de knop **Wissen** een menu. **Alles wissen** leegt de cellen, hun opmaak, hun notities, hun opmerkingen (de discussies met antwoorden) en hun koppelingen. Een discussie met meer dan 500 opmerkingen blijft behouden, en JeffCrunch meldt dat, omdat Ongedaan maken de opmerkingen die het niet heeft gelezen, niet kan herstellen. **Opmaak wissen** behoudt wat de cellen bevatten en zet de gewone opmaak terug. **Inhoud wissen** is hetzelfde als **Delete**. **Opmerkingen en notities wissen** haalt alleen de notities en opmerkingen van de geselecteerde cellen weg. **Hyperlinks wissen** haalt alleen de koppelingen weg, en de cellen behouden hun tekst. Een keuze is grijs wanneer de geselecteerde cellen niets hebben dat ze kan wissen.

## Ongedaan maken en Opnieuw

**Ctrl+Z** maakt de laatste wijziging ongedaan, en **Ctrl+Y** doet ze opnieuw. Ze staan ook op de snelle werkbalk bovenaan. Elke cel waarin u typt, is één stap, en elk wissen ook. Ongedaan maken zet de cel terug zoals hij was: de waarde, de formule, de opmaak en de formules die ervan afhangen. JeffCrunch onthoudt maximaal 1.000 stappen, minder wanneer ze heel veel cellen veranderden, zodat het nooit veel geheugen gebruikt.

## Terwijl JeffCrunch de formules berekent

Wanneer u een cel wijzigt, berekent JeffCrunch op de achtergrond de formules die ervan afhangen. De cel die u wijzigde, toont meteen zijn nieuwe inhoud, en de andere cellen worden even later bijgewerkt; de statusbalk toont **Berekenen** totdat ze klaar zijn. U kunt intussen doorgaan met typen.

## Wat niet kan worden gewijzigd

- Een cel die deel uitmaakt van een **matrixformule** of een gegevenstabel, kan niet alleen worden gewijzigd.
- Op een **beveiligd blad** kan een vergrendelde cel niet worden gewijzigd. Elke cel is vergrendeld, tenzij de werkmap aangeeft dat hij dat niet is. JeffCrunch meldt het wanneer u het probeert. Om de cel te wijzigen, kiest u op het tabblad Controleren de optie Beveiliging blad opheffen.
- De eerste rij van een **tabel** bevat de namen van de kolommen. Typ een nieuwe naam in een van die cellen om de kolom te hernoemen: de formules die de oude naam gebruiken, volgen mee, en de tabel in het bestand blijft geldig. Een naam die een andere kolom al heeft, krijgt een nummer erachter, een naam kan niet leeg zijn, en **Delete** wist die rij niet.
- Een werkmap die niet helemaal kon worden gelezen, kan worden bekeken, maar niet worden gewijzigd of opgeslagen.
- Een functie die JeffCrunch nog niet kent, laat een nieuwe formule zonder waarde in de cel achter; de werkmap vraagt het programma dat hem opent om de formule te berekenen.
- Opmaak, sorteren en zoeken en vervangen komen in een latere versie. Invoegen en verwijderen, knippen en plakken, doorvoeren en de afmetingen van rijen en kolommen staan in [Rijen, kolommen en cellen invoegen en verwijderen](insert-delete), [Knippen, kopiëren en plakken](clipboard), [Cellen doorvoeren en optellen](fill-sum) en [De breedte van kolommen en de hoogte van rijen wijzigen](sizes).

## Een symbool invoegen

Klik op **Invoegen > Symbolen > Symbool** om het vak Symbool te openen. Kies een teken en klik op **Invoegen**, of dubbelklik erop. Het komt in op de cursor van de invoer die u typt. Typt u niet, dan opent de eigen invoer van de cel met het symbool aan het eind, en **Enter** zet het in de cel. Voorbeeld: staat in een cel het woord *Totaal* en kiest u het eurosteken, dan wordt de invoer *Totaal€*. De symbolen die u het laatst gebruikte, staan eerst in de lijst.

**Het teken uit een lettertype halen.** Kies een lettertype in het vak **Lettertype** en kies dan het teken. Het teken komt in een eigen stuk tekst dat in dat lettertype wordt gezet, en de rest van de tekst van de cel behoudt het lettertype van de cel. Voorbeeld: in een cel met *Totaal* maakt een vinkje uit een symboollettertype *Totaal✓*, waarbij alleen het vinkje in dat lettertype staat. Met **(gewone tekst)** gekozen, tekent het lettertype van de cel het symbool, zodat een symbool dat het lettertype niet heeft als vakje verschijnt. In een formule gaat alleen het teken erin.

**Speciale tekens.** Het tabblad **Speciale tekens** toont de vaste spatie en het vaste koppelteken, het optionele afbreekstreepje, het en-streepje en em-streepje, de en- en em-spatie, de aanhalingstekens en de optionele onderbreking zonder breedte, elk met zijn naam. Dubbelklik op een ervan om het in te voegen.

**Een symbool een sneltoets geven.** Kies een symbool of een speciaal teken, klik op **Sneltoets**, druk de toetsen in die u wilt, zoals **Ctrl+Alt+Shift+J**, en klik op **OK**. De toetsen zetten dat symbool dan in, op dezelfde manier als het vak dat doet. Laat de toetsen leeg en klik op **OK** om ze te verwijderen. JeffCrunch meldt het wanneer een andere opdracht die toetsen al heeft. Het symbool wordt een opdracht, dus het staat ook onder **Bestand > Opties > Aanpassen > Toetsenbord**, waar u de toetsen kunt wijzigen (zie [Sneltoetsen](keyboard)).

## Nieuwe werkmappen

Kies in **Bestand > Opties > Nieuwe werkmappen** het lettertype, de tekengrootte en het aantal bladen waarmee elke nieuwe werkmap begint. Werkmappen die u al hebt, worden niet gewijzigd.
