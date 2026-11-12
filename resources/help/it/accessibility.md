# Lettori di schermo, alto contrasto e testo alternativo
<!-- keywords: accessibilità, lettore di schermo, narratore, non vedenti, ipovisione, alto contrasto, modalità scura, tema scuro, contorno di messa a fuoco, testo alternativo, descrizione immagine, zoom, ingrandisci, solo tastiera -->

JeffPub è progettato in modo che anche chi usa un lettore di schermo, la tastiera o colori ad alto contrasto possa lavorare con il programma.

## Lettori di schermo

Un **lettore di schermo** è un programma che legge ad alta voce ciò che appare sullo schermo. JeffPub funziona con lettori di schermo come Narratore su Windows. Ogni controllo ha un nome e una descrizione. Questo vale per le schede, i pulsanti, le raccolte e i pulsanti dei colori della barra multifunzione, per l'elenco delle pagine, la pagina, i riquadri attività e la barra di stato.

Anche gli oggetti sulla pagina vengono letti. Sulla pagina, premi **Tab** per selezionare l'oggetto successivo: il lettore di schermo dice di che tipo è e che cosa contiene, ad esempio "Casella di testo: Saldi di primavera" oppure "Immagine: un fienile rosso", poi dove si trova e quali sono le sue dimensioni. Un'immagine senza testo alternativo viene letta come "Immagine, senza testo alternativo", così sai quali immagini devono ancora riceverlo. Premi **Invio** per scrivere in una casella di testo: il lettore di schermo può poi leggerne il testo, la riga o la parola in cui si trova il cursore e ciò che selezioni.

- L'elenco delle pagine legge ogni pagina come "Pagina 1", "Pagina 2" e così via. Se hai dato un nome a una pagina, il nome segue, come in "Pagina 3, Copertina".
- L'area della pagina si chiama "Pagina". La sua descrizione indica i tasti da usare: Tab seleziona l'oggetto successivo, i tasti freccia lo spostano, Invio modifica il suo testo e Canc lo elimina.
- Le schede vengono lette come schede della barra multifunzione. Le schede che compaiono quando selezioni qualcosa indicano anche il gruppo a cui appartengono, come Strumenti immagine.

## Usare la tastiera

Puoi raggiungere la barra multifunzione, l'elenco delle pagine e gli oggetti sulla pagina usando solo la tastiera. Premi F6 per spostarti tra la barra multifunzione, l'elenco delle pagine e la pagina. Premi Alt per visualizzare i suggerimenti tasti. Un contorno colorato indica quale controllo ha il focus. Vedi [Usare JeffPub con la tastiera](keyboard).

## Cambiare i colori

- **Chiaro o scuro**: fai clic su **File > Opzioni** e, nella scheda **Generale**, imposta **Tema** su **Chiaro**, **Scuro** oppure **Usa impostazione di sistema**. La modifica viene applicata subito.
- **Alto contrasto**: su Windows, quando attivi un tema ad alto contrasto, JeffPub lo segue. Usa i colori scelti da Windows e cambia quando attivi o disattivi il tema.

## Rendere le immagini più facili da capire

Il **testo alternativo** è una breve descrizione di un'immagine o di un altro oggetto. I lettori di schermo la leggono ad alta voce. JeffPub la inserisce anche nelle pagine Web e negli e-book che salvi.

1. Fai clic con il pulsante destro del mouse sull'immagine o sull'oggetto.
2. Fai clic su **Formato oggetto…**.
3. Fai clic sulla scheda **Testo alternativo**.
4. Digita una descrizione nella casella **Testo alternativo**.
5. Fai clic su **OK**.

Descrivi in poche parole che cosa mostra l'immagine, ad esempio "Due bambini che piantano un albero". Le icone e i codici a barre che inserisci ricevono automaticamente un testo alternativo.

## Controllare la pagina

Fai clic su **Revisione > Verifica accessibilità** per elencare ciò che potrebbe essere difficile da usare per alcune persone. Oltre alle immagini e alle forme senza testo alternativo, il controllo individua altri tre problemi.

- **Testo difficile da leggere.** Il testo deve avere un **contrasto** sufficiente rispetto allo sfondo: il riempimento della sua casella oppure il colore della pagina, se la casella non ha riempimento. Il controllo calcola il contrasto come rapporto. Un rapporto di 4,5 a 1 è sufficiente per il testo normale, mentre 3 a 1 è sufficiente per il testo grande (18 punti o più, oppure 14 punti o più in grassetto). Ad esempio, il grigio #777777 su bianco dà 4,48 a 1, appena sotto il limite, mentre #767676 dà 4,54 a 1. Scegli un colore del testo più scuro oppure uno sfondo più chiaro. Quando il testo si trova sopra un'immagine, il controllo non può verificarlo e aggiunge un suggerimento per controllare il contrasto a occhio.
- **Oggetti letti in un ordine diverso.** Un lettore di schermo e un PDF con tag leggono gli oggetti di una pagina dal fondo della pila verso l'alto. Se questo ordine non va dall'alto in basso e da sinistra a destra, il controllo elenca come suggerimenti gli oggetti fuori posto. Fai clic su **Ordine di lettura** per vedere gli oggetti nell'ordine in cui vengono letti. Seleziona un oggetto e fai clic su **Letto prima** o **Letto dopo**, oppure trascinalo. Ogni spostamento è un passaggio di Annulla e cambia anche quale oggetto si trova davanti agli altri.
- **Una tabella senza riga di intestazione.** Un lettore di schermo usa la prima riga di una tabella per dare un nome alle colonne. Una tabella la cui prima riga non è contrassegnata come riga di intestazione viene elencata come avviso. Fai clic sul problema, quindi fai clic su **Usa la prima riga come intestazione** per attivare la riga di intestazione. È un passaggio di Annulla.

## Ingrandire la pagina

Ingrandisci per leggere il testo piccolo. Usa i controlli **Zoom** nella barra di stato, tieni premuto CTRL e ruota la rotellina del mouse, oppure scegli una dimensione nella casella **Zoom** della scheda **Visualizza**. Il cursore dello zoom va da circa il 10% al 1600%.

## Vedi anche

- [Usare JeffPub con la tastiera](keyboard)
- [Aggiungere e regolare le immagini](pictures)
- [Modificare le impostazioni di JeffPub](options)
- [Salvare come immagine, pagina Web o e-book](export)
