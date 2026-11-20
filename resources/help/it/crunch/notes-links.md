# Note, commenti e collegamenti

<!-- keywords: sposta nota, ridimensiona nota, dimensioni nota, casella nota, reimposta dimensioni nota, tastiera, lingua, lingua di correzione, imposta lingua, non controllare ortografia, traduci, traduzione, libretranslate, riquadro commenti, nuovo commento, rispondi, risolvi, riapri, discussione, commento precedente, commento successivo, mostra commenti, converti note in commenti, converti commenti in note, ortografia, controllo ortografia, errore di ortografia, F7, ignora tutto, cambia tutto, aggiungi al dizionario, thesaurus, sinonimi, elenco parole, nota, note, commento, commenti, passaggio del puntatore, segno rosso, angolo, nuova nota, modifica nota, elimina nota, mostra nota, nascondi nota, mostra tutte le note, collegamento, collegamenti, collegamento ipertestuale, indirizzo, pagina web, posta elettronica, posizione in questa cartella di lavoro, Ctrl+K, Ctrl+clic, descrizione comando, apri collegamento, rimuovi collegamento, conversazione, statistiche cartella di lavoro, conteggio, celle, formule, grafici, tabelle pivot, accessibilità, testo alternativo, celle unite, file recenti -->

## Note

Una **nota** è un breve testo associato a una cella. Una cella che ne contiene una mostra un piccolo segno rosso nell'angolo in alto a destra. Posiziona il puntatore sulla cella e la nota compare accanto a essa; sposta il puntatore altrove e la nota scompare.

Per aggiungere una nota a una cella, selezionala e usa **Revisione > Nuova nota** (oppure premi **Maiusc+F2**). Digita il testo e fai clic su **OK**. Su una cella che ha già una nota lo stesso comando diventa **Modifica nota**. Se svuoti il testo, la nota viene eliminata.

