# Aprire e salvare presentazioni
<!-- keywords: macro, macro abilitate, pptm, potm, ppsm, apri, salva, salva con nome, otp, libreoffice, lasciati fuori, chiudi, nuovo, file recenti, aggiunti, tipi di file, pptx, potx, ppsx, ppt, odp, modello, presentazione, rinomina, Ctrl+S, backup, copia di backup, formati di file, file vecchi, documento OpenDocument, mantenuti, non visualizzati, caratteri, incorpora caratteri, caratteri incorporati, carattere mancante, salva una copia, sola lettura, apri in sola lettura, apri come copia, nuova finestra, copia di backup, miei modelli, salva come modello, ripara, apri e ripara, danneggiato -->

JeffDeck salva il lavoro in file .pptx e in file .odp, e apre diversi tipi di file di presentazione.

## Creare una nuova presentazione

Premi CTRL+N. Si apre una nuova finestra con una presentazione vuota. Oppure fai clic su **File > Nuovo** e poi su **Presentazione vuota**. Se la finestra in cui ti trovi contiene una presentazione nuova che nessuno ha modificato, la presentazione vuota la sostituisce. Altrimenti si apre in una finestra propria.

## Salvare

Premi CTRL+S, oppure fai clic su **File > Salva**. La prima volta, JeffDeck chiede un nome e una posizione. Dopo, Salva sostituisce il file con il tuo lavoro più recente. Un messaggio in basso dice "Salvato" e indica il nome del file.

Un asterisco (*) dopo il nome nella barra del titolo indica che ci sono modifiche non salvate. Salva spesso. Se JeffDeck si chiude all'improvviso, consulta [Recuperare il lavoro non salvato](recover).

Se un messaggio dice che alcuni oggetti "non possono ancora essere salvati in questo tipo di file", il file viene salvato, ma JeffDeck non aveva modo di scrivere quegli oggetti. Conserva anche il file originale. Quando salvi un file .odp, il messaggio elenca ciò che quel tipo di file non può contenere. La sezione "Salvare come file .odp" più avanti spiega che cosa non può contenere.

## Salvare con un nuovo nome o tipo

Fai clic su **File > Salva con nome**, oppure premi F12. Digita un nome, scegli il tipo e fai clic su **Salva**. Le scelte sono:

- **Presentazione (*.pptx)** è la scelta normale.
- **Modello (*.potx)** salva le diapositive come modello da cui avviare nuove presentazioni.
- **Presentazione automatica (*.ppsx)** salva una presentazione da riprodurre.
- **Presentazione con macro (*.pptm)**, **Modello con macro (*.potm)** e **Presentazione slide show con macro (*.ppsm)** sono gli stessi tre tipi per una presentazione che contiene **macro**. Una macro è un piccolo programma che accompagna un file. JeffDeck non esegue mai le macro, e non le legge né le mostra mai. Se apri uno di questi file e lo salvi con lo stesso tipo, le macro vengono mantenute come erano. Se lo salvi come .pptx, .potx o .ppsx, le macro vengono lasciate fuori e JeffDeck te lo comunica. Una presentazione senza macro può essere salvata anche con questi tipi, ma non ha macro da conservare. Se le macro di un file sono troppo grandi per essere conservate (più di 32 MB in una parte, o 64 MB in totale), JeffDeck le lascia fuori e te lo segnala all'apertura.
- **Presentazione OpenDocument (*.odp)** salva le diapositive nel tipo di file usato da LibreOffice e da altri programmi gratuiti.
- **Modello di presentazione OpenDocument (*.otp)** è la forma di modello di questo tipo.

Se ometti l'estensione, JeffDeck aggiunge quella corrispondente al tipo. Dopo un salvataggio, la barra del titolo mostra il nuovo nome e continui a lavorare nel nuovo file.

Una presentazione aperta da un modello viene salvata come nuova presentazione: **Salva** chiede un nome, così il modello resta invariato. Lo stesso vale per una presentazione aperta da un file .ppt, .pps o .pot più vecchio. Una presentazione aperta da un file .odp viene salvata di nuovo come file .odp, sullo stesso file.

## Salvare una copia

Fai clic su **File > Salva una copia** e scegli un nome. JeffDeck scrive la presentazione in quel file, e tu continui a lavorare in quella che avevi aperto. Il suo nome e le sue modifiche non salvate restano come erano.

## Conservare una copia di backup

Fai clic su **File > Opzioni**, e nella scheda **Generale** attiva **Crea sempre una copia di backup**. Ogni volta che salvi, JeffDeck copia prima il file precedente in un file chiamato "Backup di" seguito dal nome del file, nella stessa cartella. Se la copia non può essere creata, l'app chiede se salvare comunque senza di essa. La risposta predefinita è No, a meno che tu non dica Sì, e il file resta come era.

## Salvare come modello

