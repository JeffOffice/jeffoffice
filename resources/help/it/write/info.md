# Info: proprietà, password e Ispeziona

<!-- keywords: verifica accessibilità, informazioni, proprietà, titolo, autore, oggetto, parole chiave, categorie, commenti, società, responsabile, proprietà personalizzate, docproperty, contrassegna come finale, finale, sola lettura, password, cifra, proteggi, password di apertura, ispeziona documento, informazioni personali, testo nascosto, xml personalizzato, verifica compatibilità, cronologia versioni, versioni, conserva una versione, torna indietro, ripristina, confronta versioni, mostra nella cartella, copia percorso, cartella, percorso, info, file -->

Fai clic su **File > Informazioni** per vedere i dati del documento e proteggerlo prima di condividerlo. Premi **Esc** per tornare al documento.

## Dove si trova il file

In alto, la pagina mostra dove è salvato il file. Fai clic su **Mostra nella cartella** per aprire quella cartella. Fai clic su **Copia percorso** per copiare il percorso, così puoi incollarlo in un messaggio di posta elettronica o in una chat.

## Proprietà

Le proprietà sono dati sul documento, come il titolo e chi lo ha scritto. La pagina contiene una casella per ciascuna: **Titolo**, **Autore**, **Oggetto**, **Parole chiave**, **Categorie**, **Commenti**, **Società** e **Responsabile**. Digita nella casella e premi **Tab** o **Invio**. JeffWrite conserva la modifica e la salva alla prossima operazione di salvataggio del documento.

Sotto le caselle, la pagina mostra i dati calcolati da JeffWrite: quando il documento è stato creato, salvato e stampato, e quante pagine, parole, caratteri e paragrafi contiene.

### Proprietà personalizzate

Fai clic su **Proprietà personalizzate** per creare proprietà tue, come **Progetto** o **Cliente**. Fai clic su **Aggiungi**, digita un nome, scegli il tipo di valore (testo, numero intero, numero, sì o no, oppure data) e digita il valore.

Un campo può mostrare una proprietà personalizzata. Fai clic su **Inserisci > Campo** e scegli un campo. Modifica il codice in `DOCPROPERTY "Cliente"` e fai clic su **OK**. Il testo mostra il valore. Dopo aver modificato la proprietà, premi **F9** per aggiornarlo. Vedi [Campi](fields).

## Contrassegna come finale

Fai clic su **Contrassegna come finale** per indicare ai lettori che il documento è completo. JeffWrite salva il documento e lo rende disponibile solo in lettura. Quando qualcuno lo apre, la barra del titolo indica che è di sola lettura e una nota avvisa che è stato contrassegnato come finale. È un modo discreto per dire «per favore, non modificare questo». Non è un blocco, perché chiunque può scegliere di nuovo **Contrassegnato come finale** per modificarlo.

## Cifra con password

Fai clic su **Cifra con password** per chiedere una password ogni volta che il file viene aperto. Digita la password due volte e fai clic su **Proteggi**. Il file viene cifrato al salvataggio, quindi nessuno può leggerlo senza la password. JeffWrite usa il tipo di protezione robusto usato anche dagli altri elaboratori di testi, così anche loro possono aprire il file con la password.

- Una password **non può essere recuperata** se la dimentichi. Conservala in un luogo sicuro.
- Le versioni conservate e la copia di backup seguono la password: sono protette con essa e cambiano quando la modifichi o la rimuovi.
- Per cambiare la password, fai di nuovo clic sulla stessa scheda. Fai clic su **Rimuovi password** per eliminarla. La modifica viene salvata alla prossima operazione di salvataggio.
- Le password funzionano per i file **.docx**. Un file **.odt** non può ancora contenere una password, quindi JeffWrite non salva un documento protetto come .odt: te lo segnala invece di salvarlo senza password.

Per aprire un file protetto, aprilo normalmente. JeffWrite chiede la password. Se non è corretta, puoi riprovare oppure fare clic su **Annulla**.

## Ispeziona documento

Fai clic su **Ispeziona documento** prima di inviare un documento a qualcuno. L'elenco mostra cosa contiene il file che potresti non voler condividere, con il numero di elementi trovati per ciascun tipo:

- **Commenti** sono note lasciate dalle persone a margine del testo.
- **Modifiche registrate** sono modifiche non ancora accettate o rifiutate. **Accetta tutto** le mantiene come parte del testo.
- **Proprietà del documento e informazioni personali** sono l'autore, la persona che ha salvato per ultima il file, la società e il responsabile.
- **Testo nascosto** è testo impostato come nascosto. Non viene mostrato né stampato, ma è presente nel file.
- **Intestazioni e piè di pagina** sono il testo in alto e in basso nelle pagine.
- **Dati XML personalizzati** sono dati aggiunti al file da altri programmi. Non sono visibili nella pagina.

Fai clic su **Rimuovi tutto** accanto a un tipo per eliminarlo. Per annullare le modifiche al testo usa **Ctrl+Z**. Salva il documento dopo per conservare la copia pulita.

## Verifica compatibilità

Fai clic su **Verifica compatibilità** per vedere cosa un tipo di file meno recente non può contenere. JeffWrite salva i file **.docx** e **.odt**. Un file .docx contiene tutto il documento. Un file .odt non può contenere alcune cose, e questo elenco le indica. Se non compare nulla, non si perde niente.

Quando salvi come file .odt, JeffWrite elenca ciò che il file non può contenere **prima** di scrivere qualsiasi cosa. Puoi scegliere **Salva comunque come .odt**, **Salva invece come .docx** (il file .docx viene salvato accanto, e non si perde nulla) oppure **Annulla**.

## Verifica accessibilità

Fai clic su **Verifica accessibilità** per aprire l'elenco degli elementi che possono essere difficili da leggere o usare per alcune persone, come un'immagine senza descrizione. È la stessa verifica di **Revisione > Verifica accessibilità**. Vedi [Conteggio parole, impostazione della lingua, verifica dell'accessibilità e ricerca di parole](proofing-tools).

## Cronologia versioni

Fai clic su **Cronologia versioni** per conservare sul computer copie con nome del documento. Nulla viene inviato altrove. Una versione è una copia completa del documento com'è in quel momento. Conservane una prima di una modifica importante, così potrai tornarci. Il documento deve essere salvato prima, perché le versioni vengono conservate con il nome del file.

- **Conserva una versione** chiede un nome, ad esempio "Prima della modifica", e conserva una copia.
- **Apri una copia** apre la versione scelta in una nuova finestra. La versione stessa non cambia.
- **Confronta** affianca il documento e la versione scelta come un unico documento, con le differenze mostrate come modifiche registrate. Vedi [Traccia modifiche](track-changes).
- **Torna a questa versione** rende il documento uguale alla versione scelta. JeffWrite conserva prima il documento com'era, come nuova versione, così non si perde nulla. Salva il documento per mantenere la modifica.
- **Elimina** rimuove la versione scelta.

JeffWrite conserva fino a 50 versioni di un file ed elimina le più vecchie quando ce ne sono di più.
