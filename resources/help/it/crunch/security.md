# Sicurezza: a cosa può accedere un file
<!-- keywords: centro protezione, sicurezza, macro, file collegati, collegamenti, immagini collegate, contenuti multimediali collegati, internet, scaricati, sola lettura, protetti, collegamenti esterni, programmi, sicuro -->

Un file può puntare ad altre risorse: altri file, pagine web, video, perfino programmi. La pagina **Sicurezza** serve a decidere come JeffCrunch si comporta in questi casi. Fai clic su **File > Opzioni**, quindi sulla scheda **Sicurezza**. Fai clic su **OK** per confermare le modifiche oppure su **Annulla** per scartarle.

Queste scelte valgono per tutte le app di JeffOffice: se ne cambi una in un'app, cambia in tutte. Una riga relativa a qualcosa che un'app non prevede riporta **Questo non si applica** ed è in grigio.

## Macro

Le macro sono piccoli programmi contenuti in un file. JeffCrunch le conserva nel file, quindi restano presenti quando salvi, e **non le esegue mai**. Non esiste alcuna impostazione che le esegua.

## File collegati e collegamenti a cartelle di lavoro

Riguarda le altre cartelle di lavoro da cui una cartella di lavoro ricava i numeri, chiamate **collegamenti**. La scelta determina cosa fa JeffCrunch quando apre una cartella di lavoro che ne contiene.

- **Chiedi prima** è la scelta abituale. JeffCrunch chiede se aggiornare i numeri dalle altre cartelle di lavoro.
- **Aggiorna** li aggiorna senza chiedere.
- **Non aggiornare mai** mantiene i numeri memorizzati nel file.

Una scelta fatta per una cartella di lavoro in **Dati > Modifica collegamenti** (**Richiesta all'avvio**) prevale per quella cartella di lavoro. Una cartella di lavoro impostata su "non aggiornare mai" viene sempre lasciata invariata. Una cartella di lavoro che si trova su un altro computer non viene mai letta all'apertura di un file, qualunque sia la scelta. Puoi comunque usare **Aggiorna valori** in **Modifica collegamenti** in qualsiasi momento.

## Video e suoni collegati

Questo non si applica in JeffCrunch: il programma non ha video o suoni collegati da riprodurre. La riga è in grigio.

## File da internet o da altri computer

Un file scaricato, oppure presente su un altro computer della rete, potrebbe non essere sicuro. Questa scelta stabilisce come JeffCrunch lo apre.

- **Apri in sola lettura prima** è la scelta abituale. Il file si apre per essere letto, ma non può essere modificato. Una barra sotto il file lo segnala. Fai clic su **Abilita modifica** nella barra per modificare solo quel file (fino alla chiusura del programma). Quando lo salvi sul posto, il contrassegno di download viene rimosso dal file. In alternativa usa **Salva con nome** per creare una copia, oppure cambia questa impostazione con la scelta successiva.
- **Apri normalmente** apre tutti i file nello stesso modo.

Un file aperto con **Apri come copia**, **Apri e ripara** oppure **Apri in sola lettura** dalla schermata **File** si apre in quel modo, qualunque sia l'impostazione.

## Collegamenti che avviano un programma

Un collegamento che avvierebbe un programma o uno script (un file con estensione .exe, .bat, .sh e simili) viene **sempre rifiutato**. JeffCrunch te lo segnala e non esegue nulla. Un collegamento a un file su un altro computer non viene mai seguito senza chiederti prima conferma. Nessuna impostazione cambia questo comportamento.

## Vedi anche

- [Modificare le impostazioni di JeffCrunch](options)
- [Note e collegamenti](notes-links)
