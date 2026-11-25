# Come vengono mostrate e calcolate le formule

<!-- keywords: spill, spilled, #SPILL!, matrice dinamica, intervallo di spill, A1#, errore successivo, errore precedente, scorrere gli errori, traccia errore, calcola prima del salvataggio, formula, formule, calcola, calcolo, ricalcola, funzione, funzioni, valore, risultato, memorizzato, F9, circolare, mostra formule, barra della formula, errore, punta, puntamento, clic su una cella, riferimento, F4, verifica errori, ignora errore, valuta formula, passo, finestra di controllo, controllo, LET, dare un nome a un valore, funzione cella, funzione info, filterxml, xml, xpath, precisione come visualizzata, imposta precisione come visualizzata, arrotonda, arrotondamento, decimali, cifre -->

Una **formula** è una cella che ricava il proprio valore da altre celle, ad esempio =SOMMA(B3:B8). La cella mostra il valore e la **barra della formula** mostra la formula.

## Che cosa fa JeffCrunch con le formule

Quando apri una cartella di lavoro, il file contiene l'ultimo valore di ogni formula, così il foglio può essere mostrato subito. Poi JeffCrunch ricalcola ogni formula, alcune alla volta, in modo che la finestra resti reattiva. La barra di stato indica **Calcolo** con una percentuale mentre il lavoro è in corso. Al termine, le celle contengono i risultati calcolati da JeffCrunch. Per ricalcolare tutto, fai clic su **Ricalcola tutto** nella scheda **Formule** oppure premi **F9**.

## Quando JeffCrunch non riesce a calcolare una formula

JeffCrunch dispone di più di 400 funzioni e ne aggiunge continuamente di nuove. Una formula che usa una funzione non ancora disponibile in JeffCrunch conserva il valore memorizzato nel file. Lo stesso vale per una formula che fa riferimento a un'altra cartella di lavoro. Le celle che dipendono da queste vengono calcolate a partire dai valori memorizzati.

## Funzioni che richiedono alcune precisazioni

- **Obbligazioni e prestiti.** PREZZO, RENDIMENTO, DURATA, PREZZO.PRIMO.IRR, e le altre funzioni sulle obbligazioni contano i giorni secondo la **base** indicata (0 significa 30 giorni per mese in un anno di 360 giorni, 1 significa i giorni effettivi, e così via). RENDIMENTO, RENDIMENTO.PRIMO.IRR, RENDIMENTO.ULTIMO.IRR e TASSO trovano il risultato provando diversi valori finché il prezzo o la rata corrispondono. Si fermano quando il risultato è esatto fino a circa 15 cifre, quindi le ultime cifre possono differire da quelle di altri programmi.
- **Distribuzioni e test.** Le funzioni che terminano con .DIST, .INV, .TEST e i nomi più vecchi come DISTRIB.NORM e DISTRIB.T danno gli stessi risultati: un nome precedente viene conservato affinché i vecchi file continuino a funzionare. Anche le funzioni *.INV trovano il risultato provando dei valori.
- **Elenchi che riempiono più celle.** ORDINA, ORDINA.PER, FILTRO, UNICO, SEQUENZA, MATR.CASUALE, CERCA.X, MATR.PRODOTTO, MATR.INVERSA e REGR.LIN restituiscono un blocco di valori. Digita una di queste funzioni in una sola cella e il blocco si estende alle celle sotto e a fianco (vedi **Formule che riempiono le celle accanto**), oppure inseriscila come formula matriciale (**Ctrl+Maiusc+Invio**) sulle celle che deve riempire.
- **Testi in altri sistemi di scrittura.** LUNGHEZZAB, SINISTRAB, STRINGAB, DESTRAB, TROVAB, RICERCAB e SOSTITUISCIB contano ogni carattere come uno, come accade su un computer non configurato per il testo a doppio byte. FONETICO restituisce il testo della cella, perché JeffCrunch non conserva le letture del testo giapponese.
- **CONVERTI.** Le unità si scrivono come testo, con maiuscole e minuscole come indicato nell'elenco della funzione: "kg", "lbm", "mi", "F". Un prefisso metrico può precedere un'unità che lo accetta, come "km" o "mL".

