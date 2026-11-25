# Hoe formules worden getoond en berekend
<!-- keywords: overloop, overlopen, #SPILL!, dynamische matrix, overloopbereik, A1#, volgende fout, vorige fout, fouten doorlopen, foutherleiding, berekenen voor opslaan, formule, formules, berekenen, berekening, opnieuw berekenen, functie, functies, waarde, resultaat, opgeslagen, F9, kringverwijzing, formules weergeven, formulebalk, fout, wijzen, klikken op een cel, verwijzing, F4, foutcontrole, fout negeren, formule evalueren, stap, controlevenster, controleren, naam geven aan een waarde, celfunctie, infofunctie, filterxml, xml, xpath, precisie zoals weergegeven, precisie instellen zoals weergegeven, afronden, afronding, decimalen, cijfers -->

Een **formule** is een cel die zijn waarde berekent uit andere cellen, zoals =SOM(B3:B8). De cel toont de waarde, en de **formulebalk** toont de formule.

## Wat JeffCrunch met formules doet

Wanneer u een werkmap opent, bevat het bestand de laatste waarde van elke formule, zodat het blad meteen kan worden getoond. Daarna berekent JeffCrunch elke formule opnieuw, een paar tegelijk, zodat het venster snel blijft. De statusbalk toont **Berekenen** met een percentage terwijl dat gebeurt. Wanneer het klaar is, bevatten de cellen de eigen resultaten van JeffCrunch. Om alles opnieuw te berekenen, klikt u op **Alles opnieuw berekenen** op het tabblad **Formules**, of drukt u op **F9**.

## Wanneer JeffCrunch een formule niet kan berekenen

JeffCrunch heeft meer dan 400 functies, en het aantal groeit nog. Een formule die een functie gebruikt die JeffCrunch nog niet heeft, behoudt de waarde die het bestand ervoor heeft opgeslagen. Dat geldt ook voor een formule die naar een andere werkmap verwijst. Cellen die daarvan afhangen, worden berekend uit de opgeslagen waarden.

## Functies die een paar aanvullingen nodig hebben

- **Obligaties en leningen.** PRIJS, RENDEMENT, DUURZAAMHEID, ODDFPRIJS en de andere obligatiefuncties tellen de dagen zoals de **basis** die u opgeeft aangeeft (0 is 30 dagen per maand in een jaar van 360 dagen, 1 zijn de werkelijke dagen, enzovoort). RENDEMENT, ODDFRENDEMENT, ODDLRENDEMENT en RENTEVOET vinden hun antwoord door waarden te proberen totdat de prijs of betaling klopt. Ze stoppen wanneer het antwoord tot ongeveer 15 cijfers juist is, dus de laatste cijfers kunnen afwijken van die in een ander programma.
- **Verdelingen en toetsen.** De functies die eindigen op .VERD, .INV, .TOETS, en de oudere namen zoals NORM.VERD en T.VERD, geven dezelfde antwoorden: een oudere naam blijft behouden zodat oude bestanden blijven werken. De *.INV-functies worden ook gevonden door waarden te proberen.
- **Lijsten die meerdere cellen vullen.** SORTEREN, SORTEREN.OP, FILTER, UNIEK, REEKS, ARRAY.WILLEKEURIG, XVERT, MMULT, MINVERSE en LINEAIR.VERBAND geven een blok waarden. Typt u er een in één cel, dan loopt het blok over in de cellen eronder en ernaast (zie **Formules die de cellen ernaast vullen**), of voert u het in als matrixformule (**Ctrl+Shift+Enter**) over de cellen die het moet vullen.
- **Teksten in andere schriften.** LENGTEB, LINKSB, DEELB, RECHTSB, VINDENB, ZOEKENB en VERVANGENB tellen elk teken als één, zoals op een computer die niet is ingesteld voor tweebytetekst. FONETISCH geeft de tekst van de cel, omdat JeffCrunch de lezingen van Japanse tekst niet bewaart.
- **CONVERTEREN.** Eenheden worden als tekst geschreven, met hoofdletters en kleine letters zoals in de lijst van de functie: "kg", "lbm", "mi", "F". Een metrisch voorvoegsel kan voor een eenheid die dat accepteert, zoals "km" of "mL".

## MOD: de rest

