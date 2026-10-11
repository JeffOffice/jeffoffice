# Righelli, guide e allineamento automatico
<!-- keywords: righello, righelli, punto zero, origine del righello, guida, guide, guida righello, guide margini, griglia, colonne, linea di base, blocca, aggancia, allineamento automatico, allinea alle guide, allinea agli oggetti, unità di misura, pollici, centimetri, mostra guide, limiti -->

Le guide sono linee che aiutano ad allineare gli elementi. Vengono visualizzate sullo schermo ma non vengono mai stampate. Con l'allineamento automatico, gli oggetti si agganciano a una guida o a un altro oggetto quando vi si avvicinano.

## Righelli e unità di misura

Per attivare o disattivare i righelli, scegliere **Visualizza > Mostra > Righelli**. Su ogni righello una linea rossa segue il puntatore e la parte del righello corrispondente all'oggetto selezionato appare ombreggiata. Durante la digitazione in una casella di testo, il righello superiore mostra gli indicatori di rientro e di tabulazione.

Per cambiare le unità di misura, fare clic su **File > Opzioni**, aprire la scheda **Avanzate** e scegliere **Unità di misura**: pollici, centimetri, millimetri, punti o pica. In qualsiasi casella di dimensioni o posizione è anche possibile digitare un'unità, ad esempio 5 cm.

## Spostare il punto zero

Ogni righello misura a partire dall'angolo superiore sinistro della pagina: lo 0 corrisponde all'angolo e i numeri aumentano man mano che ci si allontana. Per misurare da un altro punto, ad esempio dall'angolo di un oggetto da misurare, spostare il punto zero:

- Tenere premuto **MAIUSC** e trascinare lungo un righello con il pulsante **destro** del mouse. Lo zero di quel righello si sposta nel punto in cui si rilascia il pulsante.
- Per spostare entrambi i righelli contemporaneamente, trascinare dal piccolo riquadro in cui i due righelli si incontrano.
- Per riportare lo zero di un righello all'angolo della pagina, fare doppio clic su quel righello. Per riportarli entrambi, fare doppio clic sul piccolo riquadro. Durante la digitazione in una casella di testo, il doppio clic sul righello superiore apre invece le impostazioni **Tabulazioni**.

I numeri e le tacche dei due righelli della pagina partono dal punto zero, così come la posizione indicata nella barra di stato. Se lo zero si trova a 7 cm dal bordo, un oggetto a 2 cm dal bordo sinistro della pagina viene indicato lì come -5 cm, perché si trova 5 cm prima dello zero. Le caselle di posizione (come quelle della finestra **Misure**), la finestra **Guide righello** e le guide trascinate dai righelli continuano invece a misurare dall'angolo della pagina. Gli indicatori di rientro e di tabulazione sul righello del testo continuano a misurare dalla casella di testo.

Il punto zero viene salvato con la pubblicazione e resta quindi al suo posto alla successiva apertura del file. Una nuova pubblicazione inizia con entrambi gli zeri all'angolo della pagina. Lo spostamento dello zero si può annullare in un solo passaggio con **Annulla**.

## I quattro tipi di guide

- Le **guide margini** (rosa) indicano i margini. Si impostano in **Progettazione pagina > Imposta pagina > Margini**.
- Le **guide griglia** (blu) dividono la pagina in colonne e righe. Appartengono a una pagina master.
- Le **guide linea di base** (linee punteggiate color sabbia) indicano le righe su cui poggia il testo.
- Le **guide righello** (verdi) sono quelle posizionate dall'utente.

Per mostrarle o nasconderle tutte, scegliere **Visualizza > Mostra > Guide**. **Visualizza > Mostra > Linee di base** (CTRL+F7) mostra o nasconde le guide linea di base.

## Impostare colonne e righe

Fare clic su **Progettazione pagina > Imposta pagina > Guide**. In **Guide predefinite** scegliere Nessuna griglia, 2 colonne, 3 colonne, 4 colonne oppure Griglia 2 × 2, 3 × 3 o 4 × 4. Per un controllo maggiore, scegliere **Guide griglia e linea di base…**. La finestra **Guide di layout** contiene le schede seguenti:

- **Guide margini**: i quattro margini.
- **Guide griglia**: colonne, righe e lo spazio tra di esse. È anche possibile aggiungere una guida centrale.
- **Guide linea di base**: la spaziatura tra le righe e il punto in cui inizia la prima.

Per far poggiare il testo sulle guide linea di base, aprire la finestra **Paragrafo** e selezionare **Allinea testo alle guide linea di base**.

## Aggiungere guide righello

- Trascinare dal righello superiore per creare una guida orizzontale. Trascinare dal righello sinistro per crearne una verticale.
- In alternativa, scegliere **Aggiungi guida righello orizzontale** o **Aggiungi guida righello verticale** nel menu **Guide**. Ogni nuova guida viene posizionata accanto all'ultima.
- Trascinare una guida per spostarla. Trascinarla di nuovo sul righello per eliminarla.
- **Guide righello…** consente di digitare posizioni esatte. Permette anche di aggiungere una serie di guide a spaziatura uniforme.
- **Cancella tutte le guide righello** rimuove le guide dalla pagina corrente.

Le guide di una pagina master vengono visualizzate in tutte le pagine che la utilizzano. Per spostarle, passare alla pagina master.

## Attivare o disattivare l'allineamento automatico

Usare **Progettazione pagina > Layout**:

- **Allinea alle guide**: gli oggetti si agganciano alle guide margini, alle guide griglia, alle guide righello, ai bordi della pagina e al centro della pagina.
- **Allinea agli oggetti**: gli oggetti si agganciano ai bordi e ai centri degli altri oggetti.

Entrambe le opzioni sono inizialmente attive. L'aggancio avviene quando ci si trova a pochi pixel di distanza. Tenere premuto **ALT** durante il trascinamento per disattivarlo per quello spostamento.

## Argomenti correlati

- [Spostare, allineare, raggruppare e disporre gli oggetti su livelli](arrange)
- [Pagine master, intestazioni e numeri di pagina](master-pages)
- [Dimensioni pagina, margini e layout](page-setup)
- [La scheda Progettazione pagina](tab-page-design)
