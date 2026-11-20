# Cellen invullen

<!-- keywords: symbool, symboollettertype, speciale tekens, sneltoets voor een symbool, automatische correctie, slimme aanhalingstekens, streepje, breuk, koppeling, adres, vetgedrukt woord, deel van een cel, opmaak in tekst, deel van tekst opmaken, kleur van een woord, invoeren, typen, invoer, bewerken, wijzigen, cel, invoerbalk, formulebalk, F2, dubbelklikken, Tab, Esc, annuleren, ongedaan maken, opnieuw, verwijderen, wissen, procent, valuta, geld, datum, tijd, getal, tekst, apostrof, formule, is gelijk aan, vervangen, limiet, tekens -->

Om iets in een cel te zetten, selecteert u de cel en begint u met typen. Wat u typt, vervangt wat de cel bevatte. Druk op **Enter** om het te bewaren, of op **Esc** om het op te geven en de cel te laten zoals die was.

## Drie manieren om te beginnen

- **Typ over de cel heen.** Selecteer een cel en typ. Het eerste teken dat u typt, begint de invoer, en de cel toont wat u typt terwijl u doorgaat.
- **Bewerk wat in de cel staat.** Druk op **F2** of dubbelklik op de cel. De cel opent met zijn inhoud, en de cursor staat aan het einde. Gebruik de pijltoetsen om in de tekst te bewegen.
- **Gebruik de formulebalk.** Klik in de formulebalk, naast het vak Naam, en typ daar. De cel toont ook wat u typt.

Wanneer u over een cel heen typt, beëindigt een pijltoets de invoer en verplaatst de selectie. Wanneer u bewerkt wat in de cel staat, bewegen de pijltoetsen door de tekst. Druk op **F2** om van de ene naar de andere modus te gaan.

## De invoer afronden

| Toetsen | Wat ze doen |
| --- | --- |
| Enter | Bewaart de invoer en gaat omlaag (of waarheen **Bestand > Opties > Bewerken** aangeeft) |
| Shift+Enter | Bewaart de invoer en gaat de andere kant op |
| Ctrl+Enter | Zet de invoer in alle geselecteerde cellen en blijft waar u bent |
| Tab | Bewaart de invoer en gaat naar rechts |
| Shift+Tab | Bewaart de invoer en gaat naar links |
| Pijltoetsen | Bewaart de invoer en gaat één cel verder (alleen wanneer u over de cel heen typte) |
| Esc | Geeft de invoer op |
| Alt+Enter | Begint een nieuwe regel binnen de cel |
| Ctrl+; | Zet de datum van vandaag erin (in de invoer, wanneer u er een typt) |
| Ctrl+Shift+; | Zet de huidige tijd erin |
| Alt+omlaag | Toont de woorden die al in de kolom staan, en u kiest er een |

**Ctrl+Enter** met meerdere geselecteerde cellen zet wat u typte in alle cellen. Een formule wordt geschreven voor de cel waarin u typte, en in elk van de andere cellen leest ze als een kopie ervan zou lezen: als u **=A1** typt terwijl het blok E6:F7 is geselecteerd, wordt dat **=B2** in F7. Het is één stap van Ongedaan maken.

**Alt+omlaag** op een cel toont, in volgorde en elk één keer, de woorden in de cellen erboven en eronder die niet door een lege cel zijn gescheiden. Als u er een kiest, wordt het getypt. Een cel met een eigen lijst toont in plaats daarvan die lijst.

In **Bestand > Opties > Bewerken** kiest u of **Enter** naar beneden, rechts, omhoog of links gaat, of helemaal niet verplaatst.

Als u met **Tab** in meerdere cellen langs een rij typt en daarna op **Enter** drukt, gaat de selectie naar de volgende rij, in de kolom waarin u begon. Als u eerst een blok cellen selecteert, verplaatsen **Enter** en **Tab** zich door het blok.

## Automatische correctie tijdens het typen

Wanneer u na een woord een spatie of een leesteken zoals een punt typt, kan JeffCrunch het woord verbeteren zodra u het afrondt. Het corrigeert veelvoorkomende typefouten (zoals "teh" in plaats van "the"), twee hoofdletters aan het begin van een woord, een kleine letter die een zin of een dag van de week begint, en een rechte aanhalingsteken (dat wordt een krul-aanhalingsteken). Ook zet het twee koppeltekens tussen woorden om in een lang streepje, en 1/2, 1/4 en 3/4 in ½, ¼ en ¾ wanneer de invoer woorden bevat. Een getal zoals 1/2 dat alleen wordt getypt, blijft zoals altijd een datum of een breuk.

