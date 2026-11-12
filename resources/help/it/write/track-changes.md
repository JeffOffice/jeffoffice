# Tenere traccia delle modifiche e accettarle o rifiutarle

<!-- keywords: confronta, confronta documenti, due versioni, cosa è cambiato, revisioni, tenere traccia delle modifiche, modifiche registrate, markup, accetta, rifiuta, accetta tutto, rifiuta tutto, visualizza per revisione, markup semplice, tutto il markup, nessun markup, originale, mostra markup, fumetti, riquadro revisioni, barra di modifica, inserimento, eliminazione, ctrl+maiusc+e, autore, revisore, nome utente, modifiche di altri -->

**Revisioni** conserva una traccia di ogni modifica che apporti, così che tu (o qualcun altro) possa vedere che cosa è cambiato e decidere se mantenerlo. Usalo quando modifichi un documento che appartiene a qualcun altro, oppure che qualcuno rivedrà dopo di te.

## Attivare e disattivare Revisioni

Fai clic su **Revisione > Revisioni**, oppure premi **Ctrl+Maiusc+E**. La barra di stato in fondo alla finestra indica se la funzione è attiva. Un clic su quella scritta la attiva o la disattiva. L'impostazione viene salvata nel documento, quindi resta attiva quando il documento torna da qualcun altro.

Quando è attiva:

- **Il testo che digiti** viene contrassegnato come inserimento.
- **Il testo che elimini** non viene rimosso. Resta nel testo, barrato, in modo che possa essere ripristinato. Fa eccezione il testo che hai scritto tu stesso mentre le revisioni erano attive: eliminarlo lo rimuove e basta, perché nessun altro lo ha mai visto.
- **Un'interruzione di paragrafo che inserisci** (Invio) viene contrassegnata come inserita, e una che elimini (Backspace all'inizio di un paragrafo) come eliminata.
- **Una modifica dell'aspetto** (grassetto, carattere, dimensione, colore, allineamento, rientri o stile) viene registrata insieme all'aspetto che il testo aveva prima.

Ogni modifica viene salvata con chi l'ha fatta e quando. Il nome è quello indicato in **File > Opzioni**, alla voce **Nome utente**. Se non è corretto, modificalo lì.

Ogni modifica costituisce un'unica operazione, quindi **Ctrl+Z** la annulla come qualsiasi altra modifica.

## Com'è fatta una modifica

Per impostazione predefinita, le modifiche di ogni persona hanno un colore proprio. La prima persona la cui modifica compare nel documento riceve il primo colore, la successiva il secondo, e così via; ciascuno mantiene lo stesso colore finché il documento è aperto. Il testo inserito è sottolineato con il colore della persona, e il testo eliminato è barrato. Il testo spostato è verde: doppio barrato dove è stato tolto e doppio sottolineato dove è arrivato. Il testo la cui formattazione è stata modificata è in grassetto. Una barra nel margine sinistro segna ogni riga che contiene una modifica.

Puoi cambiare tutto questo. Fai clic su **Revisione > Revisioni > Opzioni revisioni** per aprire la finestra **Opzioni traccia modifiche**. Alla voce **Markup**, ogni tipo di modifica ha un **Aspetto** e un **Colore**:

- **Inserimenti**: sottolineato, doppia sottolineatura, barrato, grassetto, corsivo, solo colore, oppure (nessuno).
- **Eliminazioni**: barrato, doppio barrato, nascosto, accento circonflesso, o solo colore. Con **Nascosto**, le parole eliminate non compaiono nella riga. Con **Accento circonflesso**, non compaiono neanche, ma un piccolo ^ indica dove si trovavano. Anche il testo spostato via viene omesso quando per le eliminazioni scegli **Fumetti**, ma non altrimenti.
- **Spostato da** e **Spostato in**: l'aspetto e i colori del testo spostato. Sono disponibili finché è selezionata la casella **Tieni traccia degli spostamenti**. Se la deselezioni, il testo spostato compare come una normale eliminazione e un normale inserimento.
- **Formattazione**: grassetto, solo colore, oppure (nessuno). Se deselezioni **Tieni traccia della formattazione**, le modifiche di formattazione non vengono contrassegnate nel testo.
- **Barre di modifica**: bordo sinistro, bordo destro, bordo esterno (il lato destro delle pagine dispari e il lato sinistro delle pagine pari), oppure (nessuno), e il colore della barra.

Ogni casella **Colore** offre **Per autore** (il colore proprio di ciascuna persona) oppure un colore unico per tutte le persone. Queste scelte sono personali: vengono memorizzate su questo computer, non nel documento, valgono per ogni documento e per ogni finestra aperta, e ogni finestra cambia non appena fai clic su **OK**. Le barre vengono disegnate allo stesso modo sullo schermo, in una stampa e in un PDF, ogni volta che il markup è visualizzato.

## Scegliere che cosa vedere: Visualizza per revisione

