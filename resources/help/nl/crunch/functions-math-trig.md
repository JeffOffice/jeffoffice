# Wiskundige en goniometrische functies
<!-- keywords: functies, ABS, ACOS, ACOSH, ACOT, ACOTH, AGGREGATE, ARABIC, ASIN, ASINH, ATAN, ATAN2, ATANH, BASE, CEILING, CEILING.MATH, CEILING.PRECISE, COMBIN, COMBINA, COS, COSH, COT, COTH, CSC, CSCH, DECIMAL, DEGREES, EVEN, EXP, FACT, FACTDOUBLE, FLOOR, FLOOR.MATH, FLOOR.PRECISE, GCD, INT, ISO.CEILING, LCM, LN, LOG, LOG10, MAX, MDETERM, MIN, MINVERSE, MMULT, MOD, MROUND, MULTINOMIAL, MUNIT, ODD, PERMUT, PI, POWER, PRODUCT, QUOTIENT, RADIANS, RAND, RANDARRAY, RANDBETWEEN, ROMAN, ROUND, ROUNDDOWN, ROUNDUP, SEC, SECH, SEQUENCE, SERIESSUM, SIGN, SIN, SINH, SQRT, SQRTPI, SUBTOTAL, SUM, SUMPRODUCT, SUMSQ, SUMX2MY2, SUMX2PY2, SUMXMY2, TAN, TANH, TRUNC -->

Dit zijn de wiskundige en goniometrische functies die JeffCrunch kan berekenen. Elke functie laat zien hoe ze wordt geschreven, wat ze doet en wat elk van haar argumenten is. Een argument tussen haakjes is optioneel, en "..." betekent dat u zoveel extra argumenten van het laatste type kunt opgeven als u nodig hebt. Om een functie uit te proberen, ziet u [Functie kiezen](insert-function). Terwijl u in een cel een functie typt, toont een kleine tip onder de cel de argumenten ervan, en als u op de naam van de functie in de tip klikt, opent u het gedeelte hierover.

## ABS

`ABS(number)`

Geeft een getal zonder teken.

- `number`: Het getal waarmee wordt gewerkt.

## ACOS

`ACOS(number)`

Geeft de hoek, in radialen, die bij een gegeven cosinus hoort.

- `number`: Het getal waarmee wordt gewerkt.

## ACOSH

`ACOSH(number)`

Geeft de inverse hyperbolische cosinus van een getal.

- `number`: Het getal waarmee wordt gewerkt.

## ACOT

`ACOT(number)`

Geeft de inverse cotangens van een getal, als hoek in radialen.

- `number`: Het getal waarmee wordt gewerkt.

## ACOTH

`ACOTH(number)`

Geeft de inverse hyperbolische cotangens van een getal.

- `number`: Het getal waarmee wordt gewerkt.

## AGGREGATE

`AGGREGATE(function_num, options, ref1, ...)`

Voert een berekening uit op een lijst, zoals een som of een mediaan, en kan verborgen rijen en fouten negeren.

- `function_num`: Welke berekening wordt uitgevoerd: 1 gemiddelde, 2 aantal, 3 aantal niet-lege cellen, 4 maximum, 5 minimum, 6 product, 7 en 8 standaarddeviatie, 9 som, 10 en 11 variantie.
- `options`: Wat wordt genegeerd: 0 geneste totalen, 1 ook verborgen rijen, 2 ook fouten, 3 beide, 4 tot 7 om geneste totalen te behouden.
- `ref1`: De eerste cel of het eerste celbereik.
- `...`: Zoveel extra argumenten als nodig, van dezelfde soort als het argument ervoor.

## ARABIC

`ARABIC(text)`

Leest Romeinse cijfers in tekst als getal.

- `text`: De tekst waarmee wordt gewerkt, of een cel die die bevat.

## ASIN

`ASIN(number)`

Geeft de hoek, in radialen, die bij een gegeven sinus hoort.

- `number`: Het getal waarmee wordt gewerkt.

## ASINH

`ASINH(number)`

Geeft de inverse hyperbolische sinus van een getal.

- `number`: Het getal waarmee wordt gewerkt.

## ATAN

`ATAN(number)`

Geeft de hoek, in radialen, die bij een gegeven tangens hoort.

- `number`: Het getal waarmee wordt gewerkt.

## ATAN2

`ATAN2(x_num, y_num)`

Geeft de hoek, in radialen, van het punt met x en y.

- `x_num`: De x-waarde.
- `y_num`: De y-waarde.

