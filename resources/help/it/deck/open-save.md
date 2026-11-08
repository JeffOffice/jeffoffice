# Aprire e salvare le presentazioni

<!-- keywords: apri, salva, salva con nome, otp, libreoffice, lasciati fuori, chiudi, nuovo, file recenti, aggiunti, tipi di file, pptx, potx, ppsx, ppt, odp, modello, presentazione automatica, rinomina, Ctrl+S, backup, formati di file, file vecchi, documento OpenDocument, mantenuto, non visualizzato, caratteri, incorpora tipi di carattere, caratteri incorporati, tipo di carattere mancante, salva una copia, sola lettura, apri come copia, nuova finestra, copia di backup, i miei modelli, salva come modello, ripara, apri e ripara, danneggiato -->

JeffDeck salva il tuo lavoro in file .pptx e in file .odp, e apre diversi tipi di file di presentazione.

## Avviare una nuova presentazione

Premi Ctrl+N. Si apre una nuova finestra con una presentazione vuota. Oppure fai clic su **File > Nuovo** e poi su **Presentazione vuota**. Se la finestra in cui ti trovi contiene una nuova presentazione che nessuno ha ancora modificato, la presentazione vuota prende il suo posto. Altrimenti si apre in una finestra a sé.

## Salvare

Premi Ctrl+S, oppure fai clic su **File > Salva**. La prima volta, JeffDeck chiede un nome e una posizione. Dopo, Salva sostituisce il file con il tuo lavoro più recente. In fondo appare un messaggio che dice "Salvato" e indica il nome del file.

Un asterisco (*) dopo il nome nella barra del titolo indica che ci sono modifiche non salvate. Salva spesso. Se JeffDeck si interrompe all'improvviso, consulta [Recuperare il lavoro non salvato](recover).

Se un messaggio dice che alcuni oggetti "non possono ancora essere salvati in questo tipo di file", il file viene salvato, ma JeffDeck non aveva modo di scrivere quegli oggetti. Conserva anche il file originale. Quando salvi un file .odp, il messaggio elenca ciò che questo tipo di file non può contenere. La sezione "Salvare come file .odp" qui sotto spiega che cosa non può contenere.

## Salvare con un nuovo nome o tipo

Fai clic su **File > Salva con nome**, oppure premi F12. Digita un nome, scegli il tipo e fai clic su **Salva**. Le scelte sono:

- **Presentazione (*.pptx)** è la scelta normale.
- **Modello (*.potx)** salva le diapositive come schema da cui iniziare nuove presentazioni.
- **Presentazione automatica (*.ppsx)** salva una presentazione da riprodurre.
- **Presentazione OpenDocument (*.odp)** salva le diapositive nel tipo di file usato da LibreOffice e da altri programmi gratuiti.
- **Modello di presentazione OpenDocument (*.otp)** è la forma come modello di questo tipo.

Se ometti l'estensione, JeffDeck aggiunge quella corrispondente al tipo scelto. Dopo il salvataggio, la barra del titolo mostra il nuovo nome e continui a lavorare nel nuovo file.

Una presentazione aperta da un modello viene salvata come nuova presentazione: **Salva** chiede un nome, così il modello resta com'era. Lo stesso vale per una presentazione aperta da un file .ppt, .pps o .pot meno recente. Una presentazione aperta da un file .odp viene salvata di nuovo come .odp, nello stesso file.

## Salvare una copia

Fai clic su **File > Salva una copia** e scegli un nome. JeffDeck scrive la presentazione in quel file, e tu continui a lavorare in quella che avevi aperta. Il suo nome e le sue modifiche non salvate restano invariati.

## Conservare una copia di backup

Fai clic su **File > Opzioni**, e nella scheda **Generale** attiva **Crea sempre una copia di backup**. Ogni volta che salvi, JeffDeck copia prima il file precedente in un file chiamato "Backup di" seguito dal nome del file, nella stessa cartella. Se la copia non può essere creata, l'app chiede se salvare senza di essa. La risposta predefinita è No, a meno che tu non risponda Sì, e il file resta com'era.

## Salvare come modello

