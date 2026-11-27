# Tenere traccia delle modifiche, e accettarle o rifiutarle

<!-- keywords: confronta, confronta documenti, due versioni, cosa è cambiato, traccia modifiche, modifiche tracciate, revisioni, markup, accetta, rifiuta, accetta tutto, rifiuta tutto, visualizza per revisione, markup semplice, tutto il markup, nessun markup, originale, mostra markup, fumetti, riquadro revisioni, barra delle modifiche, inserimento, eliminazione, ctrl+maiusc+e, autore, revisore, nome utente, modifiche di altri -->

**Traccia modifiche** registra ogni modifica che fai, così tu o un'altra persona potete vedere che cosa è cambiato e decidere se mantenerlo. Usala quando modifichi un documento che appartiene a qualcun altro, o che qualcuno revisionerà dopo di te.

## Attivare e disattivare Traccia modifiche

Fai clic su **Revisione > Traccia modifiche**, oppure premi **Ctrl+Maiusc+E**. La barra di stato in fondo alla finestra indica se la funzione è attiva. Facendo clic su queste parole, la funzione viene attivata o disattivata. L'impostazione viene salvata nel documento, quindi resta attiva quando il documento torna da qualcun altro.

Quando è attiva:

- **Ciò che digiti** viene contrassegnato come inserimento.
- **Ciò che elimini** non viene rimosso. Resta nel testo, barrato, così può essere ripristinato. Fanno eccezione il testo che hai digitato tu stesso mentre la traccia era attiva: eliminandolo, viene semplicemente rimosso, perché nessun altro lo ha mai visto.
- **Un'interruzione di paragrafo** (Invio) che inserisci viene contrassegnata come inserita, e una che elimini (Backspace all'inizio di un paragrafo) come eliminata.
- **Una modifica dell'aspetto** (grassetto, carattere, dimensione, colore, allineamento, rientri o stile) viene registrata insieme all'aspetto che il testo aveva prima.

Ogni modifica viene salvata con il nome di chi l'ha fatta e la data e l'ora. Il nome è quello indicato in **File > Opzioni**, alla voce **Nome utente**. Se non è corretto, modificalo lì.

Ogni modifica è un singolo passaggio, quindi **Ctrl+Z** la annulla come qualsiasi altra modifica.

## Come appare una modifica

Per impostazione predefinita, le modifiche di ciascuna persona hanno un colore proprio. La prima persona la cui modifica compare nel documento riceve il primo colore, la successiva il secondo, e così via; ogni persona mantiene lo stesso colore finché il documento è aperto. Il testo inserito è sottolineato con il colore della persona, e il testo eliminato è barrato. Il testo spostato è verde: barrato doppio dove è partito, e sottolineato doppio dove è arrivato. Il testo la cui formattazione è stata modificata è in grassetto. Una barra nel margine sinistro indica ogni riga che contiene una modifica.

Puoi cambiare tutto questo. Fai clic su **Revisione > Traccia modifiche > Opzioni traccia modifiche** per aprire la finestra **Opzioni traccia modifiche**. In **Markup** ogni tipo di modifica ha un **Aspetto** e un **Colore**:

- **Inserimenti**: sottolineato, sottolineato doppio, barrato, grassetto, corsivo, solo colore, oppure (nessuno).
- **Eliminazioni**: barrato, barrato doppio, nascosto, accento circonflesso, oppure solo colore. Con **Nascosto**, le parole eliminate non compaiono nella riga. Con **Accento circonflesso** non compaiono neanche, e un piccolo ^ indica dove si trovavano. Anche il testo spostato via viene nascosto quando per le eliminazioni scegli **Fumetti**, ma non altrimenti.
- **Spostato da** e **Spostato in**: aspetto e colori per il testo spostato. Sono disponibili finché è selezionata la casella **Traccia spostamenti**. Se la deselezioni, il testo spostato appare come una normale eliminazione e un normale inserimento.
- **Formattazione**: grassetto, solo colore, oppure (nessuno). Se deselezioni **Traccia formattazione**, le modifiche di formato non vengono segnalate nel testo.
- **Barre delle modifiche**: bordo sinistro, bordo destro, bordo esterno (il lato destro delle pagine dispari e il lato sinistro di quelle pari), oppure (nessuna), e il colore della barra.

