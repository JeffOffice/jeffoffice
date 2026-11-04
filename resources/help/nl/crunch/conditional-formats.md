# Voorwaardelijke opmaak
<!-- keywords: voorwaardelijke opmaak, voorwaardelijke notatie, regel, markeren, kleurenschaal, gegevensbalk, pictogramserie, pictogrammen, boven, onder, boven gemiddelde, onder gemiddelde, duplicaat, uniek, formule, stoppen als waar, prioriteit, balken, pijlen, verkeerslichten, verloop, afwijkende opmaak, knop Voorwaardelijke opmaak, regels wissen, groter dan, kleiner dan, tussen, tekst die bevat, dubbele waarden, top 10 -->

Een **voorwaardelijke opmaak** is een regel die bepaalt hoe een cel eruitziet, wanneer de inhoud van de cel aan een voorwaarde voldoet. Voorbeeld: een regel zegt "kleur een cel rood als deze lager is dan 100". Een cel met 80 is dan rood, een cel met 120 niet, en als u 80 verandert in 150, verdwijnt het rood vanzelf. De eigen opmaak van de cel wordt niet aangeraakt: de regel bepaalt alleen wat er wordt getekend.

## Wat JeffCrunch laat zien

JeffCrunch toont de regels die een werkmap heeft:

- **Markeringsregels**: een regel die het lettertype, de opvulling, de rand of de getalnotatie wijzigt van de cellen die eraan voldoen. De voorwaarde kan een vergelijking met een waarde zijn (groter dan, tussen, gelijk aan, enzovoort), een formule, tekst die bepaalde woorden bevat, begint met of eindigt op, een datum zoals "vorige week", cellen die leeg zijn of een fout bevatten, waarden die tot de hoogste of laagste behoren (een getal of een percentage), waarden boven of onder het gemiddelde, en waarden die dubbel of uniek voorkomen.
- **Kleurenschalen**: elke cel krijgt een kleur die tussen twee of drie kleuren beweegt, naarmate het getal van laag naar hoog gaat. Een schaal van wit naar groen maakt de grootste getallen het groenst.
- **Gegevensbalken**: een balk in de cel die langer wordt naarmate het getal groter is. Balken kunnen één effen kleur hebben of uitlopen, kunnen een rand hebben, kunnen een as hebben en kunnen voor negatieve getallen de andere kant op lopen, in een andere kleur.
- **Pictogramseries**: een klein plaatje in de cel dat aangeeft in welk deel van het bereik het getal valt, zoals een pijl omhoog, een pijl opzij of een pijl omlaag, of een rood, geel of groen stoplicht. Er zijn twintig series.

Als twee regels iets zeggen over dezelfde cel, wint de regel die in de volgorde van de werkmap het eerst komt, voor wat beide instellen. Een regel die is gemarkeerd als "stoppen als waar" beëindigt het zoeken. De getallen in een kleurenschaal, een gegevensbalk of een pictogramserie worden berekend over de eigen cellen van de regel, en opnieuw berekend zodra een van die cellen verandert.

## Wat hetzelfde blijft

- Regels volgen de cellen waarbij ze horen. Rijen of kolommen invoegen of verwijderen, knippen en plakken: de regels blijven de juiste cellen bestrijken; **Ongedaan maken** zet ze terug.
- Een regel die JeffCrunch niet tekent, blijft precies zoals hij was in het bestand bewaard.
- Bij opslaan worden de regels geschreven die de werkmap had. Wanneer u regels maakt of wist op een blad, worden de regels van dat blad opnieuw geschreven met uw wijzigingen.

## Een regel maken

1. Selecteer de cellen.
2. Klik op **Start > Voorwaardelijke opmaak**.
3. Kies onder **Regels voor celmarkering** de optie **Groter dan**, **Kleiner dan**, **Tussen**, **Gelijk aan**, **Tekst die bevat** of **Dubbele waarden**. Kies onder **Regels voor boven/onder** de optie **Top 10 items**, **Onderste 10 items**, **Top 10 %**, **Onderste 10 %**, **Boven gemiddelde** of **Onder gemiddelde**.
4. Typ de waarde (of de twee waarden voor **Tussen**, de tekst voor **Tekst die bevat**, of het aantal voor de regels Top en Onderste). Bij **Top 10 items** is "10" het aantal cellen dat de opmaak krijgt. U kunt dit in elk getal veranderen.
5. Kies hoe de cellen eruit moeten zien, bijvoorbeeld **Lichtrode vulling met donkerrode tekst**, en klik op **OK**.

De regel wordt meteen getekend. Elke nieuwe regel komt eerst, dus als twee regels hetzelfde willen wijzigen, wint de nieuwste. Het maken van een regel is één stap die u ongedaan kunt maken.

## Regels wissen

**Regels uit geselecteerde cellen wissen** verwijdert elke regel die op een van de geselecteerde cellen staat. **Regels uit dit blad wissen** verwijdert alle regels van het blad. Elk van beide is één stap die u ongedaan kunt maken. Een keuze doet niets wanneer er geen regel is om te wissen.

## Wat nog niet is gedaan

- Gegevensbalken, kleurenschalen en pictogramseries kunnen worden getoond, maar nog niet worden gemaakt. Het venster waarin alle regels worden weergegeven, gesorteerd en bewerkt (**Regels beheren**), is er nog niet. Een regel die een formule gebruikt, kan nog niet vanuit het menu worden gemaakt.
- Een blad met gegevensbalken in de vorm van een nieuwer bestand behoudt die balken zoals ze zijn, en JeffCrunch maakt of wist geen regels op dat blad. Breng die wijzigingen aan in het programma waarmee de werkmap is gemaakt.
- Een werkmap die is geopend uit een oudere bestandssoort, behoudt zijn regels wanneer u hem opslaat als werkmap in .xlsx-formaat, en daarna kunt u regels toevoegen.
