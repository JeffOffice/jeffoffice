# Impostare le pagine
<!-- keywords: layout di pagina, impostazione pagina, margini, orientamento, verticale, orizzontale, carta, foglio, dimensioni, formato, area di stampa, interruzione di pagina, interruzioni, stampa titoli, titoli, ripetere righe, adatta alla pagina, adattare, griglia, intestazioni, opzioni del foglio -->

La scheda **Layout di pagina** stabilisce come un foglio viene suddiviso in pagine quando viene stampato o convertito in PDF. Ogni foglio mantiene le proprie impostazioni, che vengono salvate nella cartella di lavoro. Ogni modifica corrisponde a un passaggio che è possibile annullare.

## Margini, orientamento e dimensioni

- **Margini** imposta lo spazio vuoto intorno alle celle. **Standard**, **Larghi** e **Stretti** sono impostazioni predefinite. **Margini personalizzati** apre la finestra di dialogo Imposta pagina, in cui è possibile digitare ciascun margine e indicare la distanza dell'intestazione e del piè di pagina dal bordo del foglio.
- **Orientamento** può essere **Verticale** (il foglio è più alto che largo) o **Orizzontale** (più largo che alto).
- **Dimensioni** indica il formato della carta: A4, Letter, Legal e altri. **Altre dimensioni foglio** li elenca tutti.

## Area di stampa

Per impostazione predefinita, un foglio stampa tutto ciò che contiene qualcosa: valori, formule, celle con un bordo o un riempimento visibile e qualsiasi grafico, immagine o forma. Per stampare solo una parte:

1. Selezionare le celle desiderate.
2. Scegliere **Area di stampa > Imposta area di stampa**.

**Aggiungi selezione all'area di stampa** aggiunge un secondo blocco di celle all'area di stampa. Ogni blocco viene stampato su pagine separate. **Cancella area di stampa** torna a stampare tutto. Le righe o le colonne inserite o eliminate all'interno dell'area di stampa la modificano insieme alle celle.

## Interruzioni di pagina

Il punto in cui una pagina finisce e inizia la successiva viene calcolato in base alla carta, ai margini e alle dimensioni delle righe e delle colonne: una riga o una colonna non viene mai tagliata a metà. Per terminare una pagina prima, selezionare la cella sotto e a destra del punto in cui deve trovarsi l'interruzione e scegliere **Interruzioni > Inserisci interruzione di pagina**. Se è selezionata un'intera riga, l'interruzione viene inserita sopra di essa; se è selezionata un'intera colonna, viene inserita alla sua sinistra. **Rimuovi interruzione di pagina** elimina l'interruzione sopra o a sinistra della cella selezionata, mentre **Reimposta tutte le interruzioni di pagina** elimina tutte le interruzioni impostate. Le interruzioni di pagina impostate vengono ignorate quando un foglio viene ridotto per adattarlo alla pagina.

## La visualizzazione Interruzioni di pagina

**Visualizza > Interruzioni di pagina** (o il pulsante in basso a destra nella finestra) mostra il foglio suddiviso nelle pagine su cui viene stampato. Le celle stampate restano invariate, tutto il resto è ombreggiato e ogni pagina mostra il proprio numero sopra le celle. Una linea tratteggiata indica un'interruzione calcolata da JeffCrunch, mentre una linea continua indica un'interruzione impostata dall'utente.

- **Spostare un'interruzione.** Trascinare una linea su un'altra riga o colonna. Diventa un'interruzione impostata in quel punto e le pagine successive vengono suddivise di nuovo.
- **Rimuovere un'interruzione.** Trascinare una linea fuori dalle celle stampate, sui numeri di riga o sulle lettere di colonna.
- **Modificare l'area di stampa.** Trascinare il bordo blu delle celle stampate. Se non è stata impostata un'area di stampa, le celle stampate automaticamente diventano l'area di stampa, con il bordo spostato.

