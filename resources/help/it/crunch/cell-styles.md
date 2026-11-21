# Stili cella

<!-- keywords: modifica stile cella, cambia stile, unisci stili, importa stili, stile cella, stili, stile con nome, Positivo, Negativo, Neutro, Intestazione, Titolo, Totale, Accento, Input, Output, Calcolo, Cella di controllo, Testo esplicativo, Nota, Testo di avviso, Cella collegata, Virgola, Valuta, Percentuale, Normale, nuovo stile, nome, elimina stile, rimuovi stile -->

Uno **stile cella** è un insieme con nome di caratteristiche: un carattere, un riempimento, un bordo, un formato numero e l'allineamento del testo. Se assegni alla cella lo stile "Titolo 1", la cella assume tutte queste caratteristiche in una volta. Gli stili mantengono coerente una cartella di lavoro, perché tutte le celle con lo stesso stile hanno lo stesso aspetto.

## Applicare uno stile

1. Seleziona le celle.
2. Fai clic su **Home > Stili cella** e scegli uno stile. Il menu mostra un piccolo esempio di ciascuno stile.

Il menu è suddiviso nei seguenti gruppi:

- **Positivo, negativo e neutro**: Normale, Positivo, Negativo e Neutro.
- **Dati e modello**: Calcolo, Cella di controllo, Testo esplicativo, Input, Cella collegata, Nota, Output e Testo di avviso.
- **Titoli e intestazioni**: Titolo, Intestazione 1-4 e Totale.
- **Stili cella con tema**: quattro tonalità di ciascuno dei sei colori di accento del tema della cartella di lavoro.
- **Formato numero**: Virgola, Virgola [0], Valuta, Valuta [0] e Percentuale.
- **Personalizzati**: stili che la cartella di lavoro ha già con altri nomi.

Uno stile già presente nella cartella di lavoro viene usato così com'è, con l'aspetto che ha lì. Uno stile che la cartella di lavoro non possiede viene creato la prima volta che lo usi, a partire dal primo carattere della cartella di lavoro, e da quel momento fa parte del file insieme agli altri.

## Creare uno stile personalizzato

1. Formatta una cella come desideri: carattere, riempimento, bordo, formato numero e così via.
2. Lascia il cursore su quella cella. Seleziona le celle che devono ricevere lo stile.
3. Fai clic su **Home > Stili cella > Nuovo stile cella**, digita un nome e fai clic su **OK**.

Lo stile mantiene l'aspetto della cella con il cursore, e alle celle selezionate viene assegnato. Lo stile compare ora nel gruppo **Personalizzati** del menu ed è salvato nella cartella di lavoro. Ogni stile deve avere un nome diverso. Un nome già in uso (non conta se in maiuscolo o minuscolo) viene rifiutato, così come un nome vuoto.

## Modificare uno stile

Fai clic su **Home > Stili cella > Modifica stile cella** e scegli uno stile presente nella cartella di lavoro (uno che la raccolta propone è nella cartella di lavoro una volta che una cella l'ha usato). Si apre la finestra di dialogo Formato celle, che parte dall'aspetto dello stile. Cambia ciò che desideri e fai clic su **OK**: lo stile assume la modifica, e con lui ogni cella che lo usa, in un'unica operazione annullabile. Cambia solo ciò che una cella riceve dallo stile: una cella con una formattazione propria la conserva, quindi una cella con un riempimento tutto suo mantiene quel riempimento anche quando cambia il riempimento dello stile. Anche intere colonne e righe formattate con lo stile ricevono la modifica. Un foglio protetto contro la formattazione delle celle, che contiene una cella con lo stile, rifiuta la modifica e non cambia nulla. Non puoi modificare Normale in questo modo. Lo stile viene salvato nella cartella di lavoro con il nuovo aspetto, e una cella a cui lo assegni in seguito lo avrà.

## Unire stili da un'altra cartella di lavoro

Apri la cartella di lavoro che contiene gli stili, poi, nella cartella di lavoro che deve riceverli, fai clic su **Home > Stili cella > Unisci stili** e scegli la cartella di lavoro. Viene aggiunto ogni stile che questa cartella non possiede. Quando uno stile ha lo stesso nome qui (non conta se in maiuscolo o minuscolo) ma un aspetto diverso, ti viene chiesto una sola volta se sostituirlo: **Sostituisci** gli assegna l'aspetto dell'altra cartella di lavoro, come fa Modifica (le celle che lo usano cambiano solo in ciò che ricevono dallo stile), mentre **Mantieni** lascia il tuo. Un colore che è un colore del tema viene importato come il colore che mostra nell'altra cartella di lavoro, a meno che entrambe le cartelle abbiano lo stesso tema. Gli stili con un aspetto identico vengono ignorati. L'operazione è un'unica fase annullabile: Annulla rimuove ogni stile aggiunto e ripristina ogni aspetto modificato. Gli stili dell'altra cartella di lavoro vengono solo letti e tale cartella non viene modificata. Vengono importati al massimo 1.000 stili alla volta (quelli in più vengono rifiutati e non si unisce nulla); se l'altra cartella di lavoro è occupata, ti viene chiesto di riprovare; Normale resta invariato, e uno stile che hai eliminato qui resta eliminato.

## Eliminare uno stile

Fai clic su **Home > Stili cella > Elimina stile cella** e scegli lo stile. Ogni cella che lo usa torna a Normale, l'aspetto semplice, e lo stile scompare dal menu. L'operazione è un'unica fase annullabile, e annullando si ripristinano lo stile e l'aspetto delle celle. Non puoi eliminare Normale. Il file della cartella di lavoro conserva il nome dello stile, contrassegnato come nascosto, in modo che anche gli altri programmi non lo mostrino.

## Note

- Applicare uno stile è un'unica fase annullabile. Sostituisce carattere, riempimento, bordo, formato numero, allineamento e protezione della cella con quelli dello stile.
- Una cella ricorda il proprio stile. Formattare la cella in seguito, ad esempio mettendola in grassetto, modifica solo quella cella, e la cella continua a seguire il resto dello stile.
- JeffCrunch non dispone ancora di un comando per cambiare la definizione di uno stile o per importare stili da un'altra cartella di lavoro. Una cartella di lavoro che contiene stili propri li mostra nel gruppo **Personalizzati** e nel gruppo a cui appartengono.
