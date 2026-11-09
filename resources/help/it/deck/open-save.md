# Aprire e salvare presentazioni
<!-- keywords: apri, salva, salva con nome, otp, libreoffice, lasciati fuori, chiudi, nuovo, file recenti, aggiunti, tipi di file, pptx, potx, ppsx, ppt, odp, modello, presentazione, rinomina, Ctrl+S, backup, copia di backup, formati di file, file vecchi, documento aperto, mantenuto, non mostrato, carattere, tipi di carattere, incorpora caratteri, incorpora tipi di carattere, carattere mancante, salva una copia, sola lettura, apri come copia, nuova finestra, salva come modello, modelli personali, ripara, apri e ripara, danneggiato, password, crittografa -->

JeffDeck salva il tuo lavoro in file .pptx e in file .odp, e apre diversi tipi di file di presentazione.

## Avviare una nuova presentazione

Premi Ctrl+N. Si apre una nuova finestra con una presentazione vuota. In alternativa, fai clic su **File > Nuovo** e poi su **Presentazione vuota**. Se la finestra in cui ti trovi contiene una nuova presentazione che nessuno ha modificato, la presentazione vuota la sostituisce. Altrimenti, si apre in una finestra a sé.

## Salvare

Premi Ctrl+S oppure fai clic su **File > Salva**. La prima volta, JeffDeck chiede un nome e una posizione. Successivamente, Salva sostituisce il file con la versione più recente del tuo lavoro. In basso compare un messaggio che dice "Salvato" e il nome del file.

Un asterisco (*) dopo il nome nella barra del titolo indica che ci sono modifiche non salvate. Salva spesso. Se JeffDeck si blocca all'improvviso, consulta [Recuperare il lavoro non salvato](recover).

Se un messaggio dice che alcuni oggetti "non possono essere salvati in questo tipo di file", il file viene salvato, ma JeffDeck non ha modo di scrivere quegli oggetti. Conserva anche il file originale. Quando salvi un file .odp, il messaggio elenca ciò che quel tipo di file non può contenere. La sezione "Salvare come file .odp" più sotto spiega cosa non può contenere.

## Salvare con un nuovo nome o tipo

Fai clic su **File > Salva con nome**, oppure premi F12. Digita un nome, scegli il tipo e fai clic su **Salva**. Le scelte sono:

- **Presentazione (*.pptx)** è la scelta normale.
- **Modello (*.potx)** salva le diapositive come schema da cui iniziare nuove presentazioni.
- **Presentazione automatica (*.ppsx)** salva una presentazione da riprodurre.
- **Presentazione OpenDocument (*.odp)** salva le diapositive nel tipo di file usato da LibreOffice e da altri programmi gratuiti.
- **Modello di presentazione OpenDocument (*.otp)** è la forma di modello di questo tipo.

Se ometti l'estensione, JeffDeck aggiunge quella corrispondente al tipo scelto. Dopo un salvataggio, la barra del titolo mostra il nuovo nome e continui a lavorare nel nuovo file.

Una presentazione aperta da un modello viene salvata come nuova presentazione: **Salva** chiede un nome, così il modello resta com'era. Lo stesso vale per una presentazione aperta da un file .ppt, .pps o .pot meno recente. Una presentazione aperta da un file .odp viene salvata di nuovo come file .odp, sullo stesso file.

## Salvare una copia

Fai clic su **File > Salva una copia** e scegli un nome. JeffDeck scrive la presentazione in quel file, e tu continui a lavorare in quella che avevi aperto. Il suo nome e le sue modifiche non salvate restano come erano.

## Conservare una copia di backup

Fai clic su **File > Opzioni**, e nella scheda **Generale** attiva **Crea sempre una copia di backup**. Ogni volta che salvi, JeffDeck copia prima il file precedente in un file chiamato "Backup di" seguito dal nome del file, nella stessa cartella. Se la copia non può essere creata, l'app chiede se salvare senza di essa. La risposta predefinita è No: in questo caso il file resta com'era. Se rispondi Sì, il salvataggio avviene senza copia di backup.

## Salvare come modello

Un modello è una presentazione che usi come punto di partenza. Fai clic su **File > Salva come modello** e digita un nome. Il modello (un file **.potx**) viene collocato in **Modelli personali**, e la tua presentazione non viene modificata. Per avviare una nuova presentazione da questo modello, fai clic su **File > Nuovo**, quindi fai clic sul modello in **Modelli personali**. La nuova presentazione non ha ancora un file proprio, quindi il modello non viene modificato. Per rimuovere un modello dall'elenco, fai clic con il pulsante destro sul suo nome e scegli **Rimuovi da Modelli personali**.

