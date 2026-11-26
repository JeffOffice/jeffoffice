# Hoe formules worden getoond en berekend
<!-- keywords: overloop, overloop, #SPILL!, dynamische matrix, overloopbereik, A1#, volgende fout, vorige fout, fouten doorlopen, foutspoor, berekenen voor opslaan, formule, formules, berekenen, berekening, opnieuw berekenen, functie, functies, waarde, resultaat, opgeslagen, F9, circulair, formules weergeven, formulebalk, fout, aanwijzen, klikken op cel, verwijzing, F4, foutcontrole, fout negeren, formule evalueren, stap, controlevenster, controleren, LET, naam voor waarde, celfunctie, info-functie, FILTERXML, xml, xpath, nauwkeurigheid zoals weergegeven, afronden, decimalen, cijfers-->

Een **formule** is een cel die zijn waarde berekent uit andere cellen, zoals =SOM(B3:B8). De cel toont de waarde, en de **formulebalk** toont de formule.

## Wat JeffCrunch met formules doet

Wanneer u een werkmap opent, bevat het bestand de laatste waarde van elke formule, zodat het blad meteen kan worden getoond. Daarna berekent JeffCrunch elke formule opnieuw, een paar tegelijk, zodat het venster snel blijft. De statusbalk toont **Berekenen** met een percentage terwijl dat gebeurt. Wanneer het klaar is, bevatten de cellen de eigen resultaten van JeffCrunch. Om alles opnieuw te berekenen, klikt u op **Alles opnieuw berekenen** op het tabblad **Formules**, of drukt u op **F9**.

## Wanneer JeffCrunch een formule niet kan berekenen

JeffCrunch heeft meer dan 400 functies, en het aantal groeit nog. Een formule die een functie gebruikt die JeffCrunch nog niet heeft, behoudt de waarde die het bestand ervoor heeft opgeslagen. Dat geldt ook voor een formule die naar een andere werkmap verwijst. Cellen die daarvan afhankelijk zijn, worden berekend op basis van de opgeslagen waarden.

## Functies die een paar toelichtingen nodig hebben

- **Obligaties en leningen.** PRICE, YIELD, DURATION, ODDFPRICE en de andere obligatiefuncties tellen dagen op de manier die de **basis** aangeeft die u opgeeft (0 is 30 dagen per maand in een jaar van 360 dagen, 1 zijn de werkelijke dagen, enzovoort). YIELD, ODDFYIELD, ODDLYIELD en RATE zoeken hun antwoord door waarden te proberen totdat de prijs of betaling klopt. Ze stoppen wanneer het antwoord tot ongeveer 15 cijfers juist is, dus de laatste cijfers kunnen afwijken van die in een ander programma.
- **Verdelingen en toetsen.** De functies die eindigen op .DIST, .INV, .TEST, en de oudere namen zoals NORMDIST en TDIST, geven dezelfde antwoorden: een oudere naam blijft bewaard zodat oude bestanden blijven werken. De *.INV-functies worden ook gevonden door waarden te proberen.
- **Lijsten die meerdere cellen vullen.** SORT, SORTBY, FILTER, UNIQUE, SEQUENCE, RANDARRAY, XLOOKUP, MMULT, MINVERSE en LINEST geven een blok waarden. Typ er een in één cel en het blok loopt over in de cellen eronder en ernaast (zie **Formules die de cellen ernaast vullen**), of voer het in als matrixformule (**Ctrl+Shift+Enter**) over de cellen die het moet vullen.
- **Teksten in andere schriften.** LENB, LEFTB, MIDB, RIGHTB, FINDB, SEARCHB en REPLACEB tellen elk teken als één, zoals op een computer die niet is ingesteld voor dubbelbytetekst. PHONETIC geeft de tekst van de cel, omdat JeffCrunch de uitspraak van Japanse tekst niet bewaart.
- **CONVERT.** Eenheden worden als tekst geschreven, met hoofdletters en kleine letters zoals in de lijst van de functie: "kg", "lbm", "mi", "F". Een metrisch voorvoegsel kan voor een eenheid die dat aanneemt, zoals "km" of "mL".

## MOD: de rest

