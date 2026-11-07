# Come vengono mostrate e calcolate le formule

<!-- keywords: errore successivo, errore precedente, scorrere gli errori, tracciare errore, calcola prima del salvataggio, formula, formule, calcolo, calcolare, ricalcolare, funzione, funzioni, valore, risultato, memorizzato, F9, circolare, mostra formule, barra della formula, errore, punto, puntare, clic su una cella, riferimento, F4, verifica errori, ignora errore, valuta formula, passo, finestra di controllo, controllo, LET, assegnare nomi a valori, funzione cella, funzione info, FILTRO.XML, xml, xpath -->

Una **formula** è una cella che calcola il proprio valore a partire da altre celle, come =SOMMA(B3:B8). La cella mostra il valore, e la **barra della formula** mostra la formula.

## Che cosa fa JeffCrunch con le formule

Quando apri una cartella di lavoro, il file contiene l'ultimo valore di ogni formula, così il foglio può essere mostrato subito. Poi JeffCrunch ricalcola ogni formula, alcune alla volta, in modo che la finestra resti rapida. La barra di stato mostra **Calcolo** con una percentuale mentre il lavoro è in corso. Al termine, le celle contengono i risultati di JeffCrunch. Per ricalcolare tutto di nuovo, fai clic su **Ricalcola tutto** nella scheda **Formule**, oppure premi **F9**.

## Quando JeffCrunch non può calcolare una formula

JeffCrunch dispone di più di 400 funzioni, e il loro numero continua a crescere. Una formula che usa una funzione non ancora disponibile in JeffCrunch conserva il valore memorizzato nel file. Lo stesso vale per una formula che fa riferimento a un'altra cartella di lavoro. Le celle che dipendono da queste vengono calcolate a partire dai valori memorizzati.

## Funzioni che richiedono alcune precisazioni

- **Obbligazioni e prestiti.** PREZZO, REND, DURATA, PREZZO.PRIMO.IRR e le altre funzioni sulle obbligazioni contano i giorni secondo la **base** che indichi (0 significa 30 giorni al mese in un anno di 360 giorni, 1 significa i giorni effettivi, e così via). REND, REND.PRIMO.IRR, REND.ULTIMO.IRR e TASSO trovano il risultato provando valori finché il prezzo o il pagamento coincidono. Si fermano quando il risultato è corretto fino a circa 15 cifre, quindi le ultime cifre possono differire da quelle di un altro programma.
- **Distribuzioni e test.** Le funzioni che terminano in .DIST, .INV, .TEST e i nomi più vecchi, come DISTRIB.NORM e DISTRIB.T, danno gli stessi risultati: un nome precedente viene mantenuto affinché i file vecchi continuino a funzionare. Anche le funzioni *.INV trovano il risultato provando valori.
- **Elenchi che riempiono più celle.** ORDINA, ORDINA.PER, FILTRO, UNICI, SEQUENZA, MATR.CASUALE, CERCA.X, MATR.PRODOTTO, MATR.INVERSA e REGR.LIN restituiscono un blocco di valori. Inseriscile come formula matriciale (**Ctrl+Maiusc+Invio**) sulle celle che devono riempire. Non è ancora disponibile la propagazione automatica di una formula nelle celle adiacenti.
- **Testi in altri sistemi di scrittura.** LUNGHEZZAB, SINISTRAB, STRINGAB, DESTRAB, TROVAB, RICERCAB e SOSTITUISCIB contano ogni carattere come uno, come accade in un computer non configurato per il testo a doppio byte. FONETICA restituisce il testo della cella, perché JeffCrunch non conserva le letture del testo giapponese.
- **CONVERTI.** Le unità si scrivono come testo, con maiuscole e minuscole come mostrate nell'elenco della funzione: "kg", "lbm", "mi", "F". Un prefisso metrico può precedere un'unità che lo accetta, come "km" o "mL".

## MOD: il resto

**RESTO(n; d)** restituisce ciò che rimane di **n** dopo aver tolto il maggior numero di copie intere di **d**: è n - d * INT(n / d). Esempio: **=RESTO(10; 3)** dà 1, e **=RESTO(-10; 3)** dà 2, perché INT(-10 / 3) è -4 e -10 - 3 * -4 è 2. Il risultato ha il segno di **d**. Se **d** è 0, il risultato è #DIV/0!.

## Mostrare le formule