## Aprire

Premi Ctrl+O per scegliere subito un file. Oppure fai clic su **File > Apri**, e poi su **Sfoglia**, oppure su un nome in **Aggiunti** o **Presentazioni recenti**.

Il file si apre in questa finestra se contiene una nuova presentazione che nessuno ha modificato. Altrimenti, si apre in una finestra a sé.

Fai clic con il pulsante destro su un nome nell'elenco per **Aggiungi all'elenco** (resta in cima), **Rimuovi dagli elementi aggiunti** o **Rimuovi dall'elenco**. Un nome in grigio indica che JeffDeck non riesce più a trovare quel file.

## Altri modi per aprire un file

Nella pagina **Apri**, fai clic su **Altri modi per aprire**. Puoi anche fare clic con il pulsante destro su un nome nell'elenco dei recenti.

- **Apri in sola lettura** apre la presentazione solo per la lettura. Puoi guardarla, scorrere le diapositive e copiare, ma non puoi modificare nulla. La barra del titolo riporta "Sola lettura". Se premi **Salva**, JeffDeck chiede un nuovo nome, così il file aperto non viene modificato.
- **Apri come copia** apre le diapositive come nuova presentazione senza un file proprio. **Salva** chiede dove collocarla.
- **Apri in una nuova finestra** apre il file in una finestra a sé, anche quando la finestra in cui ti trovi è vuota.
- **Apri e ripara** serve per un file .pptx danneggiato, per esempio uno troncato durante la copia. JeffDeck legge tutte le parti che riesce a leggere, conserva una parte interrotta fino al punto in cui si è fermata ed esclude una parte che non riesce a leggere. Le diapositive riparate si aprono come copia chiamata "Riparata" seguito dal nome, e un messaggio indica cosa è stato escluso. Il file danneggiato non viene modificato.

## File protetti da password

Un file **.pptx** che chiede una password si apre normalmente: JeffDeck chiede la password, e puoi riprovare se non è corretta, oppure fare clic su **Annulla**. Il file resta protetto quando lo salvi. Per cambiare o rimuovere la password, usa **File > Informazioni > Cifra con password** (vedi La schermata File). Un file **.odp** non può ancora contenere una password.

## Caratteri nel file

Un carattere è la forma delle lettere. Non tutti i computer hanno tutti i caratteri. Se a un computer manca un carattere usato dalle tue diapositive, disegna le lettere con un altro carattere. Le diapositive possono quindi apparire diverse.

JeffDeck può inserire nel file i caratteri usati dalle diapositive. In questo modo le diapositive appaiono uguali su qualsiasi computer. Questa operazione si chiama incorporamento. JeffDeck la esegue a ogni salvataggio. Per disattivarla, fai clic su **File > Opzioni**, apri la scheda **Salvataggio** e deseleziona **Incorpora tipi di carattere nel file**.

- JeffDeck inserisce solo i caratteri che arrivano con il programma, come Fraunces, Lato e Open Sans. Gli altri computer non li hanno. Non inserisce mai un carattere che tutti i computer hanno, come Calibri o Arial. Esclude un carattere la cui licenza dice che non può essere copiato in un file.
- JeffDeck inserisce ogni stile usato dalle diapositive: normale, grassetto, corsivo e grassetto corsivo. Se le diapositive usano solo lo stile normale, viene inserito solo quello.
- JeffDeck inserisce ogni lettera di ciascun carattere, non solo quelle usate. Così chi apre il file può ancora modificare il testo.
- Il file diventa più grande. Ogni stile di un carattere aggiunge circa da 40 a 700 KB (un KB è mille byte). Il file viene compresso al salvataggio, quindi cresce di circa la metà di quella quantità.

Quando apri un file che contiene caratteri, JeffDeck li usa mentre la presentazione è aperta. Le diapositive appaiono come apparivano dove sono state create. JeffDeck non installa i caratteri sul tuo computer e non li copia da nessuna parte. Appartengono solo a quella presentazione: non compaiono negli elenchi dei tuoi caratteri e nessun'altra presentazione li usa. Spariscono quando chiudi la presentazione. Se il tuo computer ha già un carattere, JeffDeck usa la tua copia.

