# Trova, sostituisci e vai a
<!-- keywords: trova, sostituisci, cambia, cerca, trova cosa, sostituisci con, sostituisci tutto, trova successivo, cerca, cambia in, cambia tutto, stesse maiuscole, carattere jolly, asterisco, punto interrogativo, tilde, distingui maiuscole, cella intera, trova per formato, formato, sostituisci nelle note, vai a, riferimento, nome, seleziona per tipo, vuote, costanti, formule, errori, area corrente, formula matrice, differenze di riga, differenze di colonna, ultima cella, celle visibili, formattazione condizionale, convalida, Ctrl+F, Ctrl+H, F5, note -->

## Trova

Premi **Ctrl+F** (oppure scegli **Trova** nella scheda Home, quindi di nuovo **Trova**). Digita il testo in **Trova cosa** e premi **Invio** per passare alla cella successiva che lo contiene, oppure scegli **Trova tutti** per ottenere un elenco di tutte le celle corrispondenti. Fai clic su una riga dell'elenco per andare a quella cella.

| Opzione | Cosa fa |
| --- | --- |
| **Dove** | Cerca in questo foglio oppure in tutti i fogli visibili. Con più di una cella selezionata, una ricerca in un foglio guarda solo in quelle celle. |
| **Ordine** | Visita le celle per righe (da sinistra a destra, poi verso il basso) oppure per colonne (dall'alto in basso, poi verso destra). **Trova successivo** si ferma alla prima corrispondenza in quell'ordine. |
| **Cerca in** | **Formule** cerca in ciò che è digitato in ogni cella, **Valori** in ciò che viene visualizzato, e **Note** nelle note delle celle. |
| **Formato…** | Trova solo le celle che hanno un determinato formato. **Formato…** apre la finestra di dialogo Formato celle, avviata dalla cella con il cursore: ciò che modifichi lì è ciò che viene cercato (un carattere in grassetto, un colore di riempimento, un formato numero). **Formato da cella** prende tutti gli aspetti del formato della cella con il cursore, e **Cancella formato** torna a cercare solo nel testo. La finestra di dialogo indica che cosa viene cercato, e lo segnala se il formato scelto non contiene nulla. L'aspetto che una formattazione condizionale attribuisce a una cella non viene cercato, solo il formato che la cella ha di suo. Senza testo in **Trova cosa**, **Trova tutti** e **Trova successivo** trovano le celle che hanno il formato; con il testo, una cella deve avere entrambi. |
| **Distingui maiuscole/minuscole** | Le lettere maiuscole e minuscole devono coincidere. |
| **Solo celle intere** | L'intera cella deve contenere il testo, non solo una parte di essa. |

Nel testo digitato in **Trova cosa**, **?** indica un singolo carattere qualsiasi e **\*** indica una sequenza qualsiasi di caratteri (anche nessuno). Digita **~** davanti a ?, \* o ~ per cercare il simbolo stesso: **a~\*b** trova a\*b.

La ricerca procede un po' alla volta, così puoi continuare a lavorare. **Esc** (oppure **Annulla**) la interrompe, e la interrompe anche la modifica di una cella. Un testo molto lungo richiede più tempo a ogni passo rispetto a uno breve, quindi un foglio con testi lunghi lascia comunque libera la finestra.

Le note delle celle seguono le celle: dopo un ordinamento, oppure dopo l'inserimento o l'eliminazione di righe o colonne, una ricerca nelle note trova ogni nota nella cella in cui si trova ora. Una nota di una cella eliminata non compare più nella ricerca (e ricompare con Annulla). Se una cartella di lavoro è stata modificata in così tanti modi che le note non possono essere seguite in tutti, ti viene chiesto di salvarla prima; una cartella aperta da un tipo di file più vecchio o da un file di testo ha le note in una forma che JeffCrunch non cerca, e ti viene segnalato.

## Sostituisci

Premi **Ctrl+H**, digita il nuovo testo in **Sostituisci con**, e scegli un pulsante. **Sostituisci** inserisce il nuovo testo nella cella con il cursore, se la contiene, e passa alla successiva. **Sostituisci tutto** lo fa in ogni cella che lo contiene, come un'unica operazione che puoi annullare; una modifica di grandi dimensioni viene applicata un po' alla volta. Il nuovo testo viene inserito come se lo digitassi: un numero diventa un numero, e una formula modificata resta una formula.

Per sostituire nelle note, scegli **Note** in **Cerca in** nella scheda Sostituisci. **Sostituisci tutto** modifica allora le parole di ogni nota che contiene il testo, su questo foglio o su tutti i fogli, come un'unica operazione che puoi annullare, mentre **Sostituisci** modifica la nota della cella con il cursore e passa alla successiva. Una nota che è una conversazione di commenti viene lasciata invariata (modificali nella scheda Revisione), e ti viene indicato quante ne sono state lasciate. Non viene modificato nulla, e ti viene spiegato il motivo, se un foglio con una nota da modificare è protetto, oppure se una nota resterebbe senza parole o con più di 32.767 caratteri. Una nota le cui parole vengono sostituite perde la data in cui è stata scritta, come accade quando ne modifichi le parole. Le note vengono salvate con la cartella di lavoro, in tutte le forme che prevedono note. I **Valori** possono essere cercati ma non sostituiti, perché un valore è il risultato di una formula.

Non viene modificato nulla, e ti viene spiegato il motivo, se una cella da modificare è bloccata in un foglio protetto, fa parte di una formula matrice o di una tabella dati, si trova nella prima riga di una tabella, oppure non sarebbe più una formula.

## Vai a

Premi **F5** (oppure **Ctrl+G**), oppure digita nella casella Nome. Puoi indicare una cella come B7, un intervallo come B3:D9, colonne o righe intere (C:E, 4:9), una cella di un altro foglio (Foglio2!A1, oppure 'Il mio foglio'!A1), oppure il nome di un intervallo. Un nome che indica le stesse celle su un gruppo di fogli, come Gen:Dic!B2, porta al primo foglio del gruppo.

## Seleziona per tipo

Scegli **Seleziona per tipo** nel menu **Trova** per selezionare le celle in base a ciò che contengono:

- **Celle vuote**, **Costanti** (digitate), **Formule** e **Celle con errore**. Per le costanti e le formule puoi escludere numeri, testo, VERO e FALSO, oppure errori.
- **Area corrente intorno alla cella con il cursore** e **Ultima cella utilizzata**.
- **La formula matrice in cui si trova il cursore** seleziona tutte le celle di quella formula matrice.
- **Celle diverse dalla cella del cursore nella stessa riga** e **nella stessa colonna** trovano le celle che non sono uguali alla cella con il cursore. Seleziona prima un intervallo, quindi posiziona il cursore sulla cella con cui le altre devono coincidere. In ogni riga (o colonna) dell'intervallo, una cella viene selezionata quando contiene un numero o un testo diverso, oppure quando contiene una formula che non è la stessa una volta spostati i nomi di cella fino alla posizione di quella cella. Una cella con una formula viene selezionata anche quando la cella del cursore non ne ha una. Se selezioni una sola cella, JeffCrunch considera l'area intorno ad essa.
- **Celle che le formule nella selezione leggono** e **Celle con formule che leggono la selezione** selezionano le celle di questo foglio usate dalle formule delle celle selezionate, oppure le celle le cui formule usano le celle selezionate. Con una sola cella selezionata, partono da quella cella. Seleziona **segui fino a tutti i livelli** per proseguire da ciascuna cella trovata (le celle che queste leggono, e così via); senza di essa, viene selezionato solo il primo livello. La ricerca parte da al massimo 1.000 delle celle selezionate, e la barra di stato indica quando una ricerca si è fermata a causa di un limite.
- **Celle visibili (non in righe o colonne nascoste)**, **Celle con formattazione condizionale** e **Celle con una regola per i valori immessi** (tutte, oppure solo quelle con la stessa regola della cella con il cursore).

Il primo blocco trovato viene selezionato e gli altri vengono evidenziati con una tinta; la tinta scompare quando fai clic su una cella.
