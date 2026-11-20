# Apri, salva, stampa ed esporta
<!-- keywords: caratteri nel file, caratteri incorporati, carattere mancante, e-book, ebook, epub, lettore e-book, capitoli, sommario, pagina web, html, salva come pagina web, pagina web filtrata, posta elettronica, eml, segnalibri, segnalibri pdf, maiusc+f5, ultima modifica, dove eri rimasto, modelli predefiniti, raccolta, lettera, rapporto, breve, manoscritto, notiziario, apri, salva, salva con nome, docx, doc, dot, odt, opendocument, rtf, file meno recenti, modello, dotx, recenti, fissa, nuovo, chiudi, stampa, pdf, esporta, recupera, recupero automatico, non salvato, arresto anomalo, schermata file, formati di file, salva una copia, sola lettura, apri in sola lettura, apri come copia, nuova finestra, copia di backup, i miei modelli, salva come modello, ott, ripara, apri e ripara, danneggiato, password, cifra, proteggi con password -->

Fai clic su **File** in alto a sinistra per visualizzare la schermata File. Contiene le pagine Nuovo, Apri, Stampa ed Esporta, oltre ai comandi Salva e Salva con nome. Premi **Esc** per tornare al documento.

## Creare un nuovo documento

Premi **Ctrl+N**, oppure fai clic su **File > Nuovo** e poi su **Documento vuoto**. Il nuovo documento si apre in una finestra propria. Se la finestra in cui ti trovi è vuota e non è mai stata salvata, il nuovo documento ne prende il posto.

## Aprire un documento

Premi **Ctrl+A**, oppure fai clic su **File > Apri**. Fai clic su **Sfoglia** per scegliere un file. JeffWrite apre questi tipi di file:

- **.docx** è un documento.
- **.dotx** è un modello. Un modello si apre come nuovo documento creato a partire da esso, così il modello resta invariato.
- **.docm** e **.dotm** sono un documento e un modello che possono contenere **macro**, piccoli programmi allegati al file. JeffWrite non le esegue mai: le conserva così come erano quando salvi in .docm o .dotm. Se salvi in .docx o .odt, le macro vengono omesse e JeffWrite te lo segnala.
- **.txt** è testo semplice, solo le parole. Quando lo apri, JeffWrite lo legge subito se è in ASCII semplice o Unicode. In caso contrario, chiede con quale **codifica** leggerlo e mostra le prime righe, così puoi verificare che le lettere siano corrette. Una **codifica** è il modo in cui un file memorizza le lettere, ad esempio UTF-8 o Windows 1252. Quando salvi in .txt, scegli la codifica e la fine delle righe, e il file conserva solo le parole: niente caratteri, immagini o linee di tabella.
- **.html** e **.htm** sono pagine web. JeffWrite legge le parole, i titoli, il grassetto, il corsivo, la sottolineatura, i colori, gli elenchi e le tabelle. I punti e i numeri degli elenchi vengono importati come segni digitati, mentre immagini e collegamenti vengono omessi. Non scarica nulla da internet e ignora gli script. Una pagina web non viene mai sovrascritta: **Salva** chiede un nuovo nome e salva un file .docx. Per creare una pagina web, vedi **Salva come pagina web** più avanti.
- **.doc** e **.dot** sono i tipi di documento e modello meno recenti. JeffWrite li legge e salva le modifiche come nuovo file .docx.
- **.odt**, **.ott** e **.fodt** sono file OpenDocument. JeffWrite li legge tutti e tre. Salva i file .odt e salva le modifiche fatte a un file .ott o .fodt come nuovo file .odt. Vedi [Apri e salva file OpenDocument](open-odt).
- **.rtf** è Rich Text Format. JeffWrite lo legge e lo salva. Vedi [Apri file RTF](open-rtf).

La pagina **Apri** elenca anche i documenti aperti o salvati di recente. Fai clic su uno per aprirlo. Fai clic destro su uno per fissarlo in cima all'elenco o per rimuoverlo dall'elenco.

