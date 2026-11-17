# Apri, salva, stampa ed esporta
<!-- keywords: e-book, ebook, epub, lettore di e-book, capitoli, sommario, pagina web, html, salva come pagina web, pagina web filtrata, posta elettronica, email, eml, segnalibri, segnalibri pdf, maiusc+f5, ultima modifica, dove eri rimasto, modelli predefiniti, raccolta, lettera, rapporto, breve, manoscritto, notiziario, apri, salva, salva con nome, docx, doc, dot, odt, opendocument, rtf, file meno recenti, modello, dotx, recenti, fissa, nuovo, chiudi, stampa, pdf, esporta, recupera, recupero automatico, non salvato, arresto anomalo, schermata file, formati di file, salva una copia, sola lettura, apri come copia, nuova finestra, copia di backup, i miei modelli, salva come modello, ott, ripara, apri e ripara, danneggiato, password, cifra, proteggi con password -->

Fai clic su **File** in alto a sinistra per vedere la schermata File. Contiene le pagine Nuovo, Apri, Stampa ed Esporta, e i comandi Salva e Salva con nome. Premi **Esc** per tornare al documento.

## Creare un nuovo documento

Premi **CTRL+N**, oppure fai clic su **File > Nuovo** e poi su **Documento vuoto**. Il nuovo documento si apre in una finestra propria. Se la finestra in cui ti trovi è vuota e non è mai stata salvata, il nuovo documento ne prende il posto.

## Aprire un documento

Premi **CTRL+A**, oppure fai clic su **File > Apri**. Fai clic su **Sfoglia** per scegliere un file. JeffWrite apre questi tipi di file:

- **.docx** è un documento.
- **.dotx** è un modello. Un modello si apre come nuovo documento creato a partire da esso, quindi il modello resta invariato.
- **.docm** e **.dotm** sono un documento e un modello che possono contenere **macro**, piccoli programmi che accompagnano un file. JeffWrite non li esegue mai: li conserva come erano quando salvi come .docm o .dotm. Se salvi come .docx o .odt, le macro vengono omesse e JeffWrite te lo segnala.
- **.txt** è testo normale, solo le parole. Quando lo apri, JeffWrite lo legge subito se è ASCII semplice o Unicode. In caso contrario, JeffWrite chiede con quale **codifica** leggerlo e mostra le prime righe, così puoi verificare che le lettere siano corrette. La **codifica** è il modo in cui un file memorizza le lettere, ad esempio UTF-8 o Windows 1252. Quando salvi come .txt, scegli la codifica e i fine riga, e il file conserva solo le parole: niente caratteri, immagini o linee di tabella.
- **.html** e **.htm** sono pagine web. JeffWrite legge le parole, i titoli, il grassetto, il corsivo, la sottolineatura, i colori, gli elenchi e le tabelle. I punti e i numeri degli elenchi vengono importati come segni digitati, mentre immagini e collegamenti vengono omessi. Non scarica mai nulla da internet e ignora gli script. Una pagina web non viene mai sovrascritta: **Salva** chiede un nuovo nome e salva un file .docx. Per creare una pagina web, vedi **Salva come pagina web** più avanti.
- **.doc** e **.dot** sono i tipi di documento e modello più vecchi. JeffWrite li legge e salva ciò che fai con essi come nuovo file .docx.
- **.odt**, **.ott** e **.fodt** sono file OpenDocument. JeffWrite legge tutti e tre. Salva i file .odt e salva ciò che fai con un file .ott o .fodt come nuovo file .odt. Vedi [Aprire e salvare file OpenDocument](open-odt).
- **.rtf** è Rich Text Format. JeffWrite lo legge e lo salva. Vedi [Aprire file RTF](open-rtf).

La pagina **Apri** elenca anche i documenti aperti o salvati di recente. Fai clic su uno per aprirlo. Fai clic con il pulsante destro su un documento per fissarlo in cima all'elenco o per rimuoverlo dall'elenco.

Se un file contiene parti che JeffWrite non può ancora modificare, come il layout di una tabella o un'intestazione, le conserva. Vedi [Benvenuti in JeffWrite](start). Se una parte del file non può essere letta affatto, JeffWrite ti dice cosa ha omesso. Non sovrascrive quindi quel file: **Salva** chiede un nuovo nome, così l'originale conserva ciò che la copia ha perso.

## Salvare un documento

