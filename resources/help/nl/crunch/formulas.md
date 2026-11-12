# Hoe formules worden getoond en berekend
<!-- keywords: overloop, overgelopen, #SPILL!, dynamische matrix, overloopbereik, A1#, volgende fout, vorige fout, door fouten bladeren, fout traceren, berekenen voor opslaan, formule, formules, berekenen, berekening, opnieuw berekenen, functie, functies, waarde, resultaat, opgeslagen, F9, circulair, kringverwijzing, formules weergeven, formulebalk, fout, wijzen, aanwijzen, klik op een cel, verwijzing, F4, foutcontrole, fout negeren, formule evalueren, stap, controlevenster, controleren, laten, waarde een naam geven, celfunctie, infofunctie, filterxml, xml, xpath, lus, kringformule, iteratie -->

Een **formule** is een cel die zijn waarde berekent uit andere cellen, zoals =SOM(B3:B8). De cel toont de waarde, en de **formulebalk** toont de formule.

## Wat JeffCrunch met formules doet

Wanneer u een werkmap opent, bevat het bestand de laatste waarde van elke formule, zodat het blad meteen kan worden getoond. Daarna berekent JeffCrunch elke formule opnieuw, een paar tegelijk, zodat het venster snel blijft. De statusbalk toont **Berekenen** met een percentage zolang dat bezig is. Wanneer het klaar is, bevatten de cellen de eigen resultaten van JeffCrunch. Om alles opnieuw te berekenen, klikt u op **Alles opnieuw berekenen** op het tabblad **Formules**, of drukt u op **F9**.

## Wanneer JeffCrunch een formule niet kan berekenen

JeffCrunch heeft meer dan 400 functies, en het aantal groeit nog. Een formule die een functie gebruikt die JeffCrunch nog niet heeft, behoudt de waarde die het bestand ervoor heeft opgeslagen. Dat geldt ook voor een formule die naar een andere werkmap verwijst. Cellen die daarvan afhangen, worden berekend op basis van de opgeslagen waarden.

## Functies die een paar toelichtingen nodig hebben

- **Obligaties en leningen.** PRIJS, RENDEMENT, DUUR, ODDFPRIJS en de andere obligatiefuncties tellen dagen volgens de **basis** die u opgeeft (0 is 30 dagen per maand in een jaar van 360 dagen, 1 zijn de werkelijke dagen, enzovoort). RENDEMENT, ODDFRENDEMENT, ODDLRENDEMENT en RENTE zoeken hun antwoord door waarden te proberen totdat de koers of de betaling past. Ze stoppen wanneer het antwoord tot op ongeveer 15 cijfers klopt, dus de laatste cijfers kunnen afwijken van die in een ander programma.
- **Verdelingen en toetsen.** De functies die eindigen op .VERD, .INV, .TEST, en de oudere namen zoals NORMVERD en TVERD, geven dezelfde antwoorden: een oudere naam blijft bewaard zodat oude bestanden blijven werken. De *.INV-functies worden ook gevonden door waarden te proberen.
- **Lijsten die meerdere cellen vullen.** SORTEREN, SORTEREN.OP, FILTER, UNIEK, REEKS, MATRIJSWILLEKEUR, XLOOKUP, MMULT, INVERSE.MATRIX en LINEAIR.REGRESSIE geven een blok waarden. Typt u er een in één cel, dan loopt het blok over in de cellen eronder en ernaast (zie **Formules die de cellen ernaast vullen**), of voert u het in als matrixformule (**Ctrl+Shift+Enter**) over de cellen die het moet vullen.
- **Teksten in andere schriften.** LENGTEB, LINKSB, DEELB, RECHTSB, VINDENB, ZOEKENB en VERVANGENB tellen elk teken als één, zoals op een computer die niet is ingesteld op tweebytetekst. TONEN geeft de tekst van de cel, omdat JeffCrunch de uitspraak van Japanse tekst niet bewaart.
- **CONVERTEREN.** Eenheden worden als tekst geschreven, met hoofdletters en kleine letters zoals in de lijst van de functie: "kg", "lbm", "mi", "F". Een metrisch voorvoegsel kan voor een eenheid die het aanneemt, zoals "km" of "mL".

