# Info: proprietà, password e Verifica documento
<!-- keywords: verifica accessibilità, informazioni, proprietà, titolo, autore, oggetto, parole chiave, categorie, commenti, società, responsabile, proprietà personalizzate, docproperty, contrassegna come finale, finale, sola lettura, password, cifra, proteggi, password di apertura, verifica documento, informazioni personali, testo nascosto, xml personalizzato, verifica compatibilità, cronologia versioni, versioni, mantieni una versione, torna indietro, ripristina, confronta versioni, mostra nella cartella, copia percorso, cartella, percorso -->

Fai clic su **File > Informazioni** per vedere i dati del documento e proteggerlo prima di condividerlo. Premi **Esc** per tornare al documento.

## Dove si trova il file

In alto, la pagina mostra dove è salvato il file. Fai clic su **Mostra nella cartella** per aprire quella cartella. Fai clic su **Copia percorso** per copiare il percorso, così puoi incollarlo in un messaggio di posta elettronica o in una chat.

## Proprietà

Le proprietà sono informazioni sul documento, come il titolo e l'autore. La pagina ha una casella per ciascuna: **Titolo**, **Autore**, **Oggetto**, **Parole chiave**, **Categorie**, **Commenti**, **Società** e **Responsabile**. Digita nella casella e premi **Tab** o **Invio**. JeffWrite mantiene la modifica e la salva la prossima volta che salvi il documento.

Sotto le caselle, la pagina mostra ciò che JeffWrite calcola: quando il documento è stato creato, salvato e stampato, e quante pagine, parole, caratteri e paragrafi contiene.

### Proprietà personalizzate

Fai clic su **Proprietà personalizzate** per creare proprietà tue, come **Progetto** o **Cliente**. Fai clic su **Aggiungi**, digita un nome, scegli il tipo di valore (testo, numero intero, numero, sì o no, oppure data) e digita il valore.

Un campo può mostrare una proprietà personalizzata. Fai clic su **Inserisci > Campo** e scegli un campo. Cambia il suo codice in `DOCPROPERTY "Cliente"` e fai clic su **OK**. Il testo mostra il valore. Premi **F9** per aggiornarlo dopo aver modificato la proprietà. Vedi [Campi](fields).

## Contrassegna come finale

Fai clic su **Contrassegna come finale** per indicare ai lettori che il documento è completato. JeffWrite salva il documento e poi lo apre solo in lettura. Quando qualcuno lo apre, la barra del titolo indica che è in sola lettura, e una nota avvisa che è stato contrassegnato come finale. È un modo delicato per dire "per favore, non modificare questo". Non è un blocco, perché chiunque può scegliere di nuovo **Contrassegnato come finale** per modificarlo.

## Cifra con password

Fai clic su **Cifra con password** per chiedere una password ogni volta che il file viene aperto. Digita due volte la password e fai clic su **Proteggi**. Il file viene reso illeggibile al salvataggio, quindi nessuno può leggerlo senza la password. JeffWrite usa il tipo di protezione robusto che usano anche gli altri elaboratori di testi, quindi anche loro possono aprire il file con la password.

- Una password **non può essere recuperata** se la dimentichi. Conservala in un luogo sicuro.
- Le versioni salvate e la copia di backup seguono la password: sono protette con essa e cambiano quando la modifichi o la rimuovi.
- Per cambiare la password, fai di nuovo clic sulla stessa scheda. Fai clic su **Rimuovi password** per eliminarla. La modifica viene salvata al prossimo salvataggio.
- Le password funzionano per i file **.docx** e per i file **.odt**. Un file .odt è protetto nel modo descritto dallo standard OpenDocument, quindi anche gli altri programmi che seguono lo standard possono aprirlo con la password. Un file di testo semplice o .rtf non può contenere una password, quindi JeffWrite non salva un documento protetto in quei formati. Te lo segnala invece di salvarlo senza password.

