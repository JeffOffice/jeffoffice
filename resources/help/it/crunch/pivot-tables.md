# Tabelle pivot
<!-- keywords: tabella pivot, inserire tabella pivot, creare, crea, getpivotdata, INFO.DATI.TAB.PIVOT, genera getpivotdata, aggiorna, aggiorna tutto, cambia origine dati, elenco campi, campi, righe, colonne, valori, filtri, filtro rapporto, totale complessivo, subtotale, layout, compatto, struttura, tabulare, cancella, impostazioni campo valore, somma, conteggio, media, riepilogo, report -->

Una tabella pivot è un piccolo report creato da un elenco lungo. L'elenco contiene, ad esempio, una riga per ogni vendita; il report somma le vendite per area e per mese. Non è necessario scrivere formule. Basta indicare quali colonne dell'elenco vanno sul lato, quali in alto e quale va sommata, e il report si compila da solo.

## Un esempio

Ecco un elenco nelle celle da A1 a C7. Ogni riga è una vendita.

| Region | Month | Amount |
|---|---|---|
| East | Jan | 10 |
| East | Jan | 5 |
| East | Feb | 20 |
| West | Jan | 7 |
| West | Feb | 3 |
| West | Feb | 4 |

Una tabella pivot con **Region** sul lato, **Month** in alto e la somma di **Amount** al centro ha questo aspetto.

| Sum of Amount | Month | | |
|---|---|---|---|
| Region | Jan | Feb | Grand Total |
| East | 15 | 20 | 35 |
| West | 7 | 7 | 14 |
| Grand Total | 22 | 27 | 49 |

East in gennaio vale 15 perché vengono sommate le due righe di East per gennaio, 10 e 5. La colonna Grand Total somma ogni riga, la riga Grand Total somma ogni colonna e la cella nell'angolo somma tutto.

I termini usati:

- Un **campo** è una colonna dell'elenco. Region, Month e Amount sono i campi.
- **Righe**, **Colonne** e **Valori** sono le tre posizioni in cui può trovarsi un campo. Un campo in Righe produce una riga per ognuna delle sue voci diverse; un campo in Colonne produce una colonna per ognuna; un campo in Valori è quello che viene sommato. Una quarta posizione, **Filtri**, contiene un campo che esclude le righe non scelte.
- La **cache** (cache pivot) è una copia dell'elenco che la tabella pivot conserva nel file, per mostrare i totali senza leggere le celle. Il report viene calcolato dalla cache, non dalle celle, quindi non cambia quando cambiano le celle. **Aggiorna** copia di nuovo le celle nella cache.

## Creare una tabella pivot

1. Fare clic su una cella dell'elenco oppure selezionare le celle da usare. L'elenco deve avere una riga di nomi sopra i dati.
2. Scegliere **Inserisci**, quindi **Tabella pivot**.
3. La finestra mostra le celle che contengono i dati. Modificarle se non sono corrette. È anche possibile digitare il nome di una tabella.
4. Scegliere dove inserire la tabella pivot: **Un nuovo foglio** (impostazione predefinita) oppure una cella di questo foglio o di un altro. Scegliere una posizione libera, perché la tabella vuota occupa circa diciotto righe e tre colonne.
5. Scegliere **OK**. La tabella è inizialmente vuota e si apre il riquadro **Campi tabella pivot**. Selezionare i campi desiderati: un campo numerico viene sommato, qualsiasi altro campo diventa una riga. Spostare poi i campi nelle caselle come descritto nelle sezioni seguenti.

Il nuovo foglio è un passaggio di Annulla a sé, così come la tabella. Quando si salva in formato .xlsx, la tabella viene scritta come la scrive il programma originale, quindi lì si apre come tabella pivot. Una copia salvata in formato .xls o .ods mantiene i numeri che la tabella mostra nelle celle, ma non la tabella stessa, e il salvataggio lo segnala.

## GETPIVOTDATA: leggere un numero da una tabella pivot

`GETPIVOTDATA` restituisce il numero che una tabella pivot mostra per i valori e le righe e colonne indicate. Per l'esempio precedente:

`=GETPIVOTDATA("Sum of Amount", $E$3, "Region", "East", "Month", "Jan")` restituisce 15.

- Il primo argomento è il nome del valore, come lo mostra la tabella (ad esempio Sum of Amount). Funziona anche il nome del campo sommato (Amount).
- Il secondo argomento è una qualsiasi cella della tabella pivot.
- Seguono coppie formate da un campo e da una delle sue voci. Se si indicano solo Region, East, si ottiene il totale di East, 35. Se non si indica nulla, si ottiene il totale complessivo, 49. I campi indicati devono essere i primi campi delle righe, o i primi campi delle colonne, nell'ordine.
- Se la tabella non mostra il numero, la cella mostra `#REF!`.

Quando si fa clic su un valore di una tabella pivot durante la digitazione di una formula, JeffCrunch scrive automaticamente questa formula, in modo che continui a leggere il numero corretto anche dopo la riorganizzazione della tabella. Per disattivare questa funzione, scegliere **Opzioni** nella scheda **Tabella pivot** e deselezionare la casella di controllo relativa a GETPIVOTDATA.

## Cosa fa JeffCrunch con una tabella pivot

