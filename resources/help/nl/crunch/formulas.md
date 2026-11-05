# Hoe formules worden weergegeven en berekend
<!-- keywords: formule, formules, berekenen, berekening, opnieuw berekenen, functie, functies, waarde, resultaat, opgeslagen, F9, kringverwijzing, formules weergeven, formulebalk, fout, punt, aanwijzen, klikken op een cel, verwijzing, F4, foutcontrole, fout negeren, formule evalueren, stap, controlevenster, controleren -->

Een **formule** is een cel die zijn waarde berekent uit andere cellen, zoals =SOM(B3:B8). De cel toont de waarde, en de **formulebalk** toont de formule.

## Wat JeffCrunch met formules doet

Wanneer u een werkmap opent, bevat het bestand de laatste waarde van elke formule, zodat het blad meteen kan worden getoond. Daarna berekent JeffCrunch elke formule opnieuw, een paar tegelijk, zodat het venster snel blijft. De statusbalk toont **Berekenen** met een percentage terwijl dit gebeurt. Wanneer het klaar is, bevatten de cellen de eigen resultaten van JeffCrunch. Wilt u alles opnieuw berekenen, klik dan op **Alles opnieuw berekenen** op het tabblad **Formules**, of druk op **F9**.

## Wanneer JeffCrunch een formule niet kan berekenen

JeffCrunch heeft meer dan 400 functies, en het aantal groeit nog. Een formule die een functie gebruikt die JeffCrunch nog niet heeft, behoudt de waarde die het bestand ervoor heeft opgeslagen. Dat geldt ook voor een formule die naar een andere werkmap verwijst. Cellen die daarvan afhankelijk zijn, worden berekend op basis van de opgeslagen waarden.

## Functies die een paar aanvullingen nodig hebben

- **Obligaties en leningen.** PRIJS, RENDEMENT, DUURZAAMHEID, ODDFPRIJS en de andere obligatiefuncties tellen dagen op de manier die de **basis** aangeeft die u opgeeft (0 is 30 dagen per maand in een jaar van 360 dagen, 1 zijn de werkelijke dagen, enzovoort). RENDEMENT, ODDFRENDEMENT, ODDLRENDEMENT en RENTE zoeken hun antwoord door waarden te proberen totdat de koers of de betaling klopt. Ze stoppen wanneer het antwoord tot op ongeveer 15 cijfers nauwkeurig is, dus de laatste cijfers kunnen afwijken van die in een ander programma.
- **Verdelingen en toetsen.** De functies die eindigen op .VERD, .INV, .TOETS, en de oudere namen zoals NORMVERD en TVERD, geven dezelfde antwoorden: een oudere naam blijft bewaard zodat oude bestanden blijven werken. Ook de *.INV-functies zoeken hun antwoord door waarden te proberen.
- **Lijsten die meerdere cellen vullen.** SORTEREN, SORTEREN.OP, FILTEREN, UNIEK, REEKS, WILLEKEURIGE.MATRIX, XZOEKEN, MMULT, MINVERSE en LINEAIR.VERBAND geven een blok waarden. Voer ze in als matrixformule (**Ctrl+Shift+Enter**) over de cellen die ze moeten vullen. Het laten overlopen van een formule naar de cellen ernaast is nog niet mogelijk.
- **Teksten in andere schriften.** LENGTEB, LINKSB, DEELB, RECHTSB, VINDENB, ZOEKENB en VERVANGENB tellen elk teken als één, zoals op een computer die niet is ingesteld voor dubbelbytetekst. FONETISCH geeft de tekst van de cel, omdat JeffCrunch de uitspraken van Japanse tekst niet bewaart.
- **CONVERTEREN.** Eenheden worden als tekst geschreven, met hoofdletters en kleine letters zoals in de lijst van de functie: "kg", "lbm", "mi", "F". Een metrisch voorvoegsel kan voor een eenheid die er een accepteert, zoals "km" of "mL".

## MOD: de rest

**REST(n; d)** geeft wat er van **n** overblijft nadat zoveel mogelijk hele keren **d** ervan is afgetrokken: het is n - d * INT(n / d). Voorbeeld: **=REST(10; 3)** is 1, en **=REST(-10; 3)** is 2, omdat INT(-10 / 3) gelijk is aan -4 en -10 - 3 * -4 gelijk is aan 2. Het antwoord heeft het teken van **d**. Als **d** 0 is, is het antwoord #DEEL/0!.

## Formules weergeven