## MOD: il resto

**RESTO(n; d)** restituisce ciò che resta di **n** dopo aver tolto il numero intero più grande possibile di volte **d**: è n - d * INT(n / d). Esempio: **=RESTO(10; 3)** dà 1, e **=RESTO(-10; 3)** dà 2, perché INT(-10 / 3) è -4 e -10 - 3 * -4 è 2. Il risultato ha il segno di **d**. Se **d** è 0, il risultato è #DIV/0!.

## Mostrare le formule

Fai clic su **Mostra formule** nella scheda **Formule**, oppure premi **Ctrl+`** (su Mac, il tasto Control e non il tasto Comando), per vedere la formula di ogni cella al posto del suo valore. Le colonne che contengono formule vengono allargate, fino a un certo limite, in modo che le formule si vedano per intero; tornano alla larghezza originale quando disattivi l'opzione, e le larghezze nel file non vengono modificate.

## Modificare le formule

Per modificare una formula, seleziona la sua cella, premi **F2**, modificala e premi **Invio**. Puoi modificarla anche nella barra della formula. Quando cambi una cella, le formule che ne dipendono vengono ricalcolate in background. Vedi [Digitare nelle celle](edit-cells).

## Scrivere una formula

Digita **=** e le prime lettere di una funzione: compare un breve elenco delle funzioni e dei [nomi](names) che iniziano in quel modo, con una descrizione per ciascuno. Premi **Tab** per scegliere quella evidenziata. Usa le frecce **Giù** e **Su** per evidenziarne un'altra, poi **Tab** o **Invio**. **Esc** chiude l'elenco. Quando sei tra le parentesi di una funzione, i suoi argomenti compaiono sotto la cella, con quello che stai digitando in grassetto. L'elenco e gli argomenti sono gli stessi che mostra **Scegli una funzione**. Gli argomenti sono separati da virgole, oppure da punti e virgola nelle impostazioni in cui la virgola è usata come separatore decimale.

## Nomi delle funzioni nella tua lingua

Quando JeffCrunch è visualizzato in tedesco, francese, spagnolo, italiano, olandese, portoghese o russo, puoi scrivere una formula con i nomi delle funzioni della tua lingua, ad esempio `=SUMME(A1;A2)` in tedesco o `=SOMME(A1;A2)` in francese. Funzionano anche i nomi inglesi, quindi una formula copiata dal web non deve essere modificata. La barra della formula, la cella, l'elenco delle funzioni e **Scegli una funzione** mostrano i nomi della tua lingua.

La cartella di lavoro non cambia quando passi a un'altra lingua. Un file conserva sempre i nomi inglesi, così una cartella creata in una lingua si apre e si legge correttamente in un'altra lingua, e in qualsiasi altro programma. I nomi locali sono disponibili in tedesco, francese, spagnolo, italiano, olandese, portoghese (Brasile e Portogallo) e russo; i nomi inglesi funzionano sempre, in ogni lingua. I programmi in ucraino e indonesiano mantengono i nomi inglesi per quasi tutte le funzioni. Quando una lingua ha due nomi comuni per una stessa funzione, vengono riconosciuti entrambi e si mostra quello più diffuso. Una funzione che non ha un nome consueto nella tua lingua mantiene quello inglese. Anche VERO e FALSO seguono la lingua. I valori di errore della tua lingua, come `#NV` in tedesco, vengono riconosciuti quando digiti una formula e mostrati nella barra della formula, ma una cella con un errore mostra sempre la forma standard, come `#N/A`.

Quando le impostazioni regionali del computer usano la virgola come separatore decimale, gli argomenti di una funzione sono separati dal punto e virgola. Una matrice tra parentesi graffe con più di una riga viene mostrata con un punto e virgola tra i valori di una riga e una barra rovesciata tra le righe, come `{1;2\3;4}`, e puoi digitarla in questa forma. Una matrice digitata senza barra rovesciata viene letta nel modo in cui un file la scrive: virgole tra i valori di una riga, punti e virgola tra le righe, e il punto come separatore decimale.

Per digitare solo nomi inglesi, fai clic su **File > Opzioni > Formule** e seleziona **Usa nomi di funzione in inglese**.

