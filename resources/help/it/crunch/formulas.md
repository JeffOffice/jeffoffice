# Come vengono mostrate e calcolate le formule

<!-- keywords: spill, spilled, #SPILL!, matrice dinamica, intervallo di spill, A1#, errore successivo, errore precedente, scorri gli errori, traccia errore, calcola prima del salvataggio, formula, formule, calcola, calcolo, ricalcola, funzione, funzioni, valore, risultato, memorizzato, F9, circolare, mostra formule, barra della formula, errore, punta, puntamento, fai clic su una cella, riferimento, F4, controllo errori, ignora errore, valuta formula, passo, finestra di controllo, controlla, LET, assegna un nome a un valore, funzione CELLA, funzione INFO, FILTRAXML, xml, xpath, precisione come visualizzata, imposta precisione come visualizzata, arrotonda, arrotondamento, decimali, cifre -->

Una **formula** è una cella che calcola il proprio valore da altre celle, ad esempio =SOMMA(B3:B8). La cella mostra il valore, e la **barra della formula** mostra la formula.

## Che cosa fa JeffCrunch con le formule

Quando apri una cartella di lavoro, il file contiene l'ultimo valore di ogni formula, così il foglio può essere mostrato subito. Poi JeffCrunch ricalcola ogni formula, alcune alla volta, in modo che la finestra resti veloce. La barra di stato mostra **Calcolo** con una percentuale mentre lavora. Al termine, le celle contengono i risultati di JeffCrunch. Per ricalcolare tutto, fai clic su **Ricalcola tutto** nella scheda **Formule**, oppure premi **F9**.

## Quando JeffCrunch non può calcolare una formula

JeffCrunch ha più di 400 funzioni, e il loro numero continua a crescere. Una formula che usa una funzione non ancora disponibile in JeffCrunch conserva il valore memorizzato nel file. Lo stesso vale per una formula che fa riferimento a un'altra cartella di lavoro. Le celle che dipendono da queste vengono calcolate a partire dai valori memorizzati.

## Funzioni che richiedono alcune note

- **Obbligazioni e prestiti.** PREZZO, REND, DURATA, PREZZO.PRIMO.IRR e le altre funzioni sulle obbligazioni contano i giorni secondo la **base** indicata (0 significa 30 giorni al mese in un anno di 360 giorni, 1 significa i giorni effettivi, e così via). REND, REND.PRIMO.IRR, REND.ULTIMO.IRR e TASSO trovano il risultato provando dei valori finché il prezzo o la rata corrispondono. Si fermano quando il risultato è esatto fino a circa 15 cifre, quindi le ultime cifre possono differire da quelle di un altro programma.
- **Distribuzioni e test.** Le funzioni che terminano con .DISTRIB, .INV, .TEST e i nomi più vecchi come DISTRIB.NORM e DISTRIB.T danno le stesse risposte: un nome più vecchio viene mantenuto perché i file precedenti continuino a funzionare. Le funzioni *.INV trovano il risultato anch'esse provando dei valori.
- **Liste che riempiono più celle.** ORDINA, ORDINA.PER, FILTRA, UNICI, SEQUENZA, MATR.CASUALE, CERCA.X, MMULT, MATR.INVERSA e REGR.LIN restituiscono un blocco di valori. Scrivine una in una sola cella e il blocco si estende nelle celle sotto e accanto (vedi **Formule che riempiono le celle accanto a sé**), oppure inseriscila come formula matriciale (**Ctrl+Maiusc+Invio**) sulle celle che deve riempire.
- **Testi in altri sistemi di scrittura.** LUNGHEZZAB, SINISTRAB, STRINGA.ESTRAI.B, DESTRAB, TROVA.B, RIFERIMENTO.B e SOSTITUISCI.B contano ogni carattere come uno, come accade su un computer non impostato per il testo a doppio byte. FONETICO restituisce il testo della cella, perché JeffCrunch non conserva le letture del testo giapponese.
- **CONVERTI.** Le unità si scrivono come testo, con le maiuscole e le minuscole indicate nell'elenco della funzione: "kg", "lbm", "mi", "F". Un prefisso metrico può precedere un'unità che lo accetta, come "km" o "mL".

## MOD: il resto

