# Tabelle pivot

<!-- keywords: elenca formule, campo calcolato, campo formula, pagine filtro report, tabella pivot, tabella pivot, approfondisci, analisi dettagliata, riduci livello, analisi di sintesi, inserisci tabella pivot, crea, fai, seleziona, sposta tabella pivot, getpivotdata, genera getpivotdata, aggiorna, aggiorna tutto, cambia origine dati, elenco campi, campi, righe, colonne, valori, filtri, filtro report, totale generale, subtotale, layout, compatto, struttura, tabulare, cancella, impostazioni campo valore, somma, conteggio, media, mostra valori come, percentuale del totale, differenza, totale parziale, formato numero, impostazioni campo, ordina, filtra, primi, raggruppa, separa gruppo, date, mesi, anni, espandi, comprimi, opzioni, stile, righe a bande, righe vuote, ripeti etichette, aggiorna all'apertura del file, grafico pivot, segmentazione dati, sequenza temporale, mostra dettagli, righe dietro un valore -->

Una tabella pivot è un piccolo rapporto ricavato da un elenco lungo. L'elenco ha una riga per ogni vendita, per esempio; il rapporto somma le vendite per area e per mese. Non devi scrivere nessuna formula. Indichi quali colonne dell'elenco vanno sul lato, quali in alto e quale va sommata, e il rapporto si compila da solo.

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

Est a gennaio vale 15 perché si sommano le due righe Est di gennaio, 10 e 5. La colonna Totale generale somma ogni riga, la riga Totale generale somma ogni colonna, e la cella in basso a destra somma tutto.

I termini usati qui:

- **Campo** è una colonna dell'elenco. Area, Mese e Importo sono i campi.
- **Righe**, **Colonne** e **Valori** sono i tre posti in cui può andare un campo. Un campo in Righe dà una riga per ciascuno dei suoi diversi valori; un campo in Colonne dà una colonna per ciascuno; un campo in Valori è quello che viene sommato. Un quarto posto, **Filtri**, contiene un campo che esclude le righe che non scegli.
- Una **cache** (la cache della tabella pivot) è una copia del tuo elenco che la tabella pivot conserva nel file, così può mostrare i totali senza leggere le celle. Il rapporto viene calcolato dalla cache, non dalle celle, quindi non cambia quando cambiano le celle. **Aggiorna** copia di nuovo le celle nella cache.

## Creare una tabella pivot

1. Fai clic su una cella dell'elenco, oppure seleziona le celle che vuoi usare. L'elenco deve avere una riga di nomi sopra i dati.
2. Scegli **Inserisci**, quindi **Tabella pivot**.
3. La finestra mostra le celle in cui si trovano i dati. Cambiale se non sono corrette. Puoi anche digitare il nome di una tabella.
4. Scegli dove va la tabella pivot: **Nuovo foglio** (predefinito), oppure una cella di questo foglio o di un altro. Scegli un punto in cui non ci sia nulla, perché la tabella vuota occupa circa diciotto righe e tre colonne.
5. Scegli **OK**. La tabella inizia vuota e si apre il riquadro **Campi tabella pivot**. Seleziona i campi che vuoi: un campo di numeri viene sommato, mentre ogni altro campo diventa una riga. Poi spostali nelle caselle come spiegano le sezioni seguenti.

Un nuovo foglio è un passaggio autonomo di **Annulla**, e così lo è la tabella. Quando salvi in .xlsx la tabella viene scritta nel modo in cui la scrive il programma originale, quindi si apre lì come tabella pivot. Una copia salvata in .xls o .ods conserva nelle sue celle i numeri che la tabella mostra, ma non la tabella stessa, e il salvataggio te lo segnala.

## GETPIVOTDATA: leggere un numero da una tabella pivot

`GETPIVOTDATA` restituisce il numero che una tabella pivot mostra per i valori e per le righe e colonne che indichi. Per l'esempio sopra:

