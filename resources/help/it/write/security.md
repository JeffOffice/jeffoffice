# Sicurezza: a cosa può accedere un file

<!-- keywords: centro protezione, sicurezza, macro, file collegati, collegamenti, immagini collegate, contenuti multimediali collegati, internet, scaricato, sola lettura, protetto, collegamenti esterni, programmi, sicuro -->

Un file può far riferimento ad altre cose: file, pagine web, video, perfino programmi. La pagina **Sicurezza** serve a decidere come JeffWrite si comporta in questi casi. Fai clic su **File > Opzioni**, quindi sulla scheda **Sicurezza**. Fai clic su **OK** per mantenere le modifiche, oppure su **Annulla** per scartarle.

Queste scelte valgono per tutte le app di JeffOffice: se ne cambi una in un'app, la modifica vale in tutte. Una riga relativa a una funzione che l'app non possiede riporta **Non si applica** ed è in grigio.

## Macro

Le macro sono piccoli programmi contenuti in un file. JeffWrite le conserva nel file, quindi restano lì quando salvi, e **non le esegue mai**. Non esiste alcuna impostazione che le esegue.

## File collegati e collegamenti a cartelle di lavoro

Questa voce non si applica in JeffWrite: il programma non ha file collegati da aggiornare. La riga è in grigio.

## Filmati e suoni collegati

Questa voce non si applica in JeffWrite: il programma non ha filmati o suoni collegati da riprodurre. La riga è in grigio.

## File da internet o da altri computer

Un file scaricato, oppure presente su un altro computer della tua rete, potrebbe non essere sicuro. Questa scelta stabilisce come JeffWrite lo apre.

- **Apri prima in sola lettura** è la scelta abituale. Il file si apre per essere letto, ma non può essere modificato. Una barra sotto il file lo segnala. Fai clic su **Abilita modifica** nella barra per modificare solo quel file (fino alla chiusura del programma). Quando lo salvi sul posto, il contrassegno di download viene rimosso dal file. Oppure usa **Salva con nome** per crearne una copia, o cambia questa impostazione con la scelta successiva.
- **Apri normalmente** apre tutti i file allo stesso modo.

Un file aperto con **Apri come copia**, **Apri e ripara** oppure **Apri in sola lettura** dalla schermata **File** si apre in quel modo, qualunque sia l'impostazione.

## Collegamenti che avviano un programma

Un collegamento che avvierebbe un programma o uno script (un file con estensione .exe, .bat, .sh e simili) viene **sempre rifiutato**. JeffWrite te lo segnala e nulla viene eseguito. Un collegamento a un file presente su un altro computer non viene mai seguito senza prima chiederti conferma. Nessuna impostazione cambia questo comportamento.

## Vedi anche

- [Modificare le impostazioni di JeffWrite](options)
- [Collegamenti](links)
