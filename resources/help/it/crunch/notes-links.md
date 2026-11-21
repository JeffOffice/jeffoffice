# Note, commenti e collegamenti

<!-- keywords: rimuovi collegamenti ipertestuali, rimuovi aspetto collegamento, sposta nota, ridimensiona nota, casella nota, ripristina dimensioni nota, tastiera, lingua, lingua di correzione, imposta lingua, non controllare ortografia, traduci, traduzione, libretranslate, riquadro commenti, nuovo commento, rispondi, risolvi, riapri, discussione, commento precedente, commento successivo, mostra commenti, converti note in commenti, converti commenti in note, ortografia, controllo ortografia, errore ortografico, F7, ignora tutto, cambia tutto, aggiungi al dizionario, thesaurus, sinonimi, elenco di parole, nota, note, commento, commenti, passa il puntatore, segno rosso, angolo, nuova nota, modifica nota, elimina nota, mostra nota, nascondi nota, mostra tutte le note, collegamento, collegamenti, collegamento ipertestuale, indirizzo, pagina web, e-mail, posizione in questa cartella di lavoro, Ctrl+K, Ctrl+clic, descrizione comando, apri collegamento, rimuovi collegamento, conversazione, statistiche cartella di lavoro, conteggio, celle, formule, grafici, tabelle pivot, accessibilità, testo alternativo, celle unite, file recenti -->

## Note

Una **nota** è un breve testo associato a una cella. Una cella che ne contiene una mostra un piccolo segno rosso nell'angolo in alto a destra. Posiziona il puntatore sulla cella e la nota compare accanto a essa; quando sposti il puntatore, la nota scompare.

Per aggiungere una nota a una cella, selezionala e usa **Revisione > Nuova nota** (oppure premi **Maiusc+F2**). Digita il testo e fai clic su **OK**. Su una cella che ha già una nota, lo stesso comando riporta **Modifica nota**. Se cancelli il testo, la nota viene eliminata.

- **Elimina nota** elimina le note delle celle selezionate.
- **Mostra/Nascondi nota** mantiene visibile sul foglio, in una casella accanto a ciascuna cella, la nota delle celle selezionate, oppure la nasconde di nuovo.
- **Mostra tutte le note** fa lo stesso per tutte le note del foglio; lo stesso pulsante diventa allora **Nascondi tutte le note**.
- **Nota precedente** e **Nota successiva** passano da una cella con nota alla successiva.
- **Sposta o ridimensiona nota** sposta o ridimensiona con la tastiera la casella di una nota visibile. Seleziona la cella e usa il comando (**Revisione > Note**, oppure fai clic destro sulla cella). Poi i **tasti freccia** spostano la casella di 10 pixel, **Maiusc con un tasto freccia** la rende più larga, più stretta, più alta o più bassa di 10 pixel, e **Ctrl** (**Opzione** su un Mac) riduce il passo a 1 pixel. Le combinazioni con Alt o Meta (oppure Comando su un Mac) in questo frangente non fanno nulla, e qualsiasi altro comando, un cambio di foglio, un Annulla o un Ripeti fanno perdere la casella. **Invio** mantiene la casella dove si trova, come un unico passaggio annullabile, e **Esc** annulla la modifica. Un lettore di schermo indica la posizione della casella dopo ogni tasto, in pixel con zoom al 100% (quanto è distante da sinistra e dall'alto rispetto all'angolo in alto a sinistra della cella, e quanto è larga e alta la casella), e lo fa quando rilasci un tasto tenuto premuto, non a ogni ripetizione; indica anche quando la casella viene mantenuta o abbandonata. Una casella può essere larga al massimo 640 pixel e alta al massimo 480, e inizia entro 1.000 pixel dalla sua cella, come quando la trascini. Trascinare la casella e i suoi otto quadratini con il puntatore produce lo stesso risultato.
- **Ripristina dimensioni nota** riporta la casella della nota alle dimensioni predefinite: accanto alla cella, con la dimensione necessaria al testo.