Un modello è una presentazione che usi come punto di partenza. Fai clic su **File > Salva come modello** e digita un nome. Il modello (un file **.potx**) viene messo in **Miei modelli**, e la tua presentazione non cambia. Per avviare una nuova presentazione da esso, fai clic su **File > Nuovo**, e poi sul modello in **Miei modelli**. La nuova presentazione non ha ancora un file proprio, quindi il modello non cambia. Per rimuovere un modello dall'elenco, fai clic con il pulsante destro sul suo nome e scegli **Rimuovi da Miei modelli**.

## Aprire

Premi CTRL+O per scegliere subito un file. Oppure fai clic su **File > Apri**, e poi su **Sfoglia**, oppure su un nome sotto **Aggiunti** o **Presentazioni recenti**.

Il file si apre in questa finestra se contiene una presentazione nuova che nessuno ha modificato. Altrimenti si apre in una finestra propria.

Fai clic con il pulsante destro su un nome nell'elenco per **Aggiungi all'elenco** (resta in cima), **Rimuovi dagli elementi aggiunti** o **Rimuovi dall'elenco**. Un nome in grigio indica che JeffDeck non riesce a trovare quel file in questo momento.

## Altri modi per aprire un file

Nella pagina **Apri**, fai clic su **Altri modi per aprire**. Puoi anche fare clic con il pulsante destro su un nome nell'elenco dei recenti.

- **Apri in sola lettura** apre la presentazione solo per la lettura. Puoi guardarla, scorrere le diapositive e copiare, ma non puoi modificare nulla. La barra del titolo indica "Sola lettura". Se premi **Salva**, JeffDeck chiede un nuovo nome, così il file che hai aperto non viene modificato.
- **Apri come copia** apre le diapositive come nuova presentazione senza un file proprio. **Salva** chiede dove metterla.
- **Apri in una nuova finestra** apre il file in una finestra propria, anche quando la finestra in cui ti trovi è vuota.
- **Apri e ripara** serve per un file .pptx o .ppt danneggiato, ad esempio uno che è stato troncato durante una copia. JeffDeck recupera quanto più possibile: legge ogni parte che riesce a leggere, conserva una parte interrotta fino al punto in cui si è fermata e lascia fuori una parte che non riesce a leggere. Per un vecchio file .ppt, trova le diapositive in base al loro contenuto quando gli elenchi che indicano dove sono conservate risultano danneggiati o mancanti. Quando gli elenchi che indicano come si collegano le parti del file sono danneggiati, il programma prende ogni parte come seguito di quella precedente e mostra ciò che trova. Quella parte potrebbe appartenere a una copia più vecchia della presentazione o a un'altra parte del file, quindi il messaggio indica la diapositiva da cui inizia e ti chiede di confrontarla con un backup, se ne hai uno. Se l'ipotesi riguarda prima della prima diapositiva, il messaggio dice che l'intera presentazione è stata ricostruita. Se le immagini non sono state recuperate completamente, il messaggio lo dice. Se è andato perso l'elenco che dà l'ordine delle diapositive, queste vengono disposte nell'ordine in cui sono state salvate, che potrebbe non essere quello originale. Un file protetto da password non può essere riparato. Le diapositive riparate si aprono come copia chiamata "Riparata" seguito dal nome, e un messaggio ti dice che cosa è stato fatto. Il file danneggiato non viene modificato.

## File protetti da password

Un file **.pptx** o **.odp** che richiede una password si apre normalmente: JeffDeck chiede la password, e puoi riprovare se non è corretta, oppure fare clic su **Annulla**. Il file resta protetto quando lo salvi. Per cambiare o rimuovere la password, usa **File > Informazioni > Cifra con password** (vedi La schermata File). Anche una copia salvata come file .odp mantiene la password.

## Caratteri nel file

Un carattere è la forma delle lettere. Non tutti i computer hanno tutti i caratteri. Se a un computer manca un carattere usato dalle diapositive, disegna le lettere con un altro carattere. In questo caso le diapositive possono apparire diverse.

JeffDeck può inserire nel file i caratteri usati dalle diapositive. Così le diapositive appaiono uguali su qualsiasi computer. Questa operazione si chiama incorporamento. JeffDeck la esegue a ogni salvataggio. Per disattivarla, fai clic su **File > Opzioni**, apri la scheda **Salva** e deseleziona **Incorpora tipi di carattere nel file**.

- JeffDeck inserisce solo i caratteri che arrivano con il programma, come Fraunces, Lato e Open Sans. Altri computer non li hanno. Non inserisce mai un carattere che ogni computer possiede, come Calibri o Arial. Lascia fuori un carattere la cui licenza non ne consente la copia in un file.
- JeffDeck inserisce ogni stile usato dalle diapositive: normale, grassetto, corsivo e grassetto corsivo. Se le diapositive usano solo lo stile normale, viene inserito solo quello.
- JeffDeck inserisce tutte le lettere di ogni carattere, non solo quelle usate. Così chi apre il file può ancora modificare il testo.
- Il file diventa più grande. Ogni stile di un carattere aggiunge circa da 40 a 700 KB (un KB è mille byte). Il file viene compresso al salvataggio, quindi cresce di circa la metà di questa dimensione.

