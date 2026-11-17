# Tabelle pivot

<!-- keywords: altro file, altra cartella di lavoro, origine esterna, formule elenco, campo calcolato, elemento calcolato, ordine di calcolo, campo formula, pagine filtro report, tabella pivot, analisi dettagliata, approfondisci, riduci livello, inserisci tabella pivot, crea, realizza, seleziona, sposta tabella pivot, getpivotdata, genera getpivotdata, aggiorna, aggiorna tutto, cambia origine dati, elenco campi, campi, righe, colonne, valori, filtri, filtro report, totale generale, subtotale, layout, compatto, struttura, tabulare, cancella, impostazioni campo valore, somma, conteggio, media, mostra valori come, percentuale del totale, differenza, totale parziale, formato numero, impostazioni campo, ordina, filtra, primi, raggruppa, separa, date, mesi, anni, espandi, comprimi, opzioni, stile, righe a bande, righe vuote, ripeti etichette, aggiorna all'apertura del file, grafico pivot, segmentazione dati, sequenza temporale, mostra dettagli, analisi dettagliata, righe dietro un valore -->

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
| Area | Gen | Feb | Totale generale |
| Est | 15 | 20 | 35 |
| Ovest | 7 | 7 | 14 |
| Totale generale | 22 | 27 | 49 |

Est a gennaio vale 15 perché si sommano le due righe Est di gennaio, 10 e 5. La colonna Totale generale somma ogni riga, la riga Totale generale somma ogni colonna, e la cella in basso a destra somma tutto.

I termini usati qui:

- **Campo** è una colonna dell'elenco. Area, Mese e Importo sono i campi.
- **Righe**, **Colonne** e **Valori** sono i tre punti in cui un campo può essere inserito. Un campo in Righe dà una riga per ciascun suo valore diverso; un campo in Colonne dà una colonna per ciascuno; un campo in Valori è quello che viene sommato. Un quarto punto, **Filtri**, contiene un campo che esclude le righe che non si scelgono.
- Una **cache** (la cache della tabella pivot) è una copia dell'elenco che la tabella pivot conserva nel file, così può mostrare i totali senza leggere le celle. Il rapporto viene calcolato dalla cache, non dalle celle, quindi non cambia quando cambiano le celle. **Aggiorna** copia di nuovo le celle nella cache.

## Creare una tabella pivot

1. Fare clic su una cella dell'elenco, oppure selezionare le celle da usare. L'elenco richiede una riga di nomi sopra i dati.
2. Scegliere **Inserisci**, quindi **Tabella pivot**.
3. La finestra mostra le celle in cui si trovano i dati. Modificarle se non sono corrette. Si può anche digitare il nome di una tabella. Per usare un elenco che si trova in un altro file, scegliere **Un intervallo in un altro file**, selezionare il file con **Sfoglia** e digitare l'intervallo, oppure il nome di una tabella o di un nome definito in quel file, ad esempio `Foglio1!A1:D100`. JeffCrunch legge le cartelle di lavoro (`.xlsx`, `.xlsm`, `.xls` e `.ods`) e i file `.csv`.
4. Scegliere dove collocare la tabella pivot: **Nuovo foglio** (impostazione predefinita), oppure una cella di questo foglio o di un altro. Scegliere un punto in cui non ci sia nient'altro, perché la tabella vuota occupa circa diciotto righe e tre colonne.
5. Scegliere **OK**. La tabella inizia vuota e si apre il riquadro **Campi tabella pivot**. Selezionare i campi che servono: un campo di numeri viene sommato, mentre ogni altro campo diventa una riga. Poi spostare i campi nelle caselle come spiegano le sezioni successive.

Quando i dati si trovano in un altro file, la tabella pivot conserva una copia di quei dati, quindi la tabella funziona e viene salvata anche senza quel file. Vedere Dati in un altro file più avanti.

Un nuovo foglio costituisce un passaggio separato di **Annulla**, così come la tabella. Quando si salva come .xlsx, la tabella viene scritta nel modo in cui la scrive il programma originale, quindi si apre lì come tabella pivot. Una copia salvata come .xls o .ods conserva nelle celle i numeri mostrati dalla tabella, ma non la tabella stessa, e il salvataggio lo segnala.