Un modello è una presentazione che usi come punto di partenza. Fai clic su **File > Salva come modello** e digita un nome. Il modello (un file **.potx**) viene inserito in **I miei modelli**, e la tua presentazione non viene modificata. Per iniziare una nuova presentazione da un modello, fai clic su **File > Nuovo**, poi fai clic sul modello in **I miei modelli**. La nuova presentazione non ha ancora un file proprio, quindi il modello non viene modificato. Per rimuovere un modello dall'elenco, fai clic con il pulsante destro su di esso e scegli **Rimuovi da I miei modelli**.

## Aprire

Premi Ctrl+O per scegliere subito un file. Oppure fai clic su **File > Apri**, poi su **Sfoglia**, oppure su un nome in **Aggiunti** o **Presentazioni recenti**.

Il file si apre in questa finestra se contiene una nuova presentazione che nessuno ha ancora modificato. Altrimenti si apre in una finestra a sé.

Fai clic con il pulsante destro su un nome nell'elenco per **Aggiungi all'elenco** (resta in cima), **Rimuovi dagli elementi aggiunti** o **Rimuovi dall'elenco**. Un nome in grigio indica che JeffDeck non trova più quel file.

## Altri modi per aprire un file

Nella pagina **Apri**, fai clic su **Altri modi per aprire**. Puoi anche fare clic con il pulsante destro su un nome nell'elenco dei recenti.

- **Apri in sola lettura** apre la presentazione solo per la lettura. Puoi guardarla, scorrere le diapositive e copiare, ma non puoi modificare nulla. La barra del titolo indica "Sola lettura". Se premi **Salva**, JeffDeck chiede un nuovo nome, così il file aperto non viene modificato.
- **Apri come copia** apre le diapositive come nuova presentazione senza un file proprio. **Salva** chiede dove metterla.
- **Apri in una nuova finestra** apre il file in una finestra a sé, anche quando la finestra in cui ti trovi è vuota.
- **Apri e ripara** serve per un file .pptx danneggiato, ad esempio uno troncato durante la copia. JeffDeck legge ogni parte che riesce a leggere, conserva una parte interrotta fino al punto in cui si è fermata ed esclude una parte che non riesce a leggere. Le diapositive riparate si aprono come copia denominata "Riparata" seguito dal nome, e un messaggio ti dice che cosa è stato escluso. Il file danneggiato non viene modificato.

## Caratteri nel file

Un carattere è la forma delle lettere. Non tutti i computer hanno tutti i caratteri. Se a un computer manca un carattere usato dalle diapositive, le lettere vengono disegnate con un altro carattere. In questo caso le diapositive possono avere un aspetto diverso.

JeffDeck può inserire nel file i caratteri usati dalle diapositive. Così le diapositive hanno lo stesso aspetto su qualsiasi computer. Questa operazione si chiama incorporamento. JeffDeck la esegue a ogni salvataggio. Per disattivarla, fai clic su **File > Opzioni**, apri la scheda **Salva** e deseleziona **Incorpora tipi di carattere nel file**.

- JeffDeck inserisce solo i caratteri che arrivano insieme al programma, come Fraunces, Lato e Open Sans. Altri computer non li hanno. Non inserisce mai un carattere presente su ogni computer, come Calibri o Arial. Lascia fuori un carattere la cui licenza non ne consente la copia in un file.
- JeffDeck inserisce ogni stile usato dalle diapositive: normale, grassetto, corsivo e grassetto corsivo. Se le diapositive usano solo lo stile normale, viene inserito solo quello.
- JeffDeck inserisce ogni lettera di ciascun carattere, non solo quelle che hai usato. Così chi apre il file può comunque modificare le parole.
- Il file diventa più grande. Ogni stile di un carattere aggiunge da circa 40 a 700 KB (un KB è mille byte). Il file viene compresso al salvataggio, quindi cresce di circa la metà di quella quantità.

Quando apri un file che contiene caratteri incorporati, JeffDeck li usa mentre la presentazione è aperta. Le diapositive hanno l'aspetto che avevano dove sono state create. JeffDeck non installa i caratteri sul tuo computer e non li copia da nessuna parte. Appartengono solo a quella presentazione: non compaiono negli elenchi dei caratteri e nessun'altra presentazione li usa. Spariscono quando chiudi la presentazione. Se il tuo computer ha già un carattere, JeffDeck usa la tua copia.

JeffDeck legge i caratteri dai file .pptx creati con altri programmi di presentazione, anche quando il programma li ha compressi. Legge anche i caratteri dei file .odp.

