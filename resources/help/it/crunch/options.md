# Modificare le impostazioni di JeffCrunch
<!-- keywords: opzioni formule, impostazioni di calcolo, variazione massima, iterazione, recupero, recupera lavoro non salvato, recupero automatico, correzione, modifica, completamento automatico, maniglia di riempimento, trascina e rilascia, posizioni decimali, opzioni, impostazioni, preferenze, statistiche di utilizzo, numero di installazione, privacy, informativa sulla privacy, JP_NO_TELEMETRY, elimina statistiche, aggiornamenti, informazioni, modifica nella cella, colore griglia, zoom predefinito, larghezza standard, informazioni personali, personalizza tastiera, personalizza la barra multifunzione, personalizza la barra degli strumenti rapida, barra degli strumenti rapida, aggiungi un comando alla barra degli strumenti rapida, nascondi una scheda, nascondi un gruppo, aggiungi una scheda, aggiungi un gruppo, rinomina una scheda, ripristina la barra multifunzione, modifica tasti di scelta rapida, riassegna tasti, ripristina tasti -->

Le impostazioni si trovano nella finestra **Opzioni di JeffCrunch**. Fai clic su **File > Opzioni** per aprirla. Fai clic su **OK** per mantenere le modifiche oppure su **Annulla** per scartarle.

## Generale

**Mostra la Mini barra degli strumenti alla selezione** è attiva per impostazione predefinita. Stabilisce se la piccola barra di formattazione compare accanto al testo selezionato e in cima al menu di scelta rapida delle celle (vedi [Digitare nelle celle](edit-cells)). La scelta è condivisa dalle app di JeffOffice.

## Modifica

La pagina **Modifica** cambia il modo in cui si digita nelle celle.

- **Dopo aver premuto Invio, sposta la selezione** indica dove va la selezione quando completi un inserimento: in basso, a destra, in alto, a sinistra, oppure resta ferma.
- **Completa automaticamente il testo in base alle celle sopra e sotto** si chiama completamento automatico. Quando digiti le prime lettere di un testo e in quella colonna solo un testo inizia con quelle lettere, JeffCrunch mostra il resto, selezionato. Premi **Invio** per confermarlo. Continua a digitare per ignorarlo, oppure premi **Backspace** per eliminarlo. Il completamento considera solo il blocco di celle accanto a quella in cui stai scrivendo, e solo i testi, non i numeri.
- **Usa la maniglia di riempimento e trascina le celle per spostarle o copiarle** attiva il piccolo quadratino nell'angolo della selezione e il trascinamento del bordo della selezione. Disattivalo se sposti le celle per sbaglio. Copia e Incolla continuano a funzionare.
- **Mostra uno zero nelle celle che hanno valore zero (questo foglio)** attiva o disattiva la visualizzazione degli zeri per il foglio che stai guardando. Quando è disattivata, una cella che contiene 0 appare vuota, ma conserva lo 0 e nei calcoli vale 0. La scelta viene salvata con il foglio.
- **Inserisci un separatore decimale nei numeri interi digitati** si chiama posizioni decimali automatiche. Imposta **Posizioni decimali** su 2 e digitando 1234 si inserisce 12,34. Impostalo su -2 e digitando 5 si inserisce 500. Un numero digitato con il separatore decimale viene inserito così come lo scrivi.
- **Consenti modifica diretta nelle celle** è attiva per impostazione predefinita. Quando la disattivi, un inserimento digitato o il tasto F2 portano alla barra della formula, e la cella mostra il risultato solo quando completi l'inserimento.

## Opzioni formule

La pagina **Formule** stabilisce come la cartella di lavoro aperta calcola le sue formule: **Calcolo** (automatico, automatico tranne le tabelle dati, oppure manuale), se calcolare le formule prima del salvataggio della cartella quando il calcolo è manuale, se una formula che fa riferimento a se stessa viene calcolata con iterazioni, il numero massimo di iterazioni e la variazione che ne interrompe il ciclo. Queste impostazioni appartengono alla cartella di lavoro, non a te: vengono salvate nel suo file e non vengono conservate per le altre cartelle di lavoro, e le scelte sono disattivate quando non è aperta alcuna cartella. La variazione è mostrata con tutte le sue cifre, e puoi digitarla come numero, ad esempio 0,0000001 oppure 1e-7. Viene scritta solo un'impostazione che hai modificato: **OK** senza modifiche non aggiunge un passaggio di Annulla e lascia la cartella com'era. Una modifica è un singolo passaggio di Annulla. Il formato più recente della cartella e il formato .xls meno recente conservano tutte le impostazioni. Una copia .ods conserva le impostazioni di iterazione, ma non ha un posto per il calcolo manuale: la copia calcola quindi le formule man mano che cambiano, e il rapporto mostrato prima del salvataggio della copia lo indica. Le stesse impostazioni si trovano nella scheda **Formule**.

