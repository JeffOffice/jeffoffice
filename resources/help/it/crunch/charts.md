# Grafici, immagini e forme
<!-- keywords: grafico, diagramma, istogramma, colonne, barre, linee, torta, area, dispersione, combinato, legenda, titolo, griglia, etichette dati, dati grafico, tipo di grafico, aspetto, stili, layout, spostare, ridimensionare, immagine, forma, oggetto, serie -->

Un grafico è un'immagine dei numeri che resta collegata a essi: se modifichi un numero, il grafico cambia di conseguenza. Anche le immagini e le forme si trovano sopra le celle, ma non sono collegate ad alcun numero.

## Creare un grafico

1. Seleziona i numeri da rappresentare nel grafico. Se la prima riga contiene nomi e la prima colonna contiene etichette, seleziona anche queste. JeffCrunch esamina la prima riga e la prima colonna e le usa come nomi ed etichette quando contengono parole (o date, nel caso delle etichette).
2. Nella scheda **Inserisci**, nel gruppo **Grafici**, scegli un tipo: **Istogramma**, **Barre**, **Linee**, **Torta**, **Area**, **Dispersione** o **Combinato**. La piccola freccia accanto a ogni tipo contiene le relative varianti, come le colonne in pila, le linee con indicatori o un grafico a dispersione con punti, con punti collegati o con linee smussate.
3. Per vedere prima un'anteprima di tutti i tipi, scegli **Altri grafici**. L'elenco a sinistra mostra ogni tipo disegnato a partire dalle tue celle. **Scambia righe e colonne** inverte l'orientamento delle serie.

Se selezioni una sola cella, JeffCrunch crea il grafico del blocco di celle con contenuto che la circonda.

Le serie seguono le colonne quando la selezione ha almeno tante righe quante colonne, e seguono le righe quando ha più colonne che righe. Un grafico a torta contiene una sola serie. Un grafico a dispersione prende i valori orizzontali dalla prima colonna.

Esempio: selezioni A1:C4, dove A2:A4 contengono T1, T2 e T3, B1 e C1 contengono Est e Ovest e le altre celle contengono numeri. Il grafico ha due serie, Est e Ovest, e tre gruppi di colonne, da T1 a T3.

## Spostarlo e ridimensionarlo

Fai clic su un grafico, un'immagine o una forma per selezionarlo. Trascinalo per spostarlo oppure trascina uno degli otto quadratini sul bordo per ridimensionarlo.

- Tieni premuto **Alt** durante il trascinamento per allineare un bordo al bordo di una cella.
- Tieni premuto **Maiusc** mentre trascini un angolo per mantenere le proporzioni.
- I tasti di direzione lo spostano di alcuni pixel alla volta. **Ctrl** con un tasto di direzione lo sposta di un pixel, **Maiusc** con un tasto di direzione lo ingrandisce o lo riduce e **Alt** con un tasto di direzione lo porta al bordo successivo di una cella.
- Premendo **Esc** durante il trascinamento l'oggetto torna nella posizione iniziale.

Ogni spostamento o ridimensionamento è un'operazione che puoi annullare. Fai clic con il pulsante destro del mouse sull'oggetto e scegli **Proprietà** per digitarne le dimensioni e per scegliere il suo comportamento quando inserisci, elimini o ridimensioni celle: spostarsi e ridimensionarsi con le celle, spostarsi con le celle mantenendo le dimensioni, oppure non spostarsi. La scelta viene salvata nel file.

## Modificare il contenuto di un grafico

Quando un grafico è selezionato, accanto a esso compaiono due pulsanti e sulla barra multifunzione compare la scheda **Grafico**.

- Il pulsante **+** (e **Elementi grafico** nella scheda) attiva o disattiva titolo, assi, titoli degli assi, griglia, legenda ed etichette dati del grafico, e stabilisce dove posizionare la legenda e le etichette.
- Il pulsante con il pennello (e **Colori** e **Aspetto** nella scheda) applica al grafico uno dei set di colori e uno degli stili di JeffCrunch. **Layout** imposta più elementi contemporaneamente.
- **Dati grafico** elenca le serie del grafico. Puoi aggiungerle, modificarle, rimuoverle e riordinarle, cambiare le celle lette da ciascuna, impostare le celle con le etichette, scambiare righe e colonne e scegliere come rappresentare le celle vuote e le righe e colonne nascoste.
- **Tipo di grafico** cambia il tipo per l'intero grafico o per ciascuna serie: colonne con una linea sovrapposta oppure una serie su un asse verticale secondario.

Un riferimento a celle si scrive come **Foglio1!$B$2:$B$9**, con il nome del foglio tra virgolette singole se contiene spazi.

## Tagliare, copiare e incollare un oggetto

Un'immagine o una forma si copia e si incolla come un grafico. Una forma che usa qualcosa di diverso da un'immagine della cartella di lavoro, ad esempio un collegamento a un file, non può essere copiata e JeffCrunch ne indica il motivo.


Se copi un grafico e lo incolli nella stessa cartella di lavoro, in qualsiasi foglio, la copia legge le stesse celle dell'originale. Se lo incolli in un'altra cartella di lavoro, mantiene i numeri che mostrava ma non legge più le celle, perché le celle che leggerebbe in quella cartella non sono quelle da cui è stato creato.

## Assegnargli un nome

Quando un oggetto è selezionato, la casella Nome ne mostra il nome. Digita un nuovo nome nella casella e premi **Invio**. Annulla ripristina il nome precedente e il nuovo nome viene salvato nel file.

## Eliminarlo

Premi **Canc** con l'oggetto selezionato. Annulla lo ripristina.

## Cosa viene salvato

Tutto quanto descritto sopra viene salvato nel file nel formato letto dagli altri programmi per fogli di calcolo, quindi un grafico creato qui si apre allo stesso modo anche in quei programmi. Un grafico proveniente da un altro programma con elementi che JeffCrunch non può conservare quando riscrive un grafico (un riempimento sfumato, una linea di tendenza, un grafico tridimensionale) può comunque essere spostato, ridimensionato ed eliminato, ma i suoi elementi e i suoi dati non vengono modificati qui.
