# Funzioni logiche
<!-- keywords: funzioni, E, AND, SCEGLI, CHOOSE, FALSO, FALSE, SE, IF, SE.ERRORE, IFERROR, SE.NON.DISP, IFNA, SE.MULTIPLO, IFS, LET, NON, NOT, O, OR, CAMBIA, SWITCH, VERO, TRUE, O.ESCLUSIVO, XOR -->

Ecco le funzioni logiche che JeffCrunch può calcolare. Ogni funzione mostra come si scrive, cosa fa e quali sono i suoi argomenti. Un argomento tra parentesi quadre è facoltativo, e "..." indica che se ne possono aggiungere quanti ne servono dopo l'ultimo. Per provare una funzione, vedi [Scegli una funzione](insert-function). Mentre digiti una funzione in una cella, sotto la cella compare un piccolo suggerimento con i suoi argomenti; facendo clic sul nome della funzione nel suggerimento si apre la sua scheda in questa pagina.

## E

`E(logico1, ...)`

Vero quando tutte le condizioni sono vere.

- `logico1`: Il primo valore o test, che è vero o falso.
- `...`: Quanti altri ne servono, dello stesso tipo del precedente.

## SCEGLI

`SCEGLI(indice, valore1, ...)`

Sceglie il valore di un elenco che si trova nella posizione indicata dall'indice.

- `indice`: Quale valore restituire, contando il primo come 1.
- `valore1`: Il primo valore o intervallo di celle.
- `...`: Quanti altri ne servono, dello stesso tipo del precedente.

## FALSO

`FALSO()`

Il valore logico falso.

## SE

`SE(condizione; valore_se_vero; [valore_se_falso])`

Restituisce un valore quando una condizione è vera e un altro quando non lo è.

- `condizione`: Un valore o un test che è vero o falso.
- `valore_se_vero`: Ciò che restituire se la condizione è vera.
- `valore_se_falso` (facoltativo): Ciò che restituire se la condizione è falsa (FALSO se omesso).

## SE.ERRORE

`SE.ERRORE(valore; valore_se_errore)`

Restituisce un valore oppure un valore sostitutivo se il valore è un errore.

- `valore`: Il valore su cui lavorare, o una cella che lo contiene.
- `valore_se_errore`: Ciò che restituire se il valore è un errore.

## SE.NON.DISP

`SE.NON.DISP(valore; valore_se_non_disp)`

Restituisce un valore oppure un valore sostitutivo se il valore è l'errore di valore non disponibile.

- `valore`: Il valore su cui lavorare, o una cella che lo contiene.
- `valore_se_non_disp`: Ciò che restituire se il valore è l'errore di valore non disponibile.

## SE.MULTIPLO

`SE.MULTIPLO(condizione1; valore1; ...)`

Verifica le condizioni in sequenza e restituisce il valore associato alla prima che risulta vera.

- `condizione1`: Il primo test, che è vero o falso.
- `valore1`: Il primo valore o intervallo di celle.
- `...`: Quanti altri ne servono, dello stesso tipo del precedente.

## LET

`LET(nome1; valore1; ...)`

Assegna nomi ai valori all'interno di una sola formula, poi calcola un'operazione che li utilizza.

- `nome1`: Un nome da assegnare al valore che segue. Può essere usato nei valori successivi e nel calcolo.
- `valore1`: Il primo valore o intervallo di celle.
- `...`: Quanti altri ne servono, dello stesso tipo del precedente.

## NON

`NON(logico)`

Trasforma il vero in falso e il falso in vero.

- `logico`: Un valore o un test che è vero o falso.

## O

`O(logico1, ...)`

Vero quando almeno una delle condizioni è vera.

- `logico1`: Il primo valore o test, che è vero o falso.
- `...`: Quanti altri ne servono, dello stesso tipo del precedente.

## CAMBIA

`CAMBIA(espressione; valore1; risultato1; ...)`

Confronta un valore con un elenco di valori e restituisce il risultato associato alla prima corrispondenza.

- `espressione`: Il valore da confrontare con i valori che seguono.
- `valore1`: Il primo valore o intervallo di celle.
- `risultato1`: Il risultato da restituire quando il valore che lo precede corrisponde.
- `...`: Quanti altri ne servono, dello stesso tipo del precedente.

## VERO

`VERO()`

Il valore logico vero.

## O.ESCLUSIVO

`O.ESCLUSIVO(logico1, ...)`

Vero quando un numero dispari di condizioni è vero.

- `logico1`: Il primo valore o test, che è vero o falso.
- `...`: Quanti altri ne servono, dello stesso tipo del precedente.
