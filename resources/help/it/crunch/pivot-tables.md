# Tabelle pivot

<!-- keywords: altro file, altra cartella di lavoro, origine esterna, elenca formule, campo calcolato, elemento calcolato, ordine di calcolo, campo formula, pagine filtro report, tabella pivot, analisi dettagliata, approfondisci, riduci livello, inserisci tabella pivot, crea, realizza, seleziona, sposta tabella pivot, GETPIVOTDATA, genera GETPIVOTDATA, aggiorna, aggiorna tutto, cambia origine dati, elenco campi, campi, righe, colonne, valori, filtri, filtro report, totale generale, subtotale, layout, compatta, struttura, tabulare, cancella, impostazioni campo valore, somma, conteggio, media, mostra valori come, percentuale del totale, differenza, totale parziale, formato numero, impostazioni campo, ordina, filtra, primi, raggruppa, separa, date, mesi, anni, espandi, comprimi, opzioni, stile, righe a bande, righe vuote, ripeti etichette, aggiorna all'apertura del file, grafico pivot, segmentazione, sequenza temporale, mostra dettagli, analisi dettagliata, righe dietro un valore -->

Una tabella pivot è un piccolo rapporto ricavato da un lungo elenco. L'elenco ha una riga per ogni vendita, per esempio; il rapporto somma le vendite per area e per mese. Non occorre scrivere alcuna formula. Si indica quali colonne dell'elenco vanno sul lato, quali in alto e quale va sommata, e il rapporto si compila da solo.

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
| Area | Gen | Feb | Totale complessivo |
| Est | 15 | 20 | 35 |
| Ovest | 7 | 7 | 14 |
| Totale complessivo | 22 | 27 | 49 |

Est a gennaio vale 15 perché si sommano le due righe di Est di gennaio, 10 e 5. La colonna Totale complessivo somma ogni riga, la riga Totale complessivo somma ogni colonna, e la cella in basso a destra somma tutto.

I termini usati qui:

- **Campo** è una colonna dell'elenco. Area, Mese e Importo sono i campi.
- **Righe**, **Colonne** e **Valori** sono le tre posizioni in cui può finire un campo. Un campo in Righe dà una riga per ciascuno dei suoi valori diversi; un campo in Colonne dà una colonna per ciascuno; un campo in Valori è quello che viene sommato. Una quarta posizione, **Filtri**, contiene un campo che esclude le righe che non si scelgono.
- Una **cache** (la cache della tabella pivot) è una copia dell'elenco che la tabella pivot conserva nel file, così da poter mostrare i totali senza leggere le celle. Il rapporto viene calcolato dalla cache, non dalle celle, quindi non cambia quando cambiano le celle. **Aggiorna** copia di nuovo le celle nella cache.

## Creare una tabella pivot

1. Fare clic su una cella dell'elenco, oppure selezionare le celle che si vogliono usare. L'elenco richiede una riga di nomi sopra i dati.
2. Scegliere **Inserisci**, quindi **Tabella pivot**.
3. La finestra mostra le celle in cui si trovano i dati. Modificarle se non sono corrette. È anche possibile digitare il nome di una tabella. Per usare un elenco che si trova in un altro file, scegliere **Un intervallo in un altro file**, selezionare il file con **Sfoglia** e digitare l'intervallo, oppure il nome di una tabella o di un nome definito in quel file, ad esempio `Foglio1!A1:D100`. JeffCrunch legge le cartelle di lavoro (`.xlsx`, `.xlsm`, `.xls` e `.ods`) e i file `.csv`.
4. Scegliere dove collocare la tabella pivot: **Nuovo foglio** (impostazione predefinita), oppure una cella di questo foglio o di un altro. Scegliere un punto in cui non ci sia nient'altro, perché la tabella vuota occupa circa diciotto righe e tre colonne.
5. Scegliere **OK**. La tabella parte vuota e si apre il riquadro **Campi tabella pivot**. Selezionare i campi desiderati: un campo di numeri viene sommato, mentre ogni altro campo diventa una riga. Quindi spostare i campi nelle caselle come spiegano le sezioni successive.

Quando i dati si trovano in un altro file, la tabella pivot conserva una copia di essi, così la tabella funziona e si salva anche senza quel file. Vedere Dati in un altro file più avanti.

