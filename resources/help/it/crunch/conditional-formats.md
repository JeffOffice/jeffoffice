# Formattazione condizionale

<!-- keywords: formattazione condizionale, formato condizionale, regola, evidenzia, scala di colori, barra dei dati, set di icone, icone, primi, ultimi, sopra la media, sotto la media, duplicato, univoco, formula, interrompi se vero, priorità, barre, frecce, semafori, sfumatura, formato differenziale, pulsante Formattazione condizionale, cancella regole, maggiore di, minore di, compreso tra, testo che contiene, valori duplicati, primi 10 -->

Una **formattazione condizionale** è una regola che cambia l'aspetto di una cella quando il suo contenuto soddisfa una condizione. Esempio: una regola dice "colora una cella di rosso se è minore di 100". Una cella che contiene 80 è rossa, una cella che contiene 120 non lo è e, se cambi 80 in 150, il rosso scompare da solo. La formattazione propria della cella non viene modificata: la regola decide soltanto che cosa viene disegnato.

## Che cosa mostra JeffCrunch

JeffCrunch mostra le regole che una cartella di lavoro contiene:

- **Regole di evidenziazione**: una regola che modifica il carattere, il riempimento, il bordo o il formato numero delle celle che la soddisfano. La condizione può essere un confronto con un valore (maggiore di, compreso tra, uguale a e così via), una formula, un testo che contiene, inizia con o termina con determinate parole, una data come "settimana scorsa", celle vuote o con errori, valori tra i più alti o i più bassi (un numero o una percentuale), valori superiori o inferiori alla media, e valori duplicati o univoci.
- **Scale di colori**: ogni cella viene riempita con un colore che passa tra due o tre colori man mano che il suo numero va dal valore più basso a quello più alto. Una scala dal bianco al verde rende più verdi i numeri più grandi.
- **Barre dei dati**: una barra nella cella, tanto più lunga quanto più il numero è grande. Le barre possono essere di un colore pieno o sfumare, possono avere un bordo e un asse, e possono andare nella direzione opposta, con un altro colore, per i numeri negativi.
- **Set di icone**: una piccola immagine nella cella che indica in quale parte dell'intervallo si trova il numero, come una freccia verso l'alto, una freccia orizzontale o una freccia verso il basso; oppure un semaforo rosso, giallo o verde. Ci sono venti set.

Quando due regole riguardano la stessa cella, vince quella che viene prima nell'ordine della cartella di lavoro per ciò che entrambe impostano, e una regola contrassegnata come "interrompi se vero" termina la ricerca. I valori di una scala di colori, di una barra dei dati o di un set di icone vengono calcolati sulle celle proprie della regola, e vengono ricalcolati ogni volta che una di quelle celle cambia.

## Che cosa resta invariato

- Le regole seguono le celle a cui appartengono. Inserisci o elimina righe o colonne, taglia e incolla: le regole continuano a coprire le celle giuste. **Annulla** le ripristina.
- Una regola che JeffCrunch non disegna viene conservata nel file così com'è.
- Il salvataggio scrive le regole che la cartella di lavoro contiene. Quando crei o cancelli regole in un foglio, le regole del foglio vengono scritte di nuovo con le tue modifiche.

## Creare una regola

1. Seleziona le celle.
2. Fai clic su **Home > Formattazione condizionale**.
3. In **Regole evidenziazione celle**, scegli **Maggiore di**, **Minore di**, **Compreso tra**, **Uguale a**, **Testo che contiene** o **Valori duplicati**. In **Regole primi/ultimi**, scegli **Primi 10 elementi**, **Ultimi 10 elementi**, **Primi 10%**, **Ultimi 10%**, **Sopra la media** o **Sotto la media**.
4. Digita il valore (o i due valori per **Compreso tra**, il testo per **Testo che contiene**, oppure quanti elementi per le regole Primi e Ultimi). Per **Primi 10 elementi**, "10" è il numero di celle che ricevono l'aspetto. Puoi cambiarlo con un numero qualsiasi.
5. Scegli l'aspetto delle celle, ad esempio **Riempimento rosso chiaro con testo rosso scuro**, e fai clic su **OK**.

La regola viene disegnata subito. Ogni nuova regola viene messa per prima, quindi quando due regole vogliono modificare la stessa cosa, vince quella più recente. La creazione di una regola è un'operazione che puoi annullare con un solo passaggio.

## Cancellare regole

**Cancella regole dalle celle selezionate** elimina tutte le regole presenti in una qualsiasi delle celle selezionate. **Cancella regole da questo foglio** elimina tutte le regole del foglio. Ciascuna operazione può essere annullata con un solo passaggio. Se non ci sono regole da cancellare, la scelta non produce effetti.

## Che cosa non è ancora disponibile

- Barre dei dati, scale di colori e set di icone possono essere visualizzati, ma non ancora creati. La finestra che elenca, ordina e modifica tutte le regole (**Gestisci regole**) non è ancora disponibile. Una regola che usa una formula non può ancora essere creata dal menu.
- Un foglio che contiene barre dei dati in formato di un file più recente le conserva così come sono, e JeffCrunch non crea né cancella regole su quel foglio. Apporta queste modifiche con il programma che ha creato la cartella di lavoro.
- Una cartella di lavoro aperta da un tipo di file meno recente conserva le proprie regole quando la salvi come cartella di lavoro .xlsx, e dopo puoi aggiungere altre regole.