`=GETPIVOTDATA("Somma di Importo"; $E$3; "Area"; "Est"; "Mese"; "Gen")` restituisce 15.

- Il primo argomento è il nome del valore, come lo mostra la tabella (per esempio Somma di Importo). Funziona anche il nome del campo che viene sommato (Importo).
- Il secondo argomento è una qualsiasi cella della tabella pivot.
- Seguono coppie di un campo e di uno dei suoi elementi. Se indichi solo Area ed Est, ottieni il totale di Est, 35. Se non indichi nulla, ottieni il totale generale, 49. I campi che indichi devono essere i primi campi delle righe, o i primi campi delle colonne, in ordine.
- Se la tabella non mostra il numero, la cella visualizza `#RIF!`.

Quando, mentre digiti una formula, fai clic su un valore di una tabella pivot, JeffCrunch scrive questa formula al posto tuo, così continua a leggere il numero giusto anche dopo che la tabella viene riorganizzata. Per disattivarlo, scegli **Opzioni** nella scheda **Tabella pivot** e deseleziona la casella relativa a GETPIVOTDATA.

## Che cosa fa JeffCrunch con una tabella pivot

Una tabella pivot in una cartella di lavoro viene mostrata come è stata salvata. Se non modifichi nulla, il file viene salvato con la tabella pivot esattamente com'era. Quando la aggiorni o la modifichi, JeffCrunch calcola di nuovo il rapporto, inserisce i nuovi numeri nelle celle e riscrive la tabella pivot in modo che il programma che l'ha creata la apra senza problemi.

JeffCrunch elabora le tabelle pivot che sommano, contano, calcolano medie e così via, che raggruppano date e numeri, e che mostrano valori come percentuale, differenza o totale parziale. Una tabella pivot che contiene un campo o un elemento calcolato, che mostra un valore come classifica o come percentuale di un elemento padre, che ha gruppi di elementi creati a mano, oppure che usa un modello di dati viene mostrata e conservata come salvata, e i comandi ti avvisano che non possono modificarla.

Fai clic su una cella qualsiasi di una tabella pivot. Compare la scheda **Tabella pivot**.

## Aggiornare

**Aggiorna** legge le celle da cui è creata la tabella pivot e ricalcola il rapporto. Le nuove voci si aggiungono dopo quelle già presenti; le voci scomparse vengono eliminate. Le larghezze delle colonne e il layout che hai scelto restano. **Aggiorna tutto**, nella scheda **Dati** e nella scheda **Tabella pivot**, fa lo stesso per tutte le tabelle pivot della cartella di lavoro.

Se il rapporto dovesse estendersi su celle che contengono dati, JeffCrunch non aggiorna la tabella. Ti indica quali celle sono di intralcio. Svuotale o sposta la tabella pivot, poi aggiorna di nuovo.

**Annulla** annulla un aggiornamento in un solo passaggio.

## Cambia origine dati

**Cambia origine dati** chiede le celle in cui si trova l'elenco, compresa la prima riga dei nomi. Scrivile così: `Foglio1!A1:D100`, oppure digita il nome di una tabella o un nome che hai creato. JeffCrunch legge le celle e ricalcola il rapporto. Se un campo dell'elenco ha lo stesso nome nelle nuove celle, il suo posto nel rapporto resta invariato.

## Cancella

**Cancella** ha due scelte. **Cancella filtri** elimina tutti i filtri e mostra di nuovo tutti gli elementi. **Cancella tutto** elimina tutti i campi dal rapporto e lo lascia vuoto, pronto per essere costruito di nuovo.

## Analisi dettagliata e analisi di sintesi

## Pagine filtro report

Un filtro sta sopra una tabella pivot e ti permette di guardare un solo elemento alla volta, per esempio un mese. **Mostra pagine filtro report**, nella scheda **Tabella pivot**, crea una pagina per ogni elemento in una volta. Ogni pagina è un nuovo foglio, con il nome dell'elemento e con una propria copia della tabella. Il filtro di ciascuna copia è impostato su quell'elemento. Se la tabella ha più di un filtro, scegli quale usare. Un solo **Annulla** elimina tutte le pagine.

