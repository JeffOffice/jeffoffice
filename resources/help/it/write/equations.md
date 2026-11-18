# Equazioni
<!-- keywords: scheda equazione, strutture, simboli, converti, equazione, equazioni, matematica, formula, frazione, radice quadrata, apice, pedice, esponente, somma, integrale, greco, simbolo, inserisci equazione, alt+=, accento, cappuccio, vettore, barra, matrice, parentesi, equazioni predefinite, galleria, modifica equazione, modificare un'equazione -->

Un'**equazione** è una riga di formula matematica, ad esempio una frazione o una radice quadrata. Questo argomento spiega come digitarne una e inserirla in un documento.

## Inserire un'equazione

1. Posiziona il cursore dove deve comparire l'equazione.
2. Fai clic su **Inserisci > Equazione**, oppure premi **Alt+=**.
3. Digita l'equazione su una sola riga. Sotto la casella vedi l'equazione disegnata come apparirà, e la riga di testo che mostrerà la pagina. Per partire da un'equazione nota, scegline una dall'elenco **Equazioni predefinite** nella casella. Fai clic su **OK**.

L'equazione viene salvata nel documento nella forma standard per la matematica, in modo che anche altri programmi possano aprirla.

## Come digitarla

- **Frazioni:** digita una barra. `a/b` è a diviso b. `(x+1)/(x-1)` tratta ogni gruppo tra parentesi come un unico elemento. Funziona anche `\frac{1}{2}`.
- **Potenze:** digita un accento circonflesso. `x^2` è x al quadrato. `e^(i*pi)` usa il gruppo tra parentesi.
- **Pedici:** digita un trattino basso. `x_i` è x con una i piccola in basso. Puoi usarli insieme, come in `x_i^2`.
- **Radici:** digita `sqrt(x+1)` per una radice quadrata. Digita `sqrt[3]{x}` per una radice cubica.
- **Somme, integrali e produttorie:** digita `\sum_(i=1)^n i`, `\int_0^1 x` oppure `\prod_(k=1)^n k`. La parte dopo il trattino basso va in basso (o alla base del simbolo di integrale), e la parte dopo l'accento circonflesso va in alto.
- **Simboli:** digita una barra rovesciata e un nome. `\alpha` è α, `\pi` è π, `\infty` è ∞, `\leq` è ≤, `\neq` è ≠, `\times` è ×, `\pm` è ±, e `\to` è →. Funzionano circa 70 nomi.
- **Accenti:** `\hat{x}`, `\check{x}`, `\tilde{x}`, `\acute{x}`, `\grave{x}`, `\breve{x}`, `\dot{x}`, `\ddot{x}`, `\bar{x}` e `\vec{v}` mettono un segno sopra la lettera. `\overline{ab}` disegna una linea sopra il gruppo, e `\underline{ab}` una linea sotto.
- **Parentesi che si allungano:** digita `\left( ... \right)`, e le parentesi si adattano all'altezza di ciò che c'è in mezzo. Usa una parentesi qualsiasi dopo `\left` e `\right`, oppure un punto per omettere un lato, come in `\left[ x \right.`.
- **Matrici:** una **matrice** è una griglia di numeri. Digita `\matrix{a & b \\ c & d}`. Metti una `&` tra le celle di una riga e due barre rovesciate tra le righe. Usa `\pmatrix` per parentesi tonde attorno alla griglia, `\bmatrix` per parentesi quadre e `\vmatrix` per linee verticali.
- **Funzioni:** `sin`, `cos`, `tan`, `log`, `ln`, `exp`, `lim`, `min` e `max` mantengono intero il proprio nome.

Nella pagina, JeffWrite mostra un'equazione come una riga di testo. L'equazione nel file conserva però le sue frazioni e radici, così che altri programmi la impaginino correttamente.

## Digitarla in UnicodeMath o LaTeX

