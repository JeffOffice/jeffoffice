# Modificare le impostazioni di JeffDeck

<!-- keywords: opzioni, impostazioni, preferenze, tema, modalità scura, lingua, nome utente, iniziali, righello, blocca, ortografia, correzione automatica, virgolette, aggiornamenti, beta, statistiche di utilizzo, informazioni, salva, incorpora tipi di carattere, caratteri, personalizza tastiera, personalizza la barra multifunzione, personalizza la barra degli strumenti rapida, barra degli strumenti rapida, aggiungi comando alla barra degli strumenti rapida, nascondi scheda, nascondi gruppo, aggiungi scheda, aggiungi gruppo, rinomina scheda, ripristina la barra multifunzione, cambia tasti di scelta rapida, riassegna tasti, ripristina tasti, opzioni di JeffDeck, tema scuro, lingua di visualizzazione, numero di installazione -->

Le impostazioni si trovano nella finestra **Opzioni di JeffDeck**. Per aprirla, fai clic su **File > Opzioni**. La finestra ha sette schede. Fai clic su **OK** per confermare le modifiche oppure su **Annulla** per scartarle.

## Generale

- **Nome utente** e **Iniziali** compaiono sui commenti che scrivi. Vedi [Commenti](comments).
- **Tema** è **Usa impostazione di sistema**, **Chiaro** oppure **Scuro**. La modifica è visibile subito.
- **Lingua di visualizzazione** è la lingua delle scritte di JeffDeck. **Usa lingua di sistema** segue la lingua del computer. Le altre lingue compaiono in questo elenco quando sono disponibili le traduzioni. Una nuova lingua viene applicata la volta successiva che apri JeffDeck.
- **Verifica aggiornamenti all'avvio di JeffDeck** fa sì che JeffDeck cerchi una versione più recente ogni volta che si avvia. Se c'è, la finestra **Aggiornamento disponibile** mostra le novità. Scegli **Aggiorna ora**, **Più tardi** oppure **Ignora questa versione**. Su Windows, **Aggiorna ora** scarica il programma di installazione. Installa la nuova versione di tutte le app di JeffOffice e ti chiede di chiudere quelle aperte. Negli altri computer il pulsante è **Apri pagina di download**: apre la pagina da cui puoi ottenere la nuova versione. Se anche JeffPub è aperto, solo uno dei due ti avvisa di una nuova versione ogni giorno.
- **Ricevi le versioni beta (build preliminari per i test)** fa sì che JeffDeck proponga anche le versioni beta. Escono prima di quelle regolari e possono presentare qualche imperfezione.
- **Invia statistiche di utilizzo anonime** ti permette di condividere dati su come viene usato JeffDeck. Anonimi significa che i dati non dicono nulla su di te. Quando è attiva, JeffDeck invia questi dati una volta al giorno e il giorno in cui aggiorni: che si tratta di JeffDeck, la sua versione, il sistema operativo e la lingua, quante volte è stato avviato, con quale frequenza viene usato ogni comando e che tipo di file hai aperto o salvato (ad esempio .pptx). Se JeffDeck si chiude improvvisamente, invia anche una segnalazione di arresto anomalo la volta successiva che si avvia. La segnalazione indica in quale punto del programma si è fermato, così il problema può essere trovato e risolto. JeffDeck non invia mai i tuoi file, i loro nomi o il loro contenuto, né quanto digiti. Se l'opzione è disattivata, non viene inviata nemmeno alcuna segnalazione di arresto anomalo. JeffDeck ti chiede di questo la prima volta che si avvia, nella finestra **Aiutaci a migliorare JeffDeck**. Non lo chiede se hai già scelto in JeffPub o durante l'installazione.
- **Informativa sulla privacy**, accanto a questa casella, apre la nostra informativa all'indirizzo jeffoffice.com/privacy. Spiega quali dati vengono inviati, dove vanno e per quanto tempo li conserviamo. Conserviamo le statistiche per 24 mesi.
- **Numero di installazione** è mostrato sotto la casella. È un numero casuale generato per la tua copia di JeffOffice, uguale in tutte le app di JeffOffice. Non dice nulla su di te. Fai clic su **Copia** per copiarlo. Se le statistiche sono disattivate, o se non è ancora stato generato alcun numero, compare **nessuno**.
- Per far eliminare le statistiche che abbiamo già, invia il tuo numero di installazione a info@jeffoffice.com **prima** di disattivare le statistiche. Disattivarle cancella il numero dal computer e, senza di esso, non possiamo capire quali registrazioni sono tue.
- Per disattivare le statistiche senza aprire JeffDeck, imposta la variabile di ambiente `JP_NO_TELEMETRY` con un valore qualsiasi prima di avviarlo. Non viene allora conteggiato né inviato nulla, e JeffDeck non pone la domanda del primo avvio.

