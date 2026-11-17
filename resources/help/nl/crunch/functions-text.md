# Tekstfuncties
<!-- keywords: functies, ASC, BAHTTEXT, CHAR, CLEAN, CODE, CONCAT, CONCATENATE, DBCS, DOLLAR, ENCODEURL, EXACT, FILTERXML, FIND, FINDB, FIXED, LEFT, LEFTB, LEN, LENB, LOWER, MID, MIDB, NUMBERVALUE, PHONETIC, PROPER, REPLACE, REPLACEB, REPT, RIGHT, RIGHTB, SEARCH, SEARCHB, SUBSTITUTE, T, TEXT, TEXTJOIN, TRIM, UNICHAR, UNICODE, UPPER, VALUE, tekstfunctie, formule, tekst bewerken, tekens, functies voor tekst -->

Dit zijn de tekstfuncties die JeffCrunch kan berekenen. Elke functie laat zien hoe ze wordt geschreven, wat ze doet en wat elk van de argumenten is. Een argument tussen haakjes is optioneel, en "..." betekent dat u zoveel extra van het laatste argument kunt opgeven als nodig is. Om een functie uit te proberen, ziet u [Een functie kiezen](insert-function). Terwijl u in een cel een functie typt, toont een kleine tip onder de cel de argumenten ervan, en als u in de tip op de naam van de functie klikt, opent zich het onderdeel hierover.

## ASC

`ASC(tekst)`

Zet volbrede letters en cijfers in tekst om in halfbrede letters en cijfers.

- `tekst`: De tekst waarmee wordt gewerkt, of een cel die deze bevat.

## BAHTTEXT

`BAHTTEXT(getal)`

Schrijft een getal in Thaise woorden, met het woord voor baht.

- `getal`: Het getal waarmee wordt gewerkt.

## CHAR

`CHAR(getal)`

Geeft het teken dat bij een codenummer hoort.

- `getal`: Het getal waarmee wordt gewerkt.

## CLEAN

`CLEAN(tekst)`

Verwijdert tekens uit een tekst die niet worden afgedrukt.

- `tekst`: De tekst waarmee wordt gewerkt, of een cel die deze bevat.

## CODE

`CODE(tekst)`

Geeft het codenummer van het eerste teken van een tekst.

- `tekst`: De tekst waarmee wordt gewerkt, of een cel die deze bevat.

## CONCAT

`CONCAT(tekst1, ...)`

Voegt teksten en de teksten in celbereiken samen tot één tekst.

- `tekst1`: De eerste tekst, of celbereik met tekst.
- `...`: Zoveel extra als nodig, van hetzelfde soort als het argument ervoor.

## CONCATENATE

`CONCATENATE(tekst1, ...)`

Voegt teksten samen tot één tekst.

- `tekst1`: De eerste tekst, of celbereik met tekst.
- `...`: Zoveel extra als nodig, van hetzelfde soort als het argument ervoor.

## DBCS

`DBCS(tekst)`

Zet halfbrede letters en cijfers in tekst om in volbrede letters en cijfers.

- `tekst`: De tekst waarmee wordt gewerkt, of een cel die deze bevat.

## DOLLAR

`DOLLAR(getal, [decimalen])`

Zet een getal om in tekst als geldbedrag.

- `getal`: Het getal waarmee wordt gewerkt.
- `decimalen` (optioneel): Hoeveel cijfers na de komma worden bewaard.

## ENCODEURL

`ENCODEURL(tekst)`

Zet tekst om zodat deze in een webadres kan worden gebruikt.

- `tekst`: De tekst waarmee wordt gewerkt, of een cel die deze bevat.

## EXACT

`EXACT(tekst1, tekst2)`

Waar als twee teksten precies gelijk zijn, inclusief hoofdletters.

- `tekst1`: De eerste tekst, of celbereik met tekst.
- `tekst2`: De tweede tekst.

## FILTERXML

`FILTERXML(xml, xpath)`

Haalt waarden uit tekst die XML bevat, met behulp van een XPath-expressie.

- `xml`: De tekst die de XML bevat.
- `xpath`: De XPath die aangeeft welke delen moeten worden teruggegeven, zoals //item/prijs.

## FIND

`FIND(zoektekst, binnentekst, [startgetal])`

Geeft de positie waar een tekst begint binnen een andere tekst, en onderscheidt hoofdletters van kleine letters.

- `zoektekst`: De tekst waarnaar wordt gezocht.
- `binnentekst`: De tekst waarin wordt gezocht.
- `startgetal` (optioneel): De positie in de tekst waar wordt begonnen, waarbij het eerste teken als 1 telt.

## FINDB

`FINDB(zoektekst, binnentekst, [startgetal])`

