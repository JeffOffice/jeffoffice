# Knippen, kopiëren en plakken
<!-- keywords: knippen, kopiëren, plakken, klembord, verplaatsen, dupliceren, plakken speciaal, waarden, formules, opmaak, transponeren, lege cellen overslaan, optellen, aftrekken, vermenigvuldigen, delen, bewerking, samengevoegd, tekst, tab, html, webpagina, ander programma, Esc, bewegende rand, rand, kopie -->

**Kopiëren** (**{key:Copy}**) zet een kopie van de geselecteerde cellen op het klembord, een plek waar de computer telkens één ding bewaart. **Plakken** (**{key:Paste}**) zet wat op het klembord staat in de cellen, te beginnen bij de geselecteerde cel. **Knippen** (**{key:Cut}**) werkt als kopiëren, maar bij het plakken worden de cellen verplaatst en wordt de oude plek leeggemaakt.

Een bewegende stippellijn geeft aan welke cellen zijn gekopieerd of geknipt. Druk op **Esc** om de rand weg te halen; de cellen zijn dan niet meer klaar om te plakken. Geknipte cellen worden ook losgelaten als u iets anders wijzigt voordat u ze plakt (typen, invoegen, ongedaan maken): knip ze opnieuw als u ze nog wilt verplaatsen.

## Plakken binnen JeffCrunch

Alles gaat mee: de waarden, de formules, de opmaak en de samengevoegde cellen. Een formule blijft verwijzen naar dezelfde plaatsen *ten opzichte van waar ze staat*. Voorbeeld: kopieer **=A1*2** uit B1 en plak deze in B5, en de formule wordt **=A5*2**. Een deel met een dollarteken, zoals **$A$1**, blijft zoals het is. Als u een blok selecteert dat een geheel aantal keren zo groot is als wat u hebt gekopieerd, wordt de kopie herhaald om het blok te vullen: kopieer één cel, selecteer twintig cellen, plak, en alle twintig krijgen de inhoud.

Een geplakt getal dat te breed is voor een kolom die nog haar oorspronkelijke breedte heeft, maakt de kolom breder.

## Knippen verplaatst cellen

Als u cellen knipt en plakt, worden de cellen echt verplaatst. Elke formule die naar die cellen verwees, verwijst ernaar op hun nieuwe plek. Voorbeeld: B1 bevat **=A1+A2**. Knip A1:A2 en plak in E4, en B1 wordt **=E4+E5**. Wat er stond in de cellen waaroverheen u plakt, verdwijnt (**Ongedaan maken** haalt het terug), en een formule die een van die cellen las, geeft **#VERW!** weer, omdat de cel die ze las er niet meer is. Als de geknipte cellen maar een deel van een samengevoegde cel zijn, of als u over een deel ervan plakt, wordt er niets verplaatst: selecteer dan de hele samengevoegde cel. Als u cellen naar een ander blad knipt, blijven de formules de cellen lezen die ze eerder lazen (ze vermelden welk blad). Na knippen en plakken wordt het klembord leeggemaakt.

## Plakken speciaal

Met **Ctrl+Alt+V**, of met de pijl onder **Plakken** op het tabblad Start, kiest u wat u wilt plakken:

- **Alles**, **Formules**, **Waarden** of **Opmaak**. *Waarden* zet een formule om in het getal of de tekst die ze weergeeft.
- Een **bewerking**: **Optellen**, **Aftrekken**, **Vermenigvuldigen** of **Delen**. De gekopieerde getallen worden gecombineerd met de getallen die al in de cellen staan. Voorbeeld: de cellen bevatten 10 en 20, u hebt 5 gekopieerd en u kiest *Vermenigvuldigen*: ze worden 50 en 100.
- **Lege cellen overslaan**: lege cellen in de kopie laten de cellen waarop ze terechtkomen ongewijzigd.
- **Transponeren**: rijen worden kolommen en kolommen worden rijen.

## Tussen programma's

Bij het kopiëren zet JeffCrunch de cellen ook als tekst op het klembord (cellen gescheiden door tabs, één rij per regel) en als tabel voor webpagina's, zodat andere programma's ze kunnen overnemen. Als u iets plakt dat niet uit JeffCrunch komt, zoals tekst met tabs of een tabel uit een webpagina of een andere spreadsheet, komt elk deel in een eigen cel en wordt het gelezen alsof u het hebt getypt: getallen, datums en formules die met een isgelijkteken beginnen. Kopiëren uit een andere werkmap die in JeffCrunch is geopend, neemt waarden, formules en getalnotaties mee, maar niet de overige opmaak.

## Wat niet kan worden geplakt

JeffCrunch meldt het en wijzigt niets als het plakken zou terechtkomen in een deel van een **matrixformule**, over de rij met kolomnamen van een **tabel**, in vergrendelde cellen van een **beveiligd blad**, voorbij het einde van het blad of in meer dan 100.000 cellen. Een hele **matrixformule** kan worden geknipt en verplaatst (selecteer al haar cellen); een deel ervan, een gegevenstabel of de rij met namen van een tabel niet.