## ATANH

`ATANH(number)`

Geeft de inverse hyperbolische tangens van een getal.

- `number`: Het getal waarmee wordt gewerkt.

## BASE

`BASE(number, radix, [min_length])`

Schrijft een getal in een ander grondtal, van 2 tot 36, als tekst.

- `number`: Het getal waarmee wordt gewerkt.
- `radix`: Het grondtal van het getal: van 2 tot 36.
- `min_length` (optioneel): Het kleinste aantal tekens van het resultaat; kortere resultaten worden links aangevuld met nullen.

## CEILING

`CEILING(number, [significance])`

Rondt een getal naar boven af op het dichtstbijzijnde veelvoud.

- `number`: Het getal waarmee wordt gewerkt.
- `significance` (optioneel): Het getal naar het veelvoud waarvan wordt afgerond.

## CEILING.MATH

`CEILING.MATH(number, [significance], [mode])`

Rondt een getal naar boven af op het dichtstbijzijnde veelvoud, met een keuze voor negatieve getallen.

- `number`: Het getal waarmee wordt gewerkt.
- `significance` (optioneel): Het getal naar het veelvoud waarvan wordt afgerond.
- `mode` (optioneel): Hoe negatieve getallen worden afgerond: 0 of weggelaten is de gebruikelijke manier, elk ander getal keert die om.

## CEILING.PRECISE

`CEILING.PRECISE(number, [significance])`

Rondt een getal naar boven af op het dichtstbijzijnde veelvoud, ongeacht het teken van het veelvoud.

- `number`: Het getal waarmee wordt gewerkt.
- `significance` (optioneel): Het getal naar het veelvoud waarvan wordt afgerond.

## COMBIN

`COMBIN(number, number_chosen)`

Telt de manieren om items uit een groep te kiezen wanneer de volgorde niet telt.

- `number`: Het getal waarmee wordt gewerkt.
- `number_chosen`: Hoeveel items in elke groep worden gekozen.

## COMBINA

`COMBINA(number, number_chosen)`

Geeft het aantal manieren om items te kiezen wanneer een item meer dan één keer mag worden gekozen.

- `number`: Het getal waarmee wordt gewerkt.
- `number_chosen`: Hoeveel items in elke groep worden gekozen.

## COS

`COS(number)`

Geeft de cosinus van een hoek in radialen.

- `number`: Het getal waarmee wordt gewerkt.

## COSH

`COSH(number)`

Geeft de hyperbolische cosinus van een getal.

- `number`: Het getal waarmee wordt gewerkt.

## COT

`COT(angle)`

Geeft de cotangens van een hoek in radialen.

- `angle`: De hoek, in radialen.

## COTH

`COTH(number)`

Geeft de hyperbolische cotangens van een getal.

- `number`: Het getal waarmee wordt gewerkt.

## CSC

`CSC(angle)`

Geeft de cosecans van een hoek in radialen.

- `angle`: De hoek, in radialen.

## CSCH

`CSCH(number)`

Geeft de hyperbolische cosecans van een getal.

- `number`: Het getal waarmee wordt gewerkt.

## DECIMAL

`DECIMAL(text, radix)`

Leest tekst die in een grondtal van 2 tot 36 is geschreven als getal.

- `text`: De tekst waarmee wordt gewerkt, of een cel die die bevat.
- `radix`: Het grondtal van het getal: van 2 tot 36.

## DEGREES

`DEGREES(angle)`

Zet radialen om in graden.

- `angle`: De hoek, in radialen.

## EVEN

`EVEN(number)`

Rondt een getal naar boven af op het dichtstbijzijnde even geheel getal.

- `number`: Het getal waarmee wordt gewerkt.

## EXP

`EXP(number)`

Geeft e tot een macht.

- `number`: Het getal waarmee wordt gewerkt.

## FACT

`FACT(number)`

Geeft de faculteit van een getal.

- `number`: Het getal waarmee wordt gewerkt.

## FACTDOUBLE

`FACTDOUBLE(number)`

Geeft de dubbele faculteit van een getal: het product van elk tweede getal tot 1 of 2.

- `number`: Het getal waarmee wordt gewerkt.

## FLOOR

`FLOOR(number, [significance])`

Rondt een getal naar beneden af op het dichtstbijzijnde veelvoud.

- `number`: Het getal waarmee wordt gewerkt.
- `significance` (optioneel): Het getal naar het veelvoud waarvan wordt afgerond.

## FLOOR.MATH

