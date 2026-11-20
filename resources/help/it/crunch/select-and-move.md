# Selezionare celle e spostarsi

<!-- keywords: selezione, seleziona, selezionare, cella, celle, intervallo, riga, colonna, clic, trascina, trascinamento, maiusc, ctrl, freccia, frecce, home, fine, pagina su, pagina giù, tab, invio, intera riga, intera colonna, casella nome, vai a, sposta celle, bordo trascinamento, scorrimento, barra di stato, somma, media, conteggio, conta -->

**Selezionare** celle significa scegliere quelle a cui si applicherà il comando successivo. Le celle selezionate sono evidenziate in un colore più chiaro e hanno un bordo verde. La **cella attiva** è quella con il cursore e rimane bianca all'interno della selezione.

## Con il mouse

- **Fai clic** su una cella per selezionarla. Fai clic all'interno di una cella unita per selezionare l'intera cella unita.
- **Trascina** sulle celle per selezionare un blocco. Se trascini oltre il bordo della finestra, il foglio scorre.
- **Maiusc+clic** su un'altra cella per selezionare dalla cella attiva fino a quella.
- Fai clic su una **lettera di colonna** o su un **numero di riga** per selezionare l'intera colonna o riga. Trascina sulle lettere o sui numeri, oppure usa Maiusc+clic, per selezionarne più di una.
- Fai clic sull'**angolo** in cui si incontrano le lettere e i numeri per selezionare tutto il foglio.
- Tieni premuto **Ctrl** mentre fai clic o trascini per **aggiungere un intervallo** alla selezione. Gli intervalli aggiunti sono evidenziati e quello con il cursore ha il bordo verde. **Canc** elimina tutti gli intervalli in un'unica operazione. Un comando che lavora su un solo intervallo alla volta, come Copia, Riempi, Ordina, Trova nella selezione, Sostituisci tutto o una formattazione, te lo segnala e non modifica nulla; fai clic una volta per tornare a un solo intervallo. Passare a un altro foglio oppure inserire o eliminare righe o colonne riporta anch'essi a un solo intervallo.
- **Trascina il bordo** della selezione per spostare le celle in un'altra posizione. Posiziona il puntatore sul bordo verde finché non assume la forma di quattro frecce. Un contorno tratteggiato mostra dove finiranno le celle. Tutto ciò che fa riferimento alle celle, come una formula, le segue. Tieni premuto **Ctrl** mentre trascini per copiare le celle anziché spostarle. Tieni premuto **Maiusc** mentre trascini per inserire le celle tra altre celle invece di sovrascriverle. Per fare lo stesso senza mouse, taglia o copia le celle e usa **Inserisci celle tagliate o copiate** (vedi [Inserire ed eliminare](insert-delete)). Se fai clic sul bordo senza spostare il mouse, la cella viene semplicemente selezionata.
- Ruota la **rotellina del mouse** per scorrere di tre righe. Tieni premuto **Maiusc** per scorrere in orizzontale, oppure **Ctrl** per lo zoom.

## Con la tastiera

I tasti freccia spostano la cella attiva, saltando le righe e le colonne nascoste. Tieni premuto **Maiusc** per estendere la selezione. **Ctrl** con un tasto freccia porta al margine dei dati: alla fine di una serie di celle che contengono qualcosa oppure, a partire dalla fine di una serie, alla cella successiva che contiene qualcosa. Vedi [Usare JeffCrunch con la tastiera](keyboard) per l'elenco completo dei tasti.

**Ctrl+A** seleziona il blocco di celle intorno alla cella attiva, delimitato da righe e colonne vuote; premilo di nuovo per selezionare l'intero foglio. Se la selezione è già più grande di quel blocco, Ctrl+A seleziona subito l'intero foglio. **Ctrl+Maiusc+\*** seleziona solo il blocco. Premi **Fine**, poi un tasto freccia, per andare al margine dei dati come fa Ctrl con la freccia; la barra di stato mostra **FINE** finché non premi il tasto freccia, un altro tasto o fai clic.

## Vai a una cella

Digita un indirizzo nella **Casella Nome**, ad esempio **C5**, **A1:D10**, **B:D** o **3:7**, e premi **Invio**. Premi **F5** (oppure Ctrl+G) per aprire **Vai a** e digitare lì l'indirizzo. Separa gli indirizzi con virgole, ad esempio **A1:B2,D4**, per selezionare più intervalli contemporaneamente.

## I numeri della selezione

Quando selezioni due o più celle, la barra di stato mostra la **media**, il **conteggio** e la **somma** delle celle. Il conteggio è il numero di celle che contengono qualcosa; media e somma usano solo i numeri. Le righe e le colonne nascoste non vengono considerate. Per copiare la selezione come testo, premi **Ctrl+C**.

## Il messaggio della barra di stato

A sinistra della barra di stato compare **Pronto** quando puoi lavorare, **Invio** mentre scrivi in una cella, **Modifica** dopo aver premuto F2, **Punto** quando puoi fare clic o usare le frecce per inserire l'indirizzo di una cella in una formula, e **FINE** dopo aver premuto il tasto Fine. Quando è attivo un filtro, la barra indica anche quante righe ha lasciato il filtro, ad esempio **Trovati 3 di 8 record**.
