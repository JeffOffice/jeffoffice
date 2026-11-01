# Aprire e salvare cartelle di lavoro
<!-- keywords: nuovo, nuova, vuota, cartel1, book1, senza titolo, aprire, apri, salvare, salva, salva con nome, file, cartella di lavoro, xlsx, xlsm, xls, ods, csv, tsv, file di testo, delimitato, separato, importare, importazione, esportare, esportazione, recenti, aggiunti, trascinare, trascinamento, rilasciare, caricamento, grande, grandi dimensioni, macro, non salvato, modifiche, chiudere, copia, catena di calcolo, modello, xltx, xltm, impossibile aprire, danneggiato, crittografato, protetto, password -->

Quando JeffCrunch si avvia senza un file da aprire, la finestra contiene una nuova cartella di lavoro vuota con un foglio, denominata Book1. Ogni nuova cartella di lavoro prende il numero successivo (Book2, Book3 e così via) e un numero non viene riutilizzato dopo la chiusura della relativa finestra. Per crearne un'altra, premi **{key:New}** oppure fai clic su **File**, quindi su **Nuovo** e scegli **Cartella di lavoro vuota**: si apre in una finestra a sé. Una nuova cartella di lavoro che non hai modificato lascia il posto al file che apri nella sua finestra; una che hai modificato rimane, e il file si apre in un'altra finestra. **Salva** su una nuova cartella di lavoro chiede un nome, come **Salva con nome**, e la salva come file .xlsx. Chiudendo una nuova cartella di lavoro non modificata non viene chiesto nulla.

Per aprire una cartella di lavoro, premi **{key:Open}** oppure fai clic su **File** e poi su **Apri**, e scegli **Sfoglia**. Puoi anche trascinare una cartella di lavoro dal file manager nella finestra. Una cartella di lavoro già aperta in una finestra non viene aperta una seconda volta: quella finestra viene portata in primo piano.

JeffCrunch apre le cartelle di lavoro **.xlsx** e **.xlsm** (quelle che contengono macro) e i modelli **.xltx** e **.xltm** (un modello che contiene macro). Non esegue le macro e le lascia intatte: il progetto delle macro, la sua firma e le parti che vi fanno riferimento vengono riscritti esattamente com'erano. Un modello viene salvato come modello e una cartella di lavoro con macro come cartella di lavoro con macro, perché il tipo fa parte del file.

## Altri tipi di file

JeffCrunch apre anche le cartelle di lavoro **.xls** meno recenti, i fogli di calcolo OpenDocument (**.ods** e il formato flat **.fods**) e i file di testo con valori (**.csv**, **.tsv** e **.txt**). Una cartella di lavoro aperta da uno di questi file viene salvata come nuovo file **.xlsx**: quando premi **{key:Save}**, JeffCrunch ti avvisa e chiede un nome, e il file aperto non viene mai modificato.

Di un foglio di calcolo OpenDocument vengono letti i fogli, i valori, le formule (con i risultati memorizzati nel file), i formati numerici, i tipi di carattere, i riempimenti, i bordi, le celle unite, la larghezza delle colonne e l'altezza delle righe, le righe e le colonne nascoste, i riquadri bloccati e i nomi. Grafici, immagini e altri disegni, commenti, tabelle pivot, macro, formati condizionali e convalida dati non vengono ancora letti: un messaggio in alto indica quali di questi elementi sono presenti nel file, e una copia salvata da qui non li conterrà. Un foglio di calcolo protetto da password non viene aperto: aprilo nel programma con cui è stato creato e salvane una copia senza password.

