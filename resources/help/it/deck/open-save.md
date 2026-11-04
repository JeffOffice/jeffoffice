# Apri e salva presentazioni

<!-- keywords: apri, salva, salva con nome, otp, libreoffice, escludi, chiudi, nuovo, file recenti, aggiunti, tipi di file, pptx, potx, ppsx, ppt, odp, modello, presentazione, rinomina, Ctrl+S, backup, copia di backup, formati di file, file vecchi, documento OpenDocument, mantenuto, non visualizzato, carattere, caratteri, incorpora tipi di carattere, tipo di carattere mancante, salva una copia, sola lettura, apri come copia, nuova finestra, copia di backup, modelli personali, salva come modello, ripara, apri e ripara, danneggiato -->

JeffDeck salva il tuo lavoro in file .pptx e in file .odp, e apre diversi tipi di file di presentazione.

## Avviare una nuova presentazione

Premi Ctrl+N. Si apre una nuova finestra con una presentazione vuota. Oppure fai clic su **File > Nuovo** e poi su **Presentazione vuota**. Se la finestra in cui ti trovi contiene una nuova presentazione che nessuno ha modificato, la presentazione vuota la sostituisce. Altrimenti si apre in una finestra propria.

## Salvare

Premi Ctrl+S oppure fai clic su **File > Salva**. La prima volta, JeffDeck chiede un nome e una posizione. Dopo, Salva sostituisce il file con il lavoro più recente. In basso compare un messaggio che dice "Salvato" e il nome del file.

Un asterisco (*) dopo il nome nella barra del titolo indica che ci sono modifiche non salvate. Salva spesso. Se JeffDeck si interrompe all'improvviso, consulta [Recupera lavoro non salvato](recover).

Se un messaggio dice che alcuni oggetti "non possono essere salvati in questo tipo di file", il file viene salvato, ma JeffDeck non aveva modo di scrivere quegli oggetti. Conserva anche il file originale. Quando salvi un file .odp, il messaggio elenca ciò che quel tipo di file non può contenere. La sezione "Salvare come file .odp" più avanti spiega che cosa non può contenere.

## Salvare con un nuovo nome o tipo

Fai clic su **File > Salva con nome**, oppure premi F12. Digita un nome, scegli il tipo e fai clic su **Salva**. Le scelte sono:

- **Presentazione (*.pptx)** è la scelta normale.
- **Modello (*.potx)** salva le diapositive come schema da cui iniziare nuove presentazioni.
- **Presentazione automatica (*.ppsx)** salva una presentazione da riprodurre.
- **Presentazione OpenDocument (*.odp)** salva le diapositive nel tipo di file usato da LibreOffice e da altri programmi gratuiti.
- **Modello di presentazione OpenDocument (*.otp)** è la forma modello di questo tipo.

Se ometti l'estensione, JeffDeck aggiunge quella corrispondente al tipo scelto. Dopo il salvataggio, la barra del titolo mostra il nuovo nome e continui a lavorare nel nuovo file.

Una presentazione aperta da un modello viene salvata come nuova presentazione: **Salva** chiede un nome, così il modello resta com'era. Lo stesso vale per una presentazione aperta da un file .ppt, .pps o .pot meno recente. Una presentazione aperta da un file .odp viene salvata di nuovo come file .odp, nello stesso file.

## Salvare una copia

Fai clic su **File > Salva una copia** e scegli un nome. JeffDeck scrive la presentazione in quel file e tu continui a lavorare in quella che avevi aperto. Il suo nome e le modifiche non salvate restano come erano.

## Conservare una copia di backup

Fai clic su **File > Opzioni**, e nella scheda **Generale** attiva **Crea sempre una copia di backup**. Ogni volta che salvi, JeffDeck copia prima il file precedente in un file chiamato "Backup di" seguito dal nome del file, nella stessa cartella. Se la copia non può essere creata, l'app chiede se salvare senza di essa. La risposta predefinita è No, a meno che tu non risponda Sì, e il file resta com'era.

## Salvare come modello

