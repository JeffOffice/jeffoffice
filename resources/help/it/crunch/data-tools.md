# Strumenti dati

<!-- keywords: previsione foglio, previsione, valori futuri, intervallo di confidenza, stagionalità, tendenza, lisciamento, maschera, maschera dati, inserimento dati, record, nuovo record, trova successivo, trova precedente, criteri, gestione scenari, scenari, analisi what-if, e se, riepilogo scenari, dati, scheda dati, testo in colonne, dividi, separatore, virgola, tabulazione, larghezza fissa, rimuovi duplicati, ricerca obiettivo, subtotale, totali, filtro avanzato, condizioni, convalida dati, cerchia dati non validi, ovale rosso, cancella cerchi di convalida, consolida, unisci intervalli, consenti, elenco, elenco a discesa, messaggio di input, avviso di errore, righe ripetute, unico, file di testo, csv, da testo, colonne, delimitatore, virgolette, modifica collegamenti, aggiorna valori, cambia origine, collegamenti, esterno, origine, interrompi collegamento, verifica stato, apri origine -->

La scheda **Dati** contiene gli strumenti che lavorano su un elenco di valori: recuperare testo da un file, ordinare un elenco, dividere il testo in colonne ed eliminare le righe ripetute.

## Da testo/CSV

**Da testo/CSV** apre un file di testo (.csv, .tsv o .txt). Equivale a **Apri file di testo** nella scheda File. JeffCrunch chiede come è suddiviso il file in colonne e che cosa contiene ciascuna colonna, quindi lo mostra come nuova cartella di lavoro. La stessa finestra si apre quando apri un file di testo o usi **Incolla importazione testo**.

La finestra indovina il contenuto di ciascuna colonna dalle prime 200 righe del file e mostra l'ipotesi nella casella sopra la colonna. Modifica qualsiasi colonna prima di aprire il file. Un tipo che hai modificato non viene mai sostituito da un'ipotesi successiva, anche se in seguito cambi il separatore. Un'ipotesi fa solo tre cose a una colonna: lascia la colonna **Come digitato**, che è il modo in cui si legge un file quando non si indovina nulla; la imposta come **Testo** per proteggerla; oppure la imposta come date in un ordine che i valori dimostrano. Ecco le regole:

- Una colonna resta **Come digitato** quando Come digitato legge correttamente i suoi valori: numeri, valori vero e falso, orari, date scritte con i nomi dei mesi, numeri con segno come -5 e +3, parole e numeri di versione come 1.13.2. Cambiarli peggiorerebbe le cose.
- Una colonna è una **data** solo quando ogni valore contiene un anno di quattro cifre (oppure è una data completa, come 2024-05-13) e può essere letto come data in un solo ordine (mese, giorno, anno; giorno, mese, anno; oppure anno, mese, giorno). Le sue date sono mostrate come le date che digiti. Quando più di un ordine va bene per tutti i valori, come 05/06/2024, la colonna resta **Testo**, la finestra la elenca e puoi scegliere l'ordine dalla sua casella.
- I valori con tre numeri e senza anno di quattro cifre, come 1.13.2 o 13.05.24, non vengono mai trasformati in date dall'ipotesi. Se Come digitato li manterrebbe come testo, la colonna resta Come digitato; se Come digitato li trasformerebbe in date, la colonna diventa **Testo**. In entrambi i casi la finestra elenca una colonna che potrebbe contenere date con anni a due cifre, così puoi scegliere un ordine.
- Una colonna è **Testo** quando un valore ha uno zero davanti a più cifre o ha più di 15 cifre, quando un segno più precede sette o più cifre (un numero di telefono), oppure quando un esponente come 1E5 si trova accanto ad altri valori.
- Un valore che inizia con il segno di uguale è lasciato alla casella **Leggi le formule**. Quando la casella è selezionata, la colonna resta Come digitato, così il valore viene letto come formula. Quando non è selezionata, la colonna è **Testo**.
- Seleziona **La prima riga contiene i nomi delle colonne** e la prima riga non viene usata per l'ipotesi. La finestra la seleziona quando la prima riga è testo sopra numeri o date.
- Le colonne oltre la centesima vengono importate come testo.

