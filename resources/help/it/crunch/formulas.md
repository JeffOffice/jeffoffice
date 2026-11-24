# Come vengono mostrate e calcolate le formule

<!-- keywords: spill, spilled, #SPILL!, matrice dinamica, intervallo di spill, A1#, errore successivo, errore precedente, scorri gli errori, traccia errore, calcola prima del salvataggio, formula, formule, calcola, calcolo, ricalcola, funzione, funzioni, valore, risultato, memorizzato, F9, circolare, mostra formule, barra della formula, errore, punta, puntamento, clic su una cella, riferimento, F4, verifica errori, ignora errore, valuta formula, passo, finestra di controllo, controllo, LET, assegna un nome a un valore, funzione cella, funzione info, filterxml, xml, xpath, precisione come visualizzata, imposta precisione come visualizzata, arrotonda, arrotondamento, decimali, cifre -->

Una **formula** è una cella che ricava il proprio valore da altre celle, come =SOMMA(B3:B8). La cella mostra il valore, e la **barra della formula** mostra la formula.

## Che cosa fa JeffCrunch con le formule

Quando apri una cartella di lavoro, il file contiene l'ultimo valore di ogni formula, così il foglio può essere mostrato subito. Poi JeffCrunch calcola di nuovo ogni formula, poche alla volta, in modo che la finestra resti rapida. La barra di stato indica **Calcolo in corso** con una percentuale mentre lavora. Quando ha finito, le celle contengono i risultati di JeffCrunch. Per calcolare di nuovo tutto, fai clic su **Ricalcola tutto** nella scheda **Formule**, oppure premi **F9**.

## Quando JeffCrunch non può calcolare una formula

JeffCrunch ha più di 400 funzioni, e il loro numero continua a crescere. Una formula che usa una funzione non ancora presente in JeffCrunch conserva il valore memorizzato dal file. Lo stesso vale per una formula che fa riferimento a un'altra cartella di lavoro. Le celle che dipendono da queste formule vengono calcolate a partire dai valori memorizzati.

## Funzioni che richiedono alcune precisazioni

- **Obbligazioni e prestiti.** PREZZO, RENDITA, DURATA, PREZZO.PRIMO.IRR e le altre funzioni sulle obbligazioni contano i giorni secondo la **base** che indichi (0 significa 30 giorni al mese in un anno di 360 giorni, 1 significa i giorni effettivi, e così via). RENDITA, RENDITA.PRIMO.IRR, RENDITA.ULTIMO.IRR e TASSO trovano il risultato provando dei valori finché il prezzo o la rata corrispondono. Si fermano quando la risposta è esatta fino a circa 15 cifre, quindi le ultime cifre possono differire da quelle di un altro programma.
- **Distribuzioni e test.** Le funzioni che terminano con .DISTRIB, .INV, .TEST, e i nomi precedenti come DISTRIB.NORM e DISTRIB.T, danno gli stessi risultati: un nome precedente viene mantenuto perché i file vecchi continuino a funzionare. Anche le funzioni *.INV trovano il risultato provando dei valori.
- **Elenchi che occupano più celle.** ORDINA, ORDINA.PER, FILTRA, UNICI, SEQUENZA, MATR.CASUALE, XCERCA, MMULT, MATR.INVERSA e REGR.LIN restituiscono un blocco di valori. Digitane una in una sola cella e il blocco si estende nelle celle sotto e accanto ad essa (vedi **Formule che occupano le celle accanto**), oppure inseriscila come formula matriciale (**Ctrl+Maiusc+Invio**) sulle celle che deve riempire.
- **Testi in altre scritture.** LUNGHEZZA.B, SINISTRA.B, STRINGA.ESTRAI.B, DESTRA.B, TROVA.B, RICERCA.B e SOSTITUISCI.B contano ogni carattere come uno, come accade su un computer non predisposto per il testo a doppio byte. FONETICA restituisce il testo della cella, perché JeffCrunch non conserva le letture del testo giapponese.
- **CONVERTI.** Le unità si scrivono come testo, con maiuscole e minuscole come indicato nell'elenco della funzione: "kg", "lbm", "mi", "F". Un prefisso del sistema metrico può precedere un'unità che lo ammette, come "km" o "mL".