Accanto alla casella c'è un elenco di formati di digitazione: **Riga semplice** (quello descritto sopra), **UnicodeMath** e **LaTeX**. Sono due modi di scrivere la matematica come testo semplice, già noti a molti utenti. Quando scegli un altro formato, quanto hai digitato viene riscritto in quel formato.

- **UnicodeMath:** `a/b` è una frazione, `x^2_i` ha una potenza e un pedice, `√(x)` è una radice, e `∑_(i=1)^n i` è una somma. Scrivi le parole tra virgolette, come `"se x"`, e restano testo normale invece di lettere matematiche. `(a|b)` crea parentesi con due parti, e `\matrix(a&b@c&d)` crea una matrice (`&` tra le celle, `@` tra le righe).
- **LaTeX:** `\frac{a}{b}` è una frazione, `\sqrt[3]{x}` è una radice cubica, `\sum_{i=1}^n i` è una somma, `\text{se x}` è testo normale, e `\begin{pmatrix} a & b \\ c & d \end{pmatrix}` è una matrice.

I due pulsanti **Professionale** e **Lineare** cambiano il modo in cui la casella mostra l'equazione. Professionale la disegna costruita, come apparirà. Lineare mostra solo la riga che hai digitato. Se un'equazione non può essere scritta nel formato scelto, la casella lo segnala e mantiene quanto digitato.

## La scheda Equazione

Seleziona un'equazione (fai clic su di essa, oppure sposta il cursore sopra con Maiusc), e alla fine della barra multifunzione compare una scheda chiamata **Equazione**. Scompare quando selezioni altro.

- **Strumenti:** **Modifica equazione** apre la casella. **Professionale** e **Lineare** scelgono come si apre la casella: costruita oppure solo come riga. Il documento, di per sé, mostra sempre un'equazione come riga di testo. **Testo normale** apre la casella con una coppia di virgolette aggiunta, per parole che non sono matematica.
- **Converti:** scegli il formato in cui si apre la casella: **Riga semplice**, **UnicodeMath** o **LaTeX**. La volta successiva che modifichi un'equazione, verrà scritta nel formato scelto. Un'equazione che nessuna riga del tipo semplice può scrivere, come una somma con i limiti sopra e sotto, si apre invece in UnicodeMath.
- **Simboli:** lettere greche, operatori, relazioni, frecce, insiemi e logica. Scegli un simbolo, e la casella si apre con il simbolo aggiunto alla riga.
- **Strutture:** frazione, script, radicale, integrale, operatore a grandi dimensioni, parentesi, funzione, accento, limite, operatore e matrice. Fai clic su un pulsante per la sua forma abituale, oppure sulla freccia accanto per altre forme, come una derivata, una radice con indice o una matrice identità.

Una struttura viene inserita alla fine della riga, con lettere di esempio al suo interno, e la prima è selezionata così da poterci scrivere sopra. La casella ha le stesse strutture in una fila di piccoli pulsanti sopra la riga. Lì una struttura viene inserita nel punto del cursore e, se una parte della riga è selezionata, quella parte finisce nel primo posto della struttura.

## Equazioni predefinite

Fai clic su **Inserisci > Equazioni predefinite** per vedere un elenco di equazioni note, come la formula risolutiva dell'equazione di secondo grado e l'area del cerchio. Fai clic su una di esse per inserirla nel punto del cursore.

## Modificare un'equazione

Posiziona il cursore accanto a un'equazione, oppure selezionala, e fai clic su **Inserisci > Equazione**. La casella si apre con l'equazione scritta su una riga. Modificala e fai clic su **OK**. Per annullare basta un passaggio.

## Equazioni in altri documenti

JeffWrite mostra le equazioni contenute in un documento e le conserva così come erano. Un'equazione creata da un altro programma si apre in UnicodeMath, oppure in LaTeX quando UnicodeMath non riesce a scriverla, così da poterla modificare. Se nessuno dei due può scrivere tutte le sue parti, ad esempio i segni di raggruppamento, la casella parte dal testo dell'equazione. Fai clic su **Annulla** per conservare l'equazione così com'è, oppure digita una nuova equazione per sostituirla.
