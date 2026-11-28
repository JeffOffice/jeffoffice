# Modificare le impostazioni di JeffDeck

<!-- keywords: opzioni, impostazioni, preferenze, tema, modalità scura, lingua, nome utente, iniziali, righello, scatta, ortografia, correzione automatica, virgolette, aggiornamenti, beta, statistiche di utilizzo, informazioni, salva, incorpora caratteri, caratteri, personalizza tastiera, personalizza la barra multifunzione, personalizza la barra degli strumenti rapida, barra degli strumenti rapida, aggiungi un comando alla barra degli strumenti rapida, nascondi una scheda, nascondi un gruppo, aggiungi una scheda, aggiungi un gruppo, rinomina una scheda, ripristina la barra multifunzione, cambia tasti di scelta rapida, riassegna tasti, ripristina tasti -->

Le impostazioni si trovano nella finestra **Opzioni di JeffDeck**. Per aprirla, fai clic su **File > Opzioni**. La finestra contiene sette schede. Fai clic su **OK** per conservare le modifiche oppure su **Annulla** per scartarle.

## Generale

- **Nome utente** e **Iniziali** vengono mostrati sui commenti che scrivi. Vedi [Commenti](comments).
- **Tema** è **Usa impostazione di sistema**, **Chiaro** oppure **Scuro**. La modifica è visibile subito.
- **Lingua di visualizzazione** è la lingua delle scritte di JeffDeck. **Usa lingua di sistema** segue il computer. Le altre lingue compaiono in questo elenco quando sono disponibili le traduzioni. Una nuova lingua viene applicata alla successiva apertura di JeffDeck.
- **Verifica automaticamente la presenza di aggiornamenti all'avvio di JeffDeck** fa sì che JeffDeck cerchi una versione più recente ogni volta che si avvia. Se ce n'è una, la finestra **Aggiornamento disponibile** mostra le novità. Scegli **Aggiorna ora**, **Più tardi** oppure **Ignora questa versione**. In Windows, **Aggiorna ora** scarica il programma di installazione. Installa la nuova versione di tutte le app di JeffOffice e chiede di chiudere quelle aperte. Su altri computer il pulsante è **Apri pagina di download**, che apre la pagina da cui scaricare la nuova versione. Se anche JeffPub è aperto, solo uno dei due ti segnala una nuova versione ogni giorno.
- **Ricevi le versioni beta (build preliminari per i test)** fa sì che JeffDeck proponga anche le versioni beta. Escono prima di quelle ordinarie e possono presentare qualche imperfezione.
- **Invia statistiche di utilizzo anonime** ti permette di condividere dati su come viene usato JeffDeck. Anonimi significa che i dati non dicono nulla su chi sei. Quando l'opzione è attiva, JeffDeck invia questi dati una volta al giorno e il giorno in cui aggiorni: che si tratta di JeffDeck, la sua versione, il sistema operativo e la lingua, quante volte è stato avviato, con quale frequenza viene usato ogni comando e che tipo di file hai aperto o salvato (ad esempio .pptx). Se JeffDeck si chiude inaspettatamente, invia anche un rapporto di arresto anomalo al successivo avvio. Il rapporto indica in quale punto del programma si è interrotto, così il problema può essere individuato e risolto. JeffDeck non invia mai i tuoi file, i loro nomi o il loro contenuto, né quello che digiti. Se l'opzione è disattivata, non viene inviato neppure alcun rapporto di arresto anomalo. Al primo avvio JeffDeck ti chiede di scegliere questa opzione, nella finestra **Aiuta a migliorare JeffDeck**. Non lo chiede se hai già scelto in JeffPub o durante l'installazione.
- **Informativa sulla privacy**, accanto alla casella, apre la nostra informativa all'indirizzo jeffoffice.com/privacy. Vi si spiega che cosa viene inviato, dove va e per quanto tempo lo conserviamo. Conserviamo le statistiche per 24 mesi.
- **Numero di installazione** è mostrato sotto la casella. È un numero casuale generato per la tua copia di JeffOffice, uguale in tutte le app di JeffOffice. Non dice nulla su di te. Fai clic su **Copia** per copiarlo. Se le statistiche sono disattivate, o se non è ancora stato generato alcun numero, viene mostrato **nessuno**.
- Per eliminare le statistiche che abbiamo già ricevute, invia il tuo numero di installazione a info@jeffoffice.com **prima** di disattivare le statistiche. Disattivandole, il numero viene cancellato dal computer e, senza di esso, non possiamo stabilire quali registrazioni sono tue.
- Per disattivare le statistiche senza aprire JeffDeck, imposta la variabile d'ambiente `JP_NO_TELEMETRY` con un valore qualsiasi prima di avviarlo. Così non viene conteggiato né inviato nulla, e JeffDeck non pone la domanda del primo avvio.

