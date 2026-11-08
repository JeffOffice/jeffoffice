# Strumenti dati
<!-- keywords: gestione scenari, scenari, what-if, che succede se, riepilogo scenario, dati, scheda dati, testo in colonne, dividi, separatore, virgola, tabulazione, larghezza fissa, rimuovi duplicati, ricerca obiettivo, subtotale, totali, filtro avanzato, condizioni, convalida dati, cerchia dati non validi, ovale rosso, cancella cerchi di convalida, consolida, unisci intervalli, consenti, elenco, discesa, messaggio di input, avviso di errore, righe ripetute, univoci, file di testo, csv, da testo, colonne, delimitatore, virgolette, modifica collegamenti, aggiorna valori, cambia origine, collegamenti, esterni, origine, interrompi collegamento, verifica stato, apri origine -->

La scheda **Dati** contiene gli strumenti che lavorano su un elenco di valori: recuperare testo da un file, ordinare un elenco, dividere il testo in colonne ed eliminare le righe ripetute.

## Da testo/CSV

**Da testo/CSV** apre un file di testo (.csv, .tsv o .txt). Funziona come **Apri file di testo** nella scheda File. JeffCrunch chiede come è diviso il file in colonne e che tipo di testo contiene, quindi lo mostra come nuova cartella di lavoro.

## Modifica collegamenti

Se una cartella di lavoro contiene formule che leggono un'altra cartella di lavoro (ad esempio ='[Prezzi.xlsx]Foglio1'!A1), **Modifica collegamenti** nel gruppo **Recupera dati** elenca le altre cartelle di lavoro, dette origini. Ogni riga indica dove si trova l'origine e il suo stato. Finché la cartella di lavoro non viene riaperta con le sue origini, mostra i numeri che il file ha memorizzato da esse.

- **Verifica stato** cerca ciascuna origine su questo computer e indica **Trovata** oppure che non è stata trovata. Un'origine sul Web non viene recuperata, quindi il suo stato indica che non è stata verificata.
- **Aggiorna valori** legge l'origine scelta da questo computer e assegna a ogni cella che è solo un riferimento a una sua cella il valore che tale cella ha ora. Una cella che elabora ulteriormente il valore, come `=[1]Prezzi!A1*2`, mantiene il numero memorizzato nel file, e il messaggio sotto l'elenco indica quante celle sono state aggiornate e quante no. Aggiorna valori è un'unica operazione, quindi **Annulla** ripristina i valori precedenti. I nuovi valori vengono inoltre scritti nella parte del file dedicata al collegamento al momento del salvataggio, così la cartella di lavoro li mostra alla successiva apertura, anche quando l'origine non è presente.
- **Cambia origine** consente di scegliere un altro file per il collegamento selezionato, ad esempio quando la cartella di lavoro è stata spostata. Le formule restano invariate. Il nuovo percorso viene scritto nel file al salvataggio, e **Annulla** annulla la modifica.
- **Apri origine** apre l'origine scelta in una nuova finestra, quando può essere trovata.
- **Interrompi collegamento** trasforma ogni cella che legge l'origine scelta nel valore che mostra ora, e un nome che indicava una posizione al suo interno in **#RIF!**. JeffCrunch chiede conferma prima di procedere. Interrompi collegamento è un'unica operazione, quindi **Annulla** ripristina formule, nomi e collegamenti, fino al salvataggio della cartella di lavoro. Quando non resta alcun collegamento, il file viene salvato senza le parti che li contenevano.

## Ordina e filtra

Il gruppo **Ordina e filtra** contiene gli stessi comandi del menu **Ordina e filtra righe** nella scheda Home. Vedi [Ordinare e filtrare](sort-filter).

## Testo in colonne

Usa questa funzione quando una colonna contiene più parti di testo che vuoi inserire in colonne separate, come "Rossi, Anna, 31".

