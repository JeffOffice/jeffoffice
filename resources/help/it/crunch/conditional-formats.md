# Formattazione condizionale
<!-- keywords: formattazione condizionale, formato condizionale, formati condizionali, regola, evidenziare, evidenziazione, scala colori, scala di colori, barre dei dati, barra dati, set di icone, icone, primi, ultimi, sopra la media, sotto la media, duplicati, valori univoci, formula, interrompi se vera, priorità, barre, frecce, semafori, sfumatura, formato differenziale -->

Un **formato condizionale** è una regola che cambia l'aspetto di una cella quando il suo contenuto soddisfa una condizione. Esempio: una regola dice "colora di rosso una cella se è minore di 100". Una cella che contiene 80 è rossa, una cella che contiene 120 no, e quando si cambia 80 in 150 il rosso scompare da solo. Il formato della cella non viene modificato: la regola decide solo cosa viene visualizzato.

## Cosa mostra JeffCrunch

JeffCrunch mostra le regole presenti in una cartella di lavoro:

- **Regole di evidenziazione**: una regola che cambia il carattere, il riempimento, il bordo o il formato numero delle celle che la soddisfano. La condizione può essere un confronto con un valore (maggiore di, tra, uguale a e così via), una formula, un testo che contiene determinate parole, inizia o termina con esse, una data come "settimana scorsa", celle vuote o con errori, valori compresi tra i più alti o i più bassi (un numero o una percentuale), valori sopra o sotto la media e valori duplicati o univoci.
- **Scale di colori**: ogni cella viene riempita con un colore che varia tra due o tre colori man mano che il suo numero passa da basso ad alto. Una scala dal bianco al verde rende più verdi i numeri più grandi.
- **Barre dei dati**: una barra nella cella, tanto più lunga quanto più grande è il numero. Le barre possono essere a tinta unita o sfumate, avere un bordo e un asse, e per i numeri negativi vanno nella direzione opposta, con un altro colore.
- **Set di icone**: una piccola immagine nella cella che indica in quale parte dell'intervallo si trova il numero, ad esempio una freccia in su, una freccia orizzontale o una freccia in giù, oppure un semaforo rosso, giallo o verde. I set disponibili sono venti.

Quando due regole riguardano la stessa cella, per gli aspetti impostati da entrambe prevale quella che viene prima nell'ordine della cartella di lavoro, e una regola contrassegnata con "interrompi se vera" termina la ricerca. I numeri di una scala di colori, di una barra dei dati o di un set di icone vengono calcolati sulle celle della regola stessa e vengono ricalcolati quando una di quelle celle cambia.

## Cosa rimane invariato

- Le regole seguono le celle a cui appartengono. Se si inseriscono o eliminano righe o colonne, oppure si taglia e incolla, le regole continuano a coprire le celle giuste; **Annulla** le ripristina.
- Una regola che JeffCrunch non visualizza viene conservata nel file così com'è.
- Il salvataggio scrive le regole presenti nella cartella di lavoro, senza modificarle.

## Cosa non è ancora disponibile

**Non è ancora possibile creare, modificare o eliminare una regola.** JeffCrunch non ha un comando **Formattazione condizionale**: mostra ciò che la cartella di lavoro contiene già e lo mantiene corretto durante le modifiche. La modifica delle regole arriverà in una versione successiva. Fino ad allora, creare o modificare le regole nel programma con cui è stata creata la cartella di lavoro.
