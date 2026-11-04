# Inserire testo nelle celle
<!-- keywords: digitare, digitazione, invio, modifica, modificare, cambiare, cella, voce, barra della formula, F2, doppio clic, Tab, Esc, annulla immissione, annulla, ripeti, eliminare, cancellare, percentuale, valuta, denaro, data, ora, numero, testo, apostrofo, formula, uguale, sostituire, limite, caratteri -->

Per inserire un contenuto in una cella, seleziona la cella e inizia a digitare. Quello che digiti sostituisce il contenuto della cella. Premi **Invio** per confermarlo oppure **Esc** per annullare l'immissione e lasciare la cella com'era.

## Tre modi per iniziare

- **Digitare direttamente nella cella.** Seleziona una cella e digita. Il primo carattere avvia l'immissione, e la cella mostra quello che scrivi man mano.
- **Modificare il contenuto della cella.** Premi **F2** oppure fai doppio clic sulla cella. La cella si apre con il suo contenuto, e il cursore si trova alla fine. Usa i tasti freccia per spostarti nel testo.
- **Usare la barra della formula.** Fai clic nella barra della formula, accanto alla casella Nome, e digita lì. Anche la cella mostra quello che scrivi.

Quando digiti direttamente nella cella, un tasto freccia completa l'immissione e sposta la selezione. Quando modifichi il contenuto della cella, i tasti freccia si spostano nel testo. Premi **F2** per passare dall'una all'altra modalità.

## Completare l'immissione

