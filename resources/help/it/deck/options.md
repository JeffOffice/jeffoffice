# Modificare le impostazioni di JeffDeck
<!-- keywords: opzioni, impostazioni, preferenze, tema, modalità scura, tema scuro, lingua, nome utente, iniziali, righello, allineamento automatico, scatto, ortografia, correzione automatica, virgolette, aggiornamenti, beta, statistiche di utilizzo, informazioni, salvataggio, incorpora caratteri, tipi di carattere, personalizza tastiera, personalizza barra multifunzione, personalizza barra degli strumenti rapida, barra degli strumenti rapida, aggiungi comando alla barra degli strumenti rapida, nascondi scheda, nascondi gruppo, aggiungi scheda, aggiungi gruppo, rinomina scheda, reimposta barra multifunzione, cambia tasti di scelta rapida, riassegna tasti, reimposta tasti -->

Le impostazioni si trovano nella finestra **Opzioni di JeffDeck**. Per aprirla, fai clic su **File > Opzioni**. La finestra ha sette schede. Fai clic su **OK** per confermare le modifiche oppure su **Annulla** per scartarle.

## Generale

- **Nome utente** e **Iniziali** compaiono sui commenti che scrivi. Vedi [Commenti](comments).
- **Tema** può essere **Usa impostazione di sistema**, **Chiaro** o **Scuro**. La modifica è visibile subito.
- **Lingua di visualizzazione** è la lingua dei testi di JeffDeck. **Usa lingua di sistema** segue la lingua del computer. Nell'elenco compaiono le altre lingue quando sono disponibili le traduzioni. Una nuova lingua viene applicata alla successiva apertura di JeffDeck.
- **Verifica automaticamente la presenza di aggiornamenti all'avvio di JeffDeck** fa in modo che JeffDeck cerchi una versione più recente ogni volta che si avvia. Se c'è, la finestra **Aggiornamento disponibile** mostra le novità. Scegli **Aggiorna ora**, **Più tardi** oppure **Ignora questa versione**. In Windows, **Aggiorna ora** scarica il programma di installazione. Installa la nuova versione di tutte le app di JeffOffice e ti chiede di chiudere quelle aperte. Sugli altri computer il pulsante è **Apri pagina di download**, che apre la pagina da cui scaricare la nuova versione. Se è aperto anche JeffPub, solo uno dei due ti segnala una nuova versione ogni giorno.
- **Ricevi le versioni beta (build preliminari per i test)** fa in modo che JeffDeck proponga anche le versioni beta. Escono prima di quelle regolari e possono presentare qualche imperfezione.
- **Invia statistiche di utilizzo anonime** ti permette di condividere dati sull'uso di JeffDeck. Anonimi significa che i dati non dicono nulla su chi sei. Quando l'opzione è attiva, JeffDeck invia questi dati una volta al giorno e il giorno dell'aggiornamento: che si tratta di JeffDeck, la sua versione, il sistema operativo e la lingua, quante volte è stato avviato, con quale frequenza viene usato ogni comando e il tipo di file aperto o salvato (ad esempio .pptx). Se JeffDeck si chiude inaspettatamente, invia anche una segnalazione di arresto anomalo al successivo avvio. La segnalazione indica in quale punto del programma si è interrotto, così il problema può essere individuato e risolto. JeffDeck non invia mai i tuoi file, i loro nomi, il loro contenuto né quello che digiti. Se l'opzione è disattivata, non viene inviata nemmeno alcuna segnalazione di arresto anomalo. Al primo avvio JeffDeck ti chiede di scegliere, nella finestra **Aiuta a migliorare JeffDeck**. Non lo chiede se hai già scelto in JeffPub o durante l'installazione.
- **Informativa sulla privacy**, accanto a questa casella, apre la nostra informativa all'indirizzo jeffoffice.com/privacy. Vi si spiega quali dati vengono inviati, dove vanno e per quanto tempo li conserviamo. Conserviamo le statistiche per 24 mesi.
- **Numero di installazione** è mostrato sotto la casella di controllo. È un numero casuale generato per la tua copia di JeffOffice, uguale in ogni app di JeffOffice. Non dice nulla su di te. Fai clic su **Copia** per copiarlo. Se le statistiche sono disattivate, o se non è ancora stato generato alcun numero, compare **nessuno**.
- Per far eliminare le statistiche che abbiamo già, invia il tuo numero di installazione a info@jeffoffice.com **prima** di disattivarle. Disattivandole, il numero viene cancellato dal computer e senza di esso non possiamo sapere quali registrazioni sono tue.
- Per disattivare le statistiche senza aprire JeffDeck, imposta la variabile d'ambiente `JP_NO_TELEMETRY` con un valore qualsiasi prima di avviarlo. In questo modo non viene conteggiato né inviato nulla, e JeffDeck non pone la domanda del primo avvio.

**Nome utente**, **Iniziali**, **Tema** e **Lingua di visualizzazione**, così come le scelte su aggiornamenti e statistiche, valgono per tutte le app di JeffOffice: se le modifichi qui, cambiano in ciascuna di esse.

## Correzione

