# Sorteren en filteren
<!-- keywords: sorteren, filteren, filter, autofilter, oplopend, aflopend, A-Z, regels, sorteerregels, koprij, kleur, lijst, maanden, weekdagen, hoofdletters, kolommen, grootste, kleinste, gemiddelde, tussen, begint met, bevat, datum, wissen, opnieuw toepassen, vervolgkeuzelijst, verborgen rijen, tabel, Ctrl+Shift+L -->

## Sorteren

Selecteer een cel in uw gegevens en kies **Sorteren van A naar Z** of **Sorteren van Z naar A** (**Sorteren en filteren** op het tabblad Start). JeffCrunch neemt het hele blok gegevens rond de cel, houdt een koprij apart als die wordt gevonden (tekst boven getallen) en zet de rijen op volgorde volgens de kolom waarin u staat. Als u meerdere cellen hebt geselecteerd, worden alleen die gesorteerd; staan er gegevens naast, dan wordt gevraagd of die moeten meegaan.

In de volgorde komen eerst getallen, dan tekst, dan WAAR en ONWAAR en dan fouten; lege cellen komen altijd als laatste. Hoofdletters en kleine letters gelden als gelijk, tenzij u iets anders aangeeft. Tekst wordt overal op dezelfde manier gesorteerd: volgens de letters van het Engels, waarbij een letter met accent bij de letter zonder accent wordt geplaatst, ongeacht de taal van de computer.

Kies **Sorteren…** om het dialoogvenster Sorteren te openen:

- Voeg een regel toe voor elke kolom waarop u wilt sorteren. Een latere regel beslist bij gelijke waarden in de eerdere regels. Met **Omhoog** en **Omlaag** wijzigt u de volgorde van de regels, en met **Regel dupliceren** begint u vanaf een bestaande regel.
- Sorteer **Op basis van** waarden, de **opvulkleur** van de cellen of de **tekstkleur**. Kies bij een kleur of die **Bovenaan** of **Onderaan** komt; de andere rijen behouden hun volgorde. Een kleur is de kleur die er uiteindelijk uitkomt: een kleur uit het palet en een kleur met dezelfde waarden voor rood, groen en blauw zijn gelijk.
- Bij waarden kan de volgorde **A–Z**, **Z–A** of een lijst zijn: de dagen van de week en de maanden (voluit en afgekort, in het Engels en in uw eigen taal), of een lijst die u zelf typt, met komma's tussen de waarden. Waarden die niet in de lijst staan, komen na de waarden die er wel in staan.
- **De eerste rij bevat de kolomnamen** houdt de eerste rij buiten het sorteren. **Hoofdlettergevoelig** maakt onderscheid tussen hoofdletters en kleine letters (kleine letters eerst). **Kolommen sorteren, geen rijen** zet de kolommen op volgorde volgens de waarden in een rij.

Een formule verhuist mee met haar rij, en verwijzingen naar cellen in dezelfde rij blijven in die rij. Een verwijzing naar een cel in een andere rij wordt aangepast alsof de formule is gekopieerd. Wat bij een cel hoort, gaat mee: de opmaak, de notitie en de koppeling van de cel verhuizen naar de nieuwe plaats van de cel. Regels die voor een celbereik gelden (wat mag worden ingevoerd, en voorwaardelijke opmaak) blijven waar ze zijn, omdat ze voor de plaatsen gelden en niet voor de cellen. Sorteren is één stap die u ongedaan kunt maken, en met **Esc** stopt u een grote sorteerbewerking; een grote sortering wordt beetje bij beetje uitgevoerd.

Sommige cellen kunnen niet worden gesorteerd, en u krijgt te horen waarom: vergrendelde cellen op een beveiligd blad, cellen die deel uitmaken van een matrix, samengevoegde cellen die niet allemaal even groot zijn (of die meer dan één rij beslaan), samengevoegde cellen die door de rand van de selectie worden doorsneden, en de koprij van een tabel.

## Filteren

