# Formattazione condizionale
<!-- keywords: formattazione condizionale, formato condizionale, regola, evidenzia, scala di colori, barra dati, set di icone, icone, primi, ultimi, sopra la media, sotto la media, duplicato, univoco, formula, interrompi se vero, priorità, barre, frecce, semafori, sfumatura, formato differenziale, pulsante Formattazione condizionale, cancella regole, maggiore di, minore di, compreso tra, testo che contiene, valori duplicati, primi 10 -->

Una **formattazione condizionale** è una regola che cambia l'aspetto di una cella quando il suo contenuto soddisfa una condizione. Esempio: una regola dice "colora una cella di rosso se è inferiore a 100". Una cella che contiene 80 è rossa, una cella che contiene 120 non lo è e, quando cambi 80 in 150, il rosso scompare da solo. Il formato della cella non viene modificato: la regola decide soltanto che cosa viene disegnato.

## Che cosa mostra JeffCrunch

JeffCrunch mostra le regole presenti in una cartella di lavoro:

- **Regole di evidenziazione**: una regola che cambia il carattere, il riempimento, il bordo o il formato numero delle celle che la soddisfano. La condizione può essere un confronto con un valore (maggiore di, compreso tra, uguale a e così via), una formula, un testo che contiene, inizia con o termina con alcune parole, una data come "settimana scorsa", celle vuote o con errori, valori tra i più alti o più bassi (un numero o una percentuale), valori superiori o inferiori alla media, e valori duplicati o univoci.
- **Scale di colori**: ogni cella viene riempita con un colore che passa da due o tre colori man mano che il suo numero va da basso ad alto. Una scala dal bianco al verde rende più verdi i numeri più grandi.
- **Barre dati**: una barra nella cella lunga quanto il numero è grande. Le barre possono avere un colore pieno o sfumare, possono avere un bordo, un asse, e possono andare nel senso opposto, con un altro colore, per i numeri negativi.
- **Set di icone**: una piccola immagine nella cella che indica in quale parte dell'intervallo si trova il numero, come una freccia in su, una freccia laterale o una freccia in giù; oppure un semaforo rosso, giallo o verde. Ci sono venti set.

Quando due regole riguardano la stessa cella, vince quella che viene prima nell'ordine della cartella di lavoro per ciò che entrambe impostano, e una regola contrassegnata con "interrompi se vero" termina la ricerca. I numeri di una scala di colori, di una barra dati o di un set di icone vengono calcolati sulle celle della regola, e vengono ricalcolati quando una di quelle celle cambia.

## Che cosa resta invariato

- Le regole seguono le celle a cui appartengono. Inserendo o eliminando righe o colonne, oppure tagliando e incollando, le regole continuano a coprire le celle giuste; **Annulla** le ripristina.
- Una regola che JeffCrunch non disegna viene conservata nel file così com'è.
- Il salvataggio scrive le regole che la cartella di lavoro aveva. Quando crei o cancelli regole in un foglio, le regole del foglio vengono scritte di nuovo con le tue modifiche.

## Creare una regola

1. Seleziona le celle. Puoi selezionare intere colonne o l'intero foglio; la regola viene conservata per l'intervallo, non per ogni singola cella.
2. Fai clic su **Home > Formattazione condizionale**.
3. In **Regole evidenziazione celle**, scegli **Maggiore di**, **Minore di**, **Compreso tra**, **Uguale a**, **Testo che contiene**, **Una data che si verifica** o **Valori duplicati**. In **Regole primi/ultimi**, scegli **Primi 10 elementi**, **Ultimi 10 elementi**, **Primi 10%**, **Ultimi 10%**, **Superiore alla media** o **Inferiore alla media**.
4. Digita il valore (oppure i due valori per **Compreso tra**, il testo per **Testo che contiene**, o quanti elementi per le regole Primi e Ultimi), oppure scegli le date. Per **Primi 10 elementi**, "10" indica quante celle assumono l'aspetto indicato. Puoi cambiarlo con un altro numero.
5. Scegli l'aspetto delle celle, ad esempio **Riempimento rosso chiaro con testo rosso scuro**, e fai clic su **OK**. L'esempio accanto all'elenco mostra l'aspetto.

Per creare un aspetto personalizzato, scegli **Formato personalizzato...** in fondo all'elenco. Si apre una finestra con quattro schede: **Numero**, **Carattere** (lo stile, la sottolineatura, il barrato e il colore), **Bordo** (solo tipi di linea sottili) e **Riempimento** (un colore, oppure un motivo con due colori). La finestra parte dall'aspetto che avevi scelto, e una parte che non modifichi resta com'era. Fai clic su **OK** e l'esempio mostra il tuo aspetto.

