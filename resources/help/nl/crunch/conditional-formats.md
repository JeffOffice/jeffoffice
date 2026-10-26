# Voorwaardelijke opmaak
<!-- keywords: voorwaardelijke opmaak, opmaakregel, regel, markeren, kleurenschaal, gegevensbalk, pictogrammenset, pictogrammen, hoogste, laagste, boven gemiddelde, onder gemiddelde, dubbele waarden, unieke waarden, formule, stoppen indien waar, prioriteit, balken, pijlen, verkeerslichten, kleurovergang, differentiële opmaak -->

**Voorwaardelijke opmaak** is een regel die het uiterlijk van een cel wijzigt wanneer de inhoud ervan aan een voorwaarde voldoet. Voorbeeld: een regel zegt "maak een cel rood als de waarde kleiner is dan 100". Een cel met 80 wordt rood, een cel met 120 niet, en als u 80 wijzigt in 150, verdwijnt het rood vanzelf. De eigen opmaak van de cel blijft ongewijzigd: de regel bepaalt alleen wat er wordt weergegeven.

## Wat JeffCrunch weergeeft

JeffCrunch geeft de regels van een werkmap weer:

- **Markeringsregels**: een regel die het lettertype, de opvulling, de rand of de getalnotatie wijzigt van de cellen die eraan voldoen. De voorwaarde kan zijn: een vergelijking met een waarde (groter dan, tussen, gelijk aan, enzovoort), een formule, tekst die bepaalde woorden bevat, ermee begint of erop eindigt, een datum zoals "vorige week", cellen die leeg zijn of een fout bevatten, waarden die tot de hoogste of laagste behoren (een aantal of een percentage), waarden boven of onder het gemiddelde, en dubbele of unieke waarden.
- **Kleurenschalen**: elke cel krijgt een opvulkleur die tussen twee of drie kleuren verloopt naarmate het getal van laag naar hoog gaat. Bij een schaal van wit naar groen zijn de grootste getallen het groenst.
- **Gegevensbalken**: een balk in de cel die langer is naarmate het getal groter is. Balken kunnen een effen kleur hebben of vervagen, een rand en een as hebben, en lopen bij negatieve getallen de andere kant op, in een andere kleur.
- **Pictogrammensets**: een klein pictogram in de cel dat aangeeft in welk deel van het bereik het getal valt, zoals een pijl omhoog, een pijl opzij of een pijl omlaag, of een rood, geel of groen licht. Er zijn twintig sets.

Als twee regels iets zeggen over dezelfde cel, wint de regel die als eerste komt in de volgorde van de werkmap voor wat ze allebei instellen, en een regel met "stoppen indien waar" beëindigt de zoektocht. De getallen in een kleurenschaal, gegevensbalk of pictogrammenset worden berekend over de eigen cellen van de regel, en opnieuw berekend als een van die cellen wijzigt.

## Wat hetzelfde blijft

- Regels volgen de cellen waar ze bij horen. Als u rijen of kolommen invoegt of verwijdert, of knipt en plakt, gelden de regels nog steeds voor de juiste cellen; met **Ongedaan maken** zet u ze terug.
- Een regel die JeffCrunch niet weergeeft, blijft ongewijzigd in het bestand bewaard.
- Bij het opslaan worden de regels van de werkmap ongewijzigd opgeslagen.

## Wat nog niet mogelijk is

**U kunt nog geen regels maken, wijzigen of verwijderen.** JeffCrunch heeft geen opdracht **Voorwaardelijke opmaak**: het programma geeft weer wat de werkmap al bevat en houdt dat correct tijdens uw bewerkingen. Het bewerken van regels komt in een latere versie. Maak of wijzig regels tot die tijd in het programma waarmee de werkmap is gemaakt.
