# Grafici, sparkline, immagini e forme

<!-- keywords: pulsanti dei campi, pulsanti dei campi del grafico pivot, raggruppa, separa, allinea, allinea a sinistra, distribuisci, seleziona oggetti, più forme, seleziona più oggetti, casella di selezione, filtri del grafico, nascondi serie, grafici consigliati, suggerimenti grafico, grafico, diagramma, istogramma, colonne, barre, linee, torta, aree, dispersione, combinato, legenda, titolo, linee della griglia, etichette dati, dati del grafico, tipo di grafico, aspetto, layout, sposta, ridimensiona, immagine, forma, oggetto, serie, linea di tendenza, tendenza, equazione, R al quadrato, secondo asse, sparkline, asse delle date, vittoria/sconfitta, minigrafico, indicatori, nome, testo alternativo, scala logaritmica, salva come immagine, modello, crtx, testo del titolo, chiave della legenda, riquadro di selezione, riempimento forma, contorno forma, ruota, capovolgi, ritaglia, modifica testo, bordo immagine, inserisci immagine, icone, icona, wmf, emf, stili forma, stili, inserisci forma, casella di testo, forme, freccia, ovale, rettangolo, stella, diagramma, diagrammi, immagini online, immagini gratuite, openverse, wikimedia commons, licenza, crediti, testo artistico, wordart, deformazione, comprimi immagini, riduci dimensioni immagini, dimensioni file, ppi, elimina aree ritagliate, codice a barre, codice QR | barcode, QR code -->

Un grafico è un'immagine di numeri che resta legata ad essi: se cambi un numero, il grafico cambia di conseguenza. Immagini e forme si collocano sopra le celle allo stesso modo, ma non sono legate ad alcun numero.

## Creare un grafico

1. Seleziona i numeri da rappresentare. Se la prima riga contiene nomi e la prima colonna contiene etichette, selezionale anche tu. JeffCrunch controlla la prima riga e la prima colonna e le usa come nomi e come etichette quando sono testo (o date, per le etichette).
2. Nella scheda **Inserisci**, nel gruppo **Grafici**, scegli un tipo: **Colonna**, **Barra**, **Linea**, **Torta**, **Area**, **Dispersione** o **Combinato**. La piccola freccia accanto a un tipo mostra le sue varianti, come colonne impilate, una linea con indicatori oppure un grafico a dispersione di punti, di punti collegati o di linee lisciate.
3. Per vedere subito tutti i tipi, scegli **Altri grafici**. L'elenco a sinistra mostra ogni tipo disegnato con i tuoi dati. **Scambia righe e colonne** inverte il senso delle serie.

Non sai quale tipo scegliere? Seleziona le celle e scegli **Grafici consigliati** nella scheda **Inserisci**. JeffCrunch esamina le celle e propone alcuni tipi adatti, dal più indicato. Ad esempio, date disposte in verticale suggeriscono una linea, e una sola colonna di pochi numeri positivi con etichette suggerisce una torta. Ogni tipo viene disegnato con le tue celle. Scegline uno e fai clic su **OK**.

Se selezioni una sola cella, JeffCrunch crea il grafico con il blocco di celle attorno ad essa che contiene dei dati.

Le serie scorrono lungo le colonne quando la selezione ha almeno tante righe quante colonne, e lungo le righe quando ha più colonne che righe. Una torta contiene una sola serie. Un grafico a dispersione prende i valori orizzontali dalla prima colonna.

Esempio: selezioni A1:C4, dove A2:A4 contengono Q1, Q2 e Q3, B1 e C1 contengono Est e Ovest, e il resto sono numeri. Il grafico ha due serie, Est e Ovest, e tre gruppi di colonne, da Q1 a Q3.

## Nascondere temporaneamente una serie

Seleziona un grafico. Accanto ad esso compaiono tre piccoli pulsanti. Quello con l'imbuto è **Filtri grafico**. Mostra le serie del grafico, ciascuna con una casella di controllo. Deseleziona una casella per escludere quella serie dal grafico. Selezionala di nuovo per riportarla nella sua posizione. Funziona anche **Annulla**. Un grafico conserva sempre almeno una serie. Una serie esclusa resta nel file quando lo salvi e rimane nascosta quando apri il file, quindi non si perde nulla. Seleziona di nuovo la casella per riportarla. (Un grafico ad anello mostra tutte le serie quando viene aperto.)

