# Apri, salva, stampa ed esporta
<!-- keywords: email, eml, segnalibri, segnalibri pdf, maiusc+f5, ultima modifica, dove eri rimasto, modelli predefiniti, raccolta, lettera, rapporto, breve, manoscritto, notiziario, apri, salva, salva con nome, docx, doc, dot, odt, opendocument, rtf, file meno recenti, modello, dotx, recenti, fissa, nuovo, chiudi, stampa, pdf, esporta, recupera, recupero automatico, non salvato, arresto anomalo, schermata file, formati file, salva una copia, sola lettura, apri in sola lettura, apri come copia, nuova finestra, copia di backup, modelli personali, salva come modello, ott, ripara, apri e ripara, danneggiato -->

Fai clic su **File** in alto a sinistra per visualizzare la schermata File. Contiene le pagine Nuovo, Apri, Stampa ed Esporta, oltre ai comandi Salva e Salva con nome. Premi **Esc** per tornare al documento.

## Creare un nuovo documento

Premi **Ctrl+N**, oppure fai clic su **File > Nuovo** e poi su **Documento vuoto**. Il nuovo documento si apre in una finestra propria. Se la finestra in cui ti trovi è vuota e non è mai stata salvata, il nuovo documento la sostituisce.

## Aprire un documento

Premi **Ctrl+O**, oppure fai clic su **File > Apri**. Fai clic su **Sfoglia** per scegliere un file. JeffWrite apre questi tipi di file:

- **.docx** è un documento.
- **.dotx** è un modello. Un modello si apre come nuovo documento basato su di esso, così il modello resta invariato.
- **.docm** e **.dotm** sono un documento e un modello che possono contenere **macro**, piccoli programmi che accompagnano un file. JeffWrite non le esegue mai. Le conserva come erano quando salvi come .docm o .dotm. Se salvi come .docx o .odt, le macro vengono omesse e JeffWrite te lo segnala.
- **.txt** è testo normale, solo le parole. All'apertura, JeffWrite lo legge subito se è in ASCII o Unicode semplice. In caso contrario, chiede in quale **codifica** leggerlo e mostra le prime righe, così puoi verificare che le lettere siano corrette. La **codifica** è il modo in cui un file memorizza le lettere, ad esempio UTF-8 o Windows 1252. Quando salvi come .txt, scegli la codifica e i caratteri di fine riga, e il file conserva solo le parole: niente caratteri, immagini o linee di tabella.
- **.doc** e **.dot** sono i tipi di documento e modello meno recenti. JeffWrite li legge e salva le modifiche come nuovo file .docx.
- **.odt**, **.ott** e **.fodt** sono file OpenDocument. JeffWrite legge tutti e tre. Salva i file .odt e salva come nuovo file .odt le modifiche apportate a un file .ott o .fodt. Vedi [Aprire e salvare file OpenDocument](open-odt).
- **.rtf** è Rich Text Format. JeffWrite lo legge e lo salva. Vedi [Aprire file RTF](open-rtf).

La pagina **Apri** elenca anche i documenti aperti o salvati di recente. Fai clic su uno per aprirlo. Fai clic con il pulsante destro su un documento per fissarlo in cima all'elenco o rimuoverlo dall'elenco.

Se un file contiene parti che JeffWrite non può ancora modificare, come il layout di una tabella o un'intestazione, le conserva. Vedi [Benvenuti in JeffWrite](start). Se una parte del file non può essere letta affatto, JeffWrite ti dice cosa ha omesso. Non sovrascrive quindi quel file: Salva chiede un nuovo nome, così l'originale conserva ciò che la copia ha perso.

## Salvare un documento

Premi **Ctrl+S** per salvare. La prima volta, JeffWrite chiede un nome. **Salva con nome** (F12) salva una copia con un nuovo nome. Scegli **Modello (*.dotx)** per salvare un modello, **Documento con macro (*.docm)** per conservare le macro, **Testo normale (*.txt)** per le sole parole, oppure **Rich Text Format (*.rtf)** per un file leggibile da altri programmi. Un file proveniente da un tipo meno recente viene sempre salvato con un nuovo nome, così il file originale non viene mai sovrascritto.

## Salvare una copia

Fai clic su **File > Salva una copia**. Scegli un nome. JeffWrite scrive il documento in quel file e tu continui a lavorare nel documento che avevi aperto. Il suo nome e le modifiche non salvate restano invariati. È utile quando vuoi inviare una versione a qualcuno e continuare a scrivere.

## Conservare una copia di backup

Fai clic su **File > Opzioni** e, nella pagina **Generale**, attiva **Crea sempre una copia di backup**. Ogni volta che salvi, JeffWrite copia prima il file precedente in uno chiamato "Backup di" seguito dal suo nome, nella stessa cartella. Se non è possibile creare la copia, JeffWrite chiede se salvare comunque. La risposta predefinita è No, a meno che tu non risponda Sì, e il file resta com'era. In questo modo puoi tornare all'ultima versione salvata.

## Salvare come modello

Un modello è un documento che usi come punto di partenza. Fai clic su **File > Salva come modello** e digita un nome. Il modello finisce in **Modelli personali**, e il documento su cui stai scrivendo non viene modificato. In **Salva con nome** puoi anche scegliere **Modello (*.dotx)** oppure **Modello di testo OpenDocument (*.ott)** per salvare un file modello dove preferisci.

