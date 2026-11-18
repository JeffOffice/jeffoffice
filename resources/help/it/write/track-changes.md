# Tieni traccia delle modifiche e accettale o rifiutale

<!-- keywords: confronta, confronta documenti, due versioni, cosa è cambiato, traccia modifiche, modifiche registrate, revisioni, markup, accetta, rifiuta, accetta tutto, rifiuta tutto, visualizza per revisione, markup semplice, tutto il markup, nessun markup, originale, mostra markup, fumetti, riquadro revisioni, barra delle modifiche, inserimento, eliminazione, ctrl+maiusc+e, autore, revisore, nome utente, modifiche di un altro utente -->

**Traccia modifiche** registra ogni modifica che fai, così tu o qualcun altro potete vedere che cosa è cambiato e decidere se mantenerla. Usala quando modifichi un documento che appartiene a qualcun altro, oppure che qualcuno rivedrà dopo di te.

## Attivare e disattivare Traccia modifiche

Fai clic su **Revisione > Traccia modifiche**, oppure premi **Ctrl+Maiusc+E**. La barra di stato in fondo alla finestra indica se la funzione è attiva. Un clic su quelle parole la attiva o la disattiva. L'impostazione viene salvata nel documento, quindi resta attiva quando il documento torna da qualcun altro.

Quando è attiva:

- **Quello che digiti** viene contrassegnato come inserimento.
- **Quello che elimini** non viene rimosso. Resta nel testo, barrato, in modo da poterlo ripristinare. Fa eccezione il testo che hai digitato tu stesso mentre la traccia era attiva: eliminarlo lo toglie e basta, perché nessun altro lo ha mai visto.
- **Un'interruzione di paragrafo** che inserisci (Invio) viene contrassegnata come inserita, e una che elimini (Backspace all'inizio di un paragrafo) come eliminata.
- **Una modifica dell'aspetto** (grassetto, carattere, dimensione, colore, allineamento, rientri o stile) viene registrata insieme all'aspetto che il testo aveva prima.

Ogni modifica viene salvata con il nome di chi l'ha fatta e con la data e l'ora. Il nome è quello indicato in **File > Opzioni**, alla voce **Nome utente**. Se è sbagliato, cambialo lì.

Ogni modifica è un'unica operazione, quindi **Ctrl+Z** la annulla come qualsiasi altra modifica.

## Come appare una modifica

Per impostazione predefinita, le modifiche di ogni persona hanno un colore proprio. La prima persona le cui modifiche compaiono nel documento riceve il primo colore, la seconda il secondo colore, e così via. Una persona mantiene lo stesso colore finché il documento è aperto. Il testo inserito è sottolineato con il colore della persona, e il testo eliminato è barrato. Il testo spostato è verde: barrato doppio dove è stato tolto e sottolineato doppio dove è arrivato. Il testo di cui è stato modificato l'aspetto è in grassetto. Una barra nel margine sinistro segna ogni riga che contiene una modifica.

Puoi cambiare tutto questo. Fai clic su **Revisione > Traccia modifiche > Opzioni traccia modifiche** per aprire la finestra **Opzioni traccia modifiche**. Nella sezione **Markup**, ogni tipo di modifica ha un **Aspetto** e un **Colore**:

- **Inserimenti**: sottolineato, sottolineato doppio, barrato, grassetto, corsivo, solo colore, oppure (nessuno).
- **Eliminazioni**: barrato, barrato doppio, nascosto, accento circonflesso, oppure solo colore. Con **Nascosto**, le parole eliminate non compaiono nella riga. Con **Accento circonflesso** non compaiono neppure, ma un piccolo ^ indica dove si trovavano. Anche il testo spostato via viene omesso quando per le eliminazioni scegli **Fumetti**, ma non in altri casi.
- **Spostato da** e **Spostato a**: l'aspetto e i colori del testo spostato. Sono disponibili quando è selezionata la casella **Traccia spostamenti**. Se la deselezioni, il testo spostato appare come una normale eliminazione e un normale inserimento.
- **Formattazione**: grassetto, solo colore, oppure (nessuno). Se deselezioni **Traccia formattazione**, le modifiche di formato non vengono contrassegnate nel testo.
- **Barre delle modifiche**: bordo sinistro, bordo destro, bordo esterno (il lato destro delle pagine dispari e il lato sinistro di quelle pari), oppure (nessuno), e il colore della barra.

Ogni casella **Colore** offre **Per autore** (il colore proprio di ciascuna persona) oppure un unico colore per tutti. Queste scelte sono personali: vengono memorizzate su questo computer, non nel documento, valgono per ogni documento e per ogni finestra aperta, e ogni finestra cambia non appena fai clic su **OK**. Le barre vengono disegnate allo stesso modo sullo schermo, nelle stampe e in un PDF, ogni volta che il markup è visualizzato.

## Scegliere cosa vedere: Visualizza per revisione

Fai clic su **Revisione > Visualizza per revisione** e scegli una voce:

- **Tutto il markup** mostra ogni modifica nel testo, con le barre nel margine.
- **Markup semplice** mostra il documento come apparirà con le modifiche accettate, e una barra nel margine accanto a ogni riga che contiene una modifica.
- **Nessun markup** mostra il documento come apparirà con tutte le modifiche accettate, senza altro.
- **Originale** mostra il documento come era prima di qualsiasi modifica: il testo eliminato torna al suo posto e il testo inserito scompare.

