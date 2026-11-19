# Grafici, sparkline, immagini e forme

<!-- keywords: oggetto incorporato, inserisci oggetto, oggetto da intervallo, oggetto da grafico, oggetto da pagina, incolla come oggetto, elenco oggetti, modifica oggetto, pulsanti dei campi, pulsanti campi tabella pivot, raggruppa, separa, allinea, allinea a sinistra, distribuisci, seleziona oggetti, più forme, seleziona più elementi, riquadro di selezione, filtri grafico, nascondi serie, grafici consigliati, suggerimenti grafico, grafico, diagramma, colonne, barre, linea, torta, aree, dispersione, combinato, legenda, titolo, linee della griglia, etichette dati, dati grafico, tipo di grafico, stili, layout, sposta, ridimensiona, immagine, forma, oggetto, serie, linea di tendenza, tendenza, equazione, R al quadrato, secondo asse, sparkline, sparkline, asse delle date, vittoria/sconfitta, mini grafico, indicatori, nome, testo alternativo, logaritmico, scala logaritmica, salva come immagine, modello, crtx, testo del titolo, chiave di legenda, riquadro di selezione, riempimento forma, contorno forma, ruota, capovolgi, ritaglia, modifica testo, bordo immagine, inserisci immagine, icone, icona, wmf, emf, stili forma, stili, inserisci forma, casella di testo, forme, freccia, ovale, rettangolo, stella, diagramma, diagrammi, immagini online, immagini gratuite, openverse, wikimedia commons, licenza, crediti, testo artistico, wordart, deformazione, comprimi immagini, rimpicciolisci immagini, dimensione file, ppi, elimina aree ritagliate, codice a barre, codice QR, collega forma a cella, la forma mostra una cella, la casella di testo mostra una cella, valore della cella in una forma -->

Un grafico è un'immagine di numeri che resta legata a essi: se cambi un numero, il grafico cambia con lui. Immagini e forme si posizionano sopra le celle allo stesso modo, ma non sono legate ad alcun numero.

## Creare un grafico

1. Seleziona i numeri da rappresentare. Se la prima riga contiene nomi e la prima colonna contiene etichette, seleziona anche quelle. JeffCrunch controlla la prima riga e la prima colonna e le usa come nomi e come etichette quando sono parole (o date, per le etichette).
2. Nella scheda **Inserisci**, nel gruppo **Grafici**, scegli un tipo: **Colonna**, **Barra**, **Linea**, **Torta**, **Area**, **Dispersione** o **Combinato**. La piccola freccia accanto a un tipo mostra le sue varianti, come le colonne impilate, una linea con indicatori, oppure un grafico a dispersione di punti, di punti collegati o di linee smussate.
3. Per vedere subito tutti i tipi, scegli **Altri grafici**. L'elenco a sinistra mostra ogni tipo disegnato con i tuoi dati. **Scambia righe e colonne** inverte la disposizione delle serie.

Non sai quale tipo scegliere? Seleziona le celle e scegli **Grafici consigliati** nella scheda **Inserisci**. JeffCrunch esamina le celle ed elenca alcuni tipi adatti, dal più indicato. Ad esempio, le date disposte in verticale suggeriscono una linea, mentre una sola colonna di pochi numeri positivi con etichette suggerisce una torta. Ogni tipo viene disegnato con i tuoi dati. Scegline uno e fai clic su **OK**.

Se selezioni una sola cella, JeffCrunch usa il blocco di celle attorno a essa che contiene dati.

Le serie scorrono lungo le colonne quando la selezione ha almeno tante righe quante colonne, e lungo le righe quando ha più colonne che righe. Una torta contiene una sola serie. Un grafico a dispersione prende i valori orizzontali dalla prima colonna.

Esempio: selezioni A1:C4, dove A2:A4 contengono Q1, Q2 e Q3, B1 e C1 contengono Est e Ovest, e il resto sono numeri. Il grafico ha due serie, Est e Ovest, e tre gruppi di colonne, da Q1 a Q3.

## Nascondere temporaneamente una serie

Seleziona un grafico. Accanto ad esso compaiono tre piccoli pulsanti. Quello con l'imbuto è **Filtri grafico**. Mostra le serie del grafico, ciascuna con una casella di controllo. Deseleziona una casella per escludere quella serie dal grafico. Selezionala di nuovo per riportarla nello stesso punto. Funziona anche **Annulla**. Un grafico conserva sempre almeno una serie. Una serie esclusa resta nel file quando salvi e rimane nascosta quando apri il file, quindi non si perde nulla. Selezionala di nuovo per riportarla. (Un grafico a ciambella mostra tutte le serie quando viene aperto.)

## Spostare e ridimensionare

Fai clic su un grafico, un'immagine o una forma per selezionarlo. Trascinalo per spostarlo, oppure trascina uno degli otto quadratini sul bordo per ridimensionarlo.