**MOD(n; d)** geeft wat er overblijft van **n** nadat de meeste hele keren **d** eraf zijn gehaald: het is n - d * INT(n / d). Voorbeeld: **=MOD(10; 3)** is 1, en **=MOD(-10; 3)** is 2, omdat INT(-10 / 3) gelijk is aan -4 en -10 - 3 * -4 gelijk is aan 2. Het antwoord heeft het teken van **d**. Als **d** 0 is, is het antwoord #DEEL/0!.

## Formules weergeven

Klik op **Formules weergeven** op het tabblad **Formules**, of druk op **Ctrl+`** (op een Mac de Control-toets, niet de Command-toets), om in plaats van de waarde van elke cel de formule te zien. De kolommen met formules worden breder, tot een bepaalde grens, zodat de formules volledig worden getoond; ze krijgen weer hun oorspronkelijke breedte wanneer u dit uitschakelt, en de breedten in uw bestand worden niet gewijzigd.

## Formules wijzigen

Om een formule te wijzigen, selecteert u de cel, drukt u op **F2**, bewerkt u de formule en drukt u op **Enter**. U kunt de formule ook in de formulebalk bewerken. Wanneer u een cel wijzigt, worden de formules die ervan afhankelijk zijn op de achtergrond opnieuw berekend. Zie [Cellen invoeren](edit-cells).

## Een formule schrijven

Typ **=** en de eerste letters van een functie, dan toont een korte lijst de functies en [namen](names) die zo beginnen, met een zin over elk. Druk op **Tab** om de gemarkeerde te kiezen. Gebruik de pijltjestoetsen **Omlaag** en **Omhoog** om een andere te markeren, en daarna **Tab** of **Enter**. **Esc** sluit de lijst. Zodra u binnen de haakjes van een functie bent, worden de argumenten onder de cel getoond, met het argument dat u typt in vet. De lijst en de argumenten zijn dezelfde als die **Functie kiezen** toont. Argumenten worden gescheiden door komma's, of door puntkomma's waar uw taal een komma als decimaalteken gebruikt.

## Functienamen in uw taal

Wanneer JeffCrunch in het Duits, Frans, Spaans, Italiaans, Nederlands, Portugees of Russisch wordt getoond, kunt u een formule typen met de functienamen van uw taal, zoals `=SUMME(A1;A2)` in het Duits of `=SOMME(A1;A2)` in het Frans. De Engelse namen werken ook, dus een formule die u van het web kopieert, hoeft niet te worden aangepast. De formulebalk, de cel, de lijst met functies en **Functie kiezen** tonen de namen van uw taal.

De werkmap verandert niet wanneer u van taal wisselt. Een bestand bewaart altijd de Engelse namen, zodat een werkmap die in de ene taal is gemaakt, in een andere taal en in elk ander programma goed opent en wordt gelezen. Lokale namen zijn beschikbaar in het Duits, Frans, Spaans, Italiaans, Nederlands, Portugees (Brazilië en Portugal) en Russisch; de Engelse namen werken altijd, in elke taal. Oekraïense en Indonesische programma's houden voor bijna elke functie de Engelse namen. Als een taal twee gangbare namen voor een functie heeft, worden beide gelezen en wordt de meest gebruikelijke getoond. Een functie die in uw taal geen gebruikelijke naam heeft, houdt de Engelse. WAAR en ONWAAR volgen ook uw taal. De foutwaarden van uw taal, zoals `#NV` in het Duits, worden gelezen wanneer u een formule typt en worden in de formulebalk getoond, maar een cel met een fout toont altijd de standaardvorm, zoals `#N/A`.

Waar de regionale instellingen van uw computer de komma als decimaalteken gebruiken, worden de argumenten van een functie onderscheiden door een puntkomma. Een matrix tussen accolades met meer dan één rij wordt getoond met een puntkomma tussen de waarden van een rij en een backslash tussen de rijen, zoals `{1;2\3;4}`, en u kunt hem zo typen. Een matrix die u zonder backslash typt, wordt gelezen zoals een bestand hem schrijft: komma's tussen de waarden van een rij, puntkomma's tussen rijen, en een punt als decimaalteken.

Om alleen Engelse namen te typen, klikt u op **Bestand > Opties > Formules** en vinkt u **Engelse functienamen gebruiken** aan.

## Naar cellen wijzen

