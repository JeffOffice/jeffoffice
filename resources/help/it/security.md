# Sicurezza: a cosa può accedere un file

<!-- keywords: centro protezione, sicurezza, macro, file collegati, collegamenti, immagini collegate, contenuti multimediali collegati, internet, scaricato, sola lettura, protetto, collegamenti esterni, programmi, sicuro -->

Un file può far riferimento ad altre risorse: file, pagine web, filmati, perfino programmi. La pagina **Sicurezza** serve a decidere come JeffPub si comporta in questi casi. Fai clic su **File > Opzioni**, quindi sulla scheda **Sicurezza**. Fai clic su **OK** per confermare le modifiche oppure su **Annulla** per scartarle.

Queste scelte valgono per tutte le app di JeffOffice: se ne modifichi una in un'app, cambia in tutte. Una riga relativa a una funzione che un'app non possiede riporta **Non applicabile** ed è disattivata.

## Macro

Le macro sono piccoli programmi contenuti in un file. JeffPub le conserva nel file, quindi restano presenti quando salvi, e **non le esegue mai**. Non esiste alcuna impostazione che le esegua.

## File collegati e collegamenti a cartelle di lavoro

Riguarda le immagini **collegate** a un file invece di essere conservate all'interno della pubblicazione. La scelta determina cosa fa JeffPub quando apre una pubblicazione che ne contiene.

- **Aggiorna automaticamente** è la scelta abituale. All'apertura di una pubblicazione, JeffPub rilegge il file di ogni immagine collegata se il file si trova nella stessa cartella della pubblicazione, così l'immagine è aggiornata. Non chiede mai conferma.
- **Non aggiornare mai** lascia ogni immagine com'era al momento del salvataggio. Ciascuna mostra **Non aggiornata** finché non usi **Aggiorna collegamento** su di essa. **Esporta in PDF** e **Stampa** indicano poi nella barra di stato che per alcune immagini sono state usate anteprime, perché nella pubblicazione non è presente una copia completa.

JeffPub legge solo un file che si trova nella cartella della pubblicazione (o in una cartella che hai indicato come sicura). Ogni altro file collegato attende finché non lo richiedi.

## Collegamenti a filmati e suoni

Questa voce non si applica in JeffPub: il programma non ha filmati o suoni collegati da riprodurre. La riga è disattivata.

## File da Internet o da altri computer

Un file scaricato, oppure presente su un altro computer della rete, potrebbe non essere sicuro. Questa scelta stabilisce come JeffPub lo apre.

- **Apri prima in sola lettura** è la scelta abituale. Il file si apre per la lettura, ma non può essere modificato. Una barra sotto il file lo segnala. Fai clic su **Abilita modifica** nella barra per modificare solo quel file (fino alla chiusura del programma). Quando lo salvi poi sullo stesso percorso, JeffPub rimuove il contrassegno di download dal file. Oppure usa **Salva con nome** per creare una copia, o passa all'opzione successiva di questa impostazione.
- **Apri normalmente** apre tutti i file nello stesso modo.

Un file aperto con **Apri come copia**, **Apri e ripara** o **Apri in sola lettura** dalla schermata **File** si apre in quel modo, indipendentemente da questa impostazione.

## Collegamenti che avviano un programma

Un collegamento che avvierebbe un programma o uno script (un file con estensione .exe, .bat, .sh e simili) viene **sempre rifiutato**. JeffPub te lo segnala e nulla viene eseguito. Un collegamento a un file su un altro computer non viene mai seguito senza che tu lo confermi prima. Nessuna impostazione modifica questo comportamento.

## Vedi anche

- [Modificare le impostazioni di JeffPub](options)
- [Immagini](pictures)
