# Typen in cellen
<!-- keywords: symbool, symboollettertype, speciale tekens, sneltoets voor een symbool, automatische correctie, slimme aanhalingstekens, streepje, breuk, koppeling, adres, vetgedrukt woord, deel van een cel, opgemaakte tekst, deel van tekst opmaken, kleur een woord, invoeren, typen, invoer, bewerken, wijzigen, cel, invoer, formulebalk, F2, dubbelklikken, Tab, Esc, afbreken, ongedaan maken, opnieuw, verwijderen, wissen, percentage, valuta, geld, datum, tijd, getal, tekst, apostrof, formule, is-gelijk-teken, vervangen, limiet, tekens -->

Als u iets in een cel wilt zetten, selecteert u de cel en begint u met typen. Wat u typt, vervangt wat de cel eerder bevatte. Druk op **Enter** om de invoer te bewaren, of op **Esc** om de invoer te annuleren en de cel te laten zoals hij was.

## Drie manieren om te beginnen

- **Typ over de cel.** Selecteer een cel en typ. Het eerste teken dat u typt, begint de invoer, en de cel toont wat u typt terwijl u verder gaat.
- **Bewerk wat in de cel staat.** Druk op **F2** of dubbelklik op de cel. De cel opent met zijn inhoud, en de cursor staat aan het eind. Gebruik de pijltjestoetsen om in de tekst te bewegen.
- **Gebruik de formulebalk.** Klik in de formulebalk, naast het naamvak, en typ daar. De cel toont ook wat u typt.

Als u over een cel typt, rondt een pijltjestoets de invoer af en verplaatst de selectie. Als u bewerkt wat in de cel staat, verplaatsen de pijltjestoetsen de cursor in de tekst. Druk op **F2** om van de ene naar de andere manier te gaan.

## De invoer afronden

| Toetsen | Wat ze doen |
| --- | --- |
| Enter | Bewaart de invoer en gaat omlaag (of waarheen **Bestand > Opties > Bewerken** aangeeft) |
| Shift+Enter | Bewaart de invoer en gaat de andere kant op |
| Ctrl+Enter | Zet de invoer in alle geselecteerde cellen en blijft waar u bent |
| Tab | Bewaart de invoer en gaat naar rechts |
| Shift+Tab | Bewaart de invoer en gaat naar links |
| Pijltjestoetsen | Bewaart de invoer en gaat één cel verder (alleen als u over de cel hebt getypt) |
| Esc | Annuleert de invoer |
| Alt+Enter | Begint een nieuwe regel in de cel |
| Ctrl+; | Zet de datum van vandaag erin (in de invoer, als u er een typt) |
| Ctrl+Shift+; | Zet de huidige tijd erin |
| Alt+Omlaag | Toont de woorden die al in de kolom staan, en kies er een |

**Ctrl+Enter** met meerdere geselecteerde cellen zet wat u typte in alle cellen. Een formule wordt geschreven voor de cel waarin u typte, en in elk van de andere cellen werkt hij als een kopie daarvan zou werken: als u **=A1** typt terwijl het blok E6:F7 is geselecteerd, wordt dat in F7 **=B2**. Het is één stap voor Ongedaan maken.

**Alt+Omlaag** op een cel toont, in volgorde en elk woord één keer, de woorden uit de cellen boven en onder die cel die niet door een lege cel zijn gescheiden. Als u er een kiest, wordt die getypt. Heeft een cel een eigen lijst, dan toont Alt+Omlaag die lijst.

In **Bestand > Opties > Bewerken** kiest u of **Enter** omlaag, naar rechts, omhoog of naar links gaat, of niet verplaatst.

Als u met **Tab** in meerdere cellen langs een rij typt en daarna op **Enter** drukt, gaat de selectie naar de volgende rij, in de kolom waar u begon. Als u eerst een blok cellen selecteert, verplaatsen **Enter** en **Tab** de selectie door het blok.

## Automatische correctie tijdens het typen

Wanneer u een spatie of een leesteken, zoals een punt, na een woord typt, kan JeffCrunch het woord corrigeren zodra u het afrondt. Dit gebeurt bij veelvoorkomende typefouten (zoals "teh" in plaats van "the"), bij twee hoofdletters aan het begin van een woord, bij een kleine letter die een zin of een dag van de week begint, en bij een rechte aanhalingsteken (dat wordt een krul-aanhalingsteken). Ook worden twee koppeltekens tussen woorden omgezet in een lang streepje, en 1/2, 1/4 en 3/4 in ½, ¼ en ¾, als de invoer woorden bevat. Een getal zoals 1/2 dat u alleen typt, blijft zoals altijd een datum of een breuk.

