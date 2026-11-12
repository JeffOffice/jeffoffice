# Apertura e salvataggio di cartelle di lavoro
<!-- keywords: recupero lavoro non salvato, recupero automatico, arresto anomalo, copia, formato meno recente, opendocument, compatibilità, nuovo, vuoto, libro1, senza titolo, apri, salva, salva con nome, file, cartella di lavoro, xlsx, xlsm, xls, ods, csv, tsv, file di testo, delimitato, importa, esporta, recenti, aggiunti, trascina, rilascia, caricamento, grande, grandi, macro, non salvato, modifiche, chiudi, copia, catena di calcolo, modello, xltx, xltm, impossibile aprire, danneggiato, cifrato, password, salva una copia, sola lettura, apri in sola lettura, apri come copia, nuova finestra, copia di backup, modelli personali, salva come modello, ots, ripara, apri e ripara, modelli predefiniti, budget, fattura, foglio ore, prestito, calendario, registro voti, larghezza fissa, colonne fisse, separatore decimale, separatore delle migliaia, segno meno finale, html, pagina web, htm, tabelle da una pagina -->

Quando JeffCrunch si avvia senza un file da aprire, la finestra contiene una nuova cartella di lavoro vuota con un solo foglio, chiamata Libro1. Ogni nuova cartella di lavoro riceve il numero successivo (Libro2, Libro3, e così via), e un numero non viene riutilizzato dopo la chiusura della sua finestra. Per avviarne un'altra, premi **{key:New}**, oppure fai clic su **File**, poi su **Nuovo**, e scegli **Cartella di lavoro vuota**: si apre in una finestra propria. Una nuova cartella di lavoro che non hai modificato lascia il posto quando apri un file nella sua finestra; una che hai modificato resta, e il file si apre in un'altra finestra. **Salva** su una nuova cartella di lavoro chiede un nome, come fa **Salva con nome**, e la salva come file .xlsx. Chiudere una nuova cartella di lavoro non modificata non chiede nulla.

Per aprire una cartella di lavoro, premi **{key:Open}**, oppure fai clic su **File** e poi su **Apri**, e scegli **Sfoglia**. Puoi anche trascinare una cartella di lavoro dal gestore dei file sulla finestra. Una cartella di lavoro già aperta in una finestra non viene aperta una seconda volta: quella finestra passa in primo piano.

JeffCrunch apre le cartelle di lavoro **.xlsx** e **.xlsm** (quelle che contengono macro), e i modelli **.xltx** e **.xltm** (un modello che contiene macro). Non esegue le macro e le lascia intatte: il progetto di macro, la sua firma e le parti che le indicano vengono riscritte esattamente come erano. Un modello viene salvato come modello, e una cartella di lavoro con macro come cartella di lavoro con macro, perché il tipo fa parte del file.

## Recupero del lavoro non salvato

Se JeffCrunch si chiude all'improvviso, per esempio per un'interruzione di corrente, al successivo avvio compare un elenco delle copie che ha conservato del tuo lavoro non salvato. Scegli quali aprire e fai clic su **Apri**; ciascuna si apre come nuova cartella di lavoro senza ancora un file, quindi fai clic su **Salva** e scegli un nome. **Elimina** cancella le copie selezionate, e **Non ora** le lascia per più tardi. Puoi richiedere l'elenco in qualsiasi momento con **File > Apri > Recupera lavoro non salvato**. Vedi [Modificare le impostazioni di JeffCrunch](options) per sapere con quale frequenza vengono create le copie.

## Altri tipi di file

