# Cellen doorvoeren en optellen
<!-- keywords: doorvoeren, vullen, reeks, slepen, vulgreep, AutoAanvullen, Ctrl+D, Ctrl+R, omlaag, rechts, omhoog, links, getal, datum, weekdag, maand, jaar, stap, stapwaarde, eindwaarde, kopiëren, Som, optellen, totaal, gemiddelde, aantal, max, min, Alt+is-gelijkteken, autosom -->

## Doorvoeren

Met doorvoeren zet u voort wat u begonnen bent. Selecteer de cellen die u hebt ingevuld, sleep het kleine vierkantje op de hoek van de selectie (de **vulgreep**) langs een kolom of rij en laat los. Een stippelrand geeft aan hoe ver het doorvoeren gaat.

| U begint met | Doorvoeren geeft |
| --- | --- |
| 1, 2 | 3, 4, 5 enzovoort: de stap ertussen wordt voortgezet |
| Eén getal, zoals 5 | 5, 5, 5: kopieën (houd Ctrl ingedrukt tijdens het slepen voor het omgekeerde) |
| Een datum | De volgende dagen (gebruik de knop **Opties voor doorvoeren** of **Doorvoeren > Reeks** voor weekdagen, maanden of jaren) |
| ma, of januari | di, wo, ... of februari, maart, ... in uw taal |
| Item 1 | Item 2, Item 3, ... |
| Een formule | De formule gekopieerd, met de verwijzingen verschoven zoals bij plakken |

**Doorvoeren > Omlaag** (**Ctrl+D**) en **Doorvoeren > Rechts** (**Ctrl+R**) kopiëren de eerste rij of kolom van de selectie naar de rest. **Doorvoeren > Omhoog** en **Doorvoeren > Links** werken vanaf de laatste. **Doorvoeren > Reeks** vraagt naar het type (lineair, groei, datum), de datumeenheid, de stapwaarde en waar moet worden gestopt. Elke doorgevoerde cel krijgt de opmaak van de cel waarop deze volgt. Een cel die niet kan worden gewijzigd (onderdeel van een matrix, een vergrendelde cel of de koprij van een tabel), stopt het doorvoeren, en er verandert niets.

Nadat u de vulgreep hebt gesleept, verschijnt de kleine knop **Opties voor doorvoeren** op de hoek van wat is doorgevoerd. Met het menu ervan voert u opnieuw door op een andere manier, en het doorvoeren blijft één stap voor Ongedaan maken: **Cellen kopiëren**, **Reeks doorvoeren**, **Alleen opmaak doorvoeren** (de cellen behouden hun inhoud en krijgen de opmaak), **Doorvoeren zonder opmaak** (de cellen behouden hun eigen opmaak) en, voor datums, **Dagen doorvoeren**, **Weekdagen doorvoeren**, **Maanden doorvoeren** en **Jaren doorvoeren**. De knop verdwijnt zodra u iets anders doet.

Doorvoeren gaat niet in of uit samengevoegde cellen, omdat er dan waarden zouden blijven staan in cellen die door het samengevoegde gebied worden verborgen. Hef eerst het samenvoegen op.

**Doorvoeren > Uitvullen** zet de tekst van de geselecteerde cellen van één kolom in regels die zo breed zijn als de kolom, in de cellen vanaf de eerste naar beneden; overgebleven cellen van de selectie worden leeggemaakt. Als de tekst meer cellen nodig heeft dan u hebt geselecteerd, vraagt JeffCrunch of de cellen eronder mogen worden gebruikt. Dit werkt alleen met tekst.

**Doorvoeren > Over werkbladen** kopieert de geselecteerde cellen naar dezelfde cellen van de werkbladen die u in een lijst aanvinkt: alles, alleen de inhoud of alleen de opmaak. Dit is één stap voor Ongedaan maken.

## Som

**Start > Som** (**Alt en het is-gelijkteken**) zet een totaal in de cel die u selecteert. Er wordt gekeken naar de getallen boven de cel; als die er niet zijn, naar de getallen links ervan. De formule wordt in de cel gezet zodat u deze kunt controleren, en met **Enter** houdt u deze. De pijl naast Som geeft de andere totalen: **Gemiddelde**, **Aantal getallen**, **Max** en **Min**.

Selecteer eerst een blok, dan krijgt elke kolom van het blok een eigen formule in de lege cellen eronder (of, bij één rij, rechts ervan).
