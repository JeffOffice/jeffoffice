# Apri, salva, stampa ed esporta
<!-- keywords: caratteri nel file, caratteri incorporati, carattere mancante, e-book, ebook, epub, lettore e-book, capitoli, sommario, pagina web, html, salva come pagina web, pagina web filtrata, email, eml, segnalibri, segnalibri pdf, maiusc+f5, ultima modifica, dove eri rimasto, modelli predefiniti, raccolta, lettera, rapporto, relazione, manoscritto, notiziario, apri, salva, salva con nome, docx, doc, dot, odt, opendocument, rtf, file meno recenti, modello, dotx, recenti, fissa, nuovo, chiudi, stampa, pdf, esporta, recupera, recupero automatico, non salvato, arresto anomalo, schermata file, formati di file, salva una copia, sola lettura, apri in sola lettura, apri come copia, nuova finestra, copia di backup, i miei modelli, salva come modello, ott, ripara, apri e ripara, danneggiato, password, cifra, proteggi con password -->

Fai clic su **File** in alto a sinistra per vedere la schermata File. Contiene le pagine Nuovo, Apri, Stampa ed Esporta, oltre ai comandi Salva e Salva con nome. Premi **Esc** per tornare al documento.

## Creare un nuovo documento

Premi **Ctrl+N**, oppure fai clic su **File > Nuovo** e poi su **Documento vuoto**. Il nuovo documento si apre in una finestra propria. Se la finestra in cui ti trovi è vuota e non è mai stata salvata, il nuovo documento prende il suo posto.

## Aprire un documento

Premi **Ctrl+O**, oppure fai clic su **File > Apri**. Fai clic su **Sfoglia** per scegliere un file. JeffWrite apre questi tipi di file:

- **.docx** è un documento.
- **.dotx** è un modello. Un modello si apre come nuovo documento creato a partire da esso, così il modello resta invariato.
- **.docm** e **.dotm** sono un documento e un modello che possono contenere **macro**, piccoli programmi che accompagnano un file. JeffWrite non le esegue mai: le conserva così come erano quando salvi come .docm o .dotm. Se salvi come .docx o .odt, le macro vengono omesse e JeffWrite te lo segnala.
- **.txt** è testo normale, solo le parole. All'apertura, JeffWrite lo legge subito se è in ASCII semplice o Unicode. Se non lo è, JeffWrite chiede con quale **codifica** leggerlo e mostra le prime righe, così puoi verificare che le lettere siano corrette. Una **codifica** è il modo in cui un file memorizza le lettere, come UTF-8 o Windows 1252. Quando salvi come .txt, scegli la codifica e i caratteri di fine riga, e il file conserva solo le parole: niente caratteri, immagini o linee di tabella.
- **.html** e **.htm** sono pagine web. JeffWrite legge le parole, i titoli, il grassetto, il corsivo, la sottolineatura, i colori, gli elenchi e le tabelle. I punti e i numeri degli elenchi vengono importati come segni digitati, mentre immagini e collegamenti vengono omessi. Non scarica mai nulla da internet e ignora gli script. Una pagina web non viene mai sovrascritta: **Salva** chiede un nuovo nome e salva un file .docx. Per creare una pagina web, vedi **Salvare come pagina web** più avanti.
- **.doc** e **.dot** sono i tipi di documento e modello più vecchi. JeffWrite li legge e salva ciò che fai con essi come nuovo file .docx.
- **.odt**, **.ott** e **.fodt** sono file OpenDocument. JeffWrite li legge tutti e tre. Salva i file .odt e salva ciò che fai con un file .ott o .fodt come nuovo file .odt. Vedi [Aprire e salvare file OpenDocument](open-odt).
- **.rtf** è Rich Text Format. JeffWrite lo legge e lo salva. Vedi [Aprire file RTF](open-rtf).

La pagina **Apri** elenca anche i documenti aperti o salvati di recente. Fai clic su uno per aprirlo. Fai clic con il pulsante destro su uno per fissarlo in cima all'elenco oppure per rimuoverlo dall'elenco.