Un nuovo foglio costituisce un passaggio a sé di Annulla, e lo stesso vale per la tabella. Quando si salva come .xlsx, la tabella viene scritta nel modo in cui la scrive il programma originale, quindi si apre lì come tabella pivot. Una copia salvata come .xls o .ods conserva nelle celle i numeri mostrati dalla tabella, ma non la tabella stessa, e il salvataggio lo segnala.

## GETPIVOTDATA: leggere un solo numero da una tabella pivot

`GETPIVOTDATA` restituisce il numero che una tabella pivot mostra per i valori, le righe e le colonne indicati. Per l'esempio sopra:

`=GETPIVOTDATA("Somma di Importo"; $E$3; "Area"; "Est"; "Mese"; "Gen")` restituisce 15.

- Il primo argomento è il nome del valore, così come lo mostra la tabella (per esempio Somma di Importo). Funziona anche il nome del campo che viene sommato (Importo).
- Il secondo argomento è una qualsiasi cella della tabella pivot.
- Seguono coppie di un campo e di uno dei suoi elementi. Indicando solo Area ed Est si ottiene il totale di Est, 35. Senza indicare nulla si ottiene il totale complessivo, 49. I campi indicati devono essere i primi campi delle righe o i primi campi delle colonne, nell'ordine.
- Se la tabella non mostra il numero, la cella visualizza `#RIF!`.

Quando si fa clic su un valore di una tabella pivot mentre si digita una formula, JeffCrunch scrive per conto dell'utente questa formula, in modo che continui a leggere il numero giusto dopo che la tabella è stata riorganizzata. Per disattivare questa funzione, scegliere **Opzioni** nella scheda **Tabella pivot** e deselezionare la casella relativa a GETPIVOTDATA.

## Come JeffCrunch tratta una tabella pivot

Una tabella pivot presente in una cartella di lavoro viene mostrata come è stata salvata. Se non si modifica nulla, il file viene salvato con la tabella pivot esattamente com'era. Quando la si aggiorna o la si modifica, JeffCrunch ricalcola il rapporto, inserisce i nuovi numeri nelle celle e riscrive la tabella pivot in modo che il programma che l'ha creata la apra senza segnalare problemi.

JeffCrunch calcola le tabelle pivot che sommano, contano, calcolano medie e così via, che raggruppano date e numeri, e che mostrano i valori come percentuale, differenza o totale parziale. Una tabella pivot che mostra un valore come classifica o come percentuale di un elemento padre, che contiene gruppi di elementi creati a mano o che usa un modello di dati viene mostrata e conservata come salvata, e i comandi segnalano che non è possibile modificarla.

Fare clic su una cella qualsiasi di una tabella pivot. Compare la scheda **Tabella pivot**.

## Aggiorna

**Aggiorna** legge le celle da cui è costruita la tabella pivot e ricalcola il rapporto. I nuovi elementi si aggiungono dopo quelli già presenti; gli elementi che non esistono più vengono eliminati. Le larghezze delle colonne e il layout scelto restano invariati. **Aggiorna tutto**, nella scheda **Dati** e nella scheda **Tabella pivot**, fa lo stesso per tutte le tabelle pivot della cartella di lavoro.

Se il rapporto dovesse estendersi su celle che contengono dati, JeffCrunch non aggiorna e indica quali celle ostacolano l'operazione. Cancellarle o spostare la tabella pivot, quindi aggiornare di nuovo.

**Annulla** riporta la situazione precedente all'aggiornamento con un solo passaggio.

## Cambia origine dati

**Cambia origine dati** chiede le celle in cui si trova l'elenco, compresa la prima riga dei nomi. Digitarle nella forma `Foglio1!A1:D100`, oppure digitare il nome di una tabella o di un nome definito. JeffCrunch legge le celle e ricalcola il rapporto. Se un campo dell'elenco ha lo stesso nome nelle nuove celle, la sua posizione nel rapporto resta invariata.

È anche possibile far puntare la tabella a un intervallo di un altro file. Nella finestra, scegliere **Un intervallo in un altro file**, selezionare il file e digitare l'intervallo al suo interno. Scegliendo **Un intervallo in questa cartella di lavoro** la tabella torna a puntare a celle di questa cartella. Ciascuna di queste modifiche costituisce un passaggio di Annulla.

