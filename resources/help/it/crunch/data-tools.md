# Strumenti dati
<!-- keywords: gestione scenari, scenari, what-if, che succede se, riepilogo scenari, dati, scheda dati, testo in colonne, dividi, separatore, virgola, tabulazione, larghezza fissa, rimuovi duplicati, ricerca obiettivo, subtotale, totali, filtro avanzato, condizioni, convalida dati, cerchia dati non validi, ovale rosso, cancella cerchi di convalida, consolida, unisci intervalli, consenti, elenco, discesa, messaggio di input, avviso di errore, righe ripetute, univoci, file di testo, csv, da testo, colonne, delimitatore, virgolette, modifica collegamenti, collegamenti, esterni, origine, interrompi collegamento, verifica stato, apri origine -->

La scheda **Dati** contiene gli strumenti che lavorano su un elenco di valori: recuperare testo da un file, ordinare un elenco, suddividere il testo in colonne ed eliminare le righe ripetute.

## Da testo/CSV

**Da testo/CSV** apre un file di testo (.csv, .tsv o .txt). Funziona come **Apri file di testo** nella scheda **File**. JeffCrunch chiede come è suddiviso il file in colonne e che tipo di testo contiene, quindi lo mostra in una nuova cartella di lavoro.

## Modifica collegamenti

Se una cartella di lavoro contiene formule che leggono un'altra cartella di lavoro (come ='[Prezzi.xlsx]Foglio1'!A1), **Modifica collegamenti** nel gruppo **Recupera dati** elenca tali cartelle, dette origini. Ogni riga indica dove si trova l'origine e il suo stato. Finché la cartella di lavoro non viene aperta di nuovo con le sue origini, mostra i numeri che il suo file ha memorizzato da esse.

- **Verifica stato** cerca ogni origine su questo computer e indica **Trovata** oppure che non è stata trovata. Un'origine sul Web non viene scaricata, quindi il suo stato indica che non è stata verificata.
- **Apri origine** apre l'origine scelta in una nuova finestra, quando può essere trovata.
- **Interrompi collegamento** trasforma ogni cella che legge l'origine scelta nel valore che mostra in quel momento, e un nome che indicava una posizione al suo interno in **#RIF!**. L'operazione non può essere annullata, quindi JeffCrunch chiede conferma. Quando non rimane alcun collegamento, il file viene salvato senza le parti che li contenevano.

## Ordina e filtra

Il gruppo **Ordina e filtra** contiene gli stessi comandi del menu **Ordina e filtra** nella scheda Home. Vedi [Ordina e filtra](sort-filter).

## Testo in colonne

Usa questa funzione quando una colonna contiene più parti di testo che vuoi trovare in colonne separate, come "Rossi, Anna, 31".

1. Seleziona le celle di **una** colonna.
2. Scegli **Testo in colonne** nella scheda **Dati**.
3. Scegli come dividere il testo:
   - **Dividi in corrispondenza di un separatore**: seleziona i caratteri che separano le parti: **Tabulazione**, **Punto e virgola**, **Virgola**, **Spazio** oppure **Altro**, dove digiti il carattere desiderato. **Considera separatori consecutivi come uno solo** evita che due virgole di fila creino una parte vuota. **Testo tra virgolette** indica il carattere che racchiude una parte da non dividere, come "Lee, Bo". All'interno delle virgolette, due caratteri di seguito valgono come uno solo.
   - **Dividi in posizioni fisse**: digita le posizioni in cui inizia una nuova colonna, contando i caratteri dal numero 1. Ad esempio, 5, 12 inizia una nuova colonna al quinto e al dodicesimo carattere.
4. Scegli il tipo di ogni parte: **Generale** interpreta numeri e date come se fossero stati digitati, mentre **Testo** mantiene ogni parte così com'è, quindi "007" resta "007".
5. In **Inserisci le parti a partire da**, digita la prima cella per le parti. Inizialmente corrisponde alla prima cella selezionata.
6. La casella sotto le scelte mostra le prime righe divise secondo le opzioni scelte. Premi **OK**.

