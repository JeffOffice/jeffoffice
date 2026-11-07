# Strumenti dati
<!-- keywords: gestione scenari, scenari, what-if, e se, riepilogo scenario, dati, scheda dati, testo in colonne, dividi, separatore, virgola, tabulazione, larghezza fissa, rimuovi duplicati, ricerca obiettivo, subtotale, totali, filtro avanzato, condizioni, convalida dati, cerchia dati non validi, ovale rosso, cancella cerchi di convalida, consolida, unisci intervalli, consenti, elenco, discesa, messaggio di input, avviso di errore, righe ripetute, univoci, file di testo, csv, da testo, colonne, delimitatore, virgolette, modifica collegamenti, aggiorna valori, cambia origine, collegamenti, esterno, origine, interrompi collegamento, verifica stato, apri origine -->

La scheda **Dati** contiene gli strumenti che agiscono su un elenco di valori: ottenere testo da un file, ordinare un elenco, dividere il testo in colonne ed eliminare le righe ripetute.

## Da testo/CSV

**Da testo/CSV** apre un file di testo (.csv, .tsv o .txt). Funziona come **Apri file di testo** nella scheda File. JeffCrunch chiede come è suddiviso il file in colonne e che tipo di testo contiene, quindi lo mostra in una nuova cartella di lavoro.

## Modifica collegamenti

Se una cartella di lavoro contiene formule che leggono un'altra cartella di lavoro (come ='[Prezzi.xlsx]Foglio1'!A1), il comando **Modifica collegamenti** nel gruppo **Recupera dati** elenca le altre cartelle di lavoro, dette origini. Ogni riga indica dove si trova l'origine e il suo stato. Finché la cartella di lavoro non viene riaperta con le sue origini, mostra i numeri memorizzati nel file.

- **Verifica stato** cerca ogni origine su questo computer e indica **Trovata**, oppure che non è stata trovata. Un'origine sul Web non viene recuperata, quindi il suo stato indica che non è stata verificata.
- **Aggiorna valori** legge dal computer l'origine scelta e assegna a ogni cella che è solo un riferimento a una delle sue celle il valore attuale di quella cella. Una cella che elabora ulteriormente il valore, come `=[1]Prezzi!A1*2`, mantiene il numero memorizzato nel file. Il messaggio sotto l'elenco indica quante celle sono state aggiornate e quante no. Aggiorna valori è un'unica operazione, quindi **Annulla** ripristina i valori precedenti.
- **Cambia origine** consente di scegliere un altro file per il collegamento selezionato, ad esempio quando la cartella di lavoro è stata spostata. Le formule restano invariate. Il nuovo percorso viene scritto nel file al salvataggio, e **Annulla** annulla la modifica.
- **Apri origine** apre l'origine selezionata in una nuova finestra, quando può essere trovata.
- **Interrompi collegamento** trasforma ogni cella che legge l'origine selezionata nel valore che mostra ora, e un nome che indicava una posizione in essa in **#RIF!**. L'operazione non può essere annullata, quindi JeffCrunch chiede conferma. Quando non resta alcun collegamento, il file viene salvato senza le parti che li contenevano.

## Ordina e filtra

Il gruppo **Ordina e filtra** contiene gli stessi comandi del menu **Ordina e filtra** nella scheda Home. Vedi [Ordina e filtra](sort-filter).

## Testo in colonne

Usa questo comando quando una colonna contiene più parti di testo che vuoi separare in colonne diverse, come "Rossi, Anna, 31".

1. Seleziona le celle di **una sola** colonna.
2. Scegli **Testo in colonne** nella scheda **Dati**.
3. Scegli come dividere il testo:
   - **Dividi in corrispondenza di un separatore**: seleziona i caratteri che separano le parti: **Tabulazione**, **Punto e virgola**, **Virgola**, **Spazio** oppure **Altro**, dove digiti il carattere desiderato. **Considera separatori consecutivi come uno solo** evita che due virgole di seguito creino una parte vuota. **Testo tra virgolette** indica il carattere che racchiude una parte da non dividere, come "Lee, Bo". Tra le virgolette, due caratteri uguali di seguito valgono come uno.
   - **Dividi in posizioni fisse**: digita le posizioni in cui inizia una nuova colonna, contando i caratteri da 1. Ad esempio, 5, 12 fa iniziare una nuova colonna al quinto e al dodicesimo carattere.
