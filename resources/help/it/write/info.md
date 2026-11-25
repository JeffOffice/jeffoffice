# Info: proprietà, password e Ispeziona

<!-- keywords: verifica accessibilità, info, informazioni, proprietà, titolo, autore, oggetto, parole chiave, categorie, commenti, società, responsabile, proprietà personalizzate, docproperty, contrassegna come finale, finale, sola lettura, password, cifra, proteggi, password di apertura, ispeziona documento, informazioni personali, testo nascosto, xml personalizzato, verifica compatibilità, cronologia versioni, versioni, conserva una versione, torna indietro, ripristina, confronta versioni, mostra nella cartella, copia percorso, cartella, percorso -->

Fai clic su **File > Info** per vedere le informazioni sul documento e per proteggerlo prima di condividerlo. Premi **Esc** per tornare al documento.

## Dove si trova il file

In alto, la pagina mostra dove è salvato il file. Fai clic su **Mostra nella cartella** per aprire quella cartella. Fai clic su **Copia percorso** per copiare il percorso e incollarlo in un messaggio di posta elettronica o in una chat.

## Proprietà

Le proprietà sono informazioni sul documento, come il titolo e l'autore. La pagina contiene una casella per ciascuna: **Titolo**, **Autore**, **Oggetto**, **Parole chiave**, **Categorie**, **Commenti**, **Società** e **Responsabile**. Digita nella casella e premi **Tab** o **Invio**. JeffWrite conserva la modifica e la salva la volta successiva che salvi il documento.

Sotto le caselle, la pagina mostra i dati calcolati da JeffWrite: quando il documento è stato creato, salvato e stampato, e quante pagine, parole, caratteri e paragrafi contiene.

### Proprietà personalizzate

Fai clic su **Proprietà personalizzate** per creare proprietà tue, come **Progetto** o **Cliente**. Fai clic su **Aggiungi**, digita un nome, scegli il tipo di valore (testo, numero intero, numero, sì o no, oppure data) e digita il valore.

Un campo può mostrare una proprietà personalizzata. Fai clic su **Inserisci > Campo** e scegli un campo. Cambia il codice in `DOCPROPERTY "Cliente"` e fai clic su **OK**. Il testo mostra il valore. Dopo aver cambiato la proprietà, premi **F9** per aggiornarlo. Vedi [Campi](fields).

## Contrassegna come finale

Fai clic su **Contrassegna come finale** per indicare ai lettori che il documento è completo. JeffWrite salva il documento e poi lo apre solo in lettura. Quando qualcuno lo apre, la barra del titolo indica che è di sola lettura, e una nota spiega che è stato contrassegnato come finale. È un modo delicato per dire "per favore, non modificare questo". Non è un blocco, perché chiunque può scegliere di nuovo **Contrassegnato come finale** per modificarlo.

## Apri sempre in sola lettura

Fai clic su **Apri sempre in sola lettura** perché il documento si apra ogni volta solo in lettura, così nessuno lo modifica per sbaglio. La scelta viene salvata con il file quando lo salvi. La volta successiva che qualcuno apre il file, la barra di stato indica che è aperto in sola lettura. Per modificarlo, apri **File > Info** e fai clic su **Si apre sempre in sola lettura**. Basta un clic, quindi si tratta di un promemoria e non di un blocco. Gli altri programmi non leggono questa scelta, e un file .odt non può contenerla: salva prima il documento come file .docx. Verifica compatibilità la segnala per un file .odt.

## Cifra con password

Fai clic su **Cifra con password** per chiedere una password ogni volta che il file viene aperto. Digita due volte la password e fai clic su **Proteggi**. Il file viene reso illeggibile al momento del salvataggio, quindi nessuno può leggerlo senza la password. JeffWrite usa il tipo di protezione robusto che usano anche gli altri elaboratori di testi, quindi anche loro possono aprire il file con la password.

