# Funzioni di ricerca e riferimento
<!-- keywords: funzioni, ADDRESS, AREE, COLONNA, COLONNE, FILTER, FORMULATESTO, GETPIVOTDATA, HLOOKUP, COLLEGAMENTO IPERTESTUALE, INDICE, INDIRETTO, LOOKUP, CONFRONTA, OFFSET, RIGA, RIGHE, ORDINA, ORDINAPER, TRASPONI, UNICO, VLOOKUP, XLOOKUP, XCONFRONTA, funzione, ricerca, riferimento, cerca verticale, cerca orizzontale -->

Queste sono le funzioni di ricerca e riferimento che JeffCrunch è in grado di calcolare. Ognuna mostra come si scrive, che cosa fa e quali sono i suoi argomenti. Un argomento tra parentesi quadre è facoltativo, e "..." indica che si possono aggiungere quanti altri ne servono dell'ultimo. Per provarne una, vedi [Scegli una funzione](insert-function). Mentre si digita una funzione in una cella, un piccolo suggerimento sotto la cella mostra i suoi argomenti, e un clic sul nome della funzione nel suggerimento apre la sua scheda qui.

## ADDRESS

`ADDRESS(num_riga, num_colonna, [num_assoluto], [a1], [testo_foglio])`

Scrive come testo l'indirizzo di una cella, a partire dal numero di riga e di colonna.

- `num_riga`: Il numero della riga nell'intervallo, contando la prima riga come 1.
- `num_colonna`: Il numero della colonna nell'intervallo, contando la prima colonna come 1.
- `num_assoluto` (facoltativo): Quanta parte dell'indirizzo è fissa: 1 tutto, 2 solo la riga, 3 solo la colonna, 4 nulla.
- `a1` (facoltativo): Vero o omesso per leggere l'indirizzo come lettera di colonna e numero di riga; falso per leggerlo come numeri di riga e di colonna.
- `testo_foglio` (facoltativo): Il nome del foglio da inserire prima dell'indirizzo.

## AREE

`AREE(riferimento)`

Restituisce il numero di intervalli di celle separati in un riferimento.

- `riferimento`: Una cella o un intervallo di celle.

## COLONNA

`COLONNA([riferimento])`

Restituisce il numero di colonna di una cella.

- `riferimento` (facoltativo): Una cella o un intervallo di celle.

## COLONNE

`COLONNE(matrice)`

Conta le colonne di un intervallo di celle.

- `matrice`: Un intervallo di celle, oppure un elenco di valori scritto tra parentesi graffe.

## FILTRA

`FILTER(matrice, includi, [se_vuoto])`

Restituisce le righe o le colonne di un intervallo che superano un test.

