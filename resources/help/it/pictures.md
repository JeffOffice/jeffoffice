# Aggiungere e regolare le immagini
<!-- keywords: effetti immagine, ombreggiatura, riflesso, alone, bordi sfumati, smussatura, rotazione 3D, reimposta immagine e dimensioni, foto, immagine, inserisci immagine, ritaglia, clip art, pagina PDF, jpg, png, svg, ricolora, testo alternativo, didascalia, segnaposto, gestione grafica, immagini online, collega a file, immagine collegata, inserisci e collega, incorpora, immagine mancante, ricollega, ritaglio, sfondo, rimozione sfondo, foto, immagine, collegamento -->

Un'immagine si trova all'interno di una cornice. È possibile ritagliare l'immagine, oppure spostare o ridimensionare la cornice.

## Inserire un'immagine da un file

1. Fare clic su **Inserisci > Illustrazioni > Immagini**.
2. Nella finestra **Inserisci immagine**, scegliere un file. JeffPub apre file PNG, JPEG, GIF, BMP, TIFF, WebP, SVG, WMF, EMF, ICO e PDF.
3. Scegliere **Inserisci**, **Collega al file** oppure **Inserisci e collega** nella casella **Inserisci come**, che si trova accanto all'elenco dei file e si raggiunge con il tasto Tab. Ogni volta che la finestra si apre è selezionato **Inserisci**. Vedere "Collegare un'immagine al suo file" più avanti.
4. Fare clic su **Apri**.

L'immagine viene inserita al centro della pagina. È anche possibile trascinare un file immagine sulla pagina; in questo caso si usa sempre **Inserisci**.

JeffPub legge i file immagine fino a 256 MB. Un file più grande viene rifiutato, così come un file vuoto o un elemento che non è un normale file su disco, come un dispositivo o una pipe. Il messaggio indica che JeffPub non può leggerlo come immagine.

Se si scelgono più immagini contemporaneamente, vengono disposte come miniature nell'area di lavoro, lo spazio grigio accanto alla pagina. Trascinare sulla pagina quelle desiderate.

Se un PDF ha più pagine, la finestra **Inserisci pagina PDF** mostra ciascuna pagina. Sceglierne una e fare clic su **OK**.

## Collegare un'immagine al suo file

**Inserisci** copia l'immagine nella pubblicazione. Questa copia segue la pubblicazione ovunque venga spostata, ma una pubblicazione con molte foto grandi diventa a sua volta grande. Un collegamento mantiene invece le foto nei loro file originali.

- **Inserisci** copia l'immagine nella pubblicazione. È la scelta abituale.
- **Collega al file** conserva solo la posizione del file e un'anteprima ridotta, di non più di 512 pixel sul lato più lungo. L'immagine nella pagina, e in una stampa, un PDF o un e-book, proviene dal file a piena qualità. Se il file viene spostato o eliminato, la pagina mostra invece l'anteprima ridotta.
- **Inserisci e collega** fa entrambe le cose. La pubblicazione conserva l'immagine completa e il collegamento. Quando si apre la pubblicazione e il file è cambiato, JeffPub lo legge di nuovo e sostituisce la propria copia.

JeffPub ricorda il percorso completo del file e anche la sua posizione rispetto alla pubblicazione, ad esempio "la cartella Foto accanto a essa". Se si sposta la pubblicazione insieme alle sue immagini, quest'ultimo riferimento permette di ritrovarle. Mantenere le cartelle come sono.

JeffPub apre da solo il file di un'immagine collegata solo se il file si trova nella cartella della pubblicazione o in una cartella al suo interno. Un'immagine collegata a un file situato altrove mostra la sua anteprima salvata, sfocata, ed è contrassegnata come **Non aggiornata**, finché non la si seleziona e non si fa clic su **Aggiorna collegamento** in Gestione grafica. In questo modo una pubblicazione ricevuta da qualcun altro non può accedere ai file del computer. Per caricare automaticamente le immagini, tenerle nella cartella della pubblicazione.

**Gestione grafica** mostra se ciascuna immagine è **Incorporata**, **Collegata**, **Mancante**, **Modificata** o **Non aggiornata**, e ha pulsanti per aggiornare un collegamento, indirizzarlo verso un altro file o trasformarlo in una copia. Vedere [Gestione grafica](graphics-manager).

Quando si salva una pubblicazione come file `.pub`, si crea un file Pack and Go o la si invia per e-mail, JeffPub inserisce nel file ciascuna immagine collegata per intero, così da non dipendere dai file che restano sul computer. Se in quel momento un file collegato manca, oppure l'immagine è **Non aggiornata**, viene inserita la sua piccola anteprima. Un file `.pub` aperto non contiene mai collegamenti: le sue immagini vengono importate come copie.

## Aggiungere un segnaposto per immagine

