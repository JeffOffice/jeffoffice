# Taglia, copia e incolla

<!-- keywords: taglia, copia, incolla, appunti, sposta, duplica, incolla speciale, valori, formule, formati, trasponi, salta celle vuote, aggiungi, sottrai, moltiplica, dividi, operazione, unite, testo, tabulazione, html, pagina web, altro programma, Esc, bordo tratteggiato, bordo, larghezze colonne, note, commenti, formati numero, tutti tranne i bordi, immagine, bitmap, copia come immagine, immagine stampata, come stampato, incolla collegamento, collegamento ipertestuale, importazione testo, dividi testo, separatore, imposta incolla predefinito, incolla predefinito, larghezza fissa -->

**Copia** (**{key:Copy}**) inserisce negli Appunti una copia delle celle selezionate, un'area in cui il computer conserva un elemento alla volta. **Incolla** (**{key:Paste}**) inserisce il contenuto degli Appunti nelle celle, a partire dalla cella selezionata. **Taglia** (**{key:Cut}**) funziona come copia, ma al momento dell'incollamento le celle vengono spostate e la posizione di origine viene svuotata.

Un bordo tratteggiato animato indica le celle copiate o tagliate. Premi **Esc** per eliminare il bordo: le celle non sono più pronte per essere incollate. Anche le celle tagliate vengono rilasciate se, prima dell'incollamento, esegui un'altra modifica (digitazione, inserimento, annullamento): tagliale di nuovo se vuoi ancora spostarle.

## Incollare all'interno di JeffCrunch

Vengono incollati tutti gli elementi: valori, formule, formati e celle unite. Una formula mantiene il riferimento alle stesse celle *relativamente alla propria posizione*. Esempio: copia **=A1*2** da B1 e incollala in B5, e diventa **=A5*2**. Un riferimento con il segno del dollaro, come **$A$1**, rimane invariato. Se selezioni un intervallo che è un multiplo esatto delle dimensioni di ciò che hai copiato, la copia viene ripetuta per riempirlo: copia una cella, seleziona venti celle, incolla, e tutte e venti riceveranno il contenuto.

Un numero incollato troppo largo per una colonna che mantiene ancora la larghezza iniziale allarga la colonna.

## Taglia sposta le celle

Quando tagli celle e le incolli, le celle vengono davvero spostate. Ogni formula che si riferiva a esse punta alla loro nuova posizione. Esempio: B1 contiene **=A1+A2**. Taglia A1:A2 e incolla in E4: B1 diventa **=E4+E5**. Il contenuto delle celle sovrascritte viene eliminato (**Annulla** lo ripristina), e una formula che leggeva una di quelle celle mostra **#RIF!**, perché la cella letta non esiste più. Se le celle tagliate sono solo una parte di una cella unita, o se incolli solo su una parte di una cella unita, non viene spostato nulla: seleziona l'intera cella unita. Quando tagli celle e le incolli in un altro foglio, le formule continuano a leggere le celle che leggevano prima (ne indicano il foglio). Dopo un taglia e incolla gli Appunti vengono svuotati.

## Incolla speciale

**Ctrl+Alt+V**, oppure la freccia sotto **Incolla** nella scheda Home, consente di scegliere che cosa incollare:

- **Tutto**, **Formule**, **Valori** o **Formati**. *Valori* trasforma una formula nel numero o nel testo che mostra.
- **Tutto tranne i bordi**: tutto, ma le celle mantengono i bordi che avevano.
- **Formule e formati numero** oppure **Valori e formati numero**: le formule (o i valori) vengono incollati insieme al modo in cui sono mostrati i numeri, come percentuale o data, ma non i colori, i tipi di carattere o i bordi. Le celle mantengono l'aspetto che avevano.
- **Larghezze colonne**: le colonne in cui incolli ricevono le larghezze delle colonne copiate.
- **Commenti e note**: le note delle celle copiate vengono inserite nelle celle in cui incolli. Nessun valore di cella cambia.
- Un'**operazione**: **Aggiungi**, **Sottrai**, **Moltiplica** o **Dividi**. I numeri copiati vengono combinati con i numeri già presenti nelle celle. Esempio: le celle contengono 10 e 20, hai copiato 5 e hai scelto *Moltiplica*: diventano 50 e 100.
- **Salta celle vuote**: le celle vuote della copia lasciano invariate le celle su cui atterrano.
- **Trasponi**: le righe diventano colonne e le colonne diventano righe.

