# Grafici, sparkline, immagini e forme

<!-- keywords: grafico, diagramma, colonne, barre, linee, torta, area, dispersione, combinato, legenda, titolo, linee della griglia, etichette dati, dati del grafico, tipo di grafico, aspetto, layout, sposta, ridimensiona, immagine, forma, oggetto, serie, linea di tendenza, tendenza, equazione, R al quadrato, secondo asse, sparkline, sparkline, asse delle date, vittoria/sconfitta, grafico minimo, indicatori, nome, testo alternativo, scala logaritmica, salva come immagine, modello, crtx, testo del titolo, chiave di legenda -->

Un grafico è un'immagine di numeri che resta collegata a essi: se cambi un numero, il grafico cambia di conseguenza. Immagini e forme si collocano sopra le celle allo stesso modo, ma non sono collegate ad alcun numero.

## Creare un grafico

1. Seleziona i numeri da rappresentare. Se la prima riga contiene nomi e la prima colonna contiene etichette, seleziona anche quelle. JeffCrunch esamina la prima riga e la prima colonna e le usa come nomi e come etichette quando sono parole (o date, per le etichette).
2. Nella scheda **Inserisci**, nel gruppo **Grafici**, scegli un tipo: **Colonne**, **Barre**, **Linee**, **Torta**, **Area**, **Dispersione** o **Combinato**. La piccola freccia accanto a un tipo mostra le sue varianti, come le colonne in pila, una linea con indicatori oppure un grafico a dispersione di punti, di punti collegati o di linee smussate.
3. Per vedere prima ogni tipo, scegli **Altri grafici**. L'elenco a sinistra mostra ciascun tipo disegnato con i tuoi dati. **Scambia righe e colonne** inverte il modo in cui le serie sono disposte.

Se selezioni una sola cella, JeffCrunch crea il grafico con il blocco di celle attorno ad essa che contiene dei dati.

Le serie sono disposte lungo le colonne quando la selezione ha almeno tante righe quante colonne, e lungo le righe quando ha più colonne che righe. Un grafico a torta contiene una sola serie. Un grafico a dispersione prende i valori orizzontali dalla prima colonna.

Esempio: selezioni A1:C4, dove A2:A4 contengono Q1, Q2 e Q3, B1 e C1 contengono Est e Ovest, e il resto sono numeri. Il grafico ha due serie, Est e Ovest, e tre gruppi di colonne, da Q1 a Q3.

## Spostare e ridimensionare

Fai clic su un grafico, un'immagine o una forma per selezionarlo. Trascinalo per spostarlo, oppure trascina uno degli otto quadratini sul bordo per ridimensionarlo.

- Tieni premuto **Alt** mentre trascini per allineare un bordo al bordo di una cella.
- Tieni premuto **Maiusc** mentre trascini un angolo per mantenere le proporzioni.
- I tasti freccia spostano l'oggetto di alcuni pixel alla volta. **Ctrl** con una freccia lo sposta di un pixel, **Maiusc** con una freccia lo ingrandisce o lo riduce, e **Alt** con una freccia lo porta al bordo successivo di una cella.
- **Esc** mentre trascini lo riporta alla posizione iniziale.

Ogni spostamento o ridimensionamento è un passaggio che puoi annullare. Fai clic con il pulsante destro sull'oggetto e scegli **Proprietà** per digitare il **Nome** (ogni oggetto di un foglio deve avere il proprio) e il **Testo alternativo** (una breve frase che descrive a chi non può vedere l'oggetto ciò che mostra), per digitarne le dimensioni e per scegliere cosa fa quando inserisci, elimini o ridimensioni celle: spostarsi e ridimensionarsi con le celle, spostarsi con esse ma mantenere le proprie dimensioni, oppure restare dove si trova. La scelta viene salvata nel file. **Stampa questo oggetto**, nella stessa finestra, disattiva l'oggetto nelle pagine stampate (resta comunque visibile sullo schermo); anche il file conserva questa scelta.

## Modificare ciò che mostra un grafico

Quando un grafico è selezionato, accanto ad esso compaiono due pulsanti e nella barra multifunzione appare la scheda **Grafico**.