Klik op **Formules weergeven** op het tabblad **Formules**, of druk op **Ctrl+`** (op een Mac op de Control-toets, niet op de Command-toets), om in elke cel de formule te zien in plaats van de waarde. De kolommen met formules worden breder gemaakt, tot een bepaalde grens, zodat de formules volledig zichtbaar zijn. Bij uitschakelen krijgen ze weer hun oorspronkelijke breedte, en de kolombreedten in uw bestand worden niet gewijzigd.

## Formules wijzigen

Om een formule te wijzigen, selecteert u de cel, drukt u op **F2**, past u de formule aan en drukt u op **Enter**. U kunt de formule ook in de formulebalk bewerken. Wanneer u een cel wijzigt, worden de formules die ervan afhankelijk zijn op de achtergrond opnieuw berekend. Zie [Cellen invullen](edit-cells).

## Een formule schrijven

Typ **=** en de eerste letters van een functie, dan verschijnt een korte lijst met de functies en [namen](names) die zo beginnen, met een zin over elke functie. Druk op **Tab** om de gemarkeerde te kiezen. Gebruik de pijltjestoetsen **Omlaag** en **Omhoog** om een andere te markeren, en dan **Tab** of **Enter**. **Esc** sluit de lijst. Zodra u binnen de haakjes van een functie bent, worden de argumenten onder de cel getoond, waarbij het argument dat u typt vetgedrukt is. De lijst en de argumenten zijn dezelfde als die **Functie kiezen** toont. Argumenten worden gescheiden door puntkomma's, of door komma's waar uw taal een komma als decimaalteken gebruikt.

## Naar cellen wijzen

In plaats van het adres van een cel te typen, kunt u ernaar wijzen. Terwijl u een formule typt, plaatst u de cursor waar een cel kan komen (direct na **=**, een operator zoals **+**, een openingshaak of een komma) en klikt u op de cel. Het adres wordt voor u geschreven. Sleep over cellen om een bereik zoals B2:D5 te schrijven. Klik opnieuw voordat u iets anders typt, dan vervangt de nieuwe cel de zojuist geschreven cel. Houd **Ctrl** ingedrukt en klik om na een komma nog een adres toe te voegen. Klik op een kolomletter om een hele kolom te schrijven, zoals A:A, of op een rijnummer voor een hele rij, zoals 3:3, en sleep langs de letters of cijfers voor B:D of 2:5.

De pijltjestoetsen werken ook. Druk op een pijltoets en een aanwijzer beweegt weg van de cel waarin u typt, en het adres wordt geschreven; druk op meer pijltjes om verder te gaan, of houd **Shift** ingedrukt om de aanwijzer over een blok uit te rekken. De statusbalk toont **Aanwijzen** terwijl de pijltjestoetsen aanwijzen, **Invoeren** terwijl u typt, en **Bewerken** terwijl de pijltjestoetsen de cursor in de tekst verplaatsen. Druk op **F2** om te wisselen tussen aanwijzen en de tekst bewerken.

Om naar een cel op een ander blad te wijzen, klikt u op het tabblad van dat blad terwijl de cursor op een plaats staat waar een cel kan komen, en klikt u daarna op de cel. De naam van het blad wordt erbij geschreven, tussen aanhalingstekens wanneer dat nodig is, zoals in 'Mijn blad'!B3. **Enter** brengt u terug naar het blad waarop u begon en zet de formule in de cel. **Esc** annuleert alles, ook het wisselen van blad.

U kunt ook de rand van een omlijnd blok op het blad slepen om dat adres te verplaatsen, of het vierkantje in de hoek slepen om het uit te rekken; de formule verandert terwijl u sleept.

Druk op **F4** met de cursor in of direct na een adres om delen ervan vast te zetten met een dollarteken: A1 wordt $A$1, dan A$1, dan $A1, en dan weer A1.

Elk adres in de formule krijgt een kleur in de tekst, en de cellen waarnaar het verwijst worden op het blad in dezelfde kleur omlijnd, zodat u in één oogopslag ziet wat de formule leest. Een klik na een volledig adres, zoals =A1, beëindigt de invoer en selecteert de cel waarop u klikte, zoals bij elke andere tekst.

## Gegevenstabellen

Een gegevenstabel (wat-als-tabel) toont wat een formule oplevert voor veel waarden van een of twee cellen. Open een werkmap die er een bevat, dan berekent JeffCrunch de cellen ervan: bij een tabel met twee invoerwaarden staat de formule in de cel linksboven in de tabel, de waarden die over de rij worden geprobeerd gaan in de invoercel voor de rij, en de waarden die omlaag worden geprobeerd gaan in de invoercel voor de kolom. Bij een tabel met één invoerwaarde staan de waarden omlaag in de kolom links ervan (en staan de formules in de rij erboven), of over de rij erboven (en staan de formules in de kolom links ervan). Elke cel van de tabel is het resultaat van de formule, en van elke formule die van de invoercel afhangt, met de invoer ingesteld op die waarde; de rest van de werkmap wordt niet aangepast. In de modus **Automatisch** worden de tabellen samen met de formules berekend, en de formules die de cellen van een tabel lezen, volgen. Het maken van een nieuwe gegevenstabel wordt nog niet aangeboden. Een tabel die niet kan worden berekend (een formule die van de invoer afhangt, gebruikt een functie die JeffCrunch niet heeft, of verandert uit zichzelf, zoals WILLEKEURIGE.GETAL of NU) behoudt de waarden die in het bestand staan.

## Met de hand berekenen

Klik op **Berekeningsmodus** op het tabblad **Formules** om te kiezen hoe de werkmap zijn formules berekent:

- **Automatisch** berekent ze terwijl u cellen wijzigt.
- **Automatisch, tabellen handmatig** doet hetzelfde, maar laat de cellen van gegevenstabellen zoals ze zijn totdat u op **F9** drukt.
- **Handmatig** berekent niets, ook geen gegevenstabellen, totdat u erom vraagt. De statusbalk toont **Niet bijgewerkt** na een wijziging. Druk op **F9** (**Alles opnieuw berekenen**) om alles te berekenen, of op **Shift+F9** (**Dit blad opnieuw berekenen**) om de formules van het blad waarop u staat te berekenen, samen met wat er verder wachtte. Een werkmap die op Handmatig staat, wordt geopend met de resultaten die het bestand bevat.

De keuze wordt bij de werkmap opgeslagen, en het is één stap die u ongedaan kunt maken.

## Formules die naar zichzelf verwijzen

Een formule die zijn eigen resultaat leest, rechtstreeks of via andere cellen, kan niet op de gebruikelijke manier worden berekend. JeffCrunch meldt dit één keer, noemt de cellen en omlijnt ze op het blad; ze behouden de waarden die ze hadden. Klik op **Kringverwijzingen** op het tabblad **Formules** voor de lijst, en dubbelklik op een cel in de lijst om er naartoe te gaan.

Sommige modellen hebben zo'n kring opzettelijk nodig. Kies **Kringformules iteratief berekenen** in het menu **Berekeningsmodus**, dan worden de formules steeds opnieuw berekend, waarbij elke formule de nieuwste waarden van de andere leest, totdat geen enkele waarde meer verandert dan het bedrag dat u opgeeft, of het aantal rondes dat u toestaat, is bereikt.

## Zien wat een formule leest

Selecteer een cel en klik op **Weergeven welke cellen deze cel beïnvloeden** om pijlen te tekenen vanaf de cellen die de formule van die cel leest. Klik nogmaals om een stap verder terug te gaan. **Weergeven welke cellen deze cel gebruiken** tekent pijlen naar de formules die de cel lezen, en elke klik gaat een stap verder. Een klein blaadje staat voor een cel op een ander blad. Er worden hoogstens 2.000 pijlen getekend, en de statusbalk meldt wanneer er meer waren. **Pijlen wissen** wist ze, en elke wijziging in de werkmap doet dat ook.

## Foutcontrole

Kies **Foutcontrole** in de groep **Controleren** van het tabblad Formules om het blad te doorzoeken op cellen die mogelijk verkeerd zijn. Er wordt op twee dingen gelet: een formule die een fout oplevert (zoals #DEEL/0!), en een getal dat als tekst is opgeslagen, wat formules kunnen overslaan.

Het dialoogvenster toont één cel tegelijk, gaat naar die cel op het blad en legt in één zin uit wat er aan de hand is. Met **Volgende** en **Vorige** bladert u door de cellen. Een klein groen driehoekje in de linkerbovenhoek van een cel markeert die cel op het blad. **Fout negeren** haalt de cel van de lijst, verwijdert het driehoekje en wordt in het bestand opgeslagen. Eén stap Ongedaan maken brengt de cel terug. Als er niets mis is, krijgt u dat te horen.

## Formule evalueren

Gebruik dit om te zien hoe een formule tot zijn waarde komt. Selecteer een cel met een formule en kies **Formule evalueren**. De formule wordt getoond met het volgende deel onderstreept. Druk op **Evalueren** om dat deel uit te rekenen; de waarde ervan neemt de plaats in. De delen komen in deze volgorde: elke afzonderlijke cel van links, dan elke functie met haar haakjes, te beginnen bij de binnenste, en dan wat overblijft. **Opnieuw starten** begint opnieuw. Als een deel niet kan worden berekend, meldt het dialoogvenster waarom en blijft de formule zoals hij was.

## Controlevenster

Het **Controlevenster** is een klein venster dat zichtbaar blijft terwijl u werkt. Het toont cellen die u in de gaten wilt houden, met hun blad, cel, waarde en formule, ook als ze op een ander blad staan. Selecteer cellen en druk op **Controle toevoegen**. Selecteer rijen in de lijst en druk op **Controle verwijderen**. Dubbelklik op een rij om naar de cel te gaan. De waarden veranderen wanneer het blad opnieuw wordt berekend. De lijst blijft bewaard zolang de werkmap open is.
