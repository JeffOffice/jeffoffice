# Tabelle pivot
<!-- keywords: elenca formule, campo calcolato, elemento calcolato, ordine di calcolo, campo formula, mostra pagine filtro report, tabella pivot, analisi dettagliata, approfondisci, riduci livello, inserisci tabella pivot, crea, fai, seleziona, sposta tabella pivot, getpivotdata, genera getpivotdata, aggiorna, aggiorna tutto, cambia origine dati, elenco campi, campi, righe, colonne, valori, filtri, filtro report, totale generale, subtotale, layout, compatto, struttura, tabulare, cancella, impostazioni campo valore, somma, conteggio, media, mostra valori come, percentuale del totale, differenza, totale parziale, formato numero, impostazioni campo, ordina, filtra, primi, raggruppa, separa, date, mesi, anni, espandi, comprimi, opzioni, stile, righe a bande, righe vuote, ripeti etichette, aggiorna all'apertura del file, grafico pivot, segmentazione dati, sequenza temporale, mostra dettagli, analisi dettagliata, righe dietro un valore -->

Una tabella pivot è un piccolo rapporto ricavato da un lungo elenco. L'elenco ha una riga per ogni vendita, ad esempio; il rapporto somma le vendite per area e per mese. Non occorre scrivere alcuna formula. Si indica quali colonne dell'elenco vanno sul lato, quali in alto e quale va sommata, e il rapporto si compila da solo.

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

Est a gennaio vale 15 perché si sommano le due righe di Est di gennaio, 10 e 5. La colonna Totale generale somma ogni riga, la riga Totale generale somma ogni colonna, e la cella d'angolo somma tutto.

I termini usati qui:

- **Campo** è una colonna dell'elenco. Area, Mese e Importo sono campi.
- **Righe**, **Colonne** e **Valori** sono i tre posti in cui può andare un campo. Un campo in Righe dà una riga per ciascun suo valore diverso; un campo in Colonne dà una colonna per ciascuno; un campo in Valori è quello che viene sommato. Un quarto posto, **Filtri**, contiene un campo che esclude le righe non scelte.
- Una **cache** (la cache della tabella pivot) è una copia del tuo elenco che la tabella pivot conserva all'interno del file, così può mostrare i totali senza leggere le celle. Il rapporto viene calcolato dalla cache, non dalle celle, quindi non cambia quando cambiano le celle. **Aggiorna** copia di nuovo le celle nella cache.

## Creare una tabella pivot

1. Fai clic su una cella dell'elenco, oppure seleziona le celle che vuoi usare. L'elenco richiede una riga di nomi sopra i dati.
2. Scegli **Inserisci**, quindi **Tabella pivot**.
3. La finestra mostra le celle in cui si trovano i dati. Modificale se non sono corrette. Puoi anche digitare il nome di una tabella.
4. Scegli dove inserire la tabella pivot: **Nuovo foglio** (predefinito), oppure una cella di questo foglio o di un altro. Scegli un punto libero, perché la tabella vuota occupa circa diciotto righe e tre colonne.
5. Scegli **OK**. La tabella inizia vuota e si apre il riquadro **Campi tabella pivot**. Seleziona i campi che ti servono: un campo di numeri viene sommato, mentre ogni altro campo diventa una riga. Poi sposta i campi nelle caselle, come spiegano le sezioni seguenti.

Un nuovo foglio è un passaggio autonomo di Annulla, e così lo è la tabella. Quando salvi in .xlsx, la tabella viene scritta nel modo in cui la scriverebbe il programma originale, quindi si apre lì come tabella pivot. Una copia salvata in .xls o .ods conserva nelle celle i numeri che la tabella mostra, ma non la tabella stessa, e il salvataggio te lo segnala.

## GETPIVOTDATA: leggere un numero da una tabella pivot

`GETPIVOTDATA` restituisce il numero che una tabella pivot mostra per i valori e per le righe e colonne che indichi. Per l'esempio sopra:

`=GETPIVOTDATA("Somma di Importo"; $E$3; "Area"; "Est"; "Mese"; "Gen")` restituisce 15.

- Il primo argomento è il nome del valore, come lo mostra la tabella (ad esempio Somma di Importo). Funziona anche il nome del campo che viene sommato (Importo).
- Il secondo argomento è una cella qualsiasi della tabella pivot.
- Seguono coppie di un campo e di uno dei suoi valori. Se indichi solo Area ed Est, ottieni il totale di Est, 35. Se non indichi nulla, ottieni il totale generale, 49. I campi che indichi devono essere i primi campi delle righe, oppure i primi campi delle colonne, in ordine.
- Se la tabella non mostra il numero, la cella mostra `#RIF!`.

