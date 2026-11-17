# Aggiungere e regolare le immagini
<!-- keywords: effetti immagine, ombreggiatura, riflesso, alone, bordi sfumati, smussatura, rotazione 3D, reimposta immagine e dimensioni, foto, immagine, inserisci immagine, ritaglia, clip art, pagina PDF, jpg, png, svg, ricolora, testo alternativo, didascalia, segnaposto, gestione grafica, immagini online, collega a file, immagine collegata, inserisci e collega, incorpora, immagine mancante, ricollega -->

Un'immagine si trova all'interno di una cornice. Puoi ritagliare l'immagine e spostare o ridimensionare la cornice.

## Inserire un'immagine da un file

1. Fare clic su **Inserisci > Illustrazioni > Immagini**.
2. Nella finestra **Inserisci immagine**, scegliere un file. JeffPub apre file PNG, JPEG, GIF, BMP, TIFF, WebP, SVG, WMF, EMF, ICO e PDF.
3. Scegliere **Inserisci**, **Collega a file** oppure **Inserisci e collega** nella casella **Inserisci come**, che si trova accanto all'elenco dei file e si raggiunge con il tasto Tab. Ogni volta che la finestra si apre è selezionato **Inserisci**. Vedere "Collegare un'immagine al suo file", più avanti.
4. Fare clic su **Apri**.

L'immagine viene inserita al centro della pagina. È anche possibile trascinare un file immagine sulla pagina; in questo caso si usa sempre **Inserisci**.

JeffPub legge file immagine fino a 256 MB. Un file più grande viene rifiutato, così come un file vuoto o qualsiasi elemento che non sia un file ordinario su disco, ad esempio un dispositivo o una pipe. Il messaggio indica che JeffPub non può leggerlo come immagine.

Se si scelgono più immagini insieme, vengono disposte come miniature nell'area di lavoro, lo spazio grigio accanto alla pagina. Trascinare sulla pagina quelle che si desiderano.

Se un PDF ha più di una pagina, la finestra **Inserisci pagina PDF** mostra ciascuna pagina. Sceglierne una e fare clic su **OK**.

## Collegare un'immagine al suo file

**Inserisci** copia l'immagine all'interno della pubblicazione. Questa copia va dove va la pubblicazione, ma una pubblicazione con molte foto grandi diventa a sua volta grande. Un collegamento lascia invece le foto nei rispettivi file.

- **Inserisci** copia l'immagine nella pubblicazione. È la scelta usuale.
- **Collega a file** conserva solo la posizione del file e un'anteprima ridotta, di non più di 512 pixel sul lato più lungo. L'immagine nella pagina, e nella stampa, in un PDF o in un e-book, viene presa dal file alla piena qualità. Se il file viene spostato o eliminato, la pagina mostra l'anteprima ridotta.
- **Inserisci e collega** fa entrambe le cose. La pubblicazione conserva l'immagine intera e il collegamento. Quando si apre la pubblicazione e il file è cambiato, JeffPub lo legge di nuovo e sostituisce la propria copia.

JeffPub ricorda la posizione completa del file e anche la sua posizione rispetto alla pubblicazione, ad esempio "la cartella Foto accanto a essa". Se si sposta la pubblicazione insieme alle sue immagini, vale la seconda indicazione, così le immagini vengono ritrovate. Mantenere le cartelle come sono.

JeffPub apre da solo il file di un'immagine collegata solo se il file si trova nella cartella della pubblicazione o in una cartella al suo interno. Un'immagine collegata a un file situato altrove mostra la sua anteprima salvata, sfocata, ed è contrassegnata come **Non aggiornata**, finché non la si seleziona e si fa clic su **Aggiorna collegamento** in Gestione grafica. Questo impedisce che una pubblicazione ricevuta da altri acceda al proprio computer. Per caricare le immagini automaticamente, tenerle nella cartella della pubblicazione.

**Gestione grafica** indica se ciascuna immagine è **Incorporata**, **Collegata**, **Mancante**, **Modificata** o **Non aggiornata**, e include pulsanti per aggiornare un collegamento, puntarlo a un altro file o trasformarlo in una copia. Vedere [Gestione grafica](graphics-manager).

