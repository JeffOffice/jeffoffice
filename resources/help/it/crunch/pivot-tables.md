# Tabelle pivot
<!-- keywords: tabella pivot, approfondisci, riduci livello, inserisci tabella pivot, crea, fai, seleziona, sposta tabella pivot, GETPIVOTDATA, genera GETPIVOTDATA, aggiorna, aggiorna tutto, cambia origine dati, elenco campi, campi, righe, colonne, valori, filtri, filtro rapporto, totale complessivo, subtotale, layout, compatto, struttura, tabulare, cancella, impostazioni campo valore, somma, conteggio, media, mostra valori come, percentuale del totale, differenza, totale parziale, formato numero, impostazioni campo, ordina, filtra, primi, raggruppa, separa, date, mesi, anni, espandi, comprimi, opzioni, stile, righe a bande, righe vuote, ripeti etichette, aggiorna all'apertura del file, grafico pivot, sezione dati, sequenza temporale, mostra dettagli, righe dietro un valore, tabella di riepilogo -->

Una tabella pivot è un piccolo rapporto ricavato da un lungo elenco. L'elenco ha una riga per ogni vendita, per esempio; il rapporto somma le vendite per regione e per mese. Non devi scrivere alcuna formula. Scegli quali colonne dell'elenco vanno sul lato sinistro, quali in alto e quale va sommata: il rapporto si compila da solo.

## Un esempio

Ecco un elenco nelle celle da A1 a C7. Ogni riga è una vendita.

| Regione | Mese | Importo |
|---|---|---|
| Est | Gen | 10 |
| Est | Gen | 5 |
| Est | Feb | 20 |
| Ovest | Gen | 7 |
| Ovest | Feb | 3 |
| Ovest | Feb | 4 |

Una tabella pivot con **Regione** sul lato sinistro, **Mese** in alto e la somma di **Importo** al centro appare così.

| Somma di Importo | Mese | | |
|---|---|---|---|
| Regione | Gen | Feb | Totale complessivo |
| Est | 15 | 20 | 35 |
| Ovest | 7 | 7 | 14 |
| Totale complessivo | 22 | 27 | 49 |

Est a gennaio vale 15 perché si sommano le due righe di Est di gennaio, 10 e 5. La colonna Totale complessivo somma ogni riga, la riga Totale complessivo somma ogni colonna, e la cella in alto a sinistra somma tutto.

I termini usati qui:

- **Campo** è una colonna dell'elenco. Regione, Mese e Importo sono i campi.
- **Righe**, **Colonne** e **Valori** sono i tre posti in cui può andare un campo. Un campo nelle Righe produce una riga per ciascuno dei suoi valori diversi; un campo nelle Colonne produce una colonna per ciascuno; un campo nei Valori è quello che viene sommato. Un quarto posto, **Filtri**, contiene un campo che esclude le righe che non scegli.
- Una **cache** (la cache della tabella pivot) è una copia del tuo elenco che la tabella pivot conserva all'interno del file, così da poter mostrare i totali senza leggere le celle. Il rapporto viene calcolato a partire dalla cache, non dalle celle, quindi non cambia quando cambiano le celle. **Aggiorna** copia di nuovo le celle nella cache.

## Creare una tabella pivot