Vengono controllate solo le prime 200 righe, quindi un valore più in basso può riservare sorprese. In una colonna di date, un valore più in basso che non è una data nell'ordine scelto viene importato come testo, mai come data sbagliata. In una colonna Testo, ogni valore viene importato così come è scritto. In una colonna Come digitato, un valore più in basso viene letto come Come digitato lo legge in qualsiasi altro punto.

**Incolla importazione testo** inserisce il testo come se lo avessi digitato, con i separatori decimali e delle migliaia impostati nelle tue preferenze. Per questo la sua finestra non ha le caselle per tali separatori, e l'ipotesi legge il testo nello stesso modo in cui lo fa l'incolla. La finestra per un file le ha.

## Da file

**Da file**, nel gruppo **Recupera dati**, carica le righe di un file JSON, di un file XML o di un'altra cartella di lavoro come tabella. È possibile caricare solo un file presente su questo computer. Un indirizzo web o un file su un altro computer vengono rifiutati.

1. Fai clic su **Da file**, quindi su **Sfoglia** e scegli il file. Subito compaiono le prime righe, con il tipo che JeffCrunch ha riconosciuto per ogni colonna in base a quelle righe: numero, data, vero o falso, oppure testo. Il file viene controllato per intero al caricamento, quindi una colonna può diventare testo. Nel foglio non cambia nulla finché non fai clic su **Carica**.
2. Quando il file contiene più di un elenco, scegli quale caricare. In un file JSON è un elenco di elementi con valori denominati, come `$.items`. In un file XML è un elemento che si ripete, come `/feed/entry`. In una cartella di lavoro è un foglio, una tabella o un intervallo denominato.
3. Un elemento JSON dentro un altro elemento diventa una colonna chiamata `esterno.interno`, fino a quattro livelli. Un elenco di valori semplici viene unito in una sola cella (i primi 10.000 valori) oppure distribuito su più colonne (le prime 16), a tua scelta. In XML, gli attributi e gli elementi figli diventano colonne. Due valori diversi non condividono mai una colonna: quando due finirebbero con lo stesso nome, al secondo viene aggiunto un numero oppure, in XML, il suo prefisso, e una nota lo segnala. Un nome ripetuto dentro un singolo elemento JSON mantiene l'ultimo valore, e una nota lo segnala. Un numero troppo grande o troppo piccolo per una cella viene mantenuto come testo, così come è scritto, e un carattere che non può essere letto viene mostrato come segno sostitutivo; entrambi i casi sono segnalati nelle note.
4. Seleziona **Carica tutte le colonne come testo** per mantenere ogni valore così come è scritto. Una colonna di numeri con uno zero iniziale, come 007, viene sempre mantenuta come testo.
5. Digita la cella in cui va la tabella, quindi fai clic su **Carica**. Se la tabella occuperebbe celle che contengono dati, JeffCrunch chiede conferma prima di sostituirli. Se non c'è spazio, oppure il foglio è protetto, viene segnalato e non cambia nulla.

La tabella è un solo passaggio di **Annulla**. Ricorda il file e le tue scelte. Per leggere di nuovo il file, fai clic su una cella della tabella e fai clic su **Aggiorna** nella scheda Dati. **Aggiorna tutto** è il pulsante grande accanto: legge ogni tabella creata con Da file, quindi aggiorna ogni tabella pivot, il tutto come un solo passaggio di Annulla. La tabella cresce o si riduce secondo le righe del file. Le colonne aggiunte accanto alla tabella restano: una formula copiata in basso viene riportata nelle nuove righe, e le righe che non ci sono più nel file vengono eliminate anche da esse. Se il file non esiste più o non può essere letto, viene segnalato e non cambia nulla. Se sono cambiate le colonne del file, viene indicato quali mancano o sono nuove, e non cambia nulla. Crea di nuovo la tabella con Da file, oppure ripristina le colonne nel file.

