# Grafici

<!-- keywords: grafico, grafici, diagramma, istogramma, grafico a colonne, grafico a barre, grafico a linee, grafico a torta, grafico a ciambella, area, dispersione, inserisci grafico, dati grafico, modifica dati, titolo grafico, legenda, etichette dati, tipo di grafico, formato grafico, assi, titolo asse, linee della griglia, stile grafico, elementi grafico, layout, colori, stili, scambia righe e colonne, tipo grafico, formato asse, barre di errore, linea di tendenza, filtri grafico, elementi del grafico -->

Un **grafico** trasforma i numeri in un'immagine, per esempio in colonne, in una linea o in fette di una torta. Questo argomento spiega come inserire un grafico in un documento, come digitarne i numeri e come modificarne l'aspetto.

## Inserire un grafico

1. Posiziona il cursore nel punto in cui deve comparire il grafico.
2. Fai clic su **Inserisci > Grafico** e scegli un tipo: colonne, colonne impilate, colonne impilate al 100%, barre, barre impilate, linee, linee con indicatori, torta, ciambella, area, area impilata, dispersione, radar, radar con indicatori, radar riempito, massimo-minimo-chiusura, apertura-massimo-minimo-chiusura, volume-massimo-minimo-chiusura o volume-apertura-massimo-minimo-chiusura.

Un grafico **azionario** mostra i prezzi: per ogni categoria una linea va dal prezzo più alto a quello più basso, con un piccolo punto per la chiusura oppure, quando c'è l'apertura, una barra tra apertura e chiusura (chiara per un aumento, scura per un calo). Il grafico viene costruito con le colonne esattamente nell'ordine in cui si trovano, da sinistra a destra, e nessun controllo verifica che i numeri siano adatti alla loro posizione; perciò inseriscili in questo ordine: massimo, minimo e chiusura; apertura, massimo, minimo e chiusura; volume, massimo, minimo e chiusura; oppure volume, apertura, massimo, minimo e chiusura. Se le colonne sono in un altro ordine il grafico viene comunque disegnato, ma i segni hanno un significato diverso (il punto della chiusura va sulla terza colonna dei prezzi e la scatola va dalla prima alla ultima colonna dei prezzi). Con il volume, questo è letto sull'asse sinistro come colonne, mentre i prezzi sono letti su un secondo asse a destra. Se il numero di colonne non corrisponde al tipo, JeffOffice indica quante ne servono e non crea alcun grafico.

Un grafico **impilato** sovrappone le serie una sull'altra. Un grafico **impilato al 100%** rende ogni pila della stessa altezza, così vedi le proporzioni. Un grafico a **dispersione** colloca ogni punto dove indicano i suoi due numeri.

Il grafico viene inserito con numeri di esempio, allineato al testo, come una lettera grande. Si apre la scheda **Formato grafico**.

## Digitare i propri numeri

Seleziona il grafico e fai clic su **Formato grafico > Modifica dati**. Si apre una tabella.

- La prima colonna contiene le parole sotto i punti (le **categorie**), per esempio i mesi.
- La prima riga contiene il nome di ogni **serie**, cioè di un insieme di numeri, per esempio di un prodotto.
- Le altre celle contengono i numeri.

Fai clic su **Aggiungi riga** o **Aggiungi colonna** per aggiungerne, oppure su **Rimuovi ultima riga** o **Rimuovi ultima colonna** per toglierne. Fai clic su **OK**. Se una cella non contiene un numero, il grafico non viene modificato e la barra di stato indica la riga interessata. Un grafico può avere fino a 200 categorie e 24 serie.

Per un grafico a dispersione, la prima colonna contiene i numeri che collocano i punti da sinistra a destra.

## Grafici salvati da un altro programma

Alcuni grafici salvati da altri programmi conservano i propri numeri in una piccola **cartella di lavoro** (un file di foglio di calcolo) memorizzata all'interno del file. Seleziona un grafico di questo tipo e fai clic su **Formato grafico > Modifica dati**. La tabella mostra le celle di quella cartella di lavoro che il grafico legge. Modifica un numero, un nome sotto un punto o il nome di una serie, quindi fai clic su **OK**. La modifica viene applicata insieme alla cartella di lavoro e al grafico, come un'unica operazione annullabile, così anche gli altri programmi mostrano i nuovi numeri.

Non puoi aggiungere o rimuovere righe o colonne di questo tipo di grafico, e i comandi **Dati grafico** e **Scambia righe e colonne** sono disattivati, perché il grafico è legato a quelle celle. Lo stesso accade quando il grafico legge celle di un altro file, in un intervallo che non è una sola colonna o una sola riga, oppure in celle che contengono una formula, una data o parole dove dovrebbe esserci un numero. Il comando è disattivato e la sua descrizione comandi spiega il motivo.

## Cambiarne l'aspetto

Questi comandi si trovano nella scheda **Formato grafico**. Ciascuno è un'unica operazione annullabile, e il grafico conserva i propri colori e il resto dell'aspetto.

- **Cambia tipo di grafico** trasforma il grafico in un altro tipo con gli stessi numeri. Per passare a un tipo azionario, le serie del grafico devono essere nell'ordine richiesto da quel tipo; se non lo sono, viene indicato l'ordine corretto e il grafico resta invariato.
- **Titolo grafico** mostra o nasconde il titolo. **Modifica titolo grafico** ne cambia il testo.
- **Legenda** è la chiave che indica quale colore corrisponde a quale serie. Puoi collocarla a destra, in alto, a sinistra o in basso, oppure eliminarla.
- **Assi** mostra o nasconde la linea delle parole sotto il grafico (le **categorie**) e la linea dei numeri accanto a essa (i **valori**), e imposta un titolo per ciascuna. Un grafico a torta non ha assi.
- **Linee della griglia** disegna linee leggere attraverso il grafico per aiutare a seguire i numeri. Le linee **principali** si trovano in corrispondenza dei numeri principali sul lato, mentre le linee **secondarie** si trovano tra di esse.
- **Stili grafico** sceglie uno degli otto stili complessivi.
- **Etichette dati** scrive ogni numero sulla sua colonna, sul suo punto o sulla sua fetta.
- **Disposizione testo**, **Testo alternativo**, e **Altezza** e **Larghezza** funzionano come per le immagini. Vedi [Aggiungere e modificare le immagini](pictures).