Ciascuno di questi comandi è un passaggio annullabile. Una nota segue la sua cella quando si inseriscono o eliminano righe o colonne, e quando le celle vengono ordinate. **Cancella tutto** rimuove le note e i commenti dalle celle che cancella.

Una cella può anche contenere una conversazione di **commenti**, con risposte. Vedi la parte successiva.

Le note possono essere salvate in una cartella di lavoro .xlsx, in una cartella di lavoro .ods e in una cartella di lavoro .xls. Una cartella di lavoro aperta da un file .ods o .xls mostra le sue note, e puoi aggiungerle, modificarle, spostarle ed eliminarle come in una cartella di lavoro .xlsx. La data di una nota .ods non viene conservata quando JeffCrunch salva. Una conversazione di commenti salvata come file .ods o .xls diventa una nota semplice con il suo testo, e resta una nota semplice quando il file viene aperto. Un file .xls conserva la distanza di una casella dalla cella in una dimensione fissa, quindi una casella collocata lontano all'interno di una cella larga può spostarsi un po'. Una cartella di lavoro aperta da un file di testo non riporta note.

## Commenti

Un **commento** è simile a una nota, ma avvia una **discussione**: le persone possono rispondere, e una di loro può contrassegnare la discussione come **risolta** quando la domanda ha ricevuto risposta. Ogni commento mostra chi lo ha scritto e quando. Una cella con una discussione ha lo stesso segno rosso di una cella con una nota. Posiziona il puntatore sulla cella per leggere l'intera discussione, con ogni risposta rientrata sotto il commento a cui risponde. Una discussione risolta inizia con la parola **Risolto**.

Nella scheda **Revisione**, nel gruppo **Commenti**:

- **Nuovo commento** (**Ctrl+Maiusc+F2**) avvia una discussione sulla cella selezionata. Digita il testo e fai clic su **OK**. Funziona su una cella che non ha note né commenti.
- **Rispondi** aggiunge il tuo testo alla discussione sulla cella selezionata.
- **Risolvi discussione** contrassegna la discussione come conclusa. Lo stesso pulsante diventa poi **Riapri discussione**, che la riapre.
- **Elimina commento** elimina le discussioni delle celle selezionate.
- **Commento precedente** e **Commento successivo** passano da una cella con discussione alla successiva.
- **Mostra commenti** apre il riquadro **Commenti** accanto al foglio. Il riquadro elenca ogni discussione del foglio come una scheda: la cella, ciascun commento con autore e ora, e i pulsanti **Rispondi**, **Risolvi** o **Riapri**, ed **Elimina**. **Vai a** porta alla cella. Il riquadro segue ogni modifica, anche l'annullamento.
- **Converti note in commenti** trasforma ogni nota semplice del foglio in una discussione con un solo commento. **Converti commenti in note** riporta ogni discussione a una nota. Una nota non ha risposte, quindi i commenti di una discussione diventano un'unica nota con tutto il loro testo; JeffCrunch chiede conferma prima di farlo su una discussione che ha delle risposte.

Ciascuno di questi comandi è un passaggio annullabile.

I programmi che non sanno mostrare le discussioni visualizzano al loro posto una nota semplice. JeffCrunch conserva quella nota, con il testo della discussione, a ogni salvataggio. Discussioni, risposte e autori restano come erano quando salvi e riapri la cartella di lavoro. Una discussione con più di 500 commenti viene mostrata e conservata, ma JeffCrunch non vi aggiunge nulla e non la modifica.

I commenti sono salvati in una cartella di lavoro .xlsx. Una cartella di lavoro aperta da un tipo di file più vecchio, o da un file di testo, conserva i suoi commenti solo dopo che la salvi come cartella di lavoro .xlsx.

## Collegamenti

Un **collegamento** ti porta da qualche parte quando lo segui: a una pagina web, a un file, a un indirizzo e-mail, oppure a una posizione in questa cartella di lavoro (una cella, più celle o un nome).

