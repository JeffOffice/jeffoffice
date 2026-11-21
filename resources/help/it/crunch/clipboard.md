# Taglia, copia e incolla

<!-- keywords: taglia, copia, incolla, appunti, sposta, duplica, speciale, valori, formule, formati, trasponi, salta celle vuote, somma, sottrai, moltiplica, dividi, operazione, unite, testo, tabulazione, html, pagina web, altro programma, Esc, bordo tratteggiato, bordo, larghezze colonne, note, commenti, formati numero, tutti tranne i bordi, immagine, bitmap, copia come immagine, immagine stampata, come stampato, incolla collegamento, collegamento ipertestuale, importazione testo, testo delimitato, trascina testo, virgola, punto e virgola, csv, separatore, imposta incolla predefinito, incolla predefinito, larghezza fissa, copy, paste, clipboard -->

**Copia** (**{key:Copy}**) mette una copia delle celle selezionate negli Appunti, un'area di memoria in cui il computer conserva un elemento alla volta. **Incolla** (**{key:Paste}**) inserisce il contenuto degli Appunti nelle celle, a partire dalla cella selezionata. **Taglia** (**{key:Cut}**) funziona come copia, ma al momento dell'incolla le celle vengono spostate e la posizione originale viene svuotata.

Un bordo tratteggiato animato indica le celle copiate o tagliate. Premi **Esc** per eliminare il bordo; le celle non sono più pronte per essere incollate. Anche le celle tagliate vengono rilasciate se, prima dell'incolla, apporti un'altra modifica (digitando, inserendo o annullando): se vuoi ancora spostarle, tagliale di nuovo.

## Incollare all'interno di JeffCrunch

Viene incollato tutto: i valori, le formule, i formati e le celle unite. Una formula mantiene il riferimento alle stesse celle *relativamente alla propria posizione*. Esempio: copia **=A1*2** da B1 e incollala in B5: diventa **=A5*2**. Un riferimento con il segno del dollaro, come **$A$1**, resta invariato. Se selezioni un blocco che è un multiplo esatto delle dimensioni di ciò che hai copiato, la copia viene ripetuta per riempirlo: copia una cella, seleziona venti celle, incolla, e tutte e venti ricevono il contenuto.

Un numero incollato che è troppo largo per una colonna con la larghezza iniziale allarga la colonna.

## Taglia sposta le celle

Quando tagli delle celle e le incolli, le celle vengono davvero spostate. Ogni formula che puntava a quelle celle punta ora alla loro nuova posizione. Esempio: B1 contiene **=A1+A2**. Taglia A1:A2 e incolla in E4: B1 diventa **=E4+E5**. Ciò che si trovava nelle celle sovrascritte viene eliminato (**Annulla** lo ripristina), e una formula che leggeva una di quelle celle mostra **#RIF!**, perché la cella letta non esiste più. Se le celle tagliate sono solo una parte di una cella unita, o se incolli su una sola parte di una cella unita, non viene spostato nulla: seleziona l'intera cella unita. Quando tagli celle e le incolli in un altro foglio, le formule continuano a leggere le celle che leggevano prima (indicando il foglio). Dopo un taglia e incolla gli Appunti vengono svuotati.

## Incolla speciale

**Ctrl+Alt+V**, oppure la freccia sotto **Incolla** nella scheda Home, consente di scegliere che cosa incollare:

- **Tutto**, **Formule**, **Valori** o **Formati**. *Valori* trasforma una formula nel numero o nel testo che mostra.
- **Tutto tranne i bordi**: tutto, ma le celle mantengono i bordi che avevano.
- **Formule e formati numero** oppure **Valori e formati numero**: le formule (o i valori) arrivano insieme al modo in cui sono mostrati i numeri, come percentuale o data, ma non i colori, i tipi di carattere o i bordi. Le celle mantengono l'aspetto che avevano.
- **Larghezze colonne**: le colonne in cui incolli ricevono le larghezze delle colonne copiate.
- **Commenti e note**: le note delle celle copiate vengono inserite nelle celle in cui incolli. Nessun valore di cella cambia.
- Un'**operazione**: **Somma**, **Sottrai**, **Moltiplica** o **Dividi**. I numeri copiati vengono combinati con quelli già presenti nelle celle. Esempio: le celle contengono 10 e 20, hai copiato 5 e hai scelto *Moltiplica*: diventano 50 e 100.
- **Salta celle vuote**: le celle vuote nella copia lasciano invariate le celle su cui cadono.
- **Trasponi**: le righe diventano colonne e le colonne diventano righe.

