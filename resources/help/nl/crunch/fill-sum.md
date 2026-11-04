# Cellen doorvoeren en optellen
<!-- keywords: doorvoeren, reeks, slepen, greep, dubbelklikken, aangepaste lijst, AutoAanvullen, Ctrl+D, Ctrl+R, omlaag, rechts, omhoog, links, getal, datum, weekdag, maand, jaar, stap, stoppen, kopiëren, Som, som, gemiddelde, aantal, maximum, minimum, Alt+is-gelijk-teken, totaal -->

## Doorvoeren

Met doorvoeren zet u voort wat u al hebt ingevuld. Selecteer de cellen die u hebt ingevuld, sleep daarna het kleine vierkantje in de hoek van de selectie (de **vulgreep**) langs een kolom of een rij, en laat los. Een gestreepte rand laat zien hoe ver er wordt doorgevoerd.

| U begint met | Doorvoeren geeft |
| --- | --- |
| 1, 2 | 3, 4, 5, enzovoort: de stap tussen de getallen wordt voortgezet |
| Eén getal, zoals 5 | 5, 5, 5: kopieën (houd Ctrl ingedrukt tijdens het slepen voor de tegenovergestelde keuze) |
| Een datum | De volgende dagen (gebruik de knop **Opties voor doorvoeren**, of **Bewerken > Reeks doorvoeren**, voor weekdagen, maanden of jaren) |
| Ma, of januari | Di, wo, ... of februari, maart, ... in uw taal |
| Item 1 | Item 2, Item 3, ... |
| Een woord uit een aangepaste lijst, zoals Klein | Het volgende woord van de lijst, en na het laatste weer het eerste |
| Een formule | De formule gekopieerd, met de verwijzingen verschoven zoals bij plakken |

Dubbelklik op de vulgreep om door te voeren tot waar de gegevens naast de selectie reiken. JeffCrunch kijkt eerst naar de kolom links van de selectie en daarna naar de kolom rechts. Het stopt bij de eerste lege cel in die kolom.

## Aangepaste lijsten

Een **aangepaste lijst** is een rij woorden waarlangs Doorvoeren loopt, zoals de dagen van de week. Om er een te maken, klikt u op **Bestand > Opties > Aangepaste lijsten**. Typ één lijst per regel, met komma's tussen de woorden, bijvoorbeeld `Klein, Middel, Groot`. Klik op **OK**. Typ nu **Klein** in een cel en sleep de vulgreep: de cellen worden gevuld met Middel, Groot, Klein, enzovoort. Hoofdletters doen er niet toe wanneer JeffCrunch naar een woord in een lijst zoekt, en de nieuwe cellen nemen de hoofdletters over van de cel waarmee u bent begonnen. Een lijst heeft minstens twee woorden nodig. De dagen van de week en de maanden zijn ingebouwd.

**Doorvoeren > Omlaag** (**Ctrl+D**) en **Doorvoeren > Rechts** (**Ctrl+R**) kopiëren de eerste rij of kolom van de selectie over de rest. **Doorvoeren > Omhoog** en **Doorvoeren > Links** werken vanaf de laatste. **Bewerken > Reeks doorvoeren** vraagt naar het soort (lineair, groei, datum), de eenheid van een datum, de stap en het punt waarop moet worden gestopt. Elke gevulde cel krijgt de opmaak van de cel waarop hij volgt. Een cel die niet kan worden gewijzigd (deel van een matrix, een vergrendelde cel of een koptekst van een tabel) stopt het doorvoeren, en er verandert niets.

Nadat u de vulgreep hebt gesleept, verschijnt de kleine knop **Opties voor doorvoeren** in de hoek van wat is doorgevoerd. Het menu ervan voert het doorvoeren opnieuw uit op een andere manier, en het doorvoeren blijft één stap van Ongedaan maken: **Cellen kopiëren**, **Reeks doorvoeren**, **Alleen opmaak doorvoeren** (de cellen houden hun inhoud en nemen de opmaak over), **Doorvoeren zonder opmaak** (de cellen houden hun eigen opmaak), en, voor datums, **Dagen doorvoeren**, **Weekdagen doorvoeren**, **Maanden doorvoeren** en **Jaren doorvoeren**. De knop verdwijnt wanneer u iets anders doet.

Bij samengevoegde cellen wordt niet doorgevoerd, naar binnen of naar buiten, omdat er dan waarden zouden achterblijven in cellen die het samengevoegde gebied verbergt. Hef de samenvoeging eerst op.

**Doorvoeren > Uitvullen** zet de tekst van de geselecteerde cellen van één kolom in regels die even breed zijn als de kolom, in de cellen vanaf de eerste naar beneden; overgebleven cellen van de selectie worden leeggemaakt. Als de tekst meer cellen nodig heeft dan u hebt geselecteerd, vraagt JeffCrunch of de cellen eronder mogen worden gebruikt. Het werkt alleen op tekst.

**Doorvoeren naar werkbladen** kopieert de geselecteerde cellen naar dezelfde cellen van de bladen die u in een lijst aanvinkt: alles, alleen de inhoud, of alleen de opmaak. Het is één stap van Ongedaan maken.

## Som

**Start > Som** (**Alt en het is-gelijk-teken**) zet een totaal in de cel die u selecteert. Het kijkt naar de getallen boven de cel; als die er niet zijn, naar de getallen links ervan. De formule wordt voor u in de cel gezet om te controleren, en **Enter** houdt hem. De pijl naast Som geeft de andere totalen: **Gemiddelde**, **Aantal getallen**, **Maximum** en **Minimum**.

Selecteer eerst een blok, dan krijgt elke kolom van het blok zijn eigen formule in de lege cellen eronder (of, bij één rij, rechts ervan).
