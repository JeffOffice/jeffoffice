# Grafici, sparkline, immagini e forme

<!-- keywords: raggruppa, separa gruppo, allinea, allinea a sinistra, distribuisci, seleziona oggetti, più forme, selezione multipla, trascina riquadro, filtri grafico, nascondi serie, grafici consigliati, suggerimenti grafico, grafico, diagramma, colonne, barre, linea, torta, aerea, dispersione, combinato, legenda, titolo, linee della griglia, etichette dati, dati grafico, tipo di grafico, aspetto, layout, sposta, ridimensiona, immagine, forma, oggetto, serie, linea di tendenza, tendenza, equazione, R al quadrato, secondo asse, sparkline, sparkline, asse delle date, vittoria/sconfitta, mini grafico, indicatori, nome, testo alternativo, logaritmica, scala logaritmica, salva come immagine, modello, crtx, testo del titolo, chiave di legenda, riquadro di selezione, riempimento forma, contorno forma, ruota, capovolgi, ritaglia, modifica testo, bordo immagine, inserisci immagine, icone, icona, wmf, emf, stili forma, stili, inserisci forma, casella di testo, forme, freccia, ovale, rettangolo, stella -->

Un grafico è una rappresentazione di numeri che resta collegata ad essi: se cambi un numero, il grafico cambia di conseguenza. Immagini e forme si collocano sopra le celle allo stesso modo, ma non sono collegate ad alcun numero.

## Creare un grafico

1. Seleziona i numeri che vuoi rappresentare. Se la prima riga contiene nomi e la prima colonna contiene etichette, seleziona anche quelle. JeffCrunch controlla la prima riga e la prima colonna e le usa come nomi e come etichette quando sono parole (o date, per le etichette).
2. Nella scheda **Inserisci**, nel gruppo **Grafici**, scegli un tipo: **Colonna**, **Barra**, **Linea**, **Torta**, **Area**, **Dispersione** o **Combinato**. La piccola freccia accanto a un tipo mostra le sue varianti, ad esempio colonne impilate, una linea con indicatori o un grafico a dispersione di punti, di punti collegati o di linee smussate.
3. Per vedere subito tutti i tipi, scegli **Altri grafici**. L'elenco a sinistra mostra ogni tipo disegnato con i tuoi dati. **Scambia righe e colonne** inverte il senso delle serie.

Non sai quale tipo scegliere? Seleziona le celle e scegli **Grafici consigliati** nella scheda **Inserisci**. JeffCrunch esamina le celle ed elenca alcuni tipi adatti, dal più indicato. Ad esempio, date disposte in verticale suggeriscono una linea, mentre una sola colonna di pochi numeri positivi con etichette suggerisce una torta. Ogni tipo viene disegnato con i tuoi dati. Scegline uno e fai clic su **OK**.

Se selezioni una sola cella, JeffCrunch rappresenta il blocco di celle attorno ad essa che contiene dei dati.

Le serie scorrono lungo le colonne quando la selezione ha almeno tante righe quante colonne, e lungo le righe quando ha più colonne che righe. Una torta contiene una sola serie. Un grafico a dispersione prende i valori orizzontali dalla prima colonna.

Esempio: selezioni A1:C4, dove A2:A4 contengono Q1, Q2 e Q3, B1 e C1 contengono Est e Ovest, e il resto sono numeri. Il grafico ha due serie, Est e Ovest, e tre gruppi di colonne, da Q1 a Q3.

## Nascondere per il momento una serie

Seleziona un grafico. Accanto ad esso compaiono tre piccoli pulsanti. Quello con l'imbuto è **Filtri grafico**. Mostra le serie del grafico, ognuna con una casella di controllo. Deseleziona una casella per escludere quella serie dal grafico. Selezionala di nuovo per riportarla nella stessa posizione. Anche **Annulla** funziona. Un grafico conserva sempre almeno una serie. Una serie esclusa resta nel file quando lo salvi e rimane nascosta quando apri il file, quindi non si perde nulla. Seleziona di nuovo la casella per riportarla. (Un grafico ad anello mostra tutte le serie quando viene aperto.)

## Spostare e ridimensionare

Fai clic su un grafico, un'immagine o una forma per selezionarlo. Trascinalo per spostarlo, oppure trascina uno degli otto quadratini sul suo bordo per ridimensionarlo.

- Tieni premuto **Alt** mentre trascini per allineare un bordo al bordo di una cella.
- Tieni premuto **Maiusc** mentre trascini un angolo per mantenere le proporzioni.
- I tasti freccia lo spostano di alcuni pixel alla volta. **Ctrl** con una freccia lo sposta di un pixel, **Maiusc** con una freccia lo ingrandisce o lo riduce, e **Alt** con una freccia lo porta fino al bordo successivo di una cella.
- **Esc** mentre trascini lo riporta alla posizione iniziale.

