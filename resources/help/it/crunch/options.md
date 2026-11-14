# Modificare le impostazioni di JeffCrunch

<!-- keywords: recupero, recupera lavoro non salvato, recupero automatico, correzione, modifica, completamento automatico, maniglia di riempimento, trascina e rilascia, cifre decimali, opzioni, impostazioni, preferenze, statistiche di utilizzo, numero di installazione, privacy, informativa sulla privacy, JP_NO_TELEMETRY, elimina statistiche, aggiornamenti, informazioni, modifica nella cella, colore griglia, zoom predefinito, larghezza standard, informazioni personali, personalizza tastiera, personalizza la barra multifunzione, personalizza la barra degli strumenti rapida, barra degli strumenti rapida, aggiungi un comando alla barra degli strumenti rapida, nascondi una scheda, nascondi un gruppo, aggiungi una scheda, aggiungi un gruppo, rinomina una scheda, reimposta la barra multifunzione, cambia tasti di scelta rapida, riassegna tasti, reimposta tasti -->

Le impostazioni si trovano nella finestra **Opzioni di JeffCrunch**. Fai clic su **File > Opzioni** per aprirla. Fai clic su **OK** per mantenere le modifiche oppure su **Annulla** per scartarle.

## Modifica

La pagina **Modifica** cambia il modo in cui funziona la digitazione nelle celle.

- **Dopo aver premuto Invio, sposta la selezione** indica dove si sposta la selezione al termine di una voce: in basso, a destra, in alto, a sinistra, oppure non si sposta.
- **Suggerisci il completamento del testo dalle celle sopra e sotto** si chiama completamento automatico. Quando digiti le prime lettere di un testo e in quella colonna un solo testo inizia con esse, JeffCrunch mostra il resto, selezionato. Premi **Invio** per accettarlo. Continua a digitare per ignorarlo, oppure premi **Backspace** per eliminarlo. Il completamento considera solo il blocco di celle accanto a quella in cui stai scrivendo, e solo i testi, non i numeri.
- **Usa la maniglia di riempimento e trascina le celle per spostarle o copiarle** attiva il piccolo quadratino nell'angolo della selezione e il trascinamento del bordo della selezione. Disattivalo se sposti le celle per errore. Copia e Incolla continuano a funzionare.
- **Mostra uno zero nelle celle che hanno valore zero (questo foglio)** attiva o disattiva la visualizzazione degli zeri nel foglio che stai guardando. Quando è disattivata, una cella che contiene 0 appare vuota, ma contiene ancora lo 0 e viene sommata come 0. La scelta viene salvata con il foglio.
- **Inserisci un separatore decimale nei numeri interi digitati** si chiama posizioni decimali automatiche. Imposta **Posizioni decimali** su 2 e digitando 1234 si inserisce 12,34. Impostalo su -2 e digitando 5 si inserisce 500. Un numero digitato con il separatore decimale viene inserito così come lo scrivi.
- **Consenti la modifica direttamente nelle celle** è attiva per impostazione predefinita. Quando la disattivi, una voce digitata o F2 va nella barra della formula e la cella mostra il risultato solo al termine della modifica.

## Opzioni di salvataggio

**Salva informazioni di recupero automatico ogni** imposta ogni quanto JeffCrunch conserva una copia di una cartella di lavoro con modifiche non ancora salvate, da 1 a 120 minuti. La copia è un file .xlsx in una cartella che JeffCrunch riserva a questo scopo, creata in background così puoi continuare a digitare. La copia viene eliminata quando salvi o chiudi la cartella di lavoro. Se JeffCrunch si chiude inaspettatamente, alla successiva apertura propone le copie. Puoi anche fare clic su **File > Apri > Recupera lavoro non salvato**. Una cartella di lavoro protetta con password non viene copiata, perché la copia non sarebbe protetta.

## Opzioni di correzione

La pagina **Strumenti di correzione** è condivisa dalle app di JeffOffice. Stabilisce se l'ortografia viene controllata durante la digitazione in una cella (disattiva **Controlla ortografia durante la digitazione** per eliminare le sottolineature rosse ondulate in tutte le finestre aperte e nelle altre app; **Controllo ortografia** nella scheda Revisione continua a funzionare), se le parole in maiuscolo e le parole con numeri vengono ignorate durante il controllo, se la Correzione automatica corregge le parole mentre digiti, se le virgolette dritte diventano curve, se trattini, frazioni e ordinali vengono modificati e se un indirizzo web diventa un collegamento. Vedi [Controllo ortografia](notes-links) e [Digitare nelle celle](edit-cells).

## Opzioni di visualizzazione

- **Colore griglia** è il colore delle linee tra le celle sullo schermo. È una scelta solo per il tuo schermo, non viene salvata nella cartella di lavoro.
- **Zoom di una nuova cartella di lavoro** è la dimensione con cui appaiono i fogli di una nuova cartella di lavoro, da 10% a 400%.
- **Larghezza colonna standard (questo foglio)** è la larghezza di ogni colonna di questo foglio che non ha una larghezza propria. Corrisponde a **Larghezza predefinita** nella scheda Home.

