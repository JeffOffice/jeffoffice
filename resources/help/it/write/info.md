# Info: proprietà, password e Ispeziona

<!-- keywords: verifica accessibilità, informazioni, proprietà, titolo, autore, oggetto, parole chiave, categorie, commenti, società, responsabile, proprietà personalizzate, docproperty, contrassegna come finale, finale, sola lettura, password, cifra, proteggi, password di apertura, ispeziona documento, informazioni personali, testo nascosto, XML personalizzato, verifica compatibilità, converti, modalità di compatibilità, cronologia versioni, versioni, conserva una versione, torna indietro, ripristina, confronta versioni, mostra nella cartella, copia percorso, cartella, percorso -->

Fai clic su **File > Informazioni** per vedere i dati del documento e proteggerlo prima di condividerlo. Premi **Esc** per tornare al documento.

## Dove si trova il file

In alto, la pagina mostra dove è salvato il file. Fai clic su **Mostra nella cartella** per aprire quella cartella. Fai clic su **Copia percorso** per copiare il percorso e incollarlo in un'e-mail o in una chat.

## Proprietà

Le proprietà sono dati sul documento, come il titolo e l'autore. La pagina contiene una casella per ciascuna: **Titolo**, **Autore**, **Oggetto**, **Parole chiave**, **Categorie**, **Commenti**, **Società** e **Responsabile**. Digita nella casella e premi **Tab** o **Invio**. JeffWrite conserva la modifica e la salva la volta successiva che salvi il documento.

Sotto le caselle, la pagina mostra i dati calcolati da JeffWrite: quando il documento è stato creato, salvato e stampato, e quante pagine, parole, caratteri e paragrafi contiene.

### Proprietà personalizzate

Fai clic su **Proprietà personalizzate** per creare proprietà tue, come **Progetto** o **Cliente**. Fai clic su **Aggiungi**, digita un nome, scegli il tipo di valore (testo, numero intero, numero, sì o no, oppure data) e digita il valore.

Un campo può mostrare una proprietà personalizzata. Fai clic su **Inserisci > Campo** e scegli un campo. Cambia il codice in `DOCPROPERTY "Client"` e fai clic su **OK**. Il testo mostra il valore. Dopo aver modificato la proprietà, premi **F9** per aggiornarlo. Vedi [Campi](fields).

## Contrassegna come finale

Fai clic su **Contrassegna come finale** per indicare ai lettori che il documento è completo. JeffWrite salva il documento e poi lo mantiene solo in lettura. Quando qualcuno lo apre, la barra del titolo indica che è di sola lettura, e una nota avvisa che è stato contrassegnato come finale. È un modo garbato per dire "per favore, non modificare questo". Non è un blocco, perché chiunque può scegliere di nuovo **Contrassegnato come finale** per modificarlo.

## Apri sempre in sola lettura

Fai clic su **Apri sempre in sola lettura** per aprire il documento solo in lettura ogni volta, così nessuno lo modifica per errore. La scelta viene salvata con il file quando lo salvi. La volta successiva che qualcuno apre il file, la barra di stato indica che è aperto in sola lettura. Per modificarlo, apri **File > Informazioni** e fai clic su **Si apre sempre in sola lettura**. Basta un clic, quindi è un promemoria e non un blocco. Gli altri programmi non leggono questa scelta, e un file .odt non può contenerla: salva prima il documento come file .docx. Verifica compatibilità la segnala per un file .odt.

## Cifra con password

Fai clic su **Cifra con password** per chiedere una password ogni volta che il file viene aperto. Digita due volte la password e fai clic su **Proteggi**. Il file viene cifrato quando lo salvi, così nessuno può leggerlo senza la password. JeffWrite usa la protezione robusta che usano anche altri elaboratori di testi, quindi anche questi possono aprire il file con la password.

- Una password **non può essere recuperata** se la dimentichi. Conservala in un luogo sicuro.
- Le versioni conservate e la copia di backup seguono la password: sono protette con essa e cambiano quando la cambi o la rimuovi.
- Per cambiare la password, fai di nuovo clic sulla stessa scheda. Fai clic su **Rimuovi password** per eliminarla. La modifica viene salvata alla prossima volta che salvi.
- Le password funzionano per i file **.docx** e per i file **.odt**. Un file .odt è protetto secondo lo standard OpenDocument, quindi anche gli altri programmi che seguono lo standard possono aprirlo con la password. Un file di testo semplice o .rtf non può contenere una password, quindi JeffWrite non salva un documento protetto in quei formati. Te lo segnala invece di salvarlo senza password.

