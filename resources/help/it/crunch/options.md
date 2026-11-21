# Modificare le impostazioni di JeffCrunch
<!-- keywords: impostazioni formule, opzioni di calcolo, variazione massima, iterazione, recupero, recupero lavoro non salvato, recupero automatico, correzione, modifica, completamento automatico, maniglia di riempimento, trascina, cifre decimali, opzioni, impostazioni, preferenze, statistiche di utilizzo, numero di installazione, privacy, informativa sulla privacy, JP_NO_TELEMETRY, elimina statistiche, aggiornamenti, informazioni, modifica nelle celle, colore griglia, zoom predefinito, larghezza standard, informazioni personali, personalizza tastiera, personalizza la barra multifunzione, personalizza la barra degli strumenti rapida, barra degli strumenti rapida, aggiungi un comando alla barra degli strumenti rapida, nascondi una scheda, nascondi un gruppo, aggiungi una scheda, aggiungi un gruppo, rinomina una scheda, reimposta la barra multifunzione, cambia tasti di scelta rapida, riassegna tasti, reimposta tasti -->

Le impostazioni si trovano nella finestra **Opzioni di JeffCrunch**. Fai clic su **File > Opzioni** per aprirla. Fai clic su **OK** per mantenere le modifiche oppure su **Annulla** per scartarle.

## Generale

**Mostra la Mini barra degli strumenti alla selezione** è attiva per impostazione predefinita. Stabilisce se la piccola barra di formattazione compare accanto al testo selezionato e in cima al menu di scelta rapida delle celle (vedi [Digitare nelle celle](edit-cells)). La scelta è condivisa dalle app di JeffOffice.

## Modifica

La pagina **Modifica** cambia il modo in cui si digita nelle celle.

- **Dopo aver premuto Invio, sposta la selezione** indica dove va la selezione quando termini una voce: giù, a destra, su, a sinistra, oppure resta ferma.
- **Propone di completare un testo dalle celle sopra e sotto** si chiama completamento automatico. Quando digiti le prime lettere di un testo e in quella colonna solo un testo inizia con esse, JeffCrunch mostra il resto, selezionato. Premi **Invio** per confermarlo. Continua a digitare per ignorarlo, oppure premi **Backspace** per eliminarlo. Il completamento considera solo il blocco di celle accanto a quella in cui stai digitando, e solo i testi, non i numeri.
- **Usa la maniglia di riempimento e trascina le celle per spostarle o copiarle** attiva il piccolo quadratino all'angolo della selezione e il trascinamento del bordo della selezione. Disattivala se sposti le celle per errore. Copia e Incolla continuano a funzionare.
- **Mostra uno zero nelle celle che hanno valore zero (questo foglio)** attiva o disattiva gli zeri per il foglio che stai visualizzando. Quando è disattivata, una cella che contiene 0 appare vuota, ma conserva lo 0 e viene sommata come 0. La scelta viene salvata con il foglio.
- **Inserisci un separatore decimale nei numeri interi digitati** si chiama posizioni decimali automatiche. Imposta **Posizioni** su 2: digitando 1234 si inserisce 12,34. Impostala su -2: digitando 5 si inserisce 500. Un numero digitato con il separatore decimale viene inserito così come lo scrivi.

- **Consenti la modifica direttamente nelle celle** è attiva per impostazione predefinita. Quando la disattivi, un valore digitato o il tasto F2 portano alla barra della formula, e la cella mostra il risultato solo quando termini l'inserimento.

## Opzioni delle formule

La pagina **Formule** stabilisce come la cartella di lavoro aperta calcola le sue formule: **Calcolo** (automatico, automatico eccetto tabelle dati, oppure manuale), se calcolare le formule prima del salvataggio della cartella quando il calcolo è manuale, se una formula che fa riferimento a se stessa viene calcolata girandoci intorno, il numero massimo di giri e la variazione che li interrompe. Queste impostazioni appartengono alla cartella di lavoro, non a te: sono salvate nel suo file e non vengono conservate per le altre cartelle di lavoro, e le scelte sono disattivate quando non è aperta alcuna cartella. La variazione è mostrata con tutte le sue cifre, e puoi digitarla come numero, ad esempio 0,0000001 oppure 1e-7. Viene scritta solo un'impostazione che hai modificato: **OK** senza modifiche non aggiunge alcun passaggio di Annulla e lascia la cartella com'è. Ogni modifica è un passaggio di Annulla. Il formato di file più recente e il vecchio formato .xls conservano tutte le impostazioni. Una copia in .ods conserva le impostazioni di iterazione, ma non ha un modo per il calcolo manuale: la copia calcola quindi le formule man mano che cambiano, e il rapporto mostrato prima del salvataggio della copia lo segnala. Le stesse impostazioni si trovano nella scheda **Formule**.