Quando fai clic su un valore di una tabella pivot mentre digiti una formula, JeffCrunch scrive al posto tuo questa formula, in modo che continui a leggere il numero giusto anche dopo che la tabella è stata riorganizzata. Per disattivarlo, scegli **Opzioni** nella scheda **Tabella pivot** e deseleziona la casella relativa a GETPIVOTDATA.

## Che cosa fa JeffCrunch con una tabella pivot

Una tabella pivot in una cartella di lavoro viene mostrata come era stata salvata. Se non cambi nulla, il file viene salvato con la tabella pivot esattamente com'era. Quando la aggiorni o la modifichi, JeffCrunch ricalcola il rapporto, inserisce i nuovi numeri nelle celle e riscrive la tabella pivot in modo che il programma che l'ha creata la apra senza problemi.

JeffCrunch calcola le tabelle pivot che sommano, contano, fanno la media e così via, che raggruppano date e numeri, e che mostrano i valori come percentuale, differenza o totale parziale. Una tabella pivot che mostra un valore come classifica o come percentuale di un elemento padre, che ha gruppi di elementi creati a mano o che usa un modello dati viene mostrata e conservata come salvata, e i comandi ti segnalano che non è possibile modificarla.

Fai clic su una cella qualsiasi di una tabella pivot. Compare la scheda **Tabella pivot**.

## Aggiorna

**Aggiorna** legge le celle da cui è composta la tabella pivot e ricalcola il rapporto. I nuovi elementi si aggiungono dopo quelli già presenti; gli elementi che non esistono più vengono eliminati. Le larghezze delle colonne e la struttura scelta restano invariate. **Aggiorna tutto**, nella scheda **Dati** e nella scheda **Tabella pivot**, fa lo stesso per tutte le tabelle pivot della cartella di lavoro.

Se il rapporto dovesse estendersi su celle che contengono dati, JeffCrunch non esegue l'aggiornamento e ti indica quali celle ostacolano l'operazione. Cancella quelle celle o sposta la tabella pivot, quindi aggiorna di nuovo.

**Annulla** annulla un aggiornamento con un solo passaggio.

## Cambia origine dati

**Cambia origine dati** chiede le celle in cui si trova l'elenco, compresa la prima riga dei nomi. Digitale come `Foglio1!A1:D100`, oppure digita il nome di una tabella o di un nome che hai creato. JeffCrunch legge le celle e ricalcola il rapporto. Se un campo dell'elenco ha lo stesso nome nelle nuove celle, la sua posizione nel rapporto resta invariata.

## Cancella

**Cancella** offre due scelte. **Cancella filtri** rimuove tutti i filtri e mostra di nuovo tutti i valori. **Cancella tutto** elimina tutti i campi dal rapporto e lo lascia vuoto, pronto per essere ricostruito.

## Analisi dettagliata e analisi di sintesi

## Mostra pagine filtro report

Un filtro si trova sopra una tabella pivot e permette di esaminare un solo elemento alla volta, ad esempio un mese. **Mostra pagine filtro report**, nella scheda **Tabella pivot**, crea una pagina per ciascun elemento contemporaneamente. Ogni pagina è un nuovo foglio, denominato con l'elemento, con una propria copia della tabella. Il filtro di ciascuna copia è impostato su quell'elemento. Se la tabella ha più filtri, scegli quale usare. Un solo **Annulla** elimina tutte le pagine.

Quando un rapporto ha più di un campo nelle righe, un elemento del primo campo può essere aperto o chiuso. Fai clic sulla cella con il nome dell'elemento e scegli **Riduci livello** per chiuderlo: le righe del campo successivo sotto di esso vengono nascoste e resta solo il suo totale. **Analisi dettagliata** lo riapre. Un elemento dell'ultimo campo non ha nulla sotto di sé. Ciascuna operazione richiede un solo **Annulla**.

## Selezionare e spostare

**Seleziona**, nella scheda **Tabella pivot**, seleziona una parte del rapporto: **Intera tabella pivot**, **Valori** (i numeri) oppure **Etichette** (i nomi sul lato; se il rapporto non ha nomi sul lato, i nomi in alto). **Sposta tabella pivot…** chiede una cella dello stesso foglio e ridisegna il rapporto con la prima cella lì. Le celle che lascia vengono svuotate. Le formule che puntano alle celle del rapporto le seguono, come accade quando tagli e incolli. Se il nuovo posto contiene dati, JeffCrunch non sposta il rapporto e te lo segnala. Un solo **Annulla** lo riporta indietro.

