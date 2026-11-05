# Tabelle pivot
<!-- keywords: pagine filtro report, tabella pivot, approfondisci, riduci livello, inserisci tabella pivot, crea, crea tabella pivot, seleziona, sposta tabella pivot, GETPIVOTDATA, genera GETPIVOTDATA, aggiorna, aggiorna tutto, cambia origine dati, elenco campi, campi, righe, colonne, valori, filtri, filtro report, totale generale, subtotale, layout, compatto, struttura, tabulare, cancella, impostazioni campo valore, somma, conteggio, media, mostra valori come, percentuale del totale, differenza, totale parziale, formato numero, impostazioni campo, ordina, filtra, primi, raggruppa, separa, date, mesi, anni, espandi, comprimi, opzioni, stile, righe a bande, righe vuote, ripeti etichette, aggiorna all'apertura del file, grafico pivot, segmentazione dati, sequenza temporale, mostra dettagli, approfondisci, righe dietro un valore -->

Una tabella pivot è un piccolo rapporto ricavato da un lungo elenco. L'elenco ha una riga per ogni vendita, per esempio; il rapporto somma le vendite per area e per mese. Non devi scrivere alcuna formula. Indichi quali colonne dell'elenco vanno sul lato, quali in alto e quale va sommata, e il rapporto si compila da solo.

## Un esempio

Ecco un elenco nelle celle da A1 a C7. Ogni riga è una vendita.

| Area | Mese | Importo |
|---|---|---|
| Est | Gen | 10 |
| Est | Gen | 5 |
| Est | Feb | 20 |
| Ovest | Gen | 7 |
| Ovest | Feb | 3 |
| Ovest | Feb | 4 |

Una tabella pivot con **Area** sul lato, **Mese** in alto e la somma di **Importo** al centro appare così.

| Somma di Importo | Mese | | |
|---|---|---|---|
| Area | Gen | Feb | Totale generale |
| Est | 15 | 20 | 35 |
| Ovest | 7 | 7 | 14 |
| Totale generale | 22 | 27 | 49 |

Est a gennaio vale 15 perché si sommano le due righe di Est di gennaio, 10 e 5. La colonna **Totale generale** somma ogni riga, la riga **Totale generale** somma ogni colonna, e la cella d'angolo somma tutto.

I termini usati qui:

- **Campo** è una colonna dell'elenco. Area, Mese e Importo sono campi.
- **Righe**, **Colonne** e **Valori** sono i tre posti in cui può andare un campo. Un campo in Righe genera una riga per ciascuno dei suoi valori diversi; un campo in Colonne genera una colonna per ciascuno; un campo in Valori è quello che viene sommato. Un quarto posto, **Filtri**, contiene un campo che esclude le righe che non scegli.
- Una **cache** (la cache della tabella pivot) è una copia del tuo elenco che la tabella pivot conserva nel file, così può mostrare i totali senza leggere le celle. Il rapporto viene calcolato dalla cache, non dalle celle, quindi non cambia quando cambiano le celle. **Aggiorna** copia di nuovo le celle nella cache.

## Crea una tabella pivot

1. Fai clic su una cella dell'elenco, oppure seleziona le celle che vuoi usare. L'elenco deve avere una riga di nomi sopra i dati.
2. Scegli **Inserisci**, quindi **Tabella pivot**.
3. La finestra mostra le celle in cui si trovano i dati. Modificale se non sono corrette. Puoi anche digitare il nome di una tabella.
4. Scegli dove va la tabella pivot: **Nuovo foglio** (impostazione predefinita), oppure una cella di questo foglio o di un altro. Scegli un punto libero, perché la tabella vuota occupa circa diciotto righe e tre colonne.
5. Scegli **OK**. La tabella parte vuota e si apre il riquadro **Campi tabella pivot**. Seleziona i campi che ti servono: un campo di numeri viene sommato, un altro campo diventa una riga. Poi spostali nelle caselle come spiegano le sezioni seguenti.

Un nuovo foglio costituisce un passaggio di **Annulla** a sé stante, e così la tabella. Quando salvi in formato .xlsx, la tabella viene scritta nel modo in cui la scrive il programma originale, così si apre lì come tabella pivot. Una copia salvata in formato .xls o .ods conserva nelle sue celle i numeri che la tabella mostra, ma non la tabella stessa, e il salvataggio te lo segnala.

## GETPIVOTDATA: leggere un numero da una tabella pivot

`GETPIVOTDATA` restituisce il numero che una tabella pivot mostra per i valori, le righe e le colonne che indichi. Per l'esempio sopra:

