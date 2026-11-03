# Come vengono visualizzate e calcolate le formule
<!-- keywords: formula, formule, calcolo, calcolare, ricalcolare, ricalcolo, funzione, funzioni, valore, risultato, memorizzato, F9, circolare, riferimento circolare, mostra formule, barra della formula, errore, indicare, puntare, fare clic su una cella, riferimento, F4, riferimento assoluto -->

Una **formula** è una cella che calcola il proprio valore a partire da altre celle, ad esempio =SOMMA(B3:B8). La cella mostra il valore, mentre la **barra della formula** mostra la formula.

## Come JeffCrunch gestisce le formule

Quando apri una cartella di lavoro, il file contiene l'ultimo valore di ogni formula, quindi il foglio può essere visualizzato subito. Poi JeffCrunch ricalcola tutte le formule, poche alla volta, in modo che la finestra resti reattiva. Durante l'operazione la barra di stato mostra **Calcolo in corso** con una percentuale. Al termine, le celle contengono i risultati calcolati da JeffCrunch. Per ricalcolare tutto, fai clic su **Ricalcola tutto** nella scheda **Formule** oppure premi **F9**.

## Quando JeffCrunch non riesce a calcolare una formula

JeffCrunch dispone di oltre 200 funzioni e il loro numero continua a crescere. Una formula che usa una funzione non ancora disponibile in JeffCrunch mantiene il valore memorizzato nel file. Lo stesso vale per una formula che fa riferimento a un'altra cartella di lavoro. Le celle che dipendono da queste formule vengono calcolate a partire dai valori memorizzati.

## RESTO: il resto della divisione

**RESTO(n; d)** restituisce ciò che resta di **n** dopo averne sottratto il maggior numero possibile di multipli interi di **d**: equivale a n - d * INT(n / d). Esempio: **=RESTO(10; 3)** dà 1, e **=RESTO(-10; 3)** dà 2, perché INT(-10 / 3) è -4 e -10 - 3 * -4 è 2. Il risultato ha lo stesso segno di **d**. Se **d** è 0, il risultato è #DIV/0!.

## Mostrare le formule