## Il riquadro dei campi

**Elenco campi**, nella scheda **Tabella pivot**, apre il riquadro **Campi tabella pivot**. Vi compaiono i campi dell'elenco con una casella accanto a ciascuno e, sotto, i quattro posti: **Filtri**, **Colonne**, **Righe** e **Valori**.

- Seleziona un campo per aggiungerlo. Un campo che contiene solo numeri va in **Valori** ed è sommato; ogni altro va in **Righe**.
- Deseleziona un campo per toglierlo da ogni posto.
- Fai clic su un campo in uno dei quattro posti e scegli **Sposta in** per metterlo in un altro. I comandi **Sposta su** e **Sposta giù** ne cambiano l'ordine nel suo posto, e **Rimuovi campo** lo toglie.
- Con più di un valore, in **Colonne** compare una riga chiamata **Valori**. Spostala in **Righe** per mettere i valori sul lato.
- **Impostazioni campo valore** (doppio clic su un valore, oppure il pulsante) imposta il nome che il rapporto mostra per il valore e il modo in cui viene totalizzato: **Somma**, **Conteggio**, **Conteggio numeri**, **Media**, **Max**, **Min**, **Prodotto**, **DevStd** e **DevStdp** (la deviazione standard di un campione, o di tutti i valori), e **Var** e **Varp** (la varianza degli stessi). Imposta anche il modo in cui il valore viene mostrato e il suo formato numero. Vedi la sezione successiva.
- **Impostazioni campo** (doppio clic su un campo in Filtri, Colonne o Righe, oppure il pulsante) imposta un solo campo. Vedi "Impostazioni campo".

Ogni modifica è un passaggio di **Annulla**.

## Mostra valori come e formato numero di un valore

In **Impostazioni campo valore**, **Mostra valori come** cambia ciò che mostra una cella del rapporto.

- **% del totale generale**: la cella divisa per il totale di tutto il rapporto. Nell'esempio, Est a gennaio è 15 su 49, cioè il 30,6%.
- **% del totale di colonna** e **% del totale di riga**: la cella divisa per il totale della sua colonna o della sua riga.
- **Indice**: il confronto tra la cella e ciò che la sua riga e la sua colonna darebbero se i dati fossero distribuiti in modo uniforme. Un numero superiore a 1 indica più della quota uniforme.
- **Differenza da**, **% di** e **% differenza da**: la cella confrontata con un altro elemento di un campo. Scegli il **Campo base** (un campo in Righe o Colonne) e l'**Elemento base** (uno dei suoi elementi, oppure quello precedente o successivo). Ad esempio, con Mese come campo base e Gen come elemento base, febbraio mostra di quanto è maggiore o minore di gennaio. L'elemento base stesso resta vuoto per una differenza, e vale 100% per **% di**.
- **Totale parziale in**: ogni cella somma le celle che la precedono lungo il campo base.
- **Classifica dal più piccolo al più grande** e **Classifica dal più grande al più piccolo**: ogni cella mostra il suo posto, 1, 2, 3 e così via, tra le celle del campo base nella stessa riga o colonna. Il posto 1 è il numero più piccolo, oppure il più grande.
- **% del totale padre di riga** e **% del totale padre di colonna**: ogni cella mostra quale parte del suo padre rappresenta. Il padre di un elemento è l'elemento un livello più in alto, come l'anno che contiene un mese. Un elemento senza padre è mostrato come parte del totale generale.

**Formato numero** accetta un codice di formato, come `0,00`, `#.##0` o `0,0%`. Lascialo vuoto per mantenere il formato che hanno le celle. Un valore mostrato come percentuale riceve `0,00%`, a meno che tu non scelga un altro formato. Le celle del rapporto assumono subito il formato. **Annulla** lo riporta indietro insieme ai numeri.

## Impostazioni campo

Seleziona un campo nella casella **Filtri**, **Colonne** o **Righe** e scegli **Impostazioni campo** nella scheda **Tabella pivot** (oppure fai doppio clic su di esso).

