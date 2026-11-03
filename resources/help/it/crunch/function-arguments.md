# Immettere una funzione con la finestra di dialogo Argomenti funzione
<!-- keywords: argomenti funzione, argomento, fx, risultato formula, Maiusc+F3, selezionare celle, intervallo, facoltativo, ripetere, modificare una funzione, funzione nidificata, funzione annidata, errore -->

La finestra di dialogo **Argomenti funzione** aiuta a compilare una funzione un argomento alla volta e mostra il risultato della formula prima di inserirla nella cella.

## Aprirla

- Scegliere **Scegli una funzione** nella scheda Formule (**Maiusc+F3**) oppure fare clic su **fx** accanto alla barra della formula, quindi scegliere una funzione. La finestra di dialogo si apre per quella funzione.
- Per modificare una funzione già presente in una formula, posizionare il cursore tra le sue parentesi (nella cella o nella barra della formula) e premere **Maiusc+F3** o fare clic su **fx**. La finestra di dialogo si apre per quella funzione, con i relativi argomenti nelle caselle. In una formula come =SOMMA(A1; SE(B1>2; 5; 6)), con il cursore all'interno di SE si modifica SE, mentre con il cursore in SOMMA ma all'esterno di SE si modifica SOMMA.

## Compilarla

Ogni argomento ha una riga: il nome, una casella in cui digitarlo e, a destra, il suo valore attuale, ad esempio 12,5, "testo", VERO oppure {1;2;3} per un intervallo di celle. Un nome in grassetto è obbligatorio; un nome non in grassetto è facoltativo e può essere lasciato vuoto.

- **Tab** passa alla casella successiva e **Maiusc+Tab** a quella precedente.
- La riga sotto le caselle descrive cosa fa la funzione, mentre la riga successiva descrive l'argomento in cui ci si trova.
- Funzioni come SOMMA accettano tutti gli argomenti necessari: quando si digita nell'ultima casella, ne compare una nuova.
- Il pulsante a destra di una casella consente di indicare le celle. La finestra di dialogo si riduce a una piccola barra e si selezionano le celle nel foglio con il mouse (durante il trascinamento la casella mostra il loro indirizzo). Fare clic sul pulsante della barra o premere **Invio** per tornare alla finestra di dialogo.

## Il risultato

**Risultato formula =** mostra il risultato dell'intera funzione, pochi istanti dopo che si smette di digitare. Nel foglio non viene scritto nulla finché non si preme **OK**. Se c'è un problema, una riga sotto il risultato lo descrive a parole: ad esempio che un valore non è stato trovato, che si sta dividendo per zero o che un numero non è consentito. Il risultato di una formula che richiederebbe troppo tempo per essere calcolata qui non viene mostrato: premere **OK** e il foglio la calcolerà.

**Guida relativa a questa funzione** apre questa pagina.

## Completare

Premere **Invio** o fare clic su **OK** per inserire la funzione. In una cella in cui non è in corso un inserimento, la formula viene scritta nella cella. In una formula che si sta digitando, la funzione viene inserita nella posizione del cursore o sostituisce quella modificata, ed è possibile continuare a digitare. Premendo **Esc** o facendo clic su **Annulla** non viene modificato nulla.