## Personalizza

La scheda **Personalizza** ha tre pagine: **Barra multifunzione**, **Barra degli strumenti rapida** e **Tastiera**. La pagina **Tastiera** serve a scegliere i tasti di scelta rapida dei comandi di JeffCrunch. Vedi "Scegliere i propri tasti di scelta rapida" nell'argomento sulla tastiera.

### Personalizzare la barra multifunzione

La pagina **Barra multifunzione** elenca le schede della barra, ciascuna con i suoi gruppi.

- **Nascondere o mostrare una scheda o un gruppo.** Deseleziona la casella accanto per nasconderlo e selezionala per mostrarlo di nuovo. Un gruppo nascosto non occupa spazio, e i suoi comandi continuano a funzionare con i tasti di scelta rapida. **Home** non ha la casella, perché resta sempre visibile, e così il pulsante **File**. Una scheda che compare solo quando è selezionato qualcosa, come un'immagine o una tabella, si trova alla fine dell'elenco e non ha la casella.
- **Creare una scheda o un gruppo personale.** Digita un nome nella casella **Nome** e fai clic su **Nuova scheda**. Per creare un gruppo, scegli una scheda e fai clic su **Nuovo gruppo**. Un gruppo personale può essere inserito in qualsiasi scheda, anche in una scheda che compare solo quando è selezionato qualcosa.
- **Inserire comandi nel proprio gruppo.** Scegli il gruppo, trova un comando nell'elenco **Comandi** (digita nella casella **Cerca comandi** per restringere l'elenco) e fai clic su **Aggiungi comando**. Può essere inserito qualsiasi comando che ha un nome. Un comando può trovarsi in un gruppo una sola volta.
- **Rinominare.** Scegli una scheda, un gruppo o un comando personale, digita il nuovo nome e fai clic su **Rinomina**. Un comando rinominato fa la stessa cosa di prima, e il nuovo nome è solo il testo del suo pulsante. Per ripristinare il nome originale di un comando, rinominalo con un nome vuoto.
- **Riordinare.** Scegli una scheda e fai clic su **Sposta su** o **Sposta giù**. Puoi spostare sia le schede della barra sia quelle personali, ma non le schede che compaiono solo quando è selezionato qualcosa. I tuoi gruppi e i tuoi comandi si spostano allo stesso modo all'interno della propria scheda o del proprio gruppo. I gruppi della barra mantengono il loro ordine.
- **Eliminare.** Scegli una scheda, un gruppo o un comando personale e fai clic su **Rimuovi**.

**Reimposta uno** ripristina la scheda scelta: viene mostrata, i suoi gruppi vengono mostrati e i gruppi che le hai aggiunto spariscono. Una scheda personale viene eliminata. **Reimposta tutto** ripristina l'intera barra, compreso l'ordine delle schede.

Non cambia nulla finché non fai clic su **OK**. Da quel momento ogni finestra di JeffCrunch aperta mostra la tua barra, e così ogni finestra che aprirai in seguito. JeffCrunch conserva solo ciò che differisce dalla barra standard, quindi una scheda o un gruppo che una versione successiva aggiunge compare autonomamente. Le tue schede e i tuoi gruppi ricevono suggerimenti da tastiera (le lettere che compaiono premendo **Alt**) che non si sovrappongono agli altri. Puoi avere 20 schede personali, 30 gruppi personali per scheda e 60 comandi per gruppo.

### Personalizzare la barra degli strumenti rapida

La pagina **Barra degli strumenti rapida** serve per la piccola barra in alto a sinistra della finestra. L'elenco a sinistra mostra ciò che c'è ora nella barra, mentre l'elenco **Comandi** a destra contiene tutti i comandi che hanno un nome (digita nella casella **Cerca comandi** per restringerlo, come nella pagina **Barra multifunzione**).

- **Aggiungere un comando.** Scegli il comando nell'elenco **Comandi** e fai clic su **Aggiungi comando**. Viene messo alla fine della barra. Un comando che apre un menu mantiene il suo menu nella barra. Un comando può trovarsi nella barra una sola volta.
- **Clic destro.** Puoi anche fare clic destro su un comando della barra multifunzione e scegliere **Aggiungi alla barra degli strumenti rapida**, oppure fare clic destro su un pulsante della barra degli strumenti rapida e scegliere **Rimuovi dalla barra degli strumenti rapida**. Lo stesso menu contiene **Personalizza la barra degli strumenti rapida** e **Personalizza la barra multifunzione**, che aprono questa pagina.
- **Spostarla sotto la barra multifunzione.** Seleziona **Mostra la barra degli strumenti rapida sotto la barra multifunzione** per sistemare la barra su una riga sotto la barra multifunzione invece che in alto a sinistra. Anche i due menu del clic destro contengono **Mostra la barra degli strumenti rapida sotto la barra multifunzione**. JeffCrunch ricorda la scelta.
- **Aggiungere un separatore.** Fai clic su **Aggiungi separatore** per inserire una sottile linea alla fine, quindi spostala dove preferisci.
- **Riordinare.** Scegli un comando o un separatore nell'elenco di sinistra e fai clic su **Sposta su** o **Sposta giù**.
- **Eliminare.** Scegli l'elemento e fai clic su **Rimuovi**.