Un file può contenere un carattere che JeffDeck non ha. Quando salvi, JeffDeck rimette quel carattere nel file, purché le diapositive lo usino ancora. Lo fa indipendentemente dal fatto che **Incorpora tipi di carattere nel file** sia attivo o meno. Se un carattere del file è danneggiato o non può essere letto, il messaggio che vedi all'apertura del file lo indica.

## Tipi di file che JeffDeck legge

- **.pptx, .potx e .ppsx** sono un unico formato in tre forme: presentazione, modello e presentazione automatica. JeffDeck legge e scrive tutte e tre.
- **.ppt, .pps e .pot** sono il tipo più vecchio. JeffDeck li legge ma non può scriverli, quindi **Salva** chiede un nuovo nome e salva un .pptx (un .ppsx per un .pps). Non sovrascrive mai l'originale.
- **.odp e .otp** sono presentazioni in formato documento aperto e i loro modelli. JeffDeck legge e scrive entrambi. Un .odp aperto qui viene salvato come .odp. Un .otp è un modello, quindi **Salva** chiede un nuovo nome e propone un .odp.

Se una parte del file non può essere letta, JeffDeck apre il resto ed elenca ciò che ha escluso. Se l'intero file non può essere letto, ti dice il motivo.

## Salvare come file .odp

Un file .odp si apre in LibreOffice Impress con le tue diapositive, i master, le immagini, le note e i commenti, e con l'aspetto che JeffDeck mostra. Contiene animazioni, transizioni tra diapositive, presentazioni personalizzate e le impostazioni della presentazione. Il carattere di ogni parola viene salvato con il nome, quindi un computer che non ha quel carattere mostra un altro carattere.

JeffDeck non legge ancora le animazioni, i commenti, le presentazioni personalizzate né le impostazioni della presentazione da un file .odp. Sono nel file e LibreOffice li riproduce, ma quando apri il file .odp in JeffDeck, li elenca come esclusi. Conserva il file .pptx se vuoi continuare a modificarli qui.

Un file .odp non ha spazio per alcune cose. JeffDeck le elenca dopo il salvataggio, nel messaggio in fondo alla finestra:

- **Grafici, diagrammi ed equazioni** vengono salvati come immagini. Puoi vederli, ma non puoi modificarne i numeri o le parti nel file .odp.
- **Le sezioni** non vengono salvate.
- **Smussature e ombreggiature interne** vengono lasciate fuori, così come aloni e riflessi sul testo. Un'ombreggiatura sul testo viene mantenuta. Un riflesso su una forma, una linea con sfumatura, una linea di due o tre tratti e la dimensione di un'ombra vengono mantenuti in un modo che solo JeffDeck legge. Altri programmi mostrano la forma senza il riflesso, la linea nel suo primo colore, un tratto singolo e l'ombra alla dimensione dell'oggetto.
- **Effetti di animazione** che partono al clic su un oggetto, ed effetti che cambiano un colore, vengono lasciati fuori. Un clic su un video, per metterlo in pausa o riprodurlo di nuovo, viene mantenuto.
- **Un video o un suono collegato da un altro file**, non incorporato nella presentazione, conserva il suo indirizzo e la sua immagine nel file .odp. LibreOffice lo riproduce se riesce a trovare il file. I video e i suoni incorporati nella presentazione vengono mantenuti nel file .odp e vengono riprodotti.

Conserva anche il file .pptx, se ti servono di nuovo queste cose.

## Che cosa viene mantenuto ma non mostrato

Alcune parti di un file sono cose che JeffDeck non può ancora mostrare o modificare. Le conserva insieme alle diapositive e le riscrive al salvataggio, così non vanno perse. Sono compresi le animazioni, che vengono riprodotte nella presentazione, i video e i suoni, che non vengono ancora riprodotti, e un effetto di transizione per cui JeffDeck non ha una forma, che viene riprodotto come dissolvenza. Grafici e diagrammi vengono mostrati come erano stati salvati. Vedi [Benvenuti in JeffDeck](start).

## Vedi anche

- [Recuperare il lavoro non salvato](recover)
- [La schermata File](file-screen)
- [Riutilizzare diapositive da un'altra presentazione](reuse)
- [Stampa, PDF e immagini delle diapositive](printing)
