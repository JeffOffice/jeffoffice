# Come vengono mostrate e calcolate le formule
<!-- keywords: spill, spilled, #SPILL!, matrice dinamica, intervallo di spill, A1#, errore successivo, errore precedente, scorri gli errori, traccia errore, calcola prima del salvataggio, formula, formule, calcola, calcolo, ricalcola, funzione, funzioni, valore, risultato, memorizzato, F9, circolare, mostra formule, barra della formula, errore, punta, puntamento, clic sulla cella, riferimento, F4, controllo errori, ignora errore, valuta formula, passo, finestra di controllo, controllo, LET, assegna un nome a un valore, funzione CELLA, funzione INFO, FILTRO.XML, xml, xpath, precisione come visualizzata, imposta precisione come visualizzata, arrotonda, arrotondamento, decimali, cifre -->

Una **formula** è una cella che ricava il proprio valore da altre celle, come =SOMMA(B3:B8). La cella mostra il valore, e la **barra della formula** mostra la formula.

## Che cosa fa JeffCrunch con le formule

Quando apri una cartella di lavoro, il file contiene l'ultimo valore di ogni formula, così il foglio può essere mostrato subito. Poi JeffCrunch ricalcola ogni formula, alcune alla volta, in modo che la finestra resti veloce. La barra di stato mostra **Calcolo in corso** con una percentuale mentre lo fa. Al termine, le celle contengono i risultati di JeffCrunch. Per ricalcolare tutto, fai clic su **Ricalcola tutto** nella scheda **Formule** oppure premi **F9**.

## Quando JeffCrunch non può calcolare una formula

JeffCrunch dispone di più di 400 funzioni, e il numero è in crescita. Una formula che usa una funzione non ancora disponibile in JeffCrunch mantiene il valore memorizzato nel file. Lo stesso vale per una formula che fa riferimento a un'altra cartella di lavoro. Le celle che dipendono da queste vengono calcolate a partire dai valori memorizzati.

## Funzioni che richiedono qualche nota

- **Obbligazioni e prestiti.** PREZZO, RENDIMENTO, DURATA, PREZZO.PRIMO.IRR e le altre funzioni sulle obbligazioni contano i giorni secondo la **base** che indichi (0 è 30 giorni al mese in un anno di 360 giorni, 1 sono i giorni effettivi, e così via). RENDIMENTO, RENDITA.PRIMO.IRR, RENDITA.ULTIMO.IRR e TASSO trovano il risultato provando dei valori finché il prezzo o la rata corrispondono. Si fermano quando il risultato è esatto fino a circa 15 cifre, quindi le ultime cifre possono differire da quelle di un altro programma.
- **Distribuzioni e test.** Le funzioni che terminano in .DISTRIB, .INV, .TEST, e i nomi più vecchi come NORMDIST e TDIST, danno le stesse risposte: un nome più vecchio viene mantenuto perché i file vecchi continuino a funzionare. Anche le funzioni *.INV trovano il risultato provando dei valori.
- **Elenchi che riempiono più celle.** ORDINA, ORDINA.PER, FILTRA, UNICI, SEQUENZA, MATR.CASUALE, CERCA.X, MATR.PRODOTTO, MATR.INVERSA e REGR.LIN restituiscono un blocco di valori. Scrivine una in una sola cella e il blocco va in spill nelle celle sotto e accanto ad essa (vedi **Formule che riempiono le celle accanto a sé**), oppure inseriscila come formula matriciale (**Ctrl+Maiusc+Invio**) sulle celle che deve riempire.
- **Testi in altri alfabeti.** LUNGHEZZAB, SINISTRAB, STRINGAB, DESTRAB, TROVAB, RICERCAB e SOSTITUISCIB contano ogni carattere come uno, come accade su un computer non predisposto per il testo a doppio byte. FONETICA restituisce il testo della cella, perché JeffCrunch non conserva le letture del testo giapponese.
- **CONVERTI.** Le unità si scrivono come testo, con maiuscole e minuscole come mostrato nell'elenco della funzione: "kg", "lbm", "mi", "F". Un prefisso metrico può precedere un'unità che lo ammette, come in "km" o "mL".