**Reimposta uno** ripristina il comando scelto: nella posizione in cui JeffCrunch di solito lo colloca, oppure fuori dalla barra se di norma JeffCrunch non lo mette lì. **Reimposta tutto** ripristina la barra degli strumenti rapida predefinita di JeffCrunch. Reimposta tutto in questa pagina non modifica la barra multifunzione, e Reimposta tutto nella pagina **Barra multifunzione** non modifica la barra degli strumenti rapida.

Non cambia nulla finché non fai clic su **OK**. Da quel momento ogni finestra di JeffCrunch aperta mostra la tua barra, e così ogni finestra che aprirai in seguito. La barra può contenere in tutto 100 comandi e separatori. Quando i pulsanti sono più di quanti ne entrano in un terzo della larghezza della finestra, i restanti si trovano dietro un pulsante **»** alla fine della barra, così il titolo della finestra non viene mai spinto fuori dallo schermo. JeffCrunch conserva in elenco un comando anche quando una versione successiva non lo ha più, e lo mostra di nuovo se il comando ritorna. I pulsanti ricevono suggerimenti da tastiera: i numeri da 1 a 9 e poi due lettere, nell'ordine in cui si trovano nella barra. La barra si trova sempre in alto a sinistra, accanto al pulsante **File**.

## Opzioni di privacy

- **Rimuovi le informazioni personali da questo file al salvataggio** elimina autore, ultima persona che ha salvato il file, società, responsabile, nonché i nomi e gli orari di note e commenti, dal file che viene scritto (una cartella di lavoro, una copia e una cartella di lavoro inviata per posta elettronica). Il contenuto del file resta invariato. Le proprietà personalizzate del file non vengono modificate, e il documento aperto non viene alterato. È la stessa scelta in tutte le app di JeffOffice: cambiala in una e cambierà in tutte. Per farlo una sola volta, usa **Ispeziona cartella di lavoro** in [Informazioni](info). La stessa casella si trova nella pagina **Informazioni**.

## Statistiche di utilizzo

**Invia statistiche di utilizzo anonime** consente di condividere dati su come viene usato JeffCrunch. Anonime significa che i dati non dicono nulla su chi sei. Quando questa opzione è attiva, JeffCrunch invia questi dati una volta al giorno e il giorno in cui aggiorni: che si tratta di JeffCrunch, la sua versione, il sistema operativo e la lingua, quante volte è stato avviato, con quale frequenza viene usato ogni comando e che tipo di file hai aperto o salvato. Se JeffCrunch si chiude inaspettatamente, alla successiva apertura invia anche una segnalazione di arresto anomalo. JeffCrunch non invia mai i tuoi file, i loro nomi o il loro contenuto, né quello che digiti. Se l'opzione è disattivata, non viene inviata neppure alcuna segnalazione di arresto anomalo.

La scelta riguarda tutte le app di JeffOffice: cambiala qui e cambierà in ciascuna di esse. JeffCrunch ti chiede la prima volta che viene avviato, nella finestra **Aiutaci a migliorare JeffCrunch**.

- **Informativa sulla privacy**, accanto alla casella di controllo, apre la nostra informativa sulla privacy all'indirizzo jeffoffice.com/privacy. Vi si spiega quali dati vengono inviati, dove vanno e per quanto tempo li conserviamo. Conserviamo le statistiche per 24 mesi.
- **Numero di installazione** è mostrato sotto la casella di controllo. È un numero casuale generato per la tua copia di JeffOffice, uguale in tutte le app di JeffOffice. Non dice nulla su di te. Fai clic su **Copia** per copiarlo. Se le statistiche sono disattivate, oppure non è ancora stato generato alcun numero, compare **nessuno**.
- Per far eliminare le statistiche che già conserviamo, invia il tuo numero di installazione a info@jeffoffice.com **prima** di disattivare le statistiche. Disattivandole, il numero viene cancellato dal tuo computer e, senza di esso, non possiamo stabilire quali registrazioni sono tue.
- Per disattivare le statistiche senza aprire JeffCrunch, imposta la variabile d'ambiente `JP_NO_TELEMETRY` su un valore qualsiasi prima di avviarlo. Da quel momento nulla viene conteggiato né inviato, e JeffCrunch non pone la domanda del primo avvio.

## Argomenti correlati

- [Benvenuto in JeffCrunch](start)