Se un file contiene parti che JeffWrite non può ancora modificare, come il layout di una tabella o un'intestazione, le conserva. Vedi [Benvenuti in JeffWrite](start). Se una parte del file non può essere letta affatto, JeffWrite ti dice che cosa ha omesso. In quel caso non salva sopra il file: **Salva** chiede un nuovo nome, così l'originale conserva ciò che la copia ha perso.

## Salvare un documento

Premi **Ctrl+S** per salvare. La prima volta, JeffWrite chiede un nome. **Salva con nome** (F12) salva una copia con un nuovo nome. Scegli **Modello (*.dotx)** per salvare un modello, **Documento con macro (*.docm)** per conservare le macro, **Testo normale (*.txt)** per le sole parole, oppure **Rich Text Format (*.rtf)** per un file leggibile da altri programmi. Un file proveniente da un tipo più vecchio viene sempre salvato con un nuovo nome, così il file originale non viene mai sovrascritto.

## Salvare una copia

Fai clic su **File > Salva una copia**. Scegli un nome. JeffWrite scrive il documento in quel file e tu continui a lavorare nel documento che avevi aperto. Il nome e le modifiche non salvate restano invariati. È utile quando vuoi inviare una versione a qualcuno e continuare a scrivere.

## Conservare una copia di backup

Fai clic su **File > Opzioni** e, nella pagina **Generale**, attiva **Crea sempre una copia di backup**. Ogni volta che salvi, JeffWrite copia prima il file precedente in uno chiamato "Backup di" seguito dal suo nome, nella stessa cartella. Se non riesce a creare la copia, JeffWrite chiede se salvare comunque senza di essa. La risposta predefinita è No, a meno che tu non risponda Sì, e il file resta com'era. Così puoi tornare all'ultima versione salvata.

## Salvare come modello

Un modello è un documento che usi come punto di partenza. Fai clic su **File > Salva come modello** e digita un nome. Il modello finisce in **I miei modelli**, e il documento che stai scrivendo non viene modificato. In **Salva con nome** puoi anche scegliere **Modello (*.dotx)** oppure **Modello di testo OpenDocument (*.ott)** per salvare un file modello dove preferisci.

Per iniziare un nuovo documento dal tuo modello, fai clic su **File > Nuovo**. I tuoi modelli sono elencati in **I miei modelli**. Fai clic su uno di essi. Si apre un nuovo documento con il testo e gli stili del modello. Per rimuovere un modello dall'elenco, fai clic con il pulsante destro su di esso e scegli **Rimuovi da I miei modelli**.

## Altri modi per aprire un file

Nella pagina **Apri**, fai clic su **Altri modi per aprire**. Puoi anche fare clic con il pulsante destro su un nome nell'elenco dei recenti. Ogni modo chiede un file, oppure usa quello su cui hai fatto clic:

- **Apri in sola lettura** apre il file solo per la lettura. Puoi guardare, scorrere, cercare e copiare, ma non puoi modificare nulla. La barra del titolo riporta "Sola lettura". Se premi **Salva**, JeffWrite chiede un nuovo nome, così il file aperto non viene modificato.
- **Apri come copia** apre un nuovo documento con il testo del file. Non ha un file proprio, quindi **Salva** chiede dove metterlo. Il file aperto non viene mai modificato.
- **Apri in nuova finestra** apre il file in una finestra propria, anche quando la finestra in cui ti trovi è vuota.
- **Apri e ripara** serve per un file .docx o .doc danneggiato, per esempio uno interrotto durante la copia. Vedi più avanti.

## Apri e ripara

A volte un file .docx o .doc è danneggiato. Una copia può essersi interrotta a metà, oppure un disco può aver commesso errori. Un file .docx è un insieme di parti più piccole, come il testo, gli stili e le immagini. **Apri e ripara** legge ogni parte che può. Una parte interrotta a metà viene conservata fino al punto in cui si è interrotta. Una parte che non può essere letta viene omessa. Se l'elenco delle parti o i collegamenti tra di esse sono scomparsi, JeffWrite li ricostruisce.

