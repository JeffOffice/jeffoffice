# Cellen doorvoeren en optellen

<!-- keywords: doorvoeren, reeks, slepen, vulgreep, dubbelklikken, aangepaste lijst, AutoAanvullen, Ctrl+D, Ctrl+R, omlaag, rechts, omhoog, links, getal, datum, weekdag, maand, jaar, stap, stoppen, kopiëren, Som, som, gemiddelde, aantal, maximum, minimum, Alt+is-gelijk-aan, totaal, Flitsvulling, Ctrl+E, patroon, namen splitsen, doorvoeren, opvullen, reeks invullen -->

## Doorvoeren

Doorvoeren zet voort wat u hebt begonnen. Selecteer de cellen die u hebt ingevuld en sleep daarna het kleine vierkantje op de hoek van de selectie (de **vulgreep**) langs een kolom of een rij. Laat de muisknop los. Een gestreepte rand laat zien hoe ver het doorvoeren gaat.

| U begint met | De doorvoering geeft |
| --- | --- |
| 1, 2 | 3, 4, 5, enzovoort: de stap tussen de getallen gaat door |
| Een enkel getal, zoals 5 | 5, 5, 5: kopieën (houd Ctrl ingedrukt tijdens het slepen voor de tegenovergestelde keuze) |
| Een datum | De volgende dagen (gebruik de knop **Opties voor doorvoeren**, of **Start > Reeks**, voor weekdagen, maanden of jaren) |
| Ma, of januari | Di, wo, ... of februari, maart, ... in uw taal |
| Item 1 | Item 2, Item 3, ... |
| Een woord uit een aangepaste lijst, zoals Klein | Het volgende woord van de lijst, en na het laatste weer het eerste |
| Een formule | De formule gekopieerd, met de verwijzingen verschoven zoals bij plakken |

Dubbelklik op de vulgreep om naar beneden door te voeren, zo ver als de gegevens naast de selectie reiken. JeffCrunch kijkt eerst naar de kolom links en daarna naar de kolom rechts. Het stopt bij de eerste lege cel in die kolom.

## Aangepaste lijsten

Een **aangepaste lijst** is een rij woorden waar doorvoeren doorheen gaat, zoals de dagen van de week. Om er een te maken, klikt u op **Bestand > Opties > Aangepaste lijsten**. Typ elke lijst op een eigen regel, met komma's tussen de woorden, bijvoorbeeld `Klein, Middel, Groot`. Klik op **OK**. Typ nu **Klein** in een cel en sleep de vulgreep: de cellen worden gevuld met Middel, Groot, Klein, enzovoort. Hoofdletters doen er niet toe wanneer JeffCrunch een woord in een lijst zoekt, en de nieuwe cellen nemen de hoofdletters over van de cel waarmee u bent begonnen. Een lijst heeft minstens twee woorden nodig. De dagen van de week en de maanden zijn al ingebouwd.

**Opvullen > Omlaag** (**Ctrl+D**) en **Opvullen > Rechts** (**Ctrl+R**) kopiëren de eerste rij of kolom van de selectie over de rest. **Opvullen > Omhoog** en **Opvullen > Links** werken vanaf de laatste. **Start > Reeks** vraagt naar het soort (lineair, groei, datum), de eenheid van een datum, de stap en waar moet worden gestopt. Elke gevulde cel krijgt de opmaak van de cel waarop hij volgt. Een cel die niet kan worden gewijzigd (deel van een matrix, een vergrendelde cel of een koprij van een tabel) stopt het doorvoeren, en er verandert niets.

Nadat u de vulgreep hebt gesleept, verschijnt de kleine knop **Opties voor doorvoeren** op de hoek van wat is gevuld. Het menu doet het doorvoeren opnieuw op een andere manier, en het doorvoeren blijft één stap van Ongedaan maken: **Cellen kopiëren**, **Reeks doorvoeren**, **Alleen opmaak doorvoeren** (de cellen houden hun inhoud en krijgen de opmaak), **Doorvoeren zonder opmaak** (de cellen houden hun eigen opmaak), en voor datums **Dagen doorvoeren**, **Weekdagen doorvoeren**, **Maanden doorvoeren** en **Jaren doorvoeren**. De knop verdwijnt wanneer u iets anders doet.

Een doorvoering gaat niet in samengevoegde cellen en komt er niet uit, omdat er dan waarden zouden blijven staan in cellen die het samengevoegde gebied verbergt. Hef de samenvoeging eerst op.

**Opvullen > Uitvullen** zet de tekst van de geselecteerde cellen van één kolom in regels die even breed zijn als de kolom, in de cellen vanaf de eerste naar beneden; overgebleven cellen van de selectie worden leeggemaakt. Als de tekst meer cellen nodig heeft dan u hebt geselecteerd, vraagt JeffCrunch of de cellen eronder mogen worden gebruikt. Het werkt alleen op tekst.

**Opvullen > Flitsvulling** (**Ctrl+E**) vult een kolom voor u aan op basis van een patroon dat u laat zien. Typ het gewenste resultaat met de hand in de eerste een of twee cellen van een kolom, naast de cellen waaruit het komt. Zet daarna de cursor in die kolom en kies Flitsvulling. JeffCrunch bepaalt hoe uw voorbeelden zijn gemaakt uit de cellen ernaast, en vult de lege cellen op dezelfde manier in. Voorbeeld: kolom A bevat namen zoals **Smith, Ann**. In B1 typt u **Ann Smith**. Flitsvulling vult B2 en lager met de andere namen die op dezelfde manier zijn omgedraaid. Het kan een woord uit een cel nemen, de eerste letter van een woord, of de eerste of laatste paar letters, in hoofdletters, kleine letters of zoals ze waren, en het kan tekst toevoegen die in elk voorbeeld hetzelfde is, zoals een spatie of een punt. Als het geen patroon vindt, laat het dat weten. Typ dan een of twee voorbeelden extra en probeer het opnieuw. Het is één stap van Ongedaan maken.

**Opvullen > Doorvoeren naar werkbladen** kopieert de geselecteerde cellen naar dezelfde cellen van de bladen die u in een lijst aanvinkt: alles, alleen de inhoud of alleen de opmaak. Het is één stap van Ongedaan maken.

## Optellen

**Start > Som** (**Alt en het is-gelijk-aanteken**) zet een totaal in de cel die u selecteert. Het kijkt naar de getallen boven de cel; als die er niet zijn, naar de getallen links ervan. De formule wordt in de cel gezet zodat u hem kunt controleren, en **Enter** bevestigt hem. De pijl naast Som geeft de andere totalen: **Gemiddelde**, **Aantal getallen**, **Max** en **Min**.

Selecteer eerst een blok, dan krijgt elke kolom van het blok een eigen formule in de lege cellen eronder (of, bij een enkele rij, in de cellen rechts ervan).
