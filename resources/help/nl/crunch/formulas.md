# Hoe formules worden weergegeven en berekend
<!-- keywords: fout traceren, berekenen voor opslaan, formule, formules, berekenen, berekening, opnieuw berekenen, functie, functies, waarde, resultaat, opgeslagen, F9, kringverwijzing, formules weergeven, formulebalk, fout, aanwijzen, wijzen, klikken op een cel, verwijzing, F4, foutcontrole, fout negeren, formule evalueren, stap, controlevenster, controleren, let, waarde een naam geven, celfunctie, infofunctie, filterxml, xml, xpath-->

Een **formule** is een cel die zijn waarde berekent uit andere cellen, zoals =SOM(B3:B8). De cel toont de waarde, en de **formulebalk** toont de formule.

## Wat JeffCrunch met formules doet

Wanneer u een werkmap opent, bevat het bestand de laatste waarde van elke formule, zodat het blad meteen kan worden getoond. Daarna berekent JeffCrunch elke formule opnieuw, een paar tegelijk, zodat het venster snel blijft. De statusbalk toont **Berekenen** met een percentage terwijl dit gebeurt. Wanneer het klaar is, bevatten de cellen de eigen resultaten van JeffCrunch. Om alles opnieuw te berekenen, klikt u op **Alles opnieuw berekenen** op het tabblad **Formules**, of drukt u op **F9**.

## Wanneer JeffCrunch een formule niet kan berekenen

JeffCrunch heeft meer dan 400 functies, en het aantal groeit nog. Een formule die een functie gebruikt die JeffCrunch nog niet heeft, behoudt de waarde die het bestand ervoor heeft opgeslagen. Dat geldt ook voor een formule die naar een andere werkmap verwijst. Cellen die daarvan afhankelijk zijn, worden berekend op basis van de opgeslagen waarden.

## Functies die een paar opmerkingen nodig hebben

- **Obligaties en leningen.** PRIJS, RENDEMENT, DUURZAAMHEID, ONEVENPRIJS en de andere obligatiefuncties tellen dagen op de manier die de **basis** aangeeft die u opgeeft (0 is 30 dagen per maand in een jaar van 360 dagen, 1 zijn de werkelijke dagen, enzovoort). RENDEMENT, ONEVENRENDEMENT, ONEVENLAATSTRENDEMENT en RENTE vinden hun antwoord door waarden te proberen totdat de prijs of betaling klopt. Ze stoppen wanneer het antwoord tot ongeveer 15 cijfers juist is, dus de laatste cijfers kunnen afwijken van die in een ander programma.
- **Verdelingen en toetsen.** De functies die eindigen op .VERD, .INV, .TOETS en de oudere namen zoals NORMVERD en TVERD, geven dezelfde antwoorden: een oudere naam blijft behouden zodat oude bestanden blijven werken. De *.INV-functies worden ook gevonden door waarden te proberen.
- **Lijsten die meerdere cellen vullen.** SORTEREN, SORTEREN.OP, FILTER, UNIEK, REEKS, WILLEKEURIGEMATRIX, XZOEKEN, MMULT, MINVERSE en LINEAIR geven een blok waarden. Voer ze in als matrixformule (**Ctrl+Shift+Enter**) over de cellen die ze moeten vullen. Het laten overlopen van een formule naar de cellen ernaast is nog niet mogelijk.
- **Teksten in andere schriften.** LENGTEB, LINKSB, DEELB, RECHTSB, VINDENB, ZOEKENB en VERVANGENB tellen elk teken als één, zoals op een computer die niet is ingesteld voor dubbelbytetekst. PHONETIC geeft de tekst van de cel, omdat JeffCrunch de lezingen van Japanse tekst niet bewaart.
- **CONVERTEREN.** Eenheden worden als tekst geschreven, met hoofd- en kleine letters zoals in de lijst van de functie: "kg", "lbm", "mi", "F". Een metrisch voorvoegsel kan voor een eenheid die dat accepteert, zoals "km" of "mL".

## MOD: de rest

**REST(n; d)** geeft wat er van **n** overblijft nadat zo veel mogelijk hele keren **d** is afgetrokken: het is n - d * INT(n / d). Voorbeeld: **=REST(10; 3)** is 1, en **=REST(-10; 3)** is 2, omdat INT(-10 / 3) gelijk is aan -4 en -10 - 3 * -4 gelijk is aan 2. Het antwoord heeft het teken van **d**. Als **d** 0 is, is het antwoord #DEEL/0!.

