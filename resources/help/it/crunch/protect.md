# Proteggere un foglio o la cartella di lavoro
<!-- keywords: proteggi, protezione, password, blocco, celle bloccate, sblocca, rimuovi protezione, struttura, fogli, sola lettura, consenti, formato celle, inserisci righe, ordina, filtra, hash, consenti intervalli modificabili, intervallo modificabile, password intervallo -->

## Proteggere un foglio

**Revisione > Proteggi foglio** impedisce che il foglio venga modificato per errore. In un foglio protetto, le celle **bloccate** non possono essere modificate. Ogni cella è bloccata finché non si decide diversamente: seleziona le celle che possono essere modificate, apri **Formato celle > Protezione** e deseleziona **Bloccata**. (Questo conta solo quando il foglio è protetto.)

La finestra di dialogo chiede che cosa tutti possono ancora fare nel foglio: selezionare celle bloccate o sbloccate, formattare celle, modificare colonne o righe, inserire o eliminare righe e colonne, inserire collegamenti, ordinare, usare filtri, e modificare oggetti di disegno e note. Ciò che non viene selezionato è negato, con un messaggio che indica che cosa è stato negato.

Una **password** è facoltativa. Se ne imposti una, il foglio può essere sbloccato solo con essa. JeffCrunch conserva una forma cifrata della password (un hash con sale, ripetuto 100.000 volte) e non la password stessa, quindi non è in grado di mostrarla in seguito: conservala in un luogo sicuro. Le password create da altri programmi, nella forma meno recente o in quella più recente, vengono verificate allo stesso modo.

Proteggere e rimuovere la protezione sono operazioni che si possono annullare. Per rimuovere la protezione, usa **Rimuovi protezione foglio**, e digita la password, se ce n'è una.

Alcune operazioni restano vietate su un foglio protetto, qualunque cosa sia consentita: unire celle, ed eliminare righe o colonne che contengono celle bloccate.

## Consentire la modifica di alcune celle: Consenti intervalli modificabili

**Revisione > Consenti intervalli modificabili** elenca gli **intervalli** che le persone possono modificare anche quando il foglio è protetto. Un intervallo è un blocco di celle, ad esempio A1:C5. Fai clic su **Nuovo** per aggiungerne uno. Assegnagli un **titolo**, digita le celle che copre (usa uno spazio tra un blocco e l'altro) e, se vuoi, una **password**. Un intervallo senza password può essere modificato da chiunque. Un intervallo con password può essere modificato solo dopo che, la prima volta che si tenta di modificarne una cella, viene digitata la password. Dopodiché l'intervallo resta aperto fino alla chiusura della cartella di lavoro.

**Modifica** cambia un intervallo, ed **Elimina** lo rimuove. Puoi impostare gli intervalli prima di proteggere il foglio. Vengono salvati con la cartella di lavoro, e le password sono conservate nella stessa forma cifrata della password del foglio. Consenti intervalli modificabili è un'operazione che si può annullare. Quando inserisci o elimini righe, colonne o celle, gli intervalli si spostano insieme alle loro celle, si restringono o si allargano, oppure scompaiono se tutte le loro celle vengono eliminate. Annulla li ripristina.

## Proteggere la cartella di lavoro

**Revisione > Proteggi struttura cartella di lavoro** blocca la **struttura**: non è possibile aggiungere, eliminare, rinominare, spostare, nascondere o mostrare alcun foglio. Può anche chiedere che le finestre restino come sono. Può avere anche una password. **Rimuovi protezione cartella di lavoro** la disattiva.
