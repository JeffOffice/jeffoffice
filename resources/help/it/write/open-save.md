# Apri, salva, stampa ed esporta
<!-- keywords: pagina web, html, salva come pagina web, pagina web filtrata, posta elettronica, eml, segnalibri, segnalibri pdf, maiusc+f5, ultima modifica, dove ti sei fermato, modelli predefiniti, raccolta, lettera, rapporto, relazione, manoscritto, notiziario, apri, salva, salva con nome, docx, doc, dot, odt, opendocument, rtf, file meno recenti, modello, dotx, recenti, aggiungi, fissa, nuovo, chiudi, stampa, pdf, esporta, recupera, recupero automatico, non salvato, arresto anomalo, schermata file, formati file, salva una copia, sola lettura, apri in sola lettura, apri come copia, nuova finestra, copia di backup, i miei modelli, salva come modello, ott, ripara, apri e ripara, danneggiato, password, cifra, proteggi con password, copia di sicurezza, backup, file, documento, collegamento, ctrl+s, ctrl+p, ctrl+n, ctrl+o -->

Fai clic su **File** in alto a sinistra per vedere la schermata File. Contiene le pagine Nuovo, Apri, Stampa ed Esporta, e i comandi Salva e Salva con nome. Premi **Esc** per tornare al documento.

## Creare un nuovo documento

Premi **Ctrl+N**, oppure fai clic su **File > Nuovo** e poi su **Documento vuoto**. Un nuovo documento si apre in una finestra propria. Se la finestra in cui ti trovi è vuota e non è mai stata salvata, il nuovo documento la sostituisce.

## Aprire un documento

Premi **Ctrl+O**, oppure fai clic su **File > Apri**. Fai clic su **Sfoglia** per scegliere un file. JeffWrite apre questi tipi di file:

- **.docx** è un documento.
- **.dotx** è un modello. Un modello si apre come nuovo documento creato a partire da esso, così il modello resta com'è.
- **.docm** e **.dotm** sono un documento e un modello che possono contenere **macro**, piccoli programmi che accompagnano un file. JeffWrite non le esegue mai, e le conserva come erano quando salvi come .docm o .dotm. Se salvi come .docx o .odt, le macro vengono omesse e JeffWrite te lo segnala.
- **.txt** è testo normale, solo le parole. Quando lo apri, JeffWrite lo legge subito se è ASCII semplice o Unicode. Se non lo è, JeffWrite chiede con quale **codifica** leggerlo e mostra le prime righe, così puoi verificare che le lettere siano corrette. La **codifica** è il modo in cui un file memorizza le lettere, come UTF-8 o Windows 1252. Quando salvi come .txt, scegli la codifica e i caratteri di fine riga, e il file conserva solo le parole: niente tipi di carattere, immagini o linee di tabella.
- **.html** e **.htm** sono pagine web. JeffWrite legge le parole, i titoli, il grassetto, il corsivo, la sottolineatura, i colori, gli elenchi e le tabelle. I punti e i numeri degli elenchi arrivano come segni digitati, mentre immagini e collegamenti vengono omessi. Non scarica mai nulla da internet e ignora gli script. Una pagina web non viene mai sovrascritta: **Salva** chiede un nuovo nome e salva un file .docx. Per creare una pagina web, vedi **Salva come pagina web** più avanti.
- **.doc** e **.dot** sono i tipi di documento e modello meno recenti. JeffWrite li legge e salva ciò che fai con essi come nuovo file .docx.
- **.odt**, **.ott** e **.fodt** sono file OpenDocument. JeffWrite li legge tutti e tre. Salva i file .odt, e salva ciò che fai con un file .ott o .fodt come nuovo file .odt. Vedi [Aprire e salvare file OpenDocument](open-odt).
- **.rtf** è Rich Text Format. JeffWrite lo legge e lo salva. Vedi [Aprire file RTF](open-rtf).

La pagina **Apri** elenca anche i documenti che hai aperto o salvato di recente. Fai clic su uno per aprirlo. Fai clic con il pulsante destro su uno per fissarlo in cima all'elenco, o per toglierlo dall'elenco.

Se un file contiene parti che JeffWrite non può ancora modificare, come il layout di una tabella o un'intestazione, le conserva. Vedi [Benvenuti in JeffWrite](start). Se una parte del file non può essere letta affatto, JeffWrite ti dice che cosa ha omesso. Non salva poi sopra quel file: Salva chiede un nuovo nome, così l'originale conserva ciò che la copia ha perso.

## Salvare un documento

