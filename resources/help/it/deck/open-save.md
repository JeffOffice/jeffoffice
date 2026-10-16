# Aprire e salvare le presentazioni
<!-- keywords: aprire, apri, salvare, salva, salva con nome, otp, libreoffice, omesso, tralasciato, chiudere, chiudi, nuovo, nuova presentazione, file recenti, aggiunti, tipi di file, formati, pptx, potx, ppsx, ppt, odp, modello, presentazione automatica, rinominare, CTRL+S, backup, copia di sicurezza, formati di file, file vecchi, OpenDocument, conservato, non visualizzato, tipi di carattere, font, incorpora tipi di carattere, caratteri incorporati, carattere mancante -->

JeffDeck salva il lavoro in file .pptx e in file .odp e apre diversi tipi di file di presentazione.

## Creare una nuova presentazione

Premi CTRL+N. Si apre una nuova finestra con una presentazione vuota. In alternativa, fai clic su **File > Nuovo** e quindi su **Presentazione vuota**. Se la finestra in cui ti trovi contiene una nuova presentazione che nessuno ha ancora modificato, quella vuota prende il suo posto. Altrimenti, si apre in una finestra separata.

## Salvare

Premi CTRL+S oppure fai clic su **File > Salva**. La prima volta, JeffDeck chiede un nome e una posizione. In seguito, Salva sostituisce il file con il lavoro più recente. Un messaggio in basso indica "Salvato" e il nome del file.

Un asterisco (*) dopo il nome nella barra del titolo indica che ci sono modifiche non salvate. Salva spesso. Se JeffDeck si chiude in modo imprevisto, vedi [Recuperare il lavoro non salvato](recover).

Se un messaggio indica che alcuni oggetti "non possono ancora essere salvati in questo tipo di file", il file viene salvato, ma JeffDeck non è riuscito a scrivere quegli oggetti. Conserva anche il file originale. Quando salvi un file .odp, il messaggio elenca ciò che quel tipo di file non può contenere. La sezione "Salvare come file .odp" più avanti spiega cosa non può contenere.

## Salvare con un nuovo nome o tipo

Fai clic su **File > Salva con nome** oppure premi F12. Digita un nome, scegli il tipo e fai clic su **Salva**. Le opzioni sono:

- **Presentazione (*.pptx)** è la scelta normale.
- **Modello (*.potx)** salva le diapositive come modello da cui iniziare nuove presentazioni.
- **Presentazione automatica (*.ppsx)** salva una presentazione pronta per essere riprodotta.
- **Presentazione OpenDocument (*.odp)** salva le diapositive nel tipo di file usato da LibreOffice e da altri programmi gratuiti.
- **Modello di presentazione OpenDocument (*.otp)** è la versione modello di quel tipo di file.

Se ometti l'estensione, JeffDeck aggiunge quella corrispondente al tipo. Dopo il salvataggio, la barra del titolo mostra il nuovo nome e continui a lavorare nel nuovo file.

Una presentazione aperta da un modello viene salvata come nuova presentazione: **Salva** chiede un nome, in modo che il modello rimanga invariato. Lo stesso vale per una presentazione aperta da un file .ppt, .pps o .pot meno recente. Una presentazione aperta da un file .odp viene salvata di nuovo come file .odp, sovrascrivendo lo stesso file.

## Aprire

Premi CTRL+O per scegliere subito un file. In alternativa, fai clic su **File > Apri** e quindi su **Sfoglia** oppure su un nome in **Aggiunti** o **Presentazioni recenti**.

Il file si apre in questa finestra se contiene una nuova presentazione che nessuno ha ancora modificato. Altrimenti, si apre in una finestra separata.

Fai clic con il pulsante destro del mouse su un nome nell'elenco per scegliere **Aggiungi all'elenco** (il nome rimane in cima), **Rimuovi dagli elementi aggiunti** o **Rimuovi dall'elenco**. Un nome in grigio indica che JeffDeck non trova più quel file.

## Tipi di carattere nel file

Un tipo di carattere definisce la forma delle lettere. Non tutti i computer dispongono di tutti i tipi di carattere. Se in un computer manca un tipo di carattere usato nelle diapositive, le lettere vengono visualizzate con un altro tipo di carattere e le diapositive possono apparire diverse.

JeffDeck può inserire nel file i tipi di carattere usati nelle diapositive, in modo che appaiano uguali in qualsiasi computer. Questa operazione si chiama incorporamento. JeffDeck la esegue a ogni salvataggio. Per disattivarla, fai clic su **File > Opzioni**, apri la scheda **Salvataggio** e deseleziona **Incorpora tipi di carattere nel file**.

- JeffDeck incorpora solo i tipi di carattere forniti con il programma, ad esempio Fraunces, Lato e Open Sans, che gli altri computer non hanno. Non incorpora mai un tipo di carattere presente in tutti i computer, come Calibri o Arial. Esclude i tipi di carattere la cui licenza non ne consente la copia in un file.
- JeffDeck incorpora ogni stile usato nelle diapositive: normale, grassetto, corsivo e grassetto corsivo. Se le diapositive usano solo lo stile normale, viene incorporato solo quello.
- JeffDeck incorpora tutte le lettere di ogni tipo di carattere, non solo quelle usate, così chi apre il file può comunque modificare il testo.
- Il file diventa più grande. Ogni stile di un tipo di carattere aggiunge da 40 a 700 KB circa (un KB equivale a mille byte). Il file viene compresso durante il salvataggio, quindi aumenta di circa la metà.