Per aprire un file protetto, aprilo come al solito. JeffWrite chiede la password. Se non è corretta, puoi riprovare oppure fare clic su **Annulla**.

JeffWrite apre anche un vecchio file **.doc** che richiede una password, e la chiede nello stesso modo. JeffWrite non salva mai file .doc, quindi **Salva con nome** chiede un nuovo nome, e la copia .docx mantiene la stessa password.

## Verifica documento

Fai clic su **Verifica documento** prima di inviare un documento a qualcuno. Elenca ciò che il file contiene e che potresti non voler condividere, con il numero di elementi trovati per ciascun tipo:

- **Commenti** sono note lasciate dalle persone a margine.
- **Revisioni** sono modifiche non ancora accettate o rifiutate. **Accetta tutto** le mantiene come parte del testo.
- **Proprietà del documento e informazioni personali** sono l'autore, la persona che l'ha salvato per ultima, la società e il responsabile.
- **Testo nascosto** è testo impostato come nascosto. Non viene mostrato né stampato, ma è presente nel file.
- **Intestazioni e piè di pagina** sono il testo in alto e in basso nelle pagine.
- **Dati XML personalizzati** sono dati aggiunti al file da altri programmi. Non sono visibili in pagina.

Fai clic su **Rimuovi tutto** accanto a un tipo per eliminarlo. Puoi annullare le modifiche al testo con **Ctrl+Z**. Salva il documento dopo, per conservare la copia pulita.

**Rimuovi le informazioni personali da questo file al salvataggio**, sotto le schede, lo fa ogni volta che salvi, per qualsiasi file, senza modificare il documento aperto. È la stessa casella presente in **Opzioni**, e uguale in tutte le app di JeffOffice.

## Verifica compatibilità

Fai clic su **Verifica compatibilità** per vedere che cosa non può contenere un tipo di file meno recente. JeffWrite salva file **.docx** e **.odt**. Un file .docx contiene tutto il documento. Un file .odt non può contenere alcune cose, e questo elenco le indica. Se non compare nulla, non si perde nulla.

Quando salvi come file .odt, JeffWrite elenca ciò che il file non può contenere **prima** di scrivere qualsiasi cosa. Puoi scegliere **Salva comunque**, **Salva come .docx** (il file .docx viene creato accanto, e non si perde nulla) oppure **Annulla**. Seleziona **Non chiedere più per questo file** e JeffWrite smette di chiederlo per quel file fino alla chiusura del programma.

## Verifica accessibilità

Fai clic su **Verifica accessibilità** per aprire l'elenco degli elementi che possono essere difficili da leggere o usare per alcune persone, come un'immagine senza descrizione. È la stessa verifica di **Revisione > Verifica accessibilità**. Vedi [Conteggio parole, impostazione della lingua, verifica dell'accessibilità e ricerca di parole](proofing-tools).

## Cronologia versioni

Fai clic su **Cronologia versioni** per conservare copie con nome del documento sul tuo computer. Non viene inviato nulla altrove. Una versione è una copia completa del documento com'è in quel momento. Conservane una prima di una modifica importante, così puoi tornare indietro. Il documento deve essere salvato prima, perché le versioni vengono conservate con il nome del file.

- **Mantieni una versione** chiede un nome, ad esempio "Prima della modifica", e conserva una copia.
- **Apri una copia** apre la versione scelta in una nuova finestra. La versione stessa non cambia.
- **Confronta** affianca il documento e la versione scelta come un unico documento, con le differenze mostrate come revisioni. Vedi [Traccia le modifiche](track-changes).
- **Torna a questa versione** rende il documento uguale alla versione scelta. JeffWrite conserva prima il documento com'era, come nuova versione, così nulla va perso. Salva il documento per mantenere la modifica.
- **Elimina** rimuove la versione scelta.

JeffWrite conserva fino a 50 versioni di un file ed elimina le più vecchie quando sono di più.