## Dati in un altro file

Una tabella pivot creata da un altro file conserva i dati da cui è stata costruita all'interno di questa cartella di lavoro, così è possibile aprire, modificare e salvare la cartella di lavoro anche quando l'altro file non c'è più. **Aggiorna** legge di nuovo l'altro file. Se il file è stato spostato, manca, è troppo grande, non è una cartella di lavoro o non contiene più l'intervallo, la tabella conserva i dati che aveva e un messaggio segnala che il file non può essere letto. Un file che si trova su un altro computer della rete non viene letto finché non lo si sceglie personalmente in **Cambia origine dati**. Un aggiornamento legge ciascun file una sola volta, e al massimo 32 file, 1 GB e 30 secondi in tutto; una tabella che supera questi limiti conserva i propri dati.

Se una cartella di lavoro indica che la tabella deve essere letta di nuovo all'apertura, e i dati della tabella si trovano in un altro file, JeffCrunch chiede prima conferma, come fa per i collegamenti ad altre cartelle di lavoro. Finché non si risponde sì, non viene letto alcun file.

Una cartella di lavoro salvata come .xlsx memorizza dove si trova il file. Scegliendo il file nella finestra viene memorizzato il suo indirizzo completo. Una cartella di lavoro creata da un altro programma, la cui tabella punta a un file con un percorso breve come `Prezzi.xlsx`, conserva quel percorso, e JeffCrunch cerca il file nella stessa cartella della cartella di lavoro. Quando si usa **Salva con nome** per salvare la cartella di lavoro in un'altra cartella, un percorso di questo tipo viene riscritto in modo da indicare sempre lo stesso file. Aggiorna non segue eventuali collegamenti che il file di origine ha ad altri file.

## Cancella

**Cancella** offre due scelte. **Cancella filtri** rimuove tutti i filtri e mostra di nuovo tutte le voci. **Cancella tutto** elimina tutti i campi dal rapporto e lo lascia vuoto, pronto per essere ricostruito.

## Analisi dettagliata e analisi di sintesi

## Pagine filtro report

Un filtro si trova sopra una tabella pivot e permette di esaminare un elemento alla volta, per esempio un mese. **Mostra pagine filtro report** nella scheda **Tabella pivot** crea una pagina per ogni elemento contemporaneamente. Ogni pagina è un nuovo foglio, con il nome dell'elemento e con una propria copia della tabella. Il filtro di ciascuna copia è impostato su quell'elemento. Se la tabella ha più di un filtro, si sceglie quale usare. Un solo **Annulla** elimina tutte le pagine.

Quando un rapporto ha più di un campo nelle righe, un elemento del primo campo può essere aperto o chiuso. Fare clic sulla cella con il nome dell'elemento e scegliere **Riduci livello** per chiuderlo: le righe del campo successivo che stanno sotto di esso vengono nascoste e resta solo il suo totale. **Analisi dettagliata** lo riapre. Un elemento dell'ultimo campo non ha nulla sotto di sé. Ogni operazione è un solo **Annulla**.

## Selezionare e spostare

**Seleziona**, nella scheda **Tabella pivot**, seleziona una parte del rapporto: **Intera tabella pivot**, **Valori** (i numeri) oppure **Etichette** (i nomi sul lato; se il rapporto non ne ha sul lato, i nomi in alto). **Sposta tabella pivot…** chiede una cella dello stesso foglio e ridisegna il rapporto con la prima cella in quel punto. Le celle che restano libere vengono svuotate. Le formule che puntano alle celle del rapporto le seguono, come accade quando si taglia e si incolla. Se il nuovo punto contiene già qualcosa, JeffCrunch non sposta il rapporto e lo segnala. Un solo **Annulla** lo riporta al punto di partenza.

## Il riquadro dei campi

**Elenco campi**, nella scheda **Tabella pivot**, apre il riquadro **Campi tabella pivot**. Mostra i campi dell'elenco con una casella accanto a ciascuno, e sotto di essi le quattro posizioni: **Filtri**, **Colonne**, **Righe** e **Valori**.