Un modello è una presentazione che usi come punto di partenza. Fai clic su **File > Salva come modello** e digita un nome. Il modello (un file **.potx**) viene messo in **Modelli personali**, e la tua presentazione non viene modificata. Per avviare una nuova presentazione dal modello, fai clic su **File > Nuovo**, e poi sul modello in **Modelli personali**. La nuova presentazione non ha ancora un file proprio, quindi il modello non viene modificato. Per rimuovere un modello dall'elenco, fai clic con il pulsante destro su di esso e scegli **Rimuovi da Modelli personali**.

## Aprire

Premi Ctrl+O per scegliere subito un file. Oppure fai clic su **File > Apri**, e poi su **Sfoglia**, oppure su un nome in **Aggiunti** o **Presentazioni recenti**.

Il file si apre in questa finestra se contiene una nuova presentazione che nessuno ha modificato. Altrimenti si apre in una finestra propria.

Fai clic con il pulsante destro su un nome nell'elenco per **Aggiungi all'elenco** (resta in cima), **Rimuovi dagli elementi aggiunti** oppure **Rimuovi dall'elenco**. Un nome in grigio indica che JeffDeck non riesce a trovare il file in questo momento.

## Altri modi per aprire un file

Nella pagina **Apri**, fai clic su **Altri modi per aprire**. Puoi anche fare clic con il pulsante destro su un nome nell'elenco dei recenti.

- **Apri in sola lettura** apre la presentazione solo per la lettura. Puoi guardarla, scorrere le diapositive e copiare, ma non puoi modificare nulla. La barra del titolo mostra "Sola lettura". Se premi **Salva**, JeffDeck chiede un nuovo nome, così il file aperto non viene modificato.
- **Apri come copia** apre le diapositive come nuova presentazione senza un file proprio. **Salva** chiede dove metterla.
- **Apri in una nuova finestra** apre il file in una finestra propria, anche quando la finestra in cui ti trovi è vuota.
- **Apri e ripara** serve per un file .pptx danneggiato, per esempio uno che è stato troncato durante la copia. JeffDeck legge tutte le parti che riesce a leggere, conserva una parte interrotta fino al punto in cui si è fermata ed esclude una parte che non riesce a leggere. Le diapositive riparate si aprono come copia chiamata "Riparata" seguito dal nome, e un messaggio ti dice che cosa è stato escluso. Il file danneggiato non viene modificato.

## Caratteri nel file

Un carattere è la forma delle lettere. Non tutti i computer hanno tutti i caratteri. Se a un computer manca un carattere usato dalle diapositive, le lettere vengono disegnate con un altro carattere. Le diapositive possono quindi apparire diverse.

JeffDeck può inserire nel file i caratteri usati dalle diapositive. Così le diapositive hanno lo stesso aspetto su qualsiasi computer. Questa operazione si chiama incorporamento. JeffDeck la esegue a ogni salvataggio. Per disattivarla, fai clic su **File > Opzioni**, apri la scheda **Salva** e deseleziona **Incorpora tipi di carattere nel file**.

- JeffDeck inserisce solo i caratteri che arrivano con il programma, come Fraunces, Lato e Open Sans. Gli altri computer non li hanno. Non inserisce mai un carattere presente su ogni computer, come Calibri o Arial. Esclude un carattere la cui licenza non ne consente la copia nel file.
- JeffDeck inserisce ogni stile usato dalle diapositive: normale, grassetto, corsivo e grassetto corsivo. Se le diapositive usano solo lo stile normale, viene inserito solo quello.
- JeffDeck inserisce ogni lettera di ciascun carattere, non solo quelle che hai usato. Così chi apre il file può comunque modificare le parole.
- Il file diventa più grande. Ogni stile di un carattere aggiunge da circa 40 a 700 KB (un KB è mille byte). Il file viene compresso al salvataggio, quindi cresce di circa la metà di questa cifra.

Quando apri un file che contiene caratteri incorporati, JeffDeck li usa mentre la presentazione è aperta. Le diapositive appaiono come erano state create. JeffDeck non installa i caratteri sul computer e non li copia da nessuna parte. Appartengono solo a quella presentazione: non compaiono negli elenchi dei caratteri e nessun'altra presentazione li usa. Vengono rimossi quando chiudi la presentazione. Se il computer ha già un carattere, JeffDeck usa la sua copia.

JeffDeck legge i caratteri dai file .pptx creati da altri programmi di presentazione, anche quando il programma li ha compressi. Legge anche i caratteri dai file .odp.

