# Modificare le impostazioni di JeffCrunch

<!-- keywords: recupero, recupera lavoro non salvato, recupero automatico, correzione, modifica, completamento automatico, maniglia di riempimento, trascina e rilascia, cifre decimali, opzioni, impostazioni, preferenze, statistiche di utilizzo, numero di installazione, privacy, informativa sulla privacy, JP_NO_TELEMETRY, elimina statistiche, aggiornamenti, informazioni, modifica nella cella, colore griglia, zoom predefinito, larghezza standard, informazioni personali, personalizza tastiera, personalizza la barra multifunzione, personalizza la barra degli strumenti rapida, barra degli strumenti rapida, aggiungi un comando alla barra degli strumenti rapida, nascondi una scheda, nascondi un gruppo, aggiungi una scheda, aggiungi un gruppo, rinomina una scheda, reimposta la barra multifunzione, cambia tasti di scelta rapida, riassegna tasti, reimposta tasti -->

Le impostazioni si trovano nella finestra **Opzioni di JeffCrunch**. Fai clic su **File > Opzioni** per aprirla. Fai clic su **OK** per mantenere le modifiche oppure su **Annulla** per scartarle.

## Modifica

La pagina **Modifica** cambia il modo in cui funziona la digitazione nelle celle.

- **Dopo aver premuto Invio, sposta la selezione** indica dove va la selezione quando completi una voce: in basso, a destra, in alto, a sinistra, oppure resta dove si trova.
- **Suggerisci il completamento del testo dalle celle sopra e sotto** si chiama completamento automatico. Quando digiti le prime lettere di un testo e in quella colonna un solo testo inizia con quelle lettere, JeffCrunch mostra il resto, selezionato. Premi **Invio** per confermarlo. Continua a digitare per ignorarlo, oppure premi **Backspace** per eliminarlo. Il completamento considera solo il blocco di celle accanto a quella in cui stai scrivendo, e solo i testi, non i numeri.
- **Usa la maniglia di riempimento e trascina le celle per spostarle o copiarle** attiva il piccolo quadratino nell'angolo della selezione e il trascinamento del bordo della selezione. Disattivalo se sposti le celle per errore. Copia e Incolla continuano a funzionare.
- **Mostra uno zero nelle celle che hanno valore zero (questo foglio)** attiva o disattiva la visualizzazione degli zeri per il foglio che stai guardando. Quando è disattivata, una cella che contiene 0 appare vuota, ma contiene ancora lo 0 e viene sommata come 0. La scelta viene salvata con il foglio.
- **Inserisci un separatore decimale nei numeri interi digitati** si chiama inserimento automatico delle cifre decimali. Se imposti **Posizioni** a 2, digitando 1234 viene inserito 12,34. Se la imposti a -2, digitando 5 viene inserito 500. Un numero digitato con il separatore decimale viene inserito così come lo scrivi.

- **Consenti la modifica direttamente nelle celle** è attiva per impostazione predefinita. Quando la disattivi, una voce digitata o il tasto F2 portano alla barra della formula, e la cella mostra il risultato solo al termine della modifica.

## Opzioni di salvataggio

**Salva informazioni di recupero automatico ogni** imposta ogni quanto JeffCrunch conserva una copia di una cartella di lavoro con modifiche non ancora salvate, da 1 a 120 minuti. La copia è un file .xlsx in una cartella che JeffCrunch riserva a questo scopo, creata in background in modo che tu possa continuare a scrivere. Viene eliminata quando salvi o chiudi la cartella di lavoro. Se JeffCrunch si chiude improvvisamente, alla successiva apertura propone le copie disponibili. Puoi anche fare clic su **File > Apri > Recupera lavoro non salvato**. Una cartella di lavoro protetta con password non viene copiata, perché la copia non sarebbe protetta.

## Opzioni di correzione

La pagina **Strumenti di correzione** è condivisa dalle app di JeffOffice. Stabilisce se l'ortografia viene controllata durante la digitazione in una cella (disattiva **Controlla ortografia durante la digitazione** per eliminare le sottolineature rosse ondulate in tutte le finestre aperte e nelle altre app; **Controllo ortografia** nella scheda Revisione continua a funzionare), se le parole in maiuscolo e le parole con numeri vengono ignorate durante il controllo ortografico, se la Correzione automatica corregge le parole mentre digiti, se le virgolette dritte diventano curve, se trattini, frazioni e ordinali vengono modificati e se un indirizzo web diventa un collegamento. Vedi [Ortografia](notes-links) e [Digitare nelle celle](edit-cells).

## Opzioni di visualizzazione

- **Colore griglia** è il colore delle linee tra le celle sullo schermo. È una scelta solo per il tuo schermo, non viene salvata nella cartella di lavoro.
- **Zoom di una nuova cartella di lavoro** è la dimensione con cui appaiono i fogli di una nuova cartella di lavoro, da 10% a 400%.
- **Larghezza colonna standard (questo foglio)** è la larghezza di ogni colonna di questo foglio che non ha una larghezza propria. Coincide con **Larghezza predefinita** nella scheda Home.