`FLOOR.MATH(number, [significance], [mode])`

Rondt een getal naar beneden af op het dichtstbijzijnde veelvoud, met een keuze voor negatieve getallen.

- `number`: Het getal waarmee wordt gewerkt.
- `significance` (optioneel): Het getal naar het veelvoud waarvan wordt afgerond.
- `mode` (optioneel): Hoe negatieve getallen worden afgerond: 0 of weggelaten is de gebruikelijke manier, elk ander getal keert die om.

## FLOOR.PRECISE

`FLOOR.PRECISE(number, [significance])`

Rondt een getal naar beneden af op het dichtstbijzijnde veelvoud, ongeacht het teken van het veelvoud.

- `number`: Het getal waarmee wordt gewerkt.
- `significance` (optioneel): Het getal naar het veelvoud waarvan wordt afgerond.

## GCD

`GCD(number1, ...)`

Geeft het grootste geheel getal dat alle getallen deelt.

- `number1`: Het eerste getal, of celbereik.
- `...`: Zoveel extra argumenten als nodig, van dezelfde soort als het argument ervoor.

## INT

`INT(number)`

Rondt een getal naar beneden af op het gehele getal eronder.

- `number`: Het getal waarmee wordt gewerkt.

## ISO.CEILING

`ISO.CEILING(number, [significance])`

Rondt een getal naar boven af op het dichtstbijzijnde veelvoud, ongeacht het teken van het veelvoud.

- `number`: Het getal waarmee wordt gewerkt.
- `significance` (optioneel): Het getal naar het veelvoud waarvan wordt afgerond.

## LCM

`LCM(number1, ...)`

Geeft het kleinste geheel getal waardoor alle getallen deelbaar zijn.

- `number1`: Het eerste getal, of celbereik.
- `...`: Zoveel extra argumenten als nodig, van dezelfde soort als het argument ervoor.

## LN

`LN(number)`

Geeft de natuurlijke logaritme van een getal.

- `number`: Het getal waarmee wordt gewerkt.

## LOG

`LOG(number, [base])`

Geeft de logaritme van een getal met een grondtal (10 als er geen wordt opgegeven).

- `number`: Het getal waarmee wordt gewerkt.
- `base` (optioneel): Het grondtal van de logaritme (10 als weggelaten).

## LOG10

`LOG10(number)`

Geeft de logaritme van een getal met grondtal 10.

- `number`: Het getal waarmee wordt gewerkt.

## MAX

`MAX(number1, ...)`

Geeft het grootste getal.

- `number1`: Het eerste getal, of celbereik.
- `...`: Zoveel extra argumenten als nodig, van dezelfde soort als het argument ervoor.

## MDETERM

`MDETERM(array)`

Geeft de determinant van een vierkant getalbereik.

- `array`: Een celbereik, of een lijst met waarden tussen accolades.

## MIN

`MIN(number1, ...)`

Geeft het kleinste getal.

- `number1`: Het eerste getal, of celbereik.
- `...`: Zoveel extra argumenten als nodig, van dezelfde soort als het argument ervoor.

## MINVERSE

`MINVERSE(array)`

Geeft de inverse van een vierkant getalbereik.

- `array`: Een celbereik, of een lijst met waarden tussen accolades.

## MMULT

`MMULT(array1, array2)`

Geeft het product van twee getalbereiken, als matrices.

- `array1`: Het eerste celbereik of de eerste lijst met waarden.
- `array2`: Het tweede celbereik of de tweede lijst met waarden, even groot als de eerste.

## MOD

`MOD(number, divisor)`

Geeft het restant na het delen van het ene getal door het andere.

- `number`: Het getal waarmee wordt gewerkt.
- `divisor`: Het getal waardoor moet worden gedeeld.

## MROUND

`MROUND(number, multiple)`

Rondt een getal af op het dichtstbijzijnde veelvoud.

- `number`: Het getal waarmee wordt gewerkt.
- `multiple`: Het getal naar het veelvoud waarvan wordt afgerond.

## MULTINOMIAL

`MULTINOMIAL(number1, ...)`

Geeft de faculteit van de som van de getallen, gedeeld door het product van hun faculteiten.

- `number1`: Het eerste getal, of celbereik.
- `...`: Zoveel extra argumenten als nodig, van dezelfde soort als het argument ervoor.

## MUNIT

`MUNIT(dimension)`

Geeft de eenheidsmatrix van een bepaalde grootte.

- `dimension`: De grootte van het vierkante bereik: hoeveel rijen en kolommen.