## MOD: de rest

**REST(n; d)** geeft wat er overblijft van **n** nadat zo veel mogelijk hele keren **d** is afgetrokken: het is n - d * INT(n / d). Voorbeeld: **=REST(10; 3)** is 1, en **=REST(-10; 3)** is 2, omdat INT(-10 / 3) gelijk is aan -4 en -10 - 3 * -4 gelijk is aan 2. Het antwoord heeft het teken van **d**. Als **d** 0 is, is het antwoord #DEEL/0!.

## Formules weergeven

Klik op **Formules weergeven** op het tabblad **Formules**, of druk op **Ctrl+`** (op een Mac de Control-toets, niet de Command-toets), om in plaats van de waarde de formule in elke cel te zien. De kolommen met formules worden breder gemaakt, tot een bepaalde grens, zodat de formules volledig zichtbaar zijn. Ze krijgen hun oorspronkelijke breedte terug wanneer u dit uitschakelt, en de kolombreedten in uw bestand worden niet gewijzigd.

## Formules wijzigen

Om een formule te wijzigen, selecteert u de cel, drukt u op **F2**, bewerkt u de formule en drukt u op **Enter**. U kunt de formule ook in de formulebalk bewerken. Wanneer u een cel wijzigt, worden de formules die ervan afhangen op de achtergrond opnieuw berekend. Zie [Cellen invoeren](edit-cells).

## Een formule schrijven

Typ **=** en de eerste letters van een functie, dan toont een korte lijst de functies en [namen](names) die zo beginnen, met een zin over elk. Druk op **Tab** om de gemarkeerde te kiezen. Met de pijltjes **Omlaag** en **Omhoog** markeert u een andere, en daarna drukt u op **Tab** of **Enter**. **Esc** sluit de lijst. Zodra u binnen de haakjes van een functie staat, worden de argumenten onder de cel getoond, waarbij het argument dat u typt vetgedrukt is. De lijst en de argumenten zijn dezelfde als die **Functie kiezen** toont. Argumenten worden gescheiden door puntkomma's, of door komma's waar uw taal een komma als decimaalteken gebruikt.

## Cellen aanwijzen

In plaats van het adres van een cel te typen, kunt u de cel aanwijzen. Terwijl u een formule typt, zet u de cursor waar een cel kan komen (direct na **=**, een operator zoals **+**, een openingshaakje of een komma) en klikt u op de cel. Het adres wordt voor u ingevuld. Sleep over cellen om een bereik zoals B2:D5 te schrijven. Klik opnieuw voordat u iets anders typt, dan vervangt de nieuwe cel die net is geschreven. Houd **Ctrl** ingedrukt en klik om na een komma een ander adres toe te voegen. Klik op een kolomletter om een hele kolom te schrijven, zoals A:A, of op een rijnummer om een hele rij te schrijven, zoals 3:3, en sleep langs de letters of cijfers voor B:D of 2:5.

De pijltoetsen werken ook. Druk op een pijltoets en een aanwijzer verplaatst zich weg van de cel waarin u typt, en het adres wordt geschreven; druk op meer pijltoetsen om verder te gaan, of houd **Shift** ingedrukt om de aanwijzer over een blok uit te rekken. De statusbalk toont **Aanwijzen** terwijl de pijltoetsen aanwijzen, **Enter** terwijl u typt, en **Bewerken** terwijl de pijltoetsen de cursor in de tekst verplaatsen. Druk op **F2** om te wisselen tussen aanwijzen en de tekst bewerken.

Om een cel op een ander blad aan te wijzen, klikt u op het tabblad van dat blad terwijl de cursor op een plek staat waar een cel kan komen, en klikt u daarna op de cel. De naam van het blad wordt ermee geschreven, tussen aanhalingstekens wanneer dat nodig is, zoals in 'Mijn blad'!B3. **Enter** brengt u terug naar het blad waarop u begon en zet de formule in de cel. **Esc** annuleert alles, ook de wisseling van blad.

U kunt ook de rand van een omlijnd blok op het blad slepen om dat adres te verplaatsen, of het vierkantje in de hoek slepen om het uit te rekken; de formule verandert terwijl u sleept.

Druk op **F4** met de cursor in of direct na een adres om delen ervan vast te zetten met een dollarteken: A1 wordt $A$1, dan A$1, dan $A1, en dan weer A1.

Elk adres in de formule krijgt een kleur in de tekst, en de cellen waarnaar het verwijst, worden in dezelfde kleur omlijnd op het blad, zodat u in één oogopslag ziet wat de formule leest. Een klik na een compleet adres, zoals =A1, beëindigt de invoer en selecteert de cel waarop u klikt, zoals bij elke andere tekst.

## Gegevenstabellen

Een gegevenstabel (wat-als-tabel) toont wat een formule oplevert voor veel waarden van een of twee cellen. Opent u een werkmap die er een bevat, dan berekent JeffCrunch de cellen ervan: bij een tabel met twee invoerwaarden staat de formule in de cel linksboven van de tabel, de waarden die horizontaal worden geprobeerd gaan in de invoercel voor de rij, en die verticaal gaan in de invoercel voor de kolom. Bij een tabel met één invoerwaarde staan de waarden onder elkaar in de kolom links ervan (en staan de formules in de rij erboven), of naast elkaar in de rij erboven (en staan de formules in de kolom links ervan). Elke cel van de tabel is het resultaat van de formule, en van elke formule die van de invoercel afhangt, met de invoer op die waarde; de rest van de werkmap blijft ongewijzigd. In de modus **Automatisch** worden de tabellen samen met de formules berekend, en de formules die de cellen van een tabel lezen, volgen daarop. Het maken van een nieuwe gegevenstabel wordt nog niet aangeboden. Een tabel die niet kan worden berekend (een formule die van de invoer afhangt en een functie gebruikt die JeffCrunch niet heeft, of zichzelf wijzigt, zoals ASELECT of NU) behoudt de waarden die in het bestand staan.

## Met de hand berekenen

Klik op **Berekeningsmodus** op het tabblad **Formules** om te kiezen hoe de werkmap de formules berekent:

- **Automatisch** berekent ze wanneer u cellen wijzigt.
- **Automatisch, tabellen handmatig** doet hetzelfde, maar laat de cellen van gegevenstabellen staan zoals ze zijn tot u op **F9** drukt.
- **Handmatig** berekent niets, gegevenstabellen inbegrepen, totdat u erom vraagt. De statusbalk toont **Verouderd** na een wijziging. Druk op **F9** (**Alles opnieuw berekenen**) om alles te berekenen, of op **Shift+F9** (**Dit blad opnieuw berekenen**) om de formules van het blad waarop u staat te berekenen, en wat er verder op wachtte. Een werkmap die op Handmatig is ingesteld, opent met de resultaten die het bestand bevat. **Berekenen voor opslaan**, ook in het menu **Berekeningsmodus**, staat in eerste instantie aan: wanneer u een handmatige werkmap opslaat die verouderd is, worden de formules eerst berekend, zodat het bestand actuele getallen bevat. Schakel dit uit om de getallen op te slaan zoals ze zijn.

De keuze wordt met de werkmap opgeslagen, en het is één stap die u ongedaan kunt maken.

## Formules die naar zichzelf verwijzen

Een formule die zijn eigen resultaat leest, direct of via andere cellen, kan niet op de gebruikelijke manier worden berekend. JeffCrunch meldt dat één keer, noemt de cellen en omlijnt ze op het blad; ze behouden de waarden die ze hadden. Klik op **Kringverwijzingen** op het tabblad **Formules** voor de lijst, en dubbelklik op een cel in de lijst om daarheen te gaan.

Sommige modellen hebben zo'n kring opzettelijk nodig. Kies **Kringformules iteratief berekenen** in het menu **Berekeningsmodus**, dan worden de formules steeds opnieuw berekend, elk met de nieuwste waarden van de andere, totdat geen waarde nog met meer verandert dan het bedrag dat u opgeeft, of het aantal rondes dat u toestaat, is bereikt.

## Zien wat een formule leest

Selecteer een cel en klik op **Toon wat deze cel voedt** om pijlen te tekenen naar de cellen die de formule leest. Klik nogmaals om een stap verder terug te gaan. **Toon wat deze cel gebruikt** tekent pijlen naar de formules die de cel lezen, en elke klik gaat een stap verder. Een klein blad staat voor een cel op een ander blad. Er worden ten hoogste 2.000 pijlen getekend; de statusbalk meldt wanneer het er meer waren. **Pijlen wissen** wist ze, en elke wijziging in de werkmap doet dat ook.

Wanneer een cel een fout toont, zoals #DEEL/0!, selecteert u die en klikt u op **Fout traceren**. Er worden pijlen getekend naar de cellen die het leest en die ook een fout tonen. Klik nogmaals om een stap verder terug te gaan. Wanneer geen enkele cel die hij leest een fout toont, meldt de statusbalk dat deze cel de fout veroorzaakt, en dat is de cel die u moet herstellen.

Om elke fout op het blad langs te gaan, klikt u op **Volgende fout** of **Vorige fout**. De cellen die een fout tonen, worden rij na rij bezocht, en na de laatste begint het weer bij de eerste. Elke cel wordt geselecteerd, en de pijlen worden helemaal getekend tot de cel die de fout veroorzaakt. De statusbalk zegt welke fout u bekijkt, en hoeveel er zijn.

## Foutcontrole

Kies **Foutcontrole** in de groep **Controleren** op het tabblad Formules om het blad na te lopen op cellen die mogelijk fout zijn. Het zoekt naar twee dingen: een formule die tot een fout leidt (zoals #DEEL/0!), en een getal dat als tekst is opgeslagen, wat formules kunnen overslaan.

Het dialoogvenster toont één cel tegelijk, gaat naar die cel op het blad en zegt in één zin wat er aan de hand is. Met **Volgende** en **Vorige** gaat u door de cellen. Een klein groen driehoekje in de linkerbovenhoek van een cel markeert die cel op het blad. **Fout negeren** haalt de cel van de lijst, verwijdert het driehoekje en wordt in het bestand opgeslagen. Eén stap Ongedaan maken brengt de cel terug. Is er niets mis, dan krijgt u dat te horen.

## Formule evalueren

Gebruik dit om te zien hoe een formule tot zijn waarde komt. Selecteer een cel met een formule en kies **Formule evalueren**. De formule wordt getoond met het volgende deel onderstreept. Druk op **Evalueren** om dat deel te berekenen; de waarde ervan neemt zijn plaats in. De delen komen in deze volgorde: elke losse cel van links, dan elke functie met haar haakjes, beginnend bij de binnenste, en dan wat er overblijft. **Opnieuw** begint opnieuw. Als een deel niet kan worden berekend, zegt het dialoogvenster waarom en blijft de formule zoals hij was.

## Controlevenster

Het **Controlevenster** is een klein venster dat in beeld blijft terwijl u werkt. Het toont cellen die u in de gaten wilt houden, met hun blad, cel, waarde en formule, ook wanneer ze op een ander blad staan. Selecteer cellen en druk op **Controle toevoegen**. Selecteer rijen in de lijst en druk op **Controle verwijderen**. Dubbelklik op een rij om naar de cel te gaan. De waarden veranderen wanneer het blad opnieuw wordt berekend. De lijst blijft bewaard zolang de werkmap open is.

## Formules die de cellen ernaast vullen

Wanneer een formule een blok waarden geeft, vult het blok de cellen onder en naast de cel waarin u de formule typte. Dit heet overloop. Probeer =REEKS(3) in A1: A1 toont 1, en A2 en A3 tonen 2 en 3. Een formule loopt over wanneer ze een functie aanroept die een blok geeft (SORTEREN, SORTEREN.OP, FILTER, UNIEK, REEKS, MATRIJSWILLEKEUR, TRANSPONEREN, XLOOKUP of MMULT), een blok tussen accolades schrijft, een overloopbereik gebruikt (hieronder), of een rekenkundige bewerking op een bereik uitvoert, zoals in =A1:A3*2.

- **De cellen die het vult, bevatten geen eigen formule.** Ze tonen de waarden. Wanneer u er een selecteert, toont de formulebalk in grijs de formule van de cel die hem vult. Als u erop Enter drukt zonder iets te wijzigen, verandert er niets. Om het blok te wijzigen, wijzigt u eerst die eerste cel.
- **Het blok volgt het resultaat.** Als de formule na een wijziging in wat hij leest meer of minder waarden geeft, wordt het blok groter of kleiner, en formules die die cellen lezen, worden opnieuw berekend.
- **Een cel in de weg geeft #SPILL!.** Als een cel waar het blok zou komen een waarde of een formule bevat, toont de eerste cel **#SPILL!** en wordt niets ingevuld. Wis de cel in de weg en het blok vult zich meteen. Typen in een cel die het blok vult, heeft hetzelfde effect: de eerste cel toont #SPILL!.
- **Een blok heeft een maximale grootte.** Een blok dat groter is dan het blad, of meer dan 200.000 cellen heeft, geeft #SPILL!.
- **A1# is het hele blok.** Schrijf de cel die de formule bevat en een **#**, zoals in =SOM(A1#), om het hele blok te gebruiken zoals het nu is, vanaf dit blad of een ander (=SOM(Blad1!A1#)). Als A1 niet overloopt, is A1# #VERW!.

Een overloop wordt met het bestand opgeslagen als dynamische matrix, zodat het programma waarin het bestand is gemaakt het ook als overloop toont. Opslaan in de oudere .xls- of .ods-vorm bewaart de waarden, en de formule als matrixformule over het blok waar de oudere vorm de gebruikte functies heeft. Compatibiliteit controleren noemt deze formules.

## Waarden in een formule een naam geven

**LET** geeft een waarde in het midden van een formule een naam, zodat u die één keer schrijft en opnieuw gebruikt. De paren van een naam en een waarde komen eerst, en de berekening komt als laatste. In =LET(prijs; B2; btw; prijs*0,08; prijs+btw) staat de naam **prijs** voor de waarde in B2, wordt **btw** daaruit berekend, en telt het laatste deel ze op. Een naam kan de namen ervoor gebruiken, en een LET kan tot 126 paren bevatten. Een naam werkt alleen binnen de eigen LET, en een LET binnen een andere mag een naam opnieuw gebruiken. Een naam kan ook voor een bereik van cellen staan, zoals in =LET(rijen; B2:B9; SOM(rijen)/AANTAL(rijen)).

## Iets over een cel of de werkmap opvragen

**CEL** geeft informatie over een cel. Het eerste argument bepaalt welke: "adres", "rij", "kol", "inhoud", "type", "opmaak" (hoe getallen worden getoond, zoals "F2" voor twee decimalen, "P0" voor een percentage, of "D1" voor een datum), "kleur" (1 wanneer negatieve getallen in kleur worden getoond), "haakjes" (1 wanneer positieve getallen tussen haakjes worden getoond), "voorvoegsel" (het teken van een tekst die links, rechts of gecentreerd is uitgelijnd), "beveiligd" (1 wanneer de cel vergrendeld is), "breedte" (de breedte van de kolom in tekens), en "bestandsnaam" (de map, het bestand en het blad, zoals /home/ik/[Budget.xlsx]Blad1; leeg totdat de werkmap is opgeslagen). **INFO** geeft informatie over het programma en de werkmap: "directory", "numbestand" (hoeveel bladen zichtbaar zijn), "oorsprong", "osversie", "herberekening" (Automatisch of Handmatig), "release", en "systeem".

## Waarden uit XML halen

**FILTERXML** neemt tekst die XML bevat en een XPath, een korte manier om aan te geven welke delen u wilt, en geeft de gevonden waarden. =FILTERXML(A1; "//item/prijs") geeft elke prijs in de tekst in A1, de ene onder de andere. Gebruik "/winkel/item[2]/naam" voor het tweede item, "//item[@id='b']/naam" voor het item waarvan de id b is, of "aantal(//item)" voor het aantal. Als de tekst geen geldige XML is, of het pad er geen is, is het antwoord #WAARDE!. Als niets overeenkomt, is het #N/B. XML met een documenttype wordt geweigerd.