**REST(n; d)** geeft wat er overblijft van **n** nadat zoveel mogelijk hele keren **d** eraf is gehaald: het is n - d * INT(n / d). Voorbeeld: **=REST(10; 3)** is 1, en **=REST(-10; 3)** is 2, omdat INT(-10 / 3) gelijk is aan -4 en -10 - 3 * -4 gelijk is aan 2. Het antwoord heeft het teken van **d**. Als **d** 0 is, is het antwoord #DEEL/0!.

## Formules weergeven

Klik op **Formules weergeven** op het tabblad **Formules**, of druk op **Ctrl+`** (op een Mac de Control-toets, niet de Command-toets), om in plaats van de waarde van elke cel de formule te zien. De kolommen met formules worden breder, tot een bepaalde grens, zodat de formules volledig zichtbaar zijn; ze krijgen hun oorspronkelijke breedte terug wanneer u dit uitschakelt, en de breedten in uw bestand worden niet gewijzigd.

## Formules wijzigen

Om een formule te wijzigen, selecteert u de cel, drukt u op **F2**, past u de formule aan en drukt u op **Enter**. U kunt de formule ook in de formulebalk bewerken. Wanneer u een cel wijzigt, worden de formules die ervan afhangen op de achtergrond opnieuw berekend. Zie [Typen in cellen](edit-cells).

## Een formule schrijven

Typ **=** en de eerste letters van een functie, en een korte lijst toont de functies en [namen](names) die zo beginnen, met een zin over elk ervan. Druk op **Tab** om de gemarkeerde te kiezen. Gebruik de pijltjes **Omlaag** en **Omhoog** om een andere te markeren, en daarna **Tab** of **Enter**. **Esc** sluit de lijst. Zodra u binnen de haakjes van een functie bent, staan de argumenten onder de cel, met het argument dat u typt in vet. De lijst en de argumenten zijn dezelfde als die **Functie kiezen** toont. Argumenten worden gescheiden door komma's, of door puntkomma's waar uw taal een komma als decimaalteken gebruikt.

## Functienamen in uw taal

Wanneer JeffCrunch in het Duits, Frans, Spaans, Italiaans, Nederlands, Portugees of Russisch wordt getoond, kunt u een formule typen met de functienamen van uw taal, zoals `=SUMME(A1;A2)` in het Duits of `=SOMME(A1;A2)` in het Frans. De Engelse namen werken ook, dus een formule die u van het web kopieert, hoeft niet te worden aangepast. De formulebalk, de cel, de lijst met functies en **Functie kiezen** tonen de namen van uw taal.

De werkmap verandert niet wanneer u van taal wisselt. Een bestand bewaart altijd de Engelse namen, zodat een werkmap die in de ene taal is gemaakt in een andere taal, en in elk ander programma, goed opent en wordt gelezen. Lokale namen zijn beschikbaar in het Duits, Frans, Spaans, Italiaans, Nederlands, Portugees (Brazilië en Portugal) en Russisch; de Engelse namen werken altijd in elke taal. Oekraïense en Indonesische programma's behouden de Engelse namen voor bijna alle functies. Waar een taal twee gebruikelijke namen voor een functie heeft, worden beide gelezen en wordt de meest gebruikelijke getoond. Een functie die in uw taal geen gebruikelijke naam heeft, behoudt de Engelse. WAAR en ONWAAR volgen ook uw taal. De foutwaarden van uw taal, zoals `#NV` in het Duits, worden gelezen wanneer u een formule typt en worden in de formulebalk getoond, maar een cel met een fout toont altijd de standaardvorm, zoals `#N/A`.

Waar uw computer de komma als decimaalteken gebruikt, worden de argumenten van een functie onderscheiden door een puntkomma. Een matrix tussen accolades met meer dan één rij wordt getoond met een puntkomma tussen de waarden van een rij en een backslash tussen de rijen, zoals `{1;2\3;4}`, en u kunt hem zo typen. Een matrix die zonder backslash wordt getypt, wordt gelezen zoals een bestand die schrijft: komma's tussen de waarden van een rij, puntkomma's tussen rijen, en een punt als decimaalteken.

Om alleen Engelse functienamen te typen, klikt u op **Bestand > Opties > Formules** en vinkt u **Engelse functienamen gebruiken** aan.

