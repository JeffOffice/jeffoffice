# Taglia, copia e incolla

<!-- keywords: taglia, copia, incolla, appunti, sposta, duplica, speciale, valori, formule, formati, trasponi, salta celle vuote, somma, sottrai, moltiplica, dividi, operazione, unite, testo, tabulazione, html, pagina web, altro programma, Esc, bordo tratteggiato, larghezza colonne, note, commenti, formati numero, tutto tranne i bordi, immagine, bitmap, copia come immagine, immagine stampata, come stampato, incolla collegamento, collegamento ipertestuale, importazione testo, dividi testo, separatore -->

**Copia** (**{key:Copy}**) mette una copia delle celle selezionate negli Appunti, un'area in cui il computer conserva un elemento alla volta. **Incolla** (**{key:Paste}**) inserisce il contenuto degli Appunti nelle celle, a partire dalla cella selezionata. **Taglia** (**{key:Cut}**) funziona come copia, ma quando si incolla le celle vengono spostate e la posizione di partenza viene svuotata.

Un bordo tratteggiato animato indica le celle copiate o tagliate. Premere **Esc** per eliminare il bordo: le celle non sono più pronte per essere incollate. Le celle tagliate vengono inoltre rilasciate se si esegue un'altra modifica prima di incollarle (digitazione, inserimento, annullamento): per spostarle, occorre tagliarle di nuovo.

## Incollare all'interno di JeffCrunch

Vengono incollati tutti gli elementi: valori, formule, formati e celle unite. Una formula continua a far riferimento alle stesse posizioni *relativamente alla propria posizione*. Ad esempio, se si copia **=A1*2** da B1 e la si incolla in B5, diventa **=A5*2**. Una parte preceduta da un segno di dollaro, come **$A$1**, resta invariata. Se si seleziona un intervallo che è un multiplo esatto delle dimensioni di ciò che è stato copiato, la copia viene ripetuta per riempirlo: si copia una cella, si selezionano venti celle, si incolla, e tutte e venti ricevono il contenuto.

Un numero incollato che è troppo largo per una colonna che mantiene ancora la larghezza iniziale allarga la colonna.

## Taglia sposta le celle

Quando si tagliano celle e le si incollano, le celle vengono davvero spostate. Ogni formula che puntava a esse punta alla loro nuova posizione. Ad esempio, B1 contiene **=A1+A2**. Se si tagliano A1:A2 e si incollano in E4, B1 diventa **=E4+E5**. Il contenuto delle celle su cui si incolla viene eliminato (**Annulla** lo ripristina), e una formula che leggeva una di quelle celle mostra **#RIF!**, perché la cella che leggeva non esiste più. Se le celle tagliate sono solo una parte di una cella unita, oppure si incolla solo su una parte di una cella unita, non viene spostato nulla: selezionare l'intera cella unita. Quando si tagliano celle e le si incollano su un altro foglio, le formule continuano a leggere le celle che leggevano prima (ne indicano il foglio). Dopo un'operazione di taglia e incolla, gli Appunti vengono svuotati.

## Incolla speciale

**Ctrl+Alt+V**, oppure la freccia sotto **Incolla** nella scheda Home, consente di scegliere che cosa incollare:

- **Tutto**, **Formule**, **Valori** o **Formati**. *Valori* trasforma una formula nel numero o nel testo che mostra.
- **Tutto tranne i bordi**: tutto, ma le celle mantengono i bordi che avevano.
- **Formule e formati numero** oppure **Valori e formati numero**: le formule (o i valori) vengono incollati insieme al modo in cui sono visualizzati i numeri, come percentuale o data, ma non i colori, i tipi di carattere o i bordi. Le celle mantengono l'aspetto che avevano.
- **Larghezze colonne**: le colonne su cui si incolla assumono le larghezze delle colonne copiate.
- **Commenti e note**: le note delle celle copiate vengono applicate alle celle su cui si incolla. Nessun valore di cella cambia.
- Un'**operazione**: **Somma**, **Sottrai**, **Moltiplica** o **Dividi**. I numeri copiati vengono combinati con quelli già presenti nelle celle. Esempio: le celle contengono 10 e 20, si copia 5 e si sceglie *Moltiplica*: diventano 50 e 100.
- **Salta celle vuote**: le celle vuote della copia lasciano invariate le celle su cui cadono.
- **Trasponi**: le righe diventano colonne e le colonne diventano righe.

