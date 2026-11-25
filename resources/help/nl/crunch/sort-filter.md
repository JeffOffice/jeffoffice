# Sorteren en filteren
<!-- keywords: sorteren, filteren, autofilter, oplopend, aflopend, A-Z, regels, kop, kleur, lijst, maanden, weekdagen, hoofdletters, kolommen, grootste, kleinste, gemiddelde, tussen, begint met, bevat, datum, wissen, opnieuw toepassen, vervolgkeuzelijst, verborgen rijen, tabel, Ctrl+Shift+L -->

## Sorteren

Selecteer een cel in uw gegevens en kies **Sorteren van A naar Z** of **Sorteren van Z naar A** (**Sorteren en filteren** op het tabblad Start). JeffCrunch neemt het hele gegevensblok rond de cel, behoudt een kopregel waar die wordt gevonden (tekst boven getallen), en zet de rijen op volgorde op basis van de kolom waarin u staat. Als u meerdere cellen hebt geselecteerd, sorteert het programma alleen die cellen; staan er gegevens naast, dan vraagt het of die mee moeten worden gesorteerd.

De volgorde is: eerst getallen, dan tekst, dan WAAR en ONWAAR, dan fouten; lege cellen komen altijd als laatste. Hoofdletters en kleine letters worden als gelijk behandeld, tenzij u anders aangeeft. Tekst wordt overal op dezelfde manier geordend: op de letters van het Engels, waarbij een letter met accent naast de gewone letter wordt geplaatst, ongeacht de taal van de computer.

Kies **Sorteren…** om het dialoogvenster Sorteren te openen:

- Voeg per kolom waarop u wilt sorteren een regel toe. Een latere regel doorbreekt de gelijkspelen van de eerdere regels. Gebruik **Omhoog verplaatsen** en **Omlaag verplaatsen** om de volgorde van de regels te wijzigen, en **Regel dupliceren** om van een bestaande regel te vertrekken.
- Sorteer **op** waarden, de **opvullingskleur** van de cellen of hun **tekstkleur**. Bij een kleur kiest u of die **Eerst** of **Laatst** komt; de overige rijen behouden hun volgorde. Een kleur is de kleur die hij wordt: een kleur uit het palet en een kleur met dezelfde hoeveelheden rood, groen en blauw zijn gelijk.
- Bij waarden kan de volgorde **A–Z**, **Z–A** of een lijst zijn: de dagen van de week en de maanden (voluit en afgekort, in het Engels en in uw taal), of een lijst die u typt, met komma's tussen de waarden. Waarden die niet in de lijst staan, komen na de waarden die er wel in staan.
- **De eerste rij bevat de kolomnamen** houdt de eerste rij buiten de sortering. **Hoofdletters meetellen** maakt onderscheid tussen hoofdletters en kleine letters (kleine letters eerst). **Kolommen sorteren, geen rijen** zet de kolommen op volgorde op basis van de waarden in een rij. **Getallen in tekst op waarde tellen** zet "Artikel 2" vóór "Artikel 10" (zonder deze optie worden letters en cijfers één voor één vergeleken, en komt "Artikel 10" eerst). De keuze geldt voor de sortering die u uitvoert; het venster onthoudt haar voor de volgende sortering en voor **Opnieuw toepassen**. Het bestand slaat de keuze niet op, dus **Opnieuw toepassen** nadat u het bestand opnieuw opent, sorteert op de gewone manier. Compatibiliteit controleren meldt dit zolang een bewaarde sortering van een filter of tabel op deze manier is gemaakt (een latere gewone sortering ervan haalt de melding weg). Getallen van elke lengte tellen op waarde.

Een formule verhuist mee met zijn rij, en verwijzingen naar cellen in dezelfde rij blijven in de rij. Een verwijzing naar een cel in een andere rij wordt aangepast alsof de formule was gekopieerd. Wat aan een cel is gekoppeld, gaat met die cel mee: de celopmaak, de notitie en de koppeling verhuizen naar de plek waar de cel naartoe gaat. De regels die een celbereik bestrijken (wat er mag worden ingevoerd, en voorwaardelijke opmaak) blijven staan waar ze staan, omdat ze de plaatsen bestrijken en niet de cellen. Sorteren is één stap die u ongedaan kunt maken, en met **Esc** stopt u een grote sortering; een grote sortering wordt beetje bij beetje uitgevoerd.

