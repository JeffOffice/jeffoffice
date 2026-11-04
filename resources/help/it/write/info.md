# Info: proprietà, password e Ispeziona
<!-- keywords: info, proprietà, titolo, autore, oggetto, parole chiave, categorie, commenti, società, responsabile, proprietà personalizzate, docproperty, contrassegna come finale, finale, sola lettura, password, cifra, proteggi, password di apertura, ispeziona documento, informazioni personali, testo nascosto, xml personalizzato, verifica compatibilità, mostra nella cartella, copia percorso, cartella, percorso -->

Fai clic su **File > Info** per vedere le informazioni sul documento e proteggerlo prima di condividerlo. Premi **Esc** per tornare al documento.

## Dove si trova il file

In alto, la pagina mostra dove è salvato il file. Fai clic su **Mostra nella cartella** per aprire quella cartella. Fai clic su **Copia percorso** per copiare il percorso e incollarlo in un messaggio di posta elettronica o in una chat.

## Proprietà

Le proprietà sono informazioni sul documento, come il titolo e chi lo ha scritto. La pagina contiene una casella per ciascuna: **Titolo**, **Autore**, **Oggetto**, **Parole chiave**, **Categorie**, **Commenti**, **Società** e **Responsabile**. Digita nella casella e premi **Tab** o **Invio**. JeffWrite conserva la modifica e la salva la volta successiva che salvi il documento.

Sotto le caselle, la pagina mostra i dati calcolati da JeffWrite: la data di creazione, di salvataggio e di stampa del documento, e il numero di pagine, parole, caratteri e paragrafi.

### Proprietà personalizzate

Fai clic su **Proprietà personalizzate** per creare proprietà tue, ad esempio **Progetto** o **Cliente**. Fai clic su **Aggiungi**, digita un nome, scegli il tipo di valore (testo, numero intero, numero, sì o no, oppure data) e digita il valore.

Un campo può mostrare una proprietà personalizzata. Fai clic su **Inserisci > Campo** e scegli un campo. Cambia il codice in `DOCPROPERTY "Cliente"` e fai clic su **OK**. Il testo mostra il valore. Premi **F9** per aggiornarlo dopo aver modificato la proprietà. Vedi [Campi](fields).

## Contrassegna come finale

Fai clic su **Contrassegna come finale** per indicare ai lettori che il documento è completato. JeffWrite salva il documento e poi lo apre solo in lettura. Quando qualcuno lo apre, la barra del titolo indica che è di sola lettura, e una nota gli comunica che è stato contrassegnato come finale. È un modo garbato per dire «per favore, non modificare». Non è un blocco, perché chiunque può scegliere di nuovo **Contrassegnato come finale** per modificarlo.

## Cifra con password

Fai clic su **Cifra con password** per chiedere una password ogni volta che il file viene aperto. Digita la password due volte e fai clic su **Proteggi**. Il file viene cifrato al salvataggio, quindi nessuno può leggerlo senza la password. JeffWrite usa lo stesso tipo di protezione robusta degli altri elaboratori di testi, per cui anche questi possono aprire il file con la password.

- Una password **non può essere recuperata** se la dimentichi. Conservala in un luogo sicuro.
- Per cambiare la password, fai clic di nuovo sulla stessa scheda. Fai clic su **Rimuovi password** per eliminarla. La modifica viene salvata al prossimo salvataggio.
- Le password funzionano per i file **.docx**. Un file **.odt** non può ancora contenere una password, quindi JeffWrite non salva un documento protetto come .odt. Te lo segnala invece di salvarlo senza password.

Per aprire un file protetto, aprilo come di consueto. JeffWrite chiede la password. Se non è corretta, puoi riprovare oppure fare clic su **Annulla**.

## Ispeziona documento

Fai clic su **Ispeziona documento** prima di inviare un documento a qualcuno. L'elenco mostra che cosa contiene il file e che potresti non voler condividere, con il numero di elementi trovati per ciascun tipo:

- **Commenti** sono note lasciate dalle persone a margine del testo.
- **Modifiche registrate** sono modifiche non ancora accettate o rifiutate. **Accetta tutto** mantiene ogni modifica come parte del testo.
- **Proprietà del documento e informazioni personali** sono l'autore, la persona che ha salvato il documento per ultima, la società e il responsabile.
- **Testo nascosto** è testo impostato come nascosto. Non viene visualizzato né stampato, ma è presente nel file.
- **Intestazioni e piè di pagina** sono il testo in alto e in basso nelle pagine.
- **Dati XML personalizzati** sono dati aggiunti al file da altri programmi. Non sono visibili in pagina.

Fai clic su **Rimuovi tutto** accanto a un tipo per eliminarlo. Puoi annullare le modifiche al testo con **Ctrl+Z**. Salva il documento al termine per conservare la copia pulita.

## Verifica compatibilità

Fai clic su **Verifica compatibilità** per vedere che cosa non può essere contenuto in un tipo di file meno recente. JeffWrite salva i file **.docx** e **.odt**. Un file .docx contiene tutto il documento. Un file .odt non può contenere alcune cose, e questo elenco le indica. Se non compare nulla, non si perde niente.