## GETPIVOTDATA: leggere un numero da una tabella pivot

`GETPIVOTDATA` restituisce il numero che una tabella pivot mostra per i valori e per le righe e colonne indicati. Per l'esempio sopra:

`=GETPIVOTDATA("Somma di Importo"; $E$3; "Area"; "Est"; "Mese"; "Gen")` restituisce 15.

- Il primo argomento è il nome del valore, come lo mostra la tabella (ad esempio Somma di Importo). Funziona anche il nome del campo che viene sommato (Importo).
- Il secondo argomento è una cella qualsiasi della tabella pivot.
- Seguono coppie formate da un campo e da uno dei suoi elementi. Indicando solo Area ed Est, si ottiene il totale di Est, 35. Senza indicare nulla, si ottiene il totale generale, 49. I campi indicati devono essere i primi campi nelle righe, oppure i primi campi nelle colonne, in quest'ordine.
- Se la tabella non mostra il numero, la cella visualizza `#RIF!`.

Quando, mentre si digita una formula, si fa clic su un valore di una tabella pivot, JeffCrunch scrive questa formula al posto dell'utente, in modo che continui a leggere il numero giusto dopo che la tabella viene riorganizzata. Per disattivarlo, scegliere **Opzioni** nella scheda **Strumenti tabella pivot** e deselezionare la casella relativa a GETPIVOTDATA.

## Che cosa fa JeffCrunch con una tabella pivot

Una tabella pivot presente in una cartella di lavoro viene mostrata così come è stata salvata. Se non si modifica nulla, il file viene salvato con la tabella pivot esattamente com'era. Quando la si aggiorna o la si modifica, JeffCrunch ricalcola il rapporto, inserisce i nuovi numeri nelle celle e riscrive la tabella pivot in modo che il programma che l'ha creata la apra senza errori.

JeffCrunch elabora le tabelle pivot che sommano, contano, calcolano medie e così via, che raggruppano date e numeri, e che mostrano i valori come percentuale, differenza o totale parziale. Una tabella pivot che mostra un valore come classifica o come percentuale di un elemento padre, che contiene gruppi di elementi creati a mano, oppure che usa un modello di dati viene mostrata e conservata come salvata, e i comandi segnalano che non è possibile modificarla.

Fare clic su una cella qualsiasi di una tabella pivot. Compare la scheda **Strumenti tabella pivot**.

## Aggiorna

**Aggiorna** legge le celle da cui è composta la tabella pivot e ricalcola il rapporto. I nuovi elementi si aggiungono dopo quelli già presenti; quelli eliminati vengono tolti. Larghezze di colonna e layout scelto restano invariati. **Aggiorna tutto**, nella scheda **Dati** e nella scheda **Strumenti tabella pivot**, esegue la stessa operazione per tutte le tabelle pivot della cartella di lavoro.

Se il rapporto dovesse estendersi su celle che contengono dati, JeffCrunch non aggiorna la tabella e indica quali celle sono di intralcio. Cancellarle o spostare la tabella pivot, quindi aggiornare di nuovo.

**Annulla** annulla un aggiornamento con un solo passaggio.

## Cambia origine dati

**Cambia origine dati** chiede le celle in cui si trova l'elenco, compresa la prima riga dei nomi. Digitarle come `Foglio1!A1:D100`, oppure digitare il nome di una tabella o un nome definito. JeffCrunch legge le celle e ricalcola il rapporto. Se un campo dell'elenco ha lo stesso nome nelle nuove celle, la sua posizione nel rapporto resta invariata.

Si può anche indirizzare la tabella verso un intervallo di un altro file. Nella finestra scegliere **Un intervallo in un altro file**, selezionare il file e digitare l'intervallo al suo interno. Scegliere **Un intervallo in questa cartella di lavoro** riporta la tabella alle celle di questo file. Ciascuna di queste modifiche è un passaggio di **Annulla**.

## Dati in un altro file

