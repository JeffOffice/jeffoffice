# Note e collegamenti

<!-- keywords: nota, note, commento, commenti, passaggio del mouse, segno rosso, angolo, nuova nota, modifica nota, elimina nota, mostra nota, nascondi nota, mostra tutte le note, collegamento, collegamenti, collegamento ipertestuale, file recenti, indirizzo, pagina web, posta elettronica, posizione in questa cartella di lavoro, Ctrl+K, Ctrl+clic, descrizione comando, apri collegamento, rimuovi collegamento, conversazione, statistiche cartella di lavoro, conteggio, celle, formule, grafici, tabelle pivot, accessibilità, testo alternativo, celle unite -->

## Note

Una **nota** è poche parole associate a una cella. Una cella che ne contiene una mostra un piccolo segno rosso nell'angolo in alto a destra. Posiziona il puntatore sulla cella e la nota compare accanto ad essa; allontana il puntatore e la nota scompare.

Per inserire una nota in una cella, selezionala e usa **Revisione > Nuova nota** (oppure premi **Maiusc+F2**). Digita il testo e fai clic su **OK**. Su una cella che ha già una nota, lo stesso comando diventa **Modifica nota**. Se elimini il testo, la nota viene rimossa.

- **Elimina nota** rimuove le note delle celle selezionate.
- **Mostra o nascondi nota** mantiene sempre visibile la nota delle celle selezionate sul foglio, in una casella accanto a ogni cella, oppure la nasconde di nuovo.
- **Mostra tutte le note** fa lo stesso per tutte le note del foglio; il pulsante diventa poi **Nascondi tutte le note**.
- **Nota precedente** e **Nota successiva** passano da una cella con nota alla successiva.

Ognuna di queste operazioni può essere annullata con un solo passaggio. Una nota resta associata alla sua cella quando si inseriscono o eliminano righe o colonne e quando le celle vengono ordinate. **Cancella tutto** rimuove le note dalle celle che cancella.

Una cartella di lavoro può contenere una conversazione di commenti, con risposte. JeffCrunch mostra l'intera conversazione come una nota e la conserva al salvataggio, ma non la modifica: la nota è di sola lettura.

Le note possono essere salvate in una cartella di lavoro .xlsx. Una cartella di lavoro aperta da un tipo di file meno recente o da un file di testo conserva le note solo dopo che la salvi come cartella di lavoro .xlsx.

## Collegamenti

Un **collegamento** ti porta da qualche altra parte quando lo segui: a una pagina web, a un file, a un indirizzo di posta elettronica o a una posizione in questa cartella di lavoro (una cella, più celle o un nome).

Per inserire un collegamento in una cella, selezionala e usa **Inserisci > Collegamento** (oppure premi **Ctrl+K**). Scegli dove deve portare, digita l'indirizzo o la posizione (oppure scegli uno dei file aperti di recente da **Oppure un file recente**) e, se vuoi, il testo che la cella deve mostrare e una **descrizione comando**. Su una cella che ha già un collegamento, lo stesso comando diventa **Modifica collegamento**. **Rimuovi collegamento** elimina i collegamenti dalle celle selezionate.

Posiziona il puntatore su una cella con un collegamento per vedere dove porta. Per seguirlo, tieni premuto **Ctrl** e fai clic sulla cella, oppure seleziona la cella e usa **Apri collegamento** dal menu di scelta rapida (clic con il pulsante destro). Un semplice clic si limita a selezionare la cella.

JeffCrunch non apre mai nulla da solo. Una pagina web o un indirizzo di posta elettronica si apre nel programma che usi di solito quando segui il collegamento. Un collegamento a un file, o a un altro tipo di indirizzo, ti chiede prima conferma. Un collegamento che eseguirebbe uno script non viene mai aperto.

Un collegamento resta associato alla sua cella quando si inseriscono o eliminano righe o colonne e quando le celle vengono ordinate, ed è salvato con la cartella di lavoro.

## Statistiche cartella di lavoro

**Statistiche cartella di lavoro** (scheda **Revisione**) conta ciò che la cartella di lavoro contiene: quanti **Fogli** ha, quante **Celle con dati** (celle che contengono un numero, un testo o una formula), quante **Formule**, quanti **Grafici** e quante **Tabelle pivot**. I fogli eliminati non vengono conteggiati. Gli stessi dati si trovano nella pagina **Info** della scheda File.

## Verifica accessibilità

**Verifica accessibilità** (scheda **Revisione**) apre una piccola finestra che elenca gli elementi della cartella di lavoro che alcuni lettori trovano difficili da usare, oppure che un lettore di schermo (un programma che legge ad alta voce il contenuto dello schermo) non riesce a descrivere. Ogni riga inizia con **Errore**, **Avviso** o **Suggerimento**. Fai clic su una riga per andare alla cella.

Verifica:

- Un grafico o un'immagine senza **testo alternativo** (le parole che descrivono cosa mostra). Questo è un errore.
- Una tabella senza riga di intestazione, quindi con colonne senza nome. Questo è un errore.
- **Celle unite**, che sono difficili da raggiungere con la tastiera. Questo è un avviso.
- Testo difficile da leggere sullo sfondo, come testo grigio chiaro su bianco. JeffCrunch misura quanto il testo risalta rispetto al riempimento. Il testo piccolo richiede un rapporto di almeno 4,5 a 1, mentre il testo grande (18 punti, oppure 14 punti in grassetto) richiede 3 a 1. Il programma indica la prima cella di ogni combinazione di carattere e riempimento. Questo è un avviso.
- Un foglio che conserva ancora il nome assegnato automaticamente, come Foglio1. Questo è un suggerimento.

Premi **Controlla di nuovo** dopo aver apportato modifiche.