## Personalizza

La scheda **Personalizza** ha tre pagine: **Barra multifunzione**, **Barra degli strumenti rapida** e **Tastiera**. La pagina **Tastiera** serve a scegliere i tasti di scelta rapida dei comandi di JeffCrunch. Vedi "Scegli i tuoi tasti di scelta rapida" nell'argomento sulla tastiera.

### Personalizza la barra multifunzione

La pagina **Barra multifunzione** elenca le schede della barra, ciascuna con i propri gruppi.

- **Nascondi o mostra una scheda o un gruppo.** Deseleziona la casella accanto per nasconderlo e selezionala per mostrarlo di nuovo. Un gruppo nascosto non occupa spazio, e i suoi comandi continuano a funzionare con i tasti di scelta rapida. **Home** non ha la casella, perché resta sempre visibile, e così il pulsante **File**. Una scheda che compare solo quando è selezionato qualcosa, come un'immagine o una tabella, si trova alla fine dell'elenco e non ha la casella.
- **Crea una scheda o un gruppo personalizzato.** Digita un nome nella casella **Nome** e fai clic su **Nuova scheda**. Per creare un gruppo, scegli una scheda e fai clic su **Nuovo gruppo**. Un gruppo personalizzato può stare in qualsiasi scheda, anche in una scheda che compare solo quando è selezionato qualcosa.
- **Inserisci comandi nel tuo gruppo.** Scegli il tuo gruppo, trova un comando nell'elenco **Comandi** (digita nella casella **Cerca comandi** per restringere l'elenco) e fai clic su **Aggiungi comando**. Può entrare nel gruppo qualsiasi comando che abbia un nome. Un comando può comparire una sola volta in un gruppo.
- **Rinomina.** Scegli una scheda, un gruppo o un comando personalizzato, digita il nuovo nome e fai clic su **Rinomina**. Un comando rinominato esegue la stessa operazione di prima, e il nuovo nome compare solo sul suo pulsante. Per ripristinare il nome originale di un comando, rinominalo con un nome vuoto.
- **Riordina.** Scegli una scheda e fai clic su **Sposta su** o **Sposta giù**. Puoi spostare sia le schede della barra sia le tue, ma non le schede che compaiono solo quando è selezionato qualcosa. I tuoi gruppi e i tuoi comandi si spostano allo stesso modo all'interno della loro scheda o del loro gruppo. I gruppi predefiniti della barra mantengono il loro ordine.
- **Elimina.** Scegli una scheda, un gruppo o un comando personalizzato e fai clic su **Rimuovi**.

**Reimposta una** ripristina la scheda scelta: torna visibile, i suoi gruppi tornano visibili e i gruppi che hai aggiunto spariscono. Una scheda personalizzata viene eliminata. **Reimposta tutto** ripristina l'intera barra multifunzione, compreso l'ordine delle schede.

Non cambia nulla finché non fai clic su **OK**. Poi tutte le finestre di JeffCrunch aperte mostrano la tua barra multifunzione, e così ogni finestra che aprirai in seguito. JeffCrunch conserva solo ciò che differisce dalla barra predefinita, così una scheda o un gruppo aggiunto da una versione successiva compare automaticamente. Le tue schede e i tuoi gruppi ricevono i suggerimenti da tastiera (le lettere che compaiono quando premi **Alt**), che non si sovrappongono agli altri. Puoi avere 20 schede personalizzate, 30 gruppi personalizzati per scheda e 60 comandi per gruppo.

### Personalizza la barra degli strumenti rapida

La pagina **Barra degli strumenti rapida** riguarda la piccola barra in alto a sinistra della finestra. L'elenco di sinistra mostra ciò che è presente ora sulla barra, mentre l'elenco **Comandi** a destra contiene tutti i comandi che hanno un nome (digita nella casella **Cerca comandi** per restringerlo, come nella pagina **Barra multifunzione**).

- **Aggiungi un comando.** Scegliilo nell'elenco **Comandi** e fai clic su **Aggiungi comando**. Viene messo alla fine della barra. Un comando che apre un menu mantiene il suo menu sulla barra. Un comando può comparire sulla barra una sola volta.
- **Clic destro.** Puoi anche fare clic con il pulsante destro su un comando della barra multifunzione e scegliere **Aggiungi alla barra degli strumenti rapida**, oppure fare clic con il pulsante destro su un pulsante della barra e scegliere **Rimuovi dalla barra degli strumenti rapida**. Lo stesso menu contiene **Personalizza la barra degli strumenti rapida** e **Personalizza la barra multifunzione**, che aprono questa pagina. 
- **Spostala sotto la barra multifunzione.** Seleziona **Mostra barra degli strumenti rapida sotto la barra multifunzione** per collocare la barra in una riga sotto la barra multifunzione invece che in alto a sinistra. Anche i due menu del clic destro contengono **Mostra barra degli strumenti rapida sotto la barra multifunzione**. JeffCrunch ricorda la scelta.
- **Aggiungi un separatore.** Fai clic su **Aggiungi separatore** per inserire una linea sottile alla fine, poi spostala dove preferisci.
- **Riordina.** Scegli un comando o un separatore nell'elenco di sinistra e fai clic su **Sposta su** o **Sposta giù**.
- **Elimina.** Scegli l'elemento e fai clic su **Rimuovi**.