**Nome utente**, **Iniziali**, **Tema** e **Lingua di visualizzazione**, nonché le scelte su aggiornamenti e statistiche, valgono per tutte le app di JeffOffice: se le cambi qui, cambiano in ciascuna di esse.

## Strumenti di correzione

Queste caselle controllano l'ortografia e l'aiuto alla digitazione. Vedi [Formattare il testo](text).

- **Controlla ortografia durante la digitazione**
- **Verifica la grammatica durante la digitazione** segnala con una linea blu punteggiata gli errori chiari di grammatica e di stile nei testi in inglese, in tutte le varianti regionali. I testi in altre lingue vengono controllati solo per l'ortografia.
- **Impostazioni grammatica** apre un elenco dei tipi di problemi da cercare: grammatica, punteggiatura, maiuscole, parole confuse, ripetizioni e stile (quest'ultimo è disattivato finché non lo attivi).
- **Ignora parole in MAIUSCOLO**
- **Ignora parole che contengono numeri**
- **Sostituisci testo durante la digitazione (Correzione automatica)** corregge i comuni errori di digitazione. In **Opzioni correzione automatica**, la scheda **Simboli matematici** contiene un interruttore, **Sostituisci i nomi dei simboli matematici durante la digitazione**, disattivato finché non lo attivi. Con l'opzione attiva, una barra rovesciata seguita da un nome, come \alpha, diventa il simbolo (α) quando digiti uno spazio o un segno dopo di essa. Il nome deve iniziare una parola, quindi un percorso di cartella come C:\temp\pi resta come l'hai digitato. La scheda elenca tutti i nomi. JeffDeck, JeffWrite e JeffPub li sostituiscono; JeffCrunch non modifica nulla in una cella.
- **Sostituisci virgolette dritte con virgolette curve** trasforma " e ' in virgolette curve.
- **Formattazione automatica durante la digitazione (trattini, frazioni, ordinali, elenchi automatici)**
- **Trasforma gli indirizzi web in collegamenti ipertestuali durante la digitazione** rende collegamento un indirizzo web o e-mail quando premi la barra spaziatrice dopo di esso. Annulla elimina solo il collegamento.

## Visualizza

- **Mostra righello** mostra i righelli lungo i bordi della diapositiva. Equivale a **Visualizza > Mostra > Righello** e vale per ogni finestra, sia quelle già aperte sia quelle che apri in seguito.
- **Blocca oggetti su altri oggetti** fa sì che un oggetto trascinato si allinei con gli altri oggetti della diapositiva.

## Avanzate

- **Usa il tasto Inserisci per attivare e disattivare la modalità di sovrascrittura** è disattivato all'inizio. Quando è attivo, il tasto **Inserisci** alterna due modi di digitare in una casella di testo. Normalmente ogni lettera digitata sposta in avanti quelle che la seguono. In modalità **sovrascrittura** ogni lettera digitata sostituisce quella che si trova dopo il cursore, fino alla fine del paragrafo. La barra di stato mostra **Inserisci** oppure **Sovrascrittura**, così sai in quale modalità ti trovi. Premi di nuovo **Inserisci** per tornare indietro.

## Salva

- **Incorpora tipi di carattere nel file** inserisce nel file, al salvataggio, i tipi di carattere usati dalle diapositive, così le diapositive hanno lo stesso aspetto anche su un computer che non li possiede. L'opzione è attiva finché non la disattivi. Vengono incorporati solo i tipi di carattere forniti con JeffDeck, e il file diventa più grande. Per i dettagli, consulta l'argomento Aprire e salvare presentazioni, nella sezione "Tipi di carattere nel file".
- **Rimuovi le informazioni personali da questo file al salvataggio** elimina dal file salvato (un file .pptx o .odp, un modello e una presentazione inviata per e-mail) l'autore, chi ha salvato il file per ultimo, l'azienda, il responsabile, nonché nomi, iniziali e orari dei commenti. Il contenuto del file resta invariato. Le proprietà personalizzate del file restano come sono e il documento aperto non viene modificato. È la stessa scelta in tutte le app di JeffOffice: se la cambi in una, cambia in tutte. La stessa casella si trova nella pagina **Info**.

## Personalizza

La scheda **Personalizza** ha tre pagine: **Barra multifunzione**, **Barra degli strumenti rapida** e **Tastiera**. La pagina **Tastiera** serve a scegliere i tasti di scelta rapida per i comandi di JeffDeck. Vedi "Scegliere i propri tasti di scelta rapida" nell'argomento sulla tastiera.

### Personalizzare la barra multifunzione

La pagina **Barra multifunzione** elenca le schede della barra, ciascuna con i suoi gruppi.

- **Nascondere o mostrare una scheda o un gruppo.** Deseleziona la casella accanto a una scheda o a un gruppo per nasconderlo, e selezionala per mostrarlo di nuovo. Un gruppo nascosto non occupa spazio, e i suoi comandi continuano a funzionare con i tasti di scelta rapida. **Home** non ha casella, perché resta sempre visibile, e così il pulsante **File**. Una scheda che compare solo quando si seleziona qualcosa, come un'immagine o una tabella, si trova in fondo all'elenco e non ha casella.
- **Creare una scheda o un gruppo personalizzati.** Digita un nome nella casella **Nome** e fai clic su **Nuova scheda**. Per creare un gruppo, scegli una scheda e fai clic su **Nuovo gruppo**. Un gruppo personalizzato può stare in qualsiasi scheda, anche in una scheda che compare solo quando si seleziona qualcosa.
- **Inserire comandi nel proprio gruppo.** Scegli il gruppo, trova un comando nell'elenco **Comandi** (digita nella casella **Cerca comandi** per restringere l'elenco) e fai clic su **Aggiungi comando**. Può entrare nel gruppo qualsiasi comando che abbia un nome. Un comando può comparire una sola volta in un gruppo.
- **Rinominare.** Scegli una scheda, un gruppo o un comando personalizzato, digita il nuovo nome e fai clic su **Rinomina**. Un comando rinominato fa la stessa cosa di prima, e il nuovo nome è solo il testo del suo pulsante. Per rinominare un comando con un nome vuoto e riavere il suo nome originale.
- **Riordinare.** Scegli una scheda e fai clic su **Sposta su** o **Sposta giù**. Puoi spostare sia le schede della barra sia le tue, ma non le schede che compaiono solo quando si seleziona qualcosa. I tuoi gruppi e i tuoi comandi si spostano allo stesso modo all'interno della scheda o del gruppo. I gruppi della barra mantengono il loro ordine.
- **Eliminare.** Scegli una scheda, un gruppo o un comando personalizzato e fai clic su **Rimuovi**.