- **Nome personalizzato** cambia il nome che il rapporto mostra per il campo.
- **Subtotali**: **Automatico** mostra un totale per ciascun gruppo del campo, **Nessuno** non lo mostra.
- **Mostra etichette elementi in**: forma compatta, a struttura o tabulare, come in Layout rapporto.
- **Inserisci una riga vuota dopo ogni elemento** aggiunge una riga vuota dopo ogni gruppo.
- **Ripeti etichette elementi** mostra il nome di un elemento esterno su ogni riga del suo gruppo (forme a struttura e tabulare).
- **Ordina**: mantieni l'ordine in cui si trovano gli elementi (manuale), oppure ordina dalla A alla Z o dalla Z alla A in base alle etichette. **Dal più piccolo al più grande per valore** e **Dal più grande al più piccolo per valore** ordinano gli elementi in base ai numeri di un campo valore. Il rapporto mantiene l'ordinamento quando cambiano i dati.
- **Filtro** esclude elementi in base alle loro etichette (inizia con, termina con, contiene, uguale a) oppure in base a un valore (maggiore di, minore di, compreso tra e così via), oppure mantiene solo i primi o gli ultimi elementi in base a un valore. Il filtro viene calcolato quando scegli OK. Nasconde gli elementi esclusi; le caselle di controllo qui sotto indicano quali sono, e puoi selezionare un elemento per mostrarlo di nuovo.
- **Mostra questi elementi** è un elenco degli elementi del campo con una casella di controllo per ciascuno. Deseleziona una casella per nascondere l'elemento, e i totali lo escludono.

## Raggruppare date e numeri

Seleziona un campo di date o di numeri in **Righe** o **Colonne** e scegli **Raggruppa campo** nella scheda **Tabella pivot**. I gruppi vengono creati dai dati, quindi in quella colonna devono esserci solo numeri o solo date, senza testo e senza celle vuote.

- **Date**: seleziona come raggruppare: **Secondi**, **Minuti**, **Ore**, **Giorni**, **Mesi**, **Trimestri** oppure **Anni**. La parte più fine che selezioni resta nel campo. Per ogni parte più grossolana JeffCrunch aggiunge un campo, chiamato come quella parte (ad esempio **Anni**), e lo inserisce prima del campo data nello stesso posto. **Inizio** e **Fine** sono la prima e l'ultima data; lasciali così per usare la data minima e massima dei dati. Le date prima dell'inizio o dopo la fine vanno in due gruppi aggiuntivi, che iniziano con `<` e `>`.
- **Numeri**: **Inizio**, **Fine** e **Intervallo** creano gruppi come 1-10, 11-20 e così via.

**Separa** elimina i gruppi e i campi aggiunti per essi. Aggiorna ricrea i gruppi con i nuovi dati. Quando due tabelle pivot sono create dagli stessi dati, non si possono creare gruppi, perché un gruppo è una parte dei dati che esse condividono.

## Campi calcolati

Un campo calcolato è un campo che crei con una formula. Usa altri campi dell'elenco. Ad esempio, un premio pari a un decimo dell'importo è `=Importo*0,1`.

Scegli **Campo calcolato** nella scheda **Tabella pivot**. Digita un nome e una formula. Scrivi nella formula il nome di un campo, oppure fai doppio clic su di esso nell'elenco **Campi** per inserirlo. Racchiudi tra apici singoli un nome che contiene spazi, come in `='Prezzo unitario'*Quantità`. Puoi usare numeri, `+ - * / ^ %`, confronti e alcune funzioni come `SE`, `ARROTONDA`, `SOMMA`, `MIN`, `MAX`, `MEDIA` e `ASS`.

Scegli **Aggiungi**. Il nuovo campo viene inserito nella casella **Valori**. Compare anche nell'elenco dei campi, dove puoi selezionarlo o deselezionarlo come ogni altro campo. Non può andare nelle righe, nelle colonne o nei filtri.

Un campo calcolato viene calcolato in ogni cella del rapporto. Il nome di un campo rappresenta il totale di quel campo nella cella, quindi `=Importo*Importo` eleva al quadrato il totale della cella. Non eleva al quadrato ciascuna riga e poi somma. Un totale generale usa i totali dell'intero elenco.

**Elenca formule** (accanto a **Campo calcolato**) crea un nuovo foglio che riporta ogni campo calcolato con la sua formula, e ogni elemento calcolato con la sua formula, il suo campo e il suo ordine di calcolo. **Annulla** elimina le celle, e un altro **Annulla** elimina il foglio.

