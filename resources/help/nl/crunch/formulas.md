# Hoe formules worden weergegeven en berekend
<!-- keywords: volgende fout, vorige fout, door fouten bladeren, fout traceren, berekenen voor opslaan, formule, formules, berekenen, berekening, opnieuw berekenen, functie, functies, waarde, resultaat, opgeslagen, F9, kring, kringverwijzing, formules weergeven, formulebalk, fout, aanwijzen, cel aanklikken, verwijzing, F4, foutcontrole, fout negeren, formule evalueren, stap, controlevenster, controle, LET, naam voor waarde, celfunctie, infofunctie, filterxml, xml, xpath-->

Een **formule** is een cel die zijn waarde berekent uit andere cellen, zoals =SOM(B3:B8). De cel toont de waarde, en de **formulebalk** toont de formule.

## Wat JeffCrunch met formules doet

Wanneer u een werkmap opent, bevat het bestand de laatste waarde van elke formule, zodat het blad meteen kan worden getoond. Daarna berekent JeffCrunch elke formule opnieuw, een paar tegelijk, zodat het venster snel blijft. De statusbalk toont **Berekenen** met een percentage zolang dat bezig is. Als het klaar is, bevatten de cellen de eigen resultaten van JeffCrunch. Om alles opnieuw te berekenen, klikt u op **Alles opnieuw berekenen** op het tabblad **Formules**, of drukt u op **F9**.

## Wanneer JeffCrunch een formule niet kan berekenen

JeffCrunch heeft meer dan 400 functies, en het aantal groeit nog. Een formule die een functie gebruikt die JeffCrunch nog niet heeft, behoudt de waarde die het bestand ervoor heeft opgeslagen. Dat geldt ook voor een formule die naar een andere werkmap verwijst. Cellen die van die formules afhangen, worden berekend op basis van de opgeslagen waarden.

## Functies die een paar toelichtingen nodig hebben

- **Obligaties en leningen.** PRIJS, RENDEMENT, DUURZAAMHEID, ONREGELM.PRIJS en de andere obligatiefuncties tellen dagen op de manier die de **basis** die u opgeeft aangeeft (0 is 30 dagen per maand in een jaar van 360 dagen, 1 zijn de werkelijke dagen, enzovoort). RENDEMENT, ONREGELM.RENDEMENT, ONREGELM.RENDEMENT en RENTE zoeken het antwoord door waarden te proberen totdat de prijs of betaling klopt. Ze stoppen wanneer het antwoord tot ongeveer 15 cijfers nauwkeurig is, dus de laatste cijfers kunnen afwijken van die in een ander programma.
- **Verdelingen en toetsen.** De functies die eindigen op .VERD, .INV, .TOETS en de oudere namen zoals NORMVERD en TVERD, geven dezelfde antwoorden: een oudere naam blijft bewaard zodat oude bestanden blijven werken. De *.INV-functies worden ook gevonden door waarden te proberen.
- **Lijsten die meerdere cellen vullen.** SORTEREN, SORTEREN.OP, FILTER, UNIEK, REEKS, WILLEKEURIGE.MATRIX, ZOEKEN.XL, MMULT, MINVERSE en LINEAIR geven een blok waarden. Voer ze in als matrixformule (**Ctrl+Shift+Enter**) over de cellen die ze moeten vullen. Het laten overlopen van een formule naar de cellen ernaast wordt nog niet ondersteund.
- **Teksten in andere schriften.** LENGTEB, LINKSB, DEELTEKSTB, RECHTSB, VINDENB, ZOEKENB en VERVANGENB tellen elk teken als één, zoals op een computer die niet is ingesteld op dubbelbyte-tekst. FONETISCH geeft de tekst van de cel, omdat JeffCrunch de lezing van Japanse tekst niet bewaart.
- **CONVERTEREN.** Eenheden worden als tekst geschreven, met hoofd- en kleine letters zoals in de lijst van de functie: "kg", "lbm", "mi", "F". Een metrisch voorvoegsel kan voor een eenheid die dat accepteert, zoals "km" of "mL".

## MOD: de rest

**REST(n; d)** geeft wat er overblijft van **n** nadat het grootste aantal hele keren **d** eraf is gehaald: het is n - d * GEHEEL(n / d). Voorbeeld: **=REST(10; 3)** is 1, en **=REST(-10; 3)** is 2, omdat GEHEEL(-10 / 3) gelijk is aan -4 en -10 - 3 * -4 gelijk is aan 2. Het antwoord heeft het teken van **d**. Als **d** 0 is, is het antwoord #DEEL/0!.