## Incolla collegamento, Incolla come collegamento ipertestuale e Incolla importazione testo

Altre tre scelte si trovano nella freccia sotto **Incolla**. Funzionano con celle copiate nella stessa cartella di lavoro, tranne l'ultima.

- **Incolla collegamento** inserisce in ogni cella una formula che punta alla cella copiata. Esempio: si copia A1:B2, si seleziona D4 e si usa **Incolla collegamento**: D4 diventa **=Foglio1!$A$1**. Quando A1 cambia, anche D4 cambia di conseguenza. Una cella copiata vuota mostra 0.
- **Incolla come collegamento ipertestuale** inserisce in ogni cella le stesse parole che mostra la cella copiata, rendendo ciascuna un collegamento alla cella copiata. Tenere premuto **Ctrl** e fare clic su una di esse per passare a quella cella. Funziona con un massimo di 2.000 celle alla volta.
- **Incolla importazione testo** serve per il testo proveniente da un altro programma. Apre la stessa finestra che si usa per aprire un file di testo: si indica che cosa separa i campi (una virgola, una tabulazione, uno spazio, un punto e virgola, una barra verticale o altro), che cosa racchiude un campo, la prima riga da usare e come leggere ciascuna colonna (così come è digitata, come testo, come data oppure esclusa). Scegliere *testo* per una colonna di codici come 007, così gli zeri vengono mantenuti. I campi vanno nelle celle, a partire dalla cella selezionata, come un'unica operazione che si può annullare.

## Il menu Incolla e le immagini

La freccia sotto **Incolla** elenca le scelte più usate, e inoltre **Nessun bordo**, **Formule e formattazione numero**, **Valori e formattazione numero**, **Mantieni larghezze colonne di origine** e **Commenti e note**. Sono le stesse scelte di Incolla speciale, con un solo clic per ciascuna.

La freccia accanto a **Copia** offre **Copia come immagine** e **Copia come bitmap**. Ciascuna mette nell'area di appunti un'immagine delle celle selezionate, disegnata come appaiono a schermo, con la griglia. Una bitmap ha le dimensioni dello schermo. Un'immagine ha il doppio dei punti, quindi resta nitida quando viene ingrandita. **Copia come immagine stampata** disegna le celle come le mostrerebbe la carta: la griglia compare solo se l'impostazione di pagina prevede di stamparla. Incollarla in JeffWrite o JeffDeck come immagine. Se le celle non entrano in una sola immagine, selezionarne meno.

## Tra programmi

Quando si copia, JeffCrunch mette le celle negli Appunti anche come testo (celle separate da tabulazioni, una riga per linea) e come tabella per le pagine web, così che altri programmi possano usarle. Quando si incolla qualcosa che non proviene da JeffCrunch, come testo con tabulazioni, oppure una tabella da una pagina web o da un altro foglio di calcolo, ogni elemento va nella propria cella e viene letto come se fosse stato digitato: numeri, date e formule che iniziano con un segno di uguale. Copiando da un'altra cartella di lavoro aperta in JeffCrunch si ottengono valori, formule e formati numero, ma non gli altri formati.

## Che cosa non si può incollare

JeffCrunch avvisa e non modifica nulla quando l'incollatura interesserebbe una parte di una **formula matriciale**, la riga dei nomi di colonna di una **tabella**, celle bloccate di un **foglio protetto**, oltre la fine del foglio, oppure più di 100.000 celle. Un'intera **formula matriciale** può essere tagliata e spostata (selezionare tutte le sue celle); una sua parte, una tabella dati o la riga dei nomi di una tabella non possono esserlo.