## Puntare alle celle

Invece di digitare l'indirizzo di una cella, puoi puntarla. Mentre digiti una formula, posiziona il cursore dove può andare una cella (subito dopo **=**, dopo un operatore come **+**, una parentesi aperta o una virgola) e fai clic sulla cella. Il suo indirizzo viene scritto per te. Trascina sulle celle per scrivere un intervallo come B2:D5. Fai di nuovo clic prima di digitare altro, e la nuova cella sostituisce quella appena scritta. Tieni premuto **Ctrl** e fai clic per aggiungere un altro indirizzo dopo una virgola. Fai clic su una lettera di colonna per scrivere un'intera colonna, come A:A, oppure su un numero di riga per scrivere un'intera riga, come 3:3, e trascina lungo le lettere o i numeri per B:D o 2:5.

Funzionano anche i tasti freccia. Premi una freccia e un puntatore si sposta dalla cella in cui stai scrivendo, e il suo indirizzo viene scritto; premi altre frecce per continuare, oppure tieni premuto **Maiusc** per estendere il puntatore su un intervallo. La barra di stato indica **Punta** mentre le frecce puntano, **Invio** mentre digiti, e **Modifica** mentre le frecce spostano il cursore nel testo. Premi **F2** per passare dalla modalità di puntamento alla modifica del testo.

Per puntare a una cella di un altro foglio, fai clic sulla scheda di quel foglio mentre il cursore è dove può andare una cella, poi fai clic sulla cella. Il nome del foglio viene scritto insieme all'indirizzo, tra apici quando necessario, come in 'Il mio foglio'!B3. **Invio** ti riporta al foglio da cui sei partito e inserisce la formula nella sua cella. **Esc** annulla tutto, compreso il cambio di foglio.

Puoi anche trascinare il bordo di un intervallo evidenziato sul foglio per spostare quell'indirizzo, oppure trascinare il suo quadratino d'angolo per allungarlo; la formula cambia mentre trascini.

Premi **F4** con il cursore dentro o subito dopo un indirizzo per bloccarne alcune parti con il simbolo del dollaro: A1 diventa $A$1, poi A$1, poi $A1, e poi di nuovo A1.

Ogni indirizzo della formula è colorato nel testo, e le celle che nomina sono evidenziate sul foglio con lo stesso colore, così puoi vedere a colpo d'occhio che cosa legge la formula. Un clic dopo un indirizzo completo, come =A1, termina l'inserimento e seleziona la cella su cui hai fatto clic, come avviene per qualsiasi altro testo.

## Tabelle dei dati

