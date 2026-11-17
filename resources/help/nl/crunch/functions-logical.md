# Logische functies
<!-- keywords: functies, AND, CHOOSE, FALSE, IF, IFERROR, IFNA, IFS, LET, NOT, OR, SWITCH, TRUE, XOR, EN, KIEZEN, ONWAAR, ALS, ALS.FOUT, ALS.NB, ALS.VOORWAARDEN, NIET, OF, SCHAKELEN, WAAR, EXCLUSIEF OF -->

Dit zijn de logische functies die JeffCrunch kan berekenen. Bij elke functie ziet u hoe ze wordt geschreven, wat ze doet en wat elk van haar argumenten is. Een argument tussen haakjes is optioneel, en "..." betekent dat u zoveel extra argumenten van het laatste soort kunt toevoegen als nodig is. Om een functie uit te proberen, zie [Een functie kiezen](insert-function). Terwijl u in een cel een functie typt, verschijnt onder de cel een kleine tip met de argumenten. Als u op de naam van de functie in de tip klikt, opent zich het onderdeel hier.

## EN

`AND(logische1, ...)`

Waar wanneer aan alle voorwaarden is voldaan.

- `logische1`: De eerste waarde of test, die waar of onwaar is.
- `...`: Zoveel extra als nodig, van dezelfde soort als het voorgaande.

## KIEZEN

`CHOOSE(index, waarde1, ...)`

Kiest de waarde in een lijst die op de plaats staat die de index aangeeft.

- `index`: Welke waarde moet worden gegeven, waarbij de eerste als 1 wordt geteld.
- `waarde1`: De eerste waarde of het eerste celbereik.
- `...`: Zoveel extra als nodig, van dezelfde soort als het voorgaande.

## ONWAAR

`FALSE()`

De logische waarde onwaar.

## ALS

`IF(voorwaarde, waarde_als_waar, [waarde_als_onwaar])`

Geeft de ene waarde wanneer een voorwaarde geldt en een andere wanneer dat niet zo is.

- `voorwaarde`: Een waarde of test die waar of onwaar is.
- `waarde_als_waar`: Wat moet worden gegeven als de voorwaarde waar is.
- `waarde_als_onwaar` (optioneel): Wat moet worden gegeven als de voorwaarde onwaar is (ONWAAR als dit wordt weggelaten).

## ALS.FOUT

`IFERROR(waarde, waarde_bij_fout)`

Geeft een waarde, of een vervanging wanneer die waarde een fout is.

- `waarde`: De waarde waarmee wordt gewerkt, of een cel die deze bevat.
- `waarde_bij_fout`: Wat moet worden gegeven als de waarde een fout is.

## ALS.NB

`IFNA(waarde, waarde_bij_nb)`

Geeft een waarde, of een vervanging wanneer die waarde de fout 'niet beschikbaar' is.

- `waarde`: De waarde waarmee wordt gewerkt, of een cel die deze bevat.
- `waarde_bij_nb`: Wat moet worden gegeven als de waarde de fout 'niet beschikbaar' is.

## ALS.VOORWAARDEN

`IFS(voorwaarde1, waarde1, ...)`

Test voorwaarden na elkaar en geeft de waarde die bij de eerste voorwaarde hoort die geldt.

- `voorwaarde1`: De eerste test, die waar of onwaar is.
- `waarde1`: De eerste waarde of het eerste celbereik.
- `...`: Zoveel extra als nodig, van dezelfde soort als het voorgaande.

## LET

`LET(naam1, waarde1, ...)`

Geeft namen aan waarden binnen één formule en berekent daarna een berekening die die namen gebruikt.

- `naam1`: Een naam die wordt gegeven aan de waarde die erop volgt. Deze kan worden gebruikt in de waarden erna en in de berekening.
- `waarde1`: De eerste waarde of het eerste celbereik.
- `...`: Zoveel extra als nodig, van dezelfde soort als het voorgaande.

## NIET

`NOT(logisch)`

Maakt waar onwaar en onwaar waar.

- `logisch`: Een waarde of test die waar of onwaar is.

## OF

`OR(logische1, ...)`

Waar wanneer aan ten minste één van de voorwaarden is voldaan.

- `logische1`: De eerste waarde of test, die waar of onwaar is.
- `...`: Zoveel extra als nodig, van dezelfde soort als het voorgaande.

## SCHAKELEN

`SWITCH(uitdrukking, waarde1, resultaat1, ...)`

Vergelijkt een waarde met een lijst van waarden en geeft het resultaat dat hoort bij de eerste overeenkomst.

- `uitdrukking`: De waarde die wordt vergeleken met de waarden die volgen.
- `waarde1`: De eerste waarde of het eerste celbereik.
- `resultaat1`: Het resultaat dat moet worden gegeven wanneer de waarde ervoor overeenkomt.
- `...`: Zoveel extra als nodig, van dezelfde soort als het voorgaande.

## WAAR

`TRUE()`

De logische waarde waar.

## EXCLUSIEF OF

`XOR(logische1, ...)`

Waar wanneer een oneven aantal van de voorwaarden waar is.

- `logische1`: De eerste waarde of test, die waar of onwaar is.
- `...`: Zoveel extra als nodig, van dezelfde soort als het voorgaande.
