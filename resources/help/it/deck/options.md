# Modificare le impostazioni di JeffDeck

<!-- keywords: opzioni, impostazioni, preferenze, tema, modalità scura, lingua, nome utente, iniziali, righello, scatto, ortografia, correzione automatica, virgolette, aggiornamenti, beta, statistiche di utilizzo, informazioni, salvataggio, incorpora tipi di carattere, tipi di carattere, personalizza tastiera, personalizza barra multifunzione, personalizza barra degli strumenti rapida, barra degli strumenti rapida, aggiungi un comando alla barra degli strumenti rapida, nascondi una scheda, nascondi un gruppo, aggiungi una scheda, aggiungi un gruppo, rinomina una scheda, reimposta la barra multifunzione, cambia tasti di scelta rapida, riassegna tasti, reimposta tasti -->

Le impostazioni di JeffDeck si trovano nella finestra **Opzioni di JeffDeck**. Per aprirla, fai clic su **File > Opzioni**. Ha cinque schede. Fai clic su **OK** per mantenere le modifiche, oppure su **Annulla** per scartarle.

## Generale

- **Nome utente** e **Iniziali** vengono mostrati nei commenti che scrivi. Vedi [Commenti](comments).
- **Tema** può essere **Usa impostazione di sistema**, **Chiaro** o **Scuro**. La modifica è visibile subito.
- **Lingua di visualizzazione** è la lingua delle parole di JeffDeck. **Usa lingua di sistema** segue il computer. Le altre lingue compaiono in questo elenco quando sono disponibili le traduzioni. Una nuova lingua viene applicata alla successiva apertura di JeffDeck.
- **Verifica aggiornamenti all'avvio di JeffDeck** fa in modo che JeffDeck cerchi una versione più recente ogni volta che si avvia. Se ce n'è una, una finestra **Aggiornamento disponibile** mostra le novità. Scegli **Aggiorna ora**, **Più tardi** o **Ignora questa versione**. In Windows, **Aggiorna ora** scarica il programma di installazione. Installa la nuova versione di tutte le app di JeffOffice e ti chiede di chiudere quelle eventualmente aperte. Sugli altri computer, il pulsante è **Apri pagina di download**. Apre la pagina da cui puoi ottenere la nuova versione. Se è aperto anche JeffPub, solo uno dei due ti segnala una nuova versione ogni giorno.
- **Ricevi le versioni beta (build preliminari per i test)** fa in modo che JeffDeck proponga anche le versioni beta. Escono prima di quelle regolari e possono presentare qualche imperfezione.
- **Invia statistiche di utilizzo anonime** ti permette di condividere dati su come viene usato JeffDeck. Anonime significa che i dati non dicono nulla su chi sei. Quando l'opzione è attiva, JeffDeck invia questi dati una volta al giorno e nel giorno in cui aggiorni: che si tratta di JeffDeck, la sua versione, il sistema operativo e la lingua, quante volte è stato avviato, con quale frequenza viene usato ciascun comando e che tipo di file hai aperto o salvato (ad esempio .pptx). Se JeffDeck si chiude improvvisamente, alla volta successiva in cui si avvia invia anche una segnalazione di arresto anomalo. La segnalazione indica in quale punto del programma si è interrotto, così il problema può essere individuato e risolto. JeffDeck non invia mai i tuoi file, i loro nomi o il loro contenuto, né quello che digiti. Se questa opzione è disattivata, non viene inviata nemmeno alcuna segnalazione di arresto anomalo. JeffDeck lo chiede la prima volta che si avvia, in una finestra chiamata **Aiuta a migliorare JeffDeck**. Non lo chiede se hai già scelto in JeffPub o durante l'installazione.
- **Informativa sulla privacy** accanto a questa casella apre la nostra informativa all'indirizzo jeffoffice.com/privacy. Spiega quali dati vengono inviati, dove vanno e per quanto tempo li conserviamo. Conserviamo le statistiche per 24 mesi.
- **Numero di installazione** è mostrato sotto la casella di controllo. È un numero casuale generato per la tua copia di JeffOffice, uguale in tutte le app di JeffOffice. Non dice nulla su di te. Fai clic su **Copia** per copiarlo. Se le statistiche sono disattivate, o se non è ancora stato generato un numero, viene mostrato **nessuno**.
- Per far cancellare le statistiche che abbiamo già, invia il tuo numero di installazione a info@jeffoffice.com **prima** di disattivarle. Disattivandole, il numero viene cancellato dal computer e, senza di esso, non possiamo stabilire quali registrazioni sono tue.
- Per disattivare le statistiche senza aprire JeffDeck, imposta la variabile d'ambiente `JP_NO_TELEMETRY` con un valore qualsiasi prima di avviarlo. Così non viene conteggiato né inviato nulla, e JeffDeck non pone la domanda della prima esecuzione.