Quando un rapporto ha più di un campo nelle righe, un elemento del primo campo può essere aperto o chiuso. Fai clic sulla cella con il nome dell'elemento e scegli **Riduci livello** per chiuderlo: le righe del campo successivo che stanno sotto di esso vengono nascoste e resta solo il suo totale. **Approfondisci** lo riapre. Un elemento dell'ultimo campo non ha nulla sotto di sé. Ogni operazione è un solo **Annulla**.

## Seleziona e sposta

**Seleziona**, nella scheda **Tabella pivot**, seleziona una parte del rapporto: **Intera tabella pivot**, **Valori** (i numeri), oppure **Etichette** (i nomi sul lato; se il rapporto non ha nomi sul lato, i nomi in alto). **Sposta tabella pivot…** chiede una cella dello stesso foglio e ridisegna il rapporto con la sua prima cella lì. Le celle che lascia vengono svuotate. Le formule che puntano alle celle del rapporto le seguono, come accade quando tagli e incolli. Se la nuova posizione contiene qualcosa, JeffCrunch non sposta il rapporto e te lo segnala. Un solo **Annulla** lo riporta indietro.

## Il riquadro dei campi

**Elenco campi**, nella scheda **Tabella pivot**, apre il riquadro **Campi tabella pivot**. Mostra i campi dell'elenco con una casella accanto a ciascuno, e sotto le quattro posizioni: **Filtri**, **Colonne**, **Righe** e **Valori**.

- Seleziona un campo per aggiungerlo. Un campo che contiene solo numeri va in **Valori** ed è sommato; gli altri vanno in **Righe**.
- Deseleziona un campo per toglierlo da ovunque.
- Fai clic su un campo in una delle quattro posizioni e scegli **Sposta in** per metterlo in un'altra. Le voci **Sposta su** e **Sposta giù** cambiano il suo ordine nella posizione, e **Rimuovi campo** lo elimina.
- Con più di un valore, nelle **Colonne** compare una riga chiamata **Valori**. Spostala in **Righe** per mettere i valori sul lato.
- **Impostazioni campo valore** (doppio clic su un valore, oppure il pulsante) imposta il nome che il rapporto mostra per il valore e il modo in cui viene totalizzato: **Somma**, **Conteggio**, **Conteggio numeri**, **Media**, **Max**, **Min**, **Prodotto**, **DevStd** e **DevStdp** (la deviazione standard di un campione, o di tutti i valori), e **Var** e **Varp** (la varianza degli stessi). Imposta anche il modo in cui il valore viene mostrato e il suo formato numero. Vedi la sezione seguente.
- **Impostazioni campo** (doppio clic su un campo in Filtri, Colonne o Righe, oppure il pulsante) imposta un solo campo. Vedi "Impostazioni campo".

Ogni modifica è un passaggio di **Annulla**.

## Mostra valori come e formato numero di un valore

In **Impostazioni campo valore**, **Mostra valori come** cambia ciò che mostra una cella del rapporto.

- **% del totale generale**: la cella divisa per il totale di tutto il rapporto. Nell'esempio, Est a gennaio è 15 su 49, cioè 30,6%.
- **% del totale di colonna** e **% del totale di riga**: la cella divisa per il totale della sua colonna o della sua riga.
- **Indice**: il confronto tra la cella e ciò che otterrebbero la sua riga e la sua colonna se i dati fossero distribuiti in modo uniforme. Un numero sopra 1 significa più della quota uniforme.
- **Differenza da**, **% di** e **% differenza da**: la cella confrontata con un altro elemento di un campo. Scegli il **Campo base** (un campo in Righe o in Colonne) e l'**Elemento base** (uno dei suoi elementi, oppure quello precedente o successivo). Per esempio, con Mese come campo base e Gen come elemento base, febbraio mostra di quanto è maggiore o minore di gennaio. L'elemento base stesso resta vuoto per una differenza, e vale 100% per **% di**.
- **Totale parziale in**: ogni cella somma le celle che la precedono lungo il campo base.
- **Classifica dal più piccolo al più grande** e **Classifica dal più grande al più piccolo**: ogni cella mostra il suo posto, 1, 2, 3 e così via, tra le celle del campo base nella stessa riga o colonna. Il posto 1 è il numero più piccolo, oppure il più grande.
- **% del totale di riga padre** e **% del totale di colonna padre**: ogni cella mostra quale parte del suo padre è. Il padre di un elemento è l'elemento un livello più in alto, per esempio l'anno che contiene un mese. Un elemento senza padre viene mostrato come parte del totale generale.