Selecteer een cel in uw gegevens en kies **Filter** (**Ctrl+Shift+L**). Op elke cel van de eerste rij verschijnt een knop. Klik op een knop voor het volgende:

- **Sorteren van A naar Z** en **Sorteren van Z naar A**, en **Sorteren op kleur**.
- Een lijst met de waarden in de kolom, met selectievakjes, een zoekvak en **(Alles)**. Een kolom met datums wordt weergegeven per jaar, maand en dag, en de waarden die geen datum zijn, staan daaronder. Schakel uit wat u niet wilt zien en klik op **OK**. Bij zoeken worden de waarden weergegeven die al uw woorden bevatten; met **(Alle gevonden items)** schakelt u alleen die in of uit. De lijst beslaat de hele kolom. Heeft de kolom meer verschillende waarden dan de lijst kan bevatten, dan wordt dat gemeld en wordt de lijst ingekort; gebruik voor de rest een voorwaarde.
- **Filteren op kleur**, op opvulkleur of tekstkleur.
- **Getalfilters**, **Tekstfilters** of **Datumfilters**: is gelijk aan, is groter dan, is groter dan of gelijk aan, is kleiner dan, is kleiner dan of gelijk aan, tussen, de grootste of kleinste (items of procent), boven of onder gemiddelde, begint met, eindigt op, bevat, datums in een periode (vandaag, deze week, volgende maand, vorig kwartaal, tot nu toe dit jaar, een kwartaal of maand van een willekeurig jaar), en **Aangepaste voorwaarde** met twee vergelijkingen die met En of Of zijn verbonden. In de tekst van een voorwaarde staat ? voor één willekeurig teken en \* voor een willekeurig aantal tekens.
- **Alles weergeven** van de kolom.

Een knop met een trechter erop geeft aan dat een kolom een filter heeft. De rijen die niet overeenkomen, worden verborgen en behouden hun rijnummers. **Wissen** (in het menu **Sorteren en filteren**) verwijdert alle voorwaarden en geeft de rijen weer; **Opnieuw toepassen** past de voorwaarden opnieuw toe nadat u een waarde hebt gewijzigd. Kies nogmaals **Filter** om de knoppen uit te schakelen.

Rijen die door een filter zijn verborgen, worden apart gehouden van rijen die u zelf hebt verborgen: **Rijen zichtbaar maken** geeft geen rij weer die door het filter is verborgen, en het wissen van het filter geeft geen rij weer die u zelf hebt verborgen. Als u een gefilterd blok sorteert, worden alleen de zichtbare rijen onderling op volgorde gezet.

Een filter werkt op een heel blad, tot 1.048.576 rijen, beetje bij beetje; **Wissen** en het uitschakelen van het filter werken ongeacht de grootte. Wat een filter moet doen, is beperkt tot 40 miljoen controles (het aantal rijen maal het aantal kolommen met een voorwaarde, plus enkele voor de waarden die in de voorwaarden staan); een filter dat meer vraagt, meldt dat en wordt niet gemaakt. U kunt dan minder rijen filteren of minder voorwaarden gebruiken. Een filter van het blad geldt niet voor cellen van een tabel: een tabel heeft eigen knoppen, die op dezelfde manier werken (zie [Tabellen](tables)).

Een lijst met waarden in een voorwaarde wordt bewaard zoals een bestand die schrijft, zonder taalnotatie in getallen en datums (1,234.50). Een waarde komt overeen met een cel die de waarde in uw eigen taal of op die manier weergeeft; een bestand dat in de ene taal is gefilterd, filtert in een andere taal dezelfde cellen.

Het filter wordt met de voorwaarden en de verborgen rijen in het bestand opgeslagen, zodat het bestand wordt geopend met dezelfde zichtbare rijen. Bij het opnieuw openen van een bestand verschijnt de trechter op elke kolom met een voorwaarde; rijen die binnen het bereik van het filter verborgen zijn, gelden als verborgen door het filter.