- `matrice`: Un intervallo di celle, oppure un elenco di valori scritto tra parentesi graffe.
- `includi`: Una colonna o una riga di valori vero e falso, uno per ogni riga o colonna dell'intervallo: vero la mantiene.
- `se_vuoto` (facoltativo): Ciò che restituire quando nessuna riga o colonna supera il test (l'errore #CALC! se omesso).

## FORMULATESTO

`FORMULATEXT(riferimento)`

Restituisce come testo la formula di una cella.

- `riferimento`: Una cella o un intervallo di celle.

## INFO.TABELLA.PIVOT

`GETPIVOTDATA(campo_dati, tabella_pivot, [campo1], [elemento1], ...)`

Restituisce il valore che una tabella pivot mostra per un valore e per gli elementi delle sue righe e colonne. Una cella della tabella è il secondo argomento; i campi indicati devono essere i primi delle righe o delle colonne.

- `campo_dati`: Il nome del valore da restituire, tra virgolette, come lo mostra la tabella pivot ("Somma di Importo").
- `tabella_pivot`: Una cella qualsiasi della tabella pivot.
- `campo1` (facoltativo): Il nome di un campo delle righe o delle colonne, tra virgolette (può essere omesso insieme al suo elemento).
- `elemento1` (facoltativo): L'elemento di quel campo per cui restituire il valore, tra virgolette.
- `...`: Quanti altri ne servono, dello stesso tipo del precedente.

## HLOOKUP

`HLOOKUP(valore_cercato, matrice_tabella, num_indice_riga, [intervallo_cerca])`

Cerca un valore nella prima riga di una tabella e restituisce un valore della stessa colonna.

- `valore_cercato`: Il valore da cercare.
- `matrice_tabella`: L'intervallo di celle in cui cercare; il valore cercato deve trovarsi nella sua prima colonna.
- `num_indice_riga`: Il numero della riga della tabella da cui restituire un valore, contando la prima riga come 1.
- `intervallo_cerca` (facoltativo): Falso trova una corrispondenza esatta; vero o omesso trova il valore più vicino inferiore a quello cercato (la prima colonna deve essere ordinata in modo crescente).

## COLLEGAMENTO.IPERTESTUALE

`HYPERLINK(posizione_collegamento, [nome_descrittivo])`

Crea una cella che apre un collegamento quando viene fatto clic su di essa.

- `posizione_collegamento`: Dove porta il collegamento: un indirizzo web, un file o una posizione in questa cartella di lavoro.
- `nome_descrittivo` (facoltativo): Il testo da mostrare nella cella (il collegamento stesso se omesso).

## INDICE

`INDEX(matrice, num_riga, [num_colonna], [num_area])`

Restituisce il valore in una riga e in una colonna di un intervallo di celle.

- `matrice`: Un intervallo di celle, oppure un elenco di valori scritto tra parentesi graffe.
- `num_riga`: Il numero della riga nell'intervallo, contando la prima riga come 1.
- `num_colonna` (facoltativo): Il numero della colonna nell'intervallo, contando la prima colonna come 1.
- `num_area` (facoltativo): Quale intervallo usare quando il primo argomento ne indica più di uno (1 per il primo).

## INDIRETTO

`INDIRECT(testo_rif, [a1])`

Restituisce la cella a cui punta un indirizzo scritto come testo.

- `testo_rif`: Un indirizzo scritto come testo, ad esempio "B5".
- `a1` (facoltativo): Vero o omesso per leggere l'indirizzo come lettera di colonna e numero di riga; falso per leggerlo come numeri di riga e di colonna.

## LOOKUP

`LOOKUP(valore_cercato, vettore_cerca, [vettore_risultato])`

Cerca un valore in una riga o in una colonna e restituisce il valore nello stesso punto di un'altra.

- `valore_cercato`: Il valore da cercare.
- `vettore_cerca`: L'unica riga o l'unica colonna di celle in cui cercare.
- `vettore_risultato` (facoltativo): L'unica riga o l'unica colonna di celle da cui prendere il risultato.

## CONFRONTA

`MATCH(valore_cercato, matrice_cerca, [tipo_confronto])`

Restituisce la posizione di un valore in un elenco.

- `valore_cercato`: Il valore da cercare.
- `matrice_cerca`: L'intervallo di celle, una riga o una colonna, in cui cercare.
- `tipo_confronto` (facoltativo): 1 o omesso trova il valore più grande non superiore a quello cercato (l'elenco deve essere ordinato in modo crescente); 0 trova una corrispondenza esatta; -1 trova il valore più piccolo non inferiore a quello cercato (l'elenco deve essere ordinato in modo decrescente).

## OFFSET

`OFFSET(riferimento, righe, colonne, [altezza], [larghezza])`

Restituisce le celle distanti un numero dato di righe e colonne da una cella di partenza.

- `riferimento`: Una cella o un intervallo di celle.
- `righe`: Di quante righe spostarsi: positivo verso il basso, negativo verso l'alto.
- `colonne`: Di quante colonne spostarsi: positivo verso destra, negativo verso sinistra.
- `altezza` (facoltativo): Quante righe deve avere il risultato in altezza.
- `larghezza` (facoltativo): Quante colonne deve avere il risultato in larghezza.

## RIGA

`ROW([riferimento])`

Restituisce il numero di riga di una cella.

- `riferimento` (facoltativo): Una cella o un intervallo di celle.

## RIGHE

`ROWS(matrice)`

Conta le righe di un intervallo di celle.

- `matrice`: Un intervallo di celle, oppure un elenco di valori scritto tra parentesi graffe.

## ORDINA

`SORT(matrice, [indice_ordinamento], [ordine], [per_colonna])`

Restituisce un intervallo con le sue righe o colonne ordinate.

- `matrice`: Un intervallo di celle, oppure un elenco di valori scritto tra parentesi graffe.
- `indice_ordinamento` (facoltativo): Il numero della colonna (o della riga) in base a cui ordinare (1 se omesso).
- `ordine` (facoltativo): 1 o omesso per crescente; -1 per decrescente.
- `per_colonna` (facoltativo): Vero per lavorare sulle colonne; falso o omesso per lavorare sulle righe.

## ORDINAPER

`SORTBY(matrice, matrice_ordine1, [ordine1], ...)`

Restituisce un intervallo disposto secondo l'ordine di uno o più altri intervalli.

- `matrice`: Un intervallo di celle, oppure un elenco di valori scritto tra parentesi graffe.
- `matrice_ordine1`: L'intervallo di celle secondo cui ordinare il primo; deve avere la stessa lunghezza del primo.
- `ordine1` (facoltativo): 1 o omesso per crescente; -1 per decrescente.
- `...`: Quanti altri ne servono, dello stesso tipo del precedente.

## TRASPONI

`TRANSPOSE(matrice)`

Trasforma le righe in colonne e le colonne in righe.

- `matrice`: Un intervallo di celle, oppure un elenco di valori scritto tra parentesi graffe.

## UNICO

`UNIQUE(matrice, [per_colonna], [una_sola_volta])`

Restituisce le righe o le colonne diverse di un intervallo.

- `matrice`: Un intervallo di celle, oppure un elenco di valori scritto tra parentesi graffe.
- `per_colonna` (facoltativo): Vero per lavorare sulle colonne; falso o omesso per lavorare sulle righe.
- `una_sola_volta` (facoltativo): Vero per restituire solo gli elementi che compaiono una sola volta; falso o omesso per restituire ogni elemento diverso.

## VLOOKUP

`VLOOKUP(valore_cercato, matrice_tabella, num_indice_colonna, [intervallo_cerca])`

Cerca un valore nella prima colonna di una tabella e restituisce un valore della stessa riga.

- `valore_cercato`: Il valore da cercare.
- `matrice_tabella`: L'intervallo di celle in cui cercare; il valore cercato deve trovarsi nella sua prima colonna.
- `num_indice_colonna`: Il numero della colonna della tabella da cui restituire un valore, contando la prima colonna come 1.
- `intervallo_cerca` (facoltativo): Falso trova una corrispondenza esatta; vero o omesso trova il valore più vicino inferiore a quello cercato (la prima colonna deve essere ordinata in modo crescente).

## XLOOKUP

`XLOOKUP(valore_cercato, matrice_cerca, matrice_restituita, [se_non_trovato], [modo_confronto], [modo_ricerca])`

Cerca un valore in un elenco e restituisce l'elemento corrispondente di un altro elenco.

- `valore_cercato`: Il valore da cercare.
- `matrice_cerca`: L'intervallo di celle, una riga o una colonna, in cui cercare.
- `matrice_restituita`: L'intervallo di celle da cui prendere la corrispondenza.
- `se_non_trovato` (facoltativo): Ciò che restituire quando non c'è corrispondenza (l'errore #N/D se omesso).
- `modo_confronto` (facoltativo): 0 o omesso per una corrispondenza esatta, -1 per il valore inferiore successivo, 1 per il valore superiore successivo, 2 per usare i caratteri jolly.
- `modo_ricerca` (facoltativo): 1 o omesso per cercare dal primo, -1 dall'ultimo, 2 o -2 per una ricerca binaria su un elenco ordinato.

## XCONFRONTA

`XMATCH(valore_cercato, matrice_cerca, [modo_confronto], [modo_ricerca])`

Restituisce la posizione di un valore in un elenco, con opzioni per il modo di confronto e il verso di ricerca.

- `valore_cercato`: Il valore da cercare.
- `matrice_cerca`: L'intervallo di celle, una riga o una colonna, in cui cercare.
- `modo_confronto` (facoltativo): 0 o omesso per una corrispondenza esatta, -1 per il valore inferiore successivo, 1 per il valore superiore successivo, 2 per usare i caratteri jolly.
- `modo_ricerca` (facoltativo): 1 o omesso per cercare dal primo, -1 dall'ultimo, 2 o -2 per una ricerca binaria su un elenco ordinato.
