# Grafici, sparkline, immagini e forme

<!-- keywords: grafico, diagramma, colonne, barre, linea, torta, area, dispersione, combinato, legenda, titolo, linee della griglia, etichette dati, dati grafico, tipo di grafico, aspetto, layout, sposta, ridimensiona, immagine, forma, oggetto, serie, linea di tendenza, tendenza, equazione, R al quadrato, secondo asse, sparkline, mini grafico, asse delle date, vittoria/sconfitta, grafico minuscolo, indicatori, grafici sparkline -->

Un grafico è un'immagine di numeri che resta collegata ad essi: se si modifica un numero, il grafico cambia di conseguenza. Immagini e forme si collocano sopra le celle allo stesso modo, ma non sono collegate ad alcun numero.

## Creare un grafico

1. Seleziona i numeri che vuoi rappresentare nel grafico. Se la prima riga contiene dei nomi e la prima colonna contiene delle etichette, seleziona anche quelle. JeffCrunch controlla la prima riga e la prima colonna e le usa come nomi e come etichette quando contengono parole (o date, per le etichette).
2. Nella scheda **Inserisci**, nel gruppo **Grafici**, scegli un tipo: **Colonna**, **Barra**, **Linea**, **Torta**, **Area**, **Dispersione** o **Combinato**. La piccola freccia accanto a un tipo mostra le sue varianti, come le colonne impilate, una linea con indicatori o un grafico a dispersione di punti, di punti uniti o di linee smussate.
3. Per vedere subito tutti i tipi, scegli **Altri grafici**. L'elenco a sinistra mostra ciascun tipo disegnato con i tuoi dati. **Scambia righe e colonne** inverte il senso delle serie.

Se selezioni una sola cella, JeffCrunch rappresenta il blocco di celle che la circonda e che contiene dei dati.

Le serie si sviluppano lungo le colonne quando la selezione ha almeno tante righe quante colonne, e lungo le righe quando ha più colonne che righe. Un grafico a torta contiene una sola serie. Un grafico a dispersione prende i valori orizzontali dalla prima colonna.

Esempio: selezioni A1:C4, dove A2:A4 contengono Q1, Q2 e Q3, B1 e C1 contengono Est e Ovest, e il resto sono numeri. Il grafico ha due serie, Est e Ovest, e tre gruppi di colonne, da Q1 a Q3.

## Spostare e ridimensionare

Fai clic su un grafico, un'immagine o una forma per selezionarlo. Trascinalo per spostarlo, oppure trascina uno degli otto quadratini sul bordo per ridimensionarlo.

- Tieni premuto **Alt** mentre trascini per allineare un bordo al bordo di una cella.
- Tieni premuto **Maiusc** mentre trascini un angolo per mantenere le proporzioni.
- I tasti freccia spostano l'oggetto di alcuni pixel alla volta. **Ctrl** con una freccia lo sposta di un pixel, **Maiusc** con una freccia lo ingrandisce o lo riduce, e **Alt** con una freccia lo porta fino al bordo successivo di una cella.
- **Esc** mentre trascini lo riporta alla posizione iniziale.

Ogni spostamento o ridimensionamento è un'azione che puoi annullare. Fai clic con il pulsante destro sull'oggetto e scegli **Proprietà** per digitare le sue dimensioni e per scegliere cosa succede quando inserisci, elimini o ridimensioni le celle: spostarsi e ridimensionarsi con le celle, spostarsi con esse ma mantenere le proprie dimensioni, oppure restare dov'è. La scelta viene salvata nel file. **Stampa questo oggetto**, nella stessa finestra, disattiva l'oggetto nelle pagine stampate (resta visibile sullo schermo); anche il file conserva questa scelta.

## Modificare ciò che mostra un grafico

Quando un grafico è selezionato, accanto ad esso compaiono due pulsanti e nella barra multifunzione compare la scheda **Grafico**.

