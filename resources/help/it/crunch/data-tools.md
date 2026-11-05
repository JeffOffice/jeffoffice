# Strumenti dati
<!-- keywords: dati, scheda dati, testo in colonne, dividi, separatore, virgola, tabulazione, larghezza fissa, rimuovi duplicati, ricerca obiettivo, subtotale, totali, filtro avanzato, condizioni, convalida dati, cerchia dati non validi, ovale rosso, cancella cerchi di convalida, consolida, unisci intervalli, consenti, elenco, discesa, messaggio di input, avviso di errore, righe ripetute, univoci, file di testo, csv, da testo, colonne, delimitatore, virgolette -->

La scheda **Dati** contiene gli strumenti che lavorano su un elenco di valori: recuperare testo da un file, ordinare un elenco, dividere il testo in colonne ed eliminare le righe ripetute.

## Da testo/CSV

**Da testo/CSV** apre un file di testo (.csv, .tsv o .txt). Funziona come **Apri file di testo** nella scheda File. JeffCrunch chiede come è suddiviso il file in colonne e che tipo di testo contiene, quindi lo mostra come nuova cartella di lavoro.

## Ordina e filtra

Il gruppo **Ordina e filtra** contiene gli stessi comandi del menu **Ordina e filtra** nella scheda Home. Vedi [Ordinare e filtrare](sort-filter).

## Testo in colonne

Usalo quando una colonna contiene più porzioni di testo che vuoi inserire in colonne separate, ad esempio "Rossi, Anna, 31".

1. Seleziona le celle di **una** colonna.
2. Scegli **Testo in colonne** nella scheda **Dati**.
3. Scegli come dividere il testo:
   - **Dividi in corrispondenza di un separatore**: seleziona i caratteri che separano le porzioni: **Tabulazione**, **Punto e virgola**, **Virgola**, **Spazio** oppure **Altro**, dove digiti il carattere che preferisci. **Considera separatori consecutivi come uno solo** evita che due virgole di fila creino una porzione vuota. **Testo tra virgolette** indica il carattere che racchiude una porzione da non dividere, come "Lee, Bo". Dentro le virgolette, due caratteri di seguito valgono per uno solo.
   - **Dividi in posizioni fisse**: digita le posizioni in cui inizia una nuova colonna, contando i caratteri a partire da 1. Ad esempio, 5, 12 fa iniziare una nuova colonna al quinto e al dodicesimo carattere.
4. Scegli il tipo di ciascuna porzione: **Generale** legge numeri e date come se li avessi digitati, mentre **Testo** mantiene ogni porzione così com'è, quindi "007" resta "007".
5. In **Inserisci le parti a partire da**, digita la prima cella per le parti. Inizialmente è la prima cella selezionata.
6. La casella sotto le opzioni mostra le prime righe divise secondo le scelte fatte. Premi **OK**.

Le parti vengono inserite nelle celle a destra della prima cella e sostituiscono il contenuto presente. Testo in colonne è un'unica operazione che puoi annullare.

## Rimuovi duplicati

Usalo per conservare solo la prima di ogni gruppo di righe ripetute.

1. Seleziona le celle dell'elenco oppure una cella al suo interno. JeffCrunch considera l'intero blocco di dati attorno alla cella.
2. Scegli **Rimuovi duplicati**.
3. Seleziona **I miei dati hanno intestazioni** se la prima riga contiene i nomi delle colonne. Quella riga resta e non viene confrontata.
4. Seleziona le colonne che devono coincidere perché due righe siano considerate ripetute. Due righe sono ripetute solo se tutte le colonne selezionate sono uguali. Lettere maiuscole e minuscole sono considerate uguali, e una formula viene confrontata in base al valore che restituisce.
5. Premi **OK**. La finestra indica quante righe sono state rimosse e quante ne restano.

Le righe sotto una riga rimossa salgono all'interno delle celle selezionate, con i rispettivi formati e formule. Le celle sotto la selezione non si spostano. Le righe vuote rimaste in fondo alla selezione vengono svuotate. Rimuovi duplicati è un'unica operazione che puoi annullare.

## Ricerca obiettivo