## ODD

`ODD(number)`

Rondt een getal naar boven af op het dichtstbijzijnde oneven geheel getal.

- `number`: Het getal waarmee wordt gewerkt.

## PERMUT

`PERMUT(number, number_chosen)`

Telt de manieren om items uit een groep te kiezen wanneer de volgorde wel telt.

- `number`: Het getal waarmee wordt gewerkt.
- `number_chosen`: Hoeveel items in elke groep worden gekozen.

## PI

`PI()`

Geeft het getal pi.

## POWER

`POWER(number, power)`

Verheft een getal tot een macht.

- `number`: Het getal waarmee wordt gewerkt.
- `power`: De macht waartoe het getal wordt verheven.

## PRODUCT

`PRODUCT(number1, ...)`

Vermenigvuldigt getallen met elkaar.

- `number1`: Het eerste getal, of celbereik.
- `...`: Zoveel extra argumenten als nodig, van dezelfde soort als het argument ervoor.

## QUOTIENT

`QUOTIENT(numerator, denominator)`

Geeft het gehele deel van een deling.

- `numerator`: Het getal dat wordt gedeeld.
- `denominator`: Het getal waardoor moet worden gedeeld.

## RADIANS

`RADIANS(angle)`

Zet graden om in radialen.

- `angle`: De hoek, in radialen.

## RAND

`RAND()`

Geeft een willekeurig getal van 0 tot, maar niet inclusief, 1.

## RANDARRAY

`RANDARRAY([rows], [columns], [min], [max], [whole_number])`

Geeft een blok willekeurige getallen.

- `rows` (optioneel): Hoeveel rijen omlaag of omhoog: positief omlaag, negatief omhoog.
- `columns` (optioneel): Hoeveel kolommen breed het resultaat is (1 als weggelaten).
- `min` (optioneel): Het kleinste getal dat het resultaat mag hebben (0 als weggelaten).
- `max` (optioneel): Het grootste getal dat het resultaat mag hebben (1 als weggelaten).
- `whole_number` (optioneel): Waar voor gehele getallen; onwaar of weggelaten voor getallen met decimalen.

## RANDBETWEEN

`RANDBETWEEN(bottom, top)`

Geeft een willekeurig geheel getal tussen twee getallen.

- `bottom`: De kleinste waarde die het resultaat mag hebben.
- `top`: De grootste waarde die het resultaat mag hebben.

## ROMAN

`ROMAN(number, [form])`

Schrijft een geheel getal van 0 tot 3999 als Romeinse cijfers in tekst.

- `number`: Het getal waarmee wordt gewerkt.
- `form` (optioneel): Hoe kort de Romeinse cijfers zijn: 0 of weggelaten voor de klassieke vorm, tot 4 voor de kortste.

## ROUND

`ROUND(number, num_digits)`

Rondt een getal af op een aantal decimalen.

- `number`: Het getal waarmee wordt gewerkt.
- `num_digits`: Op hoeveel decimalen wordt afgerond: positief voor rechts van de komma, 0 voor een geheel getal, negatief voor links.

## ROUNDDOWN

`ROUNDDOWN(number, num_digits)`

Rondt een getal naar beneden af, richting nul, op een aantal decimalen.

- `number`: Het getal waarmee wordt gewerkt.
- `num_digits`: Op hoeveel decimalen wordt afgerond: positief voor rechts van de komma, 0 voor een geheel getal, negatief voor links.

## ROUNDUP

`ROUNDUP(number, num_digits)`

Rondt een getal naar boven af, van nul weg, op een aantal decimalen.

- `number`: Het getal waarmee wordt gewerkt.
- `num_digits`: Op hoeveel decimalen wordt afgerond: positief voor rechts van de komma, 0 voor een geheel getal, negatief voor links.

## SEC

`SEC(angle)`

Geeft de secans van een hoek in radialen.

- `angle`: De hoek, in radialen.

## SECH

`SECH(number)`

Geeft de hyperbolische secans van een getal.

- `number`: Het getal waarmee wordt gewerkt.

## SEQUENCE

`SEQUENCE(rows, [columns], [start], [step])`

Geeft een blok getallen die oplopen vanaf een startwaarde met een stap.

- `rows`: Hoeveel rijen omlaag of omhoog: positief omlaag, negatief omhoog.
- `columns` (optioneel): Hoeveel kolommen breed het resultaat is (1 als weggelaten).
- `start` (optioneel): Het eerste getal (1 als weggelaten).
- `step` (optioneel): Hoeveel er bij elk getal wordt opgeteld (1 als weggelaten; voor GESTEP het getal waarmee wordt vergeleken, 0 als weggelaten).