**Nome utente**, **Iniziali**, **Tema** e **Lingua di visualizzazione**, nonché le scelte su aggiornamenti e statistiche, valgono per tutte le app di JeffOffice: se cambi una di queste impostazioni qui, cambia in ciascuna di esse.

## Verifica ortografia

Queste caselle controllano l'ortografia e gli aiuti alla digitazione. Vedi [Digitare e formattare il testo](text).

- **Controlla ortografia durante la digitazione**
- **Ignora parole in MAIUSCOLO**
- **Ignora parole che contengono numeri**
- **Sostituisci testo durante la digitazione (Correzione automatica)** corregge gli errori di digitazione più comuni.
- **Sostituisci virgolette dritte con virgolette curve** trasforma " e ' in virgolette curve.
- **Formattazione automatica durante la digitazione (trattini, frazioni, ordinali, elenchi automatici)**
- **Trasforma gli indirizzi web in collegamenti ipertestuali durante la digitazione** trasforma un indirizzo web o di posta elettronica in un collegamento quando premi la barra spaziatrice dopo di esso. Annulla elimina solo il collegamento.

## Visualizza

- **Mostra righelli** mostra i righelli lungo i bordi della diapositiva. Equivale a **Visualizza > Mostra > Righello** e vale per ogni finestra, quelle già aperte e quelle che apri in seguito.
- **Blocca gli oggetti sugli altri oggetti** fa allineare un oggetto che trascini con gli altri oggetti della diapositiva.

## Salva

- **Incorpora tipi di carattere nel file** inserisce nel file, al salvataggio, i tipi di carattere usati nelle diapositive, così le diapositive hanno lo stesso aspetto anche su un computer che non dispone di quei tipi di carattere. L'opzione è attiva finché non la disattivi. Vengono incorporati solo i tipi di carattere forniti con JeffDeck, e il file diventa più grande. L'argomento Aprire e salvare presentazioni contiene i dettagli, alla voce "Tipi di carattere nel file".
- **Rimuovi le informazioni personali da questo file al salvataggio** elimina dal file scritto (un file .pptx o .odp, un modello e una presentazione inviata per posta elettronica) l'autore, la persona che ha salvato per ultima il file, la società, il responsabile e i nomi, le iniziali e gli orari dei commenti. Il contenuto del file resta com'è. Le proprietà personalizzate del file vengono lasciate invariate e il documento aperto non viene modificato. È la stessa scelta in ogni app di JeffOffice: cambiala in una e cambia in tutte. La stessa casella si trova nella pagina **Informazioni**.

## Personalizza

La scheda **Personalizza** ha tre pagine: **Barra multifunzione**, **Barra degli strumenti rapida** e **Tastiera**. La pagina **Tastiera** serve a scegliere i tasti di scelta rapida dei comandi di JeffDeck. Vedi "Scegliere i propri tasti di scelta rapida" nell'argomento sulla tastiera.

### Personalizzare la barra multifunzione

La pagina **Barra multifunzione** elenca le schede della barra, ciascuna con i suoi gruppi.

- **Nascondi o mostra una scheda o un gruppo.** Deseleziona la casella accanto per nasconderlo e selezionala per mostrarlo di nuovo. Un gruppo nascosto non occupa spazio, e i suoi comandi restano disponibili con i tasti di scelta rapida. **Home** non ha casella, perché resta sempre visibile, e così il pulsante **File**. Una scheda che compare solo quando è selezionato qualcosa, come un'immagine o una tabella, si trova alla fine dell'elenco e non ha casella.
- **Crea una scheda o un gruppo personalizzato.** Digita un nome nella casella **Nome** e fai clic su **Nuova scheda**. Per creare un gruppo, scegli una scheda e fai clic su **Nuovo gruppo**. Un gruppo personalizzato può stare in qualsiasi scheda, anche in una scheda che compare solo quando è selezionato qualcosa.
- **Inserisci comandi nel tuo gruppo.** Scegli il gruppo, cerca un comando nell'elenco **Comandi** (digita nella casella **Cerca comandi** per restringere l'elenco) e fai clic su **Aggiungi comando**. Può essere inserito qualsiasi comando che ha un nome. Un comando può trovarsi in un gruppo una sola volta.
- **Rinomina.** Scegli una scheda, un gruppo o un comando personalizzato, digita il nuovo nome e fai clic su **Rinomina**. Un comando rinominato esegue la stessa operazione di prima, e il nuovo nome è solo il testo del suo pulsante. Per dare a un comando il suo nome originale, rinominalo con un nome vuoto.
- **Riordina.** Scegli una scheda e fai clic su **Sposta su** o **Sposta giù**. Puoi spostare sia le schede della barra sia le tue, ma non le schede che compaiono solo quando è selezionato qualcosa. I tuoi gruppi e i tuoi comandi si spostano allo stesso modo all'interno della loro scheda o gruppo. I gruppi propri della barra mantengono il loro ordine.
- **Rimuovi.** Scegli una scheda, un gruppo o un comando personalizzato e fai clic su **Rimuovi**.