## MOD: il resto

**MOD(n; d)** restituisce ciò che resta di **n** dopo aver tolto il maggior numero intero di copie di **d**: è n - d * INT(n / d). Esempio: **=MOD(10; 3)** dà 1, e **=MOD(-10; 3)** dà 2, perché INT(-10 / 3) è -4 e -10 - 3 * -4 è 2. Il risultato ha il segno di **d**. Se **d** è 0, il risultato è #DIV/0!.

## Mostra le formule

Fai clic su **Mostra formule** nella scheda **Formule**, oppure premi **Ctrl+`** (su un Mac, il tasto Control e non il tasto Comando), per vedere la formula di ogni cella al posto del suo valore. Le colonne che contengono formule vengono allargate, fino a un limite, così che le formule si leggano per intero; tornano alla larghezza originaria quando disattivi la funzione, e le larghezze nel file non vengono modificate.

## Modificare le formule

Per modificare una formula, seleziona la sua cella, premi **F2**, modificala e premi **Invio**. Puoi anche modificarla nella barra della formula. Quando cambi una cella, le formule che dipendono da essa vengono ricalcolate in background. Vedi [Digitare nelle celle](edit-cells).

## Scrivere una formula

Digita **=** e le prime lettere di una funzione: compare un breve elenco con le funzioni e i [nomi](names) che iniziano in quel modo, con una frase per ciascuno. Premi **Tab** per prendere quella evidenziata. Usa i tasti **Freccia giù** e **Freccia su** per evidenziarne un'altra, quindi **Tab** o **Invio**. **Esc** chiude l'elenco. Quando sei dentro le parentesi di una funzione, i suoi argomenti compaiono sotto la cella, con quello che stai digitando in grassetto. L'elenco e gli argomenti sono gli stessi mostrati da **Scegli una funzione**. Gli argomenti sono separati da virgole, oppure da punti e virgola dove la tua lingua usa la virgola come separatore decimale.

## Nomi delle funzioni nella tua lingua

Quando JeffCrunch è mostrato in tedesco, francese, spagnolo, italiano, olandese, portoghese o russo, puoi digitare una formula con i nomi delle funzioni usati nella tua lingua, come `=SUMME(A1;A2)` in tedesco o `=SOMME(A1;A2)` in francese. Funzionano anche i nomi inglesi, quindi una formula copiata dal Web non va modificata. La barra della formula, la cella, l'elenco delle funzioni e **Scegli una funzione** mostrano i nomi della tua lingua.

La cartella di lavoro non cambia quando cambi lingua. Un file mantiene sempre i nomi inglesi, così una cartella di lavoro creata in una lingua si apre e si legge correttamente in un'altra, e in qualsiasi altro programma. I nomi locali sono disponibili in tedesco, francese, spagnolo, italiano, olandese, portoghese (Brasile e Portogallo) e russo; i nomi inglesi funzionano sempre, in ogni lingua. I programmi in ucraino e indonesiano mantengono i nomi inglesi per quasi tutte le funzioni. Quando una lingua ha due nomi comuni per una funzione, vengono letti entrambi e viene mostrato quello più usato. Una funzione che non ha un nome abituale nella tua lingua conserva quello inglese. VERO e FALSO seguono anch'essi la tua lingua. I valori di errore della tua lingua, come `#N/D` in tedesco, vengono riconosciuti quando digiti una formula e vengono mostrati nella barra della formula, ma una cella con un errore mostra sempre la forma standard, come `#N/D`.

Quando le impostazioni internazionali del computer usano la virgola come separatore decimale, gli argomenti di una funzione sono separati da un punto e virgola. Una matrice tra parentesi graffe con più di una riga viene mostrata con un punto e virgola tra i valori di una riga e una barra rovesciata tra le righe, come `{1;2\3;4}`, e puoi digitarla in questo modo. Una matrice digitata senza barra rovesciata viene letta come la scrive un file: virgole tra i valori di una riga, punti e virgola tra le righe, e il punto come separatore decimale.

