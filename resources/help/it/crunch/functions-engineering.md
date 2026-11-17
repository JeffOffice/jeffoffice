# Funzioni di ingegneria
<!-- keywords: funzioni, BESSELI, BESSELJ, BESSELK, BESSELY, BIN2DEC, BIN2HEX, BIN2OCT, BITAND, BITLSHIFT, BITOR, BITRSHIFT, BITXOR, COMPLEX, CONVERT, DEC2BIN, DEC2HEX, DEC2OCT, DELTA, ERF, ERF.PRECISE, ERFC, ERFC.PRECISE, GESTEP, HEX2BIN, HEX2DEC, HEX2OCT, IMABS, IMAGINARY, IMARGUMENT, IMCONJUGATE, IMCOS, IMCOSH, IMCOT, IMCSC, IMCSCH, IMDIV, IMEXP, IMLN, IMLOG10, IMLOG2, IMPOWER, IMPRODUCT, IMREAL, IMSEC, IMSECH, IMSIN, IMSINH, IMSQRT, IMSUB, IMSUM, IMTAN, OCT2BIN, OCT2DEC, OCT2HEX -->

Queste sono le funzioni di ingegneria che JeffCrunch può calcolare. Ogni funzione mostra come si scrive, cosa fa e quali sono i suoi argomenti. Un argomento tra parentesi quadre è facoltativo, e "..." indica che è possibile aggiungere quanti altri ne servono dell'ultimo tipo. Per provarne una, vedi [Scegli una funzione](insert-function). Mentre digiti una funzione in una cella, sotto la cella compare un piccolo suggerimento con i suoi argomenti: facendo clic sul nome della funzione nel suggerimento si apre la sua scheda in questa pagina.

## BESSELI

`BESSELI(x; n)`

Restituisce la funzione di Bessel modificata di prima specie.

- `x`: Il valore x da elaborare.
- `n`: Il primo esponente di x, o l'ordine della funzione, un numero intero pari o superiore a 0.

## BESSELJ

`BESSELJ(x; n)`

Restituisce la funzione di Bessel di prima specie.

- `x`: Il valore x da elaborare.
- `n`: Il primo esponente di x, o l'ordine della funzione, un numero intero pari o superiore a 0.

## BESSELK

`BESSELK(x; n)`

Restituisce la funzione di Bessel modificata di seconda specie.

- `x`: Il valore x da elaborare.
- `n`: Il primo esponente di x, o l'ordine della funzione, un numero intero pari o superiore a 0.

## BESSELY

`BESSELY(x; n)`

Restituisce la funzione di Bessel di seconda specie.

- `x`: Il valore x da elaborare.
- `n`: Il primo esponente di x, o l'ordine della funzione, un numero intero pari o superiore a 0.

## BIN2DEC

`BIN2DEC(numero)`

Converte un numero binario in un numero decimale.

- `numero`: Il numero da elaborare.

## BIN2HEX

`BIN2HEX(numero; [posizioni])`

Converte un numero binario in un numero esadecimale.

- `numero`: Il numero da elaborare.
- `posizioni` (facoltativo): Il numero di caratteri a cui completare il risultato con zeri a sinistra.

## BIN2OCT

`BIN2OCT(numero; [posizioni])`

Converte un numero binario in un numero ottale.

- `numero`: Il numero da elaborare.
- `posizioni` (facoltativo): Il numero di caratteri a cui completare il risultato con zeri a sinistra.

## BITAND

`BITAND(numero1; numero2)`

Restituisce i bit impostati in entrambi i numeri interi.

- `numero1`: Il primo numero o intervallo di celle.
- `numero2`: Il secondo numero.

## BITLSHIFT

`BITLSHIFT(numero; quantità_spostamento)`

Sposta i bit di un numero intero verso sinistra.

- `numero`: Il numero da elaborare.
- `quantità_spostamento`: Quanti bit spostare: i numeri positivi e negativi vanno in direzioni opposte.

## BITOR

`BITOR(numero1; numero2)`

Restituisce i bit impostati in almeno uno dei due numeri interi.

- `numero1`: Il primo numero o intervallo di celle.
- `numero2`: Il secondo numero.

## BITRSHIFT

`BITRSHIFT(numero; quantità_spostamento)`

Sposta i bit di un numero intero verso destra.

- `numero`: Il numero da elaborare.
- `quantità_spostamento`: Quanti bit spostare: i numeri positivi e negativi vanno in direzioni opposte.

## BITXOR

`BITXOR(numero1; numero2)`

Restituisce i bit impostati in uno solo dei due numeri interi, ma non in entrambi.

- `numero1`: Il primo numero o intervallo di celle.
- `numero2`: Il secondo numero.

## COMPLEX

`COMPLEX(parte_reale; parte_immaginaria; [suffisso])`

Crea un numero complesso dalle sue parti reale e immaginaria.

