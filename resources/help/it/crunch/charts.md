# Grafici, sparkline, immagini e forme

<!-- keywords: pulsanti campo, pulsanti grafico pivot, raggruppa, separa, allinea, allinea a sinistra, distribuisci, seleziona oggetti, più forme, seleziona più oggetti, riquadro di selezione, casella di trascinamento, filtri grafico, nascondi serie, grafici consigliati, suggerimenti grafico, grafico, diagramma, colonne, barre, linea, torta, aree, dispersione, combinato, legenda, titolo, linee della griglia, etichette dati, dati grafico, tipo di grafico, aspetto, layout, sposta, ridimensiona, immagine, forma, oggetto, serie, linea di tendenza, tendenza, equazione, R al quadrato, secondo asse, sparkline, sparkline, asse delle date, vittoria/sconfitta, mini grafico, indicatori, nome, testo alternativo, logaritmica, scala logaritmica, salva come immagine, modello, crtx, testo del titolo, chiave della legenda, riquadro di selezione, riempimento forma, contorno forma, ruota, capovolgi, ritaglia, modifica testo, bordo immagine, inserisci immagine, icone, icona, wmf, emf, stili forma, stili, inserisci forma, casella di testo, forme, freccia, ovale, rettangolo, stella, diagramma, diagrammi, grafico a dispersione, colonne impilate -->

Un grafico è un'immagine di numeri che resta collegata ai numeri: se cambi un numero, il grafico cambia insieme a esso. Immagini e forme si posizionano sopra le celle allo stesso modo, ma non sono collegate ad alcun numero.

## Creare un grafico

1. Seleziona i numeri che vuoi rappresentare. Se la prima riga contiene nomi e la prima colonna contiene etichette, seleziona anche quelle. JeffCrunch controlla la prima riga e la prima colonna e le usa come nomi ed etichette quando sono parole (o date, per le etichette).
2. Nella scheda **Inserisci**, nel gruppo **Grafici**, scegli un tipo: **Colonna**, **Barre**, **Linea**, **Torta**, **Aree**, **Dispersione** o **Combinato**. La piccola freccia accanto a un tipo mostra le sue varianti, come le colonne impilate, una linea con indicatori oppure un grafico a dispersione di punti, di punti uniti o di linee smussate.
3. Per vedere subito tutti i tipi, scegli **Altri grafici**. L'elenco a sinistra mostra ogni tipo disegnato con i tuoi dati. **Scambia righe e colonne** inverte il modo in cui sono disposte le serie.

Non sai quale tipo scegliere? Seleziona le celle e scegli **Grafici consigliati** nella scheda **Inserisci**. JeffCrunch esamina le celle ed elenca alcuni tipi adatti, dal più indicato. Ad esempio, date disposte in verticale suggeriscono una linea, mentre una colonna di pochi numeri positivi con etichette suggerisce una torta. Ogni tipo viene disegnato con le tue celle. Scegline uno e fai clic su **OK**.

Se selezioni una sola cella, JeffCrunch rappresenta il blocco di celle intorno a essa che contiene dati.

Le serie scorrono lungo le colonne quando la selezione ha almeno tante righe quante colonne, e lungo le righe quando ha più colonne che righe. Una torta contiene una sola serie. Un grafico a dispersione prende i valori orizzontali dalla prima colonna.

Esempio: selezioni A1:C4, dove A2:A4 contengono Q1, Q2 e Q3, B1 e C1 contengono Est e Ovest, e il resto sono numeri. Il grafico avrà due serie, Est e Ovest, e tre gruppi di colonne, da Q1 a Q3.

## Nascondere una serie temporaneamente

Seleziona un grafico. Accanto a esso compaiono tre piccoli pulsanti. Quello con l'imbuto è **Filtri grafico**. Mostra le serie del grafico, ciascuna con una casella di controllo. Deseleziona una casella per escludere quella serie dal grafico. Selezionala di nuovo per riportarla nella stessa posizione. Anche **Annulla** funziona. Un grafico conserva sempre almeno una serie. Una serie esclusa resta nel file al salvataggio e rimane nascosta quando apri il file, quindi non si perde nulla. Seleziona di nuovo la casella per riportarla. (Un grafico a ciambella mostra tutte le serie quando viene aperto.)

