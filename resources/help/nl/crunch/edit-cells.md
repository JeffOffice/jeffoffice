# Gegevens invoeren in cellen
<!-- keywords: symbool, symboollettertype, speciale tekens, sneltoets voor symbool, automatische correctie, slimme aanhalingstekens, streepje, breuk, koppeling, adres, vetgedrukt woord, deel van een cel, opgemaakte tekst, deel van tekst opmaken, woord kleuren, invoeren, typen, invoer, bewerken, wijzigen, cel, invoerbalk, formulebalk, F2, dubbelklikken, Tab, Esc, annuleren, ongedaan maken, opnieuw, verwijderen, wissen, percentage, valuta, geld, datum, tijd, getal, tekst, apostrof, formule, is gelijk aan, vervangen, limiet, tekens -->

Als u iets in een cel wilt zetten, selecteert u de cel en begint u met typen. Wat u typt, vervangt wat de cel bevatte. Druk op **Enter** om het te bewaren, of op **Esc** om het op te geven en de cel te laten zoals die was.

## Drie manieren om te beginnen

- **Typ over de cel heen.** Selecteer een cel en typ. Het eerste teken dat u typt, begint de invoer, en de cel toont wat u typt terwijl u doorgaat.
- **Bewerk wat in de cel staat.** Druk op **F2** of dubbelklik op de cel. De cel opent met zijn inhoud en de cursor staat aan het einde. Gebruik de pijltjestoetsen om in de tekst te bewegen.
- **Gebruik de formulebalk.** Klik in de formulebalk, naast het naamvak, en typ daar. De cel toont ook wat u typt.

Wanneer u over een cel heen typt, beëindigt een pijltjestoets de invoer en verplaatst de selectie. Wanneer u bewerkt wat in de cel staat, verplaatsen de pijltjestoetsen zich in de tekst. Druk op **F2** om van het ene naar het andere te schakelen.

## De invoer afronden

| Toetsen | Wat ze doen |
| --- | --- |
| Enter | De invoer bewaren en omlaag gaan (of waarheen **Bestand > Opties > Bewerken** aangeeft) |
| Shift+Enter | De invoer bewaren en de andere kant op gaan |
| Ctrl+Enter | De invoer in alle geselecteerde cellen zetten en op uw plaats blijven |
| Tab | De invoer bewaren en naar rechts gaan |
| Shift+Tab | De invoer bewaren en naar links gaan |
| Pijltjestoetsen | De invoer bewaren en één cel verplaatsen (alleen wanneer u over de cel heen typte) |
| Esc | De invoer opgeven |
| Alt+Enter | Een nieuwe regel in de cel beginnen |
| Ctrl+; | De datum van vandaag invoegen (in de invoer, wanneer u er een typt) |
| Ctrl+Shift+; | De huidige tijd invoegen |
| Alt+Omlaag | De woorden tonen die al in de kolom staan, en er een kiezen |

**Ctrl+Enter** met meerdere geselecteerde cellen zet wat u typte in alle cellen. Een formule wordt geschreven voor de cel waarin u typte, en in elk van de andere cellen leest hij zoals een kopie ervan zou lezen: als u **=A1** typt terwijl het blok E6:F7 is geselecteerd, staat er in F7 **=B2**. Het is één stap van Ongedaan maken.

**Alt+Omlaag** op een cel toont, in volgorde en elk één keer, de woorden in de cellen boven en onder die cel die niet door een lege cel zijn gescheiden. Als u er een kiest, wordt die getypt. Een cel met een eigen lijst toont die lijst.

In **Bestand > Opties > Bewerken** kiest u of **Enter** omlaag, naar rechts, omhoog of naar links gaat, of helemaal niet verplaatst.

Als u met **Tab** in meerdere cellen op een rij typt en daarna op **Enter** drukt, gaat de selectie naar de volgende rij, in de kolom waar u begon. Als u eerst een blok cellen selecteert, verplaatsen **Enter** en **Tab** zich door het blok.