- Selezionare un campo per aggiungerlo. Un campo che contiene solo numeri va in **Valori** e viene sommato; ogni altro va in **Righe**.
- Deselezionare un campo per rimuoverlo da ogni posizione.
- Fare clic su un campo in una delle quattro posizioni e scegliere **Sposta in** per metterlo in un'altra. Le voci **Sposta su** e **Sposta giù** ne cambiano l'ordine nella propria posizione, e **Rimuovi campo** lo elimina.
- Con più di un valore, in **Colonne** compare una riga denominata **Valori**. Spostarla in **Righe** per mettere i valori sul lato.
- **Impostazioni campo valore** (doppio clic su un valore, oppure il pulsante) imposta il nome che il rapporto mostra per il valore e il modo in cui viene totalizzato: **Somma**, **Conteggio**, **Conteggio numeri**, **Media**, **Max**, **Min**, **Prodotto**, **DevStd** e **DevStdp** (lo scarto quadratico medio di un campione, oppure di tutti i valori), e **Var** e **Varp** (la varianza degli stessi). Imposta anche il modo in cui il valore viene mostrato e il suo formato numero. Vedere la sezione seguente.
- **Impostazioni campo** (doppio clic su un campo in Filtri, Colonne o Righe, oppure il pulsante) imposta un singolo campo. Vedere "Impostazioni campo".

Ogni modifica costituisce un passaggio di **Annulla**.

## Mostra valori come e formato numero di un valore

In **Impostazioni campo valore**, **Mostra valori come** cambia ciò che mostra una cella del rapporto.

- **% del totale complessivo**: la cella divisa per il totale di tutto il rapporto. Nell'esempio, Est a gennaio vale 15 su 49, cioè 30,6%.
- **% del totale di colonna** e **% del totale di riga**: la cella divisa per il totale della sua colonna o della sua riga.
- **Indice**: il confronto tra la cella e ciò che la sua riga e la sua colonna darebbero se i dati fossero distribuiti in modo uniforme. Un numero superiore a 1 indica più della quota uniforme.
- **Differenza da**, **% di** e **% differenza da**: la cella confrontata con un altro elemento di un campo. Scegliere il **Campo base** (un campo in Righe o in Colonne) e l'**Elemento base** (uno dei suoi elementi, oppure quello precedente o successivo). Per esempio, con Mese come campo base e Gen come elemento base, febbraio mostra di quanto è maggiore o minore di gennaio. Per una differenza l'elemento base stesso resta vuoto, mentre per **% di** vale 100%.
- **Totale parziale in**: ogni cella somma le celle che la precedono lungo il campo base.
- **Classifica dal più piccolo al più grande** e **Classifica dal più grande al più piccolo**: ogni cella mostra la sua posizione, 1, 2, 3 e così via, tra le celle del campo base nella stessa riga o colonna. Il rango 1 è il numero più piccolo, oppure il più grande.
- **% del totale della riga padre** e **% del totale della colonna padre**: ogni cella mostra quale parte del suo padre rappresenta. Il padre di un elemento è l'elemento di livello superiore, come l'anno che contiene un mese. Un elemento senza padre viene mostrato come parte del totale complessivo.

**Formato numero** accetta un codice di formato, come `0,00`, `#.##0` o `0,0%`. Lasciarlo vuoto per mantenere il formato che hanno le celle. Un valore mostrato come percentuale riceve `0,00%`, a meno che non si scelga un altro formato. Le celle del rapporto assumono subito il formato. **Annulla** lo ripristina insieme ai numeri.

## Impostazioni campo

Selezionare un campo nella casella **Filtri**, **Colonne** o **Righe** e scegliere **Impostazioni campo** nella scheda **Tabella pivot** (oppure fare doppio clic su di esso).

