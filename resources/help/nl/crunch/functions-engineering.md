# Technische functies
<!-- keywords: functies, BESSELI, BESSELJ, BESSELK, BESSELY, BIN2DEC, BIN2HEX, BIN2OCT, BITAND, BITLSHIFT, BITOR, BITRSHIFT, BITXOR, COMPLEX, CONVERT, DEC2BIN, DEC2HEX, DEC2OCT, DELTA, ERF, ERF.PRECISE, ERFC, ERFC.PRECISE, GESTEP, HEX2BIN, HEX2DEC, HEX2OCT, IMABS, IMAGINARY, IMARGUMENT, IMCONJUGATE, IMCOS, IMCOSH, IMCOT, IMCSC, IMCSCH, IMDIV, IMEXP, IMLN, IMLOG10, IMLOG2, IMPOWER, IMPRODUCT, IMREAL, IMSEC, IMSECH, IMSIN, IMSINH, IMSQRT, IMSUB, IMSUM, IMTAN, OCT2BIN, OCT2DEC, OCT2HEX -->

Dit zijn de technische functies die JeffCrunch kan berekenen. Elke functie laat zien hoe hij wordt geschreven, wat hij doet en wat elk van zijn argumenten is. Een argument tussen haakjes is optioneel, en "..." betekent dat u zoveel extra argumenten van het laatste type kunt opgeven als u nodig hebt. Om een functie uit te proberen, zie [Een functie kiezen](insert-function). Terwijl u een functie in een cel typt, toont een klein tipje onder de cel de argumenten ervan, en als u op de naam van de functie in de tip klikt, wordt het onderdeel hier geopend.

## BESSELI

`BESSELI(x, n)`

Geeft de gemodificeerde Besselfunctie van de eerste soort.

- `x`: De x-waarde waarmee moet worden gewerkt.
- `n`: De eerste macht van x, of de orde van de functie, een geheel getal van 0 of hoger.

## BESSELJ

`BESSELJ(x, n)`

Geeft de Besselfunctie van de eerste soort.

- `x`: De x-waarde waarmee moet worden gewerkt.
- `n`: De eerste macht van x, of de orde van de functie, een geheel getal van 0 of hoger.

## BESSELK

`BESSELK(x, n)`

Geeft de gemodificeerde Besselfunctie van de tweede soort.

- `x`: De x-waarde waarmee moet worden gewerkt.
- `n`: De eerste macht van x, of de orde van de functie, een geheel getal van 0 of hoger.

## BESSELY

`BESSELY(x, n)`

Geeft de Besselfunctie van de tweede soort.

- `x`: De x-waarde waarmee moet worden gewerkt.
- `n`: De eerste macht van x, of de orde van de functie, een geheel getal van 0 of hoger.

## BIN2DEC

`BIN2DEC(getal)`

Zet een binair getal om in een decimaal getal.

- `getal`: Het getal waarmee wordt gewerkt.

## BIN2HEX

`BIN2HEX(getal, [plaatsen])`

Zet een binair getal om in een hexadecimaal getal.

- `getal`: Het getal waarmee wordt gewerkt.
- `plaatsen` (optioneel): Het aantal tekens waarmee het resultaat links wordt aangevuld met nullen.

## BIN2OCT

`BIN2OCT(getal, [plaatsen])`

Zet een binair getal om in een octaal getal.

- `getal`: Het getal waarmee wordt gewerkt.
- `plaatsen` (optioneel): Het aantal tekens waarmee het resultaat links wordt aangevuld met nullen.

## BITAND

`BITAND(getal1, getal2)`

Geeft de bits die in beide van twee gehele getallen zijn ingesteld.

- `getal1`: Het eerste getal, of celbereik.
- `getal2`: Het tweede getal.

## BITLSHIFT

`BITLSHIFT(getal, verschuiving)`

Verschuift de bits van een geheel getal naar links.

- `getal`: Het getal waarmee wordt gewerkt.
- `verschuiving`: Het aantal bits dat wordt verschoven: positieve en negatieve getallen gaan de andere kant op.

## BITOR

`BITOR(getal1, getal2)`

Geeft de bits die in een van beide van twee gehele getallen zijn ingesteld.

- `getal1`: Het eerste getal, of celbereik.
- `getal2`: Het tweede getal.

## BITRSHIFT

`BITRSHIFT(getal, verschuiving)`

Verschuift de bits van een geheel getal naar rechts.

- `getal`: Het getal waarmee wordt gewerkt.
- `verschuiving`: Het aantal bits dat wordt verschoven: positieve en negatieve getallen gaan de andere kant op.

## BITXOR

`BITXOR(getal1, getal2)`

Geeft de bits die in een van beide van twee gehele getallen zijn ingesteld, maar niet in beide.

- `getal1`: Het eerste getal, of celbereik.
- `getal2`: Het tweede getal.

## COMPLEX

`COMPLEX(reëel_getal, imaginair_getal, [achtervoegsel])`

