# Apertura e salvataggio delle cartelle di lavoro
<!-- keywords: copia, formato meno recente, opendocument, compatibilità, nuovo, vuoto, cartella1, senza titolo, apri, salva, salva con nome, file, cartella di lavoro, xlsx, xlsm, xls, ods, csv, tsv, file di testo, delimitato, importa, esporta, recenti, aggiunti, trascina, rilascia, caricamento, grande, grandi, macro, non salvato, modifiche, chiudi, copia, catena di calcolo, modello, xltx, xltm, impossibile aprire, danneggiato, cifrato, password, salva una copia, sola lettura, apri in sola lettura, apri come copia, nuova finestra, copia di backup, modelli personali, salva come modello, ots, ripara, apri e ripara -->

Quando JeffCrunch si avvia senza un file da aprire, la finestra contiene una nuova cartella di lavoro vuota con un solo foglio, chiamata Cartella1. Ogni nuova cartella di lavoro prende il numero successivo (Cartella2, Cartella3, e così via), e un numero non viene riutilizzato dopo la chiusura della sua finestra. Per avviarne un'altra, premi **{key:Nuovo}**, oppure fai clic su **File**, quindi su **Nuovo**, e scegli **Cartella di lavoro vuota**; si apre in una finestra propria. Una nuova cartella di lavoro che non hai modificato lascia il posto quando apri un file nella sua finestra; una che hai modificato resta, e il file si apre in un'altra finestra. **Salva** su una nuova cartella di lavoro chiede un nome, così come **Salva con nome**, e la salva come file .xlsx. Chiudendo una nuova cartella di lavoro che non hai modificato, non viene chiesto nulla.

Per aprire una cartella di lavoro, premi **{key:Apri}**, oppure fai clic su **File** e poi su **Apri**, e scegli **Sfoglia**. Puoi anche trascinare una cartella di lavoro dal gestore dei file sulla finestra. Una cartella di lavoro già aperta in una finestra non viene aperta una seconda volta: quella finestra viene portata in primo piano.

JeffCrunch apre le cartelle di lavoro **.xlsx** e le cartelle di lavoro **.xlsm** (quelle che contengono macro), e i modelli **.xltx** e **.xltm** (un modello che contiene macro). Non esegue le macro e le conserva intatte: il progetto macro, la sua firma e le parti che li richiamano vengono riscritti esattamente come erano. Un modello viene salvato come modello, e una cartella di lavoro con macro come cartella di lavoro con macro, perché il tipo fa parte del file.

## Altri tipi di file

JeffCrunch apre anche le cartelle di lavoro **.xls** meno recenti, i fogli di calcolo OpenDocument (**.ods** e il formato piatto **.fods**) e i file di testo con valori (**.csv**, **.tsv** e **.txt**). Una cartella di lavoro aperta da uno di questi file viene salvata come nuovo file **.xlsx** (oppure come copia **.xls** o **.ods**: vedi sotto): quando premi **{key:Salva}**, JeffCrunch te lo segnala e chiede un nome, e il file aperto non viene mai modificato.

Un foglio di calcolo OpenDocument viene letto con i suoi fogli, valori, formule (con i risultati conservati nel file), formati numero, tipi di carattere, riempimenti, bordi, celle unite, larghezze di colonna e altezze di riga, righe e colonne nascoste, riquadri bloccati e nomi. Grafici, immagini e altri disegni, commenti, tabelle pivot, macro, formattazione condizionale e convalida dei dati non vengono ancora letti; un messaggio in alto indica quali di questi elementi sono presenti nel file, e una copia salvata da qui non li conterrà. Un foglio di calcolo protetto da password chiede la password all'apertura: vedi [Aprire cartelle di lavoro protette da password](open-password).

Un file di testo corrisponde a un foglio. JeffCrunch individua come sono separati i valori (da virgole, punti e virgola, tabulazioni o barre verticali) e come sono codificate le lettere (UTF-8 o UTF-16, oppure la tabella codici della tua lingua), e legge ogni valore come lo digiteresti nella tua lingua: un numero come 1.234,5 in tedesco, una percentuale, un importo in valuta, una data con l'anno (come 15/03/2024, oppure 15.03.2024 in tedesco), un orario, VERO o FALSO (nella tua lingua, come WAHR e FALSCH in tedesco, e in inglese). Un valore che inizia con il segno di uguale è una formula. Le cifre che iniziano con uno zero (come 007) e i numeri di più di 15 cifre restano testo, così un codice postale o un numero di conto non vengono modificati; lo stesso vale per un giorno e un mese senza anno, in qualsiasi ordine e in qualsiasi lingua (come 3/4, 12.5, 1-2, 1 marzo o 25 dic), perché l'anno dovrebbe essere inventato, e per testi come il nome del gene MARCH1. Un foglio contiene 1.048.576 righe e 16.384 colonne; se il file ne ha di più, JeffCrunch apre ciò che ci sta e te lo segnala.