Premi **Ctrl+S** per salvare. La prima volta, JeffWrite chiede un nome. **Salva con nome** (F12) salva una copia con un nuovo nome. Scegli **Modello (*.dotx)** per salvare un modello, **Documento con macro (*.docm)** per conservare le macro, **Testo normale (*.txt)** per le sole parole, oppure **Rich Text Format (*.rtf)** per un file che altri programmi possono leggere. Un file proveniente da un tipo di file meno recente viene sempre salvato con un nuovo nome, così il vecchio file non viene mai sovrascritto.

## Salvare una copia

Fai clic su **File > Salva una copia**. Scegli un nome. JeffWrite scrive il documento in quel file, e tu continui a lavorare nel documento che avevi aperto. Il suo nome e le modifiche non salvate restano come erano. È utile quando vuoi inviare una versione a qualcuno e continuare a scrivere.

## Conservare una copia di backup

Fai clic su **File > Opzioni**, e nella pagina **Generali** attiva **Crea sempre una copia di backup**. Ogni volta che salvi, JeffWrite copia prima il vecchio file in uno chiamato "Backup di" seguito dal suo nome, nella stessa cartella. Se la copia non può essere creata, JeffWrite chiede se salvare senza di essa. La risposta predefinita è No, a meno che tu non risponda Sì; in questo caso il file resta com'era. Poi puoi tornare all'ultima versione salvata.

## Salvare come modello

Un modello è un documento che usi come punto di partenza. Fai clic su **File > Salva come modello** e digita un nome. Il modello finisce in **I miei modelli**, e il documento che stai scrivendo non viene modificato. In **Salva con nome** puoi anche scegliere **Modello (*.dotx)**, oppure **Modello di testo OpenDocument (*.ott)**, per salvare un file modello dove preferisci.

Per creare un nuovo documento a partire dal tuo modello, fai clic su **File > Nuovo**. I tuoi modelli sono elencati sotto **I miei modelli**. Fai clic su uno di essi. Si apre un nuovo documento con il testo e gli stili del modello. Per rimuovere un modello dall'elenco, fai clic con il pulsante destro su di esso e scegli **Rimuovi da I miei modelli**.

## Altri modi per aprire un file

Nella pagina **Apri**, fai clic su **Altri modi per aprire**. Puoi anche fare clic con il pulsante destro su un nome nell'elenco dei recenti. Ogni modo chiede un file, oppure usa quello su cui hai fatto clic:

- **Apri in sola lettura** apre il file solo per la lettura. Puoi guardare, scorrere, cercare e copiare, ma non modificare nulla. La barra del titolo riporta "Sola lettura". Se premi **Salva**, JeffWrite chiede un nuovo nome, così il file aperto non viene modificato.
- **Apri come copia** apre un nuovo documento con il testo del file. Non ha un file proprio, quindi **Salva** chiede dove metterlo. Il file aperto non viene mai modificato.
- **Apri in nuova finestra** apre il file in una finestra propria, anche quando la finestra in cui ti trovi è vuota.
- **Apri e ripara** serve per un file .docx danneggiato, ad esempio uno interrotto durante la copia. Vedi più avanti.

## Apri e ripara

A volte un file .docx è danneggiato. Una copia può essersi interrotta a metà, oppure un disco può aver commesso errori. Un file .docx è un insieme di parti più piccole, come il testo, gli stili e le immagini. **Apri e ripara** legge ogni parte che riesce a leggere. Una parte interrotta a metà viene conservata fino al punto in cui si è interrotta. Una parte che non può essere letta viene omessa. Se l'elenco delle parti o i collegamenti tra di esse sono andati persi, JeffWrite li ricrea.

Il documento riparato si apre come copia con un nome come "Lettera riparata". Un messaggio ti dice che cosa è stato conservato fino a un'interruzione e che cosa è stato omesso. Il file danneggiato non viene modificato. Salva la copia riparata con un nome proprio e verificala prima di fidarti di essa.

## Recuperare il lavoro non salvato

JeffWrite salva ogni pochi minuti una copia di riserva del lavoro non salvato, come file in una cartella propria. Se il programma si chiude in modo imprevisto, alla successiva apertura propone quelle copie. Fai clic su **File > Apri > Recupera lavoro non salvato** per vederle in un secondo momento. Puoi scegliere quali aprire, e salvare ciascuna dove preferisci.

## Iniziare da un modello

**File > Nuovo** mostra il **Documento vuoto** e una serie di modelli forniti con JeffWrite, raggruppati per tipo: una lettera, un rapporto, una relazione, un manoscritto e un notiziario. Ogni immagine è la prima pagina del documento che otterrai. Un **modello** è un documento già predisposto, con i suoi stili, margini e testi di esempio. Fai clic su uno, e sostituisci i testi di esempio con i tuoi. I modelli che hai salvato tu sono elencati sotto **I miei modelli**.