- `parte_reale`: La parte reale del numero complesso.
- `parte_immaginaria`: La parte immaginaria del numero complesso.
- `suffisso` (facoltativo): i o j, per indicare la parte immaginaria (i se omesso).

## CONVERT

`CONVERT(numero; unità_di_partenza; unità_di_arrivo)`

Converte un numero da un'unità di misura a un'altra.

- `numero`: Il numero da elaborare.
- `unità_di_partenza`: L'unità in cui si trova il numero, come testo, ad esempio "mi" o "lbm".
- `unità_di_arrivo`: L'unità in cui convertire il numero, come testo, ad esempio "km" o "kg".

## DEC2BIN

`DEC2BIN(numero; [posizioni])`

Converte un numero decimale in un numero binario.

- `numero`: Il numero da elaborare.
- `posizioni` (facoltativo): Il numero di caratteri a cui completare il risultato con zeri a sinistra.

## DEC2HEX

`DEC2HEX(numero; [posizioni])`

Converte un numero decimale in un numero esadecimale.

- `numero`: Il numero da elaborare.
- `posizioni` (facoltativo): Il numero di caratteri a cui completare il risultato con zeri a sinistra.

## DEC2OCT

`DEC2OCT(numero; [posizioni])`

Converte un numero decimale in un numero ottale.

- `numero`: Il numero da elaborare.
- `posizioni` (facoltativo): Il numero di caratteri a cui completare il risultato con zeri a sinistra.

## DELTA

`DELTA(numero1; [numero2])`

Indica se due numeri sono uguali, con 1 per sì e 0 per no.

- `numero1`: Il primo numero o intervallo di celle.
- `numero2` (facoltativo): Il secondo numero.

## ERF

`ERF(limite_inferiore; [limite_superiore])`

Restituisce la funzione di errore tra due limiti oppure da 0.

- `limite_inferiore`: Il limite inferiore dell'intervallo.
- `limite_superiore` (facoltativo): Il limite superiore dell'intervallo (se omesso, vale solo il limite inferiore).

## ERF.PRECISE

`ERF.PRECISE(x)`

Restituisce la funzione di errore da 0 a un limite.

- `x`: Il valore x da elaborare.

## ERFC

`ERFC(x)`

Restituisce la funzione di errore complementare da un limite all'infinito.

- `x`: Il valore x da elaborare.

## ERFC.PRECISE

`ERFC.PRECISE(x)`

Restituisce la funzione di errore complementare da un limite all'infinito.

- `x`: Il valore x da elaborare.

## GESTEP

`GESTEP(numero; [passo])`

Indica se un numero è almeno pari a un passo, con 1 per sì e 0 per no.

- `numero`: Il numero da elaborare.
- `passo` (facoltativo): Il valore con cui confrontare il numero (0 se omesso).

## HEX2BIN

`HEX2BIN(numero; [posizioni])`

Converte un numero esadecimale in un numero binario.

- `numero`: Il numero da elaborare.
- `posizioni` (facoltativo): Il numero di caratteri a cui completare il risultato con zeri a sinistra.

## HEX2DEC

`HEX2DEC(numero)`

Converte un numero esadecimale in un numero decimale.

- `numero`: Il numero da elaborare.

## HEX2OCT

`HEX2OCT(numero; [posizioni])`

Converte un numero esadecimale in un numero ottale.

- `numero`: Il numero da elaborare.
- `posizioni` (facoltativo): Il numero di caratteri a cui completare il risultato con zeri a sinistra.

## IMABS

`IMABS(numero_complesso)`

Restituisce il modulo di un numero complesso.

- `numero_complesso`: Un numero complesso scritto come testo, ad esempio "3+4i".

## IMAGINARY

`IMAGINARY(numero_complesso)`

Restituisce la parte immaginaria di un numero complesso.

- `numero_complesso`: Un numero complesso scritto come testo, ad esempio "3+4i".

## IMARGUMENT

`IMARGUMENT(numero_complesso)`

Restituisce l'angolo di un numero complesso, in radianti.

- `numero_complesso`: Un numero complesso scritto come testo, ad esempio "3+4i".

## IMCONJUGATE

`IMCONJUGATE(numero_complesso)`

Restituisce il coniugato di un numero complesso.

- `numero_complesso`: Un numero complesso scritto come testo, ad esempio "3+4i".

## IMCOS

`IMCOS(numero_complesso)`

Restituisce il coseno di un numero complesso.

- `numero_complesso`: Un numero complesso scritto come testo, ad esempio "3+4i".

## IMCOSH

`IMCOSH(numero_complesso)`

Restituisce il coseno iperbolico di un numero complesso.

- `numero_complesso`: Un numero complesso scritto come testo, ad esempio "3+4i".

## IMCOT

`IMCOT(numero_complesso)`

Restituisce la cotangente di un numero complesso.