## Spostare e ridimensionare

Fai clic su un grafico, un'immagine o una forma per selezionarlo. Trascinalo per spostarlo, oppure trascina uno degli otto quadratini sul bordo per ridimensionarlo.

- Tieni premuto **Alt** mentre trascini per allineare un bordo al bordo di una cella.
- Tieni premuto **Maiusc** mentre trascini un angolo per mantenere le proporzioni.
- I tasti freccia spostano l'oggetto di alcuni pixel alla volta. **Ctrl** con una freccia lo sposta di un pixel, **Maiusc** con una freccia lo ingrandisce o lo riduce, e **Alt** con una freccia lo porta fino al bordo successivo di una cella.
- **Esc** durante il trascinamento lo riporta alla posizione iniziale.

Ogni spostamento o ridimensionamento è un'azione che puoi annullare. Fai clic con il pulsante destro sull'oggetto e scegli **Proprietà** per digitare il **Nome** (ogni oggetto del foglio deve avere il proprio) e il **Testo alternativo** (una breve frase che spiega a chi non può vedere l'oggetto che cosa mostra), per digitare le dimensioni e per scegliere come si comporta quando inserisci, elimini o ridimensioni celle: spostare e ridimensionare con le celle, spostarsi con esse ma mantenere le dimensioni, oppure restare dove si trova. La scelta viene salvata nel file. **Stampa questo oggetto**, nella stessa finestra, esclude l'oggetto dalle pagine stampate (resta visibile sullo schermo); anche questa scelta viene mantenuta nel file.

## Modificare ciò che mostra un grafico

Quando un grafico è selezionato, accanto a esso compaiono due pulsanti e nella barra multifunzione appare la scheda **Grafico**.

- Il pulsante **+** (e **Elementi grafico** nella scheda) attiva o disattiva il titolo del grafico, gli assi, i titoli degli assi, le linee della griglia, la legenda e le etichette dati, e indica dove vanno la legenda e le etichette.
- Il pulsante pennello (e **Colori** e **Aspetto** nella scheda) assegna al grafico uno dei set di colori e uno degli aspetti di JeffCrunch. **Layout** imposta più elementi in una sola volta.
- **Dati grafico** elenca le serie del grafico. Puoi aggiungerle, modificarle, rimuoverle e riordinarle, cambiare le celle che ciascuna legge, impostare le celle con le etichette, scambiare righe e colonne e scegliere come rappresentare le celle vuote e le righe e colonne nascoste.
- **Pulsanti campo** (nel menu **+**, solo per un grafico pivot) mostra piccoli pulsanti con i nomi dei campi della tabella pivot a cui il grafico è collegato: i filtri del report sotto il titolo e le righe, i valori e le colonne lungo il bordo inferiore. Attiva o disattiva ciascun tipo oppure scegli **Mostra tutto** o **Nascondi tutto**. Un nuovo grafico pivot li mostra.
- **Barre di errore** (nel menu **+**, per un grafico a colonne, barre, linee, aree o dispersione) disegna una piccola linea sopra e sotto ogni punto per mostrare di quanto il valore reale potrebbe discostarsi. Scegli **Errore standard**, **Percentuale (5%)**, **Deviazione standard** o **Valore fisso (1)**. Scegli **Personalizzato** per usare importi tuoi: digita le celle che contengono gli importi sopra ogni punto (ad esempio `=Foglio1!$D$2:$D$4`), poi le celle per gli importi sotto. Lasciane una vuota per disegnare le barre da un solo lato. Le barre cambiano quando cambiano quelle celle. Un file che contiene per ogni punto importi propri li conserva.
- **Linee** (nel menu **+**, per un grafico a linee) include **Linee di caduta**, che vanno dal punto più alto di ogni categoria fino all'asse, **Linee alto-basso**, che uniscono il punto più alto e quello più basso di ogni categoria, e **Barre su/giù**, che sono riquadri tra la prima e l'ultima serie di ogni categoria. Una barra verso l'alto indica che l'ultima serie è più alta. Le linee alto-basso e le barre su/giù richiedono due serie o più. I grafici ad aree hanno le linee di caduta.
- **Linea di tendenza** (nel menu **+**) disegna una linea adattata ai numeri di ogni serie, così puoi vedere la direzione in cui vanno. Scegli **Lineare** per una linea retta, **Esponenziale** per numeri che crescono di una percentuale costante, **Logaritmica** per numeri che salgono rapidamente e poi si stabilizzano, **Polinomiale** per una curva con curve, **Potenza**, oppure **Media mobile**, che disegna la media degli ultimi numeri. **Mostra equazione** scrive la formula della linea sul grafico, e **Mostra R al quadrato** indica quanto bene la linea si adatta ai dati (1 indica un adattamento perfetto; 0 indica nessun adattamento). La linea di tendenza ha una voce propria nella legenda.
- Quando una serie è disegnata rispetto a un secondo asse verticale, i menu **Assi** e **Titoli degli assi** hanno le voci **Secondo orizzontale** e **Secondo verticale** per quella serie. Il menu **Legenda** ha **Sopra il grafico**, che colloca la legenda sull'area del grafico invece che accanto ad essa.
- **Etichette dati** include anche **Etichetta contiene** (il valore, il nome della categoria, il nome della serie, la percentuale e la chiave della legenda, cioè un piccolo quadrato del colore della serie, in qualsiasi combinazione), **Separatore** (ciò che si trova tra questi elementi: virgola, punto e virgola, punto, a capo o spazio) e **Formato numero…** (un codice di formato come 0,0 per i numeri nelle etichette; lascialo vuoto per mostrare i numeri così come sono). **Valore da celle…** mostra il testo di alcune celle come etichette, una cella per ogni punto. Digita le celle, ad esempio =Foglio1!$D$2:$D$4. Le etichette cambiano quando cambiano le celle. Lascialo vuoto per eliminarle. Se disattivi tutti i contenuti, le etichette scompaiono.
- **Formato asse orizzontale…** e **Formato asse verticale…** nella scheda **Grafico** aprono una finestra per un singolo asse. Imposta i valori minimo e massimo e le unità principale e secondaria (ciascuna automatica oppure un numero che digiti), se i valori vanno in ordine inverso, le unità di visualizzazione (le etichette mostrano i numeri divisi, ad esempio per 1.000 per le migliaia, quindi 30.000 si legge come 30; seleziona **Mostra il nome delle unità nel grafico** per stampare il nome, come Migliaia, insieme al titolo dell'asse), se la scala è logaritmica (ogni segno è una base moltiplicata per il precedente, come 1, 10, 100 e 1000, adatta a numeri che differiscono molto di grandezza e che devono essere tutti maggiori di 0), il punto in cui l'altro asse interseca questo, il formato numerico delle sue etichette, i segni di graduazione, la posizione delle etichette e di quanto sono ruotate. Solo un asse numerico ha limiti e unità.
- **Sposta grafico…** nella scheda **Grafico** chiede dove deve andare il grafico. Scegli **Nuovo foglio** e digita un nome, e il grafico avrà un foglio grafico proprio: un foglio che contiene solo il grafico, senza celle. Scegli **Oggetto in** e seleziona un foglio di lavoro, e il grafico verrà collocato su quel foglio. Un grafico su un foglio grafico può essere spostato solo su un foglio di lavoro, e il foglio grafico che lascia viene eliminato. Il grafico continua a leggere le celle che leggeva. Su un foglio di lavoro mantiene le sue dimensioni e la sua posizione sul foglio, anche dove righe e colonne sono più alte o più larghe, e si sposta e ridimensiona con le celle come prima. Non vengono proposti i fogli nascosti. È una sola azione di **Annulla**, anche dopo il salvataggio, a meno che un foglio sia stato eliminato dall'operazione. Un file in formato `.ods` non può contenere un foglio grafico, quindi salva un foglio grafico nel formato `.xlsx`.
- **Titolo grafico** e **Titoli degli assi** (nel menu **+**) hanno anche **Testo del titolo…**. Digita il testo che vuoi. Oppure digita un segno di uguale e una cella, come **=Foglio1!A1**, e il titolo mostrerà il contenuto di quella cella. Quando la cella cambia, cambia anche il titolo. Il file conserva il collegamento.
- **Salva come immagine…** nella scheda **Grafico** (e nel menu del clic destro) salva il grafico selezionato come file immagine **PNG** o **JPEG**, con dimensioni doppie rispetto a quelle sul foglio, per mantenerlo nitido.
- **Salva come modello…** e **Applica modello…** (nella scheda **Grafico**, nel gruppo **Tipo**) conservano l'aspetto di un grafico in un file **.crtx** e lo applicano a un altro grafico. L'aspetto comprende il tipo, i colori, le impostazioni del titolo e della legenda, gli assi e i caratteri. Il grafico al quale lo applichi mantiene i propri numeri e le celle che legge. Un'azione di **Annulla** elimina di nuovo l'aspetto.
- **Tipo di grafico** cambia il tipo per l'intero grafico oppure per ciascuna serie: colonne con una linea sopra, oppure una serie rispetto a un secondo asse verticale.

Un riferimento a celle si scrive come **Foglio1!$B$2:$B$9**, con il nome del foglio tra apici singoli quando contiene spazi.

## Sparkline

Uno sparkline è un piccolo grafico che sta dentro una sola cella. Mostra l'andamento di una riga o di una colonna di numeri accanto ad essi.

1. Seleziona i numeri. Per uno sparkline per riga, seleziona più righe di numeri.
2. Nella scheda **Inserisci**, nel gruppo **Sparkline**, scegli **Linea**, **Colonna** o **Vittoria/Sconfitta**. **Linea** unisce i numeri con una linea. **Colonna** disegna una barra per ogni numero. **Vittoria/Sconfitta** disegna una barra verso l'alto per ogni numero maggiore di zero e una barra verso il basso per ogni numero minore di zero, tutte della stessa dimensione, adatto a risultati che sono solo positivi o negativi.
3. Nella finestra **Crea sparkline**, controlla l'**Intervallo dati** (le celle che gli sparkline leggono) e digita l'**Intervallo di posizione** (le celle in cui vanno). Scegli una cella di posizione per ogni riga di dati, oppure per ogni colonna di dati. Se scegli una sola cella, lo sparkline legge tutti i dati.

Gli sparkline creati in una sola operazione formano un gruppo e condividono lo stesso aspetto. Fai clic su una cella con uno sparkline e comparirà la scheda **Sparkline**.

- **Modifica dati** cambia i dati e la posizione dell'intero gruppo, oppure i dati di quello su cui hai fatto clic. Imposta anche come trattare le celle vuote (lasciare uno spazio, considerarle zero o unire i punti attraverso di esse) e se leggere le celle nelle righe e colonne nascoste.
- **Tipo** passa da linea a colonna a vittoria/sconfitta.
- **Mostra** evidenzia i punti: **Punto massimo**, **Punto minimo**, **Punti negativi**, **Primo punto**, **Ultimo punto** e, per una linea, **Indicatori** su ogni punto.
- **Stili** assegna al gruppo un insieme di colori. **Colore sparkline** imposta il colore della linea o delle barre e lo **Spessore** (quanto è spessa la linea, in punti). **Colore indicatore** imposta il colore di ciascun tipo di punto evidenziato.
- **Asse** mostra una linea allo zero, traccia i numeri da destra a sinistra e imposta i valori minimo e massimo dell'asse verticale. **Automatico per ogni sparkline** adatta ciascuno ai propri numeri. **Uguale per tutti gli sparkline** assegna all'intero gruppo una sola scala, così puoi confrontarli. **Valore personalizzato** ti permette di digitare un numero. **Asse delle date…** distanzia i punti in base alle date: digita le celle che contengono la data di ciascun punto, e un punto con una data successiva si trova più avanti. Lascialo vuoto per distribuire di nuovo i punti in modo uniforme.
- **Gruppo** riunisce in un solo gruppo gli sparkline selezionati. **Separa** assegna a ciascuno un aspetto proprio. **Cancella** elimina gli sparkline selezionati, oppure i loro interi gruppi.

Uno sparkline segue le sue celle: se inserisci una riga sopra i dati, continua a leggere gli stessi numeri. Ogni modifica è un'azione che puoi annullare. Gli sparkline vengono salvati nel file in un formato che gli altri programmi per fogli di calcolo sanno leggere.

## Inserire un'immagine, una forma o una casella di testo

Nella scheda **Inserisci**, nel gruppo **Illustrazioni**, **Immagine** apre una finestra di selezione file. Scegli un file immagine (PNG, JPEG, GIF, BMP, TIFF o SVG) e questo compare sul foglio nella cella selezionata. Un'immagine più larga di 480 punti viene ridotta per adattarsi, mantenendo la sua forma. La cartella di lavoro conserva l'immagine stessa. Un'immagine di un tipo che la cartella di lavoro non può contenere così com'è (ad esempio un SVG) viene conservata come PNG, cioè un'immagine semplice fatta di punti. Un file che non è un'immagine, oppure più grande di 16 MB, viene rifiutato con un messaggio. Un file WMF o EMF (i tipi di immagine che alcuni programmi meno recenti creano a partire da linee e forme) viene disegnato e conservato anch'esso come PNG.

**Icone** mostra la stessa finestra di piccole immagini a tratto che hanno JeffPub e JeffDeck, con una casella per cercarle per nome. Scegline una o più, e ciascuna viene inserita sul foglio come immagine, larga e alta 48 punti. Viene disegnata in grigio scuro, con una dimensione che resta nitida quando la ingrandisci. Ciascuna è un'unica azione che puoi annullare.

**Forme** apre un elenco: rettangolo, rettangolo arrotondato, ovale, triangolo, rombo, frecce, una stella, un cuore, una nuvola, un fumetto e una linea. Scegline una e comparirà sul foglio in azzurro chiaro con un bordo più scuro. **Casella di testo**, nel gruppo **Testo**, inserisce una casella vuota senza riempimento né bordo.

Con una forma selezionata, **Stili** nella scheda **Formato forma** le assegna un aspetto già pronto: un riempimento e un contorno che si abbinano, come azzurro chiaro con un bordo blu più scuro. Gli ultimi due mantengono solo il contorno o solo il riempimento. Un clic è un'azione che puoi annullare. **Riempimento forma** e **Contorno forma** cambiano ciascuna parte separatamente.

Ogni operazione è un'azione che puoi annullare. L'oggetto nuovo viene selezionato, così puoi trascinarlo, ridimensionarlo, portarlo avanti o indietro, assegnargli un nome e un testo alternativo in **Proprietà**, oppure premere **Canc**.

## Modificare l'aspetto di un'immagine, di una forma o di una casella di testo

Seleziona un'immagine e compare la scheda **Formato immagine**. Seleziona una forma, una linea o una casella di testo e compare la scheda **Formato forma**. Ogni modifica è un'azione che puoi annullare, e l'oggetto mantiene il suo nome, il suo testo alternativo, la sua posizione e il suo ordine rispetto agli altri oggetti.

- **Riempimento forma** e **Contorno forma** scelgono un colore, oppure nessuno. **Stile linea** imposta lo spessore (quanto è spessa la linea) e i tratteggi (continui, tratteggiati o punteggiati). Un'immagine ha **Bordo immagine**, che è il suo contorno.
- **Ruota** fa girare l'oggetto di un quarto di giro a destra o a sinistra, oppure lo capovolge orizzontalmente o verticalmente.
- **Ritaglia**, per un'immagine, la riduce a una forma di finestra: quadrata, 4 di larghezza per 3 di altezza, 3 di larghezza per 4 di altezza, 16 per 9 oppure 9 per 16. Il ritaglio viene fatto in modo uniforme dal centro di ciò che si vede, e l'immagine viene ingrandita quanto basta a riempire ciò che resta. **Mostra l'immagine intera** ripristina la parte ritagliata. Il file dell'immagine non viene modificato: la parte ritagliata viene solo nascosta.
- **Modifica testo** serve per forme e caselle di testo. Digita le parole e scegli come si dispongono in orizzontale e in verticale, la dimensione, il grassetto o il normale, e se vanno a capo all'interno della forma o restano su una sola riga.
- **Riquadro di selezione** apre un elenco di tutti gli oggetti del foglio per nome, quello in primo piano per primo. Fai clic su un nome per selezionare quell'oggetto, utile quando gli oggetti si nascondono a vicenda. **Porta avanti** e **Porta indietro** lo spostano nell'elenco.
- **Proprietà** imposta le dimensioni, il nome, il testo alternativo e il comportamento dell'oggetto quando le celle vengono spostate o ingrandite.

## Tagliare, copiare e incollare un oggetto

Un'immagine o una forma si copia e si incolla come un grafico. Una forma che usa qualcosa di diverso da un'immagine della cartella di lavoro, come un collegamento a un file, non può essere copiata, e JeffCrunch ti dice il motivo.

Copia un grafico e incollalo nella stessa cartella di lavoro, su un foglio qualsiasi: la copia legge le stesse celle del primo. Incollalo in un'altra cartella di lavoro e conserva i numeri che mostrava, ma non legge più celle, perché le celle che leggerebbe lì non sono quelle da cui è stato creato.

## Selezionare più oggetti, raggrupparli e allinearli

Fai clic su un oggetto, poi tieni premuto Maiusc o Ctrl e fai clic su un altro per selezionare anche questo. Fai di nuovo clic su un oggetto selezionato tenendo premuto Maiusc o Ctrl per deselezionarlo. Gli oggetti selezionati hanno ciascuno una cornice tratteggiata. Puoi anche scegliere **Seleziona oggetti** nella scheda **Formato forma** e trascinare un riquadro sopra celle vuote: vengono selezionati tutti gli oggetti che rientrano interamente nel riquadro. Premi Esc, oppure scegli di nuovo **Seleziona oggetti**, per tornare alla selezione delle celle. Premi Canc per eliminare in un solo passaggio tutti gli oggetti selezionati.

Con due o più oggetti selezionati, la scheda **Formato forma** ha questi comandi nel gruppo **Disponi**:

- **Raggruppa** unisce le forme, le immagini e le linee selezionate in un unico oggetto. Si sposta e si ridimensiona come un solo elemento, e il file lo conserva come gruppo. Un grafico non può essere inserito in un gruppo.
- **Separa** scompone un gruppo selezionato. Ogni parte torna dove si trova sul foglio.
- **Allinea** allinea gli oggetti selezionati: **Allinea a sinistra**, **Allinea al centro**, **Allinea a destra**, **Allinea in alto**, **Allinea al centro verticalmente** e **Allinea in basso**. Si allineano al bordo o al centro dell'intero insieme. **Distribuisci orizzontalmente** e **Distribuisci verticalmente** distribuiscono in modo uniforme tre o più oggetti tra i due estremi.

Raggruppa, Separa e ciascuna scelta di allineamento sono un'azione unica che puoi annullare.

## Dare un nome

Con un oggetto selezionato, la casella del nome mostra il suo nome. Digita un nuovo nome e premi **Invio**. **Annulla** ripristina il nome precedente, e il nuovo nome viene salvato nel file.

## Eliminare un oggetto

Con l'oggetto selezionato, premi **Canc**. **Annulla** lo ripristina.

## Che cosa viene salvato

Tutto quanto descritto sopra viene salvato nel file in un formato che gli altri programmi per fogli di calcolo sanno leggere, quindi un grafico creato qui si apre allo stesso modo anche lì. Un grafico proveniente da un altro programma che contiene elementi che JeffCrunch non può conservare quando riscrive un grafico (barre di errore, un grafico tridimensionale) può comunque essere spostato, ridimensionato ed eliminato, ma i suoi elementi e i suoi dati non vengono modificati qui.

## Diagrammi di altri programmi

Un **diagramma** è un'immagine di riquadri e frecce, ad esempio un processo, creata da un altro programma. JeffCrunch mostra un diagramma a partire dall'immagine che il file conserva accanto ad esso. Fai clic per selezionarlo e trascinalo per spostarlo. Quando salvi il file, il diagramma viene riscritto esattamente com'era, così il programma che lo ha creato può ancora modificarlo.

JeffCrunch non può ancora modificare il contenuto di un diagramma, e un diagramma non può essere copiato né inserito in un gruppo. Se il file non conserva un'immagine di un diagramma, JeffCrunch mostra un semplice riquadro grigio che indica che si tratta di un diagramma.
