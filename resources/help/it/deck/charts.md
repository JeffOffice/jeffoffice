# Grafici
<!-- keywords: grafico, grafici, diagramma, grafico a colonne, grafico a barre, grafico a linee, grafico a torta, grafico a ciambella, area, dispersione, inserisci grafico, dati grafico, modifica dati, titolo grafico, legenda, etichette dati, tipo di grafico, formato grafico, assi, titolo asse, linee della griglia, stile grafico, parti del grafico, layout, colori, aspetti, dati del grafico, scambia righe e colonne, tipo grafico, formatta asse, barre di errore, linea di tendenza, filtri grafico, elementi grafico -->

Un **grafico** trasforma i numeri in un'immagine, come colonne, una linea o fette di una torta. Questo argomento spiega come inserire un grafico in una diapositiva, come digitare i suoi numeri e come cambiarne l'aspetto.

## Inserire un grafico in una diapositiva

1. Vai alla diapositiva in cui deve comparire il grafico.
2. Fai clic su **Inserisci > Grafico** e scegli un tipo: colonne, colonne impilate, colonne impilate al 100%, barre, barre impilate, linee, linee con indicatori, torta, ciambella, area, area impilata, dispersione, radar, radar con indicatori, radar riempito, massimo-minimo-chiusura, apertura-massimo-minimo-chiusura, volume-massimo-minimo-chiusura o volume-apertura-massimo-minimo-chiusura.

Un grafico **azionario** mostra i prezzi: per ogni categoria una linea va dal prezzo più alto a quello più basso, con un piccolo punto per la chiusura oppure, se c'è l'apertura, una barra tra apertura e chiusura (chiara in caso di rialzo, scura in caso di ribasso). Il grafico viene creato dalle colonne esattamente nell'ordine in cui si trovano, da sinistra a destra, e nulla verifica che i numeri corrispondano alla posizione; perciò inseriscili in quest'ordine: massimo, minimo e chiusura; apertura, massimo, minimo e chiusura; volume, massimo, minimo e chiusura; oppure volume, apertura, massimo, minimo e chiusura. Se le colonne sono in un altro ordine il grafico viene comunque disegnato, ma i segni hanno un altro significato (il punto della chiusura va sulla terza colonna dei prezzi e la casella va dalla prima alla ultima colonna dei prezzi). Con il volume, il volume è rappresentato da colonne lette sull'asse di sinistra, mentre i prezzi sono letti su un secondo asse a destra. Se il numero di colonne non corrisponde al tipo, JeffDeck indica quante ne servono e non crea alcun grafico.

Un grafico **a colonne impilate** sovrappone le serie. Un grafico **impilato al 100%** rende ogni pila della stessa altezza, così vedi le quote. Un grafico **a dispersione** pone ogni punto dove lo indicano i suoi due numeri.

Il grafico compare con numeri di esempio, al centro della diapositiva, e si apre la scheda **Formato grafico**. Un grafico è un oggetto come un'immagine o una forma: trascinalo per spostarlo, trascina una maniglia per cambiarne le dimensioni (il grafico viene ridisegnato per adattarsi, quindi le sue parole mantengono la forma corretta) e ruotalo con **Ruota** nel gruppo **Disegno** della scheda **Home**.

## Digitare i propri numeri

Seleziona il grafico e fai clic su **Formato grafico > Modifica dati**. Si apre una tabella.

- La prima colonna contiene le parole sotto i punti (le **categorie**), ad esempio i mesi.
- La prima riga contiene il nome di ciascuna **serie**, cioè un insieme di numeri, ad esempio un prodotto.
- Le altre celle contengono i numeri.

Fai clic su **Aggiungi riga** o **Aggiungi colonna** per aggiungerne, oppure su **Rimuovi ultima riga** o **Rimuovi ultima colonna** per eliminarne. Fai clic su **OK**. Se una cella non contiene un numero, il grafico non viene modificato e la barra di stato indica la riga interessata. Un grafico può avere fino a 200 categorie e 24 serie.

Per un grafico a dispersione, la prima colonna contiene i numeri che collocano i punti da sinistra a destra.

## Grafici salvati da altri programmi

Alcuni grafici salvati da altri programmi conservano i propri numeri in una piccola **cartella di lavoro** (un file di foglio di calcolo) incorporata nel file. Seleziona un grafico di questo tipo e fai clic su **Formato grafico > Modifica dati**. La tabella mostra le celle della cartella di lavoro che il grafico legge. Cambia un numero, un nome sotto un punto o il nome di una serie, quindi fai clic su **OK**. La modifica viene applicata sia alla cartella di lavoro sia al grafico, come un'unica operazione annullabile, così anche gli altri programmi mostrano i nuovi numeri.

Non puoi aggiungere o rimuovere righe o colonne di un grafico di questo tipo, e **Dati grafico** e **Scambia righe e colonne** sono disattivati, perché il grafico è collegato a quelle celle. Lo stesso vale quando il grafico legge celle di un altro file, un intervallo che non è una sola colonna o una sola riga, oppure celle che contengono una formula, una data o parole dove dovrebbe esserci un numero. Il comando è disattivato e la sua descrizione spiega il motivo.

## Cambiare l'aspetto

Questi comandi si trovano nella scheda **Formato grafico**. Ognuno è un'unica operazione annullabile e il grafico mantiene i propri colori e il resto del suo aspetto.

