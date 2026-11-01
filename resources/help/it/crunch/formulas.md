# Come vengono visualizzate e calcolate le formule
<!-- keywords: formula, formule, calcolo, calcolare, ricalcolare, ricalcolo, funzione, funzioni, valore, risultato, valore memorizzato, F9, riferimento circolare, circolare, mostra formule, barra della formula, errore -->

Una **formula** è una cella che calcola il proprio valore a partire da altre celle, ad esempio =SOMMA(B3:B8). La cella mostra il valore, mentre la **barra della formula** mostra la formula.

## Cosa fa JeffCrunch con le formule

Quando apri una cartella di lavoro, il file contiene l'ultimo valore di ogni formula, quindi il foglio può essere visualizzato subito. Poi JeffCrunch ricalcola tutte le formule, poche alla volta, in modo che la finestra resti reattiva. Durante il calcolo, la barra di stato mostra **Calcolo in corso** con una percentuale. Al termine, le celle contengono i risultati calcolati da JeffCrunch. Per ricalcolare tutto, fai clic su **Ricalcola tutto** nella scheda **Formule** oppure premi **F9**.

## Quando JeffCrunch non riesce a calcolare una formula

JeffCrunch dispone di oltre 200 funzioni e il loro numero continua a crescere. Una formula che usa una funzione non ancora disponibile in JeffCrunch mantiene il valore memorizzato nel file. Lo stesso vale per una formula che fa riferimento a un'altra cartella di lavoro. Le celle che dipendono da queste formule vengono calcolate a partire dai valori memorizzati.

## Mostrare le formule

Fai clic su **Mostra formule** nella scheda **Formule** oppure premi **CTRL+`** (su Mac, il tasto Control, non il tasto Comando) per vedere in ogni cella la formula al posto del valore. Le colonne che contengono formule vengono allargate, fino a un certo limite, in modo che le formule siano visibili per intero; quando disattivi l'opzione, le colonne tornano alla larghezza precedente e le larghezze salvate nel file non vengono modificate.

## Modificare le formule

Per modificare una formula, seleziona la cella, premi **F2**, modifica la formula e premi **INVIO**. Puoi anche modificarla nella barra della formula. Quando modifichi una cella, le formule che dipendono da essa vengono ricalcolate in background. Vedi [Digitare nelle celle](edit-cells).

## Scrivere una formula

Digita **=** e le prime lettere di una funzione: viene visualizzato un breve elenco delle funzioni e dei [nomi](names) che iniziano con quelle lettere, ciascuno con una breve descrizione. Premi **TAB** per inserire l'elemento evidenziato. Usa i tasti **FRECCIA GIÙ** e **FRECCIA SU** per evidenziarne un altro, quindi premi **TAB** o **INVIO**. **ESC** chiude l'elenco. Quando ti trovi tra le parentesi di una funzione, sotto la cella vengono visualizzati i relativi argomenti, con quello che stai digitando in grassetto. L'elenco e gli argomenti sono gli stessi mostrati da **Scegli una funzione**. Gli argomenti sono separati da virgole oppure da punti e virgola nelle lingue che usano la virgola come separatore decimale.

## Tabelle dati

Una tabella dati (tabella di simulazione) mostra il risultato di una formula per molti valori di una o due celle. Quando apri una cartella di lavoro che ne contiene una, JeffCrunch ne calcola le celle: in una tabella a due input, la formula si trova nella cella dell'angolo superiore sinistro della tabella, i valori da provare disposti in orizzontale vengono inseriti nella cella di input della riga e quelli disposti in verticale nella cella di input della colonna. In una tabella a un input, i valori sono disposti in verticale nella colonna a sinistra (e le formule si trovano nella riga sopra di essa) oppure in orizzontale nella riga in alto (e le formule si trovano nella colonna a sinistra). Ogni cella della tabella contiene il risultato della formula, e di tutte le formule che dipendono dalla cella di input, con l'input impostato su quel valore; il resto della cartella di lavoro non viene modificato. In modalità **Automatico** le tabelle vengono calcolate insieme alle formule, seguite dalle formule che leggono le celle delle tabelle. Non è ancora possibile creare una nuova tabella dati. Una tabella che non può essere calcolata (perché una formula che dipende dall'input usa una funzione non disponibile in JeffCrunch o cambia da sola, come CASUALE o ADESSO) mantiene i valori contenuti nel file.

## Calcolo manuale

Fai clic su **Modalità di calcolo** nella scheda **Formule** per scegliere come vengono calcolate le formule della cartella di lavoro:

- **Automatico** le calcola man mano che modifichi le celle.
- **Automatico, tabelle manualmente** fa lo stesso, ma lascia invariate le celle delle tabelle dati finché non premi **F9**.
- **Manuale** non calcola nulla, tabelle dati comprese, finché non lo richiedi. Dopo una modifica, la barra di stato mostra **Non aggiornato**. Premi **F9** (**Ricalcola tutto**) per ricalcolare tutto oppure **MAIUSC+F9** (**Ricalcola questo foglio**) per calcolare le formule del foglio corrente e tutto ciò che era in attesa. Una cartella di lavoro impostata su Manuale si apre con i risultati contenuti nel file.

L'impostazione viene salvata con la cartella di lavoro e può essere annullata in un unico passaggio.

## Formule che fanno riferimento a se stesse

Una formula che legge il proprio risultato, direttamente o tramite altre celle, non può essere calcolata nel modo consueto. JeffCrunch lo segnala una volta, indica le celle interessate e le evidenzia con un contorno nel foglio; le celle mantengono i valori che avevano. Fai clic su **Riferimenti circolari** nella scheda **Formule** per visualizzarne l'elenco e fai doppio clic su una cella dell'elenco per spostarti su di essa.

Alcuni modelli richiedono intenzionalmente un riferimento circolare. Scegli **Calcolo iterativo delle formule circolari** nel menu **Modalità di calcolo**: le formule vengono ricalcolate ripetutamente, ognuna leggendo i valori più recenti delle altre, finché nessun valore cambia più della quantità specificata o non viene raggiunto il numero massimo di iterazioni consentito.

## Vedere cosa legge una formula

Seleziona una cella e fai clic su **Mostra celle che alimentano questa cella** per tracciare frecce a partire dalle celle lette dalla sua formula. Fai di nuovo clic per risalire di un altro livello. **Mostra celle che usano questa cella** traccia frecce verso le formule che leggono la cella e ogni clic avanza di un livello. Una piccola icona a forma di pagina rappresenta una cella di un altro foglio. Vengono tracciate al massimo 2.000 frecce e, se ce ne sono di più, la barra di stato lo segnala. **Rimuovi frecce** le cancella, così come qualsiasi modifica alla cartella di lavoro.