- Il pulsante **+** (e **Elementi grafico** nella scheda) attiva o disattiva il titolo del grafico, gli assi, i titoli degli assi, le linee della griglia, la legenda e le etichette dati, e stabilisce dove si trovano la legenda e le etichette.
- Il pulsante a pennello (e **Colori** e **Stili** nella scheda) assegna al grafico uno dei set di colori e uno degli stili di JeffCrunch. **Layout** imposta più elementi contemporaneamente.
- **Dati grafico** elenca le serie del grafico. Puoi aggiungerle, modificarle, rimuoverle e riordinarle, cambiare le celle che ciascuna legge, impostare le celle con le etichette, scambiare righe e colonne, e scegliere come rappresentare le celle vuote e le righe e colonne nascoste.
- **Linea di tendenza** (nel menu **+**) disegna una linea adattata ai numeri di ciascuna serie, così puoi vedere dove stanno andando. Scegli **Lineare** per una retta, **Esponenziale** per numeri che crescono di una percentuale costante, **Logaritmica** per numeri che salgono rapidamente e poi si stabilizzano, **Polinomiale** per una curva con delle ansa, **Potenza**, oppure **Media mobile**, che disegna la media degli ultimi numeri. **Mostra equazione** scrive sul grafico la formula della linea, e **Mostra R al quadrato** indica quanto bene la linea si adatta ai dati (1 è un adattamento perfetto; 0 non è alcun adattamento). La linea di tendenza ha una propria voce nella legenda.
- Quando una serie è rappresentata con un secondo asse verticale, i menu **Assi** e **Titoli degli assi** hanno le voci **Orizzontale secondario** e **Verticale secondario** per essa. Il menu **Legenda** ha **Sopra il grafico**, che colloca la legenda sull'area del grafico invece che accanto ad esso.
- **Etichette dati** ha anche **Etichetta contiene** (il valore, il nome della categoria, il nome della serie e la percentuale, in qualsiasi combinazione), **Separatore** (ciò che sta tra loro: una virgola, un punto e virgola, un punto, un a capo o uno spazio) e **Formato numero…** (un codice di formato come 0,0 per i numeri nelle etichette; lascialo vuoto per mostrare i numeri così come sono). Se disattivi tutti i contenuti, le etichette scompaiono.
- **Formato asse orizzontale…** e **Formato asse verticale…** nella scheda **Grafico** aprono una finestra per un singolo asse. Imposta il valore minimo e massimo e le unità principali e secondarie (ciascuna automatica o un numero che digiti), se i valori vanno in ordine inverso, dove l'altro asse lo interseca, il formato numerico delle sue etichette, i suoi segni di graduazione, dove si trovano le sue etichette e di quanti gradi sono ruotate. Solo un asse numerico ha limiti e unità.
- **Tipo di grafico** cambia il tipo per l'intero grafico o per ciascuna serie: colonne con una linea sopra, oppure una serie rispetto a un secondo asse verticale.

Un riferimento a celle si scrive come **Foglio1!$B$2:$B$9**, con il nome del foglio tra apici singoli quando contiene spazi.

## Sparkline

Uno sparkline è un piccolo grafico che sta all'interno di una sola cella. Mostra la tendenza di una riga o di una colonna di numeri proprio accanto ad essi.

1. Seleziona i numeri. Per uno sparkline per ogni riga, seleziona più righe di numeri.
2. Nella scheda **Inserisci**, nel gruppo **Sparkline**, scegli **Linea**, **Colonna** o **Vittoria/Sconfitta**. **Linea** unisce i numeri con una linea. **Colonna** disegna una barra per ogni numero. **Vittoria/Sconfitta** disegna una barra verso l'alto per ogni numero maggiore di zero e una barra verso il basso per ogni numero minore di zero, tutte della stessa altezza, il che è adatto a risultati che sono soltanto positivi o negativi.
3. Nella finestra **Crea sparkline**, controlla l'**Intervallo dati** (le celle che gli sparkline leggono) e digita l'**Intervallo di posizione** (le celle in cui vanno). Scegli una cella di posizione per ogni riga di dati, oppure per ogni colonna di dati. Se scegli una sola cella, lo sparkline legge tutti i dati.

