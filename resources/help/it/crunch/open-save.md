# Aprire e salvare le cartelle di lavoro
<!-- keywords: copia, formato precedente, opendocument, compatibilità, nuovo, nuova, vuota, cartel1, senza titolo, apri, aprire, salva, salvare, salva con nome, file, cartella di lavoro, xlsx, xlsm, xls, ods, csv, tsv, file di testo, delimitato, separato, importa, importare, esporta, esportare, recenti, aggiunti, trascina, trascinare, rilascia, caricamento, grande, grandi dimensioni, macro, non salvato, modifiche, chiudi, chiudere, catena di calcolo, modello, xltx, xltm, impossibile aprire, danneggiato, crittografato, password -->

Quando JeffCrunch si avvia senza un file da aprire, la finestra contiene una nuova cartella di lavoro vuota con un solo foglio, denominata Cartel1. Ogni nuova cartella di lavoro prende il numero successivo (Cartel2, Cartel3 e così via) e un numero non viene riutilizzato dopo la chiusura della sua finestra. Per crearne un'altra, premi **{key:New}**, oppure fai clic su **File**, quindi su **Nuovo**, e scegli **Cartella di lavoro vuota**: si apre in una finestra separata. Una nuova cartella di lavoro che non hai modificato lascia il posto al file che apri nella sua finestra; se invece l'hai modificata, rimane aperta e il file si apre in un'altra finestra. Il comando **Salva** su una nuova cartella di lavoro chiede un nome, come **Salva con nome**, e la salva come file .xlsx. Chiudendo una nuova cartella di lavoro che non hai modificato non viene chiesto nulla.

Per aprire una cartella di lavoro, premi **{key:Open}**, oppure fai clic su **File** e quindi su **Apri**, e scegli **Sfoglia**. Puoi anche trascinare una cartella di lavoro dalla gestione file nella finestra. Una cartella di lavoro già aperta in una finestra non viene aperta una seconda volta: viene portata in primo piano la finestra che la contiene.

JeffCrunch apre le cartelle di lavoro **.xlsx** e **.xlsm** (quelle che contengono macro) e i modelli **.xltx** e **.xltm** (un modello che contiene macro). Non esegue le macro e le lascia intatte: il progetto delle macro, la sua firma e le parti che vi fanno riferimento vengono riscritti esattamente come erano. Un modello viene salvato come modello e una cartella di lavoro con macro come cartella di lavoro con macro, perché il tipo fa parte del file.

## Altri tipi di file

JeffCrunch apre anche le cartelle di lavoro **.xls** meno recenti, i fogli di calcolo OpenDocument (**.ods** e il formato flat **.fods**) e i file di testo con valori (**.csv**, **.tsv** e **.txt**). Una cartella di lavoro aperta da uno di questi file viene salvata come nuovo file **.xlsx** (o come copia **.xls** o **.ods**: vedi sotto): quando premi **{key:Save}**, JeffCrunch te lo segnala e chiede un nome, e il file che hai aperto non viene mai modificato.

Di un foglio di calcolo OpenDocument vengono letti i fogli, i valori, le formule (con i risultati memorizzati nel file), i formati numerici, i tipi di carattere, i riempimenti, i bordi, le celle unite, le larghezze delle colonne e le altezze delle righe, le righe e le colonne nascoste, i riquadri bloccati e i nomi. Grafici, immagini e altri disegni, commenti, tabelle pivot, macro, formati condizionali e convalida dati non vengono ancora letti; un messaggio nella parte superiore indica quali di questi elementi sono presenti nel file, e una copia salvata da JeffCrunch non li conterrà. Per un foglio di calcolo protetto da password, la password viene chiesta all'apertura: vedi [Aprire cartelle di lavoro protette da password](open-password).

Un file di testo corrisponde a un solo foglio. JeffCrunch rileva come sono separati i valori (da virgole, punti e virgola, tabulazioni o barre verticali) e come sono codificati i caratteri (UTF-8 o UTF-16, oppure la tabella codici della tua lingua), e legge ogni valore come lo digiteresti nella tua lingua: un numero come 1.234,5 in italiano, una percentuale, un importo in valuta, una data con l'anno (ad esempio 15/03/2024, o 3/15/2024 in inglese), un'ora, VERO o FALSO (nella tua lingua, ad esempio VERO e FALSO in italiano, e in inglese). Un valore che inizia con il segno di uguale è una formula. Le cifre che iniziano con uno zero (ad esempio 007) e i numeri con più di 15 cifre restano testo, in modo che un codice postale o un numero di conto non vengano alterati; lo stesso vale per un giorno e un mese senza anno, in qualsiasi ordine e in qualsiasi lingua (ad esempio 3/4, 12.5, 1-2, 1 marzo o 25 dic), perché l'anno andrebbe inventato, e per testi come il nome del gene MARCH1. Un foglio contiene 1.048.576 righe e 16.384 colonne; se il file ne ha di più, JeffCrunch apre la parte che rientra nel foglio e te lo segnala.