1. Seleziona le celle di **una** colonna.
2. Scegli **Testo in colonne** nella scheda **Dati**.
3. Scegli come dividere il testo:
   - **Dividi in corrispondenza di un separatore**: seleziona i caratteri che si trovano tra le parti: **Tabulazione**, **Punto e virgola**, **Virgola**, **Spazio** oppure **Altro**, dove digiti il carattere desiderato. **Considera separatori consecutivi come uno solo** evita che due virgole di seguito creino una parte vuota. **Testo tra virgolette** indica il carattere che racchiude una parte che non deve essere divisa, come "Lee, Bo". All'interno delle virgolette, due caratteri di seguito corrispondono a uno solo.
   - **Dividi in posizioni fisse**: digita le posizioni in cui inizia una nuova colonna, contando i caratteri da 1. Ad esempio, 5, 12 inizia una nuova colonna al quinto e al dodicesimo carattere.
4. Scegli il tipo di ciascuna parte: **Generale** legge numeri e date come se li avessi digitati, mentre **Testo** mantiene ogni parte com'è, quindi "007" resta "007".
5. In **Inserisci le parti a partire da**, digita la prima cella per le parti. Inizialmente è la prima cella selezionata.
6. La casella sotto le opzioni mostra le prime righe divise secondo le scelte fatte. Premi **OK**.

Le parti vanno nelle celle a destra della prima cella e sostituiscono il contenuto che vi si trovava. Testo in colonne è un'unica operazione che puoi annullare.

## Rimuovi duplicati

Usa questa funzione per conservare solo la prima di ogni gruppo di righe ripetute.

1. Seleziona le celle dell'elenco, oppure una cella al suo interno. JeffCrunch prende l'intero blocco di dati intorno a una singola cella.
2. Scegli **Rimuovi duplicati**.
3. Seleziona **I miei dati hanno intestazioni** se la prima riga contiene i nomi. Quella riga resta e non viene confrontata.
4. Seleziona le colonne che devono coincidere perché due righe siano considerate ripetute. Due righe sono ripetute solo se tutte le colonne selezionate sono uguali. Lettere maiuscole e minuscole sono considerate uguali, e una formula conta come il valore che restituisce.
5. Premi **OK**. La finestra indica quante righe sono state rimosse e quante ne restano.

Le righe sotto una riga rimossa salgono all'interno delle celle selezionate, con i loro formati e le loro formule. Le celle sotto la selezione non si spostano. Le righe vuote che restano in fondo alla selezione vengono cancellate. Rimuovi duplicati è un'unica operazione che puoi annullare.

## Ricerca obiettivo

Usa questa funzione quando conosci il risultato che vuoi ma non il numero che lo produce. Ad esempio, una cella con il pagamento contiene una formula che usa un tasso di interesse, e vuoi che il pagamento sia 500. Ricerca obiettivo prova numeri nella cella del tasso finché il pagamento non è 500.

1. Scegli **Ricerca obiettivo** nella scheda **Dati**.
2. In **Cella con la formula**, digita la cella che contiene la formula, ad esempio B3. Inizialmente è la cella selezionata.
3. In **Valore desiderato**, digita il risultato che vuoi ottenere dalla formula.
4. In **Cella da modificare**, digita la cella che Ricerca obiettivo può modificare, ad esempio B1. Deve contenere un numero, non una formula.
5. Premi **OK**.

Se Ricerca obiettivo trova un numero, questo resta nella cella da modificare e la barra di stato indica quale è. Se non ci riesce, non cambia nulla e viene segnalato. Prova un altro numero di partenza nella cella da modificare e verifica che la formula usi davvero quella cella. Ricerca obiettivo è un'unica operazione che puoi annullare.

## Gestione scenari

Uno scenario è un insieme con nome di valori per alcune celle. Usa gli scenari per provare ipotesi "what if" su un foglio. Ad esempio, conserva uno scenario "Vendite basse" e uno "Vendite alte" per le stesse celle di input, e mostra quello che vuoi esaminare.

1. Scegli **Gestione scenari** nella scheda **Dati**.
2. Premi **Aggiungi**. Digita un nome, ad esempio Vendite alte. In **Celle modificate**, digita le celle che lo scenario imposta, ad esempio B1:B3 oppure D5. Uno scenario può modificare fino a 32 celle. Le celle selezionate vengono compilate automaticamente. Ogni cella parte dal valore che contiene ora. Digita nella tabella il valore che questo scenario deve usare e premi **OK**.
3. Scegli uno scenario nell'elenco e premi **Mostra** per inserire i suoi valori nelle celle. Anche un doppio clic sul nome ha lo stesso effetto. Premi **Modifica** per cambiarne il nome, il commento o i valori. Premi **Elimina** per rimuoverlo.