## Spostare e ridimensionare

Fai clic su un grafico, un'immagine o una forma per selezionarlo. Trascinalo per spostarlo, oppure trascina uno degli otto quadratini sul bordo per ridimensionarlo.

- Tieni premuto **Alt** mentre trascini per allineare un bordo al bordo di una cella.
- Tieni premuto **Maiusc** mentre trascini un angolo per mantenere le proporzioni.
- I tasti freccia lo spostano di alcuni pixel alla volta. **Ctrl** con una freccia lo sposta di un pixel, **Maiusc** con una freccia lo ingrandisce o lo rimpicciolisce, e **Alt** con una freccia lo porta al bordo successivo di una cella.
- **Esc** mentre trascini lo riporta alla posizione iniziale.

Ogni spostamento o ridimensionamento è un passaggio che puoi annullare. Fai clic con il pulsante destro sull'oggetto e scegli **Proprietà** per digitare il **Nome** (ogni oggetto di un foglio deve avere il proprio) e il **Testo alternativo** (una breve frase che descrive l'oggetto a chi non può vederlo), per digitarne le dimensioni, e per scegliere cosa fa quando inserisci, elimini o ridimensioni celle: si sposta e si ridimensiona con le celle, si sposta con esse ma mantiene le proprie dimensioni, oppure resta dov'è. La scelta viene salvata nel file. **Stampa questo oggetto**, nella stessa finestra, esclude l'oggetto dalle pagine stampate (resta visibile sullo schermo); anche questa scelta viene mantenuta nel file.

## Modificare cosa mostra un grafico

Quando un grafico è selezionato, accanto ad esso compaiono due pulsanti e sulla barra multifunzione compare la scheda **Grafico**.