- Tieni premuto **Alt** mentre trascini per allineare un bordo al bordo di una cella.
- Tieni premuto **Maiusc** mentre trascini un angolo per mantenere le proporzioni.
- I tasti freccia spostano l'oggetto di alcuni pixel alla volta. **Ctrl** con una freccia lo sposta di un pixel, **Maiusc** con una freccia lo ingrandisce o lo rimpicciolisce, e **Alt** con una freccia lo porta al bordo successivo di una cella.
- **Esc** mentre trascini lo riporta alla posizione iniziale.

Ogni spostamento o ridimensionamento è un passaggio che puoi annullare. Fai clic destro sull'oggetto e scegli **Proprietà** per digitare il **Nome** (ogni oggetto di un foglio deve avere il proprio) e il **Testo alternativo** (una breve frase che spiega a chi non può vedere l'oggetto che cosa mostra), per digitarne le dimensioni e per scegliere cosa succede quando inserisci, elimini o ridimensioni celle: spostarsi e ridimensionarsi con le celle, spostarsi con esse ma mantenere le dimensioni, oppure restare dove si trova. La scelta viene salvata nel file. **Stampa questo oggetto**, nella stessa finestra, disattiva l'oggetto nelle pagine stampate (resta visibile sullo schermo); anche il file conserva questa scelta.

## Cambiare ciò che mostra un grafico

Quando un grafico è selezionato, accanto ad esso compaiono due pulsanti e sulla barra multifunzione appare la scheda **Grafico**.