- **Nome personalizzato** cambia il nome che il rapporto mostra per il campo.
- **Subtotali**: **Automatico** mostra un totale per ogni gruppo del campo, **Nessuno** non lo mostra.
- **Mostra etichette elementi in**: forma compatta, a struttura o tabulare, come in Layout report.
- **Inserisci una riga vuota dopo ogni elemento** aggiunge una riga vuota dopo ciascun gruppo.
- **Ripeti etichette elementi** mostra il nome di un elemento esterno su ogni riga del suo gruppo (forme a struttura e tabulare).
- **Ordina**: mantiene l'ordine in cui si trovano gli elementi (manuale), oppure ordina dalla A alla Z o dalla Z alla A in base alle etichette. **Dal più piccolo al più grande per valore** e **Dal più grande al più piccolo per valore** ordinano gli elementi in base ai numeri di un campo valore. Il rapporto mantiene l'ordinamento quando cambiano i dati.
- **Filtro** esclude elementi in base alle loro etichette (inizia con, termina con, contiene, uguale a) o in base a un valore (maggiore di, minore di, compreso tra e così via), oppure mantiene solo i primi o gli ultimi elementi in base a un valore. Il filtro viene calcolato quando si sceglie OK. Nasconde gli elementi che esclude; le caselle sottostanti indicano quali sono, e selezionando un elemento lo si mostra di nuovo.
- **Mostra questi elementi** è un elenco degli elementi del campo, ciascuno con una casella di controllo. Deselezionare una casella per nascondere l'elemento, e i totali lo escludono.

## Raggruppare date e numeri

Selezionare un campo di date o di numeri in **Righe** o in **Colonne** e scegliere **Raggruppa campo** nella scheda **Tabella pivot**. I gruppi vengono creati dai dati, quindi in quella colonna devono esserci solo numeri o solo date, senza testo e senza celle vuote.

- **Date**: selezionare il criterio di raggruppamento: **Secondi**, **Minuti**, **Ore**, **Giorni**, **Mesi**, **Trimestri** oppure **Anni**. La parte più fine selezionata resta sul campo. Per ogni parte più grossolana JeffCrunch aggiunge un campo, denominato di conseguenza (per esempio **Anni**), e lo colloca prima del campo data nello stesso punto. **Inizia da** e **Termina a** indicano la prima e l'ultima data; lasciarle così come sono per usare la data minima e quella massima dei dati. Le date precedenti l'inizio o successive alla fine finiscono in due gruppi aggiuntivi, che iniziano con `<` e `>`.
- **Numeri**: **Inizia da**, **Termina a** e **Di** creano gruppi come 1-10, 11-20 e così via.

**Separa** elimina i gruppi e i campi aggiunti per essi. **Aggiorna** ricrea i gruppi a partire dai nuovi dati. Quando due tabelle pivot sono create dagli stessi dati, non è possibile creare gruppi, perché un gruppo è una parte dei dati che esse condividono.

## Campi calcolati

Un campo calcolato è un campo creato con una formula. Usa altri campi dell'elenco. Per esempio, un premio pari a un decimo dell'importo è `=Importo*0,1`.

Scegliere **Campo calcolato** nella scheda **Tabella pivot**. Digitare un nome e una formula. Scrivere nella formula il nome di un campo, oppure fare doppio clic su di esso nell'elenco **Campi** per inserirlo. Un nome che contiene spazi va racchiuso tra apici singoli, come in `='Prezzo unitario'*Quantità`. Si possono usare numeri, `+ - * / ^ %`, confronti e alcune funzioni come `SE`, `ARROTONDA`, `SOMMA`, `MIN`, `MAX`, `MEDIA` e `ASS`.

Scegliere **Aggiungi**. Il nuovo campo viene inserito nella casella **Valori**. Compare anche nell'elenco dei campi, dove può essere selezionato o deselezionato come ogni altro campo. Non può andare nelle righe, nelle colonne o nei filtri.

Un campo calcolato viene calcolato in ogni cella del rapporto. Il nome di un campo rappresenta il totale di quel campo nella cella, quindi `=Importo*Importo` eleva al quadrato il totale della cella. Non eleva al quadrato ciascuna riga per poi sommarle. Un totale complessivo usa i totali dell'intero elenco.

**Elenca formule** (accanto a **Campo calcolato**) crea un nuovo foglio che riporta il nome e la formula di ogni campo calcolato, e ogni elemento calcolato con la sua formula, il suo campo e il suo ordine di calcolo. **Annulla** elimina le celle, e un altro **Annulla** elimina il foglio.