JeffCrunch apre anche le cartelle di lavoro **.xls** meno recenti, i fogli di calcolo OpenDocument (**.ods** e il formato piatto **.fods**) e i file di testo con valori (**.csv**, **.tsv** e **.txt**). Una cartella di lavoro aperta da uno di questi file viene salvata come nuovo file **.xlsx** (oppure come copia **.xls** o **.ods**: vedi sotto): quando premi **{key:Save}**, JeffCrunch te lo segnala e chiede un nome, e il file aperto non viene mai modificato. Le immagini, le forme, le caselle di testo, i gruppi, le linee e il Testo artistico di un file **.ods** passano nel nuovo file con posizione, dimensioni, rotazioni, ordine, nomi, testo alternativo, riempimenti e contorni; i suoi grafici vengono trasferiti con essi, disegnati a partire dalle celle che leggono.

Un foglio di calcolo OpenDocument viene letto con fogli, valori, formule (con i risultati conservati nel file), formati numerici, tipi di carattere, riempimenti, bordi, celle unite, larghezze di colonna e altezze di riga, righe e colonne nascoste, riquadri bloccati e nomi. Grafici, immagini e altri disegni, commenti, tabelle pivot, macro, formattazione condizionale e convalida dei dati non sono ancora letti: un messaggio in alto indica quali di questi elementi contiene il file, e una copia salvata da qui non li avrà. Un foglio di calcolo protetto da password chiede la password all'apertura: vedi [Aprire cartelle di lavoro protette da password](open-password).

Un file di testo corrisponde a un solo foglio. JeffCrunch stabilisce come sono separati i suoi valori (da virgole, punti e virgola, tabulazioni o barre verticali) e come sono codificate le lettere (UTF-8 o UTF-16, oppure la tabella codici della tua lingua), e legge ogni valore come lo digiteresti nella tua lingua: un numero come 1.234,5 in tedesco, una percentuale, un importo in valuta, una data con l'anno (come 15/3/2024, oppure 15.03.2024 in tedesco), un orario, VERO o FALSO (nella tua lingua, come WAHR e FALSCH in tedesco, e in inglese). Un valore che inizia con il segno di uguale è una formula. Le cifre che iniziano con uno zero (come 007) e i numeri di più di 15 cifre restano testo, così un codice postale o un numero di conto non viene modificato; restano testo anche un giorno e un mese senza anno, in qualsiasi ordine e in qualsiasi lingua (come 3/4, 12.5, 1-2, 1 marzo o 25 dic), perché l'anno dovrebbe essere inventato, e testi come il nome del gene MARCH1. Un foglio contiene 1.048.576 righe e 16.384 colonne; se il file ne ha di più, JeffCrunch apre ciò che ci sta e te lo segnala.

Quando apri un file di testo, una finestra mostra prima le sue prime righe e chiede come va letto: che cosa separa i campi (una qualsiasi tra virgola, punto e virgola, tabulazione, spazio, barra verticale o un carattere che digiti, con la possibilità di considerare più separatori consecutivi come uno solo), che cosa racchiude un campo che contiene un separatore, la codifica del testo (Unicode, le tabelle codici di Windows, le pagine ISO 8859, Mac Roman e quelle dell'Asia orientale), e la riga da cui iniziare. Sopra ogni colonna scegli come va letta: **Come digitato**, **Testo** (per conservare un codice come 007 o un numero di conto lungo così com'è), una **Data** nell'ordine che indichi, oppure **Escludi**. Alcuni file allineano i valori in colonne della stessa larghezza, con spazi tra di esse e senza alcun separatore. Seleziona **Le colonne hanno una larghezza fissa**, e le prime righe del file compaiono in un riquadro. Fai clic tra due caratteri per far iniziare una colonna in quel punto, e fai di nuovo clic per eliminarla; oppure digita le posizioni in **Le colonne iniziano a**, per esempio 10, 25, 40. Ogni parte viene tagliata in quei punti, e gli spazi intorno vengono eliminati. **Separatore decimale** e **Separatore delle migliaia** indicano a JeffCrunch come sono scritti i numeri in questo file, indipendentemente dalla lingua che usi tu, così 1.234,50 vale milleduecentotrentaquattro virgola cinque quando il file usa la virgola come separatore decimale. Seleziona **Un segno meno può essere posto dopo il numero** per i file che scrivono -12 come 12-. L'anteprima segue ogni modifica e mostra al massimo le prime 200 righe e 100 colonne, ma l'intero file viene aperto. **File > Apri > Apri file di testo** chiede anche quando il nome del file è insolito.