## Naar cellen wijzen

In plaats van het adres van een cel te typen, kunt u ernaar wijzen. Terwijl u een formule typt, zet u de cursor waar een cel kan komen (direct na **=**, een operator zoals **+**, een openingshaakje of een komma) en klikt u op de cel. Het adres wordt voor u geschreven. Sleep over cellen om een bereik zoals B2:D5 te schrijven. Klik opnieuw voordat u iets anders typt, en de nieuwe cel vervangt de zojuist geschreven cel. Houd **Ctrl** ingedrukt en klik om na een komma nog een adres toe te voegen. Klik op een kolomletter om een hele kolom te schrijven, zoals A:A, of op een rijnummer om een hele rij te schrijven, zoals 3:3, en sleep langs de letters of nummers voor B:D of 2:5.

De pijltoetsen werken ook. Druk op een pijltoets en een aanwijzer verplaatst zich weg van de cel waarin u typt, en het adres wordt geschreven; druk op meer pijltjes om verder te gaan, of houd **Shift** ingedrukt om de aanwijzer over een blok uit te rekken. De statusbalk toont **Wijzen** terwijl de pijltoetsen wijzen, **Invoeren** terwijl u typt, en **Bewerken** terwijl de pijltoetsen de cursor in de tekst verplaatsen. Druk op **F2** om te wisselen tussen wijzen en de tekst bewerken.

Om naar een cel op een ander blad te wijzen, klikt u op het tabblad van dat blad terwijl de cursor staat waar een cel kan komen, en daarna op de cel. De naam van het blad wordt meegeschreven, tussen aanhalingstekens wanneer dat nodig is, zoals in 'Mijn blad'!B3. **Enter** brengt u terug naar het blad waarop u begon en zet de formule in de cel. **Esc** annuleert alles, ook de wisseling van blad.

U kunt ook de rand van een omlijnd bereik op het blad slepen om dat adres te verplaatsen, of het vierkantje in de hoek slepen om het uit te rekken; de formule verandert terwijl u sleept.

Druk op **F4** met de cursor in of direct na een adres om delen ervan met een dollarteken vast te zetten: A1 wordt $A$1, dan A$1, dan $A1, en dan weer A1.

Elk adres in de formule krijgt een kleur in de tekst, en de cellen die het noemt, worden in dezelfde kleur op het blad omlijnd, zodat u in één oogopslag ziet wat de formule leest. Een klik na een compleet adres, zoals =A1, beëindigt de invoer en selecteert de cel waarop u klikte, zoals bij elke andere tekst.

## Gegevenstabellen

Een gegevenstabel (wat-als-tabel) laat zien wat een formule oplevert voor veel waarden van één of twee cellen. Opent u een werkmap die er een bevat, dan berekent JeffCrunch de cellen ervan: bij een tabel met twee invoerwaarden staat de formule in de cel linksboven van de tabel, de waarden die over de rij worden geprobeerd komen in de invoercel voor de rij, en die naar beneden in de invoercel voor de kolom. Bij een tabel met één invoerwaarde staan de waarden onder elkaar in de kolom links ervan (en staan de formules in de rij erboven), of naast elkaar in de rij erboven (en staan de formules in de kolom links ervan). Elke cel van de tabel is het resultaat van de formule, en van elke formule die van de invoercel afhangt, met de invoer op die waarde; de rest van de werkmap blijft ongemoeid. In de modus **Automatisch** worden de tabellen samen met de formules berekend, en de formules die de cellen van een tabel lezen, volgen. Het maken van een nieuwe gegevenstabel wordt nog niet aangeboden. Een tabel die niet kan worden berekend (een formule die van de invoer afhangt gebruikt een functie die JeffCrunch niet heeft, of die zichzelf verandert, zoals ASELECT of NU) behoudt de waarden die in het bestand staan.

## Met de hand berekenen

Klik op **Berekeningsmodus** op het tabblad **Formules** om te kiezen hoe de werkmap zijn formules berekent:

- **Automatisch** berekent ze terwijl u cellen wijzigt.
- **Automatisch, tabellen handmatig** doet hetzelfde, maar laat de cellen van gegevenstabellen met rust tot u op **F9** drukt.
- **Handmatig** berekent niets, ook geen gegevenstabellen, tot u erom vraagt. De statusbalk toont **Niet meer actueel** na een wijziging. Druk op **F9** (**Alles opnieuw berekenen**) om alles te berekenen, of op **Shift+F9** (**Dit blad opnieuw berekenen**) om de formules van het blad waarop u staat te berekenen, en wat verder wacht. Een werkmap die op Handmatig staat, opent met de resultaten die het bestand bevat. **Berekenen voor opslaan**, ook in het menu **Berekeningsmodus**, staat standaard aan: wanneer u een werkmap in de handmatige modus opslaat die niet meer actueel is, worden de formules eerst berekend, zodat het bestand actuele getallen bevat. Schakel het uit om de getallen zoals ze zijn op te slaan.

De keuze wordt met de werkmap opgeslagen en is één stap die u ongedaan kunt maken.

## Formules die naar zichzelf verwijzen

Een formule die zijn eigen resultaat leest, rechtstreeks of via andere cellen, kan niet op de gewone manier worden berekend. JeffCrunch meldt dat één keer, noemt de cellen en omlijnt ze op het blad; ze behouden de waarden die ze hadden. Klik op **Kringverwijzingen** op het tabblad **Formules** voor de lijst, en dubbelklik op een cel erin om er naartoe te gaan.

Sommige modellen hebben zo'n kring met opzet nodig. Kies **Kringformules iteratief berekenen** in het menu **Berekeningsmodus**, en de formules worden steeds opnieuw berekend, elk met de laatste waarden van de andere, totdat geen waarde meer verandert dan het bedrag dat u opgeeft, of het aantal rondes dat u toestaat is bereikt.

## Houd de getallen zoals de cellen ze tonen

Een cel kan minder cijfers tonen dan hij bevat. Als een cel 2,675 bevat en 2,68 toont, gebruiken de formules die hem lezen nog steeds 2,675. Soms wilt u dat ze gebruiken wat u ziet. Kies dan **Precisie instellen zoals weergegeven** in het menu **Berekeningsopties** op het tabblad **Formules**.

JeffCrunch waarschuwt u eerst, omdat de verborgen cijfers verloren gaan. Kies **Annuleren** en er verandert niets. Kies **OK** en elk getal in de werkmap wordt afgerond op wat de cel toont. Een cel die twee decimalen toont, behoudt er twee. Een cel die een percentage toont, behoudt het percentage. Een cel met een datum behoudt de dag, en een cel met een tijd behoudt het kleinste deel van de tijd dat hij toont, zoals de minuut. Een cel die **Standaard** toont, behoudt al zijn cijfers. Bij een grote werkmap verschijnt tijdens dit proces een melding met een knop **Annuleren**. Annuleer terwijl de melding zichtbaar is, en er verandert niets.

Daarna wordt elk getal dat u typt of plakt, en elk antwoord van een formule, op dezelfde manier afgerond. De formules zelf worden niet gewijzigd. De keuze wordt met de werkmap opgeslagen.

Afronden is één stap die u ongedaan kunt maken, en **Ongedaan maken** brengt de getallen terug. Als u **Precisie instellen zoals weergegeven** nogmaals kiest, wordt het uitgeschakeld. Dat stopt alleen het afronden vanaf dat moment. De getallen die al waren afgerond, blijven afgerond.

## Zien wat een formule leest

Selecteer een cel en klik op **Toon wat deze cel voedt** om pijlen te tekenen naar de cellen die de formule ervan leest. Klik opnieuw om nog één stap verder terug te gaan. **Toon wat deze cel gebruikt** tekent pijlen naar de formules die de cel lezen, en elke klik gaat een stap verder. Een klein blad staat voor een cel op een ander blad. Er worden ten hoogste 2.000 pijlen getekend, en de statusbalk meldt wanneer er meer waren. **Pijlen wissen** wist ze, en dat doet elke wijziging in de werkmap ook.

Wanneer een cel een fout toont, zoals #DEEL/0!, selecteert u hem en klikt u op **Foutherleiding**. Er worden pijlen getekend naar de cellen die hij leest en die ook een fout tonen. Klik opnieuw om nog een stap verder terug te gaan. Wanneer geen enkele cel die hij leest een fout toont, meldt de statusbalk dat deze cel de fout veroorzaakt, en dat is de cel die u moet oplossen.