## Il pulsante Opzioni di incolla

Subito dopo aver incollato celle copiate in JeffCrunch, compare un piccolo pulsante **Opzioni di incolla** accanto all'ultima cella incollata. Fai clic per scegliere un altro modo di inserire le stesse celle: **Mantieni formattazione origine** (tutto), **Valori**, **Formule**, **Formattazione**, **Nessun bordo**, **Formule e formattazione numeri**, **Valori e formattazione numeri**, **Mantieni larghezze colonne di origine**, oppure **Trasponi**. JeffCrunch annulla l'incolla e lo ripete in quel modo. La modifica è un'unica operazione: un solo **Annulla** annulla l'intero incolla, indipendentemente da quante volte hai cambiato idea. Esempio: copia A1:B2, dove B1 è **=A1*2**, e incolla in F5. Scegli **Valori** e G5 contiene il numero 2 invece di **=F5*2**; scegli **Mantieni formattazione origine** e la formula torna quella di prima.

Da tastiera, dopo un incolla premi il tasto **Ctrl** (il tasto **Control** su un Mac) per aprire il menu del pulsante, poi usa i tasti freccia e **Invio** per scegliere. Il pulsante scompare quando modifichi una cella o qualsiasi altro elemento della cartella di lavoro, copi qualcos'altro, premi **Esc** oppure fai scorrere fuori vista l'ultima cella incollata (ricompare quando torni a scorrere fino a essa). Non viene offerto dopo un taglia e incolla, né per il testo proveniente da un altro programma.

## Incolla collegamento, Incolla come collegamento ipertestuale e Importazione testo

Altre tre scelte si trovano nella freccia sotto **Incolla**. Funzionano sulle celle copiate nella stessa cartella di lavoro, tranne l'ultima.

- **Incolla collegamento** inserisce in ogni cella una formula che punta alla cella copiata. Esempio: copia A1:B2, seleziona D4 e usa **Incolla collegamento**: D4 diventa **=Foglio1!$A$1**. Quando A1 cambia, anche D4 cambia. Una cella copiata vuota mostra 0.
- **Incolla come collegamento ipertestuale** inserisce in ogni cella le stesse parole che mostra la cella copiata e le trasforma in un collegamento alla cella copiata. Tieni premuto **Ctrl** e fai clic su una di esse per andare a quella cella. Funziona su un massimo di 2.000 celle alla volta.
- **Importazione testo** serve per il testo proveniente da un altro programma. Apre la stessa finestra che si usa per aprire un file di testo: indichi che cosa separa i vari elementi (una virgola, una tabulazione, uno spazio, un punto e virgola, una barra verticale o altro), che cosa racchiude un elemento tra virgolette, la prima riga da usare e come leggere ogni colonna (così com'è digitata, come testo, come data, oppure da escludere). Alcuni testi allineano i propri elementi in colonne della stessa larghezza, con spazi tra l'uno e l'altro. Seleziona **Le colonne hanno una larghezza fissa** e indica dove inizia ogni colonna: ogni riga viene tagliata in quei punti e gli spazi attorno a ciascun elemento vengono eliminati. Scegli *Testo* per una colonna di codici come 007, così gli zeri restano. Gli elementi vengono inseriti nelle celle, a partire dalla cella selezionata, come un'unica operazione annullabile.

## Testo con virgole, punti e virgola o colonne

Quando il testo che incolli da un altro programma è chiaramente una tabella, JeffCrunch apre la stessa finestra che si usa per aprire un file di testo, così puoi controllare come viene suddiviso prima che qualcosa finisca nelle celle. Ciò accade solo con un normale **Incolla** (**{key:Paste}**), mai con **Incolla speciale** o con una delle sue scelte. Succede quando il testo ha almeno due righe e nessuna tabulazione, e in più si verifica una delle due condizioni: ogni riga ha lo stesso numero di elementi, almeno tre, separati da virgole, punti e virgola o barre verticali, con qualcosa in ogni colonna (un elemento tra virgolette può contenere una virgola); oppure gli elementi sono allineati in colonne della stessa larghezza, con le stesse spaziature di spazi in ogni riga (almeno tre righe e almeno due spazi). I numeri con separatore delle migliaia, come 1,234, e le righe di codice o gli elenchi che terminano con una virgola o un punto e virgola non vengono interpretati come tabella. La finestra si apre con la suddivisione del testo rilevata. Se i numeri contengono virgole e gli elementi sono separati da punti e virgola, la virgola è il separatore decimale. Fai clic su **OK** per inserire gli elementi, oppure su **Annulla** per non inserire nulla.

Esempio: copia queste righe da un'email.

    nome,qtà,prezzo
    Mela,12,1.50
    Pera,3,0.25

Incollale con una terza colonna, ad esempio un prezzo, e la finestra mostra tre colonne, con **Virgola** selezionata. Una frase su una sola riga, righe con un numero diverso di virgole, o righe con soli due elementi vengono incollate come parole, senza finestra. Il testo con tabulazioni va direttamente nelle celle, come sempre. Per aprire la finestra con un testo qualsiasi, scegli **Importazione testo** (vedi sopra).

Lo stesso accade quando trascini del testo da un altro programma e lo rilasci sulla griglia: viene inserito a partire dalla cella selezionata. Un rilascio in qualsiasi altro punto della finestra non produce alcun effetto.

## Imposta incolla predefinito

**Imposta incolla predefinito** è l'ultima scelta nella freccia sotto **Incolla**. Stabilisce che cosa inseriscono il pulsante **Incolla** e **{key:Paste}** quando hai copiato delle celle: tutto (all'inizio), oppure solo i valori, solo le formule, solo la formattazione, e così via. Esempio: scegli *Valori*, e una formula copiata arriva come il numero che mostra. La scelta viene mantenuta per la volta successiva. Non modifica **Incolla speciale** né le scelte del menu, e il testo proveniente da altri programmi viene incollato così com'è. Scegli *Tutto* per tornare alla situazione iniziale.

