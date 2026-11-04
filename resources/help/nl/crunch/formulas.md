# Hoe formules worden getoond en berekend
<!-- keywords: formule, formules, berekenen, berekening, opnieuw berekenen, functie, functies, waarde, resultaat, opgeslagen, F9, kringverwijzing, formules weergeven, formulebalk, fout, punt, aanwijzen, klikken op een cel, verwijzing, F4 -->

Een **formule** is een cel die zijn waarde berekent uit andere cellen, zoals =SOM(B3:B8). De cel toont de waarde en de **formulebalk** toont de formule.

## Wat JeffCrunch met formules doet

Wanneer u een werkmap opent, bevat het bestand de laatste waarde van elke formule, zodat het blad meteen kan worden getoond. Daarna berekent JeffCrunch elke formule opnieuw, een paar tegelijk, zodat het venster vlot blijft. De statusbalk toont **Berekenen** met een percentage zolang dat bezig is. Wanneer het klaar is, bevatten de cellen de eigen resultaten van JeffCrunch. Wilt u alles opnieuw berekenen, klik dan op **Alles opnieuw berekenen** op het tabblad **Formules**, of druk op **F9**.

## Wanneer JeffCrunch een formule niet kan berekenen

JeffCrunch heeft meer dan 400 functies, en het aantal groeit nog. Een formule die een functie gebruikt die JeffCrunch nog niet heeft, behoudt de waarde die het bestand ervoor heeft opgeslagen. Dat geldt ook voor een formule die naar een andere werkmap verwijst. Cellen die daarvan afhankelijk zijn, worden berekend op basis van de opgeslagen waarden.

## Functies die een paar opmerkingen nodig hebben

- **Obligaties en leningen.** PRICE, YIELD, DURATION, ODDFPRICE en de andere obligatiefuncties tellen dagen op de manier die het opgegeven **grondslag** aangeeft (0 is 30 dagen per maand in een jaar van 360 dagen, 1 zijn de werkelijke dagen, enzovoort). YIELD, ODDFYIELD, ODDLYIELD en RATE vinden hun antwoord door waarden te proberen totdat de koers of betaling klopt. Ze stoppen wanneer het antwoord tot ongeveer 15 cijfers nauwkeurig is, dus de laatste cijfers kunnen verschillen van die in een ander programma.
- **Verdelingen en toetsen.** De functies die eindigen op .DIST, .INV, .TEST, en de oudere namen zoals NORMDIST en TDIST, geven dezelfde antwoorden: een oudere naam blijft bewaard zodat oude bestanden blijven werken. De *.INV-functies worden ook gevonden door waarden te proberen.
- **Lijsten die meerdere cellen vullen.** SORT, SORTBY, FILTER, UNIQUE, SEQUENCE, RANDARRAY, XLOOKUP, MMULT, MINVERSE en LINEST geven een blok waarden. Voer ze in als matrixformule (**Ctrl+Shift+Enter**) over de cellen die ze moeten vullen. Het laten overlopen van een formule naar de naastgelegen cellen is nog niet mogelijk.
- **Teksten in andere schriften.** LENB, LEFTB, MIDB, RIGHTB, FINDB, SEARCHB en REPLACEB tellen elk teken als één, zoals op een computer die niet is ingesteld voor dubbelbytetekst. PHONETIC geeft de tekst van de cel, omdat JeffCrunch de lezingen van Japanse tekst niet bewaart.
- **CONVERT.** Eenheden worden als tekst geschreven, met hoofd- en kleine letters zoals in de lijst van de functie: "kg", "lbm", "mi", "F". Een metrisch voorvoegsel kan voor een eenheid die dat accepteert, zoals "km" of "mL".

## MOD: de rest

**MOD(n; d)** geeft wat er overblijft van **n** nadat zo veel mogelijk hele keren **d** is afgetrokken: het is n - d * INT(n / d). Voorbeeld: **=MOD(10; 3)** is 1, en **=MOD(-10; 3)** is 2, omdat INT(-10 / 3) gelijk is aan -4 en -10 - 3 * -4 gelijk is aan 2. Het antwoord heeft het teken van **d**. Als **d** 0 is, is het antwoord #DEEL/0!.