## Opzioni di salvataggio

**Salva informazioni di recupero automatico ogni** stabilisce ogni quanto JeffCrunch conserva una copia di una cartella di lavoro con modifiche non ancora salvate, da 1 a 120 minuti. La copia è un file .xlsx in una cartella che JeffCrunch usa per questo scopo, creata in background mentre puoi continuare a digitare. Viene eliminata quando salvi o chiudi la cartella di lavoro. Se JeffCrunch si chiude all'improvviso, alla successiva apertura propone le copie. Puoi anche fare clic su **File > Apri > Recupera lavoro non salvato**. Una cartella di lavoro protetta da password non viene copiata, perché la copia non sarebbe protetta.

## Opzioni di correzione

La pagina **Correzione** è condivisa dalle app di JeffOffice. Stabilisce se l'ortografia viene controllata mentre digiti in una cella (disattiva **Controlla ortografia durante la digitazione** per eliminare le sottolineature rosse ondulate in tutte le finestre aperte e nelle altre app; **Ortografia** nella scheda Revisione continua a funzionare), se le parole in maiuscolo e le parole con numeri vengono ignorate durante il controllo, se la correzione automatica corregge le parole mentre digiti, se le virgolette dritte diventano curve, se trattini, frazioni e ordinali vengono modificati e se un indirizzo web diventa un collegamento. Vedi [Ortografia](notes-links) e [Digitare nelle celle](edit-cells).

## Opzioni di visualizzazione

- **Colore griglia** è il colore delle linee tra le celle sullo schermo. È una scelta solo per il tuo schermo, non viene salvata nella cartella di lavoro.
- **Zoom di una nuova cartella di lavoro** indica quanto grandi appaiono i fogli di una nuova cartella di lavoro, da 10% a 400%.
- **Larghezza colonna standard (questo foglio)** è la larghezza di ogni colonna di questo foglio che non ha una larghezza propria. Corrisponde a **Larghezza predefinita** nella scheda Home.

## Personalizza

La scheda **Personalizza** ha tre pagine, **Barra multifunzione**, **Barra degli strumenti rapida** e **Tastiera**. La pagina **Tastiera** serve a scegliere i tasti di scelta rapida dei comandi di JeffCrunch. Vedi "Scegliere i propri tasti di scelta rapida" nell'argomento sulla tastiera.

### Personalizzare la barra multifunzione

La pagina **Barra multifunzione** elenca le schede della barra, ciascuna con i suoi gruppi.

- **Nascondere o mostrare una scheda o un gruppo.** Deseleziona la casella accanto per nasconderla, e selezionala per mostrarla di nuovo. Un gruppo nascosto non occupa spazio, e i suoi comandi restano disponibili con i tasti di scelta rapida. **Home** non ha una casella, perché resta sempre visibile, e così il pulsante **File**. Una scheda che compare solo quando si seleziona qualcosa, come un'immagine o una tabella, si trova in fondo all'elenco e non ha una casella.
- **Creare una scheda o un gruppo personale.** Digita un nome nella casella **Nome** e fai clic su **Nuova scheda**. Per creare un gruppo, scegli una scheda e fai clic su **Nuovo gruppo**. Un gruppo personale può andare in qualsiasi scheda, anche in una che compare solo quando si seleziona qualcosa.
- **Inserire comandi nel proprio gruppo.** Scegli il tuo gruppo, cerca un comando nell'elenco **Comandi** (digita nella casella **Cerca comandi** per restringere l'elenco) e fai clic su **Aggiungi comando**. Può andare nel gruppo qualsiasi comando che ha un nome. Un comando può comparire una sola volta in un gruppo.
- **Rinominare.** Scegli una scheda, un gruppo o un comando personale, digita il nuovo nome e fai clic su **Rinomina**. Un comando rinominato fa la stessa cosa, e il nuovo nome è solo quello che mostra il suo pulsante. Per ripristinare il nome originale di un comando, rinominalo con un nome vuoto.
- **Riordinare.** Scegli una scheda e fai clic su **Sposta su** o **Sposta giù**. Puoi spostare le schede della barra come le tue, ma non quelle che compaiono solo quando si seleziona qualcosa. I tuoi gruppi e i tuoi comandi si spostano allo stesso modo all'interno della loro scheda o del loro gruppo. I gruppi della barra mantengono il loro ordine.
- **Eliminare.** Scegli una scheda, un gruppo o un comando personale e fai clic su **Rimuovi**.

