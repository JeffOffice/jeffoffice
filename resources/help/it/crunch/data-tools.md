# Strumenti dati

<!-- keywords: previsione foglio, previsione, valori futuri, intervallo di confidenza, stagionalità, tendenza, livellamento, maschera, maschera dati, inserimento dati, record, nuovo record, trova successivo, trova precedente, criteri, gestione scenari, scenari, what-if, cosa succede se, riepilogo scenari, dati, scheda dati, testo in colonne, dividi, separatore, virgola, tabulazione, larghezza fissa, rimuovi duplicati, ricerca obiettivo, subtotale, totali, filtro avanzato, condizioni, convalida dati, cerchia dati non validi, ovale rosso, cancella cerchi di convalida, consolida, unisci intervalli, consenti, elenco, discesa, messaggio di input, avviso di errore, righe ripetute, univoci, file di testo, csv, da testo, colonne, delimitatore, virgolette, modifica collegamenti, aggiorna valori, cambia origine, collegamenti, esterno, origine, interrompi collegamento, verifica stato, apri origine</!-->

La scheda **Dati** contiene gli strumenti che lavorano su un elenco di valori: recuperare testo da un file, ordinare un elenco, dividere il testo in colonne ed eliminare le righe ripetute.

## Da testo/CSV

**Da testo/CSV** apre un file di testo (.csv, .tsv o .txt). Equivale a **Apri file di testo** nella scheda **File**. JeffCrunch chiede come il file è suddiviso in colonne e che tipo di testo contiene, quindi lo mostra come nuova cartella di lavoro.

## Da file

**Da file** nel gruppo **Recupera dati** carica le righe di un file JSON, di un file XML o di un'altra cartella di lavoro come tabella. È possibile caricare solo un file presente su questo computer. Un indirizzo web o un file su un altro computer vengono rifiutati.

1. Fai clic su **Da file**, quindi su **Sfoglia** e scegli il file. Le prime righe compaiono subito, con il tipo che JeffCrunch ha riconosciuto per ogni colonna in base a quelle righe: numero, data, vero o falso, oppure testo. Il file viene controllato per intero al caricamento, quindi una colonna può risultare di tipo testo. Nel foglio non cambia nulla finché non fai clic su **Carica**.
2. Se il file contiene più elenchi, scegli quale caricare. In un file JSON è un elenco di elementi con valori denominati, ad esempio `$.items`. In un file XML è un elemento che si ripete, ad esempio `/feed/entry`. In una cartella di lavoro è un foglio, una tabella o un intervallo denominato.
3. Un elemento JSON contenuto in un altro elemento diventa una colonna chiamata `esterno.interno`, fino a quattro livelli. Un elenco di valori semplici viene unito in una sola cella (i primi 10.000 valori) oppure distribuito su più colonne (le prime 16), a tua scelta. In XML, gli attributi e gli elementi figli diventano colonne. Due valori diversi non condividono mai una colonna: se due finirebbero con lo stesso nome, al secondo viene aggiunto un numero oppure, in XML, il suo prefisso, e una nota lo segnala. Un nome ripetuto all'interno di un elemento JSON mantiene l'ultimo valore, e una nota lo segnala. Un numero troppo grande o troppo piccolo per una cella viene mantenuto come testo, così come è scritto, e un carattere che non può essere letto viene mostrato con un segno sostitutivo; entrambi i casi vengono segnalati con una nota.
4. Seleziona **Carica tutte le colonne come testo** per mantenere ogni valore così come è scritto. Una colonna di numeri con uno zero iniziale, come 007, viene sempre mantenuta come testo.
5. Digita la cella in cui inserire la tabella, quindi fai clic su **Carica**. Se la tabella occuperebbe celle che contengono dati, JeffCrunch chiede conferma prima di sostituirli. Se non c'è spazio o il foglio è protetto, viene segnalato e non cambia nulla.