## Automatische correctie tijdens het typen

Wanneer u na een woord een spatie of een leesteken zoals een punt typt, kan JeffCrunch het woord corrigeren zodra u het afrondt. Het corrigeert veelvoorkomende typefouten (zoals "teh" voor "the"), twee hoofdletters aan het begin van een woord, een kleine letter die een zin of een dag van de week begint, en een rechte aanhalingsteken (dat wordt een krul). Ook zet het twee koppeltekens tussen woorden om in een lange streep, en 1/2, 1/4 en 3/4 in ½, ¼ en ¾, wanneer de invoer woorden bevat. Een getal zoals 1/2 dat alleen wordt getypt, blijft zoals altijd een datum of een breuk.

Elke correctie is een stap op zich. Druk direct na een correctie op **Ctrl+Z** om alleen die correctie ongedaan te maken en wat u typte te behouden. In een formule wordt niets veranderd: een invoer die begint met een is-gelijk-teken, of met een plus- of minteken gevolgd door een formule, blijft zoals u hem typte. Een webadres (zoals https://example.org) of een e-mailadres dat de hele invoer vormt, wordt bij het afronden een koppeling in de cel, als **Webadressen direct omzetten in hyperlinks tijdens het typen** aan staat.

U zet deze opties aan of uit in **Bestand > Opties > Controle**. Een verkeerd gespeld woord wordt al tijdens het typen gemarkeerd: zie [Spelling](notes-links).

## Een deel van de tekst opmaken

Sommige woorden in een cel kunnen er anders uitzien dan de rest. Een woord kan bijvoorbeeld vet of rood zijn.

1. Druk op F2 om de cel te bewerken (of dubbelklik erop).
2. Selecteer de woorden die u wilt wijzigen, met Shift en de pijltjestoetsen of met de muis.
3. Gebruik **Vet**, **Cursief**, **Onderstrepen**, **Doorhalen**, **Lettertype**, **Lettergrootte** of **Lettertypekleur** op het tabblad **Start**, of Ctrl+B, Ctrl+I en Ctrl+U.
4. Druk op Enter.

Alleen de geselecteerde woorden veranderen. De rest van de cel behoudt de eigen opmaak van de cel. Het invoervak toont de woorden in één opmaak terwijl u bewerkt, en de cel toont elk deel in zijn eigen opmaak wanneer u op Enter drukt. Als u later woorden typt of verwijdert, verschuift de opmaak mee met de woorden. Een lettertype of grootte kiezen zonder woorden te selecteren, wijzigt de hele cel, zoals altijd.

## De minibalk

Een kleine balk met de opmaak die u het meest gebruikt, verschijnt waar u werkt, zodat u niet naar het lint hoeft. Hij neemt nooit het toetsenbord van de cel over: wat u typte, blijft waar het was.

- **Geselecteerde tekst in een cel:** selecteer woorden met de muis terwijl u een cel bewerkt, en de balk verschijnt boven die woorden. Hij heeft de vakken **lettertype** en **grootte**, en de knoppen **Vet**, **Cursief**, **Onderstrepen**, **Lettergrootte vergroten**, **Lettergrootte verkleinen** en **Lettertypekleur**. Ze wijzigen alleen de woorden die u selecteerde, net als op het lint.
- **Geselecteerde cellen:** klik met de rechtermuisknop op de cellen, dan staat de balk bovenaan het snelmenu. Ook heeft hij **Vulkleur**, **Randen**, **Valuta**, **Procentopmaak**, **Komma-opmaak**, **Meer decimalen**, **Minder decimalen**, **Cellen samenvoegen en centreren** en **Opmaak kopiëren**. Dat zijn de eigen opdrachten van het lint, en elk ervan is één stap die u ongedaan kunt maken.

De balk boven geselecteerde tekst vervaagt wanneer u de aanwijzer ervandaan beweegt, en verdwijnt wanneer de aanwijzer ver weg is. Hij verdwijnt ook wanneer u typt, op Esc drukt of scrolt. Om de balk uit te schakelen, haalt u het vinkje weg bij **Minibalk weergeven bij selectie** op de pagina **Algemeen** van **Bestand > Opties**; de andere JeffOffice-apps volgen die keuze ook. De knop Snelle analyse die onder een geselecteerd bereik verschijnt, is in deze versie niet aanwezig.

Cellen die opmaak binnen de tekst hebben uit een ander programma, tonen die opmaak, behouden haar en slaan haar weer op. Een cel die u wijzigt, behoudt zijn opmaak. Opmaak binnen een tekst geldt voor tekst, niet voor getallen of formules. Een kopie die is opgeslagen als een ouder .xls-bestand of als .ods, behoudt de woorden, en bij het opslaan wordt gemeld dat de opmaak binnen de woorden niet wordt bewaard.

## Wat u typt, wordt

JeffCrunch leest wat u typt op de manier die de instellingen van uw computer aangeven. Het decimaalteken, het duizendtalteken, het valutateken en de volgorde van dag en maand in een datum komen allemaal uit die instellingen. Een getal dat u typt in een cel zonder eigen opmaak, krijgt de opmaak die het toont zoals u het typte.

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

Een cel die al een eigen opmaak heeft, zoals een percentage of een datum, behoudt die. Een cel waarvan de opmaak **Tekst** is, bewaart alles wat u typt als tekst, behalve een formule.

Om iets dat op een getal lijkt als tekst te bewaren, zoals een code **007**, typt u eerst een apostrof: **'007**. De apostrof hoort niet bij de tekst. Wanneer u de cel bewerkt, ziet u hem weer.

## Formules

Een formule begint met een is-gelijk-teken. **=B2*2** verdubbelt bijvoorbeeld het getal in B2. U typt de namen van functies in hoofdletters of kleine letters; JeffCrunch schrijft ze in hoofdletters.

De namen van functies zijn altijd de Engelse, in elke taal: typ **SUM**, niet de naam die uw taal eraan geeft. De leestekens ertussen volgen de instellingen van uw computer. Waar het decimaalteken een komma is, zoals in het Duits en Frans, scheidt u de argumenten van een functie met een puntkomma en schrijft u een komma in een getal: **=SOM(A1;A2)*0,5**. De formulebalk en **F2** tonen formules op dezelfde manier. Waar het decimaalteken een punt is, scheidt u argumenten met een komma: **=SUM(A1,A2)*0.5**. Het bestand bewaart altijd de vorm die elk programma leest, met komma's tussen argumenten en punten in getallen, zodat het overal hetzelfde opent.

Als een formule een fout bevat, zoals een haakje dat nooit wordt gesloten, vertelt JeffCrunch wat er niet klopt. De invoer blijft zoals u hem typte, met de cursor bij de fout, zodat u hem kunt herstellen. Er verandert niets totdat de formule klopt.

Een cel bevat maximaal 32.767 tekens, en een formule maximaal 8.192. JeffCrunch laat u weten wanneer een invoer langer is.

## Cellen wissen

Selecteer de cellen en druk op **Delete**. Dat wist wat de cellen bevatten en behoudt hun opmaak, en het is één stap die u ongedaan kunt maken. JeffCrunch wist tegelijk maximaal 100.000 cellen.

Op het tabblad **Start** heeft de knop **Wissen** een menu. **Alles wissen** leegt de cellen, hun opmaak, hun notities, hun opmerkingen (de discussies met antwoorden) en hun koppelingen. Een discussie met meer dan 500 opmerkingen blijft behouden, en JeffCrunch meldt dat, omdat Ongedaan maken de opmerkingen die het nooit heeft gelezen niet kan herstellen. **Opmaak wissen** behoudt wat de cellen bevatten en zet de gewone opmaak terug. **Inhoud wissen** is hetzelfde als **Delete**. **Opmerkingen en notities wissen** haalt alleen de notities en opmerkingen van de geselecteerde cellen weg. **Hyperlinks wissen** haalt alleen de koppelingen weg, en de cellen behouden hun tekst. Een keuze is grijs wanneer de geselecteerde cellen niets hebben dat ze kan wissen.

## Ongedaan maken en Opnieuw

**Ctrl+Z** maakt de laatste wijziging ongedaan, en **Ctrl+Y** doet ze opnieuw. Ze staan ook op de snelle werkbalk bovenaan. Elke cel die u typt, is één stap, en elk wissen ook. Ongedaan maken zet de cel terug zoals die was: de waarde, de formule, de opmaak en de formules die ervan afhangen. JeffCrunch onthoudt maximaal 1.000 stappen, minder wanneer ze heel veel cellen wijzigden, zodat het nooit veel geheugen gebruikt.

**Meerdere stappen tegelijk terug.** Op de snelle werkbalk heeft **Ongedaan maken** en **Opnieuw** elk een pijltje ernaast. Klik op het pijltje om de stappen te zien, de nieuwste eerst (maximaal 100 worden getoond). Beweeg de aanwijzer omlaag over de lijst om aan te geven hoeveel stappen u bedoelt (de regel onder de lijst zegt bijvoorbeeld "3 acties ongedaan maken"), en klik om ze uit te voeren. Als u de derde stap kiest, worden de eerste drie ongedaan gemaakt. De stappen blijven in volgorde, dus Opnieuw brengt ze op dezelfde manier terug.

## Herhalen

**F4** herhaalt uw laatste opdracht op de cellen die nu zijn geselecteerd. Voorbeeld: maak **A1** vet, selecteer **C4:C9** en druk op **F4**, dan worden de cellen in C4:C9 vet, in één stap die u ongedaan kunt maken. Herhalen werkt voor opmaak, celstijlen, wissen, en het invoegen of verwijderen van rijen en kolommen. De infotekst van de knop, of de opdracht **Herhalen**, noemt wat er zou gebeuren ("Vet herhalen"). **Ctrl+Y** herhaalt ook, maar alleen wanneer er geen stap is om opnieuw te doen (zolang er een is, is **Ctrl+Y** Opnieuw). Terwijl u een formule typt, wijzigt **F4** nog steeds een verwijzing tussen relatief en vast, zoals voorheen. Opdrachten die een venster openen en het typen in een cel worden niet herhaald.

## Terwijl JeffCrunch de formules berekent

Wanneer u een cel wijzigt, berekent JeffCrunch op de achtergrond de formules die ervan afhangen. De cel die u wijzigde, toont meteen zijn nieuwe inhoud, en de andere cellen werken even later bij; de statusbalk toont **Berekenen** totdat ze klaar zijn. U kunt in de tussentijd doortypen.

## Wat niet kan worden gewijzigd

- Een cel die deel uitmaakt van een **matrixformule** of een gegevenstabel, kan niet alleen worden gewijzigd. Om een matrixformule te maken, selecteert u de cellen die hij moet vullen, typt u de formule en drukt u op **Ctrl+Shift+Enter**. Eén formule vult dan alle cellen, en de formulebalk toont hem tussen accolades, zoals **{=A1:A3*2}**. Om hem te wijzigen, selecteert u een willekeurige cel van de matrix, drukt u op **F2**, wijzigt u de formule en drukt u opnieuw op **Ctrl+Shift+Enter**. Om hem te verwijderen, selecteert u alle cellen ervan en drukt u op **Delete**. Beide werken op de hele matrix, in één stap van Ongedaan maken. F2 op een willekeurige cel van een matrix opent de formule van de matrix, en een beveiligd blad of een bereik met wachtwoord weigert of vraagt zoals bij elke andere cel. Een matrix of een deel ervan kopiëren, en vullen vanuit een van zijn cellen, geven de waarden die hij toont, niet een formule.
- Op een **beveiligd blad** kan een vergrendelde cel niet worden gewijzigd. Elke cel is vergrendeld, tenzij de werkmap aangeeft dat hij dat niet is. JeffCrunch laat u weten wanneer u dat probeert. Om de cel te wijzigen, kiest u op het tabblad Controleren de opdracht Beveiliging blad opheffen.
- De eerste rij van een **tabel** bevat de namen van de kolommen. Typ een nieuwe naam in een van die cellen om de kolom te hernoemen: de formules die de oude naam gebruiken volgen, en de tabel in het bestand blijft geldig. Een naam die een andere kolom al heeft, krijgt een getal erachter, een naam kan niet leeg zijn, en **Delete** wist die rij niet.
- Een werkmap die niet in zijn geheel kon worden gelezen, kan worden bekeken maar niet gewijzigd of opgeslagen.
- Een functie die JeffCrunch nog niet heeft, laat een nieuwe formule zonder waarde in de cel achter; de werkmap vraagt het programma dat hem opent om de formule daarna te berekenen.
- Opmaak, sorteren, en zoeken en vervangen komen in een latere versie. Rijen, kolommen en cellen invoegen en verwijderen, knippen en plakken, doorvoeren, en de afmetingen van rijen en kolommen staan in [Rijen, kolommen en cellen invoegen en verwijderen](insert-delete), [Knippen, kopiëren en plakken](clipboard), [Cellen doorvoeren en optellen](fill-sum), en [De breedte van kolommen en de hoogte van rijen wijzigen](sizes).

## Een symbool invoegen

Klik op **Invoegen > Symbolen > Symbool** om het venster Symbool te openen. Kies een teken en klik op **Invoegen**, of dubbelklik erop. Het komt in op de cursor van de invoer die u typt. Als u niet typt, opent de eigen invoer van de cel met het symbool aan het einde, en **Enter** zet het in de cel. Voorbeeld: met een cel die het woord *Totaal* bevat, maakt het euroteken de invoer *Totaal€*. De symbolen die u het laatst gebruikte, staan eerst in de lijst.

**Het teken uit een lettertype halen.** Kies een lettertype in het vak **Lettertype**, en kies daarna het teken. Het teken komt in een eigen stuk tekst dat in dat lettertype staat, en de rest van de tekst van de cel houdt het lettertype van de cel. Voorbeeld: in een cel met *Totaal* maakt een vinkje uit een symboollettertype *Totaal✓*, met alleen het vinkje in dat lettertype. Met **(normale tekst)** gekozen, tekent het lettertype van de cel het symbool, dus een symbool dat het lettertype mist, verschijnt als een vakje. In een formule komt alleen het teken erin.

**Speciale tekens.** Het tabblad **Speciale tekens** toont de vaste spatie en het vaste koppelteken, het optionele afbreekstreepje, het korte en lange streepje, de korte en lange spatie, de aanhalingstekens, en het breedteloze optionele afbreekpunt, elk met zijn naam. Dubbelklik op een teken om het in te voegen.

**Een symbool een sneltoets geven.** Kies een symbool of een speciaal teken, klik op **Sneltoets**, druk op de toetsen die u wilt, zoals **Ctrl+Alt+Shift+J**, en klik op **OK**. De toetsen zetten dat symbool dan in, op dezelfde manier als het venster dat doet. Laat de toetsen leeg en klik op **OK** om ze te verwijderen. JeffCrunch laat u weten wanneer een andere opdracht die toetsen al heeft. Het symbool wordt een opdracht, dus het staat ook onder **Bestand > Opties > Aanpassen > Toetsenbord**, waar u de toetsen kunt wijzigen (zie [Sneltoetsen](keyboard)).

## Nieuwe werkmappen

In **Bestand > Opties > Nieuwe werkmappen** kiest u het lettertype, de lettergrootte en het aantal bladen waarmee elke nieuwe werkmap begint. Werkmappen die u al hebt, worden niet gewijzigd.
