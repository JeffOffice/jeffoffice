# Trova, sostituisci e vai a

<!-- keywords: trova, sostituisci, cambia, cerca, carattere jolly, asterisco, punto interrogativo, tilde, maiuscole minuscole, cella intera, vai a, riferimento, nome, seleziona per tipo, spazi vuoti, costanti, formule, errori, area corrente, formula matriciale, differenze di riga, differenze di colonna, ultima cella, celle visibili, formattazione condizionale, convalida, Ctrl+F, Ctrl+H, F5, note -->

## Trova

Premi **Ctrl+F** (oppure scegli **Trova** nella scheda Home, poi di nuovo **Trova**). Digita il testo e premi **Invio** per passare alla cella successiva che lo contiene, oppure scegli **Trova tutti** per ottenere un elenco di tutte le celle corrispondenti. Fai clic su una riga dell'elenco per andare a quella cella.

| Opzione | Cosa fa |
| --- | --- |
| **Dove** | Cerca in questo foglio, oppure in tutti i fogli visibili. Con più di una cella selezionata, una ricerca in un foglio cerca solo in quelle celle. |
| **Ordine** | Visita le celle per righe (da sinistra a destra, poi in basso) oppure per colonne (dall'alto in basso, poi a destra). **Successivo** si ferma alla prima corrispondenza in quest'ordine. |
| **Cerca in** | **Formule** cerca in ciò che è digitato in ogni cella, **Valori** in ciò che la cella mostra, e **Note** nelle note delle celle. |
| **Maiuscole/minuscole** | Lettere maiuscole e minuscole devono coincidere. |
| **Cella intera** | Il contenuto dell'intera cella deve essere il testo, non solo una parte di esso. |

Nel testo cercato, **?** rappresenta un singolo carattere qualsiasi e **\*** una sequenza di caratteri qualsiasi (anche nessuno). Digita **~** davanti a ?, \* o ~ per cercare proprio quel segno: **a~\*b** trova a\*b.

La ricerca procede un po' alla volta, così puoi continuare a lavorare. **Esc** (oppure **Annulla**) la interrompe, e la interrompe anche la modifica di una cella. Un testo molto lungo richiede più tempo a ogni passaggio rispetto a uno breve, quindi un foglio con testi lunghi lascia comunque libera la finestra.

Le note sulle celle seguono le celle: dopo un ordinamento, oppure dopo l'inserimento o l'eliminazione di righe o colonne, una ricerca nelle note trova ogni nota nella cella in cui si trova ora. Una nota su una cella eliminata non compare più nella ricerca (e ricompare con Annulla). Se una cartella di lavoro è stata modificata in così tanti modi che le note non possono essere seguite in tutte le modifiche, ti viene chiesto di salvarla prima. Una cartella di lavoro aperta da un tipo di file meno recente o da un file di testo ha le note in una forma che JeffCrunch non cerca, e ti viene segnalato.

## Sostituisci

Premi **Ctrl+H**. **Cambia** inserisce il nuovo testo nella cella con il cursore, se contiene il testo, e passa alla successiva. **Sostituisci tutto** lo fa in ogni cella che lo contiene, come un'unica operazione che puoi annullare; una modifica ampia viene applicata un po' alla volta. Il nuovo testo viene inserito come se lo avessi digitato: un numero resta un numero, e una formula modificata resta una formula.

Non viene modificato nulla, e ti viene spiegato il motivo, se una cella da modificare è bloccata in un foglio protetto, fa parte di una formula matriciale o di una tabella dati, si trova nella prima riga di una tabella, oppure non sarebbe più una formula.

## Vai a

Premi **F5** (oppure **Ctrl+G**), oppure digita nella Casella Nome. Puoi indicare una cella come B7, un intervallo come B3:D9, colonne o righe intere (C:E, 4:9), una cella di un altro foglio (Foglio2!A1, oppure 'Il mio foglio'!A1), oppure il nome di un intervallo.

## Seleziona per tipo

Scegli **Seleziona per tipo** nel menu **Trova** per selezionare le celle in base al loro contenuto:

- **Celle vuote**, **Costanti** (digitate), **Formule** e **Celle con errori**. Per costanti e formule puoi escludere numeri, testo, VERO e FALSO, oppure errori.
- **Il blocco intorno alla cella con il cursore** e **L'ultima cella utilizzata**.
- **La formula matriciale in cui si trova il cursore** seleziona tutte le celle di quella formula matriciale.
- **Celle diverse dalla cella con il cursore nella riga** e **nella colonna** trovano le celle che non corrispondono alla cella con il cursore. Seleziona prima un blocco, poi posiziona il cursore sulla cella a cui le altre devono corrispondere. In ogni riga (o colonna) del blocco viene scelta una cella quando contiene un numero o un testo diverso, oppure quando contiene una formula che non è uguale una volta spostati i nomi di cella verso la posizione di quella cella. Una cella con una formula viene scelta anche quando la cella con il cursore non ne ha una. Se selezioni una sola cella, JeffCrunch considera il blocco intorno ad essa.
- **Celle visibili** (non in righe o colonne nascoste), **Celle con formattazione condizionale** e **Celle con una regola per i valori ammessi** (tutte, oppure solo quelle con la stessa regola della cella con il cursore).

Il primo blocco trovato viene selezionato e gli altri vengono evidenziati con un colore; il colore scompare quando fai clic su una cella.