Per digitare solo nomi inglesi, fai clic su **File > Opzioni > Formule** e seleziona **Usa nomi di funzione in inglese**.

## Puntare alle celle

Invece di digitare l'indirizzo di una cella, puoi puntarla. Mentre digiti una formula, metti il cursore dove può andare una cella (subito dopo **=**, un operatore come **+**, una parentesi aperta o una virgola) e fai clic sulla cella. Il suo indirizzo viene scritto per te. Trascina sulle celle per scrivere un intervallo come B2:D5. Fai clic di nuovo prima di digitare altro, e la nuova cella sostituisce quella appena scritta. Tieni premuto **Ctrl** e fai clic per aggiungere un altro indirizzo dopo una virgola. Fai clic su una lettera di colonna per scrivere un'intera colonna come A:A, o su un numero di riga per scrivere un'intera riga come 3:3, e trascina lungo le lettere o i numeri per B:D o 2:5.

Funzionano anche i tasti freccia. Premi una freccia e un puntatore si sposta lontano dalla cella in cui stai scrivendo, e il suo indirizzo viene scritto; premi altre frecce per proseguire, oppure tieni premuto **Maiusc** per estendere il puntatore su un intervallo. La barra di stato indica **Punta** mentre i tasti freccia puntano, **Invio** mentre digiti, e **Modifica** mentre i tasti freccia spostano il cursore nel testo. Premi **F2** per passare dal puntamento alla modifica del testo.

Per puntare una cella di un altro foglio, fai clic sulla linguetta di quel foglio mentre il cursore è dove può andare una cella, quindi fai clic sulla cella. Il nome del foglio viene scritto insieme all'indirizzo, tra apici quando serve, come in 'Il mio foglio'!B3. **Invio** ti riporta al foglio da cui sei partito e inserisce la formula nella sua cella. **Esc** annulla tutto, compreso il cambio di foglio.

Puoi anche trascinare il bordo di un intervallo evidenziato sul foglio per spostare quell'indirizzo, oppure trascinare il suo quadratino d'angolo per estenderlo; la formula cambia mentre trascini.

Premi **F4** con il cursore dentro o subito dopo un indirizzo per bloccare alcune parti con il segno del dollaro: A1 diventa $A$1, poi A$1, poi $A1, e poi di nuovo A1.

Ogni indirizzo della formula è colorato nel testo, e le celle che nomina sono evidenziate sul foglio con lo stesso colore, così vedi a colpo d'occhio che cosa legge la formula. Un clic dopo un indirizzo completo, come =A1, termina l'inserimento e seleziona la cella su cui hai fatto clic, come accade per qualsiasi altro testo.

## Tabelle dati