Mostra, Aggiungi, Modifica ed Elimina sono ciascuna un'operazione che puoi annullare.

**Riepilogo** affianca tutti gli scenari in un nuovo foglio chiamato Riepilogo scenario. Chiede le celle risultato, cioè le celle che vuoi confrontare, ad esempio il totale in B10. Per ogni scenario, il riepilogo mostra i valori delle celle modificate e il risultato delle celle risultato, calcolato con i valori di quello scenario. Una colonna chiamata Valori correnti mostra com'erano le celle quando hai creato il riepilogo. Le tue celle restano invariate.

Gli scenari vengono salvati nella cartella di lavoro. Una cartella di lavoro creata con un altro programma conserva i suoi scenari, che qui puoi vedere. Una copia salvata come file .xls meno recente o come .ods non li conserva, e il salvataggio te lo segnala.

## Subtotale

Usa questa funzione per inserire una riga di totali sotto ogni gruppo di righe che hanno lo stesso valore in una colonna, e un totale generale in fondo. L'elenco deve avere una riga di intestazione e deve essere ordinato per la colonna con cui si raggruppa, in modo che i valori uguali siano vicini.

1. Seleziona una cella dell'elenco.
2. Scegli **Subtotale** nel gruppo **Struttura** della scheda **Dati**.
3. In **Ad ogni variazione di**, scegli la colonna che raggruppa le righe, ad esempio Regione.
4. In **Usa funzione**, scegli cosa calcolare: **Somma**, **Conteggio**, **Media**, **Max**, **Min** e altre.
5. In **Aggiungi subtotale a**, seleziona ogni colonna da totalizzare.
6. Lascia selezionato **Sostituisci subtotali correnti** per sostituire i subtotali già presenti. Premi **OK**.

Ogni riga di totale riporta "Totale Est" (o il valore a cui si riferisce) e contiene una formula SUBTOTALE, quindi ignora le altre righe di totale. Le righe di ogni gruppo vengono raggruppate, così puoi usare i pulsanti 1, 2 e 3 accanto alle righe per mostrare solo i totali. Per eliminare i totali, apri di nuovo **Subtotale** e premi **Rimuovi tutto**. Aggiungere o rimuovere subtotali è un'unica operazione che puoi annullare.

## Filtro avanzato

I pulsanti di filtro scelgono le righe una colonna alla volta. **Avanzate** (nel gruppo **Ordina e filtra**) consente di scrivere le condizioni nel foglio, così possono essere lunghe quanto serve.

1. Accanto all'elenco, copia le intestazioni delle colonne da verificare in una riga di celle vuote.
2. Sotto ogni intestazione, scrivi una condizione. Le condizioni nella **stessa riga** devono essere tutte vere (è un "e"). Le condizioni in **righe diverse** sono alternative: una riga dell'elenco deve soddisfare solo una di quelle righe (è un "o").
3. Scegli **Avanzate**. Verifica che **Intervallo elenco** sia l'elenco con la sua riga di intestazione, e digita nella casella **Intervallo condizioni** le celle delle intestazioni e delle condizioni.
4. Scegli **Nascondi le altre righe nell'elenco**, oppure **Copia le righe in un'altra posizione** e digita la prima cella della posizione, ad esempio H1. Seleziona **Escludi le righe che ripetono una precedente** per copiarne solo una per ciascun tipo.
5. Premi **OK**.

Come scrivere una condizione:

- Una parola, ad esempio **An**, seleziona il testo che inizia con quella parola. Lettere maiuscole e minuscole sono considerate uguali. **?** rappresenta un carattere qualsiasi e ***** una sequenza qualsiasi di caratteri.
- Un numero seleziona le celle uguali a quel numero.
- **=Ann** seleziona il testo esattamente uguale ad Ann. **=** da solo seleziona le celle vuote.
- **>**, **>=**, **<**, **<=** e **<>** (diverso) davanti a un numero o a una parola li confrontano, ad esempio **>25**. **<>** da solo seleziona le celle non vuote.

