# Hoe formules worden weergegeven en berekend
<!-- keywords: formule, formules, berekenen, berekening, opnieuw berekenen, herberekenen, functie, functies, waarde, resultaat, opgeslagen, F9, kringverwijzing, kringformule, formules weergeven, formulebalk, fout, foutwaarde, aanwijzen, cel aanklikken, celverwijzing, verwijzing, F4, absolute verwijzing -->

Een **formule** is een cel die haar waarde berekent uit andere cellen, zoals =SOM(B3:B8). De cel geeft de waarde weer en de **formulebalk** geeft de formule weer.

## Wat JeffCrunch met formules doet

Wanneer u een werkmap opent, bevat het bestand de laatste waarde van elke formule, zodat het blad meteen kan worden weergegeven. Daarna berekent JeffCrunch elke formule opnieuw, een paar tegelijk, zodat het venster snel blijft reageren. Ondertussen staat in de statusbalk **Berekenen** met een percentage. Daarna bevatten de cellen de eigen resultaten van JeffCrunch. Als u alles opnieuw wilt berekenen, klikt u op het tabblad **Formules** op **Alles opnieuw berekenen**, of drukt u op **F9**.

## Als JeffCrunch een formule niet kan berekenen

JeffCrunch heeft meer dan 200 functies, en dat aantal groeit nog. Een formule die een functie gebruikt die JeffCrunch nog niet heeft, behoudt de waarde die in het bestand is opgeslagen. Dat geldt ook voor een formule die naar een andere werkmap verwijst. Cellen die daarvan afhangen, worden berekend op basis van de opgeslagen waarden.

## REST: de rest van een deling

**REST(n; d)** geeft wat er van **n** overblijft nadat er zo veel mogelijk hele keren **d** van is afgehaald: het is n - d * INTEGER(n / d). Voorbeeld: **=REST(10; 3)** is 1, en **=REST(-10; 3)** is 2, omdat INTEGER(-10 / 3) gelijk is aan -4 en -10 - 3 * -4 gelijk is aan 2. Het antwoord heeft het teken van **d**. Als **d** 0 is, is het antwoord #DEEL/0!.

## Formules weergeven