In plaats van het adres van een cel te typen, kunt u ernaar wijzen. Terwijl u een formule typt, plaatst u de cursor waar een cel kan staan (direct na **=**, een operator zoals **+**, een openingshaakje of een komma) en klikt u op de cel. Het adres wordt voor u geschreven. Sleep over cellen om een bereik zoals B2:D5 te schrijven. Klik opnieuw voordat u iets anders typt, dan vervangt de nieuwe cel de cel die net is geschreven. Houd **Ctrl** ingedrukt en klik om na een komma nog een adres toe te voegen. Klik op een kolomletter om een hele kolom zoals A:A te schrijven, of op een rijnummer om een hele rij zoals 3:3 te schrijven, en sleep langs de letters of nummers voor B:D of 2:5.

De pijltjestoetsen werken ook. Druk op een pijltoets en een aanwijzer beweegt weg van de cel waarin u typt, en het adres wordt geschreven; druk op meer pijltjes om verder te gaan, of houd **Shift** ingedrukt om de aanwijzer over een blok uit te rekken. De statusbalk toont **Wijzen** terwijl de pijltjestoetsen wijzen, **Enter** terwijl u typt, en **Bewerken** terwijl de pijltjestoetsen de cursor in de tekst verplaatsen. Druk op **F2** om te wisselen tussen wijzen en de tekst bewerken.

Om naar een cel op een ander blad te wijzen, klikt u op het tabblad van dat blad terwijl de cursor staat waar een cel kan komen, en daarna klikt u op de cel. De naam van het blad wordt erbij geschreven, tussen aanhalingstekens waar die nodig zijn, zoals in 'Mijn blad'!B3. **Enter** brengt u terug naar het blad waarop u begon en zet de formule in de cel. **Esc** annuleert alles, ook de overstap naar het andere blad.

U kunt ook de rand van een omlijnd blok op het blad slepen om dat adres te verplaatsen, of het vierkantje in de hoek slepen om het uit te rekken; de formule verandert terwijl u sleept.

Druk op **F4** met de cursor in of direct na een adres om delen ervan met een dollarteken vast te zetten: A1 wordt $A$1, dan A$1, dan $A1, en dan weer A1.

Elk adres in de formule wordt in de tekst in kleur weergegeven, en de cellen die het noemt, worden op het blad in dezelfde kleur omlijnd, zodat u in één oogopslag ziet wat de formule leest. Een klik na een compleet adres, zoals =A1, beëindigt de invoer en selecteert de cel waarop u klikte, zoals bij elke andere tekst.

## Gegevenstabellen

Een gegevenstabel (wat-als-tabel) toont wat een formule oplevert voor veel waarden van één of twee cellen. Opent u een werkmap die er een bevat, dan berekent JeffCrunch de cellen ervan: bij een tabel met twee invoerwaarden staat de formule in de cel linksboven van de tabel, de waarden die over de rij worden geprobeerd gaan in de invoercel voor de rij, en die naar beneden gaan in de invoercel voor de kolom. Bij een tabel met één invoerwaarde staan de waarden onder elkaar in de kolom links ervan (en staan de formules in de rij erboven), of naast elkaar in de rij erboven (en staan de formules in de kolom links ervan). Elke cel van de tabel is het resultaat van de formule, en van elke formule die van de invoercel afhangt, met de invoer op die waarde; er wordt verder niets in de werkmap aangeraakt. In de modus **Automatisch** worden de tabellen samen met de formules berekend, en de formules die de cellen van een tabel lezen, volgen. Een nieuwe gegevenstabel maken wordt nog niet aangeboden. Een tabel die niet berekend kan worden (een formule die van de invoer afhangt gebruikt een functie die JeffCrunch niet heeft, of verandert vanzelf, zoals ASELECT of NU), behoudt de waarden die in het bestand staan.

## Met de hand berekenen

Klik op **Berekeningsmodus** op het tabblad **Formules** om te kiezen hoe de werkmap zijn formules berekent:

- **Automatisch** berekent ze terwijl u cellen wijzigt.
- **Automatisch, tabellen handmatig** doet hetzelfde, maar laat de cellen van gegevenstabellen staan zoals ze zijn tot u op **F9** drukt.
- **Handmatig** berekent niets, ook geen gegevenstabellen, tot u erom vraagt. De statusbalk toont **Verouderd** na een wijziging. Druk op **F9** (**Alles opnieuw berekenen**) om alles te berekenen, of op **Shift+F9** (**Dit blad opnieuw berekenen**) om de formules van het blad waarop u staat te berekenen, samen met wat er verder wachtte. Een werkmap die op Handmatig staat, opent met de resultaten die het bestand bevat. **Berekenen voor opslaan**, ook in het menu **Berekeningsmodus**, staat in eerste instantie aan: wanneer u een handmatige werkmap opslaat die verouderd is, worden de formules eerst berekend, zodat het bestand actuele getallen bevat. Schakel dit uit om de getallen op te slaan zoals ze zijn.

De keuze wordt bij de werkmap opgeslagen, en het is één stap die u ongedaan kunt maken.

## Formules die naar zichzelf verwijzen

Een formule die zijn eigen resultaat leest, rechtstreeks of via andere cellen, kan niet op de gebruikelijke manier worden berekend. JeffCrunch meldt dat één keer, noemt de cellen en omlijnt ze op het blad; ze behouden de waarden die ze hadden. Klik op **Kringverwijzingen** op het tabblad **Formules** voor de lijst, en dubbelklik op een cel in de lijst om er naartoe te gaan.

Sommige modellen hebben zo'n kring bewust nodig. Kies **Kringformules iteratief berekenen** in het menu **Berekeningsmodus**, dan worden de formules steeds opnieuw berekend, waarbij elke formule de nieuwste waarden van de andere leest, totdat geen waarde meer dan het bedrag dat u opgeeft verandert, of het aantal rondes dat u toestaat is gebruikt.

## De getallen houden zoals de cellen ze tonen

Een cel kan minder cijfers tonen dan hij bevat. Als een cel 2,675 bevat en 2,68 toont, gebruiken de formules die hem lezen nog steeds 2,675. Soms wilt u dat ze gebruiken wat u ziet. Kies dan **Nauwkeurigheid zoals weergegeven instellen** in het menu **Berekeningsopties** op het tabblad **Formules**.

JeffCrunch waarschuwt u eerst, omdat de verborgen cijfers verloren gaan. Kies **Annuleren** en er verandert niets. Kies **OK** en elk getal in de werkmap wordt afgerond op wat zijn cel toont. Een cel die twee decimalen toont, houdt er twee. Een cel die een percentage toont, houdt het percentage. Een cel met een datum houdt de dag, en een cel met een tijd houdt het kleinste deel van de tijd dat hij toont, zoals de minuut. Een cel die **Algemeen** toont, houdt al zijn cijfers. Bij een grote werkmap verschijnt tijdens dit proces een melding met een knop **Annuleren**. Annuleer terwijl de melding er staat, dan wordt er niets gewijzigd.

Daarna wordt elk getal dat u typt of plakt, en elk antwoord dat een formule geeft, op dezelfde manier afgerond. De formules zelf worden niet gewijzigd. De keuze wordt bij de werkmap opgeslagen.

Afronden is één stap die u ongedaan kunt maken, en **Ongedaan maken** zet de getallen terug. Als u **Nauwkeurigheid zoals weergegeven instellen** opnieuw kiest, wordt het uitgeschakeld. Dat stopt alleen het afronden vanaf dat moment. De getallen die al waren afgerond, blijven afgerond.

## Zien wat een formule leest

Selecteer een cel en klik op **Wat voedt deze cel** om pijlen te tekenen naar de cellen die de formule leest. Klik nogmaals om een stap verder terug te gaan. **Wat gebruikt deze cel** tekent pijlen naar de formules die de cel lezen; elke klik gaat één stap verder. Een klein blad staat voor een cel op een ander blad. Er worden maximaal 2.000 pijlen getekend, en de statusbalk meldt wanneer het er meer waren. **Pijlen wissen** verwijdert ze, en dat gebeurt ook bij elke wijziging in de werkmap.

Wanneer een cel een fout toont, zoals #DEEL/0!, selecteert u hem en klikt u op **Foutspoor**. Er worden pijlen getekend naar de cellen die hij leest en die ook een fout tonen. Klik nogmaals om een stap verder terug te gaan. Wanneer geen enkele cel die hij leest een fout toont, meldt de statusbalk dat deze cel zelf de fout veroorzaakt, en dat is de cel die u moet verbeteren.

