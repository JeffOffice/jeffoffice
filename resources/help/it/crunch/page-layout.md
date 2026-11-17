# Imposta le pagine
<!-- keywords: sfondo, immagine dietro le celle, affianca, carta da parati, layout pagina, margini, orientamento, verticale, orizzontale, carta, dimensioni, area di stampa, interruzione di pagina, interruzioni, titoli di stampa, titoli, ripeti righe, adatta alla pagina, linee di griglia, intestazioni, opzioni foglio, da destra a sinistra, speculare -->

La scheda **Layout di pagina** stabilisce come un foglio viene suddiviso in pagine quando viene stampato o convertito in PDF. Ogni foglio conserva le proprie scelte, salvate nella cartella di lavoro. Ogni modifica è un singolo passaggio che puoi annullare.

## Margini, orientamento e dimensioni

- **Margini** imposta lo spazio vuoto attorno alle celle. **Standard**, **Ampi** e **Stretti** sono insiemi predefiniti. **Margini personalizzati** apre la finestra di dialogo Imposta pagina, dove puoi digitare ciascun margine e indicare a quale distanza dal bordo del foglio si trovano l'intestazione e il piè di pagina.
- **Orientamento** è **Verticale** (la carta è più alta che larga) o **Orizzontale** (più larga che alta).
- **Dimensioni** indica il formato della carta: A4, A5, Legal e altri. **Altre dimensioni carta** li elenca tutti.

## Area di stampa

Per impostazione predefinita un foglio stampa tutto ciò che contiene qualcosa: valori, formule, celle con un bordo o un riempimento visibili, e qualsiasi grafico, immagine o forma. Per stampare solo una parte:

1. Seleziona le celle che desideri.
2. Scegli **Area di stampa > Imposta area di stampa**.

**Aggiungi selezione all'area di stampa** inserisce un secondo blocco di celle nell'area di stampa. Ogni blocco viene stampato su pagine proprie. **Cancella area di stampa** torna a stampare tutto. Le righe o le colonne che inserisci o elimini all'interno dell'area di stampa la modificano insieme alle celle.

## Interruzioni di pagina

La posizione in cui una pagina termina e la successiva inizia dipende dalla carta, dai margini e dalle dimensioni di righe e colonne: una riga o una colonna non viene mai tagliata a metà. Per terminare una pagina prima, seleziona la cella sotto e a destra del punto in cui deve trovarsi l'interruzione, quindi scegli **Interruzioni > Inserisci interruzione di pagina**. Con una riga intera selezionata, l'interruzione va sopra di essa; con una colonna intera selezionata, va a sinistra. **Rimuovi interruzione di pagina** elimina l'interruzione sopra o a sinistra della cella selezionata, e **Reimposta tutte le interruzioni di pagina** elimina tutte quelle impostate. Le interruzioni impostate vengono ignorate mentre il foglio è adattato alla pagina.

## La visualizzazione Interruzioni di pagina

**Visualizza > Interruzioni di pagina** (oppure il pulsante in basso a destra della finestra) mostra il foglio suddiviso nelle pagine su cui viene stampato. Le celle che vengono stampate restano come sono, tutto il resto è ombreggiato, e ogni pagina riporta il proprio numero sopra le sue celle. Una linea tratteggiata indica un'interruzione calcolata da JeffCrunch, una linea continua un'interruzione impostata da te.

- **Spostare un'interruzione.** Trascina una linea su un'altra riga o colonna. Diventa un'interruzione impostata in quel punto, e le pagine successive vengono suddivise di nuovo.
- **Rimuovere un'interruzione.** Trascina una linea fuori dalle celle stampate, sui numeri di riga o sulle lettere di colonna.
- **Modificare l'area di stampa.** Trascina il bordo blu delle celle stampate. Se non hai impostato un'area di stampa, le celle che si stampano da sole diventano un'area, con quel bordo spostato.

Ogni trascinamento è un passaggio di Annulla. Mentre il foglio è adattato alla pagina, le sue interruzioni vengono ignorate, quindi le linee non possono essere trascinate. **Normale** ripristina le celle come erano. La visualizzazione in cui è stato lasciato ciascun foglio viene salvata con la cartella di lavoro.

## La visualizzazione Layout di pagina

**Visualizza > Layout di pagina** (oppure il pulsante centrale in basso a destra della finestra) mostra il foglio come le pagine su cui viene stampato, una sotto l'altra, ciascuna come un foglio di carta con i suoi margini e uno spazio tra le pagine. Lungo la parte superiore e a sinistra corrono i righelli. Intestazione e piè di pagina vengono disegnati nel punto in cui verranno stampati, così come le immagini che contengono.