Per inserire un collegamento in una cella, selezionala e usa **Inserisci > Collegamento** (oppure premi **Ctrl+K**). Scegli dove deve portare, digita l'indirizzo o la posizione (oppure scegli uno dei file aperti di recente da **Oppure un file recente**) e, se vuoi, il testo che la cella deve mostrare e una **descrizione comando**. Su una cella che ha già un collegamento, lo stesso comando riporta **Modifica collegamento**. **Rimuovi collegamento** elimina i collegamenti dalle celle selezionate. **Rimuovi collegamenti ipertestuali** (nel menu **Cancella** della scheda Home, e nel menu di scelta rapida di una cella con collegamento) elimina i collegamenti e anche l'aspetto che il collegamento dà alla cella, cioè il colore blu e la sottolineatura, così che non appaia più come un collegamento. Il resto della cella resta com'è: riempimento, formato numero, bordi, allineamento e carattere proprio. Una cella che hai colorato di blu e sottolineato a mano, senza lo stile Collegamento ipertestuale, mantiene quell'aspetto. Entrambe le operazioni sono passaggi annullabili, e un foglio protetto che non consente i collegamenti le rifiuta. I collegamenti e il loro aspetto vengono salvati con la cartella di lavoro, in tutti i formati che supportano i collegamenti.

Posiziona il puntatore su una cella con collegamento per vedere dove porta. Per seguirlo, tieni premuto **Ctrl** e fai clic sulla cella, oppure seleziona la cella e usa **Apri collegamento** dal menu di scelta rapida. Un semplice clic seleziona soltanto la cella.

JeffCrunch non apre mai nulla da solo. Una pagina web o un indirizzo e-mail si apre nel tuo programma abituale quando segui il collegamento. Un collegamento a un file, o a un altro tipo di indirizzo, ti chiede prima conferma. Un collegamento che eseguirebbe uno script non viene mai aperto.

Un collegamento segue la sua cella quando si inseriscono o eliminano righe o colonne, e quando le celle vengono ordinate, ed è salvato con la cartella di lavoro.

## Statistiche cartella di lavoro

**Statistiche cartella di lavoro** (scheda **Revisione**) conta ciò che la cartella di lavoro contiene: quanti **Fogli** ha, quante **Celle con dati** (celle che contengono un numero, un testo o una formula), quante **Formule**, quanti **Grafici** e quante **Tabelle pivot**. I fogli eliminati non vengono conteggiati. Gli stessi numeri compaiono nella pagina **Informazioni** della scheda File.

## Ortografia

**Controllo ortografia** (scheda **Revisione**, oppure **F7**) controlla le parole nelle celle di testo, una parola errata alla volta. Se hai selezionato più celle, controlla solo quelle; altrimenti controlla l'intero foglio. Non esamina formule, numeri o date.

La finestra mostra la parola che non è nel dizionario, una casella chiamata **Cambia in** con il suggerimento più probabile e un elenco di altri suggerimenti. Scegli un suggerimento o digita il tuo, quindi scegli:

- **Cambia** sostituisce la parola errata con quella nuova in quella cella. Ogni modifica è un passaggio annullabile.
- **Cambia tutto** la sostituisce con la stessa parola in ogni cella controllata, come un unico passaggio annullabile.
- **Ignora** lascia la parola così com'è per questa volta. **Ignora tutto** la lascia così com'è ovunque, finché non chiudi JeffCrunch.
- **Aggiungi** inserisce la parola nel tuo dizionario personale, così non verrà più segnalata. Il tuo dizionario è condiviso con le altre app di JeffOffice.

In **File > Opzioni > Strumenti di correzione** decidi se vengono controllate le parole in maiuscolo e quelle con numeri. JeffCrunch controlla nella lingua in cui è visualizzato, se ha un dizionario per quella lingua, e in inglese americano in caso contrario.

Mentre digiti in una cella, una parola che non è nel dizionario viene sottolineata con una linea rossa ondulata. Fai clic destro sulla parola per scegliere una correzione, oppure **Ignora tutto** o **Aggiungi al dizionario**. Le celle della griglia non vengono sottolineate, e una voce che inizia con il segno di uguale non viene mai controllata. Per vedere, modificare, importare o esportare l'elenco delle parole aggiunte, e per elencare le parole che vengono sempre segnalate come errate, fai clic su **File > Opzioni > Strumenti di correzione > Dizionario personalizzato**. I tuoi elenchi sono gli stessi in ogni app di JeffOffice.