Per modificare un campo, selezionare il suo nome nella casella **Nome**. Compare la sua formula. Modificarla e scegliere **Modifica**. **Elimina** rimuove il campo e il suo valore. Ogni operazione è un passaggio di **Annulla**. Un campo calcolato viene salvato con il file e viene conservato quando il rapporto viene aggiornato. Due tabelle pivot create dagli stessi dati non possono averne uno, perché un campo calcolato è una parte dei dati che esse condividono.

## Elementi calcolati

Un elemento calcolato è un elemento creato a partire da altri elementi dello stesso campo. Per esempio, il campo Area ha gli elementi Est e Ovest. Un nuovo elemento chiamato Entrambi può essere `=Est+Ovest`.

Fare clic su una cella della tabella pivot e scegliere **Elemento calcolato** nella scheda **Tabella pivot**. Scegliere il **Campo**, che deve trovarsi nelle righe o nelle colonne. Digitare un **Nome** e una **Formula**. Scrivere nella formula il nome di un elemento, oppure fare doppio clic su di esso nell'elenco **Elementi** per inserirlo. Un nome che contiene spazi va racchiuso tra apici singoli, come in `='Est grande'+Ovest`. Si possono usare numeri, `+ - * / ^ %` e parentesi.

Il nuovo elemento ottiene una riga nel rapporto. In ogni colonna mostra la formula calcolata a partire dalle righe degli elementi che nomina. L'elemento nuovo viene conteggiato nei totali. Un totale di somme, come un totale complessivo o un subtotale, aggiunge la riga del nuovo elemento alle righe degli altri elementi. Quindi, con Est a 35, Ovest a 14 e Entrambi (Est più Ovest) a 49, il totale complessivo è 98. Se un elemento nominato nella formula è nascosto, la cella mostra **#RIF!**. Se la formula nomina un elemento che non esiste, la cella mostra **#NOME?**.

Per modificare un elemento, selezionare il suo nome nella casella **Nome**. Compare la sua formula. Modificarla e scegliere **Modifica**. **Elimina** rimuove l'elemento e la sua riga. Ogni operazione è un passaggio di **Annulla**. Un elemento calcolato viene salvato con il file e viene conservato quando il rapporto viene aggiornato. Due tabelle pivot create dagli stessi dati non possono averne uno, perché un elemento calcolato è una parte dei dati che esse condividono.

### Ordine di calcolo

A volte una cella ha due elementi calcolati: uno nella sua riga e l'altro nella sua colonna. L'ordine in cui vengono calcolati può cambiare il risultato. Supponiamo che l'elemento di riga sia `=Est-Ovest` e quello di colonna sia `=Gen+1`. Calcolato in un ordine, la cella vale (Est - Ovest) + 1. Nell'altro ordine vale (Est + 1) - (Ovest + 1), che è un numero diverso.

**Ordine di calcolo**, nella scheda **Tabella pivot**, elenca gli elementi calcolati nell'ordine in cui vengono calcolati. L'elemento più in basso nell'elenco viene calcolato per ultimo. Selezionare un elemento e scegliere **Sposta su** o **Sposta giù**. L'ordine viene salvato con il file come ordine degli elementi, e gli altri programmi lo leggono allo stesso modo. **Elenca formule** mostra il posto di ciascun elemento in questo ordine.

## Mostrare le righe dietro un valore

Fare doppio clic su un valore di una tabella pivot, oppure scegliere **Mostra dettagli** nella scheda **Tabella pivot**. JeffCrunch crea un nuovo foglio, denominato Dettaglio 1, 2 e così via, con le righe dell'elenco che compongono il numero. I nomi dei campi si trovano nella prima riga. F2 modifica ancora la cella, se la cella può essere modificata. **Annulla** elimina le righe, e un altro **Annulla** elimina il foglio.

## Espandere e comprimere

Un campo i cui elementi hanno altri elementi sotto di sé può mostrare o nascondere le righe sotto i suoi elementi. **Espandi intero campo** e **Comprimi intero campo** lo fanno per ogni elemento del campo selezionato nel riquadro.

## Opzioni tabella pivot

**Opzioni** nella scheda **Tabella pivot** apre una finestra con le impostazioni della tabella.