La regola viene applicata subito. Ogni nuova regola viene messa per prima, quindi quando due regole vogliono modificare la stessa cosa, vince la più recente. Creare una regola è un'operazione che puoi annullare.

## Barre dati, scale di colori e set di icone

Fai clic su **Home > Formattazione condizionale**, poi su **Barre dati**, **Scale di colori** o **Set di icone**, e fai clic su una scelta. La regola viene applicata subito alle celle selezionate, senza una finestra da compilare, come un'unica operazione che puoi annullare.

- **Barre dati** sono disponibili in sei colori, ciascuno con riempimento sfumato o a tinta unita.
- **Scale di colori** offre dodici scelte, con due o tre colori. Il nome indica prima il colore dei numeri più alti.
- **Set di icone** offre venti set: direzionali (frecce e triangoli), forme (semafori e segnali), indicatori (simboli e bandiere) e valutazioni (stelle, quarti e caselle). Le icone dividono l'intervallo in passi uguali.

**Nuova regola** in fondo al menu apre un'unica finestra per ogni tipo di regola, compresa una regola con una formula personalizzata, e una barra dati, una scala di colori o un set di icone con impostazioni proprie.

## Gestisci regole

**Home > Formattazione condizionale > Gestisci regole** elenca le regole del foglio nell'ordine in cui vengono applicate. Usa **Nuova regola**, **Modifica regola** ed **Elimina regola**; **Sposta su** e **Sposta giù** cambiano l'ordine; digita altre celle in **Si applica a**; e seleziona **Interrompi se vero**. Modifica regola cambia tutto ciò che una regola contiene:

- per una **barra dati**: che cosa rappresentano la barra più corta e quella più lunga (valore più basso o più alto, un numero, una percentuale, un percentile o una formula), riempimento sfumato o a tinta unita, i colori, un bordo, il colore delle barre negative, dove si trova l'asse, in quale senso vanno le barre, e **Mostra solo barra**;
- per una **scala di colori**: due o tre colori, e che cosa rappresenta ciascun colore;
- per un **set di icone**: lo stile delle icone, il valore da cui inizia ciascuna icona, **Inverti ordine icone**, e **Mostra solo icona**;
- per le altre regole: il valore, il testo, le date o la formula, e l'aspetto. Una regola che un file ha importato con un aspetto proprio è elencata con l'aspetto **Formato personalizzato**, e l'esempio mostra come appare davvero. Se modifichi un'altra caratteristica della regola, il suo aspetto resta esattamente come lo aveva il file. Scegli **Formato personalizzato...** per cambiare l'aspetto.

Facendo clic su **OK** tutte le modifiche vengono applicate come un'unica operazione che puoi annullare. Una regola che non hai modificato conserva esattamente ciò che aveva nel file, qualunque sia la sua posizione nell'elenco; una regola modificata viene scritta di nuovo. La formula di una regola viene letta dalla cella in alto a sinistra del suo intervallo, e una formula che digiti per una nuova regola viene letta come se fosse scritta nella cella con il cursore. Un foglio protetto che non consente di formattare le celle non accetta regole.

## Che cosa conservano gli altri tipi di file

Solo una cartella di lavoro .xlsx conserva le formattazioni condizionali. Il salvataggio in un tipo di file meno recente (.xls o .ods) le omette, e Verifica compatibilità lo segnala, indicando barre dati, scale di colori e set di icone quando il foglio ne contiene. Un file .csv non conserva alcuna formattazione. Le formattazioni condizionali vengono stampate così come appaiono sullo schermo.

## Cancella regole

**Cancella regole dalle celle selezionate** elimina tutte le regole presenti in una qualsiasi delle celle selezionate. **Cancella regole da questo foglio** elimina tutte le regole del foglio. Ciascuna operazione può essere annullata. Una scelta non ha effetto quando non ci sono regole da cancellare.

## Che cosa non è ancora disponibile

- Un foglio che contiene barre dati nella forma di un file più recente le conserva così come sono, e JeffCrunch non crea, non modifica e non cancella regole su quel foglio. Apporta queste modifiche con il programma che ha creato la cartella di lavoro.
- Una cartella di lavoro aperta da un tipo di file meno recente conserva le sue regole quando la salvi come cartella di lavoro .xlsx, e dopo puoi aggiungere altre regole.
- Un aspetto personalizzato non può includere una linea media, spessa, doppia o inclinata, una linea diagonale, un riempimento sfumato, né un tipo o una dimensione di carattere: queste non fanno parte di ciò che una regola può modificare.