1. Fai clic su una cella dell'elenco, oppure seleziona le celle che vuoi usare. L'elenco deve avere una riga di nomi sopra i dati.
2. Scegli **Inserisci**, poi **Tabella pivot**.
3. La finestra mostra le celle in cui si trovano i dati. Modificale se non sono corrette. Puoi anche digitare il nome di una tabella.
4. Scegli dove va la tabella pivot: **Nuovo foglio** (l'impostazione predefinita), oppure una cella di questo foglio o di un altro. Scegli un punto in cui non c'è nient'altro, perché la tabella vuota occupa circa diciotto righe e tre colonne.
5. Scegli **OK**. La tabella inizia vuota e si apre il riquadro **Campi tabella pivot**. Seleziona i campi che ti servono: un campo di numeri viene sommato, mentre ogni altro campo diventa una riga. Poi spostali nelle caselle come spiegano le sezioni seguenti.

Il nuovo foglio è un passaggio separato di **Annulla**, e così lo è la tabella. Quando salvi in .xlsx, la tabella viene scritta nel modo in cui la scrive il programma originale, quindi si apre lì come tabella pivot. Una copia salvata in .xls o .ods conserva nelle sue celle i numeri che la tabella mostra, ma non la tabella stessa, e il salvataggio te lo segnala.

## GETPIVOTDATA: leggere un numero da una tabella pivot

`GETPIVOTDATA` restituisce il numero che una tabella pivot mostra per i valori, le righe e le colonne che indichi. Per l'esempio sopra:

`=GETPIVOTDATA("Somma di Importo"; $E$3; "Regione"; "Est"; "Mese"; "Gen")` restituisce 15.

- Il primo argomento è il nome del valore, come lo mostra la tabella (per esempio Somma di Importo). Funziona anche il nome del campo che viene sommato (Importo).
- Il secondo argomento è una cella qualsiasi della tabella pivot.
- Seguono coppie formate da un campo e da uno dei suoi elementi. Se indichi solo Regione ed Est, ottieni il totale di Est, 35. Se non indichi nulla, ottieni il totale complessivo, 49. I campi che indichi devono essere i primi campi delle righe, oppure i primi campi delle colonne, nell'ordine in cui compaiono.
- Se la tabella non mostra il numero richiesto, la cella mostra `#RIF!`.

Quando, mentre scrivi una formula, fai clic su un valore di una tabella pivot, JeffCrunch scrive questa formula al posto tuo, così continua a leggere il numero giusto anche dopo che la tabella viene riorganizzata. Per disattivarlo, scegli **Opzioni** nella scheda **Tabella pivot** e deseleziona la casella di controllo relativa a GETPIVOTDATA.

## Che cosa fa JeffCrunch con una tabella pivot

Una tabella pivot in una cartella di lavoro viene mostrata come è stata salvata. Se non modifichi nulla, il file viene salvato con la tabella pivot esattamente com'era. Quando la aggiorni o la modifichi, JeffCrunch calcola di nuovo il rapporto, inserisce i nuovi numeri nelle celle e riscrive la tabella pivot in modo che il programma che l'ha creata la apra senza problemi.

JeffCrunch calcola le tabelle pivot che sommano, contano, calcolano medie e così via, che raggruppano date e numeri e che mostrano i valori come percentuale, differenza o totale parziale. Una tabella pivot che contiene un campo o un elemento calcolato, che mostra un valore come classifica o come percentuale di un elemento padre, che ha gruppi di elementi creati a mano oppure che usa un modello di dati viene mostrata e conservata come salvata, e i comandi ti avvisano che non possono modificarla.

Fai clic su una cella qualsiasi di una tabella pivot. Compare la scheda **Tabella pivot**.

## Aggiornare

**Aggiorna** legge le celle da cui è composta la tabella pivot e calcola di nuovo il rapporto. I nuovi elementi si aggiungono dopo quelli già presenti; gli elementi che non ci sono più vengono eliminati. Le larghezze delle colonne e il layout che hai scelto restano invariati. **Aggiorna tutto**, nella scheda **Dati** e nella scheda **Tabella pivot**, fa lo stesso per tutte le tabelle pivot della cartella di lavoro.

Se il rapporto dovesse allargarsi su celle che contengono già dei dati, JeffCrunch non aggiorna la tabella e ti indica quali celle sono di intralcio. Cancellale o sposta la tabella pivot, poi aggiorna di nuovo.

**Annulla** annulla un aggiornamento con un solo passaggio.

## Cambia origine dati

**Cambia origine dati** chiede le celle in cui si trova l'elenco, compresa la prima riga con i nomi. Digitale nella forma `Foglio1!A1:D100`, oppure digita il nome di una tabella o di un nome che hai creato. JeffCrunch legge le celle e calcola di nuovo il rapporto. Se un campo dell'elenco ha lo stesso nome nelle nuove celle, il suo posto nel rapporto resta invariato.

## Cancella

**Cancella** ha due scelte. **Cancella filtri** rimuove tutti i filtri e mostra di nuovo tutti gli elementi. **Cancella tutto** toglie tutti i campi dal rapporto e lascia la tabella vuota, pronta per essere costruita di nuovo.

## Approfondire e ridurre il livello

Quando un rapporto ha più di un campo nelle righe, un elemento del primo campo può essere aperto o chiuso. Fai clic sulla cella con il nome dell'elemento e scegli **Riduci livello** per chiuderlo: le righe del campo successivo che stanno sotto di lui vengono nascoste e resta solo il suo totale. **Approfondisci** lo riapre. Un elemento dell'ultimo campo non ha nulla sotto di sé. Ciascuna di queste azioni si annulla con un passaggio di **Annulla**.

## Selezionare e spostare

**Seleziona**, nella scheda **Tabella pivot**, seleziona una parte del rapporto: **Intera tabella pivot**, **Valori** (i numeri) oppure **Etichette** (i nomi sul lato sinistro; se il rapporto non ne ha, i nomi in alto). **Sposta tabella pivot…** chiede una cella dello stesso foglio e ridisegna il rapporto con la sua prima cella in quel punto. Le celle che lascia vengono svuotate. Le formule che puntano alle celle del rapporto le seguono, come accade quando tagli e incolli. Se il nuovo punto contiene già dei dati, JeffCrunch non sposta il rapporto e te lo segnala. Un solo **Annulla** lo riporta indietro.

## Il riquadro dei campi

**Elenco campi**, nella scheda **Tabella pivot**, apre il riquadro **Campi tabella pivot**. Mostra i campi dell'elenco, con una casella accanto a ciascuno, e sotto di essi i quattro posti: **Filtri**, **Colonne**, **Righe** e **Valori**.

- Seleziona un campo per aggiungerlo. Un campo che contiene solo numeri va nei **Valori** e viene sommato; qualsiasi altro va nelle **Righe**.
- Deseleziona un campo per toglierlo da tutti i posti.
- Fai clic su un campo in uno dei quattro posti e scegli **Sposta in** per metterlo in un altro. Le voci **Sposta su** e **Sposta giù** ne cambiano l'ordine nel suo posto, e **Rimuovi campo** lo toglie.
- Con più di un valore, nelle **Colonne** compare una riga chiamata **Valori**. Spostala nelle **Righe** per mettere i valori sul lato sinistro.
- **Impostazioni campo valore** (doppio clic su un valore, oppure il pulsante) imposta il nome che il rapporto mostra per il valore e il modo in cui viene totalizzato: **Somma**, **Conteggio**, **Conta numeri**, **Media**, **Max**, **Min**, **Prodotto**, **Dev.std (campione)** e **Dev.std.pop (tutti i valori)**, **Var (campione)** e **Var.pop (tutti i valori)**. Imposta anche il modo in cui il valore viene mostrato e il suo formato numero. Vedi la sezione seguente.
- **Impostazioni campo** (doppio clic su un campo nei Filtri, nelle Colonne o nelle Righe, oppure il pulsante) imposta un solo campo. Vedi "Impostazioni campo".

Ogni modifica è un passaggio di **Annulla**.

## Mostra valori come, e formato numero di un valore

In **Impostazioni campo valore**, **Mostra valori come** cambia ciò che mostra una cella del rapporto.

- **% del totale complessivo**: la cella divisa per il totale di tutto il rapporto. Nell'esempio, Est a gennaio è 15 su 49, cioè 30,6%.
- **% del totale colonna** e **% del totale riga**: la cella divisa per il totale della sua colonna o della sua riga.
- **Indice**: il confronto tra la cella e ciò che la sua riga e la sua colonna darebbero se i dati fossero distribuiti in modo uniforme. Un numero superiore a 1 indica più della quota uniforme.
- **Differenza da**, **% di** e **% differenza da**: la cella confrontata con un altro elemento di un campo. Scegli il **Campo base** (un campo nelle Righe o nelle Colonne) e l'**Elemento base** (uno dei suoi elementi, oppure quello precedente o successivo). Per esempio, con Mese come campo base e Gen come elemento base, febbraio mostra quanto è maggiore o minore di gennaio. L'elemento base stesso resta vuoto per una differenza, e vale 100% per **% di**.
- **Totale parziale in**: ogni cella somma le celle che la precedono lungo il campo base.

**Formato numero** accetta un codice di formato, come `0,00`, `#.##0` o `0,0%`. Lascialo vuoto per mantenere il formato che hanno le celle. Un valore mostrato come percentuale riceve `0,00%`, a meno che tu non scelga un altro formato. Le celle del rapporto assumono subito il formato. **Annulla** lo riporta indietro insieme ai numeri.

## Impostazioni campo

Seleziona un campo nella casella **Filtri**, **Colonne** o **Righe** e scegli **Impostazioni campo** nella scheda **Tabella pivot** (oppure fai doppio clic su di esso).

- **Nome personalizzato** cambia il nome che il rapporto mostra per il campo.
- **Subtotali**: **Automatico** mostra un totale per ciascun gruppo del campo, **Nessuno** non lo mostra.
- **Mostra etichette elementi in**: formato compatto, struttura o tabulare, come in **Layout report**.
- **Inserisci una riga vuota dopo ogni elemento** mette una riga vuota dopo ciascun gruppo.
- **Ripeti etichette elementi (formato struttura e tabella)** mostra il nome di un elemento esterno su ogni riga del suo gruppo.
- **Ordina**: mantieni l'ordine in cui si trovano gli elementi (manuale), oppure ordina dalla A alla Z o dalla Z alla A per etichetta. **Dal più piccolo al più grande per valore** e **Dal più grande al più piccolo per valore** ordinano gli elementi in base ai numeri di un campo valore. Il rapporto mantiene l'ordinamento quando cambiano i dati.
- **Filtro** esclude degli elementi in base alle loro etichette (inizia con, termina con, contiene, uguale a) o in base a un valore (maggiore di, minore di, compreso tra e così via), oppure mantiene solo i primi o gli ultimi elementi in base a un valore. Il filtro viene applicato quando scegli OK. Nasconde gli elementi esclusi; le caselle sotto mostrano quali sono, e puoi selezionare un elemento per mostrarlo di nuovo.
- **Mostra questi elementi** è un elenco degli elementi del campo, ciascuno con una casella. Deseleziona una casella per nascondere l'elemento, e i totali non lo considereranno.

## Raggruppare date e numeri

Seleziona un campo di date o di numeri nelle **Righe** o nelle **Colonne** e scegli **Raggruppa campo** nella scheda **Tabella pivot**. I gruppi si creano dai dati, quindi in quella colonna devono esserci solo numeri o solo date, senza testo e senza celle vuote.

- **Date**: seleziona come raggruppare: **Secondi**, **Minuti**, **Ore**, **Giorni**, **Mesi**, **Trimestri** o **Anni**. La parte più fine che selezioni resta nel campo. Per ogni parte più grossolana JeffCrunch aggiunge un campo, che porta il suo nome (per esempio **Anni**), e lo inserisce prima del campo data, nello stesso posto. **Inizia da** e **Termina a** sono la prima e l'ultima data; lasciali così come sono per usare la data minima e massima dei dati. Le date precedenti all'inizio o successive alla fine vanno in due gruppi aggiuntivi, che iniziano con `<` e `>`.
- **Numeri**: **Inizia da**, **Termina a** e **Per** creano gruppi come 1-10, 11-20 e così via.

**Separa** elimina i gruppi e i campi aggiunti per essi. Aggiorna ricrea i gruppi a partire dai nuovi dati. Quando due tabelle pivot sono create dagli stessi dati, non è possibile creare gruppi, perché un gruppo è una parte dei dati che esse condividono.

## Mostrare le righe dietro un valore

Fai doppio clic su un valore di una tabella pivot, oppure scegli **Mostra dettagli** nella scheda **Tabella pivot**. JeffCrunch crea un nuovo foglio, chiamato Dettaglio 1, 2 e così via, con le righe dell'elenco che compongono quel numero. I nomi dei campi si trovano nella prima riga. F2 modifica ancora la cella, se la cella è modificabile. **Annulla** elimina le righe, e un altro **Annulla** elimina il foglio.

## Espandere e comprimere

Un campo i cui elementi hanno altri elementi sotto di sé può mostrare o nascondere le righe sotto i suoi elementi. **Espandi intero campo** e **Comprimi intero campo** lo fanno per tutti gli elementi del campo che selezioni nel riquadro.

## Opzioni tabella pivot

**Opzioni**, nella scheda **Tabella pivot**, apre una finestra con le impostazioni della tabella.

- **Nome** è il nome della tabella pivot. Nessun'altra tabella pivot della cartella di lavoro può avere lo stesso nome.
- **Per valori di errore mostra** e **Per celle vuote mostra** inseriscono un testo a tua scelta in quelle celle.
- **Unisci e centra celle con etichette**, **Adatta larghezza colonne all'aggiornamento della tabella** e **Mantieni la formattazione delle celle all'aggiornamento della tabella** vengono salvate con la tabella.
- **Mostra totali generali per righe** e **per colonne** corrispondono alle scelte di Totali complessivi.
- **Aggiorna i dati all'apertura del file** legge di nuovo le celle ogni volta che si apre la cartella di lavoro. È un passaggio di Annulla, come ogni aggiornamento.

## Stile: intestazioni, bande e galleria

Le **Opzioni stile**, nella scheda **Tabella pivot**, attivano o disattivano la riga di intestazione, la prima colonna (intestazioni di riga) e le bande di righe e di colonne. **Stili** ha una galleria di aspetti, e **Cancella stile** toglie l'aspetto. Questi aspetti sono disegnati come quelli delle tabelle. **Righe vuote** e **Ripeti etichette** impostano la riga vuota e le etichette ripetute per tutti i campi delle righe in una sola volta.

## Struttura: subtotali, totali complessivi e layout

- **Subtotali** mostra o nasconde il totale di ciascun gruppo, in alto o in basso rispetto al gruppo. Quando nelle Righe ci sono più campi, il totale di ciascun campo esterno è un subtotale.
- **Totali complessivi** offre quattro scelte: disattivati per righe e colonne, attivati per entrambi, attivati solo per le righe (una colonna di totali a destra, uno per ogni riga) e attivati solo per le colonne (una riga di totali in basso).
- **Layout report** è la forma delle etichette. In formato **compatto**, tutti i campi delle Righe condividono una sola colonna, e un nuovo gruppo viene mostrato un po' più a destra. In formato **struttura**, ogni campo ha una sua colonna, e ogni gruppo ha una riga propria sopra i suoi elementi. In formato **tabulare**, ogni campo ha una sua colonna e il nome del gruppo sta sulla sua prima riga, con il subtotale sotto il gruppo.

## Lo stesso elenco, con un filtro

Sposta **Mese** dalle **Colonne** ai **Filtri**. Il rapporto perde le sue colonne, e sopra di esso compare una riga con **Mese** e **(Tutti)**. I numeri sono ora il totale di ciascuna regione su tutti i mesi.

## Grafico pivot

Fai clic su una cella di una tabella pivot e scegli **Inserisci**, poi **Grafico pivot**. JeffCrunch crea un grafico a colonne dei valori della tabella. Esclude i totali complessivi. Il grafico viene segnato nel file come grafico della tabella pivot. Legge le celle della tabella, quindi i suoi numeri cambiano quando aggiorni la tabella o ne modifichi il layout. Se la tabella ottiene più righe o più colonne, crea di nuovo il grafico, perché il grafico conserva le celle da cui è stato creato. Un grafico pivot proveniente da un file viene mostrato come è stato salvato.

## Sezioni dati e sequenze temporali

Una sezione dati è un insieme di pulsanti che filtra una tabella pivot, mentre una sequenza temporale è un cursore che fa lo stesso per le date.

Fai clic su una cella di una tabella pivot, poi fai clic su **Sezioni dati** nella scheda Tabella pivot. Si apre il riquadro **Sezioni dati», che elenca le sezioni dati e le sequenze temporali della cartella di lavoro. Ogni sezione dati ha un pulsante per ciascun elemento del suo campo. I pulsanti blu sono gli elementi che la tabella mostra.

Fai clic su un pulsante per mostrare solo quell'elemento. Fai di nuovo clic per mostrare tutti gli elementi. Tieni premuto **Ctrl** e fai clic per aggiungere un elemento a quelli mostrati, oppure per toglierne uno. Ogni clic è un passaggio di Annulla, e Annulla riporta la tabella com'era. Quando salvi, il file conserva la scelta della sezione dati.

**Connessioni report** indica quali tabelle pivot filtra una sezione dati. Una sequenza temporale mostra l'intervallo di date che ha scelto. JeffCrunch mostra le sezioni dati in questo riquadro, non sopra le celle, e non modifica ancora una sequenza temporale. Entrambe vengono conservate nel file come sono state salvate.

## Limiti

Una tabella pivot può avere fino a 2.000 campi, un elenco fino a cinque milioni di righe e un rapporto fino a quattro milioni di celle. Un file che supera questi limiti viene mostrato come salvato e non può essere modificato. La rilettura di centomila righe di venti colonne e il calcolo del rapporto hanno richiesto circa mezzo secondo sul computer su cui è stato misurato; un computer più lento impiega di più.
