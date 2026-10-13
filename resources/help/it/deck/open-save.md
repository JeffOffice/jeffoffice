# Aprire e salvare presentazioni
<!-- keywords: aprire, salvare, salva con nome, otp, libreoffice, omesso, escluso, chiudere, nuovo, nuova presentazione, file recenti, aggiunti, tipi di file, formati, pptx, potx, ppsx, ppt, odp, modello, presentazione automatica, rinominare, CTRL+S, backup, copia, formati di file, file vecchi, open document, conservato, non visualizzato, tipi di carattere, font, incorpora tipi di carattere, caratteri incorporati, carattere mancante -->

JeffDeck salva il lavoro in file .pptx e in file .odp, e apre diversi tipi di file di presentazione.

## Iniziare una nuova presentazione

Premere CTRL+N. Si apre una nuova finestra con una presentazione vuota. In alternativa, fare clic su **File > Nuovo** e quindi su **Presentazione vuota**. Se la finestra corrente contiene una nuova presentazione non ancora modificata, la presentazione vuota la sostituisce. Altrimenti, si apre in una finestra separata.

## Salvare

Premere CTRL+S oppure fare clic su **File > Salva**. La prima volta JeffDeck chiede un nome e una posizione. In seguito, Salva sostituisce il file con il lavoro più recente. Un messaggio nella parte inferiore indica "Salvato" e il nome del file.

Un asterisco (*) dopo il nome nella barra del titolo indica che ci sono modifiche non salvate. Salvare spesso. Se JeffDeck si chiude in modo imprevisto, vedere [Recuperare il lavoro non salvato](recover).

Se un messaggio indica che alcuni oggetti "non possono ancora essere salvati in questo tipo di file", il file viene salvato, ma JeffDeck non è stato in grado di scrivere quegli oggetti. Conservare anche il file originale. Quando si salva un file .odp, il messaggio elenca gli elementi che questo tipo di file non può contenere. La sezione "Salvare come file .odp" più avanti spiega quali sono.

## Salvare con un nuovo nome o tipo

Fare clic su **File > Salva con nome** oppure premere F12. Digitare un nome, scegliere il tipo e fare clic su **Salva**. Le opzioni sono:

- **Presentazione (*.pptx)** è la scelta normale.
- **Modello (*.potx)** salva le diapositive come modello da cui iniziare nuove presentazioni.
- **Presentazione automatica (*.ppsx)** salva una presentazione da riprodurre.
- **Presentazione OpenDocument (*.odp)** salva le diapositive nel tipo di file usato da LibreOffice e da altri programmi gratuiti.
- **Modello di presentazione OpenDocument (*.otp)** è la versione modello di questo tipo di file.

Se si omette l'estensione, JeffDeck aggiunge quella corrispondente al tipo. Dopo il salvataggio, la barra del titolo mostra il nuovo nome e si continua a lavorare nel nuovo file.

Una presentazione aperta da un modello viene salvata come nuova presentazione: **Salva** chiede un nome, così il modello resta invariato. Lo stesso vale per una presentazione aperta da un file .ppt, .pps o .pot di versioni precedenti. Una presentazione aperta da un file .odp viene salvata di nuovo come file .odp, sovrascrivendo lo stesso file.

## Aprire

Premere CTRL+O per scegliere subito un file. In alternativa, fare clic su **File > Apri** e quindi su **Sfoglia** o su un nome in **Aggiunti** o **Presentazioni recenti**.

Il file si apre nella finestra corrente se questa contiene una nuova presentazione non ancora modificata. Altrimenti, si apre in una finestra separata.

Fare clic con il pulsante destro del mouse su un nome nell'elenco per scegliere **Aggiungi all'elenco** (il nome resta in cima), **Rimuovi dall'elenco degli elementi aggiunti** o **Rimuovi dall'elenco**. Un nome in grigio indica che JeffDeck non riesce più a trovare il file.

## Tipi di carattere nel file

Un tipo di carattere definisce la forma delle lettere. Non tutti i computer hanno tutti i tipi di carattere. Se in un computer manca un tipo di carattere usato nelle diapositive, le lettere vengono visualizzate con un altro tipo di carattere e le diapositive possono avere un aspetto diverso.

JeffDeck può inserire nel file i tipi di carattere usati nelle diapositive, in modo che abbiano lo stesso aspetto in qualsiasi computer. Questa operazione si chiama incorporamento. JeffDeck la esegue a ogni salvataggio. Per disattivarla, fare clic su **File > Opzioni**, aprire la scheda **Salvataggio** e deselezionare **Incorpora tipi di carattere nel file**.

- JeffDeck incorpora solo i tipi di carattere forniti con il programma, come Fraunces, Lato e Open Sans, che gli altri computer non hanno. Non incorpora mai un tipo di carattere presente in tutti i computer, come Calibri o Arial. Esclude i tipi di carattere la cui licenza non ne consente la copia in un file.
- JeffDeck incorpora ogni stile usato nelle diapositive: normale, grassetto, corsivo e grassetto corsivo. Se le diapositive usano solo lo stile normale, viene incorporato solo quello.
- JeffDeck incorpora tutte le lettere di ogni tipo di carattere, non solo quelle usate. In questo modo chi apre il file può comunque modificare il testo.
- Il file diventa più grande. Ogni stile di un tipo di carattere aggiunge da 40 a 700 KB circa (un KB equivale a mille byte). Il file viene compresso al salvataggio, quindi aumenta di circa la metà.