**Reimposta una** ripristina la scheda scelta: torna visibile, tornano visibili i suoi gruppi, e i gruppi che le hai aggiunto scompaiono. Una scheda personale viene eliminata. **Reimposta tutto** ripristina l'intera barra, compreso l'ordine delle schede.

Nulla cambia finché non fai clic su **OK**. Poi ogni finestra di JeffCrunch aperta mostra la tua barra multifunzione, e così ogni finestra che aprirai in seguito. JeffCrunch conserva solo ciò che differisce dalla barra abituale, quindi una scheda o un gruppo aggiunto da una versione successiva compare automaticamente. Le tue schede e i tuoi gruppi ricevono suggerimenti da tastiera (le lettere che compaiono premendo **Alt**) che non si sovrappongono agli altri. Puoi avere 20 schede personalizzate, 30 gruppi personalizzati per scheda e 60 comandi per gruppo.

### Personalizzare la barra degli strumenti rapida

La pagina **Barra degli strumenti rapida** riguarda la piccola barra in alto a sinistra della finestra. Il suo elenco a sinistra mostra ciò che è presente ora nella barra, e l'elenco **Comandi** a destra contiene tutti i comandi che hanno un nome (digita nella casella **Cerca comandi** per restringerlo, come nella pagina **Barra multifunzione**).

- **Aggiungere un comando.** Scegli il comando nell'elenco **Comandi** e fai clic su **Aggiungi comando**. Viene inserito in fondo alla barra. Un comando che apre un menu mantiene il suo menu nella barra. Un comando può comparire nella barra una sola volta.
- **Clic destro.** Puoi anche fare clic destro su un comando della barra multifunzione e scegliere **Aggiungi alla barra degli strumenti rapida**, oppure fare clic destro su un pulsante della barra e scegliere **Rimuovi dalla barra degli strumenti rapida**. Lo stesso menu contiene **Personalizza la barra degli strumenti rapida** e **Personalizza la barra multifunzione**, che aprono questa pagina.
- **Spostare la barra sotto la barra multifunzione.** Seleziona **Mostra la barra degli strumenti rapida sotto la barra multifunzione** per mettere la barra in una riga sotto la barra multifunzione invece che in alto a sinistra. Anche i due menu del clic destro contengono **Mostra la barra degli strumenti rapida sotto la barra multifunzione**. JeffCrunch ricorda la scelta.
- **Aggiungere un separatore.** Fai clic su **Aggiungi separatore** per inserire una linea sottile in fondo, poi spostala dove preferisci.
- **Riordinare.** Scegli un comando o un separatore nell'elenco a sinistra e fai clic su **Sposta su** o **Sposta giù**.
- **Eliminare.** Scegli l'elemento e fai clic su **Rimuovi**.

**Reimposta una** ripristina il comando scelto: nella posizione in cui JeffCrunch di solito lo mette, oppure fuori dalla barra se di solito JeffCrunch non lo mette lì. **Reimposta tutto** ripristina la barra con cui JeffCrunch viene fornito. Reimposta tutto in questa pagina non modifica la barra multifunzione, e Reimposta tutto nella pagina **Barra multifunzione** non modifica la barra degli strumenti rapida.