Una tabella pivot creata da un altro file conserva al suo interno, in questa cartella di lavoro, i dati da cui è stata creata, così è possibile aprire, modificare e salvare la cartella anche quando l'altro file non c'è più. **Aggiorna** legge di nuovo l'altro file. Se il file è stato spostato, manca, è troppo grande, non è una cartella di lavoro o non contiene più l'intervallo, la tabella mantiene i dati che aveva e un messaggio indica che il file non può essere letto. Un file che si trova su un altro computer della rete non viene letto finché non lo si sceglie personalmente in **Cambia origine dati**. Un aggiornamento legge ciascun file una sola volta, e al massimo 32 file, 1 GB e 30 secondi in tutto; una tabella che supera questi limiti conserva i propri dati.

Se una cartella di lavoro indica che la tabella deve essere letta di nuovo all'apertura, e i dati della tabella si trovano in un altro file, JeffCrunch chiede prima conferma, nello stesso modo in cui lo fa per i collegamenti ad altre cartelle di lavoro. Finché non si risponde di sì, non viene letto alcun file.

Una cartella di lavoro salvata come .xlsx memorizza dove si trova il file. Scegliendo il file nella finestra si memorizza il suo indirizzo completo. Una cartella di lavoro creata da un altro programma, la cui tabella punta a un file tramite un percorso breve come `Prezzi.xlsx`, mantiene quel percorso, e JeffCrunch cerca il file nella stessa cartella della cartella di lavoro. Quando si usa **Salva con nome** per salvare la cartella di lavoro in un'altra cartella, un percorso di questo tipo viene riscritto in modo che indichi ancora lo stesso file. Aggiorna non segue eventuali collegamenti che l'altro file ha verso altri file.

## Cancella

**Cancella** ha due scelte. **Cancella filtri** rimuove tutti i filtri e mostra di nuovo tutti gli elementi. **Cancella tutto** elimina tutti i campi dal rapporto e lo lascia vuoto, pronto per essere costruito di nuovo.

## Analisi dettagliata e riduzione del livello

## Pagine filtro report

Un filtro sta sopra una tabella pivot e consente di esaminare un elemento alla volta, ad esempio un mese. **Mostra pagine filtro report**, nella scheda **Strumenti tabella pivot**, crea una pagina per ciascun elemento in una sola operazione. Ogni pagina è un nuovo foglio, denominato con il nome dell'elemento, con una propria copia della tabella. Il filtro di ciascuna copia è impostato su quell'elemento. Se la tabella ha più di un filtro, si sceglie quello da usare. Un solo **Annulla** elimina tutte le pagine.

Quando un rapporto ha più di un campo nelle righe, un elemento del primo campo può essere aperto o chiuso. Fare clic sulla cella con il nome dell'elemento e scegliere **Riduci livello** per chiuderlo: le righe del campo successivo che si trovano sotto di esso vengono nascoste e resta solo il suo totale. **Analisi dettagliata** lo riapre. Un elemento dell'ultimo campo non ha nulla sotto di sé. Ciascuna operazione è un solo **Annulla**.

## Selezionare e spostare

**Seleziona**, nella scheda **Strumenti tabella pivot**, seleziona una parte del rapporto: **Intera tabella pivot**, **Valori** (i numeri), oppure **Etichette** (i nomi sul lato; se il rapporto non ne ha sul lato, i nomi in alto). **Sposta tabella pivot…** chiede una cella dello stesso foglio e ridisegna il rapporto con la sua prima cella in quella posizione. Le celle lasciate libere vengono svuotate. Le formule che puntano alle celle del rapporto le seguono, come avviene quando si taglia e si incolla. Se la nuova posizione contiene già qualcosa, JeffCrunch non sposta il rapporto e lo segnala. Un solo **Annulla** lo riporta indietro.

## Il riquadro dei campi

**Elenco campi**, nella scheda **Strumenti tabella pivot**, apre il riquadro **Campi tabella pivot**. Vi compaiono i campi dell'elenco, ciascuno con una casella accanto, e più sotto i quattro punti: **Filtri**, **Colonne**, **Righe** e **Valori**.

