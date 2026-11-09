# PDF e tipografie

<!-- keywords: pdf, crea pdf, salva come pdf, esporta pdf, pdf/x, pdf/a, archiviazione, archivio, tipografia, stampa commerciale, cmyk, abbondanza, segni di taglio, profilo colore, tinta piatta, sovrastampa, password, cifra, proteggi con password, documento pdf, stampa professionale -->

Un **PDF** è un file che appare uguale su qualsiasi computer. Usalo per inviare una pubblicazione via e-mail, pubblicarla online o inviarla a una tipografia.

## Creare un PDF

1. Fai clic su **File > Esporta**, quindi su **Crea PDF**.
2. In **Ottimizza per**, scegli una qualità. **dpi** indica i punti per pollice. Più punti significano immagini più nitide e file più grandi.
3. In **Pagine**, scegli **Tutte le pagine**, **Pagina corrente** oppure **Pagine da** una pagina **a** un'altra.
4. Fai clic su **Salva PDF…**, digita un nome e fai clic su **Salva**.

| Scelta | Immagini | Ideale per |
|---|---|---|
| Dimensioni minime | 96 dpi | Visualizzazione a schermo |
| Standard | 150 dpi | Condivisione online |
| Stampa di alta qualità | 300 dpi | Stampanti desktop |
| Stampa commerciale | Risoluzione piena | Una tipografia. Aggiunge segni di taglio, segni di abbondanza, segni di registro, barre dei colori e informazioni sul lavoro. |

Altre opzioni:

- **Includi proprietà documento** inserisce nel PDF titolo, autore, oggetto e parole chiave.
- **Apri il PDF dopo il salvataggio** mostra subito il PDF nel visualizzatore.
- **Fogli per opuscolo** compare per gli opuscoli. Inserisce due pagine per lato nell'ordine di piegatura. Deselezionala per avere una pagina per pagina PDF.

## PDF/A per conservare documenti

**PDF/A** è un PDF progettato per durare per anni. Contiene tutti i tipi di carattere e usa colori standard, senza effetti di trasparenza. Seleziona **PDF/A per l'archiviazione a lungo termine**, oppure fai clic su **File > Esporta**, quindi su **Crea PDF/A per l'archiviazione**.

## PDF/X per una tipografia

**PDF/X** è un PDF più rigoroso, richiesto dalle tipografie. I colori sono espressi come quantità di inchiostro, i tipi di carattere sono incorporati e il formato di taglio e l'abbondanza sono indicati. L'**abbondanza** è l'immagine o il colore in più che supera il bordo della pagina, così la rifilatura non lascia un bordo bianco.

1. Fai clic su **File > Esporta**, quindi su **Crea PDF/X per una tipografia**. Oppure seleziona **PDF/X per una tipografia commerciale** nella finestra **Crea PDF**. Non è possibile usarlo insieme a PDF/A.
2. In **Condizione di stampa**, scegli quella indicata dalla tipografia.
3. Se la tipografia richiede segni di taglio e abbondanza, scegli anche **Stampa commerciale** in **Ottimizza per**.
4. Fai clic su **Salva PDF…**.

Le condizioni disponibili sono:

- **PDF/X-1a** per offset rotativa USA (SWOP) o offset europeo (FOGRA39). Appiattisce gli effetti di trasparenza.
- **PDF/X-4** per carta patinata premium (FOGRA51, PSO Coated v3), oppure con il profilo colore della tua stampante (un file .icc che scegli tu). Mantiene gli effetti di trasparenza.

Il profilo colore PSO Coated v3 non è incluso in JeffPub. La prima volta che lo usi, JeffPub chiede conferma prima di scaricarlo, una sola volta, dall'European Color Initiative.

Prima di creare il file, JeffPub controlla se ci sono immagini sotto i 150 ppi (pixel per pollice) alla loro dimensione stampata e tipi di carattere mancanti su questo computer. Fai clic su **Crea comunque il PDF**, **Apri Verifica struttura** oppure **Annulla**.

## Proteggere un PDF con una password

Nella finestra **Crea PDF**, fai clic su **Cifra con una password…**. Digita due volte la password. Chiunque apra il PDF dovrà quindi digitarla. La password protegge davvero il file: le pagine sono cifrate con crittografia robusta (AES-256) e nessuno può leggerle senza di essa.

Puoi anche selezionare **Limita stampa, copia e modifica**, inserire una seconda password e scegliere cosa è consentito. Queste limitazioni sono solo una richiesta. La maggior parte dei lettori PDF le rispetta, ma non impediscono a chi lo voglia di aggirarle, quindi non contare su di esse per tenere riservato un file.

Una password dimenticata non può essere recuperata, quindi conservala in un posto sicuro. La password serve solo per quel PDF e non viene mai salvata. Un PDF con password non può essere PDF/A o PDF/X, quindi questi due formati sono disattivati mentre è impostata una password. In **File > Condividi**, **Cifra con una password…** sopra **Invia come PDF** applica una password al PDF da allegare a un messaggio e-mail.

Quando inserisci come immagine un PDF protetto da password, JeffPub chiede la password. Se è errata, JeffPub la richiede di nuovo; fai clic su **Annulla** per interrompere. La pagina nella tua pubblicazione è una copia e non richiede password. Se il proprietario del PDF ha chiesto che non venga copiato, JeffPub lo inserisce comunque e mostra una nota che lo segnala; quella richiesta non è un blocco.

Un'immagine collegata a un PDF protetto da password compare come **Non aggiornata** quando apri la pubblicazione, e JeffPub non chiede mai la password da solo. Fai clic su **Aggiorna collegamento** per essere interpellato.

## Impostare i colori per una tipografia

Fai clic su **File > Informazioni**, quindi sulla scheda **Impostazioni di stampa commerciale**:

- **Modello colore** può essere **RGB**, **Colore singolo (tinta piatta)**, **Tinte piatte**, **Quadricromia (CMYK)** oppure **Quadricromia più tinte piatte**. La quadricromia usa gli inchiostri ciano, magenta, giallo e nero. Una tinta piatta è un inchiostro speciale miscelato dalla tipografia. Aggiungine una per volta in **Tinte piatte**.
- **Sovrastampa il nero nei file per la tipografia (PDF/X)** fa sì che il nero stampi sopra i colori sottostanti, così un registro leggermente spostato non lascia un bordo bianco. Scegli cosa coprire: testo sotto una certa dimensione, linee o riempimenti.

## Vedi anche

- [Stampare una pubblicazione](printing)
- [Salvare come immagine, pagina web o e-book](export)
- [Verificare la struttura](design-checker)
