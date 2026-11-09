# Modifica le impostazioni di JeffPub
<!-- keywords: opzioni, impostazioni, preferenze, tema, modalità scura, unità di misura, pollici, recupero automatico, backup, lingua, nome utente, sposta leggermente, aggiornamenti, beta, statistiche di utilizzo, informazioni -->

Le impostazioni si trovano nella finestra **Opzioni di JeffPub**. Fai clic su **File > Opzioni** per aprirla. Ha quattro schede. Fai clic su **OK** per mantenere le modifiche oppure su **Annulla** per scartarle.

## Generale

- **Nome utente** e **Iniziali** vengono salvati come autore delle nuove pubblicazioni e mostrati nei commenti.
- **Tema** può essere **Usa impostazione di sistema**, **Chiaro** o **Scuro**. La modifica è visibile subito.
- **Lingua di visualizzazione** è la lingua dei testi di JeffPub. **Usa lingua di sistema** segue il computer. Le altre lingue compaiono in questo elenco quando sono disponibili le traduzioni. Una nuova lingua viene applicata alla successiva apertura di JeffPub.
- **Invia statistiche di utilizzo anonime** e **Controlla aggiornamenti all'avvio di JeffPub** sono le stesse scelte presenti nella pagina **Informazioni** (più avanti).

**Nome utente**, **Iniziali**, **Tema** e **Lingua di visualizzazione**, così come le scelte su aggiornamenti e statistiche, valgono per tutte le app di JeffOffice: se le modifichi qui, cambiano in ciascuna di esse.

## Strumenti di correzione

Queste caselle di controllo gestiscono il controllo ortografico e gli aiuti alla digitazione. Vedi [Controllo ortografia e sillabazione](proofing).

- **Controlla ortografia durante la digitazione**
- **Ignora parole in MAIUSCOLO**
- **Ignora parole che contengono numeri**
- **Sostituisci testo durante la digitazione (Correzione automatica)** corregge gli errori di battitura più comuni.
- **Sostituisci virgolette dritte con virgolette curve** trasforma " e ' in virgolette curve.
- **Formatta automaticamente durante la digitazione (trattini, frazioni, ordinali, elenchi automatici)**
- **Trasforma gli indirizzi web in collegamenti ipertestuali durante la digitazione** rende collegamento un indirizzo web o di posta elettronica quando premi la barra spaziatrice dopo di esso. Annulla elimina solo il collegamento.

## Salvataggio

- **Salva informazioni di recupero automatico ogni** imposta ogni quanto JeffPub crea copie di riserva del lavoro non salvato, da 1 a 120 minuti. Vedi [Recupera lavoro non salvato](recover).
- **Crea sempre una copia di backup** conserva il file precedente con il nome «Backup di» seguito dal nome del file ogni volta che salvi.

## Avanzate

- **Unità di misura** possono essere **Pollici**, **Centimetri**, **Millimetri**, **Punti** o **Picas**. Determinano i righelli e tutte le caselle di misura. Finché non scegli un'unità, JeffPub usa i pollici se la regione del computer utilizza i pollici, come negli Stati Uniti, e i centimetri altrove. Un punto equivale a 1/72 di pollice e una pica a 12 punti.
- **I tasti di direzione spostano leggermente gli oggetti di** stabilisce di quanto un tasto freccia sposta l'oggetto selezionato.
- **Mostra questo numero di pubblicazioni recenti** indica quanti nomi elenca la pagina **Apri**, da 0 a 50.
- **Durante la selezione, seleziona automaticamente l'intera parola** seleziona le parole intere mentre trascini il mouse sul testo.
- **Consenti trascinamento della selezione del testo** ti permette di spostare il testo selezionato trascinandolo.
- **Sillabazione automatica nelle nuove caselle di testo** attiva la sillabazione, che divide le parole lunghe a fine riga, nelle caselle di testo che aggiungi.

## Aggiornamenti, statistiche e nome

Fai clic su **File > Informazioni** per trovare queste opzioni. Le due caselle di controllo sono presenti anche nella scheda **Generale** di **File > Opzioni**, insieme a una terza, **Ricevi le versioni beta**.

- **Verifica aggiornamenti** cerca subito una versione più recente. Se ne esiste una, la finestra **Aggiornamento disponibile** mostra le novità. Scegli **Aggiorna ora**, **Più tardi** o **Ignora questa versione**.
- **Ricevi le versioni beta (build preliminari per i test)** fa sì che JeffPub proponga anche le versioni beta, che escono prima di quelle regolari e possono presentare qualche imperfezione.
- **Verifica automaticamente la disponibilità di aggiornamenti** fa sì che JeffPub li controlli a ogni avvio.
- **Invia statistiche di utilizzo anonime** è una tua scelta: decidi se condividere dati sull'uso di JeffPub. Quando l'opzione è attiva, JeffPub invia questi dati una volta al giorno e nel giorno in cui aggiorni: la sua versione, il sistema operativo e la lingua, e la frequenza di utilizzo di ogni comando. Se JeffPub si chiude in modo imprevisto, invia anche una segnalazione di arresto al successivo avvio. La segnalazione indica in quale punto del programma si è interrotto, così il problema può essere individuato e risolto. Non invia mai i tuoi file, i loro nomi o il loro contenuto, né quanto digitato. Fai clic su **Cosa viene inviato** per saperne di più. Al primo avvio JeffPub ti chiede una conferma nella finestra **Aiutaci a migliorare JeffPub**. Su Windows, la configurazione potrebbe chiederlo prima. La scelta vale per tutte le app di JeffOffice: JeffDeck invia lo stesso tipo di dati su sé stesso, solo se hai scelto di condividerli.
- **Informativa sulla privacy** accanto a questa casella apre la nostra informativa all'indirizzo jeffoffice.com/privacy. Spiega quali dati vengono inviati, dove vanno e per quanto tempo li conserviamo. Conserviamo le statistiche per 24 mesi.
- **Numero di installazione** è mostrato sotto la casella di controllo. È un numero casuale generato per la tua copia di JeffOffice, uguale in tutte le app di JeffOffice. Non dice nulla su di te. Fai clic su **Copia** per copiarlo. Se le statistiche sono disattivate, o se non è ancora stato generato alcun numero, viene mostrato **nessuno**.
- Per chiedere la cancellazione delle statistiche che già conserviamo, invia il tuo numero di installazione a info@jeffoffice.com **prima** di disattivare le statistiche. Disattivandole, il numero viene cancellato dal computer e, senza di esso, non possiamo stabilire quali registrazioni sono tue.
- Per disattivare definitivamente le statistiche senza aprire JeffPub, imposta la variabile d'ambiente `JP_NO_TELEMETRY` con un valore qualsiasi prima di avviarlo. In questo modo nulla viene conteggiato né inviato, e JeffPub non pone la domanda del primo avvio.
- **Il tuo nome** corrisponde a **Nome utente** indicato sopra.

## Vedi anche

- [Recupera lavoro non salvato](recover)
- [Controllo ortografia e sillabazione](proofing)
- [Ottieni assistenza e segnala problemi](support)
