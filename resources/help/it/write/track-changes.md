# Traccia modifiche, e accettarle o rifiutarle
<!-- keywords: confronta, confronta documenti, due versioni, cosa è cambiato, traccia modifiche, modifiche registrate, revisioni, markup, accetta, rifiuta, accetta tutto, rifiuta tutto, visualizzazione per revisione, markup semplice, tutto il markup, nessun markup, originale, mostra markup, fumetti, riquadro revisioni, barra delle modifiche, inserimento, eliminazione, ctrl+maiusc+e, autore, revisore, nome utente, modifiche di altre persone -->

**Traccia modifiche** registra ogni modifica che apporti, così tu o qualcun altro potete vedere che cosa è cambiato e decidere se mantenerlo. Usala quando modifichi un documento di qualcun altro, oppure un documento che qualcuno rivedrà dopo di te.

## Attivare e disattivare Traccia modifiche

Fai clic su **Revisione > Traccia modifiche**, oppure premi **CTRL+MAIUSC+E**. La barra di stato in fondo alla finestra indica se la funzione è attiva. Un clic su quelle parole la attiva o la disattiva. L'impostazione viene salvata nel documento, quindi resta attiva quando il documento torna da un'altra persona.

Quando è attiva:

- **Ciò che digiti** viene contrassegnato come inserimento.
- **Ciò che elimini** non viene rimosso. Resta nel testo, barrato, così può essere ripristinato. Fa eccezione il testo che hai digitato tu stesso mentre la traccia era attiva: eliminarlo lo rimuove e basta, perché nessun altro lo ha mai visto.
- **Un'interruzione di paragrafo** che inserisci (Invio) viene contrassegnata come inserita, e una che elimini (Backspace all'inizio di un paragrafo) come eliminata.
- **Una modifica dell'aspetto** (grassetto, carattere, dimensione, colore, allineamento, rientri o stile) viene registrata insieme all'aspetto che il testo aveva prima.

Ogni modifica viene salvata con l'autore e la data. Il nome è quello indicato in **File > Opzioni**, alla voce **Nome utente**. Se è sbagliato, cambialo lì.

Ogni modifica costituisce un passaggio, quindi **CTRL+Z** la annulla come qualsiasi altra modifica.

## Come appare una modifica

Per impostazione predefinita, le modifiche di ciascuna persona hanno un colore proprio. La prima persona la cui modifica compare nel documento riceve il primo colore, la seconda il secondo, e così via; ciascuna mantiene lo stesso colore finché il documento è aperto. Il testo inserito è sottolineato con il colore della persona, e il testo eliminato è barrato. Il testo spostato è verde: barrato doppio dove è stato tolto, e sottolineato doppio dove è arrivato. Il testo la cui formattazione è stata modificata è in grassetto. Una barra nel margine sinistro segna ogni riga che contiene una modifica.

Puoi cambiare tutto questo. Fai clic su **Revisione > Traccia modifiche > Opzioni traccia modifiche** per aprire la finestra **Opzioni traccia modifiche**. Nella sezione **Markup**, ogni tipo di modifica ha un **Aspetto** e un **Colore**:

- **Inserimenti**: sottolineato, sottolineato doppio, barrato, grassetto, corsivo, solo colore, oppure (nessuno).
- **Eliminazioni**: barrato, barrato doppio, nascosto, accento circonflesso, oppure solo colore. Con **Nascosto**, le parole eliminate non compaiono nella riga. Con **Accento circonflesso**, non compaiono neppure, e un piccolo ^ indica dove si trovavano. Anche il testo spostato via non compare quando scegli **Fumetti** per le eliminazioni, ma non in caso contrario.
- **Spostato da** e **Spostato a**: gli aspetti e i colori del testo spostato. Sono disponibili quando è selezionata la casella **Traccia spostamenti**. Se la deselezioni, il testo spostato appare come una normale eliminazione e un normale inserimento.
- **Formattazione**: grassetto, solo colore, oppure (nessuno). Se deselezioni **Traccia formattazione**, le modifiche di formattazione non vengono contrassegnate nel testo.
- **Barre delle modifiche**: bordo sinistro, bordo destro, bordo esterno (il lato destro delle pagine dispari e il lato sinistro di quelle pari), oppure (nessuno), e il colore della barra.

Ogni casella **Colore** offre **Per autore** (il colore proprio di ciascuna persona) oppure un unico colore per tutti. Queste scelte sono personali: vengono conservate su questo computer, non nel documento, valgono per ogni documento e per ogni finestra aperta, e ogni finestra cambia non appena fai clic su **OK**. Le barre vengono disegnate allo stesso modo sullo schermo, in una stampa e in un PDF, ogni volta che il markup è visualizzato.