Gli sparkline creati in un solo passaggio formano un gruppo e condividono lo stesso aspetto. Fai clic su una cella con uno sparkline e compare la scheda **Sparkline**.

- **Modifica dati** cambia i dati e la posizione dell'intero gruppo, oppure i dati di quello su cui hai fatto clic. Stabilisce anche cosa fare con le celle vuote (lasciare uno spazio, trattarle come zero oppure unire i punti attraverso di esse) e se leggere le celle nelle righe e colonne nascoste.
- **Tipo** passa da linea a colonna e vittoria/sconfitta.
- **Mostra** evidenzia i punti: **Punto massimo**, **Punto minimo**, **Punti negativi**, **Primo punto**, **Ultimo punto* e, per una linea, **Indicatori** su ogni punto.
- **Stili** assegna al gruppo un set di colori. **Colore sparkline** imposta il colore della linea o delle barre e lo **Spessore** (quanto è spessa la linea, in punti). **Colore indicatore** imposta il colore di ciascun tipo di punto evidenziato.
- **Asse** mostra una linea allo zero, disegna i numeri da destra a sinistra e imposta il valore minimo e massimo dell'asse verticale. **Automatico per ogni sparkline** adatta ciascuno ai propri numeri. **Uguale per tutti gli sparkline** dà all'intero gruppo una sola scala, così puoi confrontarli. **Valore personalizzato** ti permette di digitare un numero. **Asse delle date…** distanzia i punti in base alle date: digita le celle che contengono la data di ciascun punto, e un punto con una data successiva si trova più avanti. Lascialo vuoto per distribuire di nuovo i punti in modo uniforme.
- **Raggruppa** riunisce gli sparkline selezionati in un solo gruppo. **Separa** dà a ciascuno il proprio aspetto. **Cancella** elimina gli sparkline selezionati, oppure i loro interi gruppi.

Uno sparkline segue le proprie celle: se inserisci una riga sopra i dati, continua a leggere gli stessi numeri. Ogni modifica è un'azione che puoi annullare. Gli sparkline vengono salvati nel file in una forma che altri programmi per fogli di calcolo leggono.

## Tagliare, copiare e incollare un oggetto

Un'immagine o una forma si copia e si incolla come un grafico. Una forma che usa qualcosa di diverso da un'immagine della cartella di lavoro, ad esempio un collegamento a un file, non può essere copiata, e JeffCrunch ti spiega il motivo.

Copia un grafico e incollalo nella stessa cartella di lavoro, su un foglio qualsiasi: la copia legge le stesse celle della prima. Se lo incolli in un'altra cartella di lavoro, mantiene i numeri che mostrava ma non legge più le celle, perché le celle che leggerebbe lì non sono quelle da cui è stato creato.

## Assegnare un nome

Con un oggetto selezionato, la casella Nome mostra il suo nome. Digita un nuovo nome e premi **Invio**. Annulla ripristina il nome precedente, e il nuovo nome viene salvato nel file.

## Eliminare un oggetto

Con l'oggetto selezionato, premi **Canc**. Annulla lo riporta.

## Cosa viene salvato

Tutto quanto descritto sopra viene salvato nel file in una forma che altri programmi per fogli di calcolo leggono, quindi un grafico creato qui si apre allo stesso modo in quegli altri programmi. Un grafico proveniente da un altro programma, con elementi che JeffCrunch non può conservare quando riscrive un grafico (barre di errore, un grafico tridimensionale), può comunque essere spostato, ridimensionato ed eliminato, ma i suoi elementi e i suoi dati non vengono modificati qui.
