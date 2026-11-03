# Riempire le celle e calcolarne la somma
<!-- keywords: riempimento, riempire, serie, trascinare, quadratino di riempimento, riempimento automatico, Ctrl+D, Ctrl+R, giù, destra, su, sinistra, numero, data, giorno feriale, mese, anno, incremento, valore limite, copiare, Somma, somma, totale, media, conteggio, massimo, minimo, Alt+uguale -->

## Riempimento

Il riempimento continua ciò che hai iniziato. Seleziona le celle che hai compilato, quindi trascina il quadratino nell'angolo della selezione (il **quadratino di riempimento**) lungo una colonna o una riga e rilascia il pulsante del mouse. Un bordo tratteggiato mostra fin dove arriverà il riempimento.

| Valori iniziali | Risultato del riempimento |
| --- | --- |
| 1, 2 | 3, 4, 5 e così via: l'incremento tra i valori continua |
| Un singolo numero, ad esempio 5 | 5, 5, 5: copie (tieni premuto Ctrl durante il trascinamento per ottenere l'effetto opposto) |
| Una data | I giorni successivi (usa il pulsante **Opzioni di riempimento** o **Riempimento > Serie** per giorni feriali, mesi o anni) |
| Lun o gennaio | Mar, Mer, ... oppure febbraio, marzo, ... nella tua lingua |
| Elemento 1 | Elemento 2, Elemento 3, ... |
| Una formula | La formula copiata, con i riferimenti spostati come quando la incolli |

**Riempimento > Giù** (**Ctrl+D**) e **Riempimento > Destra** (**Ctrl+R**) copiano la prima riga o colonna della selezione nelle altre. **Riempimento > Su** e **Riempimento > Sinistra** partono dall'ultima. **Riempimento > Serie** chiede il tipo (lineare, crescita, data), l'unità di data, il valore di incremento e il valore limite. Ogni cella riempita assume il formato della cella che la precede. Una cella che non può essere modificata (parte di una matrice, una cella bloccata o l'intestazione di una tabella) interrompe il riempimento e non viene modificato nulla.

Dopo aver trascinato il quadratino di riempimento, nell'angolo dell'area riempita compare il piccolo pulsante **Opzioni di riempimento**. Il suo menu ripete il riempimento in un altro modo, e il riempimento resta un unico passaggio di Annulla: **Copia celle**, **Riempi serie**, **Riempi solo formattazione** (le celle mantengono il contenuto e assumono il formato), **Riempi senza formattazione** (le celle mantengono il proprio formato) e, per le date, **Riempi giorni**, **Riempi giorni feriali**, **Riempi mesi** e **Riempi anni**. Il pulsante scompare non appena esegui un'altra operazione.

Il riempimento non entra né esce da celle unite, perché lascerebbe valori in celle nascoste dall'area unita. Dividi prima le celle.

**Riempimento > Giustifica** distribuisce il testo delle celle selezionate di una colonna in righe larghe quanto la colonna, nelle celle a partire dalla prima verso il basso; le celle della selezione che avanzano vengono svuotate. Se il testo richiede più celle di quelle selezionate, JeffCrunch chiede se usare le celle sottostanti. Funziona solo con il testo.

**Riempimento > Tra fogli di lavoro** copia le celle selezionate nelle stesse celle dei fogli selezionati in un elenco: tutto, solo il contenuto o solo i formati. È un unico passaggio di Annulla.

## Somma

**Home > Somma** (**Alt e il segno di uguale**) inserisce un totale nella cella selezionata. Prende in considerazione i numeri sopra la cella oppure, se non ce ne sono, quelli alla sua sinistra. La formula viene inserita nella cella perché tu possa controllarla, e premendo **Invio** la confermi. La freccia accanto a Somma offre gli altri totali: **Media**, **Conta numeri**, **Max** e **Min**.

Se selezioni prima un blocco, ogni colonna del blocco riceve una propria formula nelle celle vuote sottostanti (o, per una singola riga, alla sua destra).