Le parti vengono inserite nelle celle a destra della prima cella e sostituiscono il contenuto precedente. Testo in colonne è un'operazione che puoi annullare con un solo passaggio.

## Rimuovi duplicati

Usa questa funzione per mantenere solo la prima di ogni gruppo di righe ripetute.

1. Seleziona le celle dell'elenco oppure una cella al suo interno. JeffCrunch considera l'intero blocco di dati intorno alla cella.
2. Scegli **Rimuovi duplicati**.
3. Seleziona **I miei dati hanno intestazioni** se la prima riga contiene i nomi. Quella riga resta e non viene confrontata.
4. Seleziona le colonne che devono coincidere perché due righe siano considerate ripetute. Due righe sono ripetute solo se tutte le colonne selezionate sono uguali. Lettere maiuscole e minuscole sono considerate uguali, e una formula conta come il valore che restituisce.
5. Premi **OK**. La finestra indica quante righe sono state rimosse e quante ne restano.

Le righe sotto una riga rimossa salgono all'interno delle celle selezionate, con i rispettivi formati e formule. Le celle sotto la selezione non si spostano. Le righe vuote rimaste in fondo alla selezione vengono svuotate. Rimuovi duplicati è un'operazione che puoi annullare con un solo passaggio.

## Ricerca obiettivo

Usa questa funzione quando conosci il risultato che vuoi ma non il numero che lo produce. Ad esempio, una cella di pagamento contiene una formula che usa un tasso di interesse, e vuoi che il pagamento sia 500. Ricerca obiettivo prova dei numeri nella cella del tasso finché il pagamento non è 500.

1. Scegli **Ricerca obiettivo** nella scheda **Dati**.
2. In **Cella con la formula**, digita la cella che contiene la formula, come B3. Inizialmente corrisponde alla cella selezionata.
3. In **Valore desiderato**, digita il risultato che vuoi ottenere dalla formula.
4. In **Cella da modificare**, digita la cella che Ricerca obiettivo può modificare, come B1. Deve contenere un numero, non una formula.
5. Premi **OK**.

Se Ricerca obiettivo trova un numero, questo resta nella cella da modificare e la barra di stato indica quale sia. Se non riesce, non cambia nulla e viene segnalato. Prova con un numero iniziale diverso nella cella da modificare e verifica che la formula usi davvero quella cella. Ricerca obiettivo è un'operazione che puoi annullare con un solo passaggio.

## Gestione scenari

Uno scenario è un insieme con nome di valori per alcune celle. Usa gli scenari per provare ipotesi "what if" su un unico foglio. Ad esempio, puoi conservare uno scenario "Vendite basse" e uno "Vendite alte" per le stesse celle di input, e mostrare quello che vuoi esaminare.

1. Scegli **Gestione scenari** nella scheda **Dati**.
2. Premi **Aggiungi**. Digita un nome, come Vendite alte. In **Celle da modificare**, digita le celle che lo scenario imposta, come B1:B3 oppure D5. Uno scenario può modificare fino a 32 celle. Le celle selezionate vengono compilate automaticamente. Ogni cella parte dal valore che contiene in questo momento. Digita nella tabella il valore che questo scenario deve usare e premi **OK**.
3. Scegli uno scenario nell'elenco e premi **Mostra** per inserire i suoi valori nelle celle. Anche fare doppio clic sul nome ha lo stesso effetto. Premi **Modifica** per cambiarne il nome, il commento o i valori. Premi **Elimina** per rimuoverlo.

Mostra, Aggiungi, Modifica ed Elimina sono operazioni che puoi annullare, ciascuna con un solo passaggio.

