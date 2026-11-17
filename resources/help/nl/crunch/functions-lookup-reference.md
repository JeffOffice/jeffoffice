# Opzoek- en verwijzingsfuncties
<!-- keywords: functies, ADRES, AANTAL.BEREIKEN, KOLOM, AANTAL.KOLOMMEN, FILTER, FORMULETEKST, DRAAITABEL.OPHALEN, HORIZ.ZOEKEN, HYPERLINK, INDEX, INDIRECT, ZOEKEN, VERGELIJKEN, VERSCHUIVING, RIJ, AANTAL.RIJEN, SORTEREN, SORTEREN.OP, TRANSPONEREN, UNIEK, VERT.ZOEKEN, XLOOKUP, XVERGELIJKEN, functie, formule, opzoeken, verwijzing, zoeken, adres, kolom, rij, sorteren, uniek, filteren, lookup, zoekfunctie -->

Dit zijn de opzoek- en verwijzingsfuncties die JeffCrunch kan berekenen. Bij elke functie ziet u hoe deze wordt geschreven, wat de functie doet en wat elk van de argumenten betekent. Een argument tussen haakjes is optioneel, en "..." betekent dat u zoveel extra van het laatste argument kunt opgeven als nodig is. Om een functie uit te proberen, zie [Een functie kiezen](insert-function). Terwijl u in een cel een functie typt, verschijnt onder de cel een kleine tip met de argumenten. Als u op de naam van de functie in de tip klikt, wordt het onderdeel van deze pagina geopend.

## ADRES

`ADRES(rijnummer, kolomnummer, [abs_getal], [a1], [bladnaam_tekst])`

Schrijft het adres van een cel als tekst, op basis van het rij- en kolomnummer.

- `rijnummer`: Het nummer van de rij in het bereik, waarbij de eerste rij als 1 telt.
- `kolomnummer`: Het nummer van de kolom in het bereik, waarbij de eerste kolom als 1 telt.
- `abs_getal` (optioneel): Hoeveel van het adres vastgezet is: 1 alles, 2 alleen de rij, 3 alleen de kolom, 4 niets.
- `a1` (optioneel): Waar of weggelaten om het adres te lezen als kolomletter en rijnummer; onwaar om het te lezen als rij- en kolomnummers.
- `bladnaam_tekst` (optioneel): De naam van het blad die voor het adres wordt geplaatst.

## AANTAL.BEREIKEN

`AANTAL.BEREIKEN(verwijzing)`

Geeft het aantal afzonderlijke celbereiken in een verwijzing.

- `verwijzing`: Een cel of een celbereik.

## KOLOM

`KOLOM([verwijzing])`

Geeft het kolomnummer van een cel.

- `verwijzing` (optioneel): Een cel of een celbereik.

## AANTAL.KOLOMMEN

`AANTAL.KOLOMMEN(matrix)`

Telt de kolommen in een celbereik.

- `matrix`: Een celbereik, of een lijst met waarden die tussen accolades is geschreven.

## FILTER

`FILTER(matrix, opnemen, [als_leeg])`

Geeft de rijen of kolommen van een bereik die aan een test voldoen.

