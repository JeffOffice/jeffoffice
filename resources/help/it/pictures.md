# Aggiungere e regolare immagini
<!-- keywords: foto, fotografia, immagine, inserisci immagine, ritaglia, ritagliare, clip art, pagina PDF, jpg, png, svg, ricolora, testo alternativo, didascalia, segnaposto, gestione grafica, immagini online, collega al file, immagine collegata, inserisci e collega, incorpora, immagine mancante, ricollega -->

Un'immagine è contenuta in una cornice. È possibile ritagliare l'immagine e spostare o ridimensionare la cornice.

## Inserire un'immagine da un file

1. Fare clic su **Inserisci > Illustrazioni > Immagini**.
2. Nella finestra **Inserisci immagine** scegliere un file. JeffPub apre file PNG, JPEG, GIF, BMP, TIFF, WebP, SVG, WMF, EMF, ICO e PDF.
3. Scegliere **Inserisci**, **Collega al file** o **Inserisci e collega** nella casella **Inserisci come**, che si trova accanto all'elenco dei file ed è raggiungibile con il tasto TAB. A ogni apertura della finestra è selezionato **Inserisci**. Vedere "Collegare un'immagine al relativo file" più avanti.
4. Fare clic su **Apri**.

L'immagine viene posizionata al centro della pagina. È anche possibile trascinare un file immagine sulla pagina; in questo caso viene sempre usato **Inserisci**.

JeffPub legge file immagine fino a 256 MB. Un file più grande viene rifiutato, così come un file vuoto o qualsiasi elemento che non sia un normale file su disco, ad esempio un dispositivo o una pipe. Il messaggio indica che JeffPub non è in grado di leggerlo come immagine.

Se si scelgono più immagini contemporaneamente, vengono disposte come anteprime nell'area di lavoro, lo spazio grigio accanto alla pagina. Trascinare sulla pagina quelle desiderate.

Se un PDF contiene più pagine, la finestra **Inserisci pagina PDF** mostra ogni pagina. Sceglierne una e fare clic su **OK**.

## Collegare un'immagine al relativo file

**Inserisci** inserisce una copia dell'immagine nella pubblicazione. La copia segue sempre la pubblicazione, ma una pubblicazione con molte foto di grandi dimensioni diventa a sua volta grande. Un collegamento, invece, mantiene le foto nei rispettivi file.

- **Inserisci** copia l'immagine nella pubblicazione. È la scelta abituale.
- **Collega al file** conserva solo il percorso del file e una piccola anteprima, di non più di 512 pixel sul lato più lungo. L'immagine nella pagina, così come in una stampa, in un PDF o in un e-book, viene letta dal file alla qualità massima. Se il file viene spostato o eliminato, nella pagina viene invece mostrata la piccola anteprima.
- **Inserisci e collega** esegue entrambe le operazioni. La pubblicazione conserva l'immagine completa e il collegamento. Quando si apre la pubblicazione e il file è stato modificato, JeffPub lo rilegge e sostituisce la propria copia.

JeffPub memorizza il percorso completo del file e anche la sua posizione rispetto alla pubblicazione, ad esempio "la cartella Foto accanto alla pubblicazione". Se si spostano insieme la pubblicazione e le relative immagini, prevale la seconda informazione e le immagini vengono ritrovate. Mantenere invariata la struttura delle cartelle.

JeffPub apre automaticamente il file di un'immagine collegata solo se si trova nella cartella della pubblicazione o in una sua sottocartella. Un'immagine collegata a un file in qualsiasi altra posizione mostra l'anteprima salvata, sfocata, ed è contrassegnata come **Non aggiornata**, finché non la si seleziona e si fa clic su **Aggiorna collegamento** in Gestione grafica. In questo modo una pubblicazione ricevuta da altri non può accedere al contenuto del computer. Per caricare le immagini automaticamente, conservarle nella cartella della pubblicazione.

Il riquadro **Gestione grafica** indica se ogni immagine è **Incorporata**, **Collegata**, **Mancante**, **Modificata** o **Non aggiornata** e include pulsanti per aggiornare un collegamento, indirizzarlo a un altro file o trasformarlo in una copia. Vedere [Gestione grafica](graphics-manager).

Quando si salva una pubblicazione come file `.pub`, si crea un file Pack and Go o la si invia per posta elettronica, JeffPub include per intero ogni immagine collegata, in modo che il file non dipenda da file rimasti altrove. Se in quel momento un file collegato manca, oppure l'immagine è **Non aggiornata**, viene inclusa la sua piccola anteprima. Un file `.pub` aperto non contiene mai collegamenti: le sue immagini vengono importate come copie.

## Aggiungere un segnaposto immagine

Un segnaposto è una cornice vuota che riserva uno spazio per un'immagine. Fare clic su **Inserisci > Illustrazioni > Segnaposto immagine**, quindi fare doppio clic sulla cornice e scegliere un file. L'immagine riempie la cornice e i bordi in eccesso vengono nascosti. Un segnaposto vuoto non viene stampato.

## Trovare immagini gratuite online

