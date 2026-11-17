# Funzioni di testo
<!-- keywords: funzioni, ASC, BAHTTEXT, CAR, CARATT, PULISCI, CODICE, CONCAT, CONCATENA, DBCS, DOLLARO, VALUTA, CODIFICA URL, UGUALE, FILTRO.XML, TROVA, TROVA.B, FISSO, SINISTRA, SINISTRA.B, LUNGHEZZA, LEN, LUNGHEZZA.B, MINUSC, LOWER, STRINGA.ESTRAI, STRINGA.ESTRAI.B, VALORE.NUMERO, FONETICO, MAIUSC.INIZ, SOSTITUISCI, SOSTITUISCI.B, RIPETI, DESTRA, DESTRA.B, CERCA, CERCA.B, SOSTITUISCI, T, TESTO, TESTO.UNISCI, ANNULLA.SPAZI, CAR.UNICODE, UNICODE, MAIUSC, VALORE -->

Queste sono le funzioni di testo che JeffCrunch può calcolare. Ogni funzione mostra come si scrive, che cosa fa e quali sono i suoi argomenti. Un argomento tra parentesi quadre è facoltativo, mentre "..." indica che puoi aggiungere quanti altri ne servono dell'ultimo. Per provare una funzione, vedi [Scegliere una funzione](insert-function). Mentre digiti una funzione in una cella, sotto la cella compare un piccolo suggerimento con i suoi argomenti; se fai clic sul nome della funzione nel suggerimento, si apre la sua scheda in questa pagina.

## ASC

`ASC(testo)`

Converte in caratteri a larghezza dimezzata le lettere e le cifre a larghezza piena presenti nel testo.

- `testo`: Il testo da elaborare, oppure una cella che lo contiene.

## BAHTTEXT

`BAHTTEXT(numero)`

Scrive un numero in parole tailandesi con la parola per il baht.

- `numero`: Il numero da elaborare.

## CAR

`CAR(numero)`

Restituisce il carattere corrispondente a un codice numerico.

- `numero`: Il numero da elaborare.

## PULISCI

`PULISCI(testo)`

Elimina dal testo i caratteri che non vengono stampati.

- `testo`: Il testo da elaborare, oppure una cella che lo contiene.

## CODICE

`CODICE(testo)`

Restituisce il codice numerico del primo carattere di un testo.

- `testo`: Il testo da elaborare, oppure una cella che lo contiene.

## CONCAT

`CONCAT(testo1, ...)`

Unisce in un unico testo più testi e i testi contenuti in intervalli di celle.

- `testo1`: Il primo testo, oppure un intervallo di celle con testo.
- `...`: Quanti altri ne servono, dello stesso tipo del precedente.

## CONCATENA

`CONCATENA(testo1, ...)`

Unisce più testi in uno solo.

- `testo1`: Il primo testo, oppure un intervallo di celle con testo.
- `...`: Quanti altri ne servono, dello stesso tipo del precedente.

## DBCS

`DBCS(testo)`

Converte in caratteri a larghezza piena le lettere e le cifre a larghezza dimezzata presenti nel testo.

- `testo`: Il testo da elaborare, oppure una cella che lo contiene.

## DOLLARO

`DOLLARO(numero, [decimali])`

Trasforma un numero in testo come importo di denaro.

- `numero`: Il numero da elaborare.
- `decimali` (facoltativo): Quante cifre mantenere dopo la virgola decimale.

## CODIFICA URL

`CODIFICA.URL(testo)`

Modifica il testo in modo che possa essere usato in un indirizzo web.

- `testo`: Il testo da elaborare, oppure una cella che lo contiene.

## UGUALE

`UGUALE(testo1, testo2)`

Restituisce VERO quando due testi sono esattamente uguali, maiuscole comprese.

- `testo1`: Il primo testo, oppure un intervallo di celle con testo.
- `testo2`: Il secondo testo.

## FILTRO.XML

`FILTRO.XML(xml, xpath)`

Estrae valori da un testo che contiene XML, usando un'espressione XPath.

- `xml`: Il testo che contiene l'XML.
- `xpath`: L'XPath che indica quali parti restituire, ad esempio //item/price.

## TROVA

`TROVA(testo_da_trovare, testo_in_cui_cercare, [num_inizio])`

Restituisce la posizione in cui un testo inizia all'interno di un altro, distinguendo maiuscole e minuscole.

