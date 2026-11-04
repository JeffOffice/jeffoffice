# Come vengono mostrate e calcolate le formule
<!-- keywords: formula, formule, calcolo, calcolare, ricalcolare, funzione, funzioni, valore, risultato, memorizzato, F9, circolare, mostra formule, barra della formula, errore, punto, puntamento, clic su una cella, riferimento, F4, calcolo, somma -->

Una **formula** è una cella che ricava il proprio valore da altre celle, come =SOMMA(B3:B8). La cella mostra il valore e la **barra della formula** mostra la formula.

## Che cosa fa JeffCrunch con le formule

Quando apri una cartella di lavoro, il file contiene l'ultimo valore di ogni formula, così il foglio può essere mostrato subito. Poi JeffCrunch calcola di nuovo ogni formula, qualche formula alla volta, in modo che la finestra resti rapida. La barra di stato indica **Calcolo in corso** con una percentuale mentre lavora. Al termine, le celle contengono i risultati di JeffCrunch. Per calcolare di nuovo tutto, fai clic su **Ricalcola tutto** nella scheda **Formule**, oppure premi **F9**.

## Quando JeffCrunch non riesce a calcolare una formula

JeffCrunch dispone di più di 400 funzioni, e il loro numero è in crescita. Una formula che usa una funzione non ancora disponibile in JeffCrunch conserva il valore memorizzato nel file. Lo stesso vale per una formula che fa riferimento a un'altra cartella di lavoro. Le celle che dipendono da queste formule vengono calcolate a partire dai valori memorizzati.

## Funzioni che richiedono alcune note

- **Obbligazioni e prestiti.** PREZZO, REND, DURATA, PREZZO.PRIMO.DIS e le altre funzioni sulle obbligazioni contano i giorni secondo la **base** indicata (0 significa 30 giorni al mese in un anno di 360 giorni, 1 significa giorni effettivi, e così via). REND, REND.PRIMO.IRR, REND.ULTIMO.IRR e TASSO trovano il risultato provando dei valori finché il prezzo o la rata coincidono. Si fermano quando la risposta è corretta fino a circa 15 cifre, quindi le ultime cifre possono differire da quelle di un altro programma.
- **Distribuzioni e test.** Le funzioni che terminano in .DIST, .INV, .TEST, e i nomi più vecchi come DISTRIB.NORM e DISTRIB.T, danno gli stessi risultati: un nome precedente viene mantenuto affinché i file vecchi continuino a funzionare. Anche le funzioni *.INV si trovano provando dei valori.
- **Elenchi che riempiono più celle.** ORDINA, ORDINA.PER, FILTRA, UNICI, SEQUENZA, MATR.CASUALE, CERCA.X, MMULT, MATR.INVERSA e REGR.LIN restituiscono un blocco di valori. Inseriscili come formula matriciale (**Ctrl+Maiusc+Invio**) sulle celle che devono riempire. Non è ancora possibile far riempire alle formule le celle vicine in modo automatico.
- **Testi in altri alfabeti.** LUNGHEZZAB, SINISTRAB, STRINGA.ESTRAI.B, DESTRAB, TROVAB, RICERCAB e SOSTITUISCIB contano ogni carattere come uno, come avviene su un computer non configurato per il testo a doppio byte. FONETICO restituisce il testo della cella, perché JeffCrunch non conserva le letture del testo giapponese.
- **CONVERTI.** Le unità si scrivono come testo, con lettere maiuscole e minuscole come indicato nell'elenco della funzione: "kg", "lbm", "mi", "F". Un prefisso metrico può precedere un'unità che lo accetta, come "km" o "mL".

## MOD: il resto

**MOD(n; d)** restituisce ciò che resta di **n** dopo aver tolto il maggior numero intero di volte di **d**: è n - d * INT(n / d). Esempio: **=MOD(10; 3)** dà 1, e **=MOD(-10; 3)** dà 2, perché INT(-10 / 3) è -4 e -10 - 3 * -4 è 2. Il risultato ha il segno di **d**. Se **d** è 0, il risultato è #DIV/0!.