## MOD: il resto

**MOD(n; d)** restituisce ciò che resta di **n** dopo aver tolto il numero intero più grande di copie di **d**: è n - d * INT(n / d). Esempio: **=MOD(10; 3)** dà 1, e **=MOD(-10; 3)** dà 2, perché INT(-10 / 3) è -4 e -10 - 3 * -4 fa 2. Il risultato ha il segno di **d**. Se **d** è 0, il risultato è #DIV/0!.

## Mostra le formule

Fai clic su **Mostra formule** nella scheda **Formule**, oppure premi **Ctrl+`** (su un Mac, il tasto Control e non il tasto Command), per vedere la formula di ogni cella al posto del suo valore. Le colonne che contengono formule vengono allargate, fino a un certo limite, in modo che le formule si leggano per intero; tornano alla larghezza originale quando disattivi l'opzione, e le larghezze nel file non vengono modificate.

## Modificare le formule

Per modificare una formula, seleziona la sua cella, premi **F2**, modificala e premi **Invio**. Puoi anche modificarla nella barra della formula. Quando cambi una cella, le formule che dipendono da essa vengono ricalcolate in background. Vedi [Digitare nelle celle](edit-cells).

## Scrivere una formula

Digita **=** e le prime lettere di una funzione: compare un breve elenco delle funzioni e dei [nomi](names) che iniziano così, con una frase su ciascuno. Premi **Tab** per scegliere quello evidenziato. Usa le frecce **Giù** e **Su** per evidenziarne un altro, poi **Tab** o **Invio**. **Esc** chiude l'elenco. Una volta dentro le parentesi di una funzione, i suoi argomenti compaiono sotto la cella, con quello che stai digitando in grassetto. L'elenco e gli argomenti sono gli stessi mostrati da **Inserisci funzione**. Gli argomenti si separano con virgole, oppure con punti e virgola dove la tua lingua usa la virgola come separatore decimale.

## Nomi delle funzioni nella tua lingua

Quando JeffCrunch è visualizzato in tedesco, francese, spagnolo, italiano, olandese, portoghese o russo, puoi scrivere una formula con i nomi delle funzioni usati nella tua lingua, come `=SUMME(A1;A2)` in tedesco o `=SOMME(A1;A2)` in francese. Funzionano anche i nomi inglesi, quindi una formula copiata dal web non richiede modifiche. La barra della formula, la cella, l'elenco delle funzioni e **Inserisci funzione** mostrano i nomi della tua lingua.

La cartella di lavoro non cambia quando cambi lingua. Un file mantiene sempre i nomi inglesi, quindi una cartella creata in una lingua si apre e si legge correttamente in un'altra lingua, e in qualsiasi altro programma. I nomi locali sono disponibili in tedesco, francese, spagnolo, italiano, olandese, portoghese (Brasile e Portogallo) e russo; i nomi inglesi funzionano sempre in ogni lingua. I programmi in ucraino e indonesiano mantengono i nomi inglesi per quasi tutte le funzioni. Quando una lingua ha due nomi comuni per una funzione, vengono letti entrambi e viene mostrato quello più diffuso. Una funzione che non ha un nome abituale nella tua lingua mantiene quello inglese. VERO e FALSO seguono anch'essi la lingua. I valori di errore della tua lingua, come `#N/D` in tedesco, vengono letti quando digiti una formula e vengono mostrati nella barra della formula, ma una cella con un errore mostra sempre la forma standard, come `#N/D`.

Quando le impostazioni regionali del computer usano la virgola come separatore decimale, gli argomenti di una funzione si distinguono con il punto e virgola. Una matrice tra parentesi graffe con più di una riga viene mostrata con un punto e virgola tra i valori di una riga e una barra rovesciata tra le righe, come `{1;2\3;4}`, e puoi digitarla in questo modo. Una matrice digitata senza barra rovesciata viene letta come la scrive un file: virgole tra i valori di una riga, punti e virgola tra le righe, e un punto come separatore decimale.

Per digitare solo nomi inglesi, fai clic su **File > Opzioni > Formule** e seleziona **Usa nomi di funzione in inglese**.

## Puntare alle celle