- `testo_da_trovare`: Il testo da cercare.
- `testo_in_cui_cercare`: Il testo in cui cercare.
- `num_inizio` (facoltativo): La posizione nel testo da cui iniziare, contando il primo carattere come 1.

## TROVA.B

`TROVA.B(testo_da_trovare, testo_in_cui_cercare, [num_inizio])`

Restituisce la posizione in cui un testo compare per la prima volta in un altro testo, distinguendo maiuscole e minuscole.

- `testo_da_trovare`: Il testo da cercare.
- `testo_in_cui_cercare`: Il testo in cui cercare.
- `num_inizio` (facoltativo): La posizione nel testo da cui iniziare, contando il primo carattere come 1.

## FISSO

`FISSO(numero, [decimali], [senza_separatore])`

Trasforma un numero in testo con un numero fisso di decimali.

- `numero`: Il numero da elaborare.
- `decimali` (facoltativo): Quante cifre mantenere dopo la virgola decimale.
- `senza_separatore` (facoltativo): VERO per non inserire i separatori che raggruppano le migliaia.

## SINISTRA

`SINISTRA(testo, [num_caratteri])`

Restituisce i primi caratteri di un testo.

- `testo`: Il testo da elaborare, oppure una cella che lo contiene.
- `num_caratteri` (facoltativo): Quanti caratteri prendere (1 se omesso).

## SINISTRA.B

`SINISTRA.B(testo, [num_caratteri])`

Restituisce i primi caratteri del testo.

- `testo`: Il testo da elaborare, oppure una cella che lo contiene.
- `num_caratteri` (facoltativo): Quanti caratteri prendere (1 se omesso).

## LUNGHEZZA

`LUNGHEZZA(testo)`

Conta i caratteri di un testo.

- `testo`: Il testo da elaborare, oppure una cella che lo contiene.

## LUNGHEZZA.B

`LUNGHEZZA.B(testo)`

Restituisce il numero di caratteri del testo.

- `testo`: Il testo da elaborare, oppure una cella che lo contiene.

## MINUSC

`MINUSC(testo)`

Trasforma un testo in lettere minuscole.

- `testo`: Il testo da elaborare, oppure una cella che lo contiene.

## STRINGA.ESTRAI

`STRINGA.ESTRAI(testo, inizio_num, num_caratteri)`

Restituisce caratteri dalla parte centrale di un testo.

- `testo`: Il testo da elaborare, oppure una cella che lo contiene.
- `inizio_num`: La posizione nel testo da cui iniziare, contando il primo carattere come 1.
- `num_caratteri`: Quanti caratteri prendere (1 se omesso).

## STRINGA.ESTRAI.B

`STRINGA.ESTRAI.B(testo, inizio_num, num_caratteri)`

Restituisce caratteri dalla parte centrale del testo.

- `testo`: Il testo da elaborare, oppure una cella che lo contiene.
- `inizio_num`: La posizione nel testo da cui iniziare, contando il primo carattere come 1.
- `num_caratteri`: Quanti caratteri prendere (1 se omesso).

## VALORE.NUMERO

`VALORE.NUMERO(testo, [separatore_decimale], [separatore_gruppo])`

Legge un testo come numero, con i separatori che indichi.

- `testo`: Il testo da elaborare, oppure una cella che lo contiene.
- `separatore_decimale` (facoltativo): Il carattere che indica i decimali nel testo (il punto se omesso).
- `separatore_gruppo` (facoltativo): Il carattere che raggruppa le cifre nel testo (la virgola se omessa).

## FONETICO

`FONETICO(riferimento)`

Restituisce il testo di una cella, per le letture del testo giapponese.

- `riferimento`: Una cella o un intervallo di celle.

## MAIUSC.INIZ

`MAIUSC.INIZ(testo)`

Mette in maiuscolo la prima lettera di ogni parola di un testo.

- `testo`: Il testo da elaborare, oppure una cella che lo contiene.

## SOSTITUISCI

`SOSTITUISCI(testo_originale, inizio_num, num_caratteri, nuovo_testo)`

Inserisce nuovo testo al posto di alcuni caratteri di un testo.

- `testo_originale`: Il testo da modificare.
- `inizio_num`: La posizione nel testo da cui iniziare, contando il primo carattere come 1.
- `num_caratteri`: Quanti caratteri prendere (1 se omesso).
- `nuovo_testo`: Il testo da inserire.

## SOSTITUISCI.B

`SOSTITUISCI.B(testo_originale, inizio_num, num_caratteri, nuovo_testo)`