## Formules weergeven

Klik op **Formules weergeven** op het tabblad **Formules**, of druk op **Ctrl+`** (op een Mac de Control-toets, niet de Command-toets), om in elke cel de formule te zien in plaats van de waarde. De kolommen met formules worden breder, tot een bepaalde grens, zodat de formules volledig zichtbaar zijn. Ze krijgen hun oorspronkelijke breedte terug wanneer u dit uitschakelt, en de kolombreedten in uw bestand worden niet gewijzigd.

## Formules wijzigen

Om een formule te wijzigen, selecteert u de cel, drukt u op **F2**, past u de formule aan en drukt u op **Enter**. U kunt de formule ook in de formulebalk bewerken. Wanneer u een cel wijzigt, worden de formules die ervan afhangen op de achtergrond opnieuw berekend. Zie [Typen in cellen](edit-cells).

## Een formule schrijven

Typ **=** en de eerste letters van een functie. Er verschijnt een korte lijst met de functies en [namen](names) die met die letters beginnen, met een zin over elke functie. Druk op **Tab** om de gemarkeerde functie te kiezen. Gebruik de pijltjes **Omlaag** en **Omhoog** om een andere te markeren, en druk daarna op **Tab** of **Enter**. **Esc** sluit de lijst. Zodra u binnen de haakjes van een functie staat, worden de argumenten onder de cel getoond, met het argument dat u typt in vet. De lijst en de argumenten zijn dezelfde als die **Een functie kiezen** toont. Argumenten worden gescheiden door komma's, of door puntkomma's wanneer uw taal een komma gebruikt als decimaalteken.

## Naar cellen wijzen

In plaats van het adres van een cel te typen, kunt u ernaar wijzen. Terwijl u een formule typt, plaatst u de cursor waar een cel kan komen (direct na **=**, na een operator zoals **+**, na een openende haak of na een komma) en klikt u op de cel. Het adres wordt voor u ingevuld. Sleep over cellen om een blok te schrijven, zoals B2:D5. Klik opnieuw voordat u iets anders typt, dan vervangt de nieuwe cel de zojuist geschreven cel. Houd **Ctrl** ingedrukt en klik om na een komma nog een adres toe te voegen. Klik op een kolomletter om een hele kolom te schrijven, zoals A:A, of op een rijnummer om een hele rij te schrijven, zoals 3:3. Sleep langs de letters of cijfers voor B:D of 2:5.

De pijltjestoetsen werken ook. Drukt u op een pijltoets, dan verplaatst een aanwijzer zich weg van de cel waarin u typt, en wordt het adres geschreven. Druk op meer pijltjes om verder te gaan, of houd **Shift** ingedrukt om de aanwijzer over een blok uit te rekken. De statusbalk toont **Aanwijzen** terwijl de pijltjestoetsen aanwijzen, **Invoeren** terwijl u typt, en **Bewerken** terwijl de pijltjestoetsen de cursor in de tekst verplaatsen. Druk op **F2** om te wisselen tussen aanwijzen en de tekst bewerken.

Om naar een cel op een ander blad te wijzen, klikt u op het tabblad van dat blad terwijl de cursor staat waar een cel kan komen, en daarna klikt u op de cel. De naam van het blad wordt meegeschreven, tussen aanhalingstekens wanneer dat nodig is, zoals in 'Mijn blad'!B3. **Enter** brengt u terug naar het blad waarop u begon en zet de formule in de cel. **Esc** annuleert alles, ook de wisseling van blad.

U kunt ook de rand van een omlijnd blok op het blad slepen om dat adres te verplaatsen, of het vierkantje in de hoek slepen om het uit te rekken; de formule verandert terwijl u sleept.

Druk op **F4** met de cursor in of direct na een adres om delen ervan vast te zetten met een dollarteken: A1 wordt $A$1, dan A$1, dan $A1, en dan weer A1.

Elk adres in de formule krijgt een kleur in de tekst, en de cellen waarnaar het verwijst worden met dezelfde kleur omlijnd op het blad. Zo ziet u in één oogopslag wat de formule leest. Een klik na een compleet adres, zoals =A1, beëindigt de invoer en selecteert de cel waarop u klikte, zoals bij elke andere tekst.

## Gegevenstabellen

Een gegevenstabel (wat-als-tabel) toont wat een formule oplevert voor veel waarden van één of twee cellen. Wanneer u een werkmap opent die er een bevat, berekent JeffCrunch de cellen ervan: voor een tabel met twee invoerwaarden staat de formule in de cel linksboven in de tabel, de waarden die u horizontaal wilt proberen gaan in de invoercel voor de rij, en de waarden die verticaal staan gaan in de invoercel voor de kolom. Bij een tabel met één invoerwaarde staan de waarden onder elkaar in de kolom links ervan (en staan de formules in de rij erboven), of naast elkaar in de rij erboven (en staan de formules in de kolom links ervan). Elke cel van de tabel is het resultaat van de formule, en van elke formule die van de invoercel afhangt, met de invoer ingesteld op die waarde. Verder wordt niets in de werkmap aangeraakt. In de modus **Automatisch** worden de tabellen samen met de formules berekend, en de formules die de cellen van een tabel lezen, volgen mee. Het maken van een nieuwe gegevenstabel wordt nog niet aangeboden. Een tabel die niet berekend kan worden (een formule die van de invoer afhangt gebruikt een functie die JeffCrunch niet heeft, of die zichzelf wijzigt, zoals ASELECT of NU), behoudt de waarden die in het bestand staan.

## Met de hand berekenen

Klik op **Berekeningsmodus** op het tabblad **Formules** om te kiezen hoe de werkmap zijn formules berekent:

- **Automatisch** berekent ze terwijl u cellen wijzigt.
- **Automatisch, behalve tabellen** doet hetzelfde, maar laat de cellen van gegevenstabellen met rust tot u op **F9** drukt.
- **Handmatig** berekent niets, ook geen gegevenstabellen, totdat u erom vraagt. De statusbalk toont **Verouderd** na een wijziging. Druk op **F9** (**Alles opnieuw berekenen**) om alles te berekenen, of op **Shift+F9** (**Dit blad opnieuw berekenen**) om de formules van het blad waarop u staat te berekenen, samen met alles wat nog wachtte. Een werkmap die op Handmatig staat, wordt geopend met de resultaten die het bestand bevat. **Berekenen voor opslaan**, ook in het menu **Berekeningsmodus**, staat standaard aan: wanneer u een werkmap in de modus Handmatig opslaat die niet meer up-to-date is, worden de formules eerst berekend, zodat het bestand actuele getallen bevat. Schakel dit uit om de getallen op te slaan zoals ze zijn.

De keuze wordt bewaard in de werkmap, en het is één stap die u ongedaan kunt maken.

## Formules die naar zichzelf verwijzen

Een formule die direct of via andere cellen naar zijn eigen resultaat verwijst, kan niet op de gewone manier worden berekend. JeffCrunch meldt dit één keer, noemt de cellen en omlijnt ze op het blad. Ze behouden de waarden die ze hadden. Klik op **Kringverwijzingen** op het tabblad **Formules** voor de lijst, en dubbelklik op een cel in de lijst om daar naartoe te gaan.

Sommige modellen hebben zo'n kring bewust nodig. Kies **Kringformules iteratief berekenen** in het menu **Berekeningsmodus**, dan worden de formules steeds opnieuw berekend, waarbij elke formule de laatste waarden van de andere leest, totdat geen waarde meer verandert met meer dan het bedrag dat u opgeeft, of totdat het aantal rondes dat u toestaat is bereikt.

## Zien wat een formule leest

Selecteer een cel en klik op **Weergeven welke cellen deze cel beïnvloeden** om pijlen te tekenen vanaf de cellen die de formule van die cel leest naar die cel. Klik nogmaals om één stap verder terug te gaan. **Weergeven welke cellen deze cel gebruiken** tekent pijlen vanaf de cel naar de formules die de cel lezen; elke klik gaat één stap verder. Een klein blaadje staat voor een cel op een ander blad. Er worden hoogstens 2.000 pijlen getekend, en de statusbalk meldt wanneer er meer waren. **Pijlen wissen** verwijdert ze, en elke wijziging in de werkmap doet dat ook.

Wanneer een cel een fout toont, zoals #DEEL/0!, selecteert u die cel en klikt u op **Fout traceren**. Er worden pijlen getekend naar de cellen die ze leest en die ook een fout tonen. Klik nogmaals om één stap verder terug te gaan. Wanneer geen van de cellen die ze leest een fout toont, meldt de statusbalk dat deze cel de fout veroorzaakt, en dat is de cel die u moet herstellen.

Om alle fouten op het blad langs te gaan, klikt u op **Volgende fout** of **Vorige fout**. De cellen die een fout tonen worden rij voor rij bezocht, en na de laatste begint het weer bij de eerste. Elke cel wordt geselecteerd, en de pijlen worden helemaal getekend tot de cel die de fout veroorzaakt. De statusbalk toont om welke fout het gaat en hoeveel er zijn.

## Foutcontrole

Kies **Foutcontrole** in de groep **Controleren** op het tabblad Formules om het blad te doorzoeken op cellen die mogelijk fout zijn. Er worden twee dingen gezocht: een formule die een fout oplevert (zoals #DEEL/0!), en een getal dat als tekst is opgeslagen, wat formules mogelijk overslaan.

Het dialoogvenster toont één cel per keer, gaat naar die cel op het blad en vertelt in een zin wat er aan de hand is. Met **Volgende** en **Vorige** gaat u door de cellen. Een klein groen driehoekje in de linkerbovenhoek van een cel markeert die cel op het blad. **Fout negeren** haalt de cel van de lijst, verwijdert het driehoekje en wordt in het bestand opgeslagen. Eén stap Ongedaan maken brengt het terug. Als er niets mis is, krijgt u dat te horen.

## Formule evalueren

Gebruik dit om te zien hoe een formule tot zijn waarde komt. Selecteer een cel met een formule en kies **Formule evalueren**. De formule wordt getoond met het volgende deel onderstreept. Druk op **Evalueren** om dat deel te berekenen; de waarde ervan neemt de plaats in. De delen komen in deze volgorde: elke afzonderlijke cel van links, dan elke functie met haakjes, te beginnen bij de binnenste, en daarna wat er overblijft. **Opnieuw starten** begint opnieuw. Als een deel niet berekend kan worden, vertelt het dialoogvenster waarom en blijft de formule zoals hij was.

## Controlevenster

Het **Controlevenster** is een klein venster dat zichtbaar blijft terwijl u werkt. Het toont cellen die u in de gaten wilt houden, met hun blad, cel, waarde en formule, ook wanneer ze op een ander blad staan. Selecteer cellen en druk op **Controle toevoegen**. Selecteer rijen in de lijst en druk op **Controle verwijderen**. Dubbelklik op een rij om naar de cel te gaan. De waarden veranderen wanneer het blad opnieuw wordt berekend. De lijst blijft bewaard zolang de werkmap open is.

## Namen geven aan waarden binnen een formule

**LET** geeft een naam aan een waarde in het midden van een formule, zodat u die één keer schrijft en opnieuw kunt gebruiken. De paren van een naam en een waarde komen eerst, en de berekening komt als laatste. In =LET(prijs; B2; btw; prijs*0,08; prijs+btw) staat de naam **prijs** voor de waarde in B2, wordt **btw** daaruit berekend, en telt het laatste deel ze op. Een naam kan de namen ervoor gebruiken. Een naam werkt alleen binnen de eigen LET, en een LET binnen een andere mag een naam opnieuw gebruiken. Een naam kan ook een blok cellen voorstellen, zoals in =LET(rijen; B2:B9; SOM(rijen)/AANTAL(rijen)).

## Informatie vragen over een cel of de werkmap

**CEL** geeft informatie over een cel. Het eerste argument bepaalt welke: "adres", "rij", "kolom", "inhoud", "type", "opmaak" (hoe getallen worden getoond, zoals "F2" voor twee decimalen, "P0" voor een percentage, of "D1" voor een datum), "kleur" (1 wanneer negatieve getallen in kleur worden getoond), "haakjes" (1 wanneer positieve getallen tussen haakjes worden getoond), "voorvoegsel" (het teken van een tekst die links, rechts of gecentreerd is uitgelijnd), "beveiligd" (1 wanneer de cel is vergrendeld), "breedte" (de breedte van de kolom in tekens) en "bestandsnaam" (de map, het bestand en het blad, zoals /home/me/[Budget.xlsx]Blad1; leeg totdat de werkmap is opgeslagen). **INFO** geeft informatie over het programma en de werkmap: "directory", "numfile" (hoeveel bladen worden getoond), "origin", "osversion", "recalc" (Automatisch of Handmatig), "release" en "system".

## Waarden uit XML halen

**FILTERXML** neemt tekst die XML bevat en een XPath, een korte manier om aan te geven welke delen u wilt, en geeft de waarden die het vindt. =FILTERXML(A1; "//item/price") geeft elke prijs in de tekst in A1, onder elkaar. Gebruik "/shop/item[2]/name" voor het tweede item, "//item[@id='b']/name" voor het item met id b, of "count(//item)" voor het aantal items. Als de tekst geen geldige XML is, of het pad geen geldig pad is, is het antwoord #WAARDE!. Als niets overeenkomt, is het #N/B. XML met een documenttype wordt geweigerd.