Quando si salva una pubblicazione come file `.pub`, si crea un file Pack and Go o la si invia per posta elettronica, JeffPub inserisce per intero ciascuna immagine collegata nel file, così non dipende dai file che restano indietro. Se in quel momento un file collegato manca, oppure l'immagine è **Non aggiornata**, viene inserita la sua piccola anteprima. Un file `.pub` aperto non contiene mai collegamenti: le sue immagini vengono importate come copie.

## Aggiungere un segnaposto per immagine

Un segnaposto è una cornice vuota che riserva lo spazio per un'immagine. Fare clic su **Inserisci > Illustrazioni > Segnaposto immagine**, quindi fare doppio clic sulla cornice e scegliere un file. L'immagine riempie la cornice e i bordi che sporgono vengono nascosti. Un segnaposto vuoto non viene stampato.

## Trovare immagini gratuite online

Fare clic su **Inserisci > Illustrazioni > Immagini online**. Il riquadro **Immagini online** cerca in due librerie gratuite di immagini con licenza aperta: Openverse, che comprende molte raccolte, e Wikimedia Commons, la libreria delle immagini di Wikipedia.

1. Scegliere una libreria, digitare ciò che si cerca, ad esempio "fienile rosso", e premere Invio.
2. Fare clic su un'immagine per vedere chi l'ha realizzata e la sua licenza. La licenza è l'insieme delle regole per usare l'immagine. La maggior parte chiede di citare l'autore.
3. Fare clic su **Inserisci**. JeffPub mette l'immagine nella pagina e usa il suo titolo come testo alternativo. Con **Aggiungi un credito sotto l'immagine** selezionato, sotto di essa compare una riga come "Fienile rosso" di Pat Lee, CC BY 2.0.

Le parole della ricerca vengono inviate alla libreria scelta. Verificare che la licenza di ciascuna immagine sia compatibile con l'uso previsto.

Una ricerca mostra fino a 24 immagini. JeffPub usa solo indirizzi web sicuri (https) provenienti da una libreria ed esclude le immagini il cui indirizzo non lo è. Da Wikimedia Commons, un'immagine più larga di 1.920 pixel viene scaricata con quella larghezza, non nella versione originale caricata. Un download superiore a 50 MB, o che richiede troppo tempo, viene interrotto e il riquadro indica che l'immagine non può essere scaricata o caricata.

## Ritagliare un'immagine

Il ritaglio nasconde i bordi di un'immagine.

1. Fare clic sull'immagine, quindi su **Formato immagine > Ritaglia > Ritaglia**.
2. Trascinare una maniglia per nascondere parte dell'immagine. Trascinare l'immagine per farla scorrere all'interno della cornice.
3. Fare di nuovo clic su **Ritaglia**, oppure premere **Esc**.