- **Nome** è il nome della tabella pivot. Nessun'altra tabella pivot della cartella di lavoro può avere lo stesso nome.
- **Per i valori di errore mostra** e **Per le celle vuote mostra** inseriscono in quelle celle un testo personalizzato.
- **Unisci e allinea al centro le celle con le etichette**, **Adatta la larghezza delle colonne all'aggiornamento della tabella** e **Mantieni la formattazione delle celle all'aggiornamento della tabella** vengono salvate con la tabella.
- **Mostra totali generali per le righe** e **per le colonne** equivalgono alle scelte dei Totali generali.
- **Aggiorna i dati all'apertura del file** legge di nuovo le celle ogni volta che la cartella di lavoro viene aperta. Costituisce un passaggio di Annulla, come ogni aggiornamento.

## Stile: intestazioni, bande e galleria

Le **Opzioni stile** nella scheda **Tabella pivot** attivano o disattivano la riga di intestazione, la prima colonna (intestazioni di riga) e le bande di righe e di colonne. **Stili** offre una galleria di aspetti, e **Cancella stile** elimina l'aspetto. Questi aspetti vengono disegnati nello stesso modo degli aspetti delle tabelle. **Righe vuote** e **Ripeti etichette** impostano la riga vuota e le etichette ripetute per tutti i campi delle righe contemporaneamente.

## Struttura: subtotali, totali generali e layout

- **Subtotali** mostra o nasconde il totale di ogni gruppo, in alto o in basso rispetto al gruppo. Quando nelle Righe c'è più di un campo, il totale di ciascun campo esterno è un subtotale.
- **Totali generali** offre quattro scelte: disattivati per righe e colonne, attivati per entrambi, attivati solo per le righe (una colonna di totali a destra, uno per ogni riga) e attivati solo per le colonne (una riga di totali in basso).
- **Layout report** determina la forma delle etichette. In forma **compatta**, tutti i campi delle Righe condividono una sola colonna, e un nuovo gruppo viene mostrato un po' più a destra. In forma **a struttura**, ogni campo ha una colonna propria, e ogni gruppo ha una riga propria sopra i suoi elementi. In forma **tabulare**, ogni campo ha una colonna propria e il nome del gruppo sta sulla sua prima riga, con il subtotale sotto il gruppo.

## Lo stesso elenco, con un filtro

Spostare **Mese** da **Colonne** a **Filtri**. Il rapporto perde le sue colonne, e una riga sopra di esso mostra **Mese** e **(Tutti)**. I numeri ora sono il totale di ciascuna area su tutti i mesi.

## Grafico pivot

Fare clic su una cella di una tabella pivot e scegliere **Inserisci**, quindi **Grafico pivot**. JeffCrunch crea un grafico a colonne dei valori della tabella. Esclude i totali complessivi. Il grafico viene segnato nel file come grafico della tabella pivot. Legge le celle della tabella, quindi i suoi numeri cambiano quando si aggiorna la tabella o se ne cambia il layout. Se la tabella acquisisce altre righe o colonne, creare di nuovo il grafico, perché il grafico conserva le celle da cui è stato creato. Un grafico pivot proveniente da un file viene mostrato come salvato.

## Segmentazioni e sequenze temporali

Una segmentazione è un insieme di pulsanti che filtra una tabella pivot, mentre una sequenza temporale è un cursore che fa lo stesso per le date.

Fare clic su una cella di una tabella pivot, quindi su **Segmentazioni** nella scheda Tabella pivot. Si apre il riquadro Segmentazioni, che elenca le segmentazioni e le sequenze temporali della cartella di lavoro. Una segmentazione ha un pulsante per ciascun elemento del suo campo. I pulsanti in blu sono gli elementi mostrati dalla tabella.

Fare clic su un pulsante per mostrare solo quell'elemento. Fare di nuovo clic per mostrare tutti gli elementi. Tenere premuto **Ctrl** e fare clic per aggiungere un elemento a quelli mostrati, oppure per toglierlo. Ogni clic costituisce un passaggio di Annulla, e Annulla riporta la tabella allo stato precedente. Al salvataggio il file conserva la scelta della segmentazione.

