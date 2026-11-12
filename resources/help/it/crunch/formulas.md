# Come vengono mostrate e calcolate le formule

<!-- keywords: #SPILL!, spill, spilled, matrice dinamica, intervallo di spill, A1#, errore successivo, errore precedente, scorri gli errori, traccia errore, calcola prima del salvataggio, formula, formule, calcola, calcolo, ricalcola, funzione, funzioni, valore, risultato, memorizzato, F9, circolare, mostra formule, barra della formula, errore, punta, puntamento, fai clic su una cella, riferimento, F4, verifica errori, ignora errore, valuta formula, passo, finestra di controllo, controllo, LET, assegna un nome a un valore, funzione CELL, funzione INFO, FILTERXML, xml, xpath -->

Una **formula** è una cella che calcola il proprio valore da altre celle, ad esempio =SOMMA(B3:B8). La cella mostra il valore, e la **barra della formula** mostra la formula.

## Che cosa fa JeffCrunch con le formule

Quando apri una cartella di lavoro, il file contiene l'ultimo valore di ogni formula, così il foglio può essere mostrato subito. Poi JeffCrunch calcola di nuovo ogni formula, qualche alla volta, in modo che la finestra resti veloce. La barra di stato mostra **Calcolo in corso** con una percentuale mentre lavora. Al termine, le celle contengono i risultati di JeffCrunch. Per calcolare di nuovo tutto, fai clic su **Ricalcola tutto** nella scheda **Formule**, oppure premi **F9**.

## Quando JeffCrunch non riesce a calcolare una formula

JeffCrunch ha più di 400 funzioni, e il loro numero continua a crescere. Una formula che usa una funzione che JeffCrunch non ha ancora mantiene il valore che il file memorizza per essa. Lo stesso vale per una formula che fa riferimento a un'altra cartella di lavoro. Le celle che dipendono da queste vengono calcolate a partire dai valori memorizzati.

## Funzioni che richiedono qualche precisazione

- **Obbligazioni e prestiti.** PREZZO, RENDITA, DURATA, PREZZO.PRIMO.DISP e le altre funzioni sulle obbligazioni contano i giorni secondo la **base** che indichi (0 significa 30 giorni al mese in un anno di 360 giorni, 1 significa i giorni effettivi, e così via). RENDITA, RENDITA.PRIMO.DISP, RENDITA.ULTIMO.DISP e TASSO trovano il risultato provando dei valori finché il prezzo o la rata coincidono. Si fermano quando il risultato è corretto fino a circa 15 cifre, quindi le ultime cifre possono differire da quelle di un altro programma.
- **Distribuzioni e test.** Le funzioni che terminano con .DISTR, .INV, .TEST e i nomi precedenti come DISTRIB.NORM e DISTRIB.T danno le stesse risposte: un nome precedente viene mantenuto affinché i vecchi file continuino a funzionare. Anche le funzioni *.INV trovano il risultato provando dei valori.
- **Elenchi che riempiono più celle.** ORDINA, ORDINA.PER, FILTRO, UNICI, SEQUENZA, MATR.CASUALE, CERCA.X, MATR.MULT, MATR.INVERSA e REGR.LIN restituiscono un blocco di valori. Scrivine una in una sola cella e il blocco si estende sulle celle sotto e accanto ad essa (vedi **Formule che riempiono le celle accanto**), oppure inseriscila come formula matriciale (**Ctrl+Maiusc+Invio**) sulle celle che deve riempire.
- **Testi in altri alfabeti.** LUNGHEZZAB, SINISTRAB, STRINGAB, DESTRAB, TROVA.B, RICERCAB e SOSTITUISCIB contano ogni carattere come uno, come accade su un computer non configurato per il testo a doppio byte. FONETICO restituisce il testo della cella, perché JeffCrunch non conserva le letture del testo giapponese.
- **CONVERTI.** Le unità si scrivono come testo, con maiuscole e minuscole come mostrate nell'elenco della funzione: "kg", "lbm", "mi", "F". Un prefisso metrico può precedere un'unità che lo accetta, come "km" o "mL".