Per aprire un file protetto, aprilo come al solito. JeffWrite chiede la password. Se non è corretta, puoi riprovare o fare clic su **Annulla**.

JeffWrite apre anche un vecchio file **.doc** che richiede una password, e la chiede allo stesso modo. JeffWrite non salva mai file .doc, quindi **Salva con nome** chiede un nuovo nome, e la copia .docx mantiene la stessa password.

## Ispeziona documento

Fai clic su **Ispeziona documento** prima di inviare un documento a qualcuno. Elenca ciò che il file contiene e che potresti non voler condividere, con il numero di elementi trovati per ciascun tipo:

- **Commenti** sono note lasciate dalle persone a margine.
- **Revisioni** sono modifiche non ancora accettate o rifiutate. **Accetta tutto** le rende parte del testo.
- **Proprietà del documento e informazioni personali** sono l'autore, la persona che ha salvato per ultima, la società e il responsabile.
- **Testo nascosto** è testo impostato come nascosto. Non viene mostrato né stampato, ma è presente nel file.
- **Intestazioni e piè di pagina** sono il testo in alto e in basso nelle pagine.
- **Dati XML personalizzati** sono dati aggiunti al file da altri programmi. Non sono visibili in pagina.
- **Collegamenti esterni** sono collegamenti a altri file, non collegamenti web: un'immagine o un oggetto soltanto collegato (mostra un file che non è nel documento), e un campo che prende testo o immagini da un altro file (INCLUDEPICTURE, INCLUDETEXT, LINK, DDE e DDEAUTO). **Rimuovi tutto** trasforma ogni campo nel testo che mostra in questo momento, elimina ogni immagine o oggetto collegato e non salva il collegamento nel file. Gli altri file non vengono toccati. Anche i collegamenti in intestazioni, piè di pagina, note a piè di pagina e note di chiusura vengono contati e rimossi. I collegamenti web restano. Vengono cercati anche i collegamenti dentro commenti, caselle di testo e le parti che il programma conserva così come sono (come il testo dei disegni). Un'immagine collegata che contiene anche una propria copia dell'immagine resta, come immagine senza collegamento, e il collegamento non viene salvato nel file. Rimuovere i collegamenti dentro commenti e nelle altre parti conservate così come sono non può essere annullato; le caselle di testo si annullano con un solo passo di Annulla. Il modello a cui è collegato il documento (il suo percorso spesso contiene un nome utente) conta come collegamento esterno: **Rimuovi tutto** lo scollega, e il documento mantiene i propri stili. Gli altri collegamenti a file non vengono eliminati, e una riga sotto il conteggio li indica: l'origine dati della stampa unione, i riquadri, i sottodocumenti dentro altre parti, i dati esterni di un grafico, i filmati e i suoni collegati e i fogli di stile collegati. La stessa riga conta anche altri collegamenti in un campo, in un commento, in una casella di testo o nell'elenco dei collegamenti del file che nessun elemento del documento usa, e lo indica quando l'indirizzo di un'immagine collegata non è nel file o quando una parte è in una codifica che il controllo non riesce a leggere. In quel caso la riga dice "Non controllato completamente" invece di "Nessuno trovato". Un'unica operazione **Rimuovi tutto** elimina al massimo 100.000 collegamenti nelle parti conservate; se ne restano altri, la riga indica quanti sono, e puoi eseguire di nuovo **Rimuovi tutto**.
- **Immagini originali conservate sotto gli effetti immagine** sono le immagini da cui è stato ottenuto un effetto come Sfocatura o Mosaico. Il file conserva l'originale accanto all'immagine finita, e chiunque riceva il file può estrarlo. **Rimuovi originali** conserva solo l'immagine finita, e **Reimposta immagine** non può più ripristinare l'originale.
- **Parti ritagliate delle immagini** sono le parti che hai ritagliato e nascosto. Sono ancora nell'immagine, e chiunque riceva il file può vederle. **Rimuovi parti ritagliate** le elimina, e la pagina appare esattamente come prima. Un'immagine con effetti diventa prima la sua immagine finita.

Fai clic su **Rimuovi tutto** accanto a un tipo per eliminarlo. Puoi annullare le modifiche al testo con **Ctrl+Z**. I collegamenti eliminati da commenti e altre parti conservate, e i dati XML personalizzati, non possono essere annullati. Salva il documento in seguito per conservare la copia pulita.