Ogni casella **Colore** offre **Per autore** (il colore proprio di ciascuna persona) oppure un solo colore per tutti. Queste scelte sono tue: vengono memorizzate su questo computer, non nel documento, si applicano a ogni documento e a ogni finestra aperta, e ogni finestra cambia non appena fai clic su **OK**. Le barre sono disegnate allo stesso modo sullo schermo, in una stampa e in un PDF, ogni volta che il markup è visualizzato.

## Scegli cosa vedere: Visualizza per revisione

Fai clic su **Revisione > Visualizza per revisione** e scegli una voce:

- **Tutto il markup** mostra ogni modifica nel testo, con le barre nel margine.
- **Markup semplice** mostra il documento come apparirà con le modifiche accettate, e una barra nel margine accanto a ogni riga che contiene una modifica.
- **Nessun markup** mostra il documento come apparirà con tutte le modifiche accettate, senza altro.
- **Originale** mostra il documento com'era prima di qualsiasi modifica: il testo eliminato torna al suo posto e il testo inserito scompare.

Queste opzioni cambiano solo ciò che vedi. Non modificano il documento e non vengono salvate con esso. Un documento che contiene modifiche si apre con **Tutto il markup**.

**Revisione > Mostra markup** sceglie quali tipi di modifica vengono mostrati: **Inserimenti ed eliminazioni**, **Formattazione** e **Commenti**. Sotto, scegli come mostrare il testo eliminato. **Mostra tutte le revisioni nel testo** lo conserva nel testo, barrato. **Mostra revisioni nei fumetti** toglie il testo eliminato dalla riga e lo inserisce in un fumetto nel margine. **Mostra solo commenti e formattazione nei fumetti** mantiene inserimenti ed eliminazioni nel testo.

## Accettare o rifiutare le modifiche

Accettare una modifica significa mantenerla in modo definitivo. Rifiutarla significa annullarla.

- **Revisione > Accetta** accetta la modifica in cui si trova il cursore (oppure ogni modifica nel testo selezionato), poi passa alla successiva. **Revisione > Rifiuta** fa lo stesso, annullando la modifica.
- Fai clic sulla freccia sotto **Accetta** o **Rifiuta** per altre opzioni: **Accetta tutte le modifiche visualizzate**, **Accetta tutte le modifiche** e **Accetta tutte le modifiche e interrompi revisioni**; e **Rifiuta tutte le modifiche visualizzate** e **Rifiuta tutte le modifiche**. "Visualizzate" indica le modifiche dei tipi attivati in **Mostra markup** (e, nel **Riquadro revisioni**, del revisore scelto).
- **Precedente** e **Successiva**, nel gruppo **Modifiche**, passano da una modifica all'altra senza cambiare nulla. Con il cursore all'inizissimo del testo, di un'intestazione o di una nota, **Successiva** va a una modifica che inizia proprio lì.

Ciascuno di questi comandi è un singolo passaggio, quindi **Ctrl+Z** riporta il documento esattamente com'era.

Accettare un'eliminazione rimuove definitivamente il testo. Rifiutare un inserimento rimuove anch'esso il testo. Accettare un inserimento mantiene il testo e toglie il segno che era nuovo. Rifiutare un'eliminazione mantiene il testo e toglie la barratura. Rifiutare una modifica dell'aspetto ripristina l'aspetto che il testo aveva. Il testo spostato è una coppia di modifiche, una dove è partito e una dove è arrivato, unite dal nome dello spostamento. Quando la coppia è contrassegnata con lo stesso nome, come la salvano altri programmi, accettare o rifiutare una delle due metà fa lo stesso con l'altra, quindi il testo viene spostato in modo definitivo oppure riportato al suo posto. Un nome usato più di una volta, oppure con una sola metà, non unisce nulla: ogni metà è una modifica a sé.

