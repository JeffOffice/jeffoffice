# Aprire e salvare presentazioni
<!-- keywords: aprire, apri, salvare, salva, salva con nome, otp, libreoffice, omesso, escluso, chiudere, nuova, file recenti, aggiunti, tipi di file, formati, pptx, potx, ppsx, ppt, odp, modello, presentazione automatica, rinominare, CTRL+S, backup, copia di sicurezza, formati di file, file vecchi, apri documento, conservato, non visualizzato, tipi di carattere, font, incorpora tipi di carattere, caratteri incorporati, carattere mancante -->

JeffDeck salva il lavoro in file .pptx e in file .odp e apre diversi tipi di file di presentazione.

## Iniziare una nuova presentazione

Premi CTRL+N. Si apre una nuova finestra con una presentazione vuota. In alternativa, fai clic su **File > Nuovo** e quindi su **Presentazione vuota**. Se la finestra corrente contiene una nuova presentazione che nessuno ha ancora modificato, la presentazione vuota prende il suo posto. Altrimenti, si apre in una finestra separata.

## Salvare

Premi CTRL+S oppure fai clic su **File > Salva**. La prima volta, JeffDeck chiede un nome e una posizione. In seguito, Salva sostituisce il file con il lavoro più recente. Un messaggio nella parte inferiore della finestra mostra "Salvato" e il nome del file.

Un asterisco (*) dopo il nome nella barra del titolo indica che ci sono modifiche non salvate. Salva spesso. Se JeffDeck si chiude in modo imprevisto, vedi [Recuperare il lavoro non salvato](recover).

Se un messaggio indica che alcuni oggetti "non possono ancora essere salvati in questo tipo di file", il file viene salvato, ma JeffDeck non è stato in grado di scrivere quegli oggetti. Conserva anche il file originale. Quando salvi un file .odp, il messaggio elenca ciò che quel tipo di file non può contenere. La sezione "Salvare come file .odp" più avanti descrive ciò che non può contenere.

## Salvare con un nuovo nome o tipo

Fai clic su **File > Salva con nome** oppure premi F12. Digita un nome, scegli il tipo e fai clic su **Salva**. Le opzioni sono:

- **Presentazione (*.pptx)** è la scelta normale.
- **Modello (*.potx)** salva le diapositive come modello da cui creare nuove presentazioni.
- **Presentazione automatica (*.ppsx)** salva una presentazione da riprodurre.
- **Presentazione OpenDocument (*.odp)** salva le diapositive nel tipo di file usato da LibreOffice e da altri programmi gratuiti.
- **Modello di presentazione OpenDocument (*.otp)** è la versione modello di quel tipo di file.

Se ometti l'estensione, JeffDeck aggiunge quella corrispondente al tipo. Dopo il salvataggio, la barra del titolo mostra il nuovo nome e continui a lavorare nel nuovo file.

Una presentazione aperta da un modello viene salvata come nuova presentazione: **Salva** chiede un nome, in modo che il modello rimanga invariato. Lo stesso vale per una presentazione aperta da un file .ppt, .pps o .pot meno recente. Una presentazione aperta da un file .odp viene salvata di nuovo come file .odp, sovrascrivendo lo stesso file.

## Aprire

Premi CTRL+O per scegliere subito un file. In alternativa, fai clic su **File > Apri** e quindi su **Sfoglia** oppure su un nome in **Aggiunti** o **Presentazioni recenti**.

Il file si apre in questa finestra se contiene una nuova presentazione che nessuno ha ancora modificato. Altrimenti, si apre in una finestra separata.

Fai clic con il pulsante destro del mouse su un nome nell'elenco per scegliere **Aggiungi all'elenco** (il nome rimane in cima), **Rimuovi dagli elementi aggiunti** o **Rimuovi dall'elenco**. Un nome in grigio indica che JeffDeck al momento non riesce a trovare il file.

## Tipi di carattere nel file

Un tipo di carattere definisce la forma delle lettere. Non tutti i computer hanno tutti i tipi di carattere. Se in un computer manca un tipo di carattere usato nelle diapositive, le lettere vengono visualizzate con un altro tipo di carattere e le diapositive possono avere un aspetto diverso.

JeffDeck può inserire nel file i tipi di carattere usati nelle diapositive, in modo che abbiano lo stesso aspetto su qualsiasi computer. Questa operazione si chiama incorporamento. JeffDeck la esegue a ogni salvataggio. Per disattivarla, fai clic su **File > Opzioni**, apri la scheda **Salvataggio** e deseleziona **Incorpora tipi di carattere nel file**.

- JeffDeck incorpora solo i tipi di carattere forniti con il programma, ad esempio Fraunces, Lato e Open Sans, che gli altri computer non hanno. Non incorpora mai un tipo di carattere presente su tutti i computer, ad esempio Calibri o Arial. Esclude i tipi di carattere la cui licenza non ne consente la copia in un file.
- JeffDeck incorpora ogni stile usato nelle diapositive: normale, grassetto, corsivo e grassetto corsivo. Se le diapositive usano solo lo stile normale, viene incorporato solo quello.
- JeffDeck incorpora tutte le lettere di ogni tipo di carattere, non solo quelle usate. In questo modo chi apre il file può comunque modificare il testo.
- Il file diventa più grande. Ogni stile di un tipo di carattere aggiunge da 40 a 700 KB circa (un KB equivale a mille byte). Il file viene compresso durante il salvataggio, quindi aumenta di circa la metà.