JeffDeck legge i caratteri dai file .pptx creati da altri programmi di presentazione, anche quando il programma li ha compressi. Legge anche i caratteri dai file .odp.

Un file può contenere un carattere che JeffDeck non ha. Quando salvi, JeffDeck rimette quel carattere nel file, purché le diapositive lo usino ancora. Lo fa che l'opzione **Incorpora tipi di carattere nel file** sia attiva o no. Se un carattere del file è danneggiato o non può essere letto, il messaggio che vedi all'apertura del file lo indica.

## Tipi di file letti da JeffDeck

- **.pptx, .potx e .ppsx** sono un unico formato in tre forme: presentazione, modello e presentazione automatica. JeffDeck legge e scrive tutte e tre.
- **.ppt, .pps e .pot** sono il tipo più vecchio. JeffDeck li legge ma non può scriverli, quindi **Salva** chiede un nuovo nome e salva un .pptx (un .ppsx per un .pps). Non sovrascrive mai l'originale.
- **.odp e .otp** sono presentazioni in formato OpenDocument e i relativi modelli. JeffDeck legge e scrive entrambi. Un .odp aperto qui viene salvato come .odp. Un .otp è un modello, quindi **Salva** chiede un nuovo nome e propone un .odp.

Se una parte del file non può essere letta, JeffDeck apre il resto ed elenca ciò che ha escluso. Se l'intero file non può essere letto, indica il motivo.

## Salvare come file .odp

Un file .odp si apre in LibreOffice Impress con le tue diapositive, i master, le immagini, le note e i commenti, e con l'aspetto che JeffDeck mostra. Contiene animazioni, transizioni tra diapositive, presentazioni personalizzate e le impostazioni della presentazione. Il carattere di ogni parola viene salvato con il nome, quindi un computer che non ha il carattere mostra un altro carattere.

JeffDeck non legge ancora le animazioni, i commenti, le presentazioni personalizzate né le impostazioni della presentazione da un file .odp. Sono nel file e LibreOffice li riproduce, ma quando apri il file .odp in JeffDeck, li elenca come esclusi. Conserva il file .pptx se vuoi continuare a modificarli qui.

Un file .odp non ha spazio per alcune cose. Dopo il salvataggio, JeffDeck le elenca nel messaggio in basso nella finestra:

- **Grafici, diagrammi e equazioni** vengono salvati come immagini. Puoi vederli, ma non puoi modificarne i numeri o le parti nel file .odp.
- **Le sezioni** non vengono salvate.
- **Smussature e ombre interne** vengono escluse, così come bagliori e riflessi sul testo. L'ombra sul testo viene mantenuta. Un riflesso su una forma, una linea con sfumatura, una linea a due o tre tratti e la dimensione di un'ombra vengono mantenuti in un modo che solo JeffDeck legge. Altri programmi mostrano la forma senza il riflesso, la linea nel suo primo colore, un tratto singolo e l'ombra alla dimensione dell'oggetto.
- **Effetti di animazione** che iniziano con un clic su un oggetto ed effetti che cambiano un colore vengono esclusi. Un clic su un video, per metterlo in pausa o riprodurlo di nuovo, viene mantenuto.
- **Un video o un suono collegato** da un altro file, non incorporato nella presentazione, mantiene il suo indirizzo e la sua immagine nel file .odp. LibreOffice lo riproduce se riesce a trovare il file. Video e suoni incorporati nella presentazione vengono mantenuti nel file .odp e vengono riprodotti.

Conserva anche il file .pptx, se ti servono di nuovo queste cose.

## Ciò che viene mantenuto ma non mostrato

Alcune parti di un file sono elementi che JeffDeck non può ancora mostrare o modificare. Li conserva insieme alle diapositive e li riscrive al salvataggio, così non vanno persi. Tra questi ci sono le animazioni, che vengono riprodotte nella presentazione, i video e i suoni, che per ora non vengono riprodotti, e un effetto di transizione per cui JeffDeck non ha una forma, che viene riprodotto come dissolvenza. Grafici e diagrammi appaiono come erano stati salvati. Vedi [Benvenuti in JeffDeck](start).

## Argomenti correlati

- [Recuperare il lavoro non salvato](recover)
- [La schermata File](file-screen)
- [Riutilizzare diapositive da un'altra presentazione](reuse)
- [Stampa, PDF e immagini delle diapositive](printing)