- **Elimina nota** rimuove le note delle celle selezionate.
- **Mostra o nascondi nota** mantiene sempre visibile, in una casella accanto a ciascuna cella, la nota delle celle selezionate, oppure la nasconde di nuovo.
- **Mostra tutte le note** fa lo stesso per tutte le note del foglio; il pulsante diventa allora **Nascondi tutte le note**.
- **Nota precedente** e **Nota successiva** spostano da una cella con nota alla successiva.
- **Sposta o ridimensiona nota** sposta o ridimensiona con i tasti la casella di una nota visualizzata. Seleziona la cella e usa il comando (**Revisione > Note**, oppure fai clic destro sulla cella). Poi i **tasti freccia** spostano la casella di 10 pixel, **Maiusc con una freccia** la rende più larga, più stretta, più alta o più bassa di 10 pixel, e **Ctrl** (**Opzione** su Mac) rende il passo di 1 pixel; le combinazioni con Alt o Meta (o Comando su Mac) in quel frangente non fanno nulla, e qualsiasi altro comando, un cambio di foglio, un Annulla o un Ripristina fa perdere la casella. **Invio** mantiene la casella dove si trova, come un unico passaggio annullabile, e **Esc** annulla lo spostamento. Un lettore di schermo indica la posizione della casella dopo ogni tasto, in pixel con zoom al 100% (quanto a destra e quanto in basso rispetto all'angolo superiore sinistro della cella, e quanto è larga e alta la casella), e lo comunica quando rilasci un tasto tenuto premuto, non a ogni ripetizione; annuncia anche quando la casella viene mantenuta o persa. Una casella è larga al massimo 640 pixel e alta al massimo 480, e inizia entro 1.000 pixel dalla sua cella, come quando la trascini. Trascinare la casella e i suoi otto quadratini con il puntatore produce lo stesso risultato.
- **Reimposta dimensioni nota** riporta la casella della nota alle impostazioni predefinite: accanto alla cella, con le dimensioni necessarie al testo.

Ciascuno di questi comandi è un passaggio annullabile. Una nota segue la sua cella quando si inseriscono o eliminano righe o colonne, e quando le celle vengono ordinate. **Cancella tutto** elimina le note e i commenti delle celle che cancella.

Una cella può anche contenere una conversazione di **commenti**, con risposte. Vedi la parte successiva.

Le note possono essere salvate in una cartella di lavoro .xlsx, .ods e .xls. Una cartella di lavoro aperta da un file .ods o .xls mostra le sue note, e puoi aggiungerle, modificarle, spostarle ed eliminarle come in una cartella di lavoro .xlsx. La data di una nota .ods non viene conservata quando JeffCrunch salva. Una conversazione di commenti salvata come file .ods o .xls diventa una nota semplice con il suo testo, e resta una nota semplice quando il file viene aperto. Un file .xls memorizza la distanza di una casella dalla cella in una dimensione fissa, quindi una casella posta molto a destra in una cella larga può spostarsi leggermente. Una cartella di lavoro aperta da un file di testo non include note.

## Commenti

Un **commento** è simile a una nota, ma avvia una **discussione**: altre persone possono rispondere, e una di esse può contrassegnare la discussione come **risolta** quando la domanda ha ricevuto risposta. Ogni commento indica chi lo ha scritto e quando. Una cella con una discussione mostra lo stesso segno rosso di una cella con una nota. Posiziona il puntatore sulla cella per leggere l'intera discussione, con ogni risposta rientrata sotto il commento a cui risponde. Una discussione risolta inizia con la parola **Risolto**.

Nella scheda **Revisione**, nel gruppo **Commenti**:

- **Nuovo commento** (**Ctrl+Maiusc+F2**) avvia una discussione sulla cella selezionata. Digita il testo e fai clic su **OK**. Funziona su una cella che non ha note né commenti.
- **Rispondi** aggiunge il tuo testo alla discussione sulla cella selezionata.
- **Risolvi discussione** contrassegna la discussione come conclusa. Lo stesso pulsante diventa poi **Riapri discussione**, che la riapre.
- **Elimina commento** rimuove le discussioni delle celle selezionate.
- **Commento precedente** e **Commento successivo** spostano da una cella con discussione alla successiva.
- **Mostra commenti** apre il riquadro **Commenti** accanto al foglio. Elenca ogni discussione del foglio come una scheda: la cella, ciascun commento con autore e ora, e i pulsanti **Rispondi**, **Risolvi** o **Riapri**, ed **Elimina**. **Vai a** sposta alla cella. Il riquadro segue ogni modifica, anche quella annullata con Annulla.
- **Converti note in commenti** trasforma ogni nota semplice del foglio in una discussione con un solo commento. **Converti commenti in note** trasforma ogni discussione di nuovo in una nota. Una nota non ha risposte, quindi i commenti di una discussione diventano un'unica nota con tutto il loro testo, e JeffCrunch chiede conferma prima di farlo su una discussione che ha risposte.

Ciascuno di questi comandi è un passaggio annullabile.

I programmi che non sanno mostrare le discussioni visualizzano al loro posto una nota semplice. JeffCrunch conserva questa nota, con il testo della discussione, a ogni salvataggio. Discussioni, risposte e autori restano invariati quando salvi e riapri la cartella di lavoro. Una discussione con più di 500 commenti viene mostrata e conservata, ma JeffCrunch non vi aggiunge nulla e non la modifica.

I commenti sono salvati in una cartella di lavoro .xlsx. Una cartella di lavoro aperta da un tipo di file più vecchio, o da un file di testo, conserva i suoi commenti solo dopo che la salvi come cartella di lavoro .xlsx.

## Collegamenti

Un **collegamento** ti porta altrove quando lo segui: a una pagina web, a un file, a un indirizzo di posta elettronica o a una posizione di questa cartella di lavoro (una cella, più celle o un nome).

Per inserire un collegamento in una cella, selezionala e usa **Inserisci > Collegamento** (oppure premi **Ctrl+K**). Scegli dove deve portare, digita l'indirizzo o la posizione (oppure scegli uno dei file aperti di recente da **O un file recente**) e, se vuoi, il testo che la cella deve mostrare e una **descrizione comando**. Su una cella che ha già un collegamento lo stesso comando diventa **Modifica collegamento**. **Rimuovi collegamento** elimina i collegamenti delle celle selezionate.

Posiziona il puntatore su una cella con collegamento per vedere dove porta. Per seguirlo, tieni premuto **Ctrl** e fai clic sulla cella, oppure seleziona la cella e usa **Apri collegamento** dal menu del clic destro. Un semplice clic seleziona soltanto la cella.

JeffCrunch non apre mai nulla da solo. Una pagina web o un indirizzo di posta elettronica si apre nel programma abituale quando segui il collegamento. Un collegamento a un file, o a un altro tipo di indirizzo, ti chiede prima conferma. Un collegamento che eseguirebbe uno script non viene mai aperto.

Un collegamento segue la sua cella quando si inseriscono o eliminano righe o colonne, e quando le celle vengono ordinate; viene inoltre salvato con la cartella di lavoro.

## Statistiche cartella di lavoro

**Statistiche cartella di lavoro** (scheda **Revisione**) conta il contenuto della cartella di lavoro: quanti **Fogli** contiene, quante **Celle con dati** (celle che contengono un numero, un testo o una formula), quante **Formule**, quanti **Grafici** e quante **Tabelle pivot**. I fogli eliminati non vengono conteggiati. Gli stessi numeri compaiono nella pagina **Informazioni** della scheda File.

## Ortografia

**Ortografia** (scheda **Revisione**, oppure **F7**) controlla le parole delle celle di testo, una parola sbagliata alla volta. Se hai selezionato più di una cella, controlla solo quelle. Altrimenti controlla l'intero foglio. Non esamina formule, numeri o date.

La finestra mostra la parola che non è nel dizionario, una casella **Cambia in** con la proposta migliore e un elenco di altri suggerimenti. Scegli un suggerimento o digita la tua parola, quindi scegli:

- **Cambia** sostituisce la parola sbagliata con quella nuova in quella cella. Ogni modifica è un passaggio annullabile.
- **Cambia tutto** la sostituisce con la nuova parola in tutte le celle controllate, come un unico passaggio annullabile.
- **Ignora** lascia la parola così com'è questa volta. **Ignora tutto** la lascia così ovunque finché non chiudi JeffCrunch.
- **Aggiungi** inserisce la parola nel tuo dizionario personale, così non verrà più segnalata. Il dizionario è condiviso con le altre app di JeffOffice.

**File > Opzioni > Strumenti di correzione** stabilisce se vengono controllate le parole in maiuscolo e quelle che contengono numeri. JeffCrunch controlla nella lingua in cui viene visualizzato, se ne ha un dizionario, e in inglese americano in caso contrario.

Mentre digiti in una cella, una parola che non è nel dizionario riceve una sottolineatura rossa ondulata. Fai clic destro sulla parola per scegliere una correzione, oppure per selezionare **Ignora tutto** o **Aggiungi al dizionario**. Le celle della griglia non vengono sottolineate, e un valore che inizia con il segno di uguale non viene mai controllato. Per vedere, modificare, importare o esportare l'elenco delle parole aggiunte, e per elencare le parole che vengono sempre segnalate come errate, fai clic su **File > Opzioni > Strumenti di correzione > Dizionario personalizzato**. Gli elenchi sono gli stessi in ogni app di JeffOffice.

## Imposta la lingua di alcune celle

Il controllo ortografico verifica ogni cella nella sua lingua. Per cambiarla, seleziona le celle e fai clic su **Imposta lingua di correzione** nel gruppo **Lingua** della scheda **Revisione**. Scegli la lingua nell'elenco, quindi fai clic su **OK**. Una cella non contrassegnata viene controllata nella lingua impostata in **File > Opzioni**. Un foglio di calcolo OpenDocument (.ods) mantiene la lingua di ciascuna cella. Una cartella di lavoro .xlsx non ha un posto per questa informazione, quindi la lingua viene conservata solo finché la cartella di lavoro è aperta.

## Non controllare alcune celle

Alcuni testi non devono essere controllati: un codice, un nome o una riga in una lingua inventata. Seleziona le celle e fai clic su **Non controllare ortografia** nel gruppo **Lingua**. **Ortografia** e le sottolineature rosse ondulate della cella li saltano. Fai di nuovo clic sul pulsante per controllare di nuovo le celle. Un file .ods conserva questo contrassegno, mentre una cartella di lavoro .xlsx lo conserva solo finché la cartella di lavoro è aperta.

## Traduci testo

1. Seleziona le celle, oppure parte delle parole che stai digitando in una cella. Il testo delle celle selezionate viene inviato una riga alla volta.
2. Fai clic su **Traduci** nel gruppo **Lingua**.
3. Scegli la lingua in cui tradurre.
4. Fai clic su **OK**.

Il browser web apre quindi un servizio di traduzione chiamato LibreTranslate, con il tuo testo già inserito. Non viene inviato nulla finché non fai clic su **OK**. Puoi cambiare l'indirizzo del servizio nella casella, ad esempio con uno che gestisci tu. Vengono usati solo indirizzi web sicuri (https), e indirizzi http per questo computer. Vengono inviate al massimo 5.000 lettere.

## Thesaurus

**Thesaurus** (scheda **Revisione**, oppure **Maiusc+F7**) trova parole che hanno quasi lo stesso significato di una parola in una cella. Cerca la prima parola del testo nella cella in cui ti trovi. Digita un'altra parola in **Cerca** e premi **Invio** per cercare quella. Scegli una parola nell'elenco e fai clic su **Sostituisci**, oppure fai doppio clic su di essa. La parola nella cella viene sostituita, e una prima lettera maiuscola resta maiuscola. **Cerca** cerca la parola scelta, per seguire una catena di significati. L'elenco delle parole funziona in inglese.

## Verifica accessibilità

**Verifica accessibilità** (scheda **Revisione**) apre una piccola finestra che elenca ciò che nella cartella di lavoro è difficile da usare per alcuni lettori, o da far leggere a un lettore di schermo (un programma che legge ad alta voce lo schermo). Ogni riga inizia con **Errore**, **Avviso** o **Suggerimento**. Fai clic su una riga per andare alla cella.

Cerca:

- Un grafico o un'immagine senza **testo alternativo** (le parole che descrivono ciò che mostra). Questo è un errore.
- Una tabella senza riga di intestazione, quindi con colonne senza nome. Questo è un errore.
- **Celle unite**, difficili da percorrere con la tastiera. Questo è un avviso.
- Testo difficile da leggere sul colore di sfondo, come testo grigio chiaro su bianco. JeffCrunch misura quanto il testo risalta sul riempimento. Il testo piccolo richiede un rapporto di almeno 4,5 a 1, mentre il testo grande (18 punti, oppure grassetto da 14 punti) richiede 3 a 1. Indica la prima cella di ogni combinazione di carattere e riempimento. Questo è un avviso.
- Un foglio che conserva ancora il nome assegnato automaticamente, come Foglio1. Questo è un suggerimento.

Premi **Verifica di nuovo** dopo aver apportato modifiche.