**Riepilogo** affianca tutti gli scenari in un nuovo foglio chiamato Riepilogo scenari. Chiede le celle del risultato, cioè le celle che vuoi confrontare, come il totale in B10. Per ogni scenario, il riepilogo mostra i valori delle celle modificate e i valori delle celle del risultato, calcolati con i valori di quello scenario. Una colonna chiamata Valori correnti mostra come erano le celle quando hai creato il riepilogo. Le tue celle restano come erano.

Gli scenari sono salvati nella cartella di lavoro. Una cartella di lavoro creata con un altro programma conserva i suoi scenari, che qui sono visibili. Una copia salvata come file .xls meno recente o come .ods non li conserva, e il salvataggio lo segnala.

## Subtotale

Usa questa funzione per inserire una riga di totali sotto ogni gruppo di righe che hanno lo stesso valore in una colonna, e un totale generale in fondo. L'elenco deve avere una riga di intestazione e deve essere ordinato in base alla colonna di raggruppamento, così che i valori uguali siano vicini.

1. Seleziona una cella dell'elenco.
2. Scegli **Subtotale** nel gruppo **Struttura** della scheda **Dati**.
3. In **Ad ogni variazione di**, scegli la colonna che raggruppa le righe, come Regione.
4. In **Usa funzione**, scegli il calcolo da eseguire: **Somma**, **Conteggio**, **Media**, **Max**, **Min** e altri.
5. In **Aggiungi subtotale a**, seleziona ogni colonna da totalizzare.
6. Lascia selezionato **Sostituisci subtotali correnti** per sostituire i subtotali già presenti. Premi **OK**.

Ogni riga di totale riporta "Totale Est" (o il valore a cui si riferisce) e contiene una formula SUBTOTALE, quindi ignora le altre righe di totale. Le righe di ogni gruppo sono raggruppate, quindi puoi usare i pulsanti 1, 2 e 3 accanto alle righe per mostrare solo i totali. Per eliminare i totali, apri di nuovo **Subtotale** e premi **Rimuovi tutto**. Aggiungere o rimuovere subtotali è un'operazione che puoi annullare con un solo passaggio.

## Filtro avanzato

I pulsanti di filtro scelgono le righe una colonna alla volta. **Avanzato** (nel gruppo **Ordina e filtra**) ti permette di scrivere le condizioni sul foglio, quindi possono essere lunghe quanto serve.

1. Accanto all'elenco, copia le intestazioni delle colonne da verificare in una riga di celle vuote.
2. Sotto ogni intestazione, scrivi una condizione. Le condizioni nella **stessa riga** devono essere tutte vere (questo è "e"). Le condizioni in **righe diverse** sono alternative: una riga dell'elenco deve soddisfare solo una di quelle righe (questo è "o").
3. Scegli **Avanzato**. Verifica che **Intervallo elenco** sia l'elenco con la sua riga di intestazione, e digita nelle **Intervallo condizioni** le celle delle intestazioni e delle condizioni.
4. Seleziona **Nascondi le altre righe nell'elenco**, oppure **Copia le righe in un'altra posizione** e digita la prima cella della destinazione, come H1. Seleziona **Escludi le righe che ripetono una precedente** per copiarne solo una per ogni valore.
5. Premi **OK**.

Come scrivere una condizione:

- Una parola, come **An**, seleziona il testo che inizia con quella parola. Lettere maiuscole e minuscole sono considerate uguali. **?** rappresenta un qualsiasi carattere singolo e ***** una sequenza qualsiasi di caratteri.
- Un numero seleziona le celle uguali ad esso.
- **=Ann** seleziona il testo esattamente uguale a Ann. **=** da solo seleziona le celle vuote.
- **>**, **>=**, **<**, **<=** e **<>** (diverso) prima di un numero o di una parola li confrontano, come **>25**. **<>** da solo seleziona le celle non vuote.

Una riga di condizioni vuota seleziona tutte le righe. Per mostrare di nuovo le righe nascoste, scegli **Cancella**. La copia viene creata subito e non cambia quando cambia l'elenco. Filtro avanzato è un'operazione che puoi annullare con un solo passaggio.