`=GETPIVOTDATA("Somma di Importo"; $E$3; "Area"; "Est"; "Mese"; "Gen")` restituisce 15.

- Il primo argomento è il nome del valore, come lo mostra la tabella (per esempio Somma di Importo). Funziona anche il nome del campo che viene sommato (Importo).
- Il secondo argomento è una qualsiasi cella della tabella pivot.
- Seguono coppie di un campo e di uno dei suoi elementi. Se indichi solo Area ed Est, ottieni il totale di Est, 35. Se non indichi nulla, ottieni il totale generale, 49. I campi che indichi devono essere i primi campi delle righe, o i primi campi delle colonne, in ordine.
- Se la tabella non mostra il numero, la cella visualizza `#RIF!`.

Quando fai clic su un valore di una tabella pivot mentre digiti una formula, JeffCrunch scrive questa formula al posto tuo, così continua a leggere il numero giusto anche dopo che la tabella è stata riorganizzata. Per disattivarla, scegli **Opzioni** nella scheda **Tabella pivot** e deseleziona la casella relativa a GETPIVOTDATA.

## Che cosa fa JeffCrunch con una tabella pivot

Una tabella pivot in una cartella di lavoro viene mostrata come era stata salvata. Se non cambi nulla, il file viene salvato con la tabella pivot esattamente com'era. Quando la aggiorni o la modifichi, JeffCrunch ricalcola il rapporto, inserisce i nuovi numeri nelle celle e riscrive la tabella pivot in modo che il programma che l'ha creata la apra senza problemi.

JeffCrunch calcola le tabelle pivot che sommano, contano, calcolano medie e così via, che raggruppano date e numeri, e che mostrano i valori come percentuale, differenza o totale parziale. Una tabella pivot che ha un campo o un elemento calcolato, che mostra un valore come posizione o come percentuale di un elemento padre, che ha gruppi di elementi creati a mano o che usa un modello di dati viene mostrata e conservata come salvata, e i comandi ti avvisano che non possono modificarla.

Fai clic su una cella qualsiasi di una tabella pivot. Compare la scheda **Tabella pivot**.

## Aggiorna

**Aggiorna** legge le celle da cui è ricavata la tabella pivot e ricalcola il rapporto. Le nuove voci si aggiungono dopo quelle già presenti; le voci che non ci sono più vengono eliminate. Le larghezze delle colonne e la disposizione scelta restano invariate. **Aggiorna tutto**, nella scheda **Dati** e nella scheda **Tabella pivot**, fa lo stesso per tutte le tabelle pivot della cartella di lavoro.

Se il rapporto dovesse allargarsi sopra celle che contengono qualcosa, JeffCrunch non lo aggiorna e ti indica quali celle sono di intralcio. Svuotale o sposta la tabella pivot, quindi aggiorna di nuovo.

**Annulla** annulla un aggiornamento con un solo passaggio.

## Cambia origine dati

**Cambia origine dati** chiede le celle in cui si trova l'elenco, compresa la prima riga dei nomi. Digitale nella forma `Foglio1!A1:D100`, oppure digita il nome di una tabella o di un nome che hai creato. JeffCrunch legge le celle e ricalcola il rapporto. Se un campo dell'elenco ha lo stesso nome nelle nuove celle, la sua posizione nel rapporto resta invariata.

## Cancella

**Cancella** offre due scelte. **Cancella filtri** rimuove tutti i filtri e mostra di nuovo tutti i valori. **Cancella tutto** toglie tutti i campi dal rapporto e lo lascia vuoto, pronto per essere costruito di nuovo.

## Approfondisci e riduci livello

## Mostra pagine filtro report

Un filtro sta sopra una tabella pivot e ti permette di vedere un elemento alla volta, per esempio un mese. **Mostra pagine filtro report**, nella scheda **Tabella pivot**, crea una pagina per ogni elemento contemporaneamente. Ogni pagina è un nuovo foglio, denominato con il nome dell'elemento, con una propria copia della tabella. Il filtro di ciascuna copia è impostato su quell'elemento. Se la tabella ha più di un filtro, scegli quale usare. Un solo **Annulla** elimina tutte le pagine.

Quando un rapporto ha più di un campo nelle righe, un elemento del primo campo può essere aperto o chiuso. Fai clic sulla cella con il nome dell'elemento e scegli **Riduci livello** per chiuderlo: le righe del campo successivo sotto di lui vengono nascoste e resta visibile solo il suo totale. **Approfondisci** lo riapre. Un elemento dell'ultimo campo non ha nulla sotto di sé. Ogni operazione corrisponde a un **Annulla**.