Sommige cellen kunnen niet worden gesorteerd, en u krijgt te horen waarom: cellen die op een beveiligd blad zijn vergrendeld, deel van een matrix, samengevoegde cellen die niet allemaal even groot zijn (of die over meer dan één rij lopen), samengevoegde cellen die door de rand van de selectie worden doorsneden, en de kopregel van een tabel.

## Filteren

Selecteer een cel in uw gegevens en kies **Filter** (**Ctrl+Shift+L**). Op elke cel van de eerste rij verschijnt een knop. Klik erop om te zien:

- **Sorteren van A naar Z** en **Sorteren van Z naar A**, en **Sorteren op kleur**.
- Een lijst met de waarden in de kolom, met selectievakjes, een zoekvak en **(Alles)**. Een kolom met datums wordt per jaar, maand en dag weergegeven; waarden in die kolom die geen datum zijn, staan eronder. Schakel uit wat u niet wilt en klik op **OK**. Zoeken toont de waarden die al uw woorden bevatten; **(Alle gevonden items)** vinkt dan alleen die aan of uit. De lijst omvat de hele kolom. Heeft de kolom meer verschillende waarden dan de lijst kan bevatten, dan wordt dat gemeld en wordt de lijst afgekapt; gebruik voor de rest een voorwaarde.
- **Kleur behouden**, op opvullingskleur of tekstkleur.
- **Getalfilters**, **Tekstfilters** of **Datumfilters**: is gelijk aan, boven, minstens, onder, maximaal, tussen, de grootste of kleinste (items of procent), boven of onder het gemiddelde, begint met, eindigt op, bevat, datums in een periode (vandaag, huidige week, volgende maand, vorig kwartaal, tot nu toe dit jaar, een kwartaal of een maand van elk jaar), en **Aangepaste voorwaarde** van twee vergelijkingen die met En of Of zijn verbonden. In de tekst van een voorwaarde staan ? en \* voor elk willekeurig teken en elk willekeurig stuk tekst.
- **Alles weergeven** van de kolom.

Een knop met een trechter erop toont een kolom met een filter. De rijen die niet overeenkomen, worden verborgen en behouden hun rijnummers. **Wissen** (in het menu **Sorteren en filteren**) haalt alle voorwaarden weg en toont de rijen; **Opnieuw toepassen** berekent de voorwaarden opnieuw nadat u een waarde hebt gewijzigd. **Filteren** nogmaals kiezen schakelt de knoppen weer uit.

Rijen die door een filter zijn verborgen, worden apart gehouden van rijen die u zelf hebt verborgen: **Rijen zichtbaar maken** toont geen rij die het filter verbergt, en het wissen van het filter toont geen rij die u zelf hebt verborgen. Bij het sorteren van een gefilterd blok worden alleen de rijen die zichtbaar zijn onderling op volgorde gezet.

Een filter werkt op een heel blad, tot 1.048.576 rijen, beetje bij beetje; **Wissen** en het uitschakelen van het filter werken bij elke omvang. Wat een filter moet doen, is beperkt tot 40 miljoen controles (de rijen maal de kolommen met een voorwaarde, met een deel voor de waarden die de voorwaarden bevatten); een filter dat meer vraagt, meldt dat en wordt niet uitgevoerd. U kunt dan minder rijen filteren of minder voorwaarden gebruiken. Een filter van het blad omvat geen cellen van een tabel: een tabel heeft eigen knoppen, die op dezelfde manier werken (zie [Tabellen](tables)).

Een lijst met waarden in een voorwaarde wordt opgeslagen zoals een bestand die schrijft, zonder taal in de getallen en datums (1,234.50); een waarde komt overeen met een cel die die waarde in uw taal toont of op die manier. Een bestand dat in de ene taal is gefilterd, filtert in een andere taal dezelfde cellen.

Het filter wordt met zijn voorwaarden en de rijen die het verbergt in het bestand opgeslagen, zodat het bestand opent met dezelfde zichtbare rijen. Bij het opnieuw openen van een bestand verschijnt de trechter op elke kolom met een voorwaarde; rijen die in het bereik van het filter zijn verborgen, tellen als rijen van het filter.
