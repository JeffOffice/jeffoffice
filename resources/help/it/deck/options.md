# Modifica le impostazioni di JeffDeck
<!-- keywords: opzioni, impostazioni, preferenze, tema, modalità scura, lingua, nome utente, iniziali, righello, scatto, ortografia, correzione automatica, virgolette, aggiornamenti, beta, statistiche di utilizzo, informazioni, salva, incorpora tipi di carattere, tipi di carattere, personalizza tastiera, personalizza barra multifunzione, personalizza barra degli strumenti rapida, barra degli strumenti rapida, aggiungi comando alla barra degli strumenti rapida, nascondi scheda, nascondi gruppo, aggiungi scheda, aggiungi gruppo, rinomina scheda, reimposta barra multifunzione, cambia tasti di scelta rapida, riassegna tasti, reimposta tasti -->

Le impostazioni si trovano nella finestra **Opzioni di JeffDeck**. Per aprirla, fai clic su **File > Opzioni**. Contiene cinque schede. Fai clic su **OK** per mantenere le modifiche oppure su **Annulla** per eliminarle.

## Generale

- **Nome utente** e **Iniziali** vengono mostrati nei commenti che scrivi. Vedi [Commenti](comments).
- **Tema** può essere **Usa impostazione di sistema**, **Chiaro** o **Scuro**. La modifica viene applicata subito.
- **Lingua di visualizzazione** è la lingua delle scritte di JeffDeck. **Usa lingua di sistema** segue la lingua del computer. Le altre lingue compaiono in questo elenco quando sono disponibili le traduzioni. Una nuova lingua viene applicata la volta successiva che apri JeffDeck.
- **Verifica aggiornamenti all'avvio di JeffDeck** fa sì che JeffDeck cerchi una versione più recente ogni volta che si avvia. Se ce n'è una, la finestra **Aggiornamento disponibile** mostra le novità. Scegli **Aggiorna ora**, **Più tardi** oppure **Ignora questa versione**. In Windows, **Aggiorna ora** scarica il programma di installazione. Questo installa la nuova versione di tutte le app di JeffOffice e ti chiede di chiudere quelle aperte. Negli altri computer il pulsante è **Apri pagina di download**. Apre la pagina da cui puoi ottenere la nuova versione. Se anche JeffPub è aperto, solo uno dei due ti segnala una nuova versione ogni giorno.
- **Ricevi le versioni beta (build preliminari per i test)** fa sì che JeffDeck proponga anche le versioni beta. Escono prima di quelle regolari e possono presentare qualche imperfezione.
- **Invia statistiche di utilizzo anonime** consente di condividere informazioni su come viene usato JeffDeck. Anonime significa che le informazioni non dicono nulla su chi sei. Quando l'opzione è attiva, JeffDeck invia questi dati una volta al giorno e il giorno in cui aggiorni: che si tratta di JeffDeck, la sua versione, il sistema operativo e la lingua, quante volte è stato avviato, con quale frequenza viene usato ciascun comando e che tipo di file hai aperto o salvato (ad esempio .pptx). Se JeffDeck si chiude improvvisamente, alla volta successiva in cui si avvia invia anche una segnalazione di arresto anomalo. La segnalazione indica in quale punto del programma si è interrotto, in modo che il problema possa essere individuato e risolto. JeffDeck non invia mai i tuoi file, i loro nomi o il loro contenuto, né quanto digiti. Se l'opzione è disattivata, non viene inviata nemmeno alcuna segnalazione di arresto anomalo. JeffDeck chiede di questo la prima volta che si avvia, nella finestra **Aiutaci a migliorare JeffDeck**. La domanda non viene posta se hai già scelto in JeffPub o durante l'installazione.
- **Informativa sulla privacy**, accanto a questa casella, apre la nostra informativa all'indirizzo jeffoffice.com/privacy. Spiega quali dati vengono inviati, dove vanno e per quanto tempo li conserviamo. Le statistiche vengono conservate per 24 mesi.
- **Numero di installazione** è mostrato sotto la casella. È un numero casuale generato per la tua copia di JeffOffice, uguale in tutte le app di JeffOffice. Non dice nulla su di te. Fai clic su **Copia** per copiarlo. Se le statistiche sono disattivate, o se non è ancora stato generato alcun numero, viene mostrato **nessuno**.
- Per far eliminare le statistiche che abbiamo già, invia il tuo numero di installazione a info@jeffoffice.com **prima** di disattivarle. Disattivandole, il numero viene cancellato dal computer e, senza di esso, non possiamo capire quali registrazioni sono tue.
- Per disattivare le statistiche senza aprire JeffDeck, imposta la variabile d'ambiente `JP_NO_TELEMETRY` su un valore qualsiasi prima di avviarlo. Così nulla viene conteggiato né inviato, e JeffDeck non pone la domanda della prima esecuzione.