Klik op het tabblad **Formules** op **Formules weergeven**, of druk op **Ctrl+`** (op een Mac de Control-toets, niet de Command-toets), om in elke cel de formule te zien in plaats van de waarde. Kolommen met formules worden breder gemaakt, tot een bepaalde grens, zodat de formules helemaal zichtbaar zijn. Als u de weergave uitschakelt, krijgen de kolommen weer hun eigen breedte; de breedten in uw bestand worden niet gewijzigd.

## Formules wijzigen

Als u een formule wilt wijzigen, selecteert u de cel, drukt u op **F2**, bewerkt u de formule en drukt u op **Enter**. U kunt de formule ook in de formulebalk bewerken. Wanneer u een cel wijzigt, worden de formules die ervan afhangen op de achtergrond opnieuw berekend. Zie [Typen in cellen](edit-cells).

## Een formule schrijven

Typ **=** en de eerste letters van een functie: er verschijnt een korte lijst met de functies en [namen](names) die zo beginnen, met bij elk een korte beschrijving. Druk op **Tab** om de gemarkeerde te kiezen. Gebruik **Pijl-omlaag** en **Pijl-omhoog** om een andere te markeren en druk daarna op **Tab** of **Enter**. Met **Esc** sluit u de lijst. Zodra u tussen de haakjes van een functie staat, worden de argumenten onder de cel weergegeven, met het argument dat u aan het typen bent vet. De lijst en de argumenten zijn dezelfde als in **Functie kiezen**. Argumenten worden gescheiden door komma's, of door puntkomma's als uw taal een komma als decimaalteken gebruikt, zoals in het Nederlands.

## Cellen aanwijzen

In plaats van het adres van een cel te typen, kunt u de cel aanwijzen. Zet tijdens het typen van een formule de cursor op een plek waar een cel kan staan (direct na **=**, na een operator zoals **+**, na een haakje openen of na een scheidingsteken) en klik op de cel. Het adres wordt dan voor u geschreven. Sleep over cellen om een bereik te schrijven, zoals B2:D5. Klikt u nogmaals voordat u iets anders typt, dan vervangt de nieuwe cel de zojuist geschreven cel. Houd **Ctrl** ingedrukt en klik om nog een adres toe te voegen na een scheidingsteken. Klik op een kolomletter om een hele kolom te schrijven, zoals A:A, of op een rijnummer voor een hele rij, zoals 3:3, en sleep langs de letters of nummers voor B:D of 2:5.

De pijltoetsen werken ook. Druk op een pijltoets: er beweegt een aanwijzer weg van de cel waarin u typt en het adres ervan wordt geschreven. Druk op meer pijltoetsen om verder te gaan, of houd **Shift** ingedrukt om de aanwijzer over een bereik uit te rekken. In de statusbalk staat **Aanwijzen** wanneer de pijltoetsen cellen aanwijzen, **Invoeren** terwijl u typt en **Bewerken** wanneer de pijltoetsen de cursor in de tekst verplaatsen. Druk op **F2** om te wisselen tussen aanwijzen en tekst bewerken.

Als u een cel op een ander blad wilt aanwijzen, klikt u op de tab van dat blad terwijl de cursor op een plek staat waar een cel kan staan, en klikt u daarna op de cel. De naam van het blad wordt erbij geschreven, zo nodig tussen aanhalingstekens, zoals in 'Mijn blad'!B3. Met **Enter** gaat u terug naar het blad waarop u begon en komt de formule in haar cel. Met **Esc** maakt u alles ongedaan, ook het wisselen van blad.

U kunt ook de rand van een omkaderd bereik op het blad slepen om dat adres te verplaatsen, of het vierkantje in de hoek slepen om het uit te rekken; de formule verandert terwijl u sleept.

Druk op **F4** met de cursor in of direct na een adres om delen ervan vast te zetten met een dollarteken: A1 wordt $A$1, daarna A$1, daarna $A1 en daarna weer A1.

Elk adres in de formule krijgt een kleur in de tekst, en de cellen die het aanduidt, worden op het blad in dezelfde kleur omkaderd. Zo ziet u in één oogopslag wat de formule leest. Klikt u na een volledig adres, zoals =A1, dan wordt de invoer beëindigd en wordt de cel geselecteerd waarop u klikt, net als bij andere tekst.

## Gegevenstabellen

Een gegevenstabel (wat-als-tabel) laat zien wat een formule oplevert voor veel waarden van een of twee cellen. Open een werkmap met een gegevenstabel en JeffCrunch berekent de cellen ervan: bij een tabel met twee invoerwaarden staat de formule in de linkerbovencel van de tabel, gaan de horizontale waarden naar de rij-invoercel en de verticale waarden naar de kolominvoercel. Bij een tabel met één invoerwaarde staan de waarden onder elkaar in de kolom links (en de formules in de rij erboven) of naast elkaar in de rij erboven (en de formules in de kolom links). Elke cel van de tabel is het resultaat van de formule, en van elke formule die van de invoercel afhangt, met de invoer op die waarde; verder wordt niets in de werkmap gewijzigd. In de modus **Automatisch** worden de tabellen samen met de formules berekend, en volgen de formules die cellen van een tabel lezen. Een nieuwe gegevenstabel maken is nog niet mogelijk. Een tabel die niet kan worden berekend (omdat een formule die van de invoer afhangt een functie gebruikt die JeffCrunch niet heeft, of vanzelf verandert, zoals ASELECT of NU) behoudt de waarden uit het bestand.

## Handmatig berekenen

Klik op het tabblad **Formules** op **Berekeningsmodus** om te kiezen hoe de werkmap haar formules berekent:

- **Automatisch** berekent ze terwijl u cellen wijzigt.
- **Automatisch, behalve tabellen** doet hetzelfde, maar laat de cellen van gegevenstabellen ongewijzigd totdat u op **F9** drukt.
- **Handmatig** berekent niets, ook geen gegevenstabellen, totdat u erom vraagt. Na een wijziging staat in de statusbalk **Verouderd**. Druk op **F9** (**Alles opnieuw berekenen**) om alles te berekenen, of op **Shift+F9** (**Dit blad opnieuw berekenen**) om de formules van het huidige blad te berekenen, plus alles wat nog wachtte. Een werkmap die op Handmatig staat, wordt geopend met de resultaten uit het bestand.

De keuze wordt met de werkmap opgeslagen en is één stap die u ongedaan kunt maken.

## Formules die zichzelf lezen

Een formule die haar eigen resultaat leest, direct of via andere cellen, kan niet op de gewone manier worden berekend. JeffCrunch meldt dat één keer, noemt de cellen en omkadert ze op het blad; ze behouden hun vorige waarden. Klik op het tabblad **Formules** op **Kringverwijzingen** voor de lijst en dubbelklik op een cel in de lijst om erheen te gaan.

Sommige modellen hebben zo'n kring juist nodig. Kies in het menu **Berekeningsmodus** de optie **Kringformules iteratief berekenen**: de formules worden dan steeds opnieuw berekend, waarbij elke formule de nieuwste waarden van de andere leest, totdat geen waarde meer verandert dan de opgegeven hoeveelheid of het toegestane aantal iteraties is bereikt.

## Zien wat een formule leest

Selecteer een cel en klik op **Weergeven welke cellen deze cel beïnvloeden** om pijlen te tekenen vanaf de cellen die de formule leest. Klik nogmaals om nog een stap verder terug te gaan. **Weergeven welke cellen deze cel gebruiken** tekent pijlen naar de formules die de cel lezen, en elke klik gaat een stap verder. Een klein pagina-pictogram staat voor een cel op een ander blad. Er worden maximaal 2.000 pijlen getekend; de statusbalk meldt het als er meer waren. Met **Pijlen wissen** verwijdert u ze, en dat gebeurt ook bij elke wijziging in de werkmap.