Fai clic su **Revisione > Visualizza per revisione** e scegli una voce:

- **Tutto il markup** mostra ogni modifica nel testo, con le barre nel margine.
- **Markup semplice** mostra il documento come sarà con le modifiche accettate, e una barra nel margine accanto a ogni riga che contiene una modifica.
- **Nessun markup** mostra il documento come sarà con tutte le modifiche accettate, senza altro.
- **Originale** mostra il documento come era prima di qualsiasi modifica: il testo eliminato torna al suo posto e il testo inserito scompare.

Queste opzioni cambiano soltanto ciò che vedi. Non modificano il documento e non vengono salvate con esso. Un documento che contiene modifiche si apre con **Tutto il markup**.

**Revisione > Mostra markup** sceglie quali tipi di modifica mostrare: **Inserimenti ed eliminazioni**, **Formattazione** e **Commenti**. Sotto, scegli come mostrare il testo eliminato. **Mostra tutte le revisioni nel testo** lo conserva nel testo, barrato. **Mostra revisioni in fumetti** toglie il testo eliminato dalla riga e lo mette in un fumetto nel margine. **Mostra solo commenti e formattazione in fumetti** lascia inserimenti ed eliminazioni nel testo.

## Accettare o rifiutare le modifiche

Accettare una modifica significa mantenerla in modo definitivo. Rifiutarla significa annullarla.

- **Revisione > Accetta** accetta la modifica in cui si trova il cursore (oppure ogni modifica nel testo selezionato), poi passa alla successiva. **Revisione > Rifiuta** fa lo stesso, annullando la modifica.
- Fai clic sulla freccia sotto **Accetta** o **Rifiuta** per altre opzioni: **Accetta tutte le modifiche visualizzate**, **Accetta tutte le modifiche** e **Accetta tutte le modifiche e interrompi revisioni**; e **Rifiuta tutte le modifiche visualizzate** e **Rifiuta tutte le modifiche**. "Visualizzate" indica le modifiche dei tipi attivati da **Mostra markup** (e, nel riquadro **Riquadro revisioni**, del revisore scelto).
- **Precedente** e **Successiva**, nel gruppo **Modifiche**, passano da una modifica alla successiva senza cambiare nulla.

Ognuna di queste operazioni costituisce un'unica azione, quindi **Ctrl+Z** riporta il documento esattamente com'era.

Accettare un'eliminazione rimuove il testo in modo definitivo. Rifiutare un inserimento rimuove anch'esso il testo. Accettare un inserimento mantiene il testo e toglie il segno che era una novità. Rifiutare un'eliminazione mantiene il testo e toglie la barratura. Rifiutare una modifica dell'aspetto ripristina l'aspetto che il testo aveva. Il testo spostato è una coppia di modifiche, una dove è stato tolto e una dove è arrivato. Quando la coppia è contrassegnata con lo stesso nome, come la salvano altri programmi, accettare o rifiutare una delle due metà produce lo stesso effetto sull'altra, così il testo viene spostato in modo definitivo oppure riportato al suo posto.

## Il riquadro Revisioni

Fai clic su **Revisione > Riquadro revisioni** per aprire un elenco delle modifiche del documento: tipo, autore, data e parole. Un clic su una voce la mostra nel testo. Usa **Accetta**, **Rifiuta**, **Precedente** e **Successiva** sotto l'elenco, oppure scegli un revisore nella casella sopra di esso per vedere solo le modifiche di quella persona.

## Che cosa non viene ancora tenuto traccia

Le modifiche a tabelle (righe, celle, unione di celle), a numerazione ed elenchi, a sezioni e impostazione di pagina, e al testo di intestazioni, piè di pagina e note vengono eseguite normalmente, senza registrazione. Le modifiche registrate da altri programmi in quei punti vengono conservate e salvate così come sono.

## Confrontare due versioni

Confronta trova le differenze tra due versioni di un documento e le mostra come modifiche registrate.

1. Apri la versione più vecchia. È l'**originale**.
2. Fai clic su **Confronta** nel gruppo **Confronta** della scheda **Revisione**.
3. Fai clic su **Sfoglia** e scegli la versione più recente. È la versione **rivista**.
4. Digita il nome con cui firmare le modifiche. Scegli le opzioni che preferisci, ad esempio ignorare le maiuscole.
5. Fai clic su **OK**.

Si apre una nuova finestra con il testo originale e le modifiche che lo trasformano nel testo rivisto. I due file non vengono modificati. Usa **Accetta** e **Rifiuta** come per qualsiasi modifica registrata. Se accetti tutte le modifiche, ottieni il testo rivisto; se le rifiuti tutte, ottieni il testo originale.

Confronta esamina il testo del corpo del documento. Non esamina tabelle, intestazioni, piè di pagina, note a piè di pagina né l'aspetto del testo. Una modifica del solo aspetto non viene rilevata.