Se un file contiene parti che JeffWrite non sa ancora modificare, come la struttura di una tabella o un'intestazione, le conserva. Vedi [Benvenuto in JeffWrite](start). Se una parte del file non può essere letta affatto, JeffWrite ti dice che cosa ha omesso. In quel caso non salva sopra il file originale: **Salva** chiede un nuovo nome, così l'originale conserva ciò che la copia ha perso.

## Salvare un documento

Premi **Ctrl+S** per salvare. La prima volta, JeffWrite chiede un nome. **Salva con nome** (F12) salva una copia con un nuovo nome. Scegli **Modello (*.dotx)** per salvare un modello, **Documento con macro (*.docm)** per conservare le macro, **Testo normale (*.txt)** per le sole parole, oppure **Rich Text Format (*.rtf)** per un file leggibile da altri programmi. Un file proveniente da un tipo meno recente viene sempre salvato con un nuovo nome, così il file originale non viene mai sovrascritto.

## Salvare una copia

Fai clic su **File > Salva una copia**. Scegli un nome. JeffWrite scrive il documento in quel file e tu continui a lavorare sul documento che avevi aperto. Il suo nome e le modifiche non salvate restano come erano. È utile quando vuoi inviare una versione a qualcuno e continuare a scrivere.

## Conservare una copia di backup

Fai clic su **File > Opzioni** e, nella pagina **Generale**, attiva **Crea sempre una copia di backup**. Ogni volta che salvi, JeffWrite copia prima il file precedente in uno chiamato "Backup di" seguito dal nome del file, nella stessa cartella. Se la copia non può essere creata, JeffWrite chiede se salvare comunque senza di essa. La risposta predefinita è No, a meno che tu non risponda Sì, e in quel caso il file resta com'era. Così puoi tornare all'ultima versione salvata.

## Salvare come modello

Un modello è un documento che usi come punto di partenza. Fai clic su **File > Salva come modello** e digita un nome. Il modello viene inserito in **I miei modelli**, e il documento che stai scrivendo non viene modificato. In **Salva con nome** puoi anche scegliere **Modello (*.dotx)** oppure **Modello di testo OpenDocument (*.ott)** per salvare un file modello dove preferisci.

Per creare un nuovo documento a partire dal tuo modello, fai clic su **File > Nuovo**. I tuoi modelli sono elencati in **I miei modelli**. Fai clic su uno di essi. Si apre un nuovo documento con il testo e gli stili del modello. Per rimuovere un modello dall'elenco, fai clic destro su di esso e scegli **Rimuovi da I miei modelli**.

## Altri modi per aprire un file

Nella pagina **Apri**, fai clic su **Altri modi per aprire**. Puoi anche fare clic destro su un nome nell'elenco dei recenti. Ogni modo chiede un file oppure usa quello su cui hai fatto clic:

- **Apri in sola lettura** apre il file solo per la lettura. Puoi leggere, scorrere, cercare e copiare, ma non modificare nulla. La barra del titolo riporta "Sola lettura". Se premi **Salva**, JeffWrite chiede un nuovo nome, così il file aperto non viene modificato.
- **Apri come copia** apre un nuovo documento con il testo del file. Non ha un file proprio, quindi **Salva** chiede dove metterlo. Il file aperto non viene mai modificato.
- **Apri in nuova finestra** apre il file in una finestra propria, anche quando la finestra in cui ti trovi è vuota.
- **Apri e ripara** serve per un file .docx o .doc danneggiato, ad esempio uno che è stato troncato durante la copia. Vedi sotto.

## Apri e ripara

A volte un file .docx o .doc è danneggiato. Una copia si è interrotta a metà, oppure un disco ha commesso errori. Un file .docx è un insieme di parti più piccole, come il testo, gli stili e le immagini. **Apri e ripara** legge ogni parte che riesce a leggere. Una parte interrotta a metà viene conservata fino al punto in cui si è fermata. Una parte che non può essere letta viene omessa. Se l'elenco delle parti o i collegamenti tra di esse sono andati persi, JeffWrite li ricostruisce.

