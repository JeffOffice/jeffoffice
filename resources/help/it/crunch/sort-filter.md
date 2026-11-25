# Ordinamento e filtro
<!-- keywords: ordina, ordinamento, filtro, filtra, filtro automatico, crescente, decrescente, A-Z, regole, intestazione, colore, elenco, mesi, giorni feriali, maiuscole, colonne, più grande, più piccolo, media, compreso tra, inizia con, contiene, data, cancella, riapplica, elenco a discesa, righe nascoste, tabella, Ctrl+Maiusc+L -->

## Ordinamento

Seleziona una cella dei dati e scegli **Ordina dalla A alla Z** o **Ordina dalla Z alla A** (**Ordina e filtra** nella scheda Home). JeffCrunch prende l'intero blocco di dati intorno alla cella, mantiene una riga di intestazione dove la trova (testo sopra i numeri) e dispone le righe in ordine in base alla colonna in cui ti trovi. Con più celle selezionate ordina solo quelle; se accanto ci sono dati, chiede se includerli nell'ordinamento.

Nell'ordine, i numeri vengono prima, poi il testo, poi VERO e FALSO, poi gli errori; le celle vuote finiscono sempre per ultime. Lettere maiuscole e minuscole sono considerate uguali, salvo diversa indicazione. Il testo viene ordinato allo stesso modo ovunque: secondo le lettere dell'alfabeto inglese, con una lettera accentata collocata accanto a quella senza accento, qualunque sia la lingua del computer.

Scegli **Ordina…** per aprire la finestra di dialogo Ordina:

- Aggiungi una regola per ogni colonna in base alla quale ordinare. Una regola successiva spezza i pareggi di quelle precedenti. Usa **Sposta in alto** e **Sposta giù** per cambiare l'ordine delle regole, e **Duplica regola** per partire da una regola esistente.
- Ordina **In base a** valori, al **colore di riempimento** delle celle o al loro **colore del testo**. Per un colore, scegli se mettere le righe **In alto** o **In basso**; le altre righe mantengono il loro ordine. Un colore è quello che risulta: uno scelto dalla tavolozza e uno con gli stessi valori di rosso, verde e blu sono lo stesso colore.
- Per i valori, l'ordine può essere **Dalla A alla Z**, **Dalla Z alla A** oppure un elenco: i giorni della settimana e i mesi (per esteso e abbreviati, in inglese e nella tua lingua), oppure un elenco che digiti, con virgole tra i valori. I valori che non sono nell'elenco vengono dopo quelli che vi compaiono.
- **La prima riga contiene i nomi delle colonne** esclude la prima riga dall'ordinamento. **Distingui maiuscole/minuscole** fa sì che maiuscole e minuscole siano diverse (prima le minuscole). **Ordina colonne, non righe** dispone le colonne in ordine in base ai valori di una riga. **Numeri nel testo in base al valore** mette "Item 2" prima di "Item 10" (senza questa opzione, lettere e cifre vengono confrontate una alla volta, e "Item 10" viene per primo). La scelta vale per l'ordinamento in corso; la finestra la ricorda per il successivo ordinamento e per **Riapplica**. Il file non la conserva: dopo aver riaperto il file, **Riapplica** ordina nel modo semplice, e **Verifica compatibilità** lo segnala quando un ordinamento mantenuto di un filtro o di una tabella è stato fatto in questo modo (un successivo ordinamento semplice ne elimina la nota). I numeri di qualsiasi lunghezza vengono confrontati per valore.

Una formula si sposta con la sua riga, e i suoi riferimenti alle celle della stessa riga restano nella riga. Un riferimento a una cella di un'altra riga viene modificato come se la formula fosse stata copiata. Ciò che è collegato a una cella la segue: la formattazione della cella, la sua nota e il suo collegamento si spostano nel punto in cui finisce la cella. Le regole che riguardano un intervallo di celle (ciò che può essere immesso e le formattazioni condizionali) restano dove sono, perché si riferiscono alle posizioni e non alle celle. L'ordinamento è un'unica operazione annullabile, e **Esc** interrompe un ordinamento lungo; un ordinamento di grandi dimensioni viene eseguito un po' alla volta.

