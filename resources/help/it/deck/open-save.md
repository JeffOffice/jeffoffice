# Aprire e salvare presentazioni
<!-- keywords: aprire, apri, salvare, salva, salva con nome, chiudere, nuovo, nuova presentazione, file recenti, aggiunti, tipi di file, pptx, potx, ppsx, ppt, odp, modello, presentazione automatica, rinominare, CTRL+S, backup, copia di sicurezza, formati di file, file vecchi, open document, conservato, non visualizzato -->

JeffDeck salva il lavoro in file .pptx e apre diversi tipi di file di presentazione.

## Iniziare una nuova presentazione

Premere CTRL+N. Si apre una nuova finestra con una presentazione vuota. In alternativa, fare clic su **File > Nuovo** e quindi su **Presentazione vuota**. Se la finestra corrente contiene una nuova presentazione che nessuno ha ancora modificato, la presentazione vuota la sostituisce. Altrimenti, si apre in una finestra separata.

## Salvare

Premere CTRL+S oppure fare clic su **File > Salva**. La prima volta, JeffDeck chiede un nome e una posizione. In seguito, Salva sostituisce il file con le ultime modifiche. Un messaggio nella parte inferiore indica "Salvato" e il nome del file.

Un asterisco (*) dopo il nome nella barra del titolo indica che ci sono modifiche non salvate. Salvare spesso. Se JeffDeck si chiude inaspettatamente, vedere [Recuperare il lavoro non salvato](recover).

Se un messaggio indica che alcuni oggetti "non possono ancora essere salvati in questo tipo di file", il file viene salvato, ma JeffDeck non è stato in grado di scrivere quegli oggetti. Conservare anche il file originale.

## Salvare con un nuovo nome o un nuovo tipo

Fare clic su **File > Salva con nome** oppure premere F12. Digitare un nome, scegliere il tipo e fare clic su **Salva**. Le opzioni disponibili sono:

- **Presentazione (*.pptx)** è la scelta normale.
- **Modello (*.potx)** salva le diapositive come modello da cui iniziare nuove presentazioni.
- **Presentazione automatica (*.ppsx)** salva una presentazione da riprodurre.

Se non si specifica l'estensione, JeffDeck aggiunge quella corrispondente al tipo scelto. Dopo il salvataggio, la barra del titolo mostra il nuovo nome e si continua a lavorare nel nuovo file.

Una presentazione aperta da un modello viene salvata come nuova presentazione: **Salva** chiede un nome, in modo che il modello rimanga invariato. Lo stesso vale per una presentazione aperta da un file .ppt, .pps o .pot meno recente, oppure da un file .odp o .otp.

## Aprire

Premere CTRL+O per scegliere subito un file. In alternativa, fare clic su **File > Apri** e quindi su **Sfoglia** oppure su un nome in **Aggiunti** o **Presentazioni recenti**.

Il file si apre nella finestra corrente se questa contiene una nuova presentazione che nessuno ha ancora modificato. Altrimenti, si apre in una finestra separata.

Fare clic con il pulsante destro del mouse su un nome nell'elenco per scegliere **Aggiungi all'elenco** (il nome rimane in cima), **Rimuovi dagli elementi aggiunti** o **Rimuovi dall'elenco**. Un nome in grigio indica che JeffDeck non riesce più a trovare il file.

## Tipi di file letti da JeffDeck

- **.pptx, .potx e .ppsx** sono un unico formato in tre forme: presentazione, modello e presentazione automatica. JeffDeck legge e scrive tutti e tre.
- **.ppt, .pps e .pot** sono il formato meno recente. JeffDeck li legge ma non li scrive, quindi **Salva** chiede un nuovo nome e salva un file .pptx (un file .ppsx per un .pps). Il file originale non viene mai sovrascritto.
- **.odp e .otp** sono presentazioni Open Document e i relativi modelli. JeffDeck li legge e li salva come nuovo file .pptx. Il file originale non viene mai sovrascritto.

Se una parte di un file non può essere letta, JeffDeck apre il resto ed elenca ciò che ha escluso. Se non è possibile leggere l'intero file, ne indica il motivo.

## Elementi conservati ma non visualizzati

Alcune parti di un file sono elementi che JeffDeck non è ancora in grado di visualizzare o modificare. JeffDeck li conserva insieme alle relative diapositive e li riscrive al salvataggio, in modo che non vadano persi. Tra questi ci sono le animazioni, che vengono riprodotte durante la presentazione, i filmati e i suoni, che non vengono ancora riprodotti, e gli effetti di transizione non supportati da JeffDeck, che vengono riprodotti come dissolvenza. I grafici e i diagrammi SmartArt vengono visualizzati così come sono stati salvati. Vedere [Benvenuti in JeffDeck](start).

## Argomenti correlati

- [Recuperare il lavoro non salvato](recover)
- [La schermata File](file-screen)
- [Riutilizzare diapositive di un'altra presentazione](reuse)
- [Stampa, PDF e immagini delle diapositive](printing)
