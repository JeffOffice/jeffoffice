# Voorwaardelijke opmaak

<!-- keywords: voorwaardelijke opmaak, voorwaardelijke opmaak toepassen, regel, markeren, kleurenschaal, gegevensbalk, pictogramserie, pictogrammen, boven, onder, boven gemiddelde, onder gemiddelde, dubbel, uniek, formule, stoppen als waar, prioriteit, balken, pijlen, verkeerslichten, kleurovergang, afwijkende opmaak, knop Voorwaardelijke opmaak, regels wissen, groter dan, kleiner dan, tussen, tekst die bevat, dubbele waarden, top 10 -->

Een **voorwaardelijke opmaak** is een regel die de weergave van een cel verandert wanneer de inhoud van de cel aan een voorwaarde voldoet. Voorbeeld: een regel zegt "kleur een cel rood als deze lager is dan 100". Een cel met 80 is rood, een cel met 120 is niet rood, en wanneer u 80 verandert in 150, verdwijnt de rode kleur vanzelf. De eigen opmaak van de cel wordt niet aangepast: de regel bepaalt alleen wat er wordt getekend.

## Wat JeffCrunch laat zien

JeffCrunch toont de regels die een werkmap heeft:

- **Regels voor celmarkering**: een regel die het lettertype, de opvulling, de rand of de getalnotatie wijzigt van de cellen die eraan voldoen. De voorwaarde kan een vergelijking met een waarde zijn (groter dan, tussen, gelijk aan, enzovoort), een formule, tekst die bepaalde woorden bevat, begint met of eindigt op, een datum zoals "vorige week", cellen die leeg zijn of een fout bevatten, waarden die tot de hoogste of laagste behoren (een getal of een percentage), waarden boven of onder het gemiddelde, en waarden die dubbel of uniek zijn.
- **Kleurenschalen**: elke cel krijgt een kleur die tussen twee of drie kleuren beweegt naarmate het getal van laag naar hoog gaat. Een schaal van wit naar groen maakt de grootste getallen het groenst.
- **Gegevensbalken**: een balk in de cel die langer wordt naarmate het getal groter is. Balken kunnen een effen kleur hebben of uitlopen, een rand hebben, een as tonen en voor negatieve getallen de andere kant op lopen, in een andere kleur.
- **Pictogramseries**: een klein pictogram in de cel dat aangeeft in welk deel van het bereik het getal valt, zoals een pijl omhoog, een zijwaartse pijl of een pijl omlaag; of een rood, geel of groen verkeerslicht. Er zijn twintig series.

Wanneer twee regels iets zeggen over dezelfde cel, wint de regel die in de volgorde van de werkmap het eerst komt voor wat ze allebei instellen, en een regel met het vinkje "Stoppen als waar" beëindigt het zoeken. De getallen in een kleurenschaal, een gegevensbalk of een pictogramserie worden geteld over de eigen cellen van de regel, en opnieuw geteld wanneer een van die cellen verandert.

## Wat hetzelfde blijft

- Regels volgen de cellen waartoe ze behoren. Voegt u rijen of kolommen in of verwijdert u ze, knipt en plakt u, dan dekken de regels nog steeds de juiste cellen; **Ongedaan maken** herstelt ze.
- Een regel die JeffCrunch niet tekent, blijft precies zoals hij was in het bestand bewaard.
- Bij opslaan worden de regels geschreven die de werkmap had. Wanneer u regels op een blad maakt of wist, worden de regels van het blad opnieuw geschreven met uw wijzigingen.

## Een regel maken

1. Selecteer de cellen. U kunt hele kolommen of het hele blad selecteren; een regel geldt voor het bereik, niet voor elke cel afzonderlijk.
2. Klik op **Start > Voorwaardelijke opmaak**.
3. Kies onder **Regels voor celmarkering** **Groter dan**, **Kleiner dan**, **Tussen**, **Gelijk aan**, **Tekst die bevat**, **Een datum die voorkomt** of **Dubbele waarden**. Kies onder **Regels voor boven- en onderwaarden** **Top 10 items**, **Onderste 10 items**, **Top 10 %**, **Onderste 10 %**, **Boven gemiddelde** of **Onder gemiddelde**.
4. Typ de waarde (of de twee waarden bij **Tussen**, de tekst bij **Tekst die bevat**, of het aantal bij de regels voor boven- en onderwaarden), of kies de datums. Bij **Top 10 items** is "10" het aantal cellen dat de opmaak krijgt. U kunt dit in elk getal wijzigen.
5. Kies hoe de cellen eruit moeten zien, bijvoorbeeld **Lichtrode vulling met donkerrode tekst**, en klik op **OK**. Het voorbeeld naast de lijst toont de opmaak.

Om een eigen opmaak te maken, kiest u onderaan de lijst **Aangepaste opmaak...**. Er opent een venster met vier tabbladen: **Getal**, **Lettertype** (de stijl, onderstreping, doorhaling en kleur), **Rand** (alleen dunne lijnsoorten) en **Opvulling** (een kleur, of een patroon met twee kleuren). Het venster begint met de opmaak die u had gekozen, en een onderdeel dat u niet wijzigt, blijft zoals het was. Klik op **OK** en het voorbeeld toont uw opmaak.

