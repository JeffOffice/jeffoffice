# Strumenti dati

<!-- keywords: previsione, foglio di previsione, valori futuri, intervallo di confidenza, stagionalità, tendenza, lisciamento, maschera, modulo dati, inserimento dati, record, nuovo record, trova successivo, trova precedente, criteri, gestione scenari, scenari, what-if, ipotesi, riepilogo scenari, dati, scheda dati, testo in colonne, dividi, separatore, virgola, tabulazione, larghezza fissa, rimuovi duplicati, ricerca obiettivo, subtotale, totali, filtro avanzato, condizioni, convalida dati, cerchia dati non validi, ovale rosso, cancella cerchi di convalida, consolida, unisci intervalli, consenti, elenco, discesa, messaggio di input, avviso di errore, righe ripetute, unico, file di testo, csv, da testo, colonne, delimitatore, virgolette, modifica collegamenti, aggiorna valori, cambia origine, collegamenti, esterno, origine, interrompi collegamento, verifica stato, apri origine -->

La scheda **Dati** contiene gli strumenti che lavorano su un elenco di valori: recuperare testo da un file, ordinare un elenco, dividere il testo in colonne ed eliminare le righe ripetute.

## Da Testo/CSV

**Da Testo/CSV** apre un file di testo (.csv, .tsv o .txt). Funziona come **Apri file di testo** nella scheda **File**. JeffCrunch chiede come è suddiviso il file in colonne e che tipo di testo contiene, quindi lo mostra come nuova cartella di lavoro.

## Modifica collegamenti

Se una cartella di lavoro contiene formule che leggono un'altra cartella di lavoro (come ='[Prezzi.xlsx]Foglio1'!A1), **Modifica collegamenti** nel gruppo **Recupera dati** elenca le altre cartelle di lavoro, dette origini. Ogni riga indica dove si trova l'origine e il suo stato. Finché la cartella di lavoro non viene riaperta con le sue origini, mostra i numeri che il suo file ha memorizzato da esse.

- **Verifica stato** cerca ogni origine su questo computer e indica **Trovata**, oppure che non è stata trovata. Un'origine sul Web non viene recuperata, quindi il suo stato indica che non è stata verificata.
- **Aggiorna valori** legge dal computer l'origine scelta e assegna a ogni cella che è solo un riferimento a una delle sue celle il valore che tale cella ha ora. Una cella che fa altri calcoli con il valore, come `=[1]Prezzi!A1*2`, conserva il numero memorizzato nel suo file, e il messaggio sotto l'elenco indica quante celle sono state aggiornate e quante no. Aggiorna valori è un'unica operazione, quindi **Annulla** ripristina i valori precedenti. I nuovi valori vengono inoltre scritti nella parte del file dedicata al collegamento quando salvi, così la cartella di lavoro li mostra alla prossima apertura, anche se l'origine non è presente.
- **Cambia origine** permette di scegliere un altro file per il collegamento selezionato, ad esempio quando la cartella di lavoro è stata spostata. Le formule restano invariate. Il nuovo percorso viene scritto nel file al salvataggio, e **Annulla** annulla la modifica.
- **Apri origine** apre l'origine scelta in una nuova finestra, quando può essere trovata.
- **Interrompi collegamento** trasforma ogni cella che legge l'origine scelta nel valore che mostra ora, e un nome che indicava un punto della cartella di lavoro in **#RIF!**. JeffCrunch chiede conferma prima. Interrompi collegamento è un'unica operazione, quindi **Annulla** ripristina le formule, i nomi e i collegamenti, finché non salvi la cartella di lavoro. Quando non resta alcun collegamento, il file viene salvato senza le parti che li contenevano.

## Maschera

**Maschera** (nel gruppo **Strumenti dati** e nella casella di ricerca) mostra una riga alla volta di un elenco, in una piccola maschera. Un elenco è una tabella, oppure un intervallo di celle con una riga di nomi in alto e una riga di dati sotto ogni nome. Fai prima clic su una cella qualsiasi dell'elenco.