Premi **CTRL+S** per salvare. La prima volta, JeffWrite chiede un nome. **Salva con nome** (F12) salva una copia con un nuovo nome. Scegli **Modello (*.dotx)** per salvare un modello, **Documento con macro (*.docm)** per conservare le macro, **Testo normale (*.txt)** per le sole parole, oppure **Rich Text Format (*.rtf)** per un file leggibile da altri programmi. Un file proveniente da un tipo di file meno recente viene sempre salvato con un nuovo nome, così il file vecchio non viene mai sovrascritto.

## Salvare una copia

Fai clic su **File > Salva una copia**. Scegli un nome. JeffWrite scrive il documento in quel file e tu continui a lavorare nel documento che avevi aperto. Il suo nome e le sue modifiche non salvate restano come erano. È utile quando vuoi inviare una versione a qualcuno e continuare a scrivere.

## Conservare una copia di backup

Fai clic su **File > Opzioni** e, nella pagina **Generale**, attiva **Crea sempre una copia di backup**. Ogni volta che salvi, JeffWrite copia prima il file precedente in uno chiamato "Backup di" seguito dal suo nome, nella stessa cartella. Se la copia non può essere creata, JeffWrite chiede se salvare senza di essa. La risposta predefinita è No, a meno che tu non dica Sì, e il file resta com'era. Così puoi tornare all'ultima versione salvata.

## Salvare come modello

Un modello è un documento che usi come punto di partenza. Fai clic su **File > Salva come modello** e digita un nome. Il modello finisce in **I miei modelli**, e il documento su cui stai lavorando non viene modificato. In **Salva con nome** puoi anche scegliere **Modello (*.dotx)** oppure **Modello di testo OpenDocument (*.ott)** per salvare un file modello dove preferisci.

Per iniziare un nuovo documento dal tuo modello, fai clic su **File > Nuovo**. I tuoi modelli sono elencati in **I miei modelli**. Fai clic su uno di essi. Si apre un nuovo documento con il testo e gli stili del modello. Per rimuovere un modello dall'elenco, fai clic con il pulsante destro su di esso e scegli **Rimuovi da I miei modelli**.

## Altri modi per aprire un file

Nella pagina **Apri**, fai clic su **Altri modi per aprire**. Puoi anche fare clic con il pulsante destro su un nome nell'elenco dei recenti. Ogni modo chiede un file oppure usa quello su cui hai fatto clic:

- **Apri in sola lettura** apre il file solo per la lettura. Puoi guardare, scorrere, cercare e copiare, ma non puoi modificare nulla. La barra del titolo mostra "Sola lettura". Se premi **Salva**, JeffWrite chiede un nuovo nome, così il file aperto non viene modificato.
- **Apri come copia** apre un nuovo documento con il testo del file. Non ha un file proprio, quindi **Salva** chiede dove metterlo. Il file aperto non viene mai modificato.
- **Apri in una nuova finestra** apre il file in una finestra propria, anche quando la finestra in cui ti trovi è vuota.
- **Apri e ripara** serve per un file .docx o .doc danneggiato, ad esempio uno che è stato troncato durante la copia. Vedi sotto.

## Apri e ripara

A volte un file .docx o .doc è danneggiato. Una copia si è interrotta a metà, oppure un disco ha commesso errori. Un file .docx è un insieme di parti più piccole, come il testo, gli stili e le immagini. **Apri e ripara** legge ogni parte che riesce a leggere. Una parte interrotta a metà viene conservata fino al punto in cui si è interrotta. Una parte che non può essere letta viene omessa. Se l'elenco delle parti o i collegamenti tra di esse sono andati persi, JeffWrite li ricostruisce.

Un file .doc meno recente è un unico grande contenitore con alcuni flussi di dati al suo interno. Uno dei flussi contiene il testo, un altro la tabella che indica dove si trova ogni porzione di testo e come appare. Se il contenitore è troncato o i suoi elenchi sono danneggiati, **Apri e ripara** individua quei flussi in base al loro contenuto e legge quanto più possibile. Se la tabella è perduta, il testo viene conservato come paragrafi semplici, senza stili, elenchi, tabelle o immagini. Un file .doc protetto da password non può essere riparato.

Il documento riparato si apre come copia con un nome del tipo "Lettera riparata". Un messaggio ti dice cosa è stato conservato fino a un'interruzione e cosa è stato omesso. Il file danneggiato non viene modificato. Salva la copia riparata con un nome proprio e verificala prima di fidarti di essa.