- `matrix`: Een celbereik, of een lijst met waarden die tussen accolades is geschreven.
- `opnemen`: Een kolom of rij met waar- en onwaarwaarden, één voor elke rij of kolom van het bereik: waar behoudt deze.
- `als_leeg` (optioneel): Wat er wordt gegeven als geen enkele rij of kolom voldoet (de fout #CALC! als dit wordt weggelaten).

## FORMULETEKST

`FORMULETEKST(verwijzing)`

Geeft de formule van een cel als tekst.

- `verwijzing`: Een cel of een celbereik.

## DRAAITABEL.OPHALEN

`DRAAITABEL.OPHALEN(gegevensveld, draaitabel, [veld1], [item1], ...)`

Geeft de waarde die een draaitabel toont voor een waarde en de items van de rijen en kolommen. Een cel van de tabel is het tweede argument; de velden die worden genoemd, moeten de eerste zijn van de rijen of van de kolommen.

- `gegevensveld`: De naam van de waarde die u wilt ophalen, tussen aanhalingstekens, zoals de draaitabel die toont ("Som van Bedrag").
- `draaitabel`: Een willekeurige cel van de draaitabel.
- `veld1` (optioneel): De naam van een veld van de rijen of van de kolommen, tussen aanhalingstekens (kan samen met het item worden weggelaten).
- `item1` (optioneel): Het item van dat veld waarvoor de waarde wordt gegeven, tussen aanhalingstekens.
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## HORIZ.ZOEKEN

`HORIZ.ZOEKEN(zoekwaarde, tabelmatrix, rijindex_getal, [benaderen])`

Zoekt een waarde in de eerste rij van een tabel en geeft een waarde uit dezelfde kolom.

- `zoekwaarde`: De waarde waarnaar wordt gezocht.
- `tabelmatrix`: Het celbereik waarin wordt gezocht; de waarde waarnaar wordt gezocht, moet in de eerste rij staan.
- `rijindex_getal`: Het nummer van de rij van de tabel waaruit een waarde wordt gegeven, waarbij de eerste rij als 1 telt.
- `benaderen` (optioneel): Onwaar zoekt naar een exacte overeenkomst; waar of weggelaten zoekt naar de dichtstbijzijnde waarde die lager is (de eerste kolom moet oplopend zijn gesorteerd).

## HYPERLINK

`HYPERLINK(linklocatie, [vriendelijke_naam])`

Maakt een cel die een koppeling opent wanneer erop wordt geklikt.

- `linklocatie`: Waar de koppeling naartoe gaat: een webadres, een bestand of een plaats in deze werkmap.
- `vriendelijke_naam` (optioneel): De tekst die in de cel wordt getoond (de koppeling zelf als dit wordt weggelaten).

## INDEX

`INDEX(matrix, rijnummer, [kolomnummer], [gebiednummer])`

Geeft de waarde op een rij en kolom van een celbereik.

- `matrix`: Een celbereik, of een lijst met waarden die tussen accolades is geschreven.
- `rijnummer`: Het nummer van de rij in het bereik, waarbij de eerste rij als 1 telt.
- `kolomnummer` (optioneel): Het nummer van de kolom in het bereik, waarbij de eerste kolom als 1 telt.
- `gebiednummer` (optioneel): Welk bereik moet worden gebruikt wanneer het eerste argument er meerdere noemt (1 voor het eerste).

## INDIRECT

`INDIRECT(verwijzingstekst, [a1])`

Geeft de cel waarnaar een als tekst geschreven adres verwijst.

- `verwijzingstekst`: Een adres dat als tekst is geschreven, zoals "B5".
- `a1` (optioneel): Waar of weggelaten om het adres te lezen als kolomletter en rijnummer; onwaar om het te lezen als rij- en kolomnummers.

## ZOEKEN

`ZOEKEN(zoekwaarde, zoekvector, [resultaatvector])`

Zoekt een waarde in één rij of kolom en geeft de waarde op dezelfde plaats in een andere rij of kolom.

- `zoekwaarde`: De waarde waarnaar wordt gezocht.
- `zoekvector`: De ene rij of de ene kolom met cellen waarin wordt gezocht.
- `resultaatvector` (optioneel): De ene rij of de ene kolom met cellen waaruit het resultaat wordt gegeven.

## VERGELIJKEN

`VERGELIJKEN(zoekwaarde, zoekmatrix, [overeenkomsttype])`

Geeft de plaats van een waarde in een lijst.

- `zoekwaarde`: De waarde waarnaar wordt gezocht.
- `zoekmatrix`: Het celbereik, één rij of één kolom, waarin wordt gezocht.
- `overeenkomsttype` (optioneel): 1 of weggelaten zoekt de grootste waarde die niet boven de zoekwaarde ligt (de lijst moet oplopend zijn gesorteerd); 0 zoekt een exacte overeenkomst; -1 zoekt de kleinste waarde die niet onder de zoekwaarde ligt (de lijst moet aflopend zijn gesorteerd).

## VERSCHUIVING

`VERSCHUIVING(verwijzing, rijen, kolommen, [hoogte], [breedte])`

Geeft de cellen die een opgegeven aantal rijen en kolommen verwijderd liggen van een startcel.

- `verwijzing`: Een cel of een celbereik.
- `rijen`: Hoeveel rijen moet worden verschoven: positief naar beneden, negatief naar boven.
- `kolommen`: Hoeveel kolommen moet worden verschoven: positief naar rechts, negatief naar links.
- `hoogte` (optioneel): Hoeveel rijen hoog het resultaat is.
- `breedte` (optioneel): Hoeveel kolommen breed het resultaat is.

## RIJ

`RIJ([verwijzing])`

Geeft het rijnummer van een cel.

- `verwijzing` (optioneel): Een cel of een celbereik.

## AANTAL.RIJEN

`AANTAL.RIJEN(matrix)`

Telt de rijen in een celbereik.

- `matrix`: Een celbereik, of een lijst met waarden die tussen accolades is geschreven.

## SORTEREN

`SORTEREN(matrix, [sorteerindex], [sorteervolgorde], [op_kolom])`

Geeft een bereik waarvan de rijen of kolommen in volgorde zijn gezet.

- `matrix`: Een celbereik, of een lijst met waarden die tussen accolades is geschreven.
- `sorteerindex` (optioneel): Het nummer van de kolom (of rij) waarop wordt gesorteerd (1 als dit wordt weggelaten).
- `sorteervolgorde` (optioneel): 1 of weggelaten voor oplopend; -1 voor aflopend.
- `op_kolom` (optioneel): Waar om op kolommen te werken; onwaar of weggelaten om op rijen te werken.

## SORTEREN.OP

`SORTEREN.OP(matrix, sorteer_matrix1, [sorteervolgorde1], ...)`

Geeft een bereik in de volgorde van een of meer andere bereiken.

- `matrix`: Een celbereik, of een lijst met waarden die tussen accolades is geschreven.
- `sorteer_matrix1`: Het celbereik waarop het eerste bereik moet worden gesorteerd; even lang als het eerste bereik.
- `sorteervolgorde1` (optioneel): 1 of weggelaten voor oplopend; -1 voor aflopend.
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## TRANSPONEREN

`TRANSPONEREN(matrix)`

Maakt van rijen kolommen en van kolommen rijen.

- `matrix`: Een celbereik, of een lijst met waarden die tussen accolades is geschreven.

## UNIEK

`UNIEK(matrix, [op_kolom], [precies_eenmaal])`

Geeft de verschillende rijen of kolommen van een bereik.

- `matrix`: Een celbereik, of een lijst met waarden die tussen accolades is geschreven.
- `op_kolom` (optioneel): Waar om op kolommen te werken; onwaar of weggelaten om op rijen te werken.
- `precies_eenmaal` (optioneel): Waar om alleen de items te geven die één keer voorkomen; onwaar of weggelaten om elk verschillend item te geven.

## VERT.ZOEKEN

`VERT.ZOEKEN(zoekwaarde, tabelmatrix, kolomindex_getal, [benaderen])`

Zoekt een waarde in de eerste kolom van een tabel en geeft een waarde uit dezelfde rij.

- `zoekwaarde`: De waarde waarnaar wordt gezocht.
- `tabelmatrix`: Het celbereik waarin wordt gezocht; de waarde waarnaar wordt gezocht, moet in de eerste kolom staan.
- `kolomindex_getal`: Het nummer van de kolom van de tabel waaruit een waarde wordt gegeven, waarbij de eerste kolom als 1 telt.
- `benaderen` (optioneel): Onwaar zoekt naar een exacte overeenkomst; waar of weggelaten zoekt naar de dichtstbijzijnde waarde die lager is (de eerste kolom moet oplopend zijn gesorteerd).

## XLOOKUP

`XLOOKUP(zoekwaarde, zoekmatrix, retourmatrix, [als_niet_gevonden], [overeenkomstmodus], [zoekmodus])`

Zoekt een waarde op in een lijst en geeft het overeenkomende item uit een andere lijst.

- `zoekwaarde`: De waarde waarnaar wordt gezocht.
- `zoekmatrix`: Het celbereik, één rij of één kolom, waarin wordt gezocht.
- `retourmatrix`: Het celbereik waaruit de overeenkomst wordt gegeven.
- `als_niet_gevonden` (optioneel): Wat er wordt gegeven als er geen overeenkomst is (de fout #N/B als dit wordt weggelaten).
- `overeenkomstmodus` (optioneel): 0 of weggelaten voor een exacte overeenkomst, -1 voor de volgende kleinere, 1 voor de volgende grotere, 2 om jokertekens te gebruiken.
- `zoekmodus` (optioneel): 1 of weggelaten om van het eerste te zoeken, -1 van het laatste, 2 of -2 voor een binaire zoekactie in een gesorteerde lijst.

## XVERGELIJKEN

`XVERGELIJKEN(zoekwaarde, zoekmatrix, [overeenkomstmodus], [zoekmodus])`

Geeft de plaats van een waarde in een lijst, met keuzes voor de manier van overeenkomen en de zoekrichting.

- `zoekwaarde`: De waarde waarnaar wordt gezocht.
- `zoekmatrix`: Het celbereik, één rij of één kolom, waarin wordt gezocht.
- `overeenkomstmodus` (optioneel): 0 of weggelaten voor een exacte overeenkomst, -1 voor de volgende kleinere, 1 voor de volgende grotere, 2 om jokertekens te gebruiken.
- `zoekmodus` (optioneel): 1 of weggelaten om van het eerste te zoeken, -1 van het laatste, 2 of -2 voor een binaire zoekactie in een gesorteerde lijst.
