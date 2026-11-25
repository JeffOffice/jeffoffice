# Taglia, copia e incolla
<!-- keywords: taglia, copia, incolla, appunti, sposta, duplica, speciale, valori, formule, formati, trasponi, salta celle vuote, aggiungi, sottrai, moltiplica, dividi, operazione, unite, testo, scheda, html, pagina web, altro programma, Esc, bordo tratteggiato, larghezze colonne, note, commenti, formati numero, tutto tranne i bordi, immagine, bitmap, copia come immagine, immagine stampata, come stampato, incolla collegamento, collegamento ipertestuale, importazione testo, testo delimitato, trascina testo, virgola, punto e virgola, csv, separatore, imposta incolla predefinito, incolla predefinito, larghezza fissa -->

**Copia** (**{key:Copy}**) mette una copia delle celle selezionate negli Appunti, un'area di memoria che il computer conserva per un elemento alla volta. **Incolla** (**{key:Paste}**) inserisce il contenuto degli Appunti nelle celle, a partire dalla cella selezionata. **Taglia** (**{key:Cut}**) funziona come copia, ma quando incolli le celle vengono spostate e la posizione di partenza viene svuotata.

Un bordo tratteggiato animato indica le celle copiate o tagliate. Premi **Esc** per eliminare il bordo: a quel punto le celle non sono più pronte per essere incollate. Anche le celle tagliate vengono rilasciate se, prima di incollarle, esegui un'altra modifica (digitazione, inserimento, annullamento): se vuoi ancora spostarle, tagliale di nuovo.

## Incollare all'interno di JeffCrunch

Viene incollato tutto: valori, formule, formati e celle unite. Una formula continua a puntare alle stesse celle *in base alla propria posizione relativa*. Esempio: copia **=A1*2** da B1 e incollala in B5: diventa **=A5*2**. Un riferimento con il segno del dollaro, come **$A$1**, resta invariato. Se selezioni un intervallo che è un multiplo esatto delle dimensioni di ciò che hai copiato, la copia viene ripetuta per riempirlo: copia una cella, seleziona venti celle, incolla, e tutte e venti ricevono il contenuto.

Un numero incollato che è troppo largo per una colonna che ha ancora la larghezza iniziale allarga la colonna.

## Tagliare sposta le celle

Quando tagli le celle e le incolli, le celle vengono davvero spostate. Ogni formula che puntava a esse punta alla loro nuova posizione. Esempio: B1 contiene **=A1+A2**. Taglia A1:A2 e incolla in E4: B1 diventa **=E4+E5**. Il contenuto delle celle sopra cui incolli viene perso (**Annulla** lo ripristina), e una formula che leggeva una di quelle celle mostra **#RIF!**, perché la cella che leggeva non esiste più. Se le celle tagliate sono solo una parte di una cella unita, oppure incolli su una sola parte di una cella unita, non viene spostato nulla: seleziona l'intera cella unita. Quando tagli celle e le incolli in un altro foglio, le formule continuano a leggere le celle che leggevano prima (indicando il foglio). Dopo un taglia e incolla, gli Appunti vengono svuotati.

## Incolla speciale

**Ctrl+Alt+V**, oppure la freccia sotto **Incolla** nella scheda Home, permette di scegliere che cosa incollare:

- **Tutto**, **Formule**, **Valori** o **Formati**. *Valori* trasforma una formula nel numero o nel testo che mostra.
- **Tutto tranne i bordi**: tutto, ma le celle mantengono i bordi che avevano.
- **Formule e formati numero** oppure **Valori e formati numero**: le formule (o i valori) arrivano con il modo in cui sono visualizzati i numeri, come percentuale o data, ma non i colori, i tipi di carattere o i bordi. Le celle mantengono l'aspetto che avevano.
- **Larghezze colonne**: le colonne in cui incolli ricevono le larghezze delle colonne copiate.
- **Commenti e note**: le note delle celle copiate vengono messe nelle celle in cui incolli. Nessun valore di cella cambia.
- Un'**operazione**: **Aggiungi**, **Sottrai**, **Moltiplica** o **Dividi**. I numeri copiati vengono combinati con i numeri già presenti nelle celle. Esempio: le celle contengono 10 e 20, hai copiato 5 e hai scelto *Moltiplica*: diventano 50 e 100.
- **Salta celle vuote**: le celle vuote della copia lasciano invariate le celle su cui cadono.
- **Trasponi**: le righe diventano colonne e le colonne diventano righe.

