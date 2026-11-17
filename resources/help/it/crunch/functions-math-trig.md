# Funzioni matematiche e trigonometriche
<!-- keywords: funzioni, ABS, ACOS, ACOSH, ACOT, ACOTH, AGGREGATE, ARABIC, ASIN, ASINH, ATAN, ATAN2, ATANH, BASE, CEILING, CEILING.MATH, CEILING.PRECISE, COMBIN, COMBINA, COS, COSH, COT, COTH, CSC, CSCH, DECIMAL, DEGREES, EVEN, EXP, FACT, FACTDOUBLE, FLOOR, FLOOR.MATH, FLOOR.PRECISE, GCD, INT, ISO.CEILING, LCM, LN, LOG, LOG10, MAX, MDETERM, MIN, MINVERSE, MMULT, MOD, MROUND, MULTINOMIAL, MUNIT, ODD, PERMUT, PI, POWER, PRODUCT, QUOTIENT, RADIANS, RAND, RANDARRAY, RANDBETWEEN, ROMAN, ROUND, ROUNDDOWN, ROUNDUP, SEC, SECH, SEQUENCE, SERIESSUM, SIGN, SIN, SINH, SQRT, SQRTPI, SUBTOTAL, SUM, SUMPRODUCT, SUMSQ, SUMX2MY2, SUMX2PY2, SUMXMY2, TAN, TANH, TRUNC, somma, prodotto, arrotonda, radice, seno, coseno, tangente, logaritmo, fattoriale, potenza, casuale, matrice -->

Queste sono le funzioni matematiche e trigonometriche che JeffCrunch può calcolare. Ogni funzione mostra come si scrive, cosa fa e quali sono i suoi argomenti. Un argomento tra parentesi quadre è facoltativo, e "..." indica che si possono aggiungere quanti altri argomenti servono dopo l'ultimo. Per provare una funzione, vedi [Scegli una funzione](insert-function). Mentre si digita una funzione in una cella, un piccolo suggerimento sotto la cella mostra i suoi argomenti, e un clic sul nome della funzione nel suggerimento apre la sua scheda qui.

## ABS

`ABS(number)`

Restituisce un numero senza il segno.

- `number`: Il numero da elaborare.

## ACOS

`ACOS(number)`

Restituisce l'angolo, in radianti, che ha un dato coseno.

- `number`: Il numero da elaborare.

## ACOSH

`ACOSH(number)`

Restituisce il coseno iperbolico inverso di un numero.

- `number`: Il numero da elaborare.

## ACOT

`ACOT(number)`

Restituisce la cotangente inversa di un numero, come angolo in radianti.

- `number`: Il numero da elaborare.

## ACOTH

`ACOTH(number)`

Restituisce la cotangente iperbolica inversa di un numero.

- `number`: Il numero da elaborare.

## AGGREGATE

`AGGREGATE(function_num, options, ref1, ...)`

Esegue un calcolo su un elenco, come una somma o una mediana, e può escludere righe nascoste ed errori.

- `function_num`: Il calcolo da eseguire: 1 media, 2 conteggio, 3 conteggio non vuoti, 4 massimo, 5 minimo, 6 prodotto, 7 e 8 deviazione standard, 9 somma, 10 e 11 varianza.
- `options`: Cosa escludere: 0 totali annidati, 1 anche righe nascoste, 2 anche errori, 3 entrambi, da 4 a 7 per mantenere i totali annidati.
- `ref1`: La prima cella o il primo intervallo di celle.
- `...`: Quanti altri ne servono, dello stesso tipo del precedente.

## ARABIC

`ARABIC(text)`

Legge i numeri romani in un testo come numero.

- `text`: Il testo da elaborare, oppure una cella che lo contiene.

## ASIN

`ASIN(number)`

Restituisce l'angolo, in radianti, che ha un dato seno.

- `number`: Il numero da elaborare.

## ASINH

`ASINH(number)`

Restituisce il seno iperbolico inverso di un numero.

- `number`: Il numero da elaborare.

## ATAN

`ATAN(number)`

Restituisce l'angolo, in radianti, che ha una data tangente.

- `number`: Il numero da elaborare.

## ATAN2

`ATAN2(x_num, y_num)`

