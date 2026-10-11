# Verificare la struttura
<!-- keywords: verifica struttura, verifica, controllo, errori, problemi, bassa risoluzione, testo in eccesso, testo non entra, casella di testo vuota, fuori dalla pagina, verifica preliminare, preflight, controllo finale, bozza, immagine collegata, tipo di carattere mancante -->

**Verifica struttura** esamina la pubblicazione alla ricerca degli errori più comuni. Individua il testo che non entra nella casella, le caselle vuote, gli oggetti che sporgono dalla pagina e le immagini che verrebbero stampate male. Eseguirla prima di stampare o di inviare un file in tipografia.

## Eseguire Verifica struttura

Fare clic su **Revisione > Verifica > Esegui verifica struttura**. Sulla destra si apre il riquadro **Verifica struttura**. In alternativa, fare clic su **File > Proprietà**, quindi su **Esegui verifica struttura**.

Il riquadro contiene due caselle di controllo, entrambe selezionate inizialmente:

- **Esegui verifiche generali della struttura** cerca problemi di layout.
- **Esegui verifiche finali di pubblicazione** cerca i problemi rilevanti per la stampa.

Lasciarle selezionate entrambe per visualizzare tutti i tipi di problema. L'elenco si aggiorna mentre si lavora. Se non c'è nulla da correggere, viene visualizzato il messaggio "Nessun problema rilevato."

## Correggere un problema

1. Fare clic su un elemento dell'elenco in **Selezionare un elemento da correggere**.
2. Fare clic su **Vai all'elemento** oppure fare doppio clic sull'elemento.

JeffPub passa alla pagina e seleziona l'oggetto. Una volta corretto il problema, l'elemento scompare dall'elenco.

## Problemi rilevati

| Messaggio | Significato | Cosa fare |
|---|---|---|
| La pagina non ha contenuto | La pagina è vuota. | Aggiungere un contenuto o eliminare la pagina. |
| L'oggetto è parzialmente fuori dalla pagina | Una parte di un oggetto si trova fuori dalla pagina. | Spostarlo oppure lasciarlo dov'è se si tratta di un'abbondanza. |
| La casella di testo è vuota | Una casella di testo non contiene testo. | Digitare del testo o eliminarla. |
| Articolo con testo in eccesso | Un articolo è il testo che scorre attraverso caselle collegate. Questo contiene più testo di quanto ne possa entrare. | Ingrandire una casella, ridurre il testo o [collegare un'altra casella](linked-text). |
| Il testo è stato ridotto notevolmente per adattarlo | L'adattamento automatico, che riduce il testo per farlo entrare nella casella, lo ha ridotto a meno della metà delle sue dimensioni. | Ingrandire la casella o ridurre il testo. |
| Il segnaposto immagine è vuoto | Una cornice per immagine non contiene immagini. | Fare clic sulla cornice per aggiungerne una o eliminarla. |
| L'immagine non è ridimensionata in modo proporzionale | L'immagine è allungata o schiacciata. | Ridimensionarla per ripristinarne le proporzioni. |
| L'immagine è collegata, non incorporata | L'immagine è legata a un file e nella pubblicazione è memorizzata solo una piccola anteprima. Un'immagine inserita con **Inserisci e collega** viene memorizzata per intero e non compare nell'elenco. | Conservare il file originale insieme alla pubblicazione oppure fare clic su **Incorpora immagine** in [Gestione grafica](graphics-manager). |
| Immagine collegata mancante | Il file a cui punta un'immagine collegata non esiste più, quindi la pagina mostra solo la piccola anteprima, che è ciò che verrebbe stampato. | In Gestione grafica, fare clic su **Cambia collegamento…** per trovare il file, oppure rimetterlo al suo posto e fare clic su **Aggiorna collegamento**. |
| L'immagine collegata mostra solo l'anteprima fino ad Aggiorna collegamento | L'immagine è collegata a un file esterno alla cartella della pubblicazione, che JeffPub non apre automaticamente. La pagina mostra la piccola anteprima, che è ciò che verrebbe stampato. | In Gestione grafica, selezionare l'immagine e fare clic su **Aggiorna collegamento**. |
| L'immagine ha una bassa risoluzione | L'immagine ha meno di 150 ppi (pixel per pollice) alle dimensioni di stampa. | Usare un'immagine più grande o stamparla più piccola. |
| L'oggetto ha trasparenza o effetti (la stampa potrebbe essere diversa) | Le aree trasparenti e gli effetti possono apparire diversi su carta. | Stampare una pagina di prova. |
| Il tipo di carattere "nome" non è installato | Un tipo di carattere usato nel testo non è presente nel computer, quindi viene sostituito da un altro. | Installare il tipo di carattere o sceglierne un altro. |

Ogni messaggio termina con il numero di pagina, ad esempio "(Pagina 2)".

## Prima di creare un file per la tipografia

Quando si crea un file PDF/X, JeffPub segnala le immagini a bassa risoluzione e i tipi di carattere mancanti e propone di aprire Verifica struttura. Vedere [File PDF e tipografie](pdf).

## Argomenti correlati

- [File PDF e tipografie](pdf)
- [Stampare una pubblicazione](printing)
- [Far scorrere il testo tra caselle di testo](linked-text)
- [Aggiungere e regolare immagini](pictures)
- [Gestione grafica](graphics-manager)
