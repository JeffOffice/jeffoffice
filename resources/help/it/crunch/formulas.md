# Come vengono mostrate e calcolate le formule

<!-- keywords: formula, formule, calcolo, calcola, ricalcola, funzione, funzioni, valore, risultato, memorizzato, F9, circolare, mostra formule, barra della formula, errore, punta, puntamento, fai clic su una cella, riferimento, F4, controllo errori, ignora errore, valuta formula, passaggio, controllo, finestra di controllo, watch -->

Una **formula** è una cella che calcola il proprio valore a partire da altre celle, ad esempio =SOMMA(B3:B8). La cella mostra il valore, e la **barra della formula** mostra la formula.

## Che cosa fa JeffCrunch con le formule

Quando apri una cartella di lavoro, il file contiene l'ultimo valore di ogni formula, così il foglio può essere mostrato subito. Poi JeffCrunch calcola di nuovo ogni formula, qualche formula alla volta, in modo che la finestra resti veloce. La barra di stato indica **Calcolo** con una percentuale mentre il lavoro è in corso. Al termine, le celle contengono i risultati di JeffCrunch. Per calcolare di nuovo tutto, fai clic su **Ricalcola tutto** nella scheda **Formule**, oppure premi **F9**.

## Quando JeffCrunch non può calcolare una formula

JeffCrunch dispone di più di 400 funzioni, e il loro numero è in crescita. Una formula che usa una funzione non ancora disponibile in JeffCrunch conserva il valore memorizzato nel file. Lo stesso vale per una formula che fa riferimento a un'altra cartella di lavoro. Le celle che dipendono da queste formule vengono calcolate a partire dai valori memorizzati.

## Funzioni che richiedono alcune note

- **Obbligazioni e prestiti.** PREZZO, REND, DURATA, PREZZO.PRIMO.IRR, e le altre funzioni sulle obbligazioni contano i giorni secondo la **base** indicata (0 significa 30 giorni al mese in un anno di 360 giorni, 1 indica i giorni effettivi, e così via). REND, REND.PRIMO.IRR, REND.ULTIMO.IRR e TASSO trovano il risultato provando diversi valori finché il prezzo o la rata coincidono. Si fermano quando il risultato è corretto fino a circa 15 cifre, quindi le ultime cifre possono differire da quelle di un altro programma.
- **Distribuzioni e test.** Le funzioni che terminano in .DIST, .INV, .VERIFICA e i nomi più vecchi come DISTRIB.NORM e DISTRIB.T, danno le stesse risposte: un nome precedente viene mantenuto affinché i file vecchi continuino a funzionare. Anche le funzioni *.INV vengono risolte provando dei valori.
- **Elenchi che riempiono più celle.** ORDINA, ORDINA.PER, FILTRO, UNICI, SEQUENZA, MATR.CASUALE, CERCA.X, MATR.PRODOTTO, MATR.INVERSA e REGR.LIN restituiscono un intervallo di valori. Inseriscile come formula matriciale (**Ctrl+Maiusc+Invio**) sulle celle che devono riempire. Non è ancora disponibile la fuoriuscita di una formula nelle celle adiacenti.
- **Testi in altri sistemi di scrittura.** LUNGHEZZAB, SINISTRAB, STRINGA.ESTRAI.B, DESTRAB, TROVAB, RICERCAB e SOSTITUISCIB contano ciascun carattere come uno solo, come avviene su un computer non configurato per il testo a doppio byte. FONETICO restituisce il testo della cella, perché JeffCrunch non conserva le letture del testo giapponese.
- **CONVERTI.** Le unità si scrivono come testo, con le maiuscole e le minuscole indicate nell'elenco della funzione: "kg", "lbm", "mi", "F". Un prefisso del sistema metrico può precedere un'unità che lo accetta, come "km" o "mL".

## RESTO: il resto della divisione

**RESTO(n; d)** restituisce ciò che rimane di **n** dopo aver tolto il maggior numero di copie intere di **d**: è n - d * INT(n / d). Esempio: **=RESTO(10; 3)** dà 1, e **=RESTO(-10; 3)** dà 2, perché INT(-10 / 3) è -4 e -10 - 3 * -4 è 2. Il risultato ha il segno di **d**. Se **d** è 0, il risultato è #DIV/0!.

## Mostrare le formule

Fai clic su **Mostra formule** nella scheda **Formule**, oppure premi **Ctrl+'** (su Mac, il tasto Control e non il tasto Command), per vedere la formula di ogni cella al posto del suo valore. Le colonne che contengono formule vengono allargate, fino a un certo limite, in modo che le formule si leggano per intero; tornano alla larghezza originale quando disattivi l'opzione, e le larghezze nel file non vengono modificate.