Un segnaposto è una cornice vuota che riserva lo spazio per un'immagine. Fare clic su **Inserisci > Illustrazioni > Segnaposto immagine**, quindi fare doppio clic sulla cornice e scegliere un file. L'immagine riempie la cornice e i bordi che sporgono vengono nascosti. Un segnaposto vuoto non viene stampato.

## Trovare immagini gratuite online

Fare clic su **Inserisci > Illustrazioni > Immagini online**. Il riquadro **Immagini online** cerca in due librerie gratuite di immagini con licenza aperta: Openverse, che comprende molte raccolte, e Wikimedia Commons, la libreria delle immagini di Wikipedia.

1. Scegliere una libreria, digitare ciò che si cerca, ad esempio "fienile rosso", e premere Invio.
2. Fare clic su un'immagine per vedere chi l'ha realizzata e la sua licenza. La licenza è l'insieme delle regole per usare l'immagine. La maggior parte richiede di citare l'autore.
3. Fare clic su **Inserisci**. JeffPub inserisce l'immagine nella pagina e usa il suo titolo come testo alternativo. Con **Aggiungi i crediti sotto l'immagine** selezionato, sotto di essa compare una riga come "Fienile rosso" di Pat Lee, CC BY 2.0.

Le parole della ricerca vengono inviate alla libreria scelta. Verificare che la licenza di ciascuna immagine sia compatibile con l'uso che se ne vuole fare.

Una ricerca mostra fino a 24 immagini. JeffPub usa solo indirizzi web sicuri (https) di una libreria e tralascia le immagini il cui indirizzo non lo è. Da Wikimedia Commons, un'immagine più larga di 1.920 pixel viene scaricata con quella larghezza, non nella dimensione originale caricata. Un download superiore a 50 MB, oppure troppo lungo, viene interrotto e il riquadro indica che l'immagine non può essere scaricata o caricata.

## Ritagliare un'immagine

Il ritaglio nasconde i bordi di un'immagine.

1. Fare clic sull'immagine, quindi su **Formato immagine > Ritaglia > Ritaglia**.
2. Trascinare un quadratino per nascondere una parte dell'immagine. Trascinare l'immagine per farla scorrere all'interno della cornice.
3. Fare di nuovo clic su **Ritaglia**, oppure premere **Esc**.