Fai clic su **Mostra formule** nella scheda **Formule** oppure premi **Ctrl+`** (su Mac, il tasto Ctrl, non il tasto Comando) per vedere in ogni cella la formula al posto del valore. Le colonne che contengono formule vengono allargate, entro un certo limite, in modo che le formule siano visibili per intero; quando disattivi l'opzione tornano alla larghezza originale, e le larghezze salvate nel file non vengono modificate.

## Modificare le formule

Per modificare una formula, seleziona la cella, premi **F2**, modifica la formula e premi **Invio**. Puoi anche modificarla nella barra della formula. Quando modifichi una cella, le formule che dipendono da essa vengono ricalcolate in background. Vedi [Digitare nelle celle](edit-cells).

## Scrivere una formula

Digita **=** e le prime lettere di una funzione: compare un breve elenco con le funzioni e i [nomi](names) che iniziano in quel modo, ciascuno con una breve descrizione. Premi **Tab** per inserire l'elemento evidenziato. Usa i tasti **Freccia giù** e **Freccia su** per evidenziarne un altro, quindi premi **Tab** o **Invio**. **Esc** chiude l'elenco. Quando il cursore si trova tra le parentesi di una funzione, sotto la cella vengono mostrati i suoi argomenti, con quello che stai digitando in grassetto. L'elenco e gli argomenti sono gli stessi mostrati da **Scegli una funzione**. Gli argomenti sono separati da virgole oppure, nelle lingue che usano la virgola come separatore decimale, come l'italiano, da punti e virgola.

## Indicare le celle

Invece di digitare l'indirizzo di una cella, puoi indicarla. Mentre digiti una formula, posiziona il cursore in un punto in cui può essere inserita una cella (subito dopo **=**, un operatore come **+**, una parentesi aperta o un separatore di argomenti) e fai clic sulla cella. Il suo indirizzo viene inserito automaticamente. Trascina su più celle per inserire un intervallo come B2:D5. Se fai di nuovo clic prima di digitare altro, la nuova cella sostituisce quella appena inserita. Tieni premuto **Ctrl** e fai clic per aggiungere un altro indirizzo dopo un separatore. Fai clic sulla lettera di una colonna per inserire un'intera colonna, ad esempio A:A, oppure sul numero di una riga per inserire un'intera riga, ad esempio 3:3; trascina lungo le lettere o i numeri per ottenere B:D o 2:5.

Funzionano anche i tasti di direzione. Premi una freccia: un puntatore si sposta dalla cella in cui stai digitando e il suo indirizzo viene inserito; premi altre frecce per spostarlo ancora, oppure tieni premuto **Maiusc** per estendere il puntatore su un intervallo. La barra di stato mostra **Indica** quando i tasti di direzione indicano le celle, **Invio** mentre digiti e **Modifica** quando i tasti di direzione spostano il cursore nel testo. Premi **F2** per passare dall'indicazione delle celle alla modifica del testo e viceversa.

Per indicare una cella in un altro foglio, fai clic sulla scheda di quel foglio mentre il cursore si trova in un punto in cui può essere inserita una cella, quindi fai clic sulla cella. Insieme all'indirizzo viene inserito anche il nome del foglio, tra virgolette se necessario, come in 'Il mio foglio'!B3. **Invio** ti riporta al foglio di partenza e inserisce la formula nella sua cella. **Esc** annulla tutto, compreso il cambio di foglio.

Puoi anche trascinare il bordo di un intervallo evidenziato nel foglio per spostare il relativo indirizzo, oppure trascinarne il quadratino d'angolo per estenderlo; la formula si aggiorna durante il trascinamento.

Premi **F4** con il cursore all'interno o subito dopo un indirizzo per bloccarne alcune parti con il simbolo del dollaro: A1 diventa $A$1, poi A$1, poi $A1 e infine di nuovo A1.

Ogni indirizzo della formula è colorato nel testo e le celle a cui fa riferimento sono evidenziate nel foglio con lo stesso colore, così puoi vedere a colpo d'occhio quali celle legge la formula. Un clic dopo un indirizzo completo, ad esempio =A1, conclude l'immissione e seleziona la cella su cui hai fatto clic, come avviene per qualsiasi altro testo.

## Tabelle dati

Una tabella dati (tabella di simulazione) mostra il risultato di una formula per diversi valori di una o due celle. Quando apri una cartella di lavoro che ne contiene una, JeffCrunch ne calcola le celle: in una tabella con due input, la formula si trova nella cella nell'angolo superiore sinistro della tabella, i valori da provare in orizzontale vengono inseriti nella cella di input riga e quelli in verticale nella cella di input colonna. In una tabella con un solo input, i valori sono disposti in verticale nella colonna a sinistra (e le formule si trovano nella riga sopra) oppure in orizzontale nella riga sopra (e le formule si trovano nella colonna a sinistra). Ogni cella della tabella contiene il risultato della formula, e di tutte le formule che dipendono dalla cella di input, con l'input impostato su quel valore; il resto della cartella di lavoro non viene modificato. In modalità **Automatico** le tabelle vengono calcolate insieme alle formule, e le formule che leggono le celle di una tabella vengono aggiornate di conseguenza. Non è ancora possibile creare nuove tabelle dati. Una tabella che non può essere calcolata (perché una formula che dipende dall'input usa una funzione non disponibile in JeffCrunch, oppure cambia da sola, come CASUALE o ADESSO) mantiene i valori memorizzati nel file.

## Calcolo manuale

Fai clic su **Modalità di calcolo** nella scheda **Formule** per scegliere come la cartella di lavoro calcola le formule:

- **Automatico** calcola le formule mentre modifichi le celle.
- **Automatico, tabelle manualmente** funziona allo stesso modo, ma lascia invariate le celle delle tabelle dati finché non premi **F9**.
- **Manuale** non calcola nulla, tabelle dati comprese, finché non lo richiedi. Dopo una modifica, la barra di stato mostra **Non aggiornato**. Premi **F9** (**Ricalcola tutto**) per calcolare tutto, oppure **Maiusc+F9** (**Ricalcola questo foglio**) per calcolare le formule del foglio corrente e tutto ciò che era in attesa. Una cartella di lavoro impostata su Manuale si apre con i risultati memorizzati nel file.

L'impostazione viene salvata con la cartella di lavoro e può essere annullata come un'unica operazione.

## Formule che fanno riferimento a se stesse

Una formula che legge il proprio risultato, direttamente o tramite altre celle, non può essere calcolata nel modo consueto. JeffCrunch lo segnala una sola volta, indica le celle coinvolte e le evidenzia nel foglio; le celle mantengono i valori che avevano. Fai clic su **Riferimenti circolari** nella scheda **Formule** per visualizzarne l'elenco, quindi fai doppio clic su una cella dell'elenco per raggiungerla.

Alcuni modelli richiedono intenzionalmente un riferimento circolare. Scegli **Calcolo iterativo delle formule circolari** nel menu **Modalità di calcolo**: le formule vengono ricalcolate ripetutamente, ciascuna leggendo i valori più recenti delle altre, finché nessun valore cambia più della quantità specificata oppure finché non viene raggiunto il numero massimo di iterazioni consentito.

## Vedere quali celle legge una formula

Seleziona una cella e fai clic su **Mostra celle che alimentano questa cella** per disegnare frecce dalle celle lette dalla sua formula. Fai di nuovo clic per risalire di un altro livello. **Mostra celle che usano questa cella** disegna frecce verso le formule che leggono la cella, e ogni clic avanza di un livello. Una piccola icona a forma di pagina rappresenta una cella in un altro foglio. Vengono disegnate al massimo 2.000 frecce; se ce ne sono di più, la barra di stato lo segnala. **Rimuovi frecce** cancella le frecce, così come qualsiasi modifica alla cartella di lavoro.
