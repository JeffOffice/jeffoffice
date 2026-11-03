# Modificare le impostazioni di JeffDeck
<!-- keywords: opzioni, impostazioni, preferenze, tema, modalità scura, tema scuro, lingua, nome utente, iniziali, righello, blocca, allinea, controllo ortografia, correzione automatica, virgolette, aggiornamenti, beta, statistiche di utilizzo, informazioni, salvataggio, incorpora tipi di carattere, tipi di carattere, font -->

Le impostazioni si trovano nella finestra **Opzioni di JeffDeck**. Per aprirla, fare clic su **File > Opzioni**. La finestra contiene quattro schede. Fare clic su **OK** per mantenere le modifiche oppure su **Annulla** per scartarle.

## Generale

- **Nome utente** e **Iniziali** vengono visualizzati nei commenti che si scrivono. Vedere [Commenti](comments).
- **Tema** può essere **Usa impostazione di sistema**, **Chiaro** o **Scuro**. La modifica è immediata.
- **Lingua di visualizzazione** è la lingua dei testi di JeffDeck. **Usa lingua di sistema** segue l'impostazione del computer. Le altre lingue compaiono nell'elenco quando sono disponibili le traduzioni. Una nuova lingua viene applicata alla successiva apertura di JeffDeck.
- **Controlla aggiornamenti all'avvio di JeffDeck** fa sì che JeffDeck cerchi una versione più recente a ogni avvio. Se ne esiste una, la finestra **Aggiornamento disponibile** mostra le novità. Scegliere **Aggiorna ora**, **Più tardi** o **Ignora questa versione**. In Windows, **Aggiorna ora** scarica il programma di installazione, che installa la nuova versione di tutte le app di JeffOffice e chiede di chiudere quelle aperte. Negli altri computer il pulsante è **Apri pagina di download** e apre la pagina da cui scaricare la nuova versione. Se è aperto anche JeffPub, solo uno dei due segnala la nuova versione ogni giorno.
- **Ricevi le versioni beta (build preliminari per i test)** fa sì che JeffDeck proponga anche le versioni beta. Queste escono prima delle versioni normali e potrebbero non essere del tutto rifinite.
- **Invia statistiche di utilizzo anonime** consente di condividere informazioni sull'uso di JeffDeck. Anonime significa che le informazioni non dicono nulla sulla propria identità. Quando l'opzione è attiva, JeffDeck invia queste informazioni una volta al giorno e il giorno dell'aggiornamento: che si tratta di JeffDeck, la versione, il sistema operativo e la lingua, quante volte è stato avviato, con quale frequenza viene usato ogni comando e il tipo di file aperto o salvato (ad esempio .pptx). Se JeffDeck si chiude in modo imprevisto, al successivo avvio invia anche un rapporto sull'arresto anomalo, che indica il punto del programma in cui si è interrotto, così che il problema possa essere individuato e risolto. JeffDeck non invia mai i file, i loro nomi o il loro contenuto, né il testo digitato. Se l'opzione è disattivata, non viene inviato neanche il rapporto sugli arresti anomali. JeffDeck lo chiede al primo avvio, in una finestra denominata **Contribuisci a migliorare JeffDeck**. Non lo chiede se la scelta è già stata fatta in JeffPub o durante l'installazione.
- **Informativa sulla privacy**, accanto a questa casella di controllo, apre la nostra informativa sulla privacy all'indirizzo jeffoffice.com/privacy, che spiega cosa viene inviato, dove va e per quanto tempo lo conserviamo. Le statistiche vengono conservate per 24 mesi.
- **Numero di installazione** è visualizzato sotto la casella di controllo. È un numero casuale generato per la propria copia di JeffOffice, uguale in tutte le app di JeffOffice, e non dice nulla sull'utente. Fare clic su **Copia** per copiarlo. Se le statistiche sono disattivate o il numero non è ancora stato generato, viene visualizzato **nessuno**.
- Per far eliminare le statistiche già in nostro possesso, inviare il numero di installazione a info@jeffoffice.com **prima** di disattivare le statistiche. La disattivazione cancella il numero dal computer e senza di esso non è possibile sapere quali dati sono i propri.
- Per disattivare le statistiche senza aprire JeffDeck, impostare la variabile di ambiente `JP_NO_TELEMETRY` su un valore qualsiasi prima dell'avvio. In questo caso nulla viene conteggiato o inviato e JeffDeck non pone la domanda del primo avvio.

**Nome utente**, **Iniziali**, **Tema**, **Lingua di visualizzazione** e le scelte relative ad aggiornamenti e statistiche sono comuni a tutte le app di JeffOffice: se si modifica un'impostazione qui, cambia in tutte le app.

## Strumenti di correzione

Queste caselle di controllo gestiscono il controllo ortografia e l'assistenza alla digitazione. Vedere [Digitare e formattare il testo](text).

- **Controlla ortografia durante la digitazione**
- **Ignora parole in MAIUSCOLO**
- **Ignora parole che contengono numeri**
- **Sostituisci testo durante la digitazione (Correzione automatica)** corregge gli errori di digitazione più comuni.
- **Sostituisci virgolette dritte con virgolette curve** trasforma " e ' in virgolette curve.
- **Formattazione automatica durante la digitazione (trattini, frazioni, ordinali, elenchi automatici)**

## Visualizza

- **Mostra il righello** visualizza i righelli lungo i bordi della diapositiva. Equivale a **Visualizza > Mostra > Righello** e si applica a tutte le finestre, sia quelle aperte sia quelle che si apriranno.
- **Blocca gli oggetti sugli altri oggetti** allinea l'oggetto trascinato agli altri oggetti della diapositiva.

## Salvataggio

- **Incorpora tipi di carattere nel file** inserisce nel file, al salvataggio, i tipi di carattere usati nelle diapositive, in modo che abbiano lo stesso aspetto anche in un computer in cui tali tipi di carattere non sono installati. L'opzione è attiva finché non la si deseleziona. Incorpora solo i tipi di carattere forniti con JeffDeck e aumenta le dimensioni del file. Per i dettagli, vedere l'argomento Aprire e salvare presentazioni, nella sezione "Tipi di carattere nel file".

## Argomenti correlati

- [Scheda Visualizza](tab-view)
- [Recuperare il lavoro non salvato](recover)
- [La schermata File](file-screen)