## Seleziona e sposta

**Seleziona**, nella scheda **Tabella pivot**, seleziona una parte del rapporto: **Intera tabella pivot**, **Valori** (i numeri), oppure **Etichette** (i nomi sul lato; se il rapporto non ha nomi sul lato, i nomi in alto). **Sposta tabella pivot…** chiede una cella dello stesso foglio e disegna di nuovo il rapporto con la sua prima cella in quel punto. Le celle che il rapporto lascia vuote vengono svuotate. Le formule che puntano alle celle del rapporto le seguono, come avviene quando tagli e incolli. Se il nuovo posto contiene già qualcosa, JeffCrunch non sposta il rapporto e te lo segnala. Un solo **Annulla** lo riporta indietro.

## Il riquadro dei campi

**Elenco campi**, nella scheda **Tabella pivot**, apre il riquadro **Campi tabella pivot**. Elenca i campi dell'elenco con una casella accanto a ciascuno e, sotto, i quattro posti: **Filtri**, **Colonne**, **Righe** e **Valori**.

- Seleziona un campo per aggiungerlo. Un campo che contiene solo numeri va in **Valori** ed è sommato; ogni altro finisce in **Righe**.
- Deseleziona un campo per toglierlo da tutte le posizioni.
- Fai clic su un campo in una delle quattro posizioni e scegli **Sposta in** per metterlo in un'altra. Le voci **Sposta su** e **Sposta giù** cambiano il suo ordine nella posizione, e **Rimuovi campo** lo elimina.
- Con più di un valore, in **Colonne** compare una riga chiamata **Valori**. Spostala in **Righe** per mettere i valori sul lato.
- **Impostazioni campo valore** (doppio clic su un valore, oppure il pulsante) imposta il nome che il rapporto mostra per il valore e come viene calcolato: **Somma**, **Conteggio**, **Conteggio numeri**, **Media**, **Max**, **Min**, **Prodotto**, **DevStd** e **DevStdp** (la deviazione standard di un campione, o di tutti i valori), e **Var** e **Varp** (la varianza degli stessi). Imposta anche il modo in cui il valore viene mostrato e il suo formato numero. Vedi la sezione successiva.
- **Impostazioni campo** (doppio clic su un campo in Filtri, Colonne o Righe, oppure il pulsante) imposta un solo campo. Vedi "Impostazioni campo".

Ogni modifica corrisponde a un passaggio di **Annulla**.

## Mostra valori come, e il formato numero di un valore

In **Impostazioni campo valore**, **Mostra valori come** cambia ciò che mostra una cella del rapporto.

- **% del totale generale**: la cella divisa per il totale di tutto il rapporto. Nell'esempio, Est a gennaio è 15 su 49, cioè il 30,6%.
- **% del totale di colonna** e **% del totale di riga**: la cella divisa per il totale della sua colonna o della sua riga.
- **Indice**: il confronto tra la cella e ciò che la sua riga e la sua colonna darebbero se i dati fossero distribuiti in modo uniforme. Un numero sopra 1 indica più della quota uniforme.
- **Differenza da**, **% di** e **% differenza da**: la cella confrontata con un altro elemento di un campo. Scegli il **Campo base** (un campo in Righe o in Colonne) e l'**Elemento base** (uno dei suoi elementi, oppure quello precedente o successivo). Per esempio, con Mese come campo base e Gen come elemento base, Feb mostra quanto è più alto o più basso di gennaio. L'elemento base stesso resta vuoto per una differenza, e vale 100% per **% di**.
- **Totale parziale in**: ogni cella somma le celle che la precedono lungo il campo base.
- **Classifica dal più piccolo al più grande** e **Classifica dal più grande al più piccolo**: ogni cella mostra la sua posizione, 1, 2, 3 e così via, tra le celle del campo base della stessa riga o colonna. Il posto 1 è il numero più piccolo, oppure il più grande.
- **% del totale riga padre** e **% del totale colonna padre**: ogni cella mostra quale parte del suo padre rappresenta. Il padre di un elemento è l'elemento un livello sopra, per esempio l'anno che contiene un mese. Un elemento senza padre viene mostrato come parte del totale generale.

**Formato numero** accetta un codice di formato, come `0,00`, `#.##0` o `0,0%`. Lascialo vuoto per mantenere il formato che hanno le celle. Un valore mostrato come percentuale riceve `0,00%`, a meno che tu non scelga un altro formato. Le celle del rapporto assumono subito il formato. **Annulla** lo ripristina insieme ai numeri.