Una tabella dei dati (tabella di ipotesi) mostra il risultato di una formula per molti valori di una o due celle. Apri una cartella di lavoro che ne contiene una e JeffCrunch calcola le sue celle: per una tabella con due variabili, la formula si trova nella cella in alto a sinistra della tabella, i valori da provare in orizzontale vanno nella cella di input di riga e quelli in verticale nella cella di input di colonna. Per una tabella con una sola variabile, i valori vanno nella colonna a sinistra (e le formule si trovano nella riga sopra) oppure nella riga sopra (e le formule si trovano nella colonna a sinistra). Ogni cella della tabella è il risultato della formula, e di ogni formula che dipende dalla cella di input, con l'input impostato su quel valore; nient'altro nella cartella di lavoro viene modificato. In modalità **Automatica** le tabelle vengono calcolate insieme alle formule, e seguono le formule che leggono le celle della tabella. La creazione di una nuova tabella dei dati non è ancora disponibile. Una tabella che non può essere calcolata (una formula che dipende dall'input usa una funzione non disponibile in JeffCrunch, oppure cambia da sola, come CASUALE o ORA) mantiene i valori contenuti nel file.

## Calcolo manuale

Fai clic su **Modalità di calcolo** nella scheda **Formule** per scegliere come la cartella di lavoro calcola le sue formule:

- **Automatica** le calcola man mano che cambi le celle.
- **Automatica, tabelle manuali** fa lo stesso, ma lascia invariate le celle delle tabelle dei dati finché non premi **F9**.
- **Manuale** non calcola nulla, comprese le tabelle dei dati, finché non lo chiedi. La barra di stato indica **Non aggiornato** dopo una modifica. Premi **F9** (**Ricalcola tutto**) per calcolare tutto, oppure **Maiusc+F9** (**Ricalcola questo foglio**) per calcolare le formule del foglio corrente e tutto ciò che è in attesa. Una cartella di lavoro impostata su Manuale si apre con i risultati contenuti nel file. **Calcola prima del salvataggio**, anch'essa nel menu **Modalità di calcolo**, è attivo per impostazione predefinita: quando salvi una cartella di lavoro manuale non aggiornata, le formule vengono calcolate prima, così il file contiene numeri aggiornati. Disattivala per salvare i numeri così come sono.

La scelta viene salvata con la cartella di lavoro ed è un passaggio che puoi annullare.

## Formule che leggono se stesse

Una formula che legge il proprio risultato, direttamente o attraverso altre celle, non può essere calcolata nel modo consueto. JeffCrunch lo segnala una sola volta, indica le celle e le evidenzia sul foglio; queste mantengono i valori che avevano. Fai clic su **Riferimenti circolari** nella scheda **Formule** per vedere l'elenco, e fai doppio clic su una cella dell'elenco per andarvi.

Alcuni modelli richiedono intenzionalmente un ciclo di questo tipo. Scegli **Calcolo iterativo delle formule circolari** nel menu **Modalità di calcolo**: le formule vengono ricalcolate più volte, leggendo ogni volta gli ultimi valori delle altre, finché nessun valore cambia più dell'importo indicato oppure non si raggiunge il numero massimo di iterazioni.

## Usare i numeri come li mostrano le celle

Una cella può mostrare meno cifre di quante ne contenga. Se una cella contiene 2,675 e mostra 2,68, le formule che la leggono usano comunque 2,675. A volte vuoi che usino ciò che vedi. In questo caso scegli **Imposta precisione come visualizzata** nel menu **Opzioni di calcolo** della scheda **Formule**.

JeffCrunch ti avvisa prima, perché le cifre nascoste andranno perse. Scegli **Annulla** e non cambia nulla. Scegli **OK** e ogni numero della cartella di lavoro viene arrotondato a ciò che mostra la sua cella. Una cella che mostra due decimali ne mantiene due. Una cella che mostra una percentuale mantiene la percentuale. Una cella con una data mantiene il giorno, e una cella con un orario mantiene la parte più piccola di tempo che mostra, come i minuti. Una cella che mostra **Generale** mantiene tutte le sue cifre. Una cartella di lavoro grande mostra un avviso con un pulsante **Annulla** mentre l'operazione è in corso. Se annulli mentre l'avviso è visibile, non cambia nulla.

Da quel momento, ogni numero che digiti o incolli, e ogni risultato di una formula, viene arrotondato allo stesso modo. Le formule non vengono modificate. La scelta viene salvata con la cartella di lavoro.

L'arrotondamento è un passaggio che puoi annullare: **Annulla** riporta i numeri al valore precedente. Scegliere di nuovo **Imposta precisione come visualizzata** disattiva l'opzione. Questo blocca soltanto l'arrotondamento da quel momento in poi. I numeri già arrotondati restano arrotondati.

## Vedere che cosa legge una formula

Seleziona una cella e fai clic su **Mostra celle che alimentano questa cella** per disegnare frecce dalle celle che la sua formula legge. Fai di nuovo clic per risalire di un passo. **Mostra celle che usano questa cella** disegna frecce verso le formule che leggono la cella, e ogni clic procede di un passo. Una piccola pagina rappresenta una cella di un altro foglio. Vengono disegnate al massimo 2.000 frecce, e la barra di stato lo segnala quando ce ne sono di più. **Rimuovi frecce** le cancella, e le cancella anche qualsiasi modifica alla cartella di lavoro.

Quando una cella mostra un errore come #DIV/0!, selezionala e fai clic su **Traccia errore**. Vengono disegnate frecce verso le celle che legge e che a loro volta mostrano un errore. Fai di nuovo clic per risalire di un passo. Quando nessuna delle celle lette mostra un errore, la barra di stato indica che questa cella è quella che genera l'errore, ed è quella da correggere.

Per scorrere tutti gli errori del foglio, fai clic su **Errore successivo** o **Errore precedente**. Le celle che mostrano un errore vengono visitate una riga dopo l'altra, e dopo l'ultima si ricomincia dalla prima. Ogni cella viene selezionata e le frecce vengono disegnate fino alla cella che genera l'errore. La barra di stato indica quale errore stai visualizzando e quanti sono in tutto.

## Verifica errori

Scegli **Verifica errori** nel gruppo **Verifica** della scheda Formule per cercare nel foglio le celle che potrebbero essere sbagliate. Cerca una formula che restituisce un errore (come #DIV/0!), un numero memorizzato come testo (che le formule potrebbero ignorare), e altro: una formula diversa da quelle accanto, una data memorizzata come testo con un anno a due cifre, una formula in una cella non bloccata, una formula che esclude un numero accanto alle celle che usa (solo per una funzione che somma o totalizza un intervallo, come **SOMMA**, **MEDIA**, **CONTA.NUMERI**, **MIN** o **MAX**, quando l'intervallo è da solo come argomento), e una formula che usa una cella vuota.

La finestra mostra una cella alla volta, porta a quella cella sul foglio e spiega in una frase qual è il problema. **Successivo** e **Precedente** scorrono le celle. Un piccolo triangolo verde nell'angolo superiore sinistro di una cella la segnala sul foglio. Quando la cella può essere corretta, compare un pulsante con la correzione (per una formula che esclude un numero, **Includi la cella**); il pulsante modifica la formula, e un passaggio di **Annulla** ripristina la vecchia formula. **Ignora errore** rimuove la cella dall'elenco, elimina il suo triangolo e viene salvato nel file. Un passaggio di **Annulla** lo ripristina. Se non c'è nessun problema, viene segnalato. Per scegliere quali controlli sono attivi, apri **Opzioni** e la pagina **Formule**: c'è una casella di controllo per ciascuno, e una casella deselezionata impedisce di segnalare quel tipo di problema. La scelta vale per il programma e quindi per tutte le finestre aperte. Una cella può avere più di un problema, e **Ignora errore** ignora solo quello visualizzato.

## Valuta formula

Usala per vedere come una formula arriva al suo valore. Seleziona una cella che contiene una formula e scegli **Valuta formula**. La formula viene mostrata con la parte successiva sottolineata. Premi **Valuta** per calcolare quella parte; il suo valore prende il posto della parte stessa. Le parti vengono valutate in quest'ordine: prima ogni singola cella da sinistra, poi ogni funzione con le sue parentesi, partendo da quella più interna, e infine il resto. **Ricomincia** riparte dall'inizio. Se una parte non può essere calcolata, la finestra spiega perché e lascia la formula com'era.

## Finestra di controllo

La **Finestra di controllo** è una piccola finestra che resta visibile mentre lavori. Elenca le celle che vuoi tenere sotto controllo, con il foglio, la cella, il valore e la formula, anche quando si trovano su un altro foglio. Seleziona delle celle e premi **Aggiungi controllo**. Seleziona delle righe dell'elenco e premi **Elimina controllo**. Fai doppio clic su una riga per andare alla cella. I valori cambiano quando il foglio viene ricalcolato. L'elenco viene conservato finché la cartella di lavoro è aperta.

## Formule che riempiono le celle accanto

Quando una formula restituisce un blocco di valori, il blocco riempie le celle sotto e accanto alla cella in cui l'hai digitata. Questo si chiama spill (traboccamento). Prova =SEQUENZA(3) in A1: A1 mostra 1, e A2 e A3 mostrano 2 e 3. Una formula trabocca quando chiama una funzione che restituisce un blocco (ORDINA, ORDINA.PER, FILTRO, UNICO, SEQUENZA, MATR.CASUALE, MATR.TRASPOSTA, CERCA.X o MATR.PRODOTTO), scrive un blocco tra parentesi graffe, usa un intervallo di spill (vedi sotto), oppure esegue un'operazione aritmetica su un intervallo, come in =A1:A3*2.

- **Le celle riempite non contengono una formula propria.** Mostrano i valori. Quando ne selezioni una, la barra della formula mostra in grigio la formula della cella che la riempie. Premere Invio su di essa così com'è non cambia nulla. Per modificare il blocco, modifica prima di tutto quella prima cella.
- **Il blocco segue il risultato.** Se la formula restituisce più o meno valori dopo che è cambiato qualcosa che legge, il blocco cresce o si restringe, e le formule che leggono quelle celle vengono ricalcolate.
- **Una cella di ostacolo dà #SPILL!.** Se una qualsiasi cella in cui il blocco dovrebbe andare contiene un valore o una formula, la prima cella mostra **#SPILL!** e non viene riempito nulla. Svuota la cella di ostacolo e il blocco si riempie subito. Digitare in una cella che il blocco riempirebbe produce lo stesso effetto: la prima cella mostra #SPILL!.
- **Un blocco ha un limite di dimensione.** Un blocco più grande del foglio o di più di 200.000 celle dà #SPILL!.
- **A1# indica l'intero blocco.** Scrivi la cella che contiene la formula seguita da **#**, come in =SOMMA(A1#), per usare l'intero blocco così come è ora, da questo foglio o da un altro (=SOMMA(Foglio1!A1#)). Se A1 non trabocca, A1# dà #RIF!.

Uno spill viene salvato nel file come matrice dinamica, così il programma da cui il file proviene lo mostra anch'esso come spill. Il salvataggio nei formati più vecchi .xls o .ods conserva i valori e la formula come formula matriciale sul blocco, nei casi in cui il formato precedente supporta le funzioni usate. Verifica compatibilità segnala queste formule.

## Dare un nome a valori dentro una formula

**LET** assegna un nome a un valore nel mezzo di una formula, così lo scrivi una sola volta e lo riusi. Le coppie di nome e valore vengono prima, il calcolo per ultimo. In =LET(prezzo; B2; imposta; prezzo*0,08; prezzo+imposta) il nome **prezzo** rappresenta il valore in B2, **imposta** viene calcolata a partire da esso, e l'ultima parte le somma. Un nome può usare i nomi che lo precedono, e un LET può contenere fino a 126 coppie. Un nome funziona solo all'interno del proprio LET, e un LET annidato in un altro può riutilizzare un nome. Un nome può rappresentare anche un intervallo di celle, come in =LET(righe; B2:B9; SOMMA(righe)/CONTA.NUMERI(righe)).

## Chiedere informazioni su una cella o sulla cartella di lavoro

**CELLA** restituisce informazioni su una cella. Il suo primo argomento indica quali: "indirizzo", "riga", "col", "contenuto", "tipo", "formato" (come vengono mostrati i numeri, ad esempio "F2" per due decimali, "P0" per una percentuale, o "D1" per una data), "colore" (1 quando i numeri negativi sono mostrati in colore), "parentesi" (1 quando i numeri positivi sono mostrati tra parentesi), "prefisso" (il segno di un testo allineato a sinistra, a destra o al centro), "protetta" (1 quando la cella è bloccata), "larghezza" (la larghezza della colonna in caratteri) e "nomefile" (la cartella, il file e il foglio, come /home/io/[Budget.xlsx]Foglio1; vuoto finché la cartella non viene salvata). **INFO** fornisce informazioni sul programma e sulla cartella di lavoro: "directory", "numfile" (quanti fogli sono visibili), "origine", "versionesO", "recalc" (Automatico o Manuale), "release" e "sistema".

## Estrarre valori da XML

**XML.FILTRO** accetta un testo che contiene XML e un XPath, cioè un modo breve per indicare quali parti vuoi, e restituisce i valori trovati. =XML.FILTRO(A1; "//articolo/prezzo") restituisce ogni prezzo del testo in A1, uno sotto l'altro. Usa "/negozio/articolo[2]/nome" per il secondo articolo, "//articolo[@id='b']/nome" per l'articolo il cui id è b, oppure "conta(//articolo)" per sapere quanti sono. Se il testo non è XML valido, o il percorso non è valido, il risultato è #VALORE!. Se nulla corrisponde, il risultato è #N/D. XML che contiene una dichiarazione di tipo di documento viene rifiutato.
