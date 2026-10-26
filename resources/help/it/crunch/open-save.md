# Aprire e salvare cartelle di lavoro
<!-- keywords: apri, aprire, salva, salvare, salva con nome, file, cartella di lavoro, xlsx, xlsm, xls, ods, csv, tsv, file di testo, delimitato, importa, importare, esporta, esportare, recenti, aggiunti, trascina, rilascia, caricamento, grande, grandi dimensioni, macro, non salvato, modifiche, chiudi, chiudere, copia, catena di calcolo -->

Per aprire una cartella di lavoro, premi **{key:Open}**, oppure fai clic su **File**, quindi su **Apri** e scegli **Sfoglia**. Puoi anche trascinare una cartella di lavoro dal file manager sulla finestra. Una cartella di lavoro già aperta in una finestra non viene aperta una seconda volta: quella finestra viene portata in primo piano.

JeffCrunch apre le cartelle di lavoro **.xlsx** e **.xlsm** (quelle che contengono macro). Non esegue le macro e le lascia invariate.

## Altri tipi di file

JeffCrunch apre anche le cartelle di lavoro **.xls** meno recenti, i fogli di calcolo OpenDocument (**.ods** e il formato flat **.fods**) e i file di testo con valori (**.csv**, **.tsv** e **.txt**). Una cartella di lavoro aperta da uno di questi file viene salvata come nuovo file **.xlsx**: quando premi **{key:Save}**, JeffCrunch ti avvisa e chiede un nome, e il file che hai aperto non viene mai modificato.

Di un foglio di calcolo OpenDocument vengono letti i fogli, i valori, le formule (con i risultati memorizzati nel file), i formati numero, i tipi di carattere, i riempimenti, i bordi, le celle unite, la larghezza delle colonne e l'altezza delle righe, le righe e le colonne nascoste, i riquadri bloccati e i nomi. Grafici, immagini e altri disegni, commenti, tabelle pivot, macro, formati condizionali e convalida dati non vengono ancora letti; un messaggio in alto indica quali di questi elementi sono presenti nel file, e una copia salvata da JeffCrunch non li conterrà. Un foglio di calcolo protetto da password non viene aperto: aprilo nel programma con cui è stato creato e salvane una copia senza password.

Un file di testo corrisponde a un solo foglio. JeffCrunch rileva come sono separati i valori (da virgole, punti e virgola, tabulazioni o barre verticali) e come sono codificati i caratteri (UTF-8 o UTF-16, oppure la tabella codici della tua lingua), e legge ogni valore come lo digiteresti nella tua lingua: un numero come 1.234,5 in italiano, una percentuale, un importo in valuta, una data con l'anno (ad esempio 15/03/2024, o 3/15/2024 in inglese), un'ora, VERO o FALSO (nella tua lingua, come VERO e FALSO in italiano, e in inglese). Un valore che inizia con il segno di uguale è una formula. Le cifre che iniziano con uno zero (ad esempio 007) e i numeri di più di 15 cifre restano testo, in modo che un codice postale o un numero di conto non venga modificato; lo stesso vale per un giorno e un mese senza anno, in qualsiasi ordine e in qualsiasi lingua (ad esempio 3/4, 12.5, 1-2, 1 marzo o 25 dic), perché l'anno andrebbe inventato, e per testi come il nome di gene MARCH1. Un foglio contiene 1.048.576 righe e 16.384 colonne; se il file ne ha di più, JeffCrunch apre la parte che rientra nel foglio e te lo segnala.

Quando apri un file di testo, una finestra mostra prima le prime righe e chiede come leggerlo: che cosa separa i campi (virgola, punto e virgola, tabulazione, spazio, barra verticale o un carattere che digiti, in qualsiasi combinazione, con la possibilità di considerare separatori consecutivi come uno solo), quale carattere racchiude un campo che contiene un separatore, la codifica del testo e la riga da cui iniziare. Sopra ogni colonna scegli come leggerla: **Come digitato**, **Testo** (per mantenere invariato un codice articolo come 007 o un numero di conto lungo), **Data** nell'ordine che indichi, oppure **Escludi**. L'anteprima si aggiorna a ogni modifica e mostra al massimo le prime 200 righe e 100 colonne, ma viene aperto l'intero file. **File > Apri > Apri un file di testo** mostra questa finestra anche quando il nome del file è insolito.

