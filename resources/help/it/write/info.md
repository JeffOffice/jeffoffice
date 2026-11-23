# Info: proprietà, password e Ispeziona

<!-- keywords: verifica accessibilità, info, proprietà, titolo, autore, oggetto, parole chiave, categorie, commenti, società, responsabile, proprietà personalizzate, docproperty, contrassegna come finale, finale, sola lettura, password, cifra, proteggi, password di apertura, ispeziona documento, informazioni personali, testo nascosto, XML personalizzato, verifica compatibilità, cronologia versioni, versioni, conserva una versione, torna indietro, ripristina, confronta versioni, mostra nella cartella, copia percorso, cartella, percorso -->

Fai clic su **File > Info** per vedere le informazioni sul documento e per proteggerlo prima di condividerlo. Premi **Esc** per tornare al documento.

## Dove si trova il file

In alto, la pagina mostra dove è salvato il file. Fai clic su **Mostra nella cartella** per aprire quella cartella. Fai clic su **Copia percorso** per copiare il percorso, così puoi incollarlo in un messaggio di posta elettronica o in una chat.

## Proprietà

Le proprietà sono informazioni sul documento, come il titolo e chi l'ha scritto. La pagina ha una casella per ciascuna: **Titolo**, **Autore**, **Oggetto**, **Parole chiave**, **Categorie**, **Commenti**, **Società** e **Responsabile**. Digita nella casella e premi **Tab** o **Invio**. JeffWrite mantiene la modifica e la salva la prossima volta che salvi il documento.

Sotto le caselle, la pagina mostra ciò che JeffWrite calcola: quando il documento è stato creato, salvato e stampato, e quante pagine, parole, caratteri e paragrafi contiene.

### Proprietà personalizzate

Fai clic su **Proprietà personalizzate** per creare proprietà tue, come **Progetto** o **Cliente**. Fai clic su **Aggiungi**, digita un nome, scegli il tipo di valore (testo, numero intero, numero, sì o no, oppure data) e digita il valore.

Un campo può mostrare una proprietà personalizzata. Fai clic su **Inserisci > Campo** e scegli un campo. Cambia il codice in `DOCPROPERTY "Client"` e fai clic su **OK**. Il testo mostra il valore. Premi **F9** per aggiornarlo dopo aver modificato la proprietà. Vedi [Campi](fields).

## Contrassegna come finale

Fai clic su **Contrassegna come finale** per indicare ai lettori che il documento è completo. JeffWrite salva il documento e poi lo mantiene solo in lettura. Quando qualcuno lo apre, la barra del titolo indica che è di sola lettura, e una nota avvisa che è stato contrassegnato come finale. È un modo delicato per dire "per favore, non modificare questo". Non è un blocco, perché chiunque può scegliere di nuovo **Contrassegnato come finale** per modificarlo.

## Apri sempre in sola lettura

Fai clic su **Apri sempre in sola lettura** per fare in modo che il documento si apra ogni volta solo in lettura, così nessuno lo modifica per sbaglio. La scelta viene salvata con il file quando lo salvi. La volta successiva che qualcuno apre il file, la barra di stato indica che è aperto solo in lettura. Per modificarlo, apri **File > Info** e fai clic su **Si apre sempre in sola lettura**. È un solo clic, quindi si tratta di un promemoria e non di un blocco. Gli altri programmi non leggono questa scelta, e un file .odt non può contenerla: salva quindi prima il documento come file .docx. **Verifica compatibilità** la elenca per un file .odt.

## Cifra con password

Fai clic su **Cifra con password** per chiedere una password ogni volta che il file viene aperto. Digita la password due volte e fai clic su **Proteggi**. Il file viene reso illeggibile al salvataggio, quindi nessuno può leggerlo senza la password. JeffWrite usa il tipo di protezione robusto che usano gli altri elaboratori di testi, quindi anche questi possono aprire il file con la password.

- Una password **non può essere recuperata** se la dimentichi. Conservala in un luogo sicuro.
- Le versioni conservate e la copia di backup seguono la password: sono protette con essa e cambiano quando la modifichi o la rimuovi.
- Per cambiare la password, fai di nuovo clic sulla stessa scheda. Fai clic su **Rimuovi password** per eliminarla. La modifica viene salvata al prossimo salvataggio.
- Le password funzionano per i file **.docx** e per i file **.odt**. Un file .odt viene protetto nel modo descritto dallo standard OpenDocument, quindi anche gli altri programmi che seguono lo standard possono aprirlo con la password. Un file di testo semplice o .rtf non può contenere una password, quindi JeffWrite non salva un documento protetto in quei formati: ti avvisa invece di salvarlo senza password.

Per aprire un file protetto, aprilo normalmente. JeffWrite chiede la password. Se non è corretta, puoi riprovare oppure fare clic su **Annulla**.

JeffWrite apre anche un vecchio file **.doc** che richiede una password, e la chiede nello stesso modo. JeffWrite non salva mai file .doc, quindi **Salva con nome** chiede un nuovo nome, e la copia .docx mantiene la stessa password.

## Ispeziona documento

Fai clic su **Ispeziona documento** prima di inviare un documento a qualcuno. Elenca ciò che il file contiene e che potresti non voler condividere, con il numero di elementi trovati per ciascun tipo:

- **Commenti** sono note lasciate dalle persone a margine.
- **Modifiche registrate** sono modifiche non ancora accettate o rifiutate. **Accetta tutto** mantiene ogni modifica come parte del testo.
- **Proprietà del documento e informazioni personali** sono l'autore, la persona che l'ha salvato per ultima, la società e il responsabile.
- **Testo nascosto** è testo impostato come nascosto. Non appare e non viene stampato, ma è presente nel file.
- **Intestazioni e piè di pagina** sono il testo in alto e in basso nelle pagine.
- **Dati XML personalizzati** sono dati aggiunti al file da altri programmi. Non sono visibili in pagina.
- **Collegamenti esterni** sono collegamenti ad altri file, non collegamenti web: un'immagine o un oggetto solo collegato (mostra un file che non è nel documento), e un campo che prende il testo o l'immagine da un altro file (INCLUDEPICTURE, INCLUDETEXT, LINK, DDE e DDEAUTO). **Rimuovi tutto** trasforma ogni campo nel testo che mostra in questo momento, elimina ogni immagine o oggetto collegato e lascia il collegamento fuori dal file al salvataggio. Gli altri file non vengono toccati. I collegamenti in intestazioni, piè di pagina, note a piè di pagina e note di chiusura vengono contati ed eliminati allo stesso modo. I collegamenti web restano. Vengono cercati anche i collegamenti dentro i commenti, le caselle di testo e le parti che il programma conserva così come sono (come il testo dei disegni). Un'immagine collegata che contiene anche una propria copia dell'immagine resta, come immagine senza collegamento, e il collegamento viene escluso dal file al salvataggio. Eliminare i collegamenti nei commenti e nelle altre parti conservate così come sono non può essere annullato; le caselle di testo sono un'azione di Annulla. Anche il modello a cui è associato il documento (il suo percorso spesso contiene un nome utente) conta come collegamento esterno: **Rimuovi tutto** lo scollega, e il documento mantiene i propri stili. Gli altri collegamenti a file non vengono eliminati, e una riga sotto il conteggio li indica: l'origine dati della stampa unione, i riquadri, i sottodocumenti all'interno di altre parti, i dati esterni di un grafico, i filmati e i suoni collegati e i fogli di stile collegati. La stessa riga conta anche qualsiasi altro collegamento in un campo, un commento, una casella di testo o l'elenco dei collegamenti del file che nessun elemento del documento usa, e segnala quando l'indirizzo di un'immagine collegata non è nel file o quando una parte è in una codifica che il controllo non riesce a leggere. In tal caso la riga indica "Non controllato completamente" invece di "Nessuno trovato". Un'operazione **Rimuovi tutto** elimina al massimo 100.000 collegamenti nelle parti conservate; se ne restano altri, la riga indica quanti sono, e puoi eseguire di nuovo **Rimuovi tutto**.

Fai clic su **Rimuovi tutto** accanto a un tipo per eliminarlo. Puoi annullare le modifiche al testo con **CTRL+Z**. I collegamenti eliminati da commenti e altre parti conservate, e i dati XML personalizzati, non possono essere annullati. Salva il documento dopo l'operazione per conservare la copia pulita.

**Rimuovi le informazioni personali da questo file al salvataggio**, sotto le schede, lo fa ogni volta che salvi, per qualsiasi file, senza modificare il documento aperto. È la stessa casella che si trova in **Opzioni**, ed è uguale in tutte le app di JeffOffice.

## Verifica compatibilità

Fai clic su **Verifica compatibilità**, poi scegli un tipo di file per vedere ciò che non può contenere. JeffWrite salva i file **.docx**, **.odt**, **.rtf** e **.txt**. Un file .docx contiene tutto il documento. Gli altri tipi non possono contenere alcune cose, e questo elenco indica quali. Segnala anche le scelte mantenute nel file stesso, Contrassegna come finale e Apri sempre in sola lettura, quando sono attive, e la password per un tipo che non può contenerla. Se non compare nulla nell'elenco, non si perde niente.

Quando salvi come file .odt, JeffWrite elenca ciò che il file non può contenere **prima** di scrivere qualsiasi cosa. Puoi scegliere **Salva comunque**, **Salva come .docx** (il file .docx viene messo accanto a quello, e non si perde nulla) oppure **Annulla**. Seleziona **Non chiedere più per questo file** e JeffWrite smette di chiedere per quel file fino alla chiusura del programma.

## Verifica accessibilità

Fai clic su **Verifica accessibilità** per aprire l'elenco degli elementi che possono risultare difficili da leggere o usare per alcune persone, come un'immagine senza descrizione. È la stessa verifica di **Revisione > Verifica accessibilità**. Vedi [Conta parole, imposta la lingua, verifica l'accessibilità e cerca parole](proofing-tools).

## Cronologia versioni

Fai clic su **Cronologia versioni** per conservare sul computer copie con nome del documento. Nulla viene inviato altrove. Una versione è una copia completa del documento così com'è in quel momento. Conservane una prima di una modifica importante, così puoi tornare indietro. Il documento deve essere salvato prima, perché le versioni vengono conservate con il nome del file.

- **Conserva una versione** chiede un nome, come "Prima della revisione", e conserva una copia.
- **Apri una copia** apre la versione scelta in una nuova finestra. La versione stessa non cambia.
- **Confronta** affianca il documento e la versione scelta come un unico documento, con le differenze mostrate come modifiche registrate. Vedi [Tieni traccia delle modifiche](track-changes).
- **Torna a questa versione** rende il documento uguale alla versione scelta. JeffWrite conserva prima il documento com'era come nuova versione, così non si perde nulla. Salva il documento per conservare la modifica.
- **Elimina** rimuove la versione scelta.

JeffWrite conserva fino a 50 versioni di un file e elimina la più vecchia quando ce ne sono di più.