- Selezionare un campo per aggiungerlo. Un campo che contiene solo numeri va in **Valori** ed è sommato; ogni altro va in **Righe**.
- Deselezionare un campo per rimuoverlo ovunque.
- Fare clic su un campo in uno dei quattro punti e scegliere **Sposta in** per collocarlo in un altro. Le voci **Sposta su** e **Sposta giù** ne cambiano l'ordine nel proprio punto, e **Rimuovi campo** lo elimina.
- Con più di un valore, in **Colonne** compare una riga chiamata **Valori**. Spostarla in **Righe** per mettere i valori sul lato.
- **Impostazioni campo valore** (doppio clic su un valore, oppure il pulsante) imposta il nome con cui il rapporto mostra il valore e il modo in cui viene totalizzato: **Somma**, **Conteggio**, **Conteggio numeri**, **Media**, **Max**, **Min**, **Prodotto**, **DevStd** e **DevStdp** (la deviazione standard di un campione, o di tutti i valori), e **Var** e **Varp** (la varianza degli stessi). Imposta anche il modo in cui il valore viene mostrato e il suo formato numero. Vedere la sezione successiva.
- **Impostazioni campo** (doppio clic su un campo in Filtri, Colonne o Righe, oppure il pulsante) imposta un solo campo. Vedere "Impostazioni campo".

Ogni modifica è un passaggio di **Annulla**.

## Mostra valori come, e formato numero di un valore

In **Impostazioni campo valore**, **Mostra valori come** cambia ciò che mostra una cella del rapporto.

- **% del totale generale**: la cella divisa per il totale dell'intero rapporto. Nell'esempio, Est a gennaio è 15 su 49, cioè il 30,6%.
- **% del totale di colonna** e **% del totale di riga**: la cella divisa per il totale della sua colonna o della sua riga.
- **Indice**: il confronto tra la cella e ciò che la sua riga e colonna darebbero se i dati fossero distribuiti in modo uniforme. Un numero superiore a 1 indica più della quota uniforme.
- **Differenza da**, **% di** e **% differenza da**: la cella confrontata con un altro elemento di un campo. Scegliere il **Campo base** (un campo in Righe o Colonne) e l'**Elemento base** (uno dei suoi elementi, oppure quello precedente o successivo). Per esempio, con Mese come campo base e Gen come elemento base, febbraio mostra quanto è in più o in meno rispetto a gennaio. L'elemento base stesso resta vuoto per una differenza, e vale 100% per **% di**.
- **Totale parziale in**: ogni cella somma le celle che la precedono lungo il campo base.
- **Classifica dal più piccolo al più grande** e **Classifica dal più grande al più piccolo**: ogni cella mostra il suo posto, 1, 2, 3 e così via, tra le celle del campo base nella stessa riga o colonna. Il posto 1 è il numero più piccolo, oppure il più grande.
- **% del totale riga padre** e **% del totale colonna padre**: ogni cella mostra quale parte del proprio elemento padre rappresenta. Il padre di un elemento è l'elemento di un livello superiore, come l'anno che contiene un mese. Un elemento senza padre viene mostrato come parte del totale generale.

**Formato numero** accetta un codice di formato, come `0,00`, `#.##0` o `0,0%`. Lasciarlo vuoto mantiene il formato che hanno le celle. Un valore mostrato come percentuale riceve `0,00%`, a meno che non si scelga un formato. Le celle del rapporto assumono subito il formato. **Annulla** lo ripristina insieme ai numeri.

## Impostazioni campo

Selezionare un campo nella casella **Filtri**, **Colonne** o **Righe** e scegliere **Impostazioni campo** nella scheda **Strumenti tabella pivot** (oppure fare doppio clic su di esso).

- **Nome personalizzato** cambia il nome con cui il rapporto mostra il campo.
- **Subtotali**: **Automatico** mostra un totale per ciascun gruppo del campo, **Nessuno** non lo mostra.
- **Mostra etichette elementi in**: forma compatta, struttura o tabulare, come in Layout rapporto.
- **Inserisci una riga vuota dopo ogni elemento** aggiunge una riga vuota dopo ciascun gruppo.
- **Ripeti etichette elementi** mostra il nome di un elemento esterno su ogni riga del suo gruppo (forme a struttura e tabulare).
- **Ordina**: mantiene l'ordine in cui si trovano gli elementi (manuale), oppure ordina dalla A alla Z o dalla Z alla A in base alle etichette. **Dal più piccolo al più grande per valore** e **Dal più grande al più piccolo per valore** ordinano gli elementi in base ai numeri di un campo valore. Il rapporto mantiene l'ordinamento quando cambiano i dati.
- **Filtro** esclude gli elementi in base alle etichette (inizia con, termina con, contiene, uguale a) o in base a un valore (maggiore di, minore di, compreso tra e così via), oppure mantiene soltanto i primi o gli ultimi elementi in base a un valore. Il filtro viene applicato quando si sceglie OK. Nasconde gli elementi esclusi; le caselle sotto indicano quali sono, e si può selezionare un elemento per mostrarlo di nuovo.
- **Mostra questi elementi** è un elenco degli elementi del campo, ciascuno con una casella di controllo. Deselezionare una casella per nascondere l'elemento, e i totali lo escludono.