Per creare un nuovo documento dal tuo modello, fai clic su **File > Nuovo**. I tuoi modelli sono elencati sotto **Modelli personali**. Fai clic su uno di essi. Si apre un nuovo documento con il testo e gli stili del modello. Per rimuovere un modello dall'elenco, fai clic su di esso con il pulsante destro e scegli **Rimuovi da Modelli personali**.

## Altri modi per aprire un file

Nella pagina **Apri**, fai clic su **Altri modi per aprire**. Puoi anche fare clic con il pulsante destro su un nome nell'elenco Recenti. Ogni modo chiede un file, oppure usa quello su cui hai fatto clic:

- **Apri in sola lettura** apre il file solo per la lettura. Puoi guardare, scorrere, cercare e copiare, ma non puoi modificare nulla. La barra del titolo riporta "Sola lettura". Se premi **Salva**, JeffWrite chiede un nuovo nome, così il file aperto non viene modificato.
- **Apri come copia** apre un nuovo documento con il testo del file. Non ha un file proprio, quindi **Salva** chiede dove collocarlo. Il file aperto non viene mai modificato.
- **Apri in una nuova finestra** apre il file in una finestra propria, anche quando la finestra in cui ti trovi è vuota.
- **Apri e ripara** serve per un file .docx danneggiato, ad esempio uno interrotto durante la copia. Vedi sotto.

## Aprire e riparare

A volte un file .docx è danneggiato. Una copia può essersi interrotta a metà, oppure un disco può aver commesso errori. Un file .docx è un insieme di parti più piccole, come il testo, gli stili e le immagini. **Apri e ripara** legge ogni parte che riesce a leggere. Una parte interrotta a metà viene conservata fino al punto in cui si è fermata. Una parte che non può essere letta viene omessa. Se l'elenco delle parti o i collegamenti tra di esse sono andati persi, JeffWrite li ricrea.

Il documento riparato si apre come copia con un nome del tipo "Lettera riparata". Un messaggio ti indica cosa è stato conservato fino a un'interruzione e cosa è stato omesso. Il file danneggiato non viene modificato. Salva la copia riparata con un nome proprio e verificala prima di fidarti.

## Recuperare il lavoro non salvato

JeffWrite salva una copia di riserva del lavoro non salvato ogni pochi minuti, come file in una cartella dedicata. Se il programma si chiude in modo imprevisto, alla successiva apertura propone quelle copie. Fai clic su **File > Apri > Recupera lavoro non salvato** per vederle in seguito. Puoi scegliere quali aprire e salvare ciascuna dove preferisci.

## Iniziare da un modello

**File > Nuovo** mostra **Documento vuoto** e un insieme di modelli forniti con JeffWrite, raggruppati per tipo: lettera, rapporto, breve, manoscritto e notiziario. Ogni immagine è la prima pagina del documento che otterrai. Un **modello** è un documento già predisposto, con i suoi stili, margini e testi di esempio. Fai clic su uno di essi e sostituisci i testi di esempio con i tuoi. I modelli che hai salvato tu sono elencati sotto **Modelli personali**.

## Riprendere dove avevi interrotto

JeffWrite ricorda dove si trovava il cursore quando hai salvato o chiuso un file. La volta successiva che apri il file, il cursore è lì. Premi **Maiusc+F5** per passare ai punti in cui hai modificato per ultimi. Ogni pressione porta al punto precedente: gli ultimi tre punti, poi di nuovo il primo.

## Salvare come .odt

Un file .odt non può contenere tutto ciò che può contenere un file .docx. Quando salvi come .odt, JeffWrite elenca per prima cosa ciò che andrebbe perso e ti permette di salvare invece un file .docx. Vedi [Info](info).

## Stampare

Premi **Ctrl+P**, oppure fai clic su **File > Stampa**. Scegli una stampante, le copie e le pagine, controlla ogni pagina nell'anteprima e fai clic su **Stampa**. Vedi [Stampa](print).

## Creare un PDF

Fai clic su **File > Esporta > Esporta in PDF**. Scegli quali pagine includere: tutte, la pagina corrente oppure un intervallo come 1-3, 5 (anche p2s3 e s3 funzionano qui). Poi scegli un nome. Il PDF appare uguale su ogni computer, perché i tipi di carattere sono incorporati al suo interno. Il titolo, l'autore, l'oggetto e le parole chiave del documento vengono inseriti nel PDF. Ogni titolo diventa un **segnalibro**, una voce dell'elenco che il lettore PDF mostra accanto alle pagine. Fai clic su un segnalibro per passare alla sua pagina.

## Creare un messaggio di posta elettronica

Fai clic su **File > Esporta > Crea un messaggio** per creare un file di messaggio (.eml). Non viene inviato nulla. Scegli cosa inserire nel messaggio: il documento, il documento come PDF, oppure le pagine visualizzate come immagini nel messaggio (le prime 30 pagine). Apri il file con il tuo programma di posta, aggiungi il destinatario e invia.

## Vedi anche

- [Benvenuti in JeffWrite](start)
- [Stampa](print)
- [Usare JeffWrite da tastiera](keyboard)