Una segmentazione presente nel file viene disegnata anche sopra le celle, nel punto in cui il file la colloca. Mostra la sua didascalia e un pulsante per ciascun elemento. I pulsanti in blu sono gli elementi mostrati dalla tabella. Fare clic su un pulsante per mostrare solo quell'elemento, tenere premuto **Ctrl** e fare clic per aggiungere un elemento o toglierlo, e fare clic sull'imbuto in alto a destra per mostrare di nuovo tutti gli elementi. Ogni clic costituisce un passaggio di Annulla. Quando la tabella viene filtrata in altro modo, oppure si usa Annulla, i pulsanti seguono la situazione. Quando una segmentazione è troppo piccola per mostrare tutti i suoi pulsanti, sul suo lato destro compare una barra di scorrimento: far scorrere la rotellina del mouse sulla segmentazione, fare clic sopra o sotto il cursore della barra, oppure trascinarlo per raggiungere il resto. Tenere premuto **Maiusc** e fare clic su un secondo pulsante per scegliere l'intera serie di elementi dal primo pulsante cliccato fino a questo; tenere premuti **Maiusc** e **Ctrl** per aggiungere la serie a quelli già scelti. Una segmentazione stampata non può scorrere, quindi stampa i pulsanti che ci stanno, e la sua ultima riga indica quanti altri elementi ci sono. Una segmentazione che filtra una tabella diversa da una tabella pivot, oppure la cui tabella pivot non è nel file, viene disegnata con la sua didascalia e una frase che indica che JeffCrunch non può ancora usarla; viene conservata nel file così com'è. Una cartella di lavoro aperta solo in lettura, e un foglio protetto contro la modifica delle tabelle pivot, non consentono che un clic filtri la tabella. Una segmentazione viene stampata insieme al foglio, a meno che il file non lo escluda. Fare clic su un pulsante di una segmentazione la filtra, mentre fare clic sulla sua didascalia o sul suo bordo la seleziona come oggetto, come un'immagine. Una segmentazione selezionata ha delle maniglie: trascinarla per spostarla, trascinare una maniglia per ridimensionarla, oppure premere un tasto freccia per spostarla. Tenere premuto **Ctrl** o **Maiusc** e fare clic sulla didascalia per selezionarla insieme ad altri oggetti. Fare clic con il pulsante destro su una segmentazione per aprire un menu con **Elimina** e il riquadro **Segmentazioni**. Premere **Canc** per rimuovere una segmentazione selezionata: il file non conterrà più la segmentazione, e il filtro che essa applicava alla tabella pivot resta finché non lo si modifica. Spostare, ridimensionare ed eliminare sono operazioni che costituiscono ciascuna un passaggio di Annulla, e una cartella di lavoro aperta solo in lettura, oppure un foglio protetto contro la modifica degli oggetti, non le consente.

Per usare una segmentazione da tastiera, premere **F6** per passare alla prima segmentazione del foglio (**Maiusc+F6** va all'ultima). Poi usare i tasti freccia, **Home**, **Fine**, **Pagina su** e **Pagina giù** per muoversi tra i suoi pulsanti, **Spazio** o **Invio** per scegliere il pulsante, **Ctrl+Spazio** per aggiungerlo agli elementi scelti o toglierlo, **Maiusc+Spazio** per scegliere la serie dall'ultimo pulsante scelto, e **Ctrl+Alt+C** per mostrare di nuovo tutti gli elementi. Premere **Esc** per uscire dai pulsanti (i tasti freccia spostano allora la segmentazione, e **Canc** la elimina; mentre ci si trova tra i pulsanti, **Canc** non fa nulla), e **Esc** di nuovo per tornare alle celle. **F6** passa alla segmentazione successiva, e dopo l'ultima torna alle celle.

**Connessioni report** indica quali tabelle pivot filtra una segmentazione. Una sequenza temporale mostra l'intervallo di date scelto. JeffCrunch non disegna ancora una sequenza temporale sopra le celle né la modifica. Entrambe vengono conservate nel file così come sono state salvate.

## Limiti

Una tabella pivot può avere fino a 2.000 campi, un elenco fino a cinque milioni di righe e un rapporto fino a quattro milioni di celle. Un file che supera questi valori viene mostrato come salvato e non può essere modificato. Rileggere centomila righe di venti colonne e calcolare il rapporto ha richiesto circa mezzo secondo sul computer su cui è stato misurato; un computer più lento impiega più tempo.