**Reimposta una** ripristina la scheda scelta: viene mostrata, mostrati i suoi gruppi, e i gruppi che le hai aggiunto vengono eliminati. Una scheda personalizzata viene eliminata. **Reimposta tutto** ripristina l'intera barra, compreso l'ordine delle schede.

Nulla cambia finché non fai clic su **OK**. Da quel momento ogni finestra di JeffDeck aperta mostra la tua barra, e così ogni finestra che apri in seguito. JeffDeck conserva solo ciò che differisce dalla barra abituale, quindi una scheda o un gruppo aggiunto da una versione successiva compare automaticamente. Le tue schede e i tuoi gruppi ricevono suggerimenti di tasto (le lettere che compaiono quando premi **Alt**) che non si sovrappongono agli altri. Puoi avere 20 schede personalizzate, 30 gruppi personalizzati per scheda e 60 comandi per gruppo.

### Personalizzare la barra degli strumenti rapida

La pagina **Barra degli strumenti rapida** riguarda la piccola barra in alto a sinistra della finestra. L'elenco a sinistra mostra ciò che c'è ora sulla barra, mentre l'elenco **Comandi** a destra contiene tutti i comandi che hanno un nome (digita nella casella **Cerca comandi** per restringerlo, come nella pagina **Barra multifunzione**).