De regel wordt meteen getekend. Elke nieuwe regel komt bovenaan, zodat wanneer twee regels hetzelfde willen wijzigen, de nieuwere wint. Het maken van een regel is één stap die u ongedaan kunt maken.

## Gegevensbalken, kleurenschalen en pictogramseries

Klik op **Start > Voorwaardelijke opmaak**, kies vervolgens **Gegevensbalken**, **Kleurenschalen** of **Pictogramseries** en klik op een keuze. De regel wordt meteen op de geselecteerde cellen toegepast, zonder venster om in te vullen, als één stap die u ongedaan kunt maken.

- **Gegevensbalken** zijn in zes kleuren beschikbaar, elk als kleurovergang of als effen opvulling.
- **Kleurenschalen** heeft twaalf keuzes, met twee of drie kleuren. De naam vermeldt eerst de kleur van de hoogste getallen.
- **Pictogramseries** heeft twintig series: richtingen (pijlen en driehoeken), vormen (verkeerslichten en borden), indicatoren (symbolen en vlaggen) en beoordelingen (sterren, kwarten en vakjes). De pictogrammen verdelen het bereik in gelijke stappen.

**Nieuwe regel** onderaan het menu opent één venster voor elke soort regel, ook voor een regel met uw eigen formule, en voor een gegevensbalk, kleurenschaal of pictogramserie met eigen instellingen.

## Regels beheren

**Start > Voorwaardelijke opmaak > Regels beheren** toont de regels van het blad in de volgorde waarin ze werken. Gebruik **Nieuwe regel**, **Regel bewerken** en **Regel verwijderen**; met **Omhoog verplaatsen** en **Omlaag verplaatsen** wijzigt u de volgorde; typ andere cellen bij **Van toepassing op**; en vink **Stoppen als waar** aan. Met Regel bewerken wijzigt u alles wat een regel heeft:

- voor een **gegevensbalk**: wat de kortste en langste balk betekenen (laagste of hoogste waarde, een getal, een percentage, een percentiel of een formule), kleurovergang of effen opvulling, de kleuren, een rand, de kleur van negatieve balken, waar de as staat, de richting van de balken, en **Alleen balk weergeven**;
- voor een **kleurenschaal**: twee of drie kleuren, en wat elke kleur betekent;
- voor een **pictogramserie**: de pictogramstijl, de waarde waarbij elk pictogram begint, **Pictogramvolgorde omkeren**, en **Alleen pictogram weergeven**;
- voor de overige regels: de waarde, tekst, datums of formule, en de opmaak. Een regel die een bestand met een eigen opmaak heeft meegebracht, staat in de lijst met de opmaak **Aangepaste opmaak**, en het voorbeeld toont hoe die er werkelijk uitziet. Als u iets anders aan de regel wijzigt, blijft de opmaak precies zoals het bestand die had. Kies **Aangepaste opmaak...** om de opmaak te wijzigen.

Met **OK** past u alle wijzigingen in één stap toe die u ongedaan kunt maken. Een regel die u niet hebt gewijzigd, behoudt precies wat het bestand had, ongeacht de plaats in de lijst; een regel die u wijzigt, wordt opnieuw geschreven. De formule van een regel wordt gelezen vanuit de linkerbovencel van het bereik, en een formule die u voor een nieuwe regel typt, wordt gelezen alsof deze in de cel met de cursor staat. Een beveiligd blad waarop het opmaken van cellen niet is toegestaan, neemt geen regels over.

## Wat andere bestandssoorten bewaren

Alleen een .xlsx-werkmap bewaart voorwaardelijke opmaak. Opslaan als een oudere bestandssoort (.xls of .ods) laat ze weg, en Compatibiliteit controleren meldt dit, met de vermelding van gegevensbalken, kleurenschalen en pictogramseries als het blad die bevat. Een .csv-bestand bewaart helemaal geen opmaak. Voorwaardelijke opmaak wordt afgedrukt zoals ze op het scherm te zien is.

## Regels wissen

**Regels uit geselecteerde cellen wissen** verwijdert elke regel die op een van de geselecteerde cellen staat. **Regels uit dit blad wissen** verwijdert alle regels van het blad. **Regels uit deze tabel wissen** en **Regels uit deze draaitabel wissen** verwijderen de regels die op de tabel (of draaitabel) staan waarin de geselecteerde cellen zich bevinden, en zijn alleen beschikbaar wanneer de selectie in één tabel staat. Elk van deze opdrachten is één stap die u ongedaan kunt maken. Een keuze doet niets wanneer er geen regel is om te wissen.

## Wat nog niet is ondersteund

- Een blad met gegevensbalken in de vorm van een nieuwer bestand behoudt die balken zoals ze zijn, en JeffCrunch maakt, wijzigt of wist geen regels op dat blad. Breng die wijzigingen aan in het programma waarmee de werkmap is gemaakt.
- Een werkmap die is geopend uit een oudere bestandssoort, behoudt zijn regels wanneer u hem opslaat als .xlsx-werkmap, en daarna kunt u regels toevoegen.
- Een eigen opmaak kan geen middelmatige, dikke, dubbele of schuine lijn hebben, geen diagonale lijn, geen kleurovergang als opvulling, en geen lettertype of lettergrootte: die kan een regel niet wijzigen.
