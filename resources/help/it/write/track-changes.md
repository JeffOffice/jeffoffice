# Tenere traccia delle modifiche, e accettarle o rifiutarle
<!-- keywords: confronta, confronta documenti, due versioni, cosa è cambiato, traccia modifiche, modifiche registrate, revisioni, markup, accetta, rifiuta, accetta tutte, rifiuta tutte, visualizza per revisione, markup semplice, tutti i markup, nessun markup, originale, mostra markup, fumetti, riquadro revisioni, barra delle modifiche, inserimento, eliminazione, ctrl+maiusc+e, autore, revisore, nome utente, modifiche di un'altra persona -->

**Revisioni** registra ogni modifica che apporti, così che tu (o un'altra persona) possa vedere che cosa è cambiato e decidere se mantenerlo. Usala quando modifichi un documento che appartiene a qualcun altro, oppure che qualcuno rivedrà dopo di te.

## Attivare e disattivare Revisioni

Fai clic su **Revisione > Revisioni**, oppure premi **Ctrl+Maiusc+E**. La barra di stato in fondo alla finestra indica se la funzione è attiva. Un clic su quella scritta la attiva o la disattiva. L'impostazione viene salvata nel documento, quindi resta attiva quando il documento torna da un'altra persona.

Quando è attiva:

- **Ciò che digiti** viene contrassegnato come inserimento.
- **Ciò che elimini** non viene rimosso. Resta nel testo, barrato, così da poterlo recuperare. Fa eccezione il testo che hai digitato tu stesso mentre le revisioni erano attive: eliminarlo lo rimuove semplicemente, perché nessun altro lo ha mai visto.
- **Un'interruzione di paragrafo che inserisci** (Invio) viene contrassegnata come inserita, e una che elimini (Backspace all'inizio di un paragrafo) come eliminata.
- **Una modifica dell'aspetto** (grassetto, carattere, dimensione, colore, allineamento, rientri o stile) viene registrata insieme all'aspetto che il testo aveva prima.

Ogni modifica viene salvata con il nome di chi l'ha fatta e la data e l'ora. Il nome è quello indicato in **File > Opzioni**, alla voce **Nome utente**. Se è sbagliato, cambialo lì.

Ogni modifica è un'unica operazione, quindi **Ctrl+Z** la annulla come qualsiasi altra modifica.

## Com'è una modifica

Per impostazione predefinita, le modifiche di ciascuna persona hanno un colore proprio. La prima persona della cui modifica il documento mostra traccia riceve il primo colore, la successiva il secondo, e così via; una persona mantiene lo stesso colore finché il documento resta aperto. Il testo inserito è sottolineato con il colore della persona, e il testo eliminato è barrato. Il testo spostato è verde: doppio barrato dove è stato tolto, e doppio sottolineato dove è arrivato. Il testo la cui formattazione è stata modificata è in grassetto. Una barra nel margine sinistro segna ogni riga che contiene una modifica.

Puoi cambiare tutto questo. Fai clic su **Revisione > Traccia modifiche > Opzioni traccia modifiche** per aprire la finestra **Opzioni traccia modifiche**. Sotto **Markup**, ogni tipo di modifica ha un **Aspetto** e un **Colore**:

- **Inserimenti**: sottolineato, doppia sottolineatura, barrato, grassetto, corsivo, solo colore, oppure (nessuno).
- **Eliminazioni**: barrato, doppio barrato, nascosto, accento circonflesso, o solo colore. Con **Nascosto**, le parole eliminate non compaiono nella riga. Con **Accento circonflesso** non compaiono neppure, e un piccolo ^ indica dove si trovavano. Anche il testo spostato via viene omesso quando per le eliminazioni scegli **Fumetti**, ma non in caso contrario.
- **Spostato da** e **Spostato a**: aspetto e colori per il testo spostato. Sono disponibili finché è selezionata la casella **Traccia spostamenti**. Se la deselezioni, il testo spostato viene mostrato come una normale eliminazione e un normale inserimento.
- **Formattazione**: grassetto, solo colore, oppure (nessuno). Se deselezioni **Traccia formattazione**, le modifiche di formato non vengono segnalate nel testo.
- **Barre di modifica**: bordo sinistro, bordo destro, bordo esterno (il lato destro delle pagine dispari e il lato sinistro di quelle pari), oppure (nessuno), e il colore della barra.