Una riga di condizioni vuota seleziona tutte le righe. Per mostrare di nuovo le righe nascoste, scegli **Cancella**. La copia viene creata subito e non cambia quando cambia l'elenco. Filtro avanzato è un'unica operazione che puoi annullare.

## Convalida dati

Usa questa funzione per stabilire cosa può essere digitato in alcune celle, in modo che un inserimento errato venga bloccato o messo in discussione.

1. Seleziona le celle.
2. Scegli **Convalida dati** nel gruppo **Strumenti dati** della scheda **Dati**.
3. Nella scheda **Impostazioni**, scegli cosa **Consenti**: **Qualsiasi valore** (nessuna regola), **Numero intero**, **Decimale**, **Elenco**, **Data**, **Ora**, **Lunghezza testo** oppure **Personalizzato**.
4. Per un numero, una data, un'ora o una lunghezza del testo, scegli **Dati** (**compreso tra**, **non compreso tra**, **uguale a** e così via) e digita i limiti. Un limite può essere un valore oppure una formula che inizia con =.
5. Per un **Elenco**, digita le scelte separate da virgole, ad esempio Rosso,Verde,Blu, oppure digita un intervallo di celle dopo =, ad esempio =$A$1:$A$5. La cella mostra allora una freccia con le scelte. Disattiva **Elenco a discesa nella cella** per nascondere la freccia.
6. Per **Personalizzato**, digita una formula che inizia con =. L'inserimento è consentito quando la formula è vera.
7. **Ignora spazi vuoti** consente il passaggio di una cella vuota.

Nella scheda **Messaggio di input**, seleziona la casella e scrivi un titolo e un messaggio. Compaiono accanto alla cella ogni volta che viene selezionata.

Nella scheda **Avviso di errore**, scegli cosa succede a un inserimento non consentito. **Arresta** lo rifiuta. **Avviso** chiede se mantenerlo. **Informazioni** segnala la situazione e lo mantiene. Scrivi un titolo e un messaggio per l'avviso.

**Cerchia dati non validi** (accanto a **Convalida dati**) disegna un ovale rosso intorno a ogni cella del foglio il cui valore non è consentito dalla sua regola, come un numero inserito prima della regola o il risultato di una formula. Un messaggio indica se tutte le celle sono corrette. Una cella che correggi perde il suo ovale. **Cancella cerchi di convalida** elimina tutti gli ovali.

**Cancella tutto** rimuove le regole dalle celle selezionate. La nuova regola sostituisce quelle che le celle selezionate avevano già; le altre celle mantengono le proprie. Modificare le regole è un'unica operazione che puoi annullare, e le regole vengono salvate nel file.

## Consolida

Usa questa funzione per combinare intervalli simili, come la stessa tabella presente in più fogli, in un'unica tabella.

1. Scegli **Consolida** nel gruppo **Strumenti dati** della scheda **Dati**.
2. Scegli la **Funzione** con cui combinare, ad esempio **Somma** o **Media**.
3. Digita un intervallo nella casella e premi **Aggiungi**. Usa **Foglio2!A1:C10** per un intervallo di un altro foglio. Aggiungi ogni intervallo da combinare. Seleziona un intervallo nell'elenco e premi **Elimina** per toglierlo.
4. Se gli intervalli hanno nomi nella prima riga, seleziona **La riga superiore contiene i nomi delle colonne**. Se li hanno nella prima colonna, seleziona **La colonna sinistra contiene i nomi delle righe**.
5. In **Inserisci il risultato in**, digita la prima cella del risultato.
6. Seleziona **Crea collegamenti ai dati di origine** se ogni risultato deve essere una formula che legge le celle di origine, così cambia quando cambiano esse. Senza questa opzione, i risultati sono numeri semplici.
7. Premi **OK**.

Se non è selezionata nessuna opzione relativa ai nomi, vengono combinate le celle nella stessa posizione in ciascun intervallo (la prima cella di ogni intervallo, poi la seconda, e così via). Con i nomi selezionati, vengono combinate le celle con gli stessi nomi, ovunque si trovino in ciascun intervallo, e un nome presente solo in alcuni intervalli ottiene comunque una riga o una colonna. Consolida è un'unica operazione che puoi annullare.