## MOD: il resto

**RESTO(n; d)** restituisce ciò che rimane di **n** dopo aver tolto il maggior numero di copie intere di **d**: è n - d * INT(n / d). Esempio: **=RESTO(10; 3)** è 1, e **=RESTO(-10; 3)** è 2, perché INT(-10 / 3) è -4 e -10 - 3 * -4 fa 2. Il risultato ha il segno di **d**. Se **d** è 0, il risultato è #DIV/0!.

## Mostrare le formule

Fai clic su **Mostra formule** nella scheda **Formule**, oppure premi **Ctrl+'** (su Mac, il tasto Control e non il tasto Command), per vedere la formula di ogni cella al posto del suo valore. Le colonne che contengono formule vengono allargate, fino a un limite, in modo che le formule si vedano per intero; tornano alla larghezza originale quando disattivi la funzione, e le larghezze nel tuo file non vengono modificate.

## Modificare le formule

Per modificare una formula, seleziona la sua cella, premi **F2**, modificala e premi **Invio**. Puoi modificarla anche nella barra della formula. Quando cambi una cella, le formule che dipendono da essa vengono calcolate di nuovo in background. Vedi [Digitare nelle celle](edit-cells).

## Scrivere una formula

Digita **=** e le prime lettere di una funzione: compare un breve elenco delle funzioni e dei [nomi](names) che iniziano in quel modo, con una frase su ciascuno. Premi **Tab** per prendere quella evidenziata. Usa le frecce **Giù** e **Su** per evidenziarne un'altra, poi **Tab** o **Invio**. **Esc** chiude l'elenco. Quando sei dentro le parentesi di una funzione, i suoi argomenti compaiono sotto la cella, con quello che stai digitando in grassetto. L'elenco e gli argomenti sono gli stessi che mostra **Scegli una funzione**. Gli argomenti sono separati da virgole, oppure da punti e virgola dove la tua lingua usa la virgola come separatore decimale.

## Puntare alle celle

Invece di digitare l'indirizzo di una cella, puoi puntarla. Mentre digiti una formula, metti il cursore dove può andare una cella (subito dopo **=**, un operatore come **+**, una parentesi aperta o una virgola) e fai clic sulla cella. Il suo indirizzo viene scritto per te. Trascina sulle celle per scrivere un intervallo come B2:D5. Fai di nuovo clic prima di digitare qualcos'altro, e la nuova cella sostituisce quella appena scritta. Tieni premuto **Ctrl** e fai clic per aggiungere un altro indirizzo dopo una virgola. Fai clic su una lettera di colonna per scrivere un'intera colonna come A:A, o su un numero di riga per scrivere un'intera riga come 3:3, e trascina lungo le lettere o i numeri per B:D o 2:5.

Funzionano anche i tasti freccia. Premi una freccia e un puntatore si sposta lontano dalla cella in cui stai digitando, e il suo indirizzo viene scritto; premi altre frecce per proseguire, oppure tieni premuto **Maiusc** per estendere il puntatore su un intervallo. La barra di stato mostra **Punta** mentre le frecce puntano, **Invio** mentre digiti, e **Modifica** mentre le frecce spostano il cursore nel testo. Premi **F2** per passare dal puntamento alla modifica del testo.

Per puntare a una cella di un altro foglio, fai clic sulla scheda di quel foglio mentre il cursore si trova dove può andare una cella, poi fai clic sulla cella. Il nome del foglio viene scritto insieme all'indirizzo, tra apici quando serve, come in 'Il mio foglio'!B3. **Invio** ti riporta al foglio da cui sei partito e inserisce la formula nella sua cella. **Esc** annulla tutto, incluso il cambio di foglio.

Puoi anche trascinare il bordo di un intervallo evidenziato sul foglio per spostare quell'indirizzo, oppure trascinare il suo quadratino d'angolo per allungarlo; la formula cambia mentre trascini.

