# Aggiungere un'equazione
<!-- keywords: scheda equazione, strutture, simboli, converti, equazione, equazioni, matematica, formula, frazione, radice quadrata, apice, pedice, esponente, sommatoria, integrale, greco, simbolo, inserisci equazione, alt+=, accento, matrice, parentesi, equazioni predefinite, galleria, modifica equazione, vettore, pdf, jpub, file pub -->

Un'**equazione** è una formula matematica, come una frazione o una radice quadrata. In JeffPub un'equazione è un oggetto sulla pagina, come un'immagine: puoi spostarla, ridimensionarla, ruotarla e assegnarle un colore.

## Inserire un'equazione nella pagina

1. Fai clic su **Inserisci > Testo > Equazione**, oppure premi **Alt+=**.
2. Digita l'equazione su una riga. Sotto la casella vedi come apparirà. Fai clic su **OK**.

L'equazione viene inserita al centro della pagina, già selezionata. Per partire da una formula nota, fai clic sulla freccia accanto a **Equazione** e scegli un'equazione predefinita, come la formula risolutiva dell'equazione di secondo grado o l'area del cerchio.

## Come digitarla

- **Frazioni:** digita una barra. `a/b` è a su b. `(x+1)/(x-1)` tratta ogni gruppo tra parentesi come un unico elemento.
- **Potenze e pedici:** `x^2` è x al quadrato, e `x_i` è x con una i piccola in basso. Puoi usarli entrambi, come in `x_i^2`.
- **Radici:** digita `sqrt(x+1)` per una radice quadrata, oppure `sqrt[3]{x}` per una radice cubica.
- **Sommatorie, integrali e produttorie:** digita `\sum_(i=1)^n i`, `\int_0^1 x` oppure `\prod_(k=1)^n k`. La parte dopo il trattino basso va in basso, quella dopo il cursore va in alto.
- **Simboli:** digita una barra rovesciata e un nome. `\alpha` è α, `\pi` è π, `\infty` è ∞, `\leq` è ≤ e `\times` è ×. Funzionano circa 70 nomi.
- **Accenti:** `\hat{x}`, `\vec{v}` e `\bar{x}` mettono un segno sopra la lettera.
- **Parentesi che si allungano:** digita `\left( ... \right)`, e le parentesi si adattano all'altezza di ciò che c'è tra loro.
- **Matrici:** una **matrice** è una griglia di numeri. Digita `\pmatrix{a & b \\ c & d}`. Metti una `&` tra le celle di una riga e due barre rovesciate tra le righe.

## Digitarla in UnicodeMath o LaTeX

Accanto alla casella c'è un elenco di formati di digitazione: **Riga semplice** (quello usato sopra), **UnicodeMath** e **LaTeX**. Sono due modi di scrivere la matematica come testo semplice, già noti a molte persone. Quando scegli un altro formato, quanto hai digitato viene riscritto in quel formato.

- **UnicodeMath:** `a/b` è una frazione, `x^2_i` ha un apice e un pedice, `√(x)` è una radice, e `∑_(i=1)^n i` è una sommatoria. Racchiudi le parole tra virgolette, come in `"se x"`, e restano testo normale invece di lettere matematiche. `(a|b)` crea parentesi con due parti, e `\matrix(a&b@c&d)` crea una matrice (`&` tra le celle, `@` tra le righe).
- **LaTeX:** `\frac{a}{b}` è una frazione, `\sqrt[3]{x}` è una radice cubica, `\sum_{i=1}^n i` è una sommatoria, `\text{se x}` è testo normale, e `\begin{pmatrix} a & b \\ c & d \end{pmatrix}` è una matrice.

I due pulsanti **Professionale** e **Lineare** cambiano il modo in cui la casella mostra l'equazione. Professionale la disegna impaginata, come apparirà. Lineare mostra solo la riga che hai digitato. Se un'equazione non può essere scritta nel formato scelto, la casella lo segnala e conserva quanto digitato.

## Modificare un'equazione

Seleziona l'equazione e premi Invio, oppure fai doppio clic su di essa, oppure fai clic su **Inserisci > Testo > Equazione**. La casella si apre con l'equazione scritta su una riga. Modificala e fai clic su **OK**. Per annullare basta un passaggio.

Un'equazione proveniente da un file si apre in UnicodeMath, oppure in LaTeX quando UnicodeMath non può scriverla. Se nessuno dei due può farlo, la casella si apre con il testo dell'equazione. Fai clic su **OK** senza modifiche, oppure su **Annulla**, per lasciare l'equazione com'è, oppure digita una nuova formula per sostituirla.

## La scheda Equazione

Seleziona un'equazione e alla fine della barra multifunzione compare una scheda chiamata **Equazione**. Scompare quando selezioni altro.

- **Strumenti:** **Modifica equazione** apre la casella. **Professionale** e **Lineare** cambiano il modo in cui l'equazione selezionata appare sulla pagina: Professionale la disegna impaginata, Lineare mostra la riga in cui è stata digitata. La scelta viene conservata con l'equazione nel file `.jpub`, quindi è la stessa quando riapri il file. Un file `.pub` riceve l'immagine dell'equazione così come appare sulla pagina. **Testo normale** apre la casella con una coppia di virgolette aggiunta, per le parole che non sono matematica.
- **Converti:** scegli il formato in cui si apre la casella: **Riga semplice**, **UnicodeMath** oppure **LaTeX**. La volta successiva che modifichi un'equazione, sarà scritta nel formato scelto. Un'equazione che nessuna riga del tipo semplice può scrivere, come una sommatoria con i limiti sopra e sotto, si apre invece in UnicodeMath.
- **Simboli:** lettere greche, operatori, relazioni, frecce, insiemi e logica. Scegli un simbolo e la casella si apre con il simbolo aggiunto alla riga.
- **Strutture:** frazione, script, radicale, integrale, operatore grande, parentesi, funzione, accento, limite, operatore e matrice. Fai clic su un pulsante per la sua forma abituale, oppure sulla freccia accanto per altre forme, come una derivata, una radice con indice o una matrice identità.

Una struttura viene inserita alla fine della riga, con lettere di esempio, e la prima è selezionata in modo da poterla sovrascrivere. La casella mostra le stesse strutture in una fila di piccoli pulsanti sopra la riga. Qui la struttura viene inserita nel punto del cursore e, se una parte della riga è selezionata, quella parte va nel primo posto della struttura.

## Spostare, ridimensionare e colorare

Trascina l'equazione per spostarla. Trascina un angolo per ingrandirla o rimpicciolirla: l'equazione mantiene le sue proporzioni. Usa **Riempimento forma** per cambiare il colore dei caratteri.

## Salvataggio

- **.jpub** conserva l'equazione, così puoi modificarla di nuovo.
- **PDF e stampa** disegnano l'equazione come forme, quindi resta nitida a qualsiasi dimensione.
- **.pub** non supporta le equazioni. JeffPub salva ciascuna equazione come immagine, nella stessa posizione e con le stesse dimensioni, e te lo segnala al salvataggio.
- **EPUB** e **HTML** riportano l'equazione come la riga che hai digitato.