## Opzioni di salvataggio

**Salva informazioni di recupero automatico ogni** stabilisce ogni quanto JeffCrunch conserva una copia di una cartella di lavoro con modifiche non ancora salvate, da 1 a 120 minuti. La copia è un file .xlsx in una cartella che JeffCrunch usa a questo scopo, creata in background in modo che tu possa continuare a scrivere. Viene eliminata quando salvi o chiudi la cartella di lavoro. Se JeffCrunch si chiude all'improvviso, alla successiva apertura propone le copie. Puoi anche fare clic su **File > Apri > Recupera lavoro non salvato**. Una cartella di lavoro protetta da password non viene copiata, perché la copia non sarebbe protetta.

## Opzioni di correzione

La pagina **Correzione** è condivisa dalle app di JeffOffice. Stabilisce se l'ortografia viene controllata mentre digiti in una cella (disattiva **Controlla ortografia durante la digitazione** per eliminare le linee rosse ondulate in ogni finestra aperta e nelle altre app; **Ortografia** nella scheda Revisione continua a funzionare), se le parole in maiuscolo e quelle con numeri vengono ignorate nel controllo ortografico, se la correzione automatica corregge le parole mentre digiti, se le virgolette dritte diventano curve, se trattini, frazioni e numeri ordinali vengono modificati e se un indirizzo web diventa un collegamento. La scheda **Simboli matematici** di Opzioni correzione automatica serve a JeffWrite, JeffDeck e JeffPub: JeffCrunch non trasforma una barra rovesciata e un nome in un simbolo in una cella. Vedi [Ortografia](notes-links) e [Digitare nelle celle](edit-cells).

## Opzioni di visualizzazione

- **Colore griglia** è il colore delle linee tra le celle sullo schermo. È una scelta solo per il tuo schermo, non salvata nella cartella di lavoro.
- **Zoom di una nuova cartella di lavoro** indica quanto grandi appaiono i fogli di una nuova cartella di lavoro, da 10% a 400%.
- **Larghezza colonna standard (questo foglio)** è la larghezza di ogni colonna di questo foglio che non ha una larghezza propria. Coincide con **Larghezza predefinita** nella scheda Home.
- **Mostra le schede dei fogli** e **Mostra le barre di scorrimento** nascondono o mostrano la riga di schede dei fogli sotto le celle e le barre di scorrimento a destra e in basso. Sono scelte solo per il tuo schermo, mantenute per ogni cartella di lavoro che apri.
- **Mostra la nota della cella come** stabilisce come appare una cella con una nota: un piccolo segno sulla cella e la nota quando il puntatore vi si ferma sopra, solo il segno, oppure niente. Una nota che hai scelto di mostrare sempre (**Mostra nota**) resta sul foglio in ogni caso.

## Personalizza

La scheda **Personalizza** ha tre pagine: **Barra multifunzione**, **Barra degli strumenti rapida** e **Tastiera**. La pagina **Tastiera** serve a scegliere i tasti di scelta rapida dei comandi di JeffCrunch. Vedi "Scegliere i propri tasti di scelta rapida" nell'argomento sulla tastiera.

### Personalizzare la barra multifunzione

La pagina **Barra multifunzione** elenca le schede della barra, ciascuna con i suoi gruppi.