Ogni casella **Colore** offre **Per autore** (il colore proprio di ogni persona) oppure un unico colore per tutte le persone. Queste scelte sono personali: vengono conservate su questo computer, non nel documento, valgono per ogni documento e per ogni finestra aperta, e ogni finestra cambia non appena fai clic su **OK**. Le barre vengono disegnate allo stesso modo sullo schermo, in una stampa e in un PDF, ogni volta che il markup è visualizzato.

## Scegliere che cosa vedere: Visualizza per revisione

Fai clic su **Revisione > Visualizza per revisione** e scegli una opzione:

- **Tutti i markup** mostra ogni modifica nel testo, con le barre nel margine.
- **Markup semplice** mostra il documento come apparirà con le modifiche accettate, e una barra nel margine accanto a ogni riga che contiene una modifica.
- **Nessun markup** mostra il documento come apparirà con tutte le modifiche accettate, senza altro.
- **Originale** mostra il documento come era prima di qualsiasi modifica: il testo eliminato torna al suo posto e il testo inserito scompare.

Queste opzioni cambiano solo ciò che vedi. Non modificano mai il documento e non vengono salvate con esso. Un documento che contiene modifiche si apre con **Tutti i markup**.

**Revisione > Mostra markup** sceglie quali tipi di modifica mostrare: **Inserimenti ed eliminazioni**, **Formattazione** e **Commenti**. Sotto di essi, scegli come mostrare il testo eliminato. **Mostra tutte le revisioni nel testo** lo mantiene nel testo, barrato. **Mostra revisioni nei fumetti** lo toglie dalla riga e lo mette in un fumetto nel margine. **Mostra solo commenti e formattazione nei fumetti** mantiene inserimenti ed eliminazioni nel testo.

## Accettare o rifiutare le modifiche

Accettare una modifica significa mantenerla in modo definitivo. Rifiutarla significa annullarla.

- **Revisione > Accetta** accetta la modifica in cui si trova il cursore (oppure ogni modifica del testo selezionato), poi passa alla successiva. **Revisione > Rifiuta** fa lo stesso, annullando la modifica.
- Fai clic sulla freccia sotto **Accetta** o **Rifiuta** per altre opzioni: **Accetta tutte le modifiche visualizzate**, **Accetta tutte le modifiche** e **Accetta tutte le modifiche e interrompi revisioni**; e **Rifiuta tutte le modifiche visualizzate** e **Rifiuta tutte le modifiche**. "Visualizzate" indica le modifiche dei tipi attivati in **Mostra markup** (e, nel **Riquadro revisioni**, del revisore scelto).
- **Precedente** e **Successiva**, nel gruppo **Modifiche**, passano da una modifica alla successiva senza cambiare nulla. Con il cursore all'inizissimo del testo, di un'intestazione o di una nota, **Successiva** va alla modifica che inizia proprio lì.

Ciascuna di queste operazioni è un'unica azione, quindi **Ctrl+Z** riporta il documento esattamente com'era.

Accettare un'eliminazione rimuove il testo in modo definitivo. Rifiutare un inserimento rimuove anch'esso il testo. Accettare un inserimento mantiene il testo e toglie il segno che era nuovo. Rifiutare un'eliminazione mantiene il testo e toglie la barratura. Rifiutare una modifica dell'aspetto ripristina l'aspetto che il testo aveva. Il testo spostato è una coppia di modifiche, una dove è stato tolto e una dove è arrivato, unite dal nome dello spostamento. Quando la coppia è contrassegnata con lo stesso nome, come la salvano altri programmi, accettare o rifiutare una delle due metà fa lo stesso per l'altra, così il testo viene spostato in modo definitivo oppure rimesso al suo posto. Un nome usato più volte, o che compare in una sola metà, non unisce nulla: ogni metà è una modifica a sé.

## Il riquadro Revisioni

Fai clic su **Revisione > Riquadro revisioni** per aprire un elenco delle modifiche nel documento: tipo, autore, data e parole. Un clic su una voce la mostra nel testo. Usa **Accetta**, **Rifiuta**, **Precedente** e **Successiva** sotto l'elenco, oppure scegli un solo revisore dalla casella sopra di esso per vedere solo le modifiche di quella persona.