## SERIESSUM

`SERIESSUM(x, n, m, coefficients)`

Geeft de som van een machtreeks.

- `x`: De x-waarde waarmee moet worden gewerkt.
- `n`: De eerste macht van x, of de orde van de functie, een geheel getal van 0 of hoger.
- `m`: De stap die bij de macht wordt opgeteld voor elke coëfficiënt.
- `coefficients`: De getallen waarmee elke macht van x wordt vermenigvuldigd.

## SIGN

`SIGN(number)`

Geeft 1 voor een positief getal, 0 voor nul en -1 voor een negatief getal.

- `number`: Het getal waarmee wordt gewerkt.

## SIN

`SIN(number)`

Geeft de sinus van een hoek in radialen.

- `number`: Het getal waarmee wordt gewerkt.

## SINH

`SINH(number)`

Geeft de hyperbolische sinus van een getal.

- `number`: Het getal waarmee wordt gewerkt.

## SQRT

`SQRT(number)`

Geeft de vierkantswortel van een getal.

- `number`: Het getal waarmee wordt gewerkt.

## SQRTPI

`SQRTPI(number)`

Geeft de vierkantswortel van een getal maal pi.

- `number`: Het getal waarmee wordt gewerkt.

## SUBTOTAL

`SUBTOTAL(function_num, ref1, ...)`

Geeft een totaal, gemiddelde, aantal of andere waarde voor een celbereik.

- `function_num`: Welke berekening wordt uitgevoerd: 1 gemiddelde, 2 aantal, 3 aantal niet-lege cellen, 4 maximum, 5 minimum, 6 product, 7 en 8 standaarddeviatie, 9 som, 10 en 11 variantie.
- `ref1`: De eerste cel of het eerste celbereik.
- `...`: Zoveel extra argumenten als nodig, van dezelfde soort als het argument ervoor.

## SUM

`SUM(number1, ...)`

Telt getallen en de getallen in celbereiken op.

- `number1`: Het eerste getal, of celbereik.
- `...`: Zoveel extra argumenten als nodig, van dezelfde soort als het argument ervoor.

## SUMPRODUCT

`SUMPRODUCT(array1, ...)`

Vermenigvuldigt de getallen in even grote bereiken, plaats voor plaats, en telt de producten op.

- `array1`: Het eerste celbereik of de eerste lijst met waarden.
- `...`: Zoveel extra argumenten als nodig, van dezelfde soort als het argument ervoor.

## SUMSQ

`SUMSQ(number1, ...)`

Telt de kwadraten van getallen op.

- `number1`: Het eerste getal, of celbereik.
- `...`: Zoveel extra argumenten als nodig, van dezelfde soort als het argument ervoor.

## SUMX2MY2

`SUMX2MY2(array_x, array_y)`

Telt de verschillen van de kwadraten van twee reeksen getallen op.

- `array_x`: Het eerste celbereik of de eerste lijst met waarden.
- `array_y`: Het tweede celbereik of de tweede lijst met waarden, even groot als de eerste.

## SUMX2PY2

`SUMX2PY2(array_x, array_y)`

Telt de sommen van de kwadraten van twee reeksen getallen op.

- `array_x`: Het eerste celbereik of de eerste lijst met waarden.
- `array_y`: Het tweede celbereik of de tweede lijst met waarden, even groot als de eerste.

## SUMXMY2

`SUMXMY2(array_x, array_y)`

Telt de kwadraten van de verschillen van twee reeksen getallen op.

- `array_x`: Het eerste celbereik of de eerste lijst met waarden.
- `array_y`: Het tweede celbereik of de tweede lijst met waarden, even groot als de eerste.

## TAN

`TAN(number)`

Geeft de tangens van een hoek in radialen.

- `number`: Het getal waarmee wordt gewerkt.

## TANH

`TANH(number)`

Geeft de hyperbolische tangens van een getal.

- `number`: Het getal waarmee wordt gewerkt.

## TRUNC

`TRUNC(number, [num_digits])`

Snijdt een getal af na een aantal decimalen, zonder af te ronden.

- `number`: Het getal waarmee wordt gewerkt.
- `num_digits` (optioneel): Op hoeveel decimalen wordt afgerond: positief voor rechts van de komma, 0 voor een geheel getal, negatief voor links.
