# Sicurezza: a cosa può collegarsi un file
<!-- keywords: centro protezione, sicurezza, macro, file collegati, collegamenti, immagini collegate, elementi multimediali collegati, internet, scaricato, sola lettura, protetto, collegamenti esterni, programmi, sicuro -->

Un file può puntare ad altre risorse: altri file, pagine web, video, perfino programmi. La pagina **Sicurezza** serve a decidere come JeffDeck si comporta in questi casi. Fai clic su **File > Opzioni**, quindi sulla scheda **Sicurezza**. Fai clic su **OK** per mantenere le modifiche, oppure su **Annulla** per scartarle.

Queste scelte valgono per tutte le app di JeffOffice: se ne cambi una in un'app, cambia in tutte. Una riga relativa a una funzione che un'app non possiede riporta **Non si applica** ed è in grigio.

## Macro

Le macro sono piccoli programmi contenuti in un file. JeffDeck le conserva nel file, quindi restano presenti quando salvi, e **non le esegue mai**. Non esiste alcuna impostazione che le esegua.

## File collegati e collegamenti a cartelle di lavoro

Questa voce non si applica a JeffDeck, che non ha file collegati da aggiornare. La riga è in grigio.

## Video e audio collegati

Riguarda un video o un audio che una diapositiva **collega** invece di includerlo nella presentazione.

- **Non riprodurlo mai** è la scelta abituale. Un video o audio collegato indica di essere collegato e non viene riprodotto.
- **Chiedi prima** chiede una volta, la prima volta che lo riproduci, per ogni file collegato.
- **Riproducilo** lo riproduce senza chiedere.

Anche con **Riproducilo** viene riprodotto solo un file video o audio presente su questo computer. Un indirizzo web, un file su un altro computer, un programma, un collegamento o un file speciale, e un file che non è davvero un video o un audio, non vengono mai riprodotti. Un file collegato può avere una dimensione fino a 256 MB. Il file viene letto in background per un massimo di 20 secondi, così un disco lento non può bloccare JeffDeck. Un'unità di rete che sembra locale (un'unità mappata oppure una cartella in /Volumes o /mnt) viene letta come un file locale, quindi può essere lenta. Con **Chiedi prima**, una presentazione chiede una sola volta, all'avvio, per tutti i file collegati che contiene.

## File da Internet o da altri computer

Un file che hai scaricato, oppure che si trova su un altro computer della tua rete, potrebbe non essere sicuro. Questa scelta stabilisce come JeffDeck lo apre.

- **Apri prima in sola lettura** è la scelta abituale. Il file si apre per la lettura, ma non puoi modificarlo. Una barra sotto il file lo indica. Fai clic su **Abilita modifica** nella barra per modificare solo quel file (fino alla chiusura del programma). Quando lo salvi sul posto, il contrassegno di download viene rimosso dal file. Oppure usa **Salva con nome** per creare una copia, o cambia questa impostazione con la scelta successiva.
- **Apri normalmente** apre tutti i file nello stesso modo.

Un file aperto con **Apri come copia**, **Apri e ripara** oppure **Apri in sola lettura** dalla schermata **File** si apre in quel modo, indipendentemente da questa impostazione.

## Collegamenti che avviano un programma

Un collegamento che avvierebbe un programma o uno script (un file con estensione .exe, .bat, .sh e simili) viene **sempre rifiutato**. JeffDeck ti avvisa e non esegue nulla. Un collegamento a un file su un altro computer non viene mai seguito senza prima chiederti conferma. Nessuna impostazione cambia questo comportamento.

## Vedi anche

- [Modificare le impostazioni di JeffDeck](options)
- [Avviare la presentazione](show)