**Nome utente**, **Iniziali**, **Tema** e **Lingua di visualizzazione**, così come le scelte su aggiornamenti e statistiche, valgono per tutte le app di JeffOffice: se le cambi qui, cambiano in ciascuna di esse.

## Correzione

Queste caselle controllano l'ortografia e gli aiuti alla digitazione. Vedi [Digitare e formattare il testo](text).

- **Controlla ortografia durante la digitazione**
- **Verifica grammatica durante la digitazione** segnala con una linea blu tratteggiata gli errori di grammatica e di stile evidenti in testi inglesi, francesi, tedeschi e spagnoli.
- **Impostazioni grammatica** apre un elenco dei tipi di problemi da cercare: grammatica, punteggiatura, maiuscole, parole confuse, ripetizioni e stile (disattivato finché non lo attivi).
- **Ignora parole in MAIUSCOLO**
- **Ignora parole che contengono numeri**
- **Sostituisci testo durante la digitazione (Correzione automatica)** corregge i comuni errori di digitazione.
- **Sostituisci virgolette dritte con virgolette curve** trasforma " e ' in virgolette curve.
- **Formatta automaticamente durante la digitazione (trattini, frazioni, numeri ordinali, elenchi automatici)**
- **Trasforma gli indirizzi web in collegamenti ipertestuali durante la digitazione** rende un indirizzo web o di posta elettronica un collegamento quando premi la barra spaziatrice dopo di esso. Annulla elimina solo il collegamento.

## Visualizza

- **Mostra righello** mostra i righelli lungo i bordi della diapositiva. È la stessa cosa di **Visualizza > Mostra > Righello** e vale per ogni finestra, sia quelle già aperte sia quelle che apri in seguito.
- **Blocca gli oggetti sugli altri oggetti** fa sì che un oggetto trascinato si allinei con gli altri oggetti della diapositiva.

## Salva

- **Incorpora tipi di carattere nel file** inserisce nel file, al salvataggio, i tipi di carattere usati nelle diapositive, così le diapositive hanno lo stesso aspetto anche su un computer che non ha quei tipi di carattere. È attiva finché non la disattivi. Vengono inseriti solo i tipi di carattere forniti con JeffDeck, e il file diventa più grande. Per i dettagli, vedi l'argomento Aprire e salvare presentazioni, nella sezione "Tipi di carattere nel file".
- **Rimuovi le informazioni personali da questo file al salvataggio** elimina autore, persona che ha salvato per ultima il file, società, responsabile e nomi, iniziali e orari dei commenti, dal file che viene scritto (un file .pptx o .odp, un modello e una presentazione inviata per posta elettronica). Il contenuto del file resta com'è. Le proprietà personalizzate del file vengono lasciate invariate e il documento aperto non viene modificato. È la stessa scelta in ogni app di JeffOffice: cambiala in una e cambia in tutte. La stessa casella si trova nella pagina **Informazioni**.

## Personalizza

La scheda **Personalizza** contiene tre pagine: **Barra multifunzione**, **Barra degli strumenti rapida** e **Tastiera**. La pagina **Tastiera** è dove scegli i tasti di scelta rapida per i comandi di JeffDeck. Vedi "Scegliere i tasti di scelta rapida" nell'argomento sulla tastiera.

### Personalizzare la barra multifunzione

La pagina **Barra multifunzione** elenca le schede della barra, ciascuna con i suoi gruppi.

- **Nascondere o mostrare una scheda o un gruppo.** Deseleziona la casella accanto a una scheda o a un gruppo per nasconderlo, e selezionala per mostrarlo di nuovo. Un gruppo nascosto non occupa spazio, e i suoi comandi continuano a funzionare con i tasti di scelta rapida. **Home** non ha la casella, perché resta sempre visibile, e così il pulsante **File**. Una scheda che compare solo quando è selezionato qualcosa, come un'immagine o una tabella, si trova in fondo all'elenco e non ha la casella.
- **Creare una scheda o un gruppo personalizzato.** Digita un nome nella casella **Nome** e fai clic su **Nuova scheda**. Per creare un gruppo, scegli una scheda e fai clic su **Nuovo gruppo**. Un gruppo personalizzato può stare in qualsiasi scheda, anche in una scheda che compare solo quando è selezionato qualcosa.
- **Inserire comandi nel tuo gruppo.** Scegli il tuo gruppo, trova un comando nell'elenco **Comandi** (digita nella casella **Cerca comandi** per restringere l'elenco) e fai clic su **Aggiungi comando**. Può essere inserito qualsiasi comando che abbia un nome. Un comando può stare in un gruppo una sola volta.
- **Rinominare.** Scegli una scheda, un gruppo o un comando personalizzato, digita il nuovo nome e fai clic su **Rinomina**. Un comando rinominato fa la stessa cosa di prima, e il nuovo nome è solo la scritta del suo pulsante. Rinomina un comando con un nome vuoto per ridargli il suo nome originale.
- **Riordinare.** Scegli una scheda e fai clic su **Sposta su** o **Sposta giù**. Puoi spostare le schede della barra oltre alle tue, ma non quelle che compaiono solo quando è selezionato qualcosa. I tuoi gruppi e i tuoi comandi si spostano allo stesso modo all'interno della loro scheda o del loro gruppo. I gruppi della barra mantengono il loro ordine.
- **Eliminare.** Scegli una scheda, un gruppo o un comando personalizzato e fai clic su **Rimuovi**.

