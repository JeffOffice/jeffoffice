# Taglia, copia e incolla
<!-- keywords: taglia, copia, incolla, appunti, sposta, duplica, speciale, valori, formule, formati, trasponi, salta celle vuote, somma, sottrai, moltiplica, dividi, operazione, unite, testo, tabulazione, html, pagina web, altro programma, Esc, bordo tratteggiato, bordo, larghezze colonne, note, commenti, formati numero, tutto tranne i bordi, immagine, bitmap, copia come immagine, immagine stampata, come stampato, incolla collegamento, collegamento ipertestuale, importazione testo, testo delimitato, trascina testo, virgola, punto e virgola, csv, separatore, imposta incolla predefinito, incolla predefinito, larghezza fissa -->

**Copia** (**{key:Copy}**) inserisce negli Appunti una copia delle celle selezionate, un'area che il computer conserva per un elemento alla volta. **Incolla** (**{key:Paste}**) inserisce il contenuto degli Appunti nelle celle, a partire dalla cella selezionata. **Taglia** (**{key:Cut}**) funziona come copia, ma al momento dell'incollaggio le celle vengono spostate e la posizione originale viene svuotata.

Un bordo tratteggiato animato indica le celle copiate o tagliate. Premi **Esc** per eliminare il bordo: in questo caso le celle non sono più pronte per essere incollate. Anche le celle tagliate vengono rilasciate se, prima di incollarle, esegui un'altra modifica (digitazione, inserimento, annullamento): se vuoi comunque spostarle, tagliale di nuovo.

## Incolla all'interno di JeffCrunch

Vengono incollati tutti gli elementi: valori, formule, formati e celle unite. Una formula mantiene i riferimenti alle celle *relativamente alla propria posizione*. Esempio: copia **=A1*2** da B1 e incollala in B5; diventa **=A5*2**. Un riferimento con il segno del dollaro, come **$A$1**, resta invariato. Se selezioni un intervallo che è un multiplo esatto delle dimensioni di ciò che hai copiato, la copia viene ripetuta per riempirlo: copia una cella, seleziona venti celle, incolla, e tutte e venti riceveranno il contenuto.

Un numero incollato troppo largo per una colonna che ha ancora la larghezza iniziale allarga la colonna.

## Taglia sposta le celle

Quando tagli delle celle e le incolli, le celle vengono effettivamente spostate. Ogni formula che puntava a esse punta ora alla nuova posizione. Esempio: B1 contiene **=A1+A2**. Se tagli A1:A2 e incolli in E4, B1 diventa **=E4+E5**. Il contenuto delle celle sovrascritte viene perso (**Annulla** lo ripristina), e una formula che leggeva una di quelle celle mostra **#RIF!**, perché la cella letta non esiste più. Se le celle tagliate sono solo una parte di una cella unita, oppure se incolli su una sola parte di una cella unita, non si sposta nulla: seleziona l'intera cella unita. Quando tagli celle e le incolli in un altro foglio, le formule continuano a leggere le celle che leggevano prima (indicando il foglio). Dopo un taglia e incolla gli Appunti vengono svuotati.

## Incolla speciale

**Ctrl+Alt+V**, oppure la freccia sotto **Incolla** nella scheda Home, consente di scegliere cosa incollare:

- **Tutto**, **Formule**, **Valori** o **Formati**. *Valori* trasforma una formula nel numero o nel testo che mostra.
- **Tutto tranne i bordi**: tutto, ma le celle mantengono i bordi che avevano.
- **Formule e formati numero** oppure **Valori e formati numero**: le formule (o i valori) arrivano insieme al modo in cui sono mostrati i numeri, come percentuale o data, ma non i colori, i caratteri o i bordi. Le celle mantengono l'aspetto che avevano.
- **Larghezze colonne**: le colonne in cui incolli ricevono le larghezze delle colonne copiate.
- **Commenti e note**: le note delle celle copiate vengono applicate alle celle in cui incolli. Nessun valore di cella cambia.
- Un'**operazione**: **Somma**, **Sottrai**, **Moltiplica** o **Dividi**. I numeri copiati vengono combinati con quelli già presenti nelle celle. Esempio: le celle contengono 10 e 20, hai copiato 5 e hai scelto *Moltiplica*: diventano 50 e 100.
- **Salta celle vuote**: le celle vuote della copia lasciano invariate le celle su cui vengono incollate.
- **Trasponi**: le righe diventano colonne e le colonne diventano righe.

## Incolla collegamento, Incolla come collegamento ipertestuale e Importazione testo

Altre tre opzioni si trovano nella freccia sotto **Incolla**. Funzionano con celle copiate nella stessa cartella di lavoro, tranne l'ultima.

- **Incolla collegamento** inserisce in ogni cella una formula che punta alla cella copiata. Esempio: copia A1:B2, seleziona D4 e usa **Incolla collegamento**: D4 diventa **=Foglio1!$A$1**. Quando cambia A1, cambia anche D4. Una cella copiata vuota mostra 0.
- **Incolla come collegamento ipertestuale** inserisce in ogni cella le stesse parole che mostra la cella copiata, e le trasforma in un collegamento alla cella copiata. Tieni premuto **Ctrl** e fai clic su una di esse per andare a quella cella. Funziona su un massimo di 2.000 celle alla volta.
- **Importazione testo** serve per il testo proveniente da un altro programma. Apre la stessa finestra che si usa per aprire un file di testo: indichi cosa separa i campi (virgola, tabulazione, spazio, punto e virgola, barra verticale o altro), con quale carattere sono racchiusi i campi, la prima riga da usare e come leggere ogni colonna (così come digitata, come testo, come data, oppure da escludere). Alcuni testi allineano i campi in colonne della stessa larghezza, separate da spazi. Seleziona **Le colonne hanno larghezza fissa** e indica dove inizia ogni colonna: ogni riga viene tagliata in quei punti e gli spazi attorno a ogni campo vengono eliminati. Scegli *Testo* per una colonna di codici come 007, così gli zeri non vengono persi. I campi vengono inseriti nelle celle, a partire dalla cella selezionata, in un'unica operazione che puoi annullare.

