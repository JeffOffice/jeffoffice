# Inserire ed eliminare righe, colonne e celle

<!-- keywords: inserisci celle tagliate, inserisci celle copiate, sposta a destra, sposta in basso, inserisci, elimina, rimuovi, riga, colonna, cella, sposta, aggiungi, vuoto, Ctrl+più, Ctrl+meno, clic destro, intestazione, annulla, riferimento, #RIF!, formula, nome, unite, tabella, matrice, bordo, inserimento, eliminazione -->

Puoi aggiungere righe, colonne o celle vuote in mezzo ai dati, oppure rimuovere righe, colonne o celle. Tutto ciò che si trova sotto o a destra si sposta per fare spazio o per chiudere il vuoto, e le formule che fanno riferimento alle celle spostate continuano a puntare alle stesse celle.

## Inserire

1. Seleziona i numeri di riga, le lettere di colonna o le celle in cui vuoi inserire le nuove. Per aggiungere tre righe, seleziona tre righe.
2. Premi **Ctrl e il tasto più** (**Ctrl e il tasto meno** elimina), oppure usa **Home > Inserisci**, oppure fai clic con il pulsante destro sulla selezione e scegli **Inserisci**.

Se hai selezionato righe o colonne intere, le nuove vengono inserite tutte insieme, sopra le righe o a sinistra delle colonne. Se hai selezionato delle celle, una piccola finestra chiede come devono spostarsi le celle successive:

- **Sposta celle a destra** oppure **Sposta celle in basso** sposta soltanto le celle nelle colonne (o nelle righe) della selezione e lascia il resto del foglio dove si trova.
- **Riga intera** oppure **Colonna intera** inserisce intere righe o colonne.

## Inserire celle tagliate o copiate