Quando apri un file di testo, una finestra mostra prima le sue prime righe e chiede come va letto: che cosa separa i campi (una qualsiasi tra virgola, punto e virgola, tabulazione, spazio, barra verticale o un carattere che digiti, con la possibilità di considerare più separatori consecutivi come uno solo), che cosa racchiude un campo che contiene un separatore, la codifica del testo (Unicode, le tabelle codici di Windows, le pagine ISO 8859, Mac Roman e quelle dell'Asia orientale) e la riga da cui iniziare. Sopra ogni colonna scegli come va letta: **Come digitato**, **Testo** (per conservare un codice come 007 o un numero di conto lungo così com'è), una **Data** nell'ordine che indichi, oppure **Escludi**. L'anteprima segue ogni modifica e mostra al massimo le prime 200 righe e 100 colonne, ma viene aperto l'intero file. **File > Apri > Apri file di testo** chiede queste impostazioni anche quando il nome del file è insolito.

Per conservare solo i valori del foglio che stai visualizzando, fai clic su **File**, quindi su **Salva con nome**, e scegli il tipo di file **.csv** (oppure **.tsv** e **.txt**, con tabulazioni). Prima di scrivere il file, JeffCrunch indica che cosa contiene la tua cartella di lavoro e il file non conterrà (per esempio gli altri fogli, le formule, le immagini, i grafici, le note e le macro), e chiede come scriverlo, se **Salva solo valori**, se **Salva come .xlsx** invece per conservare tutto, oppure se **Annulla**:

- **Separatore.** Virgola, punto e virgola, tabulazione, spazio, barra verticale oppure un carattere che digiti. Un **.csv** usa il punto e virgola dove la tua lingua scrive la virgola nei numeri, e la virgola altrove; un **.tsv** o **.txt** usa la tabulazione.
- **Codifica del testo.** UTF-8 con il contrassegno (così altri programmi leggono correttamente le lettere accentate), UTF-8 senza contrassegno, UTF-16, oppure una tabella codici come Europa occidentale (1252), le pagine ISO 8859 o Mac Roman. Una lettera che la tabella codici non ha è scritta come punto interrogativo, e la barra di stato indica quante sono.
- **Fine riga.** Windows (ritorno a capo e avanzamento riga), Linux e Mac (avanzamento riga), oppure Mac precedenti (ritorno a capo).
- **Celle.** **Come visualizzate nelle celle** scrive ciò che vedi. **Come memorizzate** scrive ogni cifra di un numero (0,1 e non 10%), e una data come testo ISO, per esempio 2026-03-04.

Il testo va tra virgolette quando contiene il separatore, una virgoletta, un'interruzione di riga o uno spazio all'inizio o alla fine. Gli altri fogli, le formule, i formati e le impostazioni non sono nel file, e la cartella di lavoro non viene modificata scrivendone uno.

Una cartella di lavoro proveniente da un file di testo viene riscritta nella forma che il file aveva: stesso separatore, stessi fine riga e stessa codifica. Se la salvi come copia senza modificare alcuna cella, la copia è lo stesso file, byte per byte, e non viene chiesto nulla.

## Salvare una copia come .xls o .ods

Per creare una copia che i programmi meno recenti, o i programmi che leggono OpenDocument, possono aprire, fai clic su **File**, quindi su **Salva con nome**, e scegli il tipo di file **.xls** o **.ods**. La copia è creata dalla tua cartella di lavoro così com'è ora, con le modifiche non ancora salvate, e contiene:

- i valori, con i loro testi, numeri, valori logici ed errori;
- le formule, con i risultati che hanno ora (una formula matriciale resta una formula matriciale);
- i formati numero, i tipi di carattere, i riempimenti, i bordi, l'allineamento e se una cella è bloccata;
- le celle unite, le larghezze di colonna e le altezze di riga, le righe e colonne nascoste, i raggruppamenti di righe e colonne e i riquadri bloccati;
- i fogli, nel loro ordine, con i nomi e l'indicazione di quelli nascosti, i nomi definiti (e le aree di stampa);
- i collegamenti e le impostazioni di stampa di ogni foglio (carta, margini, orientamento, scala, intestazioni e piè di pagina, interruzioni di pagina).