| Tasti | Cosa fanno |
| --- | --- |
| Invio | Conferma l'immissione e sposta il cursore in basso (oppure nella direzione indicata in **File > Opzioni > Modifica**) |
| Maiusc+Invio | Conferma l'immissione e sposta il cursore nella direzione opposta |
| Ctrl+Invio | Inserisce il contenuto in tutte le celle selezionate e resta nella posizione attuale |
| Tab | Conferma l'immissione e sposta il cursore a destra |
| Maiusc+Tab | Conferma l'immissione e sposta il cursore a sinistra |
| Tasti freccia | Confermano l'immissione e spostano il cursore di una cella (solo quando hai digitato direttamente nella cella) |
| Esc | Annulla l'immissione |
| Alt+Invio | Inserisce un a capo all'interno della cella |
| Ctrl+; | Inserisce la data odierna (nell'immissione, quando stai digitando una data) |
| Ctrl+Maiusc+; | Inserisce l'ora attuale |
| Alt+Freccia giù | Elenca le parole già presenti nella colonna e ne permette la scelta |

**Ctrl+Invio** con più celle selezionate inserisce quello che hai digitato in tutte. Una formula viene scritta per la cella in cui l'hai digitata, e nelle altre celle si comporta come una sua copia: digitando **=A1** con l'intervallo E6:F7 selezionato, in F7 compare **=B2**. Si tratta di un solo passaggio di Annulla.

**Alt+Freccia giù** su una cella elenca, nell'ordine e una sola volta ciascuna, le parole contenute nelle celle sopra e sotto di essa che non sono separate da una cella vuota. Scegliere una parola la inserisce. Se la cella ha un elenco proprio, viene mostrato quell'elenco.

In **File > Opzioni > Modifica** puoi scegliere se **Invio** sposta il cursore in basso, a destra, in alto o a sinistra, oppure non lo sposta.

Se digiti in più celle di una riga con **Tab** e poi premi **Invio**, la selezione scende alla riga successiva, nella colonna da cui hai iniziato. Se prima selezioni un intervallo di celle, **Invio** e **Tab** si spostano all'interno dell'intervallo.

## Che cosa diventa quello che digiti

JeffCrunch interpreta quello che digiti in base alle impostazioni del computer. Il separatore decimale, il separatore delle migliaia, il simbolo della valuta e l'ordine di giorno e mese in una data derivano da quelle impostazioni. Un numero digitato in una cella che non ha un formato proprio riceve il formato che lo mostra come l'hai scritto.

| Digiti | La cella contiene | E mostra |
| --- | --- | --- |
| 1234,5 | il numero 1234,5 | 1234,5 |
| 5% | il numero 0,05 | 5% |
| 12,50 € | il numero 12,5 | 12,50 € |
| 1 1/2 | il numero 1,5 | 1 1/2 |
| 4/3/2025 | la data, come numero di giorni | 4/3/2025 |
| 22:30 | l'ora, come frazione di giorno | 22:30 |
| VERO | il valore logico vero | VERO |
| #N/D | l'errore #N/D | #N/D |
| =B2*2 | una formula | il risultato della formula |
| qualsiasi altra cosa | testo | il testo |

Una cella che ha già un formato proprio, come una percentuale o una data, lo mantiene. Una cella con formato **Testo** conserva tutto quello che digiti come testo, tranne le formule.

Per mantenere come testo qualcosa che sembra un numero, come il codice **007**, digita prima un apostrofo: **'007**. L'apostrofo non fa parte del testo. Quando modifichi la cella, lo vedi di nuovo.

## Formule

Una formula inizia con il segno di uguale. Ad esempio, **=B2*2** raddoppia il numero in B2. Puoi digitare i nomi delle funzioni in maiuscolo o in minuscolo: JeffCrunch li scrive in maiuscolo.

I nomi delle funzioni sono sempre quelli inglesi, in ogni lingua: digita **SUM**, non il nome che la tua lingua assegna alla funzione. I separatori tra gli elementi seguono le impostazioni del computer. Dove il separatore decimale è la virgola, come in italiano, gli argomenti di una funzione si separano con il punto e virgola e nei numeri si scrive la virgola: **=SUM(A1;A2)*0,5**. La barra della formula e **F2** mostrano le formule nello stesso modo. Dove il separatore decimale è il punto, gli argomenti si separano con la virgola: **=SUM(A1,A2)*0.5**. Il file conserva sempre la forma che tutti i programmi leggono, con le virgole tra gli argomenti e i punti nei numeri, così si apre allo stesso modo ovunque.

Se una formula contiene un errore, ad esempio una parentesi che non viene chiusa, JeffCrunch ti segnala il problema. L'immissione resta com'è stata digitata, con il cursore vicino all'errore, così puoi correggerlo. Non viene modificato nulla finché la formula non è corretta.

Una cella contiene fino a 32.767 caratteri, e una formula fino a 8.192. JeffCrunch ti avvisa quando un'immissione è più lunga.

## Cancellare il contenuto delle celle

Seleziona le celle e premi **Canc**. Vengono cancellati i contenuti delle celle e i loro formati restano invariati; si tratta di un passaggio che puoi annullare. JeffCrunch cancella fino a 100.000 celle alla volta.

Nella scheda **Home** il pulsante **Cancella** ha un menu. **Cancella tutto** svuota le celle, i loro formati, le loro note e i loro collegamenti. **Cancella formati** mantiene il contenuto delle celle e ripristina il formato normale. **Cancella contenuto** equivale a **Canc**. **Cancella commenti e note** elimina solo le note dalle celle selezionate. **Cancella collegamenti ipertestuali** elimina solo i collegamenti, e le celle mantengono il loro testo. Una voce è disattivata quando le celle selezionate non hanno nulla da cancellare.

## Annulla e Ripeti

**{key:Undo}** annulla l'ultima modifica, e **{key:Redo}** la ripete. Sono presenti anche nella barra degli strumenti rapida in alto. Ogni cella in cui digiti è un passaggio, e così ogni cancellazione. Annulla riporta la cella com'era: il valore, la formula, il formato e le formule che dipendono da essa. JeffCrunch ricorda fino a 1.000 passaggi, meno quando le modifiche riguardano moltissime celle, in modo da non occupare molta memoria.

## Mentre JeffCrunch calcola le formule

Quando modifichi una cella, JeffCrunch calcola in background le formule che dipendono da essa. La cella modificata mostra subito il nuovo contenuto, mentre le altre celle si aggiornano poco dopo; la barra di stato indica **Calcolo in corso** finché l'aggiornamento non è completato. Nel frattempo puoi continuare a digitare.

## Che cosa non si può modificare

- Una cella che fa parte di una **formula matriciale** o di una tabella dati non può essere modificata da sola.
- In un **foglio protetto**, una cella bloccata non può essere modificata. Tutte le celle sono bloccate, a meno che la cartella di lavoro non indichi il contrario. JeffCrunch ti avvisa quando tenti la modifica. Per modificare la cella, scegli Rimuovi protezione foglio nella scheda **Revisione**.
- La prima riga di una **tabella** contiene i nomi delle sue colonne. Digita un nuovo nome in una di queste celle per rinominare la colonna: le formule che usano il vecchio nome lo seguono, e la tabella nel file resta valida. Un nome già presente in un'altra colonna riceve un numero alla fine, un nome non può essere vuoto, e **Canc** non cancella quella riga.
- Una cartella di lavoro che non è stata letta per intero può essere consultata, ma non modificata né salvata.
- Una funzione che JeffCrunch non supporta ancora lascia una nuova formula senza valore nella cella; la cartella di lavoro chiede al programma che la apre successivamente di calcolare la formula.
- La formattazione, l'ordinamento e trova e sostituisci saranno disponibili in una versione successiva. Inserimento ed eliminazione, taglia e incolla, riempimento e le dimensioni di righe e colonne sono descritti in [Inserire ed eliminare righe, colonne e celle](insert-delete), [Taglia, copia e incolla](clipboard), [Riempire le celle e sommarle](fill-sum) e [Cambiare la larghezza delle colonne e l'altezza delle righe](sizes).
