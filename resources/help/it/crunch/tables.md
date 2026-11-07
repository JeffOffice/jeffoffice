# Tabelle
<!-- keywords: tabella, tabelle, rimuovi duplicati, duplicati, riepilogo, cancella stile, riferimento strutturato, riga di intestazione, riga dei totali, colonna calcolata, a bande, alternate, stile, aspetto tabella, nuovo stile tabella, stile tabella personalizzato, pulsanti filtro, Ctrl+T, converti, celle normali, ridimensiona, nome colonna, [@Colonna], Tabella1[Colonna] -->

Una tabella è un intervallo di celle con una riga di nomi sopra le colonne, trattato come un unico oggetto: ha un nome, un aspetto, pulsanti per filtrare e ordinare le sue righe, e si allarga quando vi si aggiunge qualcosa. Le formule possono richiamare le colonne per nome invece che per indirizzo di cella.

## Creare una tabella

Seleziona le celle, oppure fai clic su una cella dei dati, e scegli **Tabella** nella scheda **Inserisci** (**Ctrl+T**). JeffCrunch chiede quali celle deve comprendere la tabella e se la prima riga contiene i nomi delle colonne. I nomi che sono numeri, date o formule diventano il testo che mostrano, i nomi vuoti diventano Colonna1, Colonna2 e così via, e due colonne con lo stesso nome vengono numerate. Se non ci sono nomi, la tabella non ha una riga di nomi e le sue colonne si chiamano Colonna1, Colonna2 e così via nelle formule.

La galleria **Aspetto tabella** (scheda **Home**, oppure scheda **Tabella**) mostra gli aspetti (**Chiaro**, **Medio** e **Scuro**, ciascuno in nero e nei sei colori del tema della cartella di lavoro). **Cancella**, sotto la galleria nella scheda **Tabella**, toglie l'aspetto alla tabella. Scegliere un aspetto per celle che non sono una tabella le trasforma in una tabella. L'aspetto viene disegnato sotto ciò che una cella ha di proprio: un riempimento, dei bordi, un colore del testo o il grassetto che hai impostato su una cella restano.

Per creare un aspetto personalizzato, scegli **Nuovo stile tabella** in fondo alla galleria. Digita un nome. Poi scegli un colore di riempimento, un colore del testo e il grassetto per ciascuna parte che vuoi modificare: la riga di intestazione, le righe a bande, la riga dei totali e la prima colonna. Il nuovo aspetto compare nel gruppo **Personalizzato** in cima alla galleria e viene applicato alla tabella in cui ti trovi. Viene salvato nel file, quindi resta disponibile quando apri di nuovo il file.

## La scheda Tabella

La scheda **Tabella** compare quando il cursore si trova in una tabella.

- **Nome** è il nome con cui le formule richiamano la tabella. Ogni formula che usa il vecchio nome viene aggiornata. Un nome contiene lettere, cifre, caratteri di sottolineatura e punti, inizia con una lettera o con un carattere di sottolineatura, e non coincide con quello di un'altra tabella, con un nome della cartella di lavoro o con un indirizzo di cella come A1.
- **Riga di intestazione**, **Riga dei totali**, **Righe alternate**, **Colonne alternate**, **Prima colonna**, **Ultima colonna** e **Pulsanti filtro** sono selezionati o deselezionati per la tabella.
- **Ridimensiona tabella…** chiede le celle che deve comprendere la tabella. Mantiene la sua prima cella; righe e colonne possono essere aggiunte in fondo e a destra, oppure eliminate.
- **Riepiloga con tabella pivot** crea una tabella pivot dalle celle della tabella (vedi [Tabelle pivot](pivot-tables)). La tabella pivot legge la tabella per nome, quindi segue la tabella quando questa si allarga.
- **Rimuovi duplicati…** elimina le righe che si ripetono. Una riga si ripete quando mostra gli stessi valori, in tutte le colonne che selezioni, di una riga sopra di essa. Le maiuscole e le minuscole sono considerate uguali. Resta la prima delle righe, e le righe sotto una riga eliminata salgono di una posizione. La tabella diventa più corta e la sua riga dei totali sale con essa. Un'operazione di **Annulla** le ripristina tutte. Le celle sotto la tabella restano dove sono. Le formule che puntano alle celle della tabella, su qualsiasi foglio, seguono le celle come avviene quando elimini righe; una formula che puntava a una riga eliminata mostra #RIF!. Non funziona su una tabella con un filtro attivo.
- **Converti in celle normali** trasforma la tabella in celle ordinarie. Le celle mantengono il contenuto. Una formula che richiamava la tabella o le sue colonne ora richiama le celle.