JeffCrunch apre anche le **pagine web** (**.html** e **.htm**) e una pagina salvata con il nome di una cartella di lavoro. Ogni tabella della pagina diventa un foglio, chiamato **Tabella1**, **Tabella2**, e così via. Le celle vengono lette come se le avessi digitate: un numero è un numero, e una data è una data. Una cella unita su più colonne o righe lascia celle vuote, così le altre celle restano nelle loro colonne. Le parole fuori dalle tabelle, le immagini e l'aspetto della pagina non vengono letti, e una pagina senza tabelle non si apre. Nulla di ciò che la pagina indica viene scaricato da internet.

Per conservare solo i valori del foglio che stai visualizzando, fai clic su **File**, poi su **Salva con nome**, e scegli il tipo di file **.csv** (oppure **.tsv** e **.txt**, con tabulazioni). Prima di scrivere il file, JeffCrunch indica che cosa contiene la tua cartella di lavoro e che il file non conserverà (per esempio gli altri fogli, le formule, le immagini, i grafici, le note e le macro), e chiede come scriverlo, se **Salva solo valori**, se **Salva come .xlsx** per conservare tutto, oppure **Annulla**:

- **Separatore.** Una virgola, un punto e virgola, una tabulazione, uno spazio, una barra verticale o un carattere che digiti. Un **.csv** inizia con il punto e virgola dove la tua lingua scrive la virgola nei numeri, e con la virgola altrove; un **.tsv** o **.txt** con la tabulazione.
- **Codifica del testo.** UTF-8 con BOM (così gli altri programmi leggono correttamente le lettere accentate), UTF-8 senza BOM, UTF-16, oppure una tabella codici come Europa occidentale (1252), le pagine ISO 8859, Mac Roman, o quelle giapponese, cinese, coreana, ebraica, araba, baltica, vietnamita e thai. Una lettera per cui la tabella codici non ha un byte viene scritta come punto interrogativo, e la barra di stato indica quante sono.
- **Fine riga.** Windows (ritorno a capo e avanzamento riga), Linux e Mac (avanzamento riga), oppure Mac precedenti (ritorno a capo).
- **Celle.** **Come visualizzate nelle celle** scrive ciò che vedi. **Come memorizzate** scrive ogni cifra di un numero (0,1 e non 10%), e una data come testo ISO, per esempio 2026-03-04.

Un testo viene racchiuso tra virgolette quando contiene il separatore, una virgoletta, un'interruzione di riga, o uno spazio a una delle due estremità. Gli altri fogli, le formule, i formati e le impostazioni non sono nel file, e la cartella di lavoro non viene modificata scrivendone uno.

Una cartella di lavoro proveniente da un file di testo viene riscritta nella forma che il file aveva: stesso separatore, stesse fini riga e stessa codifica. Se la salvi come copia senza aver modificato alcuna cella, la copia è lo stesso file, byte per byte, e non viene chiesto nulla.

## Salvare una copia come .xls o .ods

Per creare una copia che programmi meno recenti, o programmi che leggono OpenDocument, possano aprire, fai clic su **File**, poi su **Salva con nome**, e scegli il tipo di file **.xls** o **.ods**. La copia viene creata dalla tua cartella di lavoro così com'è ora, con le modifiche non ancora salvate, e contiene:

- i valori, con i loro testi, numeri, valori logici ed errori;
- le formule, con i risultati che hanno ora (una formula matriciale resta una formula matriciale);
- i formati numerici, i tipi di carattere, i riempimenti, i bordi, l'allineamento e se una cella è bloccata;
- le celle unite, le larghezze di colonna e le altezze di riga, le righe e le colonne nascoste, i raggruppamenti di righe e colonne, e i riquadri bloccati;
- i fogli, nel loro ordine, con i nomi e l'indicazione di quali sono nascosti, e i nomi che hai definito (e le aree di stampa);
- i grafici di ciascun foglio (solo in una copia **.ods**): colonne, barre, linee, aree, torte, ciambelle e grafici a dispersione, con le loro serie e le celle che leggono, le categorie, i titoli, la legenda, gli assi (titoli, scala, formato numerico), le etichette dei dati, i colori e la posizione (ciò che un grafico **.ods** non può rappresentare, come linee di tendenza, barre di errore e assi secondari, viene indicato nel messaggio);
- le immagini, le forme, le caselle di testo, i gruppi, le linee e il Testo artistico di ciascun foglio (solo in una copia **.ods**), con posizione, dimensioni, rotazioni, ordine, nomi, testo alternativo, riempimenti, contorni, testo e le immagini stesse con i loro ritagli (il Testo artistico viene salvato come forma di testo ad effetto prevista da OpenDocument);
- i collegamenti, e il modo in cui stampa ciascun foglio (carta, margini, orientamento, scala, intestazioni e piè di pagina, e interruzioni di pagina).

Un file **.xls** contiene anche le tabelle dei dati e conserva le impostazioni di calcolo della cartella di lavoro. Una copia non può contenere tutto. Prima che JeffCrunch scriva il file, indica che cosa contiene la tua cartella di lavoro e il file non conserverà: grafici, immagini e altri disegni (per una copia **.xls**; una copia **.ods** li conserva, grafici compresi, tranne i diagrammi), note, tabelle pivot, tabelle, formattazione condizionale, convalida dei dati, filtri, protezione dei fogli, controlli modulo, macro e collegamenti ad altre cartelle di lavoro (le formule che li usano conservano i valori che avevano). Una formula che la forma meno recente non sa esprimere (una funzione che non possiede, oppure una cella oltre la colonna IV o la riga 65.536) viene scritta come il valore che ha dato, e il messaggio ne conta il numero. La forma meno recente si ferma anche a 65.536 righe, 256 colonne, 255 fogli e 32.767 caratteri in una cella, e a circa 4.000 formati di cella e 56 colori: ciò che non ci sta viene tagliato, e il messaggio indica quanto. Puoi poi scegliere di **Salva copia**, di **Salva come .xlsx** per conservare tutto, oppure di **Annulla**.

La copia non modifica la tua cartella di lavoro, e questa non viene contrassegnata come salvata. Il file aperto non viene mai scritto: assegna alla copia un altro nome. Una copia di un file **.xls** o **.ods** che hai aperto può essere salvata nell'altro tipo.

## Salvare una copia

Fai clic su **File > Salva una copia** e scegli un nome. JeffCrunch scrive la cartella di lavoro in quel file, con le modifiche non ancora salvate, e tu continui a lavorare nella cartella che avevi. Il suo nome e le sue modifiche non salvate restano come erano. Puoi scegliere qualsiasi tipo di file che offre **Salva con nome**. Una copia non viene mai scritta sopra il file della cartella di lavoro.

## Conservare una copia di backup

Fai clic su **File > Opzioni**, e nella pagina **Generale** attiva **Crea sempre una copia di backup**. A ogni salvataggio, JeffCrunch copia prima il vecchio file in uno chiamato "Backup di" seguito dal suo nome, nella stessa cartella. Se la copia non può essere creata, l'app chiede se salvare senza di essa. La risposta è No, a meno che tu non dica Sì, e il file resta com'era.

## Salvare come modello

Un modello è una cartella di lavoro che usi come punto di partenza. Può contenere formule, stili e fogli. Fai clic su **File > Salva come modello** e digita un nome. Il modello (un file **.xltx**) viene messo in **Modelli personali**, e la tua cartella di lavoro non viene modificata. In **Salva con nome** puoi anche scegliere **Modelli (*.xltx)**, oppure **Modelli di foglio di calcolo OpenDocument (*.ots)**, per salvare un file modello dove preferisci.