**RESTO(n; d)** restituisce ciò che resta di **n** dopo aver tolto il numero intero più grande di copie di **d**: è n - d * INT(n / d). Esempio: **=RESTO(10; 3)** dà 1, e **=RESTO(-10; 3)** dà 2, perché INT(-10 / 3) è -4 e -10 - 3 * -4 è 2. Il risultato ha il segno di **d**. Se **d** è 0, il risultato è #DIV/0!.

## Mostrare le formule

Fai clic su **Mostra formule** nella scheda **Formule**, oppure premi **Ctrl+'** (su Mac, il tasto Control e non il tasto Comando), per vedere la formula di ogni cella al posto del suo valore. Le colonne che contengono formule vengono allargate, fino a un limite, perché le formule si vedano per intero; tornano alla larghezza originaria quando disattivi la funzione, e le larghezze nel file non vengono modificate.

## Modificare le formule

Per modificare una formula, seleziona la sua cella, premi **F2**, modificala e premi **Invio**. Puoi modificarla anche nella barra della formula. Quando cambi una cella, le formule che ne dipendono vengono ricalcolate in background. Vedi [Digitare nelle celle](edit-cells).

## Scrivere una formula

Digita **=** e le prime lettere di una funzione: compare un breve elenco delle funzioni e dei [nomi](names) che iniziano in quel modo, con una frase per ciascuno. Premi **Tab** per scegliere quello evidenziato. Usa le frecce **Giù** e **Su** per evidenziarne un altro, poi **Tab** o **Invio**. **Esc** chiude l'elenco. Quando sei dentro le parentesi di una funzione, i suoi argomenti compaiono sotto la cella, con quello che stai digitando in grassetto. L'elenco e gli argomenti sono gli stessi che mostra **Scegli una funzione**. Gli argomenti sono separati da virgole, o da punti e virgola dove la tua lingua usa la virgola come separatore decimale.

## Puntare alle celle

Invece di digitare l'indirizzo di una cella, puoi indicarla. Mentre scrivi una formula, metti il cursore dove può comparire una cella (subito dopo **=**, dopo un operatore come **+**, una parentesi aperta o una virgola) e fai clic sulla cella. Il suo indirizzo viene scritto per te. Trascina sulle celle per scrivere un intervallo come B2:D5. Fai di nuovo clic prima di digitare altro, e la nuova cella sostituisce quella appena scritta. Tieni premuto **Ctrl** e fai clic per aggiungere un altro indirizzo dopo una virgola. Fai clic su una lettera di colonna per scrivere un'intera colonna come A:A, oppure su un numero di riga per scrivere un'intera riga come 3:3, e trascina lungo le lettere o i numeri per B:D o 2:5.

Funzionano anche i tasti freccia. Premi una freccia e un puntatore si sposta lontano dalla cella in cui stai scrivendo, e il suo indirizzo viene scritto; premi altre frecce per proseguire, oppure tieni premuto **Maiusc** per estendere il puntatore su un intervallo. La barra di stato mostra **Punta** mentre i tasti freccia puntano, **Invio** mentre digiti, e **Modifica** mentre i tasti freccia spostano il cursore nel testo. Premi **F2** per passare dal puntamento alla modifica del testo.

Per puntare a una cella di un altro foglio, fai clic sulla scheda di quel foglio mentre il cursore si trova dove può comparire una cella, poi fai clic sulla cella. Il nome del foglio viene scritto insieme all'indirizzo, tra apici quando serve, come in 'Il mio foglio'!B3. **Invio** ti riporta al foglio di partenza e inserisce la formula nella sua cella. **Esc** annulla tutto, compreso il cambio di foglio.

Puoi anche trascinare il bordo di un intervallo evidenziato sul foglio per spostare quell'indirizzo, oppure trascinare il suo quadratino d'angolo per allargarlo; la formula cambia mentre trascini.

Premi **F4** con il cursore dentro o subito dopo un indirizzo per fissare alcune parti con il segno del dollaro: A1 diventa $A$1, poi A$1, poi $A1, e poi di nuovo A1.

Ogni indirizzo della formula è colorato nel testo, e le celle che nomina vengono evidenziate sul foglio con lo stesso colore, così vedi a colpo d'occhio che cosa legge la formula. Un clic subito dopo un indirizzo completo, come in =A1, termina l'inserimento e seleziona la cella su cui hai fatto clic, come accade per qualsiasi altro testo.

## Tabelle dei dati

