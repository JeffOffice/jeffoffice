# Tabelle
<!-- keywords: tabella, tabelle, riferimento strutturato, riga di intestazione, riga dei totali, riga totali, colonna calcolata, righe alternate, colonne alternate, a bande, stile, aspetto tabella, pulsanti filtro, Ctrl+T, converti, celle normali, ridimensiona, nome colonna, [@Colonna], Tabella1[Colonna] -->

Una tabella è un blocco di celle con una riga di nomi sopra le colonne, trattato come un unico elemento: ha un nome, un aspetto, pulsanti per filtrare e ordinare le righe e si espande quando vi si aggiungono dati. Le formule possono fare riferimento alle colonne per nome anziché per indirizzo di cella.

## Creare una tabella

Selezionare le celle, oppure fare clic su una cella dei dati, e scegliere **Tabella** nella scheda **Inserisci** (**Ctrl+T**). JeffCrunch chiede quali celle includere nella tabella e se la prima riga contiene i nomi delle colonne. I nomi costituiti da numeri, date o formule diventano il testo che mostrano, i nomi vuoti diventano Colonna1, Colonna2 e così via, e due colonne con lo stesso nome vengono numerate. Se non ci sono nomi, la tabella non ha una riga di intestazione e nelle formule le sue colonne si chiamano Colonna1, Colonna2 e così via.

La raccolta **Aspetto tabella** (scheda **Home** o scheda **Tabella**) mostra gli stili disponibili: chiari, medi e scuri, ciascuno in nero e nei sei colori del tema della cartella di lavoro. Se se ne sceglie uno per celle che non sono una tabella, le celle diventano una tabella. L'aspetto viene applicato sotto la formattazione propria di ogni cella: il riempimento, i bordi, il colore del testo o il grassetto impostati su una cella restano invariati.

## La scheda Tabella

La scheda **Tabella** viene visualizzata quando il cursore si trova in una tabella.

- **Nome** è il nome con cui le formule fanno riferimento alla tabella. Tutte le formule che usano il nome precedente vengono aggiornate. Un nome contiene lettere, cifre, caratteri di sottolineatura e punti, inizia con una lettera o un carattere di sottolineatura e non può coincidere con il nome di un'altra tabella, con un nome della cartella di lavoro o con un indirizzo di cella come A1.
- **Riga di intestazione**, **Riga totali**, **Righe alternate**, **Colonne alternate**, **Prima colonna**, **Ultima colonna** e **Pulsanti filtro** si attivano o disattivano per la tabella.
- **Ridimensiona tabella…** chiede quali celle includere nella tabella. La prima cella resta invariata; è possibile aggiungere o togliere righe in basso e colonne a destra.
- **Converti in celle normali** trasforma la tabella in celle normali, che mantengono il loro contenuto. Le formule che facevano riferimento alla tabella o alle sue colonne fanno invece riferimento alle celle.

## Il menu delle celle

Fare clic con il pulsante destro del mouse su una cella della tabella e scegliere **Tabella**: **Inserisci riga sopra**, **Inserisci riga sotto**, **Inserisci colonna a sinistra**, **Inserisci colonna a destra**, **Elimina righe tabella**, **Elimina colonne tabella**, **Riga totali** e **Converti in celle normali**. Questi comandi inseriscono o eliminano intere righe e colonne del foglio, come Inserisci ed Elimina; una riga o colonna inserita subito dopo la fine della tabella viene inclusa nella tabella.

## Riga totali

Attivando **Riga totali**, la riga sotto la tabella contiene la parola Totale nella prima colonna e un totale nell'ultima. Ogni cella della riga ha un pulsante con cui scegliere cosa mostra il totale della colonna: la media, un conteggio, quanti numeri sono presenti, il valore più grande, il più piccolo, la somma, la deviazione standard, la varianza oppure **Formula personalizzata**. Il totale è un SUBTOTALE della colonna, quindi esclude le righe nascoste da un filtro.

## Aggiungere dati a una tabella

- Se si digita nella riga sotto una tabella o nella colonna accanto, la tabella si espande per includere la cella. **Annulla** annulla l'espansione e mantiene il testo digitato.
- Una formula digitata in una cella di una colonna altrimenti vuota viene inserita in tutte le celle della colonna, creando una colonna calcolata. Se si sceglie **Annulla** subito dopo, la formula resta solo in quella cella. Una formula digitata in una colonna calcolata modifica l'intera colonna se le altre celle contengono la formula precedente; in caso contrario, la cella resta a sé.
- Le righe inserite all'interno di una tabella ricevono le formule delle colonne calcolate. Le colonne inserite all'interno ricevono un nome nella riga di intestazione. L'eliminazione di righe o colonne all'interno la riduce; non è possibile eliminare la riga di intestazione, né tutte le colonne.

## Filtrare e ordinare una tabella

I pulsanti sui nomi filtrano e ordinano solo le righe della tabella e funzionano come i pulsanti del filtro di un foglio (vedere [Ordinare e filtrare](sort-filter)). Non è possibile filtrare una tabella se un filtro del foglio o di un'altra tabella copre le stesse righe, perché ciascuno dovrebbe sapere quali righe gli appartengono.

## Formule che fanno riferimento a una tabella

- `Vendite[Importo]` indica le celle della colonna Importo, escluso il nome e il totale.
- `[@Importo]` indica la cella nella stessa riga della formula e nel file viene scritto `Vendite[[#This Row],[Importo]]`.
- `Vendite[#Headers]`, `Vendite[#Data]`, `Vendite[#Totals]` e `Vendite[#All]` indicano rispettivamente i nomi, i dati, i totali e l'intera tabella, mentre `Vendite[[#Headers],[Importo]]` indica una sola colonna di questi.
- `Vendite[[Regione]:[Importo]]` indica le colonne da Regione a Importo.

Digitando `[` dopo il nome di una tabella viene visualizzato l'elenco delle colonne. Durante la digitazione di una formula, facendo clic su una cella di una tabella viene scritto un riferimento come `Vendite[@Importo]` o `Vendite[Importo]` (se si fa clic su un'intera colonna di dati). Rinominando una colonna nell'intestazione, il nome viene aggiornato in tutte le formule; inserendo ed eliminando colonne, le formule continuano a fare riferimento alle colonne indicate, e una colonna eliminata lascia #RIF! nelle formule che vi facevano riferimento.