Un file .doc meno recente è un unico contenitore con alcuni flussi di dati al suo interno. Un flusso contiene il testo, un altro la tabella che indica dove si trova ogni porzione di testo e come appare. Se il contenitore è troncato o le sue liste sono danneggiate, **Apri e ripara** individua questi flussi in base al loro contenuto e legge quanto più possibile. Se la tabella è andata persa, il testo viene conservato come paragrafi semplici, senza stili, elenchi, tabelle o immagini. Un file .doc protetto da password non può essere riparato.

Il documento riparato si apre come copia con un nome come "Lettera riparata". Un messaggio ti indica che cosa è stato conservato fino a un'interruzione e che cosa è stato omesso. Il file danneggiato non viene modificato. Salva la copia riparata con un nome proprio e verificala prima di fidartene.

## Recuperare il lavoro non salvato

JeffWrite salva ogni pochi minuti una copia di riserva del lavoro non salvato, come file nella propria cartella. Se il programma si chiude in modo imprevisto, alla riapertura successiva le propone. Fai clic su **File > Apri > Recupera lavoro non salvato** per vederle in un secondo momento. Puoi scegliere quali aprire e salvare ciascuna dove preferisci.

## Iniziare da un modello

**File > Nuovo** mostra **Documento vuoto** e una serie di modelli forniti con JeffWrite, raggruppati per tipo: lettera, rapporto, breve, manoscritto e notiziario. Ogni immagine è la prima pagina del documento che otterrai. Un **modello** è un documento già predisposto, con i suoi stili, margini e testi di esempio. Fai clic su uno e sostituisci i testi di esempio con i tuoi. I modelli che hai salvato tu sono elencati in **I miei modelli**.

## Riprendere da dove eri rimasto

JeffWrite ricorda dove si trovava il cursore quando hai salvato o chiuso un file. La volta successiva che apri il file, il cursore è lì. Premi **Maiusc+F5** per saltare ai punti che hai modificato per ultimi. Ogni pressione porta al punto precedente: prima gli ultimi tre punti, poi di nuovo il primo.

## Salvare come .odt

Un file .odt non può contenere tutto ciò che può contenere un file .docx. Quando salvi come .odt, JeffWrite elenca prima ciò che andrebbe perso e ti permette di salvare invece un file .docx. Vedi [Informazioni](info).

## Stampare

Premi **Ctrl+P**, oppure fai clic su **File > Stampa**. Scegli una stampante, le copie e le pagine, controlla ogni pagina nell'anteprima e fai clic su **Stampa**. Vedi [Stampa](print).

## Creare un PDF

Fai clic su **File > Esporta > Esporta in PDF**. Il PDF riporta il titolo, l'autore, l'oggetto e le parole chiave del documento presi da **File > Informazioni** (con **Includi proprietà documento** attivo), la data di creazione e JeffWrite come programma che lo ha creato. L'autore e un titolo ricavato dal nome del file restano esclusi quando le informazioni personali vengono rimosse. Ciascuno di questi campi viene conservato integralmente fino a 4.000 caratteri. Seleziona **Apri il PDF dopo il salvataggio** per far aprire il file con il programma PDF del computer al momento del salvataggio. L'opzione è disattivata finché non la selezioni. Se il computer non riesce ad aprire il file, la barra di stato lo segnala (il file viene comunque salvato). Scegli quali pagine includere: tutte, la pagina corrente oppure un intervallo come 1-3, 5 (anche p2s3 e s3 funzionano). Poi scegli come va creato:

- **Ottimizza per** ha **Standard**, che mantiene le immagini così come sono, e **Dimensioni minime**, che riduce ciascuna immagine a non più di 150 pixel per pollice, così il file è più piccolo. Usa Dimensioni minime per un PDF da pubblicare su una pagina web o da inviare per posta elettronica.
- **Includi proprietà documento** inserisce nel PDF il titolo, l'autore, l'oggetto e le parole chiave del documento.
- **Crea segnalibri dai titoli** trasforma ogni titolo in un **segnalibro**, una voce dell'elenco che il lettore PDF mostra accanto alle pagine. Fai clic su un segnalibro per andare alla sua pagina.
- **Tag della struttura del documento per l'accessibilità** etichetta titoli, paragrafi, elenchi, tabelle (con le righe di intestazione) e immagini (con il testo alternativo), e indica la lingua, così che un lettore di schermo legga il PDF nell'ordine corretto.

Infine scegli un nome. Il PDF appare uguale su ogni computer, perché i caratteri sono incorporati al suo interno. Anche il PDF che **Stampa** crea quando scegli un file PDF contiene i segnalibri e i tag. Quando sistema più pagine su un solo foglio, ogni segnalibro e ogni collegamento porta al foglio che contiene la sua pagina, e le pagine su un foglio vengono lette nell'ordine in cui sono disposte: da sinistra a destra, poi dall'alto in basso.

## Salvare come pagina web

Fai clic su **File > Esporta** e scegli uno dei due tipi di pagina web. Una **pagina web** è un file che puoi aprire in qualsiasi browser.

- **Salva come pagina web** crea un unico file .html che contiene le parole, il loro aspetto e le immagini. Puoi inviare quel file da solo. L'aspetto conservato comprende carattere, dimensione, colore, evidenziazione, grassetto, corsivo, sottolineatura, allineamento e spazio intorno ai paragrafi.
- **Salva come pagina web filtrata** crea un piccolo file .html con la sola struttura: titoli, grassetto e corsivo, elenchi, tabelle, collegamenti e immagini. Non contiene caratteri, dimensioni o colori, così potrai assegnargli un aspetto tuo in seguito.

Entrambi conservano titoli, elenchi puntati e numerati, tabelle con celle unite, collegamenti e immagini. Il testo nascosto, le parole eliminate con una modifica registrata, i commenti, le intestazioni, i piè di pagina e le note vengono omessi. Un collegamento che avvierebbe un programma o eseguirebbe uno script viene salvato come semplice testo.

## Salvare come e-book

Un **e-book** è un libro che leggi su un lettore di e-book, un telefono o un tablet. Il file è un **EPUB**. Il testo si adatta a qualsiasi schermo, così il lettore può ingrandire o rimpicciolire i caratteri.

1. Fai clic su **File > Esporta > Salva come e-book (EPUB)**.
2. Controlla **Titolo** e **Autore**. Inizialmente corrispondono a titolo e autore nelle proprietà del documento (**File > Informazioni**). Puoi modificarli solo per questo libro.
3. Attiva **Usa la prima immagine come copertina** se vuoi che la prima immagine del documento diventi la copertina. Senza di essa, il libro non ha copertina.
4. Fai clic su **OK**, poi scegli dove salvare il file.

Come viene creato il libro:

- Ogni **Titolo 1** inizia un nuovo capitolo. Il testo che precede il primo Titolo 1 costituisce un capitolo a sé.
- Il **sommario** elenca ogni Titolo 1, Titolo 2 e Titolo 3, e il lettore può saltare a ciascuno di essi. I titoli più piccoli sono nel libro ma non nel sommario.
- Immagini e tabelle restano dove le hai collocate. Il testo alternativo di un'immagine la accompagna, così un lettore di schermo può dire che cosa mostra.
- Le note a piè di pagina e le note di chiusura sono disposte alla fine del capitolo in cui si trovano. Ogni nota ha un collegamento di ritorno al punto del testo, e ogni numero nel testo è un collegamento alla propria nota.