Premi **F4** con il cursore dentro o subito dopo un indirizzo per bloccare alcune parti con il segno del dollaro: A1 diventa $A$1, poi A$1, poi $A1, e poi di nuovo A1.

Ogni indirizzo della formula è colorato nel testo, e le celle che nomina sono evidenziate sul foglio con lo stesso colore, così puoi vedere a colpo d'occhio che cosa legge la formula. Un clic dopo un indirizzo completo, come =A1, termina l'inserimento e seleziona la cella su cui hai fatto clic, come accade per qualsiasi altro testo.

## Tabelle dei dati

Una tabella dei dati (tabella di simulazione) mostra il risultato di una formula per molti valori di una o due celle. Apri una cartella di lavoro che ne contiene una, e JeffCrunch calcola le sue celle: per una tabella con due variabili, la formula si trova nella cella in alto a sinistra della tabella, i valori da provare in orizzontale vanno nella cella di input della riga, e quelli in verticale nella cella di input della colonna. Per una tabella con una sola variabile, i valori vanno in una colonna a sinistra (e le formule si trovano nella riga sopra di essa) oppure in una riga sopra di essa (e le formule si trovano nella colonna a sinistra). Ogni cella della tabella è il risultato della formula, e di ogni formula che dipende dalla cella di input, con l'input impostato a quel valore; nient'altro nella cartella di lavoro viene modificato. In modalità **Automatica** le tabelle vengono calcolate insieme alle formule, e le formule che leggono le celle di una tabella ne seguono i valori. La creazione di una nuova tabella dei dati non è ancora disponibile. Una tabella che non può essere calcolata (una formula che dipende dall'input usa una funzione che JeffCrunch non ha, oppure cambia da sola, come CASUALE o ORA) mantiene i valori memorizzati nel file.

## Calcolare manualmente

Fai clic su **Modalità di calcolo** nella scheda **Formule** per scegliere come la cartella di lavoro calcola le sue formule:

- **Automatica** le calcola man mano che modifichi le celle.
- **Automatica, tabelle manuali** fa lo stesso, ma lascia invariate le celle delle tabelle dei dati finché non premi **F9**.
- **Manuale** non calcola nulla, comprese le tabelle dei dati, finché non lo chiedi. La barra di stato mostra **Non aggiornato** dopo una modifica. Premi **F9** (**Ricalcola tutto**) per calcolare tutto, oppure **Maiusc+F9** (**Ricalcola questo foglio**) per calcolare le formule del foglio corrente e tutto ciò che era in attesa. Una cartella di lavoro impostata su Manuale si apre con i risultati contenuti nel file. **Calcola prima del salvataggio**, anch'essa nel menu **Modalità di calcolo**, è selezionata per impostazione predefinita: quando salvi una cartella di lavoro manuale non aggiornata, le formule vengono calcolate prima, così il file contiene numeri aggiornati. Disattivala per salvare i numeri così come sono.

La scelta viene salvata con la cartella di lavoro, ed è un passaggio che puoi annullare.

## Formule che leggono se stesse

Una formula che legge il proprio risultato, direttamente o attraverso altre celle, non può essere calcolata nel modo consueto. JeffCrunch te lo segnala una volta, indica le celle e le evidenzia sul foglio; esse mantengono i valori che avevano. Fai clic su **Riferimenti circolari** nella scheda **Formule** per l'elenco, e fai doppio clic su una cella dell'elenco per andarci.

Alcuni modelli richiedono di proposito un tale ciclo. Scegli **Calcolo iterativo delle formule circolari** nel menu **Modalità di calcolo**, e le formule vengono calcolate più volte, leggendo ciascuna gli ultimi valori delle altre, finché nessun valore cambia più della quantità che hai indicato oppure non viene raggiunto il numero di iterazioni consentito.

## Vedere che cosa legge una formula

Seleziona una cella e fai clic su **Mostra celle che alimentano questa cella** per disegnare frecce dalle celle che la sua formula legge. Fai di nuovo clic per risalire di un passo. **Mostra celle che usano questa cella** disegna frecce verso le formule che leggono la cella, e ogni clic va avanti di un passo. Una piccola pagina rappresenta una cella di un altro foglio. Vengono disegnate al massimo 2.000 frecce, e la barra di stato lo segnala quando ce ne sono di più. **Rimuovi frecce** le cancella, e le cancella anche qualsiasi modifica alla cartella di lavoro.

Quando una cella mostra un errore come #DIV/0!, selezionala e fai clic su **Traccia errore**. Vengono disegnate frecce verso le celle che legge e che mostrano anch'esse un errore. Fai di nuovo clic per risalire di un passo. Quando nessuna delle celle che legge mostra un errore, la barra di stato indica che questa cella è la causa dell'errore, ed è quella da correggere.

Per scorrere tutti gli errori del foglio, fai clic su **Errore successivo** o **Errore precedente**. Le celle che mostrano un errore vengono visitate una riga dopo l'altra, e dopo l'ultima si ricomincia dalla prima. Ogni cella viene selezionata, e le frecce vengono disegnate fino alla cella che causa l'errore. La barra di stato indica di quale errore si tratta e quanti sono in totale.

## Verifica errori

Scegli **Verifica errori** nel gruppo **Controllo** della scheda Formule per cercare nel foglio le celle che potrebbero essere sbagliate. Cerca due cose: una formula che restituisce un errore (come #DIV/0!), e un numero memorizzato come testo, che le formule potrebbero ignorare.

La finestra di dialogo mostra una cella alla volta, va a quella cella nel foglio e spiega in una frase che cosa non va. **Successivo** e **Precedente** scorrono le celle. Un piccolo triangolo verde nell'angolo in alto a sinistra di una cella la segnala nel foglio. **Ignora errore** toglie la cella dall'elenco, rimuove il suo triangolo, e la scelta viene salvata nel file. Un passo di **Annulla** la ripristina. Se non c'è nulla da correggere, viene segnalato.

## Valuta formula

Usalo per vedere come una formula arriva al suo valore. Seleziona una cella che contiene una formula e scegli **Valuta formula**. La formula viene mostrata con la parte successiva sottolineata. Premi **Valuta** per calcolare quella parte; il suo valore prende il suo posto. Le parti vengono prese in quest'ordine: ogni singola cella da sinistra, poi ogni funzione con le sue parentesi, partendo da quella più interna, e infine ciò che resta. **Riavvia** ricomincia da capo. Se una parte non può essere calcolata, la finestra di dialogo ne indica il motivo e lascia la formula com'era.

## Finestra di controllo

La **Finestra di controllo** è una piccola finestra che resta visibile mentre lavori. Elenca le celle che vuoi tenere sotto controllo, con il loro foglio, la cella, il valore e la formula, anche quando si trovano su un altro foglio. Seleziona delle celle e premi **Aggiungi controllo**. Seleziona delle righe nell'elenco e premi **Elimina controllo**. Fai doppio clic su una riga per andare alla cella. I valori cambiano quando il foglio viene ricalcolato. L'elenco viene conservato finché la cartella di lavoro è aperta.

## Formule che riempiono le celle accanto

Quando una formula restituisce un blocco di valori, il blocco riempie le celle sotto e accanto alla cella in cui l'hai digitata. Questo si chiama spill (traboccamento). Prova =SEQUENZA(3) in A1: A1 mostra 1, e A2 e A3 mostrano 2 e 3. Una formula trabocca quando richiama una funzione che restituisce un blocco (ORDINA, ORDINA.PER, FILTRO, UNICI, SEQUENZA, MATR.CASUALE, MATR.TRASPOSTA, CERCA.X o MATR.MULT), scrive un blocco tra parentesi graffe, usa un intervallo di spill (più sotto), oppure esegue operazioni aritmetiche su un intervallo, come in =A1:A3*2.

- **Le celle che riempie non hanno una formula propria.** Mostrano i valori. Quando ne selezioni una, la barra della formula mostra in grigio la formula della cella che la riempie. Premere Invio su di essa così com'è non cambia nulla. Per modificare il blocco, modifica prima di tutto quella prima cella.
- **Il blocco segue il risultato.** Se la formula restituisce più o meno valori dopo che è cambiato qualcosa che legge, il blocco si allarga o si restringe, e le formule che leggono quelle celle vengono calcolate di nuovo.
- **Una cella di ostacolo dà #SPILL!.** Se una qualsiasi cella in cui il blocco dovrebbe andare contiene un valore o una formula, la prima cella mostra **#SPILL!** e non viene riempito nulla. Svuota la cella di ostacolo e il blocco si riempie subito. Digitare in una cella che il blocco riempie dà lo stesso risultato: la prima cella mostra #SPILL!.
- **Un blocco ha un limite di dimensione.** Un blocco più grande del foglio, o di più di 200.000 celle, dà #SPILL!.
- **A1# è l'intero blocco.** Scrivi la cella che contiene la formula seguita da un **#**, come in =SOMMA(A1#), per usare l'intero blocco così com'è ora, da questo foglio o da un altro (=SOMMA(Foglio1!A1#)). Se A1 non trabocca, A1# dà #RIF!.

Lo spill viene salvato nel file come matrice dinamica, così il programma da cui il file proviene lo mostra anch'esso come spill. Il salvataggio nei formati più vecchi .xls o .ods mantiene i valori, e la formula come formula matriciale sul blocco, dove il formato più vecchio ha le funzioni che usa. Verifica compatibilità segnala queste formule.

## Dare nomi ai valori dentro una formula

**LET** assegna un nome a un valore nel mezzo di una formula, così lo scrivi una volta e lo usi di nuovo. Le coppie di nome e valore vengono prima, e il calcolo per ultimo. In =LET(prezzo; B2; imposta; prezzo*0,08; prezzo+imposta) il nome **prezzo** indica il valore in B2, **imposta** viene calcolato a partire da esso, e l'ultima parte li somma. Un nome può usare i nomi che lo precedono, e un LET può contenere fino a 126 coppie. Un nome funziona solo dentro il proprio LET, e un LET annidato in un altro può riutilizzare un nome. Un nome può indicare anche un intervallo di celle, come in =LET(righe; B2:B9; SOMMA(righe)/CONTA.NUMERI(righe)).

## Chiedere informazioni su una cella o sulla cartella di lavoro

**CELLA** fornisce informazioni su una cella. Il suo primo argomento indica quali: "indirizzo", "riga", "col", "contenuto", "tipo", "formato" (come vengono mostrati i numeri, ad esempio "F2" per due decimali, "P0" per una percentuale, o "D1" per una data), "colore" (1 quando i numeri negativi sono mostrati a colori), "parentesi" (1 quando i numeri positivi sono mostrati tra parentesi), "prefisso" (il segno di un testo allineato a sinistra, a destra o al centro), "protetta" (1 quando la cella è bloccata), "larghezza" (la larghezza della colonna in caratteri), e "nomefile" (la cartella, il file e il foglio, come /home/io/[Bilancio.xlsx]Foglio1; vuoto finché la cartella di lavoro non viene salvata). **INFO** fornisce informazioni sul programma e sulla cartella di lavoro: "directory", "numfile" (quanti fogli sono visualizzati), "origine", "versionesistema", "calc" (Automatico o Manuale), "versione" e "sistema".

## Estrarre valori da XML

**FILTRO.XML** prende un testo che contiene XML e un XPath, cioè un modo breve per indicare quali parti vuoi, e restituisce i valori che trova. =FILTRO.XML(A1; "//articolo/prezzo") restituisce ogni prezzo del testo in A1, uno sotto l'altro. Usa "/negozio/articolo[2]/nome" per il secondo articolo, "//articolo[@id='b']/nome" per l'articolo il cui id è b, oppure "count(//articolo)" per sapere quanti sono. Se il testo non è XML valido, o il percorso non è valido, il risultato è #VALORE!. Se nulla corrisponde, è #N/D. Non viene accettato XML che abbia una dichiarazione di tipo di documento.
