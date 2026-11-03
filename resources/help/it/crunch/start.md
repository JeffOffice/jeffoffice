# Benvenuti in JeffCrunch
<!-- keywords: benvenuto, introduzione, guida introduttiva, primi passi, principianti, avvio rapido, panoramica, cos'è JeffCrunch, nozioni di base, beta, foglio di calcolo, foglio elettronico, cartella di lavoro, foglio, celle, cosa funziona, non ancora, limiti, limitazioni -->

JeffCrunch è un programma per fogli di calcolo. È gratuito e open source. Non ci sono costi, abbonamenti né account da configurare. Fa parte delle app di JeffOffice e funziona su Windows, macOS e Linux.

Il file su cui si lavora si chiama **cartella di lavoro**. Una cartella di lavoro contiene uno o più **fogli** e ogni foglio è una griglia di **celle**. JeffCrunch salva le cartelle di lavoro come file .xlsx. Apre anche le cartelle di lavoro .xlsx e .xlsm create da altri programmi, oltre ai file .xls meno recenti, ai fogli di calcolo OpenDocument e ai file di testo con valori. Conserva gli elementi che non è ancora in grado di modificare.

## JeffCrunch è una versione beta

Questa versione di JeffCrunch è una **beta**. È pronta per il lavoro reale, ma non fa ancora tutto. Conservate una copia di ogni cartella di lavoro importante.

**Cosa funziona**

- Apertura di cartelle di lavoro: .xlsx e .xlsm, .xls meno recenti, fogli di calcolo OpenDocument (.ods e .fods) e file di testo con valori (.csv, .tsv e .txt). Salvataggio in .xlsx e di copie in .xls o .ods. Vedere [Aprire e salvare cartelle di lavoro](open-save).
- Apertura di cartelle di lavoro protette da password (.xlsx, .xls e .ods) e protezione con password di una cartella di lavoro .xlsx. Vedere [Aprire cartelle di lavoro protette da password](open-password).
- Digitazione di numeri, testo e formule nelle celle, con annulla e ripeti. Le formule vengono calcolate durante la digitazione, con più di 200 funzioni. Mentre si digita una formula è possibile fare clic sulle celle o raggiungerle con i tasti di direzione, e la finestra di dialogo **Argomenti funzione** compila gli argomenti di una funzione mostrando il risultato in tempo reale. Vedere [Digitare nelle celle](edit-cells), [Come vengono visualizzate e calcolate le formule](formulas) e [Inserire una funzione con la finestra di dialogo Argomenti funzione](function-arguments).
- Formattazione delle celle: tipi di carattere, colori, bordi, formati numerici, stili cella e celle unite. Vedere [Formattare le celle](format-cells).
- Taglia, copia e incolla, riempimento delle celle, somma, trova e sostituisci, ordinamento, filtri, tabelle e tabelle pivot. Vedere [Tabelle pivot](pivot-tables).
- Righe, colonne e fogli: inserimento, eliminazione, ridimensionamento, occultamento e spostamento.
- Grafici, immagini e forme, note e collegamenti.
- Protezione di un foglio o della cartella di lavoro.
- Impostazione delle pagine, anteprima di stampa, stampa e creazione di PDF. Vedere [Stampare e creare un PDF](printing).

**Cosa non funziona ancora**

- Non è possibile creare, modificare o eliminare regole di formattazione condizionale o di convalida dati. JeffCrunch mostra quelle presenti nella cartella di lavoro e le conserva.
- Non è possibile modificare la definizione di uno stile cella né crearne uno nuovo.
- Una nuova formula che usa una funzione non ancora disponibile in JeffCrunch resta senza valore; una formula già presente nella cartella di lavoro mantiene il risultato memorizzato nel file.
- Le macro vengono conservate e salvate, ma non vengono mai eseguite.
- Le tabelle pivot hanno alcune limitazioni, elencate in [Tabelle pivot](pivot-tables).
- Una cartella di lavoro aperta da un file .xls, .ods o di testo viene salvata come nuovo file .xlsx, a meno che non si scelga una copia .xls o .ods. Una copia non può contenere tutto e JeffCrunch indica cosa ometterà prima del salvataggio.
- Una password può proteggere una cartella di lavoro .xlsx, ma non una copia .xls o .ods.
- Una tabella dati (tabella di simulazione) può essere calcolata ma non creata.

## Le prime cinque cose da fare

1. **Aprite una cartella di lavoro.** Premete **{key:Open}** oppure fate clic su **File** e quindi su **Apri**. Le prime righe vengono visualizzate subito, anche in una cartella di lavoro molto grande, mentre JeffCrunch legge il resto.
2. **Esplorate.** Fate clic su una cella per selezionarla. Il suo indirizzo è nella **Casella Nome** in alto a sinistra e il suo contenuto nella **Barra della formula** accanto. Per spostarvi usate i tasti di direzione, la rotellina del mouse e le barre di scorrimento. Vedere [Selezionare le celle e spostarsi](select-and-move).
3. **Modificate una cella.** Selezionate una cella e digitate. Premete **Invio** per confermare. Vedere [Digitare nelle celle](edit-cells).
4. **Formattate le celle.** Selezionate le celle e usate la scheda **Home** per cambiare carattere, colori, bordi e formato numero. Vedere [Formattare le celle](format-cells).
5. **Cambiate la visualizzazione.** Usate la scheda **Visualizza** per lo zoom, per attivare o disattivare la griglia e le intestazioni e per bloccare le prime righe o le prime colonne. Vedere [Cambiare la visualizzazione](view).

## Cosa si vede

Una cartella di lavoro appare come è stata salvata: larghezza delle colonne e altezza delle righe, tipi di carattere, colori, riempimenti e bordi, celle unite, riquadri bloccati e formati numerici che trasformano un numero memorizzato in una data, una percentuale o un prezzo. Il testo troppo lungo per la cella si estende nelle celle vuote adiacenti. Un numero troppo largo per la cella viene visualizzato come una serie di simboli #; allargate la colonna oppure selezionate la cella per leggere il numero nella barra della formula. I numeri vengono scritti secondo la lingua del computer (vedere [Formati numerici e lingua](number-formats)) e i formati condizionali della cartella di lavoro, come scale di colori, barre dei dati e icone, vengono visualizzati (vedere [Formati condizionali](conditional-formats)).

Salvate il lavoro con **{key:Save}**, che scrive le modifiche nella cartella di lavoro aperta, oppure con **Salva con nome**, che le scrive in un nuovo file. Vedere [Aprire e salvare cartelle di lavoro](open-save).