Invece di digitare l'indirizzo di una cella, puoi indicarla con il puntatore. Mentre digiti una formula, metti il cursore dove può andare una cella (subito dopo **=**, dopo un operatore come **+**, una parentesi aperta o una virgola) e fai clic sulla cella. Il suo indirizzo viene scritto per te. Trascina sulle celle per scrivere un intervallo come B2:D5. Fai di nuovo clic prima di digitare altro, e la nuova cella sostituisce quella appena scritta. Tieni premuto **Ctrl** e fai clic per aggiungere un altro indirizzo dopo una virgola. Fai clic su una lettera di colonna per scrivere un'intera colonna, come A:A, o su un numero di riga per scrivere un'intera riga, come 3:3, e trascina lungo le lettere o i numeri per B:D o 2:5.

Anche i tasti freccia funzionano. Premi una freccia e un puntatore si allontana dalla cella in cui stai scrivendo, e il suo indirizzo viene scritto; premi altre frecce per proseguire, oppure tieni premuto **Maiusc** per estendere il puntatore su un intervallo. La barra di stato mostra **Punta** mentre le frecce puntano, **Invio** mentre digiti e **Modifica** mentre le frecce spostano il cursore nel testo. Premi **F2** per passare dal puntamento alla modifica del testo.

Per puntare a una cella di un altro foglio, fai clic sulla linguetta di quel foglio mentre il cursore si trova dove può andare una cella, e poi fai clic sulla cella. Il nome del foglio viene scritto insieme, tra apici quando sono necessari, come in 'Il mio foglio'!B3. **Invio** ti riporta al foglio da cui sei partito e mette la formula nella sua cella. **Esc** annulla tutto, compreso il cambio di foglio.

Puoi anche trascinare il bordo di un intervallo evidenziato sul foglio per spostare quell'indirizzo, oppure trascinare il quadratino dell'angolo per allungarlo; la formula cambia mentre trascini.

Premi **F4** con il cursore dentro o subito dopo un indirizzo per fissarne parti con il segno del dollaro: A1 diventa $A$1, poi A$1, poi $A1, e poi di nuovo A1.

Ogni indirizzo della formula è colorato nel testo, e le celle che nomina sono contornate sul foglio dello stesso colore, così vedi a colpo d'occhio che cosa legge la formula. Un clic dopo un indirizzo completo, come =A1, conclude l'inserimento e seleziona la cella su cui hai fatto clic, come accade per qualsiasi altro testo.

## Tabelle dei dati