Per avviare una cartella di lavoro dal tuo modello, fai clic su **File > Nuovo**. I tuoi modelli sono elencati sotto **Modelli personali**. Fai clic su uno di essi. Si apre una nuova cartella di lavoro con le celle e i fogli del modello. Non ha ancora un file proprio, quindi **Salva** chiede un nome, e il modello non viene modificato. Per rimuovere un modello dall'elenco, fai clic con il pulsante destro su di esso e scegli **Rimuovi da Modelli personali**.

## Altri modi per aprire un file

Nella pagina **Apri**, fai clic su **Altri modi per aprire**. Puoi anche fare clic con il pulsante destro su un nome nell'elenco dei recenti.

- **Apri in sola lettura** apre la cartella di lavoro solo per la lettura. Puoi consultare, scorrere, cercare e copiare, ma non puoi modificare una cella. La barra del titolo indica "Sola lettura". Se premi **Salva**, JeffCrunch chiede un nuovo nome, così il file aperto non viene modificato.
- **Apri come copia** apre la cartella di lavoro senza un file proprio. **Salva** chiede dove collocarla. Il file aperto non viene mai modificato.
- **Apri in una nuova finestra** apre la cartella di lavoro in una finestra propria, anche quando la finestra in cui ti trovi è vuota.
- **Apri e ripara** serve per un file .xlsx o .xls danneggiato. Vedi sotto.

## Apri e ripara

A volte un file .xlsx è danneggiato. Un download si è interrotto a metà, oppure un disco ha commesso degli errori. Un file .xlsx è un insieme di parti più piccole, come i fogli, gli stili e il testo condiviso. **Apri e ripara** legge ogni parte che riesce a leggere. Una parte interrotta a metà viene conservata fino al punto in cui si è fermata, quindi potresti ottenere le prime righe di un foglio. Una parte che non può essere letta viene esclusa. Se l'elenco delle parti o i collegamenti tra di esse sono andati persi, JeffCrunch li ricostruisce.

La cartella di lavoro riparata si apre senza un file proprio, con un nome come "Bilancio riparato". Un messaggio indica che cosa è stato conservato fino a un'interruzione e che cosa è stato escluso. Il file danneggiato non viene modificato. Salva la cartella di lavoro riparata con un nuovo nome, e verifica i numeri prima di fidarti di essi.

### Un file .xls

Anche un file .xls può essere danneggiato. Se il suo involucro esterno è troncato, **Apri e ripara** cerca nel file l'inizio della cartella di lavoro e legge da lì. Un foglio troncato conserva le celle prima del taglio. Tutto ciò che viene dopo il taglio è perso, e te lo viene detto. La cartella di lavoro riparata si apre come copia, quindi salvala con un nuovo nome.

## Cartelle di lavoro grandi

JeffCrunch legge una cartella di lavoro in background. Le prime righe del foglio che era aperto quando la cartella è stata salvata compaiono quasi subito, anche quando la cartella è molto grande. La barra di stato in basso a sinistra indica che cosa sta facendo: **Apertura**, **Lettura** di un foglio con il numero di celle lette finora, **Calcolo** con una percentuale, e poi **Pronto**.

Puoi scorrere, selezionare e copiare mentre la lettura prosegue. Le schede degli altri fogli si attivano quando vengono letti. Una cartella di lavoro danneggiata viene aperta fin dove può essere letta, e un messaggio indica che cosa è stato escluso.

## Se un file non si apre