Usalo quando conosci il risultato che vuoi ma non il numero che lo produce. Ad esempio, una cella di pagamento contiene una formula che usa un tasso di interesse, e vuoi che il pagamento sia 500. Ricerca obiettivo prova numeri nella cella del tasso finché il pagamento non risulta 500.

1. Scegli **Ricerca obiettivo** nella scheda **Dati**.
2. In **Cella con la formula**, digita la cella che contiene la formula, ad esempio B3. Inizialmente è la cella selezionata.
3. In **Valore desiderato**, digita il risultato che vuoi ottenere dalla formula.
4. In **Cella da modificare**, digita la cella che Ricerca obiettivo può modificare, ad esempio B1. Deve contenere un numero, non una formula.
5. Premi **OK**.

Se Ricerca obiettivo trova un numero, questo resta nella cella da modificare e la barra di stato ne indica il valore. Se non riesce, non cambia nulla e viene mostrato un messaggio. Prova con un altro numero iniziale nella cella da modificare e verifica che la formula usi davvero quella cella. Ricerca obiettivo è un'unica operazione che puoi annullare.

## Subtotale

Usalo per inserire una riga di totali sotto ogni gruppo di righe che hanno la stessa voce in una colonna, e un totale generale in fondo. L'elenco deve avere una riga di intestazione e deve essere ordinato per la colonna su cui raggruppi, in modo che le voci uguali siano vicine.

1. Seleziona una cella nell'elenco.
2. Scegli **Subtotale** nel gruppo **Struttura** della scheda **Dati**.
3. In **A ogni variazione di**, scegli la colonna che raggruppa le righe, ad esempio Regione.
4. In **Usa funzione**, scegli il calcolo da fare: **Somma**, **Conteggio**, **Media**, **Max**, **Min** e altri.
5. In **Aggiungi subtotale a**, seleziona ogni colonna da totalizzare.
6. Lascia selezionato **Sostituisci subtotali correnti** per sostituire i subtotali già presenti. Premi **OK**.

Ogni riga di totale riporta "Totale Est" (o la voce a cui si riferisce) e contiene una formula SUBTOTALE, quindi ignora le altre righe di totale. Le righe di ciascun gruppo vengono raggruppate, così puoi usare i pulsanti 1, 2 e 3 accanto alle righe per mostrare solo i totali. Per eliminare i totali, apri di nuovo **Subtotale** e premi **Rimuovi tutti**. Aggiungere o rimuovere subtotali è un'unica operazione che puoi annullare.

## Filtro avanzato

I pulsanti di filtro selezionano le righe una colonna alla volta. **Avanzato** (nel gruppo **Ordina e filtra**) permette di scrivere le condizioni nel foglio, quindi possono essere quante ne servono.

1. Accanto all'elenco, copia le intestazioni delle colonne da verificare in una riga di celle vuote.
2. Sotto ciascuna intestazione, scrivi una condizione. Le condizioni sulla **stessa riga** devono essere tutte vere (equivale a "e"). Le condizioni su **righe diverse** sono alternative: una riga dell'elenco deve soddisfare solo una di quelle righe (equivale a "o").
3. Scegli **Avanzato**. Verifica che **Intervallo elenco** sia l'elenco con la sua riga di intestazione, e digita nella casella **Intervallo condizioni** le celle delle intestazioni e delle condizioni.
4. Scegli **Nascondi le altre righe nell'elenco**, oppure **Copia i risultati in un'altra posizione** e digita la prima cella della posizione, ad esempio H1. Seleziona **Escludi le righe che ripetono una precedente** per copiarne solo una per ciascun tipo.
5. Premi **OK**.

Come scrivere una condizione:

- Una parola, ad esempio **An**, seleziona il testo che inizia con quella parola. Lettere maiuscole e minuscole sono considerate uguali. **?** rappresenta un solo carattere qualsiasi e ***** una sequenza qualsiasi di caratteri.
- Un numero seleziona le celle uguali ad esso.
- **=An** seleziona il testo che è esattamente An. **=** da solo seleziona le celle vuote.
- **>**, **>=**, **<**, **<=** e **<>** (diverso) prima di un numero o di una parola fanno il confronto, ad esempio **>25**. **<>** da solo seleziona le celle non vuote.