## Impostazioni campo

Seleziona un campo nella casella **Filtri**, **Colonne** o **Righe** e scegli **Impostazioni campo** nella scheda **Tabella pivot** (oppure fai doppio clic).

- **Nome personalizzato** cambia il nome che il rapporto mostra per il campo.
- **Subtotali**: **Automatici** mostra un totale per ogni gruppo del campo, **Nessuno** non lo mostra.
- **Mostra etichette elementi in**: formato compatto, struttura o tabulare, come in Layout rapporto.
- **Inserisci una riga vuota dopo ogni elemento** aggiunge una riga vuota dopo ogni gruppo.
- **Ripeti etichette elementi** mostra il nome di un elemento esterno su ogni riga del suo gruppo (formato struttura e tabulare).
- **Ordina**: mantieni l'ordine in cui si trovano gli elementi (manuale), oppure ordina dalla A alla Z o dalla Z alla A in base alle etichette. **Dal più piccolo al più grande per valore** e **Dal più grande al più piccolo per valore** ordinano gli elementi in base ai numeri di un campo valore. Il rapporto mantiene l'ordinamento quando cambiano i dati.
- **Filtro** esclude elementi in base alle etichette (inizia con, termina con, contiene, uguale a) o in base a un valore (maggiore di, minore di, compreso tra e così via), oppure mantiene solo i primi o gli ultimi elementi secondo un valore. Il filtro viene applicato quando scegli OK. Nasconde gli elementi esclusi; le caselle sottostanti indicano quali sono, e puoi selezionare un elemento per mostrarlo di nuovo.
- **Mostra questi elementi** è un elenco degli elementi del campo, ciascuno con una casella. Deseleziona una casella per nascondere l'elemento, e i totali lo escluderanno.

## Raggruppa date e numeri

Seleziona un campo di date o di numeri in **Righe** o in **Colonne** e scegli **Raggruppa campo** nella scheda **Tabella pivot**. I gruppi vengono creati dai dati, quindi in quella colonna devono esserci solo numeri o solo date, senza testo e senza celle vuote.

- **Date**: seleziona i criteri di raggruppamento: **Secondi**, **Minuti**, **Ore**, **Giorni**, **Mesi**, **Trimestri** o **Anni**. La parte più fine che selezioni resta nel campo. Per ogni parte più grossolana JeffCrunch aggiunge un campo, che porta il suo nome (per esempio **Anni**), e lo inserisce prima del campo data, nello stesso posto. **Inizia da** e **Termina a** sono la prima e l'ultima data; lasciali così per usare la data minima e massima dei dati. Le date precedenti l'inizio o successive alla fine finiscono in due gruppi aggiuntivi, che iniziano con `<` e `>`.
- **Numeri**: **Inizia da**, **Termina a** e **Con passo** creano gruppi come 1-10, 11-20 e così via.

**Separa** elimina i gruppi e i campi aggiunti per essi. Aggiornando, i gruppi vengono creati di nuovo sui nuovi dati. Quando due tabelle pivot derivano dagli stessi dati, non è possibile creare gruppi, perché un gruppo è una parte dei dati che condividono.

## Mostra le righe dietro un valore

Fai doppio clic su un valore di una tabella pivot, oppure scegli **Mostra dettagli** nella scheda **Tabella pivot**. JeffCrunch crea un nuovo foglio, denominato Dettaglio 1, 2 e così via, con le righe dell'elenco che compongono il numero. I nomi dei campi si trovano nella prima riga. F2 modifica ancora la cella, se la cella è modificabile. **Annulla** elimina le righe, e un altro **Annulla** elimina il foglio.

## Espandi e comprimi

Un campo i cui elementi hanno altri elementi sotto di sé può mostrare o nascondere le righe sotto i suoi elementi. **Espandi intero campo** e **Comprimi intero campo** lo fanno per ogni elemento del campo selezionato nel riquadro.

## Opzioni tabella pivot

**Opzioni** nella scheda **Tabella pivot** apre una finestra con le impostazioni della tabella.