Per inserire tra altre celle, anziché sopra di esse, le celle che hai tagliato o copiato, tagliale o copiale prima (**Ctrl+X** oppure **Ctrl+C**). Poi seleziona la cella in cui devono andare e usa **Home > Inserisci > Inserisci celle tagliate o copiate** (il comando diventa **Inserisci celle tagliate** o **Inserisci celle copiate** quando c'è qualcosa da inserire), oppure fai clic con il pulsante destro sulla cella e scegli la voce. Una piccola finestra chiede se le celle presenti devono **spostarsi a destra** o **spostarsi in basso** per fare spazio.

Il pulsante **Inserisci** e i tasti **Ctrl e il tasto più** (anche **Ctrl+Maiusc e il tasto più**) fanno lo stesso, ma solo quando è chiaro che cosa intendi:

- quando le celle sono **tagliate** e il taglio è ancora in attesa (lo indica il bordo tratteggiato) nel foglio di provenienza, e
- quando le celle sono **copiate** e non hai cambiato nulla da allora (nessuna digitazione, nessun'altra modifica, nessun Annulla o Ripristina).

In tutti gli altri casi, il pulsante e i tasti inseriscono celle vuote, come sempre. Dopo una modifica, la copia resta disponibile per **Incolla** e per il comando del menu. **Esc** annulla un taglio o una copia: il bordo scompare, e il pulsante e i tasti tornano a inserire celle vuote. Se tagli celle in un foglio e premi i tasti in un altro, vengono inserite celle vuote; il comando del menu indica che le celle tagliate possono essere inserite tra altre celle solo nel proprio foglio. Con righe o colonne intere selezionate il comando non è disponibile, e Inserisci aggiunge righe o colonne come prima.

- Le celle tagliate **si spostano**: lasciano il loro posto originale, e ciò che vi fa riferimento le segue. È lo stesso effetto che si ottiene tenendo premuto **Maiusc** mentre trascini il bordo della selezione. L'operazione riesce completamente oppure non viene eseguita affatto: se una parte viene rifiutata, non cambia nulla.
- Le celle copiate **vengono inserite** come copia, come farebbe **Incolla** (come indica **Imposta incolla predefinito**, con le formule modificate per la nuova posizione), e la copia resta pronta, così puoi inserirla di nuovo in un altro punto.
- Le celle vengono inserite una sola volta, nella cella in alto a sinistra di ciò che hai selezionato, anche se hai selezionato più celle di quelle copiate.
- Se l'elenco è filtrato, le celle vanno nelle righe visibili, come avviene con un'incolla, e lo spazio creato comprende anche le righe nascoste dal filtro che si trovano in mezzo. Una riga nascosta manualmente viene trattata come qualsiasi altra riga.
- Ogni operazione è un singolo passaggio che puoi annullare. Le celle tagliate possono essere inserite tra altre celle solo nel foglio di provenienza; per spostarle in un altro foglio, usa **Incolla**.
- Una cella non può essere inserita nel proprio percorso (celle tagliate da A3:A4 non possono andare tra A3 e A4), e un foglio protetto lo impedisce. JeffCrunch indica il motivo e non modifica nulla.

## Eliminare

Seleziona ciò che vuoi rimuovere e premi **Ctrl e il tasto meno**, oppure usa **Home > Elimina**. Con celle selezionate, la finestra chiede se **spostare le celle a sinistra**, **spostare le celle in alto**, oppure eliminare la **riga intera** o la **colonna intera**. L'eliminazione è un singolo passaggio, e **Annulla** ripristina tutto ciò che è stato rimosso, comprese le formule.

## Effetti sulle formule e sugli altri elementi

Una formula che fa riferimento a celle spostate punta a esse nella loro nuova posizione. Esempio: in B1 hai **=SOMMA(A1:A5)**. Se inserisci una riga sopra la riga 3, la formula diventa **=SOMMA(A1:A6)**, perché l'intervallo si allarga per includere la nuova riga. Se elimini la riga 3, diventa **=SOMMA(A1:A4)**. Una formula che puntava a una cella eliminata mostra **#RIF!**, cioè la cella di cui ha bisogno non esiste più. **Annulla** ripristina la cella e la formula.

Seguono le celle anche: le formule di tutti i fogli, i nomi (come le aree di stampa e i titoli di stampa), le celle unite, le celle a cui si applicano le regole di convalida e le formattazioni condizionali, le tabelle, i grafici, le immagini, le note, i collegamenti ipertestuali e le interruzioni di pagina.

## Operazioni non possibili

- Non è possibile inserire o eliminare celle in una parte di una **formula matrice** o di una tabella dati. Sposta le celle per intero, oppure non toccarle. Non puoi eliminare nemmeno una cella che una tabella dati usa come input.
- Una **tabella pivot** non può ancora essere spostata o tagliata: non è possibile inserire o eliminare righe, colonne o celle dove ciò la sposterebbe o la modificherebbe, né tagliare celle da essa o inserirvi celle. Il resto del foglio può essere modificato normalmente. Una tabella pivot si modifica con i comandi della scheda **Tabella pivot** (aggiorna, cambia origine dati, campi e layout), non digitando nelle sue celle.
- Una **tabella** non può essere tagliata in verticale: non puoi inserire una colonna al suo interno, né eliminare la sua riga di intestazione o tutte le sue righe. Inserisci o elimina righe al suo interno, oppure modifica le celle che la circondano.
- Un **foglio protetto** non consente di inserire o eliminare righe, colonne o celle. Per farlo, scegli Rimuovi protezione foglio nella scheda Revisione.
- Le celle alla fine del foglio che contengono dati non possono essere spinte fuori dal bordo. Eliminale prima.
- Dopo il salvataggio, **Annulla** continua a tornare indietro su ciò che hai fatto prima, comprese le righe e le colonne inserite (al salvataggio successivo verranno di nuovo rimosse). L'unica cosa che non può essere ripristinata sono le righe, le colonne o le celle **eliminate**: il file salvato ha già perso ciò che vi era contenuto, quindi **Annulla** si ferma all'ultima eliminazione salvata, e la riga in fondo alla finestra lo segnala al salvataggio.
- Un passaggio che sposta o elimina così tante celle da non poter essere contenuto da **Annulla** (più di circa 128 MB) chiede prima conferma, come fanno i programmi per fogli di calcolo più diffusi: "Questa operazione non può essere annullata." Se prosegui, l'operazione viene eseguita e **Annulla** non torna indietro né su di essa né su ciò che l'ha preceduta.