- Il pulsante **+** (e **Elementi grafico** nella scheda) attiva o disattiva titolo, assi, titoli degli assi, linee della griglia, legenda ed etichette dati del grafico, e indica dove vanno la legenda e le etichette.
- Il pulsante a pennello (e **Colori** e **Aspetto** nella scheda) assegna al grafico uno dei set di colori e uno degli aspetti di JeffCrunch. **Layout** imposta più elementi contemporaneamente.
- **Dati grafico** elenca le serie del grafico. Puoi aggiungerle, modificarle, rimuoverle e riordinarle, cambiare le celle che ciascuna legge, impostare le celle con le etichette, scambiare righe e colonne e scegliere come rappresentare celle vuote e righe e colonne nascoste.
- **Linea di tendenza** (nel menu **+**) traccia una linea adattata ai numeri di ciascuna serie, così puoi vedere dove stanno andando. Scegli **Lineare** per una retta, **Esponenziale** per numeri che crescono di una percentuale costante, **Logaritmica** per numeri che salgono rapidamente e poi si stabilizzano, **Polinomiale** per una curva con delle deformazioni, **Potenza**, oppure **Media mobile**, che traccia la media degli ultimi numeri. **Mostra equazione** scrive sul grafico la formula della linea, e **Mostra R al quadrato** scrive quanto bene la linea si adatta (1 indica un adattamento perfetto; 0 nessun adattamento). La linea di tendenza ottiene una voce propria nella legenda.
- Quando una serie è rappresentata su un secondo asse verticale, i menu **Assi** e **Titoli degli assi** hanno le voci **Secondo orizzontale** e **Secondo verticale** per essa. Il menu **Legenda** ha **Sopra il grafico**, che colloca la legenda sull'area del grafico anziché accanto ad essa.
- **Etichette dati** ha anche **Contiene etichetta** (il valore, il nome della categoria, il nome della serie, la percentuale e la chiave di legenda, cioè un piccolo quadrato del colore della serie, in qualsiasi combinazione), **Separatore** (ciò che si trova tra questi elementi: virgola, punto e virgola, punto, a capo o spazio) e **Formato numero…** (un codice di formato come 0,0 per i numeri nelle etichette; lascialo vuoto per mostrare i numeri così come sono). Disattivando tutti i contenuti, le etichette scompaiono.
- **Formato asse orizzontale…** e **Formato asse verticale…** nella scheda **Grafico** aprono una finestra per un singolo asse. Imposta il valore minimo e massimo e le unità principale e secondaria (ciascuna automatica o un numero che digiti), se i valori vanno in ordine inverso, se la scala è logaritmica (ogni segno è una base moltiplicata per il precedente, come 1, 10, 100 e 1000, adatta a numeri di grandezza molto diversa e che devono essere tutti maggiori di 0), dove l'altro asse interseca questo, il formato numerico delle sue etichette, i suoi segni di graduazione, la posizione delle etichette e di quanto sono ruotate. Solo un asse numerico ha limiti e unità.
- **Sposta grafico…** nella scheda **Grafico** chiede un altro foglio di lavoro e vi colloca il grafico, nelle stesse celle. Continua a leggere le celle che leggeva, mantiene le proprie dimensioni e la propria posizione sul foglio, anche se le righe e le colonne sono più alte o più larghe, e si sposta e ridimensiona con le celle come prima. I fogli nascosti non vengono proposti. È un solo passaggio di **Annulla**, anche dopo il salvataggio. Non è ancora possibile creare un foglio grafico (un foglio composto solo da un grafico), e un grafico che si trova su un foglio grafico non può essere spostato.
- **Titolo grafico** e **Titoli degli assi** (nel menu **+**) hanno anche **Testo del titolo…**. Digita il testo desiderato. Oppure digita un segno di uguale e una cella, come **=Foglio1!A1**, e il titolo mostrerà ciò che contiene quella cella. Quando la cella cambia, cambia anche il titolo. Il file conserva il collegamento.
- **Salva come immagine…** nella scheda **Grafico** (e nel menu del clic destro) salva il grafico selezionato come file immagine **PNG** o **JPEG**, con dimensioni doppie rispetto a quelle sul foglio, così resta nitido.
- **Salva come modello…** e **Applica modello…** (nella scheda **Grafico**, nel gruppo **Tipo**) conservano l'aspetto di un grafico in un file **.crtx** e lo applicano a un altro grafico. L'aspetto comprende il tipo, i colori, le impostazioni di titolo e legenda, gli assi e i caratteri. Il grafico a cui lo applichi mantiene i propri numeri e le celle che legge. Un solo **Annulla** rimuove di nuovo l'aspetto.
- **Tipo di grafico** cambia il tipo per l'intero grafico, oppure per ciascuna serie: colonne con una linea sopra, oppure una serie su un secondo asse verticale.

Un riferimento a celle si scrive come **Foglio1!$B$2:$B$9**, con il nome del foglio tra apici singoli quando contiene spazi.

## Sparkline