Ogni spostamento o ridimensionamento è un'operazione che puoi annullare. Fai clic con il pulsante destro sull'oggetto e scegli **Proprietà** per digitare il **Nome** (ogni oggetto di un foglio deve avere il proprio) e il **Testo alternativo** (una breve frase che spiega a chi non può vedere l'oggetto che cosa mostra), per digitare le dimensioni e per scegliere come si comporta quando inserisci, elimini o ridimensioni celle: si sposta e si ridimensiona con le celle, si sposta con esse ma mantiene le proprie dimensioni, oppure resta dov'è. La scelta viene salvata nel file. **Stampa questo oggetto**, nella stessa finestra, esclude l'oggetto dalle pagine stampate (resta comunque visibile sullo schermo); anche questa scelta viene salvata nel file.

## Modificare ciò che mostra un grafico

Quando un grafico è selezionato, accanto ad esso compaiono due pulsanti e nella barra multifunzione appare la scheda **Grafico**.

- Il pulsante **+** (e **Elementi grafico** nella scheda) attiva o disattiva titolo del grafico, assi, titoli degli assi, linee della griglia, legenda ed etichette dati, e indica dove vanno la legenda e le etichette.
- Il pulsante pennello (e **Colori** e **Stili** nella scheda) assegna al grafico uno dei set di colori e uno degli stili di JeffCrunch. **Layout** imposta più elementi insieme.
- **Dati grafico** elenca le serie del grafico. Puoi aggiungerle, modificarle, rimuoverle e riordinarle, cambiare le celle che ciascuna legge, impostare le celle con le etichette, scambiare righe e colonne e scegliere come vengono rappresentate le celle vuote e le righe e colonne nascoste.
- **Barre di errore** (nel menu **+**, per un grafico a colonne, barre, linee o dispersione) disegna una piccola linea sopra e sotto ogni punto per mostrare di quanto il valore reale potrebbe discostarsi. Scegli **Errore standard**, **Percentuale (5%)**, **Deviazione standard** oppure **Valore fisso (1)**. Un file che ha i propri valori per ogni punto li conserva.
- **Linee** (nel menu **+**, per un grafico a linee) contiene **Linee di caduta**, che scendono dal punto più alto di ogni categoria fino all'asse, **Linee alto-basso**, che uniscono il punto più alto e quello più basso di ogni categoria, e **Barre su/giù**, che sono riquadri tra la prima e l'ultima serie di ogni categoria. Una barra verso l'alto indica che l'ultima serie è più alta. Le linee alto-basso e le barre su/giù richiedono due serie o più. I grafici ad area hanno le linee di caduta.
- **Linea di tendenza** (nel menu **+**) disegna una linea che approssima i numeri di ogni serie, così puoi vedere dove si stanno dirigendo. Scegli **Lineare** per una linea retta, **Esponenziale** per numeri che crescono di una percentuale costante, **Logaritmica** per numeri che crescono rapidamente e poi si stabilizzano, **Polinomiale** per una curva con delle variazioni, **Potenza**, oppure **Media mobile**, che disegna la media degli ultimi numeri. **Mostra equazione** scrive sul grafico la formula della linea, e **Mostra R al quadrato** scrive quanto bene la linea si adatta ai dati (1 indica un adattamento perfetto; 0 indica nessun adattamento). La linea di tendenza ha una voce propria nella legenda.
- Quando una serie è disegnata rispetto a un secondo asse verticale, i menu **Assi** e **Titoli degli assi** hanno le voci **Secondo orizzontale** e **Secondo verticale** per essa. Il menu **Legenda** ha **Sopra il grafico**, che colloca la legenda sull'area del grafico invece che accanto ad esso.
- **Etichette dati** dispone anche di **Etichetta contiene** (il valore, il nome della categoria, il nome della serie, la percentuale e la chiave di legenda, cioè un piccolo quadrato del colore della serie, in qualsiasi combinazione), **Separatore** (ciò che sta tra di essi: virgola, punto e virgola, punto, a capo o spazio) e **Formato numero…** (un codice di formato come 0,0 per i numeri delle etichette; lascialo vuoto per mostrare i numeri così come sono). **Valore da celle…** mostra il testo di alcune celle come etichette, una cella per ogni punto. Digita le celle, ad esempio =Foglio1!$D$2:$D$4. Le etichette cambiano quando cambiano le celle. Lascia il campo vuoto per eliminarle. Disattivando tutti i contenuti, le etichette scompaiono.
- **Formato asse orizzontale…** e **Formato asse verticale…** nella scheda **Grafico** aprono una finestra per un singolo asse. Qui si impostano i valori minimo e massimo e le unità principale e secondaria (ciascuna automatica oppure un numero digitato), se i valori vanno in ordine inverso, le unità di visualizzazione (le etichette mostrano i numeri divisi, ad esempio per 1.000 per le migliaia, quindi 30.000 si legge come 30; seleziona **Mostra il nome delle unità nel grafico** per stampare il nome, come Migliaia, insieme al titolo dell'asse), se la scala è logaritmica (ogni segno è una base moltiplicata per il precedente, ad esempio 1, 10, 100 e 1000, adatta a numeri di grandezza molto diversa che devono essere tutti maggiori di 0), dove l'altro asse interseca questo, il formato numerico delle sue etichette, i segni di graduazione, la posizione delle etichette e di quanto sono ruotate. Solo un asse numerico ha limiti e unità.
- **Sposta grafico…** nella scheda **Grafico** chiede un altro foglio di lavoro e vi colloca il grafico, nelle stesse celle. Continua a leggere le celle che leggeva, e mantiene le sue dimensioni e la sua posizione sul foglio, anche dove righe e colonne sono più alte o più larghe, e si sposta e ridimensiona con le celle come prima. I fogli nascosti non vengono proposti. È un'unica operazione di **Annulla**, anche dopo aver salvato. Un foglio grafico (un foglio che contiene solo un grafico) non può ancora essere creato, e un grafico che si trova su un foglio grafico non può essere spostato.
- **Titolo grafico** e **Titoli degli assi** (nel menu **+**) hanno anche **Testo del titolo…**. Digita il testo che desideri. Oppure digita un segno di uguale seguito da una cella, ad esempio **=Foglio1!A1**, e il titolo mostrerà ciò che contiene quella cella. Quando la cella cambia, cambia anche il titolo. Il file conserva il collegamento.
- **Salva come immagine…** nella scheda **Grafico** (e nel menu del pulsante destro) salva il grafico selezionato come file immagine **PNG** o **JPEG**, con il doppio delle dimensioni che ha sul foglio, per restare nitido.
- **Salva come modello…** e **Applica modello…** (nella scheda **Grafico**, nel gruppo **Tipo**) conservano l'aspetto di un grafico in un file **.crtx** e lo applicano a un altro grafico. L'aspetto comprende il tipo, i colori, le impostazioni del titolo e della legenda, gli assi e i tipi di carattere. Il grafico a cui lo applichi mantiene i propri numeri e le celle che legge. Un'unica operazione di **Annulla** elimina di nuovo l'aspetto.
- **Tipo di grafico** cambia il tipo per l'intero grafico oppure per ciascuna serie: colonne con una linea sopra, oppure una serie rispetto a un secondo asse verticale.

Un riferimento a celle si scrive come **Foglio1!$B$2:$B$9**, con il nome del foglio tra apici quando contiene spazi.

## Sparkline

Uno sparkline è un piccolo grafico che sta dentro una sola cella. Mostra l'andamento di una riga o di una colonna di numeri proprio accanto ad essi.

1. Seleziona i numeri. Per uno sparkline per ogni riga, seleziona più righe di numeri.
2. Nella scheda **Inserisci**, nel gruppo **Sparkline**, scegli **Linea**, **Colonna** o **Vittoria/Sconfitta**. Una **Linea** unisce i numeri con una linea. Una **Colonna** disegna una barra per ogni numero. Una **Vittoria/Sconfitta** disegna una barra verso l'alto per ogni numero maggiore di zero e una barra verso il basso per ogni numero inferiore, tutte di uguale altezza, il che è adatto a risultati che sono soltanto buoni o cattivi.
3. Nella finestra **Crea sparkline**, verifica l'**Intervallo dati** (le celle che lo sparkline legge) e digita l'**Intervallo di posizione** (le celle in cui vanno). Scegli una cella di posizione per ogni riga di dati, oppure per ogni colonna di dati. Se scegli una sola cella, lo sparkline legge tutti i dati.

Gli sparkline creati in un unico passaggio formano un gruppo e condividono lo stesso aspetto. Fai clic su una cella con uno sparkline e compare la scheda **Sparkline**.

- **Modifica dati** cambia i dati e la posizione dell'intero gruppo, oppure i dati di quello su cui hai fatto clic. Indica anche cosa fare con le celle vuote (lasciare uno spazio, considerarle come zero oppure unire i punti attraverso di esse) e se leggere le celle nelle righe e nelle colonne nascoste.
- **Tipo** passa da linea a colonna e a vittoria/sconfitta.
- **Mostra** evidenzia dei punti: **Punto massimo**, **Punto minimo**, **Punti negativi**, **Primo punto**, **Ultimo punto» e, per una linea, **Indicatori** su ogni punto.
- **Stili** assegna al gruppo un set di colori. **Colore sparkline** imposta il colore della linea o delle barre e lo **Spessore** (quanto è spessa la linea, in punti). **Colore indicatore** imposta il colore di ciascun tipo di punto evidenziato.
- **Asse** mostra una linea allo zero, traccia i numeri da destra a sinistra e imposta i valori minimo e massimo dell'asse verticale. **Automatico per ogni sparkline** adatta ciascuno ai propri numeri. **Uguale per tutti gli sparkline** assegna all'intero gruppo una sola scala, così puoi confrontarli. **Valore personalizzato** ti permette di digitare un numero. **Asse delle date…** distanzia i punti in base alle date: digita le celle che contengono la data di ogni punto, e un punto con una data successiva si colloca più avanti. Lascia il campo vuoto per distribuire di nuovo i punti in modo uniforme.
- **Raggruppa** riunisce gli sparkline selezionati in un solo gruppo. **Separa** assegna a ciascuno un aspetto proprio. **Elimina** rimuove gli sparkline selezionati o interi gruppi.

Uno sparkline segue le sue celle: se inserisci una riga sopra i dati, continua a leggere gli stessi numeri. Ogni modifica è un'operazione che puoi annullare. Gli sparkline vengono salvati nel file in una forma che gli altri programmi di fogli di calcolo leggono.

## Inserire un'immagine, una forma o una casella di testo

Nella scheda **Inserisci**, nel gruppo **Illustrazioni**, **Immagine** apre una finestra di selezione file. Scegli un file immagine (PNG, JPEG, GIF, BMP, TIFF o SVG) e questo compare sul foglio nella cella selezionata. Un'immagine più larga di 480 pixel viene ridotta per adattarsi e mantiene la sua forma. La cartella di lavoro conserva l'immagine stessa. Un'immagine di un tipo che la cartella di lavoro non può conservare così com'è (ad esempio un SVG) viene salvata come PNG, un'immagine normale fatta di punti. Un file che non è un'immagine, oppure che supera i 16 MB, viene rifiutato con un messaggio. Un file WMF o EMF (i tipi di immagine che alcuni programmi meno recenti creano a partire da linee e forme) viene disegnato e salvato anch'esso come PNG.

**Icone** mostra la stessa finestra di piccole immagini a linee che hanno JeffPub e JeffDeck, con una casella per cercarle per nome. Scegline una o più: ciascuna compare sul foglio come immagine, larga e alta 48 punti. Viene disegnata in grigio scuro, con una dimensione che resta nitida quando la ingrandisci. Ciascuna è un'operazione che puoi annullare.

**Forme** apre un elenco: rettangolo, rettangolo arrotondato, ovale, triangolo, rombo, frecce, una stella, un cuore, una nuvola, un fumetto e una linea. Scegline una e compare sul foglio in azzurro chiaro con un bordo più scuro. **Casella di testo**, nel gruppo **Testo**, inserisce una casella vuota senza riempimento e senza bordo.

Con una forma selezionata, **Stili** nella scheda **Formato forma** le assegna un aspetto predefinito: un riempimento e un contorno che si abbinano, ad esempio azzurro chiaro con un bordo blu più scuro. Gli ultimi due mantengono solo il contorno o solo il riempimento. Un clic è un'operazione che puoi annullare. **Riempimento forma** e **Contorno forma** modificano ciascuna parte separatamente.

Ogni inserimento è un'operazione che puoi annullare. L'oggetto nuovo risulta selezionato, quindi puoi trascinarlo, ridimensionarlo, portarlo avanti o indietro, assegnargli un nome e un testo alternativo in **Proprietà**, oppure premere **Canc**.

## Modificare l'aspetto di un'immagine, una forma o una casella di testo

Seleziona un'immagine e compare la scheda **Formato immagine**. Seleziona una forma, una linea o una casella di testo e compare la scheda **Formato forma**. Ogni modifica è un'operazione che puoi annullare, e l'oggetto mantiene il proprio nome, il testo alternativo, la posizione e l'ordine rispetto agli altri oggetti.

- **Riempimento forma** e **Contorno forma** scelgono un colore, oppure nessuno. **Stile linea** imposta lo spessore (quanto è spessa la linea) e i tratteggi (continuo, tratteggiato o punteggiato). Un'immagine dispone di **Bordo immagine**, che è il suo contorno.
- **Ruota** gira l'oggetto di un quarto di giro a destra o a sinistra, oppure lo capovolge orizzontalmente o verticalmente.
- **Ritaglia**, per un'immagine, la taglia secondo una forma di finestra: quadrata, 4 per 3 in larghezza e in altezza... nello specifico 4 di larghezza per 3 di altezza, 3 di larghezza per 4 di altezza, 16 per 9 oppure 9 per 16. Il taglio viene fatto in modo uniforme dal centro di ciò che si vede, e l'immagine viene ingrandita quanto basta per riempire ciò che resta. **Mostra l'immagine intera** ripristina la parte tagliata. Il file dell'immagine non viene modificato: la parte tagliata viene solo nascosta.
- **Modifica testo** serve per forme e caselle di testo. Digita le parole e scegli come si dispongono in orizzontale e in verticale, la dimensione, il grassetto o il normale, e se vanno a capo entro la forma o restano su una sola riga.
- **Riquadro di selezione** apre un elenco di tutti gli oggetti del foglio per nome, con quello davanti per primo. Fai clic su un nome per selezionare quell'oggetto, utile quando gli oggetti si nascondono a vicenda. **Porta avanti** e **Porta indietro** lo spostano nell'elenco.
- **Proprietà** imposta le dimensioni, il nome, il testo alternativo e ciò che l'oggetto fa quando le celle vengono spostate o ingrandite.

## Tagliare, copiare e incollare un oggetto

Un'immagine o una forma si copia e si incolla come un grafico. Una forma che usa qualcosa di diverso da un'immagine della cartella di lavoro, ad esempio un collegamento a un file, non può essere copiata, e JeffCrunch ne indica il motivo.

Copia un grafico e incollalo nella stessa cartella di lavoro, su un foglio qualsiasi: la copia legge le stesse celle dell'originale. Incollalo in un'altra cartella di lavoro e conserva i numeri che mostrava, ma non legge più celle, perché le celle che leggerebbe lì non sono quelle da cui è stato creato.

## Selezionare più oggetti, raggrupparli e allinearli

Fai clic su un oggetto, poi tieni premuto Maiusc oppure Ctrl e fai clic su un altro per selezionarlo anche lui. Fai clic di nuovo su un oggetto selezionato tenendo premuto Maiusc o Ctrl per deselezionarlo. Gli oggetti selezionati mostrano ciascuno una cornice tratteggiata. Puoi anche scegliere **Seleziona oggetti** nella scheda **Formato forma** e trascinare un riquadro su celle vuote: vengono selezionati tutti gli oggetti che stanno interamente dentro il riquadro. Premi Esc, oppure scegli di nuovo **Seleziona oggetti**, per tornare a selezionare celle. Premi Canc per eliminare in un solo passaggio tutti gli oggetti selezionati.

Con due o più oggetti selezionati, la scheda **Formato forma** offre questi comandi nel gruppo **Disponi**:

- **Raggruppa** unisce le forme, le immagini e le linee selezionate in un unico oggetto. Si sposta e si ridimensiona come un unico elemento, e il file lo conserva come gruppo. Un grafico non può essere inserito in un gruppo.
- **Separa** scompone un gruppo selezionato. Ogni parte torna dove si trova sul foglio.
- **Allinea** allinea gli oggetti selezionati: **Allinea a sinistra**, **Allinea al centro**, **Allinea a destra**, **Allinea in alto**, **Allinea al centro verticalmente** e **Allinea in basso**. Si allineano al bordo o al centro dell'intero insieme. **Distribuisci orizzontalmente** e **Distribuisci verticalmente** distanziano in modo uniforme tre o più oggetti tra i due estremi.

Raggruppa, Separa e ciascuna scelta di allineamento sono un'unica operazione che puoi annullare.

## Dare un nome

Con un oggetto selezionato, la casella Nome mostra il suo nome. Digita un nuovo nome e premi **Invio**. **Annulla** ripristina il nome precedente, e il nuovo nome viene salvato nel file.

## Eliminare un oggetto

Premi **Canc** con l'oggetto selezionato. **Annulla** lo ripristina.

## Che cosa viene salvato

Tutto quanto descritto sopra viene salvato nel file in una forma che gli altri programmi di fogli di calcolo leggono, quindi un grafico creato qui si apre allo stesso modo anche lì. Un grafico proveniente da un altro programma che contiene elementi che JeffCrunch non può conservare quando riscrive un grafico (barre di errore, un grafico tridimensionale) può comunque essere spostato, ridimensionato ed eliminato, ma i suoi elementi e i suoi dati non vengono modificati qui.