Queste caselle controllano l'ortografia e gli aiuti alla digitazione. Vedi [Formattare il testo](text).

- **Controlla ortografia durante la digitazione**
- **Controlla grammatica durante la digitazione** segnala con una linea blu punteggiata gli errori evidenti di grammatica e stile nei testi in inglese, francese, tedesco e spagnolo.
- **Impostazioni grammatica** apre un elenco dei tipi di problemi da cercare: grammatica, punteggiatura, maiuscole, parole confuse, ripetizioni e stile (disattivato finché non lo attivi).
- **Ignora parole in MAIUSCOLO**
- **Ignora parole che contengono numeri**
- **Sostituisci testo durante la digitazione (Correzione automatica)** corregge gli errori di digitazione più comuni.
- **Sostituisci virgolette dritte con virgolette curve** trasforma " e ' in virgolette curve.
- **Formattazione automatica durante la digitazione (trattini, frazioni, ordinali, elenchi automatici)**
- **Trasforma gli indirizzi web in collegamenti ipertestuali durante la digitazione** trasforma un indirizzo web o di posta elettronica in un collegamento quando premi la barra spaziatrice dopo di esso. Annulla elimina solo il collegamento.

## Visualizza

- **Mostra righello** visualizza i righelli lungo i bordi della diapositiva. È lo stesso comando di **Visualizza > Mostra > Righello**, e vale per ogni finestra, quelle già aperte e quelle che apri in seguito.
- **Blocca oggetti su altri oggetti** fa allineare un oggetto che trascini agli altri oggetti della diapositiva.

## Avanzate

- **Usa il tasto Inserisci per attivare e disattivare la modalità sovrascrittura** è disattivato all'inizio. Quando è attivo, il tasto **Inserisci** passa da un modo di digitare all'altro in una casella di testo. Normalmente ogni lettera che digiti spinge in avanti quelle successive. In modalità **sovrascrittura** ogni lettera che digiti sostituisce quella che si trova dopo il cursore, fino alla fine del paragrafo. La barra di stato mostra **Inserisci** o **Sovrascrittura**, così sai in quale modalità ti trovi. Premi di nuovo **Inserisci** per tornare indietro.

## Salvataggio

- **Incorpora tipi di carattere nel file** inserisce nel file i tipi di carattere usati dalle diapositive, così le diapositive appaiono uguali su un computer che non li ha. È attivo finché non lo deselezioni. Include solo i tipi di carattere forniti con JeffDeck, e rende il file più grande. L'argomento Aprire e salvare presentazioni ne descrive i dettagli, alla voce "Tipi di carattere nel file".
- **Rimuovi le informazioni personali da questo file al salvataggio** elimina dal file scritto (un file .pptx o .odp, un modello e una presentazione inviata per posta elettronica) l'autore, la persona che ha salvato il file per ultima, la società, il responsabile e i nomi, le iniziali e le ore dei commenti. Il contenuto del file resta invariato. Le proprietà personalizzate del file non vengono modificate e il documento aperto non cambia. È la stessa scelta in ogni app di JeffOffice: se la modifichi in una, cambia in tutte. La stessa casella si trova nella pagina **Info**.

## Personalizza

La scheda **Personalizza** ha tre pagine: **Barra multifunzione**, **Barra degli strumenti rapida** e **Tastiera**. La pagina **Tastiera** serve a scegliere i tasti di scelta rapida dei comandi di JeffDeck. Vedi "Scegliere i propri tasti di scelta rapida" nell'argomento sulla tastiera.

### Personalizzare la barra multifunzione

La pagina **Barra multifunzione** elenca le schede della barra, ciascuna con i propri gruppi.

- **Nascondere o mostrare una scheda o un gruppo.** Deseleziona la casella accanto a una scheda o a un gruppo per nasconderlo, e selezionala per mostrarlo di nuovo. Un gruppo nascosto non occupa spazio e i suoi comandi restano attivi con i tasti di scelta rapida. **Home** non ha casella, perché resta sempre visibile, e così il pulsante **File**. Una scheda che compare solo quando si seleziona qualcosa, come un'immagine o una tabella, si trova alla fine dell'elenco e non ha casella.
- **Creare una scheda o un gruppo personalizzato.** Digita un nome nella casella **Nome** e fai clic su **Nuova scheda**. Per creare un gruppo, scegli una scheda e fai clic su **Nuovo gruppo**. Un gruppo personalizzato può stare in qualunque scheda, anche in una che compare solo quando si seleziona qualcosa.
- **Inserire comandi nel proprio gruppo.** Scegli il gruppo, cerca un comando nell'elenco **Comandi** (digita nella casella **Cerca comandi** per restringere l'elenco) e fai clic su **Aggiungi comando**. Può entrare nel gruppo qualsiasi comando che abbia un nome. Un comando può comparire una sola volta in un gruppo.
- **Rinominare.** Scegli una scheda, un gruppo o un comando personalizzato, digita il nuovo nome e fai clic su **Rinomina**. Un comando rinominato esegue la stessa operazione e il nuovo nome è solo il testo del suo pulsante. Per restituire a un comando il proprio nome originale, rinominalo con un nome vuoto.
- **Riordinare.** Scegli una scheda e fai clic su **Sposta su** o **Sposta giù**. Puoi spostare sia le schede della barra sia le tue, ma non le schede che compaiono solo quando si seleziona qualcosa. I tuoi gruppi e i tuoi comandi si spostano allo stesso modo all'interno della rispettiva scheda o gruppo. I gruppi della barra restano nel loro ordine.
- **Eliminare.** Scegli una scheda, un gruppo o un comando personalizzato e fai clic su **Rimuovi**.

