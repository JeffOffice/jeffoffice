# Aprire e salvare presentazioni
<!-- keywords: macro, pptm, potm, ppsm, apri, apertura, salva, salva con nome, otp, libreoffice, lasciati fuori, chiudi, nuovo, file recenti, aggiunti, tipi di file, pptx, potx, ppsx, ppt, odp, modello, presentazione, Ctrl+S, backup, formati di file, file vecchi, documento OpenDocument, mantenute, non mostrate, caratteri, incorpora caratteri, carattere mancante, salva una copia, sola lettura, apri come copia, nuova finestra, copia di backup, modelli personali, salva come modello, ripara, apri e ripara, danneggiato -->

JeffDeck salva il lavoro in file .pptx e in file .odp, e apre diversi tipi di file di presentazione.

## Creare una nuova presentazione

Premi Ctrl+N. Si apre una nuova finestra con una presentazione vuota. Oppure fai clic su **File > Nuovo** e poi su **Presentazione vuota**. Se la finestra in cui ti trovi contiene una presentazione nuova che nessuno ha ancora modificato, la presentazione vuota la sostituisce. Altrimenti si apre in una finestra propria.

## Salvare

Premi Ctrl+S, oppure fai clic su **File > Salva**. La prima volta, JeffDeck chiede un nome e una posizione. Dopo, Salva sostituisce il file con il tuo lavoro più recente. Un messaggio in basso dice "Salvato" e indica il nome del file.

Un asterisco (*) dopo il nome nella barra del titolo indica che ci sono modifiche non salvate. Salva spesso. Se JeffDeck si chiude all'improvviso, consulta [Recuperare il lavoro non salvato](recover).

Se un messaggio dice che alcuni oggetti "non possono essere salvati in questo tipo di file", il file viene salvato, ma JeffDeck non ha modo di scrivere quegli oggetti. Conserva anche il file originale. Quando salvi un file .odp, il messaggio elenca ciò che quel tipo di file non può contenere. La sezione "Salvare come file .odp" più avanti spiega che cosa non può contenere.

## Salvare con un nuovo nome o tipo

Fai clic su **File > Salva con nome**, oppure premi F12. Digita un nome, scegli il tipo e fai clic su **Salva**. Le scelte sono:

- **Presentazione (*.pptx)** è la scelta normale.
- **Modello (*.potx)** salva le diapositive come schema da cui avviare nuove presentazioni.
- **Presentazione slide show (*.ppsx)** salva una presentazione da riprodurre.
- **Presentazione con macro (*.pptm)**, **Modello con macro (*.potm)** e **Presentazione slide show con macro (*.ppsm)** sono gli stessi tre tipi per una presentazione che contiene **macro**. Una macro è un piccolo programma che accompagna un file. JeffDeck non esegue mai le macro, e non le legge né le mostra mai. Se apri uno di questi file e lo salvi con lo stesso tipo, le macro vengono mantenute così come sono. Se lo salvi come .pptx, .potx o .ppsx, le macro vengono lasciate fuori e JeffDeck te lo segnala. Una presentazione senza macro può essere salvata anche con questi tipi, ma non ha macro da conservare. Se le macro di un file sono troppo grandi per essere conservate (più di 32 MB in una parte, o 64 MB in totale), JeffDeck le lascia fuori e te lo segnala quando apri il file.
- **Presentazione OpenDocument (*.odp)** salva le diapositive nel tipo di file usato da LibreOffice e da altri programmi gratuiti.
- **Modello di presentazione OpenDocument (*.otp)** è la forma di modello di questo tipo.

Se non scrivi l'estensione, JeffDeck aggiunge quella corrispondente al tipo scelto. Dopo il salvataggio, la barra del titolo mostra il nuovo nome e tu continui a lavorare nel nuovo file.

Una presentazione aperta da un modello viene salvata come nuova presentazione: **Salva** chiede un nome, così il modello resta com'era. Lo stesso vale per una presentazione aperta da un file .ppt, .pps o .pot più vecchio. Una presentazione aperta da un file .odp viene salvata di nuovo come file .odp, sullo stesso file.

## Salvare una copia

Fai clic su **File > Salva una copia** e scegli un nome. JeffDeck scrive la presentazione in quel file, e tu continui a lavorare in quella che avevi aperta. Il suo nome e le sue modifiche non salvate restano come erano.

## Conservare una copia di backup

Fai clic su **File > Opzioni**, e nella scheda **Generale** attiva **Crea sempre una copia di backup**. Ogni volta che salvi, JeffDeck copia prima il file vecchio in uno chiamato "Backup di" seguito dal suo nome, nella stessa cartella. Se la copia non può essere creata, l'app chiede se salvare senza di essa. La risposta è No, a meno che tu non dica Sì, e il file resta com'era.

