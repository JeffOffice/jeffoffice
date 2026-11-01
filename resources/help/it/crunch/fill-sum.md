# Riempire celle e calcolarne la somma
<!-- keywords: riempimento, riempire, serie, trascinare, trascinamento, quadratino, quadratino di riempimento, riempimento automatico, Ctrl+D, Ctrl+R, giù, destra, su, sinistra, numero, data, giorno feriale, mese, anno, incremento, valore limite, copiare, somma automatica, somma, media, conteggio, max, min, Alt+uguale, totale -->

## Riempimento

Il riempimento prosegue ciò che hai iniziato. Seleziona le celle già compilate, quindi trascina il piccolo quadrato nell'angolo della selezione (il **quadratino di riempimento**) lungo una colonna o una riga e rilascia. Un bordo tratteggiato indica fin dove arriverà il riempimento.

| Valori iniziali | Risultato del riempimento |
| --- | --- |
| 1, 2 | 3, 4, 5 e così via: l'incremento tra i valori prosegue |
| Un solo numero, ad esempio 5 | 5, 5, 5: copie (tieni premuto Ctrl durante il trascinamento per ottenere il comportamento opposto) |
| Una data | I giorni successivi (usa il pulsante **Opzioni di riempimento** o **Riempimento > Serie** per giorni feriali, mesi o anni) |
| Lun o gennaio | Mar, Mer, ... oppure febbraio, marzo, ... nella tua lingua |
| Articolo 1 | Articolo 2, Articolo 3, ... |
| Una formula | La formula copiata, con i riferimenti spostati come in un'operazione Incolla |

**Riempimento > Giù** (**Ctrl+D**) e **Riempimento > Destra** (**Ctrl+R**) copiano la prima riga o colonna della selezione sulle altre. **Riempimento > Su** e **Riempimento > Sinistra** partono dall'ultima. **Riempimento > Serie** chiede il tipo (lineare, crescita, data), l'unità di data, il valore di incremento e il valore limite. Ogni cella riempita assume il formato della cella che la precede. Una cella che non può essere modificata (parte di una matrice, una cella bloccata o l'intestazione di una tabella) interrompe il riempimento e non viene modificato nulla.

Dopo aver trascinato il quadratino di riempimento, il piccolo pulsante **Opzioni di riempimento** compare nell'angolo dell'area riempita. Il suo menu ripete il riempimento in un altro modo, e il riempimento resta un unico passaggio di Annulla: **Copia celle**, **Riempi serie**, **Riempi solo formattazione** (le celle mantengono il contenuto e assumono il formato), **Riempi senza formattazione** (le celle mantengono il proprio formato) e, per le date, **Riempi giorni**, **Riempi giorni feriali**, **Riempi mesi** e **Riempi anni**. Il pulsante scompare non appena esegui un'altra operazione.

Il riempimento non entra in celle unite né ne esce, perché lascerebbe valori in celle nascoste dall'area unita. Dividi prima le celle.

**Riempimento > Giustifica** distribuisce il testo delle celle selezionate di una colonna su righe larghe quanto la colonna, nelle celle a partire dalla prima verso il basso; le celle della selezione rimaste inutilizzate vengono svuotate. Se il testo richiede più celle di quelle selezionate, JeffCrunch chiede se usare le celle sottostanti. Funziona solo con il testo.

**Riempimento > Tra fogli di lavoro** copia le celle selezionate nelle stesse celle dei fogli selezionati in un elenco: tutto, solo il contenuto o solo i formati. È un unico passaggio di Annulla.

## Somma automatica

**Home > Somma automatica** (**Alt e il segno di uguale**) inserisce un totale nella cella selezionata. Considera i numeri sopra la cella; se non ce ne sono, quelli alla sua sinistra. La formula viene inserita nella cella perché tu possa controllarla, e **Invio** la conferma. La freccia accanto a Somma automatica offre gli altri totali: **Media**, **Conta numeri**, **Max** e **Min**.

Se selezioni prima un blocco di celle, ogni colonna del blocco riceve la propria formula nelle celle vuote sottostanti (oppure, per una sola riga, alla sua destra).