**Rimuovi le informazioni personali da questo file al salvataggio**, sotto le schede, lo fa ogni volta che salvi, per qualsiasi file, senza modificare il documento aperto. È la stessa casella che si trova in **Opzioni**, e la stessa in ogni app di JeffOffice.

## Converti

Un file .docx creato con le regole di un programma più vecchio mostra **[Modalità di compatibilità]** dopo il nome nella barra del titolo. Significa che il documento è impaginato con le regole di quel programma. Un documento nuovo, un file che non lo indica, e un file .odt, .rtf, .doc o di testo non mostrano nulla. Apri **File > Informazioni** e fai clic su **Converti** per usare invece le regole più recenti. Il programma chiede prima conferma, e l'operazione non può essere annullata con Ctrl+Z. Le pagine appaiono subito come appariranno quando salverai il file e lo riaprirai. Converti modifica questi aspetti:

- **Tabelle.** Nel metodo precedente, il rientro di una tabella è misurato fino al testo della sua prima cella, quindi il bordo sinistro della tabella si trova un po' più a sinistra. Nel metodo più recente, il rientro è misurato fino al bordo della tabella, quindi le tabelle con un rientro si spostano a destra della larghezza del margine della cella.
- **Interruzioni di pagina.** Nel metodo precedente, un paragrafo che termina con un'interruzione di pagina fa iniziare la nuova pagina al paragrafo successivo. Nel metodo più recente, la fine di quel paragrafo passa alla nuova pagina e lascia una riga vuota in cima.
- **Altezze di riga.** Un file creato con il metodo più antico può richiedere righe di punti interi. Dopo Converti, le righe hanno la loro altezza naturale, quindi righe e pagine possono interrompersi in punti diversi.

La volta successiva che salvi il file, risulta nella modalità più recente e la barra del titolo non mostra più il contrassegno. Le scelte che il file imposta una per una, come la sillabazione e le pagine affiancate, restano invariate. Un documento già nella modalità più recente non ha il pulsante **Converti**.

## Verifica compatibilità

Fai clic su **Verifica compatibilità**, poi scegli un tipo di file per vedere che cosa non può contenere. JeffWrite salva i file **.docx**, **.odt**, **.rtf** e **.txt**. Un file .docx contiene tutto il documento. Gli altri tipi non possono contenere alcune cose, e questo elenco le indica. Segnala anche le scelte memorizzate nel file stesso, Contrassegna come finale e Apri sempre in sola lettura, quando sono attive, e la password per un tipo che non può contenerla. Se non compare nulla, non si perde niente.

Quando salvi come file .odt, JeffWrite elenca ciò che il file non può contenere **prima** di scrivere qualsiasi cosa. Puoi scegliere **Salva comunque**, **Salva come .docx** (il file .docx viene salvato accanto, e non si perde nulla) oppure **Annulla**. Se selezioni **Non chiedere più per questo file**, JeffWrite smette di chiedere per quel file fino alla chiusura del programma.

## Verifica accessibilità

Fai clic su **Verifica accessibilità** per aprire l'elenco degli elementi che possono risultare difficili da leggere o usare per alcune persone, come un'immagine senza descrizione. È lo stesso controllo di **Revisione > Verifica accessibilità**. Vedi [Conta parole, imposta la lingua, verifica l'accessibilità e cerca parole](proofing-tools).

## Cronologia versioni

Fai clic su **Cronologia versioni** per conservare copie con nome del documento sul tuo computer. Non viene inviato nulla da nessuna parte. Una versione è una copia completa del documento così com'è in quel momento. Conservane una prima di una modifica importante, così potrai tornare indietro. Il documento deve essere salvato prima, perché le versioni vengono conservate con il nome del file.

- **Conserva una versione** chiede un nome, ad esempio "Prima della revisione", e conserva una copia.
- **Apri una copia** apre la versione scelta in una nuova finestra. La versione stessa non cambia.
- **Confronta** affianca il documento e la versione scelta come un unico documento, con le differenze mostrate come revisioni. Vedi [Traccia le modifiche](track-changes).
- **Torna a questa versione** rende il documento uguale alla versione scelta. JeffWrite conserva prima il documento com'era, come nuova versione, quindi non si perde nulla. Salva il documento per mantenere la modifica.
- **Elimina** rimuove la versione scelta.

JeffWrite conserva fino a 50 versioni di un file ed elimina le più vecchie quando ce ne sono di più.
