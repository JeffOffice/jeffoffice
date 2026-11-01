# Hoe formules worden weergegeven en berekend
<!-- keywords: formule, formules, berekenen, berekening, herberekenen, opnieuw berekenen, functie, functies, waarde, resultaat, opgeslagen, F9, kringverwijzing, kringformule, formules weergeven, formulebalk, fout, foutmelding -->

Een **formule** is een cel die haar waarde berekent uit andere cellen, zoals =SOM(B3:B8). De cel toont de waarde en de **formulebalk** toont de formule.

## Wat JeffCrunch met formules doet

Wanneer u een werkmap opent, bevat het bestand de laatste waarde van elke formule, zodat het blad meteen kan worden weergegeven. Daarna berekent JeffCrunch elke formule opnieuw, telkens een paar tegelijk, zodat het venster snel blijft reageren. Ondertussen toont de statusbalk **Berekenen** met een percentage. Daarna bevatten de cellen de eigen resultaten van JeffCrunch. Als u alles opnieuw wilt berekenen, klikt u op **Alles opnieuw berekenen** op het tabblad **Formules** of drukt u op **F9**.

## Wanneer JeffCrunch een formule niet kan berekenen

JeffCrunch heeft meer dan 200 functies en dat aantal groeit nog. Een formule met een functie die JeffCrunch nog niet heeft, behoudt de waarde die in het bestand is opgeslagen. Hetzelfde geldt voor een formule die naar een andere werkmap verwijst. Cellen die daarvan afhangen, worden berekend op basis van de opgeslagen waarden.

## Formules weergeven

Klik op **Formules weergeven** op het tabblad **Formules** of druk op **Ctrl+`** (op een Mac de Control-toets, niet de Command-toets) om in elke cel de formule te zien in plaats van de waarde. Kolommen met formules worden breder gemaakt, tot een bepaalde grens, zodat de formules volledig zichtbaar zijn. Wanneer u dit uitschakelt, krijgen ze weer hun oorspronkelijke breedte; de kolombreedten in uw bestand worden niet gewijzigd.

## Formules wijzigen

Als u een formule wilt wijzigen, selecteert u de cel, drukt u op **F2**, bewerkt u de formule en drukt u op **Enter**. U kunt de formule ook in de formulebalk bewerken. Wanneer u een cel wijzigt, worden de formules die ervan afhangen op de achtergrond opnieuw berekend. Zie [Typen in cellen](edit-cells).

## Een formule schrijven

Typ **=** en de eerste letters van een functie: er verschijnt een korte lijst met de functies en [namen](names) die zo beginnen, met bij elk een korte beschrijving. Druk op **Tab** om de gemarkeerde te kiezen. Gebruik **Pijl-omlaag** en **Pijl-omhoog** om een andere te markeren en druk dan op **Tab** of **Enter**. Met **Esc** sluit u de lijst. Zodra u binnen de haakjes van een functie bent, worden de argumenten onder de cel weergegeven, met het argument dat u typt vetgedrukt. De lijst en de argumenten zijn dezelfde als in **Functie kiezen**. Argumenten worden gescheiden door komma's, of door puntkomma's als uw taal een komma als decimaalteken gebruikt.

## Gegevenstabellen

Een gegevenstabel (wat-als-tabel) laat zien wat een formule oplevert voor veel waarden van één of twee cellen. Open een werkmap met een gegevenstabel en JeffCrunch berekent de cellen ervan: bij een tabel met twee invoerwaarden staat de formule in de cel in de linkerbovenhoek van de tabel, gaan de waarden in de bovenste rij naar de rijinvoercel en gaan de waarden in de linkerkolom naar de kolominvoercel. Bij een tabel met één invoerwaarde staan de waarden in de kolom links (en de formules in de rij erboven) of in de rij erboven (en de formules in de kolom links). Elke cel van de tabel is het resultaat van de formule, en van elke formule die van de invoercel afhangt, met de invoer ingesteld op die waarde; verder wordt niets in de werkmap gewijzigd. In de modus **Automatisch** worden de tabellen samen met de formules berekend, en daarna de formules die cellen van een tabel lezen. Een nieuwe gegevenstabel maken is nog niet mogelijk. Een tabel die niet kan worden berekend (omdat een formule die van de invoer afhangt een functie gebruikt die JeffCrunch niet heeft, of vanzelf verandert, zoals ASELECT of NU) behoudt de waarden uit het bestand.

## Handmatig berekenen

Klik op **Berekeningsmodus** op het tabblad **Formules** om te kiezen hoe de werkmap formules berekent:

- **Automatisch** berekent formules zodra u cellen wijzigt.
- **Automatisch, behalve tabellen** doet hetzelfde, maar laat de cellen van gegevenstabellen ongewijzigd totdat u op **F9** drukt.
- **Handmatig** berekent niets, ook geen gegevenstabellen, totdat u erom vraagt. Na een wijziging toont de statusbalk **Verouderd**. Druk op **F9** (**Alles opnieuw berekenen**) om alles te berekenen, of op **Shift+F9** (**Dit blad opnieuw berekenen**) om de formules van het huidige blad te berekenen, plus alles wat verder nog wachtte. Een werkmap die op Handmatig staat, wordt geopend met de resultaten die in het bestand staan.

De keuze wordt met de werkmap opgeslagen en kan in één stap ongedaan worden gemaakt.

## Formules die naar zichzelf verwijzen

Een formule die haar eigen resultaat leest, rechtstreeks of via andere cellen, kan niet op de gewone manier worden berekend. JeffCrunch meldt dit één keer, noemt de cellen en geeft ze een omtrek op het blad; ze behouden hun oude waarden. Klik op **Kringverwijzingen** op het tabblad **Formules** voor de lijst en dubbelklik op een cel in de lijst om ernaartoe te gaan.

Sommige modellen hebben zo'n kring bewust nodig. Kies **Kringformules iteratief berekenen** in het menu **Berekeningsmodus**: de formules worden dan steeds opnieuw berekend, waarbij elke formule de nieuwste waarden van de andere leest, totdat geen waarde meer verandert dan de opgegeven hoeveelheid of het toegestane aantal iteraties is bereikt.

## Bekijken wat een formule leest

Selecteer een cel en klik op **Weergeven welke cellen deze cel beïnvloeden** om pijlen te tekenen vanaf de cellen die de formule leest. Klik nogmaals om nog een stap verder terug te gaan. **Weergeven welke cellen deze cel gebruiken** tekent pijlen naar de formules die de cel lezen; met elke klik gaat u een stap verder. Een klein paginasymbool staat voor een cel op een ander blad. Er worden maximaal 2.000 pijlen getekend; de statusbalk meldt het als het er meer waren. Met **Pijlen wissen** verwijdert u ze; dat gebeurt ook bij elke wijziging in de werkmap.