Una sparkline è un piccolo grafico che sta dentro una sola cella. Mostra l'andamento di una riga o di una colonna di numeri proprio accanto ad essi.

1. Seleziona i numeri. Per avere una sparkline per ogni riga, seleziona più righe di numeri.
2. Nella scheda **Inserisci**, nel gruppo **Sparkline**, scegli **Linea**, **Colonna** o **Vittoria/Sconfitta**. **Linea** collega i numeri con una linea. **Colonna** disegna una barra per ogni numero. **Vittoria/Sconfitta** disegna una barra verso l'alto per ogni numero maggiore di zero e una barra verso il basso per ogni numero minore di zero, tutte della stessa dimensione, il che è adatto a risultati che sono solo positivi o negativi.
3. Nella finestra **Crea sparkline**, controlla l'**Intervallo dati** (le celle che le sparkline leggono) e digita l'**Intervallo di posizione** (le celle in cui vanno). Scegli una cella di posizione per ogni riga di dati, oppure per ogni colonna di dati. Se scegli una sola cella, la sparkline legge tutti i dati.

Le sparkline create in un solo passaggio formano un gruppo e condividono lo stesso aspetto. Fai clic su una cella con una sparkline e compare la scheda **Sparkline**.

- **Modifica dati** cambia i dati e la posizione dell'intero gruppo, oppure i dati della sola sparkline su cui hai fatto clic. Indica anche cosa fare con le celle vuote (lasciare uno spazio, trattarle come zero oppure unire i punti attraverso di esse) e se leggere le celle nelle righe e colonne nascoste.
- **Tipo** passa da linea a colonna e vittoria/sconfitta.
- **Mostra** contrassegna i punti: **Punto massimo**, **Punto minimo**, **Punti negativi**, **Primo punto**, **Ultimo punto* e, per una linea, **Indicatori** su ogni punto.
- **Stili** assegna al gruppo un set di colori. **Colore sparkline** imposta il colore della linea o delle barre e lo **Spessore** (quanto è spessa la linea, in punti). **Colore indicatore** imposta il colore di ciascun tipo di punto contrassegnato.
- **Asse** mostra una linea dello zero, traccia i numeri da destra a sinistra e imposta il valore minimo e massimo dell'asse verticale. **Automatico per ogni sparkline** adatta ciascuna alla propria serie di numeri. **Uguale per tutte le sparkline** assegna all'intero gruppo una sola scala, così da poterle confrontare. **Valore personalizzato** ti permette di digitare un numero. **Asse delle date…** distanzia i punti in base alle date: digita le celle che contengono la data di ciascun punto, e un punto con una data successiva si trova più avanti. Lascialo vuoto per distribuire di nuovo i punti in modo uniforme.
- **Raggruppa** riunisce le sparkline selezionate in un solo gruppo. **Separa** assegna a ciascuna un aspetto proprio. **Cancella** elimina le sparkline selezionate oppure i loro interi gruppi.

Una sparkline segue le proprie celle: se inserisci una riga sopra i dati, continua a leggere gli stessi numeri. Ogni modifica è un passaggio che puoi annullare. Le sparkline sono salvate nel file in un formato che gli altri programmi per fogli di calcolo sanno leggere.

## Tagliare, copiare e incollare un oggetto

Un'immagine o una forma si copiano e si incollano allo stesso modo di un grafico. Una forma che usa qualcosa di diverso da un'immagine della cartella di lavoro, come un collegamento a un file, non può essere copiata, e JeffCrunch ti dice il motivo.

Copia un grafico e incollalo nella stessa cartella di lavoro, su un foglio qualsiasi: la copia legge le stesse celle della prima. Incollalo in un'altra cartella di lavoro e conserverà i numeri che mostrava, ma non leggerà più celle, perché le celle che leggerebbe lì non sono quelle da cui è stato creato.

## Dare un nome

Con un oggetto selezionato, la casella Nome mostra il suo nome. Digita un nuovo nome e premi **Invio**. **Annulla** ripristina il nome precedente, e il nuovo nome viene salvato nel file.

## Eliminare un oggetto

Premi **Canc** con l'oggetto selezionato. **Annulla** lo riporta indietro.

## Che cosa viene salvato

Tutto quanto descritto sopra viene salvato nel file in un formato che gli altri programmi per fogli di calcolo sanno leggere, quindi un grafico creato qui si apre allo stesso modo anche là. Un grafico proveniente da un altro programma che contiene elementi che JeffCrunch non può conservare quando riscrive un grafico (barre di errore, un grafico tridimensionale) può comunque essere spostato, ridimensionato ed eliminato, ma i suoi elementi e i suoi dati non vengono modificati qui.