Un file **.xls** contiene anche le tabelle dati e conserva le impostazioni di calcolo della cartella di lavoro. Una copia non può contenere tutto. Prima che JeffCrunch scriva il file, indica ciò che la tua cartella di lavoro contiene e il file non conterrà: grafici, immagini e altri disegni, note, tabelle pivot, tabelle, formattazione condizionale, convalida dei dati, filtri, protezione del foglio, controlli modulo, macro e collegamenti ad altre cartelle di lavoro (le formule che li usano conservano i valori che avevano). Una formula che la forma meno recente non sa esprimere (una funzione che non possiede, oppure una cella oltre la colonna IV o la riga 65.536) viene scritta come il valore che ha dato, e il messaggio ne conta il numero. La forma meno recente si ferma inoltre a 65.536 righe, 256 colonne, 255 fogli e 32.767 caratteri in una cella, e a circa 4.000 formati cella e 56 colori: ciò che non ci sta viene tagliato, e il messaggio dice quanto. Puoi poi scegliere se **Salva copia**, se **Salva come .xlsx** per conservare tutto, oppure se **Annulla**.

La copia non modifica la tua cartella di lavoro, che non viene contrassegnata come salvata. Il file aperto non viene mai scritto: assegna un altro nome alla copia. Una copia di un file **.xls** o **.ods** che hai aperto può essere salvata nell'altro tipo.

## Salvare una copia

Fai clic su **File > Salva una copia** e scegli un nome. JeffCrunch scrive la cartella di lavoro in quel file, con le modifiche non ancora salvate, e tu continui a lavorare nella cartella di lavoro che avevi. Il suo nome e le sue modifiche non salvate restano invariati. Puoi scegliere qualsiasi tipo di file offerto da **Salva con nome**. Una copia non viene mai scritta sopra il file della cartella di lavoro.

## Conservare una copia di backup

Fai clic su **File > Opzioni**, e nella pagina **Generale** attiva **Crea sempre una copia di backup**. Ogni volta che salvi, JeffCrunch copia prima il file precedente in uno chiamato "Backup di" seguito dal suo nome, nella stessa cartella. Se la copia non può essere creata, l'app chiede se salvare comunque. La risposta predefinita è No, a meno che tu non risponda Sì, e il file resta com'era.

## Salvare come modello

Un modello è una cartella di lavoro che usi come punto di partenza. Può contenere formule, stili e fogli. Fai clic su **File > Salva come modello** e digita un nome. Il modello (un file .xltx) va in **Modelli personali**, e la tua cartella di lavoro non viene modificata. In **Salva con nome** puoi anche scegliere **Modelli (*.xltx)**, oppure **Modelli di foglio di calcolo OpenDocument (*.ots)**, per salvare un file modello dove preferisci.

Per avviare una cartella di lavoro dal tuo modello, fai clic su **File > Nuovo**. I tuoi modelli sono elencati sotto **Modelli personali**. Fai clic su uno di essi. Si apre una nuova cartella di lavoro con le celle e i fogli del modello. Non ha ancora un file proprio, quindi **Salva** chiede un nome, e il modello non viene modificato. Per rimuovere un modello dall'elenco, fai clic destro su di esso e scegli **Rimuovi da Modelli personali**.

## Altri modi per aprire un file

Nella pagina **Apri** fai clic su **Altri modi per aprire**. Puoi anche fare clic destro su un nome nell'elenco dei file recenti.

- **Apri in sola lettura** apre la cartella di lavoro solo per la lettura. Puoi guardare, scorrere, cercare e copiare, ma non puoi modificare una cella. La barra del titolo mostra "Sola lettura". Se premi **Salva**, JeffCrunch chiede un nuovo nome, così il file aperto non viene modificato.
- **Apri come copia** apre la cartella di lavoro senza un file proprio. **Salva** chiede dove metterla. Il file aperto non viene mai modificato.
- **Apri in una nuova finestra** apre la cartella di lavoro in una finestra propria, anche quando la finestra in cui ti trovi è vuota.
- **Apri e ripara** serve per un file .xlsx danneggiato. Vedi sotto.

## Apri e ripara

A volte un file .xlsx è danneggiato. Un download si è interrotto a metà, oppure un disco ha commesso errori. Un file .xlsx è un insieme di parti più piccole, come i fogli, gli stili e il testo condiviso. **Apri e ripara** legge ogni parte che riesce a leggere. Una parte interrotta a metà viene conservata fino al punto in cui si è interrotta, quindi potresti ottenere le prime righe di un foglio. Una parte che non può essere letta viene esclusa. Se l'elenco delle parti o i collegamenti tra di esse sono andati persi, JeffCrunch li ricostruisce.