## Mostrare le formule

Fai clic su **Mostra formule** nella scheda **Formule**, oppure premi **Ctrl+'** (su Mac, il tasto Control e non il tasto Command), per vedere la formula di ogni cella al posto del suo valore. Le colonne che contengono formule vengono allargate, fino a un certo limite, in modo che le formule si vedano per intero; tornano alla larghezza originale quando disattivi l'opzione, e le larghezze del file non vengono modificate.

## Modificare le formule

Per modificare una formula, seleziona la sua cella, premi **F2**, modificala e premi **Invio**. Puoi anche modificarla nella barra della formula. Quando cambi una cella, le formule che ne dipendono vengono ricalcolate in background. Vedi [Digitare nelle celle](edit-cells).

## Scrivere una formula

Digita **=** e le prime lettere di una funzione: compare un breve elenco delle funzioni e dei [nomi](names) che iniziano in quel modo, con una frase per ciascuno. Premi **Tab** per scegliere quella evidenziata. Usa le frecce **Giù** e **Su** per evidenziarne un'altra, poi **Tab** o **Invio**. **Esc** chiude l'elenco. Quando sei dentro le parentesi di una funzione, i suoi argomenti compaiono sotto la cella, con quello che stai digitando in grassetto. L'elenco e gli argomenti sono gli stessi mostrati da **Scegli una funzione**. Gli argomenti sono separati da virgole, o da punti e virgola dove la tua lingua usa la virgola come separatore decimale.

## Puntare alle celle

Invece di digitare l'indirizzo di una cella, puoi indicarla. Mentre digiti una formula, posiziona il cursore dove può andare una cella (subito dopo **=**, dopo un operatore come **+**, una parentesi aperta o una virgola) e fai clic sulla cella. Il suo indirizzo viene scritto al posto tuo. Trascina sulle celle per scrivere un intervallo come B2:D5. Fai di nuovo clic prima di digitare altro, e la nuova cella sostituisce quella appena scritta. Tieni premuto **Ctrl** e fai clic per aggiungere un altro indirizzo dopo una virgola. Fai clic su una lettera di colonna per scrivere un'intera colonna come A:A, oppure su un numero di riga per scrivere un'intera riga come 3:3, e trascina lungo le lettere o i numeri per B:D o 2:5.

Anche i tasti freccia funzionano. Premi una freccia e un puntatore si sposta lontano dalla cella in cui stai scrivendo, e il suo indirizzo viene scritto; premi altre frecce per continuare, oppure tieni premuto **Maiusc** per estendere il puntatore su un intervallo. La barra di stato indica **Punta** mentre i tasti freccia puntano, **Invio** mentre digiti, e **Modifica** mentre i tasti freccia spostano il cursore nel testo. Premi **F2** per passare dalla modalità di puntamento a quella di modifica del testo.

Per puntare a una cella di un altro foglio, fai clic sulla scheda di quel foglio mentre il cursore è dove può andare una cella, e poi fai clic sulla cella. Il nome del foglio viene scritto insieme all'indirizzo, tra apici quando serve, come in 'Il mio foglio'!B3. **Invio** ti riporta al foglio da cui sei partito e inserisce la formula nella sua cella. **Esc** annulla tutto, compreso il cambio di foglio.

Puoi anche trascinare il bordo di un blocco evidenziato sul foglio per spostare quell'indirizzo, oppure trascinare il suo quadratino d'angolo per allargarlo: la formula cambia mentre trascini.

Premi **F4** con il cursore dentro o subito dopo un indirizzo per bloccarne parti con il segno del dollaro: A1 diventa $A$1, poi A$1, poi $A1, e poi di nuovo A1.