Una tabella dei dati (tabella di ipotesi) mostra il risultato di una formula per molti valori di una o due celle. Quando apri una cartella di lavoro che ne contiene una, JeffCrunch calcola le sue celle: per una tabella con due variabili, la formula si trova nella cella in alto a sinistra della tabella, i valori da provare in orizzontale vanno nella cella di input di riga, e quelli in verticale nella cella di input di colonna. Per una tabella con una variabile, i valori vanno nella colonna a sinistra (e le formule si trovano nella riga sopra) oppure nella riga sopra (e le formule si trovano nella colonna a sinistra). Ogni cella della tabella è il risultato della formula, e di ogni formula che dipende dalla cella di input, con l'input impostato su quel valore; nient'altro nella cartella di lavoro viene modificato. In modalità **Automatica** le tabelle vengono calcolate insieme alle formule, e le formule che leggono le celle di una tabella ne seguono i valori. La creazione di una nuova tabella dei dati non è ancora disponibile. Una tabella che non può essere calcolata (una formula che dipende dall'input usa una funzione non disponibile in JeffCrunch, oppure cambia da sola, come CASUALE o ADESSO) mantiene i valori contenuti nel file.

## Calcolo manuale

Fai clic su **Modalità di calcolo** nella scheda **Formule** per scegliere come la cartella di lavoro calcola le formule:

- **Automatica** calcola le formule mentre cambi le celle.
- **Automatica, tabelle manuali** fa lo stesso, ma lascia invariate le celle delle tabelle dei dati finché non premi **F9**.
- **Manuale** non calcola nulla, tabelle dei dati comprese, finché non lo chiedi. La barra di stato mostra **Non aggiornato** dopo una modifica. Premi **F9** (**Ricalcola tutto**) per calcolare tutto, oppure **Maiusc+F9** (**Ricalcola questo foglio**) per calcolare le formule del foglio attivo e tutto il resto che era in attesa. Una cartella di lavoro impostata su Manuale si apre con i risultati contenuti nel file. **Calcola prima del salvataggio**, anch'essa nel menu **Modalità di calcolo**, è selezionata per impostazione predefinita: quando salvi una cartella di lavoro Manuale non aggiornata, le formule vengono calcolate prima, così il file contiene numeri aggiornati. Disattivala per salvare i numeri così come sono.

La scelta viene salvata con la cartella di lavoro, ed è un'operazione che si può annullare.

## Formule che leggono se stesse

Una formula che legge il proprio risultato, direttamente o tramite altre celle, non può essere calcolata nel modo usuale. JeffCrunch lo segnala una volta, indica le celle e le contorna sul foglio; esse mantengono i valori che avevano. Fai clic su **Riferimenti circolari** nella scheda **Formule** per l'elenco, e fai doppio clic su una cella dell'elenco per andarci.

Alcuni modelli richiedono intenzionalmente un ciclo di questo tipo. Scegli **Calcolo iterativo delle formule circolari** nel menu **Modalità di calcolo**, e le formule vengono calcolate più volte, leggendo ogni volta gli ultimi valori delle altre, finché nessun valore cambia più dell'importo indicato oppure non si esauriscono le iterazioni consentite.

## Mantenere i numeri come li mostrano le celle

Una cella può mostrare meno cifre di quante ne contiene. Se una cella contiene 2,675 e mostra 2,68, le formule che la leggono usano ancora 2,675. A volte vuoi che usino ciò che vedi. In tal caso scegli **Imposta precisione come visualizzata** nel menu **Opzioni di calcolo** della scheda **Formule**.

JeffCrunch ti avvisa prima, perché le cifre nascoste andrebbero perse. Scegli **Annulla** e non cambia nulla. Scegli **OK** e ogni numero della cartella di lavoro viene arrotondato a ciò che mostra la sua cella. Una cella che mostra due decimali ne conserva due. Una cella che mostra una percentuale conserva la percentuale. Una cella con una data conserva il giorno, e una cella con un orario conserva la parte meno significativa che mostra, come i minuti. Una cella che mostra **Generale** conserva tutte le sue cifre. Una cartella di lavoro grande mostra una nota con un pulsante **Annulla** durante l'operazione. Annulla mentre la nota è visibile, e non viene modificato nulla.

Dopo questa operazione, ogni numero che digiti o incolli, e ogni risultato di una formula, viene arrotondato allo stesso modo. Le formule non vengono modificate. La scelta viene salvata con la cartella di lavoro.

L'arrotondamento è un'operazione che si può annullare, e **Annulla** ripristina i numeri. Scegliere di nuovo **Imposta precisione come visualizzata** disattiva l'opzione. Questo impedisce solo ulteriori arrotondamenti: i numeri già arrotondati restano tali.

## Vedere che cosa legge una formula

Seleziona una cella e fai clic su **Mostra cosa alimenta questa cella** per disegnare frecce dalle celle che la sua formula legge. Fai di nuovo clic per risalire di un altro passo. **Mostra cosa usa questa cella** disegna frecce verso le formule che leggono la cella, e ogni clic procede di un passo. Una piccola pagina rappresenta una cella di un altro foglio. Vengono disegnate al massimo 2.000 frecce, e la barra di stato lo segnala se erano di più. **Cancella frecce** le rimuove, e le rimuove anche ogni modifica alla cartella di lavoro.

Quando una cella mostra un errore come #DIV/0!, selezionala e fai clic su **Traccia errore**. Vengono disegnate frecce verso le celle che legge e che mostrano anch'esse un errore. Fai di nuovo clic per risalire di un altro passo. Quando nessuna delle celle che legge mostra un errore, la barra di stato indica che questa cella è quella che genera l'errore, ed è quella da correggere.

Per scorrere tutti gli errori del foglio, fai clic su **Errore successivo** o **Errore precedente**. Le celle che mostrano un errore vengono visitate una riga dopo l'altra, e dopo l'ultima ricomincia dalla prima. Ogni cella viene selezionata e le frecce vengono tracciate fino alla cella che genera l'errore. La barra di stato indica quale errore stai esaminando e quanti sono.

## Controllo errori

Scegli **Controllo errori** nel gruppo **Verifica** della scheda Formule per cercare nel foglio le celle che potrebbero essere sbagliate. Cerca una formula che restituisce un errore (come #DIV/0!), un numero memorizzato come testo (che le formule potrebbero ignorare), e altro ancora: una formula diversa da quelle accanto, una data memorizzata come testo con un anno a due cifre, una formula in una cella non bloccata, una formula che esclude un numero accanto alle celle che usa (solo per una funzione che somma un intervallo, come **SOMMA**, **MEDIA**, **CONTA.NUMERI**, **MIN** o **MAX**, quando l'intervallo compare da solo come argomento), e una formula che usa una cella vuota.

La finestra mostra una cella alla volta, passa a quella cella sul foglio e spiega in una frase qual è il problema. **Successivo** e **Precedente** passano da una cella all'altra. Un piccolo triangolo verde nell'angolo superiore sinistro di una cella la segnala sul foglio. Fai clic sul triangolo per aprire un menu accanto alla cella: indica il problema, propone la correzione quando è possibile, e offre **Ignora errore** e **Controllo errori…**. Quando la cella può essere corretta, compare un pulsante con la correzione (per una formula che esclude un numero, **Includi la cella**); esso modifica la formula, e un passo di Annulla ripristina la formula precedente. **Ignora errore** toglie la cella dall'elenco, rimuove il suo triangolo e viene salvato nel file. Un passo di Annulla lo ripristina. Se non c'è nessun problema, viene indicato. Per scegliere quali controlli sono attivi, apri **Opzioni** e la pagina **Formule**: c'è una casella per ciascuno, e una casella deselezionata impedisce di segnalare quel tipo di problema. La scelta riguarda il programma, quindi vale per tutte le finestre aperte. Una cella può avere più di un problema, e **Ignora errore** ignora solo quello visualizzato.

## Valuta formula

Usalo per vedere come una formula arriva al suo valore. Seleziona una cella che contiene una formula e scegli **Valuta formula**. La formula viene mostrata con la parte successiva sottolineata. Premi **Valuta** per calcolare quella parte; il suo valore prende il suo posto. Le parti vengono prese in quest'ordine: ogni singola cella da sinistra, poi ogni funzione con le sue parentesi, partendo da quella più interna, e infine ciò che resta. **Ricomincia** riparte dall'inizio. Se una parte non può essere calcolata, la finestra spiega il motivo e lascia la formula com'era.

## Finestra di controllo

La **Finestra di controllo** è una piccola finestra che resta visibile mentre lavori. Elenca le celle che vuoi tenere sotto controllo, con il loro foglio, la cella, il valore e la formula, anche quando si trovano su un altro foglio. Seleziona delle celle e premi **Aggiungi controllo**. Seleziona le righe dell'elenco e premi **Elimina controllo**. Fai doppio clic su una riga per andare alla cella. I valori cambiano quando il foglio viene ricalcolato. L'elenco resta disponibile finché la cartella di lavoro è aperta.

## Formule che riempiono le celle accanto a sé

Quando una formula restituisce un blocco di valori, il blocco riempie le celle sotto e accanto alla cella in cui l'hai scritta. Questo si chiama spill. Prova =SEQUENZA(3) in A1: A1 mostra 1, e A2 e A3 mostrano 2 e 3. Una formula va in spill quando richiama una funzione che restituisce un blocco (ORDINA, ORDINA.PER, FILTRA, UNICI, SEQUENZA, MATR.CASUALE, MATR.TRASPOSTA, CERCA.X o MATR.PRODOTTO), scrive un blocco tra parentesi graffe, usa un intervallo di spill (vedi sotto), oppure esegue un'operazione aritmetica su un intervallo, come in =A1:A3*2.

- **Le celle che riempie non contengono una formula propria.** Mostrano i valori. Quando ne selezioni una, la barra della formula mostra in grigio la formula della cella che la riempie. Premere Invio su di essa così com'è non cambia nulla. Per modificare il blocco, modifica prima di tutto quella prima cella.
- **Il blocco segue il risultato.** Se la formula restituisce più o meno valori dopo che cambia qualcosa che legge, il blocco cresce o si riduce, e le formule che leggono quelle celle vengono ricalcolate.
- **Una cella di ostacolo dà #SPILL!.** Se una qualsiasi cella in cui il blocco andrebbe contiene un valore o una formula, la prima cella mostra **#SPILL!** e non viene riempito nulla. Svuota la cella di ostacolo e il blocco si riempie subito. Digitare in una cella che il blocco riempie produce lo stesso effetto: la prima cella mostra #SPILL!.
- **Un blocco ha un limite di dimensione.** Un blocco più grande del foglio, o di più di 200.000 celle, dà #SPILL!.
- **A1# è l'intero blocco.** Scrivi la cella che contiene la formula e un **#**, come in =SOMMA(A1#), per usare l'intero blocco così com'è ora, da questo foglio o da un altro (=SOMMA(Foglio1!A1#)). Se A1 non va in spill, A1# dà #RIF!.

Uno spill viene salvato con il file come matrice dinamica, quindi il programma di origine del file lo mostra anch'esso come spill. Salvando nelle forme più vecchie .xls o .ods i valori vengono conservati, e la formula come formula matriciale sul blocco, dove la forma più vecchia ha le funzioni che usa. Verifica compatibilità segnala queste formule.

## Assegnare nomi ai valori dentro una formula

**LET** assegna un nome a un valore nel mezzo di una formula, così lo scrivi una volta e lo riusi. Le coppie di nome e valore vengono prima, e il calcolo per ultimo. In =LET(prezzo; B2; imposta; prezzo*0,08; prezzo+imposta) il nome **prezzo** indica il valore in B2, **imposta** viene calcolata a partire da esso, e l'ultima parte le somma. Un nome può usare i nomi che lo precedono, e un LET può contenere fino a 126 coppie. Un nome funziona solo dentro il proprio LET, e un LET annidato in un altro può riutilizzare un nome. Un nome può indicare anche un intervallo di celle, come in =LET(righe; B2:B9; SOMMA(righe)/CONTA.NUMERI(righe)).

## Chiedere informazioni su una cella o sulla cartella di lavoro

**CELLA** restituisce informazioni su una cella. Il primo argomento indica quali: "address" (indirizzo), "row" (riga), "col" (colonna), "contents" (contenuto), "type" (tipo), "format" (come vengono mostrati i numeri, come "F2" per due decimali, "P0" per una percentuale, o "D1" per una data), "color" (1 quando i numeri negativi vengono mostrati in colore), "parentheses" (1 quando i numeri positivi vengono mostrati tra parentesi), "prefix" (il segno di un testo allineato a sinistra, a destra o al centro), "protect" (1 quando la cella è bloccata), "width" (la larghezza della colonna in caratteri) e "filename" (la cartella, il file e il foglio, come /home/io/[Budget.xlsx]Foglio1; vuoto finché la cartella di lavoro non viene salvata). **INFO** fornisce informazioni sul programma e sulla cartella di lavoro: "directory", "numfile" (quanti fogli sono visualizzati), "origin" (origine), "osversion" (versione del sistema), "recalc" (Automatico o Manuale), "release" e "system" (sistema).

## Estrarre valori da XML

**FILTRO.XML** prende un testo che contiene XML e un XPath, cioè un modo breve per indicare quali parti ti servono, e restituisce i valori che trova. =FILTRO.XML(A1; "//item/price") restituisce ogni prezzo del testo in A1, uno sotto l'altro. Usa "/shop/item[2]/name" per il secondo elemento, "//item[@id='b']/name" per l'elemento il cui id è b, oppure "count(//item)" per sapere quanti sono. Se il testo non è XML valido, o il percorso non è valido, il risultato è #VALORE!. Se nulla corrisponde, è #N/D. L'XML che contiene una dichiarazione di tipo di documento viene rifiutato.
