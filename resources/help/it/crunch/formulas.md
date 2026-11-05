# Come vengono mostrate e calcolate le formule

<!-- keywords: formula, formule, calcolo, calcolare, ricalcolare, funzione, funzioni, valore, risultato, memorizzato, F9, circolare, mostra formule, barra della formula, errore, puntamento, puntare, fare clic su una cella, riferimento, F4, controllo errori, ignora errore, valuta formula, passo, controllo, finestra di controllo, LET, assegnare un nome a un valore, funzione cella, funzione info, FILTRO.XML, xml, xpath -->

Una **formula** è una cella che calcola il proprio valore da altre celle, ad esempio =SOMMA(B3:B8). La cella mostra il valore, mentre la **barra della formula** mostra la formula.

## Che cosa fa JeffCrunch con le formule

Quando apri una cartella di lavoro, il file contiene l'ultimo valore di ogni formula, così il foglio può essere mostrato subito. Poi JeffCrunch calcola di nuovo ogni formula, qualche alla volta, in modo che la finestra resti veloce. La barra di stato indica **Calcolo in corso** con una percentuale mentre lavora. Al termine, le celle contengono i risultati di JeffCrunch. Per calcolare di nuovo tutto, fai clic su **Ricalcola tutto** nella scheda **Formule** oppure premi **F9**.

## Quando JeffCrunch non può calcolare una formula

JeffCrunch dispone di più di 400 funzioni e il loro numero continua a crescere. Una formula che usa una funzione non ancora disponibile in JeffCrunch conserva il valore memorizzato nel file. Lo stesso vale per una formula che fa riferimento a un'altra cartella di lavoro. Le celle che dipendono da queste formule vengono calcolate a partire dai valori memorizzati.

## Funzioni che richiedono alcune precisazioni

- **Obbligazioni e prestiti.** PREZZO, RENDIMENTO, DURATA, PREZZO.PRIMO.IRR e le altre funzioni sulle obbligazioni contano i giorni secondo la **base** indicata (0 significa 30 giorni al mese in un anno di 360 giorni, 1 significa i giorni effettivi, e così via). RENDIMENTO, RENDITA.PRIMO.IRR, RENDITA.ULTIMO.IRR e TASSO trovano il risultato provando dei valori finché il prezzo o la rata non corrispondono. Si fermano quando il risultato è esatto fino a circa 15 cifre, quindi le ultime cifre possono differire da quelle di un altro programma.
- **Distribuzioni e test.** Le funzioni che terminano con .DIST, .INV, .TEST e i nomi precedenti come DISTRIB.NORM e DISTRIB.T danno gli stessi risultati: un nome precedente viene mantenuto affinché i file vecchi continuino a funzionare. Anche le funzioni *.INV trovano il risultato provando dei valori.
- **Elenchi che riempiono più celle.** ORDINA, ORDINA.PER, FILTRO, UNICO, SEQUENZA, MATR.CASUALE, CERCA.X, MATR.PRODOTTO, MATR.INVERSA e REGR.LIN restituiscono un intervallo di valori. Inseriscile come formula matriciale (**Ctrl+Maiusc+Invio**) sulle celle che devono riempire. Non è ancora possibile far riempire alla formula le celle vicine.
- **Testi in altri alfabeti.** LUNGHEZZAB, SINISTRAB, STRINGAB, DESTRAB, TROVA.B, RICERCA.B e SOSTITUISCIB contano ogni carattere come uno, come accade su un computer non configurato per il testo a doppio byte. FONETICO restituisce il testo della cella, perché JeffCrunch non conserva la lettura del testo giapponese.
- **CONVERTI.** Le unità si scrivono come testo, con le maiuscole e le minuscole indicate nell'elenco della funzione: "kg", "lbm", "mi", "F". Un prefisso metrico può precedere un'unità che lo ammette, ad esempio "km" o "mL".

## MOD: il resto

**RESTO(n; d)** restituisce ciò che rimane di **n** dopo aver tolto il maggior numero intero di volte di **d**: è n - d * INT(n / d). Esempio: **=RESTO(10; 3)** dà 1, e **=RESTO(-10; 3)** dà 2, perché INT(-10 / 3) è -4 e -10 - 3 * -4 è 2. Il risultato ha il segno di **d**. Se **d** è 0, il risultato è #DIV/0!.