Una tabella pivot in una cartella di lavoro viene mostrata come è stata salvata. Se non la si modifica, il file viene salvato con la tabella pivot esattamente com'era. Quando la si aggiorna o la si modifica, JeffCrunch ricalcola il report, inserisce i nuovi numeri nelle celle e riscrive la tabella pivot in modo che il programma che l'ha creata la apra senza problemi.

JeffCrunch calcola le tabelle pivot che sommano, contano, calcolano medie e così via. Una tabella pivot che raggruppa date o numeri, contiene un campo o un elemento calcolato, mostra i valori come percentuale, differenza o posizione, oppure usa un modello di dati, viene mostrata e mantenuta come salvata, e i comandi segnalano che non è possibile modificarla.

Fare clic su una cella qualsiasi di una tabella pivot. Viene visualizzata la scheda **Tabella pivot**.

## Aggiorna

**Aggiorna** legge le celle da cui è creata la tabella pivot e ricalcola il report. Le nuove voci vengono aggiunte dopo quelle già presenti; le voci non più esistenti vengono eliminate. Le larghezze delle colonne e il layout scelto restano invariati. **Aggiorna tutto**, nella scheda **Dati** e nella scheda **Tabella pivot**, esegue la stessa operazione per tutte le tabelle pivot della cartella di lavoro.

Se il report dovesse estendersi su celle che contengono qualcosa, JeffCrunch non esegue l'aggiornamento e indica quali celle sono d'intralcio. Cancellarle o spostare la tabella pivot, quindi aggiornare di nuovo.

**Annulla** annulla un aggiornamento in un solo passaggio.

## Cambia origine dati

**Cambia origine dati** chiede le celle in cui si trova l'elenco, inclusa la prima riga di nomi. Digitarle nel formato `Foglio1!A1:D100`, oppure digitare il nome di una tabella o un nome definito. JeffCrunch le legge e ricalcola il report. Se un campo dell'elenco ha lo stesso nome nelle nuove celle, la sua posizione nel report rimane invariata.

## Cancella

**Cancella** offre due opzioni. **Cancella filtri** rimuove tutti i filtri e mostra di nuovo tutte le voci. **Cancella tutto** rimuove tutti i campi dal report e lo lascia vuoto, pronto per essere ricreato.

## Il riquadro dei campi

**Elenco campi** nella scheda **Tabella pivot** apre il riquadro **Campi tabella pivot**. Il riquadro elenca i campi dell'elenco, ognuno con una casella accanto, e sotto le quattro posizioni: **Filtri**, **Colonne**, **Righe** e **Valori**.

- Selezionare un campo per aggiungerlo. Un campo che contiene solo numeri va in **Valori** e viene sommato; qualsiasi altro va in **Righe**.
- Deselezionare un campo per rimuoverlo ovunque.
- Fare clic su un campo in una delle quattro posizioni e scegliere **Sposta in** per spostarlo in un'altra. I comandi **Sposta su** e **Sposta giù** ne modificano l'ordine nella posizione, e **Rimuovi campo** lo rimuove.
- Con più di un valore, in **Colonne** compare una voce denominata **Valori**. Spostarla in **Righe** per disporre i valori sul lato.
- **Impostazioni campo valore** (doppio clic su un valore, oppure il pulsante) imposta il nome che il report mostra per il valore e il modo in cui viene calcolato il totale: **Somma**, **Conteggio**, **Conta numeri**, **Media**, **Max**, **Min**, **Prodotto**, **Dev.std** e **Dev.std.pop** (la deviazione standard di un campione o di tutti i valori) e **Var** e **Var.pop** (la varianza degli stessi).

Ogni modifica è un passaggio di **Annulla**.

## Struttura: subtotali, totali complessivi e layout

- **Subtotali** mostra o nasconde il totale di ogni gruppo, all'inizio o alla fine del gruppo. Quando in Righe è presente più di un campo, il totale di ogni campo esterno è un subtotale.
- **Totali complessivi** offre quattro opzioni: disattivati per righe e colonne, attivati per entrambe, attivati solo per le righe (una colonna di totali a destra, uno per ogni riga) e attivati solo per le colonne (una riga di totali in basso).
- **Layout report** è la disposizione delle etichette. In formato **compatto**, tutti i campi di Righe condividono una colonna e ogni nuovo gruppo è leggermente spostato a destra. In formato **struttura**, ogni campo ha una propria colonna e ogni gruppo ha una riga propria sopra i suoi elementi. In formato **tabulare**, ogni campo ha una propria colonna e il nome del gruppo si trova sulla sua prima riga, con il subtotale sotto il gruppo.

## Lo stesso elenco, con un filtro

Spostare **Month** da **Colonne** a **Filtri**. Il report perde le colonne e una riga sopra di esso mostra **Month** e **(Tutto)**. I numeri ora sono il totale di ogni area su tutti i mesi.

## Limiti

Una tabella pivot può avere fino a 2.000 campi, un elenco fino a cinque milioni di righe e un report fino a quattro milioni di celle. Un file che supera questi limiti viene mostrato come salvato e non può essere modificato. Rileggere centomila righe di venti colonne e ricalcolare il report ha richiesto circa mezzo secondo sul computer usato per la misurazione; un computer più lento impiega più tempo.