## Testo con virgole, punti e virgola o colonne

Quando il testo che incolli da un altro programma è chiaramente una tabella, JeffCrunch apre la stessa finestra usata per aprire un file di testo, così puoi verificare come viene diviso prima che qualcosa finisca nelle celle. Questo accade solo con **Incolla** normale (**{key:Paste}**), mai con **Incolla speciale** o con una delle sue opzioni. Succede quando il testo ha almeno due righe e nessuna tabulazione, e vale una di queste condizioni: ogni riga ha lo stesso numero di campi, almeno tre, separati da virgole, punti e virgola o barre verticali, con un contenuto in ogni colonna (un campo tra virgolette può contenere una virgola); oppure i campi sono allineati in colonne della stessa larghezza, con gli stessi spazi di separazione su ogni riga (almeno tre righe e almeno due separazioni). Numeri con separatore delle migliaia, come 1,234, e righe di codice o elenchi che terminano con una virgola o un punto e virgola non vengono riconosciuti come tabella. La finestra si apre con il modo in cui il testo è stato riconosciuto come diviso. Se i numeri contengono virgole e i campi sono separati da punti e virgola, la virgola è il separatore decimale. Fai clic su **OK** per inserire i campi, oppure su **Annulla** per non inserire nulla.

Esempio: copia queste righe da un messaggio di posta elettronica.

    nome,qtà,prezzo
    Mela,12,1.50
    Pera,3,0.25

Se le incolli con una terza colonna, come un prezzo, la finestra mostra tre colonne, con **Virgola** selezionata. Una frase su una sola riga, righe con un numero diverso di virgole, oppure righe con soltanto due campi vengono incollate come parole, senza finestra. Il testo con tabulazioni va direttamente nelle celle, come sempre. Per aprire la finestra per qualsiasi testo, scegli **Importazione testo** (vedi sopra).

Lo stesso accade quando trascini del testo da un altro programma e lo rilasci sulla griglia: viene inserito a partire dalla cella selezionata. Un rilascio in un'altra parte della finestra non fa nulla.

## Imposta incolla predefinito

**Imposta incolla predefinito** è l'ultima opzione nella freccia sotto **Incolla**. Stabilisce cosa inseriscono il pulsante **Incolla** e **{key:Paste}** quando hai copiato delle celle: tutto (all'inizio), oppure solo i valori, solo le formule, solo la formattazione e così via. Esempio: scegli *Valori*, e una formula copiata diventa il numero che mostra. La scelta viene mantenuta per la volta successiva. Non modifica **Incolla speciale** né le opzioni del menu, e il testo proveniente da altri programmi viene incollato così com'è. Scegli *Tutto* per ripristinare il comportamento originale.

## Il menu Incolla e le immagini

La freccia sotto **Incolla** elenca le opzioni consuete e inoltre **Senza bordi**, **Formule e formattazione numeri**, **Valori e formattazione numeri**, **Mantieni larghezze colonne di origine** e **Commenti e note**. Sono le stesse opzioni di Incolla speciale, con un solo clic ciascuna.

La freccia accanto a **Copia** contiene **Copia come immagine** e **Copia come bitmap**. Ciascuna inserisce negli Appunti un'immagine delle celle selezionate, disegnata come le vedi, con la griglia. Una bitmap ha le dimensioni dello schermo. Un'immagine ha il doppio dei punti, quindi resta nitida quando viene ingrandita. **Copia come immagine stampata** disegna le celle come le mostrerebbe la carta: la griglia compare solo se l'impostazione di pagina prevede di stamparla. Incollala in JeffWrite o JeffDeck come immagine. Se le celle non entrano in un'unica immagine, seleziona un intervallo più piccolo.

## Tra programmi

Quando copi, JeffCrunch inserisce negli Appunti anche le celle come testo (celle separate da tabulazioni, una riga per riga) e come tabella per le pagine web, così altri programmi possono utilizzarle. Quando incolli qualcosa che non proviene da JeffCrunch, come testo con tabulazioni o una tabella tratta da una pagina web o da un altro foglio di calcolo, ogni campo finisce nella propria cella e viene letto come se lo avessi digitato: numeri, date e formule che iniziano con il segno di uguale. Copiando da un'altra cartella di lavoro aperta in JeffCrunch vengono portati valori, formule e formati numero, ma non il resto della formattazione. Incollali in JeffDeck o JeffPub e diventano una tabella che conserva i caratteri, le sfumature, i bordi e i numeri così come appaiono (vedi **Incolla celle come tabella** nella Guida di quei programmi).

## Ciò che non può essere incollato

JeffCrunch lo segnala e non modifica nulla quando l'incollaggio interesserebbe una parte di una **formula matrice**, la riga dei nomi di colonna di una **tabella**, celle bloccate di un **foglio protetto**, oltre la fine del foglio, oppure più di 100.000 celle. Una formula matrice completa può essere tagliata e spostata (seleziona tutte le sue celle); una parte di essa, una tabella dati o la riga dei nomi di una tabella non possono esserlo.
