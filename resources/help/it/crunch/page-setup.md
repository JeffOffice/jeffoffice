# Finestra di dialogo Imposta pagina
<!-- keywords: imposta pagina, impostazioni pagina, finestra di dialogo, intestazione, piè di pagina, numeri di pagina, numerazione pagine, data, prima pagina, pari dispari, bianco e nero, bozza, note, errori, ordine delle pagine, centra nella pagina, numero prima pagina, codici intestazione e piè di pagina -->

Per aprirla, fai clic sulla piccola freccia nell'angolo del gruppo **Imposta pagina** nella scheda **Layout di pagina**. Contiene quattro schede. **OK** modifica il foglio corrente in un'unica operazione, che puoi annullare.

## Pagina

- **Orientamento della carta**: verticale o orizzontale.
- **Dimensioni di stampa delle celle**: una percentuale delle dimensioni reali, oppure **Riduci fino ad adattare**, con il numero di pagine in larghezza e in altezza. Con "qualsiasi numero di", la stampa in quella direzione occupa tutte le pagine necessarie.
- **Carta**: le dimensioni del foglio.
- **Inizia la numerazione delle pagine da**: il numero assegnato alla prima pagina. Se non lo specifichi, la prima pagina è la 1 e, quando stampi l'intera cartella di lavoro, la numerazione prosegue da un foglio al successivo.

## Margini

Digita lo spazio intorno alle celle e la distanza dell'intestazione e del piè di pagina dai bordi superiore e inferiore del foglio. Le caselle usano i centimetri se il computer è impostato su un'area geografica che usa il sistema metrico, altrimenti i pollici. **Orizzontalmente** e **Verticalmente** centrano le celle nella pagina.

## Intestazione e piè di pagina

L'**intestazione** è una riga di testo nella parte superiore di ogni pagina, il **piè di pagina** una riga nella parte inferiore. Scegline uno dall'elenco oppure scegli **Modifica intestazione** o **Modifica piè di pagina** per scriverne uno personalizzato. Ognuno ha una parte sinistra, una centrale e una destra, e pulsanti che inseriscono il numero di pagina, il numero totale di pagine, la data, l'ora, il nome del file, la cartella e il nome del foglio. Questi elementi cambiano da una pagina all'altra, quindi vengono calcolati al momento della stampa. Per inserire un'immagine in una parte, fai clic in quella parte e scegli **Immagine…**, quindi seleziona un file PNG, JPEG, GIF o BMP. L'immagine viene ridimensionata in modo che il lato più lungo non superi circa 5,1 cm e viene stampata nel punto in cui si trova il codice `&G`. **Rimuovi immagine** la elimina. Ogni parte può contenere una sola immagine; la prima pagina e le pagine pari possono avere immagini proprie.

Puoi anche impostare un'intestazione e un piè di pagina diversi per la prima pagina e per le pagine pari e dispari, e scegliere se il testo si ingrandisce e si riduce con le celle e se intestazione e piè di pagina si allineano ai margini.

Dietro i pulsanti ci sono brevi codici che vengono salvati nel file e che puoi digitare direttamente: **&P** è il numero di pagina, **&N** il numero totale di pagine, **&D** la data, **&T** l'ora, **&F** il nome del file, **&A** il nome del foglio e **&&** una e commerciale. **&B**, **&I** e **&U** attivano e disattivano grassetto, corsivo e sottolineato, **&12** imposta la dimensione a 12 punti e **&"Arial,Bold"** sceglie un tipo di carattere. **&L**, **&C** e **&R** iniziano rispettivamente la parte sinistra, centrale e destra.

## Foglio

- **Celle da stampare**: l'area di stampa, indicata come blocchi, ad esempio **A1:D20, F1:G9**. Lascia la casella vuota per stampare tutto.
- **Ripeti queste righe in alto** e **Ripeti queste colonne a sinistra**: i titoli di stampa, ad esempio **1:3** e **A:B**.
- **Stampa anche**: le linee tra le celle, i numeri di riga e le lettere di colonna, solo in bianco e nero (senza colori né riempimenti) e una bozza rapida (senza linee tra le celle e senza immagini, grafici o forme).
- **Note** e **Celle con errori**: come gestirle. L'impostazione per gli errori cambia ciò che viene stampato: il contenuto visualizzato nella cella, niente, due trattini o #N/D. Per le note: **Ometti** non stampa alcuna nota. **Elenca dopo l'ultima pagina** stampa ogni nota dopo l'ultima pagina del foglio, su pagine separate che proseguono la numerazione e riportano l'intestazione e il piè di pagina del foglio, nella forma "Cella A2: autore: testo della nota", nell'ordine delle celle. **Mostra nella posizione originale** viene conservato nel file per altri programmi, ma JeffCrunch non stampa ancora le note sulle celle.
- **Ordine delle pagine**: prima verso il basso e poi in orizzontale, oppure prima in orizzontale e poi verso il basso.
