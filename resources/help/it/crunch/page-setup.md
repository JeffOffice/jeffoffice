# La finestra di dialogo Imposta pagina
<!-- keywords: imposta pagina, finestra di dialogo, intestazione, piè di pagina, numeri di pagina, data, prima pagina, pari dispari, bianco e nero, bozza, note, errori, ordine delle pagine, centra nella pagina, numero della prima pagina, codici intestazione e piè di pagina, qualità di stampa, dpi, punti per pollice -->

Aprila dalla piccola freccia nell'angolo del gruppo **Imposta pagina** nella scheda **Layout di pagina**. Ha quattro schede. **OK** modifica il foglio su cui stai lavorando, come un'unica operazione che puoi annullare.

## Pagina

- **Orientamento della carta**: verticale o orizzontale.
- **Come ridurre le celle nella stampa**: una percentuale delle dimensioni reali, oppure **Riduci fino ad adattare**, con il numero di pagine in larghezza e in altezza. "Qualsiasi numero di" lascia che quella direzione occupi tutte le pagine necessarie.
- **Carta**: il formato della carta.
- **Qualità di stampa**: il numero di punti per pollice richiesto alla stampante. Più punti danno una stampa più fine, ma alcune stampanti impiegano più tempo. Se una cartella di lavoro richiede un numero che non è nell'elenco, JeffCrunch lo mantiene.
- **Inizia la numerazione delle pagine da**: il numero assegnato alla prima pagina. Se non è impostato, la prima pagina è la 1, e una cartella di lavoro stampata per intero numera le pagine in progressione da un foglio al successivo.

## Margini

Digita lo spazio intorno alle celle e la distanza di intestazione e piè di pagina dai bordi superiore e inferiore della carta. Le caselle usano i centimetri se il computer è impostato su una regione metrica, altrimenti i pollici. **Orizzontalmente** e **Verticalmente** centrano le celle nella pagina.

## Intestazione e piè di pagina

Un'**intestazione** è una riga di testo in alto in ogni pagina, mentre un **piè di pagina** è quella in basso. Scegline una dall'elenco, oppure scegli **Modifica intestazione** o **Modifica piè di pagina** per scrivere la tua. Ognuna ha una parte sinistra, una centrale e una destra, e pulsanti che inseriscono il numero di pagina, il numero di pagine, la data, l'ora, il nome del file, la cartella e il nome del foglio. Questi elementi cambiano da una pagina all'altra, quindi vengono calcolati al momento della stampa. Per inserire un'immagine in una parte, fai clic in quella parte e scegli **Immagine…**, quindi seleziona un file PNG, JPEG, GIF o BMP. L'immagine viene ridotta a non più di 5,1 cm sul lato più lungo e viene stampata dove si trova il codice `&G`. **Rimuovi immagine** la elimina. Ogni parte contiene una sola immagine, e la prima pagina e le pagine pari possono avere ciascuna immagini proprie.

Puoi anche impostare un'intestazione e un piè di pagina diversi per la prima pagina e per le pagine dispari e pari, e scegliere se il testo si ingrandisce e si riduce con le celle e se intestazione e piè di pagina si allineano ai margini.

Dietro ai pulsanti ci sono brevi codici che il file conserva, e puoi digitarli tu stesso: **&P** è il numero di pagina, **&N** il numero di pagine, **&D** la data, **&T** l'ora, **&F** il nome del file, **&A** il nome del foglio, e **&&** è una e commerciale. **&B**, **&I** e **&U** attivano e disattivano grassetto, corsivo e sottolineato, **&12** imposta la dimensione a 12 punti, e **&"Arial,Bold"** sceglie un tipo di carattere. **&L**, **&C** e **&R** iniziano la parte sinistra, centrale e destra.

## Foglio

- **Celle da stampare**: l'area di stampa, come blocchi del tipo **A1:D20, F1:G9**. Lasciala vuota per stampare tutto.
- **Ripeti queste righe in alto** e **Ripeti queste colonne a sinistra**: i titoli di stampa, come **1:3** e **A:B**.
- **Stampa anche**: le linee tra le celle, i numeri di riga e le lettere di colonna, solo in bianco e nero (senza colori né riempimenti), e una bozza rapida (senza linee tra le celle e senza immagini, grafici o forme).
- **Note** e **Celle con errori**: che cosa farne. L'impostazione per gli errori cambia ciò che la pagina mostra: come lo mostra la cella, nulla, due trattini o #N/D. Note: **Escludi** non stampa alcuna nota. **Elenca dopo l'ultima pagina** stampa ogni nota dopo l'ultima pagina del foglio, su pagine proprie che proseguono la numerazione e hanno l'intestazione e il piè di pagina del foglio, nel formato "Cella A2: l'autore: il testo della nota", nell'ordine delle celle. **Mostra dove si trovano** viene conservata nel file per altri programmi, ma JeffCrunch non stampa ancora le note sulle celle.
- **Quale pagina viene dopo**: dall'alto in basso e poi da sinistra a destra, oppure da sinistra a destra e poi dall'alto in basso.