- `numero_complesso`: Un numero complesso scritto come testo, ad esempio "3+4i".

## IMCSC

`IMCSC(numero_complesso)`

Restituisce la cosecante di un numero complesso.

- `numero_complesso`: Un numero complesso scritto come testo, ad esempio "3+4i".

## IMCSCH

`IMCSCH(numero_complesso)`

Restituisce la cosecante iperbolica di un numero complesso.

- `numero_complesso`: Un numero complesso scritto come testo, ad esempio "3+4i".

## IMDIV

`IMDIV(numero_complesso1; numero_complesso2)`

Restituisce il quoziente di due numeri complessi.

- `numero_complesso1`: Il primo numero complesso, scritto come testo, ad esempio "3+4i".
- `numero_complesso2`: Il secondo numero complesso, scritto come testo.

## IMEXP

`IMEXP(numero_complesso)`

Restituisce e elevato alla potenza di un numero complesso.

- `numero_complesso`: Un numero complesso scritto come testo, ad esempio "3+4i".

## IMLN

`IMLN(numero_complesso)`

Restituisce il logaritmo naturale di un numero complesso.

- `numero_complesso`: Un numero complesso scritto come testo, ad esempio "3+4i".

## IMLOG10

`IMLOG10(numero_complesso)`

Restituisce il logaritmo in base 10 di un numero complesso.

- `numero_complesso`: Un numero complesso scritto come testo, ad esempio "3+4i".

## IMLOG2

`IMLOG2(numero_complesso)`

Restituisce il logaritmo in base 2 di un numero complesso.

- `numero_complesso`: Un numero complesso scritto come testo, ad esempio "3+4i".

## IMPOWER

`IMPOWER(numero_complesso; numero)`

Restituisce un numero complesso elevato a una potenza.

- `numero_complesso`: Un numero complesso scritto come testo, ad esempio "3+4i".
- `numero`: Il numero da elaborare.

## IMPRODUCT

`IMPRODUCT(numero_complesso1; ...)`

Restituisce il prodotto di numeri complessi.

- `numero_complesso1`: Il primo numero complesso, scritto come testo, ad esempio "3+4i".
- `...`: Quanti altri ne servono, dello stesso tipo del precedente.

## IMREAL

`IMREAL(numero_complesso)`

Restituisce la parte reale di un numero complesso.

- `numero_complesso`: Un numero complesso scritto come testo, ad esempio "3+4i".

## IMSEC

`IMSEC(numero_complesso)`

Restituisce la secante di un numero complesso.

- `numero_complesso`: Un numero complesso scritto come testo, ad esempio "3+4i".

## IMSECH

`IMSECH(numero_complesso)`

Restituisce la secante iperbolica di un numero complesso.

- `numero_complesso`: Un numero complesso scritto come testo, ad esempio "3+4i".

## IMSIN

`IMSIN(numero_complesso)`

Restituisce il seno di un numero complesso.

- `numero_complesso`: Un numero complesso scritto come testo, ad esempio "3+4i".

## IMSINH

`IMSINH(numero_complesso)`

Restituisce il seno iperbolico di un numero complesso.

- `numero_complesso`: Un numero complesso scritto come testo, ad esempio "3+4i".

## IMSQRT

`IMSQRT(numero_complesso)`

Restituisce la radice quadrata di un numero complesso.

- `numero_complesso`: Un numero complesso scritto come testo, ad esempio "3+4i".

## IMSUB

`IMSUB(numero_complesso1; numero_complesso2)`

Restituisce la differenza tra due numeri complessi.

- `numero_complesso1`: Il primo numero complesso, scritto come testo, ad esempio "3+4i".
- `numero_complesso2`: Il secondo numero complesso, scritto come testo.

## IMSUM

`IMSUM(numero_complesso1; ...)`

Restituisce la somma di numeri complessi.

- `numero_complesso1`: Il primo numero complesso, scritto come testo, ad esempio "3+4i".
- `...`: Quanti altri ne servono, dello stesso tipo del precedente.

## IMTAN

`IMTAN(numero_complesso)`

Restituisce la tangente di un numero complesso.

- `numero_complesso`: Un numero complesso scritto come testo, ad esempio "3+4i".

## OCT2BIN

`OCT2BIN(numero; [posizioni])`

Converte un numero ottale in un numero binario.

- `numero`: Il numero da elaborare.
- `posizioni` (facoltativo): Il numero di caratteri a cui completare il risultato con zeri a sinistra.

## OCT2DEC

`OCT2DEC(numero)`

Converte un numero ottale in un numero decimale.

- `numero`: Il numero da elaborare.

## OCT2HEX

`OCT2HEX(numero; [posizioni])`

Converte un numero ottale in un numero esadecimale.

- `numero`: Il numero da elaborare.
- `posizioni` (facoltativo): Il numero di caratteri a cui completare il risultato con zeri a sinistra.