## Scegliere cosa vedere: Visualizzazione per revisione

Fai clic su **Revisione > Visualizzazione per revisione** e scegli una delle opzioni:

- **Tutto il markup** mostra ogni modifica nel testo, con le barre nel margine.
- **Markup semplice** mostra il documento come apparirà con le modifiche accettate, e una barra nel margine accanto a ogni riga che contiene una modifica.
- **Nessun markup** mostra il documento come apparirà con tutte le modifiche accettate, e nient'altro.
- **Originale** mostra il documento com'era prima di qualsiasi modifica: il testo eliminato torna al suo posto, e il testo inserito scompare.

Queste opzioni cambiano solo ciò che vedi. Non modificano il documento e non vengono salvate con esso. Un documento che contiene modifiche si apre con **Tutto il markup**.

**Revisione > Mostra markup** sceglie quali tipi di modifica vengono visualizzati: **Inserimenti ed eliminazioni**, **Formattazione** e **Commenti**. Sotto questi, scegli come mostrare il testo eliminato. **Mostra tutte le revisioni nel testo** lo mantiene nel testo, barrato. **Mostra revisioni nei fumetti** toglie il testo eliminato dalla riga e lo mette in un fumetto nel margine. **Mostra solo commenti e formattazione nei fumetti** mantiene inserimenti ed eliminazioni nel testo.

## Accettare o rifiutare le modifiche

Accettare una modifica significa mantenerla in modo definitivo. Rifiutarla significa annullarla.

- **Revisione > Accetta** accetta la modifica in cui si trova il cursore (oppure ogni modifica nel testo selezionato), poi passa alla successiva. **Revisione > Rifiuta** fa lo stesso, annullando la modifica.
- Fai clic sulla freccia sotto **Accetta** o **Rifiuta** per altre opzioni: **Accetta tutte le modifiche visualizzate**, **Accetta tutte le modifiche** e **Accetta tutte le modifiche e interrompi revisioni**; e **Rifiuta tutte le modifiche visualizzate** e **Rifiuta tutte le modifiche**. "Visualizzate" indica le modifiche dei tipi attivati in **Mostra markup** (e, nel riquadro **Revisioni**, del revisore scelto).
- **Precedente** e **Successiva** nel gruppo **Modifiche** passano da una modifica all'altra senza cambiare nulla.

Ognuna di queste operazioni costituisce un passaggio, quindi **CTRL+Z** ripristina il documento esattamente com'era.

Accettare un'eliminazione rimuove il testo in modo definitivo. Rifiutare un inserimento rimuove anche il testo. Accettare un inserimento mantiene il testo e toglie il contrassegno di novità. Rifiutare un'eliminazione mantiene il testo e toglie la barratura. Rifiutare una modifica dell'aspetto ripristina l'aspetto che il testo aveva. Il testo spostato è una coppia di modifiche: una dove è stato tolto e una dove è arrivato, unite dal nome dello spostamento. Quando la coppia è contrassegnata con lo stesso nome, come la salvano altri programmi, accettare o rifiutare una delle due metà fa lo stesso con l'altra, quindi il testo viene spostato in modo definitivo oppure riportato al suo posto. Un nome usato più volte, o con una sola metà, non unisce nulla: ogni metà è una modifica a sé stante.

## Il riquadro Revisioni

Fai clic su **Revisione > Riquadro revisioni** per aprire un elenco delle modifiche nel documento: tipo, autore, data e parole. Un clic su una voce la mostra nel testo. Usa **Accetta**, **Rifiuta**, **Precedente** e **Successiva** sotto l'elenco, oppure scegli un solo revisore dalla casella sopra di esso per vedere soltanto le modifiche di quella persona.

## Modifiche in intestazioni, piè di pagina, note e tabelle

Le modifiche apportate da altri in un'intestazione, in un piè di pagina, in una nota a piè di pagina o in una nota di chiusura vengono visualizzate ed elencate come quelle nel testo, comprese un'intestazione o un piè di pagina che nessuna pagina utilizza (quello per le pagine pari, quando il documento non usa pagine pari e dispari diverse): **Successiva** le supera, e **Accetta** e **Rifiuta** applicano la modifica su cui ti sei fermato per ultimo in quel punto. **Successiva**, **Precedente**, **Accetta**, **Rifiuta** e il riquadro **Revisioni** ti portano nell'intestazione, nel piè di pagina o nella nota che contiene la modifica; l'ordine è prima il testo, poi le note a piè di pagina, poi intestazioni e piè di pagina. **Accetta tutte** e **Rifiuta tutte** riguardano tutte queste parti.

