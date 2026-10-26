# Finestra di dialogo Imposta pagina
<!-- keywords: imposta pagina, impostazione pagina, finestra di dialogo, intestazione, piè di pagina, numeri di pagina, numerazione pagine, data, prima pagina, pagine pari e dispari, bianco e nero, bozza, note, errori, ordine delle pagine, centra nella pagina, numero prima pagina, codici intestazione e piè di pagina -->

Per aprirla, fai clic sulla piccola freccia nell'angolo del gruppo **Imposta pagina** nella scheda **Layout di pagina**. La finestra ha quattro schede. **OK** applica le modifiche al foglio corrente, come un'unica operazione che puoi annullare.

## Pagina

- **Orientamento della carta**: verticale o orizzontale.
- **Dimensioni di stampa delle celle**: una percentuale delle dimensioni reali, oppure **Riduci fino ad adattare**, indicando il numero di pagine in larghezza e in altezza. "Qualsiasi numero di" consente di usare in quella direzione tutte le pagine necessarie.
- **Carta**: le dimensioni del foglio.
- **Inizia la numerazione delle pagine da**: il numero assegnato alla prima pagina. Se non lo indichi, la prima pagina è la 1 e, quando stampi l'intera cartella di lavoro, la numerazione prosegue da un foglio all'altro.

## Margini

Digita lo spazio intorno alle celle e la distanza dell'intestazione e del piè di pagina dai bordi superiore e inferiore del foglio. Le caselle usano i centimetri se il computer è impostato su un paese che usa il sistema metrico, altrimenti i pollici. **Orizzontalmente** e **Verticalmente** centrano le celle nella pagina.

## Intestazione e piè di pagina

L'**intestazione** è una riga di testo nella parte superiore di ogni pagina, il **piè di pagina** una riga nella parte inferiore. Scegline uno dall'elenco oppure scegli **Modifica intestazione** o **Modifica piè di pagina** per scriverne uno personalizzato. Ognuno è diviso in una parte sinistra, una centrale e una destra, e dispone di pulsanti per inserire il numero di pagina, il numero totale di pagine, la data, l'ora, il nome del file, la cartella e il nome del foglio. Poiché questi elementi cambiano da una pagina all'altra, vengono calcolati al momento della stampa. Per inserire un'immagine in una parte, fai clic in quella parte e scegli **Immagine…**, quindi seleziona un file PNG, JPEG, GIF o BMP. L'immagine viene ridimensionata a non più di 5 cm sul lato più lungo e viene stampata nel punto in cui si trova il codice `&G`. **Rimuovi immagine** la elimina. Ogni parte può contenere una sola immagine, e la prima pagina e le pagine pari possono avere immagini proprie.

Puoi anche impostare un'intestazione e un piè di pagina diversi per la prima pagina e per le pagine pari e dispari, e scegliere se il testo si ingrandisce e si riduce con le celle e se l'intestazione e il piè di pagina si allineano ai margini.

Dietro i pulsanti ci sono brevi codici salvati nel file, che puoi anche digitare direttamente: **&P** è il numero di pagina, **&N** il numero totale di pagine, **&D** la data, **&T** l'ora, **&F** il nome del file, **&A** il nome del foglio e **&&** una e commerciale. **&B**, **&I** e **&U** attivano e disattivano grassetto, corsivo e sottolineatura, **&12** imposta la dimensione a 12 punti e **&"Arial,Bold"** sceglie un tipo di carattere. **&L**, **&C** e **&R** iniziano rispettivamente la parte sinistra, centrale e destra.

## Foglio

- **Celle da stampare**: l'area di stampa, come blocchi di celle, ad esempio **A1:D20, F1:G9**. Lascia la casella vuota per stampare tutto.
- **Ripeti queste righe in alto** e **Ripeti queste colonne a sinistra**: i titoli di stampa, ad esempio **1:3** e **A:B**.
- **Stampa anche**: le linee tra le celle, i numeri di riga e le lettere di colonna, solo bianco e nero (senza colori né riempimenti) e una bozza rapida (senza linee tra le celle e senza immagini, grafici o forme).
- **Note** e **Celle con errori**: come trattarle. L'impostazione per gli errori modifica ciò che appare nella pagina: il contenuto visualizzato nella cella, niente, due trattini oppure #N/D. Le note vengono conservate nel file per altri programmi, ma JeffCrunch per ora non le legge né le stampa.
- **Ordine delle pagine**: prima verso il basso e poi in orizzontale, oppure prima in orizzontale e poi verso il basso.