La tabella costituisce un passaggio di **Annulla**. Memorizza il file e le tue scelte. Per leggere di nuovo il file, fai clic su una cella della tabella e poi su **Aggiorna** nella scheda **Dati**. **Aggiorna tutto** è il pulsante grande accanto: legge tutte le tabelle create con Recupera dati, quindi aggiorna tutte le tabelle pivot, il tutto come un unico passaggio di **Annulla**. La tabella cresce o si riduce in base alle righe del file. Le colonne aggiunte accanto alla tabella restano: una formula trascinata in basso viene estesa alle nuove righe, mentre le righe non più presenti nel file vengono eliminate anche da lì. Se il file non esiste più o non può essere letto, viene segnalato e non cambia nulla. Se sono cambiate le colonne del file, viene indicato quali mancano o sono nuove, e non cambia nulla. Crea di nuovo la tabella con Recupera dati, oppure ripristina le colonne nel file.

Il collegamento al file viene conservato nella tabella, in una cartella di lavoro `.xlsx` salvata, in una parte che gli altri programmi ignorano. Il collegamento contiene il percorso completo del file su questo computer, quindi chiunque riceva la cartella di lavoro può leggere quel percorso. Per eliminarlo prima di condividere la cartella di lavoro, usa **Ispeziona**, quindi **Collegamenti a file di dati**, quindi **Rimuovi**. Una copia salvata con un altro tipo di file non lo conserva, e **Verifica compatibilità** lo indica. **Ispeziona** elenca i collegamenti in **Collegamenti a file di dati** e può rimuoverli. Le tabelle e le loro celle restano.

I file vengono letti con dei limiti: un file fino a 64 MB, 200.000 righe, 256 colonne e 32.767 caratteri in una cella. Gli XML con una dichiarazione di tipo documento o con entità vengono rifiutati. Ogni elemento tagliato da un limite viene segnalato.

## Modifica collegamenti

Se una cartella di lavoro contiene formule che leggono un'altra cartella di lavoro (come ='[Prezzi.xlsx]Foglio1'!A1), **Modifica collegamenti** nel gruppo **Recupera dati** elenca le altre cartelle di lavoro, dette origini. Ogni riga indica dove si trova l'origine e il suo stato. Finché la cartella di lavoro non viene riaperta con le sue origini, mostra i numeri memorizzati nel file.

- **Verifica stato** cerca ogni origine su questo computer e indica **Trovata**, oppure che non è stata trovata. Un'origine sul web non viene recuperata, quindi il suo stato indica che non è stata verificata.
- **Aggiorna valori** legge l'origine scelta da questo computer e assegna a ogni cella che è solo un riferimento a una sua cella il valore che quella cella ha adesso. Una cella che fa qualcosa in più con il valore, come `=[1]Prezzi!A1*2`, mantiene il numero memorizzato nel file, e il messaggio sotto l'elenco indica quante celle sono state aggiornate e quante no. **Aggiorna valori** è un unico passaggio, quindi **Annulla** ripristina i valori precedenti. I nuovi valori vengono scritti anche nella parte del file dedicata al collegamento al salvataggio, così la cartella di lavoro li mostra alla successiva apertura, anche quando l'origine non è presente.
- **Cambia origine** consente di scegliere un altro file per il collegamento selezionato, ad esempio quando la cartella di lavoro è stata spostata. Le formule restano invariate. La nuova posizione viene scritta nel file al salvataggio, e **Annulla** annulla la modifica.
- **Apri origine** apre l'origine scelta in una nuova finestra, quando può essere trovata.
- **Interrompi collegamento** trasforma ogni cella che legge l'origine scelta nel valore che mostra ora, e un nome che indicava una posizione in essa diventa **#RIF!**. JeffCrunch chiede prima conferma. Interrompi collegamento è un unico passaggio, quindi **Annulla** ripristina le formule, i nomi e i collegamenti, fino al salvataggio della cartella di lavoro. Quando non rimane alcun collegamento, il file viene salvato senza le parti che li contenevano.

## Maschera

**Maschera** (nel gruppo **Strumenti dati** e nella casella di ricerca) mostra una riga di un elenco alla volta, in una piccola maschera. Un elenco è una tabella oppure un intervallo di celle con una riga di nomi in alto e una riga di dati sotto ciascun nome. Fai prima clic su una cella qualsiasi dell'elenco.