Om alle fouten op het blad langs te gaan, klikt u op **Volgende fout** of **Vorige fout**. De cellen die een fout tonen worden rij voor rij bezocht, en na de laatste begint het weer bij de eerste. Elke cel wordt geselecteerd, en de pijlen worden helemaal getekend tot de cel die de fout veroorzaakt. De statusbalk toont welke fout u bekijkt en hoeveel er zijn.

## Foutcontrole

Kies **Foutcontrole** in de groep **Controleren** op het tabblad Formules om het blad te doorzoeken op cellen die fout kunnen zijn. Het zoekt naar een formule die een fout oplevert (zoals #DEEL/0!), naar een getal dat als tekst is opgeslagen (wat formules kunnen overslaan), en meer: een formule die niet lijkt op de formules ernaast, een datum die als tekst is opgeslagen met een jaartal van twee cijfers, een formule in een cel die niet is vergrendeld, een formule die een getal naast de gebruikte cellen weglaat (alleen voor een functie die optelt of een bereik samenvat, zoals SOM, GEMIDDELDE, AANTAL, MIN of MAX, wanneer het bereik op zichzelf als argument staat), en een formule die een lege cel gebruikt.

Het dialoogvenster toont één cel tegelijk, gaat naar die cel op het blad en zegt in één zin wat er aan de hand is. **Volgende** en **Vorige** bewegen door de cellen. Een klein groen driehoekje in de linkerbovenhoek van een cel markeert die op het blad. Wanneer de cel kan worden hersteld, verschijnt een knop met de oplossing erop (bij een formule die een getal weglaat, **Cel opnemen**); die wijzigt de formule, en één stap Ongedaan maken brengt de oude formule terug. **Fout negeren** haalt de cel van de lijst, verwijdert het driehoekje, en wordt in het bestand opgeslagen. Eén stap Ongedaan maken brengt het terug. Is er niets mis, dan krijgt u dat te horen. Om te kiezen welke van deze worden aangemerkt, opent u **Opties** en de pagina **Formules**: er is een selectievakje voor elk, en een vakje dat u leegmaakt, voorkomt dat die soort wordt aangemerkt. De keuze geldt voor het programma, dus voor elk geopend venster. Een cel kan meer dan één probleem hebben, en **Fout negeren** negeert alleen het probleem dat op dat moment wordt getoond.

## Formule evalueren

Gebruik dit om te zien hoe een formule tot zijn waarde komt. Selecteer een cel die een formule bevat en kies **Formule evalueren**. De formule wordt getoond met het volgende deel onderstreept. Druk op **Evalueren** om dat deel te berekenen; de waarde ervan neemt zijn plaats in. De delen komen in deze volgorde: elke enkele cel van links, elke functie met zijn haakjes, te beginnen bij de binnenste, en daarna wat overblijft. **Opnieuw begint opnieuw.** Als een deel niet kan worden berekend, zegt het dialoogvenster waarom en blijft de formule zoals hij was.

## Controlevenster

Het **Controlevenster** is een klein venster dat in beeld blijft terwijl u werkt. Het toont cellen die u in de gaten wilt houden, met hun blad, cel, waarde en formule, ook wanneer ze op een ander blad staan. Selecteer cellen en druk op **Controle toevoegen**. Selecteer rijen in de lijst en druk op **Controle verwijderen**. Dubbelklik op een rij om naar de cel te gaan. De waarden veranderen wanneer het blad opnieuw wordt berekend. De lijst blijft bewaard zolang de werkmap open is.

## Formules die de cellen ernaast vullen

Wanneer een formule een blok waarden geeft, vult het blok de cellen onder en naast de cel waarin u de formule typte. Dit heet overlopen. Probeer =REEKS(3) in A1: A1 toont 1, en A2 en A3 tonen 2 en 3. Een formule loopt over wanneer hij een functie aanroept die een blok geeft (SORTEREN, SORTEREN.OP, FILTER, UNIEK, REEKS, ARRAY.WILLEKEURIG, TRANSPONEREN, XVERT of MMULT), een blok tussen accolades schrijft, een overloopbereik gebruikt (hieronder), of rekenkunde op een bereik uitvoert, zoals in =A1:A3*2.

- **De cellen die het vult hebben geen eigen formule.** Ze tonen de waarden. Wanneer u er een selecteert, toont de formulebalk in grijs de formule van de cel die hem vult. Enter op de cel zoals hij is, verandert niets. Om het blok te wijzigen, wijzigt u eerst die eerste cel.
- **Het blok volgt het resultaat.** Als de formule na een wijziging in wat hij leest meer of minder waarden geeft, wordt het blok groter of kleiner, en worden de formules die die cellen lezen opnieuw berekend.
- **Een cel die in de weg staat geeft #SPILL!.** Als een cel waar het blok zou komen een waarde of een formule bevat, toont de eerste cel **#SPILL!** en wordt er niets ingevuld. Maak de cel in de weg leeg en het blok vult zich meteen. Typen in een cel die het blok vult doet hetzelfde: de eerste cel toont #SPILL!.
- **Een blok heeft een groottegrens.** Een blok dat groter is dan het blad, of meer dan 200.000 cellen heeft, geeft #SPILL!.
- **A1# is het hele blok.** Schrijf de cel die de formule bevat en een **#**, zoals in =SOM(A1#), om het hele blok te gebruiken zoals het nu is, vanaf dit blad of een ander (=SOM(Blad1!A1#)). Als A1 niet overloopt, is A1# #VERW!.

Een overlopend blok wordt met het bestand opgeslagen als dynamische matrix, zodat het programma waaruit het bestand komt het ook als overloop toont. Opslaan in de oudere .xls- of .ods-vorm behoudt de waarden, en de formule als matrixformule over het blok waar de oudere vorm de functies heeft die hij gebruikt. Compatibiliteit controleren noemt deze formules.

## Waarden binnen een formule een naam geven

**LET** geeft een waarde in het midden van een formule een naam, zodat u hem één keer schrijft en opnieuw gebruikt. De paren van een naam en een waarde komen eerst, en de berekening komt als laatste. In =LET(prijs; B2; btw; prijs*0,08; prijs+btw) staat de naam **prijs** voor de waarde in B2, wordt **btw** daaruit berekend, en telt het laatste deel ze op. Een naam kan de namen ervoor gebruiken, en een LET kan tot 126 paren bevatten. Een naam werkt alleen binnen zijn eigen LET, en een LET binnen een andere mag dezelfde naam opnieuw gebruiken. Een naam kan ook voor een bereik van cellen staan, zoals in =LET(rijen; B2:B9; SOM(rijen)/AANTAL(rijen)).

## Iets vragen over een cel of de werkmap

**CEL** geeft iets over een cel. Het eerste argument zegt wat: "adres", "rij", "kolom", "inhoud", "type", "opmaak" (hoe getallen worden getoond, zoals "F2" voor twee decimalen, "P0" voor een percentage, of "D1" voor een datum), "kleur" (1 wanneer negatieve getallen in kleur worden getoond), "haakjes" (1 wanneer positieve getallen tussen haakjes worden getoond), "voorvoegsel" (het teken van een tekst die links, rechts of gecentreerd is uitgelijnd), "beveiligd" (1 wanneer de cel is vergrendeld), "breedte" (de breedte van de kolom in tekens), en "bestandsnaam" (de map, het bestand en het blad, zoals /home/ik/[Begroting.xlsx]Blad1; leeg totdat de werkmap is opgeslagen). **INFO** geeft informatie over het programma en de werkmap: "directory", "numfile" (hoeveel bladen zichtbaar zijn), "origin", "osversion", "recalc" (Automatisch of Handmatig), "release" en "system".

## Waarden uit XML halen

**FILTERXML** neemt tekst die XML bevat en een XPath, een korte manier om aan te geven welke delen u wilt, en geeft de waarden die het vindt. =FILTERXML(A1; "//item/prijs") geeft elke prijs in de tekst in A1, één onder de andere. Gebruik "/winkel/item[2]/naam" voor het tweede item, "//item[@id='b']/naam" voor het item waarvan de id b is, of "count(//item)" voor hoeveel er zijn. Als de tekst geen geldige XML is, of het pad geen geldig pad is, is het antwoord #WAARDE!. Als niets overeenkomt, is het #N/B. XML met een documenttype wordt geweigerd.