Maakt een complex getal van het reële en het imaginaire deel.

- `reëel_getal`: Het reële deel van het complexe getal.
- `imaginair_getal`: Het imaginaire deel van het complexe getal.
- `achtervoegsel` (optioneel): i of j, om het imaginaire deel aan te geven (i als dit wordt weggelaten).

## CONVERT

`CONVERT(getal, van_eenheid, naar_eenheid)`

Zet een getal om van de ene maateenheid naar de andere.

- `getal`: Het getal waarmee wordt gewerkt.
- `van_eenheid`: De eenheid waarin het getal staat, als tekst, zoals "mi" of "lbm".
- `naar_eenheid`: De eenheid waarnaar het getal moet worden omgezet, als tekst, zoals "km" of "kg".

## DEC2BIN

`DEC2BIN(getal, [plaatsen])`

Zet een decimaal getal om in een binair getal.

- `getal`: Het getal waarmee wordt gewerkt.
- `plaatsen` (optioneel): Het aantal tekens waarmee het resultaat links wordt aangevuld met nullen.

## DEC2HEX

`DEC2HEX(getal, [plaatsen])`

Zet een decimaal getal om in een hexadecimaal getal.

- `getal`: Het getal waarmee wordt gewerkt.
- `plaatsen` (optioneel): Het aantal tekens waarmee het resultaat links wordt aangevuld met nullen.

## DEC2OCT

`DEC2OCT(getal, [plaatsen])`

Zet een decimaal getal om in een octaal getal.

- `getal`: Het getal waarmee wordt gewerkt.
- `plaatsen` (optioneel): Het aantal tekens waarmee het resultaat links wordt aangevuld met nullen.

## DELTA

`DELTA(getal1, [getal2])`

Geeft aan of twee getallen gelijk zijn, met 1 voor ja en 0 voor nee.

- `getal1`: Het eerste getal, of celbereik.
- `getal2` (optioneel): Het tweede getal.

## ERF

`ERF(ondergrens, [bovengrens])`

Geeft de foutfunctie, tussen twee grenzen of vanaf 0.

- `ondergrens`: De ondergrens van het bereik.
- `bovengrens` (optioneel): De bovengrens van het bereik (alleen de ondergrens als dit wordt weggelaten).

## ERF.PRECISE

`ERF.PRECISE(x)`

Geeft de foutfunctie van 0 tot een grens.

- `x`: De x-waarde waarmee moet worden gewerkt.

## ERFC

`ERFC(x)`

Geeft de complementaire foutfunctie van een grens tot oneindig.

- `x`: De x-waarde waarmee moet worden gewerkt.

## ERFC.PRECISE

`ERFC.PRECISE(x)`

Geeft de complementaire foutfunctie van een grens tot oneindig.

- `x`: De x-waarde waarmee moet worden gewerkt.

## GESTEP

`GESTEP(getal, [stap])`

Geeft aan of een getal ten minste gelijk is aan een stap, met 1 voor ja en 0 voor nee.

- `getal`: Het getal waarmee wordt gewerkt.
- `stap` (optioneel): Hoeveel er bij elk getal wordt opgeteld (1 als dit wordt weggelaten; voor GESTEP het getal waarmee wordt vergeleken, 0 als dit wordt weggelaten).

## HEX2BIN

`HEX2BIN(getal, [plaatsen])`

Zet een hexadecimaal getal om in een binair getal.

- `getal`: Het getal waarmee wordt gewerkt.
- `plaatsen` (optioneel): Het aantal tekens waarmee het resultaat links wordt aangevuld met nullen.

## HEX2DEC

`HEX2DEC(getal)`

Zet een hexadecimaal getal om in een decimaal getal.

- `getal`: Het getal waarmee wordt gewerkt.

## HEX2OCT

`HEX2OCT(getal, [plaatsen])`

Zet een hexadecimaal getal om in een octaal getal.

- `getal`: Het getal waarmee wordt gewerkt.
- `plaatsen` (optioneel): Het aantal tekens waarmee het resultaat links wordt aangevuld met nullen.

## IMABS

`IMABS(inumber)`

Geeft de grootte van een complex getal.

- `inumber`: Een complex getal geschreven als tekst, zoals "3+4i".

## IMAGINARY

`IMAGINARY(inumber)`

Geeft het imaginaire deel van een complex getal.

- `inumber`: Een complex getal geschreven als tekst, zoals "3+4i".

## IMARGUMENT

`IMARGUMENT(inumber)`

Geeft de hoek van een complex getal, in radialen.

- `inumber`: Een complex getal geschreven als tekst, zoals "3+4i".

## IMCONJUGATE

`IMCONJUGATE(inumber)`

Geeft de complex toegevoegde van een complex getal.

- `inumber`: Een complex getal geschreven als tekst, zoals "3+4i".

## IMCOS

`IMCOS(inumber)`

Geeft de cosinus van een complex getal.

- `inumber`: Een complex getal geschreven als tekst, zoals "3+4i".