Sostituisce una parte del testo con un altro testo.

- `testo_originale`: Il testo da modificare.
- `inizio_num`: La posizione nel testo da cui iniziare, contando il primo carattere come 1.
- `num_caratteri`: Quanti caratteri prendere (1 se omesso).
- `nuovo_testo`: Il testo da inserire.

## RIPETI

`RIPETI(testo, num_volte)`

Ripete un testo per un numero di volte specificato.

- `testo`: Il testo da elaborare, oppure una cella che lo contiene.
- `num_volte`: Quante volte ripetere il testo.

## DESTRA

`DESTRA(testo, [num_caratteri])`

Restituisce gli ultimi caratteri di un testo.

- `testo`: Il testo da elaborare, oppure una cella che lo contiene.
- `num_caratteri` (facoltativo): Quanti caratteri prendere (1 se omesso).

## DESTRA.B

`DESTRA.B(testo, [num_caratteri])`

Restituisce gli ultimi caratteri del testo.

- `testo`: Il testo da elaborare, oppure una cella che lo contiene.
- `num_caratteri` (facoltativo): Quanti caratteri prendere (1 se omesso).

## CERCA

`CERCA(testo_da_trovare, testo_in_cui_cercare, [num_inizio])`

Restituisce la posizione in cui un testo inizia all'interno di un altro, ignorando le maiuscole e consentendo di usare ? e * in ciò che si cerca.

- `testo_da_trovare`: Il testo da cercare.
- `testo_in_cui_cercare`: Il testo in cui cercare.
- `num_inizio` (facoltativo): La posizione nel testo da cui iniziare, contando il primo carattere come 1.

## CERCA.B

`CERCA.B(testo_da_trovare, testo_in_cui_cercare, [num_inizio])`

Restituisce la posizione in cui un testo compare per la prima volta in un altro testo, ignorando le maiuscole.

- `testo_da_trovare`: Il testo da cercare.
- `testo_in_cui_cercare`: Il testo in cui cercare.
- `num_inizio` (facoltativo): La posizione nel testo da cui iniziare, contando il primo carattere come 1.

## SOSTITUISCI

`SOSTITUISCI(testo, testo_originale, nuovo_testo, [num_occorrenza])`

Scambia un pezzo di testo con un altro ovunque compaia.

- `testo`: Il testo da elaborare, oppure una cella che lo contiene.
- `testo_originale`: Il testo da trovare.
- `nuovo_testo`: Il testo da inserire.
- `num_occorrenza` (facoltativo): Quale corrispondenza sostituire (tutte se omesso).

## T

`T(valore)`

Restituisce il testo stesso, e nulla per ciò che non è testo.

- `valore`: Il valore da elaborare, oppure una cella che lo contiene.

## TESTO

`TESTO(valore, formato_testo)`

Trasforma un numero in testo in un formato a scelta.

- `valore`: Il valore da elaborare, oppure una cella che lo contiene.
- `formato_testo`: Il formato numerico con cui mostrare il valore, ad esempio "0,00" o "gg/mm/aaaa".

## TESTO.UNISCI

`TESTO.UNISCI(delimitatore, ignora_vuoti, testo1, ...)`

Unisce più testi inserendo un separatore tra di essi.

- `delimitatore`: Il testo da inserire tra le parti.
- `ignora_vuoti`: VERO per omettere le parti vuote; FALSO per mantenerle.
- `testo1`: Il primo testo, oppure un intervallo di celle con testo.
- `...`: Quanti altri ne servono, dello stesso tipo del precedente.

## ANNULLA.SPAZI

`ANNULLA.SPAZI(testo)`

Elimina gli spazi in eccesso da un testo, lasciando un solo spazio tra le parole.

- `testo`: Il testo da elaborare, oppure una cella che lo contiene.

## CAR.UNICODE

`CAR.UNICODE(numero)`

Restituisce il carattere corrispondente a un numero Unicode.

- `numero`: Il numero da elaborare.

## UNICODE

`UNICODE(testo)`

Restituisce il numero Unicode del primo carattere del testo.

- `testo`: Il testo da elaborare, oppure una cella che lo contiene.

## MAIUSC

`MAIUSC(testo)`

Trasforma un testo in lettere maiuscole.

- `testo`: Il testo da elaborare, oppure una cella che lo contiene.

## VALORE

`VALORE(testo)`

Trasforma un testo che sembra un numero in un numero.

- `testo`: Il testo da elaborare, oppure una cella che lo contiene.