**Formato numero** accetta un codice di formato, come `0,00`, `#.##0` o `0,0%`. Lascialo vuoto per mantenere il formato che hanno le celle. Un valore mostrato come percentuale riceve `0,00%`, a meno che tu non scelga un altro formato. Le celle del rapporto assumono subito il formato. **Annulla** lo riporta indietro insieme ai numeri.

## Impostazioni campo

Seleziona un campo nella casella **Filtri**, **Colonne** o **Righe** e scegli **Impostazioni campo** nella scheda **Tabella pivot** (oppure fai doppio clic).

- **Nome personalizzato** cambia il nome che il rapporto mostra per il campo.
- **Subtotali**: **Automatico** mostra un totale per ogni gruppo del campo, **Nessuno** non lo mostra.
- **Mostra etichette degli elementi in**: forma compatta, struttura o tabulare, come in Layout rapporto.
- **Inserisci una riga vuota dopo ogni elemento** aggiunge una riga vuota dopo ciascun gruppo.
- **Ripeti etichette degli elementi** mostra il nome di un elemento esterno su ogni riga del suo gruppo (forme a struttura e tabulare).
- **Ordina**: mantieni l'ordine in cui si trovano gli elementi (manuale), oppure ordina dalla A alla Z o dalla Z alla A in base alle etichette. **Dal più piccolo al più grande per valore** e **Dal più grande al più piccolo per valore** ordinano gli elementi in base ai numeri di un campo valore. Il rapporto mantiene l'ordinamento quando i dati cambiano.
- **Filtra** esclude elementi in base alle loro etichette (inizia con, termina con, contiene, uguale a) o in base a un valore (maggiore di, minore di, compreso tra e così via), oppure mantiene solo i primi o gli ultimi elementi in base a un valore. Il filtro viene applicato quando scegli OK. Nasconde gli elementi che esclude; le caselle sotto indicano quali sono, e puoi selezionare un elemento per mostrarlo di nuovo.
- **Mostra questi elementi** è un elenco degli elementi del campo, con una casella per ciascuno. Deseleziona una casella per nascondere l'elemento, e i totali lo escludono.

## Raggruppare date e numeri

Seleziona un campo di date o di numeri in **Righe** o in **Colonne** e scegli **Raggruppa campo** nella scheda **Tabella pivot**. I gruppi vengono creati dai dati, quindi in quella colonna devono esserci solo numeri o date, senza testo e senza celle vuote.

- **Date**: seleziona il criterio di raggruppamento: **Secondi**, **Minuti**, **Ore**, **Giorni**, **Mesi**, **Trimestri** o **Anni**. La parte più fine che selezioni resta nel campo. Per ogni parte più grossolana JeffCrunch aggiunge un campo, con il suo nome (per esempio **Anni**), e lo mette prima del campo data, nello stesso punto. **Inizia da** e **Termina a** sono la prima e l'ultima data; lasciali così per usare la data più vecchia e quella più recente dei dati. Le date precedenti all'inizio o successive alla fine finiscono in due gruppi aggiuntivi, che iniziano con `<` e `>`.
- **Numeri**: **Inizia da**, **Termina a** e **Di** creano gruppi come 1-10, 11-20, e così via.

