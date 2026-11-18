# Equazioni
<!-- keywords: scheda equazione, strutture, simboli, converti, equazione, equazioni, matematica, formula, frazione, radice quadrata, apice, pedice, esponente, somma, integrale, greco, simbolo, inserisci equazione, accento, matrice, parentesi, equazioni predefinite, raccolta, modifica equazione, modificare equazione, ods, immagine, pdf -->

Un'**equazione** è una riga di formula matematica, come una frazione o una radice quadrata. In JeffCrunch un'equazione è un oggetto sopra le celle, come un'immagine o una casella di testo. Non è una cella: non modifica alcun numero del foglio.

## Inserire un'equazione nel foglio

1. Fai clic su **Inserisci > Simboli > Equazione**.
2. Digita l'equazione su una sola riga. Sotto la casella vedi come apparirà. Fai clic su **OK**.

L'equazione viene inserita nella cella in cui ti trovi, selezionata. Per partire da una formula già pronta, fai clic su **Equazioni predefinite** e scegline una, ad esempio la formula risolutiva dell'equazione di secondo grado o l'area del cerchio.

## Come digitarla

- **Frazioni:** digita una barra. `a/b` è a su b. `(x+1)/(x-1)` considera ogni gruppo tra parentesi come un unico elemento.
- **Potenze e pedici:** `x^2` è x al quadrato, e `x_i` è x con una i piccola in basso.
- **Radici:** digita `sqrt(x+1)` per una radice quadrata.
- **Somme, integrali e prodotti:** digita `\sum_(i=1)^n i`, `\int_0^1 x` oppure `\prod_(k=1)^n k`.
- **Simboli:** digita una barra rovesciata e un nome. `\alpha` è α, `\pi` è π, `\infty` è ∞ e `\leq` è ≤.
- **Accenti, parentesi e matrici:** `\hat{x}`, `\left( ... \right)` e `\pmatrix{a & b \\ c & d}`.

## Digitarla in UnicodeMath o LaTeX

Accanto alla casella c'è un elenco di formati di digitazione: **Linea semplice** (quello descritto sopra), **UnicodeMath** e **LaTeX**. Sono due modi di scrivere la matematica come testo semplice, già noti a molte persone. Quando scegli un altro formato, quanto hai digitato viene riscritto in quel formato.

- **UnicodeMath:** `a/b` è una frazione, `x^2_i` contiene una potenza e un pedice, `√(x)` è una radice, e `∑_(i=1)^n i` è una somma. Racchiudi le parole tra virgolette, come `"se x"`, e restano testo normale invece di diventare lettere matematiche. `(a|b)` crea parentesi con due parti, e `\matrix(a&b@c&d)` crea una matrice (`&` tra le celle, `@` tra le righe).
- **LaTeX:** `\frac{a}{b}` è una frazione, `\sqrt[3]{x}` è una radice cubica, `\sum_{i=1}^n i` è una somma, `\text{se x}` è testo normale, e `\begin{pmatrix} a & b \\ c & d \end{pmatrix}` è una matrice.

I due pulsanti **Professionale** e **Lineare** cambiano il modo in cui la casella mostra l'equazione. Professionale la disegna costruita, come apparirà. Lineare mostra solo la riga digitata. Se un'equazione non può essere scritta nel formato scelto, la casella lo segnala e conserva quanto hai digitato.

## La scheda Equazione

Quando selezioni un'equazione, alla fine della barra multifunzione compare una scheda chiamata **Equazione**. Scompare quando selezioni altro.

- **Strumenti:** **Modifica equazione** apre la casella. **Professionale** e **Lineare** scelgono come si apre la casella: costruita oppure solo la riga. **Testo normale** apre la casella con una coppia di virgolette aggiunta, per le parole che non sono formule.
- **Converti:** scegli il formato in cui si apre la casella: **Linea semplice**, **UnicodeMath** o **LaTeX**. La volta successiva che modifichi un'equazione, verrà scritta nel formato scelto. Un'equazione che nessuna riga del tipo semplice può scrivere, come una somma con i limiti sopra e sotto, si apre invece in UnicodeMath.
- **Simboli:** lettere greche, operatori, relazioni, frecce, insiemi e logica. Scegli un simbolo e la casella si apre con il simbolo aggiunto alla riga.
- **Strutture:** frazione, script, radicale, integrale, operatore grande, parentesi, funzione, accento, limite, operatore e matrice. Fai clic su un pulsante per la sua forma abituale, oppure sulla freccia accanto per altre forme, come una derivata, una radice con indice o una matrice identità.

Una struttura viene inserita alla fine della riga, con lettere di esempio al suo interno, e la prima è selezionata così puoi digitare sopra. La casella mostra le stesse strutture in una fila di piccoli pulsanti sopra la riga. Qui la struttura viene inserita nel punto del cursore e, se una parte della riga è selezionata, quella parte finisce nella prima posizione della struttura.

## Modificare, spostare e ridimensionare

Fai doppio clic sull'equazione, oppure selezionala e fai clic su **Inserisci > Simboli > Equazione**, per aprire la casella con la sua riga. Modificala e fai clic su **OK**. Per annullare basta un passaggio. Trascina l'equazione per spostarla, e trascina una maniglia per ridimensionarla; l'equazione mantiene le proporzioni all'interno della casella.

## Salvataggio

- **.xlsx** conserva l'equazione come equazione, nella forma standard per la matematica, con un'immagine accanto per i programmi che non sanno leggerla. Riaprendo il file, l'equazione viene mostrata di nuovo. Un'equazione creata in un altro programma viene mostrata allo stesso modo; se la sua matematica non può essere letta, vedi la sua immagine.
- **PDF e stampa** disegnano l'equazione come forme, quindi resta nitida a qualsiasi dimensione.
- Le copie **.ods** non contengono equazioni. JeffCrunch salva ciascuna come immagine nello stesso punto e, quando ti avvisa di ciò che la copia non conserva, indica le equazioni per nome.
