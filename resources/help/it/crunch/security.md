# Sicurezza: a cosa può accedere un file

<!-- keywords: centro protezione, sicurezza, macro, file collegati, collegamenti, immagini collegate, contenuti multimediali collegati, internet, scaricato, sola lettura, protetto, collegamenti esterni, programmi, sicuro, attendibilità, approvazioni, fidato -->

Un file può puntare ad altre cose: altri file, pagine web, filmati, perfino programmi. La pagina **Sicurezza** serve a decidere cosa fa JeffCrunch in questi casi. Fai clic su **File > Opzioni**, quindi sulla scheda **Sicurezza**. Fai clic su **OK** per conservare le modifiche oppure su **Annulla** per scartarle.

Queste scelte valgono per tutte le app di JeffOffice: se ne cambi una in un'app, cambia in tutte. Una riga relativa a una funzione che l'app non possiede riporta **Non si applica** ed è disattivata.

## Macro

Le macro sono piccoli programmi contenuti in un file. JeffCrunch le conserva nel file, quindi restano anche dopo il salvataggio, e **non le esegue mai**. Non esiste alcuna impostazione che le esegue.

## File collegati e collegamenti alle cartelle di lavoro

Riguarda gli altri file che una cartella di lavoro legge, come le cartelle di lavoro da cui prende i numeri, detti **collegamenti**, i file letti dalle sue query e i file da cui sono create le sue tabelle pivot. La scelta determina cosa fa JeffCrunch quando apri una cartella di lavoro che ne legge alcuni, e quando usi **Aggiorna** o **Aggiorna tutto** sulle sue query e sulle tabelle pivot.

- **Chiedi prima** è la scelta abituale. Prima che JeffCrunch legga uno di questi file, mostra il percorso completo del file e chiede conferma. Fai clic su **Leggi file** solo se ti fidi della provenienza della cartella di lavoro. **Non leggere**, il pulsante predefinito, non legge nulla e lascia la cartella di lavoro com'era. Questa scelta chiede ogni volta che apri una cartella di lavoro, e poi una volta per ciascun file. Dopo aver fatto clic su **Leggi file** per un file, JeffCrunch non chiede di nuovo per lo stesso file in quella finestra, finché non chiudi la finestra.
- **Aggiornali** legge un file senza chiedere solo se hai già confermato che quella stessa cartella di lavoro legga quello stesso file. Per ogni altro file, compreso un file indicato in una cartella di lavoro che ti è appena stata inviata, chiede comunque prima, nello stesso modo.
- **Non aggiornarli mai** non legge nulla all'apertura di una cartella di lavoro e non mostra alcuna domanda. I numeri memorizzati nel file restano invariati. Quando fai clic tu stesso su **Aggiorna**, si tratta di una richiesta esplicita, quindi JeffCrunch mostra la domanda in quel momento. Dopo aver fatto clic su **Leggi file**, non chiede di nuovo per lo stesso file in quella finestra.

## Cosa viene ricordato

Quando fai clic su **Leggi file** o **Leggi i file**, JeffCrunch ricorda la coppia: questa cartella di lavoro e quel file, ciascuno con il percorso completo. Viene ricordata solo la coppia. Se la cartella di lavoro viene spostata o rinominata, modificata in modo da leggere un altro file, oppure il file viene sostituito con un collegamento a un altro file, JeffCrunch chiede di nuovo. Una cartella di lavoro che non hai mai salvato non ha un percorso, quindi non viene ricordato nulla su di essa dopo la chiusura.

Le coppie sono conservate solo nelle impostazioni personali su questo computer, mai in una cartella di lavoro: una cartella di lavoro non può aggiungerne una, e chi riceve una cartella di lavoro da te non riceve le tue. Le impostazioni conservano una forma cifrata di ogni percorso, non il percorso stesso. Vengono mantenute fino a 2.000 coppie; quando ce ne sono di più, viene dimenticata per prima la coppia usata da più tempo.

**Opzioni > Sicurezza** mostra quante coppie sono ricordate (**Approvazioni ricordate**). Fai clic su **Dimentica tutto** per dimenticarle tutte in una volta. Nessuna cartella di lavoro viene modificata, e ciascuna chiederà di nuovo la volta successiva.

Una cartella di lavoro che nella propria **Richiesta all'avvio** (**Dati > Modifica collegamenti**) indica "mai" viene sempre lasciata invariata. Una cartella di lavoro di un altro computer non viene mai letta all'apertura di un file, qualunque sia la scelta. Puoi comunque usare **Aggiorna valori** in **Modifica collegamenti** in qualsiasi momento.

## Filmati e suoni collegati

Questa voce non si applica a JeffCrunch: il programma non ha filmati o suoni collegati da riprodurre. La riga è disattivata.

## File da internet o da altri computer

Un file scaricato, oppure che si trova su un altro computer della tua rete, potrebbe non essere sicuro. Questa scelta stabilisce come JeffCrunch lo apre.

- **Apri prima in sola lettura** è la scelta abituale. Il file si apre per poterlo leggere, ma non puoi modificarlo. Una barra sotto il file lo indica. Fai clic su **Abilita modifica** nella barra per modificare solo quel file (fino alla chiusura del programma). Quando lo salvi sul posto, il contrassegno di download viene rimosso dal file. Oppure usa **Salva con nome** per crearne una copia, o cambia questa impostazione con la scelta successiva.
- **Apri normalmente** apre ogni file nello stesso modo.

Un file aperto con **Apri come copia**, **Apri e ripara** oppure **Apri in sola lettura** dalla schermata **File** si apre in quel modo, indipendentemente da questa impostazione.

## Collegamenti che avviano un programma

Un collegamento che avvierebbe un programma o uno script (un file con estensione .exe, .bat, .sh e simili) viene **sempre rifiutato**. JeffCrunch te lo segnala e non esegue nulla. Un collegamento a un file su un altro computer non viene mai seguito senza prima chiederti conferma. Nessuna impostazione cambia questo comportamento.

## Vedi anche

- [Modificare le impostazioni di JeffCrunch](options)
- [Note e collegamenti](notes-links)
