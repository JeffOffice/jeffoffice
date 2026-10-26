# Grafici, immagini e forme
<!-- keywords: grafico, diagramma, istogramma, colonne, barre, linee, torta, aree, dispersione, combinato, legenda, titolo, griglia, etichette dati, dati grafico, tipo di grafico, aspetto, layout, spostare, ridimensionare, immagine, forma, oggetto, serie -->

Un grafico è una rappresentazione visiva di numeri che rimane collegata a essi: se si modifica un numero, il grafico cambia di conseguenza. Anche le immagini e le forme si trovano sopra le celle, ma non sono collegate ad alcun numero.

## Creare un grafico

1. Selezionare i numeri da rappresentare. Se la prima riga contiene nomi e la prima colonna contiene etichette, selezionare anche queste. JeffCrunch esamina la prima riga e la prima colonna e le usa come nomi ed etichette quando contengono parole (o date, per le etichette).
2. Nella scheda **Inserisci**, nel gruppo **Grafici**, scegliere un tipo: **Istogramma**, **Barre**, **Linee**, **Torta**, **Area**, **Dispersione** o **Combinato**. La piccola freccia accanto a ogni tipo contiene le varianti, ad esempio colonne in pila o linee con indicatori.
3. Per vedere prima un'anteprima di tutti i tipi, scegliere **Altri grafici**. L'elenco a sinistra mostra ogni tipo disegnato a partire dalle celle selezionate. **Scambia righe e colonne** inverte l'orientamento delle serie.

Se si seleziona una sola cella, JeffCrunch crea il grafico con il blocco di celle con contenuto che la circonda.

Le serie seguono le colonne quando la selezione ha almeno tante righe quante colonne, e seguono le righe quando ha più colonne che righe. Un grafico a torta contiene una sola serie. Un grafico a dispersione ricava i valori orizzontali dalla prima colonna.

Esempio: si seleziona A1:C4, dove A2:A4 contengono T1, T2 e T3, B1 e C1 contengono Est e Ovest, e le altre celle contengono numeri. Il grafico ha due serie, Est e Ovest, e tre gruppi di colonne, da T1 a T3.

## Spostare e ridimensionare

Fare clic su un grafico, un'immagine o una forma per selezionarlo. Trascinarlo per spostarlo, oppure trascinare uno degli otto quadratini sul bordo per ridimensionarlo.

- Tenere premuto **ALT** durante il trascinamento per allineare un bordo al bordo di una cella.
- Tenere premuto **MAIUSC** mentre si trascina un angolo per mantenere le proporzioni.
- I tasti di direzione spostano l'oggetto di alcuni pixel alla volta. **CTRL** con un tasto di direzione lo sposta di un pixel, **MAIUSC** con un tasto di direzione lo ingrandisce o lo riduce e **ALT** con un tasto di direzione lo porta al bordo di cella successivo.
- Premere **ESC** durante il trascinamento per riportarlo nella posizione iniziale.

Ogni spostamento o ridimensionamento è un'operazione che si può annullare. Fare clic con il pulsante destro del mouse sull'oggetto e scegliere **Proprietà** per digitarne le dimensioni e per scegliere il comportamento quando si inseriscono, eliminano o ridimensionano celle: spostarsi e ridimensionarsi con le celle, spostarsi con le celle mantenendo le dimensioni, oppure non spostarsi. La scelta viene salvata nel file.

## Modificare il contenuto di un grafico

Quando si seleziona un grafico, accanto a esso compaiono due pulsanti e sulla barra multifunzione compare la scheda **Grafico**.

- Il pulsante **+** (e **Elementi grafico** nella scheda) attiva o disattiva il titolo, gli assi, i titoli degli assi, la griglia, la legenda e le etichette dati del grafico, e permette di scegliere la posizione della legenda e delle etichette.
- Il pulsante con il pennello (e **Colori** e **Aspetto** nella scheda) applica al grafico uno dei set di colori e uno degli stili di JeffCrunch. **Layout** imposta più elementi contemporaneamente.
- **Dati grafico** elenca le serie del grafico. È possibile aggiungerle, modificarle, rimuoverle e riordinarle, cambiare le celle lette da ciascuna, impostare le celle con le etichette, scambiare righe e colonne e scegliere come rappresentare le celle vuote e le righe e colonne nascoste.
- **Tipo di grafico** cambia il tipo per l'intero grafico o per ciascuna serie: ad esempio colonne con una linea sovrapposta, oppure una serie su un asse verticale secondario.

Un riferimento a celle si scrive come **Foglio1!$B$2:$B$9**, con il nome del foglio tra virgolette singole se contiene spazi.

## Tagliare, copiare e incollare un oggetto

Un'immagine o una forma si copia e si incolla come un grafico. Una forma che usa qualcosa di diverso da un'immagine della cartella di lavoro, ad esempio un collegamento a un file, non può essere copiata, e JeffCrunch ne indica il motivo.


Se si copia un grafico e lo si incolla nella stessa cartella di lavoro, in qualsiasi foglio, la copia legge le stesse celle dell'originale. Se lo si incolla in un'altra cartella di lavoro, mantiene i numeri che mostrava ma non legge più le celle, perché le celle che leggerebbe lì non sono quelle da cui è stato creato.

## Assegnare un nome

Con un oggetto selezionato, la Casella Nome ne mostra il nome. Digitare un nuovo nome e premere **INVIO**. Annulla ripristina il nome precedente e il nuovo nome viene salvato nel file.

## Eliminare

Premere **CANC** con l'oggetto selezionato. Annulla lo ripristina.

## Cosa viene salvato

Tutto quanto descritto sopra viene salvato nel file nel formato letto dagli altri programmi per fogli di calcolo, quindi un grafico creato qui si apre allo stesso modo anche in quei programmi. Un grafico proveniente da un altro programma che contiene elementi che JeffCrunch non può mantenere quando riscrive un grafico (un riempimento sfumato, una linea di tendenza, un grafico tridimensionale) può comunque essere spostato, ridimensionato ed eliminato, ma i suoi elementi e i suoi dati non vengono modificati qui.