La stessa scelta si trova nella pagina **Modifica** di **Opzioni**, in due caselle: una per le celle copiate in questa cartella di lavoro e una per le celle copiate in un'altra cartella di lavoro. Finché non ne scegli una per un'altra cartella di lavoro, segue la prima.

## Il menu Incolla e le immagini

La freccia sotto **Incolla** elenca le scelte abituali e inoltre **Nessun bordo**, **Formule e formattazione numeri**, **Valori e formattazione numeri**, **Mantieni larghezze colonne di origine** e **Commenti e note**. Sono le stesse scelte di Incolla speciale, con un solo clic ciascuna.

La freccia accanto a **Copia** contiene **Copia come immagine** e **Copia come bitmap**. Ciascuna mette nei controlli degli Appunti un'immagine delle celle selezionate, disegnata come le vedi, con la griglia. Una bitmap ha le dimensioni dello schermo. Un'immagine ha il doppio dei punti, quindi resta nitida quando viene ingrandita. **Copia come immagine stampata** disegna le celle come le mostrerebbe la carta: la griglia compare solo se l'impostazione della pagina prevede di stamparla. Incollala in JeffWrite o JeffDeck come immagine. Se le celle non entrano in un'unica immagine, seleziona un intervallo più piccolo.

## Tra programmi

Quando copi, JeffCrunch mette le celle negli Appunti anche come testo (celle separate da tabulazioni, una riga per ogni riga della tabella) e come tabella per le pagine web, in modo che altri programmi possano utilizzarle. Quando incolli qualcosa che non proviene da JeffCrunch, testo con tabulazioni oppure una tabella da una pagina web o da un altro foglio di calcolo, ogni elemento viene inserito nella propria cella e interpretato come se lo avessi digitato: numeri, date e formule che iniziano con il segno di uguale. Copiando da un'altra cartella di lavoro aperta in JeffCrunch si ottengono valori, formule e formati numero, ma non gli altri formati. Incollali in JeffDeck o JeffPub e arrivano come una tabella che conserva i tipi di carattere, le sfumature, i bordi e i numeri così come appaiono (vedi **Incollare celle come tabella** nella Guida di quei programmi).

## Ciò che non può essere incollato

JeffCrunch lo segnala e non modifica nulla quando l'incolla andrebbe a interferire con una parte di una **formula matriciale**, con la riga dei nomi di colonna di una **tabella**, con celle bloccate di un **foglio protetto**, oltre la fine del foglio o in più di 100.000 celle. Un'intera **formula matriciale** può essere tagliata e spostata (seleziona tutte le sue celle); una sua parte, una tabella dati o la riga dei nomi di una tabella non possono esserlo.