Restituisce l'angolo, in radianti, del punto di coordinate x e y.

- `x_num`: Il valore x.
- `y_num`: Il valore y.

## ATANH

`ATANH(number)`

Restituisce la tangente iperbolica inversa di un numero.

- `number`: Il numero da elaborare.

## BASE

`BASE(number, radix, [min_length])`

Scrive un numero in un'altra base, da 2 a 36, come testo.

- `number`: Il numero da elaborare.
- `radix`: La base del numero: da 2 a 36.
- `min_length` (facoltativo): Il numero minimo di caratteri del risultato; i risultati più brevi vengono completati con zeri a sinistra.

## CEILING

`CEILING(number, [significance])`

Arrotonda un numero per eccesso al multiplo più vicino.

- `number`: Il numero da elaborare.
- `significance` (facoltativo): Il numero i cui multipli usare per l'arrotondamento.

## CEILING.MATH

`CEILING.MATH(number, [significance], [mode])`

Arrotonda un numero per eccesso al multiplo più vicino, con una scelta per i numeri negativi.

- `number`: Il numero da elaborare.
- `significance` (facoltativo): Il numero i cui multipli usare per l'arrotondamento.
- `mode` (facoltativo): Come arrotondare i numeri negativi: 0 o omesso è il modo usuale, qualsiasi altro numero lo inverte.

## CEILING.PRECISE

`CEILING.PRECISE(number, [significance])`

Arrotonda un numero per eccesso al multiplo più vicino, qualunque sia il segno del multiplo.

- `number`: Il numero da elaborare.
- `significance` (facoltativo): Il numero i cui multipli usare per l'arrotondamento.

## COMBIN

`COMBIN(number, number_chosen)`

Conta i modi di scegliere elementi da un gruppo quando l'ordine non conta.

- `number`: Il numero da elaborare.
- `number_chosen`: Quanti elementi scegliere in ogni gruppo.

## COMBINA

`COMBINA(number, number_chosen)`

Restituisce il numero di modi di scegliere elementi quando un elemento può essere scelto più di una volta.

- `number`: Il numero da elaborare.
- `number_chosen`: Quanti elementi scegliere in ogni gruppo.

## COS

`COS(number)`

Restituisce il coseno di un angolo espresso in radianti.

- `number`: Il numero da elaborare.

## COSH

`COSH(number)`

Restituisce il coseno iperbolico di un numero.

- `number`: Il numero da elaborare.

## COT

`COT(angle)`

Restituisce la cotangente di un angolo in radianti.

- `angle`: L'angolo, in radianti.

## COTH

`COTH(number)`

Restituisce la cotangente iperbolica di un numero.

- `number`: Il numero da elaborare.

## CSC

`CSC(angle)`

Restituisce la cosecante di un angolo in radianti.

- `angle`: L'angolo, in radianti.

## CSCH

`CSCH(number)`

Restituisce la cosecante iperbolica di un numero.

- `number`: Il numero da elaborare.

## DECIMAL

`DECIMAL(text, radix)`

Legge un testo scritto in una base da 2 a 36 come numero.

- `text`: Il testo da elaborare, oppure una cella che lo contiene.
- `radix`: La base del numero: da 2 a 36.

## DEGREES

`DEGREES(angle)`

Converte i radianti in gradi.

- `angle`: L'angolo, in radianti.

## EVEN

`EVEN(number)`

Arrotonda un numero per eccesso al numero intero pari più vicino.

- `number`: Il numero da elaborare.

## EXP

`EXP(number)`

Restituisce e elevato a una potenza.

- `number`: Il numero da elaborare.

## FACT

`FACT(number)`

Restituisce il fattoriale di un numero.

- `number`: Il numero da elaborare.

## FACTDOUBLE

`FACTDOUBLE(number)`

Restituisce il doppio fattoriale di un numero: il prodotto di ogni secondo numero fino a 1 o 2.

- `number`: Il numero da elaborare.

## FLOOR

`FLOOR(number, [significance])`

Arrotonda un numero per difetto al multiplo più vicino.

- `number`: Il numero da elaborare.
- `significance` (facoltativo): Il numero i cui multipli usare per l'arrotondamento.