## La scheda Grafico

Seleziona un grafico e si apre la scheda **Grafico** accanto alla scheda **Formato grafico**. Tutto ciò che vi si trova modifica il grafico in un'unica operazione annullabile.

- **Elementi grafico** aggiunge, rimuove o colloca una parte del grafico: **Assi**, **Titoli assi**, **Titolo grafico**, **Etichette dati** (cosa contengono, dove si trovano e il segno che le separa), **Linee della griglia**, **Legenda**, **Barre di errore** (barre brevi che mostrano di quanto un numero può essere impreciso), **Linee** (linee di caduta, linee massimo-minimo e barre di salita e discesa) e **Tabella dati** (una griglia dei numeri sotto il grafico, con una colonna per ogni categoria; il suo elenco attiva e disattiva la tabella e imposta le linee tra le righe, le linee tra le colonne, il contorno e le chiavi della legenda, che mostrano accanto al nome il colore di ogni serie; è offerta per i grafici a colonne, a linee e ad area) e **Linea di tendenza** (una linea che mostra dove si dirigono i numeri, con la sua equazione, se la vuoi).
- **Layout** imposta più parti insieme, come titolo, legenda, etichette e linee della griglia.
- **Colori** assegna alle serie un nuovo insieme di colori. **Stili** dà all'intero grafico un aspetto, come semplice, pulito, scuro o morbido.
- **Scambia righe e colonne** inverte i numeri: le serie diventano le parole sotto i punti, e quelle parole diventano le serie.
- **Dati grafico** elenca le serie. Seleziona quelle da disegnare, cambiane i nomi e spostale su o giù.
- **Modifica dati** apre la tabella dei numeri.
- **Tipo di grafico** mostra un'immagine di ciascun tipo. Scegli un tipo per l'intero grafico oppure un tipo per ciascuna serie, per esempio colonne con una linea sopra.
- **Formatta elemento grafico** imposta il riempimento e il contorno dell'area del grafico, dell'area del diagramma (il riquadro in cui si trovano i segni) o di una singola serie. Per una linea, imposta anche se è smussata e quale segno compare in ogni punto.
- **Selezione corrente** elenca le parti del grafico: l'area del grafico, l'area del diagramma e ogni serie. Scegline una e si apre la finestra di formato su quella parte con **Formato selezione**. **Reimposta in base allo stile** elimina ciò che era stato impostato sulla parte scelta e ripristina ciò che le assegna lo stile del grafico. Per l'area del grafico si tratta del riempimento, del contorno e del colore del testo impostato da uno stile (il testo chiaro dello stile scuro torna al colore del testo del tema). Per l'area del diagramma si tratta del riempimento e del contorno. Per una serie si tratta del colore (quello assegnato dalla scelta Colori o dallo stile del grafico), del contorno, dei segni così come li prevede il tipo di grafico e dei colori dei suoi punti; una linea smussata resta tale se il tipo di grafico è smussato. Non modifica titoli, legenda, etichette, assi, linee della griglia o dati, e non fa nulla su una parte che già corrisponde allo stile. È un'operazione annullabile.
- **Formato asse orizzontale** e **Formato asse verticale** impostano i numeri minimo e massimo, l'ampiezza di ciascun intervallo, l'ordine, il punto in cui l'altro asse interseca, l'aspetto dei numeri e i segni di graduazione.

## I pulsanti accanto a un grafico

Tre piccoli pulsanti si trovano a destra di un grafico selezionato.

- **Elementi grafico** (il segno più) apre lo stesso menu di **Elementi grafico** della scheda Grafico.
- **Stili e colori del grafico** (il pennello) elenca gli insiemi di colori e gli stili.
- **Filtri grafico** (l'imbuto) elenca le serie. Deseleziona una serie per escluderla dal grafico. Il file la conserva comunque, quindi ricompare quando la selezioni di nuovo. Un grafico mantiene sempre almeno una serie visibile.

## Salvare un grafico in un file OpenDocument

Se salvi il documento come **testo OpenDocument** (`.odt`), ogni grafico viene inserito come oggetto a sé e, riaperto, rimane un grafico modificabile. Restano il tipo, i numeri, il titolo, la legenda, le etichette, i colori, gli assi, le linee della griglia, fino a quattro linee di tendenza per ogni serie e le barre di errore. Alcune parti non rientrano in quel formato, come la tabella dati, le linee di caduta, le linee massimo-minimo, le barre di salita e discesa e le barre di errore i cui valori sono stati digitati. JeffWrite elenca ciò che è stato escluso quando salvi.

## Grafici in altri documenti

JeffWrite mostra i grafici contenuti in un documento e li conserva così come sono. Può cambiare il tipo, il titolo, la legenda e le etichette di un grafico che è in grado di scrivere per intero. Se un grafico ha parti che JeffWrite non modifica, come effetti speciali, lo segnala e lascia il grafico invariato. Se un grafico legge i propri numeri da un foglio di calcolo conservato con esso, puoi modificare tutto tranne i numeri.