## Modifiche in intestazioni, piè di pagina, note e tabelle

Le modifiche che altre persone hanno fatto in un'intestazione, in un piè di pagina, in una nota a piè di pagina o in una nota di chiusura vengono mostrate ed elencate come quelle nel testo, compresi un'intestazione o un piè di pagina che nessuna pagina usa (quello per le pagine pari, quando il documento non usa pagine pari e dispari diverse): **Successiva** lo supera, e **Accetta** e **Rifiuta** eseguono la modifica a cui sei arrivato per ultimo. **Successiva**, **Precedente**, **Accetta**, **Rifiuta** e il **Riquadro revisioni** ti portano nell'intestazione, nel piè di pagina o nella nota che contiene la modifica; l'ordine è prima il testo, poi le note a piè di pagina, poi le intestazioni e i piè di pagina. **Accetta tutte** e **Rifiuta tutte** riguardano tutte queste parti.

Anche le modifiche alle tabelle vengono elencate, nel testo e in intestazioni e piè di pagina: una riga o una cella inserita o eliminata, celle unite, e una modifica dell'aspetto di una tabella, di una riga o di una cella. Una barra nel margine segna le righe della riga o della cella. Accettare una riga eliminata, o rifiutare una riga inserita, rimuove la riga e le sue parole. Una riga in cui ogni cella ha la modifica è la modifica di quella riga. Rifiutare una modifica dell'aspetto ripristina l'aspetto che la tabella, la riga o la cella avevano, e rifiutare un'unione divide di nuovo le celle. Quando eliminare una modifica danneggerebbe la tabella, la modifica viene invece mantenuta e il messaggio indica quante ne sono state mantenute. Succede per una cella unita con altre in basso, che resta e perde il suo segno, e per una modifica della griglia che le righe non riescono a seguire, che resta come modifica da accettare. Le modifiche all'interno di una tabella presente in una nota a piè di pagina vengono mantenute così come sono state scritte: non vengono elencate, e Accetta e Rifiuta non le eseguono.

Uno spostamento occupa una sola riga nell'elenco e una sola modifica nel conteggio di **Accetta tutte** e **Rifiuta tutte**: la sua altra metà e i segni di paragrafo che lo accompagnavano vengono sbrigati con esso. Le modifiche all'interno di una casella di testo sono elencate una per una, dopo le modifiche nel testo e prima di quelle nelle note a piè di pagina. **Successiva** e **Precedente** aprono la casella in modalità di digitazione, con la modifica selezionata, e **Accetta** e **Rifiuta** risolvono quella modifica, ciascuna come un'unica azione. Spostare il cursore dentro la casella aperta fa proseguire **Successiva** e **Precedente** dal cursore. Una casella risolta una modifica alla volta viene salvata senza la copia che leggono i programmi più vecchi, come accade quando digiti in una casella. Vengono elencate in questo modo le prime 200 caselle che contengono modifiche. Una casella di testo che fa parte di un gruppo, o il cui testo prosegue in altre caselle, una casella dentro un'altra casella, una casella in un'intestazione o in una nota, e le caselle oltre le prime 200 non possono essere aperte in questo modo: il **Riquadro revisioni** conta le loro modifiche in una riga sotto l'elenco, e solo **Accetta tutte** e **Rifiuta tutte** le eseguono.

Una modifica alla numerazione di un paragrafo viene elencata come **Numerato** o **Numerazione modificata**. Quando una modifica dell'aspetto di un paragrafo ha registrato l'elenco e il livello che il paragrafo aveva, rifiutarla rimette il paragrafo in quell'elenco, a quel livello (oppure lo toglie dall'elenco in cui non era). Una modifica che conserva la vecchia numerazione solo come etichetta (come fanno alcuni programmi) non indica alcun elenco a cui tornare: accettarla mantiene la numerazione, e rifiutarla la toglie solo se l'etichetta è vuota; altrimenti lascia la modifica al suo posto e la conta come mantenuta. Rifiutare una numerazione aggiunta riporta il paragrafo a ciò che gli dà il suo stile (uno stile di titolo che prevede una numerazione la mantiene), e nessuna numerazione dove lo stile non ne prevede. Se l'elenco che una modifica dovrebbe ripristinare non è più nel documento, nulla cambia in quel paragrafo e la modifica viene contata come mantenuta.

