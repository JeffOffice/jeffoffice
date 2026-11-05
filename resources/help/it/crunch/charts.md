# Grafici, sparkline, immagini e forme

<!-- keywords: filtri grafico, nascondi serie, grafici consigliati, suggerimenti grafico, grafico, diagramma, colonne, barre, linea, torta, area, dispersione, combinato, legenda, titolo, linee della griglia, etichette dati, dati grafico, tipo di grafico, aspetto, layout, sposta, ridimensiona, immagine, forma, oggetto, serie, linea di tendenza, tendenza, equazione, R al quadrato, secondo asse, sparkline, asse delle date, vittoria/sconfitta, mini grafico, indicatori, nome, testo alternativo, scala logaritmica, salva come immagine, modello, crtx, testo del titolo, chiave legenda, riquadro di selezione, riempimento forma, contorno forma, ruota, capovolgi, ritaglia, modifica testo, bordo immagine, inserisci immagine, inserisci forma, casella di testo, forme, freccia, ovale, rettangolo, stella -->

Un grafico è un'immagine di numeri che resta collegata ad essi: se si modifica un numero, il grafico cambia di conseguenza. Immagini e forme si posizionano sopra le celle allo stesso modo, ma non sono collegate ad alcun numero.

## Creare un grafico

1. Seleziona i numeri da rappresentare. Se la prima riga contiene nomi e la prima colonna contiene etichette, seleziona anche quelle. JeffCrunch controlla la prima riga e la prima colonna e le usa come nomi e come etichette quando contengono testo (o date, per le etichette).
2. Nella scheda **Inserisci**, nel gruppo **Grafici**, scegli un tipo: **Colonna**, **Barra**, **Linea**, **Torta**, **Area**, **Dispersione** o **Combinato**. La piccola freccia accanto a un tipo mostra le sue varianti, come le colonne impilate, una linea con indicatori o un grafico a dispersione di punti, di punti collegati o di linee smussate.
3. Per vedere subito tutti i tipi, scegli **Altri grafici**. L'elenco a sinistra mostra ogni tipo disegnato con le tue celle. **Scambia righe e colonne** inverte il senso delle serie.

Non sai quale tipo scegliere? Seleziona le celle e scegli **Grafici consigliati** nella scheda **Inserisci**. JeffCrunch esamina le celle ed elenca alcuni tipi adatti, dal più indicato. Ad esempio, date disposte in verticale suggeriscono una linea, mentre una sola colonna di pochi numeri positivi con etichette suggerisce una torta. Ogni tipo viene disegnato con le tue celle. Scegline uno e fai clic su **OK**.

Se selezioni una sola cella, JeffCrunch crea il grafico con il blocco di celle che la circonda e che contiene dei dati.

Le serie vengono disposte lungo le colonne quando la selezione ha almeno tante righe quante colonne, e lungo le righe quando ha più colonne che righe. Una torta contiene una sola serie. Un grafico a dispersione prende i valori orizzontali dalla prima colonna.

Esempio: selezioni A1:C4, dove A2:A4 contengono Q1, Q2 e Q3, B1 e C1 contengono Est e Ovest, e il resto sono numeri. Il grafico avrà due serie, Est e Ovest, e tre gruppi di colonne, da Q1 a Q3.

## Nascondere temporaneamente una serie

Seleziona un grafico. Accanto ad esso compaiono tre piccoli pulsanti. Quello con l'imbuto è **Filtri grafico**. Mostra l'elenco delle serie del grafico, ciascuna con una casella di controllo. Deseleziona una casella per escludere la serie dal grafico. Selezionala di nuovo per riportarla nella stessa posizione. Anche **Annulla** funziona. Un grafico conserva sempre almeno una serie. Una serie esclusa non viene conservata quando salvi e chiudi il file, quindi selezionala di nuovo prima, se ti serve.

## Spostare e ridimensionare

Fai clic su un grafico, un'immagine o una forma per selezionarlo. Trascinalo per spostarlo, oppure trascina uno degli otto quadratini sul bordo per ridimensionarlo.

- Tieni premuto **Alt** mentre trascini per allineare un bordo al bordo di una cella.
- Tieni premuto **Maiusc** mentre trascini un angolo per mantenere le proporzioni.
- I tasti freccia spostano l'oggetto di pochi pixel alla volta. **Ctrl** con una freccia lo sposta di un pixel, **Maiusc** con una freccia lo ingrandisce o lo riduce, e **Alt** con una freccia lo porta fino al bordo successivo di una cella.
- Mentre trascini, **Esc** riporta l'oggetto nella posizione iniziale.

