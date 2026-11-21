# Imposta le pagine

<!-- keywords: tema, temi, colori del tema, caratteri del tema, colori, tipi di carattere, accento, carattere titoli, carattere corpo, sfondo, immagine dietro le celle, affiancata, carta da parati, layout di pagina, margini, orientamento, verticale, orizzontale, carta, dimensioni, area di stampa, interruzione di pagina, interruzioni, titoli di stampa, titoli, ripeti righe, adatta alla pagina, adatta, linee della griglia, intestazioni, opzioni foglio, da destra a sinistra, specchio -->

La scheda **Layout di pagina** decide come un foglio viene suddiviso in pagine quando viene stampato o trasformato in PDF. Ogni foglio conserva le proprie scelte, salvate nella cartella di lavoro. Ogni modifica è un singolo passaggio che puoi annullare.

## Temi, colori e caratteri

Una cartella di lavoro ha un **tema**: dodici colori e due caratteri (uno per i titoli e uno per il testo) che le sue celle, gli stili di cella, gli stili di tabella, le forme e i grafici possono usare al posto di copiarli. Quando cambi il tema, tutto ciò che lo usa cambia con esso: una cella riempita con "Colore 1, più chiaro 40%" passa al nuovo Colore 1, più chiaro della stessa quantità.

- **Temi** elenca temi pronti che impostano colori e caratteri insieme. **Cerca temi** prende il tema da un file di tema, da una cartella di lavoro, da un documento o da una presentazione. **Salva tema corrente** conserva il tema di questa cartella di lavoro in un file, e **Reimposta sul tema della cartella di lavoro** ripristina il tema che la cartella aveva quando l'hai aperta.
- **Colori** cambia solo i dodici colori. **Crea nuovi colori del tema** ti permette di impostarne di tuoi e di dar loro un nome; da quel momento tutte le app di JeffOffice li elencano.
- **Caratteri** cambia solo il carattere dei titoli e quello del testo. Le celle che usano i caratteri del tema li seguono; le celle impostate su un carattere con il suo nome lo mantengono.

Ogni scelta è un singolo passaggio che puoi annullare. I pulsanti dei colori (**Colore riempimento**, **Colore carattere**, **Bordi** e **Colore scheda**) mostrano in una riga i colori del tema, con versioni più chiare e più scure sotto ciascuno; un colore scelto da lì resta un colore del tema, quindi cambia insieme al tema. Un colore scelto da **Colori standard** o da **Altri colori** resta quello che hai scelto.

Una nuova cartella di lavoro parte con il tema di JeffOffice. Una forma o un grafico che crei qui mantiene i colori che ha ricevuto; uno che proviene dal file e usa un colore del tema segue il tema. Una cartella di lavoro aperta da un file binario meno recente o da un file OpenDocument non ha un tema da modificare, quindi questi pulsanti non offrono nulla. Quando salvi una cartella di lavoro come file OpenDocument, che non ha temi, i colori vengono scritti così come appaiono.

## Margini, orientamento e dimensioni

- **Margini** imposta lo spazio vuoto intorno alle celle. **Standard**, **Ampi** e **Stretti** sono insiemi predefiniti. **Margini personalizzati** apre la finestra di dialogo Imposta pagina, dove puoi digitare ciascun margine e indicare a quale distanza dal bordo della carta si trovano l'intestazione e il piè di pagina.
- **Orientamento** è **Verticale** (la carta è più alta che larga) oppure **Orizzontale** (più larga che alta).
- **Dimensioni** è il formato della carta: Letter, Legal, A4 e altri. **Altri formati carta** li elenca tutti.

## Area di stampa

Per impostazione predefinita un foglio stampa tutto ciò che contiene qualcosa: valori, formule, e celle con un bordo o un riempimento visibile, oltre a grafici, immagini e forme. Per stampare solo una parte:

1. Seleziona le celle che vuoi.
2. Scegli **Area di stampa > Imposta area di stampa**.