Ogni trascinamento corrisponde a un passaggio di Annulla. Quando un foglio viene ridotto per adattarlo alla pagina, le interruzioni vengono ignorate e quindi le linee non possono essere trascinate. **Normale** ripristina la visualizzazione normale delle celle. La visualizzazione in cui è stato lasciato ogni foglio viene salvata con la cartella di lavoro.

## La visualizzazione Layout di pagina

**Visualizza > Layout di pagina** (o il pulsante centrale in basso a destra nella finestra) mostra il foglio come le pagine su cui viene stampato, una sotto l'altra, ciascuna come un foglio di carta con i propri margini e uno spazio tra le pagine. I righelli si trovano lungo il bordo superiore e il bordo sinistro. L'intestazione e il piè di pagina vengono disegnati nella posizione in cui vengono stampati, comprese le immagini che contengono.

- **Lavorare nelle celle.** Fare clic su una cella per selezionarla, trascinare per selezionarne più di una e usare i tasti di direzione, la digitazione, F2 e il doppio clic come nella Visualizzazione Normale. Il testo digitato compare in una striscia sopra la cella nella relativa pagina.
- **Iniziare dalla scheda Inserisci.** **Inserisci > Intestazione e piè di pagina** mostra le pagine (se non sono già visualizzate) e apre la parte centrale dell'intestazione della prima pagina, pronta per la digitazione. Per digitare altrove, fare clic su un'altra parte dell'intestazione o del piè di pagina.
- **Digitare un'intestazione o un piè di pagina.** Spostare il puntatore sul margine superiore o inferiore di una pagina: le parti sinistra, centrale e destra si evidenziano. Fare clic su una di esse e digitare. Viene visualizzata la scheda **Intestazione e piè di pagina**, con pulsanti che inseriscono il numero di pagina, il numero di pagine, la data, l'ora, la cartella e il nome del file, il nome del foglio e un'immagine; le sue opzioni consentono di assegnare intestazioni e piè di pagina distinti alla prima pagina e alle pagine pari e dispari, e di stabilire se l'intestazione si ridimensiona con le celle e si allinea ai margini. Premere **CTRL+INVIO** o fare clic in un altro punto per terminare; **ESC** annulla l'operazione. Ogni intestazione o piè di pagina completato corrisponde a un passaggio di Annulla.
- **Zoom**: usare il dispositivo di scorrimento oppure tenere premuto **CTRL** e ruotare la rotellina del mouse.

Vengono disegnate solo le pagine visibili, quindi anche un foglio di migliaia di pagine si apre immediatamente. La visualizzazione in cui è stato lasciato ogni foglio viene salvata con la cartella di lavoro.

## Stampa titoli

**Stampa titoli** ripete righe in alto e colonne a sinistra in ogni pagina successiva a quella in cui si trovano, in modo che una riga di intestazione compaia in ogni pagina. Digitare le righe (ad esempio **1:3**) e le colonne (ad esempio **A:B**) nella scheda Foglio della finestra di dialogo Imposta pagina.

## Adatta alla pagina

**Larghezza** e **Altezza** riducono le celle finché il foglio non supera in larghezza, o in altezza, il numero di pagine indicato. **Automatico** consente a quella direzione di occupare tutte le pagine necessarie. **Proporzioni** stampa a una percentuale delle dimensioni reali, da 10 a 400, e funziona quando Larghezza e Altezza sono entrambe impostate su **Automatico**. Se la riduzione per adattare il foglio rende il testo molto piccolo, la schermata Stampa lo segnala.

## Opzioni del foglio

**Mostra griglia** e **Mostra intestazioni** modificano solo la visualizzazione sullo schermo. **Stampa griglia** stampa le linee tra le celle e **Stampa intestazioni** stampa i numeri di riga e le lettere di colonna.

## Altre opzioni

La piccola freccia nell'angolo del gruppo **Imposta pagina** apre la finestra di dialogo Imposta pagina, che contiene tutte queste opzioni e altre ancora: [La finestra di dialogo Imposta pagina](page-setup).