**Nome utente**, **Iniziali**, **Tema** e **Lingua di visualizzazione**, così come le scelte su aggiornamenti e statistiche, valgono per tutte le app di JeffOffice: se le cambi qui, cambiano in ciascuna di esse.

## Correzione

Queste caselle controllano l'ortografia e gli aiuti alla digitazione. Vedi [Digitare e formattare il testo](text).

- **Controlla ortografia durante la digitazione**
- **Verifica la grammatica durante la digitazione** segnala con una linea blu punteggiata gli errori chiari di grammatica e di stile nei testi in inglese, francese, tedesco e spagnolo.
- **Impostazioni grammatica** apre un elenco dei tipi di problemi da cercare: grammatica, punteggiatura, maiuscole, parole confuse, ripetizioni e stile (disattivato finché non lo attivi).
- **Ignora parole in MAIUSCOLO**
- **Ignora parole che contengono numeri**
- **Sostituisci testo durante la digitazione (Correzione automatica)** corregge gli errori di digitazione più comuni. In **Opzioni correzione automatica**, la scheda **Simboli matematici** contiene un interruttore, **Sostituisci i nomi dei simboli matematici durante la digitazione**, disattivato finché non lo attivi. Con l'interruttore attivo, una barra rovesciata seguita da un nome, come \alpha, diventa il simbolo (α) quando digiti uno spazio o un segno dopo di essa. Il nome deve iniziare una parola, quindi un percorso di cartella come C:\temp\pi resta come l'hai digitato. La scheda elenca tutti i nomi. JeffDeck, JeffWrite e JeffPub li sostituiscono; JeffCrunch non modifica nulla in una cella.
- **Sostituisci virgolette dritte con virgolette curve** trasforma " e ' in virgolette curve.
- **Formatta automaticamente durante la digitazione (trattini, frazioni, ordinali, elenchi automatici)**
- **Trasforma gli indirizzi web in collegamenti ipertestuali durante la digitazione** rende collegamento un indirizzo web o e-mail quando premi la barra spaziatrice dopo di esso. Annulla elimina solo il collegamento.

## Visualizza

- **Mostra il righello** mostra i righelli lungo i bordi della diapositiva. Equivale a **Visualizza > Mostra > Righello** e vale per ogni finestra, quelle già aperte e quelle che apri in seguito.
- **Blocca oggetti su altri oggetti** fa sì che un oggetto che trascini si allinei agli altri oggetti della diapositiva.

## Avanzate

- **Usa il tasto Inserisci per attivare e disattivare la modalità sovrascrittura** è disattivato all'inizio. Quando è attivo, il tasto **Inserisci** passa da un modo di digitare all'altro in una casella di testo. Normalmente ogni lettera che digiti spinge in avanti quelle successive. In modalità **sovrascrittura** ogni lettera che digiti sostituisce quella dopo il cursore, fino alla fine del paragrafo. La barra di stato mostra **Inserisci** o **Sovrascrivi** in modo che tu sappia in quale modalità ti trovi. Premi di nuovo **Inserisci** per tornare indietro.

## Salva

