# File PDF e tipografie
<!-- keywords: pdf, crea pdf, salva come pdf, esporta pdf, pdf/x, pdf/a, archivio, archiviazione, tipografia, stampa commerciale, stampa professionale, cmyk, quadricromia, abbondanza, segni di taglio, crocini, profilo colore, tinta piatta, colore spot, sovrastampa -->

Un **PDF** è un file che appare identico su qualsiasi computer. Usarlo per inviare una pubblicazione per posta elettronica, pubblicarla online o mandarla in tipografia.

## Creare un PDF

1. Fare clic su **File > Esporta**, quindi su **Crea PDF**.
2. In **Ottimizza per** scegliere una qualità. **dpi** significa punti per pollice. Più punti producono immagini più nitide e file più grandi.
3. In **Pagine** scegliere **Tutte le pagine**, **Pagina corrente** oppure **Pagine da** una pagina **a** un'altra.
4. Fare clic su **Salva PDF…**, digitare un nome e fare clic su **Salva**.

| Scelta | Immagini | Ideale per |
|---|---|---|
| Dimensioni minime | 96 dpi | Visualizzazione sullo schermo |
| Standard | 150 dpi | Condivisione online |
| Stampa di alta qualità | 300 dpi | Stampanti desktop |
| Stampa commerciale | Risoluzione piena | Una tipografia. Aggiunge segni di taglio, segni di abbondanza, segni di registro, barre dei colori e informazioni sul lavoro. |

Altre scelte:

- **Includi proprietà documento** inserisce nel PDF titolo, autore, oggetto e parole chiave.
- **Apri il PDF dopo il salvataggio** mostra subito il PDF nel visualizzatore.
- **Fogli opuscolo** compare per gli opuscoli. Dispone due pagine su ogni lato del foglio nell'ordine di piegatura. Deselezionarlo per avere una pagina per ogni pagina del PDF.

## PDF/A per l'archiviazione

Il **PDF/A** è un PDF creato per durare negli anni. Contiene tutti i tipi di carattere e usa colori standard, senza effetti di trasparenza. Selezionare **PDF/A per l'archiviazione a lungo termine** oppure fare clic su **File > Esporta**, quindi su **Crea PDF/A per l'archiviazione**.

## PDF/X per una tipografia

Il **PDF/X** è un PDF più rigoroso, richiesto dalle tipografie. I colori sono espressi come quantità di inchiostro, i tipi di carattere sono incorporati e il formato rifilato e l'abbondanza sono indicati. L'**abbondanza** è la parte di immagine o di colore che si estende oltre il bordo della pagina, in modo che dopo il taglio non resti un bordo bianco.

1. Fare clic su **File > Esporta**, quindi su **Crea PDF/X per una tipografia**. In alternativa, selezionare **PDF/X per una tipografia commerciale** nella finestra **Crea PDF**. Non è possibile usarlo insieme al PDF/A.
2. In **Condizione di stampa** scegliere quella indicata dalla tipografia.
3. Se la tipografia richiede segni di taglio e abbondanza, scegliere anche **Stampa commerciale** in **Ottimizza per**.
4. Fare clic su **Salva PDF…**.

Le condizioni disponibili sono:

- **PDF/X-1a** per offset rotativa USA (SWOP) o offset europeo (FOGRA39). Appiattisce gli effetti di trasparenza.
- **PDF/X-4** per carta patinata premium (FOGRA51, PSO Coated v3) oppure con il profilo colore della stampante (un file .icc da scegliere). Mantiene gli effetti di trasparenza.

Il profilo colore PSO Coated v3 non è incluso in JeffPub. Al primo utilizzo, JeffPub chiede conferma prima di scaricare il profilo, una sola volta, da European Color Initiative.

Prima di creare il file, JeffPub verifica la presenza di immagini con risoluzione inferiore a 150 ppi (pixel per pollice) alle dimensioni di stampa e di tipi di carattere mancanti nel computer. Fare clic su **Crea comunque il PDF**, **Apri Verifica struttura** o **Annulla**.

## Impostare i colori per una tipografia

Fare clic su **File > Proprietà**, quindi sulla scheda **Impostazioni stampa commerciale**:

- **Modello colore** può essere **RGB**, **Colore singolo (tinta piatta)**, **Tinte piatte**, **Quadricromia (CMYK)** o **Quadricromia più tinte piatte**. La quadricromia usa inchiostri ciano, magenta, giallo e nero. Una tinta piatta è un inchiostro speciale preparato dalla tipografia. Aggiungere ciascuna tinta in **Tinte piatte**.
- **Sovrastampa il nero nei file per la tipografia (PDF/X)** fa stampare il nero sopra i colori sottostanti, in modo che una lastra fuori registro non lasci bordi bianchi. È possibile scegliere a cosa applicarla: testo sotto una certa dimensione, linee o riempimenti.

## Argomenti correlati

- [Stampare una pubblicazione](printing)
- [Salvare come immagine, pagina Web o e-book](export)
- [Verificare la struttura](design-checker)