4. Scegli il tipo di ogni parte: **Generale** legge numeri e date come se li avessi digitati, e **Testo** mantiene ogni parte così com'è, quindi "007" resta "007".
5. In **Inserisci le parti a partire da**, digita la prima cella per le parti. Inizialmente corrisponde alla prima cella selezionata.
6. La casella sotto le opzioni mostra le prime righe divise come hai scelto. Premi **OK**.

Le parti vengono inserite nelle celle a destra della prima cella e sostituiscono il contenuto che vi si trovava. Testo in colonne è un'unica operazione che puoi annullare.

## Rimuovi duplicati

Usa questo comando per conservare solo la prima di ogni gruppo di righe ripetute.

1. Seleziona le celle dell'elenco oppure una cella al suo interno. JeffCrunch considera tutto il blocco di dati intorno alla cella.
2. Scegli **Rimuovi duplicati**.
3. Seleziona **I miei dati hanno una riga di intestazione** se la prima riga contiene i nomi. Quella riga resta e non viene confrontata.
4. Seleziona le colonne che devono coincidere perché due righe siano considerate ripetute. Due righe si ripetono solo se tutte le colonne selezionate sono uguali. Le maiuscole e le minuscole sono considerate uguali, e una formula conta come il valore che restituisce.
5. Premi **OK**. La finestra indica quante righe sono state rimosse e quante ne restano.

Le righe sotto una riga eliminata salgono all'interno delle celle selezionate, con i rispettivi formati e formule. Le celle sotto la selezione non si spostano. Le righe vuote che restano in fondo alla selezione vengono cancellate. Rimuovi duplicati è un'unica operazione che puoi annullare.

## Ricerca obiettivo

Usa questo comando quando conosci il risultato che vuoi ma non il numero che lo produce. Ad esempio, una cella della rata contiene una formula che usa un tasso di interesse, e vuoi che la rata sia 500. Ricerca obiettivo prova dei numeri nella cella del tasso finché la rata non è 500.

1. Scegli **Ricerca obiettivo** nella scheda **Dati**.
2. In **Cella con la formula**, digita la cella che contiene la formula, ad esempio B3. Inizialmente corrisponde alla cella selezionata.
3. In **Valore desiderato**, digita il risultato che la formula deve restituire.
4. In **Cella da modificare**, digita la cella che Ricerca obiettivo può cambiare, ad esempio B1. Deve contenere un numero, non una formula.
5. Premi **OK**.

Se Ricerca obiettivo trova un numero, questo resta nella cella da modificare e la barra di stato indica quale è. Se non lo trova, non cambia nulla e viene segnalato. Prova un valore iniziale diverso nella cella da modificare e verifica che la formula utilizzi davvero quella cella. Ricerca obiettivo è un'unica operazione che puoi annullare.

## Gestione scenari

Uno scenario è un insieme di valori con nome per alcune celle. Usa gli scenari per provare ipotesi "what if" in un solo foglio. Ad esempio, conserva uno scenario "Vendite basse" e uno "Vendite alte" per le stesse celle di input, e mostra quello che vuoi esaminare.

1. Scegli **Gestione scenari** nella scheda **Dati**.
2. Premi **Aggiungi**. Digita un nome, ad esempio Vendite alte. In **Celle modificate**, digita le celle che lo scenario imposta, ad esempio B1:B3 oppure D5. Uno scenario può modificare fino a 32 celle. Le celle selezionate vengono compilate automaticamente. Ogni cella parte dal valore che contiene ora. Digita nella tabella il valore che questo scenario deve usare e premi **OK**.
3. Scegli uno scenario nell'elenco e premi **Mostra** per inserire i suoi valori nelle celle. Anche fare doppio clic sul nome ha lo stesso effetto. Premi **Modifica** per cambiarne il nome, il commento o i valori. Premi **Elimina** per rimuoverlo.

