# Salvare come immagine, pagina Web o e-book
<!-- keywords: esporta, esportare, salva come immagine, png, jpeg, gif, tiff, svg, immagine, foto, pagina Web, html, sito, e-book, ebook, libro elettronico, epub, layout fisso, pagina per pagina, libro illustrato, crea pacchetto, pack and go, stampa fotografica, posta elettronica, email -->

Usare **File > Esporta** per trasformare la pubblicazione in un altro tipo di file. La pubblicazione stessa non viene modificata. Per i file PDF, vedere [File PDF e tipografie](pdf). Per i file .pub, vedere [Usare i file .pub](pub-files).

## Salvare le pagine come immagini

1. Fare clic su **File > Esporta**, quindi su **Salva come immagine**.
2. Digitare un nome. In **Salva come**, scegliere **PNG**, **JPEG**, **GIF**, **TIFF**, **Bitmap** o **Disegno vettoriale SVG**.
3. Fare clic su **Salva**.
4. Per tutti i tipi tranne SVG, scegliere una **Risoluzione (punti per pollice)**: 96, 150, 300 o 600. Più punti producono un'immagine più nitida e un file più grande. 300 è una buona scelta per la stampa.

Ogni pagina viene salvata in un file separato. Se la pubblicazione ha più pagine, JeffPub aggiunge al nome -1, -2 e così via.

Un file **SVG** è un disegno composto da linee e curve, quindi rimane nitido a qualsiasi dimensione. JeffPub converte le lettere in contorni, quindi il file ha lo stesso aspetto anche su un computer in cui non sono installati i tipi di carattere usati.

## Salvare come pagina Web

Fare clic su **File > Esporta**, quindi su **Salva come pagina Web**. JeffPub crea un unico file .html che si apre in qualsiasi browser Web. Ogni pagina della pubblicazione viene visualizzata come immagine, quindi le parole non possono essere selezionate né modificate. Ogni immagine è accompagnata da una versione testuale delle parole della pagina per le utilità di lettura dello schermo, e i collegamenti aggiunti continuano a funzionare.

## Salvare come e-book

Un file **EPUB** è il formato standard per gli e-book. Si apre con lettori di e-book, tablet e telefoni. È possibile scegliere tra due tipi:

- **Testo che scorre adattandosi allo schermo**: le parole vanno a capo in base allo schermo e alla dimensione del testo di ogni lettore, come in un romanzo. Le pagine non corrispondono una per una al layout.
- **Ogni pagina esattamente come progettata**: ogni pagina appare come in JeffPub, come in un libro illustrato, una rivista o un fumetto. Per leggere il testo piccolo i lettori aumentano lo zoom. Le parole restano sotto ogni pagina, quindi i lettori possono cercarle e le utilità di lettura dello schermo possono leggerle ad alta voce.

1. Fare clic su **File > Esporta**, quindi su **Salva come e-book (EPUB)**.
2. Scegliere il tipo e verificare **Titolo** e **Autore**.
3. Per il testo che scorre, lasciare selezionata l'opzione **Usa la prima pagina come copertina** oppure deselezionarla. (Un libro pagina per pagina usa sempre la prima pagina come copertina.)
4. Fare clic su **OK**, digitare un nome per il file e fare clic su **Salva**.

In un libro con testo che scorre, ogni paragrafo con lo stile Titolo 1 inizia un nuovo capitolo. I paragrafi Titolo 1, 2 e 3 formano il sommario in entrambi i tipi; un libro pagina per pagina elenca ogni titolo in corrispondenza della pagina in cui si trova. Le immagini e le tabelle restano al loro posto. Le note a piè di pagina vengono visualizzate in finestre a comparsa e le note di chiusura vengono inserite alla fine. Vedere [stili](styles) e [sommario, note a piè di pagina e note di chiusura](references).

## Crea pacchetto e stampa fotografica

- **Pack and Go: salva per una tipografia** chiede di indicare una cartella. Salva un PDF per la stampa commerciale (immagini a piena risoluzione, con segni di taglio e gli altri segni per la tipografia all'esterno della pagina), la pubblicazione e un breve file di testo con le dimensioni pagina e il numero di pagine. Se uno dei tre file non può essere salvato, ad esempio perché la cartella è piena o protetta, viene visualizzato un messaggio al posto della conferma.
- **Pack and Go: salva per un altro computer** crea un unico file ZIP contenente la pubblicazione e i tipi di carattere forniti con JeffPub che vi sono usati. Se il file ZIP non può essere salvato, viene visualizzato un messaggio. Le immagini collegate ai rispettivi file vengono memorizzate per intero nella pubblicazione. Un'immagine collegata contrassegnata come **Non aggiornata** include la piccola anteprima che mostra, perché JeffPub non ha aperto il relativo file.
- **Salva per la stampa fotografica** chiede di indicare una cartella e salva ogni pagina come JPEG a 300 dpi.

## Condividere per posta elettronica

Fare clic su **File > Condividi**. Scegliere **Invia pagina corrente per posta elettronica**, **Invia come allegato** o **Invia come PDF**. JeffPub non invia nulla: salva un file .eml, cioè un messaggio di posta elettronica da aprire nel programma di posta. Lì si aggiungono i destinatari.

## Salvare come modello

**File > Esporta > Salva come modello** aggiunge la pubblicazione a **Modelli personali**. Vedere [Iniziare una nuova pubblicazione](create).

## Argomenti correlati

- [File PDF e tipografie](pdf)
- [Stampare una pubblicazione](printing)
- [Usare i file .pub](pub-files)
- [Aprire e salvare le pubblicazioni](open-save)
