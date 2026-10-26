# Tagliare, copiare e incollare
<!-- keywords: taglia, tagliare, copia, copiare, incolla, incollare, Appunti, sposta, spostare, duplica, duplicare, incolla speciale, valori, formule, formati, trasponi, trasporre, salta celle vuote, addiziona, sottrai, moltiplica, dividi, operazione, celle unite, testo, tabulazione, html, pagina Web, altro programma, Esc, bordo tratteggiato, formiche in marcia, bordo -->

**Copia** (**{key:Copy}**) inserisce una copia delle celle selezionate negli Appunti, uno spazio in cui il computer conserva un elemento alla volta. **Incolla** (**{key:Paste}**) inserisce il contenuto degli Appunti nelle celle a partire dalla cella selezionata. **Taglia** (**{key:Cut}**) è simile a Copia, ma quando si incolla le celle vengono spostate e la posizione originale viene svuotata.

Un bordo tratteggiato in movimento indica le celle copiate o tagliate. Premere **Esc** per rimuovere il bordo; le celle non sono più pronte per essere incollate. Le celle tagliate vengono rilasciate anche se si apporta qualsiasi altra modifica prima di incollarle (digitazione, inserimento, annullamento): tagliarle di nuovo se si desidera ancora spostarle.

## Incollare in JeffCrunch

Viene incollato tutto: i valori, le formule, i formati e le celle unite. Una formula continua a fare riferimento alle stesse posizioni *rispetto a dove si trova*. Esempio: se si copia **=A1*2** da B1 e la si incolla in B5, diventa **=A5*2**. Una parte con il simbolo del dollaro, ad esempio **$A$1**, rimane invariata. Se si seleziona un blocco le cui dimensioni sono un multiplo esatto di quelle dell'elemento copiato, la copia viene ripetuta per riempirlo: copiare una cella, selezionare venti celle, incollare e tutte e venti la riceveranno.

Un numero incollato troppo largo per una colonna che ha ancora la larghezza iniziale allarga la colonna.

## Taglia sposta le celle

Quando si tagliano e si incollano delle celle, le celle vengono effettivamente spostate. Ogni formula che vi faceva riferimento punta alla loro nuova posizione. Esempio: B1 contiene **=A1+A2**. Se si taglia A1:A2 e si incolla in E4, B1 diventa **=E4+E5**. Il contenuto delle celle su cui si è incollato viene perso (**Annulla** lo ripristina) e una formula che leggeva una di quelle celle mostra **#RIF!**, perché la cella letta non esiste più. Se le celle tagliate sono solo una parte di una cella unita, o se si incolla solo su una parte di essa, non viene spostato nulla: selezionare l'intera cella unita. Quando si tagliano celle e le si incolla in un altro foglio, le formule continuano a leggere le stesse celle di prima (indicando il foglio). Dopo un'operazione di taglia e incolla, gli Appunti vengono svuotati.

## Incolla speciale

**CTRL+ALT+V**, oppure la freccia sotto **Incolla** nella scheda Home, consente di scegliere cosa incollare:

- **Tutto**, **Formule**, **Valori** o **Formati**. *Valori* trasforma una formula nel numero o nel testo che visualizza.
- Un'**operazione**: **Addiziona**, **Sottrai**, **Moltiplica** o **Dividi**. I numeri copiati vengono combinati con i numeri già presenti nelle celle. Esempio: le celle contengono 10 e 20, si è copiato 5 e si è scelto *Moltiplica*: diventano 50 e 100.
- **Salta celle vuote**: le celle vuote della copia lasciano invariate le celle su cui vengono incollate.
- **Trasponi**: le righe diventano colonne e le colonne diventano righe.

## Tra programmi

Quando si copia, JeffCrunch inserisce le celle negli Appunti anche come testo (celle separate da tabulazioni, una riga per linea) e come tabella per pagine Web, in modo che altri programmi possano utilizzarle. Quando si incolla qualcosa che non proviene da JeffCrunch, come testo con tabulazioni o una tabella da una pagina Web o da un altro foglio di calcolo, ogni elemento viene inserito in una cella distinta e interpretato come se fosse stato digitato: numeri, date e formule che iniziano con il segno di uguale. Se si copia da un'altra cartella di lavoro aperta in JeffCrunch, vengono incollati valori, formule e formati numero, ma non la formattazione restante.

## Cosa non è possibile incollare

JeffCrunch avvisa e non modifica nulla quando l'operazione interesserebbe una parte di una **formula matrice**, la riga dei nomi delle colonne di una **tabella**, celle bloccate di un **foglio protetto**, un'area oltre la fine del foglio o più di 100.000 celle. Un'intera **formula matrice** può essere tagliata e spostata (selezionandone tutte le celle); non è invece possibile farlo con una sua parte, con una tabella dati o con la riga dei nomi di una tabella.