Un file di testo corrisponde a un solo foglio. JeffCrunch rileva come sono separati i valori (da virgole, punti e virgola, tabulazioni o barre verticali) e come sono codificati i caratteri (UTF-8 o UTF-16, oppure la tabella codici della tua lingua) e legge ogni valore come lo digiteresti nella tua lingua: un numero come 1.234,5, una percentuale, un importo in valuta, una data con l'anno (ad esempio 15/03/2024, o 3/15/2024 in inglese), un'ora, VERO o FALSO (nella tua lingua e in inglese, TRUE e FALSE). Un valore che inizia con un segno di uguale è una formula. Le cifre che iniziano con uno zero (come 007) e i numeri di più di 15 cifre restano testo, in modo che un codice postale o un numero di conto non vengano modificati; lo stesso vale per un giorno e un mese senza anno, in qualsiasi ordine e in qualsiasi lingua (come 3/4, 12.5, 1-2, 1 marzo o 25 dic), perché l'anno andrebbe inventato, e per testi come il nome del gene MARCH1. Un foglio contiene 1.048.576 righe e 16.384 colonne; se il file ne ha di più, JeffCrunch apre ciò che ci sta e ti avvisa.

Quando apri un file di testo, una finestra ne mostra prima le prime righe e chiede come leggerlo: cosa separa i campi (virgola, punto e virgola, tabulazione, spazio, barra verticale o un carattere da digitare, a scelta anche più di uno, con l'opzione di considerare più separatori consecutivi come uno solo), quale carattere racchiude un campo che contiene un separatore, la codifica del testo (Unicode, le tabelle codici di Windows, le tabelle ISO 8859, Mac Roman e quelle dell'Asia orientale) e la riga da cui iniziare. Sopra ogni colonna scegli come leggerla: **Come digitato**, **Testo** (per mantenere invariato un codice articolo come 007 o un numero di conto lungo), una **Data** nell'ordine indicato oppure **Escludi**. L'anteprima segue ogni modifica e mostra al massimo le prime 200 righe e 100 colonne, ma viene aperto l'intero file. **File > Apri > Apri un file di testo** chiede come leggere il file anche quando il nome del file è insolito.

Per conservare solo i valori del foglio visualizzato, fai clic su **File**, quindi su **Salva con nome**, e scegli il tipo di file **.csv** (oppure **.tsv** e **.txt**, separati da tabulazioni). Prima di scrivere il file, JeffCrunch indica cosa contiene la cartella di lavoro che il file non potrà conservare (ad esempio gli altri fogli, le formule, le immagini, i grafici, le note e le macro), chiede come scriverlo e se **Salva solo valori**, **Salva come .xlsx** per conservare tutto, oppure **Annulla**:

- **Separatore.** Una virgola, un punto e virgola, una tabulazione, uno spazio, una barra verticale o un carattere da digitare. Un file **.csv** usa inizialmente il punto e virgola se la tua lingua scrive i numeri con la virgola, e la virgola negli altri casi; un file **.tsv** o **.txt** usa la tabulazione.
- **Codifica del testo.** UTF-8 con BOM (in modo che gli altri programmi interpretino correttamente le lettere accentate), UTF-8 senza BOM, UTF-16 oppure una tabella codici come Europa occidentale (1252), le tabelle ISO 8859 o Mac Roman. Un carattere per cui la tabella codici non prevede un byte viene scritto come punto interrogativo, e la barra di stato indica quanti ce ne sono.
- **Fine riga.** Windows (ritorno a capo e avanzamento riga), Linux e Mac (avanzamento riga) o Mac precedenti (ritorno a capo).
- **Celle.** **Come visualizzate nelle celle** scrive ciò che vedi. **Come memorizzate** scrive tutte le cifre di un numero (0,1 e non 10%) e una data come testo ISO, ad esempio 2026-03-04.

Il testo viene racchiuso tra virgolette quando contiene il separatore, una virgoletta, un'interruzione di riga o uno spazio all'inizio o alla fine. Gli altri fogli, le formule, i formati e le impostazioni non vengono inclusi nel file, e la scrittura del file non modifica la cartella di lavoro.

Una cartella di lavoro proveniente da un file di testo viene riscritta nella forma originale del file: con lo stesso separatore, gli stessi caratteri di fine riga e la stessa codifica. Se la salvi come copia senza modificare alcuna cella, la copia è identica al file, byte per byte, e non viene chiesto nulla.

## Cartelle di lavoro di grandi dimensioni