Fare clic su **Inserisci > Illustrazioni > Immagini online**. Il riquadro **Immagini online** esegue ricerche in due raccolte gratuite di immagini con licenza aperta: Openverse, che comprende molte collezioni, e Wikimedia Commons, la raccolta delle immagini di Wikipedia.

1. Scegliere una raccolta, digitare ciò che si cerca, ad esempio "fienile rosso", e premere INVIO.
2. Fare clic su un'immagine per visualizzarne l'autore e la licenza. La licenza è l'insieme di regole per l'uso dell'immagine. La maggior parte richiede di citare l'autore.
3. Fare clic su **Inserisci**. JeffPub inserisce l'immagine nella pagina e ne usa il titolo come testo alternativo. Se è selezionata l'opzione **Aggiungi i crediti sotto l'immagine**, sotto l'immagine viene aggiunta una riga come "Red barn" di Pat Lee, CC BY 2.0.

Le parole cercate vengono inviate alla raccolta scelta. Verificare che la licenza di ogni immagine sia compatibile con l'uso previsto.

Una ricerca mostra al massimo 24 immagini. JeffPub usa solo indirizzi Web sicuri (https) di una raccolta ed esclude le immagini il cui indirizzo non lo è. Da Wikimedia Commons, un'immagine più larga di 1.920 pixel viene scaricata come copia di quella larghezza, non come file originale. Un download superiore a 50 MB, o che richiede troppo tempo, viene interrotto e il riquadro indica che non è stato possibile scaricare o caricare l'immagine.

## Ritagliare un'immagine

Il ritaglio nasconde i bordi di un'immagine.

1. Fare clic sull'immagine, quindi su **Formato immagine > Ritaglia > Ritaglia**.
2. Trascinare un quadratino per nascondere parte dell'immagine. Trascinare l'immagine per farla scorrere all'interno della cornice.
3. Fare di nuovo clic su **Ritaglia** oppure premere **ESC**.

Sempre nel gruppo **Ritaglia**: **Adatta** mostra l'intera immagine nella cornice, **Riempimento** riempie la cornice, **Ritaglia a forma** ritaglia l'immagine secondo una forma, ad esempio un ovale, e **Cancella ritaglio** mostra di nuovo l'immagine intera.

## Cambiare l'aspetto di un'immagine

Nella scheda **Formato immagine** usare il gruppo **Regola**:

- **Correzioni**: scegliere una combinazione di luminosità e contrasto, da meno 40 a più 40 percento. **Formato oggetto…**, in fondo al menu, offre dei dispositivi di scorrimento.
- **Ricolora**: rende l'immagine in Scala di grigi, Seppia, Sbiadito, Bianco e nero o in una tinta di un colore. **Imposta colore trasparente** rende trasparente il colore presente nell'angolo superiore sinistro dell'immagine.
- **Trasparenza…**: rende più tenue l'intera immagine.
- **Reimposta immagine**: annulla queste modifiche.
- **Comprimi immagini…**: riduce le dimensioni del file diminuendo il dettaglio a 300, 220 o 96 ppi (pixel per pollice). Per impostazione predefinita modifica tutte le immagini ed elimina le aree ritagliate.
- **Converti in forme**: trasforma un'immagine SVG (un disegno composto da linee e forme) in forme modificabili, tagliate dove il disegno le ritaglia. Il testo e le immagini contenuti vengono esclusi.

## Didascalia, testo alternativo e scambio

- **Formato immagine > Stili immagine > Didascalia** aggiunge una casella di testo sotto l'immagine.
- Il testo alternativo è una breve descrizione per chi non può vedere l'immagine, ad esempio chi usa un'utilità per la lettura dello schermo. Fare clic con il pulsante destro del mouse sull'immagine, scegliere **Formato oggetto…**, aprire la scheda **Testo alternativo** e digitare il testo in **Testo alternativo:**.
- **Formato immagine > Inserisci > Cambia immagine** inserisce un altro file nella stessa cornice.
- Per scambiare due immagini, trascinarne una sull'altra. Quando viene visualizzato il suggerimento **Rilascia per scambiare le immagini**, rilasciare il pulsante del mouse: le immagini si scambiano le cornici e la cornice trascinata torna nella posizione originale. In alternativa, tenere premuto MAIUSC, fare clic su entrambe le immagini e quindi su **Scambia**. Per copiare un'immagine, tenere premuto CTRL durante il trascinamento. Una copia rilasciata sopra un'altra immagine rimane dove è stata posizionata e non avviene alcuno scambio.

## Visualizzare tutte le immagini

Fare clic su **Visualizza > Mostra > Gestione grafica** per elencare tutte le immagini nel riquadro **Gestione grafica**. Selezionarne una, quindi fare clic su **Vai a** per raggiungerla, su **Salva con nome…** per salvarne una copia oppure su **Sostituisci…** per scegliere un altro file. Per le immagini collegate sono disponibili anche **Aggiorna collegamento**, **Cambia collegamento…** e **Incorpora immagine**. Vedere [Gestione grafica](graphics-manager).

## Argomenti correlati

- [Gestione grafica](graphics-manager)
- [Disegnare forme e linee](shapes)
- [Spostare, allineare, raggruppare e disporre su livelli gli oggetti](arrange)
- [La scheda Formato immagine](tab-picture-format)