**Aggiungi selezione all'area di stampa** inserisce un secondo blocco di celle nell'area di stampa. Ogni blocco viene stampato su pagine proprie. **Cancella area di stampa** torna a stampare tutto. Le righe o le colonne che inserisci o elimini all'interno dell'area di stampa la modificano insieme alle celle.

## Interruzioni di pagina

Il punto in cui una pagina finisce e ne inizia un'altra è calcolato in base alla carta, ai margini e alle dimensioni di righe e colonne: una riga o una colonna non viene mai tagliata a metà. Per terminare una pagina prima, seleziona la cella sotto e a destra del punto in cui deve trovarsi l'interruzione, e scegli **Interruzioni > Inserisci interruzione di pagina**. Con un'intera riga selezionata l'interruzione va sopra di essa, e con un'intera colonna selezionata va a sinistra. **Rimuovi interruzione di pagina** elimina l'interruzione sopra o a sinistra della cella selezionata, e **Reimposta tutte le interruzioni di pagina** elimina tutte quelle che hai impostato. Le interruzioni che hai impostato vengono ignorate mentre un foglio viene ridotto per adattarsi alla pagina.

## La visualizzazione Interruzioni di pagina

**Visualizza > Interruzioni di pagina** (oppure il pulsante in basso a destra della finestra) mostra il foglio diviso nelle pagine su cui viene stampato. Le celle che vengono stampate restano come sono, tutto il resto è ombreggiato, e ogni pagina ha il suo numero sopra le proprie celle. Una linea tratteggiata è un'interruzione calcolata da JeffCrunch, una linea continua è un'interruzione impostata da te.

- **Sposta un'interruzione.** Trascina una linea su un'altra riga o colonna. Diventa un'interruzione impostata da te in quel punto, e le pagine successive vengono suddivise di nuovo.
- **Rimuovi un'interruzione.** Trascina una linea fuori dalle celle stampate, sui numeri di riga o sulle lettere di colonna.
- **Cambia l'area di stampa.** Trascina il bordo blu delle celle stampate. Se non hai impostato un'area di stampa, le celle che vengono stampate da sole diventano un'area, con quel bordo spostato.

Ogni trascinamento è un passaggio di Annulla. Mentre un foglio viene ridotto per adattarsi alla pagina, le sue interruzioni vengono ignorate, quindi le linee non possono essere trascinate. **Normale** riporta le celle come erano. La visualizzazione in cui ciascun foglio è stato lasciato viene salvata con la cartella di lavoro.

## La visualizzazione Layout di pagina

**Visualizza > Layout di pagina** (oppure il pulsante centrale in basso a destra della finestra) mostra il foglio come le pagine su cui viene stampato, una sotto l'altra, ciascuna come un foglio di carta con i propri margini e uno spazio tra le pagine. I righelli corrono lungo la parte superiore e a sinistra. Intestazione e piè di pagina vengono disegnati dove vengono stampati, e così anche le immagini che contengono.

- **Lavora nelle celle.** Fai clic su una cella per sceglierla, trascina per sceglierne più d'una, e usa i tasti freccia, la digitazione, F2 e il doppio clic come nella visualizzazione Normale. Quello che digiti va in una striscia sopra la cella, sulla sua pagina.
- **Parti dalla scheda Inserisci.** **Inserisci > Intestazione e piè di pagina** mostra le pagine (se non sono già visibili) e apre la parte centrale dell'intestazione della prima pagina, pronta per la digitazione. Fai clic su un'altra parte dell'intestazione o del piè di pagina per scrivere lì.
- **Scrivi un'intestazione o un piè di pagina.** Sposta il puntatore sul margine superiore o inferiore di una pagina: le sue parti sinistra, centrale e destra si illuminano. Fai clic su una di esse e digita. Compare la scheda **Intestazione e piè di pagina**, con pulsanti che inseriscono il numero di pagina, il numero di pagine, la data, l'ora, la cartella e il nome del file, il nome del foglio e un'immagine; le sue opzioni prevedono la prima pagina, e le pagine pari e dispari, con intestazioni e piè di pagina propri, e indicano se l'intestazione cresce con le celle e si allinea ai margini. **Intestazione** e **Piè di pagina** di quella scheda sono raccolte di modelli pronti, come **Pagina 1**, **Pagina 1 di ?**, il nome del foglio, il nome del file e la data. Sceglierne uno sostituisce l'intera intestazione o il piè di pagina delle pagine in cui stai scrivendo (le parole che stavi ancora digitando al suo interno non vengono conservate). **(nessuno)** la svuota. **Vai a intestazione** e **Vai a piè di pagina** aprono la parte centrale di quel margine per scrivere. Premi **Ctrl+Invio**, oppure fai clic in un altro punto, per terminare; **Esc** lo annulla. Ogni intestazione o piè di pagina completato è un passaggio di Annulla.
- **Zoom** con il cursore, oppure tieni premuto **Ctrl** e ruota la rotella del mouse.

