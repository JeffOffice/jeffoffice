# Imposta le pagine
<!-- keywords: layout di pagina, margini, orientamento, verticale, orizzontale, carta, formato, dimensioni, area di stampa, interruzione di pagina, interruzioni, titoli di stampa, titoli, ripeti righe, adatta alla pagina, adatta, griglia, intestazioni, opzioni foglio, da destra a sinistra, speculare, imposta pagina, stampa, pagina, margine -->

La scheda **Layout di pagina** stabilisce come un foglio viene suddiviso in pagine quando viene stampato o convertito in PDF. Ogni foglio conserva le proprie scelte, salvate nella cartella di lavoro. Ogni modifica è un'unica operazione che si può annullare.

## Margini, orientamento e dimensioni

- **Margini** imposta lo spazio vuoto attorno alle celle. **Standard**, **Ampi** e **Stretti** sono insiemi predefiniti. **Margini personalizzati** apre la finestra di dialogo Imposta pagina, dove è possibile digitare ciascun valore e indicare la distanza dell'intestazione e del piè di pagina dal bordo della carta.
- **Orientamento** può essere **Verticale** (la carta è più alta che larga) oppure **Orizzontale** (più larga che alta).
- **Dimensioni** indica il formato della carta: Letter, Legal, A4 e altri. **Altre dimensioni carta** elenca tutti i formati disponibili.

## Area di stampa

Per impostazione predefinita un foglio stampa tutto ciò che contiene: valori, formule, celle con bordo o riempimento visibile e qualsiasi grafico, immagine o forma. Per stampare solo una parte:

1. Seleziona le celle che ti interessano.
2. Scegli **Area di stampa > Imposta area di stampa**.

**Aggiungi selezione all'area di stampa** inserisce un secondo blocco di celle nell'area di stampa. Ogni blocco viene stampato su pagine proprie. **Cancella area di stampa** ripristina la stampa di tutto il contenuto. Le righe o le colonne inserite o eliminate all'interno dell'area di stampa la modificano insieme alle celle.

## Interruzioni di pagina

Il punto in cui una pagina termina e ne inizia un'altra è calcolato in base alla carta, ai margini e alle dimensioni di righe e colonne: una riga o una colonna non viene mai tagliata a metà. Per terminare una pagina prima, seleziona la cella sotto e a destra del punto in cui deve trovarsi l'interruzione, e scegli **Interruzioni > Inserisci interruzione di pagina**. Con una riga intera selezionata, l'interruzione va sopra di essa; con una colonna intera selezionata, va a sinistra. **Rimuovi interruzione di pagina** elimina l'interruzione sopra o a sinistra della cella selezionata, e **Reimposta tutte le interruzioni di pagina** elimina tutte quelle impostate manualmente. Le interruzioni di pagina impostate vengono ignorate mentre un foglio viene adattato alla pagina.

## La visualizzazione Interruzioni di pagina

**Visualizza > Interruzioni di pagina** (oppure il pulsante in basso a destra della finestra) mostra il foglio suddiviso nelle pagine su cui viene stampato. Le celle che vengono stampate restano com'erano, tutto il resto è ombreggiato, e ogni pagina ha il suo numero sopra le proprie celle. Una linea tratteggiata indica un'interruzione calcolata da JeffCrunch, una linea continua un'interruzione impostata da te.

- **Spostare un'interruzione.** Trascina una linea verso un'altra riga o colonna. Diventa un'interruzione impostata in quella posizione e le pagine successive vengono ricalcolate.
- **Rimuovere un'interruzione.** Trascina una linea fuori dalle celle stampate, sui numeri di riga o sulle lettere di colonna.
- **Modificare l'area di stampa.** Trascina il bordo blu delle celle stampate. Se non hai impostato un'area di stampa, le celle che vengono stampate da sole diventano un'area di stampa, con il bordo spostato.

Ogni trascinamento è un'operazione annullabile. Mentre un foglio viene adattato alla pagina, le sue interruzioni vengono ignorate, quindi le linee non possono essere trascinate. **Normale** riporta le celle com'erano. La visualizzazione in cui ogni foglio è stato lasciato viene salvata con la cartella di lavoro.

## La visualizzazione Layout di pagina