**Reimposta una** riporta la scheda scelta com'era: viene mostrata, sono mostrati i suoi gruppi e i gruppi che vi hai aggiunto spariscono. Una scheda personalizzata viene eliminata. **Reimposta tutto** riporta com'era l'intera barra multifunzione, compreso l'ordine delle schede.

Nulla cambia finché non fai clic su **OK**. Poi ogni finestra aperta di JeffDeck mostra la tua barra multifunzione, e così ogni finestra che apri in seguito. JeffDeck conserva solo ciò che differisce dalla barra standard, quindi una scheda o un gruppo aggiunto da una versione successiva compare da solo. Le tue schede e i tuoi gruppi ricevono suggerimenti da tastiera (le lettere che compaiono quando premi **Alt**) che non si sovrappongono agli altri. Puoi avere 20 schede personalizzate, 30 gruppi personalizzati per scheda e 60 comandi per gruppo.

### Personalizzare la barra degli strumenti rapida

La pagina **Barra degli strumenti rapida** riguarda la piccola barra in alto a sinistra della finestra. Il suo elenco di sinistra mostra ciò che c'è ora nella barra, e l'elenco **Comandi** a destra contiene tutti i comandi che hanno un nome (digita nella casella **Cerca comandi** per restringerlo, come nella pagina **Barra multifunzione**).

- **Aggiungere un comando.** Scegli un comando nell'elenco **Comandi** e fai clic su **Aggiungi comando**. Viene aggiunto alla fine della barra. Un comando che apre un menu mantiene il suo menu nella barra. Un comando può essere nella barra una sola volta.
- **Clic destro.** Puoi anche fare clic destro su un comando qualsiasi della barra multifunzione e scegliere **Aggiungi alla barra degli strumenti rapida**, oppure fare clic destro su un pulsante della barra e scegliere **Rimuovi dalla barra degli strumenti rapida**. Lo stesso menu contiene **Personalizza barra degli strumenti rapida** e **Personalizza barra multifunzione**, che aprono questa pagina.
- **Spostarla sotto la barra multifunzione.** Seleziona **Mostra barra degli strumenti rapida sotto la barra multifunzione** per mettere la barra su una riga sotto la barra multifunzione invece che in alto a sinistra. Anche i due menu del clic destro contengono **Mostra barra degli strumenti rapida sotto la barra multifunzione**. JeffDeck ricorda la scelta.
- **Aggiungere un separatore.** Fai clic su **Aggiungi separatore** per inserire una linea sottile alla fine, poi spostala dove vuoi.
- **Riordinare.** Scegli un comando o un separatore nell'elenco di sinistra e fai clic su **Sposta su** o **Sposta giù**.
- **Eliminare.** Scegli l'elemento e fai clic su **Rimuovi**.

**Reimposta una** riporta il comando scelto com'era: nel punto in cui JeffDeck di solito lo mette, oppure fuori dalla barra se di solito JeffDeck non lo mette lì. **Reimposta tutto** riporta la barra degli strumenti rapida com'è in JeffDeck. Reimposta tutto in questa pagina non modifica la barra multifunzione, e Reimposta tutto nella pagina **Barra multifunzione** non modifica la barra degli strumenti rapida.

Nulla cambia finché non fai clic su **OK**. Poi ogni finestra aperta di JeffDeck mostra la tua barra degli strumenti, e così ogni finestra che apri in seguito. La barra può contenere in tutto 100 comandi e separatori. Quando i pulsanti sono più di quanti ne stanno in un terzo della larghezza della finestra, i restanti si trovano dietro un pulsante **»** alla fine della barra, così il titolo della finestra non viene mai spinto fuori dallo schermo. JeffDeck mantiene nel tuo elenco un comando che una versione successiva non ha più, e lo mostra di nuovo se il comando ritorna. I pulsanti ricevono suggerimenti da tastiera, i numeri da 1 a 9 e poi due lettere, nell'ordine in cui si trovano sulla barra. La barra si trova sempre in alto a sinistra, accanto al pulsante **File**.

## Vedi anche

- [Scheda Visualizza](tab-view)
- [Recuperare il lavoro non salvato](recover)
- [La schermata File](file-screen)