Quando apri un file che contiene caratteri incorporati, JeffDeck li usa mentre la presentazione è aperta. Le diapositive appaiono come erano state realizzate. JeffDeck non installa i caratteri sul computer e non li copia da nessuna parte. Appartengono solo a quella presentazione: non compaiono negli elenchi dei caratteri e nessun'altra presentazione li usa. Vengono eliminati alla chiusura della presentazione. Se il computer ha già un carattere, JeffDeck usa la tua copia.

JeffDeck legge i caratteri dai file .pptx creati da altri programmi di presentazione, anche quando questi li hanno compressi. Legge anche i caratteri dei file .odp.

Un file può contenere un carattere che JeffDeck non possiede. Al salvataggio, JeffDeck rimette quel carattere nel file, purché le diapositive lo usino ancora. Lo fa indipendentemente dal fatto che **Incorpora tipi di carattere nel file** sia attivo o meno. Se un carattere del file è danneggiato o non può essere letto, il messaggio che vedi all'apertura del file lo indica.

## Tipi di file che JeffDeck legge

- **.pptx, .potx e .ppsx** sono un unico formato in tre forme: presentazione, modello e presentazione automatica. JeffDeck legge e scrive tutte e tre.
- **.ppt, .pps e .pot** sono il tipo più vecchio. JeffDeck li legge ma non può scriverli, quindi **Salva** chiede un nuovo nome e salva un .pptx (un .ppsx per un .pps). Non sovrascrive mai l'originale.
- **.odp e .otp** sono le presentazioni in formato OpenDocument e i relativi modelli. JeffDeck legge e scrive entrambi. Un .odp aperto qui viene salvato come .odp. Un .otp è un modello, quindi **Salva** chiede un nuovo nome e propone un .odp.

Se una parte del file non può essere letta, JeffDeck apre il resto ed elenca ciò che ha lasciato fuori. Se l'intero file non può essere letto, ti dice il motivo.

## Salvare come file .odp

Un file .odp si apre in LibreOffice Impress con le tue diapositive, i master, le immagini, le note e i commenti, e con l'aspetto che JeffDeck mostra. Contiene animazioni, transizioni tra diapositive, presentazioni personalizzate e le impostazioni della presentazione. Il carattere di ogni parola viene salvato con il nome, quindi un computer che non possiede il carattere mostra un altro carattere.

JeffDeck non legge ancora animazioni, commenti, presentazioni personalizzate né impostazioni della presentazione da un file .odp. Sono nel file e LibreOffice li riproduce, ma quando apri il file .odp in JeffDeck, questi elementi risultano lasciati fuori. Conserva il file .pptx se vuoi continuare a modificarli qui.

Un file .odp non ha spazio per alcune cose. JeffDeck le elenca dopo il salvataggio, nel messaggio in basso nella finestra:

- **Grafici, diagrammi ed equazioni** vengono salvati come immagini. Puoi vederli, ma non puoi modificarne i numeri o le parti nel file .odp.
- **Le sezioni** non vengono salvate.
- **Smussature e ombre interne** vengono lasciate fuori, così come bagliori e riflessi sul testo. Un'ombra sul testo viene mantenuta. Un riflesso su una forma, una linea con sfumatura, una linea di due o tre tratti e la dimensione di un'ombra vengono mantenuti in un modo che solo JeffDeck legge. Altri programmi mostrano la forma senza riflesso, la linea nel suo primo colore, un tratto singolo e l'ombra alle dimensioni dell'oggetto.
- **Effetti di animazione** che iniziano con il clic su un oggetto, ed effetti che cambiano un colore, vengono lasciati fuori. Un clic su un video, per metterlo in pausa o riprodurlo di nuovo, viene mantenuto.
- **Un video o un suono collegato** da un altro file, non incorporato nella presentazione, mantiene il suo indirizzo e la sua immagine nel file .odp. LibreOffice lo riproduce se riesce a trovare il file. I video e i suoni incorporati nella presentazione vengono mantenuti nel file .odp e vengono riprodotti.

Conserva anche il file .pptx, se ti servono queste cose.

## Ciò che viene mantenuto ma non mostrato

Alcune parti di un file sono elementi che JeffDeck non può ancora mostrare o modificare. Li conserva insieme alle diapositive e li riscrive quando salvi, così non vanno persi. Tra questi ci sono le animazioni, che vengono riprodotte durante la presentazione, i video e i suoni, che non vengono ancora riprodotti, e un effetto di transizione per cui JeffDeck non ha una forma, che viene riprodotto come dissolvenza. Grafici e diagrammi vengono mostrati come erano stati salvati. Vedi [Benvenuti in JeffDeck](start).

## Argomenti correlati

- [Recuperare il lavoro non salvato](recover)
- [La schermata File](file-screen)
- [Riutilizzare diapositive da un'altra presentazione](reuse)
- [Stampa, PDF e immagini delle diapositive](printing)