La cartella di lavoro riparata si apre senza un file proprio, con un nome come "Budget riparato". Un messaggio ti indica che cosa è stato conservato fino a un'interruzione e che cosa è stato escluso. Il file danneggiato non viene modificato. Salva la cartella di lavoro riparata con un nuovo nome, e verifica i numeri prima di fidartene.

## Cartelle di lavoro di grandi dimensioni

JeffCrunch legge una cartella di lavoro in background. Le prime righe del foglio che era aperto quando la cartella è stata salvata compaiono quasi subito, anche quando la cartella è molto grande. La barra di stato in basso a sinistra indica che cosa sta facendo: **Apertura**, **Lettura** di un foglio con il numero di celle lette finora, **Calcolo** con una percentuale, e infine **Pronto**.

Puoi scorrere, selezionare e copiare mentre la lettura prosegue. Le schede degli altri fogli si attivano quando vengono letti. Una cartella di lavoro danneggiata viene aperta per quanto è possibile leggerla, e un messaggio indica che cosa è stato escluso.

## Se un file non si apre

Quando JeffCrunch non riesce ad aprire un file, lo dice in parole semplici e lascia invariate le cartelle di lavoro aperte. Distingue un file vuoto da uno che non è affatto una cartella di lavoro (per esempio un documento PDF, un'immagine, un documento di testo o una presentazione); un pacchetto zip troncato o danneggiato, che di solito significa che un download o una copia si sono interrotti, quindi copialo di nuovo dall'originale; una cartella di lavoro protetta da password, per la quale chiede la password (vedi [Aprire cartelle di lavoro protette da password](open-password)); e un file composto che non contiene alcuna cartella di lavoro.

## Cartelle di lavoro recenti

La pagina **Apri** della schermata File elenca le cartelle di lavoro aperte di recente. Fai clic destro su una di esse per **aggiungerla** in cima all'elenco o per rimuoverla dall'elenco.

## Salvataggio

Premi **{key:Salva}**, oppure fai clic su **File** e poi su **Salva**, per salvare le modifiche nella cartella di lavoro aperta. Per conservare l'originale e creare un nuovo file, fai clic su **File** e poi su **Salva con nome**, e scegli un nome. Una cartella di lavoro con macro viene salvata come file **.xlsm**, un modello come file **.xltx** o **.xltm**, e ogni altra cartella di lavoro come file **.xlsx**. Un nome che termina con un altro tipo di cartella di lavoro non viene accettato, e il messaggio indica quale estensione usare; i nomi che terminano con **.xls**, **.ods**, **.csv**, **.tsv** e **.txt** creano copie, come descritto sopra.

JeffCrunch salva solo ciò che hai modificato. Tutto il resto del file, come gli altri fogli, i grafici, le immagini, le macro e le impostazioni che JeffCrunch non mostra, viene copiato com'era, byte per byte. In un foglio che hai modificato, vengono riscritte solo le celle che hai cambiato, insieme alle formule che dipendono da esse, le quali ottengono i nuovi valori. Se non hai modificato alcuna cella, il file viene lasciato in pace.

Se un altro programma ha modificato il file dopo che l'hai aperto, JeffCrunch non lo sovrascrive senza chiedere, perché ciò farebbe perdere le modifiche dell'altro programma. Chiede se **Salva una copia**, se **Sovrascrivi** il file, oppure se **Annulla**.

Il nuovo file viene scritto prima accanto a quello vecchio, e prende il posto di quello vecchio solo quando è completo. Se qualcosa va storto, vedi un messaggio, e il file che c'era resta com'era.

Se le formule sono ancora in fase di calcolo quando salvi, JeffCrunch attende che il calcolo finisca, così il file contiene i loro risultati. Se nel frattempo premi **Esc** per interrompere il calcolo, il salvataggio viene annullato, e la barra di stato lo indica; salva di nuovo quando vuoi. JeffCrunch lascia una cosa fuori dal file: l'elenco dell'ordine in cui un programma ha calcolato per ultimo le formule (la catena di calcolo). Il programma che aprirà la cartella di lavoro successivamente lo ricrea. Se JeffCrunch non è riuscito a calcolare alcune formule, o hai interrotto il calcolo, il file chiede a quel programma di ricalcolare tutte le formule quando apre la cartella di lavoro.

## Chiusura con modifiche

Se chiudi una cartella di lavoro che ha modifiche non salvate, JeffCrunch chiede se **Salvare** le modifiche, se **Non salvare**, oppure se **Annullare** e continuare a lavorare. Un asterisco (*) nel titolo della finestra indica che ci sono modifiche non salvate.
