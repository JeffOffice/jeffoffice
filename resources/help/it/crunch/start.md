# Benvenuti in JeffCrunch
<!-- keywords: benvenuto, come iniziare, introduzione, principiante, primi passi, avvio rapido, panoramica, che cos'è JeffCrunch, nozioni di base, beta, foglio di calcolo, cartella di lavoro, foglio, celle, cosa funziona, non ancora, limiti -->

JeffCrunch è un programma per fogli di calcolo. È gratuito e open source. Non ci sono costi, non ci sono abbonamenti e non serve creare un account. È una delle app di JeffOffice e funziona su Windows, macOS e Linux.

Il file su cui si lavora si chiama **cartella di lavoro**. Una cartella di lavoro contiene uno o più **fogli**, e ogni foglio è una griglia di **celle**. JeffCrunch salva le cartelle di lavoro in file .xlsx. Apre anche le cartelle di lavoro .xlsx e .xlsm create da altri programmi, i vecchi file .xls, i fogli di calcolo OpenDocument e i file di testo con valori. Ciò che non può ancora modificare viene conservato.

## JeffCrunch è una beta

Questa versione di JeffCrunch è una **beta**. È pronta per il lavoro reale, ma non fa ancora tutto. Conserva una copia di ogni cartella di lavoro importante.

**Cosa funziona**

- Apertura delle cartelle di lavoro: .xlsx e .xlsm, i vecchi .xls, i fogli di calcolo OpenDocument (.ods e .fods) e i file di testo con valori (.csv, .tsv e .txt). Salvataggio in .xlsx e salvataggio di copie in .xls o .ods. Vedi [Aprire e salvare le cartelle di lavoro](open-save).
- Apertura delle cartelle di lavoro protette da password (.xlsx, .xls e .ods) e protezione di una cartella di lavoro .xlsx con una password. Vedi [Aprire cartelle di lavoro protette da password](open-password).
- Digitazione di numeri, testo e formule nelle celle, con annulla e ripeti. Le formule vengono calcolate mentre si digitano, con più di 200 funzioni. Si possono fare clic o usare le frecce per selezionare celle mentre si digita una formula, e la finestra di dialogo **Argomenti funzione** compila gli argomenti di una funzione con un risultato in tempo reale. Vedi [Digitare nelle celle](edit-cells), [Come vengono mostrate e calcolate le formule](formulas) e [Inserire una funzione con la finestra di dialogo Argomenti funzione](function-arguments).
- Formattazione delle celle: caratteri, colori, bordi, formati numero, stili cella e celle unite. Vedi [Formattare le celle](format-cells).
- Taglia, copia e incolla, riempimento delle celle, somma, trova e sostituisci, ordinamento, filtri, tabelle e tabelle pivot. Vedi [Tabelle pivot](pivot-tables).
- Righe, colonne e fogli: inserimento, eliminazione, dimensionamento, nascondimento e spostamento.
- Grafici, immagini e forme, note e collegamenti.
- Protezione di un foglio o dell'intera cartella di lavoro.
- Impostazione delle pagine, anteprima di stampa, stampa e creazione di un PDF. Vedi [Stampare e creare un PDF](printing).

**Cosa non funziona ancora**

- Non è possibile creare, modificare o eliminare le regole di formattazione condizionale o le regole di convalida dei dati. JeffCrunch mostra quelle presenti in una cartella di lavoro e le conserva.
- Non è possibile modificare la definizione di uno stile cella né crearne uno nuovo.
- Una funzione che JeffCrunch non ha ancora lascia senza valore una nuova formula; una formula già presente nella cartella di lavoro mantiene il risultato memorizzato nel file.
- Le macro vengono conservate e salvate, ma non vengono mai eseguite.
- Le tabelle pivot hanno alcuni limiti, elencati in [Tabelle pivot](pivot-tables).
- Una cartella di lavoro aperta da .xls, .ods o da un file di testo viene salvata come nuovo file .xlsx, a meno che non si scelga una copia .xls o .ods. Una copia non può contenere tutto, e JeffCrunch indica che cosa lascerà fuori prima di salvare.
- Una password può proteggere una cartella di lavoro .xlsx e una copia .xls, .ods o .ots della stessa, ma non una copia di testo.
- Una tabella dati (tabella di simulazione) può essere calcolata ma non creata.

## Prima cosa fai queste cinque operazioni

1. **Apri una cartella di lavoro.** Premi **{key:Apri}**, oppure fai clic su **File** e poi su **Apri**. Le prime righe compaiono subito, anche in una cartella di lavoro molto grande, mentre JeffCrunch legge il resto.
2. **Esplora la finestra.** Fai clic su una cella per selezionarla. Il suo indirizzo si trova nella **Casella Nome** in alto a sinistra, e il suo contenuto nella **barra della formula** accanto. Usa i tasti freccia, la rotellina del mouse e le barre di scorrimento per spostarti. Vedi [Selezionare le celle e spostarsi](select-and-move).
3. **Modifica una cella.** Seleziona una cella e digita. Premi **Invio** per confermare ciò che hai digitato. Vedi [Digitare nelle celle](edit-cells).
4. **Formatta le celle.** Seleziona le celle e usa la scheda **Home** per cambiare carattere, colori, bordi e formato numero. Vedi [Formattare le celle](format-cells).
5. **Cambia la visualizzazione.** Usa la scheda **Visualizza** per impostare lo zoom, mostrare o nascondere le linee della griglia e le intestazioni, e bloccare le prime righe o le prime colonne. Vedi [Modificare la visualizzazione](view).

## Cosa si vede

Una cartella di lavoro appare come è stata salvata: la larghezza delle colonne e l'altezza delle righe, i caratteri, i colori, i riempimenti e i bordi, le celle unite, i riquadri bloccati e i formati numero che trasformano un numero memorizzato in una data, una percentuale o un prezzo. Il testo troppo lungo per la sua cella si estende nelle celle vuote accanto. Un numero troppo largo per la sua cella viene mostrato come una serie di simboli #; allarga la colonna oppure seleziona la cella per leggere il numero nella barra della formula. I numeri vengono scritti secondo le impostazioni di lingua del computer (vedi [Formati numero e lingua](number-formats)), e le formattazioni condizionali di una cartella di lavoro, come le scale di colore, le barre dati e le icone, vengono disegnate (vedi [Formattazione condizionale](conditional-formats)).

Salva il lavoro con **{key:Salva}**, che scrive le modifiche nella cartella di lavoro aperta, oppure con **Salva con nome**, che le scrive in un nuovo file. Vedi [Aprire e salvare le cartelle di lavoro](open-save).
