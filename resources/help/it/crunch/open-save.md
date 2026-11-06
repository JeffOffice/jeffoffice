# Apri e salva cartelle di lavoro
<!-- keywords: copia, formato meno recente, opendocument, compatibilità, nuovo, vuoto, cartella1, senza titolo, apri, salva, salva con nome, file, cartella di lavoro, xlsx, xlsm, xls, ods, csv, tsv, file di testo, delimitato, importa, esporta, recenti, aggiunti, trascina, rilascia, caricamento, grande, macro, non salvato, modifiche, chiudi, copia, catena di calcolo, modello, xltx, xltm, impossibile aprire, danneggiato, cifrato, password, salva una copia, sola lettura, apri in sola lettura, apri come copia, nuova finestra, copia di backup, I miei modelli, salva come modello, ots, ripara, apri e ripara, modelli predefiniti, budget, fattura, foglio ore, prestito, calendario, registro voti, larghezza fissa, colonne fisse, separatore decimale, separatore delle migliaia, segno meno finale, html, pagina web, htm, tabelle da una pagina -->

Quando JeffCrunch si avvia senza un file da aprire, la finestra contiene una nuova cartella di lavoro vuota con un solo foglio, denominata Cartella1. Ogni nuova cartella di lavoro prende il numero successivo (Cartella2, Cartella3, e così via), e un numero non viene riutilizzato dopo la chiusura della sua finestra. Per avviarne un'altra, premi **{key:New}**, oppure fai clic su **File**, poi su **Nuovo**, e scegli **Cartella di lavoro vuota**; si apre in una finestra propria. Una nuova cartella di lavoro che non hai modificato cede il posto quando apri un file nella sua finestra; una che hai modificato resta, e il file si apre in un'altra finestra. **Salva** su una nuova cartella di lavoro chiede un nome, come fa **Salva con nome**, e la salva come file .xlsx. Chiudere una nuova cartella di lavoro che non hai modificato non chiede nulla.

Per aprire una cartella di lavoro, premi **{key:Open}**, oppure fai clic su **File** e poi su **Apri**, e scegli **Sfoglia**. Puoi anche trascinare una cartella di lavoro dal gestore dei file sulla finestra. Una cartella di lavoro già aperta in una finestra non viene aperta una seconda volta: quella finestra passa in primo piano.

JeffCrunch apre le cartelle di lavoro **.xlsx** e le cartelle di lavoro **.xlsm** (quelle che contengono macro), e i modelli **.xltx** e **.xltm** (un modello che contiene macro). Non esegue le macro e le lascia intatte: il progetto di macro, la sua firma e le parti che le richiamano vengono riscritti esattamente come erano. Un modello viene salvato come modello, e una cartella di lavoro con macro come cartella di lavoro con macro, perché il tipo fa parte del file.

## Altri tipi di file

JeffCrunch apre anche le cartelle di lavoro **.xls** meno recenti, i fogli di calcolo OpenDocument (**.ods** e il formato piatto **.fods**), e i file di testo con valori (**.csv**, **.tsv** e **.txt**). Una cartella di lavoro aperta da uno di questi file viene salvata come nuovo file **.xlsx** (oppure come copia **.xls** o **.ods**: vedi sotto): quando premi **{key:Save}**, JeffCrunch te lo segnala e chiede un nome, e il file che hai aperto non viene mai modificato.

Un foglio di calcolo OpenDocument viene letto con i suoi fogli, valori, formule (con i risultati che il file conservava), formati dei numeri, tipi di carattere, riempimenti, bordi, celle unite, larghezze delle colonne e altezze delle righe, righe e colonne nascoste, riquadri bloccati e nomi. Grafici, immagini e altri disegni, commenti, tabelle pivot, macro, formattazioni condizionali e convalida dei dati non vengono ancora letti; un messaggio in alto indica quali di questi elementi contiene il file, e una copia salvata da qui non li avrà. Un foglio di calcolo protetto con password chiede la password quando lo apri: vedi [Aprire cartelle di lavoro protette da password](open-password).