## Raggruppare date e numeri

Selezionare un campo di date o di numeri in **Righe** o **Colonne** e scegliere **Raggruppa campo** nella scheda **Strumenti tabella pivot**. I gruppi vengono creati dai dati, quindi in quella colonna devono esserci solo numeri o date, senza testo e senza celle vuote.

- **Date**: selezionare il criterio di raggruppamento: **Secondi**, **Minuti**, **Ore**, **Giorni**, **Mesi**, **Trimestri** o **Anni**. La parte più fine selezionata resta nel campo. Per ogni parte più ampia JeffCrunch aggiunge un campo, denominato come essa (ad esempio **Anni**), e lo colloca prima del campo data nella stessa posizione. **Inizia da** e **Termina a** indicano la prima e l'ultima data; lasciarle come sono per usare la data minima e massima dei dati. Le date precedenti l'inizio o successive alla fine vanno in due gruppi aggiuntivi, che iniziano con `<` e `>`.
- **Numeri**: **Inizia da**, **Termina a** e **Di** creano gruppi come 1-10, 11-20 e così via.

**Separa** elimina i gruppi e i campi aggiunti per essi. Aggiorna ricrea i gruppi a partire dai nuovi dati. Quando due tabelle pivot sono create dagli stessi dati, non è possibile creare gruppi, perché un gruppo fa parte dei dati che esse condividono.

## Campi calcolati

Un campo calcolato è un campo creato con una formula. Utilizza altri campi dell'elenco. Per esempio, un premio pari a un decimo dell'importo è `=Importo*0,1`.

Scegliere **Campo calcolato** nella scheda **Strumenti tabella pivot**. Digitare un nome e una formula. Scrivere nella formula il nome di un campo, oppure fare doppio clic su di esso nell'elenco **Campi** per inserirlo. Racchiudere tra apici singoli un nome che contiene spazi, come in `='Prezzo unitario'*Quantità`. Si possono usare numeri, `+ - * / ^ %`, confronti e alcune funzioni come `SE`, `ARROTONDA`, `SOMMA`, `MIN`, `MAX`, `MEDIA` e `ASS`.

Scegliere **Aggiungi**. Il nuovo campo viene inserito nella casella **Valori**. Compare anche nell'elenco dei campi, dove si può selezionare o deselezionare come qualsiasi altro campo. Non può andare in righe, colonne o filtri.

Un campo calcolato viene determinato in ogni cella del rapporto. Il nome di un campo rappresenta il totale di quel campo nella cella, quindi `=Importo*Importo` eleva al quadrato il totale della cella. Non eleva al quadrato ciascuna riga e poi somma. Un totale generale usa i totali dell'intero elenco.

**Formule elenco** (accanto a **Campo calcolato**) crea un nuovo foglio che riporta ciascun campo calcolato con la sua formula, e ciascun elemento calcolato con la sua formula, il suo campo e il suo ordine di calcolo. **Annulla** elimina le celle, e un altro **Annulla** elimina il foglio.

Per modificare un campo, selezionarne il nome nella casella **Nome**. Compare la sua formula. Modificarla e scegliere **Modifica**. **Elimina** rimuove il campo e il suo valore. Ogni operazione è un passaggio di **Annulla**. Un campo calcolato viene salvato con il file e viene mantenuto quando il rapporto viene aggiornato. Due tabelle pivot create dagli stessi dati non possono averne uno, perché un campo calcolato fa parte dei dati che esse condividono.

## Elementi calcolati

Un elemento calcolato è un elemento creato a partire da altri elementi dello stesso campo. Per esempio, il campo Area ha gli elementi Est e Ovest. Un nuovo elemento denominato Entrambi può essere `=Est+Ovest`.