## Salvare come modello

Un modello è una presentazione che usi come punto di partenza. Fai clic su **File > Salva come modello** e digita un nome. Il modello (un file **.potx**) viene messo in **Modelli personali**, e la tua presentazione non cambia. Per avviare una nuova presentazione dal modello, fai clic su **File > Nuovo**, poi sul modello sotto **Modelli personali**. La nuova presentazione non ha ancora un file proprio, quindi il modello non cambia. Per rimuovere un modello dall'elenco, fai clic con il pulsante destro su di esso e scegli **Rimuovi da Modelli personali**.

## Aprire

Premi Ctrl+O per scegliere subito un file. Oppure fai clic su **File > Apri**, e poi su **Sfoglia**, oppure su un nome sotto **Aggiunti** o **Presentazioni recenti**.

Il file si apre in questa finestra se contiene una presentazione nuova che nessuno ha ancora modificato. Altrimenti si apre in una finestra propria.

Fai clic con il pulsante destro su un nome nell'elenco per **Aggiungi all'elenco** (resta in cima), **Rimuovi dagli elementi aggiunti** oppure **Rimuovi dall'elenco**. Un nome in grigio indica che JeffDeck non trova più quel file.

## Altri modi per aprire un file

Nella pagina **Apri**, fai clic su **Altri modi per aprire**. Puoi anche fare clic con il pulsante destro su un nome nell'elenco dei file recenti.

- **Apri in sola lettura** apre la presentazione solo per la lettura. Puoi guardarla, scorrere le diapositive e copiare, ma non puoi modificare nulla. La barra del titolo indica "Sola lettura". Se premi **Salva**, JeffDeck chiede un nuovo nome, così il file aperto non viene modificato.
- **Apri come copia** apre le diapositive come nuova presentazione senza un file proprio. **Salva** chiede dove metterla.
- **Apri in nuova finestra** apre il file in una finestra propria, anche quando la finestra in cui ti trovi è vuota.
- **Apri e ripara** serve per un file .pptx danneggiato, ad esempio uno troncato durante la copia. JeffDeck legge ogni parte che riesce a leggere, conserva una parte interrotta fino al punto in cui si è fermata e lascia fuori una parte che non riesce a leggere. Le diapositive riparate si aprono come copia chiamata "Riparata" seguito dal nome, e un messaggio ti dice che cosa è stato lasciato fuori. Il file danneggiato non viene modificato.

## File con password

Un file **.pptx** o **.odp** che chiede una password si apre come al solito: JeffDeck chiede la password, e puoi riprovare se non è corretta, oppure fare clic su **Annulla**. Il file resta protetto quando lo salvi. Per cambiare o rimuovere la password, usa **File > Informazioni > Cifra con password** (vedi La schermata File). Una copia salvata come file .odp mantiene anche la password.

## Caratteri nel file

Un carattere è la forma delle lettere. Non tutti i computer hanno tutti i caratteri. Se a un computer manca un carattere usato dalle diapositive, le lettere vengono disegnate con un altro carattere. Le diapositive possono quindi apparire diverse.

JeffDeck può inserire nel file i caratteri usati dalle diapositive. Così le diapositive appaiono uguali su qualsiasi computer. Questa operazione si chiama incorporamento. JeffDeck la esegue a ogni salvataggio. Per disattivarla, fai clic su **File > Opzioni**, apri la scheda **Salvataggio** e deseleziona **Incorpora tipi di carattere nel file**.

- JeffDeck inserisce solo i caratteri che arrivano insieme al programma, come Fraunces, Lato e Open Sans. Altri computer non li hanno. Non inserisce mai un carattere che ogni computer possiede, come Calibri o Arial. Lascia fuori un carattere la cui licenza dice che non può essere copiato in un file.
- JeffDeck inserisce ogni stile usato dalle diapositive: normale, grassetto, corsivo e grassetto corsivo. Se le diapositive usano solo lo stile normale, viene inserito solo quello.
- JeffDeck inserisce ogni lettera di ciascun carattere, non solo quelle che hai usato. Così chi apre il file può comunque modificare le parole.
- Il file diventa più grande. Ogni stile di un carattere aggiunge da circa 40 a 700 KB (un KB è mille byte). Il file viene compresso al salvataggio, quindi cresce di circa la metà di questa dimensione.