Quando si apre un file che contiene tipi di carattere incorporati, JeffDeck li usa finché la presentazione è aperta, e le diapositive hanno lo stesso aspetto che avevano nel computer in cui sono state create. JeffDeck non installa i tipi di carattere nel computer e non li copia altrove. Appartengono solo a quella presentazione: non compaiono negli elenchi dei tipi di carattere e nessun'altra presentazione li usa. Scompaiono quando si chiude la presentazione. Se il computer dispone già di un tipo di carattere, JeffDeck usa la copia installata.

JeffDeck legge i tipi di carattere dai file .pptx creati da altri programmi di presentazione, anche se il programma li ha compressi. Legge anche i tipi di carattere dai file .odp.

Un file può contenere un tipo di carattere che JeffDeck non ha. Al salvataggio, JeffDeck reinserisce quel tipo di carattere nel file, purché le diapositive lo usino ancora, indipendentemente dall'impostazione **Incorpora tipi di carattere nel file**. Se un tipo di carattere in un file è danneggiato o non può essere letto, lo segnala il messaggio visualizzato all'apertura del file.

## Tipi di file letti da JeffDeck

- **.pptx, .potx e .ppsx** sono un unico formato in tre forme: presentazione, modello e presentazione automatica. JeffDeck li legge e li scrive tutti e tre.
- **.ppt, .pps e .pot** sono i formati precedenti. JeffDeck li legge ma non può scriverli, quindi **Salva** chiede un nuovo nome e salva un file .pptx (un file .ppsx per un file .pps). Non sovrascrive mai l'originale.
- **.odp e .otp** sono presentazioni OpenDocument e i relativi modelli. JeffDeck li legge e li scrive entrambi. Un file .odp aperto in JeffDeck viene salvato come .odp. Un file .otp è un modello, quindi **Salva** chiede un nuovo nome e propone un file .odp.

Se una parte di un file non può essere letta, JeffDeck apre il resto ed elenca gli elementi esclusi. Se non è possibile leggere l'intero file, ne indica il motivo.

## Salvare come file .odp

Un file .odp si apre in LibreOffice Impress con diapositive, schemi, immagini, note e commenti, e con l'aspetto visualizzato in JeffDeck. Contiene anche animazioni, transizioni, presentazioni personalizzate e impostazioni della presentazione. Il tipo di carattere di ogni parola viene salvato per nome, quindi un computer che non dispone di quel tipo di carattere ne visualizza un altro.

JeffDeck non è ancora in grado di rileggere da un file .odp animazioni, commenti, presentazioni personalizzate e impostazioni della presentazione. Questi elementi sono presenti nel file e LibreOffice li riproduce, ma quando si apre il file .odp in JeffDeck vengono elencati come esclusi. Conservare il file .pptx se si vuole continuare a modificarli in JeffDeck.

Un file .odp non può contenere alcuni elementi. JeffDeck li elenca dopo il salvataggio, nel messaggio nella parte inferiore della finestra:

- **Grafici, diagrammi SmartArt ed equazioni** vengono salvati come immagini. Sono visibili, ma nel file .odp non è possibile modificarne i valori o le parti.
- **Sezioni**: non vengono salvate.
- **Riflessi, smussature e ombreggiature interne** vengono omessi.
- **Effetti di animazione** che iniziano al clic su un oggetto e gli effetti che cambiano un colore vengono omessi. Il clic su un video per sospenderlo o riprodurlo di nuovo viene invece conservato.
- **Un video o un audio collegato** da un altro file, anziché contenuto nella presentazione, conserva nel file .odp il percorso e l'immagine. LibreOffice lo riproduce se riesce a trovare il file. I video e gli audio contenuti nella presentazione vengono conservati nel file .odp e vengono riprodotti.

Se questi elementi servono, conservare anche il file .pptx.

## Elementi conservati ma non visualizzati

Alcune parti di un file non possono ancora essere visualizzate o modificate da JeffDeck. Il programma le conserva con le relative diapositive e le riscrive al salvataggio, in modo che non vadano perse. Tra queste ci sono le animazioni, che vengono riprodotte durante la presentazione, i video e gli audio, che non vengono ancora riprodotti, e gli effetti di transizione non supportati da JeffDeck, che vengono riprodotti come dissolvenza. Grafici e diagrammi SmartArt vengono visualizzati così come sono stati salvati. Vedere [Introduzione a JeffDeck](start).

## Argomenti correlati

- [Recuperare il lavoro non salvato](recover)
- [La schermata File](file-screen)
- [Riutilizzare diapositive di un'altra presentazione](reuse)
- [Stampa, PDF e immagini delle diapositive](printing)