Il libro è soggetto a limiti, così che un file danneggiato non possa generarne uno enorme: fino a 500 capitoli, 4.000 righe nel sommario, 1.000 immagini (nessuna oltre 32 MB) e 20.000 note. Ciò che supera un limite resta nell'ultimo capitolo oppure diventa semplice testo, e le tue parole non vengono mai perse: un'immagine oltre il limite diventa il suo testo alternativo, e una nota oltre il limite non ha formattazione. Un'immagine che un lettore non è in grado di mostrare (un metafile, una bitmap o un TIFF) viene convertita in PNG. Esportare di nuovo lo stesso documento salvato produce lo stesso libro, così il lettore ritrova il suo punto.

## Proteggere un PDF con una password

Nella finestra **Esporta in PDF**, fai clic su **Cifra con una password…**. Digita la password due volte. Chiunque apra il PDF dovrà poi digitarla. La password protegge davvero il file: le pagine sono cifrate con una crittografia robusta (AES-256), e nessuno può leggerle senza di essa.

Puoi anche selezionare **Limita stampa, copia e modifica**, indicare una seconda password e scegliere che cosa è consentito. Queste limitazioni sono solo una richiesta. La maggior parte dei lettori PDF le rispetta, ma non impedisce a chi voglia aggirarle di farlo, quindi non contarci per mantenere privato un file.

Una password dimenticata non può essere recuperata, quindi conservala in un luogo sicuro. La password serve per quel solo PDF e non viene mai salvata. Quando è impostata una password, la finestra disattiva **PDF/A**, perché un file PDF/A non può averne una.

## Creare un PDF/A

Nella finestra **Esporta in PDF**, seleziona **PDF/A, per la conservazione a lungo termine**. Un file PDF/A (PDF/A-1b) contiene tutti i caratteri che usa, indica i suoi colori e segue uno standard pensato per conservare i file per molti anni. Può essere etichettato per i lettori di schermo, ma non può avere una password.

## Creare un messaggio di posta

Fai clic su **File > Esporta > Crea un messaggio** per creare un file di messaggio (.eml). Nulla viene inviato. Scegli che cosa inserire nel messaggio: il documento, il documento come PDF, oppure le pagine visualizzate come immagini nel messaggio (le prime 30 pagine). Apri il file nel tuo programma di posta, aggiungi il destinatario e invialo. Per un PDF, puoi fare clic su **Cifra con una password…** nella finestra; comunica la password al destinatario in un altro modo, non nel messaggio.

## Caratteri nel file

Un file .docx può contenere i caratteri con cui è composto il testo, così il documento appare come il suo autore l'ha creato, anche su un computer che non ha quei caratteri. JeffWrite li legge quando apre il file. Vengono usati solo per quel documento, mentre è aperto. Non vengono mai installati sul computer, nessun altro documento o programma li vede, e scompaiono quando chiudi il documento. Un carattere contenuto nel file non viene conteggiato come carattere mancante.

JeffWrite non usa un carattere la cui licenza del produttore non ne consente l'incorporamento, un carattere danneggiato, né alcun carattere oltre i primi 64 di un file. Quando succede, il messaggio che vedi aprendo il file lo segnala, e il testo viene mostrato con un carattere simile. Il file conserva i suoi caratteri così come erano al momento del salvataggio.

Per inserire nei file salvati i caratteri usati dal testo, fai clic su **File > Opzioni**, apri la pagina **Salva** e attiva **Salva i caratteri nel file**. L'opzione è disattivata all'inizio. Quando salva un file .docx, JeffWrite inserisce allora i caratteri forniti con il programma e usati dal testo, in ogni stile usato dal documento. Non inserisce mai un carattere la cui licenza non lo consenta, e quando ne omette uno, il messaggio dopo il salvataggio ne indica il nome. Il file diventa più grande, e in un solo file possono entrare al massimo 64 caratteri e 256 MB.

## Vedi anche

- [Benvenuto in JeffWrite](start)
- [Stampa](print)
- [Usare JeffWrite con la tastiera](keyboard)
