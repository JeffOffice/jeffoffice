# Verifica la struttura

<!-- keywords: verifica struttura, verifica, errori, problemi, bassa risoluzione, overflow, testo non adatto, casella di testo vuota, fuori dalla pagina, controllo preliminare, verifica finale, prova, immagine collegata, carattere mancante, controllo, testo in eccesso, collegamento, carattere non installato -->

La **Verifica struttura** esamina la pubblicazione alla ricerca di errori frequenti. Trova testo che non entra nello spazio disponibile, caselle vuote, oggetti che escono dalla pagina e immagini che verrebbero stampate male. Eseguila prima di stampare o di inviare un file alla tipografia.

## Eseguire la Verifica struttura

Fai clic su **Revisione > Verifica > Esegui Verifica struttura**. Il riquadro **Verifica struttura** si apre a destra. Puoi anche fare clic su **File > Informazioni**, quindi su **Esegui Verifica struttura**.

Il riquadro contiene due caselle di controllo, entrambe attive all'inizio:

- **Esegui verifiche generali della struttura** cerca problemi di impaginazione.
- **Esegui verifiche finali di pubblicazione** cerca elementi importanti in fase di stampa.

Lasciale entrambe attive per vedere tutti i tipi di problemi. L'elenco si aggiorna mentre lavori. Se non c'è nulla da correggere, appare il messaggio "Nessun problema rilevato".

## Correggere un problema

1. Fai clic su una voce nell'elenco in **Selezionare un elemento da correggere**.
2. Fai clic su **Vai all'elemento**. Oppure fai doppio clic sulla voce.

JeffPub passa alla pagina e seleziona l'oggetto. Correggilo e la voce scompare dall'elenco.

## Problemi rilevati

| Messaggio | Significato | Cosa fare |
|---|---|---|
| La pagina non ha contenuto | La pagina è vuota. | Aggiungi qualcosa oppure elimina la pagina. |
| L'oggetto è parzialmente fuori dalla pagina | Una parte dell'oggetto si trova fuori dalla pagina. | Spostalo, oppure lascialo lì se è un'abbondanza. |
| La casella di testo è vuota | Una casella di testo non contiene testo. | Digita al suo interno oppure eliminala. |
| Articolo con testo nell'area di overflow | Un articolo è il testo che scorre attraverso caselle collegate. Questo ha più testo di quanto ne entri. | Ingrandisci una casella, riduci il testo oppure [collega un'altra casella](linked-text). |
| Il testo è stato ridotto notevolmente per adattarlo | Ridurre il testo per farlo entrare nella casella lo ha reso meno della metà della dimensione originale. | Ingrandisci la casella oppure usa meno testo. |
| Il segnaposto immagine è vuoto | Una cornice per immagini non contiene un'immagine. | Fai clic per aggiungerne una oppure eliminala. |
| L'immagine non è ridimensionata in modo proporzionale | L'immagine è stirata o compressa. | Ridimensionala per ripristinarne le proporzioni. |
| L'immagine è collegata, non incorporata | L'immagine è legata a un file e nella pubblicazione è memorizzata solo una piccola anteprima. Un'immagine inserita con **Inserisci e collega** viene memorizzata per intero e non compare nell'elenco. | Conserva il file originale insieme alla pubblicazione, oppure fai clic su **Incorpora immagine** in [Gestione grafica](graphics-manager). |
| Immagine collegata mancante | Il file a cui punta un'immagine collegata non esiste più, quindi la pagina mostra solo la sua piccola anteprima, che è ciò che verrebbe stampato. | In Gestione grafica, fai clic su **Cambia collegamento…** per trovare il file, oppure ripristinalo e fai clic su **Aggiorna collegamento**. |
| L'immagine collegata mostra solo l'anteprima fino ad Aggiorna collegamento | L'immagine è collegata a un file al di fuori della cartella della pubblicazione, che JeffPub non apre automaticamente. La pagina mostra la piccola anteprima, che è ciò che verrebbe stampato. | In Gestione grafica, seleziona l'immagine e fai clic su **Aggiorna collegamento**. |
| L'immagine ha una risoluzione bassa | L'immagine ha meno di 150 ppi (pixel per pollice) alla dimensione di stampa. | Usa un'immagine più grande oppure stampala più piccola. |
| L'oggetto ha trasparenza o effetti (la stampa potrebbe essere diversa) | Le aree trasparenti e gli effetti possono apparire diversi su carta. | Stampa una pagina di prova. |
| Il carattere "nome" non è installato | Un carattere usato nel testo non è presente su questo computer, quindi ne viene usato un altro al suo posto. | Installa il carattere oppure scegline un altro. |

Ogni messaggio termina con il numero di pagina, ad esempio "(Pagina 2)".

## Prima di un file per la tipografia

Quando crei un file PDF/X, JeffPub ti avvisa di immagini a bassa risoluzione e caratteri mancanti e offre di aprire la Verifica struttura. Vedi [File PDF e tipografie](pdf).

## Argomenti correlati

- [File PDF e tipografie](pdf)
- [Stampare una pubblicazione](printing)
- [Collegare il testo tra caselle di testo](linked-text)
- [Aggiungere e regolare le immagini](pictures)
- [Gestione grafica](graphics-manager)