Fare clic su una cella della tabella pivot e scegliere **Elemento calcolato** nella scheda **Strumenti tabella pivot**. Scegliere il **Campo**. Deve trovarsi nelle righe o nelle colonne. Digitare un **Nome** e una **Formula**. Scrivere nella formula il nome di un elemento, oppure fare doppio clic su di esso nell'elenco **Elementi** per inserirlo. Racchiudere tra apici singoli un nome che contiene spazi, come in `='Est grande'+Ovest`. Si possono usare numeri, `+ - * / ^ %` e parentesi.

Il nuovo elemento ottiene una riga nel rapporto. In ogni colonna mostra la formula calcolata a partire dalle righe degli elementi che nomina. Il nuovo elemento viene conteggiato nei totali. Un totale di somme, come il totale generale o un subtotale, aggiunge la riga del nuovo elemento alle righe degli altri elementi. Quindi, con Est a 35, Ovest a 14 e Entrambi (Est più Ovest) a 49, il totale generale è 98. Se un elemento nominato dalla formula è nascosto, la cella mostra **#RIF!**. Se la formula nomina un elemento che non esiste, la cella mostra **#NOME?**.

Per modificare un elemento, selezionarne il nome nella casella **Nome**. Compare la sua formula. Modificarla e scegliere **Modifica**. **Elimina** rimuove l'elemento e la sua riga. Ogni operazione è un passaggio di **Annulla**. Un elemento calcolato viene salvato con il file e viene mantenuto quando il rapporto viene aggiornato. Due tabelle pivot create dagli stessi dati non possono averne uno, perché un elemento calcolato fa parte dei dati che esse condividono.

### Ordine di calcolo

A volte una cella ha due elementi calcolati: uno nella sua riga e l'altro nella sua colonna. L'ordine in cui vengono calcolati può cambiare il risultato. Supponiamo che l'elemento di riga sia `=Est-Ovest` e quello di colonna sia `=Gen+1`. Calcolato in un ordine, la cella vale (Est - Ovest) + 1. Nell'altro ordine vale (Est + 1) - (Ovest + 1), che è un numero diverso.

**Ordine di calcolo**, nella scheda **Strumenti tabella pivot**, elenca gli elementi calcolati nell'ordine in cui vengono determinati. L'elemento più in basso nell'elenco viene calcolato per ultimo. Scegliere un elemento e fare clic su **Sposta su** o **Sposta giù**. L'ordine viene salvato con il file come ordine degli elementi, e gli altri programmi lo leggono allo stesso modo. **Formule elenco** mostra la posizione di ciascun elemento in questo ordine.

## Mostrare le righe dietro un valore

Fare doppio clic su un valore di una tabella pivot, oppure scegliere **Mostra dettagli** nella scheda **Strumenti tabella pivot**. JeffCrunch crea un nuovo foglio, denominato Dettaglio 1, 2 e così via, con le righe dell'elenco che compongono quel numero. I nomi dei campi si trovano nella prima riga. F2 modifica ancora la cella, se è modificabile. **Annulla** elimina le righe, e un altro **Annulla** elimina il foglio.

## Espandere e comprimere

Un campo i cui elementi hanno altri elementi sotto di sé può mostrare o nascondere le righe che si trovano sotto i suoi elementi. **Espandi intero campo** e **Comprimi intero campo** lo fanno per ogni elemento del campo selezionato nel riquadro.

## Opzioni tabella pivot

**Opzioni**, nella scheda **Strumenti tabella pivot**, apre una finestra con le impostazioni della tabella.

- **Nome** è il nome della tabella pivot. Nessun'altra tabella pivot della cartella di lavoro può avere lo stesso nome.
- **Per i valori di errore mostra** e **Per le celle vuote mostra** inseriscono un testo a propria scelta in quelle celle.
- **Unisci e allinea al centro le celle con etichette**, **Adatta la larghezza delle colonne quando la tabella viene aggiornata** e **Mantieni la formattazione delle celle quando la tabella viene aggiornata** vengono salvate con la tabella.
- **Mostra totali generali per le righe** e **per le colonne** equivalgono alle scelte dei Totali generali.
- **Aggiorna i dati quando il file viene aperto** legge di nuovo le celle ogni volta che la cartella di lavoro viene aperta. È un passaggio di Annulla, come qualsiasi aggiornamento.

