# Ordinare e filtrare
<!-- keywords: ordinare, ordinamento, filtrare, filtro, filtro automatico, crescente, decrescente, A-Z, regole, intestazione, colore, elenco, mesi, giorni della settimana, maiuscole, minuscole, colonne, valori più grandi, valori più piccoli, media, tra, inizia con, contiene, data, cancella, riapplica, elenco a discesa, righe nascoste, tabella, Ctrl+Maiusc+L -->

## Ordinare

Selezionare una cella nei dati e scegliere **Ordina dalla A alla Z** o **Ordina dalla Z alla A** (**Ordina e filtra** nella scheda Home). JeffCrunch considera l'intero blocco di dati intorno alla cella, mantiene una riga di intestazione se ne trova una (testo sopra numeri) e ordina le righe in base alla colonna in cui ci si trova. Se sono selezionate più celle, ordina solo quelle; se accanto ci sono altri dati, chiede se includerli.

Nell'ordinamento vengono prima i numeri, poi il testo, poi VERO e FALSO, poi gli errori; le celle vuote vanno sempre in fondo. Le lettere maiuscole e minuscole sono considerate uguali, a meno che non si scelga diversamente. Il testo viene ordinato sempre allo stesso modo: secondo l'alfabeto inglese, con ogni lettera accentata posta insieme alla lettera semplice corrispondente, qualunque sia la lingua del computer.

Scegliere **Ordina…** per aprire la finestra di dialogo Ordina:

- Aggiungere una regola per ogni colonna in base a cui ordinare. Una regola successiva risolve i casi di parità delle regole precedenti. Usare **Sposta su** e **Sposta giù** per cambiare l'ordine delle regole e **Duplica regola** per partire da una regola esistente.
- Ordinare **In base a** valori, **colore di riempimento** delle celle o **colore del testo**. Per un colore, scegliere se va **In alto** o **In basso**; le altre righe mantengono il loro ordine. Conta il colore risultante: un colore scelto dalla tavolozza e uno con gli stessi valori di rosso, verde e blu sono considerati uguali.
- Per i valori, l'ordine può essere **A–Z**, **Z–A** o un elenco: i giorni della settimana e i mesi (per esteso e abbreviati, in inglese e nella propria lingua), oppure un elenco digitato, con i valori separati da virgole. I valori non presenti nell'elenco vengono dopo quelli presenti.
- **La prima riga contiene i nomi delle colonne** esclude la prima riga dall'ordinamento. **Distingui maiuscole/minuscole** fa differenza tra lettere maiuscole e minuscole (prima le minuscole). **Ordina colonne, non righe** ordina le colonne in base ai valori di una riga.

Una formula si sposta con la sua riga e i suoi riferimenti a celle della stessa riga restano nella riga. Un riferimento a una cella di un'altra riga viene modificato come se la formula venisse copiata. Ciò che è legato a una cella la segue: il formato della cella, la nota e il collegamento si spostano nella nuova posizione della cella. Le regole che riguardano un intervallo di celle (i valori consentiti e i formati condizionali) restano dove sono, perché si riferiscono alle posizioni e non alle celle. L'ordinamento è un'unica operazione che si può annullare, e **Esc** interrompe un ordinamento di grandi dimensioni; un ordinamento di grandi dimensioni viene eseguito un po' alla volta.

Alcune celle non possono essere ordinate e ne viene indicato il motivo: celle bloccate in un foglio protetto, celle che fanno parte di una matrice, celle unite di dimensioni diverse (o che si estendono su più righe), celle unite tagliate dal bordo della selezione e la riga di intestazione di una tabella.

## Filtrare

Selezionare una cella nei dati e scegliere **Filtro** (**Ctrl+Maiusc+L**). In ogni cella della prima riga compare un pulsante. Facendo clic su un pulsante vengono visualizzati:

- **Ordina dalla A alla Z**, **Ordina dalla Z alla A** e **Ordina per colore**.
- Un elenco dei valori della colonna con caselle di controllo, una casella di ricerca e **(Tutto)**. Una colonna di date viene elencata per anno, mese e giorno, e i valori che non sono date vengono elencati sotto. Deselezionare ciò che non si desidera, quindi scegliere **OK**. La ricerca mostra i valori che contengono tutte le parole digitate; **(Tutti i risultati trovati)** seleziona o deseleziona solo quei valori. L'elenco riguarda l'intera colonna. Se la colonna contiene più valori distinti di quanti l'elenco ne possa contenere, viene indicato e l'elenco viene troncato; per gli altri valori usare una condizione.
- **Mantieni un colore**, in base al colore di riempimento o al colore del testo.
- **Condizioni numeri**, **Condizioni testo** o **Condizioni date**: uguale a, maggiore di, maggiore o uguale a, minore di, minore o uguale a, tra, i valori più grandi o più piccoli (elementi o percentuale), sopra o sotto la media, inizia con, termina con, contiene, date in un periodo (oggi, settimana corrente, mese successivo, trimestre precedente, anno fino a oggi, un trimestre o un mese di qualsiasi anno) e **Condizione personalizzata**, formata da due confronti uniti da E oppure O. Nel testo di una condizione, ? e \* rappresentano rispettivamente un singolo carattere qualsiasi e una sequenza qualsiasi di caratteri.
- **Mostra tutto** per la colonna.

Un pulsante con un imbuto indica una colonna con un filtro. Le righe che non soddisfano i criteri vengono nascoste e mantengono i loro numeri di riga. **Cancella** (nel menu **Ordina e filtra**) rimuove tutte le condizioni e mostra le righe; **Riapplica** ricalcola le condizioni dopo che si è modificato un valore. Scegliendo di nuovo **Filtro** i pulsanti vengono disattivati.

Le righe nascoste da un filtro sono distinte da quelle nascoste manualmente: **Scopri righe** non mostra una riga nascosta dal filtro e la cancellazione del filtro non mostra una riga nascosta manualmente. Ordinando un blocco filtrato, vengono ordinate solo le righe visibili, tra di loro.

Un filtro agisce su un intero foglio, fino a 1.048.576 righe, un po' alla volta; **Cancella** e la disattivazione del filtro funzionano con qualsiasi dimensione. Le operazioni richieste a un filtro sono limitate a 40 milioni di verifiche (le righe moltiplicate per le colonne con una condizione, più alcune per i valori elencati nelle condizioni); un filtro che ne richiede di più lo segnala e non viene creato: in tal caso si possono filtrare meno righe o usare meno condizioni. Un filtro del foglio non riguarda le celle di una tabella: una tabella ha i propri pulsanti, che funzionano allo stesso modo (vedere [Tabelle](tables)).

Un elenco di valori in una condizione viene conservato così come lo scrive il file, senza formati di lingua per numeri e date (1,234.50), e un valore corrisponde a una cella che lo mostra nella propria lingua o in quel formato; un file filtrato in una lingua filtra le stesse celle in un'altra.

Il filtro viene salvato nel file con le sue condizioni e le righe che nasconde, quindi il file si apre con le stesse righe visibili. Quando si riapre un file, l'imbuto compare su ogni colonna con una condizione; le righe nascoste nell'intervallo del filtro sono considerate nascoste dal filtro.