**Visualizza > Layout di pagina** (oppure il pulsante centrale in basso a destra della finestra) mostra il foglio come le pagine su cui viene stampato, una sotto l'altra, ciascuna come un foglio di carta con i suoi margini e uno spazio tra le pagine. Lungo il bordo superiore e lungo quello sinistro sono presenti i righelli. Intestazione e piè di pagina sono disegnati nel punto in cui verranno stampati, così come le immagini che contengono.

- **Lavorare nelle celle.** Fai clic su una cella per selezionarla, trascina per selezionarne più d'una, e usa i tasti freccia, la digitazione, F2 e il doppio clic come nella visualizzazione Normale. Ciò che digiti viene inserito in una barra sopra la cella, sulla sua pagina.
- **Iniziare dalla scheda Inserisci.** **Inserisci > Intestazione e piè di pagina** mostra le pagine (se non sono già visibili) e apre la parte centrale dell'intestazione della prima pagina, pronta per la digitazione. Fai clic su un'altra parte dell'intestazione o del piè di pagina per digitare lì.
- **Digitare un'intestazione o un piè di pagina.** Passa il puntatore sul margine superiore o inferiore di una pagina: le sue parti sinistra, centrale e destra si evidenziano. Fai clic su una di esse e digita. Compare la scheda **Intestazione e piè di pagina**, con pulsanti che inseriscono il numero di pagina, il numero totale di pagine, la data, l'ora, la cartella e il nome del file, il nome del foglio e un'immagine. Le sue opzioni permettono di impostare una prima pagina diversa, intestazioni e piè di pagina propri per le pagine pari e dispari, e stabiliscono se l'intestazione si adatta al contenuto delle celle e si allinea ai margini. **Intestazione** e **Piè di pagina** nella scheda sono raccolte di formati pronti, come **Pagina 1**, **Pagina 1 di ?**, il nome del foglio, il nome del file e la data. Scegliendone uno, si sostituisce l'intera intestazione o piè di pagina delle pagine in cui stai scrivendo (il testo che stavi ancora digitando non viene conservato). **(nessuna)** la svuota. **Vai a intestazione** e **Vai a piè di pagina** aprono la parte centrale di quel margine per la digitazione. Premi **Ctrl+Invio**, oppure fai clic altrove, per terminare; **Esc** annulla la modifica. Ogni intestazione o piè di pagina completata è un'operazione annullabile.
- **Ingrandire.** Usa il cursore di zoom oppure tieni premuto **Ctrl** e ruota la rotellina del mouse.

Vengono disegnate solo le pagine visibili, quindi un foglio di migliaia di pagine si apre subito. La visualizzazione in cui ogni foglio è stato lasciato viene salvata con la cartella di lavoro.

## Titoli di stampa

**Titoli di stampa** ripete le righe in alto e le colonne a sinistra su ogni pagina successiva a quella in cui si trovano, in modo che una riga di intestazione compaia su tutte le pagine. Digita le righe (ad esempio **1:3**) e le colonne (ad esempio **A:B**) nella scheda Foglio della finestra di dialogo Imposta pagina.

## Adatta alla pagina

**Larghezza** e **Altezza** riducono le celle finché il foglio non è più largo, o più alto, di quel numero di pagine. **Automatica** lascia che quella direzione usi tutte le pagine necessarie. **Scala** stampa in percentuale delle dimensioni reali, da 10 a 400, e funziona quando Larghezza e Altezza sono entrambe impostate su **Automatica**. Se la riduzione necessaria per adattare il foglio rende i caratteri molto piccoli, la schermata Stampa lo segnala.

## Opzioni foglio

**Mostra griglia** e **Mostra intestazioni** modificano solo lo schermo. **Stampa griglia** riporta sulla carta le linee tra le celle, mentre **Stampa intestazioni** riporta i numeri di riga e le lettere di colonna.

**Foglio da destra a sinistra** fa leggere il foglio da destra. La colonna A passa sul bordo destro, e le colonne si contano verso sinistra. Anche i numeri di riga passano sul bordo destro. Usalo per lingue che si scrivono da destra a sinistra, come arabo ed ebraico. Cambia l'aspetto del foglio sullo schermo, e la scelta viene salvata con il foglio, quindi resta attiva quando apri di nuovo la cartella di lavoro. Non è un'operazione annullabile: per disattivarla, fai clic di nuovo sul comando.

## Altre opzioni

La piccola freccia nell'angolo del gruppo **Imposta pagina** apre la finestra di dialogo Imposta pagina, che contiene tutte queste opzioni e altre ancora: [La finestra di dialogo Imposta pagina](page-setup).