## Mostrare le formule

Fai clic su **Mostra formule** nella scheda **Formule**, oppure premi **Ctrl+`** (su un Mac, il tasto Control e non il tasto Command), per vedere la formula di ogni cella al posto del suo valore. Le colonne che contengono formule vengono allargate, fino a un certo limite, in modo che le formule si leggano per intero; tornano alla larghezza originale quando disattivi la funzione, e le larghezze nel tuo file non vengono modificate.

## Modificare le formule

Per modificare una formula, seleziona la sua cella, premi **F2**, modificala e premi **Invio**. Puoi anche modificarla nella barra della formula. Quando cambi una cella, le formule che ne dipendono vengono ricalcolate in background. Vedi [Digitare nelle celle](edit-cells).

## Scrivere una formula

Digita **=** e le prime lettere di una funzione: compare un breve elenco delle funzioni e dei [nomi](names) che iniziano in quel modo, con una frase per ciascuno. Premi **Tab** per scegliere quella evidenziata. Usa le frecce **Giù** e **Su** per evidenziarne un'altra, poi **Tab** o **Invio**. **Esc** chiude l'elenco. Quando sei dentro le parentesi di una funzione, i suoi argomenti compaiono sotto la cella, con quello che stai digitando in grassetto. L'elenco e gli argomenti sono gli stessi che mostra **Scegli una funzione**. Gli argomenti sono separati da virgole, oppure da punti e virgola nelle impostazioni in cui la virgola indica i decimali.

## Puntare alle celle

Invece di digitare l'indirizzo di una cella, puoi indicarla. Mentre scrivi una formula, metti il cursore dove può comparire una cella (subito dopo **=**, dopo un operatore come **+**, una parentesi aperta o una virgola) e fai clic sulla cella. Il suo indirizzo viene scritto per te. Trascina sulle celle per scrivere un intervallo come B2:D5. Fai di nuovo clic prima di digitare altro, e la nuova cella sostituisce quella appena scritta. Tieni premuto **Ctrl** e fai clic per aggiungere un altro indirizzo dopo una virgola. Fai clic su una lettera di colonna per scrivere un'intera colonna, come A:A, o su un numero di riga per scrivere un'intera riga, come 3:3, e trascina lungo le lettere o i numeri per B:D o 2:5.

Funzionano anche i tasti freccia. Premi una freccia e un puntatore si sposta allontanandosi dalla cella in cui stai scrivendo, e il suo indirizzo viene scritto; premi altre frecce per proseguire, oppure tieni premuto **Maiusc** per estendere il puntatore su un intervallo. La barra di stato indica **Punta** mentre i tasti freccia puntano, **Invio** mentre scrivi, e **Modifica** mentre i tasti freccia spostano il cursore nel testo. Premi **F2** per passare dal puntamento alla modifica del testo.

Per puntare a una cella di un altro foglio, fai clic sulla linguetta di quel foglio mentre il cursore si trova dove può comparire una cella, e poi fai clic sulla cella. Il nome del foglio viene scritto insieme all'indirizzo, tra apici quando necessario, come in 'Il mio foglio'!B3. **Invio** ti riporta al foglio di partenza e inserisce la formula nella sua cella. **Esc** annulla tutto, compreso il cambio di foglio.

Puoi anche trascinare il bordo di un intervallo evidenziato sul foglio per spostare quell'indirizzo, oppure trascinare il quadratino nell'angolo per allargarlo: la formula cambia mentre trascini.

Premi **F4** con il cursore dentro o subito dopo un indirizzo per bloccarne parti con il segno del dollaro: A1 diventa $A$1, poi A$1, poi $A1, e infine di nuovo A1.

Ogni indirizzo nella formula è colorato nel testo, e le celle che nomina sono evidenziate sul foglio con lo stesso colore, così puoi vedere a colpo d'occhio che cosa legge la formula. Un clic subito dopo un indirizzo intero, come in =A1, conclude l'inserimento e seleziona la cella su cui hai fatto clic, come accade per qualsiasi altro testo.

## Tabelle dei dati

Una tabella dei dati (tabella di simulazione) mostra il risultato di una formula per molti valori di una o due celle. Quando apri una cartella di lavoro che ne contiene una, JeffCrunch calcola le sue celle: per una tabella con due variabili, la formula si trova nella cella in alto a sinistra della tabella, i valori da provare in orizzontale vanno nella cella di input di riga, e quelli in verticale nella cella di input di colonna. Per una tabella con una sola variabile, i valori vanno in verticale nella colonna a sinistra (e le formule si trovano nella riga sopra) oppure in orizzontale nella riga sopra (e le formule si trovano nella colonna a sinistra). Ogni cella della tabella è il risultato della formula, e di ogni formula che dipende dalla cella di input, con l'input impostato a quel valore; nient'altro della cartella di lavoro viene modificato. In modalità **Automatica** le tabelle vengono calcolate insieme alle formule, e le formule che leggono le celle di una tabella ne seguono i valori. La creazione di una nuova tabella dei dati non è ancora disponibile. Una tabella che non può essere calcolata (una formula che dipende dall'input usa una funzione non disponibile in JeffCrunch, oppure cambia da sola, come CASUALE o ADESSO) conserva i valori del suo file.

## Calcolo manuale

Fai clic su **Modalità di calcolo** nella scheda **Formule** per scegliere come la cartella di lavoro calcola le formule:

- **Automatica** calcola le formule man mano che modifichi le celle.
- **Automatica, tabelle manuali** fa lo stesso, ma lascia invariate le celle delle tabelle dei dati finché non premi **F9**.
- **Manuale** non calcola nulla, comprese le tabelle dei dati, finché non lo chiedi. La barra di stato indica **Non aggiornato** dopo una modifica. Premi **F9** (**Ricalcola tutto**) per calcolare tutto, oppure **Maiusc+F9** (**Ricalcola questo foglio**) per calcolare le formule del foglio in uso e quanto altro era in attesa. Una cartella di lavoro impostata su Manuale si apre con i risultati contenuti nel suo file.

La scelta viene salvata con la cartella di lavoro ed è un'operazione che puoi annullare.

## Formule che leggono se stesse

Una formula che legge il proprio risultato, direttamente o attraverso altre celle, non può essere calcolata nel modo consueto. JeffCrunch lo segnala una volta, indica le celle e le evidenzia sul foglio; esse mantengono i valori che avevano. Fai clic su **Riferimenti circolari** nella scheda **Formule** per vedere l'elenco, e fai doppio clic su una cella dell'elenco per raggiungerla.

Alcuni modelli richiedono di proposito un circolo di questo tipo. Scegli **Calcolo iterativo delle formule circolari** nel menu **Modalità di calcolo**: le formule vengono calcolate più volte, leggendo ciascuna gli ultimi valori delle altre, finché nessun valore cambia più dell'importo indicato oppure non si raggiunge il numero massimo di iterazioni impostato.

## Vedere che cosa legge una formula

Seleziona una cella e fai clic su **Mostra celle che alimentano questa cella** per disegnare frecce dalle celle che la sua formula legge. Fai di nuovo clic per risalire di un passo. **Mostra celle che usano questa cella** disegna frecce verso le formule che leggono la cella, e ogni clic va avanti di un passo. Una piccola pagina rappresenta una cella di un altro foglio. Vengono disegnate al massimo 2.000 frecce, e la barra di stato indica quando ce ne sono di più. **Rimuovi frecce** le cancella, e le cancella anche qualsiasi modifica alla cartella di lavoro.

## Controllo errori

Scegli **Controllo errori** nel gruppo **Verifica** della scheda Formule per cercare nel foglio le celle che potrebbero essere errate. Cerca due cose: una formula che restituisce un errore (come #DIV/0!), e un numero memorizzato come testo, che le formule potrebbero ignorare.

La finestra di dialogo mostra una cella alla volta, va a quella cella nel foglio e spiega in una frase qual è il problema. **Successivo** e **Precedente** scorrono le celle. Un piccolo triangolo verde nell'angolo in alto a sinistra di una cella la segnala nel foglio. **Ignora errore** toglie la cella dall'elenco, rimuove il suo triangolo e salva la scelta nel file. Un passo di **Annulla** la ripristina. Se non c'è nulla di sbagliato, lo si indica.

## Valuta formula

Usala per vedere come una formula arriva al proprio valore. Seleziona una cella che contiene una formula e scegli **Valuta formula**. La formula viene mostrata con la parte successiva sottolineata. Premi **Valuta** per calcolare quella parte; il suo valore prende il suo posto. Le parti vengono considerate in quest'ordine: prima ogni singola cella da sinistra, poi ogni funzione con le sue parentesi, partendo dalla più interna, e infine ciò che resta. **Riavvia** ricomincia dall'inizio. Se una parte non può essere calcolata, la finestra di dialogo ne spiega il motivo e lascia la formula com'era.

## Finestra di controllo

La **Finestra di controllo** è una piccola finestra che resta visibile mentre lavori. Elenca le celle che vuoi tenere sotto controllo, con il loro foglio, la cella, il valore e la formula, anche quando si trovano su un altro foglio. Seleziona le celle e premi **Aggiungi controllo**. Seleziona le righe dell'elenco e premi **Elimina controllo**. Fai doppio clic su una riga per andare alla cella. I valori cambiano quando il foglio viene ricalcolato. L'elenco resta disponibile finché la cartella di lavoro è aperta.

## Assegnare nomi ai valori dentro una formula

**LET** assegna un nome a un valore nel mezzo di una formula, così lo scrivi una volta sola e lo riutilizzi. Le coppie di nome e valore vengono prima, e il calcolo finale per ultimo. In =LET(prezzo; B2; imposta; prezzo*0,08; prezzo+imposta) il nome **prezzo** rappresenta il valore di B2, **imposta** viene calcolata a partire da esso, e l'ultima parte le somma. Un nome può usare i nomi che lo precedono. Un nome vale solo all'interno del proprio LET, e un LET contenuto in un altro può riutilizzare lo stesso nome. Un nome può rappresentare anche un intervallo di celle, come in =LET(righe; B2:B9; SOMMA(righe)/CONTA.NUMERI(righe)).

## Chiedere informazioni su una cella o sulla cartella di lavoro

**INFO.CELLA** fornisce informazioni su una cella. Il suo primo argomento indica quali: "indirizzo", "riga", "col", "contenuto", "tipo", "formato" (come vengono mostrati i numeri, ad esempio "F2" per due decimali, "P0" per una percentuale, o "D1" per una data), "colore" (1 quando i numeri negativi sono mostrati in colore), "parentesi" (1 quando i numeri positivi sono mostrati tra parentesi), "prefisso" (il segno di un testo allineato a sinistra, a destra o al centro), "protetta" (1 quando la cella è bloccata), "larghezza" (la larghezza della colonna in caratteri) e "nomefile" (la cartella, il file e il foglio, ad esempio /home/me/[Budget.xlsx]Sheet1; vuoto finché la cartella di lavoro non viene salvata). **INFO** fornisce informazioni sul programma e sulla cartella di lavoro: "directory", "numfile" (quanti fogli sono visualizzati), "origine", "versionesO", "recalc" (Automatico o Manuale), "release" e "sistema".

## Estrarre valori da XML

**FILTRO.XML** prende un testo che contiene XML e un percorso XPath, cioè un modo breve per indicare quali parti ti servono, e restituisce i valori che trova. =FILTRO.XML(A1; "//articolo/prezzo") restituisce tutti i prezzi del testo in A1, uno sotto l'altro. Usa "/negozio/articolo[2]/nome" per il secondo articolo, "//articolo[@id='b']/nome" per l'articolo il cui id è b, oppure "count(//articolo)" per sapere quanti sono. Se il testo non è XML valido, o il percorso non è valido, il risultato è #VALORE!. Se non corrisponde nulla, il risultato è #N/D. L'XML che contiene una dichiarazione di tipo di documento viene rifiutato.