Om alle fouten op het blad langs te gaan, klikt u op **Volgende fout** of **Vorige fout**. De cellen die een fout tonen, worden rij voor rij bezocht, en na de laatste begint het weer bij de eerste. Elke cel wordt geselecteerd, en de pijlen worden helemaal terug getekend naar de cel die de fout veroorzaakt. De statusbalk toont welke fout u bekijkt en hoeveel er zijn.

## Foutcontrole

Kies **Foutcontrole** in de groep **Controle** op het tabblad Formules om het blad te doorzoeken op cellen die mogelijk verkeerd zijn. Er wordt gezocht naar een formule die een fout oplevert (zoals #DEEL/0!), een getal dat als tekst is opgeslagen (wat formules kunnen overslaan), en meer: een formule die niet lijkt op de formules ernaast, een datum die als tekst is opgeslagen met een jaartal van twee cijfers, een formule in een cel die niet vergrendeld is, een formule die een getal naast de gebruikte cellen weglaat (alleen voor een functie die optelt of sommeert over een bereik, zoals **SOM**, **GEMIDDELDE**, **AANTAL**, **MIN** of **MAX**, wanneer het bereik op zichzelf als argument staat), en een formule die een lege cel gebruikt.

Het dialoogvenster toont één cel tegelijk, gaat naar die cel op het blad en zegt in één zin wat er mis is. Met **Volgende** en **Vorige** gaat u door de cellen. Een klein groen driehoekje in de linkerbovenhoek van een cel markeert hem op het blad. Klik op het driehoekje zelf om naast de cel een menu te openen: het zegt wat er mis is, biedt de oplossing aan wanneer die er is, en heeft **Fout negeren** en **Foutcontrole…**. Wanneer de cel hersteld kan worden, verschijnt een knop met de oplossing erop (bij een formule die een getal weglaat, **Cel opnemen**); die past de formule aan, en één stap Ongedaan maken zet de oude formule terug. **Fout negeren** haalt de cel van de lijst, verwijdert zijn driehoekje en wordt in het bestand opgeslagen. Eén stap Ongedaan maken haalt het terug. Als er niets mis is, wordt u dat gemeld. Om te kiezen welke van deze worden gemarkeerd, opent u **Opties** en de pagina **Formules**: er is een vinkvak voor elk, en een vakje dat u leegmaakt, voorkomt dat die soort wordt gemarkeerd. De keuze geldt voor het programma en dus voor elk geopend venster. Een cel kan meer dan één probleem hebben, en **Fout negeren** negeert alleen het probleem dat wordt getoond.

## Formule evalueren

Gebruik dit om te zien hoe een formule tot zijn waarde komt. Selecteer een cel die een formule bevat en kies **Formule evalueren**. De formule wordt getoond met het volgende deel onderstreept. Druk op **Evalueren** om dat deel te berekenen; de waarde ervan neemt zijn plaats in. De delen komen in deze volgorde: elke losse cel van links, dan elke functie met zijn haakjes, te beginnen bij de binnenste, en dan wat overblijft. **Opnieuw** begint opnieuw. Als een deel niet berekend kan worden, zegt het dialoogvenster waarom en blijft de formule zoals hij was.

## Controlevenster

Het **Controlevenster** is een klein venster dat zichtbaar blijft terwijl u werkt. Het toont cellen die u in de gaten wilt houden, met hun blad, cel, waarde en formule, ook wanneer ze op een ander blad staan. Selecteer cellen en druk op **Controle toevoegen**. Selecteer rijen in de lijst en druk op **Controle verwijderen**. Dubbelklik op een rij om naar de cel te gaan. De waarden veranderen wanneer het blad opnieuw wordt berekend. De lijst blijft bewaard zolang de werkmap open is.

## Formules die de cellen ernaast vullen

Wanneer een formule een blok waarden geeft, vult het blok de cellen onder en naast de cel waarin u de formule typte. Dit heet overlopen. Probeer =REEKS(3) in A1: A1 toont 1, en A2 en A3 tonen 2 en 3. Een formule loopt over wanneer hij een functie aanroept die een blok geeft (SORT, SORTBY, FILTER, UNIQUE, SEQUENCE, RANDARRAY, TRANSPOSE, XLOOKUP of MMULT), een blok tussen accolades schrijft, een overloopbereik gebruikt (hieronder), of een rekenkundige bewerking op een bereik uitvoert, zoals in =A1:A3*2.

- **De cellen die het vult, hebben geen eigen formule.** Ze tonen de waarden. Wanneer u er een selecteert, toont de formulebalk in grijs de formule van de cel die hem vult. Enter drukken op een zo'n cel verandert niets. Om het blok te wijzigen, wijzigt u eerst die eerste cel.
- **Het blok volgt het resultaat.** Als de formule na een wijziging in wat hij leest meer of minder waarden geeft, groeit of krimpt het blok, en formules die die cellen lezen, worden opnieuw berekend.
- **Een cel in de weg geeft #SPILL!.** Als een cel waar het blok zou komen een waarde of een formule bevat, toont de eerste cel **#SPILL!** en wordt er niets gevuld. Maak de cel in de weg leeg en het blok vult direct. Typen in een cel die het blok vult, doet hetzelfde: de eerste cel toont #SPILL!.
- **Een blok heeft een maximale grootte.** Een blok dat groter is dan het blad, of meer dan 200.000 cellen telt, geeft #SPILL!.
- **A1# is het hele blok.** Schrijf de cel die de formule bevat en een **#**, zoals in =SOM(A1#), om het hele blok te gebruiken zoals het nu is, vanaf dit blad of een ander (=SOM(Blad1!A1#)). Als A1 niet overloopt, is A1# #VERW!.

Een overloop wordt met het bestand opgeslagen als dynamische matrix, zodat het programma waarin het bestand is gemaakt de overloop ook als overloop toont. Opslaan in de oudere .xls- of .ods-vorm bewaart de waarden, en de formule als matrixformule over het blok, waar de oudere vorm de gebruikte functies heeft. Compatibiliteit controleren noemt deze formules.

## Namen geven aan waarden in een formule

**LET** geeft een naam aan een waarde in het midden van een formule, zodat u hem één keer schrijft en opnieuw gebruikt. De paren van een naam en een waarde komen eerst, en de berekening komt als laatste. In =LET(prijs; B2; btw; prijs*0,08; prijs+btw) staat de naam **prijs** voor de waarde in B2, wordt **btw** daaruit berekend, en telt het laatste deel ze op. Een naam kan de namen ervoor gebruiken, en een LET kan maximaal 126 paren bevatten. Een naam werkt alleen binnen de eigen LET, en een LET binnen een andere mag dezelfde naam opnieuw gebruiken. Een naam kan ook voor een bereik van cellen staan, zoals in =LET(rijen; B2:B9; SOM(rijen)/AANTAL(rijen)).

## Iets vragen over een cel of de werkmap

**CEL** geeft informatie over een cel. Het eerste argument zegt welke: "adres", "rij", "kol", "inhoud", "type", "opmaak" (hoe getallen worden getoond, zoals "F2" voor twee decimalen, "P0" voor een percentage of "D1" voor een datum), "kleur" (1 wanneer negatieve getallen in kleur worden getoond), "haakjes" (1 wanneer positieve getallen tussen haakjes worden getoond), "voorvoegsel" (het teken van een tekst die links, rechts of gecentreerd is uitgelijnd), "beveiligd" (1 wanneer de cel vergrendeld is), "breedte" (de breedte van de kolom in tekens), en "bestandsnaam" (de map, het bestand en het blad, zoals /home/me/[Begroting.xlsx]Blad1; leeg tot de werkmap is opgeslagen). **INFO** geeft informatie over het programma en de werkmap: "directory", "numbestand" (hoeveel bladen er worden getoond), "oorsprong", "osversie", "herberekenen" (Automatisch of Handmatig), "release" en "systeem".

## Waarden uit XML halen

**FILTERXML** neemt tekst die XML bevat en een XPath, een korte manier om aan te geven welke delen u wilt, en geeft de waarden die het vindt. =FILTERXML(A1; "//item/prijs") geeft elke prijs in de tekst in A1, de ene onder de andere. Gebruik "/winkel/item[2]/naam" voor het tweede item, "//item[@id='b']/naam" voor het item waarvan de id b is, of "count(//item)" voor het aantal. Als de tekst geen geldige XML is, of het pad geen geldig pad is, is het antwoord #WAARDE!. Als niets overeenkomt, is het #N/B. XML met een documenttype wordt geweigerd.