- **Lavorare nelle celle.** Fai clic su una cella per selezionarla, trascina per selezionarne più d'una, e usa i tasti freccia, la digitazione, F2 e il doppio clic come nella visualizzazione Normale. Quello che digiti va in una striscia sopra la cella, sulla sua pagina.
- **Partire dalla scheda Inserisci.** **Inserisci > Intestazione e piè di pagina** mostra le pagine (se non sono già visibili) e apre la parte centrale dell'intestazione della prima pagina, pronta per la digitazione. Fai clic su un'altra parte dell'intestazione o del piè di pagina per scrivere lì.
- **Digitare un'intestazione o un piè di pagina.** Sposta il puntatore sul margine superiore o inferiore di una pagina: le sue parti sinistra, centrale e destra si illuminano. Fai clic su una di esse e digita. Compare la scheda **Intestazione e piè di pagina**, con pulsanti che inseriscono il numero di pagina, il numero di pagine, la data, l'ora, la cartella e il nome del file, il nome del foglio e un'immagine; le sue opzioni consentono di impostare una prima pagina, pagine pari e dispari con intestazioni e piè di pagina propri, e stabiliscono se l'intestazione si ingrandisce con le celle e si allinea ai margini. **Intestazione** e **Piè di pagina** in quella scheda sono raccolte di modelli pronti, come **Pagina 1**, **Pagina 1 di ?**, il nome del foglio, il nome del file e la data. Sceglierne uno sostituisce l'intera intestazione o il piè di pagina delle pagine in cui stai scrivendo (le parole che stavi ancora digitando non vengono conservate). **(nessuno)** lo svuota. **Vai a intestazione** e **Vai a piè di pagina** aprono la parte centrale di quel margine per scrivere. Premi **Ctrl+Invio**, oppure fai clic in un altro punto, per terminare; **Esc** annulla la modifica. Ogni intestazione o piè di pagina completato è un passaggio di Annulla.
- **Zoom.** Usa il cursore, oppure tieni premuto **Ctrl** e ruota la rotella del mouse.

Vengono disegnate solo le pagine visibili, quindi un foglio di migliaia di pagine si apre subito. La visualizzazione in cui è stato lasciato ciascun foglio viene salvata con la cartella di lavoro.

## Titoli di stampa

**Titoli di stampa** ripete delle righe in alto, e delle colonne a sinistra, in ogni pagina successiva a quella in cui si trovano, così che una riga di intestazione compaia su ogni pagina. Digita le righe (ad esempio **1:3**) e le colonne (ad esempio **A:B**) nella scheda Foglio della finestra di dialogo Imposta pagina.

## Adatta alla pagina

**Larghezza** e **Altezza** riducono le celle fino a far sì che il foglio non sia più largo, o più alto, di tante pagine. **Automatica** permette a quella direzione di occupare tutte le pagine necessarie. **Scala** stampa a una percentuale della dimensione reale, da 10 a 400, e funziona quando Larghezza e Altezza sono entrambe **Automatica**. Se la riduzione per adattare il foglio rendesse le lettere molto piccole, la schermata Stampa lo segnala.

## Opzioni foglio

**Mostra linee di griglia** e **Mostra intestazioni** cambiano solo lo schermo. **Stampa linee di griglia** riporta sulla carta le linee tra le celle, e **Stampa intestazioni** riporta sulla carta i numeri di riga e le lettere di colonna.

**Foglio da destra a sinistra** fa leggere il foglio da destra. La colonna A passa al bordo destro, e le colonne si contano verso sinistra. Anche i numeri di riga passano al bordo destro. Usalo per le lingue scritte da destra a sinistra, come l'arabo e l'ebraico. Cambia l'aspetto del foglio sullo schermo, e la scelta viene salvata con il foglio, quindi ricompare quando apri la cartella di lavoro. Non è un passaggio annullabile: fai clic di nuovo per disattivarlo.

## Sfondo

**Sfondo** (nel gruppo **Imposta pagina**) inserisce un'immagine dietro le celle del foglio. Scegli un'immagine PNG, JPEG, GIF o BMP. L'immagine viene ripetuta come piastrelle per riempire il foglio, e si sposta con le celle quando scorri. Una cella con un colore di riempimento nasconde l'immagine, quindi assegna un riempimento alle celle che vuoi leggere con facilità. L'immagine appare solo sullo schermo: non viene stampata e non entra nel PDF.

**Elimina sfondo** rimuove l'immagine. Entrambe le operazioni sono annullabili. L'immagine viene salvata con il foglio e ricompare quando apri la cartella di lavoro. In una copia .ods viene salvata come immagine di sfondo della pagina, che altri programmi potrebbero stampare. Un file immagine più grande di 8 MB, o con troppi punti, viene rifiutato, e viene rifiutata anche un'immagine che porterebbe tutte le immagini della cartella di lavoro (sfondi, intestazioni e piè di pagina) a superare insieme i 64 MB. Un'immagine di più di 2.400 punti per lato viene comunque affiancata alla sua dimensione reale.

## Altre opzioni

La piccola freccia nell'angolo del gruppo **Imposta pagina** apre la finestra di dialogo Imposta pagina, che contiene tutte queste opzioni e altre ancora: [La finestra di dialogo Imposta pagina](page-setup).