## Il pulsante Opzioni di incolla

Subito dopo aver incollato celle copiate in JeffCrunch, accanto all'ultima cella incollata compare un piccolo pulsante **Opzioni di incolla**. Fai clic su di esso per scegliere un altro modo di inserire le stesse celle: **Mantieni formattazione di origine** (tutto), **Valori**, **Formule**, **Formattazione**, **Senza bordi**, **Formule e formattazione numeri**, **Valori e formattazione numeri**, **Mantieni larghezze colonne di origine** oppure **Trasponi**. JeffCrunch annulla l'incollamento e lo ripete con il modo scelto. La modifica occupa un solo passaggio: un singolo **Annulla** ripristina l'intero incollamento, per quante volte tu abbia cambiato idea. Esempio: copia A1:B2, dove B1 è **=A1*2**, e incolla in F5. Scegli **Valori** e G5 contiene il numero 2 invece di **=F5*2**; scegli **Mantieni formattazione di origine** e la formula torna.

Da tastiera, dopo un incollamento premi il tasto **Ctrl** (il tasto **Control** su Mac) per aprire il menu del pulsante, quindi usa i tasti freccia e **Invio** per scegliere. Il pulsante scompare quando modifichi una cella o qualsiasi altro elemento della cartella di lavoro, copi un'altra cosa, premi **Esc** oppure fai scorrere fuori vista l'ultima cella incollata (ricompare quando torni a vederla). Non viene proposto dopo un taglia e incolla, né per il testo proveniente da un altro programma.

## Incolla collegamento, Collegamento ipertestuale e Incolla importazione testo

Altre tre scelte si trovano nella freccia sotto **Incolla**. Funzionano sulle celle copiate nella stessa cartella di lavoro, tranne l'ultima.

- **Incolla collegamento** inserisce in ogni cella una formula che punta alla cella copiata. Esempio: copia A1:B2, seleziona D4 e usa **Incolla collegamento**: D4 diventa **=Sheet1!$A$1**. Quando A1 cambia, anche D4 cambia. Una cella copiata vuota mostra 0.
- **Incolla come collegamento ipertestuale** inserisce in ogni cella le stesse parole che mostra la cella copiata, e trasforma ciascuna in un collegamento alla cella copiata. Tieni premuto **Ctrl** e fai clic su una di esse per andare a quella cella. Funziona su un massimo di 2.000 celle alla volta.
- **Incolla importazione testo** serve per il testo proveniente da un altro programma. Apre la stessa finestra che si usa per aprire un file di testo: indichi cosa separa i campi (una virgola, una tabulazione, uno spazio, un punto e virgola, una barra verticale o altro), con cosa sono racchiusi i campi, la prima riga da usare e come leggere ogni colonna (così come è digitata, come testo, come data, oppure esclusa). Alcuni testi allineano i campi in colonne di larghezza uguale, con spazi tra di esse. Seleziona **Le colonne hanno una larghezza fissa** e indica dove inizia ciascuna colonna: ogni riga viene tagliata in quei punti e gli spazi attorno a ciascun campo vengono eliminati. Scegli *testo* per una colonna di codici come 007, così gli zeri restano. I campi vanno nelle celle, a partire dalla cella selezionata, come un unico passaggio che puoi annullare.

## Testo con virgole, punti e virgola o colonne