Quando apri un file che contiene tipi di carattere incorporati, JeffDeck li usa finché la presentazione è aperta, e le diapositive appaiono come nel computer in cui sono state create. JeffDeck non installa i tipi di carattere nel computer e non li copia altrove. Appartengono solo a quella presentazione: non compaiono negli elenchi dei tipi di carattere e nessun'altra presentazione li usa. Vengono rimossi alla chiusura della presentazione. Se nel computer è già presente un tipo di carattere, JeffDeck usa la copia locale.

JeffDeck legge i tipi di carattere dai file .pptx creati da altri programmi di presentazione, anche quando il programma li ha compressi. Legge anche i tipi di carattere dai file .odp.

Un file può contenere un tipo di carattere che JeffDeck non ha. Al salvataggio, JeffDeck lo reinserisce nel file, purché le diapositive lo usino ancora, indipendentemente dal fatto che **Incorpora tipi di carattere nel file** sia attivato o meno. Se un tipo di carattere nel file è danneggiato o illeggibile, il messaggio visualizzato all'apertura del file lo segnala.

## Tipi di file letti da JeffDeck

- **.pptx, .potx e .ppsx** sono un unico formato in tre versioni: presentazione, modello e presentazione automatica. JeffDeck li legge e li scrive tutti e tre.
- **.ppt, .pps e .pot** sono il formato meno recente. JeffDeck li legge ma non può scriverli, quindi **Salva** chiede un nuovo nome e salva un file .pptx (un file .ppsx per un file .pps). Non sovrascrive mai l'originale.
- **.odp e .otp** sono presentazioni OpenDocument e i relativi modelli. JeffDeck li legge e li scrive entrambi. Un file .odp aperto qui viene salvato come .odp. Un file .otp è un modello, quindi **Salva** chiede un nuovo nome e propone un file .odp.

Se una parte di un file non può essere letta, JeffDeck apre il resto ed elenca ciò che ha omesso. Se l'intero file non può essere letto, ne indica il motivo.

## Salvare come file .odp

Un file .odp si apre in LibreOffice Impress con diapositive, master, immagini, note e commenti, e con l'aspetto visualizzato da JeffDeck. Contiene anche animazioni, transizioni delle diapositive, presentazioni personalizzate e impostazioni della presentazione. Il tipo di carattere di ogni parola viene salvato per nome, quindi un computer che non lo possiede ne mostra un altro.

JeffDeck non legge ancora da un file .odp animazioni, commenti, presentazioni personalizzate e impostazioni della presentazione. Questi elementi sono presenti nel file e LibreOffice li riproduce, ma quando apri il file .odp in JeffDeck vengono elencati come omessi. Conserva il file .pptx se vuoi continuare a modificarli qui.

Un file .odp non può contenere alcuni elementi. JeffDeck li elenca dopo il salvataggio, nel messaggio nella parte inferiore della finestra:

- **Grafici, diagrammi SmartArt ed equazioni** vengono salvati come immagini. Sono visibili, ma nel file .odp non è possibile modificarne i numeri o le parti.
- **Le sezioni** non vengono salvate.
- **Riflessi, smussature e ombreggiature interne** vengono omessi, come anche gli aloni e i riflessi sul testo. L'ombreggiatura del testo viene mantenuta.
- **Gli effetti di animazione** che iniziano al clic su un oggetto e gli effetti che cambiano un colore vengono omessi. Il clic su un video per metterlo in pausa o riprodurlo di nuovo viene mantenuto.
- **Un video o un audio collegato** a un altro file, e non contenuto nella presentazione, mantiene nel file .odp il proprio indirizzo e la propria immagine. LibreOffice lo riproduce se riesce a trovare il file. I video e gli audio contenuti nella presentazione vengono mantenuti nel file .odp e riprodotti.

Conserva anche il file .pptx, se ti servono di nuovo questi elementi.

## Elementi conservati ma non visualizzati

Alcune parti di un file non possono ancora essere visualizzate o modificate in JeffDeck. Il programma le conserva con le relative diapositive e le riscrive al salvataggio, in modo che non vadano perse. Tra queste ci sono le animazioni, che vengono riprodotte nella presentazione, i video e gli audio, che non vengono ancora riprodotti, e gli effetti di transizione non supportati da JeffDeck, che vengono riprodotti come dissolvenza. Grafici e diagrammi SmartArt vengono visualizzati come sono stati salvati. Vedi [Benvenuti in JeffDeck](start).

## Argomenti correlati

- [Recuperare il lavoro non salvato](recover)
- [La schermata File](file-screen)
- [Riutilizzare diapositive da un'altra presentazione](reuse)
- [Stampa, PDF e immagini delle diapositive](printing)