- **Incorpora tipi di carattere nel file** inserisce nel file, al salvataggio, i caratteri usati dalle diapositive, così le diapositive hanno lo stesso aspetto su un computer che non ha quei caratteri. È attivo finché non lo disattivi. Incorpora solo i caratteri forniti con JeffDeck e rende il file più grande. L'argomento Aprire e salvare presentazioni ne contiene i dettagli, nella sezione "Caratteri nel file".
- **Rimuovi le informazioni personali da questo file al salvataggio** elimina da file scritti (un file .pptx o .odp, un modello e una presentazione inviata per e-mail) l'autore, chi ha salvato per ultimo il file, l'azienda, il responsabile e i nomi, le iniziali e gli orari dei commenti. Il contenuto del file resta com'è. Le proprietà personalizzate del file restano come sono, e il documento aperto non viene modificato. È la stessa scelta in tutte le app di JeffOffice: se la cambi in una, cambia in tutte. La stessa casella si trova nella pagina **Informazioni**.

## Personalizza

La scheda **Personalizza** ha tre pagine: **Barra multifunzione**, **Barra degli strumenti rapida** e **Tastiera**. La pagina **Tastiera** serve a scegliere i tasti di scelta rapida dei comandi di JeffDeck. Vedi "Scegliere i propri tasti di scelta rapida" nell'argomento sulla tastiera.

### Personalizzare la barra multifunzione

La pagina **Barra multifunzione** elenca le schede della barra, ciascuna con i suoi gruppi.

- **Nascondi o mostra una scheda o un gruppo.** Deseleziona la casella accanto per nasconderlo e selezionala per mostrarlo di nuovo. Un gruppo nascosto non occupa spazio, e i suoi comandi continuano a funzionare con i tasti di scelta rapida. **Home** non ha casella, perché resta sempre visibile, e così il pulsante **File**. Una scheda che compare solo quando si seleziona qualcosa, come un'immagine o una tabella, si trova alla fine dell'elenco e non ha casella.
- **Crea una scheda o un gruppo personale.** Digita un nome nella casella **Nome** e fai clic su **Nuova scheda**. Per creare un gruppo, scegli una scheda e fai clic su **Nuovo gruppo**. Un gruppo personale può stare in qualsiasi scheda, anche in una scheda che compare solo quando si seleziona qualcosa.
- **Inserisci comandi nel tuo gruppo.** Scegli il tuo gruppo, cerca un comando nell'elenco **Comandi** (digita nella casella **Cerca comandi** per restringere l'elenco) e fai clic su **Aggiungi comando**. Può entrare nel gruppo qualsiasi comando che abbia un nome. Un comando può stare in un gruppo una sola volta.
- **Rinomina.** Scegli una scheda, un gruppo o un comando personale, digita il nuovo nome e fai clic su **Rinomina**. Un comando rinominato fa la stessa cosa, e il nuovo nome è solo la scritta del suo pulsante. Per dare di nuovo al comando il suo nome originale, rinominalo lasciando il nome vuoto.
- **Riordina.** Scegli una scheda e fai clic su **Sposta su** o **Sposta giù**. Puoi spostare sia le schede della barra sia le tue, ma non le schede che compaiono solo quando si seleziona qualcosa. I tuoi gruppi e i tuoi comandi si spostano allo stesso modo all'interno della loro scheda o del loro gruppo. I gruppi della barra mantengono il loro ordine.
- **Elimina.** Scegli una scheda, un gruppo o un comando personale e fai clic su **Rimuovi**.

**Ripristina una** riporta la scheda scelta allo stato originale: viene mostrata, mostrati i suoi gruppi, e i gruppi che le hai aggiunto vengono tolti. Una scheda personale viene eliminata. **Ripristina tutto** riporta l'intera barra allo stato originale, compreso l'ordine delle schede.