Quando il testo che incolli da un altro programma è chiaramente una tabella, JeffCrunch apre la stessa finestra che si usa per aprire un file di testo, così puoi controllare come viene diviso prima che qualcosa finisca nelle celle. Questo avviene solo con un normale **Incolla** (**{key:Paste}**), mai con **Incolla speciale** o con una delle sue scelte. Succede quando il testo ha almeno due righe e nessuna tabulazione, e, alternativamente:

- ogni riga ha lo stesso numero di campi, almeno tre, separati da virgole, punti e virgola o barre verticali, con qualcosa in ogni colonna (un campo racchiuso tra virgolette può contenere una virgola); oppure
- i campi sono allineati in colonne di larghezza uguale, con gli stessi spazi tra di esse su ogni riga (almeno tre righe e almeno due spazi).

I numeri con la virgola delle migliaia, come 1,234, e le righe di codice o gli elenchi che terminano con una virgola o un punto e virgola non vengono considerati una tabella. La finestra si apre con il modo in cui il testo è risultato diviso. Se i numeri contengono virgole e i campi sono separati da punti e virgola, la virgola è il separatore decimale. Fai clic su **OK** per inserire i campi, oppure su **Annulla** per non inserire nulla.

Esempio: copia queste righe da un'email.

    nome,qta,prezzo
    Mela,12,1.50
    Pera,3,0.25

Incollale con una terza colonna, ad esempio un prezzo, e la finestra mostra tre colonne, con **Virgola** selezionata. Una frase su una sola riga, righe con un numero diverso di virgole, oppure righe con soltanto due campi vengono incollate come parole, senza aprire la finestra. Il testo con tabulazioni va direttamente nelle celle, come sempre. Per aprire la finestra per un qualsiasi testo, scegli **Incolla importazione testo** (vedi sopra).

Lo stesso avviene quando trascini del testo da un altro programma e lo rilasci sulla griglia: viene inserito a partire dalla cella selezionata. Un rilascio in un altro punto della finestra non produce alcun effetto.

## Imposta incolla predefinito

**Imposta incolla predefinito** è l'ultima scelta nella freccia sotto **Incolla**. Stabilisce che cosa inseriscono il pulsante **Incolla** e **{key:Paste}** quando hai copiato celle: tutto (all'inizio), oppure solo i valori, solo le formule, solo la formattazione e così via. Esempio: scegli *Valori*, e una formula copiata arriva come il numero che mostra. La scelta viene mantenuta per la volta successiva. Non cambia **Incolla speciale** né le scelte del menu, e il testo proveniente da altri programmi viene incollato così com'è. Scegli *Tutto* per tornare alla situazione iniziale.

La stessa scelta si trova nella pagina **Modifica** di **Opzioni**, in due caselle: una per le celle copiate in questa cartella di lavoro, e una per le celle copiate in un'altra cartella di lavoro. Finché non ne scegli una per un'altra cartella di lavoro, segue la prima.

## Il menu Incolla e le immagini

La freccia sotto **Incolla** elenca le scelte abituali, e inoltre **Senza bordi**, **Formule e formattazione numeri**, **Valori e formattazione numeri**, **Mantieni larghezze colonne di origine** e **Commenti e note**. Sono le stesse scelte di Incolla speciale, con un solo clic ciascuna.

La freccia accanto a **Copia** contiene **Copia come immagine** e **Copia come bitmap**. Ciascuna inserisce negli Appunti un'immagine delle celle selezionate, disegnata come le vedi, con le linee della griglia. Una bitmap ha le dimensioni dello schermo. Un'immagine ha il doppio dei punti, quindi resta nitida quando viene ingrandita. **Copia come immagine stampata** disegna le celle come le mostrerebbe la carta: le linee della griglia compaiono solo se l'impostazione di pagina le stampa. Incollala in JeffWrite o JeffDeck come immagine. Se le celle non entrano in un'unica immagine, seleziona un numero minore di celle.

## Tra programmi