Elke correctie is een stap op zich. Druk direct na een correctie op **Ctrl+Z** om alleen die correctie ongedaan te maken en wat u typte te behouden. In een formule wordt niets gewijzigd: een invoer die begint met een is-gelijk-teken, of met een plus- of minteken gevolgd door een formule, blijft zoals u hem typte. Een webadres (zoals https://example.org) of een e-mailadres dat de hele invoer vormt, wordt een koppeling in de cel wanneer u de invoer afrondt, als **Webadressen direct omzetten in hyperlinks tijdens het typen** aan staat.

U zet deze opties aan of uit in **Bestand > Opties > Controle**. Een verkeerd gespeld woord wordt tijdens het typen gemarkeerd: zie [Spelling](notes-links).

## Deel van de tekst opmaken

Sommige woorden in een cel kunnen er anders uitzien dan de rest. Zo kan één woord vetgedrukt zijn of rood.

1. Druk op F2 om de cel te bewerken (of dubbelklik erop).
2. Selecteer de woorden die u wilt wijzigen, met Shift en de pijltoetsen of met de muis.
3. Gebruik **Vet**, **Cursief**, **Onderstrepen**, **Doorhalen**, **Lettertype**, **Lettergrootte** of **Lettertypekleur** op het tabblad **Start**, of Ctrl+B, Ctrl+I en Ctrl+U.
4. Druk op Enter.

Alleen de woorden die u selecteerde, veranderen. De rest van de cel behoudt de eigen opmaak van de cel. Het invoervak toont de woorden in één opmaak terwijl u bewerkt, en de cel toont elk deel in zijn eigen opmaak wanneer u op Enter drukt. Als u later woorden typt of verwijdert, verschuift de opmaak mee met de woorden. Een lettertype of grootte kiezen zonder dat woorden zijn geselecteerd, verandert de hele cel, zoals altijd.

## De minibalk

Een kleine balk met de opmaak die u het meest gebruikt, zweeft waar u werkt, zodat u niet naar het lint hoeft. Ze neemt het toetsenbord nooit over van de cel: wat u typte, blijft waar het was.

- **Geselecteerde tekst in een cel:** selecteer woorden met de muis terwijl u een cel bewerkt, en de balk zweeft erboven. Ze heeft de vakken **lettertype** en **grootte**, **Vet**, **Cursief**, **Onderstrepen**, **Lettergrootte vergroten**, **Lettergrootte verkleinen** en **Lettertypekleur**. Ze wijzigen alleen de woorden die u selecteerde, net als op het lint.
- **Geselecteerde cellen:** klik met de rechtermuisknop op de cellen, en de balk staat bovenaan het snelmenu. Ze heeft ook **Vulkleur**, **Randen**, **Valuta**, **Procentnotatie**, **Duizendtalnotatie**, **Decimalen toevoegen**, **Decimalen verwijderen**, **Cellen samenvoegen en centreren** en **Opmaak kopiëren**. Dit zijn de eigen opdrachten van het lint, en elk is één stap die u ongedaan kunt maken.

De balk boven geselecteerde tekst vervaagt wanneer u de aanwijzer ervan wegbeweegt, en verdwijnt wanneer de aanwijzer ver weg is. Ze verdwijnt ook wanneer u typt, op Esc drukt of scrolt. Om de balk uit te schakelen, haalt u het vinkje weg bij **Minibalk weergeven bij selectie** op de pagina **Algemeen** van **Bestand > Opties**; de andere JeffOffice-apps volgen die keuze ook. De knop Snelle analyse die onder een geselecteerd bereik verschijnt, zit niet in deze versie.

Cellen die opmaak binnen de tekst hebben uit een ander programma, tonen die, behouden die en slaan die weer op. Een cel die u wijzigt, behoudt zijn opmaak. Opmaak binnen een tekst geldt voor tekst, niet voor getallen of formules. Een kopie die is opgeslagen als een oudere .xls-bestand of als .ods, behoudt de woorden, en bij het opslaan krijgt u te zien dat de opmaak binnen die woorden niet wordt behouden.

## Wat u typt, wordt

JeffCrunch leest wat u typt op de manier die uw computerinstellingen aangeven. Het decimaalteken, het duizendtalteken, het valutateken en de volgorde van dag en maand in een datum komen allemaal uit die instellingen. Een getal dat u typt in een cel zonder eigen notatie, krijgt de notatie die het toont zoals u het typte.

| U typt | De cel bevat | En toont |
| --- | --- | --- |
| 1234.5 | het getal 1234,5 | 1234,5 |
| 5% | het getal 0,05 | 5% |
| € 12,50 | het getal 12,5 | € 12,50 |
| 1 1/2 | het getal 1,5 | 1 1/2 |
| 4-3-2025 | de datum, als aantal dagen | 4-3-2025 |
| 22:30 | de tijd, als deel van een dag | 22:30 |
| WAAR | de logische waarde waar | WAAR |
| #N/B | de fout #N/B | #N/B |
| =B2*2 | een formule | het resultaat van de formule |
| al het andere | tekst | de tekst |

Een cel die al een eigen notatie heeft, zoals een procent of een datum, behoudt die. Een cel waarvan de notatie **Tekst** is, behoudt alles wat u typt als tekst, behalve een formule.

Wilt u iets dat op een getal lijkt als tekst bewaren, zoals een code **007**, typ dan eerst een apostrof: **'007**. De apostrof hoort niet bij de tekst. Wanneer u de cel bewerkt, ziet u hem weer.

## Formules

Een formule begint met een is-gelijk-teken. **=B2*2** verdubbelt bijvoorbeeld het getal in B2. U kunt namen van functies met hoofdletters of kleine letters typen; JeffCrunch schrijft ze met hoofdletters.

De namen van functies zijn altijd de Engelse, in elke taal: typ **SUM**, niet de naam die uw taal eraan geeft. De tekens ertussen volgen de instellingen van uw computer. Waar het decimaalteken een komma is, zoals in het Duits en Frans, scheidt u de argumenten van een functie met een puntkomma en schrijft u een komma in een getal: **=SOM(A1;A2)*0,5**. De formulebalk en **F2** tonen formules op dezelfde manier. Waar het decimaalteken een punt is, scheidt u argumenten met een komma: **=SOM(A1,A2)*0.5**. Het bestand bewaart altijd de vorm die elk programma leest, met komma's tussen argumenten en punten in getallen, zodat het overal hetzelfde opent.

Als een formule een fout bevat, zoals een haakje dat nooit wordt gesloten, vertelt JeffCrunch wat er mis is. De invoer blijft zoals u hem typte, met de cursor in de buurt van de fout, zodat u hem kunt verbeteren. Er wordt niets gewijzigd totdat de formule klopt.

Een cel bevat maximaal 32.767 tekens, en een formule maximaal 8.192. JeffCrunch laat u weten wanneer een invoer langer is.

## Cellen wissen

Selecteer de cellen en druk op **Delete**. Dit wist wat de cellen bevatten en behoudt hun opmaak, en het is één stap die u ongedaan kunt maken. JeffCrunch wist maximaal 100.000 cellen tegelijk.

Op het tabblad **Start** heeft de knop **Wissen** een menu. **Alles wissen** maakt de cellen leeg, en wist hun opmaak, notities, opmerkingen (de discussies met antwoorden) en koppelingen. Een discussie met meer dan 500 opmerkingen blijft behouden, en JeffCrunch zegt dat erbij, omdat Ongedaan maken de opmerkingen die het nooit heeft gelezen niet kan terughalen. **Opmaak wissen** behoudt wat de cellen bevatten en zet de gewone opmaak terug. **Inhoud wissen** is hetzelfde als **Delete**. **Opmerkingen en notities wissen** haalt alleen de notities en opmerkingen van de geselecteerde cellen weg. **Hyperlinks wissen** haalt alleen de koppelingen weg, en de cellen behouden hun tekst. Een keuze is grijs wanneer de geselecteerde cellen niets hebben om te wissen.

## Ongedaan maken en opnieuw

**Ctrl+Z** maakt de laatste wijziging ongedaan, en **Ctrl+Y** doet die opnieuw. Ze staan ook op de snelle werkbalk bovenaan. Elke cel waarin u typt, is één stap, en elk wissen ook. Ongedaan maken zet de cel terug zoals die was: de waarde, de formule, de opmaak en de formules die ervan afhangen. JeffCrunch onthoudt tot 1.000 stappen, minder wanneer ze heel veel cellen hebben gewijzigd, zodat het nooit veel geheugen gebruikt.

## Terwijl JeffCrunch de formules berekent

Wanneer u een cel wijzigt, berekent JeffCrunch op de achtergrond de formules die ervan afhangen. De cel die u wijzigde, toont meteen de nieuwe inhoud, en de andere cellen worden even later bijgewerkt; de statusbalk toont **Berekenen** totdat ze klaar zijn. U kunt intussen gewoon doorgaan met typen.

## Wat niet kan worden gewijzigd

- Een cel die deel uitmaakt van een **matrixformule** of een gegevenstabel, kan niet alleen worden gewijzigd. Om een matrixformule te maken, selecteert u de cellen die hij moet vullen, typt u de formule en drukt u op **Ctrl+Shift+Enter**. Eén formule vult dan alle cellen, en de formulebalk toont hem tussen accolades, zoals **{=A1:A3*2}**. Om hem te wijzigen, selecteert u een willekeurige cel van de matrix, drukt u op **F2**, wijzigt u de formule en drukt u opnieuw op **Ctrl+Shift+Enter**. Om hem te verwijderen, selecteert u al zijn cellen en drukt u op **Delete**. Beide werken op de hele matrix, in één stap van Ongedaan maken. F2 op een willekeurige cel van een matrix opent de formule van de matrix, en een beveiligd blad of een bereik met wachtwoord weigert of vraagt zoals bij elke andere cel. Het kopiëren van een matrix, of een deel ervan, en het doorvoeren vanuit een van zijn cellen, geven de waarden die hij toont, niet een formule.
- Op een **beveiligd blad** kan een vergrendelde cel niet worden gewijzigd. Elke cel is vergrendeld, tenzij de werkmap zegt dat hij dat niet is. JeffCrunch laat u weten wanneer u het probeert. Om de cel te wijzigen, kiest u op het tabblad Controleren de optie Beveiliging blad opheffen.
- De eerste rij van een **tabel** bevat de namen van de kolommen. Typ een nieuwe naam in een van die cellen om de kolom te hernoemen: de formules die de oude naam gebruiken, volgen, en de tabel in het bestand blijft geldig. Een naam die een andere kolom al heeft, krijgt een nummer erachter, een naam kan niet leeg zijn, en **Delete** wist die rij niet.
- Een werkmap die niet helemaal kon worden gelezen, kan worden bekeken maar niet gewijzigd of opgeslagen.
- Een functie die JeffCrunch nog niet heeft, laat een nieuwe formule zonder waarde in de cel achter; de werkmap vraagt het programma dat hem opent om de formule te berekenen.
- Opmaak, sorteren en zoeken en vervangen komen in een latere versie. Invoegen en verwijderen, knippen en plakken, doorvoeren, en de afmetingen van rijen en kolommen staan in [Rijen, kolommen en cellen invoegen en verwijderen](insert-delete), [Knippen, kopiëren en plakken](clipboard), [Cellen doorvoeren en optellen](fill-sum) en [De breedte van kolommen en de hoogte van rijen wijzigen](sizes).

## Een symbool invoegen

Klik op **Invoegen > Symbolen > Symbool** om het vak Symbool te openen. Kies een teken en klik op **Invoegen**, of dubbelklik erop. Het wordt ingevoegd op de cursor van de invoer die u typt. Als u niets typt, opent de eigen invoer van de cel met het symbool aan het einde, en **Enter** zet het in de cel. Voorbeeld: met een cel die het woord *Totaal* bevat, geeft de keuze van het eurateken de invoer *Totaal€*. De symbolen die u het laatst gebruikte, staan het eerst in de lijst.

**Het teken uit een lettertype halen.** Kies een lettertype in het vak **Lettertype** en kies daarna het teken. Het teken wordt ingevoegd in een eigen stuk dat in dat lettertype wordt gezet, en de rest van de tekst van de cel behoudt het lettertype van de cel. Voorbeeld: in een cel met *Totaal* maakt een vinkje uit een symboollettertype *Totaal✓*, waarbij alleen het vinkje in dat lettertype staat. Met **(normale tekst)** gekozen, tekent het lettertype van de cel het symbool, zodat een symbool dat het lettertype mist, als vakje verschijnt. In een formule gaat alleen het teken erin.

**Speciale tekens.** Het tabblad **Speciale tekens** toont de vaste spatie en het vaste koppelteken, het optionele afbreekstreepje, het en- en em-streepje, de en- en em-spatie, de aanhalingstekens en het breedteloze optionele breekpunt, elk met zijn naam. Dubbelklik op een ervan om het in te voegen.

**Een symbool een sneltoets geven.** Kies een symbool of een speciaal teken, klik op **Sneltoets**, druk de toetsen die u wilt, zoals **Ctrl+Alt+Shift+J**, en klik op **OK**. De toetsen zetten dan dat symbool erin, op dezelfde manier als het vak dat doet. Laat de toetsen leeg en klik op **OK** om ze weg te halen. JeffCrunch laat u weten wanneer een andere opdracht die toetsen al heeft. Het symbool wordt een opdracht, dus het staat ook in **Bestand > Opties > Aanpassen > Sneltoetsen**, waar u de toetsen kunt wijzigen (zie [Sneltoetsen](keyboard)).

## Nieuwe werkmappen

In **Bestand > Opties > Nieuwe werkmappen** kiest u het lettertype, de lettergrootte en het aantal bladen waarmee elke nieuwe werkmap begint. Werkmappen die u al had, worden niet gewijzigd.