Una riga di condizioni vuota seleziona tutte le righe. Per mostrare di nuovo le righe nascoste, scegli **Cancella**. La copia viene creata subito e non cambia quando cambia l'elenco. Filtro avanzato è un'unica operazione che puoi annullare.

## Convalida dati

Usalo per stabilire che cosa può essere digitato in alcune celle, in modo che un inserimento errato venga bloccato o messo in discussione.

1. Seleziona le celle.
2. Scegli **Convalida dati** nel gruppo **Strumenti dati** della scheda **Dati**.
3. Nella scheda **Impostazioni**, scegli cosa **Consentire**: **Qualsiasi valore** (nessuna regola), **Numero intero**, **Decimale**, **Elenco**, **Data**, **Ora**, **Lunghezza testo** oppure **Personalizzata**.
4. Per un numero, una data, un'ora o una lunghezza del testo, scegli **Dati** (**compreso tra**, **non compreso tra**, **uguale a** e così via) e digita i limiti. Un limite può essere un valore o una formula che inizia con =.
5. Per un **Elenco**, digita le scelte separate da virgole, ad esempio Rosso,Verde,Blu, oppure digita un intervallo di celle dopo =, ad esempio =$A$1:$A$5. La cella mostra allora una freccia con le scelte. Disattiva **Elenco a discesa nella cella** per nascondere la freccia.
6. Per **Personalizzata**, digita una formula che inizia con =. L'inserimento è consentito quando la formula è vera.
7. **Ignora spazi vuoti** consente il passaggio di una cella vuota.

Nella scheda **Messaggio di input**, seleziona la casella e scrivi un titolo e un messaggio. Compaiono accanto alla cella ogni volta che viene selezionata.

Nella scheda **Avviso di errore**, scegli cosa succede a un inserimento non consentito. **Arresta** lo rifiuta. **Avviso** chiede se mantenerlo. **Informazioni** avvisa chi sta scrivendo e lo mantiene. Scrivi un titolo e un messaggio per l'avviso.

**Cerchia dati non validi** (accanto a **Convalida dati**) disegna un ovale rosso attorno a ogni cella del foglio il cui valore non è consentito dalla sua regola, ad esempio un numero inserito prima della regola o il risultato di una formula. Un messaggio indica se tutte le celle sono corrette. Una cella che correggi perde il suo ovale. **Cancella cerchi di convalida** elimina tutti gli ovali.

**Cancella tutto** rimuove le regole dalle celle selezionate. La nuova regola sostituisce quelle che le celle selezionate avevano già; le altre celle mantengono le proprie. La modifica delle regole è un'unica operazione che puoi annullare, e le regole vengono salvate nel file.

## Consolida

Usalo per unire intervalli simili, come la stessa tabella presente in più fogli, in un'unica tabella.

1. Scegli **Consolida** nel gruppo **Strumenti dati** della scheda **Dati**.
2. Scegli la **Funzione** con cui combinare, ad esempio **Somma** o **Media**.
3. Digita un intervallo nella casella e premi **Aggiungi**. Usa **Foglio2!A1:C10** per un intervallo su un altro foglio. Aggiungi ogni intervallo da combinare. Seleziona un intervallo nell'elenco e premi **Elimina** per rimuoverlo.
4. Se gli intervalli hanno nomi nella prima riga, seleziona **La riga superiore contiene i nomi delle colonne**. Se hanno nomi nella prima colonna, seleziona **La colonna sinistra contiene i nomi delle righe**.
5. In **Inserisci il risultato in**, digita la prima cella del risultato.
6. Seleziona **Crea collegamenti ai dati di origine** se ogni risultato deve essere una formula che legge le celle di origine, in modo da cambiare quando cambiano quelle. Altrimenti i risultati sono numeri semplici.
7. Premi **OK**.

Senza nomi selezionati, vengono combinate le celle nella stessa posizione in ciascun intervallo (la prima cella di ogni intervallo, poi la seconda, e così via). Con i nomi selezionati, vengono combinate le celle con lo stesso nome, ovunque si trovino in ciascun intervallo, e un nome presente solo in alcuni intervalli riceve comunque una riga o una colonna. Consolida è un'unica operazione che puoi annullare.