- Il pulsante **+** (e **Elementi grafico** nella scheda) attiva o disattiva il titolo del grafico, gli assi, i titoli degli assi, le linee della griglia, la legenda e le etichette dati, e indica dove vanno la legenda e le etichette.
- Il pulsante a pennello (e **Colori** e **Stili** nella scheda) assegna al grafico uno dei set di colori e uno degli aspetti di JeffCrunch. **Layout** imposta più elementi contemporaneamente.
- **Dati grafico** elenca le serie del grafico. Puoi aggiungerle, modificarle, rimuoverle e riordinarle, cambiare le celle che ciascuna legge, impostare le celle con le etichette, scambiare righe e colonne, e scegliere come rappresentare le celle vuote e le righe e colonne nascoste.
- **Pulsanti dei campi** (nel menu **+**, solo per un grafico pivot) mostra piccoli pulsanti con i nomi dei campi della tabella pivot seguita dal grafico: i filtri del report sotto il titolo, e le righe, i valori e le colonne lungo il fondo. Attiva o disattiva ciascun tipo, oppure scegli **Mostra tutto** o **Nascondi tutto**. Un nuovo grafico pivot li mostra.
- **Barre di errore** (nel menu **+**, per un grafico a colonne, barre, linee, aree o dispersione) disegna una piccola linea sopra e sotto ogni punto per mostrare di quanto il valore reale potrebbe discostarsi. Scegli **Errore standard**, **Percentuale (5%)**, **Deviazione standard** oppure **Valore fisso (1)**. Scegli **Personalizzate** per usare importi tuoi: digita le celle che contengono gli importi sopra ogni punto (ad esempio `=Foglio1!$D$2:$D$4`), poi le celle per gli importi sotto. Lasciane una vuota per disegnare le barre da un solo lato. Le barre cambiano quando cambiano quelle celle. Un file che contiene importi propri per ogni punto li conserva.
- **Linee** (nel menu **+**, per un grafico a linee) comprende **Linee di caduta**, che scendono dal punto più alto di ogni categoria fino all'asse, **Linee alto-basso**, che uniscono il punto più alto e quello più basso di ogni categoria, e **Barre su/giù**, che sono riquadri tra la prima e l'ultima serie di ogni categoria. Una barra verso l'alto indica che l'ultima serie è più alta. Le linee alto-basso e le barre su/giù richiedono due o più serie. I grafici ad area hanno le linee di caduta.
- **Linea di tendenza** (nel menu **+**) disegna una linea adattata ai numeri di ogni serie, per capire dove si stanno dirigendo. Scegli **Lineare** per una retta, **Esponenziale** per numeri che crescono di una percentuale costante, **Logaritmica** per numeri che salgono rapidamente e poi si stabilizzano, **Polinomiale** per una curva con deviazioni, **Potenza**, oppure **Media mobile**, che disegna la media degli ultimi numeri. **Mostra equazione** scrive sul grafico la formula della linea, e **Mostra R al quadrato** scrive quanto bene la linea si adatta (1 indica un adattamento perfetto; 0 nessun adattamento). La linea di tendenza ha una voce propria nella legenda.
- Quando una serie è disegnata rispetto a un secondo asse verticale, i menu **Assi** e **Titoli degli assi** contengono le voci **Secondo orizzontale** e **Secondo verticale** per essa. Il menu **Legenda** contiene **Sopra il grafico**, che colloca la legenda sull'area del grafico invece che accanto ad essa.
- **Etichette dati** contiene anche **Etichetta contiene** (il valore, il nome della categoria, il nome della serie, la percentuale e la chiave della legenda, cioè un piccolo quadrato del colore della serie, in qualsiasi combinazione), **Separatore** (ciò che sta tra di essi: virgola, punto e virgola, punto, a capo o spazio) e **Formato numero…** (un codice di formato come 0,0 per i numeri nelle etichette; lascialo vuoto per mostrare i numeri così come sono). **Valore da celle…** mostra il testo di alcune celle come etichette, una cella per ogni punto. Digita le celle, ad esempio =Foglio1!$D$2:$D$4. Le etichette cambiano quando cambiano le celle. Lascia vuoto il campo per rimuoverle. Disattiva tutti i contenuti e le etichette scompaiono.
- **Formato asse orizzontale…** e **Formato asse verticale…** nella scheda **Grafico** aprono una finestra per un singolo asse. Imposta il valore minimo e massimo e le unità principale e secondaria (ciascuna automatica o un numero digitato), se i valori vanno in ordine inverso, le unità di visualizzazione (le etichette mostrano i numeri divisi, ad esempio per 1.000 per le migliaia, quindi 30.000 si legge 30; seleziona **Mostra il nome delle unità nel grafico** per stampare il nome, ad esempio Migliaia, insieme al titolo dell'asse), se la scala è logaritmica (ogni segno è una base moltiplicata per il precedente, ad esempio 1, 10, 100 e 1000, adatta a numeri di grandezza molto diversa che devono essere tutti maggiori di 0), il punto in cui l'altro asse interseca questo, il formato numerico delle etichette, i segni di graduazione, la posizione delle etichette e di quanto sono ruotate. Solo un asse numerico ha limiti e unità.
- **Sposta grafico…** nella scheda **Grafico** chiede dove deve andare il grafico. Scegli **Nuovo foglio** e digita un nome, e il grafico ottiene un foglio grafico proprio: un foglio che contiene solo il grafico, senza celle. Scegli **Oggetto in** e seleziona un foglio di lavoro, e il grafico viene collocato su quel foglio. Un grafico su un foglio grafico può essere spostato solo su un foglio di lavoro, e il foglio grafico da cui esce viene eliminato. Il grafico continua a leggere le celle che leggeva. Su un foglio di lavoro mantiene le sue dimensioni e la sua posizione sul foglio, anche dove righe e colonne sono più alte o più larghe, e si sposta e si ridimensiona con le celle come prima. Non vengono proposti i fogli nascosti. È un solo **Annulla**, anche dopo il salvataggio, a meno che un foglio sia stato eliminato dall'operazione. Un file in formato `.ods` non può contenere un foglio grafico, quindi salva un foglio grafico in formato `.xlsx`.
- **Titolo grafico** e **Titoli degli assi** (nel menu **+**) hanno anche **Testo del titolo…**. Digita le parole che vuoi. Oppure digita un segno di uguale e una cella, ad esempio **=Foglio1!A1**, e il titolo mostrerà il contenuto di quella cella. Quando la cella cambia, cambia anche il titolo. Il file conserva il collegamento.
- **Salva come immagine…** nella scheda **Grafico** (e nel menu del clic destro) salva il grafico selezionato come file immagine **PNG** o **JPEG**, due volte più grande di come appare sul foglio, così resta nitido.
- **Salva come modello…** e **Applica modello…** (nella scheda **Grafico**, nel gruppo **Tipo**) conservano l'aspetto di un grafico in un file **.crtx** e lo applicano a un altro grafico. L'aspetto comprende il tipo, i colori, le impostazioni del titolo e della legenda, gli assi e i caratteri. Il grafico a cui lo applichi mantiene i propri numeri e le celle che legge. Un solo **Annulla** elimina di nuovo l'aspetto.
- **Tipo di grafico** cambia il tipo per l'intero grafico, oppure per ciascuna serie: colonne con una linea sopra, oppure una serie rispetto a un secondo asse verticale.

Un riferimento a celle si scrive come **Foglio1!$B$2:$B$9**, con il nome del foglio tra apici singoli quando contiene spazi.

## Sparkline

Uno sparkline è un piccolo grafico che sta dentro una sola cella. Mostra l'andamento di una riga o di una colonna di numeri accanto ad essi.

1. Seleziona i numeri. Per uno sparkline per ogni riga, seleziona più righe di numeri.
2. Nella scheda **Inserisci**, nel gruppo **Sparkline**, scegli **Linea**, **Colonna** oppure **Vittoria/Sconfitta**. **Linea** unisce i numeri con una linea. **Colonna** disegna una barra per ogni numero. **Vittoria/Sconfitta** disegna una barra verso l'alto per ogni numero maggiore di zero e una barra verso il basso per ogni numero minore, tutte della stessa dimensione, adatto a risultati che sono solo buoni o cattivi.
3. Nella finestra **Crea sparkline**, controlla l'**Intervallo dati** (le celle che gli sparkline leggono) e digita l'**Intervallo di posizione** (le celle in cui vanno). Scegli una cella di posizione per ogni riga di dati, oppure per ogni colonna di dati. Se scegli una sola cella, lo sparkline legge tutti i dati.

Gli sparkline creati in un'unica operazione formano un gruppo e condividono lo stesso aspetto. Fai clic su una cella con uno sparkline e compare la scheda **Sparkline**.

- **Modifica dati** modifica i dati e la posizione dell'intero gruppo, oppure i dati di quello su cui hai fatto clic. Indica anche cosa fare con le celle vuote (lasciare uno spazio, trattarle come zero oppure unire i punti attraverso di esse) e se leggere le celle nelle righe e colonne nascoste.
- **Tipo** passa da linea a colonna e vittoria/sconfitta.
- **Mostra** contrassegna i punti: il **Punto massimo**, il **Punto minimo**, i **Punti negativi**, il **Primo punto**, l'**Ultimo punto** e, per una linea, i **Marcatori** su ogni punto.
- **Stili** assegna al gruppo un insieme di colori. **Colore sparkline** imposta il colore della linea o delle barre e lo **Spessore** (quanto è spessa la linea, in punti). **Colore marcatore** imposta il colore di ciascun tipo di punto contrassegnato.
- **Asse** mostra una linea allo zero, traccia i numeri da destra a sinistra e imposta il valore minimo e massimo dell'asse verticale. **Automatico per ogni sparkline** adatta ciascuno ai propri numeri. **Uguale per tutti gli sparkline** assegna all'intero gruppo una sola scala, così puoi confrontarli. **Valore personalizzato** ti permette di digitare un numero. **Asse delle date…** distanzia i punti in base alle date: digita le celle che contengono la data di ogni punto, e un punto con una data successiva si trova più avanti. Lascia vuoto il campo per distribuire di nuovo i punti in modo uniforme.
- **Raggruppa** riunisce gli sparkline selezionati in un solo gruppo. **Separa** assegna a ciascuno un aspetto proprio. **Elimina** rimuove gli sparkline selezionati, oppure i loro interi gruppi.

Uno sparkline segue le sue celle: se inserisci una riga sopra i dati, continua a leggere gli stessi numeri. Ogni modifica è un passaggio che puoi annullare. Gli sparkline vengono salvati nel file in una forma che gli altri programmi di fogli di calcolo leggono.

## Inserire un'immagine, una forma o una casella di testo

Nella scheda **Inserisci**, nel gruppo **Illustrazioni**, **Immagine** apre una finestra di selezione file. Scegli un file immagine (PNG, JPEG, GIF, BMP, TIFF o SVG) e questo viene collocato nel foglio nella cella selezionata. Un'immagine più larga di 480 punti viene rimpicciolita per adattarsi, mantenendo la sua forma. La cartella di lavoro conserva l'immagine stessa. Un'immagine di un tipo che la cartella di lavoro non può contenere così com'è (un SVG, ad esempio) viene conservata come PNG, un'immagine semplice fatta di punti. Un file che non è un'immagine, oppure più grande di 16 MB, viene rifiutato con un messaggio. Un file WMF o EMF (i tipi di immagine che alcuni programmi meno recenti creano con linee e forme) viene disegnato e conservato anch'esso come PNG.

**Icone** mostra la stessa finestra di piccole immagini a tratto che hanno JeffPub e JeffDeck, con una casella per cercarle per nome. Scegline una o più, e ognuna viene collocata nel foglio come immagine, larga e alta 48 punti. Viene disegnata in grigio scuro a una dimensione che resta nitida quando la ingrandisci. Ciascuna è un singolo passaggio che puoi annullare.

**Codice a barre** apre un breve menu con due scelte. **Codice a barre** apre una finestra per creare un solo codice a barre (**Codici a barre da celle**, più avanti, ne crea molti): ISBN per libri (con o senza prezzo), EAN-13, UPC-A, EAN-8, Code 128, Code 39 oppure un codice QR, che può contenere qualsiasi testo, come un indirizzo web. Scegli un **Tipo**, digita il testo o le cifre, e la finestra mostra il codice a barre mentre digiti, indicando in rosso cosa non va se non può essere creato. Fai clic su **Inserisci**, e il codice viene collocato nel foglio come immagine, con un passaggio che puoi annullare. Spostalo e ridimensionalo come una qualsiasi immagine. Per modificarlo, selezionalo e fai di nuovo clic su **Codice a barre**, cambia il testo e fai clic su **Aggiorna**; il nuovo codice prende il posto e la larghezza di quello vecchio. Quando salvi, viene salvato come immagine con un SVG che resta nitido, così ogni programma lo mostra, e JeffCrunch conserva le sue impostazioni nel file, quindi resta un codice a barre modificabile quando riapri la cartella di lavoro in JeffCrunch.

**Codice a barre > Codici a barre da celle** (nello stesso menu **Codice a barre**) crea un codice a barre per ogni cella compilata tra quelle selezionate, per un foglio di etichette. Seleziona le celle, fai clic su questa voce, poi scegli il tipo e l'aspetto nella stessa finestra. Ciò che digiti lì è solo un esempio, perché il testo di ciascuna cella diventa il suo codice a barre. I codici a barre vengono collocati in una colonna subito a destra delle celle selezionate, uno sotto l'altro, come un singolo passaggio che puoi annullare. Una cella il cui testo il codice non può contenere (una lettera in un EAN-13, ad esempio) viene segnalata con un messaggio e non riceve alcun codice a barre. Al massimo vengono elaborate 500 celle alla volta; seleziona meno celle se viene segnalato che sono troppe. Ogni codice a barre ricorda la cella da cui proviene e la segue: quando modifichi quella cella, il codice viene ricreato dal nuovo testo, nello stesso punto e alla stessa dimensione, come passaggio autonomo che puoi annullare. Il collegamento si sposta con la cella quando inserisci o elimini righe o colonne. Se elimini la cella, il codice a barre conserva l'ultimo testo che aveva. Al salvataggio, l'immagine nel file viene creata dal testo della cella così com'è in quel momento, così anche altri programmi mostrano il valore attuale.

**Forme** apre un elenco: rettangolo, rettangolo arrotondato, ovale, triangolo, rombo, frecce, una stella, un cuore, una nuvola, un fumetto e una linea. Scegline una e viene collocata nel foglio di azzurro chiaro con un bordo più scuro. **Casella di testo**, nel gruppo **Testo**, inserisce una casella vuota senza riempimento e senza bordo.

Con una forma selezionata, **Stili** nella scheda **Formato forma** le assegna un aspetto pronto all'uso: un riempimento e un contorno che si abbinano, ad esempio azzurro chiaro con un bordo blu più scuro. Gli ultimi due mantengono solo il contorno o solo il riempimento. Un clic è un passaggio che puoi annullare. **Riempimento forma** e **Contorno forma** modificano ciascuna parte separatamente.

## Trovare immagini gratuite online

**Immagini online**, nel gruppo **Illustrazioni** della scheda **Inserisci**, trova immagini gratuite. Provengono da due biblioteche aperte: Openverse, che cerca in molte raccolte, e Wikimedia Commons, la biblioteca delle immagini di Wikipedia.

1. Fai clic su **Immagini online…**. Si apre un riquadro a destra.
2. Scegli una biblioteca, digita ciò che cerchi e fai clic su **Cerca** (oppure premi Invio).
3. Fai clic su un'immagine per vedere chi l'ha realizzata e quale licenza ha. Una **licenza** indica come puoi usare un'immagine. Fai clic sul nome della licenza per leggerla.
4. Fai clic su **Inserisci** (oppure fai doppio clic sull'immagine).

**Cosa invia JeffCrunch.** Le parole della tua ricerca vanno alla biblioteca che hai scelto, e solo quando fai clic su **Cerca** o premi Invio. JeffCrunch poi chiede alla biblioteca le anteprime piccole di ciò che ha trovato e l'unica immagine che inserisci. Come ogni sito web, la biblioteca può vedere l'indirizzo internet del tuo computer, e viene informata che la richiesta arriva da JeffCrunch. Nulla della tua cartella di lavoro viene inviato.

L'immagine viene collocata nel foglio come un'immagine da file, e la cartella di lavoro la conserva. Il suo titolo viene salvato come **testo alternativo**. La maggior parte delle licenze chiede di citare l'autore. Quando **Aggiungi i crediti sotto l'immagine** è selezionato, JeffCrunch aggiunge una piccola casella di testo sotto l'immagine con il suo titolo, il suo autore e la sua licenza. Puoi spostarla, modificarla o eliminarla. L'immagine e i suoi crediti vengono inseriti in un solo passaggio, quindi **Annulla** rimuove entrambi. Verifica che la licenza sia compatibile con il modo in cui userai l'immagine.

## Aggiungere il testo artistico

**Testo artistico** sono parole decorate: lettere con colore, contorno, ombra o una forma curva.

1. Fai clic su **Inserisci > Testo > Testo artistico**. Si apre una galleria di stili.
2. Fai clic su uno stile. Le parole "Inserisci qui il tuo testo" vengono collocate nel foglio in quello stile.
3. Per cambiare le parole, seleziona il testo artistico e fai clic su **Modifica testo…** nella scheda **Formato forma**. Una finestra chiede le parole, il carattere, la dimensione, il grassetto e il corsivo. Una dimensione maggiore rende più grande il testo artistico.
4. Nella scheda **Formato forma**, **Stili testo artistico** dà alle lettere un altro stile, e **Deformazione testo** piega le parole in una forma, come un'onda o un arco. "Semplice" le lascia dritte.

Il testo artistico è una forma nel disegno del foglio, con la sua deformazione, il riempimento e il contorno, quindi mantiene il suo aspetto quando salvi la cartella di lavoro in formato .xlsx e la riapri, in JeffCrunch o in un altro programma. Ogni modifica è un passaggio che puoi annullare. Una cartella di lavoro salvata in formato .ods conserva anche immagini, forme, caselle di testo, gruppi, linee e testo artistico. Il testo artistico viene salvato come la forma di testo decorato che OpenDocument prevede, con parole, carattere, riempimento, contorno e deformazione, e torna testo artistico quando riapri il file .ods. Alcune deformazioni per cui OpenDocument non ha una forma (Onda 2, Onda 4, e le forme inferiore e superiore di Gonfia e Sgonfia) vengono salvate come parole diritte con le stesse lettere e lo stesso stile. Anche i grafici vengono conservati in un file .ods: vedi più avanti.

## Rimpicciolire le immagini

Un'immagine proveniente da una fotocamera può essere molto più grande di come appare sul foglio, e rende il file pesante. Fai clic su un'immagine, poi fai clic su **Comprimi immagini…** nel gruppo **Regola** della scheda **Formato immagine**. Scegli dove sarà usata la cartella di lavoro: **Stampa commerciale (300 ppi)**, **Stampa desktop (220 ppi)** oppure **Web (96 ppi)**. Lascia selezionato **Applica a tutte le immagini di questo foglio** per rimpicciolire tutte le immagini del foglio, oppure deselezionalo per rimpicciolire solo l'immagine selezionata. **Elimina le aree ritagliate delle immagini** rimuove anche le parti nascoste che hai ritagliato, e il ritaglio scompare. Un'immagine viene sostituita solo quando la nuova è più piccola. Le sue dimensioni, la sua posizione, il suo nome e il suo testo alternativo restano invariati. Annulla (Ctrl+Z) ripristina le immagini, e la cartella di lavoro salvata contiene solo le immagini più piccole. Le immagini di altri fogli non vengono modificate: vai su ciascun foglio e ripeti l'operazione. Un'immagine che è un disegno (SVG, EMF o WMF), oppure un'animazione, resta invariata.

Ciascuna operazione è un singolo passaggio che puoi annullare. L'oggetto nuovo risulta selezionato, quindi puoi trascinarlo, ridimensionarlo, portarlo avanti o indietro, assegnargli un nome e un testo alternativo in **Proprietà**, oppure premere **Canc**.

## Modificare l'aspetto di un'immagine, di una forma o di una casella di testo

Seleziona un'immagine e compare la scheda **Formato immagine**. Seleziona una forma, una linea o una casella di testo e compare la scheda **Formato forma**. Ogni modifica è un passaggio che puoi annullare, e l'oggetto mantiene il suo nome, il suo testo alternativo, la sua posizione e il suo ordine tra gli altri oggetti.

- **Riempimento forma** e **Contorno forma** scelgono un colore, oppure nessuno. **Stile linea** imposta lo spessore (quanto è spessa la linea) e il tratteggio (continuo, tratteggiato o punteggiato). Un'immagine ha **Bordo immagine**, che è il suo contorno.
- **Ruota** gira l'oggetto di un quarto di giro a destra o a sinistra, oppure lo capovolge orizzontalmente o verticalmente.
- **Ritaglia**, per un'immagine, la taglia in una forma di finestra: quadrata, 4 di larghezza per 3 di altezza, 3 di larghezza per 4 di altezza, 16 per 9 oppure 9 per 16. Il taglio viene fatto in modo uniforme dal centro di ciò che si vede, e l'immagine viene ingrandita quanto basta per riempire ciò che resta. **Mostra l'immagine intera** ripristina ciò che è stato tagliato. Il file dell'immagine non viene modificato: la parte tagliata viene solo nascosta.
- **Modifica testo** serve per forme e caselle di testo. Digita le parole, e scegli come si dispongono in orizzontale e in verticale, la dimensione, il grassetto o il normale, e se vanno a capo nella forma o restano su una riga sola. Per il testo artistico chiede invece le parole, il carattere, la dimensione, il grassetto e il corsivo.
- **Riquadro di selezione** apre un elenco di tutti gli oggetti del foglio per nome, con quello in primo piano per primo. Fai clic su un nome per selezionare quell'oggetto, utile quando gli oggetti si nascondono a vicenda. **Porta avanti** e **Porta indietro** lo spostano nell'elenco.
- **Proprietà** imposta le dimensioni, il nome, il testo alternativo e ciò che l'oggetto fa quando le celle vengono spostate o ingrandite.

## Tagliare, copiare e incollare un oggetto

Un'immagine o una forma si copia e si incolla come un grafico. Una forma che usa qualcosa di diverso da un'immagine della cartella di lavoro, come un collegamento a un file, non può essere copiata, e JeffCrunch ti dice perché.

Copia un grafico e incollalo nella stessa cartella di lavoro, su qualsiasi foglio: la copia legge le stesse celle del primo. Incollalo in un'altra cartella di lavoro e mantiene i numeri che mostrava ma non legge più celle, perché le celle che leggerebbe lì non sono quelle da cui è stato creato.

## Selezionare più oggetti, raggrupparli e allinearli

Fai clic su un oggetto, poi tieni premuto Maiusc o Ctrl e fai clic su un altro per selezionare anche quello. Fai di nuovo clic su un oggetto selezionato tenendo premuto Maiusc o Ctrl per deselezionarlo. Gli oggetti selezionati ricevono ciascuno una cornice tratteggiata. Puoi anche scegliere **Seleziona oggetti** nella scheda **Formato forma** e trascinare un riquadro sopra celle vuote: ogni oggetto che rientra interamente nel riquadro viene selezionato. Premi Esc, oppure scegli di nuovo **Seleziona oggetti**, per tornare a selezionare celle. Premi Canc per eliminare in un solo passaggio tutti gli oggetti selezionati.

Con due o più oggetti selezionati, la scheda **Formato forma** contiene questi comandi nel gruppo **Disponi**:

- **Raggruppa** unisce le forme, le immagini e le linee selezionate in un unico oggetto. Si sposta e si ridimensiona come un tutt'uno, e il file lo conserva come gruppo. Un grafico non può essere messo in un gruppo.
- **Separa** scompone un gruppo selezionato. Ogni parte torna dov'era sul foglio.
- **Allinea** allinea gli oggetti selezionati: **Allinea a sinistra**, **Allinea al centro**, **Allinea a destra**, **Allinea in alto**, **Allinea al centro verticalmente** e **Allinea in basso**. Si allineano al bordo o al centro dell'insieme completo. **Distribuisci orizzontalmente** e **Distribuisci verticalmente** distanziano in modo uniforme tre o più oggetti tra i due estremi.

Raggruppa, Separa e ciascuna scelta di allineamento sono un passaggio che puoi annullare.

## Assegnare un nome

Con un oggetto selezionato, la casella del nome mostra il suo nome. Digita un nuovo nome e premi **Invio**. Annulla ripristina il nome precedente, e il nuovo nome viene salvato nel file.

## Eliminare un oggetto

Premi **Canc** con l'oggetto selezionato. Annulla lo riporta.

## Cosa viene salvato

Tutto ciò che è descritto sopra viene salvato nel file in una forma che gli altri programmi di fogli di calcolo leggono, quindi un grafico creato qui si apre allo stesso modo altrove. Un grafico proveniente da un altro programma che contiene elementi che JeffCrunch non può conservare quando riscrive un grafico (barre di errore, un grafico tridimensionale) può comunque essere spostato, ridimensionato ed eliminato, ma i suoi elementi e i suoi dati non vengono modificati qui.

## Diagrammi di altri programmi

Un **diagramma** è un'immagine di caselle e frecce, come un processo, creata da un altro programma. JeffCrunch mostra un diagramma usando l'immagine che il file conserva accanto ad esso. Fai clic per selezionarlo e trascinalo per spostarlo. Quando salvi il file, il diagramma viene riscritto esattamente com'era, così il programma che l'ha creato può ancora modificarlo.

JeffCrunch non può ancora cambiare il contenuto di un diagramma, e un diagramma non può essere copiato né messo in un gruppo. Se il file non conserva un'immagine di un diagramma, JeffCrunch mostra un semplice riquadro grigio che indica che si tratta di un diagramma.

## Grafici nei file .ods

Una cartella di lavoro salvata in formato .ods conserva i suoi grafici: colonne, barre, linee, aree, torte, ciambelle e grafici a dispersione, ciascuno con le sue serie e le celle che legge (su questo foglio o su un altro), categorie, titolo, legenda, assi (titoli, valore minimo e massimo, formato numerico), etichette dati, colori e posizione. Un file .ods creato da un altro programma si apre con i suoi grafici, disegnati dalle celle che leggono. Ciò che un grafico .ods non può contenere viene omesso e segnalato al salvataggio: linee di tendenza, barre di errore, assi secondari e gli aspetti particolari di un singolo punto. I grafici di tipi che JeffCrunch non disegna (grafici di borsa e radar) si aprono come colonne dei loro valori.