## Riprendere dal punto in cui ti sei fermato

JeffWrite ricorda dove si trovava il cursore quando hai salvato o chiuso un file. La volta successiva che apri il file, il cursore è lì. Premi **Maiusc+F5** per saltare agli ultimi punti in cui hai modificato. Ogni pressione va al precedente: gli ultimi tre punti, poi di nuovo al primo.

## Salvare come .odt

Un file .odt non può contenere tutto ciò che può contenere un file .docx. Quando salvi come .odt, JeffWrite elenca prima ciò che andrebbe perso, e ti permette di salvare invece un file .docx. Vedi [Informazioni](info).

## Stampare

Premi **Ctrl+P**, oppure fai clic su **File > Stampa**. Scegli una stampante, il numero di copie e le pagine, controlla ogni pagina nell'anteprima e fai clic su **Stampa**. Vedi [Stampa](print).

## Creare un PDF

Fai clic su **File > Esporta > Esporta in PDF**. Scegli quali pagine includere: tutte, la pagina corrente, oppure un intervallo come 1-3, 5 (qui funzionano anche p2s3 e s3). Poi scegli un nome. Il PDF appare uguale su ogni computer, perché i tipi di carattere sono incorporati al suo interno. Il titolo, l'autore, l'oggetto e le parole chiave del documento vengono inseriti nel PDF. Ogni titolo diventa un **segnalibro**, una voce dell'elenco che un lettore PDF mostra accanto alle pagine. Fai clic su un segnalibro per saltare alla sua pagina.

## Salvare come pagina web

Fai clic su **File > Esporta** e scegli uno dei due tipi di pagina web. Una **pagina web** è un file che puoi aprire in qualsiasi browser.

- **Salva come pagina web** crea un unico file .html che contiene le tue parole, il loro aspetto e le tue immagini. Puoi inviare quel solo file. L'aspetto conservato comprende il tipo di carattere, la dimensione, il colore, l'evidenziazione, il grassetto, il corsivo, la sottolineatura, l'allineamento e lo spazio attorno ai paragrafi.
- **Salva come pagina web filtrata** crea un piccolo file .html con la sola struttura: titoli, grassetto e corsivo, elenchi, tabelle, collegamenti e immagini. Non ha tipi di carattere, dimensioni o colori, così potrai dargli in seguito il tuo aspetto.

Entrambi conservano titoli, elenchi puntati e numerati, tabelle con celle unite, collegamenti e immagini. Il testo nascosto, le parole eliminate da una modifica registrata, i commenti, le intestazioni, i piè di pagina e le note vengono omessi. Un collegamento che avvierebbe un programma o eseguirebbe uno script viene salvato come semplice testo.

## Proteggere un PDF con una password

Nella finestra **Esporta in PDF**, fai clic su **Cifra con una password…**. Digita la password due volte. Chiunque apra il PDF dovrà digitarla. La password protegge davvero il file: le pagine sono cifrate con un metodo robusto (AES-256), e nessuno può leggerle senza di essa.

Puoi anche selezionare **Limita stampa, copia e modifica**, indicare una seconda password e scegliere che cosa è consentito. Quei limiti sono solo una richiesta. La maggior parte dei lettori PDF li rispetta, ma non impediscono a chi voglia aggirarli di farlo, quindi non contare su di essi per tenere privato un file.

Una password dimenticata non può essere recuperata, quindi conservala in un luogo sicuro. La password vale solo per quel PDF e non viene mai salvata. La casella disattiva **PDF/A** quando è impostata una password, perché un file PDF/A non può averne una.

## Creare un PDF/A

Nella finestra **Esporta in PDF**, seleziona **PDF/A, per la conservazione a lungo termine**. Un file PDF/A contiene tutti i tipi di carattere che usa e segue uno standard pensato per conservare i file per molti anni. Non è ancora etichettato per i lettori di schermo, e non può avere una password.

## Creare un messaggio di posta

Fai clic su **File > Esporta > Crea un messaggio e-mail** per creare un file di messaggio (.eml). Non viene inviato nulla. Scegli cosa inserire nel messaggio: il documento, il documento come PDF, oppure le pagine visualizzate come immagini nel messaggio (le prime 30 pagine). Apri il file nel tuo programma di posta, aggiungi il destinatario e invialo. Per un PDF, puoi fare clic su **Cifra con una password…** nella finestra; comunica la password al destinatario in un modo diverso dal messaggio.

## Vedi anche

- [Benvenuti in JeffWrite](start)
- [Stampa](print)
- [Usare JeffWrite con la tastiera](keyboard)
