# Info: proprietà, password e Ispeziona documento
<!-- keywords: verifica accessibilità, info, proprietà, titolo, autore, oggetto, parole chiave, categorie, commenti, azienda, responsabile, proprietà personalizzate, docproperty, contrassegna come finale, finale, sola lettura, password, cifra, proteggi, password di apertura, ispeziona documento, informazioni personali, testo nascosto, xml personalizzato, verifica compatibilità, cronologia versioni, versioni, conserva una versione, ripristina, confronta, mostra nella cartella, copia percorso, cartella, percorso -->

Fai clic su **File > Info** per vedere le informazioni sul documento e per proteggerlo prima di condividerlo. Premi **Esc** per tornare al documento.

## Dove si trova il file

In alto, la pagina mostra dove è salvato il file. Fai clic su **Mostra nella cartella** per aprire quella cartella. Fai clic su **Copia percorso** per copiare il percorso, così puoi incollarlo in un messaggio di posta elettronica o in una chat.

## Proprietà

Le proprietà sono informazioni sul documento, come il titolo e l'autore. La pagina contiene una casella per ciascuna: **Titolo**, **Autore**, **Oggetto**, **Parole chiave**, **Categorie**, **Commenti**, **Azienda** e **Responsabile**. Scrivi nella casella e premi **Tab** o **Invio**. JeffWrite registra la modifica e la salva la prossima volta che salvi il documento.

Sotto le caselle, la pagina mostra ciò che JeffWrite calcola: quando il documento è stato creato, salvato e stampato, e quante pagine, parole, caratteri e paragrafi contiene.

### Proprietà personalizzate

Fai clic su **Proprietà personalizzate** per creare proprietà tue, come **Progetto** o **Cliente**. Fai clic su **Aggiungi**, scrivi un nome, scegli il tipo di valore (testo, numero intero, numero, sì o no, oppure data) e scrivi il valore.

Un campo può mostrare una proprietà personalizzata. Fai clic su **Inserisci > Campo** e scegli un campo. Cambia il codice in `DOCPROPERTY "Client"` e fai clic su **OK**. Il testo mostra il valore. Dopo aver cambiato la proprietà, premi **F9** per aggiornarlo. Vedi [Campi](fields).

## Contrassegna come finale

Fai clic su **Contrassegna come finale** per indicare ai lettori che il documento è completo. JeffWrite salva il documento e poi lo imposta per la sola lettura. Quando qualcuno lo apre, la barra del titolo indica che è di sola lettura, e una nota avvisa che il documento è stato contrassegnato come finale. È un modo discreto per dire "per favore, non modificare questo documento". Non è un blocco, perché chiunque può scegliere di nuovo **Contrassegnato come finale** per modificarlo.

## Cifra con password

Fai clic su **Cifra con password** per chiedere una password ogni volta che il file viene aperto. Scrivi la password due volte e fai clic su **Proteggi**. Il file viene cifrato quando lo salvi, quindi nessuno può leggerlo senza la password. JeffWrite usa lo stesso tipo di protezione robusta degli altri programmi di elaborazione testi, quindi anche questi possono aprire il file con la password.

- Una password **non può essere recuperata** se la dimentichi. Conservala in un posto sicuro.
- Per cambiare la password, fai di nuovo clic sulla stessa scheda. Fai clic su **Rimuovi password** per eliminarla. La modifica viene salvata la prossima volta che salvi.
- Le password funzionano per i file **.docx**. Un file **.odt** non può ancora contenere una password, quindi JeffWrite non salva un documento protetto come .odt. Invece ti avvisa, senza salvare senza password.

Per aprire un file protetto, aprilo come al solito. JeffWrite chiede la password. Se non è corretta, puoi riprovare oppure fare clic su **Annulla**.

## Ispeziona documento

Fai clic su **Ispeziona documento** prima di inviare un documento a qualcuno. L'elenco mostra cosa contiene il file che potresti non voler condividere, e quanti elementi di ogni tipo sono stati trovati:

- **Commenti** sono note lasciate dalle persone a margine.
- **Modifiche registrate** sono modifiche non ancora accettate o rifiutate. **Accetta tutto** le mantiene come parte del testo.
- **Proprietà del documento e informazioni personali** sono l'autore, la persona che ha salvato per ultima il file, l'azienda e il responsabile.
- **Testo nascosto** è testo impostato come nascosto. Non appare e non viene stampato, ma si trova nel file.
- **Intestazioni e piè di pagina** sono il testo in alto e in basso nelle pagine.
- **Dati XML personalizzati** sono dati aggiunti al file da altri programmi. Non si vedono in pagina.

Fai clic su **Rimuovi tutto** accanto a un tipo per eliminarlo. Puoi annullare con **Ctrl+Z** le modifiche al testo. Salva il documento al termine per conservare la copia pulita.

## Verifica compatibilità

Fai clic su **Verifica compatibilità** per vedere cosa non può contenere un tipo di file meno recente. JeffWrite salva file **.docx** e **.odt**. Un file .docx contiene tutto ciò che c'è nel documento. Un file .odt non può contenere alcune cose, e questo elenco le indica. Se non c'è nulla nell'elenco, non si perde nulla.

Quando salvi come file .odt, JeffWrite elenca ciò che il file non può contenere **prima** di scrivere qualsiasi cosa. Puoi scegliere **Salva comunque come .odt**, **Salva invece come .docx** (il file .docx viene creato accanto a quello, e non si perde nulla) oppure **Annulla**.

## Verifica accessibilità

Fai clic su **Verifica accessibilità** per aprire l'elenco degli elementi che potrebbero essere difficili da leggere o da usare per alcune persone, come un'immagine senza descrizione. È la stessa verifica di **Revisione > Verifica accessibilità**. Vedi [Conteggio parole, impostazione della lingua, verifica accessibilità e ricerca delle parole](proofing-tools).

## Cronologia versioni

Fai clic su **Cronologia versioni** per conservare sul computer copie con nome del documento. Non viene inviato nulla da nessuna parte. Una versione è una copia completa del documento così com'è in quel momento. Conservane una prima di una modifica importante, così potrai tornare indietro. Il documento deve essere salvato prima, perché le versioni vengono conservate con il nome del file.

- **Conserva una versione** chiede un nome, ad esempio "Prima della modifica", e conserva una copia.
- **Apri una copia** apre la versione scelta in una nuova finestra. La versione stessa non cambia.
- **Confronta** affianca il documento e la versione scelta come un unico documento, con le differenze mostrate come modifiche registrate. Vedi [Revisioni](track-changes).
- **Torna a questa versione** rende il documento uguale alla versione scelta. JeffWrite conserva prima il documento com'era, come nuova versione, così non si perde nulla. Salva il documento per conservare la modifica.
- **Elimina** rimuove la versione scelta.

JeffWrite conserva fino a 50 versioni di un file e elimina la più vecchia quando ce ne sono di più.