- **Cambia tipo di grafico** trasforma il grafico in un altro tipo con gli stessi numeri. Per passare a un tipo azionario, le serie del grafico devono essere nell'ordine richiesto da quel tipo; se non lo sono, JeffDeck indica l'ordine e lascia il grafico com'è.
- **Titolo grafico** mostra o nasconde il titolo. **Modifica titolo grafico** ne cambia il testo.
- **Legenda** è la chiave che indica quale colore corrisponde a quale serie. Puoi collocarla a destra, in alto, a sinistra o in basso, oppure eliminarla.
- **Assi** mostra o nasconde la linea di parole sotto il grafico (le **categorie**) e la linea di numeri accanto (i **valori**), e imposta un titolo per ciascuna. Un grafico a torta non ha assi.
- **Linee della griglia** traccia linee leggere sul grafico per aiutarti a seguire i numeri. Le linee **principali** si trovano ai numeri principali sul lato, le linee **secondarie** tra di esse.
- **Stili grafico** sceglie uno degli otto stili generali.
- **Etichette dati** scrive ciascun numero sulla sua colonna, sul suo punto o sulla sua fetta.

## La scheda Grafico

Seleziona un grafico e accanto alla scheda **Formato grafico** si apre la scheda **Grafico**. Tutto ciò che si trova qui modifica il grafico con un'unica operazione annullabile.

- **Elementi grafico** aggiunge, rimuove o posiziona una parte del grafico: **Assi**, **Titoli assi**, **Titolo grafico**, **Etichette dati** (cosa contengono, dove si trovano e il segno che le separa), **Linee della griglia**, **Legenda**, **Barre di errore** (barre corte che mostrano di quanto un numero può essere impreciso), **Linee** (linee di caduta, linee massimo-minimo e barre di salita e discesa), **Tabella dati** (una griglia dei numeri sotto il grafico, con una colonna per ciascuna categoria; il suo elenco attiva e disattiva la tabella e imposta le linee tra le righe, le linee tra le colonne, il contorno e le chiavi della legenda, che mostrano accanto al nome il colore di ciascuna serie; è disponibile per i grafici a colonne, a linee e ad area) e **Linea di tendenza** (una linea che mostra la direzione verso cui vanno i numeri, con la sua equazione se la desideri).
- **Layout** imposta più parti contemporaneamente, come il titolo, la legenda, le etichette e le linee della griglia.
- **Colori** assegna alle serie un nuovo insieme di colori. **Aspetti** dà all'intero grafico un aspetto, ad esempio semplice, pulito, scuro o morbido.
- **Scambia righe e colonne** inverte i numeri: le serie diventano le parole sotto i punti, e queste parole diventano le serie.
- **Dati grafico** elenca le serie. Seleziona quelle da disegnare, cambia i loro nomi e spostale verso l'alto o verso il basso.
- **Modifica dati** apre la tabella dei numeri.
- **Tipo grafico** mostra un'immagine di ciascun tipo. Scegli un tipo per l'intero grafico oppure un tipo per ciascuna serie, ad esempio colonne con una linea sopra.
- **Formatta elemento grafico** imposta il riempimento e la linea dell'area del grafico, dell'area di tracciamento (il riquadro in cui si trovano i segni) o di una singola serie. Per una linea, imposta anche se la linea è smussata e quale segno compare in ciascun punto.
- **Selezione corrente** elenca le parti del grafico: l'area del grafico, l'area di tracciamento e ciascuna serie. Scegline una, e **Formatta selezione** apre la finestra di formattazione su quella parte. **Ripristina in base allo stile** elimina ciò che era stato impostato sulla parte scelta e ripristina ciò che lo stile del grafico le assegna. Per l'area del grafico si tratta del riempimento, della linea e del colore del testo impostato da un aspetto (il testo chiaro dell'aspetto scuro torna al colore del testo del tema). Per l'area di tracciamento si tratta del riempimento e della linea. Per una serie si tratta del colore (quello assegnato dalla scelta Colori o dallo stile del grafico), del contorno, dei segni come li prevede il tipo di grafico e dei colori dei suoi punti; una linea smussata resta tale quando il tipo di grafico è smussato. Non modifica titoli, legenda, etichette, assi, linee della griglia o dati, e non fa nulla su una parte già conforme. È un'unica operazione di Annulla.
- **Formatta asse orizzontale** e **Formatta asse verticale** impostano i valori minimo e massimo, la dimensione di ciascun intervallo, l'ordine, il punto in cui l'altro asse lo interseca, l'aspetto dei numeri e i segni di graduazione.

## I pulsanti accanto a un grafico

Tre piccoli pulsanti si trovano a destra di un grafico selezionato.

- **Elementi grafico** (il segno più) apre lo stesso menu di **Elementi grafico** della scheda Grafico.
- **Colori e stili del grafico** (il pennello) elenca le combinazioni di colori e gli aspetti.
- **Filtri grafico** (l'imbuto) elenca le serie. Deseleziona una serie per escluderla dal grafico. Il file la conserva comunque, quindi torna visibile quando la selezioni di nuovo. Un grafico mantiene sempre almeno una serie visibile.

## Grafici in altre presentazioni

Un grafico contenuto in un file **.pptx** si apre come grafico modificabile quando i suoi numeri sono memorizzati all'interno del grafico stesso. Se un grafico legge i propri numeri da un foglio di calcolo salvato insieme ad esso, puoi modificare tutto tranne i numeri. Un grafico con parti che JeffDeck non modifica, come effetti speciali, viene conservato così com'è.

## Salvataggio, stampa e PDF

- Un file **.pptx** conserva il grafico come grafico, quindi si apre come grafico qui e in altri programmi.
- Un file **.odp** non prevede un modo per salvare un grafico che possa essere riaperto come tale. JeffDeck salva il grafico come immagine e il messaggio visualizzato dopo il salvataggio lo indica.
- **PDF** e la stampa disegnano il grafico come linee, forme e testo reale, non come immagine, quindi resta nitido a qualsiasi dimensione.