## FLOOR.MATH

`FLOOR.MATH(number, [significance], [mode])`

Arrotonda un numero per difetto al multiplo più vicino, con una scelta per i numeri negativi.

- `number`: Il numero da elaborare.
- `significance` (facoltativo): Il numero i cui multipli usare per l'arrotondamento.
- `mode` (facoltativo): Come arrotondare i numeri negativi: 0 o omesso è il modo usuale, qualsiasi altro numero lo inverte.

## FLOOR.PRECISE

`FLOOR.PRECISE(number, [significance])`

Arrotonda un numero per difetto al multiplo più vicino, qualunque sia il segno del multiplo.

- `number`: Il numero da elaborare.
- `significance` (facoltativo): Il numero i cui multipli usare per l'arrotondamento.

## GCD

`GCD(number1, ...)`

Restituisce il numero intero più grande che divide tutti i numeri.

- `number1`: Il primo numero o intervallo di celle.
- `...`: Quanti altri ne servono, dello stesso tipo del precedente.

## INT

`INT(number)`

Arrotonda un numero per difetto al numero intero inferiore più vicino.

- `number`: Il numero da elaborare.

## ISO.CEILING

`ISO.CEILING(number, [significance])`

Arrotonda un numero per eccesso al multiplo più vicino, qualunque sia il segno del multiplo.

- `number`: Il numero da elaborare.
- `significance` (facoltativo): Il numero i cui multipli usare per l'arrotondamento.

## LCM

`LCM(number1, ...)`

Restituisce il più piccolo numero intero di cui tutti i numeri sono divisori.

- `number1`: Il primo numero o intervallo di celle.
- `...`: Quanti altri ne servono, dello stesso tipo del precedente.

## LN

`LN(number)`

Restituisce il logaritmo naturale di un numero.

- `number`: Il numero da elaborare.

## LOG

`LOG(number, [base])`

Restituisce il logaritmo di un numero in una base (10 se non è indicata).

- `number`: Il numero da elaborare.
- `base` (facoltativo): La base del logaritmo (10 se omessa).

## LOG10

`LOG10(number)`

Restituisce il logaritmo in base 10 di un numero.

- `number`: Il numero da elaborare.

## MAX

`MAX(number1, ...)`

Restituisce il numero più grande.

- `number1`: Il primo numero o intervallo di celle.
- `...`: Quanti altri ne servono, dello stesso tipo del precedente.

## MDETERM

`MDETERM(array)`

Restituisce il determinante di un intervallo quadrato di numeri.

- `array`: Un intervallo di celle, oppure un elenco di valori scritto tra parentesi graffe.

## MIN

`MIN(number1, ...)`

Restituisce il numero più piccolo.

- `number1`: Il primo numero o intervallo di celle.
- `...`: Quanti altri ne servono, dello stesso tipo del precedente.

## MINVERSE

`MINVERSE(array)`

Restituisce la matrice inversa di un intervallo quadrato di numeri.

- `array`: Un intervallo di celle, oppure un elenco di valori scritto tra parentesi graffe.

## MMULT

`MMULT(array1, array2)`

Restituisce il prodotto di due intervalli di numeri, come matrici.

- `array1`: Il primo intervallo di celle o elenco di valori.
- `array2`: Il secondo intervallo di celle o elenco di valori, delle stesse dimensioni del primo.

## MOD

`MOD(number, divisor)`

Restituisce il resto della divisione di un numero per un altro.

- `number`: Il numero da elaborare.
- `divisor`: Il numero per cui dividere.

## MROUND

`MROUND(number, multiple)`

Arrotonda un numero al multiplo più vicino.

- `number`: Il numero da elaborare.
- `multiple`: Il numero i cui multipli usare per l'arrotondamento.

## MULTINOMIAL

`MULTINOMIAL(number1, ...)`

Restituisce il fattoriale della somma dei numeri diviso il prodotto dei loro fattoriali.

- `number1`: Il primo numero o intervallo di celle.
- `...`: Quanti altri ne servono, dello stesso tipo del precedente.

## MUNIT

`MUNIT(dimension)`

Restituisce la matrice identità di una dimensione.

- `dimension`: La dimensione dell'intervallo quadrato: quante righe e quante colonne.