- Ogni casella corrisponde a una colonna ed è etichettata con il nome nella riga superiore. Digita in una casella per modificare il valore. Una casella in grigio contiene una formula: la maschera mostra il risultato, e la formula si modifica nel foglio.
- **Trova successivo** e **Trova precedente** passano alla riga successiva e a quella precedente. Passare a un'altra riga, oppure premere **Invio**, scrive in questa riga quanto hai digitato.
- **Nuovo** mostra una maschera vuota. Compilala, quindi premi di nuovo **Nuovo**, **Trova successivo** oppure **Chiudi**. La riga viene aggiunta sotto l'ultima riga dell'elenco. Una tabella cresce di una riga. Se una cella sotto l'elenco non è vuota, la maschera lo segnala e non aggiunge nulla.
- **Elimina** rimuove la riga che stai visualizzando, dopo aver chiesto conferma. Le righe sotto di essa salgono. Una tabella mantiene sempre almeno una riga di dati. Una riga nascosta, oppure nascosta da un filtro, non viene mostrata nella maschera, non viene contata e non viene mai eliminata da essa. Se l'elenco è troppo lungo per essere analizzato per intero, la maschera indica che mostra solo le prime righe.
- **Ripristina** annulla quanto digitato in questa riga, prima di lasciarla.
- **Criteri** consente di cercare righe. Digita una condizione in una o più caselle, quindi premi **Trova successivo** o **Trova precedente**. Una condizione è un valore, come **Ann** (il testo corrisponde all'inizio della parola, e **?** e ***** indicano lettere), oppure un valore preceduto da un segno: **>100**, **<=5**, **<>0**, **=Oslo**. Una riga deve soddisfare tutte le condizioni digitate. Premi **Maschera** per tornare indietro, oppure **Cancella** per ricominciare. Quando sono impostati dei criteri, **Trova successivo** e **Trova precedente** saltano le righe che non li soddisfano.
- **Chiudi** scrive la riga e chiude la maschera.

Ogni riga modificata, aggiunta o eliminata costituisce un passaggio, quindi **Annulla** nel foglio la riporta indietro in una sola volta.

## Ordina e filtra

Il gruppo **Ordina e filtra** contiene gli stessi comandi del menu **Ordina e filtra** nella scheda **Home**. Vedi [Ordina e filtra](sort-filter).

## Testo in colonne

Usalo quando una colonna contiene più parti di testo che vuoi suddividere in colonne separate, come "Rossi, Anna, 31".

1. Seleziona le celle di **una sola** colonna.
2. Scegli **Testo in colonne** nella scheda **Dati**.
3. Scegli come dividere il testo:
   - **Dividi in corrispondenza di un separatore**: seleziona i caratteri che si trovano tra le parti: **Tabulazione**, **Punto e virgola**, **Virgola**, **Spazio** oppure **Altro**, dove digiti il carattere che vuoi. **Considera i separatori consecutivi come uno solo** evita che due virgole di seguito creino una parte vuota. **Testo tra virgolette** indica il carattere che racchiude una parte che non deve essere divisa, come "Lee, Bo". All'interno delle virgolette, due caratteri uguali di seguito valgono come uno solo.
   - **Dividi in posizioni fisse**: digita le posizioni in cui inizia una nuova colonna, contando i caratteri dal numero 1. Ad esempio, 5, 12 fa iniziare una nuova colonna dal quinto e dal dodicesimo carattere.
4. Scegli il tipo di ogni parte: **Generale** legge numeri e date come se li avessi digitati, mentre **Testo** mantiene ogni parte così com'è, quindi "007" resta "007".
   Per trattare diversamente una parte, digita il suo numero in **Parte** e scegli come leggerla: **Uguale a quella sopra**, **Come digitato**, **Testo**, una data scritta come **mese, giorno, anno**, **giorno, mese, anno** oppure **anno, mese, giorno**, oppure **Escludi**, che non colloca la parte da nessuna parte. La prima parte è la 1. Una parte data può avere il mese scritto per esteso (**5 mar 2024**, **5 marzo 2024**) in inglese o nella tua lingua, può essere separata da /, -, un punto, una virgola o spazi, e può essere seguita da un orario (**10:30**, **10:30:45** oppure **10:30 PM**). Una parte che non è una data nell'ordine scelto (un mese 13, un giorno inesistente, un anno che non ha due o quattro cifre, oppure un orario non valido) viene mantenuta come testo, così come è stata digitata; non viene mai letta in un altro ordine.
5. In **Inserisci le parti a partire da**, digita la prima cella per le parti. Inizialmente corrisponde alla prima cella selezionata.
6. La casella sotto le opzioni mostra le prime righe divise secondo le tue scelte. Premi **OK**.

Le parti vengono inserite nelle celle a destra della prima cella e sostituiscono il contenuto preesistente. Testo in colonne è un passaggio che puoi annullare.

## Rimuovi duplicati

Usalo per conservare solo la prima di ogni gruppo di righe ripetute.

1. Seleziona le celle dell'elenco, oppure una cella al suo interno. JeffCrunch prende l'intero blocco di dati intorno a una singola cella.
2. Scegli **Rimuovi duplicati**.
3. Seleziona **I miei dati hanno una riga di intestazione** se la prima riga contiene dei nomi. Quella riga resta e non viene confrontata.
4. Seleziona le colonne che devono coincidere perché due righe siano considerate ripetute. Due righe sono ripetute solo quando tutte le colonne selezionate sono uguali. Maiuscole e minuscole sono considerate uguali, e una formula conta come il valore che restituisce.
5. Premi **OK**. La finestra indica quante righe sono state rimosse e quante ne restano.

Le righe sotto una riga rimossa salgono all'interno delle celle selezionate, con i rispettivi formati e formule. Le celle sotto la selezione non si spostano. Le righe vuote lasciate in fondo alla selezione vengono svuotate. Rimuovi duplicati è un passaggio che puoi annullare.

## Ricerca obiettivo

Usalo quando conosci il risultato che vuoi ma non il numero che lo produce. Ad esempio, una cella di pagamento contiene una formula che usa un tasso di interesse, e vuoi che il pagamento sia 500. Ricerca obiettivo prova dei numeri nella cella del tasso finché il pagamento non è 500.

1. Scegli **Ricerca obiettivo** nella scheda **Dati**.
2. In **Cella con la formula**, digita la cella che contiene la formula, ad esempio B3. Inizialmente corrisponde alla cella selezionata.
3. In **Valore desiderato**, digita il risultato che vuoi ottenere dalla formula.
4. In **Cella da modificare**, digita la cella che Ricerca obiettivo può cambiare, ad esempio B1. Deve contenere un numero, non una formula.
5. Premi **OK**.

Se Ricerca obiettivo trova un numero, questo resta nella cella da modificare, e la barra di stato indica quale sia. Se non ci riesce, non cambia nulla e viene segnalato. Prova un numero iniziale diverso nella cella da modificare, e verifica che la formula usi davvero quella cella. Ricerca obiettivo è un passaggio che puoi annullare.

## Gestione scenari

Uno scenario è un insieme denominato di valori per alcune celle. Usa gli scenari per provare ipotesi "what if" su un unico foglio. Ad esempio, conserva uno scenario "Vendite basse" e uno scenario "Vendite alte" per le stesse celle di input, e mostra quello che vuoi esaminare.

1. Scegli **Gestione scenari** nella scheda **Dati**.
2. Premi **Aggiungi**. Digita un nome, ad esempio Vendite alte. In **Celle da modificare**, digita le celle che lo scenario imposta, ad esempio B1:B3 oppure D5. Uno scenario può modificare fino a 32 celle. Le celle selezionate vengono compilate automaticamente. Ogni cella parte dal valore che contiene ora. Digita nella tabella il valore che questo scenario deve usare, quindi premi **OK**.
3. Scegli uno scenario nell'elenco e premi **Mostra** per inserire i suoi valori nelle celle. Anche un doppio clic sul nome ha lo stesso effetto. Premi **Modifica** per cambiarne il nome, il commento o i valori. Premi **Elimina** per rimuoverlo.

Mostra, Aggiungi, Modifica ed Elimina sono ciascuno un passaggio che puoi annullare.

**Riepilogo** colloca tutti gli scenari affiancati in un nuovo foglio chiamato Riepilogo scenari. Chiede le celle del risultato, cioè le celle che vuoi confrontare, ad esempio il totale in B10. Per ogni scenario, il riepilogo mostra i valori delle celle modificate e il risultato delle celle di risultato, calcolato con i valori di quello scenario. Una colonna chiamata Valori correnti mostra com'erano le celle quando hai creato il riepilogo. Le tue celle restano come erano.

Gli scenari vengono salvati nella cartella di lavoro. Una cartella di lavoro creata con un altro programma conserva i suoi scenari, che qui puoi vedere. Una copia salvata come file .xls meno recente o come .ods non li conserva, e il salvataggio lo indica.

## Subtotale

Usalo per inserire una riga di totali sotto ogni gruppo di righe che hanno la stessa voce in una colonna, e un totale generale in fondo. L'elenco richiede una riga di intestazione e dovrebbe essere ordinato in base alla colonna da raggruppare, in modo che le voci uguali siano vicine.

1. Seleziona una cella dell'elenco.
2. Scegli **Subtotale** nel gruppo **Struttura** della scheda **Dati**.
3. In **A ogni variazione di**, scegli la colonna che raggruppa le righe, ad esempio Regione.
4. In **Usa la funzione**, scegli cosa calcolare: **Somma**, **Conta**, **Media**, **Max**, **Min** e altre.
5. In **Aggiungi subtotale a**, seleziona ogni colonna da totalizzare.
6. Lascia selezionato **Sostituisci i subtotali correnti** per sostituire i subtotali già presenti.
7. **Riepilogo sotto i dati** inizialmente segue la struttura del foglio. Lascialo selezionato per mettere ogni riga di totale sotto il suo gruppo, oppure deselezionalo per mettere ogni riga di totale sopra il suo gruppo, con il totale generale in alto. Premi **OK**.

Ogni riga di totale riporta "Totale Est" (oppure la voce a cui si riferisce) e contiene una formula SUBTOTALE, quindi ignora le altre righe di totale. Le righe di ogni gruppo sono raggruppate, così puoi usare i pulsanti 1, 2 e 3 accanto alle righe per mostrare solo i totali. Per eliminare i totali, apri di nuovo **Subtotale** e premi **Rimuovi tutto**. Se i totali sono finiti dall'altra parte di un foglio che aveva già i suoi gruppi, la barra di stato indica che quei gruppi ora hanno le righe di riepilogo da quel lato, e **Rimuovi tutto** riporta il lato com'era prima del primo subtotale, a meno che tu non l'abbia cambiato manualmente nel frattempo. Quando i totali sono sopra, i pulsanti della struttura si trovano sulle righe di totale, quindi un pulsante più o meno accanto a "Totale Est" mostra o nasconde le righe Est sottostanti. Se disattivi **Sostituisci i subtotali correnti**, i nuovi totali devono trovarsi dallo stesso lato di quelli già presenti. In caso contrario non cambia nulla, e un messaggio lo indica. Aggiungere o rimuovere subtotali è un passaggio che puoi annullare.

## Filtro avanzato

I pulsanti di filtro scelgono le righe una colonna alla volta. **Avanzato** (nel gruppo **Ordina e filtra**) permette di scrivere le condizioni nel foglio, quindi possono essere lunghe quanto serve.

1. Accanto all'elenco, copia le intestazioni delle colonne che vuoi verificare in una riga di celle vuote.
2. Sotto ogni intestazione, scrivi una condizione. Le condizioni nella **stessa riga** devono essere tutte vere (questo è "e"). Le condizioni in **righe diverse** sono alternative: una riga dell'elenco deve soddisfare solo una di quelle righe (questo è "oppure").
3. Scegli **Avanzato**. Verifica che **Intervallo elenco** sia l'elenco con la sua riga di intestazione, e digita le celle delle intestazioni e delle condizioni in **Intervallo condizioni**.
4. Scegli **Nascondi le altre righe nell'elenco**, oppure **Copia le righe in un'altra posizione** e digita la prima cella della nuova posizione, ad esempio H1. Seleziona **Escludi le righe che ripetono una precedente** per copiarne solo una per ogni tipo.
5. Premi **OK**.

Come scrivere una condizione:

- Una parola, come **An**, seleziona il testo che inizia con quella parola. Maiuscole e minuscole sono considerate uguali. **?** indica un carattere qualsiasi e ***** una sequenza qualsiasi di caratteri.
- Un numero seleziona le celle uguali ad esso.
- **=Ann** seleziona il testo esattamente uguale a Ann. **=** da solo seleziona le celle vuote.
- **>**, **>=**, **<**, **<=** e **<>** (diverso) prima di un numero o di una parola lo confrontano, ad esempio **>25**. **<>** da solo seleziona le celle non vuote.

Una riga di condizioni senza contenuto seleziona tutte le righe. Per mostrare di nuovo le righe nascoste, scegli **Cancella**. La copia viene creata subito e non cambia quando cambia l'elenco. Filtro avanzato è un passaggio che puoi annullare.

## Convalida dati

Usala per stabilire cosa si può digitare in alcune celle, in modo che un inserimento errato venga bloccato o messo in discussione.

1. Seleziona le celle.
2. Scegli **Convalida dati** nel gruppo **Strumenti dati** della scheda **Dati**.
3. Nella scheda **Impostazioni**, scegli cosa **Consenti**: **Qualsiasi valore** (nessuna regola), **Numero intero**, **Decimale**, **Elenco**, **Data**, **Ora**, **Lunghezza testo** oppure **Personalizzata**.
4. Per un numero, una data, un orario o una lunghezza del testo, scegli **Dati** (**compreso tra**, **non compreso tra**, **uguale a** e così via) e digita i limiti. Un limite può essere un valore oppure una formula che inizia con =.
5. Per un **Elenco**, digita le scelte separate da punti e virgola, ad esempio Rosso;Verde;Blu, oppure digita un intervallo di celle dopo =, ad esempio =$A$1:$A$5. La cella mostra allora una freccia con le scelte. Deseleziona **Elenco a discesa nella cella** per nascondere la freccia.
6. Per **Personalizzata**, digita una formula che inizia con =. L'inserimento è consentito quando la formula è vera.
7. **Ignora spazi vuoti** consente il passaggio di una cella vuota.

Nella scheda **Messaggio di input**, seleziona la casella e scrivi un titolo e un messaggio. Compaiono accanto alla cella ogni volta che è selezionata.

Nella scheda **Avviso di errore**, scegli cosa succede a un inserimento non consentito. **Arresta** lo rifiuta. **Avviso** chiede se mantenerlo. **Informazioni** avvisa la persona e lo mantiene. Scrivi un titolo e un messaggio per l'avviso.

**Cerchia dati non validi** (accanto a **Convalida dati**) disegna un ovale rosso intorno a ogni cella del foglio il cui valore non è consentito dalla sua regola, ad esempio un numero che c'era prima della regola o il risultato di una formula. Un messaggio indica se tutte le celle sono corrette. Una cella che correggi perde il suo ovale. **Cancella cerchi di convalida** rimuove tutti gli ovali.

**Cancella tutto** rimuove le regole dalle celle selezionate. La nuova regola sostituisce quelle già presenti nelle celle selezionate; le altre celle mantengono le proprie. La modifica delle regole è un passaggio che puoi annullare, e le regole vengono salvate nel file.

## Consolida

Usalo per unire intervalli simili, come la stessa tabella di più fogli, in un'unica tabella.

1. Scegli **Consolida** nel gruppo **Strumenti dati** della scheda **Dati**.
2. Scegli la **Funzione** con cui combinare, ad esempio **Somma** o **Media**.
3. Digita un intervallo nella casella e premi **Aggiungi**. Usa **Foglio2!A1:C10** per un intervallo su un altro foglio. Aggiungi ogni intervallo che vuoi combinare. Seleziona un intervallo nell'elenco e premi **Elimina** per rimuoverlo.
4. Se gli intervalli hanno nomi nella prima riga, seleziona **La riga superiore contiene i nomi delle colonne**. Se hanno nomi nella prima colonna, seleziona **La colonna a sinistra contiene i nomi delle righe**.
5. In **Inserisci il risultato in**, digita la prima cella per il risultato.
6. Seleziona **Crea collegamenti ai dati di origine** se ogni risultato deve essere una formula che legge le sue celle di origine, così da cambiare quando cambiano quelle. Senza questa opzione, i risultati sono numeri semplici.
7. Premi **OK**.

Se non è selezionato alcun nome, viene combinata la cella nella stessa posizione in ogni intervallo (la prima cella di ogni intervallo, poi la seconda, e così via). Con i nomi selezionati, vengono combinate le celle con gli stessi nomi, ovunque si trovino in ciascun intervallo, e un nome presente solo in alcuni intervalli ottiene comunque una riga o una colonna. Consolida è un passaggio che puoi annullare.

## Foglio di previsione

Foglio di previsione crea un nuovo foglio con una tabella e un grafico di valori futuri, calcolati a partire da una sequenza temporale e da una serie di valori. Individua la tendenza e la stagionalità ripetitiva dei valori, e aggiunge una fascia entro cui i valori futuri probabilmente rimarranno.

1. Seleziona la sequenza temporale e i valori, due colonne affiancate (prima la sequenza temporale). La sequenza temporale è composta da date o numeri distanziati in modo uniforme, ad esempio ogni mese o ogni 5 unità. Una prima riga di nomi viene esclusa.
2. Scegli **Foglio di previsione** nel gruppo **Previsione** della scheda **Dati**.
3. Un'immagine mostra la previsione e cambia quando modifichi le opzioni. Scegli **Grafico a linee** oppure **Grafico a colonne**.
4. Imposta le opzioni:
   - **Intervallo sequenza temporale** e **Intervallo valori** sono le celle da leggere. Scrivi qui altre celle se la selezione era errata.
   - **Inizio previsione** è vuoto per iniziare dopo l'ultimo punto. Una data o un numero precedente all'ultimo punto inizia da lì e usa solo i punti che lo precedono, così puoi confrontare la previsione con ciò che è accaduto.
   - **Fine previsione** è l'ultimo momento da prevedere. Si possono prevedere al massimo 10.000 punti.
   - **Intervallo di confidenza** indica quanto è sicura la fascia, inizialmente 95%.
   - **Stagionalità** viene individuata osservando i valori, oppure puoi impostare la lunghezza della stagione in punti, o scegliere nessuna. I valori devono contenere due stagioni complete.
   - **Riempi i punti mancanti con** indica cosa ottiene un momento privo di valore: una retta tra i suoi vicini, oppure zero.
   - **Aggrega i duplicati usando** indica come vengono combinati i valori con lo stesso momento: Media, Conta, ContaValori, Max, Mediana, Min oppure Somma.
   - **Includi statistiche di previsione** aggiunge una tabella con Alfa, Beta, Gamma, MASE, SMAPE, MAE e RMSE, che indicano quanto bene la previsione si adatta ai valori.
5. Se qualcosa non va, la finestra lo spiega con parole semplici (passaggi non uguali, pochi punti, testo nei valori), e **Crea** resta in attesa.
6. Premi **Crea**.

Il nuovo foglio viene inserito prima di quello su cui ti trovavi. La sua tabella ha le colonne Sequenza temporale, Valori, Previsione, Limite di confidenza inferiore e Limite di confidenza superiore. La previsione e i limiti sono formule che usano `FORECAST.ETS` e `FORECAST.ETS.CONFINT` sulle colonne Sequenza temporale e Valori della tabella stessa, quindi vengono ricalcolate quando cambi un valore. Il grafico mostra i valori, la previsione e la fascia. Foglio di previsione è un passaggio che puoi annullare.

I numeri possono differire di alcuni punti percentuali rispetto a quelli di altri programmi di fogli di calcolo, che usano un proprio metodo per adattare il livellamento, e una previsione già presente in un file mantiene i valori memorizzati nel file finché non cambiano i dati da cui legge.

Un file con più di 1.000.000 di punti non viene utilizzato.