Per conservare solo i valori del foglio visualizzato, fai clic su **File**, quindi su **Salva con nome** e scegli il tipo di file **.csv**. JeffCrunch scrive i valori come vengono visualizzati nelle celle, in UTF-8, separandoli con un punto e virgola se nella tua lingua i numeri usano la virgola e con una virgola negli altri casi, e racchiude il testo tra virgolette quando serve. Gli altri fogli, le formule, i formati e le impostazioni non vengono inclusi nel file .csv, e la cartella di lavoro non viene modificata.

## Cartelle di lavoro di grandi dimensioni

JeffCrunch legge le cartelle di lavoro in background. Le prime righe del foglio che era aperto al momento del salvataggio compaiono quasi subito, anche quando la cartella di lavoro è molto grande. La barra di stato in basso a sinistra indica l'operazione in corso: **Apertura**, **Lettura** di un foglio con il numero di celle lette finora, **Calcolo** con una percentuale e infine **Pronto**.

Durante la lettura puoi scorrere, selezionare e copiare. Le schede degli altri fogli si attivano man mano che vengono lette. Una cartella di lavoro danneggiata viene aperta fin dove è possibile leggerla, e un messaggio indica che cosa è stato escluso.

## Cartelle di lavoro recenti

La pagina **Apri** della schermata File elenca le cartelle di lavoro aperte di recente. Fai clic con il pulsante destro del mouse su una di esse per **aggiungerla** in cima all'elenco o per rimuoverla dall'elenco.

## Salvataggio

Premi **{key:Save}**, oppure fai clic su **File** e quindi su **Salva**, per salvare le modifiche nella cartella di lavoro aperta. Per conservare l'originale e creare un nuovo file, fai clic su **File**, quindi su **Salva con nome** e scegli un nome. Una cartella di lavoro con macro viene salvata come file **.xlsm**, una senza macro come file **.xlsx**.

JeffCrunch salva solo ciò che hai modificato. Tutto il resto del file, come gli altri fogli, i grafici, le immagini, le macro e le impostazioni che JeffCrunch non mostra, viene copiato così com'era, byte per byte. In un foglio modificato vengono riscritte solo le celle che hai cambiato, insieme alle formule che dipendono da esse, che ricevono i nuovi valori. Se non hai modificato alcuna cella, il file non viene toccato.

Se un altro programma ha modificato il file dopo che l'hai aperto, JeffCrunch non lo sovrascrive senza chiedere, perché le modifiche dell'altro programma andrebbero perse. Ti chiede se **Salva una copia**, **Sovrascrivi** il file oppure **Annulla**.

Il nuovo file viene prima scritto accanto a quello esistente e lo sostituisce solo quando è completo. Se qualcosa va storto, viene visualizzato un messaggio e il file esistente resta com'era.

Se al momento del salvataggio le formule sono ancora in fase di calcolo, JeffCrunch attende che il calcolo sia terminato, in modo che il file ne contenga i risultati. Se nel frattempo premi **ESC** per interrompere il calcolo, il salvataggio viene annullato e la barra di stato lo segnala; salva di nuovo quando vuoi. JeffCrunch omette un solo elemento dal file: l'elenco dell'ordine in cui un programma ha calcolato le formule l'ultima volta (la catena di calcolo). Il programma che aprirà la cartella di lavoro la ricreerà. Se JeffCrunch non è riuscito a calcolare qualche formula, o se hai interrotto il calcolo, il file chiede a quel programma di ricalcolare tutte le formule all'apertura della cartella di lavoro.

## Chiusura con modifiche

Se chiudi una cartella di lavoro con modifiche non salvate, JeffCrunch ti chiede se **Salva** le modifiche, **Non salvare** oppure **Annulla** per continuare a lavorare. Un asterisco (*) nel titolo della finestra indica la presenza di modifiche non salvate.