- Il pulsante **+** (e **Elementi grafico** nella scheda) attiva o disattiva il titolo del grafico, gli assi, i titoli degli assi, le linee della griglia, la legenda e le etichette dati, e indica dove si trovano la legenda e le etichette.
- Il pulsante a pennello (e **Colori** e **Stili** nella scheda) assegna al grafico uno dei set di colori e uno degli stili di JeffCrunch. **Layout** imposta più elementi contemporaneamente.
- **Dati grafico** elenca le serie del grafico. Puoi aggiungerle, modificarle, eliminarle e riordinarle, cambiare le celle che ciascuna legge, impostare le celle con le etichette, scambiare righe e colonne, e scegliere come trattare celle vuote e righe e colonne nascoste.
- **Pulsanti dei campi** (nel menu **+**, solo per un grafico pivot) mostra piccoli pulsanti con i nomi dei campi della tabella pivot seguita dal grafico: i filtri del report sotto il titolo, e le righe, i valori e le colonne lungo il fondo. Attiva o disattiva ciascun tipo, oppure scegli **Mostra tutto** o **Nascondi tutto**. Un nuovo grafico pivot li mostra.
- **Barre di errore** (nel menu **+**, per un grafico a colonne, barre, linee, aree o dispersione) disegna una piccola linea sopra e sotto ogni punto per mostrare quanto il valore reale potrebbe discostarsi. Scegli **Errore standard**, **Percentuale (5%)**, **Deviazione standard** o **Valore fisso (1)**. Scegli **Personalizzata** per usare importi propri: digita le celle che contengono gli importi sopra ogni punto (ad esempio `=Foglio1!$D$2:$D$4`), poi le celle per gli importi sotto. Lascia vuota una casella per disegnare le barre da un solo lato. Le barre cambiano quando cambiano quelle celle. Un file che contiene già importi propri per ogni punto li conserva.
- **Linee** (nel menu **+**, per un grafico a linee) contiene **Linee di caduta**, che scendono dal punto più alto di ogni categoria fino all'asse, **Linee max-min**, che uniscono il punto più alto e quello più basso di ogni categoria, e **Barre di salita/discesa**, che sono riquadri tra la prima e l'ultima serie di ogni categoria. Una barra di salita indica che l'ultima serie è più alta. Le linee max-min e le barre di salita/discesa richiedono due serie o più. I grafici ad area hanno le linee di caduta.
- **Tabella dati** (nel menu **+**, per un grafico a colonne, linee o aree) mette sotto il grafico una griglia con i numeri, con una colonna per ogni categoria e una riga per ogni serie. Il suo elenco attiva o disattiva la tabella e imposta le linee tra le righe, le linee tra le colonne, il contorno e le chiavi di legenda (il colore di ciascuna serie accanto al suo nome).
- **Linea di tendenza** (nel menu **+**) traccia una linea adattata ai numeri di ogni serie, così vedi dove stanno andando. Scegli **Lineare** per una linea retta, **Esponenziale** per numeri che crescono di una percentuale costante, **Logaritmica** per numeri che salgono rapidamente e poi si stabilizzano, **Polinomiale** per una curva con delle ondulazioni, **Potenza**, oppure **Media mobile**, che traccia la media degli ultimi numeri. **Mostra equazione** scrive sul grafico la formula della linea, e **Mostra R al quadrato** scrive quanto bene la linea si adatta (1 indica un adattamento perfetto; 0 nessun adattamento). La linea di tendenza ha una voce propria nella legenda.
- Quando una serie è disegnata su un secondo asse verticale, i menu **Assi** e **Titoli assi** hanno le voci **Secondo orizzontale** e **Secondo verticale** per essa. Il menu **Legenda** contiene **Sopra il grafico**, che colloca la legenda sull'area del grafico invece che accanto ad essa.
- **Etichette dati** contiene anche **Etichetta contiene** (il valore, il nome della categoria, il nome della serie, la percentuale e la chiave di legenda, cioè un piccolo quadrato del colore della serie, in qualsiasi combinazione), **Separatore** (ciò che sta tra questi elementi: virgola, punto e virgola, punto, a capo o spazio) e **Formato numero…** (un codice di formato come 0,0 per i numeri nelle etichette; lascialo vuoto per mostrare i numeri così come sono). **Valore da celle…** mostra come etichette il testo di alcune celle, una cella per ogni punto. Digita le celle, ad esempio =Foglio1!$D$2:$D$4. Le etichette cambiano quando cambiano le celle. Lascialo vuoto per eliminarle. Se disattivi tutti i contenuti, le etichette scompaiono.
- **Formato asse orizzontale…** e **Formato asse verticale…** nella scheda **Grafico** aprono una finestra per un singolo asse. Imposta i valori minimo e massimo e le unità principale e secondaria (ciascuna automatica o un numero che digiti), se i valori scorrono in ordine inverso, le unità di visualizzazione (le etichette mostrano i numeri divisi, ad esempio per 1.000 per le migliaia, quindi 30.000 appare come 30; seleziona **Mostra il nome delle unità nel grafico** per stampare il nome, come Migliaia, insieme al titolo dell'asse), se la scala è logaritmica (ogni segno è una base moltiplicata per il precedente, come 1, 10, 100 e 1000, adatta a numeri di grandezza molto diversa che devono essere tutti maggiori di 0), il punto in cui l'altro asse interseca questo, il formato numerico delle sue etichette, i segni di graduazione, la posizione delle etichette e di quanto sono ruotate. Solo un asse numerico ha limiti e unità.
- **Sposta grafico…** nella scheda **Grafico** chiede dove deve andare il grafico. Scegli **Nuovo foglio** e digita un nome, e il grafico ottiene un foglio grafico proprio: un foglio che contiene solo il grafico, senza celle. Scegli **Oggetto in** e seleziona un foglio di lavoro, e il grafico viene collocato su quel foglio. Un grafico su un foglio grafico può essere spostato solo su un foglio di lavoro, e il foglio grafico da cui esce viene eliminato. Il grafico continua a leggere le stesse celle. Su un foglio di lavoro mantiene le sue dimensioni e la sua posizione sul foglio, anche dove righe e colonne sono più alte o più larghe, e si sposta e ridimensiona con le celle come prima. Non vengono proposti i fogli nascosti. È un unico **Annulla**, anche dopo aver salvato, a meno che con l'operazione non sia stato eliminato un foglio. Un file in formato `.ods` non può contenere un foglio grafico, quindi salva un foglio grafico in formato `.xlsx`.
- **Titolo grafico** e **Titoli assi** (nel menu **+**) hanno anche **Testo del titolo…**. Digita le parole che vuoi. Oppure digita un segno di uguale e una cella, come **=Foglio1!A1**, e il titolo mostra ciò che contiene quella cella. Quando la cella cambia, cambia anche il titolo. Il file conserva il collegamento.
- **Salva come immagine…** nella scheda **Grafico** (e nel menu del clic destro) salva il grafico selezionato come file immagine **PNG** o **JPEG**, di dimensioni doppie rispetto al foglio, così resta nitido.
- **Salva come modello…** e **Applica modello…** (nella scheda **Grafico**, nel gruppo **Tipo**) conservano l'aspetto di un grafico in un file **.crtx** e lo assegnano a un altro grafico. L'aspetto comprende il tipo, i colori, le impostazioni di titolo e legenda, gli assi e i caratteri. Il grafico a cui lo applichi mantiene i propri numeri e le celle che legge. Un solo **Annulla** elimina di nuovo l'aspetto.
- **Tipo di grafico** cambia il tipo per l'intero grafico, oppure per ciascuna serie: colonne con una linea sopra, oppure una serie su un secondo asse verticale.

Un riferimento a celle si scrive come **Foglio1!$B$2:$B$9**, con il nome del foglio tra apici singoli quando contiene spazi.

## Sparkline

Uno sparkline è un piccolo grafico che sta dentro una sola cella. Mostra l'andamento di una riga o di una colonna di numeri accanto ad essi.

1. Seleziona i numeri. Per uno sparkline per ogni riga, seleziona più righe di numeri.
2. Nella scheda **Inserisci**, nel gruppo **Sparkline**, scegli **Linea**, **Colonna** o **Vittoria/Sconfitta**. **Linea** unisce i numeri con una linea. **Colonna** disegna una barra per ogni numero. **Vittoria/Sconfitta** disegna una barra verso l'alto per ogni numero maggiore di zero e una barra verso il basso per ogni numero minore, tutte della stessa dimensione, adatto a risultati che sono solo positivi o negativi.
3. Nella finestra **Crea sparkline**, verifica l'**Intervallo dati** (le celle che lo sparkline legge) e digita l'**Intervallo di posizione** (le celle in cui andranno). Scegli una cella di posizione per ogni riga di dati, oppure per ogni colonna di dati. Se scegli una sola cella, lo sparkline legge tutti i dati.

Gli sparkline creati in un'unica operazione formano un gruppo e condividono lo stesso aspetto. Fai clic su una cella con uno sparkline e compare la scheda **Sparkline**.

- **Modifica dati** cambia i dati e la posizione dell'intero gruppo, oppure i dati dello sparkline su cui hai fatto clic. Indica anche cosa fare con le celle vuote (lasciare uno spazio, trattarle come zero, oppure unire i punti attraverso di esse) e se leggere le celle nelle righe e colonne nascoste.
- **Tipo** passa da linea a colonna e vittoria/sconfitta.
- **Mostra** contrassegna i punti: il **Punto massimo**, il **Punto minimo**, i **Punti negativi**, il **Primo punto**, l'**Ultimo punto» e, per una linea, gli **Indicatori** su ogni punto.
- **Stili** assegna al gruppo un insieme di colori. **Colore sparkline** imposta il colore della linea o delle barre e lo **Spessore** (quanto è spessa la linea, in punti). **Colore indicatore** imposta il colore di ogni tipo di punto contrassegnato.
- **Asse** mostra una linea allo zero, traccia i numeri da destra a sinistra e imposta i valori minimo e massimo dell'asse verticale. **Automatico per ogni sparkline** adatta ciascuno ai propri numeri. **Uguale per tutti gli sparkline** assegna all'intero gruppo una sola scala, così puoi confrontarli. **Valore personalizzato** ti permette di digitare un numero. **Asse delle date…** distanzia i punti in base alle date: digita le celle che contengono la data di ogni punto, e un punto con una data successiva si trova più avanti. Lascialo vuoto per distribuire di nuovo i punti in modo uniforme.
- **Raggruppa** mette gli sparkline selezionati in un unico gruppo. **Separa** assegna a ciascuno un aspetto proprio. **Cancella** elimina gli sparkline selezionati, oppure i loro interi gruppi.

Uno sparkline segue le proprie celle: se inserisci una riga sopra i dati, continua a leggere gli stessi numeri. Ogni modifica è un passaggio che puoi annullare. Gli sparkline sono salvati nel file nella forma che leggono gli altri programmi per fogli di calcolo.

## Inserire un'immagine, una forma o una casella di testo

Nella scheda **Inserisci**, nel gruppo **Illustrazioni**, **Immagine** apre una finestra di selezione file. Scegli un file immagine (PNG, JPEG, GIF, BMP, TIFF o SVG) e questo viene collocato sul foglio nella cella selezionata. Un'immagine più larga di 480 pixel viene rimpicciolita per adattarsi e mantiene la sua forma. La cartella di lavoro conserva l'immagine stessa. Un'immagine di un tipo che la cartella di lavoro non può contenere così com'è (un SVG, ad esempio) viene conservata come PNG, cioè un'immagine semplice fatta di punti. Un file che non è un'immagine, oppure che supera i 16 MB, viene rifiutato con un messaggio. Un file WMF o EMF (i tipi di immagine che alcuni programmi più vecchi creano con linee e forme) viene disegnato e conservato anche come PNG.

**Icone** mostra la stessa finestra di piccole immagini a linee che hanno JeffPub e JeffDeck, con una casella per cercarle per nome. Scegline una o più, e ciascuna viene collocata sul foglio come immagine, larga e alta 48 punti. Viene disegnata in grigio scuro a una dimensione che resta nitida quando la ingrandisci. Ciascuna è un singolo passaggio che puoi annullare.

**Codice a barre** apre un breve menu con due scelte. **Codice a barre** apre una finestra per creare un solo codice a barre (**Codici a barre da celle**, più avanti, ne crea molti): ISBN per libri (con o senza prezzo), EAN-13, UPC-A, EAN-8, Code 128, Code 39 oppure un codice QR, che può contenere qualsiasi testo, come un indirizzo web. Scegli un **Tipo**, digita il testo o le cifre, e la finestra mostra il codice a barre mentre digiti e indica in rosso cosa non va se non può essere creato. Fai clic su **Inserisci** e il codice viene collocato sul foglio come immagine, con un passaggio che puoi annullare. Spostalo e ridimensionalo come qualsiasi immagine. Per modificarlo, selezionalo e fai di nuovo clic su **Codice a barre**, cambia il testo e fai clic su **Aggiorna**; il nuovo codice prende il posto e la larghezza di quello vecchio. Quando salvi, viene conservato come immagine con un SVG che resta nitido, così ogni programma lo mostra, e JeffCrunch conserva le sue impostazioni nel file, quindi resta un codice a barre modificabile quando riapri la cartella di lavoro in JeffCrunch.

**Codice a barre > Codici a barre da celle** (nello stesso menu **Codice a barre**) crea un codice a barre per ogni cella compilata tra quelle selezionate, per un foglio di etichette. Seleziona le celle, fai clic su questa voce, poi scegli il tipo e l'aspetto nella stessa finestra. Quello che digiti lì è solo un esempio, perché il testo di ogni cella diventa il suo codice a barre. I codici a barre vanno in una colonna subito a destra delle celle selezionate, uno sotto l'altro, come un unico passaggio che puoi annullare. Una cella il cui testo il codice non può contenere (una lettera in un EAN-13, ad esempio) viene indicata in un messaggio e non riceve alcun codice a barre. Al massimo 500 celle vengono elaborate per volta; seleziona meno celle se il programma segnala che sono troppe. Ogni codice a barre ricorda la cella da cui proviene e la segue: quando cambi quella cella, il codice viene creato di nuovo dal nuovo testo, nello stesso punto e alla stessa dimensione, come passaggio proprio che puoi annullare. Il collegamento si sposta con la cella quando inserisci o elimini righe o colonne. Se elimini la cella, il codice a barre conserva l'ultimo testo che aveva. Quando salvi, l'immagine nel file viene creata dal testo attuale della cella, così anche gli altri programmi mostrano il valore corrente.

**Forme** apre un elenco: rettangolo, rettangolo con angoli arrotondati, ovale, triangolo, rombo, frecce, una stella, un cuore, una nuvola, un fumetto e una linea. Scegline una e viene collocata sul foglio in azzurro chiaro con un bordo più scuro. **Casella di testo**, nel gruppo **Testo**, inserisce una casella vuota senza riempimento e senza bordo. Fai clic su **Disegna casella di testo** (il pulsante principale, o la prima scelta del suo menu), poi trascina sul foglio per disegnare la casella. Un trascinamento crea una casella della dimensione che trascini, ma mai più piccola di 18 punti (0,6 cm) per lato, e un clic di meno di 4 punti dà la dimensione abituale. Tieni premuto **Maiusc** per mantenere la casella quadrata. Il puntatore è una croce finché non rilasci il pulsante; **Esc**, o qualsiasi altro comando, rimette a posto lo strumento. La casella è un passaggio che puoi annullare.

Per cambiare la direzione del testo di una casella, selezionala e fai clic su **Direzione testo** nella scheda **Formato forma**: **Orizzontale**, **Ruota tutto il testo di 90°** oppure **Ruota tutto il testo di 270°**. Il testo viene digitato già ruotato, come viene mostrato, e una cartella di lavoro salvata in formato .xlsx o .ods conserva la direzione.

Con una forma selezionata, **Stili** nella scheda **Formato forma** le assegna un aspetto già pronto: un riempimento e un contorno abbinati, come azzurro chiaro con un bordo blu più scuro. Gli ultimi due mantengono solo il contorno oppure solo il riempimento. Un clic è un passaggio che puoi annullare. **Riempimento forma** e **Contorno forma** cambiano ciascuna parte separatamente.

## Trovare immagini gratuite online

**Immagini online**, nel gruppo **Illustrazioni** della scheda **Inserisci**, trova immagini gratuite. Provengono da due librerie aperte: Openverse, che cerca in molte raccolte, e Wikimedia Commons, la libreria delle immagini di Wikipedia.

1. Fai clic su **Immagini online…**. Si apre un riquadro sulla destra.
2. Scegli una libreria, digita ciò che cerchi e fai clic su **Cerca** (oppure premi Invio).
3. Fai clic su un'immagine per vedere chi l'ha realizzata e quale licenza ha. Una **licenza** indica come puoi usare un'immagine. Fai clic sul nome della licenza per leggerla.
4. Fai clic su **Inserisci** (oppure fai doppio clic sull'immagine).

**Cosa invia JeffCrunch.** Le parole della ricerca vanno alla libreria che scegli, e solo quando fai clic su **Cerca** o premi Invio. JeffCrunch chiede poi alla libreria le piccole anteprime di ciò che ha trovato, e l'unica immagine che inserisci. Come ogni pagina web, la libreria può vedere l'indirizzo internet del tuo computer, e le viene comunicato che è JeffCrunch a chiedere. Nulla della tua cartella di lavoro viene inviato.

L'immagine viene collocata sul foglio come un'immagine da file, e la cartella di lavoro la conserva. Il suo titolo viene salvato come **testo alternativo**. La maggior parte delle licenze chiede di citare l'autore. Quando **Aggiungi i crediti sotto l'immagine** è selezionato, JeffCrunch aggiunge sotto l'immagine una piccola casella di testo con il titolo, l'autore e la licenza. Puoi spostarla, modificarla o eliminarla. L'immagine e i suoi crediti vengono inseriti con un unico passaggio, quindi **Annulla** li rimuove entrambi. Verifica che la licenza sia compatibile con il modo in cui userai l'immagine.

## Aggiungere testo artistico

**Testo artistico** sono parole decorate: lettere con colore, contorno, ombra o forma curva.

1. Fai clic su **Inserisci > Testo > Testo artistico**. Si apre una raccolta di stili.
2. Fai clic su uno stile. Le parole "Inserisci qui il tuo testo" vengono collocate sul foglio in quello stile.
3. Per cambiare le parole, seleziona il testo artistico e fai clic su **Modifica testo…** nella scheda **Formato forma**. Una finestra chiede le parole, il carattere, la dimensione, il grassetto e il corsivo. Una dimensione maggiore rende più grande il testo artistico.
4. Nella scheda **Formato forma**, **Stili testo artistico** dà alle lettere un altro stile, e **Deformazione testo** piega le parole in una forma, come un'onda o un arco. "Normale" le mantiene dritte.

Il testo artistico è una forma del disegno del foglio, con la sua deformazione, il riempimento e il contorno, quindi mantiene il suo aspetto quando salvi la cartella di lavoro in formato .xlsx e la riapri, in JeffCrunch o in un altro programma. Ogni modifica è un passaggio che puoi annullare. Una cartella di lavoro salvata in formato .ods conserva anche immagini, forme, caselle di testo, gruppi, linee e testo artistico. Il testo artistico viene salvato come la forma di testo deformato che OpenDocument prevede, con le sue parole, il carattere, il riempimento, il contorno e la deformazione, e torna a essere testo artistico quando riapri il file .ods. Alcune deformazioni per cui OpenDocument non ha una forma (Onda 2, Onda 4, e le forme inferiore e superiore di Rigonfia e Sgonfia) vengono salvate come parole dritte con le stesse lettere e lo stesso stile. Anche i grafici sono conservati in un file .ods: vedi più avanti.

## Rimpicciolire le immagini

Un'immagine da fotocamera può essere molto più grande di come appare sul foglio, e rende il file pesante. Fai clic su un'immagine, poi su **Comprimi immagini…** nel gruppo **Regola** della scheda **Formato immagine**. Scegli dove verrà usata la cartella di lavoro: **Stampa commerciale (300 ppi)**, **Stampa desktop (220 ppi)** oppure **Web (96 ppi)**. Lascia selezionato **Applica a tutte le immagini di questo foglio** per rimpicciolire tutte le immagini del foglio, oppure deselezionalo per rimpicciolire solo l'immagine selezionata. **Elimina le aree ritagliate delle immagini** elimina anche le parti nascoste che hai ritagliato, e il ritaglio scompare. Un'immagine viene sostituita solo quando la nuova è più piccola. Dimensioni, posizione, nome e testo alternativo restano come erano. Annulla (Ctrl+Z) restituisce le immagini originali, e la cartella di lavoro salvata contiene solo le immagini più piccole. Le immagini su altri fogli non vengono modificate: vai su ciascun foglio e ripeti l'operazione. Un'immagine che è un disegno (SVG, EMF o WMF), oppure un'animazione, resta invariata.

Ogni operazione è un singolo passaggio che puoi annullare. Il nuovo oggetto risulta selezionato, quindi puoi trascinarlo, ridimensionarlo, portarlo avanti o indietro, assegnargli un nome e un testo alternativo in **Proprietà**, oppure premere **Elimina**.

## Mostrare il testo di una cella in una forma o in una casella di testo

Una forma o una casella di testo può mostrare ciò che mostra una cella, e seguirla. Seleziona la forma, fai clic sulla **barra della formula**, digita un segno di uguale e una cella, poi premi **Invio**.

- `=B2` collega la forma a B2 di questo foglio. `=Foglio2!B3` la collega a una cella di un altro foglio. `=Totale` la collega a un nome che rappresenta una cella.
- La forma mostra allora il testo della cella come lo mostra la cella, con il suo formato numerico. Quando cambi la cella, il suo formato o una formula da cui dipende, anche la forma cambia. Il testo più lungo di quello che una cella può contenere (32.767 caratteri) viene troncato. Tutte le forme collegate di una cartella di lavoro insieme mostrano al massimo 4 MB di testo: oltre questo limite, una forma mostra il suo testo troncato.
- Con la forma selezionata, la barra della formula mostra il suo collegamento, come `=Foglio1!$B$2`. Cancella la barra della formula e premi **Invio** per rimuovere il collegamento. La forma conserva allora le parole che mostra.
- Digitare parole nella forma rimuove anche il collegamento.
- Un collegamento riguarda una sola cella. Un intervallo come `=A1:B2`, un elenco, un calcolo o un nome che non è una cella viene rifiutato con un messaggio, e la forma conserva il suo collegamento.
- Se la riga o la colonna della cella viene eliminata, oppure il suo foglio, la forma mostra `#RIF!` e conserva il collegamento, come farebbe una formula. Annulla riporta la cella.
- Ogni collegamento, e ogni volta che lo rimuovi, è un passaggio che puoi annullare.

Il collegamento viene salvato in un file .xlsx insieme al testo che la cella mostrava, quindi un programma che non segue i collegamenti mostra comunque le parole, e JeffCrunch apre il file con il collegamento funzionante. OpenDocument non ha un modo per collegare una casella di testo a una cella, quindi una cartella di lavoro salvata in formato .ods conserva le parole che la forma mostra in quel momento e non il collegamento.

## Cambiare l'aspetto di un'immagine, di una forma o di una casella di testo

Seleziona un'immagine e compare la scheda **Formato immagine**. Seleziona una forma, una linea o una casella di testo e compare la scheda **Formato forma**. Ogni modifica è un passaggio che puoi annullare, e l'oggetto mantiene il suo nome, il suo testo alternativo, la sua posizione e il suo ordine rispetto agli altri oggetti.

- **Riempimento forma** e **Contorno forma** scelgono un colore, oppure nessuno. **Stile linea** imposta lo spessore (quanto è spessa la linea) e i tratteggi (continuo, tratteggiato o punteggiato). Un'immagine ha **Bordo immagine**, che è il suo contorno.
- **Ruota** fa girare l'oggetto di un quarto di giro a destra o a sinistra, oppure lo capovolge da lato a lato o dall'alto in basso.
- **Ritaglia**, per un'immagine, la taglia in una finestra di forma: quadrato, 4 di larghezza per 3 di altezza, 3 di larghezza per 4 di altezza, 16 per 9 o 9 per 16. Il taglio viene fatto in modo uniforme dal centro di ciò che si vede, e l'immagine viene resa grande quanto ciò che resta. **Mostra l'immagine intera** ripristina ciò che è stato tagliato. **Riempi** mantiene la cornice e taglia l'immagine in modo da coprirla interamente. **Adatta** mostra l'intera immagine, e la cornice assume la forma dell'immagine all'interno della vecchia. **Ritaglia in forma** (la galleria accanto a Ritaglia) taglia l'immagine in una forma come un ovale, una stella o un rettangolo con angoli arrotondati; la forma viene salvata con l'immagine. Il file dell'immagine non viene modificato: la parte tagliata viene solo nascosta.
- **Modifica testo** serve per forme e caselle di testo. Digita le parole e scegli come si dispongono in orizzontale e in verticale, la dimensione, il grassetto o il normale, e se vanno a capo nella forma oppure restano su una riga. Per il testo artistico chiede invece le parole, il carattere, la dimensione, il grassetto e il corsivo.
- **Riquadro di selezione** apre un elenco di tutti gli oggetti del foglio per nome, con quello in primo piano per primo. Fai clic su un nome per selezionare quell'oggetto, utile quando gli oggetti si nascondono a vicenda. **Porta avanti** e **Porta indietro** lo spostano nell'elenco.
- **Proprietà** imposta le dimensioni, il nome, il testo alternativo e ciò che succede all'oggetto quando le celle vengono spostate o ingrandite.

## Tagliare, copiare e incollare un oggetto

Un'immagine o una forma si copia e si incolla come un grafico. Una forma che usa qualcosa di diverso da un'immagine della cartella di lavoro, come un collegamento a un file, non può essere copiata, e JeffCrunch ti spiega il motivo.

Copia un grafico e incollalo nella stessa cartella di lavoro, su qualsiasi foglio: la copia legge le stesse celle della prima. Incollalo in un'altra cartella di lavoro e mantiene i numeri che mostrava, ma non legge più celle, perché le celle che leggerebbe lì non sono quelle da cui è stato creato.

## Selezionare più oggetti, raggrupparli e allinearli

Fai clic su un oggetto, poi tieni premuto Maiusc o Ctrl e fai clic su un altro per selezionare anche quello. Fai di nuovo clic su un oggetto selezionato tenendo premuto Maiusc o Ctrl per deselezionarlo. Gli oggetti selezionati hanno ciascuno una cornice tratteggiata. Puoi anche scegliere **Seleziona oggetti** nella scheda **Formato forma** e trascinare un riquadro su celle vuote: ogni oggetto che sta interamente nel riquadro viene selezionato. Premi Esc, oppure scegli di nuovo **Seleziona oggetti**, per tornare a selezionare celle. Premi Canc per eliminare in un solo passaggio tutti gli oggetti selezionati.

Con due o più oggetti selezionati, la scheda **Formato forma** ha queste voci nel gruppo **Disponi**:

- **Raggruppa** unisce le forme, le immagini e le linee selezionate in un unico oggetto. Si sposta e si ridimensiona come un tutt'uno, e il file lo conserva come gruppo. Un grafico non può essere messo in un gruppo.
- **Separa** scompone un gruppo selezionato. Ogni parte torna dov'è sul foglio.
- **Allinea** allinea gli oggetti selezionati: **Allinea a sinistra**, **Allinea al centro**, **Allinea a destra**, **Allinea in alto**, **Allinea al centro verticalmente** e **Allinea in basso**. Si allineano al bordo o al centro dell'insieme. **Distribuisci orizzontalmente** e **Distribuisci verticalmente** distribuiscono in modo uniforme tre o più oggetti tra i due estremi.

Raggruppa, Separa e ciascuna scelta di Allinea sono un unico passaggio che puoi annullare.

## Dare un nome

Con un oggetto selezionato, la casella del nome mostra il suo nome. Digita un nuovo nome lì e premi **Invio**. Annulla ripristina il vecchio nome, e il nuovo nome viene salvato nel file.

## Eliminare un oggetto

Premi **Canc** con l'oggetto selezionato. Annulla lo riporta.

## Cosa viene salvato

Tutto quanto descritto sopra viene salvato nel file nella forma che leggono gli altri programmi per fogli di calcolo, quindi un grafico creato qui si apre allo stesso modo altrove. Un grafico proveniente da un altro programma che contiene qualcosa che JeffCrunch non può conservare quando riscrive un grafico (barre di errore, un grafico tridimensionale) può comunque essere spostato, ridimensionato ed eliminato, ma i suoi elementi e i suoi dati non vengono modificati qui.

## Inserire un oggetto

Un **oggetto** contiene un proprio file all'interno della cartella di lavoro, così l'app che lo ha creato può riaprirlo. Mostra un'immagine di se stesso, ma non è un'immagine: resta un oggetto quando lo sposti, lo ridimensioni, lo copi o salvi la cartella di lavoro.

1. Fai clic su **Inserisci**, poi su **Oggetto**, e scegli **Da intervallo JeffCrunch**, **Da grafico JeffCrunch** oppure **Da pagina JeffWrite**.
2. Scegli il file e, se la finestra lo chiede, l'intervallo o il numero di pagina. Se hai copiato qualcosa nell'altra app, la finestra propone di usarlo.
3. L'oggetto viene inserito nella cella selezionata. Trascinalo per spostarlo e trascina una maniglia per ridimensionarlo. Un intervallo, un grafico o una pagina che ridimensioni viene disegnato di nuovo alla nuova dimensione, così resta nitido. Premi **Canc** per eliminarlo. Ciascuno è un unico passaggio di Annulla.

Per inserire ciò che hai copiato in un'altra app, fai clic sulla freccia sotto **Incolla** e scegli **Come oggetto**.

**Fai doppio clic** su un oggetto per aprirlo nell'app che lo ha creato. Salvalo lì, e l'oggetto qui mostra la modifica come un passaggio di Annulla. Una cella in cui stavi scrivendo resta invariata. Una copia incollata è un oggetto a sé: modificarne una non modifica l'altra. Lo stesso vale per un foglio che copi: i suoi oggetti sono copie, e la copia del foglio li conserva.

**File**, poi **Informazioni**, poi **Oggetti** elenca tutti gli oggetti della cartella di lavoro. Seleziona uno per aprirlo, trasformarlo in una semplice immagine oppure eliminarlo.

Un oggetto creato in un altro programma viene mostrato come l'immagine che il suo file conserva, qualunque sia il tipo di immagine. Puoi selezionarlo, spostarlo, ridimensionarlo, allinearlo con altri oggetti ed eliminarlo, e si sposta con le celle come i tuoi oggetti. Non puoi modificarlo qui, metterlo in un gruppo o cambiare il suo posto nell'ordine degli oggetti. Viene salvato esattamente come lo aveva il file, con solo la sua posizione cambiata se lo hai spostato. Lo stesso vale per un oggetto contenuto in un file .ods. Salvati in formato .xlsx o .ods, i tuoi oggetti sono nella forma standard che leggono gli altri programmi. Quando si scrive un file .ods per un oggetto la cui immagine non può essere disegnata, l'oggetto viene conservato senza immagine.

## Diagrammi di altri programmi

Un **diagramma** è un'immagine di riquadri e frecce, come un processo, creata da un altro programma. JeffCrunch mostra un diagramma dall'immagine che il file conserva accanto ad esso. Fai clic per selezionarlo, e trascinalo per spostarlo. Quando salvi il file, il diagramma viene riscritto esattamente com'era, così il programma che l'ha creato può ancora modificarlo.

JeffCrunch non può ancora cambiare il contenuto di un diagramma, e un diagramma non può essere copiato né messo in un gruppo. Se il file non conserva un'immagine di un diagramma, JeffCrunch mostra un semplice riquadro grigio che indica che si tratta di un diagramma.

## Grafici nei file .ods

Una cartella di lavoro salvata in formato .ods conserva i suoi grafici: colonne, barre, linee, aree, torte, ciambelle e grafici a dispersione, ciascuno con le sue serie e le celle che legge (su questo foglio o su un altro), categorie, titolo, legenda, assi (titoli, valore minimo e massimo, formato numerico), etichette dati, colori e posizione. Un file .ods creato da un altro programma si apre con i suoi grafici, disegnati a partire dalle celle che leggono. Ciò che un grafico .ods non ha modo di rappresentare viene escluso e segnalato quando salvi: linee di tendenza, barre di errore, assi secondari e gli stili particolari di un singolo punto. I grafici di tipi che JeffCrunch non disegna (grafici azionari e radar) si aprono come colonne dei loro valori.