Quando copi, JeffCrunch mette le celle negli Appunti anche come testo (celle separate da tabulazioni, una riga per riga) e come tabella per le pagine web, così che altri programmi possano usarle. Quando incolli qualcosa che non proviene da JeffCrunch, ovvero testo con tabulazioni o una tabella da una pagina web o da un altro foglio di calcolo, ogni campo va nella propria cella ed è letto come se lo avessi digitato: numeri, date e formule che iniziano con il segno di uguale. Copiando da un'altra cartella di lavoro aperta in JeffCrunch si ottengono valori, formule e formati numero, ma non le altre formattazioni. Incollale in JeffDeck o JeffPub e diventano una tabella che mantiene i tipi di carattere, le sfumature, i bordi e i numeri così come appaiono (vedi **Incolla celle come tabella** nella rispettiva Guida).

## Che cosa non si può incollare

JeffCrunch ti avvisa e non modifica nulla quando l'incollamento toccherebbe una parte di una **formula matriciale**, la riga con i nomi delle colonne di una **tabella**, celle bloccate di un **foglio protetto**, oltre la fine del foglio, oppure più di 100.000 celle. Una **formula matriciale** intera può essere tagliata e spostata (seleziona tutte le sue celle); una sua parte, una tabella dati o la riga dei nomi di una tabella non possono esserlo.

## Il riquadro Appunti

Il riquadro Appunti conserva le ultime 24 cose copiate o tagliate in questa finestra del programma, così puoi incollarne una più vecchia senza copiarla di nuovo. Fai clic sulla piccola freccia nell'angolo del gruppo **Appunti** nella scheda **Home** (oppure premi **Alt**, poi **H**, poi **FO**) per aprirlo. Il pannello compare accanto alla pagina.

Ogni elemento mostra una piccola anteprima e il tipo di contenuto: testo, testo formattato, celle, un'immagine, forme o diapositive. L'elemento più recente è in cima. Fai clic su un elemento per incollarlo dove si trovano il cursore o la selezione. Viene incollato come qualsiasi altro incollamento: un passaggio di **Annulla** lo ripristina, e il pulsante Opzioni di incolla compare come di consueto.

- **Incolla tutto** incolla ogni elemento, dal più vecchio al più recente, in modo che compaiano nell'ordine in cui li hai copiati.
- **Elimina** rimuove l'elemento scelto. **Cancella tutto** rimuove tutti gli elementi.
- Il riquadro raccoglie elementi solo mentre è aperto. Per continuare a raccogliere dopo averlo chiuso, seleziona **Continua a raccogliere elementi quando questo riquadro è chiuso**. La scelta viene ricordata, e la raccolta inizia all'avvio del programma, prima che tu apra il riquadro.
- Per raccogliere anche ciò che copi in altri programmi, seleziona **Raccogli anche ciò che viene copiato in altri programmi**. Vengono conservati solo testo semplice e immagini, e l'opzione si disattiva di nuovo ogni volta che apri il programma. Con questa opzione attiva, una copia fatta in un altro programma viene letta per intero prima che questo programma decida se conservarla, quindi una copia molto grande può far pausare questo programma per un momento e occupare una quantità di memoria pari alla dimensione della copia. Viene letta una sola forma: il testo, se presente, altrimenti un'unica immagine. Con l'opzione disattivata, nulla di ciò che è copiato in altri programmi viene letto.

Gli elementi sono conservati solo nella memoria del programma. Nulla viene salvato in un file, e l'elenco viene svuotato quando chiudi l'ultima finestra del programma. Un elemento resta nell'elenco anche dopo che il documento da cui proviene è stato chiuso. Un elemento più grande di 8 MB non viene conservato, e il riquadro te lo segnala; un'immagine conta 4 byte per ogni punto (pixel), quindi un'immagine di più di circa 2 milioni di pixel è troppo grande. Tutti gli elementi insieme non possono superare 64 MB: quando si supera il limite vengono eliminati prima quelli più vecchi. Ciò che un gestore di password contrassegna come segreto non viene mai conservato.