- **Nome** è il nome della tabella pivot. Nessun'altra tabella pivot della cartella di lavoro può avere lo stesso nome.
- **Per valori di errore mostra** e **Per celle vuote mostra** inseriscono un testo a tua scelta in quelle celle.
- **Unisci e allinea al centro le celle con etichette**, **Adatta larghezza colonne all'aggiornamento della tabella** e **Mantieni la formattazione delle celle all'aggiornamento della tabella** vengono salvate con la tabella.
- **Mostra totali generali per righe** e **per colonne** sono le stesse opzioni di Totali generali.
- **Aggiorna i dati all'apertura del file** rilegge le celle ogni volta che la cartella di lavoro viene aperta. È un passaggio di Annulla, come qualsiasi aggiornamento.

## Stile: intestazioni, bande e raccolta

Le **Opzioni stile**, nella scheda **Tabella pivot**, attivano o disattivano la riga di intestazione, la prima colonna (intestazioni di riga) e le bande di righe e di colonne. **Stili** offre una raccolta di aspetti, e **Cancella stile** elimina l'aspetto. Questi aspetti vengono disegnati allo stesso modo degli stili delle tabelle. **Righe vuote** e **Ripeti etichette** impostano la riga vuota e le etichette ripetute per tutti i campi delle righe in una sola volta.

## Struttura: subtotali, totali generali e layout

- **Subtotali** mostra o nasconde il totale di ogni gruppo, in alto o in basso nel gruppo. Quando nelle Righe ci sono più campi, il totale di ogni campo esterno è un subtotale.
- **Totali generali** offre quattro scelte: disattivati per righe e colonne, attivati per entrambe, attivati solo per le righe (una colonna di totali a destra, uno per ogni riga), e attivati solo per le colonne (una riga di totali in basso).
- **Layout rapporto** è la forma delle etichette. In formato **compatto**, tutti i campi delle Righe condividono una colonna, e un nuovo gruppo viene mostrato un po' più a destra. In formato **struttura**, ogni campo ha una colonna propria, e ogni gruppo ha una riga tutta sua sopra i suoi elementi. In formato **tabulare**, ogni campo ha una colonna propria e il nome del gruppo si trova sulla sua prima riga, con il subtotale sotto il gruppo.

## Lo stesso elenco, con un filtro

Sposta **Mese** da **Colonne** a **Filtri**. Il rapporto perde le sue colonne, e una riga sopra di esso mostra **Mese** e **(Tutti)**. I numeri sono ora il totale di ciascuna area su tutti i mesi.

## Grafico pivot

Fai clic su una cella di una tabella pivot e scegli **Inserisci**, quindi **Grafico pivot**. JeffCrunch crea un grafico a colonne dei valori della tabella. Esclude i totali generali. Il grafico è contrassegnato nel file come grafico della tabella pivot. Legge le celle della tabella, quindi i suoi numeri cambiano quando aggiorni la tabella o ne cambi il layout. Se la tabella acquisisce altre righe o colonne, crea di nuovo il grafico, perché il grafico conserva le celle da cui è stato creato. Un grafico pivot proveniente da un file viene mostrato come salvato.

## Segmentazioni dati e sequenze temporali

Una segmentazione dati è un insieme di pulsanti che filtra una tabella pivot, e una sequenza temporale è un cursore che fa lo stesso per le date.

Fai clic su una cella di una tabella pivot, quindi fai clic su **Segmentazioni dati** nella scheda Tabella pivot. Si apre il riquadro **Segmentazioni dati**, che elenca le segmentazioni e le sequenze temporali della cartella di lavoro. Una segmentazione ha un pulsante per ogni elemento del suo campo. I pulsanti in blu sono gli elementi mostrati dalla tabella.

Fai clic su un pulsante per mostrare solo quell'elemento. Fai di nuovo clic per mostrare tutti gli elementi. Tieni premuto **Ctrl** e fai clic per aggiungere un elemento a quelli mostrati, oppure per toglierne uno. Ogni clic corrisponde a un passaggio di Annulla, e Annulla riporta la tabella allo stato precedente. Quando salvi, il file conserva la scelta della segmentazione.

**Connessioni report** indica quali tabelle pivot filtra una segmentazione. Una sequenza temporale mostra l'intervallo di date che ha scelto. JeffCrunch mostra le segmentazioni in questo riquadro, non sopra le celle, e non modifica ancora una sequenza temporale. Entrambe vengono conservate nel file come salvate.

## Limiti

Una tabella pivot può avere fino a 2.000 campi, un elenco di fino a cinque milioni di righe e un rapporto di fino a quattro milioni di celle. Un file che supera questi limiti viene mostrato come salvato e non può essere modificato. Rileggere centomila righe di venti colonne e calcolare il rapporto ha richiesto circa mezzo secondo sul computer su cui è stato misurato; un computer più lento impiega più tempo.
