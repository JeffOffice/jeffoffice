# Sicurezza: a cosa può accedere un file
<!-- keywords: centro protezione, sicurezza, macro, file collegati, collegamenti, immagini collegate, contenuti multimediali collegati, internet, scaricati, sola lettura, protetto, collegamenti esterni, programmi, sicuro -->

Un file può rimandare ad altre cose: file, pagine web, filmati, perfino programmi. La pagina **Sicurezza** serve a decidere come JeffPub si comporta in questi casi. Fai clic su **File > Opzioni**, quindi sulla scheda **Sicurezza**. Fai clic su **OK** per mantenere le modifiche, oppure su **Annulla** per scartarle.

Queste scelte valgono per tutte le app di JeffOffice: se ne cambi una in un'app, cambia in tutte. Una riga relativa a una funzione che l'app non possiede riporta **Non applicabile** ed è disattivata.

## Macro

Le macro sono piccoli programmi contenuti in un file. JeffPub **non le esegue mai**, e non esiste alcuna impostazione che le esegua. JeffPub non conserva nemmeno le macro di una pubblicazione: quando salvi un file `.pub` aperto, la copia non ne contiene. Se ti servono ancora, conserva l'originale.

## File collegati e collegamenti a cartelle di lavoro

Riguarda le immagini **collegate** a un file invece di essere conservate all'interno della pubblicazione. La scelta riguarda ciò che JeffPub fa quando apre una pubblicazione che ne contiene.

- **Aggiornali** è la scelta usuale. Quando una pubblicazione si apre, JeffPub rilegge il file di ogni immagine collegata se il file si trova nella cartella della pubblicazione, così l'immagine è aggiornata. Non chiede mai conferma.
- **Non aggiornarli mai** lascia ogni immagine com'era al salvataggio. Ciascuna mostra **Non aggiornata** finché non usi **Aggiorna collegamento** su di essa. **Esporta in PDF** e **Stampa** segnalano allora nella barra di stato che sono state usate anteprime per immagini che non hanno una copia completa nella pubblicazione.

JeffPub legge solo un file che si trova nella cartella della pubblicazione (o in una cartella che hai indicato come sicura). Ogni altro file collegato attende finché non lo richiedi.

## Filmati e suoni collegati

Questa voce non si applica in JeffPub: non ha filmati o suoni collegati da riprodurre. La riga è disattivata.

## File da internet o da altri computer

Un file scaricato, oppure che si trova su un altro computer della rete, potrebbe non essere sicuro. Questa scelta stabilisce come JeffPub lo apre.

- **Apri prima in sola lettura** è la scelta usuale. Il file si apre per consentirti di leggerlo, ma non puoi modificarlo. Una barra sotto il file lo indica. Fai clic su **Abilita modifica** nella barra per modificare solo quel file (fino alla chiusura del programma). Quando lo salvi sul posto, JeffPub rimuove il contrassegno di download dal file. Oppure usa **Salva con nome** per crearne una copia, oppure passa all'opzione successiva in questa impostazione.
- **Apri normalmente** apre ogni file allo stesso modo.

Un file aperto con **Apri come copia**, **Apri e ripara** o **Apri in sola lettura** dalla schermata **File** si apre in quel modo, indipendentemente da questa impostazione.

## Collegamenti che avviano un programma

Un collegamento che avvierebbe un programma o uno script (un file con estensione .exe, .bat, .sh e simili) viene **sempre rifiutato**. JeffPub te lo segnala e non esegue nulla. Un collegamento a un file su un altro computer non viene mai seguito senza prima chiedertelo. Nessuna impostazione cambia questo comportamento.

## Vedi anche

- [Modificare le impostazioni di JeffPub](options)
- [Immagini](pictures)
