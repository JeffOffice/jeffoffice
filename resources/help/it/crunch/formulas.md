# Come vengono mostrate e calcolate le formule

<!-- keywords: errore di traccia, calcola prima del salvataggio, formula, formule, calcola, calcolo, ricalcola, funzione, funzioni, valore, risultato, memorizzato, F9, circolare, mostra formule, barra della formula, errore, punto, puntamento, clic su una cella, riferimento, F4, controllo errori, ignora errore, valuta formula, passo, finestra di controllo, controllo, LET, assegna un nome a un valore, funzione cella, funzione info, FILTERXML, xml, xpath -->

Una **formula** è una cella che ricava il proprio valore da altre celle, ad esempio =SOMMA(B3:B8). La cella mostra il valore, e la **barra della formula** mostra la formula.

## Che cosa fa JeffCrunch con le formule

Quando apri una cartella di lavoro, il file contiene l'ultimo valore di ogni formula, così il foglio può essere mostrato subito. Poi JeffCrunch calcola di nuovo ogni formula, qualche formula alla volta, in modo che la finestra resti veloce. La barra di stato indica **Calcolo in corso** con una percentuale mentre lavora. Al termine, le celle contengono i risultati di JeffCrunch. Per calcolare di nuovo tutto, fai clic su **Ricalcola tutto** nella scheda **Formule** oppure premi **F9**.

## Quando JeffCrunch non può calcolare una formula

JeffCrunch dispone di più di 400 funzioni, e il loro numero è in crescita. Una formula che usa una funzione non ancora disponibile in JeffCrunch conserva il valore memorizzato nel file. Lo stesso vale per una formula che fa riferimento a un'altra cartella di lavoro. Le celle che dipendono da queste formule vengono calcolate a partire dai valori memorizzati.

## Funzioni che richiedono alcune note

- **Obbligazioni e prestiti.** PREZZO, RENDIMENTO, DURATA, PREZZO.PRIMO.IRR e le altre funzioni sulle obbligazioni contano i giorni secondo la **base** che indichi (0 significa 30 giorni al mese in un anno di 360 giorni, 1 significa giorni effettivi, e così via). RENDIMENTO, RENDIMENTO.PRIMO.IRR, RENDIMENTO.ULTIMO.IRR e TASSO trovano il risultato provando diversi valori finché il prezzo o la rata coincidono. Si fermano quando il risultato è corretto fino a circa 15 cifre, quindi le ultime cifre possono differire da quelle di altri programmi.
- **Distribuzioni e test.** Le funzioni che terminano con .DISTR, .INV, .TEST e i nomi più vecchi come DISTRIB.NORM e DISTRIB.T danno le stesse risposte: un nome precedente viene mantenuto affinché i file vecchi continuino a funzionare. Anche le funzioni *.INV trovano il risultato provando diversi valori.
- **Elenchi che occupano più celle.** SORT, SORTBY, FILTER, UNIQUE, SEQUENCE, RANDARRAY, XLOOKUP, MMULT, MINVERSE e LINEST restituiscono un blocco di valori. Inseriscile come formula matriciale (**Ctrl+Maiusc+Invio**) sulle celle che devono riempire. Non è ancora possibile far estendere una formula alle celle vicine.
- **Testi in altri alfabeti.** LUNGHEZZAB, SINISTRAB, STRINGAB, DESTRAB, TROVA.B, RICERCA.B e SOSTITUISCIB contano ciascun carattere come uno, come accade su un computer non configurato per i testi a doppio byte. FONETICO restituisce il testo della cella, perché JeffCrunch non conserva le letture del testo giapponese.
- **CONVERTI.** Le unità si scrivono come testo, con maiuscole e minuscole come indicato nell'elenco della funzione: "kg", "lbm", "mi", "F". Un prefisso del sistema metrico può precedere un'unità che lo accetta, come "km" o "mL".

## MOD: il resto

**MOD(n; d)** restituisce ciò che resta di **n** dopo aver tolto il maggior numero intero di volte di **d**: è n - d * INT(n / d). Esempio: **=MOD(10; 3)** dà 1, e **=MOD(-10; 3)** dà 2, perché INT(-10 / 3) è -4 e -10 - 3 * -4 fa 2. Il risultato ha il segno di **d**. Se **d** è 0, il risultato è #DIV/0!.