Ogni spostamento o ridimensionamento è un'unica operazione che puoi annullare. Fai clic con il pulsante destro sull'oggetto e scegli **Proprietà** per digitare il **Nome** (ogni oggetto di un foglio deve avere il suo) e il **Testo alternativo** (una breve frase che spiega a chi non può vedere l'oggetto che cosa mostra), per digitare le dimensioni e per scegliere cosa fa l'oggetto quando inserisci, elimini o ridimensioni le celle: si sposta e si ridimensiona con le celle, si sposta con esse ma mantiene le dimensioni, oppure resta dov'è. La scelta viene salvata nel file. **Stampa questo oggetto**, nella stessa finestra, disattiva l'oggetto nelle pagine stampate (resta visibile sullo schermo); anche il file conserva questa scelta.

## Modificare ciò che mostra un grafico

Quando un grafico è selezionato, accanto ad esso compaiono due pulsanti e nella barra multifunzione appare la scheda **Grafico**.

- Il pulsante **+** (e **Elementi grafico** nella scheda) attiva o disattiva il titolo del grafico, gli assi, i titoli degli assi, le linee della griglia, la legenda e le etichette dati, e indica dove vanno la legenda e le etichette.
- Il pulsante a pennello (e **Colori** e **Aspetto** nella scheda) assegna al grafico uno degli insiemi di colori e uno degli stili di JeffCrunch. **Layout** imposta più elementi contemporaneamente.
- **Dati grafico** elenca le serie del grafico. Puoi aggiungerle, modificarle, eliminarle e riordinarle, cambiare le celle che ciascuna legge, impostare le celle con le etichette, scambiare righe e colonne e scegliere come rappresentare le celle vuote e le righe e colonne nascoste.
- **Linea di tendenza** (nel menu **+**) disegna una linea adattata ai numeri di ogni serie, così puoi vedere dove si stanno dirigendo. Scegli **Lineare** per una retta, **Esponenziale** per numeri che crescono di una percentuale costante, **Logaritmica** per numeri che salgono rapidamente e poi si stabilizzano, **Polinomiale** per una curva con più flessi, **Potenza** oppure **Media mobile**, che disegna la media degli ultimi numeri. **Mostra equazione** scrive sul grafico la formula della linea, e **Mostra R al quadrato** scrive quanto bene la linea si adatta (1 indica un adattamento perfetto; 0 nessun adattamento). La linea di tendenza ha una voce propria nella legenda.
- Quando una serie è rappresentata su un secondo asse verticale, i menu **Assi** e **Titoli degli assi** hanno le voci **Orizzontale secondario** e **Verticale secondario** per essa. Il menu **Legenda** ha **Sopra il grafico**, che colloca la legenda sull'area del grafico invece che accanto ad essa.
- **Etichette dati** ha anche **Etichetta contiene** (il valore, il nome della categoria, il nome della serie, la percentuale e la chiave legenda, cioè un piccolo quadrato del colore della serie, in qualunque combinazione), **Separatore** (ciò che sta tra loro: virgola, punto e virgola, punto, a capo o spazio) e **Formato numero…** (un codice di formato come 0,0 per i numeri nelle etichette; lascia vuoto per mostrare i numeri così come sono). Se disattivi tutti i contenuti, le etichette scompaiono.
- **Formato asse orizzontale…** e **Formato asse verticale…** nella scheda **Grafico** aprono una finestra per un singolo asse. Imposta il valore minimo e massimo, le unità principali e secondarie (ciascuna automatica o un numero digitato), se i valori vanno in ordine inverso, se la scala è logaritmica (ogni segno è la base per il precedente, ad esempio 1, 10, 100 e 1000, adatta a numeri di grandezza molto diversa, che devono essere tutti maggiori di 0), il punto in cui l'altro asse interseca questo, il formato numerico delle sue etichette, i segni di graduazione, la posizione delle etichette e di quanto sono ruotate. Solo un asse numerico ha limiti e unità.
- **Sposta grafico…** nella scheda **Grafico** chiede un altro foglio di lavoro e vi sposta il grafico, nelle stesse celle. Continua a leggere le celle che leggeva, e mantiene le sue dimensioni e la sua posizione nel foglio, anche quando righe e colonne sono più alte o più larghe, e si sposta e ridimensiona con le celle come prima. I fogli nascosti non vengono proposti. È una sola operazione **Annulla**, anche dopo che hai salvato. Un foglio grafico (un foglio che contiene solo un grafico) non può ancora essere creato, e un grafico che si trova su un foglio grafico non può essere spostato.
- **Testo del titolo…** è disponibile anche in **Titolo grafico** e **Titoli degli assi** (nel menu **+**). Digita le parole che vuoi. Oppure digita un segno di uguale e una cella, ad esempio **=Foglio1!A1**, e il titolo mostrerà il contenuto di quella cella. Quando la cella cambia, cambia anche il titolo. Il file conserva il collegamento.
- **Salva come immagine…** nella scheda **Grafico** (e nel menu del pulsante destro) salva il grafico selezionato come file immagine **PNG** o **JPEG**, con il doppio delle dimensioni che ha sul foglio, così resta nitido.
- **Salva come modello…** e **Applica modello…** (nella scheda **Grafico**, nel gruppo **Tipo**) conservano l'aspetto di un grafico in un file **.crtx** e lo applicano a un altro grafico. L'aspetto comprende il tipo, i colori, le impostazioni del titolo e della legenda, gli assi e i caratteri. Il grafico a cui lo applichi conserva i propri numeri e le celle che legge. Un'operazione **Annulla** rimuove di nuovo l'aspetto.
- **Tipo di grafico** cambia il tipo per l'intero grafico o per ciascuna serie: colonne con una linea sopra, oppure una serie su un secondo asse verticale.

Un riferimento a celle si scrive come **Foglio1!$B$2:$B$9**, con il nome del foglio tra apici singoli quando contiene spazi.

## Sparkline

Uno sparkline è un piccolo grafico che sta dentro una singola cella. Mostra l'andamento di una riga o di una colonna di numeri, accanto ad essi.

1. Seleziona i numeri. Per uno sparkline per ogni riga, seleziona più righe di numeri.
2. Nella scheda **Inserisci**, nel gruppo **Sparkline**, scegli **Linea**, **Colonna** o **Vittoria/sconfitta**. **Linea** unisce i numeri con una linea. **Colonna** disegna una barra per ogni numero. **Vittoria/sconfitta** disegna una barra verso l'alto per ogni numero maggiore di zero e una barra verso il basso per ogni numero minore di zero, tutte della stessa dimensione: è adatta ai risultati che sono solo positivi o negativi.
3. Nella finestra **Crea sparkline** controlla l'**Intervallo dati** (le celle che lo sparkline legge) e digita l'**Intervallo di posizione** (le celle in cui vanno gli sparkline). Scegli una cella di posizione per ogni riga di dati, o per ogni colonna di dati. Se scegli una sola cella, lo sparkline legge tutti i dati.

Gli sparkline creati in un solo passaggio formano un gruppo e condividono lo stesso aspetto. Fai clic su una cella con uno sparkline e compare la scheda **Sparkline**.

- **Modifica dati** cambia i dati e la posizione dell'intero gruppo, oppure i dati dello sparkline su cui hai fatto clic. Indica anche come trattare le celle vuote (lasciare uno spazio, considerarle come zero o unire i punti attraverso di esse) e se leggere le celle nelle righe e colonne nascoste.
- **Tipo** passa da linea a colonna e vittoria/sconfitta.
- **Mostra** contrassegna i punti: **Punto massimo**, **Punto minimo**, **Punti negativi**, **Primo punto**, **Ultimo punto* e, per una linea, **Indicatori** su ogni punto.
- **Stili** assegna al gruppo una serie di colori. **Colore sparkline** imposta il colore della linea o delle barre e lo **Spessore** (quanto è spessa la linea, in punti). **Colore indicatore** imposta il colore di ciascun tipo di punto contrassegnato.
- **Asse** mostra una linea dello zero, disegna i numeri da destra a sinistra e imposta il valore minimo e massimo dell'asse verticale. **Automatico per ogni sparkline** adatta ciascuno ai propri numeri. **Uguale per tutti gli sparkline** dà all'intero gruppo una sola scala, così puoi confrontarli. **Valore personalizzato** ti permette di digitare un numero. **Asse delle date…** distanzia i punti in base alle date: digita le celle che contengono la data di ogni punto, e un punto con una data successiva si trova più avanti. Lascia vuoto per distribuire di nuovo i punti in modo uniforme.
- **Raggruppa** riunisce gli sparkline selezionati in un solo gruppo. **Separa** assegna a ciascuno un aspetto proprio. **Cancella** elimina gli sparkline selezionati, oppure i loro interi gruppi.

Uno sparkline segue le sue celle: se inserisci una riga sopra i dati, continua a leggere gli stessi numeri. Ogni modifica è un'operazione che puoi annullare. Gli sparkline vengono salvati nel file in un formato che gli altri programmi per fogli di calcolo sanno leggere.

## Inserire un'immagine, una forma o una casella di testo

Nella scheda **Inserisci**, nel gruppo **Illustrazioni**, **Immagine** apre una finestra di selezione file. Scegli un file immagine (PNG, JPEG, GIF, BMP, TIFF o SVG) e questo compare sul foglio, nella cella selezionata. Un'immagine più larga di 480 punti immagine viene ridotta per adattarsi e mantiene la sua forma. La cartella di lavoro conserva l'immagine stessa. Un'immagine di un tipo che la cartella di lavoro non può conservare così com'è (un SVG, ad esempio) viene salvata come PNG, un'immagine semplice composta da punti. Un file che non è un'immagine, o che supera i 16 MB, viene rifiutato con un messaggio.

**Forme** apre un elenco: rettangolo, rettangolo arrotondato, ovale, triangolo, rombo, frecce, una stella, un cuore, una nuvola, un fumetto e una linea. Scegline una e comparirà sul foglio in azzurro chiaro con un bordo più scuro. **Casella di testo**, nel gruppo **Testo**, inserisce una casella vuota senza riempimento e senza bordo.

Ciascuna di queste operazioni è un unico passaggio che puoi annullare. Il nuovo oggetto risulta selezionato, quindi puoi trascinarlo, ridimensionarlo, portarlo avanti o indietro, assegnargli un nome e un testo alternativo in **Proprietà**, oppure premere **Canc**.

## Modificare l'aspetto di un'immagine, una forma o una casella di testo

Seleziona un'immagine e compare la scheda **Formato immagine**. Seleziona una forma, una linea o una casella di testo e compare la scheda **Formato forma**. Ogni modifica è un passaggio che puoi annullare, e l'oggetto conserva il nome, il testo alternativo, la posizione e l'ordine tra gli altri oggetti.

- **Riempimento forma** e **Contorno forma** scelgono un colore oppure nessun colore. **Stile linea** imposta lo spessore (quanto è spessa la linea) e il tratteggio (continuo, tratteggiato o punteggiato). Un'immagine ha **Bordo immagine**, cioè il suo contorno.
- **Ruota** gira l'oggetto di un quarto di giro a destra o a sinistra, oppure lo capovolge da lato a lato o dall'alto in basso.
- **Ritaglia**, per un'immagine, la riduce a una forma di finestra: quadrata, 4 di larghezza per 3 di altezza, 3 di larghezza per 4 di altezza, 16 per 9 o 9 per 16. Il ritaglio è fatto in modo uniforme dal centro di ciò che si vede, e l'immagine viene ingrandita quanto basta per riempire ciò che resta. **Mostra l'immagine intera** ripristina la parte ritagliata. Il file dell'immagine non viene modificato: la parte ritagliata viene solo nascosta.
- **Modifica testo** serve per forme e caselle di testo. Digita le parole e scegli come si dispongono in orizzontale e in verticale, la dimensione, il grassetto o il normale, e se vanno a capo all'interno della forma o restano su una sola riga.
- **Riquadro di selezione** apre un elenco di tutti gli oggetti del foglio per nome, con quello in primo piano in cima. Fai clic su un nome per selezionare quell'oggetto, il che è utile quando gli oggetti si nascondono a vicenda. **Porta avanti** e **Porta indietro** lo spostano nell'elenco.
- **Proprietà** imposta le dimensioni, il nome, il testo alternativo e cosa fa l'oggetto quando le celle vengono spostate o ingrandite.

## Tagliare, copiare e incollare un oggetto

Un'immagine o una forma si copiano e si incollano nello stesso modo di un grafico. Una forma che usa qualcosa di diverso da un'immagine della cartella di lavoro, come un collegamento a un file, non può essere copiata, e JeffCrunch ti spiega il motivo.

Copia un grafico e incollalo nella stessa cartella di lavoro, in un foglio qualsiasi: la copia legge le stesse celle dell'originale. Incollalo in un'altra cartella di lavoro e conserva i numeri che mostrava, ma non legge più celle, perché le celle che leggerebbe lì non sono quelle da cui è stato creato.

## Dare un nome

Con un oggetto selezionato, la casella Nome mostra il suo nome. Digita un nuovo nome e premi **Invio**. **Annulla** ripristina il vecchio nome, e il nuovo nome viene salvato nel file.

## Eliminare un oggetto

Premi **Canc** con l'oggetto selezionato. **Annulla** lo ripristina.

## Che cosa viene salvato

Tutto quanto descritto sopra viene salvato nel file in un formato che gli altri programmi per fogli di calcolo sanno leggere, quindi un grafico creato qui si apre allo stesso modo anche là. Un grafico proveniente da un altro programma che contiene elementi che JeffCrunch non può conservare quando riscrive un grafico (barre di errore, un grafico tridimensionale) può comunque essere spostato, ridimensionato ed eliminato, ma i suoi elementi e i suoi dati non vengono modificati qui.