Geeft de positie waar tekst voor het eerst voorkomt in andere tekst, en onderscheidt hoofdletters.

- `zoektekst`: De tekst waarnaar wordt gezocht.
- `binnentekst`: De tekst waarin wordt gezocht.
- `startgetal` (optioneel): De positie in de tekst waar wordt begonnen, waarbij het eerste teken als 1 telt.

## FIXED

`FIXED(getal, [decimalen], [geen_komma's])`

Zet een getal om in tekst met een vast aantal decimalen.

- `getal`: Het getal waarmee wordt gewerkt.
- `decimalen` (optioneel): Hoeveel cijfers na de komma worden bewaard.
- `geen_komma's` (optioneel): Waar om de komma's weg te laten die duizendtallen groeperen.

## LEFT

`LEFT(tekst, [aantal_tekens])`

Geeft de eerste tekens van een tekst.

- `tekst`: De tekst waarmee wordt gewerkt, of een cel die deze bevat.
- `aantal_tekens` (optioneel): Hoeveel tekens worden overgenomen (1 als dit wordt weggelaten).

## LEFTB

`LEFTB(tekst, [aantal_tekens])`

Geeft de eerste tekens van tekst.

- `tekst`: De tekst waarmee wordt gewerkt, of een cel die deze bevat.
- `aantal_tekens` (optioneel): Hoeveel tekens worden overgenomen (1 als dit wordt weggelaten).

## LEN

`LEN(tekst)`

Telt de tekens in een tekst.

- `tekst`: De tekst waarmee wordt gewerkt, of een cel die deze bevat.

## LENB

`LENB(tekst)`

Geeft het aantal tekens in tekst.

- `tekst`: De tekst waarmee wordt gewerkt, of een cel die deze bevat.

## LOWER

`LOWER(tekst)`

Zet een tekst om in kleine letters.

- `tekst`: De tekst waarmee wordt gewerkt, of een cel die deze bevat.

## MID

`MID(tekst, startgetal, aantal_tekens)`

Geeft tekens uit het midden van een tekst.

- `tekst`: De tekst waarmee wordt gewerkt, of een cel die deze bevat.
- `startgetal`: De positie in de tekst waar wordt begonnen, waarbij het eerste teken als 1 telt.
- `aantal_tekens`: Hoeveel tekens worden overgenomen (1 als dit wordt weggelaten).

## MIDB

`MIDB(tekst, startgetal, aantal_tekens)`

Geeft tekens uit het midden van tekst.

- `tekst`: De tekst waarmee wordt gewerkt, of een cel die deze bevat.
- `startgetal`: De positie in de tekst waar wordt begonnen, waarbij het eerste teken als 1 telt.
- `aantal_tekens`: Hoeveel tekens worden overgenomen (1 als dit wordt weggelaten).

## NUMBERVALUE

`NUMBERVALUE(tekst, [decimaalscheidingsteken], [groepsscheidingsteken])`

Leest tekst als getal, met de scheidingstekens die u opgeeft.

- `tekst`: De tekst waarmee wordt gewerkt, of een cel die deze bevat.
- `decimaalscheidingsteken` (optioneel): Het teken dat de decimalen in de tekst aangeeft (de punt als dit wordt weggelaten).
- `groepsscheidingsteken` (optioneel): Het teken dat de cijfers in de tekst groepeert (de komma als dit wordt weggelaten).

## PHONETIC

`PHONETIC(verwijzing)`

Geeft de tekst van een cel, voor de lezingen van Japanse tekst.

- `verwijzing`: Een cel of celbereik.

## PROPER

`PROPER(tekst)`

Geeft elk woord van een tekst een hoofdletter als eerste letter.

- `tekst`: De tekst waarmee wordt gewerkt, of een cel die deze bevat.

## REPLACE

`REPLACE(oude_tekst, startgetal, aantal_tekens, nieuwe_tekst)`

Zet nieuwe tekst in de plaats van enkele tekens van een tekst.

- `oude_tekst`: De tekst die wordt gewijzigd.
- `startgetal`: De positie in de tekst waar wordt begonnen, waarbij het eerste teken als 1 telt.
- `aantal_tekens`: Hoeveel tekens worden overgenomen (1 als dit wordt weggelaten).
- `nieuwe_tekst`: De tekst die wordt ingevoegd.

## REPLACEB

`REPLACEB(oude_tekst, startgetal, aantal_tekens, nieuwe_tekst)`

Vervangt een deel van tekst door andere tekst.

- `oude_tekst`: De tekst die wordt gewijzigd.
- `startgetal`: De positie in de tekst waar wordt begonnen, waarbij het eerste teken als 1 telt.
- `aantal_tekens`: Hoeveel tekens worden overgenomen (1 als dit wordt weggelaten).
- `nieuwe_tekst`: De tekst die wordt ingevoegd.