Una modifica a una sezione (formato carta, orientamento, margini, colonne, numerazione delle pagine o tipo di interruzione) viene elencata come **Sezione formattata**, nel paragrafo che termina la sezione, oppure alla fine del testo per l'ultima sezione. Accettarla mantiene la sezione così com'è ed elimina la registrazione, mentre rifiutarla ripristina le proprietà che la registrazione aveva conservato, e le pagine vengono impaginate di nuovo subito. Le intestazioni e i piè di pagina indicati dalla sezione non cambiano. Se le vecchie proprietà registrate non possono essere lette, rifiutare lascia la sezione e la modifica come sono, e le conta come mantenute; non viene sostituito nulla con una pagina predefinita. Quando **Accetta tutte** o **Rifiuta tutte** elimina anche un'interruzione di sezione o ne ripristina una, prima viene risolta la modifica di ciascuna sezione, così non ne resta nessuna indietro.

Una modifica che non può essere risolta senza danneggiare il documento (una tabella che si romperebbe, un elenco o una sezione il cui stato precedente non può essere ripristinato, una casella di testo troppo grande per essere riscritta) viene lasciata com'è, e il messaggio dopo **Accetta tutte** o **Rifiuta tutte** indica quante ne sono state mantenute.

## Che cosa non è ancora registrato

Le modifiche che fai tu stesso a tabelle (righe, celle, unioni), numerazione ed elenchi, sezioni e impostazioni di pagina, e al testo di intestazioni, piè di pagina e note vengono eseguite normalmente, senza registrazione. Le modifiche registrate da altri programmi nella numerazione e nelle sezioni vengono elencate e possono essere accettate e rifiutate, come descritto sopra.

## Confrontare due versioni

Confronta trova le differenze tra due versioni di un documento e le mostra come modifiche registrate.

1. Apri la versione più vecchia. È l'**originale**.
2. Fai clic su **Confronta** nel gruppo **Confronta** della scheda **Revisione**.
3. Fai clic su **Sfoglia** e scegli la versione più recente. È la versione **rivista**.
4. Digita il nome con cui firmare le modifiche. Scegli le opzioni che preferisci, ad esempio ignorare le maiuscole. **Trova il testo spostato** e **Trova le modifiche di formattazione** sono attive, a meno che tu non le disattivi.
5. Fai clic su **OK**.

Si apre una nuova finestra con il testo originale e le modifiche che lo trasformano nel testo rivisto. I due file non vengono modificati. Usa **Accetta** e **Rifiuta** come per qualsiasi modifica registrata. Se accetti tutte le modifiche, ottieni il testo rivisto. Se le rifiuti tutte, ottieni il testo originale.

Confronta esamina il testo del corpo del documento. Non prende in considerazione tabelle, intestazioni, piè di pagina o note a piè di pagina.

Un paragrafo di 12 parole o più che è stato tolto da un punto e inserito in un altro viene mostrato come **spostamento**: una registrazione dove il testo è partito e una dove è arrivato. Accettare o rifiutare una delle due le sbriga entrambe. Il paragrafo deve essere identico, oppure avere almeno 9 parole su 10 uguali e in gran parte nello stesso ordine; altrimenti viene mostrato come eliminazione e inserimento. Le parole diverse all'interno di un paragrafo spostato vengono mostrate come inserimenti ed eliminazioni all'interno dello spostamento.

Il testo uguale nelle due versioni ma con un aspetto diverso viene mostrato come **modifica di formattazione**. Confronta controlla grassetto, corsivo, sottolineatura, dimensione, carattere, colore, evidenziazione, oltre allo stile, all'allineamento, ai rientri e alla spaziatura di un paragrafo. Rifiutare la modifica ripristina il vecchio aspetto.

Se i due documenti presentano troppe modifiche, Confronta lo segnala. In tal caso mostra alcuni paragrafi come sostituiti per intero, e potrebbe non cercare spostamenti o modifiche di formattazione in tutti.