- Una password **non può essere recuperata** se la dimentichi. Conservala in un posto sicuro.
- Le versioni conservate e la copia di backup seguono la password: sono protette con essa e cambiano quando cambi o rimuovi la password.
- Per cambiare la password, fai di nuovo clic sulla stessa scheda. Fai clic su **Rimuovi password** per eliminarla. La modifica viene salvata la volta successiva che salvi.
- Le password funzionano per i file **.docx** e per i file **.odt**. Un file .odt viene protetto nel modo descritto dallo standard OpenDocument, quindi anche gli altri programmi che seguono lo standard possono aprirlo con la password. Un file di testo normale o .rtf non può contenere una password, quindi JeffWrite non salva un documento protetto in questi formati: ti avvisa invece di salvarlo senza password.

Per aprire un file protetto, aprilo come al solito. JeffWrite chiede la password. Se non è corretta, puoi riprovare oppure fare clic su **Annulla**.

JeffWrite apre anche un vecchio file **.doc** che richiede una password, e la chiede nello stesso modo. JeffWrite non salva mai un file .doc, quindi **Salva con nome** chiede un nuovo nome, e la copia .docx mantiene la stessa password.

## Ispeziona documento

Fai clic su **Ispeziona documento** prima di inviare un documento a qualcuno. Elenca ciò che il file contiene e che potresti non voler condividere, con il numero di elementi trovati per ciascun tipo:

- **Commenti** sono note lasciate dalle persone a margine.
- **Modifiche registrate** sono modifiche non ancora accettate o rifiutate. **Accetta tutto** le mantiene come parte del testo.
- **Proprietà del documento e informazioni personali** sono l'autore, la persona che ha salvato per ultima il documento, la società e il responsabile.
- **Testo nascosto** è testo impostato come nascosto. Non si vede né si stampa, ma è presente nel file.
- **Intestazioni e piè di pagina** sono il testo in alto e in basso nelle pagine.
- **Dati XML personalizzati** sono dati aggiunti al file da altri programmi. Non sono visibili in pagina.
- **Collegamenti esterni** sono collegamenti ad altri file, non collegamenti web: un'immagine o un oggetto solo collegato (mostra un file che non è nel documento), e un campo che prende il testo o l'immagine da un altro file (INCLUDEPICTURE, INCLUDETEXT, LINK, DDE e DDEAUTO). **Rimuovi tutto** trasforma ogni campo nel testo che mostra ora, elimina ogni immagine o oggetto collegato e lascia il collegamento fuori dal file al salvataggio. Gli altri file non vengono toccati. Anche i collegamenti in intestazioni, piè di pagina, note a piè di pagina e note di chiusura vengono contati e rimossi. I collegamenti web restano. Vengono cercati anche i collegamenti dentro commenti, caselle di testo e le parti che il programma conserva così come sono (ad esempio il testo dei disegni). Un'immagine collegata che contiene anche una propria copia dell'immagine resta, come immagine senza collegamento, e il collegamento viene escluso dal file al salvataggio. La rimozione dei collegamenti all'interno di commenti e delle altre parti conservate così come sono non può essere annullata; le caselle di testo sono un passaggio di Annulla. Il modello a cui il documento è associato (il suo percorso spesso contiene un nome utente) viene conteggiato come collegamento esterno: **Rimuovi tutto** lo scollega, e il documento mantiene i propri stili. Gli altri collegamenti a file non vengono eliminati, e una riga sotto il conteggio li elenca: l'origine dati della stampa unione, i riquadri, i sottodocumenti all'interno di altre parti, i dati esterni di un grafico, i filmati e i suoni collegati e i fogli di stile collegati. La stessa riga conta anche gli altri collegamenti presenti in un campo, in un commento, in una casella di testo o nell'elenco dei collegamenti del file che nessun elemento del documento usa, e segnala quando l'indirizzo di un'immagine collegata non è nel file o quando una parte è in una codifica che il controllo non riesce a leggere. In quel caso la riga riporta "Non controllato completamente" invece di "Nessuno trovato". Un solo **Rimuovi tutto** elimina al massimo 100.000 collegamenti nelle parti conservate; se ne restano, la riga indica quanti sono, e puoi eseguire di nuovo **Rimuovi tutto**.
- **Immagini originali conservate sotto gli effetti immagine** sono le immagini da cui è stato ricavato un effetto, come Sfocatura o Mosaico. Il file conserva l'originale accanto all'immagine finale, e chiunque riceva il file può estrarlo. **Rimuovi originali** conserva solo l'immagine finale, e **Reimposta immagine** non potrà più recuperare l'originale.
- **Parti ritagliate delle immagini** sono le parti che hai ritagliato e che non si vedono più. Sono ancora nell'immagine, e chiunque riceva il file può vederle. **Rimuovi parti ritagliate** le elimina, e la pagina appare esattamente come prima. Un'immagine con un effetto diventa prima la sua immagine finale.