Quando JeffCrunch non riesce ad aprire un file, lo dice con parole semplici e lascia le cartelle di lavoro aperte come sono. Distingue un file vuoto da uno che non è affatto una cartella di lavoro (per esempio un documento PDF, un'immagine, un documento di testo o una presentazione); un pacchetto zip troncato o danneggiato, che di solito significa che un download o una copia si è interrotta, quindi copialo di nuovo dall'originale; una cartella di lavoro protetta da password, per la quale chiede la password (vedi [Aprire cartelle di lavoro protette da password](open-password)); e un file composto che non contiene alcuna cartella di lavoro.

## Cartelle di lavoro recenti

La pagina **Apri** della schermata File elenca le cartelle di lavoro che hai aperto di recente. Fai clic con il pulsante destro su una di esse per **aggiungerla** in cima all'elenco o per rimuoverla dall'elenco.

## Salvataggio

Premi **{key:Save}**, oppure fai clic su **File** e poi su **Salva**, per salvare le modifiche nella cartella di lavoro aperta. Per conservare l'originale e creare un nuovo file, fai clic su **File** e poi su **Salva con nome**, e scegli un nome. Una cartella di lavoro con macro viene salvata come file **.xlsm**, un modello come file **.xltx** o **.xltm**, e ogni altra come file **.xlsx**. Un nome che termina con un altro tipo di cartella di lavoro non viene accettato, e il messaggio indica quale estensione usare; i nomi che terminano con **.xls**, **.ods**, **.csv**, **.tsv** e **.txt** creano copie, come descritto sopra.

JeffCrunch salva solo ciò che hai modificato. Tutto il resto del file, come gli altri fogli, i grafici, le immagini, le macro e le impostazioni che JeffCrunch non mostra, viene copiato com'era, byte per byte. In un foglio modificato, vengono riscritte solo le celle che hai cambiato, insieme alle formule che dipendono da esse, che ottengono i nuovi valori. Se non hai modificato alcuna cella, il file viene lasciato intatto.

Se un altro programma ha modificato il file dopo che lo hai aperto, JeffCrunch non lo sovrascrive senza chiedere, perché così andrebbero perse le modifiche dell'altro programma. Chiede se **Salvare una copia**, se **Sovrascrivere** il file, oppure se **Annullare**.

Il nuovo file viene scritto prima accanto a quello vecchio, e prende il posto di quello vecchio solo quando è completo. Se qualcosa va storto, vedi un messaggio, e il file che c'era resta com'era.

Se le formule sono ancora in fase di calcolo quando salvi, JeffCrunch attende che il calcolo finisca, così il file contiene i loro risultati. Se nel frattempo premi **Esc** per interrompere il calcolo, il salvataggio viene annullato, e la barra di stato lo segnala; salva di nuovo quando vuoi. JeffCrunch lascia fuori dal file una cosa: l'elenco dell'ordine in cui un programma ha calcolato per ultimo le formule (la catena di calcolo). Un programma che apre la cartella di lavoro successivamente lo ricostruisce. Se JeffCrunch non è riuscito a calcolare alcune formule, o hai interrotto il calcolo, il file chiede al programma che lo apre di calcolare di nuovo tutte le formule.

## Chiusura con modifiche

Se chiudi una cartella di lavoro che ha modifiche non salvate, JeffCrunch chiede se **Salvare** le modifiche, se **Non salvare**, oppure se **Annullare** e continuare a lavorare. Un asterisco (*) nel titolo della finestra indica che ci sono modifiche non salvate.

## Modelli predefiniti

In **File > Nuovo**, sotto la cartella di lavoro vuota, ci sono cartelle di lavoro già pronte, raggruppate: **Finanza** (un budget mensile, un piano di ammortamento e un registro di conto corrente), **Attività** (una fattura, una nota spese, un foglio ore, un elenco di inventario, un registro chilometri e un rapporto vendite), **Pianificazione** (un calendario e un piano di progetto) e **Istruzione** (un registro voti). Ciascuna mostra una piccola immagine della sua parte superiore. Fai clic su una per avviare una nuova cartella di lavoro a partire da essa. Scrivi nelle caselle gialle. Gli altri numeri vengono calcolati per te. La cartella di lavoro non ha un file proprio finché non la salvi.