## Recuperare il lavoro non salvato

JeffWrite salva una copia di riserva del lavoro non salvato ogni pochi minuti, come file nella propria cartella. Se il programma si chiude in modo imprevisto, alla successiva apertura propone quelle copie. Fai clic su **File > Apri > Ripristina lavoro non salvato** per vederle in seguito. Puoi scegliere quali aprire e salvare ciascuna dove preferisci.

## Iniziare da un modello

**File > Nuovo** mostra il **Documento vuoto** e una serie di modelli forniti con JeffWrite, raggruppati per tipo: una lettera, un rapporto, un breve, un manoscritto e un notiziario. Ogni immagine è la prima pagina del documento che otterrai. Un **modello** è un documento già predisposto, con i suoi stili, margini e testi di esempio. Fai clic su uno e sostituisci i testi di esempio con i tuoi. I modelli che hai salvato tu sono elencati in **I miei modelli**.

## Riprendere da dove eri rimasto

JeffWrite ricorda dove si trovava il cursore quando hai salvato o chiuso un file. La volta successiva che apri il file, il cursore è lì. Premi **Maiusc+F5** per andare ai punti modificati per ultimi. Ogni pressione porta al punto precedente: gli ultimi tre punti, poi di nuovo il primo.

## Salvare come .odt

Un file .odt non può contenere tutto ciò che può contenere un file .docx. Quando salvi come .odt, JeffWrite elenca prima ciò che andrebbe perso e ti permette di salvare invece un file .docx. Vedi [Informazioni](info).

## Stampare

Premi **CTRL+P**, oppure fai clic su **File > Stampa**. Scegli una stampante, il numero di copie e le pagine, controlla ogni pagina nell'anteprima e fai clic su **Stampa**. Vedi [Stampa](print).

## Creare un PDF

Fai clic su **File > Esporta > Esporta in PDF**. Scegli quali pagine includere: tutte, la pagina corrente oppure un intervallo come 1-3, 5 (anche p2s3 e s3 funzionano qui). Poi scegli come crearlo:

- **Ottimizza per** ha **Standard**, che mantiene le immagini come sono, e **Dimensione minima**, che riduce ogni immagine a non più di 150 pixel per pollice, così il file è più piccolo. Usa Dimensione minima per un PDF da pubblicare in una pagina web o da inviare per posta elettronica.
- **Includi proprietà del documento** inserisce nel PDF il titolo, l'autore, l'oggetto e le parole chiave del documento.
- **Crea segnalibri dai titoli** trasforma ogni titolo in un **segnalibro**, una voce dell'elenco che il lettore di PDF mostra accanto alle pagine. Fai clic su un segnalibro per passare alla sua pagina.
- **Tag della struttura del documento per l'accessibilità** etichetta i titoli, i paragrafi, gli elenchi, le tabelle (con le relative righe di intestazione) e le immagini (con il loro testo alternativo), e indica la lingua, così uno screen reader legge il PDF nell'ordine corretto.

Poi scegli un nome. Il PDF appare uguale su ogni computer, perché i caratteri sono incorporati al suo interno. Il PDF che **Stampa** crea quando scegli un file PDF contiene anche i segnalibri e i tag. Quando colloca più pagine su un solo foglio, ogni segnalibro e ogni collegamento porta al foglio che contiene la loro pagina, e le pagine su un foglio vengono lette nell'ordine in cui sono disposte: da sinistra a destra, poi dall'alto in basso.

## Salvare come pagina web

Fai clic su **File > Esporta** e scegli uno dei due tipi di pagina web. Una **pagina web** è un file che puoi aprire in qualsiasi browser.

- **Salva come pagina web** crea un unico file .html che contiene le tue parole, il loro aspetto e le tue immagini. Puoi inviare questo solo file. L'aspetto conservato comprende il carattere, la dimensione, il colore, l'evidenziazione, grassetto, corsivo, sottolineatura, allineamento e lo spazio intorno ai paragrafi.
- **Salva come pagina web filtrata** crea un piccolo file .html con la sola struttura: titoli, grassetto e corsivo, elenchi, tabelle, collegamenti e immagini. Non ha caratteri, dimensioni o colori, così puoi dargli in seguito un aspetto tuo.