Mostra, Aggiungi, Modifica ed Elimina sono ciascuna un'operazione che puoi annullare.

**Riepilogo** affianca tutti gli scenari in un nuovo foglio chiamato Riepilogo scenario. Chiede le celle risultato, cioè le celle che vuoi confrontare, come il totale in B10. Per ogni scenario, il riepilogo mostra i valori delle celle modificate e il risultato delle celle risultato, calcolati con i valori di quello scenario. Una colonna chiamata Valori correnti mostra come erano le celle al momento del riepilogo. Le tue celle restano come erano.

Gli scenari sono salvati nella cartella di lavoro. Una cartella di lavoro creata con un altro programma conserva i suoi scenari, che vedi qui. Una copia salvata come file .xls meno recente o come .ods non li contiene, e il salvataggio te lo segnala.

## Subtotale

Usa questo comando per inserire una riga di totali sotto ogni gruppo di righe che hanno lo stesso valore in una colonna, e un totale generale in fondo. L'elenco deve avere una riga di intestazione e deve essere ordinato in base alla colonna per cui raggruppi, in modo che i valori uguali siano vicini.

1. Seleziona una cella dell'elenco.
2. Scegli **Subtotale** nel gruppo **Struttura** della scheda **Dati**.
3. In **A ogni variazione di**, scegli la colonna che raggruppa le righe, ad esempio Regione.
4. In **Usa funzione**, scegli il calcolo da eseguire: **Somma**, **Conteggio**, **Media**, **Max**, **Min** e altri.
5. In **Aggiungi subtotale a**, seleziona ogni colonna da totalizzare.
6. Lascia selezionata l'opzione **Sostituisci subtotali correnti** per sostituire i subtotali già presenti. Premi **OK**.

Ogni riga di totale riporta "Totale Est" (o la voce a cui si riferisce) e contiene una formula SUBTOTALE, quindi non include le altre righe di totale. Le righe di ciascun gruppo vengono raggruppate, così puoi usare i pulsanti 1, 2 e 3 accanto alle righe per mostrare solo i totali. Per eliminare i totali, apri di nuovo **Subtotale** e premi **Rimuovi tutti**. Aggiungere o rimuovere subtotali è un'unica operazione che puoi annullare.

## Filtro avanzato

I pulsanti di filtro scelgono le righe una colonna alla volta. **Avanzato** (nel gruppo **Ordina e filtra**) consente di scrivere le condizioni sul foglio, anche molto articolate.

1. Accanto all'elenco, copia le intestazioni delle colonne da verificare in una riga di celle vuote.
2. Sotto ogni intestazione, scrivi una condizione. Le condizioni nella **stessa riga** devono essere tutte vere (è una "e"). Le condizioni in **righe diverse** sono alternative: una riga dell'elenco deve soddisfare solo una di quelle righe (è una "o").
3. Scegli **Avanzato**. Verifica che **Intervallo elenco** sia l'elenco con la sua riga di intestazione, e digita nelle **Intervallo condizioni** le celle delle intestazioni e delle condizioni.
4. Scegli **Nascondi le altre righe nell'elenco**, oppure **Copia le righe in un'altra posizione** e digita la prima cella della destinazione, ad esempio H1. Seleziona **Escludi le righe che ripetono una precedente** per copiarne una sola per ciascuna.
5. Premi **OK**.

Come scrivere una condizione:

- Una parola, come **An**, seleziona il testo che inizia con quella parola. Le maiuscole e le minuscole sono considerate uguali. **?** rappresenta un qualsiasi carattere e ***** una sequenza qualsiasi di caratteri.
- Un numero seleziona le celle uguali ad esso.
- **=Ann** seleziona il testo esattamente uguale a Ann. **=** da solo seleziona le celle vuote.
- **>**, **>=**, **<**, **<=** e **<>** (diverso) davanti a un numero o a una parola li confrontano, ad esempio **>25**. **<>** da solo seleziona le celle non vuote.

