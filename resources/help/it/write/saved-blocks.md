# Salvare un blocco e reinserirlo

<!-- keywords: blocchi salvati, testo standard, firma, carta intestata, abbreviazione di testo, abbreviazioni di testo, riutilizzare testo, salva selezione, inserisci di nuovo, F3, libreria, blocchi predefiniti, modelli di testo -->

Un **blocco** è una parte di documento che avete salvato per poterla inserire di nuovo: una firma, un paragrafo che usate in ogni lettera, un'intestazione o un'immagine con una didascalia. Un blocco conserva il testo insieme ai suoi caratteri, colori, stili, elenchi puntati e numerati, immagini, collegamenti, campi e tabelle (righe, colonne, larghezze, bordi, ombreggiature, celle unite, una riga di intestazione che si ripete e lo stile della tabella). Non conserva commenti, note a piè di pagina, note di chiusura o interruzioni di sezione (vedi Note utili).

## Salvare un blocco

1. Selezionate ciò che volete salvare.
2. Scegliete **Inserisci > Blocchi salvati > Salva selezione come blocco**.
3. Digitate un **Nome**. Potete aggiungere anche una **Categoria** e una **Descrizione**, che aiutano a ritrovare il blocco in seguito.
4. In **Raccolta**, scegliete **Blocchi salvati** (un blocco che si sceglie da una raccolta) oppure **Abbreviazioni di testo** (un blocco che si inserisce digitandone il nome).
5. In **Inserisci come**, scegliete come il blocco entra in un documento: **Così com'è, al cursore**, **In un paragrafo a sé** oppure **In una pagina a sé**.
6. Scegliete **Salva**.

Se esiste già un blocco salvato con quel nome, JeffWrite chiede se sostituirlo.

## Inserire un blocco

1. Posizionate il cursore dove deve andare il blocco. Se prima selezionate delle parole, il blocco prende il loro posto.
2. Scegliete **Inserisci > Blocchi salvati**, quindi fate clic sul blocco. Digitate nella casella di ricerca per trovare un blocco in base al nome, alla categoria o alla descrizione.

L'inserimento di un blocco si annulla con un solo passaggio. Uno stile usato dal blocco che il documento non possiede viene aggiunto; uno stile che il documento ha già con lo stesso nome non viene mai modificato. Un campo nel blocco, come il numero di pagina, viene aggiornato per il documento in cui si trova. Il primo e l'ultimo paragrafo di un blocco assumono l'aspetto dei paragrafi a cui si uniscono, come avviene con Incolla.

## Abbreviazioni di testo

Un'abbreviazione di testo è un blocco con un nome breve. Digitate il nome, quindi premete **F3** (oppure scegliete **Inserisci > Blocchi salvati > Espandi abbreviazione di testo**): il nome viene sostituito dal blocco. Il nome deve iniziare una parola: "firma" funziona dopo uno spazio, ma non all'interno di "progettazione".

## Gestire i blocchi salvati

Scegliete **Inserisci > Blocchi salvati > Gestisci blocchi salvati**. Potete vedere ogni blocco, cambiarne il nome, la categoria, la descrizione o il modo di inserimento con **Modifica proprietà**, ed **Eliminare** il blocco. **Annulla eliminazione** ripristina ciò che avete eliminato finché non chiudete la finestra. **Esporta** salva i vostri blocchi in un file, e **Importa** aggiunge i blocchi di un file del genere; un blocco con un nome che avete già viene lasciato com'è.

## Note utili

- I vostri blocchi salvati sono conservati in un unico file su questo computer, solo per il vostro account: non si trovano nei documenti e non vengono inviati da nessuna parte. Il file è `library.jblocks`, nella cartella `Saved Blocks` all'interno della cartella dei dati: `AppData\Local\JeffOffice` su Windows, `Library/Application Support/JeffOffice` su Mac e `.local/share/JeffOffice` su Linux. Tutti i programmi JeffOffice del computer condividono questo file, e due finestre possono aggiungere blocchi nello stesso momento senza perdere ciascuna quelli dell'altra.
- Un blocco può arrivare a 8 megabyte, e la libreria può contenere fino a 5.000 blocchi.
- Una tabella viene salvata come tabella quando la selezione comprende tutta la tabella. Se la selezione comprende solo una parte di una tabella, quella parte viene salvata come testo, con un paragrafo per ogni cella, e JeffWrite ve lo segnala prima di salvare.
- Un segnalibro in un blocco riceve un nome proprio quando inserite il blocco: se il documento ha già un segnalibro con quel nome, il nuovo viene chiamato con "_2", "_3" e così via.
- I commenti vengono esclusi. Le note a piè di pagina e le note di chiusura vengono escluse. Le interruzioni di sezione non vengono conservate. Forme e caselle di testo vengono conservate e reinserite insieme al blocco.
- Le modifiche registrate vengono accettate al momento del salvataggio del blocco: il testo inserito resta, mentre il testo eliminato non entra nel blocco, quindi un blocco non contiene mai testo registrato di un'altra persona.
- Un documento non aggiunge mai blocchi alla vostra libreria. Solo voi lo fate.