Entrambi conservano titoli, elenchi puntati e numerati, tabelle con celle unite, collegamenti e immagini. Il testo nascosto, le parole eliminate da una modifica registrata, i commenti, le intestazioni, i piè di pagina e le note vengono omessi. Un collegamento che avvierebbe un programma o eseguirebbe uno script viene salvato come semplice testo.

## Salvare come e-book

Un **e-book** è un libro che si legge su un lettore di e-book, un telefono o un tablet. Il file è un **EPUB**. Le parole si adattano a qualsiasi schermo, così il lettore può ingrandire o rimpicciolire le lettere.

1. Fai clic su **File > Esporta > Salva come e-book (EPUB)**.
2. Controlla il **Titolo** e l'**Autore**. Inizialmente corrispondono al titolo e all'autore nelle proprietà del documento (**File > Informazioni**). Puoi modificarli solo per questo libro.
3. Attiva **Usa la prima immagine come copertina** se vuoi che la prima immagine del documento sia la copertina. Senza questa opzione, il libro non ha copertina.
4. Fai clic su **OK**, quindi scegli dove salvare il file.

Come viene creato il libro:

- Ogni **Titolo 1** inizia un nuovo capitolo. Le parole prima del primo Titolo 1 formano un capitolo a sé.
- L'elenco **sommario** mostra ogni Titolo 1, Titolo 2 e Titolo 3, e il lettore può andare a ciascuno. I titoli più piccoli sono nel libro ma non nel sommario.
- Immagini e tabelle restano dove le hai collocate. Il testo alternativo di un'immagine la accompagna, così uno screen reader può dire cosa mostra.
- Le note a piè di pagina e le note di chiusura sono riportate alla fine del capitolo in cui si trovano. Ogni nota ha un collegamento di ritorno al punto del testo, e ogni numero nel testo è un collegamento alla sua nota.

Un libro è soggetto a limiti, così che un file danneggiato non possa generarne uno enorme: fino a 500 capitoli, 4.000 righe nel sommario, 1.000 immagini (nessuna oltre 32 MB) e 20.000 note. Ciò che supera un limite resta nell'ultimo capitolo oppure diventa semplice testo, e le tue parole non vengono mai eliminate: un'immagine oltre il limite diventa il suo testo alternativo, e una nota oltre il limite non ha formattazione. Un'immagine che un lettore non può mostrare (un metafile, una bitmap o un TIFF) viene convertita in PNG. Esportare di nuovo lo stesso documento salvato produce lo stesso libro, così un lettore ritrova il punto in cui era rimasto.

## Proteggere un PDF con una password

Nella casella **Esporta in PDF**, fai clic su **Cifra con una password…**. Digita la password due volte. Chiunque apra il PDF dovrà digitarla. La password protegge davvero il file: le pagine sono cifrate con una cifratura robusta (AES-256), e nessuno può leggerle senza di essa.

Puoi anche selezionare **Limita stampa, copia e modifica**, indicare una seconda password e scegliere cosa è consentito. Tali limitazioni sono solo una richiesta. La maggior parte dei lettori PDF le rispetta, ma non impediscono a chi voglia aggirarle di farlo, quindi non contare su di esse per mantenere privato un file.

Una password dimenticata non può essere recuperata, quindi conservala in un luogo sicuro. La password viene usata solo per quel PDF e non viene mai salvata. La casella disattiva **PDF/A** mentre è impostata una password, perché un file PDF/A non può averne una.

## Creare un PDF/A

Nella casella **Esporta in PDF**, seleziona **PDF/A, per la conservazione a lungo termine**. Un file PDF/A (PDF/A-1b) contiene tutti i caratteri che usa, dichiara i suoi colori e segue uno standard pensato per conservare i file per molti anni. Può essere etichettato per gli screen reader, e non può avere una password.

## Creare un messaggio di posta elettronica

Fai clic su **File > Esporta > Crea un messaggio di posta** per creare un file di messaggio (.eml). Non viene inviato nulla. Scegli cosa mettere nel messaggio: il documento, il documento come PDF, oppure le pagine visualizzate come immagini nel messaggio (le prime 30 pagine). Apri il file nel tuo programma di posta, aggiungi il destinatario e invialo. Per un PDF, puoi fare clic su **Cifra con una password…** nella casella; comunica la password alla persona in un altro modo, non nel messaggio.

## Vedi anche

- [Benvenuti in JeffWrite](start)
- [Stampa](print)
- [Usare JeffWrite con la tastiera](keyboard)