Anche le modifiche alle tabelle vengono elencate, nel testo e nelle intestazioni e nei piè di pagina: una riga o una cella inserita o eliminata, celle unite e una modifica dell'aspetto di una tabella, di una riga o di una cella. Una barra nel margine segna le righe della riga o della cella. Accettare una riga eliminata, o rifiutarne una inserita, rimuove la riga e il suo testo. Una riga in cui ogni cella ha la modifica è la modifica di quella riga. Rifiutare una modifica dell'aspetto ripristina l'aspetto che la tabella, la riga o la cella avevano, e rifiutare un'unione divide di nuovo le celle. Quando eliminare la modifica interromperebbe la tabella, la modifica viene mantenuta e il messaggio indica quante ne sono state mantenute. Questo accade per una cella unita con altre verso il basso, che resta e perde il suo contrassegno, e per una modifica della griglia che le righe non riescono a seguire, che resta come modifica che puoi accettare. Le modifiche all'interno di una tabella in una nota a piè di pagina vengono mantenute come sono state scritte: non sono elencate, e Accetta e Rifiuta non intervengono su di esse.

Uno spostamento occupa una sola riga nell'elenco e una sola modifica nel conteggio di **Accetta tutte** e **Rifiuta tutte**: la sua altra metà e i segni di paragrafo che lo accompagnavano vengono gestiti insieme. Le modifiche all'interno di caselle di testo non sono elencate una per una. Il riquadro **Revisioni** indica quante sono (ad esempio, "3 altre modifiche sono all'interno di caselle di testo"), e **Accetta tutte** e **Rifiuta tutte** le applicano insieme alle altre.

## Cosa non è ancora registrato

Le modifiche che fai direttamente a tabelle (righe, celle, unioni), a elenchi e numerazione, a sezioni e impostazione di pagina, e al testo di intestazioni, piè di pagina e note vengono eseguite normalmente, senza registrazione. Le modifiche registrate da altri programmi nella numerazione e nelle sezioni vengono mantenute e salvate come erano, ma non sono elencate, e Accetta e Rifiuta non intervengono su di esse.

## Confrontare due versioni

Confronta trova le differenze tra due versioni di un documento e le mostra come modifiche registrate.

1. Apri la versione più vecchia. È l'**originale**.
2. Fai clic su **Confronta** nel gruppo **Confronta** della scheda **Revisione**.
3. Fai clic su **Sfoglia** e scegli la versione più recente. È la versione **revisionata**.
4. Digita il nome con cui firmare le modifiche. Scegli le opzioni che preferisci, ad esempio ignorare le maiuscole. **Trova testo spostato** e **Trova modifiche di formato** sono attive, a meno che tu non le disattivi.
5. Fai clic su **OK**.

Si apre una nuova finestra con il testo originale e le modifiche che lo trasformano nel testo revisionato. I due file non vengono modificati. Usa **Accetta** e **Rifiuta** come per qualsiasi modifica registrata. Se accetti tutte le modifiche, ottieni il testo revisionato; se le rifiuti tutte, ottieni il testo originale.

Confronta analizza il testo del corpo del documento. Non esamina tabelle, intestazioni, piè di pagina o note a piè di pagina.

Un paragrafo di 12 parole o più, spostato da un punto all'altro del documento, viene mostrato come **spostamento**: una registrazione dove il testo è stato tolto e una dove è arrivato. Accettare o rifiutare una delle due le gestisce entrambe. Il paragrafo deve essere identico, oppure almeno 9 parole su 10 uguali e in gran parte nello stesso ordine; altrimenti viene mostrato come eliminazione e inserimento. Le parole diverse all'interno di un paragrafo spostato vengono mostrate come inserimenti ed eliminazioni all'interno dello spostamento.

Il testo uguale nelle due versioni ma con un aspetto diverso viene mostrato come **modifica di formato**. Confronta verifica grassetto, corsivo, sottolineatura, dimensione, carattere, colore, evidenziazione, oltre allo stile, all'allineamento, ai rientri e alla spaziatura di un paragrafo. Rifiutare la modifica ripristina il vecchio aspetto.

Se i due documenti contengono troppe modifiche, Confronta lo segnala. Mostra quindi alcuni paragrafi come sostituiti per intero, e potrebbe non cercare spostamenti o modifiche di formato in tutti.