Quando apri un file che contiene caratteri incorporati, JeffDeck li usa mentre la presentazione è aperta. Le diapositive appaiono come erano state create. JeffDeck non installa i caratteri sul computer e non li copia in nessun altro posto. Appartengono solo a quella presentazione: non compaiono negli elenchi dei caratteri e nessun'altra presentazione li usa. Spariscono quando chiudi la presentazione. Se il computer ha già un carattere, JeffDeck usa la sua copia.

JeffDeck legge i caratteri dai file .pptx creati con altri programmi di presentazione, anche quando il programma li ha compressi. Legge anche i caratteri dei file .odp.

Un file può contenere un carattere che JeffDeck non possiede. Al salvataggio, JeffDeck rimette quel carattere nel file, finché le diapositive lo usano ancora. Lo fa che **Incorpora tipi di carattere nel file** sia attivo o no. Se un carattere del file è danneggiato o non può essere letto, il messaggio che vedi aprendo il file lo segnala.

## Tipi di file che JeffDeck legge

- **.pptx, .potx e .ppsx** sono un unico formato in tre forme: presentazione, modello e presentazione slide show. JeffDeck legge e scrive tutti e tre.
- **.ppt, .pps e .pot** sono il tipo più vecchio. JeffDeck li legge ma non può scriverli, quindi **Salva** chiede un nuovo nome e salva un .pptx (un .ppsx per un .pps). Non sovrascrive mai l'originale.
- **.odp e .otp** sono presentazioni in formato documento aperto e i relativi modelli. JeffDeck legge e scrive entrambi. Un .odp aperto qui viene salvato come .odp. Un .otp è un modello, quindi **Salva** chiede un nuovo nome e propone un .odp.

Se una parte del file non può essere letta, JeffDeck apre il resto ed elenca ciò che ha lasciato fuori. Se l'intero file non può essere letto, ti dice perché.

## Salvare come file .odp

Un file .odp si apre in LibreOffice Impress con le diapositive, i master, le immagini, le note e i commenti, e con l'aspetto che JeffDeck mostra. Contiene animazioni, transizioni tra diapositive, presentazioni personalizzate e le impostazioni della presentazione. Il carattere di ogni parola viene salvato con il suo nome, quindi un computer che non ha il carattere mostra un altro carattere.

JeffDeck non legge ancora, da un file .odp, le animazioni, i commenti, le presentazioni personalizzate e le impostazioni della presentazione. Sono nel file e LibreOffice le riproduce, ma quando apri il file .odp in JeffDeck le elenca come lasciate fuori. Conserva il file .pptx se vuoi continuare a modificarle qui.

Un file .odp non ha spazio per alcune cose. JeffDeck le elenca dopo il salvataggio, nel messaggio in basso nella finestra:

- **Grafici, diagrammi ed equazioni** vengono salvati come immagini. Puoi vederli, ma non puoi modificare i loro numeri o le loro parti nel file .odp.
- **Le sezioni** non vengono salvate.
- **Smussature e ombreggiature interne** vengono lasciate fuori, così come bagliori e riflessi sul testo. L'ombreggiatura sul testo viene mantenuta. Il riflesso su una forma, una linea con sfumatura, una linea a due o tre tratti e la dimensione di un'ombreggiatura vengono mantenuti in un modo che solo JeffDeck legge. Altri programmi mostrano la forma senza riflesso, la linea nel suo primo colore, un solo tratto e l'ombra alla dimensione dell'oggetto.
- **Effetti di animazione** che iniziano con un clic su un oggetto, ed effetti che cambiano un colore, vengono lasciati fuori. Un clic su un video, per metterlo in pausa o riprodurlo di nuovo, viene mantenuto.
- **Un video o un suono collegato** da un altro file, non incorporato nella presentazione, mantiene il suo indirizzo e la sua immagine nel file .odp. LibreOffice lo riproduce se riesce a trovare il file. I video e i suoni incorporati nella presentazione vengono mantenuti nel file .odp e si riproducono.

Conserva anche il file .pptx, se ti servono di nuovo queste cose.

## Cosa viene mantenuto ma non mostrato

Alcune parti di un file sono cose che JeffDeck non può ancora mostrare o modificare. Le conserva insieme alle diapositive e le riscrive al salvataggio, così non vanno perse. Tra queste ci sono le animazioni, che si riproducono nella presentazione, i video e i suoni, che per ora non si riproducono, e un effetto di transizione per cui JeffDeck non ha una forma, che viene riprodotto come dissolvenza. Grafici e diagrammi appaiono come erano stati salvati. Vedi [Benvenuti in JeffDeck](start).

## Argomenti correlati

- [Recuperare il lavoro non salvato](recover)
- [La schermata File](file-screen)
- [Riutilizzare diapositive da un'altra presentazione](reuse)
- [Stampa, PDF e immagini delle diapositive](printing)