**Reimposta una** ripristina la scheda scelta: viene mostrata, sono mostrati i suoi gruppi e i gruppi che le hai aggiunto spariscono. Una scheda personalizzata viene eliminata. **Reimposta tutto** ripristina l'intera barra, compreso l'ordine delle schede.

Non cambia nulla finché non fai clic su **OK**. Poi ogni finestra di JeffDeck aperta mostra la tua barra, e così ogni finestra che apri in seguito. JeffDeck conserva solo ciò che differisce dalla barra standard, quindi una scheda o un gruppo aggiunto da una versione successiva compare automaticamente. Le tue schede e i tuoi gruppi ricevono suggerimenti da tastiera (le lettere mostrate premendo **Alt**) che non si sovrappongono agli altri. Puoi avere 20 schede personalizzate, 30 gruppi personalizzati per scheda e 60 comandi per gruppo.

### Personalizzare la barra degli strumenti rapida

La pagina **Barra degli strumenti rapida** serve per la piccola barra in alto a sinistra della finestra. L'elenco a sinistra mostra ciò che è presente ora nella barra, mentre l'elenco **Comandi** a destra contiene tutti i comandi che hanno un nome (digita nella casella **Cerca comandi** per restringerlo, come nella pagina **Barra multifunzione**).

- **Aggiungi un comando.** Scegli il comando nell'elenco **Comandi** e fai clic su **Aggiungi comando**. Viene messo alla fine della barra. Un comando che apre un menu mantiene il suo menu nella barra. Un comando può trovarsi nella barra una sola volta.
- **Clic destro.** Puoi anche fare clic destro su un comando della barra multifunzione e scegliere **Aggiungi alla barra degli strumenti rapida**, oppure fare clic destro su un pulsante della barra degli strumenti rapida e scegliere **Rimuovi dalla barra degli strumenti rapida**. Lo stesso menu contiene **Personalizza la barra degli strumenti rapida** e **Personalizza la barra multifunzione**, che aprono questa pagina.
- **Aggiungi un separatore.** Fai clic su **Aggiungi separatore** per mettere una linea sottile alla fine, poi spostala dove preferisci.
- **Riordina.** Scegli un comando o un separatore nell'elenco a sinistra e fai clic su **Sposta su** o **Sposta giù**.
- **Rimuovi.** Scegli l'elemento e fai clic su **Rimuovi**.

**Reimposta una** ripristina il comando scelto: nella posizione in cui JeffDeck di solito lo mette, oppure fuori dalla barra se di norma JeffDeck non lo inserisce lì. **Reimposta tutto** ripristina la barra con cui JeffDeck viene fornito. Reimposta tutto in questa pagina non modifica la barra multifunzione, e Reimposta tutto nella pagina **Barra multifunzione** non modifica la barra degli strumenti.

Non cambia nulla finché non fai clic su **OK**. Poi ogni finestra di JeffDeck aperta mostra la tua barra, e così ogni finestra che apri in seguito. La barra può contenere in tutto 100 comandi e separatori. Quando i pulsanti sono più di quanti ne entrano in un terzo della larghezza della finestra, i restanti si trovano dietro un pulsante **»** alla fine della barra, così il titolo della finestra non viene mai spinto fuori dallo schermo. JeffDeck conserva in elenco un comando che una versione successiva non prevede più, e lo mostra di nuovo se il comando ritorna. I pulsanti ricevono suggerimenti da tastiera, i numeri da 1 a 9 e poi due lettere, nell'ordine in cui si trovano nella barra. La barra si trova sempre in alto a sinistra, accanto al pulsante **File**.

## Vedi anche

- [Scheda Visualizza](tab-view)
- [Recuperare il lavoro non salvato](recover)
- [La schermata File](file-screen)