Quando apri un file di testo, una finestra mostra prima le sue prime righe e chiede come leggerlo: cosa separa i campi (virgola, punto e virgola, tabulazione, spazio, barra verticale o un carattere che digiti, in qualsiasi combinazione, con la possibilità di considerare più separatori consecutivi come uno solo), quale carattere racchiude un campo che contiene un separatore, la codifica del testo (Unicode, le tabelle codici di Windows, le tabelle ISO 8859, Mac Roman e quelle dell'Asia orientale) e la riga da cui iniziare. Sopra ogni colonna scegli come leggerla: **Come digitato**, **Testo** (per mantenere invariato un codice articolo come 007 o un numero di conto lungo), **Data** nell'ordine che indichi, oppure **Escludi**. L'anteprima si aggiorna a ogni modifica e mostra al massimo le prime 200 righe e 100 colonne, ma viene aperto l'intero file. **File > Apri > Apri un file di testo** mostra questa finestra anche quando il nome del file ha un'estensione insolita.

Per conservare solo i valori del foglio che stai visualizzando, fai clic su **File**, quindi su **Salva con nome**, e scegli il tipo di file **.csv** (oppure **.tsv** e **.txt**, separati da tabulazioni). Prima di scrivere il file, JeffCrunch indica quali elementi della cartella di lavoro il file non potrà contenere (ad esempio gli altri fogli, le formule, le immagini, i grafici, le note e le macro), chiede come scriverlo e se **Salva solo valori**, **Salva come .xlsx** per conservare tutto oppure **Annulla**:

- **Separatore.** Virgola, punto e virgola, tabulazione, spazio, barra verticale o un carattere che digiti. Un file **.csv** propone il punto e virgola se la tua lingua usa la virgola nei numeri, altrimenti la virgola; un file **.tsv** o **.txt** propone la tabulazione.
- **Codifica del testo.** UTF-8 con BOM (perché gli altri programmi interpretino correttamente le lettere accentate), UTF-8 senza BOM, UTF-16 o una tabella codici come Europa occidentale (1252), le tabelle ISO 8859 o Mac Roman. Un carattere che non esiste nella tabella codici viene scritto come punto interrogativo e la barra di stato indica quanti sono.
- **Fine riga.** Windows (ritorno a capo e avanzamento riga), Linux e Mac (avanzamento riga) o Mac precedenti (ritorno a capo).
- **Celle.** **Come visualizzate nelle celle** scrive ciò che vedi. **Come memorizzate** scrive tutte le cifre di un numero (0,1 e non 10%) e una data come testo ISO, ad esempio 2026-03-04.

Il testo viene racchiuso tra virgolette quando contiene il separatore, le virgolette, un'interruzione di riga o uno spazio all'inizio o alla fine. Gli altri fogli, le formule, i formati e le impostazioni non vengono inclusi nel file, e la scrittura del file non modifica la cartella di lavoro.

Una cartella di lavoro aperta da un file di testo viene riscritta nella forma del file originale: stesso separatore, stessi fine riga e stessa codifica. Se la salvi come copia senza modificare alcuna cella, la copia è identica al file, byte per byte, e non viene chiesto nulla.

## Salvare una copia come .xls o .ods

Per creare una copia che possa essere aperta da programmi meno recenti o da programmi che leggono OpenDocument, fai clic su **File**, quindi su **Salva con nome**, e scegli il tipo di file **.xls** o **.ods**. La copia viene creata dalla cartella di lavoro nel suo stato attuale, comprese le modifiche non ancora salvate, e contiene:

- i valori, con testi, numeri, valori logici ed errori;
- le formule, con i loro risultati attuali (una formula matrice resta una formula matrice);
- i formati numerici, i tipi di carattere, i riempimenti, i bordi, l'allineamento e lo stato di blocco delle celle;
- le celle unite, le larghezze delle colonne e le altezze delle righe, le righe e le colonne nascoste, le strutture di righe e colonne e i riquadri bloccati;
- i fogli, nel loro ordine, con i nomi e l'indicazione di quelli nascosti, e i nomi definiti (incluse le aree di stampa);
- i collegamenti e le impostazioni di stampa di ogni foglio (carta, margini, orientamento, proporzioni, intestazioni e piè di pagina, interruzioni di pagina).

Un file **.xls** contiene anche le tabelle dati e mantiene le impostazioni di calcolo della cartella di lavoro. Una copia non può contenere tutto. Prima di scrivere il file, JeffCrunch indica quali elementi della cartella di lavoro il file non potrà contenere: grafici, immagini e altri disegni, note, tabelle pivot, tabelle, formati condizionali, convalida dati, filtri, protezione del foglio, controlli modulo, macro e collegamenti ad altre cartelle di lavoro (le formule che li usano mantengono i valori che avevano). Una formula che il formato meno recente non è in grado di esprimere (una funzione che non esiste in quel formato o una cella oltre la colonna IV o la riga 65.536) viene scritta come il valore che ha prodotto, e il messaggio ne indica il numero. Il formato meno recente ha inoltre un limite di 65.536 righe, 256 colonne, 255 fogli e 32.767 caratteri per cella, e di circa 4.000 formati di cella e 56 colori: ciò che eccede viene tagliato e il messaggio indica quanto. Puoi quindi scegliere **Salva copia**, **Salva come .xlsx** per conservare tutto oppure **Annulla**.

La copia non modifica la cartella di lavoro, che non viene contrassegnata come salvata. Il file che hai aperto non viene mai sovrascritto: assegna alla copia un nome diverso. Un file **.xls** o **.ods** che hai aperto può essere salvato come copia nell'altro formato.

## Cartelle di lavoro di grandi dimensioni

JeffCrunch legge le cartelle di lavoro in background. Le prime righe del foglio che era aperto al momento del salvataggio vengono visualizzate quasi subito, anche quando la cartella di lavoro è molto grande. La barra di stato in basso a sinistra indica l'operazione in corso: **Apertura**, **Lettura** di un foglio con il numero di celle lette finora, **Calcolo in corso** con una percentuale e infine **Pronto**.

Durante la lettura puoi scorrere, selezionare e copiare. Le schede degli altri fogli diventano attive man mano che vengono lette. Una cartella di lavoro danneggiata viene aperta fin dove è leggibile e un messaggio indica cosa è stato omesso.

## Se un file non si apre

Quando JeffCrunch non riesce ad aprire un file, ne spiega il motivo in modo chiaro e lascia invariate le cartelle di lavoro aperte. Distingue un file vuoto da un file che non è affatto una cartella di lavoro (ad esempio un documento PDF, un'immagine, un documento di testo o una presentazione); un pacchetto zip troncato o danneggiato, che di solito indica un download o una copia interrotti, nel qual caso devi copiarlo di nuovo dall'originale; una cartella di lavoro protetta da password, di cui chiede la password (vedi [Aprire cartelle di lavoro protette da password](open-password)); e un file composito che non contiene alcuna cartella di lavoro.

## Cartelle di lavoro recenti

La pagina **Apri** della schermata File elenca le cartelle di lavoro aperte di recente. Fai clic con il pulsante destro del mouse su una di esse per **aggiungerla** in cima all'elenco o per rimuoverla dall'elenco.

## Salvataggio

Premi **{key:Save}**, oppure fai clic su **File** e quindi su **Salva**, per salvare le modifiche nella cartella di lavoro che hai aperto. Per conservare l'originale e creare un nuovo file, fai clic su **File** e quindi su **Salva con nome**, e scegli un nome. Una cartella di lavoro con macro viene salvata come file **.xlsm**, un modello come file **.xltx** o **.xltm** e qualsiasi altra cartella di lavoro come file **.xlsx**. Un nome con l'estensione di un altro tipo di cartella di lavoro non viene accettato e il messaggio indica quale estensione usare; i nomi con estensione **.xls**, **.ods**, **.csv**, **.tsv** e **.txt** creano copie, come descritto sopra.

JeffCrunch salva solo ciò che hai modificato. Tutto il resto del file, ad esempio gli altri fogli, i grafici, le immagini, le macro e le impostazioni che JeffCrunch non visualizza, viene copiato così com'era, byte per byte. In un foglio modificato vengono riscritte solo le celle che hai cambiato, insieme alle formule che ne dipendono, che ricevono i nuovi valori. Se non hai modificato alcuna cella, il file non viene toccato.

Se un altro programma ha modificato il file dopo che l'hai aperto, JeffCrunch non lo sovrascrive senza chiedere, perché le modifiche dell'altro programma andrebbero perse. Chiede invece se **Salva una copia**, **Sovrascrivi** il file oppure **Annulla**.

Il nuovo file viene prima scritto accanto a quello esistente e lo sostituisce solo quando è completo. Se qualcosa non va a buon fine, viene visualizzato un messaggio e il file esistente rimane com'era.

Se al momento del salvataggio le formule sono ancora in fase di calcolo, JeffCrunch attende il termine del calcolo, in modo che il file ne contenga i risultati. Se nel frattempo premi **Esc** per interrompere il calcolo, il salvataggio viene annullato e la barra di stato lo segnala; salva di nuovo quando vuoi. JeffCrunch omette un solo elemento dal file: l'elenco dell'ordine in cui un programma ha calcolato le formule l'ultima volta (la catena di calcolo). Il programma che aprirà la cartella di lavoro la ricreerà. Se JeffCrunch non è riuscito a calcolare qualche formula, o se hai interrotto il calcolo, il file chiede a quel programma di ricalcolare tutte le formule all'apertura della cartella di lavoro.

## Chiusura con modifiche

Se chiudi una cartella di lavoro con modifiche non salvate, JeffCrunch chiede se **Salva** le modifiche, **Non salvare** oppure **Annulla** per continuare a lavorare. Un asterisco (*) nel titolo della finestra indica la presenza di modifiche non salvate.