**Separa gruppo** elimina i gruppi e i campi che erano stati aggiunti per essi. Aggiornare ricrea i gruppi dai nuovi dati. Quando due tabelle pivot sono create dagli stessi dati, non si possono creare gruppi, perché un gruppo è una parte dei dati che condividono.

## Campi calcolati

Un campo calcolato è un campo che crei con una formula. Usa altri campi dell'elenco. Per esempio, un premio pari a un decimo dell'importo è `=Importo*0,1`.

Scegli **Campo calcolato** nella scheda **Tabella pivot**. Digita un nome e una formula. Scrivi il nome di un campo nella formula, oppure fai doppio clic su di esso nell'elenco **Campi** per inserirlo. Scrivi tra apici singoli un nome che contiene spazi, come in `='Prezzo unitario'*Quantità`. Puoi usare numeri, `+ - * / ^ %`, confronti e alcune funzioni come `SE`, `ARROTONDA`, `SOMMA`, `MIN`, `MAX`, `MEDIA` e `ASS`.

Scegli **Aggiungi**. Il nuovo campo viene messo nella casella **Valori**. Compare anche nell'elenco dei campi, dove puoi selezionarlo o deselezionarlo come ogni altro campo. Non può andare in righe, colonne o filtri.

Un campo calcolato viene calcolato in ogni cella del rapporto. Il nome di un campo rappresenta il totale di quel campo nella cella, quindi `=Importo*Importo` eleva al quadrato il totale della cella. Non eleva al quadrato ogni riga e poi le somma. Un totale generale usa i totali di tutto l'elenco.

**Elenca formule** (accanto a **Campo calcolato**) crea un nuovo foglio che riporta il nome di ogni campo calcolato e la sua formula. **Annulla** elimina le celle, e un altro **Annulla** elimina il foglio.

Per modificare un campo, scegli il suo nome nella casella **Nome**. Compare la sua formula. Modificala e scegli **Modifica**. **Elimina** rimuove il campo e il suo valore. Ogni operazione è un passaggio di **Annulla**. Un campo calcolato viene salvato con il file e viene conservato quando il rapporto viene aggiornato. Due tabelle pivot create dagli stessi dati non possono averne uno, perché un campo calcolato è parte dei dati che condividono.

## Mostrare le righe dietro un valore

Fai doppio clic su un valore di una tabella pivot, oppure scegli **Mostra dettagli** nella scheda **Tabella pivot**. JeffCrunch crea un nuovo foglio, chiamato Dettaglio 1, 2 e così via, con le righe dell'elenco che compongono il numero. I nomi dei campi sono nella prima riga. F2 modifica ancora la cella, se la cella può essere modificata. **Annulla** elimina le righe, e un altro **Annulla** elimina il foglio.

## Espandere e comprimere

Un campo i cui elementi hanno altri elementi sotto di sé può mostrare o nascondere le righe sotto i suoi elementi. **Espandi intero campo** e **Comprimi intero campo** lo fanno per ogni elemento del campo selezionato nel riquadro.

## Opzioni della tabella pivot

**Opzioni**, nella scheda **Tabella pivot**, apre una finestra con le impostazioni della tabella.

- **Nome** è il nome della tabella pivot. Nessun'altra tabella pivot della cartella di lavoro può avere lo stesso nome.
- **Per valori di errore mostra** e **Per celle vuote mostra** inseriscono un testo a tua scelta in quelle celle.
- **Unisci e allinea al centro le celle con etichette**, **Adatta larghezza colonne quando la tabella viene aggiornata** e **Mantieni la formattazione delle celle quando la tabella viene aggiornata** vengono salvate con la tabella.
- **Mostra totali generali per le righe** e **per le colonne** equivalgono alle scelte dei Totali generali.
- **Aggiorna i dati all'apertura del file** legge di nuovo le celle ogni volta che la cartella di lavoro viene aperta. È un passaggio di Annulla, come ogni aggiornamento.

## Stile: intestazioni, bande e galleria