## Stile: intestazioni, bande e galleria

Le **Opzioni stile**, nella scheda **Strumenti tabella pivot**, attivano o disattivano la riga di intestazione, la prima colonna (intestazioni di riga) e le bande di righe e di colonne. **Stili** offre una galleria di aspetti, e **Cancella stile** rimuove l'aspetto. Questi aspetti vengono disegnati nello stesso modo degli aspetti delle tabelle. **Righe vuote** e **Ripeti etichette** impostano in una sola operazione la riga vuota e le etichette ripetute per tutti i campi delle righe.

## Struttura: subtotali, totali generali e layout

- **Subtotali** mostra o nasconde il totale di ciascun gruppo, in alto o in basso nel gruppo. Quando nelle Righe c'è più di un campo, il totale di ciascun campo esterno è un subtotale.
- **Totali generali** ha quattro scelte: disattivati per righe e colonne, attivati per entrambi, attivati solo per le righe (una colonna di totali a destra, una per ogni riga), e attivati solo per le colonne (una riga di totali in basso).
- **Layout rapporto** è la forma delle etichette. In forma **compatta**, tutti i campi delle Righe condividono una colonna, e un nuovo gruppo è mostrato un po' più a destra. In forma **struttura**, ogni campo ha una propria colonna, e ogni gruppo ha una riga tutta sua sopra i suoi elementi. In forma **tabulare**, ogni campo ha una propria colonna e il nome del gruppo si trova sulla prima riga, con il subtotale sotto il gruppo.

## Lo stesso elenco, con un filtro

Spostare **Mese** da **Colonne** a **Filtri**. Il rapporto perde le sue colonne, e una riga sopra di esso mostra **Mese** e **(Tutti)**. I numeri sono ora il totale di ciascuna area su tutti i mesi.

## Grafico pivot

Fare clic su una cella di una tabella pivot e scegliere **Inserisci**, quindi **Grafico pivot**. JeffCrunch crea un grafico a colonne dei valori della tabella. Esclude i totali generali. Il grafico viene contrassegnato nel file come grafico della tabella pivot. Legge le celle della tabella, quindi i suoi numeri cambiano quando si aggiorna la tabella o si modifica il suo layout. Se la tabella acquisisce altre righe o colonne, creare di nuovo il grafico, perché il grafico conserva le celle da cui è stato creato. Un grafico pivot proveniente da un file viene mostrato come salvato.

## Segmentazioni dati e sequenze temporali

Una segmentazione dati è un insieme di pulsanti che filtra una tabella pivot, mentre una sequenza temporale è un cursore che svolge la stessa funzione per le date.

Fare clic su una cella di una tabella pivot, quindi su **Segmentazioni dati** nella scheda Strumenti tabella pivot. Si apre il riquadro Segmentazioni dati, che elenca le segmentazioni e le sequenze temporali della cartella di lavoro. Una segmentazione ha un pulsante per ciascun elemento del suo campo. I pulsanti blu sono gli elementi mostrati dalla tabella.

Fare clic su un pulsante per mostrare solo quell'elemento. Fare di nuovo clic per mostrare tutti gli elementi. Tenere premuto **Ctrl** e fare clic per aggiungere un elemento a quelli visualizzati, oppure per toglierlo. Ogni clic è un passaggio di Annulla, e Annulla riporta la tabella allo stato precedente. Al salvataggio, il file conserva la scelta fatta nella segmentazione.

**Connessioni report** indica quali tabelle pivot filtra una segmentazione. Una sequenza temporale mostra l'intervallo di date scelto. JeffCrunch mostra le segmentazioni in questo riquadro, non sopra le celle, e non modifica ancora una sequenza temporale. Entrambe vengono conservate nel file così come sono state salvate.

## Limiti

Una tabella pivot può avere fino a 2.000 campi, un elenco fino a cinque milioni di righe e un rapporto fino a quattro milioni di celle. Un file che supera questi valori viene mostrato come salvato e non può essere modificato. La rilettura di centomila righe di venti colonne e il calcolo del rapporto hanno richiesto circa mezzo secondo sul computer su cui è stata effettuata la misurazione; un computer più lento impiega di più.