Nulla cambia finché non fai clic su **OK**. Poi ogni finestra aperta di JeffDeck mostra la tua barra, e così ogni finestra che apri in seguito. JeffDeck conserva solo ciò che differisce dalla barra standard, quindi una scheda o un gruppo aggiunto da una versione successiva compare da solo. Le tue schede e i tuoi gruppi ricevono suggerimenti da tastiera (le lettere che compaiono premendo Alt) che non si sovrappongono agli altri. Puoi avere 20 schede personali, 30 gruppi personali in una scheda e 60 comandi in un gruppo.

### Personalizzare la barra degli strumenti rapida

La pagina **Barra degli strumenti rapida** riguarda la piccola barra in alto a sinistra della finestra. L'elenco a sinistra mostra ciò che c'è ora sulla barra, e l'elenco **Comandi** a destra contiene tutti i comandi che hanno un nome (digita nella casella **Cerca comandi** per restringerlo, come nella pagina **Barra multifunzione**).

- **Aggiungi un comando.** Scegli il comando nell'elenco **Comandi** e fai clic su **Aggiungi comando**. Viene messo alla fine della barra. Un comando che apre un menu mantiene il suo menu sulla barra. Un comando può stare sulla barra una sola volta.
- **Clic destro.** Puoi anche fare clic destro su un comando della barra multifunzione e scegliere **Aggiungi alla barra degli strumenti rapida**, oppure fare clic destro su un pulsante della barra e scegliere **Rimuovi dalla barra degli strumenti rapida**. Lo stesso menu contiene **Personalizza la barra degli strumenti rapida** e **Personalizza la barra multifunzione**, che aprono questa pagina.
- **Spostala sotto la barra multifunzione.** Seleziona **Mostra la barra degli strumenti rapida sotto la barra multifunzione** per mettere la barra su una riga sotto la barra multifunzione invece che in alto a sinistra. Anche i due menu del clic destro contengono **Mostra la barra degli strumenti rapida sotto la barra multifunzione**. JeffDeck ricorda la scelta.
- **Aggiungi un separatore.** Fai clic su **Aggiungi separatore** per mettere una linea sottile alla fine, poi spostala dove vuoi.
- **Riordina.** Scegli un comando o un separatore nell'elenco a sinistra e fai clic su **Sposta su** o **Sposta giù**.
- **Elimina.** Scegli l'elemento e fai clic su **Rimuovi**.

**Ripristina uno** riporta il comando scelto alla sua posizione abituale oppure, se JeffDeck non lo mette di solito sulla barra, lo toglie dalla barra. **Ripristina tutto** riporta la barra degli strumenti a quella fornita con JeffDeck. Ripristina tutto in questa pagina non cambia la barra multifunzione, e Ripristina tutto nella pagina **Barra multifunzione** non cambia la barra degli strumenti.

Nulla cambia finché non fai clic su **OK**. Poi ogni finestra aperta di JeffDeck mostra la tua barra, e così ogni finestra che apri in seguito. La barra può contenere in tutto 100 comandi e separatori. Quando ci sono più pulsanti di quanti ne entrano in un terzo della larghezza della finestra, i restanti attendono dietro un pulsante **»** alla fine della barra, così il titolo della finestra non viene mai spinto fuori dallo schermo. JeffDeck conserva un comando nel tuo elenco quando una versione successiva non lo prevede più, e lo mostra di nuovo se il comando ritorna. I pulsanti ricevono suggerimenti da tastiera: i numeri da 1 a 9 e poi due lettere, nell'ordine in cui si trovano sulla barra. La barra si trova sempre in alto a sinistra, accanto al pulsante **File**.

## Sicurezza

La scheda **Sicurezza** decide cosa fa JeffDeck con ciò a cui un file può puntare: file collegati, filmati e suoni collegati e file provenienti da Internet. Le macro sono conservate nel file e non vengono mai eseguite, e un collegamento che avvierebbe un programma viene sempre rifiutato. Vedi [Sicurezza](security).

## Argomenti correlati

- [Scheda Visualizza](tab-view)
- [Recuperare il lavoro non salvato](recover)
- [La schermata File](file-screen)