Un vecchio file .doc è un unico grande contenitore con alcuni flussi di dati al suo interno. Un flusso contiene il testo, un altro la tabella che indica dove si trova ciascun pezzo di testo e come appare. Se il contenitore è interrotto o i suoi elenchi sono danneggiati, **Apri e ripara** riconosce quei flussi dal loro contenuto e recupera quanto più possibile. Quando gli elenchi che indicano come si incastrano i pezzi sono danneggiati, il programma prende ciascun pezzo come seguito di quello precedente e mostra ciò che trova. Quel testo potrebbe appartenere a una copia precedente del documento o a un'altra parte del file, quindi il messaggio indica il paragrafo da cui inizia e ti invita a confrontarlo con un backup, se ne hai uno. Se la supposizione precede il testo, il messaggio dice che l'intero documento è stato ricomposto; se la segue, dice che è stata ricomposta la formattazione. Se le immagini non sono state recuperate completamente, il messaggio lo indica. Se la tabella è perduta, il testo viene conservato come paragrafi semplici, senza stili, elenchi, tabelle o immagini. Un file .doc con password non può essere riparato.

Il documento riparato si apre come copia con un nome del tipo "Lettera riparata". Un messaggio ti dice che cosa è stato conservato fino a un'interruzione e che cosa è stato omesso. Il file danneggiato non viene modificato. Salva la copia riparata con un nome proprio e verificala prima di fidarti di essa.

## Recuperare il lavoro non salvato

JeffWrite salva una copia di riserva del lavoro non salvato ogni pochi minuti, come file nella sua cartella. Se il programma si chiude in modo imprevisto, alla volta successiva in cui lo avvii te le propone. Fai clic su **File > Apri > Recupera lavoro non salvato** per vederle in seguito. Puoi scegliere quali aprire e salvare ciascuna dove preferisci.

## Iniziare da un modello

**File > Nuovo** mostra **Documento vuoto** e un insieme di modelli forniti con JeffWrite, raggruppati per tipo: una lettera, un rapporto, una relazione breve, un manoscritto e un notiziario. Ogni immagine mostra la prima pagina del documento che ottieni. Un **modello** è un documento già predisposto, con i suoi stili, margini e testo di esempio. Fai clic su uno e sostituisci il testo di esempio con il tuo. I modelli che hai salvato tu sono elencati in **I miei modelli**.

## Riprendere da dove ti eri fermato

JeffWrite ricorda dove si trovava il cursore quando hai salvato o chiuso un file. La volta successiva in cui apri il file, il cursore è lì. Premi **Maiusc+F5** per passare ai punti che hai modificato per ultimi. Ogni pressione porta al punto precedente: gli ultimi tre punti, poi di nuovo il primo.

## Salvare come .odt

Un file .odt non può contenere tutto ciò che può contenere un file .docx. Quando salvi come .odt, JeffWrite elenca per prima cosa ciò che andrebbe perso e ti permette di salvare invece un file .docx. Vedi [Informazioni](info).

## Stampare

Premi **Ctrl+P**, oppure fai clic su **File > Stampa**. Scegli una stampante, le copie e le pagine, controlla ciascuna pagina nell'anteprima e fai clic su **Stampa**. Vedi [Stampa](print).

## Creare un PDF

Fai clic su **File > Esporta > Esporta in PDF**. Il PDF riporta titolo, autore, oggetto e parole chiave da **File > Informazioni** (con **Includi proprietà del documento** attivo), la data di creazione e JeffWrite come programma che l'ha creato; l'autore e un titolo ricavato dal nome del file restano fuori quando le informazioni personali vengono rimosse. Ciascuno di questi valori viene conservato per intero fino a 4.000 caratteri. Seleziona **Apri il PDF dopo il salvataggio** per aprire il file nel programma PDF del computer al momento del salvataggio. È disattivato finché non lo selezioni. Se il computer non riesce ad aprire il file, la barra di stato lo segnala (il file viene comunque salvato). Scegli quali pagine includere: tutte, la pagina corrente oppure un intervallo come 1-3, 5 (anche p2s3 e s3 funzionano). Poi scegli come viene creato:

- **Ottimizza per** ha **Standard**, che mantiene le immagini così come sono, e **Dimensioni minime**, che porta ogni immagine a non più di 150 pixel per pollice (circa 59 per centimetro), in modo che il file sia più piccolo. Usa Dimensioni minime per un PDF da pubblicare su una pagina web o da inviare via email.
- **Includi proprietà del documento** inserisce nel PDF titolo, autore, oggetto e parole chiave del documento.
- **Crea segnalibri dai titoli** trasforma ogni titolo in un **segnalibro**, una voce dell'elenco che un lettore PDF mostra accanto alle pagine. Fai clic su un segnalibro per saltare alla sua pagina.
- **Tag della struttura del documento per l'accessibilità** etichetta titoli, paragrafi, elenchi, tabelle (con le relative righe di intestazione) e immagini (con il loro testo alternativo), e indica la lingua, così che uno screen reader legga il PDF nell'ordine corretto.

Infine scegli un nome. Il PDF appare uguale su ogni computer, perché i caratteri sono incorporati al suo interno. Anche il PDF che **Stampa** crea quando scegli un file PDF contiene segnalibri e tag. Quando sistema più pagine su un unico foglio, ogni segnalibro e ogni collegamento porta al foglio che contiene la sua pagina, e le pagine su un foglio vengono lette nell'ordine in cui sono disposte: da sinistra a destra, poi dall'alto in basso.

## Salvare come pagina web

Fai clic su **File > Esporta** e scegli uno dei due tipi di pagina web. Una **pagina web** è un file che puoi aprire in qualsiasi browser.

- **Salva come pagina web** crea un unico file .html che contiene le parole, il loro aspetto e le immagini. Puoi inviare quel solo file. L'aspetto conservato comprende carattere, dimensione, colore, evidenziazione, grassetto, corsivo, sottolineatura, allineamento e spazio tra i paragrafi.
- **Salva come pagina web filtrata** crea un piccolo file .html con la sola struttura: titoli, grassetto e corsivo, elenchi, tabelle, collegamenti e immagini. Non ha caratteri, dimensioni o colori, così potrai assegnargli un aspetto tuo in seguito.

Entrambi conservano titoli, elenchi puntati e numerati, tabelle con celle unite, collegamenti e immagini. Il testo nascosto, le parole eliminate con una modifica registrata, i commenti, le intestazioni, i piè di pagina e le note vengono omessi. Un collegamento che avvierebbe un programma o eseguirebbe uno script viene salvato come semplice testo.

## Salvare come e-book

Un **e-book** è un libro che leggi su un lettore di e-book, su un telefono o su un tablet. Il file è un **EPUB**. Il testo si adatta a qualsiasi schermo, così il lettore può ingrandire o rimpicciolire le lettere.

1. Fai clic su **File > Esporta > Salva come e-book (EPUB)**.
2. Controlla **Titolo** e **Autore**. Inizialmente corrispondono a titolo e autore nelle proprietà del documento (**File > Informazioni**). Puoi modificarli solo per questo libro.
3. Attiva **Usa la prima immagine come copertina** se vuoi che la prima immagine del documento sia la copertina. Senza di essa, il libro non ha copertina.
4. Fai clic su **OK**, poi scegli dove salvare il file.

Il libro viene creato così:

- Ogni **Titolo 1** inizia un nuovo capitolo. Le parole che precedono il primo Titolo 1 costituiscono un capitolo a sé.
- Il **sommario** elenca tutti i Titolo 1, Titolo 2 e Titolo 3, e il lettore può saltare a ciascuno di essi. I titoli più piccoli sono nel libro ma non nel sommario.
- Immagini e tabelle restano dove le hai inserite. Il testo alternativo di un'immagine la accompagna, così uno screen reader può descrivere ciò che mostra.
- Le note a piè di pagina e le note di chiusura sono collocate alla fine del capitolo in cui si trovano. Ogni nota ha un collegamento di ritorno al punto del testo, e ogni numero nel testo è un collegamento alla sua nota.