## Il Riquadro revisioni

Fai clic su **Revisione > Riquadro revisioni** per aprire l'elenco delle modifiche del documento: tipo, autore, data e parole. Un clic su una voce la mostra nel testo. Usa **Accetta**, **Rifiuta**, **Precedente** e **Successiva** sotto l'elenco, oppure scegli un solo revisore dalla casella sopra di esso per vedere soltanto le sue modifiche.

## Modifiche in intestazioni, piè di pagina, note e tabelle

Le modifiche apportate da altre persone in un'intestazione, in un piè di pagina, in una nota a piè di pagina o in una nota di chiusura vengono mostrate ed elencate come quelle nel testo, comprese un'intestazione o un piè di pagina che nessuna pagina usa (quello per le pagine pari, quando il documento non usa pagine pari e dispari diverse): **Successiva** le supera, e **Accetta** e **Rifiuta** eseguono la modifica a cui ti sei spostato per ultimo in quel punto. **Successiva**, **Precedente**, **Accetta**, **Rifiuta** e il **Riquadro revisioni** ti portano nell'intestazione, nel piè di pagina o nella nota che contiene la modifica; l'ordine è prima il testo, poi le note a piè di pagina, poi intestazioni e piè di pagina. **Accetta tutte** e **Rifiuta tutte** interessano tutti questi elementi.

Anche le modifiche alle tabelle sono elencate, nel testo e in intestazioni e piè di pagina: una riga o una cella inserita o eliminata, celle unite, e una modifica dell'aspetto di una tabella, di una riga o di una cella. Una barra nel margine indica le righe della riga o della cella. Accettare una riga eliminata, o rifiutare una riga inserita, rimuove la riga e il suo contenuto. Una riga in cui ogni cella presenta la modifica è la modifica di quella riga. Rifiutare una modifica dell'aspetto ripristina l'aspetto che la tabella, la riga o la cella avevano, e rifiutare un'unione divide di nuovo le celle. Quando eliminare una modifica danneggerebbe la tabella, la modifica viene mantenuta e il messaggio indica quante ne sono state conservate. Accade per una cella unita verso il basso con altre, che resta e perde il suo segno, e per una modifica alla griglia che le righe non possono seguire, che resta come modifica da accettare. Le modifiche all'interno di una tabella in una nota a piè di pagina vengono mantenute così come sono state scritte: non sono elencate, e Accetta e Rifiuta non le toccano.

Uno spostamento è una riga nell'elenco e una modifica nel conteggio di **Accetta tutte** e **Rifiuta tutte**: la sua altra metà e i segni di paragrafo che lo accompagnavano sono gestiti insieme. Le modifiche in una casella di testo sono elencate una per una, dopo quelle nel testo e prima di quelle nelle note a piè di pagina. **Successiva** e **Precedente** aprono la casella per la digitazione, con la modifica selezionata, e **Accetta** e **Rifiuta** risolvono quella modifica, ciascuna come un singolo passaggio. Spostare il cursore dentro la casella aperta fa proseguire **Successiva** e **Precedente** dalla posizione del cursore. Una casella risolta una modifica alla volta viene salvata senza la copia che leggono i programmi più vecchi, come accade quando digiti in una casella. Le prime 200 caselle che contengono modifiche vengono elencate in questo modo. Una casella di testo che fa parte di un gruppo, oppure il cui testo continua in altre caselle, una casella dentro un'altra casella, una casella in un'intestazione o in una nota, e le caselle oltre le prime 200 non possono essere aperte in questo modo: il **Riquadro revisioni** conta le loro modifiche in una riga sotto l'elenco, e solo **Accetta tutte** e **Rifiuta tutte** le gestiscono.