Elke correctie is een afzonderlijke stap. Druk direct na een correctie op **{key:Undo}** om alleen die correctie terug te draaien en wat u typte te behouden. In een formule wordt niets gewijzigd: een invoer die begint met een is-gelijk-teken, of met een plus- of minteken gevolgd door een formule, blijft zoals u hem typte. Een webadres (zoals https://example.org) of een e-mailadres dat de hele invoer vormt, wordt bij het afronden een koppeling in zijn cel, als **Webadressen direct omzetten in hyperlinks tijdens het typen** aanstaat.

U schakelt deze opties in of uit via **Bestand > Opties > Controle**. Een spelfout wordt tijdens het typen gemarkeerd: zie [Spelling](notes-links).

## Een deel van de tekst opmaken

Sommige woorden in een cel kunnen er anders uitzien dan de rest. Zo kan één woord vet of rood zijn.

1. Druk op F2 om de cel te bewerken (of dubbelklik erop).
2. Selecteer de woorden die u wilt wijzigen, met Shift en de pijltjestoetsen of met de muis.
3. Gebruik **Vet**, **Cursief**, **Onderstrepen**, **Doorhalen**, **Lettertype**, **Lettergrootte** of **Tekstkleur** op het tabblad **Start**, of Ctrl+B, Ctrl+I en Ctrl+U.
4. Druk op Enter.

Alleen de geselecteerde woorden veranderen. De rest van de cel behoudt de eigen opmaak van de cel. Tijdens het bewerken toont het invoervak de woorden in één opmaak, en de cel toont elk deel in zijn eigen opmaak wanneer u op Enter drukt. Als u later woorden typt of verwijdert, verplaatst de opmaak zich met de woorden mee. Een lettertype of lettergrootte kiezen zonder woorden te selecteren, wijzigt de hele cel, zoals altijd.

## De minibalk

Een kleine balk met de opmaak die u het meest gebruikt, verschijnt waar u werkt, zodat u niet naar het lint hoeft. De balk neemt het toetsenbord nooit over: wat u typte, blijft waar het was.

- **Geselecteerde tekst in een cel:** selecteer woorden met de muis terwijl u een cel bewerkt, en de balk verschijnt boven de woorden. Hij heeft de vakken **lettertype** en **grootte**, **Vet**, **Cursief**, **Onderstrepen**, **Tekengrootte vergroten**, **Tekengrootte verkleinen** en **Tekstkleur**. Ze wijzigen alleen de geselecteerde woorden, net als op het lint.
- **Geselecteerde cellen:** klik met de rechtermuisknop op de cellen, dan staat de balk bovenaan het snelmenu. Hij heeft ook **Opvulkleur**, **Randen**, **Valuta**, **Procentnotatie**, **Duizendtalnotatie**, **Meer decimalen**, **Minder decimalen**, **Samenvoegen en centreren** en **Opmaak kopiëren**. Dit zijn de eigen opdrachten van het lint, en elk is één stap die u ongedaan kunt maken.

De balk boven geselecteerde tekst vervaagt wanneer u de aanwijzer ervan wegbeweegt, en verdwijnt als de aanwijzer ver weg is. Hij verdwijnt ook als u typt, op Esc drukt of scrolt. Om de balk uit te schakelen, haalt u het vinkje weg bij **Minibalk weergeven bij selectie** op de pagina **Algemeen** van **Bestand > Opties**; de andere JeffOffice-apps volgen die keuze ook. De knop Snelle analyse die onder een geselecteerd bereik verschijnt, zit niet in deze versie.

Cellen die opmaak binnen de tekst hebben van een ander programma, tonen die opmaak, behouden die en slaan die weer op. Een cel die u wijzigt, behoudt zijn opmaak. Opmaak binnen een tekst geldt voor tekst, niet voor getallen of formules. Een kopie die u opslaat als een ouder .xls-bestand of als .ods, behoudt de woorden, en bij het opslaan meldt het programma dat de opmaak binnen de woorden niet wordt bewaard.

## Wat u typt, wordt

JeffCrunch leest wat u typt op de manier die is ingesteld op uw computer. Het decimaalteken, het duizendtalteken, het valutateken en de volgorde van dag en maand in een datum komen allemaal uit die instellingen. Een getal dat u typt in een cel zonder eigen opmaak, krijgt de opmaak die het getal toont zoals u het typte.

| U typt | De cel bevat | En toont |
| --- | --- | --- |
| 1234.5 | het getal 1234.5 | 1234.5 |
| 5% | het getal 0.05 | 5% |
| $12.50 | het getal 12.5 | $12.50 |
| 1 1/2 | het getal 1.5 | 1 1/2 |
| 3/4/2025 | de datum, als aantal dagen | 3/4/2025 |
| 10:30 PM | de tijd, als deel van een dag | 10:30 PM |
| TRUE | de logische waarde waar | TRUE |
| #N/A | de fout #N/A | #N/A |
| =B2*2 | een formule | het resultaat van de formule |
| al het andere | tekst | de tekst |

Een cel die al een eigen opmaak heeft, zoals een percentage of een datum, behoudt die. Een cel waarvan de opmaak **Tekst** is, behoudt alles wat u typt als tekst, behalve een formule.

Wilt u iets wat op een getal lijkt als tekst bewaren, zoals een code **007**, dan typt u eerst een apostrof: **'007**. De apostrof hoort niet bij de tekst. Als u de cel bewerkt, ziet u hem weer.

## Formules

Een formule begint met een is-gelijk-teken. **=B2*2** verdubbelt bijvoorbeeld het getal in B2. U mag namen van functies met hoofdletters of kleine letters typen; JeffCrunch schrijft ze met hoofdletters.

De namen van functies zijn altijd de Engelse, in elke taal: typ **SUM**, niet de naam die uw taal eraan geeft. De leestekens ertussen volgen de instellingen van uw computer. Waar het decimaalteken een komma is, zoals in het Duits en het Frans, scheidt u de argumenten van een functie met een puntkomma en schrijft u in een getal een komma: **=SUM(A1;A2)*0,5**. De formulebalk en **F2** tonen formules op dezelfde manier. Waar het decimaalteken een punt is, scheidt u argumenten met een komma: **=SUM(A1,A2)*0.5**. Het bestand bewaart altijd de vorm die elk programma leest, met komma's tussen argumenten en punten in getallen, zodat het overal hetzelfde opent.

Als een formule een fout heeft, zoals een haakje dat nooit wordt gesloten, vertelt JeffCrunch wat er misgaat. De invoer blijft zoals u hem typte, met de cursor bij de fout, zodat u hem kunt herstellen. Er verandert niets totdat de formule klopt.

Een cel bevat maximaal 32.767 tekens, en een formule maximaal 8.192. JeffCrunch meldt het als een invoer langer is.

## Cellen wissen

Selecteer de cellen en druk op **Delete**. Dat wist wat de cellen bevatten en behoudt hun opmaak, en het is één stap die u ongedaan kunt maken. JeffCrunch wist maximaal 100.000 cellen tegelijk.

Op het tabblad **Start** heeft de knop **Wissen** een menu. **Alles wissen** leegt de cellen, hun opmaak, hun notities, hun opmerkingen (de discussies met antwoorden) en hun koppelingen. Een discussie met meer dan 500 opmerkingen blijft behouden, en JeffCrunch meldt dat, omdat Ongedaan maken de opmerkingen die het niet heeft gelezen niet kan terughalen. **Opmaak wissen** behoudt wat de cellen bevatten en zet de gewone opmaak terug. **Inhoud wissen** is hetzelfde als **Delete**. **Opmerkingen en notities wissen** haalt alleen de notities en de opmerkingen van de geselecteerde cellen. **Hyperlinks wissen** haalt alleen de koppelingen weg, en de cellen behouden hun tekst. Een keuze is grijs als de geselecteerde cellen niets hebben wat ze kan wissen.

## Ongedaan maken en Opnieuw

**{key:Undo}** maakt de laatste wijziging ongedaan, en **{key:Redo}** voert haar opnieuw uit. Ze staan ook op de snelle werkbalk bovenaan. Elke cel die u invult, is één stap, en elk wissen is er ook één. Ongedaan maken zet de cel terug zoals hij was: de waarde, de formule, de opmaak en de formules die ervan afhangen. JeffCrunch onthoudt maximaal 1.000 stappen, minder als die heel veel cellen hebben gewijzigd, zodat het nooit veel geheugen gebruikt.

## Terwijl JeffCrunch de formules uitrekent

Wanneer u een cel wijzigt, rekent JeffCrunch op de achtergrond de formules uit die ervan afhangen. De cel die u wijzigde, toont meteen de nieuwe inhoud, en de andere cellen werken even later bij; de statusbalk zegt **Berekenen** totdat ze klaar zijn. U kunt ondertussen doorgaan met typen.

## Wat niet kan worden gewijzigd

- Een cel die deel uitmaakt van een **matrixformule** of een gegevenstabel, kan niet alleen worden gewijzigd.
- Op een **beveiligd blad** kan een vergrendelde cel niet worden gewijzigd. Elke cel is vergrendeld, tenzij de werkmap zegt dat hij dat niet is. JeffCrunch meldt het als u dat probeert. Om de cel te wijzigen, kiest u op het tabblad **Controleren** de optie **Beveiliging blad opheffen**.
- De eerste rij van een **tabel** bevat de namen van de kolommen. Typ een nieuwe naam in een van die cellen om de kolom te hernoemen: de formules die de oude naam gebruiken, volgen mee, en de tabel in het bestand blijft geldig. Een naam die een andere kolom al heeft, krijgt een nummer erachter, een naam mag niet leeg zijn, en **Delete** wist die rij niet.
- Een werkmap die niet helemaal kon worden gelezen, kan wel worden bekeken, maar niet worden gewijzigd of opgeslagen.
- Een functie die JeffCrunch nog niet kent, laat een nieuwe formule zonder waarde in de cel; de werkmap vraagt het programma dat hem opent om de formule uit te rekenen.
- Opmaak, sorteren en zoeken en vervangen komen in een latere versie. Invoegen en verwijderen, knippen en plakken, doorvoeren en de grootte van rijen en kolommen vindt u in [Rijen, kolommen en cellen invoegen en verwijderen](insert-delete), [Knippen, kopiëren en plakken](clipboard), [Cellen doorvoeren en optellen](fill-sum) en [De breedte van kolommen en de hoogte van rijen wijzigen](sizes).

## Een symbool invoegen

Klik op **Invoegen > Symbolen > Symbool** om het vak Symbool te openen. Kies een teken en klik op **Invoegen**, of dubbelklik erop. Het komt in op de cursor van de invoer die u typt. Als u niet typt, opent de eigen invoer van de cel met het symbool aan het eind, en **Enter** zet het in de cel. Voorbeeld: als een cel het woord *Totaal* bevat en u kiest het eurosymbool, wordt de invoer *Totaal€*. De symbolen die u het laatst gebruikte, staan het eerst in de lijst.

**Een teken uit een lettertype halen.** Kies een lettertype in het vak **Lettertype**, en kies daarna het teken. Het teken komt in een eigen stuk tekst dat in dat lettertype wordt gezet, en de rest van de tekst van de cel behoudt het lettertype van de cel. Voorbeeld: in een cel met *Totaal* maakt een vinkje uit een symboollettertype *Totaal✓*, waarbij alleen het vinkje in dat lettertype staat. Als **(normale tekst)** is gekozen, tekent het lettertype van de cel het symbool, dus een symbool dat het lettertype niet heeft, verschijnt als een vakje. In een formule komt alleen het teken erin.

**Speciale tekens.** Het tabblad **Speciale tekens** toont de vaste spatie en het vaste koppelteken, het optionele afbreekstreepje, het korte en lange streepje, de korte en lange spatie, de aanhalingstekens en de breedteloze optionele onderbreking, elk met zijn naam. Dubbelklik op er een om hem in te voegen.

**Een symbool een sneltoets geven.** Kies een symbool of een speciaal teken, klik op **Sneltoets**, druk de toetsen die u wilt, zoals **Ctrl+Alt+Shift+J**, en klik op **OK**. Die toetsen zetten dan dat symbool erin, net als het vak dat doet. Laat de toetsen leeg en klik op **OK** om ze te verwijderen. JeffCrunch meldt het als een andere opdracht die toetsen al heeft. Het symbool wordt een opdracht, dus het staat ook onder **Bestand > Opties > Aanpassen > Toetsenbord**, waar u de toetsen kunt wijzigen (zie [Sneltoetsen](keyboard)).

## Nieuwe werkmappen

Kies in **Bestand > Opties > Nieuwe werkmappen** het lettertype, de lettergrootte en het aantal bladen waarmee elke nieuwe werkmap begint. Werkmappen die u al hebt, worden niet gewijzigd.