Una riga di condizioni senza contenuto seleziona tutte le righe. Per mostrare di nuovo le righe nascoste, scegli **Cancella**. Una copia viene creata subito e non cambia quando cambia l'elenco. Filtro avanzato è un'unica operazione che puoi annullare.

## Convalida dati

Usa questo comando per stabilire cosa può essere digitato in alcune celle, così che un valore non consentito venga bloccato o messo in discussione.

1. Seleziona le celle.
2. Scegli **Convalida dati** nel gruppo **Strumenti dati** della scheda **Dati**.
3. Nella scheda **Impostazioni**, scegli cosa **Consenti**: **Qualsiasi valore** (nessuna regola), **Numero intero**, **Decimale**, **Elenco**, **Data**, **Ora**, **Lunghezza testo** oppure **Personalizzata**.
4. Per un numero, una data, un'ora o una lunghezza del testo, scegli **Dati** (**compreso tra**, **non compreso tra**, **uguale a** e così via) e digita i limiti. Un limite può essere un valore o una formula che inizia con =.
5. Per un **Elenco**, digita le scelte separate da virgole, ad esempio Rosso,Verde,Blu, oppure digita un intervallo di celle dopo =, ad esempio =$A$1:$A$5. La cella mostra allora una freccia con le scelte. Disattiva **Menu a discesa nella cella** per nascondere la freccia.
6. Per **Personalizzata**, digita una formula che inizia con =. Il valore è consentito quando la formula è vera.
7. **Ignora spazi vuoti** consente il passaggio di una cella vuota.

Nella scheda **Messaggio di input**, seleziona la casella e scrivi un titolo e un messaggio. Compaiono accanto alla cella ogni volta che viene selezionata.

Nella scheda **Avviso di errore**, scegli cosa succede a un valore non consentito. **Arresta** lo rifiuta. **Avviso** chiede se mantenerlo. **Informazioni** segnala la situazione e lo mantiene. Scrivi un titolo e un messaggio per l'avviso.

**Cerchia dati non validi** (accanto a **Convalida dati**) disegna un ovale rosso intorno a ogni cella del foglio il cui valore non è consentito dalla regola, come un numero inserito prima della regola o il risultato di una formula. Un messaggio indica se tutte le celle sono corrette. Una cella che correggi perde il suo ovale. **Cancella cerchi di convalida** elimina tutti gli ovali.

**Cancella tutto** rimuove le regole dalle celle selezionate. La nuova regola sostituisce le regole che le celle selezionate avevano; le altre celle mantengono le proprie. Modificare le regole è un'unica operazione che puoi annullare, e le regole vengono salvate nel file.

## Consolida

Usa questo comando per unire in un'unica tabella intervalli simili, come la stessa tabella presente in più fogli.

1. Scegli **Consolida** nel gruppo **Strumenti dati** della scheda **Dati**.
2. Scegli la **Funzione** da usare per la combinazione, come **Somma** o **Media**.
3. Digita un intervallo nella casella e premi **Aggiungi**. Usa **Foglio2!A1:C10** per un intervallo su un altro foglio. Aggiungi ogni intervallo da unire. Seleziona un intervallo nell'elenco e premi **Elimina** per toglierlo.
4. Se gli intervalli hanno nomi nella prima riga, seleziona **Riga superiore contiene i nomi delle colonne**. Se hanno nomi nella prima colonna, seleziona **Colonna sinistra contiene i nomi delle righe**.
5. In **Inserisci il risultato in**, digita la prima cella per il risultato.
6. Seleziona **Crea collegamenti ai dati di origine** se ogni risultato deve essere una formula che legge le celle di origine, così da cambiare quando cambiano quelle. Senza questa opzione, i risultati sono semplici numeri.
7. Premi **OK**.

Senza nomi selezionati, viene combinata la cella che si trova nella stessa posizione in ogni intervallo (la prima cella di ogni intervallo, poi la seconda, e così via). Con i nomi selezionati, vengono combinate le celle con lo stesso nome, ovunque si trovino in ciascun intervallo, e un nome presente solo in alcuni intervalli ottiene comunque una riga o una colonna. Consolida è un'unica operazione che puoi annullare.
