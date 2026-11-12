# Trova, sostituisci e vai a

<!-- keywords: trova, sostituisci, cambia, cerca, trova testo, sostituisci con, sostituisci tutto, trova successivo, cerca, cambia in, cambia tutto, stesse maiuscole, carattere jolly, asterisco, punto interrogativo, tilde, maiuscole/minuscole, cella intera, vai a, riferimento, nome, seleziona per tipo, spazi vuoti, costanti, formule, errori, area corrente, formula matriciale, differenze di riga, differenze di colonna, ultima cella, celle visibili, formattazione condizionale, convalida, Ctrl+F, Ctrl+H, F5, note -->

## Trova

Premi **Ctrl+F** (oppure scegli **Trova** nella scheda Home, poi di nuovo **Trova**). Digita il testo in **Trova** e premi **Invio** per passare alla cella successiva che lo contiene, oppure scegli **Trova tutti** per ottenere un elenco di tutte le celle corrispondenti. Fai clic su una riga dell'elenco per andare a quella cella.

| Opzione | Funzione |
| --- | --- |
| **Dove** | Cerca in questo foglio oppure in tutti i fogli visibili. Se è selezionata più di una cella, la ricerca in un solo foglio riguarda solo quelle celle. |
| **Ordine** | Visita le celle per righe (da sinistra a destra, poi in basso) oppure per colonne (dall'alto in basso, poi a destra). **Trova successivo** si ferma alla prima corrispondenza in quell'ordine. |
| **Cerca in** | **Formule** cerca in ciò che è digitato in ciascuna cella, **Valori** in ciò che la cella visualizza, e **Note** nelle note delle celle. |
| **Maiuscole/minuscole** | Lettere maiuscole e minuscole devono coincidere. |
| **Solo celle intere** | La cella deve contenere esattamente il testo, non solo una parte di esso. |

Nel testo digitato in **Trova** il simbolo **?** rappresenta un qualsiasi carattere singolo e **\*** una sequenza qualsiasi di caratteri (anche nessuno). Digita **~** davanti a ?, \* o ~ per cercare il simbolo stesso: **a~\*b** trova a\*b.

La ricerca procede un po' alla volta, così puoi continuare a lavorare. **Esc** (oppure **Annulla**) la interrompe, e la interrompe anche la modifica di una cella. Un testo molto lungo richiede più tempo a ogni passo rispetto a uno breve, quindi un foglio con testi lunghi lascia comunque libera la finestra.

Le note delle celle seguono le celle: dopo un ordinamento, oppure dopo l'inserimento o l'eliminazione di righe o colonne, la ricerca nelle note trova ogni nota nella cella in cui si trova ora. Una nota su una cella eliminata non compare più nella ricerca (e torna con Annulla). Se una cartella di lavoro è stata modificata in così tanti modi che le note non possono essere seguite in tutti, ti viene chiesto di salvarla prima. Una cartella di lavoro aperta da un tipo di file meno recente o da un file di testo ha le note in una forma che JeffCrunch non cerca, e te lo viene segnalato.

## Sostituisci

Premi **Ctrl+H**, digita il nuovo testo in **Sostituisci con** e scegli un pulsante. **Sostituisci** inserisce il nuovo testo nella cella con il cursore, se contiene il testo, e passa alla successiva. **Sostituisci tutto** lo fa in ogni cella che lo contiene, come un'unica operazione annullabile; una modifica di grandi dimensioni viene applicata un po' alla volta. Il nuovo testo viene inserito come se lo avessi digitato: un numero diventa un numero e una formula modificata resta una formula.

Non viene modificato nulla, e ne viene indicato il motivo, se una cella che verrebbe modificata è bloccata in un foglio protetto, fa parte di una formula matriciale o di una tabella dati, si trova nella prima riga di una tabella oppure non sarebbe più una formula.

## Vai a

Premi **F5** (oppure **Ctrl+G**), oppure digita nella Casella Nome. Puoi indicare una cella, come B7, un intervallo, come B3:D9, intere colonne o righe (C:E, 4:9), una cella di un altro foglio (Foglio2!A1, oppure 'Il mio foglio'!A1), oppure il nome di un intervallo.

## Seleziona per tipo

Scegli **Seleziona per tipo** nel menu **Trova** per selezionare le celle in base al loro contenuto:

- **Celle vuote**, **Costanti** (digitate), **Formule** e **Celle con errori**. Per costanti e formule puoi escludere numeri, testo, VERO e FALSO, oppure errori.
- **Il blocco intorno alla cella con il cursore** e **L'ultima cella utilizzata**.
- **La formula matriciale in cui si trova il cursore** seleziona tutte le celle di quella formula matriciale.
- **Celle diverse dalla cella del cursore nella stessa riga** e **nella stessa colonna** trovano le celle che non sono uguali alla cella con il cursore. Seleziona prima un blocco e posiziona il cursore sulla cella con cui le altre devono coincidere. In ogni riga (o colonna) del blocco viene scelta una cella quando contiene un numero o un testo diverso, oppure quando contiene una formula che, spostando i nomi di cella in modo che si riferiscano alla posizione di quella cella, non risulta uguale. Una cella con formula viene scelta anche quando la cella del cursore non ne ha una. Se selezioni una sola cella, JeffCrunch esamina il blocco intorno a essa.
- **Celle visualizzate** (escluse le righe o colonne nascoste), **Celle con formattazione condizionale** e **Celle con una regola per i valori immessi** (tutte, oppure solo quelle con la stessa regola della cella con il cursore).

Il primo blocco trovato viene selezionato e gli altri vengono evidenziati con un colore; il colore scompare quando fai clic su una cella.