Alcune celle non possono essere ordinate, e ti viene detto perché: celle bloccate in un foglio protetto, parte di una formula matriciale, celle unite di dimensioni diverse (o che si estendono su più righe), celle unite tagliate dal bordo della selezione e la riga di intestazione di una tabella.

## Filtro

Seleziona una cella dei dati e scegli **Filtro** (**Ctrl+Maiusc+L**). Compare un pulsante su ogni cella della prima riga. Fai clic su uno di essi per vedere:

- **Ordina dalla A alla Z** e **Ordina dalla Z alla A**, e **Ordina per colore**.
- Un elenco dei valori della colonna con caselle di controllo, una casella di ricerca e **(Tutto)**. Una colonna di date viene elencata per anno, mese e giorno, e i valori in essa che non sono date vengono elencati sotto di essi. Deseleziona ciò che non ti serve, quindi **OK**. La ricerca mostra i valori che contengono tutte le parole digitate; **(Tutti i risultati trovati)** seleziona o deseleziona solo quelli. L'elenco copre l'intera colonna. Se la colonna ha più valori diversi di quanti ne possa contenere l'elenco, viene segnalato e l'elenco viene troncato; per il resto usa una condizione.
- **Mantieni un colore**, per colore di riempimento o colore del testo.
- **Condizioni numeriche**, **Condizioni testo** o **Condizioni data**: uguale a, maggiore di, almeno, minore di, al massimo, compreso tra, il più grande o il più piccolo (elementi o percentuale), superiore o inferiore alla media, inizia con, termina con, contiene, date in un periodo (oggi, settimana corrente, mese successivo, trimestre precedente, anno fino a oggi, un trimestre o un mese di qualsiasi anno), e **Condizione personalizzata** composta da due confronti uniti da E o O. Nel testo di una condizione, ? e \* rappresentano un carattere qualsiasi e una sequenza qualsiasi di caratteri.
- **Mostra tutto** della colonna.

Un pulsante con un imbuto indica una colonna che ha un filtro. Le righe che non corrispondono vengono nascoste e mantengono i propri numeri di riga. **Cancella** (nel menu **Ordina e filtra**) rimuove tutte le condizioni e mostra le righe; **Riapplica** ricalcola le condizioni dopo che hai modificato un valore. **Filtro** di nuovo disattiva i pulsanti.

Le righe nascoste da un filtro sono tenute separate da quelle che hai nascosto tu: **Mostra righe** non mostra una riga nascosta dal filtro, e cancellare il filtro non mostra una riga che hai nascosto tu. Ordinare un blocco filtrato dispone in ordine solo le righe visibili, tra loro.

Un filtro agisce su un intero foglio, fino a 1.048.576 righe, un po' alla volta; **Cancella** e la disattivazione del filtro funzionano qualunque sia la dimensione. Ciò che si chiede a un filtro è limitato a 40 milioni di verifiche (le righe moltiplicate per le colonne che hanno una condizione, più alcune per i valori elencati nelle condizioni): un filtro che ne richiede di più lo segnala e non viene creato; puoi filtrare meno righe oppure usare meno condizioni. Un filtro del foglio non copre le celle di una tabella: una tabella ha pulsanti propri, che funzionano allo stesso modo (vedi [Tabelle](tables)).

Un elenco di valori in una condizione viene conservato come lo scrive un file, senza lingua nei numeri e nelle date (1,234.50), e un valore corrisponde a una cella che lo mostra nella tua lingua oppure in quel formato; un file filtrato in una lingua filtra le stesse celle in un'altra.

Il filtro viene salvato nel file con le sue condizioni e le righe che nasconde, quindi il file si apre con le stesse righe visibili. Riaprendo un file, l'imbuto compare su ogni colonna che ha una condizione; le righe nascoste nell'intervallo del filtro sono considerate come nascoste dal filtro.