- **Nascondere o mostrare una scheda o un gruppo.** Deseleziona la casella accanto per nasconderla e selezionala di nuovo per mostrarla. Un gruppo nascosto non occupa spazio, e i suoi comandi continuano a funzionare con i tasti di scelta rapida. **Home** non ha casella, perché resta sempre, e così il pulsante **File**. Una scheda che compare solo quando è selezionato qualcosa, come un'immagine o una tabella, si trova alla fine dell'elenco e non ha casella.
- **Creare una scheda o un gruppo personale.** Digita un nome nella casella **Nome** e fai clic su **Nuova scheda**. Per creare un gruppo, scegli una scheda e fai clic su **Nuovo gruppo**. Un gruppo personale può stare in qualsiasi scheda, anche in una scheda che compare solo quando è selezionato qualcosa.
- **Inserire comandi nel tuo gruppo.** Scegli il tuo gruppo, cerca un comando nell'elenco **Comandi** (digita nella casella **Cerca comandi** per restringere l'elenco) e fai clic su **Aggiungi comando**. Può essere inserito qualsiasi comando che abbia un nome. Un comando può stare in un gruppo una sola volta.
- **Rinominare.** Scegli una scheda, un gruppo o un comando personale, digita il nuovo nome e fai clic su **Rinomina**. Un comando rinominato fa la stessa cosa, e il nuovo nome è solo ciò che mostra il suo pulsante. Per rinominare un comando con un nome vuoto e ripristinare il suo nome originale.
- **Riordinare.** Scegli una scheda e fai clic su **Sposta su** o **Sposta giù**. Puoi spostare le schede proprie della barra e le tue, ma non le schede che compaiono solo quando è selezionato qualcosa. I tuoi gruppi e i tuoi comandi si spostano allo stesso modo all'interno della scheda o del gruppo. I gruppi propri della barra restano nel loro ordine.
- **Eliminare.** Scegli una scheda, un gruppo o un comando personale e fai clic su **Rimuovi**.

**Ripristina una** riporta la scheda scelta alla situazione iniziale: viene mostrata, i suoi gruppi vengono mostrati e i gruppi che hai aggiunto spariscono. Una scheda personale viene eliminata. **Ripristina tutto** riporta l'intera barra multifunzione alla situazione iniziale, compreso l'ordine delle schede.

Nulla cambia finché non fai clic su **OK**. Poi ogni finestra aperta di JeffCrunch mostra la tua barra multifunzione, così come ogni finestra che apri in seguito. JeffCrunch conserva solo ciò che differisce dalla barra standard, quindi una scheda o un gruppo aggiunto da una versione successiva compare da solo. Le tue schede e i tuoi gruppi hanno i suggerimenti da tastiera (le lettere che compaiono quando premi **Alt**) che non entrano in conflitto con gli altri. Puoi avere 20 schede personali, 30 gruppi personali in una scheda e 60 comandi in un gruppo.

### Personalizzare la barra degli strumenti rapida

La pagina **Barra degli strumenti rapida** riguarda la piccola barra in alto a sinistra della finestra. L'elenco a sinistra mostra ciò che è presente ora nella barra, mentre l'elenco **Comandi** a destra contiene tutti i comandi che hanno un nome (digita nella casella **Cerca comandi** per restringerlo, come nella pagina **Barra multifunzione**).

- **Aggiungere un comando.** Sceglilo nell'elenco **Comandi** e fai clic su **Aggiungi comando**. Viene messo in fondo alla barra. Un comando che apre un menu mantiene il suo menu sulla barra. Un comando può stare nella barra una sola volta.
- **Clic destro.** Puoi anche fare clic destro su un comando qualsiasi della barra multifunzione e scegliere **Aggiungi alla barra degli strumenti rapida**, oppure fare clic destro su un pulsante della barra e scegliere **Rimuovi dalla barra degli strumenti rapida**. Lo stesso menu contiene **Personalizza la barra degli strumenti rapida** e **Personalizza la barra multifunzione**, che aprono questa pagina. 
- **Spostarla sotto la barra multifunzione.** Seleziona **Mostra la barra degli strumenti rapida sotto la barra multifunzione** per mettere la barra su una riga sotto la barra multifunzione invece che in alto a sinistra. I due menu di clic destro contengono anche **Mostra la barra degli strumenti rapida sotto la barra multifunzione**. JeffCrunch ricorda la scelta.
- **Aggiungere un separatore.** Fai clic su **Aggiungi separatore** per mettere una linea sottile alla fine, poi spostala dove preferisci.
- **Riordinare.** Scegli un comando o un separatore nell'elenco di sinistra e fai clic su **Sposta su** o **Sposta giù**.
- **Eliminare.** Sceglilo e fai clic su **Rimuovi**.

**Ripristina una** riporta il comando scelto dove JeffCrunch di solito lo colloca, oppure lo toglie dalla barra quando JeffCrunch di solito non lo mette lì. **Ripristina tutto** riporta la barra degli strumenti rapida a quella con cui JeffCrunch viene fornito. Ripristina tutto in questa pagina non modifica la tua barra multifunzione, e Ripristina tutto nella pagina **Barra multifunzione** non modifica la tua barra degli strumenti.