Un libro è soggetto a limiti, così che un file danneggiato non possa generarne uno enorme: fino a 500 capitoli, 4.000 righe nel sommario, 1.000 immagini (nessuna oltre 32 MB) e 20.000 note. Ciò che supera un limite resta nell'ultimo capitolo oppure diventa semplice testo, e le tue parole non vengono mai perse: un'immagine oltre il limite è sostituita dal suo testo alternativo, e una nota oltre il limite non ha formattazione. Un'immagine che il lettore non può mostrare (un metafile, una bitmap o un TIFF) viene convertita in PNG. Esportare di nuovo lo stesso documento salvato produce lo stesso libro, così un lettore ritrova il punto in cui si era fermato.

## Proteggere un PDF con una password

Nella casella **Esporta in PDF**, fai clic su **Cifra con una password…**. Digita la password due volte. Chiunque apra il PDF dovrà digitarla. La password protegge davvero il file: le pagine sono cifrate con una crittografia robusta (AES-256), e nessuno può leggerle senza di essa.

Puoi anche selezionare **Limita stampa, copia e modifica**, indicare una seconda password e scegliere cosa è consentito. Tali limitazioni sono solo una richiesta. La maggior parte dei lettori PDF le rispetta, ma non impediscono a chi voglia aggirarle di farlo, quindi non contarci per mantenere un file riservato.

Una password dimenticata non può essere recuperata, quindi conservala in un luogo sicuro. La password viene usata solo per quel PDF e non viene mai salvata. La casella disattiva **PDF/A** mentre è impostata una password, perché un file PDF/A non può averne una.

## Creare un PDF/A

Nella casella **Esporta in PDF**, seleziona **PDF/A, per la conservazione a lungo termine**. Un file PDF/A (PDF/A-1b) contiene tutti i caratteri che usa, indica i suoi colori e rispetta uno standard pensato per conservare i file per molti anni. Può essere etichettato per gli screen reader, ma non può avere una password.

## Creare un messaggio email

Fai clic su **File > Esporta > Crea un messaggio email** per creare un file di messaggio (.eml). Non viene inviato nulla. Scegli cosa inserire nel messaggio: il documento, il documento come PDF oppure le pagine visualizzate come immagini nel messaggio (le prime 30 pagine). Apri il file nel tuo programma di posta, aggiungi il destinatario e invialo. Per un PDF, puoi fare clic su **Cifra con una password…** nella casella; comunica la password alla persona in un altro modo, non nel messaggio.

## Caratteri nel file

Un file .docx può contenere i caratteri in cui è composto il suo testo, così il documento appare come l'autore lo ha creato, anche su un computer che non ha quei caratteri. JeffWrite li legge quando apre il file. Vengono usati solo per quel documento, mentre è aperto. Non vengono mai installati sul tuo computer, nessun altro documento o programma li vede, e scompaiono quando chiudi il documento. Un carattere contenuto nel file non viene considerato un carattere mancante.

JeffWrite non usa un carattere la cui licenza del produttore vieta l'incorporamento, un carattere danneggiato, né alcun carattere oltre i primi 64 di un file. In tal caso, il messaggio che vedi all'apertura del file lo indica, e il testo viene mostrato con un carattere simile. Il file conserva i suoi caratteri così come erano quando lo salvi.

Per inserire nei file salvati i caratteri usati dal testo, fai clic su **File > Opzioni**, apri la pagina **Salva** e attiva **Salva i caratteri nel file**. All'inizio è disattivato. JeffWrite inserisce allora i caratteri che lo accompagnano e che il testo usa, in ogni stile usato dal documento, quando salva un file .docx. Non inserisce mai un carattere la cui licenza non lo consente, e quando ne omette uno, il messaggio dopo il salvataggio ne indica il nome. Il file diventa più grande, e in un unico file possono essere inseriti al massimo 64 caratteri e 256 MB.

## Vedi anche

- [Benvenuti in JeffWrite](start)
- [Stampa](print)
- [Usare JeffWrite con la tastiera](keyboard)