Una modifica alla numerazione di un paragrafo è elencata come **Numerato** o **Numerazione modificata**. Quando una modifica dell'aspetto di un paragrafo ha registrato l'elenco e il livello che il paragrafo aveva, rifiutarla rimette il paragrafo in quell'elenco, a quel livello (oppure lo toglie dall'elenco in cui non era). Una modifica che conserva la vecchia numerazione solo come etichetta (come la salvano alcuni programmi) non indica alcun elenco a cui tornare: accettarla mantiene la numerazione, e rifiutarla elimina la numerazione solo se l'etichetta è vuota; altrimenti lascia la modifica al suo posto e la conta come mantenuta. Rifiutare una numerazione aggiunta restituisce al paragrafo ciò che gli dà il suo stile (uno stile titolo che prevede una numerazione la mantiene), e nessuna numerazione dove lo stile non ne ha. Se l'elenco che una modifica dovrebbe ripristinare non è più nel documento, il paragrafo non cambia e la modifica viene contata come mantenuta.

Una modifica a una sezione (formato di pagina, orientamento, margini, colonne, numerazione delle pagine o tipo di interruzione) è elencata come **Sezione formattata**, sul paragrafo che chiude la sezione, oppure alla fine del testo per l'ultima sezione. Accettarla mantiene la sezione com'è ed elimina la registrazione; rifiutarla ripristina le proprietà registrate, e le pagine vengono impaginate di nuovo subito. Le intestazioni e i piè di pagina a cui la sezione fa riferimento non cambiano. Se le vecchie proprietà registrate non possono essere lette, rifiutare lascia la sezione e la modifica come sono, contandola come mantenuta; nulla viene sostituito con una pagina predefinita. Quando **Accetta tutte** o **Rifiuta tutte** rimuove un'interruzione di sezione o ne ripristina una, la modifica di ogni sezione viene risolta per prima, così non ne resta alcuna.

Una modifica che non può essere risolta senza danneggiare il documento (una tabella che si spezzerebbe, un elenco o una sezione il cui stato precedente non può essere ripristinato, una casella di testo troppo grande per essere riscritta) viene lasciata com'è, e il messaggio dopo **Accetta tutte** o **Rifiuta tutte** indica quante ne sono state mantenute.

## Quali modifiche vengono registrate e quali no