Un file di testo è un solo foglio. JeffCrunch stabilisce come sono separati i suoi valori (da virgole, punti e virgola, tabulazioni o barre verticali) e come sono codificati i suoi caratteri (UTF-8 o UTF-16, oppure la pagina di codice della tua lingua), e legge ogni valore come lo digiteresti nella tua lingua: un numero come 1.234,5 in tedesco, una percentuale, un importo in valuta, una data con l'anno (come 15/03/2024, o 15.03.2024 in tedesco), un orario, VERO o FALSO (nella tua lingua, come WAHR e FALSCH in tedesco, e TRUE e FALSE in inglese). Un valore che inizia con un segno di uguale è una formula. Le cifre che iniziano con uno zero (come 007) e i numeri di più di 15 cifre restano testo, così un codice postale o un numero di conto non vengono modificati; restano testo anche un giorno e un mese senza anno, in qualsiasi ordine e in qualsiasi lingua (come 3/4, 12.5, 1-2, 1 marzo o 25 dic), perché l'anno dovrebbe essere inventato, e restano testo anche parole come il nome del gene MARCH1. Un foglio contiene 1.048.576 righe e 16.384 colonne; se il file ne contiene di più, JeffCrunch apre ciò che ci sta e te lo segnala.

Quando apri un file di testo, una finestra mostra prima le sue prime righe e chiede come va letto: che cosa separa i campi (una qualsiasi tra virgola, punto e virgola, tabulazione, spazio, barra verticale o un carattere che digiti, con la possibilità di considerare più separatori consecutivi come uno solo), che cosa racchiude un campo che contiene un separatore, la codifica del testo (Unicode, le pagine di codice di Windows, le pagine ISO 8859, Mac Roman e quelle dell'Asia orientale) e la riga da cui cominciare. Sopra ogni colonna scegli come va letta: **Come digitato**, **Testo** (per conservare un codice come 007 o un lungo numero di conto così com'è), una **Data** nell'ordine che indichi, oppure **Escludi**. Alcuni file allineano i valori in colonne di larghezza uguale, con spazi tra l'uno e l'altro e nessun separatore. Seleziona **Le colonne hanno una larghezza fissa**, e le prime righe del file compaiono in un riquadro. Fai clic tra due caratteri per iniziare una colonna lì, e fai di nuovo clic per eliminarla; oppure digita le posizioni in **Le colonne iniziano a**, come 10, 25, 40. Ogni parte viene tagliata in quei punti, e gli spazi intorno vengono eliminati. **Separatore decimale** e **Separatore delle migliaia** indicano a JeffCrunch come sono scritti i numeri in questo file, a prescindere dalla tua lingua, così 1.234,50 vale milleduecentotrentaquattro e mezzo quando il file usa la virgola come separatore decimale. Seleziona **Il segno meno può essere posto dopo il numero** per i file che scrivono -12 come 12-. L'anteprima segue ogni modifica e mostra al massimo le prime 200 righe e 100 colonne, ma viene aperto l'intero file. **File > Apri > Apri un file di testo** chiede anche quando il nome del file è insolito.

JeffCrunch apre anche le **pagine web** (**.html** e **.htm**), e una pagina salvata con il nome di una cartella di lavoro. Ogni tabella della pagina diventa un foglio, denominato **Tabella1**, **Tabella2**, e così via. Le celle vengono lette come se le avessi digitate: un numero è un numero, e una data è una data. Una cella unita su più colonne o righe lascia celle vuote, così le altre celle restano nelle loro colonne. Le parole fuori dalle tabelle, le immagini e l'aspetto della pagina non vengono letti, e una pagina senza tabelle non si apre. Nulla di ciò che la pagina nomina viene scaricato da internet.

Per conservare solo i valori del foglio che stai guardando, fai clic su **File**, poi su **Salva con nome**, e scegli il tipo di file **.csv** (oppure **.tsv** e **.txt**, separati da tabulazioni). Prima di scrivere il file, JeffCrunch dice che cosa contiene la tua cartella di lavoro e il file non conserverà (per esempio gli altri fogli, le formule, le immagini, i grafici, le note e le macro), chiede come scriverlo, e se **Salva solo valori**, **Salva come .xlsx** per mantenere tutto, oppure **Annulla**:

- **Separatore.** Virgola, punto e virgola, tabulazione, spazio, barra verticale, oppure un carattere che digiti. Un **.csv** inizia con il punto e virgola dove la tua lingua scrive la virgola nei numeri, e con la virgola altrove; un **.tsv** o **.txt** con la tabulazione.
- **Codifica del testo.** UTF-8 con indicatore (così gli altri programmi leggono correttamente le lettere accentate), UTF-8 senza indicatore, UTF-16, oppure una pagina di codice come Europa occidentale (1252), le pagine ISO 8859, Mac Roman, o le pagine giapponese, cinese, coreana, ebraica, araba, baltica, vietnamita e thai. Una lettera per cui la pagina di codice non ha un byte viene scritta come punto interrogativo, e la barra di stato indica quante sono.
- **Fine riga.** Windows (ritorno a capo e avanzamento riga), Linux e Mac (avanzamento riga), oppure Mac meno recenti (ritorno a capo).
- **Celle.** **Come visualizzate nelle celle** scrive ciò che vedi. **Come memorizzate** scrive ogni cifra di un numero (0,1 e non 10%), e una data come testo ISO, come 2026-03-04.

Il testo viene racchiuso tra virgolette quando contiene il separatore, una virgoletta, un'interruzione di riga, oppure uno spazio all'inizio o alla fine. Gli altri fogli, le formule, i formati e le impostazioni non vengono inclusi nel file, e la cartella di lavoro non viene modificata dalla scrittura di un file.

Una cartella di lavoro proveniente da un file di testo viene riscritta nella forma che il file aveva: stesso separatore, stesse fini riga e stessa codifica. Se la salvi come copia senza modificare alcuna cella, la copia è lo stesso file, byte per byte, e non viene chiesto nulla.

## Salvare una copia come .xls o .ods

Per creare una copia che i programmi meno recenti, o i programmi che leggono OpenDocument, possono aprire, fai clic su **File**, poi su **Salva con nome**, e scegli il tipo di file **.xls** o **.ods**. La copia viene creata dalla tua cartella di lavoro così com'è ora, con le modifiche non ancora salvate, e contiene:

- i valori, con i loro testi, numeri, valori logici ed errori;
- le formule, con i risultati che hanno ora (una formula matriciale resta una formula matriciale);
- i formati dei numeri, i tipi di carattere, i riempimenti, i bordi, l'allineamento e se una cella è bloccata;
- le celle unite, le larghezze delle colonne e le altezze delle righe, le righe e le colonne nascoste, i raggruppamenti di righe e colonne, e i riquadri bloccati;
- i fogli, nel loro ordine, con i nomi e l'indicazione di quelli nascosti, e i nomi che hai definito (e le aree di stampa);
- i collegamenti, e il modo in cui stampa ogni foglio (la carta, i margini, l'orientamento, la scala, le intestazioni e i piè di pagina, e le interruzioni di pagina).

Un file **.xls** contiene anche le tabelle dei dati, e conserva le impostazioni di calcolo della cartella di lavoro. Una copia non può contenere tutto. Prima che JeffCrunch scriva il file, indica che cosa contiene la tua cartella di lavoro e il file non conserverà: grafici, immagini e altri disegni, note, tabelle pivot, tabelle, formattazioni condizionali, convalida dei dati, filtri, protezione del foglio, controlli di modulo, macro e collegamenti ad altre cartelle di lavoro (le formule che li usano mantengono i valori che avevano). Una formula che la forma meno recente non sa esprimere (una funzione che non possiede, oppure una cella oltre la colonna IV o la riga 65.536) viene scritta come il valore che risulta, e il messaggio le conta. La forma meno recente si ferma inoltre a 65.536 righe, 256 colonne, 255 fogli e 32.767 caratteri in una cella, e a circa 4.000 formati di cella e 56 colori: ciò che non entra viene tagliato, e il messaggio indica quanto. Puoi poi scegliere di **Salva copia**, di **Salva come .xlsx** per mantenere tutto, oppure di **Annulla**.

La copia non modifica la tua cartella di lavoro, e questa non viene contrassegnata come salvata. Il file che hai aperto non viene mai scritto: assegna alla copia un altro nome. Una copia di un file **.xls** o **.ods** che hai aperto può essere salvata nell'altro tipo.

## Salvare una copia

Fai clic su **File > Salva una copia** e scegli un nome. JeffCrunch scrive la cartella di lavoro in quel file, con le modifiche non ancora salvate, e tu continui a lavorare nella cartella di lavoro che avevi. Il suo nome e le sue modifiche non salvate restano come erano. Puoi scegliere qualsiasi tipo di file che offre **Salva con nome**. Una copia non viene mai scritta sopra il file della cartella di lavoro stessa.

## Conservare una copia di backup

Fai clic su **File > Opzioni**, e nella pagina **Generale** attiva **Crea sempre una copia di backup**. Ogni volta che salvi, JeffCrunch copia prima il vecchio file in uno chiamato "Backup di" seguito dal suo nome, nella stessa cartella. Se la copia non può essere creata, l'app chiede se salvare senza di essa. La risposta è No, a meno che tu non risponda Sì, e il file resta com'era.

## Salvare come modello

Un modello è una cartella di lavoro che usi come punto di partenza. Può contenere formule, stili e fogli. Fai clic su **File > Salva come modello** e digita un nome. Il modello (un file .xltx) viene collocato in **I miei modelli**, e la tua cartella di lavoro non viene modificata. In **Salva con nome** puoi anche scegliere **Modelli (*.xltx)**, oppure **Modelli di foglio di calcolo OpenDocument (*.ots)**, per salvare un file modello dove preferisci.

Per avviare una cartella di lavoro dal tuo modello, fai clic su **File > Nuovo**. I tuoi modelli sono elencati sotto **I miei modelli**. Fai clic su uno di essi. Si apre una nuova cartella di lavoro con le celle e i fogli del modello. Non ha ancora un file proprio, quindi **Salva** chiede un nome, e il modello non viene modificato. Per rimuovere un modello dall'elenco, fai clic destro su di esso e scegli **Rimuovi da I miei modelli**.

## Altri modi per aprire un file

Nella pagina **Apri** fai clic su **Altri modi per aprire**. Puoi anche fare clic destro su un nome nell'elenco dei file recenti.

- **Apri in sola lettura** apre la cartella di lavoro solo per la lettura. Puoi guardare, scorrere, cercare e copiare, ma non puoi modificare una cella. La barra del titolo riporta "Sola lettura". Se premi **Salva**, JeffCrunch chiede un nuovo nome, così il file che hai aperto non viene modificato.
- **Apri come copia** apre la cartella di lavoro senza un file proprio. **Salva** chiede dove collocarla. Il file che hai aperto non viene mai modificato.
- **Apri in una nuova finestra** apre la cartella di lavoro in una finestra propria, anche quando la finestra in cui ti trovi è vuota.
- **Apri e ripara** serve per un file .xlsx o .xls danneggiato. Vedi sotto.

## Apri e ripara

A volte un file .xlsx è danneggiato. Un download si è interrotto a metà, oppure un disco ha commesso degli errori. Un file .xlsx è un insieme di parti più piccole, come i fogli, gli stili e il testo condiviso. **Apri e ripara** legge ogni parte che riesce a leggere. Una parte interrotta a metà viene conservata fino al punto in cui si è fermata, quindi potresti ottenere le prime righe di un foglio. Una parte che non può essere letta viene esclusa. Se l'elenco delle parti o i collegamenti tra di esse sono andati persi, JeffCrunch li ricrea.

La cartella di lavoro riparata si apre senza un file proprio, con un nome come "Budget riparato". Un messaggio indica che cosa è stato conservato fino a un'interruzione e che cosa è stato escluso. Il file danneggiato non viene modificato. Salva la cartella di lavoro riparata con un nuovo nome, e verifica i numeri prima di fidarti di essi.

### Un file .xls

Anche un file .xls può essere danneggiato. Se il suo involucro esterno è troncato, **Apri e ripara** cerca nel file l'inizio della cartella di lavoro e legge da lì. Un foglio troncato conserva le celle prima del taglio. Ciò che viene dopo il taglio va perso, e te lo viene comunicato. La cartella di lavoro riparata si apre come copia, quindi salvala con un nuovo nome.

## Cartelle di lavoro di grandi dimensioni

JeffCrunch legge una cartella di lavoro in background. Le prime righe del foglio che era aperto quando la cartella di lavoro è stata salvata compaiono quasi subito, anche quando la cartella è molto grande. La barra di stato in basso a sinistra indica che cosa sta facendo: **Apertura in corso**, **Lettura** di un foglio con il numero di celle lette finora, **Calcolo** con una percentuale, e infine **Pronto**.

Puoi scorrere, selezionare e copiare mentre la lettura prosegue. Le schede degli altri fogli si attivano quando vengono letti. Una cartella di lavoro danneggiata viene aperta fino al punto in cui può essere letta, e un messaggio indica che cosa è stato escluso.

## Se un file non si apre

Quando JeffCrunch non riesce ad aprire un file, lo dice con parole semplici e lascia com'erano le cartelle di lavoro che hai aperto. Distingue un file vuoto da uno che non è affatto una cartella di lavoro (per esempio un documento PDF, un'immagine, un documento di testo o una presentazione); un pacchetto zip troncato o danneggiato, che di solito significa che un download o una copia è stata interrotta, quindi copialo di nuovo dall'originale; una cartella di lavoro protetta da password, per la quale chiede la password (vedi [Aprire cartelle di lavoro protette da password](open-password)); e un file composto che non contiene alcuna cartella di lavoro.