## Imposta la lingua di alcune celle

Il controllo ortografia verifica ogni cella nella sua lingua. Per cambiarla, seleziona le celle e fai clic su **Imposta lingua di correzione** nel gruppo **Lingua** della scheda **Revisione**. Scegli la lingua nell'elenco, quindi fai clic su **OK**. Una cella a cui non hai assegnato una lingua viene controllata nella lingua impostata in **File > Opzioni**. Un foglio di calcolo OpenDocument (.ods) conserva la lingua di ogni cella. Una cartella di lavoro .xlsx non ha un posto per questa informazione, quindi la lingua viene mantenuta solo finché la cartella di lavoro è aperta.

## Escludi alcune celle dal controllo

Alcuni testi non devono essere controllati: un codice, un nome o una riga in una lingua inventata. Seleziona le celle e fai clic su **Non controllare ortografia** nel gruppo **Lingua**. **Controllo ortografia** e le linee rosse ondulate nella cella le saltano. Fai di nuovo clic per controllare di nuovo le celle. Un file .ods conserva questo contrassegno, mentre una cartella di lavoro .xlsx lo conserva solo finché la cartella di lavoro è aperta.

## Traduci il testo

1. Seleziona le celle, oppure alcune parole che stai digitando in una cella. Il testo delle celle selezionate viene inviato una riga alla volta.
2. Fai clic su **Traduci** nel gruppo **Lingua**.
3. Scegli la lingua in cui tradurre.
4. Fai clic su **OK**.

Il browser web si apre quindi su un servizio di traduzione chiamato LibreTranslate, con il tuo testo già inserito. Non viene inviato nulla finché non fai clic su **OK**. Puoi cambiare l'indirizzo del servizio nella casella, per esempio con uno che gestisci tu. Vengono usati solo indirizzi web sicuri (https), e indirizzi http per questo computer. Si possono inviare al massimo 5.000 caratteri.

## Thesaurus

**Thesaurus** (scheda **Revisione**, oppure **Maiusc+F7**) trova parole di significato quasi uguale a quella di una cella. Cerca la prima parola del testo nella cella su cui ti trovi. Digita un'altra parola in **Cerca** e premi **Invio** per cercare quella. Scegli una parola nell'elenco e fai clic su **Sostituisci**, oppure fai doppio clic. La parola nella cella viene sostituita, e una lettera iniziale maiuscola resta maiuscola. **Cerca** cerca la parola che hai scelto, per seguire una catena di significati. L'elenco di parole funziona in inglese.

## Verifica accessibilità

**Verifica accessibilità** (scheda **Revisione**) apre una piccola finestra che elenca ciò che nella cartella di lavoro è difficile da usare per alcuni lettori, o da far leggere a un lettore di schermo (un programma che legge ad alta voce ciò che appare sullo schermo). Ogni riga inizia con **Errore**, **Avviso** o **Suggerimento**. Fai clic su una riga per andare alla cella.

Cerca:

- Un grafico o un'immagine senza **testo alternativo** (le parole che descrivono ciò che mostra). Questo è un errore.
- Una tabella senza riga di intestazione, quindi con colonne senza nome. Questo è un errore.
- **Celle unite**, difficili da percorrere con la tastiera. Questo è un avviso.
- Testo difficile da leggere sullo sfondo di colore su cui si trova, come un testo grigio chiaro su bianco. JeffCrunch misura quanto il testo risalta sul riempimento. Il testo piccolo richiede un rapporto di almeno 4,5 a 1, mentre il testo grande (18 punti, oppure 14 punti in grassetto) richiede 3 a 1. Vengono indicate la prima cella per ogni combinazione di carattere e riempimento. Questo è un avviso.
- Un foglio che conserva ancora il nome assegnato automaticamente, come Foglio1. Questo è un suggerimento.

Dopo aver apportato modifiche, premi **Verifica di nuovo**.