## IMCOSH

`IMCOSH(inumber)`

Geeft de hyperbolische cosinus van een complex getal.

- `inumber`: Een complex getal geschreven als tekst, zoals "3+4i".

## IMCOT

`IMCOT(inumber)`

Geeft de cotangens van een complex getal.

- `inumber`: Een complex getal geschreven als tekst, zoals "3+4i".

## IMCSC

`IMCSC(inumber)`

Geeft de cosecans van een complex getal.

- `inumber`: Een complex getal geschreven als tekst, zoals "3+4i".

## IMCSCH

`IMCSCH(inumber)`

Geeft de hyperbolische cosecans van een complex getal.

- `inumber`: Een complex getal geschreven als tekst, zoals "3+4i".

## IMDIV

`IMDIV(inumber1, inumber2)`

Geeft het quotiënt van twee complexe getallen.

- `inumber1`: Het eerste complexe getal, geschreven als tekst, zoals "3+4i".
- `inumber2`: Het tweede complexe getal, geschreven als tekst.

## IMEXP

`IMEXP(inumber)`

Geeft e verheven tot de macht van een complex getal.

- `inumber`: Een complex getal geschreven als tekst, zoals "3+4i".

## IMLN

`IMLN(inumber)`

Geeft de natuurlijke logaritme van een complex getal.

- `inumber`: Een complex getal geschreven als tekst, zoals "3+4i".

## IMLOG10

`IMLOG10(inumber)`

Geeft de logaritme met grondtal 10 van een complex getal.

- `inumber`: Een complex getal geschreven als tekst, zoals "3+4i".

## IMLOG2

`IMLOG2(inumber)`

Geeft de logaritme met grondtal 2 van een complex getal.

- `inumber`: Een complex getal geschreven als tekst, zoals "3+4i".

## IMPOWER

`IMPOWER(inumber, getal)`

Geeft een complex getal verheven tot een macht.

- `inumber`: Een complex getal geschreven als tekst, zoals "3+4i".
- `getal`: Het getal waarmee wordt gewerkt.

## IMPRODUCT

`IMPRODUCT(inumber1, ...)`

Geeft het product van complexe getallen.

- `inumber1`: Het eerste complexe getal, geschreven als tekst, zoals "3+4i".
- `...`: Zoveel extra als nodig, van dezelfde soort als het getal ervoor.

## IMREAL

`IMREAL(inumber)`

Geeft het reële deel van een complex getal.

- `inumber`: Een complex getal geschreven als tekst, zoals "3+4i".

## IMSEC

`IMSEC(inumber)`

Geeft de secans van een complex getal.

- `inumber`: Een complex getal geschreven als tekst, zoals "3+4i".

## IMSECH

`IMSECH(inumber)`

Geeft de hyperbolische secans van een complex getal.

- `inumber`: Een complex getal geschreven als tekst, zoals "3+4i".

## IMSIN

`IMSIN(inumber)`

Geeft de sinus van een complex getal.

- `inumber`: Een complex getal geschreven als tekst, zoals "3+4i".

## IMSINH

`IMSINH(inumber)`

Geeft de hyperbolische sinus van een complex getal.

- `inumber`: Een complex getal geschreven als tekst, zoals "3+4i".

## IMSQRT

`IMSQRT(inumber)`

Geeft de vierkantswortel van een complex getal.

- `inumber`: Een complex getal geschreven als tekst, zoals "3+4i".

## IMSUB

`IMSUB(inumber1, inumber2)`

Geeft het verschil van twee complexe getallen.

- `inumber1`: Het eerste complexe getal, geschreven als tekst, zoals "3+4i".
- `inumber2`: Het tweede complexe getal, geschreven als tekst.

## IMSUM

`IMSUM(inumber1, ...)`

Geeft de som van complexe getallen.

- `inumber1`: Het eerste complexe getal, geschreven als tekst, zoals "3+4i".
- `...`: Zoveel extra als nodig, van dezelfde soort als het getal ervoor.

## IMTAN

`IMTAN(inumber)`

Geeft de tangens van een complex getal.

- `inumber`: Een complex getal geschreven als tekst, zoals "3+4i".

## OCT2BIN

`OCT2BIN(getal, [plaatsen])`

Zet een octaal getal om in een binair getal.

- `getal`: Het getal waarmee wordt gewerkt.
- `plaatsen` (optioneel): Het aantal tekens waarmee het resultaat links wordt aangevuld met nullen.

## OCT2DEC

`OCT2DEC(getal)`

Zet een octaal getal om in een decimaal getal.

- `getal`: Het getal waarmee wordt gewerkt.

## OCT2HEX

`OCT2HEX(getal, [plaatsen])`

Zet een octaal getal om in een hexadecimaal getal.

- `getal`: Het getal waarmee wordt gewerkt.
- `plaatsen` (optioneel): Het aantal tekens waarmee het resultaat links wordt aangevuld met nullen.