Sempre nel gruppo **Ritaglia**: **Proporzioni** ritaglia la cornice in una forma come un quadrato, 4 di larghezza per 3 di altezza, oppure 16 per 9, intorno al suo centro (l'immagine resta dove si trova nella pagina); **Adatta** mostra l'intera immagine nella cornice; **Riempi** riempie la cornice; **Ritaglia nella forma** la rifila in una forma come un ovale; **Rimuovi ritaglio** mostra di nuovo l'intera immagine.

## Modificare l'aspetto di un'immagine

Nella scheda **Formato immagine**, usare il gruppo **Regola**:

- **Correzioni**: scegliere una coppia di luminosità e contrasto, da meno 40 a più 40 per cento. **Formato oggetto…**, in fondo al menu, mostra i cursori.
- **Ricolora**: rendere l'immagine Scala di grigi, Seppia, Sbiadito, Bianco e nero, oppure una tinta di colore di enfasi. **Imposta colore trasparente** rende trasparente il colore nell'angolo superiore sinistro dell'immagine.
- **Trasparenza…**: rendere sbiadita l'intera immagine.
- **Reimposta immagine**: annulla tutte le modifiche di questo elenco, oltre al bordo, alla forma e agli effetti dell'immagine. Si annulla anche il ritaglio, così la cornice mostra l'intera immagine. Fare clic sulla freccia accanto per **Reimposta immagine e dimensioni**, che riporta inoltre l'immagine alle dimensioni che ha nel suo file.
- **Comprimi immagini…**: riduce il file abbassando il dettaglio a 300, 220 o 96 ppi (pixel per pollice). Per impostazione predefinita modifica tutte le immagini ed elimina le aree ritagliate.
- **Converti in forme**: trasforma un'immagine SVG (un disegno fatto di linee e forme) in forme modificabili, tagliate dove il disegno le ritaglia. Il testo e le immagini al suo interno non vengono inclusi.

## Stili, bordi ed effetti dell'immagine

Usare il gruppo **Stili immagine** nella scheda **Formato immagine**:

- La **raccolta** mostra circa due dozzine di aspetti predefiniti: cornici, ombreggiature, riflessi, bordi sfumati, forme arrotondate e ovali, una rotazione nello spazio, un alone e smussature. Una smussatura è un bordo inclinato che fa sembrare l'immagine in rilievo. Ogni riquadro mostra i colori del tema. Fare clic su uno per dare all'immagine quell'aspetto. Uno stile imposta insieme forma, bordo ed effetti, e sostituisce quelli che l'immagine aveva. Il ritaglio, le dimensioni e i colori restano invariati.
- **Bordo immagine** imposta il colore, lo **Spessore** (quanto è spessa la linea) e i **Tratteggi** (linea continua, punteggiata o tratteggiata). **Nessun contorno** elimina il bordo.
- **Effetti immagine** apre **Ombreggiatura**, **Riflesso**, **Alone**, **Bordi sfumati**, **Smussatura** e **Rotazione 3D**. Ciascuno ha scelte predefinite. Sceglierne una per aggiungerla. Gli altri effetti restano. Scegliere la prima voce, come **Nessuna ombreggiatura**, per eliminarne uno. **Altre opzioni effetti…** apre **Formato oggetto**.

Ogni passaggio si annulla separatamente.

## Didascalia, testo alternativo e scambio

- **Formato immagine > Stili immagine > Didascalia** aggiunge una casella di testo sotto l'immagine.
- Il testo alternativo è una breve descrizione per chi non può vedere l'immagine, ad esempio per chi usa un lettore di schermo. Fare clic con il pulsante destro sull'immagine, scegliere **Formato oggetto…**, aprire la scheda **Testo alternativo** e digitare in **Testo alternativo**.
- **Formato immagine > Inserisci > Cambia immagine** mette un altro file nella stessa cornice. La cornice mantiene dimensioni, posizione e aspetto: bordo, forma, effetti e modifiche di colore. Se la nuova immagine ha la stessa forma di quella precedente, anche il ritaglio resta. Se ha una forma diversa, riempie la cornice.
- Per scambiare due immagini, trascinare una sull'altra. Quando la descrizione comando indica **Rilasciare per scambiare le immagini**, rilasciare il pulsante del mouse: le immagini cambiano cornice, e la cornice trascinata torna al suo posto. In alternativa, tenere premuto Maiusc, fare clic su entrambe e fare clic su **Scambia**. Tenere premuto Ctrl mentre si trascina copia invece l'immagine. Una copia rilasciata sopra un'altra immagine resta dove è stata messa, e non avviene alcuno scambio.

## Vedere tutte le immagini

Fare clic su **Visualizza > Mostra > Gestione grafica** per elencare tutte le immagini nel riquadro **Gestione grafica**. Selezionarne una, quindi fare clic su **Vai a** per raggiungerla, **Salva con nome…** per salvare una copia, oppure **Sostituisci…** per scegliere un altro file. Per le immagini collegate sono disponibili anche **Aggiorna collegamento**, **Cambia collegamento…** e **Incorpora immagine**. Vedere [Gestione grafica](graphics-manager).

## Argomenti correlati

- [Gestione grafica](graphics-manager)
- [Disegnare forme e linee](shapes)
- [Spostare, allineare, raggruppare e disporre su livelli gli oggetti](arrange)
- [La scheda Formato immagine](tab-picture-format)