Le **Opzioni stile** nella scheda **Tabella pivot** attivano o disattivano la riga di intestazione, la prima colonna (intestazioni di riga) e le bande di righe e di colonne. **Stili** ha una galleria di aspetti, e **Cancella stile** elimina l'aspetto. Questi aspetti vengono disegnati come quelli delle tabelle. **Righe vuote** e **Ripeti etichette** impostano in una volta la riga vuota e le etichette ripetute per ogni campo delle righe.

## Struttura: subtotali, totali generali e layout

- **Subtotali** mostra o nasconde il totale di ciascun gruppo, in alto o in basso nel gruppo. Quando ci sono più campi in Righe, il totale di ogni campo esterno è un subtotale.
- **Totali generali** ha quattro scelte: disattivati per righe e colonne, attivati per entrambe, attivati solo per le righe (una colonna di totali a destra, uno per ogni riga) e attivati solo per le colonne (una riga di totali in basso).
- **Layout rapporto** è la forma delle etichette. In forma **compatta**, tutti i campi delle Righe condividono una colonna, e un nuovo gruppo viene mostrato un po' più a destra. In forma **struttura**, ogni campo ha una propria colonna, e ogni gruppo ha una riga propria sopra i suoi elementi. In forma **tabulare**, ogni campo ha una propria colonna e il nome del gruppo sta sulla sua prima riga, con il subtotale sotto il gruppo.

## Lo stesso elenco, con un filtro

Sposta **Mese** da **Colonne** a **Filtri**. Il rapporto perde le sue colonne, e sopra di esso compare una riga con **Mese** e **(Tutti)**. I numeri sono ora il totale di ciascuna area su tutti i mesi.

## Grafico pivot

Fai clic su una cella di una tabella pivot e scegli **Inserisci**, quindi **Grafico pivot**. JeffCrunch crea un grafico a colonne dei valori della tabella. Esclude i totali generali. Il grafico è contrassegnato nel file come grafico della tabella pivot. Legge le celle della tabella, quindi i suoi numeri cambiano quando aggiorni la tabella o ne cambi il layout. Se la tabella ottiene più righe o colonne, crea di nuovo il grafico, perché il grafico conserva le celle da cui è stato creato. Un grafico pivot proveniente da un file viene mostrato come salvato.

## Segmentazioni dati e sequenze temporali

Una segmentazione dati è un insieme di pulsanti che filtra una tabella pivot, mentre una sequenza temporale è un cursore che fa lo stesso per le date.

Fai clic su una cella di una tabella pivot, poi su **Segmentazioni dati** nella scheda Tabella pivot. Si apre il riquadro Segmentazioni dati, che elenca le segmentazioni e le sequenze temporali della cartella di lavoro. Una segmentazione ha un pulsante per ogni elemento del suo campo. I pulsanti blu sono gli elementi che la tabella mostra.

Fai clic su un pulsante per mostrare solo quell'elemento. Fai clic di nuovo per mostrare tutti gli elementi. Tieni premuto **Ctrl** e fai clic per aggiungere un elemento a quelli mostrati, oppure per toglierne uno. Ogni clic è un passaggio di Annulla, e Annulla rimette a posto la tabella. Quando salvi, il file conserva la scelta della segmentazione.

**Connessioni report** indica quali tabelle pivot filtra una segmentazione. Una sequenza temporale mostra l'intervallo di date che ha scelto. JeffCrunch mostra le segmentazioni in questo riquadro, non sopra le celle, e non modifica ancora una sequenza temporale. Entrambe vengono conservate nel file come sono state salvate.

## Limiti

Una tabella pivot può avere fino a 2.000 campi, un elenco fino a cinque milioni di righe e un rapporto fino a quattro milioni di celle. Un file che supera questi limiti viene mostrato come salvato e non può essere modificato. Rileggere centomila righe di venti colonne e ricalcolare il rapporto ha richiesto circa mezzo secondo sul computer su cui è stato misurato; un computer più lento impiega più tempo.