Per modificare un campo, scegli il suo nome nella casella **Nome**. Compare la sua formula. Modificala e scegli **Modifica**. **Elimina** rimuove il campo e il suo valore. Ogni operazione è un passaggio di **Annulla**. Un campo calcolato viene salvato con il file e viene conservato quando il rapporto viene aggiornato. Due tabelle pivot create dagli stessi dati non possono averne uno, perché un campo calcolato è una parte dei dati che esse condividono.

## Elementi calcolati

Un elemento calcolato è un elemento che crei a partire da altri elementi dello stesso campo. Ad esempio, il campo Area ha gli elementi Est e Ovest. Un nuovo elemento chiamato Entrambi può essere `=Est+Ovest`.

Fai clic su una cella della tabella pivot e scegli **Elemento calcolato** nella scheda **Tabella pivot**. Scegli il **Campo**. Deve trovarsi nelle righe o nelle colonne. Digita un **Nome** e una **Formula**. Scrivi nella formula il nome di un elemento, oppure fai doppio clic su di esso nell'elenco **Elementi** per inserirlo. Racchiudi tra apici singoli un nome che contiene spazi, come in `='Grande Est'+Ovest`. Puoi usare numeri, `+ - * / ^ %` e parentesi.

Il nuovo elemento ottiene una riga nel rapporto. In ogni colonna mostra la formula calcolata a partire dalle righe degli elementi che nomina. Il nuovo elemento è incluso nei totali. Un totale che somma, come il totale generale o un subtotale, aggiunge la riga del nuovo elemento alle righe degli altri elementi. Quindi, con Est a 35, Ovest a 14 ed Entrambi (Est più Ovest) a 49, il totale generale è 98. Se un elemento citato nella formula è nascosto, la cella mostra **#RIF!**. Se la formula cita un elemento che non esiste, la cella mostra **#NOME?**.

Per modificare un elemento, scegli il suo nome nella casella **Nome**. Compare la sua formula. Modificala e scegli **Modifica**. **Elimina** rimuove l'elemento e la sua riga. Ogni operazione è un passaggio di **Annulla**. Un elemento calcolato viene salvato con il file e viene conservato quando il rapporto viene aggiornato. Due tabelle pivot create dagli stessi dati non possono averne uno, perché un elemento calcolato è una parte dei dati che esse condividono.

### Ordine di calcolo

A volte una cella ha due elementi calcolati. Uno si trova nella sua riga e l'altro nella sua colonna. L'ordine in cui vengono calcolati può cambiare il risultato. Supponi che l'elemento di riga sia `=Est-Ovest` e l'elemento di colonna sia `=Gen+1`. Calcolato in un ordine, il valore della cella è (Est - Ovest) + 1. Nell'altro ordine è (Est + 1) - (Ovest + 1), che è un numero diverso.

**Ordine di calcolo**, nella scheda **Tabella pivot**, elenca gli elementi calcolati nell'ordine in cui vengono calcolati. L'elemento più in basso nell'elenco viene calcolato per ultimo. Scegli un elemento e fai clic su **Sposta su** o **Sposta giù**. L'ordine viene salvato con il file come ordine degli elementi, e gli altri programmi lo leggono allo stesso modo. **Elenca formule** mostra il posto di ciascun elemento.

## Mostrare le righe dietro un valore

Fai doppio clic su un valore di una tabella pivot, oppure scegli **Mostra dettagli** nella scheda **Tabella pivot**. JeffCrunch crea un nuovo foglio, denominato Dettaglio 1, 2 e così via, con le righe dell'elenco che compongono il numero. I nomi dei campi si trovano nella prima riga. F2 modifica ancora la cella, se la cella può essere modificata. **Annulla** elimina le righe, e un altro **Annulla** elimina il foglio.

## Espandere e comprimere

Un campo i cui elementi hanno altri elementi sotto di sé può mostrare o nascondere le righe sotto i suoi elementi. **Espandi intero campo** e **Comprimi intero campo** lo fanno per ogni elemento del campo selezionato nel riquadro.

## Opzioni tabella pivot

**Opzioni**, nella scheda **Tabella pivot**, apre una finestra con le impostazioni della tabella.