Con Traccia modifiche attiva vengono registrate: la digitazione, l'eliminazione e l'incollatura di testo; Invio; la sostituzione di parole; l'aspetto di testo e paragrafi (caratteri, dimensioni, colori, allineamento, spaziatura, bordi e stili); **Maiuscole/minuscole**; simboli e caratteri speciali; interruzioni di pagina e di colonna; il segno di paragrafo di un'interruzione di sezione; e, nelle tabelle, una riga che inserisci (con **Inserisci sopra** o **Inserisci sotto**, oppure con Tab nell'ultima cella), una riga che elimini, una tabella che elimini e una tabella che inserisci. Una riga inserita o eliminata compare nel **Riquadro revisioni** come un'unica modifica, con le sue parole, e **Accetta** e **Rifiuta** la mantengono oppure la tolgono. Eliminare una riga che contiene una tabella segna anche le righe di quella tabella. Una riga inserita da qualcun altro e da te eliminata porta entrambe le modifiche, e **Accetta** o **Rifiuta** la tolgono. Una riga che hai inserito tu stesso scompare in modo definitivo quando la elimini.

Queste modifiche vengono eseguite normalmente, senza segno, e la barra di stato indica ogni volta "Questa modifica non è stata registrata come modifica tracciata": elenchi puntati, numerazione e gli altri comandi degli elenchi; **Aumenta rientro** e **Riduci rientro**; impostazione pagina, colonne, margini e numeri di riga; intestazioni e piè di pagina, e la digitazione, l'eliminazione e la sostituzione di parole in un'intestazione, un piè di pagina, una nota a piè di pagina o una nota di chiusura; l'inserimento di una nota a piè di pagina, di una nota di chiusura, di un sommario, di un indice, di un campo, di un collegamento o di un controllo del contenuto; immagini, grafici, equazioni, caselle di testo e forme disegnate; colonne, celle, unione, divisione e aspetto di una tabella; l'eliminazione di una riga che fa parte di una cella unita verso il basso tra più righe; l'ordinamento di paragrafi o di righe di una tabella; e lo spostamento di una tabella o di una sezione. Mentre digiti in un'intestazione o in una nota, il messaggio compare una sola volta, non a ogni tasto. **Annulla** riporta indietro ciascuna di queste operazioni come un singolo passaggio.

Alcune modifiche non vengono eseguite affatto mentre Traccia modifiche è attiva, perché senza un segno danneggerebbero una modifica registrata: in una tabella che contiene una riga inserita o eliminata, **Elimina colonne**, **Elimina celle**, **Unisci celle**, **Dividi celle**, **Converti in testo** e l'incollatura sopra le celle. La barra di stato lo indica. Accetta o rifiuta prima le modifiche di quella tabella, oppure disattiva Traccia modifiche. In un documento che blocca Traccia modifiche come attiva, senza possibilità di accettare o rifiutare, anche queste operazioni vengono eseguite, e la barra di stato indica che la modifica non è stata registrata.

Le modifiche registrate da altri programmi nella numerazione e nelle sezioni vengono elencate e possono essere accettate e rifiutate, come descritto sopra.

## Confrontare due versioni

Confronta trova le differenze tra due versioni di un documento e le mostra come modifiche tracciate.

1. Apri la versione più vecchia. È l'**originale**.
2. Fai clic su **Confronta** nel gruppo **Confronta** della scheda **Revisione**.
3. Fai clic su **Sfoglia** e scegli la versione più recente. È la versione **revisionata**.
4. Digita il nome con cui firmare le modifiche. Scegli le opzioni che preferisci, ad esempio ignorare le maiuscole. **Trova testo spostato** e **Trova modifiche di formattazione** sono attive finché non le disattivi.
5. Fai clic su **OK**.

Si apre una nuova finestra con il testo originale e le modifiche che lo trasformano nel testo revisionato. I due file non vengono modificati. Usa **Accetta** e **Rifiuta** come per qualsiasi modifica tracciata. Se accetti tutte le modifiche, ottieni il testo revisionato; se le rifiuti tutte, ottieni il testo originale.

Confronta esamina il testo del corpo del documento. Non esamina tabelle, intestazioni, piè di pagina o note a piè di pagina.

Un paragrafo di 12 parole o più che è stato tagliato da un punto e spostato in un altro viene mostrato come **spostamento**: un record dove il testo è partito e uno dove è arrivato. Accettare o rifiutare uno dei due li gestisce entrambi. Il paragrafo deve essere identico, oppure avere almeno 9 parole su 10 uguali e per lo più nello stesso ordine; altrimenti viene mostrato come eliminazione e inserimento. Le parole diverse all'interno di un paragrafo spostato vengono mostrate come inserimenti ed eliminazioni all'interno dello spostamento.

Il testo uguale nelle due versioni ma con un aspetto diverso viene mostrato come **modifica di formattazione**. Confronta verifica grassetto, corsivo, sottolineatura, dimensione, carattere, colore, evidenziazione, e lo stile, l'allineamento, i rientri e la spaziatura di un paragrafo. Rifiutare la modifica ripristina il vecchio aspetto.

Se i due documenti hanno troppe modifiche, Confronta lo segnala. In quel caso mostra alcuni paragrafi come sostituiti per intero, e potrebbe non cercare spostamenti o modifiche di formattazione in tutti.
