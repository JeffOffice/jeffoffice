# Inserire ed eliminare righe, colonne e celle
<!-- keywords: inserire, inserimento, eliminare, eliminazione, rimuovere, cancellare, riga, righe, colonna, colonne, cella, celle, spostare, sposta celle, aggiungere, vuota, Ctrl+più, Ctrl+meno, clic destro, intestazione, annulla, riferimento, #RIF!, formula, nome, unite, tabella, matrice, bordo -->

Puoi aggiungere righe, colonne o celle vuote in mezzo ai dati ed eliminare righe, colonne o celle. Tutto ciò che segue si sposta per fare spazio o per chiudere il vuoto, e le formule che fanno riferimento alle celle spostate continuano a puntare a quelle celle.

## Inserire

1. Seleziona i numeri di riga, le lettere di colonna o le celle nel punto in cui vuoi inserire quelle nuove. Per aggiungere tre righe, selezionane tre.
2. Premi **Ctrl e il tasto più** (**Ctrl e il tasto meno** elimina), oppure usa **Home > Inserisci**, oppure fai clic con il pulsante destro del mouse sulla selezione e scegli **Inserisci**.

Se sono selezionate righe o colonne intere, quelle nuove vengono inserite subito, sopra le righe o a sinistra delle colonne. Se sono selezionate delle celle, una piccola finestra chiede come spostare le celle successive:

- **Sposta le celle a destra** o **Sposta le celle in basso** sposta solo le celle nelle colonne (o nelle righe) della selezione e lascia il resto del foglio dov'è.
- **Riga intera** o **Colonna intera** aggiunge righe o colonne intere.

## Eliminare

Seleziona ciò che vuoi eliminare e premi **Ctrl e il tasto meno**, oppure usa **Home > Elimina**. Se sono selezionate delle celle, la finestra chiede se **spostare le celle a sinistra**, **spostare le celle in alto** oppure eliminare la **riga intera** o la **colonna intera**. L'eliminazione è un unico passaggio e **Annulla** ripristina tutto ciò che è stato eliminato, comprese le formule.

## Cosa succede dopo

Una formula che fa riferimento a celle che si spostano punta a quelle celle nella nuova posizione. Esempio: in B1 c'è **=SOMMA(A1:A5)**. Se inserisci una riga sopra la riga 3, la formula diventa **=SOMMA(A1:A6)**, perché l'intervallo si è ampliato per includere la nuova riga. Se elimini la riga 3, diventa **=SOMMA(A1:A4)**. Una formula che faceva riferimento a una cella eliminata mostra **#RIF!**, che significa che la cella necessaria non esiste più. **Annulla** ripristina la cella e la formula.

Seguono le celle anche: le formule in tutti i fogli, i nomi (come le aree di stampa e i titoli di stampa), le celle unite, le celle a cui si applicano le regole di convalida e i formati condizionali, le tabelle, i grafici, le immagini, le note, i collegamenti ipertestuali e le interruzioni di pagina.

## Cosa non si può fare

- Non è possibile inserire o eliminare celle attraverso una parte di una **formula matrice** o di una tabella dati. Spostale per intero oppure lasciale come sono. Non è nemmeno possibile eliminare una cella che una tabella dati legge come input.
- Una **tabella pivot** per ora non può essere spostata né attraversata: non è possibile inserire o eliminare righe, colonne o celle dove questo la sposterebbe o la modificherebbe, né tagliare celle al suo interno o incollarle su di essa. Il resto del foglio si può modificare normalmente. Una tabella pivot si modifica con i comandi della scheda **Tabella pivot** (aggiornamento, cambio dell'origine dati, campi e layout), non digitando nelle sue celle.
- Una **tabella** non può essere attraversata: non puoi inserire una colonna al suo interno, né eliminarne la riga di intestazione o tutte le righe. Inserisci o elimina righe al suo interno, oppure modifica le celle che la circondano.
- In un **foglio protetto** non è possibile inserire o eliminare righe, colonne o celle. Per farlo, scegli Rimuovi protezione foglio nella scheda Revisione.
- Le celle con contenuto alla fine del foglio non possono essere spinte oltre il bordo. Eliminale prima.
- Dopo il salvataggio, **Annulla** può ancora tornare indietro sulle operazioni precedenti, comprese le righe e le colonne inserite (il salvataggio successivo le toglie di nuovo). L'unica cosa che non può ripristinare sono le righe, le colonne o le celle **eliminate**: il file salvato ha perso il loro contenuto, quindi **Annulla** si ferma all'ultima eliminazione salvata, e la riga in fondo alla finestra lo segnala al momento del salvataggio.
- Un'operazione che sposta o elimina così tante celle che **Annulla** non potrebbe memorizzarla (più di circa 128 MB) chiede prima conferma, come fanno i fogli di calcolo più diffusi: "Questa operazione non può essere annullata". Se prosegui, l'operazione viene eseguita e **Annulla** non può tornare indietro né su di essa né su ciò che la precede.