Un file può contenere un carattere che JeffDeck non possiede. Al salvataggio, JeffDeck rimette quel carattere nel file, purché le diapositive lo usino ancora. Lo fa indipendentemente dal fatto che **Incorpora tipi di carattere nel file** sia attivo o no. Se un carattere del file è danneggiato o non può essere letto, il messaggio che vedi all'apertura del file lo indica.

## Tipi di file che JeffDeck legge

- **.pptx, .potx e .ppsx** sono un unico formato in tre forme: presentazione, modello e presentazione automatica. JeffDeck legge e scrive tutte e tre.
- **.ppt, .pps e .pot** sono il tipo meno recente. JeffDeck li legge ma non può scriverli, quindi **Salva** chiede un nuovo nome e salva un .pptx (un .ppsx per un .pps). Non sovrascrive mai l'originale.
- **.odp e .otp** sono le presentazioni in formato OpenDocument e i relativi modelli. JeffDeck legge e scrive entrambi. Un .odp aperto qui viene salvato come .odp. Un .otp è un modello, quindi **Salva** chiede un nuovo nome e propone un .odp.

Se una parte del file non può essere letta, JeffDeck apre il resto ed elenca ciò che ha escluso. Se l'intero file non può essere letto, dice il motivo.

## Salvare come file .odp

Un file .odp si apre in LibreOffice Impress con le diapositive, le pagine master, le immagini, le note e i commenti, e con l'aspetto che JeffDeck mostra. Contiene anche le animazioni, le transizioni tra diapositive, le presentazioni personalizzate e le impostazioni della presentazione. Il carattere di ogni parola viene salvato con il nome, quindi un computer che non ha quel carattere mostra un altro carattere.

JeffDeck, per ora, non legge di nuovo le animazioni, i commenti, le presentazioni personalizzate e le impostazioni della presentazione da un file .odp. Sono nel file e LibreOffice li riproduce, ma quando apri il file .odp in JeffDeck, li elenca come esclusi. Conserva il file .pptx se vuoi continuare a modificarli qui.

Un file .odp non prevede posto per alcune cose. JeffDeck le elenca dopo il salvataggio, nel messaggio in basso nella finestra:

- **Grafici, diagrammi SmartArt ed equazioni** vengono salvati come immagini. Puoi vederli, ma non puoi modificare i loro numeri o le loro parti nel file .odp.
- **Le sezioni** non vengono salvate.
- **Smussature e ombreggiature interne** vengono escluse, così come bagliori e riflessi sul testo. L'ombreggiatura sul testo viene mantenuta. Il riflesso su una forma, una linea con sfumatura, una linea di due o tre tratti e la dimensione di un'ombreggiatura vengono mantenuti in un modo che solo JeffDeck legge. Un altro programma mostra la forma senza riflesso, la linea nel suo primo colore, un tratto singolo e l'ombreggiatura alla dimensione dell'oggetto.
- **Gli effetti di animazione** che partono con il clic su un oggetto, e gli effetti che cambiano un colore, vengono esclusi. Il clic su un video, per metterlo in pausa o riprodurlo di nuovo, viene mantenuto.
- **Un video o un suono collegato** da un altro file, non incorporato nella presentazione, mantiene il suo indirizzo e la sua immagine nel file .odp. LibreOffice lo riproduce se riesce a trovare il file. Video e suoni presenti nella presentazione vengono mantenuti nel file .odp e vengono riprodotti.

Conserva anche il file .pptx, se ti servono di nuovo queste cose.

## Ciò che viene conservato ma non mostrato

Alcune parti di un file sono cose che JeffDeck non può ancora mostrare o modificare. Le conserva insieme alle diapositive e le riscrive al salvataggio, così non vanno perse. Tra queste ci sono le animazioni, che vengono riprodotte nella presentazione, i video e i suoni, che per ora non vengono riprodotti, e un effetto di transizione per cui JeffDeck non ha una forma, che viene riprodotto come dissolvenza. Grafici e diagrammi SmartArt vengono mostrati come erano stati salvati. Vedi [Benvenuti in JeffDeck](start).

## Vedi anche

- [Recupera lavoro non salvato](recover)
- [La schermata File](file-screen)
- [Riutilizzare diapositive da un'altra presentazione](reuse)
- [Stampa, PDF e immagini delle diapositive](printing)