Una tabella dei dati (tabella di simulazione) mostra il risultato di una formula per molti valori di una o due celle. Apri una cartella di lavoro che ne contiene una, e JeffCrunch ne calcola le celle: per una tabella con due variabili, la formula si trova nella cella in alto a sinistra della tabella, i valori da provare in orizzontale vanno nella cella di input di riga, e quelli in verticale nella cella di input di colonna. Per una tabella con una sola variabile, i valori vanno nella colonna a sinistra (e le formule nella riga sopra) oppure nella riga sopra (e le formule nella colonna a sinistra). Ogni cella della tabella è il risultato della formula, e di ogni formula che dipende dalla cella di input, con l'input impostato a quel valore; nient'altro nella cartella di lavoro viene modificato. In modalità **Automatica** le tabelle vengono calcolate insieme alle formule, e le formule che leggono le celle di una tabella si aggiornano di conseguenza. La creazione di una nuova tabella dei dati non è ancora disponibile. Una tabella che non può essere calcolata (una formula che dipende dall'input usa una funzione non disponibile in JeffCrunch, oppure cambia da sola, come CASUALE o ORA) conserva i valori contenuti nel file.

## Calcolo manuale

Fai clic su **Modalità di calcolo** nella scheda **Formule** per scegliere come la cartella di lavoro calcola le formule:

- **Automatica** calcola le formule man mano che modifichi le celle.
- **Automatica, tabelle manuali** fa lo stesso, ma lascia invariate le celle delle tabelle dei dati finché non premi **F9**.
- **Manuale** non calcola nulla, tabelle dei dati comprese, finché non lo chiedi. La barra di stato mostra **Da aggiornare** dopo una modifica. Premi **F9** (**Ricalcola tutto**) per calcolare tutto, oppure **Maiusc+F9** (**Ricalcola questo foglio**) per calcolare le formule del foglio corrente e tutto ciò che era in attesa. Una cartella di lavoro impostata su Manuale si apre con i risultati contenuti nel file. **Calcola prima del salvataggio**, anch'esso nel menu **Modalità di calcolo**, è selezionato per impostazione predefinita: quando salvi una cartella di lavoro Manuale non aggiornata, le formule vengono calcolate prima, così il file contiene numeri aggiornati. Disattivala per salvare i numeri così come sono.

La scelta viene salvata con la cartella di lavoro, ed è un passaggio che puoi annullare.

## Formule che leggono se stesse

Una formula che legge il proprio risultato, direttamente o attraverso altre celle, non può essere calcolata nel modo usuale. JeffCrunch lo segnala una volta, indica le celle e le evidenzia sul foglio; queste mantengono i valori che avevano. Fai clic su **Riferimenti circolari** nella scheda **Formule** per l'elenco, e fai doppio clic su una cella dell'elenco per raggiungerla.

Alcuni modelli richiedono davvero un ciclo di questo tipo. Scegli **Calcola formule circolari** nel menu **Modalità di calcolo**: le formule vengono ricalcolate più volte, ognuna leggendo gli ultimi valori delle altre, finché nessun valore cambia più della quantità indicata oppure non si raggiunge il numero massimo di iterazioni.

## Mantenere i numeri come li mostrano le celle

Una cella può mostrare meno cifre di quante ne contiene. Se una cella contiene 2,675 e ne mostra 2,68, le formule che la leggono usano ancora 2,675. A volte si vuole che usino ciò che si vede. In questo caso scegli **Imposta precisione come visualizzata** nel menu **Opzioni di calcolo** della scheda **Formule**.

JeffCrunch ti avvisa prima, perché le cifre nascoste andranno perse. Scegli **Annulla** e non cambia nulla. Scegli **OK** e ogni numero della cartella di lavoro viene arrotondato a ciò che mostra la sua cella. Una cella che mostra due decimali ne conserva due. Una cella che mostra una percentuale conserva la percentuale. Una cella con una data conserva il giorno, e una cella con un orario conserva la parte meno significativa dell'orario che mostra, ad esempio il minuto. Una cella che mostra **Generale** conserva tutte le sue cifre. In una cartella di lavoro grande, mentre l'operazione è in corso compare un avviso con un pulsante **Annulla**. Annulla mentre l'avviso è visibile, e non viene modificato nulla.

Da quel momento, ogni numero che digiti o incolli, e ogni risultato di una formula, viene arrotondato allo stesso modo. Le formule non vengono modificate. La scelta viene salvata con la cartella di lavoro.

L'arrotondamento è un passaggio che puoi annullare: **Annulla** ripristina i numeri. Scegliere di nuovo **Imposta precisione come visualizzata** disattiva la funzione. Questo impedisce solo l'arrotondamento da quel momento in poi; i numeri già arrotondati restano arrotondati.

## Vedere che cosa legge una formula

Seleziona una cella e fai clic su **Mostra celle che alimentano questa cella** per disegnare frecce dalle celle che la sua formula legge. Fai di nuovo clic per risalire di un passo. **Mostra celle che usano questa cella** disegna frecce verso le formule che leggono la cella, e ogni clic va avanti di un passo. Una piccola pagina rappresenta una cella di un altro foglio. Vengono disegnate al massimo 2.000 frecce, e la barra di stato lo segnala se ce n'erano di più. **Rimuovi frecce** le elimina, e le elimina anche qualsiasi modifica alla cartella di lavoro.

Quando una cella mostra un errore come #DIV/0!, selezionala e fai clic su **Traccia errore**. Vengono disegnate frecce verso le celle che legge e che mostrano anch'esse un errore. Fai di nuovo clic per risalire di un passo. Quando nessuna cella che legge mostra un errore, la barra di stato indica che questa cella è la causa dell'errore, ed è quella da correggere.

Per esaminare ogni errore del foglio, fai clic su **Errore successivo** o **Errore precedente**. Le celle che mostrano un errore vengono visitate una riga dopo l'altra, e dopo l'ultima si ricomincia dalla prima. Ogni cella viene selezionata, e le frecce vengono disegnate fino alla cella che causa l'errore. La barra di stato indica quale errore stai guardando e quanti sono in tutto.

## Controllo errori

Scegli **Controllo errori** nel gruppo **Verifica** della scheda Formule per cercare nel foglio le celle che potrebbero essere sbagliate. Cerca due cose: una formula che restituisce un errore (come #DIV/0!), e un numero memorizzato come testo, che le formule potrebbero ignorare.

La finestra di dialogo mostra una cella alla volta, va a quella cella nel foglio e spiega in una frase il problema. **Successivo** e **Precedente** scorrono le celle. Un piccolo triangolo verde nell'angolo superiore sinistro di una cella la segnala nel foglio. **Ignora errore** toglie la cella dall'elenco, rimuove il suo triangolo e viene salvato nel file. Un passo di **Annulla** lo ripristina. Se non c'è nulla che non va, ti viene detto.

## Valuta formula

Usa questa funzione per vedere come una formula arriva al suo valore. Seleziona una cella che contiene una formula e scegli **Valuta formula**. La formula viene mostrata con la parte successiva sottolineata. Premi **Valuta** per calcolare quella parte; il suo valore prende il suo posto. Le parti vengono valutate in quest'ordine: prima ogni singola cella da sinistra, poi ogni funzione con le sue parentesi, a partire da quella più interna, e infine ciò che resta. **Riavvia** ricomincia da capo. Se una parte non può essere calcolata, la finestra di dialogo spiega il motivo e lascia la formula com'era.

## Finestra di controllo

La **Finestra di controllo** è una piccola finestra che resta visibile mentre lavori. Elenca le celle che vuoi tenere sotto controllo, con il loro foglio, la cella, il valore e la formula, anche quando si trovano in un altro foglio. Seleziona delle celle e premi **Aggiungi controllo**. Seleziona delle righe nell'elenco e premi **Elimina controllo**. Fai doppio clic su una riga per andare alla cella. I valori cambiano quando il foglio viene ricalcolato. L'elenco viene conservato finché la cartella di lavoro è aperta.

## Formule che riempiono le celle accanto a sé

Quando una formula restituisce un blocco di valori, il blocco riempie le celle sotto e accanto alla cella in cui l'hai scritta. Questo si chiama spill (riversamento). Prova =SEQUENZA(3) in A1: A1 mostra 1, e A2 e A3 mostrano 2 e 3. Una formula va in spill quando richiama una funzione che restituisce un blocco (ORDINA, ORDINA.PER, FILTRA, UNICI, SEQUENZA, MATR.CASUALE, MATR.TRASPOSTA, CERCA.X o MMULT), scrive un blocco tra parentesi graffe, usa un intervallo di spill (vedi sotto), oppure esegue un'operazione aritmetica su un intervallo, come in =A1:A3*2.

- **Le celle riempite non contengono una formula propria.** Mostrano i valori. Quando ne selezioni una, la barra della formula mostra in grigio la formula della cella che la riempie. Premere Invio su di essa così com'è non cambia nulla. Per modificare il blocco, modifica prima di tutto quella prima cella.
- **Il blocco segue il risultato.** Se la formula restituisce più o meno valori dopo una modifica di ciò che legge, il blocco cresce o si riduce, e le formule che leggono quelle celle vengono ricalcolate.
- **Una cella di ostacolo genera #SPILL!.** Se una delle celle in cui il blocco dovrebbe andare contiene un valore o una formula, la prima cella mostra **#SPILL!** e non viene riempito nulla. Svuota la cella di ostacolo e il blocco si riempie subito. Digitare in una cella che il blocco dovrebbe riempire produce lo stesso effetto: la prima cella mostra #SPILL!.
- **Un blocco ha un limite di dimensione.** Un blocco più grande del foglio, o di più di 200.000 celle, genera #SPILL!.
- **A1# è l'intero blocco.** Scrivi la cella che contiene la formula seguita da **#**, come in =SOMMA(A1#), per usare l'intero blocco così com'è ora, dal foglio corrente o da un altro (=SOMMA(Foglio1!A1#)). Se A1 non va in spill, A1# dà #RIF!.

Uno spill viene salvato nel file come matrice dinamica, così il programma da cui il file proviene lo mostra anch'esso come spill. Salvando nei formati più vecchi .xls o .ods vengono conservati i valori e, dove il formato più vecchio non ha le funzioni usate, la formula come formula matriciale sul blocco. Verifica compatibilità segnala queste formule.

## Dare un nome ai valori dentro una formula

**LET** assegna un nome a un valore nel mezzo di una formula, così lo scrivi una volta e lo riusi. Le coppie di nome e valore vengono prima, e il calcolo alla fine. Nella formula =LET(prezzo; B2; imposta; prezzo*0,08; prezzo+imposta) il nome **prezzo** rappresenta il valore in B2, **imposta** viene calcolata a partire da esso, e l'ultima parte le somma. Un nome può usare i nomi che lo precedono, e un LET può contenere fino a 126 coppie. Un nome funziona solo all'interno del proprio LET, e un LET contenuto in un altro può riusare un nome. Un nome può rappresentare anche un intervallo di celle, come in =LET(righe; B2:B9; SOMMA(righe)/CONTA.NUMERI(righe)).

## Chiedere informazioni su una cella o sulla cartella di lavoro

**CELLA** fornisce informazioni su una cella. Il suo primo argomento indica quali: "address" (indirizzo), "row" (riga), "col" (colonna), "contents" (contenuto), "type" (tipo), "format" (come vengono mostrati i numeri, ad esempio "F2" per due decimali, "P0" per una percentuale o "D1" per una data), "color" (1 quando i numeri negativi sono mostrati in colore), "parentheses" (1 quando i numeri positivi sono mostrati tra parentesi), "prefix" (il segno di un testo allineato a sinistra, a destra o al centro), "protect" (1 quando la cella è bloccata), "width" (la larghezza della colonna in caratteri), e "filename" (la cartella, il file e il foglio, come /home/io/[Budget.xlsx]Foglio1; vuoto finché la cartella di lavoro non viene salvata). **INFO** fornisce informazioni sul programma e sulla cartella di lavoro: "directory", "numfile" (quanti fogli sono visualizzati), "origin" (origine), "osversion" (versione del sistema operativo), "recalc" (Automatico o Manuale), "release" e "system" (sistema).

## Estrarre valori da XML

**FILTRAXML** richiede un testo che contiene XML e un XPath, cioè un modo breve per indicare quali parti vuoi, e restituisce i valori che trova. =FILTRAXML(A1; "//articolo/prezzo") restituisce ogni prezzo del testo in A1, uno sotto l'altro. Usa "/negozio/articolo[2]/nome" per il secondo articolo, "//articolo[@id='b']/nome" per l'articolo il cui id è b, oppure "count(//articolo)" per sapere quanti sono. Se il testo non è XML valido, o il percorso non è valido, il risultato è #VALORE!. Se nulla corrisponde, è #N/D. L'XML che contiene una dichiarazione di tipo di documento viene rifiutato.