- **Aggiungere un comando.** Scegলilo nell'elenco **Comandi** e fai clic su **Aggiungi comando**. Viene messo alla fine della barra. Un comando che apre un menu conserva il suo menu sulla barra. Un comando può comparire sulla barra una sola volta.
- **Clic destro.** Puoi anche fare clic destro su un comando della barra multifunzione e scegliere **Aggiungi alla barra degli strumenti rapida**, oppure fare clic destro su un pulsante della barra e scegliere **Rimuovi dalla barra degli strumenti rapida**. Nello stesso menu ci sono **Personalizza la barra degli strumenti rapida** e **Personalizza la barra multifunzione**, che aprono questa pagina.
- **Spostarla sotto la barra multifunzione.** Seleziona **Mostra barra degli strumenti rapida sotto la barra multifunzione** per collocare la barra su una riga sotto la barra multifunzione, invece che in alto a sinistra. Anche i due menu del clic destro hanno **Mostra barra degli strumenti rapida sotto la barra multifunzione**. JeffDeck ricorda la scelta.
- **Aggiungere un separatore.** Fai clic su **Aggiungi separatore** per inserire una linea sottile alla fine, poi spostala dove preferisci.
- **Riordinare.** Scegli un comando o un separatore nell'elenco a sinistra e fai clic su **Sposta su** o **Sposta giù**.
- **Eliminare.** Scegli l'elemento e fai clic su **Rimuovi**.

**Reimposta una** ripristina il comando scelto: nella posizione in cui JeffDeck di solito lo mette, oppure fuori dalla barra se di solito JeffDeck non lo mette lì. **Reimposta tutto** ripristina la barra con cui JeffDeck viene fornito. Reimposta tutto in questa pagina non modifica la barra multifunzione, e Reimposta tutto nella pagina **Barra multifunzione** non modifica la barra degli strumenti.

Nulla cambia finché non fai clic su **OK**. Da quel momento ogni finestra di JeffDeck aperta mostra la tua barra, e così ogni finestra che apri in seguito. La barra può contenere in tutto 100 comandi e separatori. Quando i pulsanti sono più di quelli che stanno in un terzo della larghezza della finestra, i restanti si trovano dietro un pulsante **»** alla fine della barra, così il titolo della finestra non viene mai spinto fuori dallo schermo. JeffDeck conserva nel tuo elenco un comando che una versione successiva non prevede più, e lo mostra di nuovo se il comando ritorna. I pulsanti ricevono suggerimenti di tasto, i numeri da 1 a 9 seguiti da due lettere, nell'ordine in cui si trovano nella barra. La barra si trova sempre in alto a sinistra, accanto al pulsante **File**.

## Sicurezza

La scheda **Sicurezza** stabilisce che cosa fa JeffDeck con ciò a cui un file può puntare: file collegati, filmati e suoni collegati e file provenienti da Internet. Le macro sono conservate nel file e non vengono mai eseguite, e un collegamento che avvierebbe un programma viene sempre rifiutato. Vedi [Sicurezza](security).

## Argomenti correlati

- [Scheda Visualizza](tab-view)
- [Recuperare il lavoro non salvato](recover)
- [La schermata File](file-screen)