Vengono disegnate solo le pagine visibili, così un foglio di migliaia di pagine si apre subito. La visualizzazione in cui ciascun foglio è stato lasciato viene salvata con la cartella di lavoro.

## Titoli di stampa

**Titoli di stampa** ripete le righe in alto, e le colonne a sinistra, su ogni pagina successiva a quella in cui si trovano, così una riga di intestazione compare su ogni pagina. Digita le righe (ad esempio **1:3**) e le colonne (ad esempio **A:B**) nella scheda Foglio della finestra di dialogo Imposta pagina.

## Adatta alla pagina

**Larghezza** e **Altezza** riducono le celle finché il foglio non è più largo, o più alto, di quel numero di pagine. **Automatica** lascia che quella direzione usi tutte le pagine di cui ha bisogno. **Scala** stampa in percentuale della dimensione reale, da 10 a 400, e funziona quando Larghezza e Altezza sono entrambe **Automatica**. Se ridurre per adattare il foglio renderebbe le lettere molto piccole, la schermata di stampa lo segnala.

## Opzioni foglio

**Mostra linee della griglia** e **Mostra intestazioni** cambiano solo lo schermo. **Stampa griglia** porta sulla carta le linee tra le celle, e **Stampa intestazioni** porta sulla carta i numeri di riga e le lettere di colonna.

**Foglio da destra a sinistra** fa leggere il foglio da destra. La colonna A passa al bordo destro, e le colonne si contano verso sinistra. Anche i numeri di riga passano al bordo destro. Usalo per le lingue che si scrivono da destra a sinistra, come l'arabo e l'ebraico. Cambia l'aspetto del foglio sullo schermo, e la scelta viene salvata con il foglio, quindi ricompare quando apri la cartella di lavoro. Non è un passaggio che puoi annullare: fai clic di nuovo per disattivarlo.

## Sfondo

**Sfondo** (nel gruppo **Imposta pagina**) mette un'immagine dietro le celle del foglio. Scegli un'immagine PNG, JPEG, GIF o BMP. L'immagine viene ripetuta come piastrelle per riempire il foglio, e si sposta con le celle quando scorri. Una cella con un colore di riempimento nasconde l'immagine, quindi assegna un riempimento alle celle che vuoi leggere facilmente. L'immagine appare solo sullo schermo: non viene stampata e non entra in un PDF.

**Elimina sfondo** rimuove l'immagine. Entrambi sono passaggi che puoi annullare. L'immagine viene salvata con il foglio e ricompare quando apri la cartella di lavoro. In una copia .ods viene salvata come immagine di sfondo della pagina, che altri programmi potrebbero stampare. Un file immagine più grande di 8 MB, o con troppi punti, viene rifiutato, e così un'immagine che porterebbe tutte le immagini della cartella di lavoro (sfondi, intestazioni e piè di pagina) a superare insieme i 64 MB. Un'immagine di più di 2.400 punti per lato viene comunque affiancata alla sua dimensione reale.

## Altre opzioni

La piccola freccia nell'angolo del gruppo **Imposta pagina** apre la finestra di dialogo Imposta pagina, che contiene tutte queste opzioni e altre ancora: [La finestra di dialogo Imposta pagina](page-setup).
