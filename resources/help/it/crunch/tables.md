# Tabelle
<!-- keywords: tabella, tabelle, rimuovi duplicati, duplicati, riepiloga, cancella stile, riferimento strutturato, riga di intestazione, riga dei totali, colonna calcolata, righe alternate, colonne alternate, stile, aspetto tabella, pulsanti filtro, Ctrl+T, converti, celle normali, ridimensiona, nome colonna, [@Colonna], Tabella1[Colonna] -->

Una tabella è un intervallo di celle con una riga di nomi sopra le colonne, trattato come un unico elemento: ha un nome, un aspetto, pulsanti per filtrare e ordinare le sue righe, e si espande quando vi aggiungi dati. Le formule possono richiamare le sue colonne per nome invece che per indirizzo di cella.

## Creare una tabella

Seleziona le celle, oppure fai clic su una cella dei dati, e scegli **Tabella** nella scheda **Inserisci** (**Ctrl+T**). JeffCrunch chiede quali celle comprende la tabella e se la prima riga contiene i nomi delle colonne. I nomi che sono numeri, date o formule diventano il testo che mostrano; i nomi vuoti diventano Colonna1, Colonna2 e così via, e due colonne con lo stesso nome vengono numerate. Se non ci sono nomi, la tabella non ha una riga di intestazione e le sue colonne si chiamano Colonna1, Colonna2 e così via nelle formule.

La galleria **Aspetto tabella** (scheda **Home**, oppure scheda **Tabella**) mostra gli aspetti disponibili: chiaro, medio e scuro, ciascuno in nero e nei sei colori del tema della cartella di lavoro. Scegliendone uno per celle che non sono una tabella, le celle diventano una tabella. L'aspetto viene disegnato sotto ciò che una cella ha di proprio: un riempimento, dei bordi, un colore del testo o il grassetto impostati su una cella restano visibili. **Cancella** (sotto la galleria nella scheda **Tabella**) toglie l'aspetto dalla tabella.

## La scheda Tabella

La scheda **Tabella** compare quando il cursore si trova in una tabella.

- **Nome** è il nome con cui le formule richiamano la tabella. Ogni formula che usa il vecchio nome viene aggiornata. Un nome contiene lettere, cifre, caratteri di sottolineatura e punti, inizia con una lettera o con un carattere di sottolineatura, e non coincide con il nome di un'altra tabella, con un nome della cartella di lavoro o con un indirizzo di cella come A1.
- **Riga di intestazione**, **Riga dei totali**, **Righe alternate**, **Colonne alternate**, **Prima colonna**, **Ultima colonna** e **Pulsanti filtro** si attivano o si disattivano per la tabella.
- **Ridimensiona tabella…** chiede le celle che la tabella comprende. Mantiene la prima cella; righe e colonne possono essere aggiunte in fondo e a destra, oppure eliminate.
- **Riepiloga con tabella pivot** crea una tabella pivot dalle celle della tabella (vedi [Tabelle pivot](pivot-tables)). La tabella pivot legge la tabella tramite il suo nome, quindi si aggiorna quando la tabella cresce.
- **Rimuovi duplicati…** elimina le righe ripetute. Una riga è ripetuta quando mostra gli stessi valori, in tutte le colonne che selezioni, di una riga sopra di essa. Le maiuscole e le minuscole sono considerate uguali. Viene mantenuta la prima delle righe, e le righe sotto una riga eliminata salgono di posizione. La tabella diventa più corta, e la sua riga dei totali sale con essa. Un **Annulla** le ripristina tutte. Le celle sotto la tabella restano dove sono. Le formule che puntano alle celle della tabella, su qualsiasi foglio, seguono le celle come avviene quando elimini righe; una formula che puntava a una riga eliminata mostra #RIF!. Non funziona su una tabella con un filtro attivo.
- **Converti in celle normali** trasforma la tabella in celle ordinarie. Le celle mantengono il loro contenuto. Una formula che citava la tabella o le sue colonne cita invece le celle.

## Il menu delle celle

Fai clic con il pulsante destro su una cella di una tabella e scegli **Tabella**: **Inserisci riga sopra**, **Inserisci riga sotto**, **Inserisci colonna a sinistra**, **Inserisci colonna a destra**, **Elimina righe tabella**, **Elimina colonne tabella**, **Riga dei totali** e **Converti in celle normali**. Questi comandi inseriscono o eliminano righe e colonne intere del foglio, come fanno Inserisci ed Elimina; una riga inserita appena oltre la fine della tabella viene acquisita dalla tabella.

## Riga dei totali

Attiva **Riga dei totali** e la riga sotto la tabella riporta la parola Totale nella prima colonna e un totale nell'ultima. Ogni cella della riga ha un pulsante: scegli cosa mostra il totale della colonna (la media, un conteggio, il numero di valori numerici, il più grande, il più piccolo, la somma, la deviazione standard, la varianza o **Formula personalizzata**). Un totale è una SUBTOTALE della sua colonna, quindi esclude le righe nascoste da un filtro.

## Aggiungere dati a una tabella

- Digita nella riga sotto una tabella, o nella colonna accanto, e la tabella cresce per includere la cella. **Annulla** elimina l'espansione e lascia ciò che hai digitato.
- Una formula digitata in una cella di una colonna altrimenti vuota viene inserita in ogni cella di quella colonna: una colonna calcolata. Subito dopo, **Annulla** lascia la formula solo nella cella. Una formula digitata su una colonna calcolata modifica l'intera colonna, quando le altre celle contengono la vecchia formula; se alcune non la contengono, la cella resta indipendente.
- Le righe inserite all'interno di una tabella ricevono le formule delle colonne calcolate. Le colonne inserite all'interno vengono denominate nella riga di intestazione. Eliminando righe o colonne all'interno, la tabella diventa più piccola; la riga di intestazione e tutte le colonne non possono essere eliminate tutte insieme.

## Filtrare e ordinare una tabella

I pulsanti sui nomi filtrano e ordinano solo le righe della tabella. Funzionano come i pulsanti del filtro di un foglio (vedi [Ordina e filtra](sort-filter)). Una tabella non può essere filtrata mentre un filtro del foglio, o di un'altra tabella, interessa le stesse righe, perché ciascuno dovrebbe sapere quali righe gli appartengono.

## Formule che richiamano una tabella

- `Vendite[Importo]` sono le celle della colonna Importo, senza il suo nome e il suo totale.
- `[@Importo]` è la cella della riga della formula stessa, e nel file viene scritta `Vendite[[#Questa riga],[Importo]]`.
- `Vendite[#Intestazioni]`, `Vendite[#Dati]`, `Vendite[#Totali]` e `Vendite[#Tutto]` sono i nomi, i dati, i totali e l'intera tabella, e `Vendite[[#Intestazioni],[Importo]]` è una colonna di essi.
- `Vendite[[Regione]:[Importo]]` sono le colonne da Regione a Importo.

Digita `[` dopo il nome di una tabella e compare un elenco delle sue colonne. Mentre scrivi una formula, facendo clic su una cella di una tabella si inserisce un riferimento come `Vendite[@Importo]` o `Vendite[Importo]` (facendo clic su un'intera colonna dei dati). Rinominare una colonna nella sua intestazione la modifica in ogni formula; inserire ed eliminare colonne mantiene le formule puntate alle colonne che citavano, e una colonna eliminata lascia #RIF! nelle formule che la citavano.