## Convalida dati

Usa questa funzione per stabilire che cosa si può digitare in alcune celle, in modo che un inserimento errato venga bloccato o messo in discussione.

1. Seleziona le celle.
2. Scegli **Convalida dati** nel gruppo **Strumenti dati** della scheda **Dati**.
3. Nella scheda **Impostazioni**, scegli cosa **Consenti**: **Qualsiasi valore** (nessuna regola), **Numero intero**, **Decimale**, **Elenco**, **Data**, **Ora**, **Lunghezza testo** oppure **Personalizzata**.
4. Per un numero, una data, un'ora o una lunghezza del testo, scegli **Dati** (**compreso tra**, **non compreso tra**, **uguale a** e così via) e digita i limiti. Un limite può essere un valore oppure una formula che inizia con =.
5. Per un **Elenco**, digita le scelte separate da punti e virgola, come Rosso;Verde;Blu, oppure digita un intervallo di celle dopo =, come =$A$1:$A$5. La cella mostrerà poi una freccia con le scelte. Disattiva **Elenco a discesa nella cella** per nascondere la freccia.
6. Per **Personalizzata**, digita una formula che inizia con =. L'inserimento è consentito quando la formula è vera.
7. **Ignora vuote** consente il passaggio di una cella vuota.

Nella scheda **Messaggio di input**, seleziona la casella e scrivi un titolo e un messaggio. Compaiono accanto alla cella ogni volta che è selezionata.

Nella scheda **Avviso di errore**, scegli cosa succede a un inserimento non consentito. **Arresta** lo rifiuta. **Avviso** chiede se mantenerlo. **Informazioni** avvisa la persona e lo mantiene. Scrivi un titolo e un messaggio per l'avviso.

**Cerchia dati non validi** (accanto a **Convalida dati**) inserisce un ovale rosso intorno a ogni cella del foglio il cui valore non è consentito dalla sua regola, come un numero inserito prima della regola o il risultato di una formula. Un messaggio indica se tutte le celle sono corrette. Una cella che correggi perde il suo ovale. **Cancella cerchi di convalida** rimuove tutti gli ovali.

**Cancella tutto** elimina le regole dalle celle selezionate. La nuova regola sostituisce le regole che le celle selezionate avevano; le altre celle mantengono le proprie. Modificare le regole è un'operazione che puoi annullare con un solo passaggio, e le regole vengono salvate nel file.

## Consolida

Usa questa funzione per combinare intervalli simili, come la stessa tabella presente in più fogli, in un'unica tabella.

1. Scegli **Consolida** nel gruppo **Strumenti dati** della scheda **Dati**.
2. Scegli la **Funzione** con cui combinare, come **Somma** o **Media**.
3. Digita un intervallo nella casella e premi **Aggiungi**. Usa **Foglio2!A1:C10** per un intervallo su un altro foglio. Aggiungi ogni intervallo da combinare. Seleziona un intervallo nell'elenco e premi **Elimina** per toglierlo.
4. Se gli intervalli hanno nomi nella prima riga, seleziona **La riga superiore contiene i nomi delle colonne**. Se hanno nomi nella prima colonna, seleziona **La colonna sinistra contiene i nomi delle righe**.
5. In **Inserisci il risultato in**, digita la prima cella del risultato.
6. Seleziona **Crea collegamenti ai dati di origine** se ogni risultato deve essere una formula che legge le celle di origine, in modo da cambiare quando cambiano quelle. Senza questa opzione, i risultati sono numeri semplici.
7. Premi **OK**.

Se non è selezionato nessun nome, viene combinata la cella nella stessa posizione di ogni intervallo (la prima cella di ogni intervallo, poi la seconda, e così via). Con i nomi selezionati, vengono combinate le celle con lo stesso nome, ovunque si trovino in ogni intervallo, e un nome presente solo in alcuni intervalli riceve comunque una riga o una colonna. Consolida è un'operazione che puoi annullare con un solo passaggio.