Sempre nel gruppo **Ritaglia**: **Proporzioni** ritaglia la cornice in una forma come quadrato, 4 di larghezza per 3 di altezza, oppure 16 per 9, attorno al suo centro (l'immagine resta dove si trova nella pagina); **Adatta** mostra l'intera immagine nella sua cornice; **Riempi** riempie la cornice; **Ritaglia nella forma** rifila l'immagine secondo una forma come un ovale; **Rimuovi ritaglio** mostra di nuovo l'intera immagine.

## Modificare l'aspetto di un'immagine

Nella scheda **Formato immagine**, usare il gruppo **Regola**:

- **Rimozione sfondo**: elimina lo sfondo di un'immagine, ad esempio il muro uniforme dietro una persona o il bianco attorno a un logo. Fare clic sull'immagine, quindi su **Rimozione sfondo**. JeffPub individua i colori lungo i bordi dell'immagine e colora di rosa ciò che sembra sfondo. Se ne ha eliminato troppo o troppo poco, fare clic su **Mantieni aree** o **Rimuovi aree** nella scheda **Rimozione sfondo** e trascinare sull'immagine; dove due segni si incontrano, prevale quello tracciato per ultimo. **Mantieni modifiche** (oppure Invio) termina l'operazione e conserva il risultato; **Scarta tutte le modifiche** (oppure Esc) riporta tutto com'era. L'intera sessione è un'unica operazione annullabile. L'immagine non viene mai tagliata: fare di nuovo clic su **Rimozione sfondo** per continuare a contrassegnare, oppure su **Reimposta immagine** per ripristinare l'intera immagine. Un'immagine molto grande mostra un riquadro con **Annulla** mentre JeffPub lavora. Le figure formate da linee e forme (SVG) e le immagini animate non possono avere lo sfondo rimosso. Un file .jpub conserva la rimozione, così può essere modificata di nuovo. **Reimposta immagine** annulla la rimozione eseguita qui; **Cambia immagine** non è disponibile mentre si contrassegna. Passare a un'altra pagina, eliminare l'immagine o salvare termina la Rimozione sfondo e mantiene ciò che è stato contrassegnato. Un file .pub, un PDF, una pagina web o un e-book contengono solo l'immagine finita, e Verifica compatibilità lo segnala per i file .pub. L'immagine finita viene salvata alla dimensione con cui è visualizzata, quindi può apparire morbida se un altro programma la ingrandisce; JeffPub la ricostruisce dall'originale ogni volta che si salva qui.
- **Correzioni**: scegliere una coppia di luminosità e contrasto, da meno 40 a più 40 per cento. **Formato oggetto…**, in fondo al menu, mostra i cursori.
- **Ricolora**: rende l'immagine in **Scala di grigi**, **Seppia**, **Sbiadito**, **Bianco e nero** oppure in una tinta d'enfasi. **Imposta colore trasparente** rende trasparente il colore presente nell'angolo in alto a sinistra dell'immagine.
- **Trasparenza…**: sfuma l'intera immagine.
- **Reimposta immagine**: annulla tutte le modifiche di questo elenco, nonché il bordo, la forma e gli effetti dell'immagine. Anche il ritaglio viene annullato, quindi la cornice mostra l'intera immagine. Fare clic sulla freccia accanto per **Reimposta immagine e dimensioni**, che riporta inoltre l'immagine alle dimensioni che ha nel suo file.
- **Comprimi immagini…**: riduce il file abbassando il dettaglio a 300, 220 o 96 ppi (pixel per pollice). Per impostazione predefinita modifica tutte le immagini ed elimina le aree ritagliate.
- **Converti in forme**: trasforma un'immagine SVG (un disegno formato da linee e forme) in forme modificabili, tagliate dove il disegno le ritaglia. Il testo e le immagini al suo interno non vengono inclusi.

## Stili, bordi ed effetti delle immagini

Usare il gruppo **Stili immagine** nella scheda **Formato immagine**:

- La **galleria** mostra circa due dozzine di aspetti predefiniti: cornici, ombreggiature, riflessi, bordi sfumati, forme arrotondate e ovali, una rotazione nello spazio, un alone e smussature. Una smussatura è un bordo inclinato che fa sembrare l'immagine in rilievo. Ogni riquadro mostra i propri colori. Fare clic su uno per applicarne l'aspetto all'immagine. Uno stile imposta insieme forma, bordo ed effetti, e sostituisce quelli che l'immagine aveva. Il ritaglio, le dimensioni e i colori restano invariati.
- **Bordo immagine** imposta il colore, lo **Spessore** (quanto è spessa la linea) e i **Tratteggi** (una linea continua, puntinata o tratteggiata). **Nessun contorno** rimuove il bordo.
- **Effetti immagine** apre **Ombreggiatura**, **Riflesso**, **Alone**, **Bordi sfumati**, **Smussatura** e **Rotazione 3D**. Ciascuno ha scelte predefinite. Sceglierne una per aggiungere l'effetto. Gli altri effetti restano. Scegliere la prima voce, come **Nessuna ombreggiatura**, per rimuoverne uno. **Altre opzioni effetti…** apre **Formato oggetto**.

Ogni operazione è annullabile con un solo passaggio.

## Didascalia, testo alternativo e scambio

- **Formato immagine > Stili immagine > Didascalia** aggiunge una casella di testo sotto l'immagine.
- Il testo alternativo è una breve descrizione per chi non può vedere l'immagine, ad esempio chi usa un lettore dello schermo. Fare clic con il pulsante destro sull'immagine, scegliere **Formato oggetto…**, aprire la scheda **Testo alternativo** e digitare in **Testo alternativo**.
- **Formato immagine > Inserisci > Cambia immagine** inserisce un altro file nella stessa cornice. La cornice mantiene dimensioni, posizione e aspetto: bordo, forma, effetti e modifiche ai colori. Se la nuova immagine ha la stessa forma di quella precedente, anche il ritaglio resta. Se ha una forma diversa, riempie la cornice.
- Per scambiare due immagini, trascinarne una sull'altra. Quando il suggerimento indica **Rilascia per scambiare le immagini**, rilasciare il pulsante: le immagini cambiano cornice, e la cornice trascinata torna al suo posto. Oppure tenere premuto Maiusc, fare clic su entrambe e fare clic su **Scambia**. Tenere premuto Ctrl mentre si trascina serve invece a copiare un'immagine. Una copia rilasciata sopra un'altra immagine resta dove è stata messa, e non avviene alcuno scambio.

## Vedere tutte le immagini

Fare clic su **Visualizza > Mostra > Gestione grafica** per elencare tutte le immagini nel riquadro **Gestione grafica**. Selezionarne una, quindi fare clic su **Vai a** per raggiungerla, su **Salva con nome…** per salvarne una copia, oppure su **Sostituisci…** per scegliere un altro file. Per le immagini collegate sono disponibili anche **Aggiorna collegamento**, **Cambia collegamento…** e **Incorpora immagine**. Vedere [Gestione grafica](graphics-manager).

## Argomenti correlati

- [Gestione grafica](graphics-manager)
- [Disegnare forme e linee](shapes)
- [Spostare, allineare, raggruppare e disporre su livelli gli oggetti](arrange)
- [La scheda Formato immagine](tab-picture-format)
