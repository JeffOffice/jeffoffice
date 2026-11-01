# Come vengono visualizzate e calcolate le formule
<!-- keywords: formula, formule, calcolo, calcolare, ricalcolare, ricalcolo, funzione, funzioni, valore, risultato, memorizzato, F9, circolare, riferimento circolare, mostra formule, barra della formula, errore -->

Una **formula** è una cella che calcola il proprio valore a partire da altre celle, ad esempio =SOMMA(B3:B8). La cella mostra il valore, mentre la **barra della formula** mostra la formula.

## Cosa fa JeffCrunch con le formule

Quando si apre una cartella di lavoro, il file contiene l'ultimo valore di ogni formula, quindi il foglio può essere visualizzato subito. JeffCrunch ricalcola poi tutte le formule, alcune alla volta, in modo che la finestra resti reattiva. Durante l'operazione, la barra di stato mostra **Calcolo in corso…** con una percentuale. Al termine, le celle contengono i risultati calcolati da JeffCrunch. Per ricalcolare tutto, fare clic su **Ricalcola tutto** nella scheda **Formule** oppure premere **F9**.

## Quando JeffCrunch non riesce a calcolare una formula

JeffCrunch dispone di oltre 200 funzioni, e il numero è in crescita. Una formula che usa una funzione non ancora disponibile in JeffCrunch mantiene il valore memorizzato nel file. Lo stesso vale per una formula che fa riferimento a un'altra cartella di lavoro. Le celle che dipendono da queste formule vengono calcolate a partire dai valori memorizzati.

## Mostrare le formule

Fare clic su **Mostra formule** nella scheda **Formule** oppure premere **CTRL+\\`** (su Mac, il tasto Control, non il tasto Comando) per visualizzare in ogni cella la formula al posto del valore. Le colonne che contengono formule vengono allargate, entro un certo limite, in modo che le formule siano visibili per intero; quando si disattiva l'opzione, le colonne tornano alla larghezza precedente e le larghezze salvate nel file non vengono modificate.

## Modificare le formule

Per modificare una formula, selezionare la cella, premere **F2**, modificare la formula e premere **INVIO**. È anche possibile modificarla nella barra della formula. Quando si modifica una cella, le formule che dipendono da essa vengono ricalcolate in background. Vedere [Digitare nelle celle](edit-cells).

## Scrivere una formula

Digitando **=** e le prime lettere di una funzione, viene visualizzato un breve elenco delle funzioni e dei [nomi](names) che iniziano con quelle lettere, ciascuno con una breve descrizione. Premere **TAB** per inserire l'elemento evidenziato. Usare i tasti **FRECCIA GIÙ** e **FRECCIA SU** per evidenziarne un altro, quindi premere **TAB** o **INVIO**. **ESC** chiude l'elenco. Quando il cursore si trova tra le parentesi di una funzione, i relativi argomenti vengono visualizzati sotto la cella, con quello in corso di digitazione in grassetto. L'elenco e gli argomenti sono gli stessi mostrati da **Scegli una funzione**. Gli argomenti sono separati da virgole oppure, nelle lingue che usano la virgola come separatore decimale (come l'italiano), da punti e virgola.

## Calcolo manuale

Fare clic su **Modalità di calcolo** nella scheda **Formule** per scegliere come vengono calcolate le formule della cartella di lavoro:

- **Automatico** calcola le formule man mano che si modificano le celle.
- **Automatico, tabelle manualmente** funziona allo stesso modo e viene salvato nel file per i programmi che gestiscono separatamente le tabelle dati.
- **Manuale** non calcola nulla finché non lo si richiede. Dopo una modifica, la barra di stato mostra **Non aggiornato**. Premere **F9** (**Ricalcola tutto**) per calcolare tutto oppure **MAIUSC+F9** (**Ricalcola questo foglio**) per calcolare le formule del foglio corrente e qualsiasi altro calcolo in sospeso. Una cartella di lavoro impostata su Manuale si apre con i risultati contenuti nel file.

La scelta viene salvata con la cartella di lavoro e può essere annullata come singola operazione.

## Formule che fanno riferimento a se stesse

Una formula che legge il proprio risultato, direttamente o tramite altre celle, non può essere calcolata nel modo consueto. JeffCrunch lo segnala una sola volta, indica le celle coinvolte e le evidenzia con un contorno nel foglio; le celle mantengono i valori che avevano. Fare clic su **Riferimenti circolari** nella scheda **Formule** per visualizzarne l'elenco, quindi fare doppio clic su una cella dell'elenco per raggiungerla.

Alcuni modelli richiedono intenzionalmente un riferimento circolare. Scegliere **Calcolo iterativo delle formule circolari** nel menu **Modalità di calcolo**: le formule vengono calcolate ripetutamente, ognuna leggendo gli ultimi valori delle altre, finché nessun valore cambia più della quantità specificata oppure finché non si raggiunge il numero massimo di iterazioni consentito.

## Vedere cosa legge una formula

Selezionare una cella e fare clic su **Mostra celle che alimentano questa cella** per tracciare frecce dalle celle lette dalla sua formula. Fare di nuovo clic per risalire di un altro livello. **Mostra celle che usano questa cella** traccia frecce verso le formule che leggono la cella, e ogni clic avanza di un livello. Una piccola icona a forma di pagina rappresenta una cella che si trova in un altro foglio. Vengono tracciate al massimo 2.000 frecce e la barra di stato segnala se ce n'erano di più. **Rimuovi frecce** le cancella, così come qualsiasi modifica alla cartella di lavoro.