## Formules weergeven

Klik op **Formules weergeven** op het tabblad **Formules**, of druk op **Ctrl+`** (op een Mac de Control-toets, niet de Command-toets), om in plaats van de waarde de formule in elke cel te zien. De kolommen met formules worden breder gemaakt, tot een bepaalde grens, zodat de formules volledig zichtbaar zijn; ze krijgen weer hun oorspronkelijke breedte wanneer u dit uitschakelt, en de kolombreedten in uw bestand worden niet gewijzigd.

## Formules wijzigen

Om een formule te wijzigen, selecteert u de cel, drukt u op **F2**, past u de formule aan en drukt u op **Enter**. U kunt de formule ook in de formulebalk bewerken. Wanneer u een cel wijzigt, worden de formules die ervan afhankelijk zijn op de achtergrond opnieuw berekend. Zie [Typen in cellen](edit-cells).

## Een formule schrijven

Typ **=** en de eerste letters van een functie, dan toont een korte lijst de functies en [namen](names) die zo beginnen, met een zin over elk ervan. Druk op **Tab** om de gemarkeerde te kiezen. Gebruik de pijltjestoetsen **Omlaag** en **Omhoog** om een andere te markeren, en daarna **Tab** of **Enter**. **Esc** sluit de lijst. Zodra u binnen de haakjes van een functie staat, worden de argumenten onder de cel getoond, met het argument dat u typt in vet. De lijst en de argumenten zijn dezelfde als die **Functie kiezen** toont. Argumenten worden gescheiden door puntkomma's, of door komma's waar uw taal een komma als decimaalteken gebruikt.

## Naar cellen wijzen

In plaats van het adres van een cel te typen, kunt u ernaar wijzen. Terwijl u een formule typt, zet u de cursor op een plaats waar een cel kan staan (direct na **=**, een operator zoals **+**, een openingshaakje of een komma) en klikt u op de cel. Het adres wordt voor u ingevuld. Sleep over cellen om een blok zoals B2:D5 te schrijven. Klik opnieuw voordat u iets anders typt, dan vervangt de nieuwe cel de zojuist geschreven cel. Houd **Ctrl** ingedrukt en klik om na een komma nog een adres toe te voegen. Klik op een kolomletter om een hele kolom zoals A:A te schrijven, of op een rijnummer om een hele rij zoals 3:3 te schrijven, en sleep langs de letters of cijfers voor B:D of 2:5.

De pijltjestoetsen werken ook. Druk op een pijltoets en een aanwijzer beweegt weg van de cel waarin u typt, en het adres wordt geschreven; druk op meer pijltjes om verder te gaan, of houd **Shift** ingedrukt om de aanwijzer over een blok uit te rekken. De statusbalk toont **Wijzen** terwijl de pijltjestoetsen wijzen, **Invoeren** terwijl u typt, en **Bewerken** terwijl de pijltjestoetsen de cursor in de tekst verplaatsen. Druk op **F2** om te wisselen tussen wijzen en de tekst bewerken.

Om naar een cel op een ander blad te wijzen, klikt u op het tabblad van dat blad terwijl de cursor op een plaats staat waar een cel kan komen, en klikt u daarna op de cel. De naam van het blad wordt ermee geschreven, tussen aanhalingstekens wanneer dat nodig is, zoals in 'Mijn blad'!B3. **Enter** brengt u terug naar het blad waarop u begon en zet de formule in de cel. **Esc** annuleert alles, ook de wisseling van blad.

U kunt ook de rand van een omlijnd blok op het blad slepen om dat adres te verplaatsen, of het vierkantje in de hoek slepen om het uit te rekken; de formule verandert terwijl u sleept.

Druk op **F4** met de cursor in of direct na een adres om delen ervan met een dollarteken vast te zetten: A1 wordt $A$1, dan A$1, dan $A1, en dan weer A1.

Elk adres in de formule krijgt een kleur in de tekst, en de cellen waarnaar het verwijst worden op het blad in dezelfde kleur omlijnd, zodat u in één oogopslag ziet wat de formule leest. Een klik na een compleet adres, zoals =A1, beëindigt de invoer en selecteert de cel waarop u klikte, zoals bij elke andere tekst.

## Gegevenstabellen

Een gegevenstabel (wat-als-tabel) laat zien wat een formule oplevert voor veel waarden van een of twee cellen. Wanneer u een werkmap opent die er een bevat, berekent JeffCrunch de cellen ervan: voor een tabel met twee invoerwaarden staat de formule in de cel linksboven van de tabel, de waarden om te proberen in de rij staan in de invoercel voor de rij, en die in de kolom in de invoercel voor de kolom. Bij een tabel met één invoerwaarde staan de waarden onder elkaar in de kolom links ervan (en staan de formules in de rij erboven), of naast elkaar in de rij erboven (en staan de formules in de kolom links ervan). Elke cel van de tabel is het resultaat van de formule, en van elke formule die van de invoercel afhangt, met de invoer op die waarde; verder wordt niets in de werkmap aangeraakt. In de modus **Automatisch** worden de tabellen samen met de formules berekend, en de formules die de cellen van een tabel lezen, volgen mee. Het maken van een nieuwe gegevenstabel wordt nog niet aangeboden. Een tabel die niet kan worden berekend (een formule die van de invoer afhangt en een functie gebruikt die JeffCrunch niet heeft, of zichzelf wijzigt, zoals ASELECT of NU) behoudt de waarden die in het bestand staan.

## Met de hand berekenen

Klik op **Berekeningsmodus** op het tabblad **Formules** om te kiezen hoe de werkmap zijn formules berekent:

- **Automatisch** berekent ze terwijl u cellen wijzigt.
- **Automatisch, tabellen handmatig** doet hetzelfde, maar laat de cellen van gegevenstabellen zoals ze zijn totdat u op **F9** drukt.
- **Handmatig** berekent niets, ook geen gegevenstabellen, totdat u erom vraagt. De statusbalk toont **Verouderd** na een wijziging. Druk op **F9** (**Alles opnieuw berekenen**) om alles te berekenen, of op **Shift+F9** (**Dit blad opnieuw berekenen**) om de formules van het blad waarop u staat te berekenen, en wat er verder op wachtte. Een werkmap die op Handmatig staat, opent met de resultaten die het bestand bevat. **Berekenen voor opslaan**, ook in het menu **Berekeningsmodus**, staat in eerste instantie aan: wanneer u een werkmap op Handmatig opslaat die verouderd is, worden de formules eerst berekend, zodat het bestand actuele getallen bevat. Schakel het uit om de getallen op te slaan zoals ze zijn.

De keuze wordt bij de werkmap opgeslagen, en het is één stap die u ongedaan kunt maken.

## Formules die naar zichzelf verwijzen

Een formule die zijn eigen resultaat leest, rechtstreeks of via andere cellen, kan niet op de gebruikelijke manier worden berekend. JeffCrunch meldt dit één keer, noemt de cellen en omlijnt ze op het blad; ze behouden de waarden die ze hadden. Klik op **Kringverwijzingen** op het tabblad **Formules** voor de lijst, en dubbelklik op een cel in de lijst om er naartoe te gaan.

Sommige modellen hebben zo'n kring opzettelijk nodig. Kies **Kringformules iteratief berekenen** in het menu **Berekeningsmodus**, dan worden de formules steeds opnieuw berekend, waarbij elke formule de laatste waarden van de andere leest, totdat geen waarde meer dan het opgegeven bedrag verandert of het ingestelde aantal rondes is bereikt.

## Zien wat een formule leest

Selecteer een cel en klik op **Weergeven welke cellen deze cel beïnvloeden** om pijlen te tekenen vanaf de cellen die de formule leest. Klik opnieuw om nog één stap terug te gaan. **Weergeven welke cellen deze cel gebruiken** tekent pijlen naar de formules die de cel lezen; elke klik gaat één stap verder. Een klein blaadje staat voor een cel op een ander blad. Er worden maximaal 2.000 pijlen getekend, en de statusbalk meldt wanneer er meer waren. **Pijlen wissen** wist ze, en elke wijziging in de werkmap doet hetzelfde.

Wanneer een cel een fout toont, zoals #DEEL/0!, selecteert u die en klikt u op **Fout traceren**. Er worden pijlen getekend naar de cellen die het leest en die ook een fout tonen. Klik opnieuw om nog één stap terug te gaan. Wanneer geen enkele cel die het leest een fout toont, meldt de statusbalk dat deze cel de fout veroorzaakt, en dat is de cel die u moet repareren.

## Foutcontrole

Kies **Foutcontrole** in de groep **Controleren** van het tabblad Formules om het blad te doorzoeken op cellen die mogelijk fout zijn. Er wordt op twee dingen gelet: een formule die een fout oplevert (zoals #DEEL/0!), en een getal dat als tekst is opgeslagen, wat formules kunnen overslaan.

Het dialoogvenster toont één cel tegelijk, gaat naar die cel op het blad en zegt in één zin wat er aan de hand is. **Volgende** en **Vorige** bewegen door de cellen. Een klein groen driehoekje in de linkerbovenhoek van een cel markeert die cel op het blad. **Fout negeren** haalt de cel van de lijst, verwijdert het driehoekje en wordt in het bestand opgeslagen. Eén stap Ongedaan maken brengt het terug. Als er niets mis is, wordt u dat gemeld.

## Formule evalueren

Gebruik dit om te zien hoe een formule tot zijn waarde komt. Selecteer een cel met een formule en kies **Formule evalueren**. De formule wordt getoond met het volgende deel onderstreept. Druk op **Evalueren** om dat deel te berekenen; de waarde ervan neemt zijn plaats in. De delen gaan in deze volgorde: elke afzonderlijke cel van links, dan elke functie met haakjes, te beginnen bij de binnenste, en daarna wat overblijft. **Opnieuw** begint opnieuw. Als een deel niet kan worden berekend, zegt het dialoogvenster waarom en blijft de formule zoals hij was.

## Controlevenster

Het **Controlevenster** is een klein venster dat in beeld blijft terwijl u werkt. Het toont cellen die u in de gaten wilt houden, met hun blad, cel, waarde en formule, ook wanneer ze op een ander blad staan. Selecteer cellen en druk op **Controle toevoegen**. Selecteer rijen in de lijst en druk op **Controle verwijderen**. Dubbelklik op een rij om naar de cel te gaan. De waarden veranderen wanneer het blad opnieuw wordt berekend. De lijst blijft bewaard zolang de werkmap open is.

## Namen geven aan waarden in een formule

**LET** geeft een waarde in het midden van een formule een naam, zodat u die één keer schrijft en opnieuw gebruikt. De paren van een naam en een waarde komen eerst, en de berekening komt als laatste. In =LET(prijs; B2; btw; prijs*0,08; prijs+btw) staat de naam **prijs** voor de waarde in B2, wordt **btw** daaruit berekend, en telt het laatste deel ze op. Een naam kan de namen ervoor gebruiken. Een naam werkt alleen binnen zijn eigen LET, en een LET binnen een andere mag een naam opnieuw gebruiken. Een naam kan ook voor een celbereik staan, zoals in =LET(rijen; B2:B9; SOM(rijen)/AANTAL(rijen)).

## Vragen over een cel of de werkmap

**CEL** geeft informatie over een cel. Het eerste argument bepaalt wat: "adres", "rij", "kol", "inhoud", "type", "opmaak" (hoe getallen worden getoond, zoals "F2" voor twee decimalen, "P0" voor een percentage, of "D1" voor een datum), "kleur" (1 wanneer negatieve getallen in kleur worden getoond), "haakjes" (1 wanneer positieve getallen tussen haakjes worden getoond), "voorvoegsel" (het teken van een tekst die links, rechts of gecentreerd is uitgelijnd), "beveiligd" (1 wanneer de cel is vergrendeld), "breedte" (de breedte van de kolom in tekens) en "bestandsnaam" (de map, het bestand en het blad, zoals /home/ik/[Budget.xlsx]Blad1; leeg totdat de werkmap is opgeslagen). **INFO** geeft informatie over het programma en de werkmap: "directory", "numbestand" (hoeveel bladen er zichtbaar zijn), "oorsprong", "osversie", "herberekening" (Automatisch of Handmatig), "release" en "systeem".

## Waarden uit XML halen

**FILTERXML** neemt tekst die XML bevat en een XPath, een korte manier om aan te geven welke delen u wilt, en geeft de gevonden waarden terug. =FILTERXML(A1; "//item/price") geeft elke prijs in de tekst in A1, de ene onder de andere. Gebruik "/shop/item[2]/name" voor het tweede item, "//item[@id='b']/name" voor het item waarvan de id b is, of "count(//item)" voor het aantal items. Als de tekst geen geldige XML is, of het pad er geen is, is het antwoord #WAARDE!. Als niets overeenkomt, is het #N/B. XML met een documenttype wordt geweigerd.