**Reimposta uno** ripristina la scheda scelta: viene mostrata, mostrati i suoi gruppi, e spariscono i gruppi che vi hai aggiunto. Una scheda personalizzata viene eliminata. **Reimposta tutto** ripristina l'intera barra multifunzione, compreso l'ordine delle schede.

Non cambia nulla finché non fai clic su **OK**. Poi ogni finestra di JeffDeck aperta mostra la tua barra multifunzione, così come ogni finestra che apri in seguito. JeffDeck conserva solo ciò che differisce dalla barra standard, quindi una scheda o un gruppo aggiunto da una versione successiva compare automaticamente. Le tue schede e i tuoi gruppi ricevono suggerimenti da tastiera (le lettere che compaiono premendo **Alt**) che non si sovrappongono agli altri. Puoi avere 20 schede personalizzate, 30 gruppi personalizzati per scheda e 60 comandi per gruppo.

### Personalizzare la barra degli strumenti rapida

La pagina **Barra degli strumenti rapida** riguarda la piccola barra in alto a sinistra della finestra. L'elenco a sinistra mostra ciò che c'è ora nella barra, mentre l'elenco **Comandi** a destra contiene tutti i comandi che hanno un nome (digita nella casella **Cerca comandi** per restringerlo, come nella pagina **Barra multifunzione**).

- **Aggiungere un comando.** Sceglilo nell'elenco **Comandi** e fai clic su **Aggiungi comando**. Viene messo in fondo alla barra. Un comando che apre un menu mantiene il suo menu nella barra. Un comando può comparire una sola volta nella barra.
- **Clic con il pulsante destro.** Puoi anche fare clic con il pulsante destro su un comando della barra multifunzione e scegliere **Aggiungi alla barra degli strumenti rapida**, oppure fare clic con il pulsante destro su un pulsante della barra e scegliere **Rimuovi dalla barra degli strumenti rapida**. Lo stesso menu contiene **Personalizza barra degli strumenti rapida** e **Personalizza barra multifunzione**, che aprono questa pagina.
- **Spostarla sotto la barra multifunzione.** Seleziona **Mostra barra degli strumenti rapida sotto la barra multifunzione** per mettere la barra su una riga sotto la barra multifunzione anziché in alto a sinistra. Anche i due menu del clic con il pulsante destro contengono **Mostra barra degli strumenti rapida sotto la barra multifunzione**. JeffDeck ricorda la scelta.
- **Aggiungere un separatore.** Fai clic su **Aggiungi separatore** per inserire una linea sottile in fondo, poi spostala dove preferisci.
- **Riordinare.** Scegli un comando o un separatore nell'elenco di sinistra e fai clic su **Sposta su** o **Sposta giù**.
- **Eliminare.** Scegli la voce e fai clic su **Rimuovi**.

**Reimposta uno** ripristina il comando scelto: nella posizione in cui JeffDeck lo mette di solito, oppure fuori dalla barra se JeffDeck di solito non lo tiene lì. **Reimposta tutto** ripristina la barra con cui JeffDeck viene fornito. Reimposta tutto in questa pagina non modifica la barra multifunzione, e Reimposta tutto nella pagina **Barra multifunzione** non modifica la barra degli strumenti.

Non cambia nulla finché non fai clic su **OK**. Poi ogni finestra di JeffDeck aperta mostra la tua barra, così come ogni finestra che apri in seguito. La barra può contenere in tutto 100 comandi e separatori. Quando i pulsanti sono più di quanti ne stiano nel terzo della larghezza della finestra, i restanti si trovano dietro un pulsante **»** alla fine della barra, così il titolo della finestra non viene mai spinto fuori dallo schermo. JeffDeck conserva in elenco un comando che una versione successiva non prevede più, e lo mostra di nuovo se il comando ritorna. I pulsanti ricevono suggerimenti da tastiera, i numeri da 1 a 9 e poi due lettere, nell'ordine in cui si trovano nella barra. La barra si trova sempre in alto a sinistra, accanto al pulsante **File**.

## Sicurezza

La scheda **Sicurezza** decide che cosa fa JeffDeck con ciò a cui un file può rimandare: file collegati, filmati e suoni collegati e file provenienti da Internet. Le macro sono conservate nel file e non vengono mai eseguite, e un collegamento che avvierebbe un programma viene sempre rifiutato. Vedi [Sicurezza](security).

## Argomenti correlati

- [Scheda Visualizza](tab-view)
- [Recuperare il lavoro non salvato](recover)
- [La schermata File](file-screen)