## Il menu delle celle

Fai clic con il pulsante destro su una cella di una tabella e scegli **Tabella**: **Inserisci riga sopra**, **Inserisci riga sotto**, **Inserisci colonna a sinistra**, **Inserisci colonna a destra**, **Elimina righe tabella**, **Elimina colonne tabella**, **Riga dei totali** e **Converti in celle normali**. Questi comandi inseriscono o tolgono intere righe e colonne del foglio, come fanno Inserisci ed Elimina; una riga inserita appena oltre la fine della tabella viene inclusa dalla tabella.

## Riga dei totali

Attiva **Riga dei totali** e la riga sotto la tabella contiene la parola Totale nella prima colonna e un totale nell'ultima. Ogni cella della riga ha un pulsante: scegli che cosa mostra il totale della colonna (la media, un conteggio, il numero di valori numerici, il più grande, il più piccolo, la somma, la deviazione standard, la varianza o **Formula personalizzata**). Un totale è un SUBTOTALE della sua colonna, quindi esclude le righe nascoste da un filtro.

## Aggiungere dati a una tabella

- Digita nella riga sotto una tabella, o nella colonna accanto, e la tabella si allarga per includere la cella. **Annulla** elimina l'allargamento e lascia ciò che hai digitato.
- Una formula digitata in una cella di una colonna altrimenti vuota viene inserita in tutte le celle di quella colonna: una colonna calcolata. Subito dopo, **Annulla** lascia la formula nella sola cella. Una formula digitata sopra una colonna calcolata cambia l'intera colonna quando le altre celle contengono la vecchia formula; se alcune non la contengono, la cella resta isolata.
- Le righe inserite all'interno di una tabella ricevono le formule delle colonne calcolate. Le colonne inserite al suo interno vengono denominate nella riga dei nomi. Eliminare righe o colonne all'interno della tabella la rende più piccola; la riga dei nomi e tutte le colonne non possono essere eliminate tutte insieme.

## Filtrare e ordinare una tabella

I pulsanti sui nomi filtrano e ordinano soltanto le righe della tabella. Funzionano come i pulsanti del filtro di un foglio (vedi [Ordina e filtra](sort-filter)). Una tabella non può essere filtrata mentre un filtro del foglio, o di un'altra tabella, interessa le stesse righe, perché ciascuno dovrebbe sapere quali righe gli appartengono.

## Formule che richiamano una tabella

- `Vendite[Importo]` sono le celle della colonna Importo, senza il suo nome e il suo totale.
- `[@Importo]` è la cella della riga della formula stessa, e nel file viene scritta `Vendite[[#This Row],[Importo]]`.
- `Vendite[#Headers]`, `Vendite[#Data]`, `Vendite[#Totals]` e `Vendite[#All]` sono i nomi, i dati, i totali e l'intera tabella, e `Vendite[[#Headers],[Importo]]` è una colonna di questi.
- `Vendite[[Regione]:[Importo]]` sono le colonne da Regione a Importo.

Digita `[` dopo il nome di una tabella e compare un elenco delle sue colonne. Mentre digiti una formula, fare clic su una cella di una tabella scrive un riferimento come `Vendite[@Importo]` o `Vendite[Importo]` (facendo clic su un'intera colonna dei dati). Rinominare una colonna nella sua intestazione la cambia in ogni formula; inserire ed eliminare colonne mantiene le formule collegate alle colonne che richiamavano, e una colonna eliminata lascia #RIF! nelle formule che la richiamavano.