Ogni indirizzo nella formula è colorato nel testo, e le celle che nomina sono contornate sul foglio con lo stesso colore, così puoi vedere a colpo d'occhio che cosa legge la formula. Un clic dopo un indirizzo completo, come =A1, termina l'inserimento e seleziona la cella su cui hai fatto clic, come avviene per qualsiasi altro testo.

## Tabelle dati

Una tabella dati (tabella di simulazione) mostra il risultato di una formula per molti valori di una o due celle. Apri una cartella di lavoro che ne contiene una, e JeffCrunch ne calcola le celle: per una tabella con due variabili, la formula si trova nella cella in alto a sinistra della tabella, i valori da provare in orizzontale vanno nella cella di input di riga e quelli in verticale nella cella di input di colonna. Per una tabella con una sola variabile, i valori vanno nella colonna a sinistra (e le formule si trovano nella riga sopra) oppure nella riga sopra (e le formule si trovano nella colonna a sinistra). Ogni cella della tabella è il risultato della formula, e di ogni formula che dipende dalla cella di input, con l'input impostato a quel valore; nient'altro nella cartella di lavoro viene modificato. In modalità **Automatica** le tabelle vengono calcolate insieme alle formule, e le formule che leggono le celle di una tabella ne seguono i valori. La creazione di nuove tabelle dati non è ancora disponibile. Una tabella che non può essere calcolata (una formula che dipende dall'input usa una funzione non disponibile in JeffCrunch, oppure cambia da sola, come CASUALE o ADESSO) mantiene i valori contenuti nel file.

## Calcolo manuale

Fai clic su **Modalità di calcolo** nella scheda **Formule** per scegliere come la cartella di lavoro calcola le formule:

- **Automatica** calcola le formule man mano che cambi le celle.
- **Automatica, tabelle manuali** fa lo stesso, ma lascia invariate le celle delle tabelle dati finché non premi **F9**.
- **Manuale** non calcola nulla, nemmeno le tabelle dati, finché non lo richiedi. La barra di stato indica **Non aggiornato** dopo una modifica. Premi **F9** (**Ricalcola tutto**) per calcolare tutto, oppure **Maiusc+F9** (**Ricalcola questo foglio**) per calcolare le formule del foglio in cui ti trovi, insieme a tutto ciò che era in attesa. Una cartella di lavoro impostata su Manuale si apre con i risultati contenuti nel file.

La scelta viene salvata con la cartella di lavoro e può essere annullata come qualsiasi altra operazione.

## Formule che leggono se stesse

Una formula che legge il proprio risultato, direttamente o tramite altre celle, non può essere calcolata nel modo consueto. JeffCrunch lo segnala una sola volta, indica le celle e le contorna sul foglio; esse mantengono i valori che avevano. Fai clic su **Riferimenti circolari** nella scheda **Formule** per visualizzarne l'elenco, e fai doppio clic su una cella dell'elenco per andarci.

Alcuni modelli richiedono intenzionalmente un ciclo di questo tipo. Scegli **Calcolo iterativo delle formule circolari** nel menu **Modalità di calcolo**: le formule vengono calcolate più volte, ciascuna leggendo gli ultimi valori delle altre, finché nessun valore cambia più della quantità che indichi oppure non viene raggiunto il numero massimo di iterazioni impostato.

## Vedere che cosa legge una formula

Seleziona una cella e fai clic su **Mostra celle che alimentano questa cella** per tracciare frecce dalle celle che la sua formula legge. Fai di nuovo clic per risalire di un passo. **Mostra celle che usano questa cella** traccia frecce verso le formule che leggono la cella, e ogni clic va avanti di un passo. Una piccola pagina rappresenta una cella di un altro foglio. Vengono tracciate al massimo 2.000 frecce, e la barra di stato lo segnala quando ce ne sarebbero di più. **Rimuovi frecce** le cancella, e le cancella anche qualsiasi modifica alla cartella di lavoro.
