# Info: proprietà, password e Ispeziona

<!-- keywords: verifica accessibilità, informazioni, proprietà, titolo, autore, oggetto, parole chiave, categorie, commenti, società, responsabile, proprietà personalizzate, docproperty, contrassegna come finale, finale, sola lettura, password, cifra, proteggi, password di apertura, ispeziona documento, informazioni personali, testo nascosto, XML personalizzato, verifica compatibilità, cronologia versioni, versioni, mantieni una versione, torna indietro, ripristina, confronta versioni, mostra nella cartella, copia percorso, cartella, percorso -->

Fai clic su **File > Informazioni** per vedere i dati del documento e proteggerlo prima di condividerlo. Premi **Esc** per tornare al documento.

## Dove si trova il file

In alto, la pagina mostra dove è salvato il file. Fai clic su **Mostra nella cartella** per aprire quella cartella. Fai clic su **Copia percorso** per copiare il percorso, così puoi incollarlo in un messaggio di posta elettronica o in una chat.

## Proprietà

Le proprietà sono dati sul documento, come il titolo e chi lo ha scritto. La pagina ha una casella per ciascuna: **Titolo**, **Autore**, **Oggetto**, **Parole chiave**, **Categorie**, **Commenti**, **Società** e **Responsabile**. Scrivi nella casella e premi **Tab** o **Invio**. JeffWrite conserva la modifica e la salva la prossima volta che salvi il documento.

Sotto le caselle, la pagina mostra ciò che JeffWrite calcola: quando il documento è stato creato, salvato e stampato, e quante pagine, parole, caratteri e paragrafi contiene.

### Proprietà personalizzate

Fai clic su **Proprietà personalizzate** per creare proprietà tue, come **Progetto** o **Cliente**. Fai clic su **Aggiungi**, digita un nome, scegli il tipo di valore (testo, numero intero, numero, sì o no, oppure data) e digita il valore.

Un campo può mostrare una proprietà personalizzata. Fai clic su **Inserisci > Campo** e scegli un campo. Cambia il codice in `DOCPROPERTY "Cliente"` e fai clic su **OK**. Il testo mostra il valore. Dopo aver modificato la proprietà, premi **F9** per aggiornarlo. Vedi [Campi](fields).

## Contrassegna come finale

Fai clic su **Contrassegna come finale** per indicare ai lettori che il documento è completato. JeffWrite salva il documento e poi lo mantiene solo in lettura. Quando qualcuno lo apre, la barra del titolo indica che è di sola lettura, e una nota segnala che è stato contrassegnato come finale. È un modo delicato per dire "per favore, non modificare". Non è un blocco, perché chiunque può scegliere di nuovo **Contrassegnato come finale** per modificarlo.

## Apri sempre in sola lettura

Fai clic su **Apri sempre in sola lettura** perché il documento si apra ogni volta solo per la lettura, così nessuno lo modifica per errore. La scelta viene salvata con il file quando lo salvi. La volta successiva che qualcuno apre il file, la barra di stato indica che è aperto solo in lettura. Per modificarlo, apri **File > Informazioni** e fai clic su **Si apre sempre in sola lettura**. Basta un clic, quindi si tratta di un promemoria e non di un blocco. Gli altri programmi non leggono questa scelta, e un file .odt non può contenerla, quindi salva prima il documento come file .docx. Verifica compatibilità la segnala per un file .odt.

## Cifra con password

Fai clic su **Cifra con password** per chiedere una password ogni volta che il file viene aperto. Digita due volte la password e fai clic su **Proteggi**. Il file viene reso illeggibile al salvataggio, quindi nessuno può leggerlo senza la password. JeffWrite usa lo stesso tipo di protezione forte usato dagli altri elaboratori di testi, quindi anche questi possono aprire il file con la password.

- Una password **non può essere recuperata** se la dimentichi. Conservala in un luogo sicuro.
- Le versioni conservate e la copia di backup seguono la password: sono protette con essa e cambiano quando la modifichi o la rimuovi.
- Per cambiare la password, fai di nuovo clic sulla stessa scheda. Fai clic su **Rimuovi password** per eliminarla. La modifica viene salvata la prossima volta che salvi.
- Le password funzionano per i file **.docx** e per i file **.odt**. Un file .odt è protetto nel modo descritto dallo standard OpenDocument, quindi anche gli altri programmi che seguono lo standard possono aprirlo con la password. Un file di testo semplice o .rtf non può contenere una password, quindi JeffWrite non salva un documento protetto in quei formati. Te lo segnala invece di salvarlo senza password.

