# Taglia, copia e incolla

<!-- keywords: taglia, copia, incolla, appunti, sposta, duplica, speciale, valori, formule, formati, trasponi, salta celle vuote, aggiungi, sottrai, moltiplica, dividi, operazione, unite, testo, scheda, html, pagina web, altro programma, Esc, bordo tratteggiato, formato, larghezza colonne, note, commenti, formati numero, tutti tranne i bordi, immagine, bitmap, copia come immagine -->

**Copia** (**{key:Copy}**) inserisce una copia delle celle selezionate negli Appunti, un'area in cui il computer conserva un elemento alla volta. **Incolla** (**{key:Paste}**) inserisce il contenuto degli Appunti nelle celle, a partire dalla cella selezionata. **Taglia** (**{key:Cut}**) funziona come copia, ma quando si incolla, le celle vengono spostate e la posizione di origine viene svuotata.

Un bordo tratteggiato animato indica le celle copiate o tagliate. Premere **Esc** per eliminare il bordo: le celle non sono più pronte per essere incollate. Anche le celle tagliate vengono rilasciate se si esegue un'altra modifica prima di incollarle (digitazione, inserimento, annullamento): per spostarle, occorre tagliarle di nuovo.

## Incollare all'interno di JeffCrunch

Vengono incollati tutti gli elementi: valori, formule, formati e celle unite. Una formula continua a puntare alle stesse posizioni *relative alla sua posizione*. Esempio: copiare **=A1*2** da B1 e incollarla in B5: diventa **=A5*2**. Un riferimento con il segno del dollaro, ad esempio **$A$1**, resta invariato. Se si seleziona un intervallo che è un multiplo esatto delle dimensioni di ciò che è stato copiato, la copia viene ripetuta per riempirlo: copiare una cella, selezionare venti celle, incollare, e tutte e venti riceveranno il contenuto.

Un numero incollato troppo largo per una colonna che mantiene ancora la larghezza iniziale allarga la colonna.

## Taglia sposta le celle

Quando si tagliano celle e le si incollano, le celle vengono davvero spostate. Ogni formula che puntava a esse punta alla loro nuova posizione. Esempio: B1 contiene **=A1+A2**. Tagliare A1:A2 e incollare in E4: B1 diventa **=E4+E5**. Il contenuto delle celle sovrascritte viene perso (**Annulla** lo ripristina), e una formula che leggeva una di quelle celle mostra **#RIF!**, perché la cella letta non esiste più. Se le celle tagliate sono solo una parte di una cella unita, oppure si incolla solo su una parte di una cella unita, non viene spostato nulla: selezionare l'intera cella unita. Quando si tagliano celle su un altro foglio, le formule continuano a leggere le celle lette in precedenza (indicando il foglio). Dopo un taglia e incolla, gli Appunti vengono svuotati.

## Incolla speciale

**Ctrl+Alt+V**, oppure la freccia sotto **Incolla** nella scheda Home, consente di scegliere che cosa incollare:

- **Tutto**, **Formule**, **Valori** o **Formati**. *Valori* trasforma una formula nel numero o nel testo che visualizza.
- **Tutto tranne i bordi**: tutto, ma le celle mantengono i bordi che avevano.
- **Formule e formati numero**, oppure **Valori e formati numero**: le formule (o i valori) arrivano con il modo in cui sono visualizzati i numeri, ad esempio percentuale o data, ma senza colori, carattere o bordi. Le celle mantengono l'aspetto che avevano.
- **Larghezza colonne**: le colonne in cui si incolla assumono le larghezze delle colonne copiate.
- **Commenti e note**: le note delle celle copiate vengono applicate alle celle in cui si incolla. Nessun valore cambia.
- Un'**operazione**: **Aggiungi**, **Sottrai**, **Moltiplica** o **Dividi**. I numeri copiati vengono combinati con quelli già presenti nelle celle. Esempio: le celle contengono 10 e 20, si copia 5 e si sceglie *Moltiplica*: diventano 50 e 100.
- **Salta celle vuote**: le celle vuote della copia lasciano invariate le celle di destinazione.
- **Trasponi**: le righe diventano colonne e le colonne diventano righe.

## Il menu Incolla e le immagini

La freccia sotto **Incolla** elenca le scelte più comuni e inoltre **Senza bordi**, **Formule e formattazione numeri**, **Valori e formattazione numeri**, **Mantieni larghezze colonne di origine** e **Commenti e note**. Sono le stesse opzioni di Incolla speciale, con un solo clic ciascuna.

La freccia accanto a **Copia** offre **Copia come immagine** e **Copia come bitmap**. Ognuna inserisce negli Appunti un'immagine delle celle selezionate, disegnata come appaiono a schermo, con le linee della griglia. Una bitmap ha le dimensioni dello schermo. Un'immagine ha il doppio dei punti, quindi resta nitida quando viene ingrandita. Incollarla in JeffWrite o JeffDeck come immagine. Se le celle non entrano in un'unica immagine, selezionarne meno.

## Tra programmi

Quando si copia, JeffCrunch inserisce negli Appunti anche le celle come testo (celle separate da tabulazioni, una riga per ogni riga di celle) e come tabella per le pagine Web, in modo che altri programmi possano usarle. Quando si incolla qualcosa che non proviene da JeffCrunch, testo con tabulazioni oppure una tabella da una pagina Web o da un altro foglio di calcolo, ogni elemento va nella propria cella e viene interpretato come se fosse stato digitato: numeri, date e formule che iniziano con il segno di uguale. Copiando da un'altra cartella di lavoro aperta in JeffCrunch si ottengono valori, formule e formati numero, ma non gli altri formati.

## Che cosa non si può incollare

JeffCrunch segnala il problema e non modifica nulla quando l'incollatura interesserebbe una parte di una **formula matriciale**, la riga dei nomi di colonna di una **tabella**, celle bloccate di un **foglio protetto**, oltre la fine del foglio, oppure più di 100.000 celle. Una **formula matriciale** intera può essere tagliata e spostata (selezionare tutte le sue celle); una sua parte, una tabella dati o la riga dei nomi di una tabella no.