Il collegamento al file è conservato nella tabella, in una cartella di lavoro `.xlsx` salvata, in una parte che gli altri programmi ignorano. Il collegamento contiene il percorso completo del file su questo computer, quindi chiunque riceva la cartella di lavoro può leggere quel percorso. Per eliminarlo prima di condividere la cartella di lavoro, usa **Ispeziona**, quindi **Collegamenti a file di dati**, quindi **Rimuovi**. Una copia salvata con un altro tipo di file non lo conserva, e **Verifica compatibilità** lo segnala. **Ispeziona** elenca i collegamenti in **Collegamenti a file di dati** e può rimuoverli. Le tabelle e le loro celle restano.

I file vengono letti con dei limiti: un file fino a 64 MB, 200.000 righe, 256 colonne e 32.767 caratteri in una cella. Un XML con una dichiarazione di tipo di documento o con entità viene rifiutato. Ogni elemento tagliato da un limite viene segnalato.

## Modifica collegamenti

Se una cartella di lavoro contiene formule che leggono un'altra cartella di lavoro (come ='[Prezzi.xlsx]Foglio1'!A1), **Modifica collegamenti**, nel gruppo **Recupera dati**, elenca le altre cartelle di lavoro, dette origini. Ogni riga mostra dove si trova l'origine e il suo stato. Finché la cartella di lavoro non viene riaperta con le sue origini, mostra i numeri che il file ha memorizzato da esse.

- **Verifica stato** cerca ciascuna origine su questo computer e indica **Trovata**, oppure che non è stata trovata. Un'origine sul web non viene scaricata, quindi il suo stato indica che non è stata verificata.
- **Aggiorna valori** legge dal computer l'origine scelta e assegna a ogni cella che è solo un riferimento a una delle sue celle il valore che tale cella ha ora. Una cella che fa qualcosa di più con il valore, come `=[1]Prezzi!A1*2`, mantiene il numero memorizzato nel file, e il messaggio sotto l'elenco indica quante celle sono state aggiornate e quante no. Aggiorna valori è un solo passaggio, quindi **Annulla** ripristina i valori precedenti. I nuovi valori vengono anche scritti nella parte del collegamento all'interno del file quando salvi, così la cartella di lavoro li mostra alla prossima apertura, anche se l'origine non è presente.
- **Cambia origine** consente di scegliere un altro file per il collegamento selezionato, ad esempio quando la cartella di lavoro è stata spostata. Le formule restano invariate. Il nuovo percorso viene scritto nel file al salvataggio, e **Annulla** annulla la modifica.
- **Apri origine** apre l'origine scelta in una nuova finestra, quando può essere trovata.
- **Interrompi collegamento** trasforma ogni cella che legge l'origine scelta nel valore che mostra ora, e un nome che indicava una posizione al suo interno in **#RIF!**. JeffCrunch chiede conferma prima. Interrompi collegamento è un solo passaggio, quindi **Annulla** ripristina formule, nomi e collegamenti, fino al salvataggio della cartella di lavoro. Quando non rimane alcun collegamento, il file viene salvato senza le parti che li contenevano.

## Maschera

**Maschera** (nel gruppo **Strumenti dati** e nella casella di ricerca) mostra una riga di un elenco alla volta, come una piccola maschera. Un elenco è una tabella, oppure un intervallo di celle con una riga di nomi in alto e una riga di dati sotto ciascun nome. Fai prima clic su una cella qualsiasi dell'elenco.

