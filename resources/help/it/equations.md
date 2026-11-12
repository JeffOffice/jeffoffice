# Aggiungere un'equazione
<!-- keywords: equazione, equazioni, matematica, formula, frazione, radice quadrata, apice, pedice, esponente, somma, integrale, greco, simbolo, inserisci equazione, alt+=, accento, matrice, parentesi, equazioni predefinite, galleria, modificare un'equazione, modifica equazione, vettore, pdf, jpub, file pub -->

Un'**equazione** è una riga di formula matematica, come una frazione o una radice quadrata. In JeffPub un'equazione è un oggetto della pagina, come un'immagine: puoi spostarla, ridimensionarla, ruotarla e assegnarle un colore.

## Inserire un'equazione nella pagina

1. Fai clic su **Inserisci > Testo > Equazione**, oppure premi **Alt+=**.
2. Digita l'equazione su una sola riga. Sotto la casella vedi come apparirà. Fai clic su **OK**.

L'equazione viene inserita al centro della pagina, selezionata. Per partire da una formula già pronta, fai clic sulla freccia accanto a **Equazione** e scegli un'equazione predefinita, come la formula quadratica o l'area del cerchio.

## Come digitarla

- **Frazioni:** digita una barra. `a/b` è a su b. `(x+1)/(x-1)` considera ogni gruppo racchiuso tra parentesi come un unico elemento.
- **Potenze e pedici:** `x^2` è x al quadrato, e `x_i` è x con una piccola i in basso. Puoi usarli insieme, come in `x_i^2`.
- **Radici:** digita `sqrt(x+1)` per una radice quadrata, oppure `sqrt[3]{x}` per una radice cubica.
- **Somme, integrali e produttorie:** digita `\sum_(i=1)^n i`, `\int_0^1 x` oppure `\prod_(k=1)^n k`. La parte dopo il trattino basso va in basso, quella dopo l'accento circonflesso va in alto.
- **Simboli:** digita una barra rovesciata e un nome. `\alpha` è α, `\pi` è π, `\infty` è ∞, `\leq` è ≤ e `\times` è ×. Funzionano circa 70 nomi.
- **Accenti:** `\hat{x}`, `\vec{v}` e `\bar{x}` mettono un segno sopra la lettera.
- **Parentesi che si adattano:** digita `\left( ... \right)` e le parentesi si allungano fino all'altezza di ciò che c'è tra loro.
- **Matrici:** una **matrice** è una griglia di numeri. Digita `\pmatrix{a & b \\ c & d}`. Inserisci una `&` tra le celle di una riga e due barre rovesciate tra le righe.

## Modificare un'equazione

Seleziona l'equazione e premi Invio, oppure fai doppio clic su di essa, oppure fai clic su **Inserisci > Testo > Equazione**. La casella si apre con l'equazione scritta su una sola riga. Modificala e fai clic su **OK**. Per annullare basta un solo passaggio.

Se un'equazione proveniente da un file contiene parti che la casella non sa scrivere, viene mostrata così com'è. Per modificarla, digita una nuova equazione.

## Spostare, ridimensionare e colorare

Trascina l'equazione per spostarla. Trascina un angolo per ingrandirla o rimpicciolirla: l'equazione mantiene le proporzioni. Usa **Riempimento forma** per cambiare il colore dei caratteri.

## Salvataggio

- **.jpub** conserva l'equazione, così puoi modificarla di nuovo.
- **PDF e stampa** disegnano l'equazione come forme, quindi resta nitida a qualsiasi dimensione.
- **.pub** non supporta le equazioni. JeffPub salva ciascuna equazione come immagine della stessa, nella stessa posizione e con le stesse dimensioni, e te lo segnala al salvataggio.
- **EPUB** e **HTML** conservano l'equazione come la riga che hai digitato.