Per aprire un file protetto, aprilo come al solito. JeffWrite chiede la password. Se non è quella corretta, puoi riprovare oppure fare clic su **Annulla**.

JeffWrite apre anche un vecchio file **.doc** che richiede una password, e la chiede allo stesso modo. JeffWrite non salva mai file .doc, quindi **Salva con nome** chiede un nuovo nome, e la copia .docx conserva la stessa password.

## Ispeziona documento

Fai clic su **Ispeziona documento** prima di inviare un documento a qualcuno. Elenca ciò che il file contiene e che potresti non voler condividere, con il numero di elementi trovati per ciascun tipo:

- **Commenti** sono note lasciate dalle persone a margine.
- **Modifiche registrate** sono modifiche non ancora accettate o rifiutate. **Accetta tutto** mantiene ogni modifica come parte del testo.
- **Proprietà del documento e informazioni personali** sono l'autore, la persona che ha salvato per ultima il file, la società e il responsabile.
- **Testo nascosto** è testo impostato come nascosto. Non viene mostrato né stampato, ma è presente nel file.
- **Intestazioni e piè di pagina** sono il testo in alto e in basso nelle pagine.
- **Dati XML personalizzati** sono dati aggiunti al file da altri programmi. Non sono visibili in pagina.
- **Collegamenti esterni** sono collegamenti ad altri file, non collegamenti web: un'immagine o un oggetto solo collegato (mostra un file che non si trova nel documento) e un campo che prende il testo o l'immagine da un altro file (INCLUDEPICTURE, INCLUDETEXT, LINK, DDE e DDEAUTO). **Rimuovi tutto** trasforma ogni campo nel testo che mostra in quel momento, elimina ogni immagine o oggetto collegato e non salva il collegamento nel file. Gli altri file non vengono modificati. I collegamenti in intestazioni, piè di pagina e note a piè di pagina non vengono conteggiati, e i collegamenti web restano.

Fai clic su **Rimuovi tutto** accanto a un tipo per eliminarlo. Puoi annullare le modifiche al testo con **CTRL+Z**. Salva il documento dopo per conservare la copia ripulita.

**Rimuovi le informazioni personali da questo file al salvataggio**, sotto le schede, lo fa ogni volta che salvi, per qualsiasi file, senza modificare il documento aperto. È la stessa casella presente in **Opzioni**, ed è uguale in tutte le app di JeffOffice.

## Verifica compatibilità

Fai clic su **Verifica compatibilità** per vedere cosa non può contenere un tipo di file più vecchio. JeffWrite salva file **.docx** e **.odt**. Un file .docx contiene tutto il documento. Un file .odt non può contenere alcune cose, e questo elenco le indica. Se non compare nulla, non si perde nulla.

Quando salvi come file .odt, JeffWrite elenca ciò che il file non può contenere **prima** di scrivere qualsiasi cosa. Puoi scegliere **Salva comunque**, **Salva come .docx** (il file .docx viene creato accanto a quello .odt e non si perde nulla) oppure **Annulla**. Seleziona **Non chiedere più per questo file** e JeffWrite smette di chiedere per quel file fino alla chiusura del programma.

## Verifica accessibilità

Fai clic su **Verifica accessibilità** per aprire l'elenco degli elementi che possono essere difficili da leggere o da usare per alcune persone, come un'immagine senza descrizione. È la stessa verifica di **Revisione > Verifica accessibilità**. Vedi [Conta parole, imposta la lingua, verifica l'accessibilità e cerca parole](proofing-tools).

## Cronologia versioni

Fai clic su **Cronologia versioni** per conservare sul computer copie con nome del documento. Non viene inviato nulla altrove. Una versione è una copia completa del documento così com'è in quel momento. Conservane una prima di una modifica importante, così puoi tornare indietro. Il documento deve essere salvato prima, perché le versioni vengono conservate con il nome del file.

- **Mantieni una versione** chiede un nome, ad esempio "Prima della revisione", e conserva una copia.
- **Apri una copia** apre la versione scelta in una nuova finestra. La versione non cambia.
- **Confronta** affianca il documento e la versione scelta come un unico documento, con le differenze mostrate come revisioni. Vedi [Tieni traccia delle modifiche](track-changes).
- **Torna a questa versione** rende il documento uguale alla versione scelta. JeffWrite conserva prima il documento com'era, come nuova versione, così non si perde nulla. Salva il documento per mantenere la modifica.
- **Elimina** rimuove la versione scelta.

JeffWrite conserva fino a 50 versioni di un file e elimina la più vecchia quando ce ne sono di più.