Fai clic su **Mostra formule** nella scheda **Formule**, oppure premi **Ctrl+`** (su un Mac, il tasto Control e non il tasto Comando), per vedere la formula di ogni cella al posto del suo valore. Le colonne che contengono formule vengono allargate, entro un certo limite, in modo che le formule si leggano per intero; tornano alla larghezza originale quando disattivi l'opzione, e le larghezze nel file non vengono modificate.

## Modificare le formule

Per modificare una formula, seleziona la sua cella, premi **F2**, modificala e premi **Invio**. Puoi anche modificarla nella barra della formula. Quando cambi una cella, le formule che dipendono da essa vengono ricalcolate in background. Vedi [Digitare nelle celle](edit-cells).

## Scrivere una formula

Digita **=** e le prime lettere di una funzione: compare un breve elenco delle funzioni e dei [nomi](names) che iniziano in quel modo, con una frase di descrizione per ciascuno. Premi **Tab** per scegliere quello evidenziato. Usa le frecce **Giù** e **Su** per evidenziarne un altro, quindi **Tab** o **Invio**. **Esc** chiude l'elenco. Quando sei all'interno delle parentesi di una funzione, i suoi argomenti compaiono sotto la cella, con quello che stai digitando in grassetto. L'elenco e gli argomenti sono gli stessi che mostra **Scegli una funzione**. Gli argomenti sono separati da virgole, o da punti e virgola dove la tua lingua usa la virgola come separatore decimale.

## Indicare le celle

Invece di digitare l'indirizzo di una cella, puoi indicarla. Mentre digiti una formula, metti il cursore dove può comparire una cella (subito dopo **=**, un operatore come **+**, una parentesi aperta o una virgola) e fai clic sulla cella. Il suo indirizzo viene scritto automaticamente. Trascina sulle celle per scrivere un intervallo come B2:D5. Fai di nuovo clic prima di digitare altro, e la nuova cella sostituisce quella appena scritta. Tieni premuto **Ctrl** e fai clic per aggiungere un altro indirizzo dopo una virgola. Fai clic su una lettera di colonna per scrivere un'intera colonna come A:A, oppure su un numero di riga per scrivere un'intera riga come 3:3, e trascina lungo le lettere o i numeri per B:D o 2:5.

Funzionano anche i tasti freccia. Premi una freccia e un puntatore si allontana dalla cella in cui stai scrivendo, e il suo indirizzo viene scritto; premi altre frecce per proseguire, oppure tieni premuto **Maiusc** per estendere il puntatore su un intervallo. La barra di stato mostra **Punta** mentre le frecce indicano, **Invio** mentre digiti e **Modifica** mentre le frecce spostano il cursore nel testo. Premi **F2** per passare dall'indicazione alla modifica del testo.

Per indicare una cella di un altro foglio, fai clic sulla linguetta di quel foglio mentre il cursore si trova dove può comparire una cella, quindi fai clic sulla cella. Il nome del foglio viene scritto insieme all'indirizzo, tra apici quando necessario, come in 'Il mio foglio'!B3. **Invio** ti riporta al foglio da cui sei partito e inserisce la formula nella sua cella. **Esc** annulla tutto, compreso il cambio di foglio.

Puoi anche trascinare il bordo di un intervallo evidenziato sul foglio per spostare quell'indirizzo, oppure trascinare il quadratino nell'angolo per allargarlo; la formula cambia mentre trascini.

Premi **F4** con il cursore dentro o subito dopo un indirizzo per bloccarne parti con il simbolo del dollaro: A1 diventa $A$1, poi A$1, poi $A1, e infine di nuovo A1.

Ogni indirizzo della formula è colorato nel testo, e le celle che nomina sono delimitate sul foglio dello stesso colore, così puoi vedere a colpo d'occhio che cosa legge la formula. Un clic dopo un indirizzo completo, come =A1, termina l'inserimento e seleziona la cella su cui hai fatto clic, come accade per qualsiasi altro testo.

## Tabelle dei dati

Una tabella dei dati (tabella di ipotesi) mostra il risultato di una formula per molti valori di una o due celle. Quando apri una cartella di lavoro che ne contiene una, JeffCrunch calcola le sue celle: per una tabella con due variabili, la formula si trova nella cella in alto a sinistra della tabella, i valori da provare in orizzontale vanno nella cella di input di riga, e quelli in verticale nella cella di input di colonna. Per una tabella con una sola variabile, i valori vanno in verticale nella colonna a sinistra (e le formule si trovano nella riga sopra la tabella), oppure in orizzontale nella riga sopra (e le formule si trovano nella colonna a sinistra). Ogni cella della tabella è il risultato della formula, e di ogni formula che dipende dalla cella di input, con l'input impostato a quel valore; nient'altro della cartella di lavoro viene modificato. In modalità **Automatica** le tabelle vengono calcolate insieme alle formule, e le formule che leggono le celle di una tabella ne seguono i risultati. La creazione di una nuova tabella dei dati non è ancora disponibile. Una tabella che non può essere calcolata (una formula che dipende dall'input usa una funzione non disponibile in JeffCrunch, oppure cambia da sola, come CASUALE o ADESSO) conserva i valori memorizzati nel file.

## Calcolo manuale

Fai clic su **Modalità di calcolo** nella scheda **Formule** per scegliere come la cartella di lavoro calcola le formule:

- **Automatica** calcola le formule man mano che cambi le celle.
- **Automatica, tabelle manuali** fa lo stesso, ma lascia invariate le celle delle tabelle dei dati finché non premi **F9**.
- **Manuale** non calcola nulla, comprese le tabelle dei dati, finché non lo chiedi. La barra di stato mostra **Non aggiornato** dopo una modifica. Premi **F9** (**Ricalcola tutto**) per calcolare tutto, oppure **Maiusc+F9** (**Ricalcola questo foglio**) per calcolare le formule del foglio corrente e tutto ciò che era in attesa. Una cartella di lavoro impostata su Manuale si apre con i risultati contenuti nel file. **Calcola prima del salvataggio**, anch'essa nel menu **Modalità di calcolo**, è attivo per impostazione predefinita: quando salvi una cartella di lavoro manuale non aggiornata, le formule vengono calcolate prima, così il file contiene numeri aggiornati. Disattivala per salvare i numeri così come sono.

La scelta viene salvata con la cartella di lavoro, ed è un passaggio che puoi annullare.

## Formule che leggono se stesse

Una formula che legge il proprio risultato, direttamente o attraverso altre celle, non può essere calcolata nel modo consueto. JeffCrunch lo segnala una sola volta, indica le celle e le delimita sul foglio; esse conservano i valori che avevano. Fai clic su **Riferimenti circolari** nella scheda **Formule** per visualizzarne l'elenco, e fai doppio clic su una cella dell'elenco per raggiungerla.

Alcuni modelli richiedono di proposito un simile ciclo. Scegli **Calcolo iterativo delle formule circolari** nel menu **Modalità di calcolo**: le formule vengono calcolate più volte, ciascuna leggendo gli ultimi valori delle altre, finché nessun valore cambia più della quantità che indichi o non si esauriscono le iterazioni consentite.

## Vedere che cosa legge una formula

Seleziona una cella e fai clic su **Mostra celle che alimentano questa cella** per tracciare frecce dalle celle che la sua formula legge. Fai di nuovo clic per risalire di un passo. **Mostra celle che usano questa cella** traccia frecce verso le formule che leggono la cella, e ogni clic procede di un passo. Una piccola pagina rappresenta una cella di un altro foglio. Vengono tracciate al massimo 2.000 frecce, e la barra di stato lo segnala quando ce ne sono di più. **Cancella frecce** le rimuove, e le rimuove anche qualsiasi modifica alla cartella di lavoro.

Quando una cella mostra un errore come #DIV/0!, selezionala e fai clic su **Traccia errore**. Vengono tracciate frecce verso le celle che legge e che a loro volta mostrano un errore. Fai di nuovo clic per risalire di un passo. Quando nessuna delle celle lette mostra un errore, la barra di stato indica che questa cella genera l'errore, ed è quella da correggere.

Per esaminare ogni errore del foglio, fai clic su **Errore successivo** o **Errore precedente**. Le celle che mostrano un errore vengono visitate riga dopo riga, e dopo l'ultima si ricomincia dalla prima. Ogni cella viene selezionata, e le frecce vengono tracciate fino alla cella che genera l'errore. La barra di stato indica quale errore stai esaminando e quanti sono in tutto.

## Verifica errori

Scegli **Verifica errori** nel gruppo **Verifica** della scheda Formule per cercare nel foglio le celle che potrebbero essere errate. Cerca due cose: una formula che restituisce un errore (come #DIV/0!), e un numero memorizzato come testo, che le formule potrebbero ignorare.

La finestra mostra una cella alla volta, va a quella cella nel foglio e spiega in una frase qual è il problema. **Successiva** e **Precedente** scorrono le celle. Un piccolo triangolo verde nell'angolo in alto a sinistra di una cella la segnala nel foglio. **Ignora errore** toglie la cella dall'elenco, rimuove il suo triangolo, e la scelta viene salvata nel file. Un passo di **Annulla** la ripristina. Se non c'è nulla da segnalare, te lo viene comunicato.

## Valuta formula

Usala per vedere come una formula arriva al suo valore. Seleziona una cella che contiene una formula e scegli **Valuta formula**. La formula viene mostrata con la parte successiva sottolineata. Premi **Valuta** per calcolare quella parte; il suo valore prende il suo posto. Le parti vengono prese in quest'ordine: ogni singola cella da sinistra, poi ogni funzione con le sue parentesi, partendo da quella più interna, e infine il resto. **Riavvia** ricomincia da capo. Se una parte non può essere calcolata, la finestra spiega il motivo e lascia la formula com'era.

## Finestra di controllo

La **Finestra di controllo** è una piccola finestra che resta visibile mentre lavori. Elenca le celle che vuoi tenere sotto controllo, con il loro foglio, la cella, il valore e la formula, anche quando si trovano su un altro foglio. Seleziona le celle e premi **Aggiungi controllo**. Seleziona le righe nell'elenco e premi **Elimina controllo**. Fai doppio clic su una riga per andare alla cella. I valori cambiano quando il foglio viene ricalcolato. L'elenco resta disponibile finché la cartella di lavoro è aperta.

## Assegnare nomi ai valori in una formula

**LET** assegna un nome a un valore all'interno di una formula, così lo scrivi una volta sola e lo riusi. Le coppie di nome e valore vengono prima, e il calcolo per ultimo. In =LET(prezzo; B2; imposta; prezzo*0,08; prezzo+imposta) il nome **prezzo** rappresenta il valore in B2, **imposta** viene calcolata a partire da esso, e l'ultima parte le somma. Un nome può usare i nomi definiti prima di lui. Un nome vale solo all'interno del proprio LET, e un LET annidato in un altro può riutilizzare un nome. Un nome può rappresentare anche un intervallo di celle, come in =LET(righe; B2:B9; SOMMA(righe)/CONTA.NUMERI(righe)).

## Chiedere informazioni su una cella o sulla cartella di lavoro

**CELLA** restituisce informazioni su una cella. Il suo primo argomento indica quale: "indirizzo", "riga", "col", "contenuto", "tipo", "formato" (come vengono mostrati i numeri, ad esempio "F2" per due decimali, "P0" per una percentuale o "D1" per una data), "colore" (1 quando i numeri negativi sono mostrati in colore), "parentesi" (1 quando i numeri positivi sono mostrati tra parentesi), "prefisso" (il segno di un testo allineato a sinistra, a destra o al centro), "protetta" (1 quando la cella è bloccata), "larghezza" (la larghezza della colonna in caratteri) e "nomefile" (la cartella, il file e il foglio, come /home/io/[Bilancio.xlsx]Foglio1; vuoto finché la cartella di lavoro non viene salvata). **INFO** restituisce informazioni sul programma e sulla cartella di lavoro: "directory", "numfile" (quanti fogli sono visualizzati), "origine", "versionesistema", "calcolo" (Automatico o Manuale), "versione" e "sistema".

## Estrarre valori da XML

**FILTRO.XML** prende un testo che contiene XML e un XPath, cioè un modo breve per indicare le parti che ti interessano, e restituisce i valori trovati. =FILTRO.XML(A1; "//item/price") restituisce ogni prezzo presente nel testo di A1, uno sotto l'altro. Usa "/shop/item[2]/name" per il secondo elemento, "//item[@id='b']/name" per l'elemento il cui id è b, oppure "count(//item)" per sapere quanti sono. Se il testo non è XML valido, o il percorso non è valido, il risultato è #VALORE!. Se non c'è alcuna corrispondenza, il risultato è #N/D. L'XML che contiene una dichiarazione di tipo di documento viene rifiutato.