## REPT

`REPT(tekst, aantal_keer)`

Herhaalt een tekst een aantal keer.

- `tekst`: De tekst waarmee wordt gewerkt, of een cel die deze bevat.
- `aantal_keer`: Hoeveel keer de tekst wordt herhaald.

## RIGHT

`RIGHT(tekst, [aantal_tekens])`

Geeft de laatste tekens van een tekst.

- `tekst`: De tekst waarmee wordt gewerkt, of een cel die deze bevat.
- `aantal_tekens` (optioneel): Hoeveel tekens worden overgenomen (1 als dit wordt weggelaten).

## RIGHTB

`RIGHTB(tekst, [aantal_tekens])`

Geeft de laatste tekens van tekst.

- `tekst`: De tekst waarmee wordt gewerkt, of een cel die deze bevat.
- `aantal_tekens` (optioneel): Hoeveel tekens worden overgenomen (1 als dit wordt weggelaten).

## SEARCH

`SEARCH(zoektekst, binnentekst, [startgetal])`

Geeft de positie waar een tekst begint binnen een andere tekst, zonder onderscheid tussen hoofdletters en kleine letters, en met ? en * toegestaan in wat wordt gezocht.

- `zoektekst`: De tekst waarnaar wordt gezocht.
- `binnentekst`: De tekst waarin wordt gezocht.
- `startgetal` (optioneel): De positie in de tekst waar wordt begonnen, waarbij het eerste teken als 1 telt.

## SEARCHB

`SEARCHB(zoektekst, binnentekst, [startgetal])`

Geeft de positie waar tekst voor het eerst voorkomt in andere tekst, zonder onderscheid tussen hoofdletters.

- `zoektekst`: De tekst waarnaar wordt gezocht.
- `binnentekst`: De tekst waarin wordt gezocht.
- `startgetal` (optioneel): De positie in de tekst waar wordt begonnen, waarbij het eerste teken als 1 telt.

## SUBSTITUTE

`SUBSTITUTE(tekst, oude_tekst, nieuwe_tekst, [voorkomen])`

Wisselt een stuk tekst om voor een ander, overal waar het voorkomt.

- `tekst`: De tekst waarmee wordt gewerkt, of een cel die deze bevat.
- `oude_tekst`: De tekst die moet worden gevonden.
- `nieuwe_tekst`: De tekst die wordt ingevoegd.
- `voorkomen` (optioneel): Welke overeenkomst moet worden vervangen (alle als dit wordt weggelaten).

## T

`T(waarde)`

Geeft de tekst zelf, en niets voor alles wat geen tekst is.

- `waarde`: De waarde waarmee wordt gewerkt, of een cel die deze bevat.

## TEXT

`TEXT(waarde, opmaaktekst)`

Zet een getal om in tekst in een opmaak die u kiest.

- `waarde`: De waarde waarmee wordt gewerkt, of een cel die deze bevat.
- `opmaaktekst`: De getalnotatie waarin de waarde wordt weergegeven, zoals "0,00" of "dd-mm-jjjj".

## TEXTJOIN

`TEXTJOIN(scheidingsteken, lege_overslaan, tekst1, ...)`

Voegt teksten samen met een scheidingsteken ertussen.

- `scheidingsteken`: De tekst die tussen de delen wordt geplaatst.
- `lege_overslaan`: Waar om lege delen weg te laten; onwaar om ze te behouden.
- `tekst1`: De eerste tekst, of celbereik met tekst.
- `...`: Zoveel extra als nodig, van hetzelfde soort als het argument ervoor.

## TRIM

`TRIM(tekst)`

Verwijdert extra spaties uit een tekst, zodat er tussen woorden alleen enkele spaties overblijven.

- `tekst`: De tekst waarmee wordt gewerkt, of een cel die deze bevat.

## UNICHAR

`UNICHAR(getal)`

Geeft het teken dat bij een Unicode-getal hoort.

- `getal`: Het getal waarmee wordt gewerkt.

## UNICODE

`UNICODE(tekst)`

Geeft het Unicode-getal van het eerste teken van tekst.

- `tekst`: De tekst waarmee wordt gewerkt, of een cel die deze bevat.

## UPPER

`UPPER(tekst)`

Zet een tekst om in hoofdletters.

- `tekst`: De tekst waarmee wordt gewerkt, of een cel die deze bevat.

## VALUE

`VALUE(tekst)`

Zet een tekst die op een getal lijkt om in een getal.

- `tekst`: De tekst waarmee wordt gewerkt, of een cel die deze bevat.