## ODD

`ODD(number)`

Arrotonda un numero per eccesso al numero intero dispari più vicino.

- `number`: Il numero da elaborare.

## PERMUT

`PERMUT(number, number_chosen)`

Conta i modi di scegliere elementi da un gruppo quando l'ordine conta.

- `number`: Il numero da elaborare.
- `number_chosen`: Quanti elementi scegliere in ogni gruppo.

## PI

`PI()`

Restituisce il numero pi greco.

## POWER

`POWER(number, power)`

Eleva un numero a una potenza.

- `number`: Il numero da elaborare.
- `power`: La potenza a cui elevare il numero.

## PRODUCT

`PRODUCT(number1, ...)`

Moltiplica i numeri tra loro.

- `number1`: Il primo numero o intervallo di celle.
- `...`: Quanti altri ne servono, dello stesso tipo del precedente.

## QUOTIENT

`QUOTIENT(numerator, denominator)`

Restituisce la parte intera di una divisione.

- `numerator`: Il numero da dividere.
- `denominator`: Il numero per cui dividere.

## RADIANS

`RADIANS(angle)`

Converte i gradi in radianti.

- `angle`: L'angolo, in radianti.

## RAND

`RAND()`

Restituisce un numero casuale da 0 fino a 1, escluso 1.

## RANDARRAY

`RANDARRAY([rows], [columns], [min], [max], [whole_number])`

Restituisce un intervallo di numeri casuali.

- `rows` (facoltativo): Quante righe scendere: positivo verso il basso, negativo verso l'alto.
- `columns` (facoltativo): Quante colonne di larghezza ha il risultato (1 se omesso).
- `min` (facoltativo): Il numero minimo che il risultato può avere (0 se omesso).
- `max` (facoltativo): Il numero massimo che il risultato può avere (1 se omesso).
- `whole_number` (facoltativo): Vero per numeri interi; falso o omesso per numeri con decimali.

## RANDBETWEEN

`RANDBETWEEN(bottom, top)`

Restituisce un numero intero casuale compreso tra due numeri.

- `bottom`: Il valore minimo che il risultato può avere.
- `top`: Il valore massimo che il risultato può avere.

## ROMAN

`ROMAN(number, [form])`

Scrive un numero intero da 0 a 3999 come numeri romani, in forma di testo.

- `number`: Il numero da elaborare.
- `form` (facoltativo): Quanto devono essere brevi i numeri romani: 0 o omesso per la forma classica, fino a 4 per la forma più breve.

## ROUND

`ROUND(number, num_digits)`

Arrotonda un numero a un certo numero di cifre.

- `number`: Il numero da elaborare.
- `num_digits`: A quante cifre arrotondare: positivo per le cifre a destra della virgola decimale, 0 per un numero intero, negativo per quelle a sinistra.

## ROUNDDOWN

`ROUNDDOWN(number, num_digits)`

Arrotonda un numero per difetto, verso zero, a un certo numero di cifre.

- `number`: Il numero da elaborare.
- `num_digits`: A quante cifre arrotondare: positivo per le cifre a destra della virgola decimale, 0 per un numero intero, negativo per quelle a sinistra.

## ROUNDUP

`ROUNDUP(number, num_digits)`

Arrotonda un numero per eccesso, lontano da zero, a un certo numero di cifre.

- `number`: Il numero da elaborare.
- `num_digits`: A quante cifre arrotondare: positivo per le cifre a destra della virgola decimale, 0 per un numero intero, negativo per quelle a sinistra.

## SEC

`SEC(angle)`

Restituisce la secante di un angolo in radianti.

- `angle`: L'angolo, in radianti.

## SECH

`SECH(number)`

Restituisce la secante iperbolica di un numero.

- `number`: Il numero da elaborare.

## SEQUENCE

`SEQUENCE(rows, [columns], [start], [step])`

Restituisce un intervallo di numeri che crescono a partire da un valore iniziale con un passo.

- `rows`: Quante righe scendere: positivo verso il basso, negativo verso l'alto.
- `columns` (facoltativo): Quante colonne di larghezza ha il risultato (1 se omesso).
- `start` (facoltativo): Il primo numero (1 se omesso).
- `step` (facoltativo): Quanto aggiungere per ogni numero (1 se omesso; per GESTEP, il numero di confronto, 0 se omesso).

