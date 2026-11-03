# Verificare la struttura
<!-- keywords: verifica struttura, verifica, controllo, errori, problemi, bassa risoluzione, testo in eccesso, testo che non si adatta, casella di testo vuota, fuori dalla pagina, preflight, controllo finale, bozza, immagine collegata, tipo di carattere mancante -->

**Verifica struttura** esamina la pubblicazione alla ricerca dei problemi più comuni. Rileva il testo che non si adatta, le caselle vuote, gli oggetti che sporgono dalla pagina e le immagini che verranno stampate male. Eseguire la verifica prima di stampare o di inviare un file in tipografia.

## Eseguire Verifica struttura

Fare clic su **Revisione > Verifica > Esegui verifica struttura**. Sulla destra si apre il riquadro **Verifica struttura**. In alternativa, fare clic su **File > Proprietà**, quindi su **Esegui verifica struttura**.

Il riquadro contiene due caselle di controllo, inizialmente entrambe selezionate:

- **Esegui verifiche generali della struttura** cerca i problemi di layout.
- **Esegui verifiche finali di pubblicazione** cerca gli elementi importanti per la stampa.

Lasciarle selezionate entrambe per visualizzare tutti i tipi di problemi. L'elenco si aggiorna mentre si lavora. Se non c'è nulla da correggere, viene visualizzato il messaggio "Nessun problema rilevato."

## Correggere un problema

1. Fare clic su un elemento dell'elenco in **Selezionare un elemento da correggere**.
2. Fare clic su **Vai all'elemento** oppure fare doppio clic sull'elemento.

JeffPub passa alla pagina e seleziona l'oggetto. Dopo la correzione, l'elemento scompare dall'elenco.

## Problemi rilevati

| Messaggio | Significato | Soluzione |
|---|---|---|
| La pagina non ha contenuto | La pagina è vuota. | Aggiungere un contenuto o eliminare la pagina. |
| L'oggetto è parzialmente fuori dalla pagina | Una parte dell'oggetto si trova fuori dalla pagina. | Spostare l'oggetto, oppure lasciarlo così se si tratta di un'abbondanza. |
| La casella di testo è vuota | Una casella di testo non contiene testo. | Digitare del testo o eliminare la casella. |
| Articolo con testo in eccesso | Un articolo è il testo che scorre attraverso caselle collegate. Questo contiene più testo di quanto ne possa contenere. | Ingrandire una casella, ridurre il testo o [collegare un'altra casella](linked-text). |
| Il testo è stato ridotto notevolmente per adattarlo | La riduzione del testo per adattarlo alla casella lo ha portato a meno della metà delle dimensioni originali. | Ingrandire la casella o ridurre il testo. |
| Il segnaposto immagine è vuoto | Una cornice per immagini non contiene immagini. | Fare clic per aggiungerne una o eliminare il segnaposto. |
| L'immagine non è ridimensionata in modo proporzionale | L'immagine è allungata o schiacciata. | Ridimensionarla per ripristinarne le proporzioni. |
| L'immagine è collegata, non incorporata | L'immagine è collegata a un file e nella pubblicazione è memorizzata solo una piccola anteprima. Un'immagine inserita con **Inserisci e collega** viene memorizzata per intero e non compare nell'elenco. | Conservare il file originale insieme alla pubblicazione oppure fare clic su **Incorpora immagine** in [Gestione grafica](graphics-manager). |
| Immagine collegata mancante | Il file a cui punta l'immagine collegata non esiste più, quindi la pagina mostra solo la piccola anteprima, che è ciò che verrebbe stampato. | In Gestione grafica fare clic su **Cambia collegamento…** per trovare il file, oppure ripristinarlo e fare clic su **Aggiorna collegamento**. |
| L'immagine collegata mostra solo l'anteprima fino ad Aggiorna collegamento | L'immagine è collegata a un file esterno alla cartella della pubblicazione, che JeffPub non apre automaticamente. La pagina mostra la piccola anteprima, che è ciò che verrebbe stampato. | In Gestione grafica selezionare l'immagine e fare clic su **Aggiorna collegamento**. |
| L'immagine ha una risoluzione bassa | L'immagine ha meno di 150 ppi (pixel per pollice) alle dimensioni di stampa. | Usare un'immagine più grande o stamparla più piccola. |
| L'oggetto ha trasparenza o effetti (la stampa potrebbe essere diversa) | Le aree trasparenti e gli effetti possono apparire diversi su carta. | Stampare una pagina di prova. |
| Il tipo di carattere "nome" non è installato | Un tipo di carattere usato nel testo non è presente nel computer e viene sostituito da un altro. | Installare il tipo di carattere o sceglierne un altro. |

Ogni messaggio termina con il numero di pagina, ad esempio "(Pagina 2)".

## Prima di creare un file per la tipografia

Quando si crea un file PDF/X, JeffPub avvisa della presenza di immagini a bassa risoluzione e di tipi di carattere mancanti e propone di aprire Verifica struttura. Vedere [File PDF e tipografie](pdf).

## Argomenti correlati

- [File PDF e tipografie](pdf)
- [Stampare una pubblicazione](printing)
- [Far scorrere il testo tra caselle di testo](linked-text)
- [Aggiungere e regolare immagini](pictures)
- [Gestione grafica](graphics-manager)