Una tabella dati (tabella di ipotesi) mostra il risultato di una formula per molti valori di una o due celle. Quando apri una cartella di lavoro che ne contiene una, JeffCrunch calcola le sue celle: per una tabella con due variabili, la formula si trova nella cella in alto a sinistra della tabella, i valori da provare in orizzontale vanno nella cella di input di riga, e quelli in verticale nella cella di input di colonna. Per una tabella con una sola variabile, i valori vanno in colonna nella colonna a sinistra (e le formule si trovano nella riga sopra) oppure in orizzontale nella riga sopra (e le formule si trovano nella colonna a sinistra). Ogni cella della tabella è il risultato della formula, e di ogni formula che dipende dalla cella di input, con l'input impostato a quel valore; nient'altro nella cartella di lavoro viene modificato. In modalità **Automatica** le tabelle vengono calcolate insieme alle formule, e le formule che leggono le celle di una tabella ne seguono i risultati. La creazione di una nuova tabella dati non è ancora disponibile. Una tabella che non può essere calcolata (una formula che dipende dall'input usa una funzione non presente in JeffCrunch, oppure cambia da sola, come CASUALE o ORA) conserva i valori contenuti nel file.

## Calcolo manuale

Fai clic su **Modalità di calcolo** nella scheda **Formule** per scegliere come la cartella di lavoro calcola le formule:

- **Automatica** calcola le formule man mano che cambi le celle.
- **Automatica, tabelle manuali** fa la stessa cosa, ma lascia invariate le celle delle tabelle dati finché non premi **F9**.
- **Manuale** non calcola nulla, tabelle dati comprese, finché non lo chiedi. La barra di stato indica **Non aggiornato** dopo una modifica. Premi **F9** (**Ricalcola tutto**) per calcolare tutto, oppure **Maiusc+F9** (**Ricalcola questo foglio**) per calcolare le formule del foglio in cui ti trovi, insieme a tutto ciò che era in attesa. Una cartella di lavoro impostata su Manuale si apre con i risultati contenuti nel file. **Calcola prima del salvataggio**, anch'esso nel menu **Modalità di calcolo**, è selezionato per impostazione predefinita: quando salvi una cartella di lavoro manuale non aggiornata, le formule vengono calcolate prima, così il file contiene numeri aggiornati. Disattivalo per salvare i numeri così come sono.

La scelta viene salvata con la cartella di lavoro ed è un passaggio che puoi annullare.

## Formule che leggono se stesse

Una formula che legge il proprio risultato, direttamente o attraverso altre celle, non può essere calcolata nel modo usuale. JeffCrunch lo segnala una sola volta, indica le celle e le evidenzia sul foglio; esse mantengono i valori che avevano. Fai clic su **Riferimenti circolari** nella scheda **Formule** per l'elenco, e fai doppio clic su una cella dell'elenco per raggiungerla.

Alcuni modelli richiedono di proposito un circolo di questo tipo. Scegli **Calcolo iterativo delle formule circolari** nel menu **Modalità di calcolo**: le formule vengono calcolate più volte, leggendo ogni volta gli ultimi valori delle altre, finché nessun valore cambia di più della quantità indicata oppure non viene raggiunto il numero di iterazioni consentito.

## Mantenere i numeri come li mostrano le celle

Una cella può mostrare meno cifre di quante ne contiene. Se una cella contiene 2,675 e mostra 2,68, le formule che la leggono usano comunque 2,675. A volte vuoi che usino ciò che vedi. In questo caso scegli **Imposta precisione come visualizzata** nel menu **Opzioni di calcolo** della scheda **Formule**.

JeffCrunch ti avvisa prima, perché le cifre nascoste andranno perse. Scegli **Annulla** e non cambia nulla. Scegli **OK** e ogni numero della cartella di lavoro viene arrotondato a ciò che mostra la sua cella. Una cella che mostra due decimali ne conserva due. Una cella che mostra una percentuale conserva la percentuale. Una cella con una data conserva il giorno, e una cella con un orario conserva la parte meno significativa dell'orario che mostra, come il minuto. Una cella che mostra **Generale** conserva tutte le sue cifre. In una cartella di lavoro grande, durante l'operazione compare una nota con un pulsante **Annulla**. Se annulli mentre la nota è visibile, non cambia nulla.

Da quel momento, ogni numero che digiti o incolli, e ogni risultato di una formula, viene arrotondato nello stesso modo. Le formule non vengono modificate. La scelta viene salvata con la cartella di lavoro.

L'arrotondamento è un passaggio che puoi annullare: **Annulla** riporta i numeri com'erano. Scegliere di nuovo **Imposta precisione come visualizzata** disattiva la funzione. Questo impedisce solo l'arrotondamento da quel momento in poi: i numeri già arrotondati restano arrotondati.

## Vedere che cosa legge una formula

Seleziona una cella e fai clic su **Mostra celle che alimentano questa cella** per disegnare frecce dalle celle che la sua formula legge. Fai clic di nuovo per andare un passo più indietro. **Mostra celle che usano questa cella** disegna frecce verso le formule che leggono la cella, e ogni clic porta un passo più avanti. Una piccola pagina rappresenta una cella di un altro foglio. Vengono disegnate al massimo 2.000 frecce, e la barra di stato lo segnala quando ce ne sono di più. **Rimuovi frecce** le elimina, e le elimina anche qualsiasi modifica alla cartella di lavoro.

Quando una cella mostra un errore come #DIV/0!, selezionala e fai clic su **Traccia errore**. Vengono disegnate frecce verso le celle che legge e che a loro volta mostrano un errore. Fai clic di nuovo per andare un passo più indietro. Quando nessuna delle celle che legge mostra un errore, la barra di stato indica che questa cella è la causa dell'errore, ed è quella da correggere.

Per passare in rassegna ogni errore del foglio, fai clic su **Errore successivo** o **Errore precedente**. Le celle che mostrano un errore vengono visitate riga dopo riga, e dopo l'ultima si ricomincia dalla prima. Ogni cella viene selezionata, e le frecce arrivano fino alla cella che causa l'errore. La barra di stato indica di quale errore si tratta e quanti sono in tutto.

## Verifica errori

Scegli **Verifica errori** nel gruppo **Controllo** della scheda Formule per cercare nel foglio le celle che potrebbero essere sbagliate. Cerca due cose: una formula che restituisce un errore (come #DIV/0!), e un numero memorizzato come testo, che le formule potrebbero ignorare.

La finestra di dialogo mostra una cella alla volta, porta quella cella sul foglio e spiega in una frase qual è il problema. **Successivo** e **Precedente** scorrono le celle. Un piccolo triangolo verde nell'angolo in alto a sinistra di una cella la segnala sul foglio. **Ignora errore** toglie la cella dall'elenco, elimina il suo triangolo ed è salvato nel file. Un passaggio di Annulla lo riporta. Se non c'è nulla da correggere, te lo viene comunicato.

## Valuta formula

Usala per vedere come una formula arriva al suo valore. Seleziona una cella che contiene una formula e scegli **Valuta formula**. La formula compare con la parte successiva sottolineata. Premi **Valuta** per calcolare quella parte; il suo valore prende il suo posto. Le parti vengono prese in quest'ordine: prima ogni singola cella da sinistra, poi ogni funzione con le sue parentesi, a partire da quella più interna, e infine ciò che resta. **Ricomincia** riparte dall'inizio. Se una parte non può essere calcolata, la finestra di dialogo lo spiega e lascia la formula com'era.

## Finestra di controllo

La **Finestra di controllo** è una piccola finestra che resta visibile mentre lavori. Elenca le celle che vuoi tenere sotto controllo, con foglio, cella, valore e formula, anche quando si trovano su un altro foglio. Seleziona delle celle e premi **Aggiungi controllo**. Seleziona delle righe nell'elenco e premi **Elimina controllo**. Fai doppio clic su una riga per andare alla cella. I valori cambiano quando il foglio viene ricalcolato. L'elenco viene mantenuto finché la cartella di lavoro è aperta.

## Formule che occupano le celle accanto

Quando una formula restituisce un blocco di valori, il blocco riempie le celle sotto e accanto alla cella in cui l'hai digitata. Questo si chiama spill (traboccamento). Prova a scrivere =SEQUENZA(3) in A1: A1 mostra 1, e A2 e A3 mostrano 2 e 3. Una formula va in spill quando richiama una funzione che restituisce un blocco (ORDINA, ORDINA.PER, FILTRA, UNICI, SEQUENZA, MATR.CASUALE, MATR.TRASPOSTA, XCERCA o MMULT), scrive un blocco tra parentesi graffe, usa un intervallo di spill (qui sotto), oppure esegue operazioni aritmetiche su un intervallo, come in =A1:A3*2.

- **Le celle che riempie non contengono una formula propria.** Mostrano i valori. Quando ne selezioni una, la barra della formula mostra in grigio la formula della cella che la riempie. Premere Invio su di essa così com'è non cambia nulla. Per modificare il blocco, modifica prima di tutto quella prima cella.
- **Il blocco segue il risultato.** Se la formula restituisce più o meno valori dopo una modifica di ciò che legge, il blocco cresce o si restringe, e le formule che leggono quelle celle vengono ricalcolate.
- **Una cella di ostacolo dà #SPILL!.** Se una cella dove dovrebbe andare il blocco contiene un valore o una formula, la prima cella mostra **#SPILL!** e non viene riempito nulla. Svuota la cella di ostacolo e il blocco si riempie subito. Digitare in una cella che il blocco riempirebbe produce lo stesso effetto: la prima cella mostra #SPILL!.
- **Un blocco ha un limite di dimensione.** Un blocco più grande del foglio, o di più di 200.000 celle, dà #SPILL!.
- **A1# è l'intero blocco.** Scrivi la cella che contiene la formula seguita da **#**, come in =SOMMA(A1#), per usare l'intero blocco così com'è ora, dallo stesso foglio o da un altro (=SOMMA(Foglio1!A1#)). Se A1 non va in spill, A1# dà #RIF!.

Uno spill viene salvato nel file come matrice dinamica, così il programma da cui il file proviene lo mostra anch'esso come spill. Salvando nella forma più vecchia .xls o .ods i valori vengono conservati, e la formula diventa una formula matriciale sul blocco, nei casi in cui la forma più vecchia ha le funzioni usate. Verifica compatibilità segnala queste formule.

## Assegnare nomi ai valori in una formula

**LET** assegna un nome a un valore nel mezzo di una formula, così lo scrivi una volta sola e lo riusi. Le coppie di nome e valore vengono prima, e il calcolo per ultimo. In =LET(prezzo; B2; imposta; prezzo*0,08; prezzo+imposta) il nome **prezzo** indica il valore in B2, **imposta** viene calcolata a partire da esso, e l'ultima parte le somma. Un nome può usare i nomi che lo precedono, e un LET può contenere fino a 126 coppie. Un nome funziona solo all'interno del suo LET, e un LET annidato in un altro può riusare un nome. Un nome può indicare anche un intervallo di celle, come in =LET(righe; B2:B9; SOMMA(righe)/CONTA.NUMERI(righe)).

## Chiedere informazioni su una cella o sulla cartella di lavoro

**CELLA** fornisce informazioni su una cella. Il primo argomento indica quale: "indirizzo", "riga", "col", "contenuto", "tipo", "formato" (come vengono mostrati i numeri, ad esempio "F2" per due decimali, "P0" per una percentuale, o "D1" per una data), "colore" (1 quando i numeri negativi vengono mostrati in colore), "parentesi" (1 quando i numeri positivi vengono mostrati tra parentesi), "prefisso" (il segno di un testo allineato a sinistra, a destra o al centro), "protetta" (1 quando la cella è bloccata), "larghezza" (la larghezza della colonna in caratteri), e "nomefile" (la cartella, il file e il foglio, come /home/io/[Bilancio.xlsx]Foglio1; vuoto finché la cartella di lavoro non viene salvata). **INFO** fornisce informazioni sul programma e sulla cartella di lavoro: "directory", "numfile" (quanti fogli sono visualizzati), "origine", "versionesO", "recalc" (Automatico o Manuale), "release", e "sistema".

## Estrarre valori da XML

**FILTRO.XML** accetta un testo che contiene XML e un XPath, cioè un modo breve per indicare quali parti vuoi, e restituisce i valori che trova. =FILTRO.XML(A1; "//articolo/prezzo") restituisce ogni prezzo del testo in A1, uno sotto l'altro. Usa "/negozio/articolo[2]/nome" per il secondo articolo, "//articolo[@id='b']/nome" per l'articolo il cui id è b, oppure "count(//articolo)" per sapere quanti sono. Se il testo non è XML valido, o il percorso non è corretto, il risultato è #VALORE!. Se non c'è alcuna corrispondenza, il risultato è #N/D. L'XML che contiene una dichiarazione di tipo di documento viene rifiutato.