**Reimposta uno** ripristina il comando scelto: nella posizione in cui JeffCrunch di solito lo colloca, oppure fuori dalla barra se JeffCrunch di solito non lo mette lì. **Reimposta tutto** ripristina la barra degli strumenti rapida predefinita di JeffCrunch. Reimposta tutto in questa pagina non modifica la barra multifunzione, e Reimposta tutto nella pagina **Barra multifunzione** non modifica la barra degli strumenti.

Non cambia nulla finché non fai clic su **OK**. Poi tutte le finestre di JeffCrunch aperte mostrano la tua barra degli strumenti, e così ogni finestra che aprirai in seguito. La barra può contenere in tutto 100 comandi e separatori. Quando i pulsanti sono più di quelli che entrano in un terzo della larghezza della finestra, i restanti restano dietro un pulsante **»** alla fine della barra, così il titolo della finestra non viene mai spinto fuori dallo schermo. JeffCrunch conserva in elenco un comando che una versione successiva non prevede più e lo mostra di nuovo se il comando ritorna. I pulsanti ricevono i suggerimenti da tastiera, i numeri da 1 a 9 e poi due lettere, nell'ordine in cui si trovano sulla barra. La barra degli strumenti rapida si trova sempre in alto a sinistra, accanto al pulsante **File**.

## Opzioni di privacy

- **Rimuovi le informazioni personali da questo file al salvataggio** elimina autore, ultima persona che ha salvato il file, società, responsabile e i nomi e gli orari di note e commenti, dal file che viene scritto (una cartella di lavoro, una copia e una cartella di lavoro inviata per posta elettronica). Il contenuto del file resta invariato. Le proprietà personalizzate del file non vengono modificate, e il documento aperto non cambia. È la stessa opzione in tutte le app di JeffOffice: se la cambi in una, cambia in tutte. Per farlo una sola volta, usa **Ispeziona cartella di lavoro** in [Informazioni](info). La stessa casella si trova nella pagina **Informazioni**.

## Statistiche di utilizzo

**Invia statistiche di utilizzo anonime** consente di condividere dati sull'uso di JeffCrunch. Anonimo significa che i dati non dicono nulla su di te. Quando è attiva, JeffCrunch invia questi dati una volta al giorno e nel giorno in cui aggiorni: che si tratta di JeffCrunch, la sua versione, il sistema operativo e la lingua, quante volte è stato avviato, con quale frequenza viene usato ciascun comando e quale tipo di file hai aperto o salvato. Se JeffCrunch si chiude improvvisamente, alla successiva apertura invia anche un rapporto sull'arresto anomalo. JeffCrunch non invia mai i tuoi file, i loro nomi o il loro contenuto, né quello che digiti. Se l'opzione è disattivata, non viene inviato neppure il rapporto sugli arresti anomali.

La scelta vale per tutte le app di JeffOffice: se la cambi qui, cambia in ciascuna di esse. JeffCrunch chiede la preferenza la prima volta che si avvia, in una finestra chiamata **Aiutaci a migliorare JeffCrunch**.

- **Informativa sulla privacy** accanto alla casella apre la nostra informativa all'indirizzo jeffoffice.com/privacy. Vi si spiega cosa viene inviato, dove va e per quanto tempo lo conserviamo. Conserviamo le statistiche per 24 mesi.
- **Numero di installazione** è mostrato sotto la casella. È un numero casuale generato per la tua copia di JeffOffice, uguale in tutte le app di JeffOffice. Non dice nulla su di te. Fai clic su **Copia** per copiarlo. Se le statistiche sono disattivate, o se non è stato ancora generato alcun numero, appare la scritta **nessuno**.
- Per far eliminare le statistiche che abbiamo già, invia il tuo numero di installazione a info@jeffoffice.com **prima** di disattivarle. Disattivandole, il numero viene cancellato dal tuo computer e, senza di esso, non possiamo capire quali registrazioni sono tue.
- Per disattivare le statistiche senza aprire JeffCrunch, imposta la variabile d'ambiente `JP_NO_TELEMETRY` con un valore qualsiasi prima di avviarlo. Nulla viene allora conteggiato o inviato, e JeffCrunch non pone la domanda del primo avvio.

## Argomenti correlati

- [Benvenuto in JeffCrunch](start)