## Formules weergeven

Klik op **Formules weergeven** op het tabblad **Formules**, of druk op **Ctrl+`** (op een Mac de Control-toets, niet de Command-toets), om in plaats van de waarde de formule in elke cel te zien. De kolommen met formules worden breder, tot een bepaalde grens, zodat de formules volledig zichtbaar zijn. Ze krijgen hun oorspronkelijke breedte terug wanneer u dit uitschakelt, en de kolombreedten in uw bestand worden niet gewijzigd.

## Formules wijzigen

Om een formule te wijzigen, selecteert u de cel, drukt u op **F2**, past u de formule aan en drukt u op **Enter**. U kunt de formule ook in de formulebalk bewerken. Wanneer u een cel wijzigt, worden de formules die ervan afhankelijk zijn op de achtergrond opnieuw berekend. Zie [Tekst invoeren in cellen](edit-cells).

## Een formule schrijven

Typ **=** en de eerste letters van een functie, dan verschijnt een korte lijst met de functies en [namen](names) die zo beginnen, met een zin over elk. Druk op **Tab** om de gemarkeerde te kiezen. Gebruik de pijltjestoetsen **Omlaag** en **Omhoog** om een andere te markeren, en daarna **Tab** of **Enter**. **Esc** sluit de lijst. Zodra u tussen de haakjes van een functie staat, verschijnen de argumenten onder de cel, met het argument dat u typt in vet. De lijst en de argumenten zijn dezelfde als die **Functie kiezen** toont. Argumenten worden gescheiden door komma's, of door puntkomma's waar uw taal een komma als decimaalteken gebruikt.

## Naar cellen wijzen

In plaats van het adres van een cel te typen, kunt u ernaar wijzen. Terwijl u een formule typt, plaatst u de cursor waar een cel kan komen (direct na **=**, na een operator zoals **+**, een openend haakje of een komma) en klikt u op de cel. Het adres wordt voor u ingevuld. Sleep over cellen om een blok zoals B2:D5 te schrijven. Klik opnieuw voordat u iets anders typt, dan vervangt de nieuwe cel de zojuist geschreven. Houd **Ctrl** ingedrukt en klik om na een komma nog een adres toe te voegen. Klik op een kolomletter om een hele kolom zoals A:A te schrijven, of op een rijnummer voor een hele rij zoals 3:3, en sleep langs de letters of cijfers voor B:D of 2:5.

De pijltjestoetsen werken ook. Druk op een pijltoets en een aanwijzer beweegt weg van de cel waarin u typt, en het adres wordt geschreven; druk op meer pijltoetsen om verder te gaan, of houd **Shift** ingedrukt om de aanwijzer over een blok uit te rekken. De statusbalk toont **Wijzen** terwijl de pijltoetsen wijzen, **Enter** terwijl u typt, en **Bewerken** terwijl de pijltoetsen de cursor in de tekst verplaatsen. Druk op **F2** om te wisselen tussen wijzen en de tekst bewerken.

Wilt u naar een cel op een ander blad wijzen, klik dan op het tabblad van dat blad terwijl de cursor waar een cel kan komen staat, en klik daarna op de cel. De naam van het blad wordt er mee geschreven, tussen enkele aanhalingstekens wanneer dat nodig is, zoals in 'Mijn blad'!B3. **Enter** brengt u terug naar het blad waarop u begon en zet de formule in zijn cel. **Esc** annuleert alles, ook de wisseling van blad.

U kunt ook de rand van een omlijnd blok op het blad slepen om dat adres te verplaatsen, of het hoekvierkantje slepen om het uit te rekken; de formule verandert terwijl u sleept.

Druk op **F4** terwijl de cursor in of direct na een adres staat, om delen ervan vast te zetten met een dollarteken: A1 wordt $A$1, daarna A$1, dan $A1, en daarna weer A1.

Elk adres in de formule krijgt een kleur in de tekst, en de cellen waarnaar het verwijst worden in dezelfde kleur omlijnd op het blad, zodat u in één oogopslag ziet wat de formule leest. Een klik direct na een compleet adres, zoals in =A1, beëindigt de invoer en selecteert de cel waarop u klikte, zoals bij elke andere tekst.

## Gegevenstabellen

Een gegevenstabel (wat-als-tabel) toont wat een formule oplevert voor veel waarden van een of twee cellen. Opent u een werkmap met zo'n tabel, dan berekent JeffCrunch de cellen: bij een tabel met twee invoerwaarden staat de formule in de cel linksboven van de tabel, de waarden die horizontaal moeten worden geprobeerd gaan in de invoercel voor de rij, en die verticaal in de invoercel voor de kolom. Bij een tabel met één invoerwaarde staan de waarden onder elkaar in de kolom links ervan (en staan de formules in de rij erboven) of naast elkaar in de rij erboven (en staan de formules in de kolom links ervan). Elke cel van de tabel is het resultaat van de formule, en van elke formule die van de invoercel afhankelijk is, met de invoer ingesteld op die waarde; de rest van de werkmap blijft ongewijzigd. In de modus **Automatisch** worden de tabellen samen met de formules berekend, en de formules die de cellen van een tabel lezen, volgen mee. Een nieuwe gegevenstabel maken wordt nog niet aangeboden. Een tabel die niet kan worden berekend (een formule die van de invoer afhangt en een functie gebruikt die JeffCrunch niet heeft, of zichzelf wijzigt, zoals RAND of NU) behoudt de waarden die in het bestand staan.

## Met de hand berekenen

Klik op **Berekeningsmodus** op het tabblad **Formules** om te kiezen hoe de werkmap zijn formules berekent:

- **Automatisch** berekent ze terwijl u cellen wijzigt.
- **Automatisch, tabellen handmatig** doet hetzelfde, maar laat de cellen van gegevenstabellen zoals ze zijn tot u op **F9** drukt.
- **Handmatig** berekent niets, ook geen gegevenstabellen, totdat u erom vraagt. De statusbalk toont **Verouderd** na een wijziging. Druk op **F9** (**Alles opnieuw berekenen**) om alles te berekenen, of op **Shift+F9** (**Dit blad opnieuw berekenen**) om de formules van het blad waarop u staat te berekenen, en wat er verder op wachtte. Een werkmap die op Handmatig is ingesteld, opent met de resultaten die het bestand bevat.

De keuze wordt samen met de werkmap opgeslagen en is één stap die u ongedaan kunt maken.

## Formules die zichzelf lezen

Een formule die zijn eigen resultaat leest, rechtstreeks of via andere cellen, kan niet op de gebruikelijke manier worden berekend. JeffCrunch meldt dit één keer, noemt de cellen en omlijnt ze op het blad; ze behouden de waarden die ze hadden. Klik op **Kringverwijzingen** op het tabblad **Formules** voor de lijst, en dubbelklik op een cel in de lijst om er naartoe te gaan.

Sommige modellen hebben zo'n kring bewust nodig. Kies **Kringformules iteratief berekenen** in het menu **Berekeningsmodus**, dan worden de formules steeds opnieuw berekend, waarbij elke formule de laatste waarden van de andere leest, totdat geen waarde meer dan het opgegeven bedrag verandert of het opgegeven maximale aantal rondes is bereikt.

## Zien wat een formule leest

Selecteer een cel en klik op **Weergeven welke cellen deze cel beïnvloeden** om pijlen te tekenen naar de cellen die de formule van die cel leest. Klik nogmaals om één stap verder terug te gaan. **Weergeven welke cellen deze cel gebruiken** tekent pijlen naar de formules die de cel lezen; elke klik gaat één stap verder. Een kleine pagina staat voor een cel op een ander blad. Er worden hooguit 2.000 pijlen getekend, en de statusbalk meldt wanneer er meer waren. **Pijlen wissen** wist ze, en elke wijziging in de werkmap doet dat ook.