## Mostrare le formule

Fai clic su **Mostra formule** nella scheda **Formule**, oppure premi **Ctrl+'** (su un Mac, il tasto Control e non il tasto Command), per vedere la formula di ogni cella al posto del suo valore. Le colonne che contengono formule vengono allargate, fino a un certo limite, in modo che le formule si vedano per intero; tornano alla larghezza originale quando disattivi l'opzione, e le larghezze nel file non vengono modificate.

## Modificare le formule

Per modificare una formula, seleziona la sua cella, premi **F2**, modificala e premi **Invio**. Puoi modificarla anche nella barra della formula. Quando cambi una cella, le formule che dipendono da essa vengono ricalcolate in background. Vedi [Digitare nelle celle](edit-cells).

## Scrivere una formula

Digita **=** e le prime lettere di una funzione: compare un breve elenco delle funzioni e dei [nomi](names) che iniziano in quel modo, con una frase di descrizione per ciascuno. Premi **Tab** per scegliere quello evidenziato. Usa i tasti **Freccia giù** e **Freccia su** per evidenziarne un altro, quindi **Tab** o **Invio**. **Esc** chiude l'elenco. Quando sei dentro le parentesi di una funzione, i suoi argomenti compaiono sotto la cella, con quello che stai digitando in grassetto. L'elenco e gli argomenti sono gli stessi che mostra **Scegli una funzione**. Gli argomenti sono separati da virgole, o da punti e virgola dove la lingua usa la virgola come separatore decimale.

## Indicare le celle

Invece di digitare l'indirizzo di una cella, puoi indicarla. Mentre digiti una formula, posiziona il cursore dove può andare una cella (subito dopo **=**, dopo un operatore come **+**, una parentesi aperta o una virgola) e fai clic sulla cella. Il suo indirizzo viene scritto per te. Trascina sulle celle per scrivere un intervallo come B2:D5. Fai clic di nuovo prima di digitare altro, e la nuova cella sostituisce quella appena scritta. Tieni premuto **Ctrl** e fai clic per aggiungere un altro indirizzo dopo una virgola. Fai clic su una lettera di colonna per scrivere un'intera colonna, come A:A, o su un numero di riga per scrivere un'intera riga, come 3:3, e trascina lungo le lettere o i numeri per B:D o 2:5.

Funzionano anche i tasti freccia. Premi una freccia e un puntatore si sposta lontano dalla cella in cui stai scrivendo, e il suo indirizzo viene scritto; premi altre frecce per proseguire, oppure tieni premuto **Maiusc** per estendere il puntatore su un intervallo. La barra di stato indica **Punta** mentre i tasti freccia indicano, **Invio** mentre digiti, e **Modifica** mentre i tasti freccia spostano il cursore nel testo. Premi **F2** per passare dall'indicazione alla modifica del testo.

Per indicare una cella di un altro foglio, fai clic sulla linguetta di quel foglio mentre il cursore si trova dove può andare una cella, quindi fai clic sulla cella. Il nome del foglio viene scritto insieme all'indirizzo, tra apici quando necessario, come in 'Il mio foglio'!B3. **Invio** ti riporta al foglio di partenza e inserisce la formula nella sua cella. **Esc** annulla tutto, compreso il cambio di foglio.

Puoi anche trascinare il bordo di un intervallo evidenziato sul foglio per spostare quell'indirizzo, oppure trascinare il quadratino nell'angolo per allargarlo; la formula cambia mentre trascini.

Premi **F4** con il cursore dentro o subito dopo un indirizzo per fissarne alcune parti con il segno del dollaro: A1 diventa $A$1, poi A$1, poi $A1, e poi di nuovo A1.

Ogni indirizzo della formula è colorato nel testo, e le celle che nomina sono evidenziate nel foglio con lo stesso colore, così puoi vedere a colpo d'occhio che cosa legge la formula. Un clic dopo un indirizzo completo, come =A1, termina l'inserimento e seleziona la cella su cui hai fatto clic, come accade per qualsiasi altro testo.

## Tabelle dati