JeffCrunch legge una cartella di lavoro in background. Le prime righe del foglio che era aperto al momento del salvataggio compaiono quasi subito, anche quando la cartella di lavoro è molto grande. La barra di stato in basso a sinistra indica l'operazione in corso: **Apertura**, **Lettura** di un foglio con il numero di celle lette finora, **Calcolo** con una percentuale e infine **Pronto**.

Durante la lettura puoi scorrere, selezionare e copiare. Le schede degli altri fogli si attivano man mano che vengono letti. Una cartella di lavoro danneggiata viene aperta fin dove è possibile leggerla, e un messaggio indica cosa è stato escluso.

## Se un file non si apre

Quando JeffCrunch non riesce ad aprire un file, ne spiega il motivo in modo chiaro e lascia invariate le cartelle di lavoro aperte. Distingue un file vuoto da uno che non è affatto una cartella di lavoro (ad esempio un documento PDF, un'immagine, un documento di testo o una presentazione); un pacchetto zip troncato o danneggiato, che di solito indica un download o una copia interrotti (in tal caso copialo di nuovo dall'originale); una cartella di lavoro crittografata con una password, che JeffCrunch non è ancora in grado di aprire; e un file composito che non contiene alcuna cartella di lavoro.

## Cartelle di lavoro recenti

La pagina **Apri** della schermata File elenca le cartelle di lavoro aperte di recente. Fai clic con il pulsante destro del mouse su una di esse per **aggiungerla** in cima all'elenco o per rimuoverla dall'elenco.

## Salvataggio

Premi **{key:Save}** oppure fai clic su **File** e poi su **Salva** per salvare le modifiche nella cartella di lavoro aperta. Per conservare l'originale e creare un nuovo file, fai clic su **File** e poi su **Salva con nome**, e scegli un nome. Una cartella di lavoro con macro viene salvata come file **.xlsm**, un modello come file **.xltx** o **.xltm** e qualsiasi altra cartella di lavoro come file **.xlsx**. Un nome che termina con un'estensione diversa non viene accettato, e il messaggio indica quale estensione usare.

JeffCrunch salva solo ciò che hai modificato. Tutto il resto del file, come gli altri fogli, i grafici, le immagini, le macro e le impostazioni che JeffCrunch non mostra, viene copiato così com'era, byte per byte. In un foglio modificato vengono riscritte solo le celle modificate, insieme alle formule che dipendono da esse, che ricevono i nuovi valori. Se non hai modificato alcuna cella, il file non viene toccato.

Se un altro programma ha modificato il file dopo che lo hai aperto, JeffCrunch non lo sovrascrive senza chiedere, perché le modifiche dell'altro programma andrebbero perse. Chiede se **Salva una copia**, **Sovrascrivi** il file oppure **Annulla**.

Il nuovo file viene prima scritto accanto a quello vecchio e lo sostituisce solo quando è completo. Se si verifica un problema, viene visualizzato un messaggio e il file esistente rimane com'era.

Se al momento del salvataggio le formule sono ancora in fase di calcolo, JeffCrunch attende che il calcolo sia terminato, in modo che il file contenga i risultati. Se nel frattempo premi **Esc** per interrompere il calcolo, il salvataggio viene annullato e la barra di stato lo segnala; salva di nuovo quando vuoi. JeffCrunch esclude dal file un solo elemento: l'elenco dell'ordine in cui un programma ha calcolato le formule l'ultima volta (la catena di calcolo). Il programma che aprirà la cartella di lavoro la ricreerà. Se JeffCrunch non è riuscito a calcolare qualche formula, o se hai interrotto il calcolo, il file chiede a quel programma di ricalcolare tutte le formule all'apertura della cartella di lavoro.

## Chiusura con modifiche

Se chiudi una cartella di lavoro con modifiche non salvate, JeffCrunch chiede se **Salva** le modifiche, **Non salvare** oppure **Annulla** per continuare a lavorare. Un asterisco (*) nel titolo della finestra indica la presenza di modifiche non salvate.