- Ogni casella corrisponde a una colonna ed è etichettata con il nome della riga superiore. Digita in una casella per cambiare il valore. Una casella in grigio contiene una formula: la maschera mostra il risultato, e la formula si modifica nel foglio.
- **Trova successivo** e **Trova precedente** passano alla riga successiva e a quella precedente. Passando a un'altra riga, o premendo **Invio**, quanto digitato viene scritto in questa riga.
- **Nuovo** mostra una maschera vuota. Compilala, quindi premi di nuovo **Nuovo**, **Trova successivo** o **Chiudi**. La riga viene aggiunta sotto l'ultima riga dell'elenco. Una tabella cresce di una riga. Se una cella sotto l'elenco non è vuota, la maschera lo segnala e non aggiunge nulla.
- **Elimina** rimuove la riga che stai visualizzando, dopo aver chiesto conferma. Le righe sottostanti salgono di una posizione. Una tabella mantiene sempre almeno una riga di dati. Una riga nascosta, o nascosta da un filtro, non viene mostrata nella maschera, non viene conteggiata e non viene mai eliminata da essa. Se l'elenco è troppo lungo per essere esaminato interamente, la maschera indica che mostra solo le prime righe.
- **Ripristina** annulla quanto hai digitato in questa riga, prima di lasciarla.
- **Criteri** permette di cercare righe. Digita una condizione in una o più caselle, quindi premi **Trova successivo** o **Trova precedente**. Una condizione è un valore, come **Ann** (il testo corrisponde all'inizio della parola, e **?** e ***** rappresentano lettere), oppure un valore preceduto da un segno: **>100**, **<=5**, **<>0**, **=Oslo**. Una riga deve soddisfare tutte le condizioni digitate. Premi **Maschera** per tornare indietro, oppure **Cancella** per ricominciare. Finché sono impostati dei criteri, **Trova successivo** e **Trova precedente** saltano le righe che non li soddisfano.
- **Chiudi** scrive la riga e chiude la maschera.

Ogni riga che modifichi, aggiungi o elimini costituisce un'unica operazione, quindi **Annulla** nel foglio la ripristina in un solo passaggio.

## Ordina e filtra

Il gruppo **Ordina e filtra** contiene gli stessi comandi del menu **Ordina e filtra** nella scheda Home. Vedi [Ordinare e filtrare](sort-filter).

## Testo in colonne

Usa questo strumento quando una colonna contiene più parti di testo che vuoi avere in colonne separate, come "Rossi, Anna, 31".

1. Seleziona le celle di **una** sola colonna.
2. Scegli **Testo in colonne** nella scheda **Dati**.
3. Scegli come dividere il testo:
   - **Dividi in corrispondenza di un separatore**: seleziona i caratteri che separano le parti: **Tabulazione**, **Punto e virgola**, **Virgola**, **Spazio** oppure **Altro**, dove digiti il carattere desiderato. **Considera separatori consecutivi come uno solo** evita che due virgole di seguito creino una parte vuota. **Delimitatore di testo** indica il carattere che racchiude una parte che non deve essere divisa, come "Rossi, Bo". All'interno delle virgolette, due caratteri uguali di seguito valgono per uno solo.
   - **Dividi in posizioni fisse**: digita le posizioni in cui inizia una nuova colonna, contando i caratteri da 1. Ad esempio, 5, 12 fa iniziare una nuova colonna al quinto e al dodicesimo carattere.
4. Scegli il tipo di ogni parte: **Generale** legge numeri e date come se fossero stati digitati, mentre **Testo** mantiene ogni parte così com'è, quindi "007" resta "007".
5. In **Inserisci le parti a partire da**, digita la prima cella per le parti. Inizialmente corrisponde alla prima cella selezionata.
6. La casella sotto le opzioni mostra le prime righe divise come hai scelto. Premi **OK**.

Le parti vengono inserite nelle celle a destra della prima cella e sostituiscono il contenuto che vi si trovava. Testo in colonne è un'unica operazione che puoi annullare.

## Rimuovi duplicati

Usa questo strumento per conservare solo la prima di ogni gruppo di righe ripetute.

1. Seleziona le celle dell'elenco, oppure una cella al suo interno. JeffCrunch prende l'intero blocco di dati intorno a una sola cella.
2. Scegli **Rimuovi duplicati**.
3. Seleziona **I miei dati hanno una riga di intestazione** se la prima riga contiene dei nomi. Quella riga resta e non viene confrontata.
4. Seleziona le colonne che devono coincidere perché due righe siano considerate ripetute. Due righe sono ripetute solo quando tutte le colonne selezionate sono uguali. Maiuscole e minuscole sono considerate uguali, e una formula conta come il valore che produce.
5. Premi **OK**. La finestra indica quante righe sono state rimosse e quante ne restano.

Le righe sotto una riga rimossa salgono all'interno delle celle selezionate, con i loro formati e le loro formule. Le celle sotto la selezione non si spostano. Le righe vuote rimaste in fondo alla selezione vengono svuotate. Rimuovi duplicati è un'unica operazione che puoi annullare.

## Ricerca obiettivo

Usa questo strumento quando conosci il risultato che vuoi ma non il numero che lo produce. Ad esempio, una cella di pagamento contiene una formula che usa un tasso di interesse, e vuoi che il pagamento sia 500. Ricerca obiettivo prova dei numeri nella cella del tasso finché il pagamento non risulta 500.

1. Scegli **Ricerca obiettivo** nella scheda **Dati**.
2. In **Cella con la formula**, digita la cella che contiene la formula, ad esempio B3. Inizialmente corrisponde alla cella selezionata.
3. In **Valore desiderato**, digita il risultato che vuoi che la formula restituisca.
4. In **Cella da modificare**, digita la cella che Ricerca obiettivo può cambiare, ad esempio B1. Deve contenere un numero, non una formula.
5. Premi **OK**.

Se Ricerca obiettivo trova un numero, questo resta nella cella da modificare, e la barra di stato ne indica il valore. Se non riesce, non cambia nulla e viene segnalato. Prova a digitare un numero iniziale diverso nella cella da modificare e verifica che la formula utilizzi davvero quella cella. Ricerca obiettivo è un'unica operazione che puoi annullare.

## Gestione scenari

Uno scenario è un insieme di valori con un nome, assegnati ad alcune celle. Usa gli scenari per fare delle ipotesi ("what if") su un solo foglio. Ad esempio, conserva uno scenario "Vendite basse" e uno "Vendite alte" per le stesse celle di input, e mostra quello che vuoi esaminare.

1. Scegli **Gestione scenari** nella scheda **Dati**.
2. Premi **Aggiungi**. Digita un nome, ad esempio Vendite alte. In **Celle da modificare**, digita le celle che lo scenario imposta, ad esempio B1:B3 oppure D5. Uno scenario può modificare fino a 32 celle. Le celle selezionate vengono inserite automaticamente. Ogni cella parte dal valore che contiene ora. Digita nella tabella il valore che questo scenario deve usare, quindi premi **OK**.
3. Seleziona uno scenario nell'elenco e premi **Mostra** per inserire i suoi valori nelle celle. Anche fare doppio clic sul nome ha lo stesso effetto. Premi **Modifica** per cambiarne il nome, il commento o i valori. Premi **Elimina** per rimuoverlo.

Mostra, Aggiungi, Modifica ed Elimina sono ciascuna un'operazione che puoi annullare.

**Riepilogo** mette tutti gli scenari affiancati in un nuovo foglio chiamato Riepilogo scenari. Chiede le celle del risultato, cioè le celle che vuoi confrontare, ad esempio il totale in B10. Per ogni scenario, il riepilogo mostra i valori delle celle modificabili e il risultato delle celle del risultato, calcolato con i valori di quello scenario. Una colonna denominata Valori correnti mostra come erano le celle quando hai creato il riepilogo. Le tue celle restano come erano.

Gli scenari vengono salvati nella cartella di lavoro. Una cartella di lavoro creata con un altro programma conserva i suoi scenari, che qui vengono visualizzati. Una copia salvata come file .xls meno recente o come .ods non li conserva, e il salvataggio lo segnala.

## Subtotale

Usa questo strumento per inserire una riga di totali sotto ogni gruppo di righe che hanno la stessa voce in una colonna, e un totale generale in fondo. L'elenco deve avere una riga di intestazione e dev'essere ordinato per la colonna in base alla quale raggruppi, così che le voci uguali siano una accanto all'altra.

1. Seleziona una cella nell'elenco.
2. Scegli **Subtotale** nel gruppo **Struttura** della scheda **Dati**.
3. In **Ad ogni variazione di**, scegli la colonna che raggruppa le righe, ad esempio Regione.
4. In **Usa la funzione**, scegli il calcolo da eseguire: **Somma**, **Conteggio**, **Media**, **Max**, **Min** e altri.
5. In **Aggiungi subtotale a**, seleziona ogni colonna da totalizzare.
6. Lascia selezionato **Sostituisci subtotali correnti** per sostituire i subtotali già presenti. Premi **OK**.

Ogni riga di totale riporta "Totale Est" (o la voce a cui si riferisce) e contiene una formula SUBTOTALE, così esclude le altre righe di totale. Le righe di ogni gruppo vengono raggruppate, quindi puoi usare i pulsanti 1, 2 e 3 accanto alle righe per mostrare solo i totali. Per eliminare i totali, apri di nuovo **Subtotale** e premi **Rimuovi tutto**. Aggiungere o rimuovere subtotali è un'unica operazione che puoi annullare.

## Filtro avanzato

I pulsanti di filtro scelgono le righe una colonna alla volta. **Avanzate** (nel gruppo **Ordina e filtra**) permette di scrivere le condizioni nel foglio, così possono essere lunghe quanto serve.

1. Accanto all'elenco, copia le intestazioni delle colonne da verificare in una riga di celle vuote.
2. Sotto ogni intestazione, scrivi una condizione. Le condizioni nella **stessa riga** devono essere tutte vere (questo è "e"). Le condizioni in **righe diverse** sono alternative: una riga dell'elenco deve soddisfarne solo una di quelle righe (questo è "o").
3. Scegli **Avanzato**. Verifica che **Intervallo elenco** sia l'elenco con la sua riga di intestazione, e digita le celle delle intestazioni e delle condizioni in **Intervallo condizioni**.
4. Scegli **Nascondi le altre righe nell'elenco**, oppure **Copia le righe in un'altra posizione** e digita la prima cella della posizione, ad esempio H1. Seleziona **Escludi le righe che ripetono una precedente** per copiarne solo una per ciascuna.
5. Premi **OK**.

Come scrivere una condizione:

- Una parola, ad esempio **An**, seleziona il testo che inizia con quella parola. Maiuscole e minuscole sono considerate uguali. **?** indica un carattere qualsiasi e ***** una sequenza di caratteri qualsiasi.
- Un numero seleziona le celle che lo uguagliano.
- **=Ann** seleziona il testo esattamente uguale a Ann. **=** da solo seleziona le celle vuote.
- **>**, **>=**, **<**, **<=** e **<>** (diverso) davanti a un numero o a una parola confrontano con essi, ad esempio **>25**. **<>** da solo seleziona le celle non vuote.

Una riga di condizioni vuota seleziona tutte le righe. Per mostrare di nuovo le righe nascoste, scegli **Cancella**. Una copia viene creata immediatamente e non cambia quando cambia l'elenco. Filtro avanzato è un'unica operazione che puoi annullare.

## Convalida dati

Usa questo strumento per stabilire cosa può essere digitato in alcune celle, in modo che un inserimento errato venga bloccato o messo in discussione.

1. Seleziona le celle.
2. Scegli **Convalida dati** nel gruppo **Strumenti dati** della scheda **Dati**.
3. Nella scheda **Impostazioni**, scegli cosa **Consenti**: **Qualsiasi valore** (nessuna regola), **Numero intero**, **Decimale**, **Elenco**, **Data**, **Ora**, **Lunghezza testo** oppure **Personalizzata**.
4. Per un numero, una data, un'ora o una lunghezza del testo, scegli **Dati** (**compreso tra**, **non compreso tra**, **uguale a** e così via) e digita i limiti. Un limite può essere un valore oppure una formula che inizia con =.
5. Per un **Elenco**, digita le voci separate da virgole, ad esempio Rosso,Verde,Blu, oppure digita un intervallo di celle dopo =, ad esempio =$A$1:$A$5. La cella mostra poi una freccia con le voci disponibili. Disattiva **Elenco a discesa nella cella** per nascondere la freccia.
6. Per **Personalizzata**, digita una formula che inizia con =. L'inserimento è consentito quando la formula è vera.
7. **Ignora celle vuote** consente il passaggio di una cella vuota.

Nella scheda **Messaggio di input**, seleziona la casella e scrivi un titolo e un messaggio. Compaiono accanto alla cella ogni volta che è selezionata.

Nella scheda **Avviso di errore**, scegli cosa succede a un inserimento non consentito. **Arresta** lo rifiuta. **Avviso** chiede se mantenerlo. **Informazioni** segnala l'inserimento e lo mantiene. Scrivi un titolo e un messaggio per l'avviso.

**Cerchia dati non validi** (accanto a **Convalida dati**) disegna un ovale rosso intorno a ogni cella del foglio il cui valore non è consentito dalla sua regola, ad esempio un numero inserito prima dell'applicazione della regola, oppure il risultato di una formula. Un messaggio indica se tutte le celle sono corrette. Una cella che correggi perde il suo ovale. **Cancella cerchi di convalida** rimuove tutti gli ovali.

**Cancella tutto** elimina le regole dalle celle selezionate. La nuova regola sostituisce qualsiasi regola che le celle selezionate avevano già; le altre celle mantengono le proprie. La modifica delle regole è un'unica operazione che puoi annullare, e le regole vengono salvate nel file.

## Consolida

Usa questo strumento per unire in una sola tabella intervalli simili, come la stessa tabella presente in più fogli.

1. Scegli **Consolida** nel gruppo **Strumenti dati** della scheda **Dati**.
2. Scegli la **Funzione** con cui combinare, ad esempio **Somma** o **Media**.
3. Digita un intervallo nella casella e premi **Aggiungi**. Usa **Foglio2!A1:C10** per un intervallo su un altro foglio. Aggiungi ogni intervallo che vuoi combinare. Seleziona un intervallo nell'elenco e premi **Elimina** per rimuoverlo.
4. Se gli intervalli hanno dei nomi nella prima riga, seleziona **La riga superiore contiene i nomi delle colonne**. Se hanno dei nomi nella prima colonna, seleziona **La colonna sinistra contiene i nomi delle righe**.
5. In **Inserisci il risultato in**, digita la prima cella per il risultato.
6. Seleziona **Crea collegamenti ai dati di origine** se ogni risultato deve essere una formula che legge le celle di origine, così da cambiare quando cambiano quelle. Senza questa opzione, i risultati sono semplici numeri.
7. Premi **OK**.

Se non è selezionata alcuna opzione sui nomi, viene combinata la cella nella stessa posizione di ogni intervallo (la prima cella di ogni intervallo, poi la seconda, e così via). Con i nomi selezionati, vengono combinate le celle con gli stessi nomi, ovunque si trovino in ciascun intervallo, e un nome presente solo in alcuni intervalli ottiene comunque una riga o una colonna. Consolida è un'unica operazione che puoi annullare.

## Foglio di previsione

Foglio di previsione crea un nuovo foglio con una tabella e un grafico di valori futuri, calcolati a partire da una sequenza temporale e da una serie di valori. Individua la tendenza e la stagionalità ripetitiva dei valori, e aggiunge una fascia all'interno della quale i valori futuri probabilmente rimarranno.

1. Seleziona la sequenza temporale e i valori, due colonne affiancate (prima la sequenza temporale). La sequenza temporale è costituita da date o numeri distanziati in modo uniforme, ad esempio ogni mese o ogni 5 unità. Una prima riga di nomi viene esclusa.
2. Scegli **Foglio di previsione** nel gruppo **Previsione** della scheda **Dati**.
3. Un'anteprima mostra la previsione e cambia quando modifichi le opzioni. Scegli **Grafico a linee** oppure **Grafico a colonne**.
4. Imposta le opzioni:
   - **Intervallo sequenza temporale** e **Intervallo valori** sono le celle da leggere. Scrivi qui altre celle se la selezione non era corretta.
   - **Inizio previsione** è vuoto per iniziare dopo l'ultimo punto. Una data o un numero precedente all'ultimo punto inizia da quel punto e usa solo i punti che lo precedono, così puoi confrontare la previsione con quanto è realmente accaduto.
   - **Fine previsione** è l'ultimo momento da prevedere. Si possono prevedere al massimo 10.000 punti.
   - **Intervallo di confidenza** indica quanto è sicura la fascia, inizialmente 95%.
   - **Stagionalità** viene rilevata osservando i valori, oppure puoi impostare la lunghezza della stagione in punti, o scegliere nessuna. I valori devono contenere due stagioni complete.
   - **Riempi i punti mancanti con** stabilisce cosa ottiene un momento privo di valore: una retta tra i vicini, oppure zero.
   - **Aggrega i duplicati usando** stabilisce come vengono combinati i valori dello stesso momento: Media, Conteggio, CountA, Max, Mediana, Min oppure Somma.
   - **Includi statistiche di previsione** aggiunge una tabella con Alfa, Beta, Gamma, MASE, SMAPE, MAE e RMSE, che indicano quanto bene la previsione si adatta ai valori.
5. Se qualcosa non va, la finestra lo spiega in parole semplici (passi non uguali, punti troppo pochi, testo nei valori), e **Crea** resta disattivato.
6. Premi **Crea**.

Il nuovo foglio viene inserito prima di quello su cui ti trovavi. La sua tabella ha le colonne Sequenza temporale, Valori, Previsione, Limite di confidenza inferiore e Limite di confidenza superiore. La previsione e i limiti sono formule che usano `FORECAST.ETS` e `FORECAST.ETS.CONFINT` sulle colonne Sequenza temporale e Valori della tabella stessa, quindi vengono ricalcolati quando cambi un valore. Il grafico mostra i valori, la previsione e la fascia. Foglio di previsione è un'unica operazione che puoi annullare.

I numeri possono differire di alcuni punti percentuali da quelli di altri programmi di fogli di calcolo, che usano un proprio metodo per adattare il lisciamento; una previsione già contenuta in un file mantiene i valori memorizzati nel file finché non cambiano i dati che legge.

Un file con più di 1.000.000 di punti non viene elaborato.