Queste voci cambiano solo ciò che vedi. Non modificano il documento e non vengono salvate con esso. Un documento che contiene modifiche si apre con **Tutto il markup**.

**Revisione > Mostra markup** sceglie quali tipi di modifica mostrare: **Inserimenti ed eliminazioni**, **Formattazione** e **Commenti**. Sotto di essi scegli come mostrare il testo eliminato. **Mostra tutte le revisioni nel testo** lo mantiene nel testo, barrato. **Mostra revisioni nei fumetti** lo toglie dalla riga e lo mette in un fumetto nel margine. **Mostra solo commenti e formattazione nei fumetti** mantiene inserimenti ed eliminazioni nel testo.

## Accettare o rifiutare le modifiche

Accettare una modifica significa mantenerla in modo definitivo. Rifiutarla significa annullarla.

- **Revisione > Accetta** accetta la modifica in cui si trova il cursore (oppure ogni modifica nel testo selezionato), poi passa alla successiva. **Revisione > Rifiuta** fa lo stesso, annullando la modifica.
- Fai clic sulla freccia sotto **Accetta** o **Rifiuta** per altre opzioni: **Accetta tutte le modifiche visualizzate**, **Accetta tutte le modifiche** e **Accetta tutte le modifiche e interrompi revisioni**; e **Rifiuta tutte le modifiche visualizzate** e **Rifiuta tutte le modifiche**. "Visualizzate" indica le modifiche dei tipi attivati in **Mostra markup** (e, nel **Riquadro revisioni**, del revisore scelto).
- **Precedente** e **Successiva** nel gruppo **Modifiche** passano da una modifica all'altra senza cambiare nulla.

Ognuna di queste operazioni è un'unica azione, quindi **Ctrl+Z** riporta il documento esattamente com'era.

Accettare un'eliminazione rimuove il testo in modo definitivo. Rifiutare un inserimento rimuove anche il testo. Accettare un inserimento mantiene il testo e toglie il segno che indicava la novità. Rifiutare un'eliminazione mantiene il testo e toglie la barratura. Rifiutare una modifica dell'aspetto ripristina l'aspetto che il testo aveva. Il testo spostato è una coppia di modifiche: una dove è stato tolto e una dove è arrivato. Quando la coppia è contrassegnata con lo stesso nome, come la salvano altri programmi, accettare o rifiutare una delle due metà produce lo stesso effetto sull'altra, quindi il testo viene spostato in modo definitivo oppure riportato al suo posto.

## Il riquadro Revisioni

Fai clic su **Revisione > Riquadro revisioni** per aprire un elenco delle modifiche nel documento: tipo, autore, data e parole. Un clic su una voce la mostra nel testo. Usa **Accetta**, **Rifiuta**, **Precedente** e **Successiva** sotto l'elenco, oppure scegli un solo revisore dalla casella sopra di esso per vedere solo le modifiche di quella persona.

## Che cosa non viene ancora registrato

Le modifiche alle tabelle (righe, celle, unione di celle), alla numerazione e agli elenchi, alle sezioni e all'impostazione di pagina, e al testo di intestazioni, piè di pagina e note vengono eseguite normalmente, senza una registrazione. Le modifiche che altri programmi hanno registrato in questi punti vengono conservate e salvate così come sono.

## Confrontare due versioni

Confronta trova le differenze tra due versioni di un documento e le mostra come modifiche tracciate.

1. Apri la versione più vecchia. È l'**originale**.
2. Fai clic su **Confronta** nel gruppo **Confronta** della scheda **Revisione**.
3. Fai clic su **Sfoglia** e scegli la versione più recente. È la versione **rivista**.
4. Digita il nome con cui firmare le modifiche. Scegli le opzioni che preferisci, ad esempio ignorare le maiuscole. **Trova testo spostato** e **Trova modifiche di formato** sono attive finché non le disattivi.
5. Fai clic su **OK**.

Si apre una nuova finestra con il testo originale e le modifiche che lo trasformano nel testo rivisto. I due file non vengono modificati. Usa **Accetta** e **Rifiuta** come per qualsiasi modifica tracciata. Se accetti tutte le modifiche, ottieni il testo rivisto. Se le rifiuti tutte, ottieni il testo originale.

Confronta esamina il testo del corpo del documento. Non esamina tabelle, intestazioni, piè di pagina o note a piè di pagina.

Un paragrafo di 12 parole o più che è stato tagliato da un punto e incollato in un altro viene mostrato come **spostamento**: una registrazione dove il testo è uscito e una dove è arrivato. Accettare o rifiutare una delle due registrazioni interessa entrambe. Il paragrafo deve essere identico, oppure almeno 9 parole su 10 devono coincidere e in gran parte nello stesso ordine; altrimenti viene mostrato come un'eliminazione e un inserimento. Le parole diverse all'interno di un paragrafo spostato vengono mostrate come inserimenti ed eliminazioni all'interno dello spostamento.

Il testo uguale nelle due versioni ma con un aspetto diverso viene mostrato come **modifica di formattazione**. Confronta controlla grassetto, corsivo, sottolineatura, dimensione, carattere, colore, evidenziazione, e lo stile, l'allineamento, i rientri e la spaziatura di un paragrafo. Rifiutare la modifica ripristina il vecchio aspetto.

Se i due documenti contengono troppe modifiche, Confronta lo segnala. In quel caso mostra alcuni paragrafi come sostituiti per intero e potrebbe non cercare spostamenti o modifiche di formattazione in tutti.
