# Trova, sostituisci e vai a

<!-- keywords: trova, sostituisci, cambia, cerca, testo da trovare, sostituisci con, sostituisci tutto, trova successivo, cerca, cambia in, cambia tutto, stesse maiuscole, carattere jolly, asterisco, punto interrogativo, tilde, maiuscole/minuscole, cella intera, trova per formato, formato, sostituisci nelle note, vai a, riferimento, nome, seleziona per tipo, vuote, costanti, formule, errori, area corrente, formula matriciale, differenze di riga, differenze di colonna, ultima cella, celle visibili, formattazione condizionale, convalida, Ctrl+F, Ctrl+H, F5, note -->

## Trova

Premi **Ctrl+F** (oppure scegli **Trova** nella scheda Home, poi di nuovo **Trova**). Digita il testo in **Trova** e premi **Invio** per passare alla cella successiva che lo contiene, oppure scegli **Trova tutti** per ottenere un elenco di tutte le celle corrispondenti. Fai clic su una riga dell'elenco per andare a quella cella.

| Opzione | Cosa fa |
| --- | --- |
| **Dove** | Cerca in questo foglio, oppure in tutti i fogli visibili. Con più di una cella selezionata, la ricerca in un foglio si limita a quelle celle. |
| **Ordine** | Visita le celle per righe (da sinistra a destra, poi in basso) oppure per colonne (dall'alto in basso, poi a destra). **Trova successivo** si ferma alla prima corrispondenza in quell'ordine. |
| **Cerca in** | **Formule** cerca in ciò che è digitato in ogni cella, **Valori** in ciò che viene visualizzato, e **Note** nelle note delle celle. |
| **Formato...** | Trova solo le celle che hanno un determinato formato. **Formato...** apre la finestra di dialogo Formato celle, avviata dalla cella con il cursore: ciò che modifichi lì è ciò che viene cercato (un carattere in grassetto, un colore di riempimento, un formato numero). **Formato da cella** prende tutti gli elementi del formato della cella con il cursore, e **Cancella formato** torna a cercare solo il testo. La finestra di dialogo indica che cosa si sta cercando, e segnala se il formato scelto non contiene nulla. L'aspetto che una formattazione condizionale dà a una cella non viene cercato, solo il formato che la cella ha di suo. Se **Trova** è vuoto, **Trova tutti** e **Trova successivo** trovano le celle che hanno il formato; con del testo, una cella deve avere entrambi. |
| **Maiuscole/minuscole** | Le lettere maiuscole e minuscole devono coincidere. |
| **Solo celle intere** | L'intera cella deve contenere il testo, non solo una parte. |

Nel testo digitato in **Trova**, **?** rappresenta un solo carattere qualsiasi e **\*** una sequenza di caratteri qualsiasi (anche nessuno). Digita **~** prima di ?, \* o ~ per cercare il segno stesso: **a~\*b** trova a\*b.

La ricerca procede a piccoli passi, così puoi continuare a lavorare. **Esc** (o **Annulla**) la interrompe, e la interrompe anche la modifica di una cella. Un testo molto lungo richiede più tempo a ogni passo rispetto a uno breve, quindi un foglio con testi lunghi lascia comunque libera la finestra.

Le note delle celle seguono le celle: dopo un ordinamento, oppure dopo l'inserimento o l'eliminazione di righe o colonne, una ricerca nelle note trova ciascuna nota nella cella in cui si trova ora. Una nota su una cella eliminata non compare più nella ricerca (e ricompare con Annulla). Se una cartella di lavoro è stata modificata in così tanti modi che le note non possono essere seguite in tutti, ti viene chiesto di salvarla prima; una cartella aperta da un tipo di file più vecchio o da un file di testo ha le note in una forma che JeffCrunch non cerca, e te lo viene segnalato.

## Sostituisci

Premi **Ctrl+H**, digita il nuovo testo in **Sostituisci con**, e scegli un pulsante. **Sostituisci** inserisce il nuovo testo nella cella con il cursore, se la contiene, e passa alla successiva. **Sostituisci tutto** lo fa in ogni cella che lo contiene, come un'unica operazione che puoi annullare; una modifica di grandi dimensioni viene applicata a piccoli blocchi. Il nuovo testo viene inserito come se lo avessi digitato: un numero diventa un numero, e una formula modificata resta una formula.

Per sostituire nelle note, scegli **Note** in **Cerca in** nella scheda Sostituisci. **Sostituisci tutto** modifica allora le parole di ogni nota che contiene il testo, in questo foglio o in tutti i fogli, come un'unica operazione che puoi annullare; **Sostituisci** modifica la nota della cella con il cursore e passa alla successiva. Una nota che è una conversazione di commenti viene lasciata così com'è (modificala nella scheda Revisione), e ti viene indicato quante ne sono state lasciate. Non viene modificato nulla, e ti viene spiegato il motivo, se un foglio con una nota da modificare è protetto, oppure se una nota resterebbe senza parole o con più di 32.767 caratteri. Una nota le cui parole vengono sostituite perde la data in cui è stata scritta, come accade quando ne modifichi il testo. Le note vengono salvate con la cartella di lavoro, in ogni forma che prevede note. **Valori** può essere cercato ma non sostituito, perché un valore è ciò che una formula calcola.

Non viene modificato nulla, e ti viene spiegato il motivo, se una cella che dovrebbe essere modificata è bloccata in un foglio protetto, fa parte di una formula matriciale o di una tabella dati, si trova nella prima riga di una tabella, oppure non sarebbe più una formula.

## Vai a

Premi **F5** (oppure **Ctrl+G**), oppure digita nella casella Nome. Puoi indicare una cella come B7, un intervallo come B3:D9, intere colonne o righe (C:E, 4:9), una cella di un altro foglio (Foglio2!A1, oppure 'Il mio foglio'!A1), oppure il nome di un intervallo.

## Seleziona per tipo

Scegli **Seleziona per tipo** nel menu **Trova** per selezionare le celle in base al loro contenuto:

- **Celle vuote**, **Costanti** (digitate), **Formule** e **Celle con errore**. Per costanti e formule puoi escludere numeri, testo, VERO e FALSO, oppure errori.
- **Il blocco intorno alla cella con il cursore**, e **L'ultima cella utilizzata**.
- **La formula matriciale in cui si trova il cursore** seleziona tutte le celle di quella formula matriciale.
- **Celle diverse dalla cella del cursore nella stessa riga** e **nella stessa colonna** trovano le celle che non sono uguali alla cella con il cursore. Seleziona prima un blocco, poi posiziona il cursore sulla cella a cui le altre devono corrispondere. In ogni riga (o colonna) del blocco, viene scelta una cella quando contiene un numero o un testo diverso, oppure quando contiene una formula che non è uguale una volta spostati su quella cella i nomi di cella che usa. Una cella con una formula viene scelta anche quando la cella del cursore non ne ha una. Se selezioni una sola cella, JeffCrunch esamina il blocco intorno a essa.
- **Celle visualizzate** (non in righe o colonne nascoste), **Celle con formattazione condizionale**, e **Celle con una regola per i valori immessi** (tutte, oppure solo quelle con la stessa regola della cella con il cursore).

Il primo blocco trovato viene selezionato e gli altri vengono evidenziati con un colore; il colore scompare quando fai clic su una cella.
