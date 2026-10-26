# Proteggere un foglio o la cartella di lavoro
<!-- keywords: proteggere, protezione, password, bloccare, celle bloccate, sbloccare, rimuovere protezione, struttura, fogli, sola lettura, consentire, formato celle, inserire righe, ordinare, filtrare, hash -->

## Proteggere un foglio

**Revisione > Proteggi foglio** impedisce di modificare il foglio per errore. In un foglio protetto non è possibile modificare le celle **bloccate**. Tutte le celle sono bloccate finché non si indica diversamente: selezionare le celle che possono essere modificate, aprire **Formato celle > Protezione** e deselezionare **Bloccata**. (Questa impostazione ha effetto solo quando il foglio è protetto.)

La finestra di dialogo chiede quali operazioni sono ancora consentite a tutti nel foglio: selezionare celle bloccate o sbloccate, formattare le celle, modificare colonne o righe, inserire o eliminare righe e colonne, inserire collegamenti, ordinare, usare i filtri e modificare oggetti di disegno e note. Le operazioni non selezionate vengono rifiutate, con un messaggio che indica cosa è stato rifiutato.

La **password** è facoltativa. Se viene specificata, il foglio può essere sprotetto solo con questa password. JeffCrunch conserva una forma codificata della password (un hash con salt, ripetuto 100.000 volte) e mai la password stessa, quindi non è in grado di mostrarla in seguito: conservarla in un luogo sicuro. Le password create da altri programmi, sia nella forma precedente sia in quella più recente, vengono verificate allo stesso modo.

La protezione e la rimozione della protezione sono operazioni che è possibile annullare. Per rimuovere la protezione, usare **Rimuovi protezione foglio** e digitare la password, se presente.

Alcune operazioni restano comunque vietate in un foglio protetto, qualunque cosa venga consentita: unire celle ed eliminare righe o colonne che contengono celle bloccate.

## Proteggere la cartella di lavoro

**Revisione > Proteggi cartella di lavoro** blocca la **struttura**: non è possibile aggiungere, eliminare, rinominare, spostare, nascondere o scoprire fogli. È anche possibile richiedere che le finestre vengano mantenute come sono. Anche in questo caso si può specificare una password. **Rimuovi protezione cartella di lavoro** rimuove la protezione.