Fai clic su **Rimuovi tutto** accanto a un tipo per eliminarlo. Puoi annullare con **Ctrl+Z** le modifiche al testo. I collegamenti eliminati da commenti e altre parti conservate, e i dati XML personalizzati, non possono essere annullati. Salva il documento dopo, per conservare la copia pulita.

**Rimuovi le informazioni personali da questo file al salvataggio**, sotto le schede, lo fa ogni volta che salvi, per qualsiasi file, senza modificare il documento aperto. È la stessa casella che si trova in **Opzioni** ed è uguale in tutte le app di JeffOffice.

## Verifica compatibilità

Fai clic su **Verifica compatibilità**, poi scegli un tipo di file per vedere che cosa non può contenere. JeffWrite salva file **.docx**, **.odt**, **.rtf** e **.txt**. Un file .docx contiene tutto il documento. Gli altri tipi non possono contenere alcune cose, e questo elenco le indica. Indica anche le scelte conservate nel file stesso, Contrassegna come finale e Apri sempre in sola lettura, quando sono attive, e la password per un tipo che non può contenerla. Se non c'è nulla nell'elenco, non si perde nulla.

Quando salvi come file .odt, JeffWrite elenca ciò che il file non può contenere **prima** di scrivere qualsiasi cosa. Puoi scegliere **Salva comunque**, **Salva come .docx** (il file .docx viene salvato accanto, e non si perde nulla) oppure **Annulla**. Seleziona **Non chiedere più per questo file** e JeffWrite smette di chiederlo per quel file fino a quando chiudi il programma.

## Verifica accessibilità

Fai clic su **Verifica accessibilità** per aprire l'elenco di elementi che possono essere difficili da leggere o usare per alcune persone, come un'immagine senza descrizione. È la stessa verifica di **Revisione > Verifica accessibilità**. Vedi [Conteggio parole, impostazione della lingua, verifica dell'accessibilità e ricerca di parole](proofing-tools).

## Cronologia versioni

Fai clic su **Cronologia versioni** per conservare sul computer copie con nome del documento. Non viene inviato nulla altrove. Una versione è una copia completa del documento così com'è in quel momento. Conservane una prima di una modifica importante, così puoi tornare indietro. Il documento deve essere salvato prima, perché le versioni vengono conservate con il nome del file.

- **Conserva una versione** chiede un nome, ad esempio "Prima della revisione", e conserva una copia.
- **Apri una copia** apre la versione scelta in una nuova finestra. La versione stessa non cambia.
- **Confronta** affianca il documento e la versione scelta come un unico documento, con le differenze mostrate come modifiche registrate. Vedi [Traccia modifiche](track-changes).
- **Torna a questa versione** rende il documento uguale alla versione scelta. JeffWrite conserva prima il documento com'era, come nuova versione, così non si perde nulla. Salva il documento per mantenere la modifica.
- **Elimina** rimuove la versione scelta.

JeffWrite conserva fino a 50 versioni di un file e elimina la più vecchia quando ce ne sono di più.
