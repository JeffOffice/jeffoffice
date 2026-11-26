# Salvare un blocco e reinserirlo

<!-- keywords: blocchi salvati, testo standard, firma, carta intestata, abbreviazione di testo, abbreviazioni di testo, riutilizzare testo, salva selezione, inserisci di nuovo, F3, libreria -->

Un **blocco** è una parte di documento che salvi per poterla inserire di nuovo: una firma, un paragrafo che usi in ogni lettera, una carta intestata o un'immagine con la sua didascalia. Un blocco conserva il testo insieme ai caratteri, ai colori, agli stili, agli elenchi puntati e numerati, alle immagini, ai collegamenti, ai campi e alle tabelle (righe, colonne, larghezze, bordi, sfondi, celle unite, una riga di intestazione che si ripete e lo stile della tabella). Non conserva commenti, note a piè di pagina, note di chiusura o interruzioni di sezione (vedi Note utili).

## Salvare un blocco

1. Seleziona ciò che vuoi salvare.
2. Scegli **Inserisci > Blocchi salvati > Salva selezione come blocco**.
3. Digita un **Nome**. Puoi anche aggiungere una **Categoria** e una **Descrizione**, che ti aiutano a ritrovare il blocco in seguito.
4. In **Raccolta**, scegli **Blocchi salvati** (un blocco che selezioni da una raccolta) oppure **Abbreviazioni di testo** (un blocco che inserisci digitandone il nome).
5. In **Inserisci come**, scegli come il blocco entra in un documento: **Così com'è, al cursore**, **In un paragrafo a sé** oppure **In una pagina a sé**.
6. Scegli **Salva**.

Se esiste già un blocco salvato con quel nome, JeffWrite chiede se sostituirlo.

## Inserire un blocco

1. Posiziona il cursore dove deve andare il blocco. Se prima selezioni delle parole, il blocco prende il loro posto.
2. Scegli **Inserisci > Blocchi salvati**, quindi fai clic sul blocco. Digita nella casella di ricerca per trovare un blocco in base al nome, alla categoria o alla descrizione.

Inserire un blocco è un'unica operazione annullabile. Uno stile usato dal blocco che il documento non possiede viene aggiunto; uno stile che il documento ha già con lo stesso nome non viene mai modificato. Un campo del blocco, come il numero di pagina, viene aggiornato per il documento in cui si trova. Il primo e l'ultimo paragrafo di un blocco assumono l'aspetto dei paragrafi a cui si uniscono, come accade con Incolla.

## Le tue copertine, tabelle e caselle di testo

Una tua copertina, tabella o casella di testo può essere conservata in una raccolta propria, sotto quelle predefinite.

- **Pagina di copertina.** Seleziona la copertina e la sua interruzione di pagina (oppure posiziona il cursore in un punto qualsiasi senza selezionare nulla, se il documento ha già una copertina), quindi scegli **Inserisci > Pagina di copertina > Le tue copertine > Salva selezione nella Galleria copertine**.
- **Tabella.** Seleziona l'intera tabella e scegli **Inserisci > Tabella > Tabelle predefinite > Le tue tabelle > Salva selezione nella Galleria tabelle**.
- **Casella di testo.** Fai clic sulla casella di testo in modo che sia selezionata, quindi scegli **Inserisci > Casella di testo > Le tue caselle di testo > Salva selezione nella Galleria caselle di testo**.

Assegna un nome e fai clic su **Salva**. L'elemento compare poi in **Le tue copertine**, **Le tue tabelle** o **Le tue caselle di testo**. Fai clic su di esso per inserirlo. Una copertina va all'inizio del documento, in una pagina a sé, e sostituisce la copertina che il documento possiede. Una tabella o una casella di testo viene inserita al cursore. **Gestisci blocchi salvati** elenca anche queste raccolte, così puoi rinominarle, modificarle, eliminarle, esportarle e importarle.

## Abbreviazioni di testo

Un'abbreviazione di testo è un blocco con un nome breve. Digita il nome, quindi premi **F3** (oppure scegli **Inserisci > Blocchi salvati > Espandi abbreviazione di testo**): il nome viene sostituito dal blocco. Il nome deve iniziare una parola: "firma" funziona dopo uno spazio, ma non all'interno di "progettazione".

## Gestire i blocchi salvati

Scegli **Inserisci > Blocchi salvati > Gestisci blocchi salvati**. Puoi vedere ciascun blocco, cambiarne il nome, la categoria, la descrizione o il modo di inserimento con **Modifica proprietà**, ed **Elimina** il blocco. **Annulla eliminazione** ripristina ciò che hai eliminato finché non chiudi la finestra. **Esporta** salva i tuoi blocchi in un file, e **Importa** aggiunge i blocchi di un file del genere: un blocco con un nome già presente viene lasciato com'è.

## Note utili

- I tuoi blocchi salvati sono conservati in un unico file su questo computer, solo per il tuo account: non si trovano nei tuoi documenti e non vengono inviati a nessuno. Il file è `library.jblocks`, nella cartella `Saved Blocks` all'interno della cartella dei dati: `AppData\Local\JeffOffice` su Windows, `Library/Application Support/JeffOffice` su Mac e `.local/share/JeffOffice` su Linux. Tutti i programmi JeffOffice del computer lo condividono, e due finestre possono aggiungere blocchi nello stesso momento senza perdere ciascuna i blocchi dell'altra.
- Un blocco può arrivare a 8 megabyte, e la libreria può contenere fino a 5.000 blocchi.
- Una tabella viene salvata come tabella quando la selezione comprende l'intera tabella. Se la selezione comprende solo una parte di una tabella, quella parte viene salvata come testo, con un paragrafo per ogni cella, e JeffWrite te lo segnala prima del salvataggio.
- Un segnalibro contenuto in un blocco riceve un nome proprio quando inserisci il blocco: se il documento ha già un segnalibro con quel nome, quello nuovo viene chiamato con "_2", "_3" e così via.
- I commenti vengono esclusi. Le note a piè di pagina e le note di chiusura vengono escluse. Le interruzioni di sezione non vengono conservate. Forme e caselle di testo vengono conservate e riinserite insieme al blocco.
- Le modifiche registrate vengono accettate al salvataggio del blocco: le parole inserite restano, mentre quelle eliminate non vengono incluse nel blocco, che quindi non contiene mai modifiche registrate di un'altra persona.
- Un documento non aggiunge mai blocchi alla tua libreria. Solo tu puoi farlo.