Una tabella dati (tabella di simulazione) mostra il risultato di una formula per molti valori di una o due celle. Se apri una cartella di lavoro che ne contiene una, JeffCrunch calcola le sue celle: per una tabella con due input, la formula si trova nella cella in alto a sinistra della tabella, i valori da provare sulla riga vanno nella cella di input di riga, e quelli sulla colonna nella cella di input di colonna. Per una tabella con un solo input, i valori sono disposti in una colonna a sinistra (e le formule si trovano nella riga sopra) oppure in una riga sopra (e le formule si trovano nella colonna a sinistra). Ogni cella della tabella è il risultato della formula, e di ogni formula che dipende dalla cella di input, con l'input impostato su quel valore; nient'altro nella cartella di lavoro viene modificato. In modalità **Automatica** le tabelle vengono calcolate insieme alle formule, e le formule che leggono le celle di una tabella ne seguono i risultati. La creazione di una nuova tabella dati non è ancora disponibile. Una tabella che non può essere calcolata (una formula che dipende dall'input usa una funzione che JeffCrunch non possiede, oppure cambia da sola, come CASUALE o ADESSO) mantiene i valori contenuti nel suo file.

## Calcolo manuale

Fai clic su **Modalità di calcolo** nella scheda **Formule** per scegliere come la cartella di lavoro calcola le formule:

- **Automatica** calcola le formule mentre modifichi le celle.
- **Automatica, tabelle manuali** fa lo stesso, ma lascia invariate le celle delle tabelle dati finché non premi **F9**.
- **Manuale** non calcola nulla, comprese le tabelle dati, finché non lo richiedi. Dopo una modifica, la barra di stato indica **Non aggiornata**. Premi **F9** (**Ricalcola tutto**) per calcolare tutto, oppure **Maiusc+F9** (**Ricalcola questo foglio**) per calcolare le formule del foglio corrente e quanto altro era in attesa. Una cartella di lavoro impostata su Manuale si apre con i risultati contenuti nel suo file. **Calcola prima del salvataggio**, anch'essa nel menu **Modalità di calcolo**, è selezionata per impostazione predefinita: quando salvi una cartella di lavoro manuale non aggiornata, le formule vengono calcolate prima, così il file contiene numeri aggiornati. Disattivala per salvare i numeri così come sono.

La scelta viene salvata con la cartella di lavoro, ed è un passaggio che puoi annullare.

## Formule che leggono se stesse

Una formula che legge il proprio risultato, direttamente o attraverso altre celle, non può essere calcolata nel modo consueto. JeffCrunch lo segnala una volta, indica le celle e le evidenzia nel foglio; esse mantengono i valori che avevano. Fai clic su **Riferimenti circolari** nella scheda **Formule** per visualizzare l'elenco, e fai doppio clic su una cella dell'elenco per raggiungerla.

Alcuni modelli richiedono un ciclo intenzionale. Scegli **Calcola formule circolari** nel menu **Modalità di calcolo**, e le formule vengono calcolate più volte, ognuna leggendo gli ultimi valori delle altre, finché nessun valore cambia più della quantità che indichi o non viene raggiunto il numero di iterazioni consentito.

## Vedere che cosa legge una formula

Seleziona una cella e fai clic su **Mostra celle che alimentano questa cella** per disegnare frecce dalle celle che la sua formula legge. Fai clic di nuovo per risalire di un altro passo. **Mostra celle che usano questa cella** disegna frecce verso le formule che leggono la cella, e ogni clic procede di un passo. Una piccola pagina rappresenta una cella di un altro foglio. Vengono disegnate al massimo 2.000 frecce, e la barra di stato lo segnala se ce ne sono di più. **Rimuovi frecce** le cancella, e le cancella anche ogni modifica alla cartella di lavoro.

Quando una cella mostra un errore come #DIV/0!, selezionala e fai clic su **Traccia errore**. Vengono disegnate frecce verso le celle che legge e che a loro volta mostrano un errore. Fai clic di nuovo per risalire di un altro passo. Quando nessuna delle celle che legge mostra un errore, la barra di stato indica che questa cella è all'origine dell'errore, ed è quella da correggere.

## Controllo errori

Scegli **Controllo errori** nel gruppo **Verifica** della scheda Formule per esaminare il foglio alla ricerca di celle che potrebbero essere errate. Cerca due cose: una formula che restituisce un errore (come #DIV/0!), e un numero memorizzato come testo, che le formule potrebbero ignorare.

La finestra mostra una cella alla volta, si sposta su quella cella nel foglio e spiega in una frase qual è il problema. **Successivo** e **Precedente** scorrono le celle. Un piccolo triangolo verde nell'angolo superiore sinistro di una cella la segnala nel foglio. **Ignora errore** toglie la cella dall'elenco, rimuove il suo triangolo ed è salvato nel file. Un passo di **Annulla** lo ripristina. Se non c'è nessun problema, viene segnalato.

## Valuta formula

Usalo per vedere come una formula arriva al proprio valore. Seleziona una cella che contiene una formula e scegli **Valuta formula**. La formula viene mostrata con la parte successiva sottolineata. Premi **Valuta** per calcolare quella parte; il suo valore prende il suo posto. Le parti vengono prese in quest'ordine: prima ogni cella singola da sinistra, poi ogni funzione con le sue parentesi, partendo da quella più interna, infine ciò che resta. **Ricomincia** riparte dall'inizio. Se una parte non può essere calcolata, la finestra spiega il motivo e lascia la formula com'era.

## Finestra di controllo

La **Finestra di controllo** è una piccola finestra che resta visibile mentre lavori. Elenca le celle che vuoi tenere sotto controllo, con il loro foglio, la cella, il valore e la formula, anche quando si trovano su un altro foglio. Seleziona le celle e premi **Aggiungi controllo**. Seleziona le righe dell'elenco e premi **Elimina controllo**. Fai doppio clic su una riga per andare alla cella. I valori cambiano quando il foglio viene ricalcolato. L'elenco viene mantenuto finché la cartella di lavoro è aperta.

## Assegnare un nome a un valore dentro una formula

**LET** assegna un nome a un valore nel mezzo di una formula, così lo scrivi una sola volta e lo riutilizzi. Le coppie di nome e valore vengono prima, e il calcolo per ultimo. Nella formula =LET(prezzo; B2; imposta; prezzo*0,08; prezzo+imposta) il nome **prezzo** rappresenta il valore in B2, **imposta** viene calcolata a partire da esso, e l'ultima parte le somma. Un nome può usare i nomi definiti prima di esso. Un nome vale solo all'interno del suo LET, e un LET annidato in un altro può riutilizzare lo stesso nome. Un nome può rappresentare anche un intervallo di celle, come in =LET(righe; B2:B9; SOMMA(righe)/CONTA.NUMERI(righe)).

## Chiedere informazioni su una cella o sulla cartella di lavoro

**CELLA** restituisce informazioni su una cella. Il suo primo argomento indica quali: "indirizzo", "riga", "col", "contenuto", "tipo", "formato" (come vengono mostrati i numeri, ad esempio "F2" per due decimali, "P0" per una percentuale, o "D1" per una data), "colore" (1 quando i numeri negativi vengono mostrati in colore), "parentesi" (1 quando i numeri positivi vengono mostrati tra parentesi), "prefisso" (il segno di un testo allineato a sinistra, a destra o al centro), "proteggi" (1 quando la cella è bloccata), "larghezza" (la larghezza della colonna in caratteri) e "nomefile" (la cartella, il file e il foglio, come /home/io/[Budget.xlsx]Foglio1; è vuoto finché la cartella di lavoro non viene salvata). **INFO** restituisce informazioni sul programma e sulla cartella di lavoro: "directory", "numfile" (quanti fogli sono visualizzati), "origine", "osversion", "recalc" (Automatica o Manuale), "release" e "system".

## Estrarre valori da XML

**FILTRO.XML** prende un testo che contiene XML e un XPath, cioè un modo breve per indicare quali parti vuoi, e restituisce i valori che trova. =FILTRO.XML(A1; "//articolo/prezzo") restituisce, uno sotto l'altro, tutti i prezzi del testo in A1. Usa "/negozio/articolo[2]/nome" per il secondo articolo, "//articolo[@id='b']/nome" per l'articolo il cui id è b, oppure "count(//articolo)" per sapere quanti sono. Se il testo non è XML valido, o il percorso non è valido, il risultato è #VALORE!. Se non corrisponde nulla, il risultato è #N/D. Un XML che contiene una dichiarazione di tipo di documento viene rifiutato.
