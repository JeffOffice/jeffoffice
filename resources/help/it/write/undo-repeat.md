# Annulla, ripristina e ripeti
<!-- keywords: annulla, ripristina, ripeti, f4, ctrl+y, cronologia, torna indietro, esegui di nuovo, ultima azione, passaggio, undo, redo -->

**{key:Annulla}** annulla l'ultima modifica e **{key:Ripristina}** la rifà. Entrambi si trovano nella barra degli strumenti rapida, in alto. Ogni comando che dai è un passaggio, e una serie di parole digitate è un solo passaggio.

## Tornare indietro di più passaggi

Accanto a **Annulla** e **Ripristina** c'è una freccia. Fai clic sulla freccia per vedere i passaggi, dal più recente, con un massimo di 100 voci elencate. Ogni passaggio riporta il nome del comando che lo ha creato, come "Grassetto" o "Digitazione", oppure "Modifica" quando la modifica è arrivata da un'altra fonte. Sposta il puntatore lungo l'elenco per indicare quanti passaggi vuoi, quindi fai clic per eseguirli. Scegliendo il terzo passaggio annulli i primi tre. I passaggi annullati restano nell'elenco di **Ripristina** finché non fai una nuova modifica.

## Ripetere l'ultimo comando

**F4** esegue di nuovo l'ultimo comando su ciò che è selezionato ora. Ad esempio, rendi grassetto una parola, seleziona un'altra parola e premi **F4**. La seconda parola diventa grassetto, e un solo **Annulla** la riporta com'era. Una ripetizione è sempre un passaggio a sé, anche subito dopo aver digitato.

I comandi che attivano o disattivano qualcosa, come grassetto, corsivo, sottolineato, elenchi puntati e numerati, vengono ripetuti secondo il risultato ottenuto, non come interruttore. Se hai reso grassetto una parola, **F4** rende grassetta la selezione successiva, anche se una parte di essa è già in grassetto, mentre una selezione già in grassetto viene lasciata invariata. Se hai tolto il grassetto da una parola, **F4** toglie il grassetto dalla selezione successiva.

Ripeti funziona per:

- Formattazione: grassetto, corsivo, sottolineato, barrato, apice, pedice, carattere, dimensione carattere, colore carattere, evidenziazione, ombreggiatura del testo, modifica maiuscole/minuscole e cancella formattazione.
- Impostazioni di paragrafo: allineamento, rientri, spaziatura, elenchi puntati, numerazione, bordi e sfondo.
- Stili, dalla raccolta, e gli stili Normale e dei titoli.
- Comandi di tabella che inseriscono, eliminano, uniscono, allineano o impostano i bordi.
- Interruzioni di pagina e pagine vuote, e i caratteri speciali inseriti con un comando.
- Digitazione: dopo aver digitato alcune parole, **F4** scrive di nuovo le stesse parole nella posizione del cursore. Vengono conservate solo le parole di una serie, fino a 1.000 lettere, e una serie termina quando sposti il cursore. La serie è ciò che il testo è diventato dopo le correzioni, come la Correzione automatica, quindi una ripetizione inserisce quelle parole così come sono. Il tasto Backspace accorcia la serie di tante lettere quante ne cancella.

Ripeti non funziona per apertura e salvataggio, cambio di visualizzazione, Trova e sostituisci, copia e incolla, né per i comandi che aprono una finestra per chiederti qualcosa. Questi comandi non diventano mai "l'ultimo comando" per Ripeti, quindi **F4** continua a ripetere la formattazione eseguita prima di essi.

La descrizione comandi del pulsante della barra degli strumenti rapida indica cosa farebbe, ad esempio "Ripeti Grassetto (F4)". Ripeti segue le stesse regole del comando che ripete: con Revisioni attive, la ripetizione viene registrata come modifica, e in un documento aperto solo in lettura, o protetto, non fa nulla e lo indica nella barra di stato. Ogni documento ha un proprio ultimo comando, quindi Ripeti non porta mai un comando da un documento a un altro.

Ripeti funziona dove stai modificando. In una casella di testo o in una forma, scrive nella casella e esegue i comandi che la casella prevede; un comando che la casella non ha non viene ripetuto, e la barra di stato lo segnala. In un'intestazione, un piè di pagina, una nota o una cella di tabella, funziona in quel punto.

**Ctrl+Y** è **Ripristina** finché hai passaggi da ripristinare. Quando non ne hai, **Ctrl+Y** ripete come **F4**.
