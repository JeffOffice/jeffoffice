# Funzioni informative
<!-- keywords: funzioni, CELL, ERROR.TYPE, INFO, ISBLANK, ISERR, ISERROR, ISEVEN, ISFORMULA, ISLOGICAL, ISNA, ISNONTEXT, ISNUMBER, ISODD, ISREF, ISTEXT, N, NA, SHEET, SHEETS, TYPE, informazioni, funzioni informative, verifica, tipo, errore, foglio -->

Queste sono le funzioni informative che JeffCrunch può calcolare. Ognuna mostra come si scrive, che cosa fa e quali sono i suoi argomenti. Un argomento tra parentesi quadre è facoltativo, e "..." indica che è possibile aggiungere quanti altri argomenti servono dopo l'ultimo. Per provarne una, vedi [Scegli una funzione](insert-function). Mentre digiti una funzione in una cella, sotto la cella compare un piccolo suggerimento con i suoi argomenti; se fai clic sul nome della funzione nel suggerimento, si apre la sua scheda in questa pagina.

## CELL

`CELL(tipo_info, [riferimento])`

Restituisce informazioni su una cella, come il suo indirizzo o il suo formato.

- `tipo_info`: Le informazioni da restituire sulla cella, come testo quale "address", "row", "col" o "format".
- `riferimento` (facoltativo): La cella su cui restituire le informazioni (la cella con il cursore, se omesso).

## ERROR.TYPE

`ERROR.TYPE(valore_errore)`

Restituisce il numero che corrisponde a un tipo di errore.

- `valore_errore`: L'errore da cercare, oppure una cella che lo contiene.

## INFO

`INFO(tipo_info)`

Restituisce informazioni sul programma o sulla cartella di lavoro, come la cartella in uso o il modo in cui vengono eseguiti i calcoli.

- `tipo_info`: Le informazioni da restituire, come testo quale "directory" o "recalc".

## ISBLANK

`ISBLANK(valore)`

Restituisce VERO se la cella è vuota.

- `valore`: Il valore da verificare, oppure una cella che lo contiene.

## ISERR

`ISERR(valore)`

Restituisce VERO se il valore è un errore diverso da valore non disponibile.

- `valore`: Il valore da verificare, oppure una cella che lo contiene.

## ISERROR

`ISERROR(valore)`

Restituisce VERO se il valore è un errore qualsiasi.

- `valore`: Il valore da verificare, oppure una cella che lo contiene.

## ISEVEN

`ISEVEN(numero)`

Restituisce VERO se un numero intero è pari.

- `numero`: Il numero da verificare.

## ISFORMULA

`ISFORMULA(riferimento)`

Restituisce VERO se la cella contiene una formula.

- `riferimento`: Una cella o un intervallo di celle.

## ISLOGICAL

`ISLOGICAL(valore)`

Restituisce VERO se il valore è vero o falso.

- `valore`: Il valore da verificare, oppure una cella che lo contiene.

## ISNA

`ISNA(valore)`

Restituisce VERO se il valore è l'errore di valore non disponibile.

- `valore`: Il valore da verificare, oppure una cella che lo contiene.

## ISNONTEXT

`ISNONTEXT(valore)`

Restituisce VERO se il valore è qualsiasi cosa tranne un testo.

- `valore`: Il valore da verificare, oppure una cella che lo contiene.

## ISNUMBER

`ISNUMBER(valore)`

Restituisce VERO se il valore è un numero.

- `valore`: Il valore da verificare, oppure una cella che lo contiene.

## ISODD

`ISODD(numero)`

Restituisce VERO se un numero intero è dispari.

- `numero`: Il numero da verificare.

## ISREF

`ISREF(valore)`

Restituisce VERO se il valore è un riferimento a celle.

- `valore`: Il valore da verificare, oppure una cella che lo contiene.

## ISTEXT

`ISTEXT(valore)`

Restituisce VERO se il valore è un testo.

- `valore`: Il valore da verificare, oppure una cella che lo contiene.

## N

`N(valore)`

Converte un valore in un numero: le date e i valori vero diventano numeri, il testo diventa zero.

- `valore`: Il valore da convertire, oppure una cella che lo contiene.

## NA

`NA()`

Restituisce l'errore di valore non disponibile, per segnalare un valore mancante.

## SHEET

`SHEET([valore])`

Restituisce il numero di un foglio, contando il primo foglio come 1.

- `valore` (facoltativo): Il valore da elaborare, oppure una cella che lo contiene.

## SHEETS

`SHEETS([riferimento])`

Restituisce il numero di fogli in un riferimento o nell'intera cartella di lavoro.

- `riferimento` (facoltativo): Una cella o un intervallo di celle.

## TYPE

`TYPE(valore)`

Restituisce un numero che indica il tipo di valore: numero, testo, logico, errore o matrice.

- `valore`: Il valore da elaborare, oppure una cella che lo contiene.