- Ogni casella corrisponde a una colonna ed è etichettata con il nome della riga superiore. Digita in una casella per cambiare il valore. Una casella in grigio contiene una formula: la maschera mostra il risultato, e la formula si modifica nel foglio.
- **Trova successivo** e **Trova precedente** passano alla riga successiva e a quella precedente. Passare a un'altra riga, oppure premere **Invio**, scrive in questa riga ciò che hai digitato.
- **Nuovo** mostra una maschera vuota. Compilala, quindi premi di nuovo **Nuovo**, **Trova successivo** o **Chiudi**. La riga viene aggiunta sotto l'ultima riga dell'elenco. Una tabella cresce di una riga. Se una cella sotto l'elenco non è vuota, la maschera lo segnala e non aggiunge nulla.
- **Elimina** rimuove la riga che stai visualizzando, dopo averne chiesto conferma. Le righe sotto di essa salgono. Una tabella mantiene sempre almeno una riga di dati. Una riga nascosta, oppure nascosta da un filtro, non viene mostrata nella maschera, non viene conteggiata e non viene mai eliminata da essa. Se l'elenco è troppo lungo per essere esaminato per intero, la maschera indica che mostra solo le prime righe.
- **Ripristina** annulla ciò che hai digitato in questa riga, prima di lasciarla.
- **Criteri** consente di cercare righe. Digita una condizione in una o più caselle, quindi premi **Trova successivo** o **Trova precedente**. Una condizione è un valore, come **Ann** (il testo corrisponde all'inizio della parola, e **?** e ***** indicano lettere), oppure un valore preceduto da un segno: **>100**, **<=5**, **<>0**, **=Oslo**. Una riga deve soddisfare tutte le condizioni digitate. Premi **Maschera** per tornare indietro, oppure **Cancella** per ricominciare. Mentre sono impostati dei criteri, **Trova successivo** e **Trova precedente** saltano le righe che non li soddisfano.
- **Chiudi** scrive la riga e chiude la maschera.

Ogni riga che modifichi, aggiungi o elimini è un solo passaggio, quindi **Annulla** nel foglio la ripristina in un colpo solo.

## Ordina e filtra

Il gruppo **Ordina e filtra** contiene gli stessi comandi del menu **Ordina e filtra righe** della scheda Home. Vedi [Ordinare e filtrare](sort-filter).

## Testo in colonne

Usalo quando una colonna contiene più parti di testo che vuoi avere in colonne separate, come "Rossi, Anna, 31".

1. Seleziona le celle di **una** colonna.
2. Scegli **Testo in colonne** nella scheda **Dati**.
3. Scegli come dividere il testo:
   - **Dividi con un separatore**: seleziona i segni che si trovano tra le parti: **Tabulazione**, **Punto e virgola**, **Virgola**, **Spazio** oppure **Altro**, dove digiti il segno che preferisci. **Considera i separatori consecutivi come uno solo** evita che due virgole di seguito creino una parte vuota. **Testo tra virgolette** indica il segno che racchiude una parte che non deve essere divisa, come "Lee, Bo". Dentro le virgolette, due segni di seguito rappresentano un solo segno.
   - **Dividi in posizioni fisse**: digita le posizioni in cui inizia una nuova colonna, contando i caratteri da 1. Ad esempio, 5, 12 avvia una nuova colonna al quinto e al dodicesimo carattere.
4. Scegli come leggere ogni parte: **Generale** legge numeri e date come se li avessi digitati, mentre **Testo** mantiene ogni parte così come è, quindi "007" resta "007".
   Per trattare diversamente una parte, digita il suo numero in **Parte** e scegli come leggerla: **Come sopra**, **Come digitato**, **Testo**, una data scritta come **mese, giorno, anno**, **giorno, mese, anno** oppure **anno, mese, giorno**, oppure **Escludi**, che non inserisce quella parte da nessuna parte. La prima parte è la 1. Una parte data può avere il mese come nome (**5 mar 2024**, **5 marzo 2024**) in italiano o in inglese, può essere separata da /, -, un punto, una virgola o spazi, e può essere seguita da un orario (**10:30**, **10:30:45** oppure **10:30 PM**). Una parte che non è una data nell'ordine scelto (un mese 13, un giorno che non esiste, un anno che non ha due o quattro cifre, oppure un orario senza senso) viene mantenuta come testo, così come è stata digitata; non viene mai letta in un altro ordine.
5. In **Inserisci le parti a partire da**, digita la prima cella per le parti. Inizialmente corrisponde alla prima cella selezionata.
6. La casella sotto le scelte mostra le prime righe divise nel modo scelto. Premi **OK**.

Le parti vanno nelle celle a destra della prima cella e sostituiscono ciò che vi si trovava. Testo in colonne è un solo passaggio che puoi annullare.

## Rimuovi duplicati

Usalo per conservare solo la prima di ogni gruppo di righe ripetute.

1. Seleziona le celle dell'elenco, oppure una cella al suo interno. JeffCrunch prende l'intero blocco di dati intorno a una singola cella.
2. Scegli **Rimuovi duplicati**.
3. Seleziona **I miei dati hanno una riga di intestazione** se la prima riga contiene dei nomi. Quella riga resta e non viene confrontata.
4. Seleziona le colonne che devono coincidere perché due righe siano considerate ripetute. Due righe sono ripetute solo quando tutte le colonne selezionate sono uguali. Lettere maiuscole e minuscole vengono considerate uguali, e una formula conta come il valore che restituisce.
5. Premi **OK**. La finestra indica quante righe sono state rimosse e quante ne restano.

Le righe sotto una riga rimossa salgono all'interno delle celle selezionate, con i loro formati e le loro formule. Le celle sotto la selezione non si spostano. Le righe vuote lasciate in fondo alla selezione vengono cancellate. Rimuovi duplicati è un solo passaggio che puoi annullare.

## Ricerca obiettivo

Usalo quando conosci il risultato che vuoi ma non il numero che lo produce. Ad esempio, una cella di pagamento contiene una formula che usa un tasso di interesse, e vuoi che il pagamento sia 500. Ricerca obiettivo prova dei numeri nella cella del tasso finché il pagamento non è 500.

1. Scegli **Ricerca obiettivo** nella scheda **Dati**.
2. In **Cella con la formula**, digita la cella che contiene la formula, come B3. Inizialmente corrisponde alla cella selezionata.
3. In **Valore desiderato**, digita il risultato che vuoi ottenere dalla formula.
4. In **Cella da modificare**, digita la cella che Ricerca obiettivo può cambiare, come B1. Deve contenere un numero, non una formula.
5. Premi **OK**.

Se Ricerca obiettivo trova un numero, questo resta nella cella da modificare, e la barra di stato indica quale sia. Se non ci riesce, non cambia nulla e viene segnalato. Prova un numero iniziale diverso nella cella da modificare, e verifica che la formula usi davvero quella cella. Ricerca obiettivo è un solo passaggio che puoi annullare.

## Gestione scenari

Uno scenario è un insieme denominato di valori per alcune celle. Usa gli scenari per provare ipotesi "what if" su un solo foglio. Ad esempio, conserva uno scenario "Vendite basse" e uno "Vendite alte" per le stesse celle di input, e mostra quello che vuoi esaminare.

1. Scegli **Gestione scenari** nella scheda **Dati**.
2. Premi **Aggiungi**. Digita un nome, come Vendite alte. In **Celle da modificare**, digita le celle che lo scenario imposta, come B1:B3 oppure D5. Uno scenario può modificare fino a 32 celle. Le celle selezionate vengono compilate automaticamente. Ogni cella inizia con il valore che contiene ora. Digita nella tabella il valore che questo scenario deve usare, quindi premi **OK**.
3. Scegli uno scenario nell'elenco e premi **Mostra** per mettere i suoi valori nelle celle. Fare doppio clic su un nome fa lo stesso. Premi **Modifica** per cambiarne il nome, il commento o i valori. Premi **Elimina** per rimuoverlo.

Mostra, Aggiungi, Modifica ed Elimina sono ciascuno un passaggio che puoi annullare.

**Riepilogo** affianca tutti gli scenari in un nuovo foglio chiamato Riepilogo scenari. Chiede le celle del risultato, cioè le celle che vuoi confrontare, come il totale in B10. Per ogni scenario, il riepilogo mostra i valori delle celle modificate e i risultati delle celle del risultato, calcolati con i valori di quello scenario. Una colonna chiamata Valori correnti mostra come erano le celle quando hai creato il riepilogo. Le tue celle restano come erano.

Gli scenari sono salvati nella cartella di lavoro. Una cartella di lavoro creata da un altro programma conserva i suoi scenari, che qui puoi vedere. Una copia salvata come file .xls precedente oppure come .ods non li conserva, e il salvataggio lo segnala.

## Subtotale

Usalo per inserire una riga di totali sotto ogni gruppo di righe che hanno la stessa voce in una colonna, e un totale generale in fondo. L'elenco deve avere una riga di intestazione e deve essere ordinato in base alla colonna per cui raggruppi, così che le voci uguali siano vicine.

1. Seleziona una cella dell'elenco.
2. Scegli **Subtotale** nel gruppo **Struttura** della scheda **Dati**.
3. In **A ogni variazione di**, scegli la colonna che raggruppa le righe, come Regione.
4. In **Usa funzione**, scegli cosa calcolare: **Somma**, **Conta**, **Media**, **Max**, **Min** e altre.
5. In **Aggiungi subtotale a**, seleziona ogni colonna da totalizzare.
6. Lascia selezionato **Sostituisci subtotali correnti** per sostituire i subtotali già presenti.
7. **Riepilogo sotto i dati** inizialmente riflette la struttura del foglio. Lascialo selezionato per mettere ogni riga di totale sotto il suo gruppo, oppure deselezionalo per mettere ogni riga di totale sopra il suo gruppo, con il totale generale in alto. Premi **OK**.

Ogni riga di totale riporta "Totale Est" (oppure la voce a cui si riferisce) e contiene una formula SUBTOTALE, quindi esclude le altre righe di totale. Le righe di ogni gruppo vengono raggruppate, così puoi usare i pulsanti 1, 2 e 3 accanto alle righe per mostrare solo i totali. Per eliminare i totali, apri di nuovo **Subtotale** e premi **Rimuovi tutto**. Se i totali sono finiti dall'altra parte di un foglio che aveva già gruppi propri, la barra di stato segnala che tali gruppi ora hanno le loro righe di riepilogo da quel lato, e Rimuovi tutto riporta il lato com'era prima del primo subtotale, a meno che tu non l'abbia cambiato a mano nel frattempo. Quando i totali sono sopra, i pulsanti della struttura si trovano sulle righe di totale, quindi un pulsante più o meno accanto a "Totale Est" mostra o nasconde le righe di Est sotto di esso. Se deselezioni **Sostituisci subtotali correnti**, i nuovi totali devono stare dallo stesso lato di quelli già presenti. Altrimenti non cambia nulla e un messaggio lo indica. Aggiungere o rimuovere subtotali è un solo passaggio che puoi annullare.

## Filtro avanzato

I pulsanti di filtro scelgono le righe una colonna alla volta. **Avanzato** (nel gruppo **Ordina e filtra**) consente di scrivere le condizioni nel foglio, così possono essere lunghe quanto serve.

1. Accanto all'elenco, copia le intestazioni delle colonne da verificare in una riga di celle vuote.
2. Sotto ogni intestazione, scrivi una condizione. Le condizioni nella **stessa riga** devono essere tutte vere (questo è "e"). Le condizioni in **righe diverse** sono alternative: una riga dell'elenco deve soddisfarne solo una di quelle righe (questo è "oppure").
3. Scegli **Avanzato**. Verifica che **Intervallo elenco** sia l'elenco con la sua riga di intestazione, e digita nella casella **Intervallo condizioni** le celle delle intestazioni e delle condizioni.
4. Scegli **Nascondi le altre righe nell'elenco**, oppure **Copia le righe in un'altra posizione** e digita la prima cella della posizione, come H1. Seleziona **Escludi le righe che ripetono una precedente** per copiarne solo una per ciascun gruppo.
5. Premi **OK**.

Come scrivere una condizione:

- Una parola, come **An**, seleziona il testo che inizia con quella parola. Lettere maiuscole e minuscole vengono considerate uguali. **?** indica un qualsiasi carattere singolo e ***** una qualsiasi sequenza di caratteri.
- Un numero seleziona le celle uguali a esso.
- **=Ann** seleziona il testo esattamente uguale a Ann. **=** da solo seleziona le celle vuote.
- **>**, **>=**, **<**, **<=** e **<>** (diverso) prima di un numero o di una parola confrontano con esso, come **>25**. **<>** da solo seleziona le celle non vuote.

Una riga di condizioni vuota seleziona tutte le righe. Per mostrare di nuovo le righe nascoste, scegli **Cancella**. Una copia viene creata subito e non cambia quando cambia l'elenco. Filtro avanzato è un solo passaggio che puoi annullare.

## Convalida dati

Usalo per stabilire cosa si può digitare in alcune celle, così che un inserimento errato venga bloccato o messo in dubbio.

1. Seleziona le celle.
2. Scegli **Convalida dati** nel gruppo **Strumenti dati** della scheda **Dati**.
3. Nella scheda **Impostazioni**, scegli cosa **Consenti**: **Qualsiasi valore** (nessuna regola), **Numero intero**, **Decimale**, **Elenco**, **Data**, **Ora**, **Lunghezza testo** oppure **Personalizzata**.
4. Per un numero, una data, un orario o una lunghezza del testo, scegli **Dati** (**compreso tra**, **non compreso tra**, **uguale a** e così via) e digita i limiti. Un limite può essere un valore oppure una formula che inizia con =.
5. Per un **Elenco**, digita le scelte separate da virgole, come Rosso,Verde,Blu, oppure digita dopo = un intervallo di celle, come =$A$1:$A$5. La cella mostra allora una freccia con le scelte. Disattiva **Elenco a discesa nella cella** per nascondere la freccia.
6. Per **Personalizzata**, digita una formula che inizia con =. L'inserimento è consentito quando la formula è vera.
7. **Ignora spazi vuoti** consente il passaggio di una cella vuota.

Nella scheda **Messaggio di input**, seleziona la casella e scrivi un titolo e un messaggio. Compaiono accanto alla cella ogni volta che è selezionata.

Nella scheda **Avviso di errore**, scegli cosa accade a un inserimento non consentito. **Arresta** lo rifiuta. **Avviso** chiede se mantenerlo. **Informazioni** avvisa l'utente e lo mantiene. Scrivi un titolo e un messaggio per l'avviso.

**Cerchia dati non validi** (accanto a **Convalida dati**) disegna un ovale rosso intorno a ogni cella del foglio il cui valore non è consentito dalla sua regola, come un numero che c'era prima della regola, o il risultato di una formula. Un messaggio indica se ogni cella è corretta. Una cella che correggi perde il suo ovale. **Cancella cerchi di convalida** rimuove tutti gli ovali.

**Cancella tutto** elimina le regole dalle celle selezionate. La nuova regola sostituisce quelle che le celle selezionate avevano; le altre celle mantengono le proprie. La modifica delle regole è un solo passaggio che puoi annullare, e le regole sono salvate nel file.

## Consolida

Usalo per combinare intervalli simili, come la stessa tabella presente in più fogli, in un'unica tabella.

1. Scegli **Consolida** nel gruppo **Strumenti dati** della scheda **Dati**.
2. Scegli la **Funzione** con cui combinare, come **Somma** o **Media**.
3. Digita un intervallo nella casella e premi **Aggiungi**. Usa **Foglio2!A1:C10** per un intervallo su un altro foglio. Aggiungi ogni intervallo che vuoi combinare. Seleziona un intervallo nell'elenco e premi **Elimina** per toglierlo.
4. Se gli intervalli hanno dei nomi nella prima riga, seleziona **La riga superiore contiene i nomi delle colonne**. Se hanno dei nomi nella prima colonna, seleziona **La colonna sinistra contiene i nomi delle righe**.
5. In **Inserisci il risultato in**, digita la prima cella del risultato.
6. Seleziona **Crea collegamenti ai dati di origine** se ogni risultato deve essere una formula che legge le sue celle di origine, così cambia quando cambiano quelle. Senza questa opzione, i risultati sono semplici numeri.
7. Premi **OK**.

Se non è selezionato nessun nome, viene combinata la cella nella stessa posizione in ogni intervallo (la prima cella di ogni intervallo, poi la seconda, e così via). Se sono selezionati i nomi, vengono combinate le celle con gli stessi nomi, ovunque si trovino in ciascun intervallo, e un nome presente solo in alcuni intervalli ottiene comunque una riga o una colonna. Consolida è un solo passaggio che puoi annullare.

## Foglio di previsione

Foglio di previsione crea un nuovo foglio con una tabella e un grafico di valori futuri, calcolati da una sequenza temporale e da una serie di valori. Individua la tendenza e la stagionalità ricorrente dei valori, e aggiunge una fascia entro la quale i valori futuri probabilmente rimarranno.

1. Seleziona la sequenza temporale e i valori, due colonne affiancate (prima la sequenza temporale). La sequenza temporale è costituita da date o numeri a intervalli uguali, come ogni mese o ogni 5 unità. La prima riga di nomi viene esclusa.
2. Scegli **Foglio di previsione** nel gruppo **Previsione** della scheda **Dati**.
3. Un'immagine mostra la previsione e cambia quando modifichi le scelte. Scegli **Grafico a linee** oppure **Grafico a colonne**.
4. Imposta le scelte:
   - **Intervallo sequenza temporale** e **Intervallo valori** sono le celle da leggere. Scrivi qui altre celle se la selezione era sbagliata.
   - **Inizio previsione** è vuoto per iniziare dopo l'ultimo punto. Una data o un numero prima dell'ultimo punto fa iniziare la previsione da lì e usa solo i punti precedenti, così puoi confrontare la previsione con quanto è accaduto.
   - **Fine previsione** è l'ultimo momento da prevedere. Al massimo vengono previsti 10.000 punti.
   - **Intervallo di confidenza** indica quanto è affidabile la fascia, inizialmente 95%.
   - **Stagionalità** viene individuata osservando i valori, oppure puoi impostare la lunghezza della stagione in punti, oppure scegliere nessuna. I valori devono contenere due stagioni complete.
   - **Riempi i punti mancanti con** indica il valore assegnato a un momento privo di valore: una linea retta tra i suoi vicini, oppure zero.
   - **Aggrega i duplicati usando** indica come vengono combinati i valori allo stesso momento: Media, Conta, CountA, Max, Mediana, Min oppure Somma.
   - **Includi statistiche di previsione** aggiunge una tabella con Alfa, Beta, Gamma, MASE, SMAPE, MAE e RMSE, che indicano quanto bene la previsione si adatta ai valori.
5. Se qualcosa non va, la finestra lo spiega con parole semplici (passaggi non uguali, pochi punti, testo nei valori), e **Crea** resta disattivato.
6. Premi **Crea**.

Il nuovo foglio viene inserito prima di quello su cui ti trovavi. La sua tabella ha le colonne Sequenza temporale, Valori, Previsione, Limite di confidenza inferiore e Limite di confidenza superiore. La previsione e i limiti sono formule che usano `FORECAST.ETS` e `FORECAST.ETS.CONFINT` sulle colonne Sequenza temporale e Valori della tabella stessa, quindi vengono ricalcolate quando cambi un valore. Il grafico mostra i valori, la previsione e la fascia. Foglio di previsione è un solo passaggio che puoi annullare.

I numeri possono differire di qualche punto percentuale da quelli di altri programmi di fogli di calcolo, che usano un proprio metodo per adattare il lisciamento; una previsione che un file contiene già mantiene i valori memorizzati nel file finché non cambiano i dati da cui è calcolata.

Un file con più di 1.000.000 di punti non viene utilizzato.