## Cartelle di lavoro recenti

La pagina **Apri** della schermata File elenca le cartelle di lavoro che hai aperto di recente. Fai clic destro su una di esse per **aggiungerla** in cima all'elenco oppure per rimuoverla dall'elenco.

## Salvataggio

Premi **{key:Save}**, oppure fai clic su **File** e poi su **Salva**, per salvare le modifiche nella cartella di lavoro che hai aperto. Per conservare l'originale e creare un nuovo file, fai clic su **File** e poi su **Salva con nome**, e scegli un nome. Una cartella di lavoro con macro viene salvata come file **.xlsm**, un modello come file **.xltx** o **.xltm**, e qualsiasi altra come file **.xlsx**. Un nome che termina con un altro tipo di cartella di lavoro non viene accettato, e il messaggio indica quale estensione usare; i nomi che terminano con **.xls**, **.ods**, **.csv**, **.tsv** e **.txt** creano copie, come descritto sopra.

JeffCrunch salva solo ciò che hai modificato. Tutto il resto del file, come gli altri fogli, i grafici, le immagini, le macro e le impostazioni che JeffCrunch non mostra, viene copiato com'era, byte per byte. In un foglio che hai modificato, vengono riscritte solo le celle che hai cambiato, insieme alle formule che da esse dipendono, che ottengono i loro nuovi valori. Se non hai modificato alcuna cella, il file viene lasciato in pace.

Se un altro programma ha modificato il file dopo che lo hai aperto, JeffCrunch non lo sovrascrive senza chiedere, perché perderebbe le modifiche dell'altro programma. Chiede se **Salvare una copia**, se **Sovrascrivere** il file, oppure se **Annullare**.

Il nuovo file viene scritto prima accanto a quello vecchio, e prende il posto di quest'ultimo solo quando è completo. Se qualcosa va storto, viene visualizzato un messaggio, e il file che c'era resta com'era.

Se le formule sono ancora in fase di calcolo quando salvi, JeffCrunch attende che il calcolo finisca, così il file contiene i loro risultati. Se nel frattempo premi **Esc** per interrompere il calcolo, il salvataggio viene annullato e la barra di stato lo indica; salva di nuovo quando vuoi. JeffCrunch lascia fuori dal file una cosa: l'elenco dell'ordine in cui un programma ha calcolato per ultimo le formule (la catena di calcolo). Un programma che apre la cartella di lavoro in seguito lo ricrea. Se JeffCrunch non è riuscito a calcolare alcune formule, oppure hai interrotto il calcolo, il file chiede al programma che lo apre di calcolare di nuovo tutte le formule.

## Chiusura con modifiche

Se chiudi una cartella di lavoro che ha modifiche non salvate, JeffCrunch chiede se **Salvarle**, se **Non salvare**, oppure se **Annullare** e continuare a lavorare. Un asterisco (*) nel titolo della finestra indica che ci sono modifiche non salvate.

## Modelli predefiniti

In **File > Nuovo**, sotto la cartella di lavoro vuota, ci sono cartelle di lavoro pronte all'uso, raggruppate: **Finanze** (un budget mensile, un piano di ammortamento e un registro di conto corrente), **Attività** (una fattura, una nota spese, un foglio ore, un elenco di inventario, un registro chilometri e un rapporto vendite), **Pianificazione** (un calendario e un piano di progetto) e **Istruzione** (un registro voti). Ciascuno mostra una piccola immagine della sua parte superiore. Fai clic su uno di essi per avviare una nuova cartella di lavoro a partire da esso. Digita nei riquadri gialli. Gli altri numeri vengono calcolati per te. La cartella di lavoro non ha un file proprio finché non la salvi.