## SERIESSUM

`SERIESSUM(x, n, m, coefficients)`

Restituisce la somma di una serie di potenze.

- `x`: Il valore x da elaborare.
- `n`: La prima potenza di x, o l'ordine della funzione, un numero intero di 0 o più.
- `m`: Il passo da aggiungere alla potenza per ogni coefficiente.
- `coefficients`: I numeri per cui moltiplicare ogni potenza di x.

## SIGN

`SIGN(number)`

Restituisce 1 per un numero positivo, 0 per lo zero e -1 per un numero negativo.

- `number`: Il numero da elaborare.

## SIN

`SIN(number)`

Restituisce il seno di un angolo espresso in radianti.

- `number`: Il numero da elaborare.

## SINH

`SINH(number)`

Restituisce il seno iperbolico di un numero.

- `number`: Il numero da elaborare.

## SQRT

`SQRT(number)`

Restituisce la radice quadrata di un numero.

- `number`: Il numero da elaborare.

## SQRTPI

`SQRTPI(number)`

Restituisce la radice quadrata di un numero moltiplicato per pi greco.

- `number`: Il numero da elaborare.

## SUBTOTAL

`SUBTOTAL(function_num, ref1, ...)`

Restituisce un totale, una media, un conteggio o un'altra cifra per un intervallo di celle.

- `function_num`: Il calcolo da eseguire: 1 media, 2 conteggio, 3 conteggio non vuoti, 4 massimo, 5 minimo, 6 prodotto, 7 e 8 deviazione standard, 9 somma, 10 e 11 varianza.
- `ref1`: La prima cella o il primo intervallo di celle.
- `...`: Quanti altri ne servono, dello stesso tipo del precedente.

## SUM

`SUM(number1, ...)`

Somma i numeri e i numeri contenuti negli intervalli di celle.

- `number1`: Il primo numero o intervallo di celle.
- `...`: Quanti altri ne servono, dello stesso tipo del precedente.

## SUMPRODUCT

`SUMPRODUCT(array1, ...)`

Moltiplica tra loro i numeri di intervalli delle stesse dimensioni, posto per posto, e somma i prodotti.

- `array1`: Il primo intervallo di celle o elenco di valori.
- `...`: Quanti altri ne servono, dello stesso tipo del precedente.

## SUMSQ

`SUMSQ(number1, ...)`

Somma i quadrati dei numeri.

- `number1`: Il primo numero o intervallo di celle.
- `...`: Quanti altri ne servono, dello stesso tipo del precedente.

## SUMX2MY2

`SUMX2MY2(array_x, array_y)`

Somma le differenze dei quadrati di due insiemi di numeri.

- `array_x`: Il primo intervallo di celle o elenco di valori.
- `array_y`: Il secondo intervallo di celle o elenco di valori, delle stesse dimensioni del primo.

## SUMX2PY2

`SUMX2PY2(array_x, array_y)`

Somma le somme dei quadrati di due insiemi di numeri.

- `array_x`: Il primo intervallo di celle o elenco di valori.
- `array_y`: Il secondo intervallo di celle o elenco di valori, delle stesse dimensioni del primo.

## SUMXMY2

`SUMXMY2(array_x, array_y)`

Somma i quadrati delle differenze di due insiemi di numeri.

- `array_x`: Il primo intervallo di celle o elenco di valori.
- `array_y`: Il secondo intervallo di celle o elenco di valori, delle stesse dimensioni del primo.

## TAN

`TAN(number)`

Restituisce la tangente di un angolo espresso in radianti.

- `number`: Il numero da elaborare.

## TANH

`TANH(number)`

Restituisce la tangente iperbolica di un numero.

- `number`: Il numero da elaborare.

## TRUNC

`TRUNC(number, [num_digits])`

Tronca un numero a un certo numero di cifre, senza arrotondare.

- `number`: Il numero da elaborare.
- `num_digits` (facoltativo): A quante cifre arrotondare: positivo per le cifre a destra della virgola decimale, 0 per un numero intero, negativo per quelle a sinistra.
