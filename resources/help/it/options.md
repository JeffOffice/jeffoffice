# Modificare le impostazioni di JeffPub
<!-- keywords: opzioni, impostazioni, preferenze, tema, modalità scura, tema scuro, unità di misura, centimetri, pollici, recupero automatico, backup, copia di backup, lingua, nome utente, spostare leggermente, aggiornamenti, beta, statistiche di utilizzo, informazioni -->

Le impostazioni si trovano nella finestra **Opzioni di JeffPub**. Per aprirla, fare clic su **File > Opzioni**. La finestra contiene quattro schede. Fare clic su **OK** per mantenere le modifiche oppure su **Annulla** per ignorarle.

## Generale

- **Nome utente** e **Iniziali** vengono salvati come autore delle nuove pubblicazioni e visualizzati nei commenti.
- **Tema** può essere **Usa impostazione di sistema**, **Chiaro** o **Scuro**. La modifica viene applicata immediatamente.
- **Lingua di visualizzazione** è la lingua dell'interfaccia di JeffPub. **Usa lingua di sistema** segue l'impostazione del computer. Le altre lingue compaiono nell'elenco quando sono disponibili le traduzioni. La nuova lingua viene applicata alla successiva apertura di JeffPub.
- **Invia statistiche di utilizzo anonime** e **Controlla aggiornamenti all'avvio di JeffPub** corrispondono alle opzioni della pagina **Informazioni** (descritta più avanti).

**Nome utente**, **Iniziali**, **Tema**, **Lingua di visualizzazione** e le opzioni relative ad aggiornamenti e statistiche sono comuni a tutte le app di JeffOffice: se si modifica una di queste impostazioni qui, la modifica viene applicata a tutte le app.

## Strumenti di correzione

Queste caselle di controllo gestiscono il controllo ortografia e gli aiuti alla digitazione. Vedere [Controllo ortografia e sillabazione](proofing).

- **Controlla ortografia durante la digitazione**
- **Ignora parole in MAIUSCOLO**
- **Ignora parole che contengono numeri**
- **Sostituisci testo durante la digitazione (Correzione automatica)** corregge gli errori di digitazione più comuni.
- **Sostituisci virgolette dritte con virgolette curve** trasforma " e ' in virgolette curve.
- **Formattazione automatica durante la digitazione (trattini, frazioni, numeri ordinali, elenchi automatici)**

## Salvataggio

- **Salva informazioni di recupero automatico ogni** imposta la frequenza con cui JeffPub crea copie di riserva del lavoro non salvato, da 1 a 120 minuti. Vedere [Recuperare il lavoro non salvato](recover).
- **Crea sempre una copia di backup** conserva il file precedente come "Backup di" seguito dal nome del file a ogni salvataggio.

## Avanzate

- **Unità di misura** può essere **Pollici**, **Centimetri**, **Millimetri**, **Punti** o **Pica**. L'unità scelta si applica ai righelli e a tutte le caselle di misura. Finché non si effettua una scelta, JeffPub usa i pollici se nell'area geografica del computer si misura in pollici, come negli Stati Uniti, e i centimetri in tutti gli altri casi. Un punto corrisponde a 1/72 di pollice e una pica a 12 punti.
- **I tasti di direzione spostano leggermente gli oggetti di** imposta la distanza di cui un tasto di direzione sposta l'oggetto selezionato.
- **Mostra questo numero di pubblicazioni recenti** indica quanti nomi vengono elencati nella pagina **Apri**, da 0 a 50.
- **Durante la selezione, seleziona automaticamente l'intera parola** seleziona parole intere quando si trascina il puntatore sul testo.
- **Consenti trascinamento della selezione del testo** permette di spostare il testo selezionato trascinandolo.
- **Sillabazione automatica nelle nuove caselle di testo** attiva la sillabazione, che divide le parole lunghe alla fine della riga, nelle caselle di testo aggiunte.

## Aggiornamenti, statistiche e nome

Per trovare queste opzioni, fare clic su **File > Informazioni**. Le due caselle di controllo si trovano anche nella scheda **Generale** di **File > Opzioni**, insieme a una terza, **Ricevi le versioni beta**.

- **Verifica disponibilità aggiornamenti** cerca subito una versione più recente. Se è disponibile, una finestra **Aggiornamento disponibile** mostra le novità. Scegliere **Aggiorna ora**, **Più tardi** o **Ignora questa versione**.
- **Ricevi le versioni beta (build preliminari per i test)** fa sì che JeffPub proponga anche le versioni beta, che escono prima di quelle normali e potrebbero contenere imperfezioni.
- **Verifica automaticamente la disponibilità di aggiornamenti** fa sì che JeffPub esegua la verifica a ogni avvio.
- **Invia statistiche di utilizzo anonime** permette di scegliere se condividere informazioni sull'uso di JeffPub. Quando l'opzione è attiva, JeffPub invia queste informazioni una volta al giorno e il giorno in cui viene aggiornato: la versione del programma, il sistema operativo e la lingua, e la frequenza di utilizzo di ciascun comando. Se JeffPub si chiude in modo imprevisto, al successivo avvio invia anche un rapporto sull'arresto anomalo. Il rapporto indica il punto del programma in cui si è verificata l'interruzione, in modo che il problema possa essere individuato e risolto. Non vengono mai inviati i file, i loro nomi o il loro contenuto, né il testo digitato. Fare clic su **Dati inviati** per maggiori informazioni. JeffPub pone questa domanda al primo avvio, in una finestra denominata **Aiuta a migliorare JeffPub**. In Windows, la domanda potrebbe essere posta prima dal programma di installazione. La scelta vale per tutte le app di JeffOffice: JeffDeck invia lo stesso tipo di informazioni su sé stesso, solo se si è scelto di condividerle.
- **Informativa sulla privacy**, accanto a questa casella di controllo, apre la nostra informativa sulla privacy all'indirizzo jeffoffice.com/privacy. L'informativa spiega quali dati vengono inviati, dove vengono inviati e per quanto tempo vengono conservati. Le statistiche vengono conservate per 24 mesi.
- **Numero di installazione** è visualizzato sotto la casella di controllo. Si tratta di un numero casuale generato per la propria copia di JeffOffice, uguale in tutte le app di JeffOffice. Non contiene alcuna informazione personale. Fare clic su **Copia** per copiarlo. Se le statistiche sono disattivate o il numero non è ancora stato generato, viene visualizzato **nessuno**.
- Per far eliminare le statistiche già in nostro possesso, inviare il numero di installazione a info@jeffoffice.com **prima** di disattivare le statistiche. La disattivazione cancella il numero dal computer e, senza di esso, non è possibile individuare i record corrispondenti.
- Per disattivare definitivamente le statistiche senza aprire JeffPub, impostare la variabile di ambiente `JP_NO_TELEMETRY` su un valore qualsiasi prima di avviare il programma. In questo caso non viene conteggiato né inviato nulla e JeffPub non pone la domanda del primo avvio.
- **Nome** corrisponde al **Nome utente** descritto sopra.

## Argomenti correlati

- [Recuperare il lavoro non salvato](recover)
- [Controllo ortografia e sillabazione](proofing)
- [Ottenere assistenza e segnalare problemi](support)