Nulla cambia finché non fai clic su **OK**. Poi ogni finestra aperta di JeffCrunch mostra la tua barra degli strumenti, così come ogni finestra che apri in seguito. La barra può contenere in tutto 100 comandi e separatori. Quando i pulsanti sono più di quanti ne entrano in un terzo della larghezza della finestra, i restanti aspettano dietro un pulsante **»** alla fine della barra, così il titolo della finestra non viene mai spinto fuori dallo schermo. JeffCrunch conserva in il tuo elenco un comando che una versione successiva non ha più, e lo mostra di nuovo se il comando ritorna. I pulsanti hanno suggerimenti da tastiera, i numeri da 1 a 9 e poi due lettere, nell'ordine in cui si trovano sulla barra. La barra si trova sempre in alto a sinistra, accanto al pulsante **File**.

## Opzioni di privacy

- **Rimuovi le informazioni personali da questo file al salvataggio** elimina dal file che viene scritto (una cartella di lavoro, una copia e una cartella di lavoro inviata per posta elettronica) l'autore, la persona che ha salvato per ultima il file, l'azienda, il responsabile e i nomi e le date delle note e dei commenti. Il contenuto del file resta com'è. Le proprietà personalizzate del file vengono lasciate così come sono, e il documento aperto non viene modificato. È la stessa scelta in ogni app di JeffOffice: modificala in una e cambia in tutte. Per farlo una sola volta, usa **Ispeziona cartella di lavoro** in [Informazioni](info). La stessa casella si trova nella pagina **Informazioni**.

## Statistiche di utilizzo

**Invia statistiche di utilizzo anonime** ti permette di condividere dati su come viene usato JeffCrunch. Anonime significa che i dati non dicono nulla su di te. Quando è attiva, JeffCrunch invia questi dati una volta al giorno, e nel giorno in cui aggiorni: che si tratta di JeffCrunch, la sua versione, il sistema operativo e la lingua, quante volte è stato avviato, con quale frequenza viene usato ogni comando e che tipo di file hai aperto o salvato. Se JeffCrunch si chiude all'improvviso, alla successiva apertura invia anche una segnalazione di arresto anomalo. JeffCrunch non invia mai i tuoi file, i loro nomi o il loro contenuto, né quello che digiti. Se l'opzione è disattivata, non viene inviata nemmeno alcuna segnalazione di arresto anomalo.

La scelta vale per tutte le app di JeffOffice: modificala qui e cambia in ciascuna di esse. JeffCrunch la chiede alla prima apertura, nella finestra **Aiutaci a migliorare JeffCrunch**.

- **Informativa sulla privacy** accanto alla casella apre la nostra informativa all'indirizzo jeffoffice.com/privacy. Spiega cosa viene inviato, dove va e per quanto tempo lo conserviamo. Conserviamo le statistiche per 24 mesi.
- **Numero di installazione** è mostrato sotto la casella. È un numero casuale generato per la tua copia di JeffOffice, uguale in ogni app di JeffOffice. Non dice nulla su di te. Fai clic su **Copia** per copiarlo. Se le statistiche sono disattivate, o se non è ancora stato generato alcun numero, mostra **nessuno**.
- Per far eliminare le statistiche che già conserviamo, invia il tuo numero di installazione a info@jeffoffice.com **prima** di disattivare le statistiche. Disattivarle cancella il numero dal tuo computer e, senza di esso, non possiamo capire quali registrazioni sono tue.
- Per disattivare le statistiche senza aprire JeffCrunch, imposta la variabile d'ambiente `JP_NO_TELEMETRY` su un valore qualsiasi prima di avviarlo. Allora non viene conteggiato né inviato nulla, e JeffCrunch non pone la domanda della prima apertura.

## Sicurezza

La scheda **Sicurezza** stabilisce cosa fa JeffCrunch con ciò a cui un file può puntare: file collegati, filmati e suoni collegati, e file provenienti da Internet. Le macro sono conservate in un file e non vengono mai eseguite, e un collegamento che avvierebbe un programma viene sempre rifiutato. Vedi [Sicurezza](security).

## Argomenti correlati

- [Benvenuti in JeffCrunch](start)