Nulla cambia finché non fai clic su **OK**. Poi ogni finestra di JeffCrunch aperta mostra la tua barra, e così ogni finestra che aprirai in seguito. La barra può contenere in tutto 100 comandi e separatori. Quando i pulsanti sono più di quelli che stanno in un terzo della larghezza della finestra, gli altri restano dietro un pulsante **»** in fondo alla barra, così il titolo della finestra non viene mai spinto fuori dallo schermo. JeffCrunch conserva in elenco un comando quando una versione successiva non lo prevede più, e lo mostra di nuovo se il comando torna. I pulsanti ricevono suggerimenti da tastiera, i numeri da 1 a 9 seguiti da due lettere, nell'ordine in cui si trovano nella barra. La barra si trova sempre in alto a sinistra, accanto al pulsante **File**.

## Opzioni sulla privacy

- **Rimuovi le informazioni personali da questo file al salvataggio** elimina autore, ultimo utente che ha salvato il file, società, responsabile, oltre ai nomi e agli orari su note e commenti, dal file che viene scritto (una cartella di lavoro, una copia e una cartella di lavoro inviata per posta elettronica). Ciò che il file dice resta com'è. Le proprietà personalizzate del file non vengono modificate, e il documento aperto non cambia. È la stessa scelta in tutte le app di JeffOffice: modificala in una e cambia in tutte. Per farlo una sola volta, usa **Ispeziona cartella di lavoro** in [Informazioni](info). La stessa casella si trova nella pagina **Informazioni**.

## Statistiche di utilizzo

**Invia statistiche di utilizzo anonime** ti permette di condividere dati su come viene usato JeffCrunch. Anonime significa che i dati non dicono nulla su di te. Quando l'opzione è attiva, JeffCrunch invia questi dati una volta al giorno, e nel giorno in cui aggiorni: che si tratta di JeffCrunch, la sua versione, il sistema operativo e la lingua, quante volte è stato avviato, con quale frequenza viene usato ciascun comando e che tipo di file hai aperto o salvato. Se JeffCrunch si chiude all'improvviso, alla successiva apertura invia anche una segnalazione di arresto anomalo. JeffCrunch non invia mai i tuoi file, i loro nomi o il loro contenuto, né quello che digiti. Se l'opzione è disattivata, non viene inviata nemmeno alcuna segnalazione di arresto anomalo.

La scelta riguarda tutte le app di JeffOffice: modificala qui e cambia in ciascuna di esse. JeffCrunch ti chiede una prima volta, al primo avvio, nella finestra **Aiuta a migliorare JeffCrunch**.

- **Informativa sulla privacy**, accanto alla casella di controllo, apre la nostra informativa sulla privacy all'indirizzo jeffoffice.com/privacy. Spiega cosa viene inviato, dove va e per quanto tempo lo conserviamo. Conserviamo le statistiche per 24 mesi.
- **Numero di installazione** è mostrato sotto la casella di controllo. È un numero casuale generato per la tua copia di JeffOffice, uguale in tutte le app di JeffOffice. Non dice nulla su di te. Fai clic su **Copia** per copiarlo. Se le statistiche sono disattivate, o se non è ancora stato generato alcun numero, viene mostrato **nessuno**.
- Per far eliminare le statistiche che già conserviamo, invia il tuo numero di installazione a info@jeffoffice.com **prima** di disattivarle. Disattivarle cancella il numero dal tuo computer, e senza di esso non possiamo capire quali registrazioni sono tue.
- Per disattivare le statistiche senza aprire JeffCrunch, imposta la variabile d'ambiente `JP_NO_TELEMETRY` su un valore qualsiasi prima di avviarlo. Così non viene conteggiato né inviato nulla, e JeffCrunch non pone la domanda del primo avvio.

## Sicurezza

La scheda **Sicurezza** stabilisce cosa fa JeffCrunch con ciò a cui un file può puntare: file collegati, filmati e suoni collegati, e file provenienti da internet. Le macro sono conservate nel file e non vengono mai eseguite, e un collegamento che avvierebbe un programma viene sempre rifiutato. Vedi [Sicurezza](security).

## Vedi anche

- [Benvenuto in JeffCrunch](start)