## Incolla collegamento, Incolla come collegamento ipertestuale e Incolla importazione testo

Altre tre scelte si trovano nella freccia sotto **Incolla**. Funzionano con celle copiate nella stessa cartella di lavoro, tranne l'ultima.

- **Incolla collegamento** inserisce in ogni cella una formula che punta alla cella copiata. Esempio: copia A1:B2, seleziona D4 e usa **Incolla collegamento**: D4 diventa **=Foglio1!$A$1**. Quando A1 cambia, anche D4 cambia. Una cella copiata vuota mostra 0.
- **Incolla come collegamento ipertestuale** inserisce in ogni cella le stesse parole mostrate dalla cella copiata e le rende un collegamento alla cella copiata. Tieni premuto **Ctrl** e fai clic su una di esse per andare a quella cella. Funziona su un massimo di 2.000 celle alla volta.
- **Incolla importazione testo** serve per il testo proveniente da un altro programma. Apre la stessa finestra che si usa per aprire un file di testo: indichi che cosa separa i campi (virgola, tabulazione, spazio, punto e virgola, barra verticale o altro), che cosa racchiude un campo, la prima riga da usare e come leggere ogni colonna (così come digitata, come testo, come data oppure esclusa). Alcuni testi allineano i campi in colonne della stessa larghezza, con spazi tra l'una e l'altra. Seleziona **Le colonne hanno larghezza fissa** e indica dove inizia ogni colonna: ogni riga viene tagliata in quei punti e gli spazi attorno a ogni campo vengono eliminati. Scegli *Testo* per una colonna di codici come 007, così gli zeri vengono mantenuti. I campi vengono inseriti nelle celle, a partire dalla cella selezionata, come un'unica operazione che puoi annullare.

## Imposta incolla predefinito

**Imposta incolla predefinito** è l'ultima scelta della freccia sotto **Incolla**. Stabilisce che cosa inseriscono il pulsante **Incolla** e **{key:Paste}** quando hai copiato delle celle: tutto (all'inizio), oppure solo i valori, solo le formule, solo la formattazione e così via. Esempio: scegli *Valori*, e una formula copiata diventa il numero che mostra. La scelta viene mantenuta per la volta successiva. Non modifica **Incolla speciale** né le scelte del menu, e il testo proveniente da altri programmi viene incollato così com'è. Scegli *Tutto* per tornare all'impostazione iniziale.

## Il menu Incolla e le immagini

La freccia sotto **Incolla** elenca le scelte abituali, e inoltre **Senza bordi**, **Formule e formattazione numeri**, **Valori e formattazione numeri**, **Mantieni larghezze colonne di origine** e **Commenti e note**. Sono le stesse scelte di Incolla speciale, con un solo clic ciascuna.

La freccia accanto a **Copia** contiene **Copia come immagine** e **Copia come bitmap**. Ciascuna inserisce negli Appunti un'immagine delle celle selezionate, disegnata come le vedi, con la griglia. Una bitmap ha le dimensioni dello schermo. Un'immagine ha il doppio dei punti, quindi resta nitida quando viene ingrandita. **Copia come immagine stampata** disegna le celle come le mostrerebbe la carta: la griglia compare solo se l'impostazione di pagina la stampa. Incollala in JeffWrite o JeffDeck come immagine. Se le celle non entrano in una sola immagine, seleziona un intervallo più piccolo.

## Tra programmi

Quando copi, JeffCrunch inserisce le celle negli Appunti anche come testo (celle separate da tabulazioni, una riga per riga) e come tabella per le pagine web, così che altri programmi possano usarle. Quando incolli qualcosa che non proviene da JeffCrunch, come testo con tabulazioni oppure una tabella da una pagina web o da un altro foglio di calcolo, ogni campo va nella propria cella e viene letto come se fosse stato digitato: numeri, date e formule che iniziano con il segno di uguale. Copiando da un'altra cartella di lavoro aperta in JeffCrunch si ottengono valori, formule e formati numero, ma non le altre formattazioni.

## Che cosa non si può incollare

JeffCrunch avvisa e non modifica nulla quando l'incollamento interesserebbe una parte di una **formula matriciale**, la riga dei nomi di colonna di una **tabella**, celle bloccate di un **foglio protetto**, oltre la fine del foglio, oppure più di 100.000 celle. Una **formula matriciale** intera può essere tagliata e spostata (seleziona tutte le sue celle); una sua parte, una tabella dati o la riga dei nomi di una tabella non possono esserlo.