## Modificare le formule

Per modificare una formula, seleziona la sua cella, premi **F2**, modificala e premi **Invio**. Puoi anche modificarla nella barra della formula. Quando cambi una cella, le formule che ne dipendono vengono ricalcolate in background. Vedi [Digitare nelle celle](edit-cells).

## Scrivere una formula

Digita **=** e le prime lettere di una funzione: compare un breve elenco delle funzioni e dei [nomi](names) che iniziano in quel modo, con una frase per ciascuno. Premi **Tab** per scegliere quella evidenziata. Usa i tasti **Giù** e **Su** per evidenziarne un'altra, poi **Tab** o **Invio**. **Esc** chiude l'elenco. Quando sei dentro le parentesi di una funzione, i suoi argomenti compaiono sotto la cella, con quello che stai digitando in grassetto. L'elenco e gli argomenti sono gli stessi mostrati da **Scegli una funzione**. Gli argomenti sono separati da punti e virgola, oppure da virgole nelle impostazioni in cui la virgola indica il separatore decimale.

## Puntare alle celle

Invece di digitare l'indirizzo di una cella, puoi indicarla. Mentre digiti una formula, posiziona il cursore dove può comparire una cella (subito dopo **=**, dopo un operatore come **+**, una parentesi aperta o una virgola) e fai clic sulla cella. Il suo indirizzo viene scritto automaticamente. Trascina sulle celle per scrivere un intervallo come B2:D5. Fai di nuovo clic prima di digitare altro, e la nuova cella sostituisce quella appena scritta. Tieni premuto **Ctrl** e fai clic per aggiungere un altro indirizzo dopo una virgola. Fai clic su una lettera di colonna per scrivere un'intera colonna come A:A, oppure su un numero di riga per scrivere un'intera riga come 3:3; trascina lungo le lettere o i numeri per B:D o 2:5.

Funzionano anche i tasti freccia. Premi una freccia e un puntatore si sposta allontanandosi dalla cella in cui stai scrivendo, e il suo indirizzo viene scritto; premi altre frecce per proseguire, oppure tieni premuto **Maiusc** per estendere il puntatore su un intervallo. La barra di stato indica **Punta** mentre i tasti freccia puntano, **Invio** mentre digiti e **Modifica** quando i tasti freccia spostano il cursore nel testo. Premi **F2** per passare dal puntamento alla modifica del testo.

Per puntare a una cella di un altro foglio, fai clic sulla scheda di quel foglio mentre il cursore si trova dove può comparire una cella, poi fai clic sulla cella. Il nome del foglio viene scritto insieme all'indirizzo, tra apici quando necessario, come in 'Il mio foglio'!B3. **Invio** ti riporta al foglio di partenza e inserisce la formula nella sua cella. **Esc** annulla tutto, compreso il cambio di foglio.

Puoi anche trascinare il bordo di un intervallo evidenziato nel foglio per spostarne l'indirizzo, oppure trascinare il suo quadratino d'angolo per allungarlo: la formula cambia mentre trascini.

Premi **F4** con il cursore dentro o subito dopo un indirizzo per bloccarne alcune parti con il segno del dollaro: A1 diventa $A$1, poi A$1, poi $A1, e infine di nuovo A1.

Ogni indirizzo della formula è colorato nel testo, e le celle che nomina sono evidenziate nel foglio con lo stesso colore, così puoi vedere a colpo d'occhio che cosa legge la formula. Un clic dopo un indirizzo completo, come in =A1, termina l'immissione e seleziona la cella su cui hai fatto clic, come accade per qualsiasi altro testo.

## Tabelle dei dati

