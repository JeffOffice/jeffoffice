# Hoe formules worden weergegeven en berekend
<!-- keywords: formule, formules, berekenen, berekening, opnieuw berekenen, herberekenen, functie, functies, waarde, resultaat, opgeslagen, F9, kringverwijzing, circulair, formules weergeven, formulebalk, fout, foutmelding -->

Een **formule** is een cel die haar waarde berekent uit andere cellen, zoals =SOM(B3:B8). De cel geeft de waarde weer en de **formulebalk** geeft de formule weer.

## Wat JeffCrunch ermee doet

Wanneer u een werkmap opent, bevat het bestand de laatste waarde van elke formule, zodat het blad meteen kan worden weergegeven. Daarna berekent JeffCrunch elke formule opnieuw, een paar tegelijk, zodat het venster snel blijft reageren. De statusbalk geeft intussen **Berekenen** weer met een percentage. Als dat klaar is, bevatten de cellen de eigen resultaten van JeffCrunch. Als u alles opnieuw wilt berekenen, klikt u op het tabblad **Formules** op **Alles opnieuw berekenen** of drukt u op **F9**.

## Als JeffCrunch een formule niet kan berekenen

JeffCrunch heeft meer dan 200 functies, en dat aantal groeit nog. Een formule met een functie die JeffCrunch nog niet heeft, behoudt de waarde die het bestand ervoor heeft opgeslagen. Hetzelfde geldt voor een formule die naar een andere werkmap verwijst. Cellen die daarvan afhangen, worden berekend op basis van de opgeslagen waarden.

## De formules weergeven

Klik op het tabblad **Formules** op **Formules weergeven** of druk op **Ctrl+`** (op een Mac de Control-toets, niet de Command-toets) om in elke cel de formule te zien in plaats van de waarde.

## Formules wijzigen

Als u een formule wilt wijzigen, selecteert u de cel, drukt u op **F2**, bewerkt u de formule en drukt u op **Enter**. U kunt de formule ook in de formulebalk bewerken. Wanneer u een cel wijzigt, worden de formules die ervan afhangen op de achtergrond opnieuw berekend. Zie [Typen in cellen](edit-cells).

## Een formule schrijven

Typ **=** en de eerste letters van een functie, en er verschijnt een korte lijst met de functies en [namen](names) die zo beginnen, elk met een korte omschrijving. Druk op **Tab** om de gemarkeerde te kiezen. Gebruik **Pijl-omlaag** en **Pijl-omhoog** om een andere te markeren en druk dan op **Tab** of **Enter**. Met **Esc** sluit u de lijst. Zodra u tussen de haakjes van een functie staat, worden de argumenten onder de cel weergegeven, met het argument dat u typt in het vet. De lijst en de argumenten zijn dezelfde als bij **Functie kiezen…**. Argumenten worden gescheiden door komma's, of door puntkomma's als uw taal een komma als decimaalteken gebruikt.

## Handmatig berekenen

Klik op het tabblad **Formules** op **Berekeningsmodus** om te kiezen hoe de werkmap de formules berekent:

- **Automatisch** berekent ze terwijl u cellen wijzigt.
- **Automatisch, behalve tabellen** doet hetzelfde en wordt in het bestand bewaard voor programma's die gegevenstabellen apart behandelen.
- **Handmatig** berekent niets totdat u erom vraagt. Na een wijziging geeft de statusbalk **Verouderd** weer. Druk op **F9** (**Alles opnieuw berekenen**) om alles te berekenen, of op **Shift+F9** (**Dit blad opnieuw berekenen**) om de formules van het huidige blad te berekenen, plus alles wat verder nog wachtte. Een werkmap die op Handmatig staat, wordt geopend met de resultaten die in het bestand staan.

De keuze wordt met de werkmap opgeslagen en is één stap die u ongedaan kunt maken.

## Formules die naar zichzelf verwijzen

Een formule die haar eigen resultaat leest, rechtstreeks of via andere cellen, kan niet op de gewone manier worden berekend. JeffCrunch meldt dit één keer, noemt de cellen en geeft ze op het blad een omtrek; ze behouden hun eerdere waarden. Klik op het tabblad **Formules** op **Kringverwijzingen** voor de lijst en dubbelklik op een cel in de lijst om ernaartoe te gaan.

Sommige modellen hebben zo'n kring bewust nodig. Kies **Kringformules iteratief berekenen…** in het menu **Berekeningsmodus**. De formules worden dan steeds opnieuw berekend, waarbij elke formule de nieuwste waarden van de andere leest, totdat geen waarde meer verandert dan de opgegeven hoeveelheid of het toegestane aantal iteraties is bereikt.

## Zien wat een formule leest

Selecteer een cel en klik op **Weergeven welke cellen deze cel beïnvloeden** om pijlen te tekenen vanaf de cellen die de formule leest. Klik nogmaals om nog een stap verder terug te gaan. **Weergeven welke cellen deze cel gebruiken** tekent pijlen naar de formules die de cel lezen, en elke klik gaat een stap verder. Een kleine pagina staat voor een cel op een ander blad. Er worden maximaal 2.000 pijlen getekend; de statusbalk meldt het als het er meer waren. **Pijlen wissen** wist de pijlen, net als elke wijziging in de werkmap.