Quando apri un file che contiene tipi di carattere incorporati, JeffDeck li usa mentre la presentazione è aperta e le diapositive hanno lo stesso aspetto che avevano nel computer in cui sono state create. JeffDeck non installa i tipi di carattere nel computer e non li copia altrove. Appartengono solo a quella presentazione: non compaiono negli elenchi dei tipi di carattere e nessun'altra presentazione li usa. Vengono rimossi quando chiudi la presentazione. Se nel computer è già presente un tipo di carattere, JeffDeck usa la copia installata.

JeffDeck legge i tipi di carattere dai file .pptx creati con altri programmi di presentazione, anche se sono stati compressi. Legge anche i tipi di carattere dai file .odp.

Un file può contenere un tipo di carattere che JeffDeck non ha. Quando salvi, JeffDeck lo reinserisce nel file, purché le diapositive lo usino ancora, indipendentemente dal fatto che l'opzione **Incorpora tipi di carattere nel file** sia attivata. Se un tipo di carattere in un file è danneggiato o non può essere letto, il messaggio visualizzato all'apertura del file lo segnala.

## Tipi di file letti da JeffDeck

- **.pptx, .potx e .ppsx** sono un unico formato in tre varianti: presentazione, modello e presentazione automatica. JeffDeck legge e scrive tutte e tre.
- **.ppt, .pps e .pot** sono il formato meno recente. JeffDeck li legge ma non può scriverli, quindi **Salva** chiede un nuovo nome e salva un file .pptx (.ppsx per un file .pps). Non sovrascrive mai l'originale.
- **.odp e .otp** sono presentazioni OpenDocument e i relativi modelli. JeffDeck legge e scrive entrambi. Un file .odp aperto in JeffDeck viene salvato come .odp. Un file .otp è un modello, quindi **Salva** chiede un nuovo nome e propone un file .odp.

Se una parte di un file non può essere letta, JeffDeck apre il resto ed elenca ciò che ha omesso. Se non è possibile leggere l'intero file, ne indica il motivo.

## Salvare come file .odp

Un file .odp si apre in LibreOffice Impress con diapositive, master, immagini, note e commenti, e con l'aspetto visualizzato in JeffDeck. Contiene anche animazioni, transizioni, presentazioni personalizzate e impostazioni della presentazione. Il tipo di carattere di ogni parola viene salvato per nome, quindi un computer che non ha quel tipo di carattere ne visualizza un altro.

JeffDeck non è ancora in grado di rileggere da un file .odp animazioni, commenti, presentazioni personalizzate o impostazioni della presentazione. Sono presenti nel file e LibreOffice li riproduce, ma quando apri il file .odp in JeffDeck, vengono elencati come omessi. Conserva il file .pptx se vuoi continuare a modificarli in JeffDeck.

Un file .odp non può contenere alcuni elementi. JeffDeck li elenca dopo il salvataggio, nel messaggio nella parte inferiore della finestra:

- **Grafici, diagrammi SmartArt ed equazioni** vengono salvati come immagini. Sono visibili, ma nel file .odp non è possibile modificarne i valori o le parti.
- **Le sezioni** non vengono salvate.
- **Smussature e ombreggiature interne** vengono omesse, così come alone e riflesso del testo. L'ombreggiatura del testo viene mantenuta. Il riflesso di una forma, una linea con sfumatura, una linea doppia o tripla e le dimensioni di un'ombreggiatura vengono mantenuti in un modo leggibile solo da JeffDeck. Un altro programma mostra la forma senza riflesso, la linea nel primo colore, una linea singola e l'ombreggiatura con le dimensioni dell'oggetto.
- **Gli effetti di animazione** che si avviano al clic su un oggetto e gli effetti che cambiano un colore vengono omessi. Il clic su un video per sospenderlo o riprodurlo di nuovo viene mantenuto.
- **Un video o un audio collegato** da un altro file, non contenuto nella presentazione, mantiene il percorso e l'immagine nel file .odp. LibreOffice lo riproduce se riesce a trovare il file. I video e gli audio contenuti nella presentazione vengono mantenuti nel file .odp e vengono riprodotti.

Conserva anche il file .pptx se ti servono di nuovo questi elementi.

## Elementi conservati ma non visualizzati

Alcune parti di un file sono elementi che JeffDeck non è ancora in grado di visualizzare o modificare. Vengono conservati con le relative diapositive e riscritti al salvataggio, in modo che non vadano persi. Ne fanno parte le animazioni, che vengono riprodotte nella presentazione, video e audio, che non vengono ancora riprodotti, e gli effetti di transizione non supportati da JeffDeck, che vengono riprodotti come dissolvenza. Grafici e diagrammi SmartArt vengono visualizzati come sono stati salvati. Vedi [Benvenuti in JeffDeck](start).

## Argomenti correlati

- [Recuperare il lavoro non salvato](recover)
- [La schermata File](file-screen)
- [Riutilizzare diapositive da un'altra presentazione](reuse)
- [Stampa, PDF e immagini delle diapositive](printing)