- **Nome** è il nome della tabella pivot. Nessun'altra tabella pivot della cartella di lavoro può avere lo stesso nome.
- **Per i valori di errore mostra** e **Per le celle vuote mostra** inseriscono in quelle celle un testo personalizzato.
- **Unisci e allinea al centro le celle con etichette**, **Adatta la larghezza delle colonne quando la tabella viene aggiornata** e **Mantieni la formattazione delle celle quando la tabella viene aggiornata** vengono salvate con la tabella.
- **Mostra totali generali per le righe** e **per le colonne** corrispondono alle scelte di Totali generali.
- **Aggiorna i dati all'apertura del file** legge di nuovo le celle ogni volta che la cartella di lavoro viene aperta. È un passaggio di Annulla, come ogni aggiornamento.

## Stile: intestazioni, bande e galleria

Le **Opzioni stile**, nella scheda **Tabella pivot**, attivano o disattivano la riga di intestazione, la prima colonna (intestazioni di riga) e le bande di righe e di colonne. **Stili** contiene una galleria di aspetti, e **Cancella stile** rimuove l'aspetto. Questi aspetti sono disegnati come quelli delle tabelle. **Righe vuote** e **Ripeti etichette** impostano la riga vuota e le etichette ripetute per tutti i campi delle righe in una sola volta.

## Struttura: subtotali, totali generali e layout

- **Subtotali** mostra o nasconde il totale di ciascun gruppo, in alto o in basso nel gruppo. Quando ci sono più campi nelle Righe, il totale di ciascun campo esterno è un subtotale.
- **Totali generali** ha quattro scelte: disattivati per righe e colonne, attivati per entrambe, attivati solo per le righe (una colonna di totali a destra, uno per ogni riga) e attivati solo per le colonne (una riga di totali in basso).
- **Layout rapporto** è la forma delle etichette. In forma **compatta**, tutti i campi delle Righe condividono una sola colonna, e un nuovo gruppo è mostrato un po' più a destra. In forma **a struttura**, ogni campo ha una colonna propria, e ogni gruppo ha una riga tutta sua sopra i suoi elementi. In forma **tabulare**, ogni campo ha una colonna propria e il nome del gruppo si trova sulla sua prima riga, con il subtotale sotto il gruppo.

## Lo stesso elenco, con un filtro

Sposta **Mese** da **Colonne** a **Filtri**. Il rapporto perde le sue colonne, e sopra di esso una riga mostra **Mese** e **(Tutti)**. I numeri sono ora il totale di ciascuna area su tutti i mesi.

## Grafico pivot

Fai clic su una cella di una tabella pivot e scegli **Inserisci**, quindi **Grafico pivot**. JeffCrunch crea un grafico a colonne dei valori della tabella. Esclude i totali generali. Il grafico viene indicato nel file come grafico della tabella pivot. Legge le celle della tabella, quindi i suoi numeri cambiano quando aggiorni la tabella o ne modifichi la struttura. Se la tabella ottiene altre righe o colonne, crea di nuovo il grafico, perché il grafico conserva le celle da cui è stato creato. Un grafico pivot proveniente da un file viene mostrato come salvato.

## Segmentazioni dati e sequenze temporali

Una segmentazione dati è un insieme di pulsanti che filtra una tabella pivot, mentre una sequenza temporale è un cursore che fa lo stesso per le date.

Fai clic su una cella di una tabella pivot, quindi su **Segmentazioni dati** nella scheda Tabella pivot. Si apre il riquadro Segmentazioni dati, che elenca le segmentazioni e le sequenze temporali della cartella di lavoro. Una segmentazione ha un pulsante per ciascun elemento del suo campo. I pulsanti blu sono gli elementi che la tabella mostra.

Fai clic su un pulsante per mostrare solo quell'elemento. Fai di nuovo clic per mostrare tutti gli elementi. Tieni premuto **Ctrl** e fai clic per aggiungere un elemento a quelli mostrati, oppure per toglierne uno. Ogni clic è un passaggio di Annulla, e Annulla riporta la tabella com'era. Quando salvi, il file conserva la scelta della segmentazione.

**Report collegati** indica quali tabelle pivot filtra una segmentazione. Una sequenza temporale mostra l'intervallo di date che ha scelto. JeffCrunch mostra le segmentazioni in questo riquadro, non sopra le celle, e non modifica ancora una sequenza temporale. Entrambe vengono conservate nel file come sono state salvate.

## Limiti

Una tabella pivot può avere fino a 2.000 campi, un elenco fino a cinque milioni di righe e un rapporto fino a quattro milioni di celle. Un file che supera questi limiti viene mostrato come salvato e non può essere modificato. Rileggere centomila righe di venti colonne e calcolare il rapporto ha richiesto circa mezzo secondo sul computer su cui è stato misurato; un computer più lento impiega più tempo.
