# Selezionare celle e spostarsi

<!-- keywords: seleziona, selezione, cella, intervallo, riga, colonna, clic, trascina, maiusc, ctrl, freccia, home, fine, pagina su, pagina giù, tab, invio, intera riga, intera colonna, casella nome, vai a, sposta celle, trascina bordo, scorri, barra di stato, somma, media, conteggio -->

**Selezionare** celle significa scegliere quelle a cui si applicherà il comando successivo. Le celle selezionate sono evidenziate con un colore di sfondo e un bordo verde intorno. La **cella attiva** è quella con il cursore e rimane bianca all'interno della selezione.

## Con il mouse

- **Fai clic** su una cella per selezionarla. Fai clic all'interno di una cella unita per selezionare l'intera cella unita.
- **Trascina** su più celle per selezionare un blocco. Se trascini oltre il bordo della finestra, il foglio scorre.
- **Maiusc+clic** su un'altra cella per selezionare dalla cella attiva fino a quel punto.
- Fai clic su una **lettera di colonna** o su un **numero di riga** per selezionare l'intera colonna o riga. Trascina sulle lettere o sui numeri, oppure usa Maiusc+clic, per selezionarne più di una.
- Fai clic sull'**angolo** in cui si incontrano le lettere e i numeri per selezionare tutto il foglio.
- Tieni premuto **Ctrl** mentre fai clic o trascini per **aggiungere un intervallo** alla selezione. Gli intervalli aggiunti sono evidenziati e quello con il cursore ha il bordo verde. **Canc** elimina tutti gli intervalli in un'unica operazione. Un comando che lavora su un solo intervallo alla volta, come Copia, Riempi, Ordina, Trova nella selezione, Sostituisci tutto o un formato, lo segnala e non modifica nulla; fai clic una volta per tornare a un solo intervallo. Passare a un altro foglio, oppure inserire o eliminare righe o colonne, riporta anch'esso a un solo intervallo.
- **Trascina il bordo** della selezione per spostare le celle in un altro punto. Posiziona il puntatore sul bordo verde finché non assume la forma di quattro frecce. Un contorno tratteggiato indica dove atterreranno le celle. Tutto ciò che fa riferimento alle celle, ad esempio una formula, le segue. Tieni premuto **Ctrl** mentre trascini per copiare le celle invece di spostarle. Per spostare le celle su un altro foglio, inizia a trascinare il bordo, poi tieni premuto **Alt** e rilascia il pulsante sulla scheda di quel foglio: le celle finiscono nelle stesse posizioni di quel foglio (se tieni premuto anche **Ctrl**, vengono copiate lì). Tenere premuto **Alt** prima di premere il pulsante del mouse non avvia questo tipo di trascinamento. Le celle che si trovano già nella posizione di destinazione vengono sovrascritte, come accade quando si spostano celle sullo stesso foglio, e **Annulla** ripristina entrambi i fogli in un solo passaggio. Il foglio su cui rilasci deve consentire l'operazione: un foglio protetto e le celle che tagliano celle unite, una tabella o una tabella pivot vengono rifiutati, senza modificare nulla. Un foglio nascosto non ha scheda, e il pulsante **Nuovo foglio** non è una scheda, quindi nessuno dei due riceve celle. Premere e rilasciare **Alt** durante il trascinamento non visualizza i suggerimenti tasti. Per fare lo stesso da tastiera, taglia le celle, vai all'altro foglio e incolla. Tieni premuto **Maiusc** mentre trascini per inserire le celle tra altre celle invece di sovrascriverle. Per fare lo stesso senza mouse, taglia o copia le celle e usa **Inserisci celle tagliate o copiate** (vedi [Inserire ed eliminare](insert-delete)). Se fai clic sul bordo senza spostare il mouse, la cella viene selezionata normalmente.
- Ruota la **rotella del mouse** per scorrere di tre righe. Tieni premuto **Maiusc** per scorrere in orizzontale, oppure tieni premuto **Ctrl** per lo zoom.

## Con la tastiera

I tasti freccia spostano la cella attiva, saltando le righe e le colonne nascoste. Tieni premuto **Maiusc** per estendere la selezione. **Ctrl** con una freccia va al limite dei dati: alla fine di una serie di celle che contengono qualcosa oppure, dalla fine di una serie, alla cella successiva che contiene qualcosa. Consulta [Usare JeffCrunch con la tastiera](keyboard) per tutti i tasti.

**Ctrl+A** seleziona il blocco di celle intorno alla cella attiva, delimitato da righe e colonne vuote; premilo di nuovo per selezionare l'intero foglio. Se la selezione è già più grande di quel blocco, Ctrl+A seleziona subito l'intero foglio. **Ctrl+Maiusc+*** seleziona solo il blocco. Premi **Fine**, poi una freccia, per andare al limite dei dati come fa **Ctrl** con la freccia; la barra di stato mostra **Fine** finché non premi una freccia, un altro tasto o non fai clic.

## Vai a una cella

Digita un indirizzo nella **Casella Nome**, ad esempio **C5**, **A1:D10**, **B:D** o **3:7**, e premi **Invio**. Premi **F5** (oppure Ctrl+G) per aprire **Vai a** e digita l'indirizzo lì. Separa gli indirizzi con virgole, ad esempio **A1:B2,D4**, per selezionare più intervalli contemporaneamente.

## I numeri della selezione

Quando selezioni due o più celle, la barra di stato mostra la **media**, il **conteggio** e la **somma** delle celle. Il conteggio è il numero di celle che contengono qualcosa; media e somma usano solo i numeri. Le righe e le colonne nascoste vengono escluse. Per copiare la selezione come testo, premi **Ctrl+C**.

## Che cosa indica la barra di stato

A sinistra della barra di stato compare **Pronto** quando puoi lavorare, **Invio** mentre scrivi in una cella, **Modifica** dopo aver premuto F2, **Punta** mentre puoi fare clic o usare le frecce per scegliere una cella e inserirne l'indirizzo in una formula, e **Fine** dopo aver premuto il tasto Fine. Quando un filtro è attivo, indica anche quante righe ha lasciato il filtro, ad esempio **Trovati 3 record su 8**.
