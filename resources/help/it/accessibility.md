# Lettori di schermo, alto contrasto e testo alternativo
<!-- keywords: accessibilità, lettore di schermo, narratore, non vedenti, ipovisione, alto contrasto, modalità scura, tema scuro, contorno di messa a fuoco, testo alternativo, descrizione immagine, zoom, ingrandisci, solo tastiera, accessibility, screen reader -->

JeffPub è progettato in modo che anche chi usa lettori di schermo, la tastiera o colori ad alto contrasto possa utilizzarlo.

## Lettori di schermo

Un **lettore di schermo** è un programma che legge ad alta voce ciò che appare sullo schermo. JeffPub funziona con lettori di schermo come Narratore in Windows. Ogni controllo ha un nome e una descrizione. Questo vale per le schede, i pulsanti, le raccolte e i pulsanti colore della barra multifunzione, l'elenco delle pagine, la pagina, i riquadri attività e la barra di stato.

Vengono letti anche gli oggetti presenti nella pagina. Nella pagina, premi **Tab** per selezionare l'oggetto successivo: il lettore di schermo dice di che tipo è e che cosa contiene, ad esempio "Casella di testo: Saldi di primavera" oppure "Immagine: un fienile rosso", poi dove si trova e quanto è grande. Un'immagine senza testo alternativo viene letta come "Immagine, nessun testo alternativo", così sai quali hanno ancora bisogno di una descrizione. Premi **Invio** per digitare in una casella di testo: il lettore di schermo può leggerne il testo, la riga o la parola su cui si trova il cursore e ciò che selezioni.

- L'elenco delle pagine legge ogni pagina come "Pagina 1", "Pagina 2" e così via. Se hai dato un nome a una pagina, il nome segue, come in "Pagina 3, Copertina".
- L'area della pagina si chiama "Pagina". La sua descrizione indica i tasti da usare: Tab seleziona l'oggetto successivo, i tasti freccia lo spostano, Invio modifica il suo testo e Canc lo elimina.
- Le schede vengono lette come schede della barra multifunzione. Le schede che compaiono quando selezioni qualcosa indicano anche il gruppo a cui appartengono, come Strumenti immagine.

## Usare la tastiera

Puoi raggiungere la barra multifunzione, l'elenco delle pagine e gli oggetti nella pagina usando solo la tastiera. Premi F6 per spostarti tra la barra multifunzione, l'elenco delle pagine e la pagina. Premi Alt per vedere i suggerimenti tasti. Un contorno colorato indica quale controllo ha il fuoco. Vedi [Usare JeffPub con la tastiera](keyboard).

## Modificare i colori

- **Chiaro o scuro**: fai clic su **File > Opzioni** e, nella scheda **Generale**, imposta **Tema** su **Chiaro**, **Scuro** oppure **Usa impostazione di sistema**. La modifica viene applicata subito.
- **Alto contrasto**: in Windows, quando attivi un tema ad alto contrasto, JeffPub lo segue. Usa i colori scelti da Windows e cambia quando attivi o disattivi il tema.

## Rendere le immagini più facili da capire

Il **testo alternativo** è una breve descrizione di un'immagine o di un altro oggetto. I lettori di schermo lo leggono ad alta voce. JeffPub lo include anche nelle pagine web e negli e-book che salvi.

1. Fai clic con il pulsante destro sull'immagine o sull'oggetto.
2. Fai clic su **Formato oggetto…**.
3. Fai clic sulla scheda **Testo alternativo**.
4. Digita una descrizione nella casella **Testo alternativo**.
5. Fai clic su **OK**.

Descrivi in poche parole ciò che l'immagine mostra, ad esempio "Due bambini che piantano un albero". Icone e codici a barre che inserisci ricevono automaticamente un testo alternativo.

## Controllare la pagina

Fai clic su **Revisione > Verifica accessibilità** per elencare ciò che potrebbe risultare difficile da usare per alcune persone. Oltre alle immagini e alle forme senza testo alternativo, controlla altri cinque aspetti.

- **Testo difficile da leggere.** Il testo deve avere un **contrasto** sufficiente con ciò che si trova dietro: il riempimento della sua casella oppure il colore della pagina, se la casella non ha riempimento. La verifica calcola il contrasto come rapporto. Un rapporto di 4,5 a 1 è sufficiente per il testo normale, e 3 a 1 per il testo grande (18 punti o più, oppure 14 punti o più in grassetto). Ad esempio, il grigio #777777 su bianco ha un rapporto di 4,48 a 1, appena sotto il limite, mentre #767676 ha un rapporto di 4,54 a 1. Scegli un colore del testo più scuro oppure uno sfondo più chiaro. Se il testo si trova sopra un'immagine, la verifica non può stabilirlo e aggiunge un suggerimento a controllare il contrasto a occhio.
- **Oggetti letti in un ordine diverso.** Un lettore di schermo e un PDF con tag leggono gli oggetti di una pagina dal fondo della pila verso l'alto. Se quest'ordine non va dall'alto in basso e poi da sinistra a destra, la verifica elenca come suggerimenti gli oggetti fuori posto. Fai clic su **Ordine di lettura** per vedere gli oggetti nell'ordine in cui vengono letti. Seleziona un oggetto e fai clic su **Leggi prima** o **Leggi dopo**, oppure trascinalo. Ogni spostamento è un passaggio di Annulla e cambia anche quale oggetto si trova in primo piano.
- **Tabella senza riga di intestazione.** Un lettore di schermo usa la prima riga di una tabella per dare un nome alle colonne. Una tabella la cui prima riga non è contrassegnata come riga di intestazione viene segnalata come avviso. Fai clic sul problema, quindi su **Usa la prima riga come intestazione** per attivare la riga di intestazione. È un passaggio di Annulla.
- **Tabella con celle unite.** Un lettore di schermo può perdere il punto in cui si trova in una tabella quando le celle sono unite, quindi una tabella con una cella unita viene segnalata come avviso. Mantieni intera ogni cella della tabella: separa le celle unite oppure usa due tabelle.
- **Collegamento che non indica la destinazione.** Un lettore di schermo può leggere i collegamenti di una pagina come elenco, quindi il testo di ogni collegamento deve indicare la pagina che apre. Un collegamento il cui testo è il proprio indirizzo, è solo "qui", "clicca qui", "leggi di più" o una parola simile, oppure è vuoto, viene segnalato come avviso. Cambia il testo, ad esempio "Il nostro rapporto annuale" invece di "clicca qui". I primi 50 vengono elencati uno per uno, e i restanti sono conteggiati in una sola riga.

## Ingrandire la pagina

Ingrandisci per leggere testi piccoli. Usa i controlli **Zoom** nella barra di stato, tieni premuto CTRL e ruota la rotella del mouse, oppure scegli una dimensione nella casella **Zoom** della scheda **Visualizza**. Il cursore dello zoom va da circa il 10% al 1600%.

## Argomenti correlati

- [Usare JeffPub con la tastiera](keyboard)
- [Aggiungere e regolare le immagini](pictures)
- [Modificare le impostazioni di JeffPub](options)
- [Salvare come immagine, pagina web o e-book](export)