Una tabella dei dati (tabella di ipotesi) mostra il risultato di una formula per molti valori di una o due celle. Apri una cartella di lavoro che ne contiene una, e JeffCrunch ne calcola le celle: per una tabella con due variabili, la formula si trova nella cella in alto a sinistra della tabella, i valori da provare sulla riga vanno nella cella di input di riga, e quelli nella colonna vanno nella cella di input di colonna. Per una tabella con una sola variabile, i valori vanno nella colonna a sinistra della tabella (e le formule si trovano nella riga sopra di essa), oppure nella riga sopra la tabella (e le formule si trovano nella colonna a sinistra). Ogni cella della tabella è il risultato della formula, e di ogni formula che dipende dalla cella di input, con l'input impostato su quel valore; nient'altro nella cartella di lavoro viene modificato. In modalità **Automatica** le tabelle vengono calcolate insieme alle formule, e le formule che leggono le celle di una tabella ne seguono i risultati. La creazione di una nuova tabella dei dati non è ancora disponibile. Una tabella che non può essere calcolata (una formula che dipende dall'input usa una funzione non disponibile in JeffCrunch, oppure cambia da sola, come CASUALE o ORA) mantiene i valori contenuti nel file.

## Calcolare manualmente

Fai clic su **Modalità di calcolo** nella scheda **Formule** per scegliere come la cartella di lavoro calcola le formule:

- **Automatica** le calcola man mano che cambi le celle.
- **Automatica, tabelle manuali** fa lo stesso, ma lascia invariate le celle delle tabelle dei dati finché non premi **F9**.
- **Manuale** non calcola nulla, tabelle dei dati comprese, finché non lo richiedi. La barra di stato indica **Non aggiornata** dopo una modifica. Premi **F9** (**Ricalcola tutto**) per calcolare tutto, oppure **Maiusc+F9** (**Ricalcola questo foglio**) per calcolare le formule del foglio corrente e tutto ciò che era in attesa. Una cartella di lavoro impostata su Manuale si apre con i risultati contenuti nel file.

La scelta viene salvata con la cartella di lavoro, ed è un'operazione che puoi annullare.

## Formule che leggono se stesse

Una formula che legge il proprio risultato, direttamente o attraverso altre celle, non può essere calcolata nel modo usuale. JeffCrunch lo segnala una sola volta, indica le celle e le evidenzia nel foglio; esse mantengono i valori che avevano. Fai clic su **Riferimenti circolari** nella scheda **Formule** per visualizzare l'elenco, e fai doppio clic su una cella dell'elenco per raggiungerla.

Alcuni modelli richiedono di proposito un ciclo di questo tipo. Scegli **Calcolo iterativo delle formule circolari** nel menu **Modalità di calcolo**: le formule vengono calcolate più volte, leggendo ciascuna gli ultimi valori delle altre, finché nessun valore cambia più della soglia che hai indicato o non viene raggiunto il numero massimo di iterazioni impostato.

## Vedere che cosa legge una formula

Seleziona una cella e fai clic su **Mostra celle che alimentano questa cella** per tracciare frecce dalle celle che la sua formula legge. Fai di nuovo clic per risalire di un passo. **Mostra celle che usano questa cella** traccia frecce verso le formule che leggono la cella, e ogni clic procede di un passo. Una piccola pagina rappresenta una cella di un altro foglio. Vengono tracciate al massimo 2.000 frecce, e la barra di stato lo segnala se ce n'erano di più. **Rimuovi frecce** le cancella, e le cancella anche qualsiasi modifica alla cartella di lavoro.

## Controllo errori

Scegli **Controllo errori** nel gruppo **Verifica** della scheda Formule per esaminare il foglio alla ricerca di celle che potrebbero essere errate. Cerca due cose: una formula che restituisce un errore (come #DIV/0!), e un numero memorizzato come testo, che le formule potrebbero ignorare.

La finestra di dialogo mostra una cella alla volta, vi porta sul foglio e spiega con una frase quale sia il problema. **Successivo** e **Precedente** scorrono le celle. Un piccolo triangolo verde nell'angolo superiore sinistro di una cella la segnala sul foglio. **Ignora errore** toglie la cella dall'elenco, rimuove il suo triangolo ed è salvato nel file. Un passo di **Annulla** lo ripristina. Se non c'è nulla che non va, viene segnalato.

## Valuta formula

Usala per vedere come una formula arriva al proprio valore. Seleziona una cella che contiene una formula e scegli **Valuta formula**. La formula viene mostrata con la parte successiva sottolineata. Premi **Valuta** per calcolare quella parte: il suo valore prende il suo posto. Le parti vengono prese in quest'ordine: ogni singola cella da sinistra, poi ogni funzione con le sue parentesi, a partire da quella più interna, infine ciò che resta. **Riavvia** ricomincia dall'inizio. Se una parte non può essere calcolata, la finestra di dialogo lo spiega e lascia la formula com'era.

## Finestra di controllo

La **Finestra di controllo** è una piccola finestra che resta visibile mentre lavori. Elenca le celle che vuoi tenere sotto controllo, con il loro foglio, la cella, il valore e la formula, anche quando si trovano su un altro foglio. Seleziona le celle e premi **Aggiungi controllo**. Seleziona le righe nell'elenco e premi **Elimina controllo**. Fai doppio clic su una riga per raggiungere la cella. I valori cambiano quando il foglio viene ricalcolato. L'elenco viene conservato finché la cartella di lavoro è aperta.
