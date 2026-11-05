# Digitare nelle celle
<!-- keywords: digitare, digitazione, invio, modifica, modificare, cambia, cella, voce, barra della formula, F2, doppio clic, Tab, Esc, annulla immissione, annulla, ripeti, elimina, cancella, percentuale, valuta, denaro, data, ora, numero, testo, apostrofo, formula, uguale, sostituisci, limite, caratteri -->

Per inserire un contenuto in una cella, seleziona la cella e inizia a digitare. Quello che digiti sostituisce il contenuto precedente della cella. Premi **Invio** per confermarlo oppure **Esc** per annullare l'immissione e lasciare la cella com'era.

## Tre modi per iniziare

- **Digita sopra la cella.** Seleziona una cella e digita. Il primo carattere avvia l'immissione, e la cella mostra ciò che digiti man mano.
- **Modifica il contenuto della cella.** Premi **F2** oppure fai doppio clic sulla cella. La cella si apre con il suo contenuto, e il cursore si trova alla fine. Usa i tasti freccia per spostarti nel testo.
- **Usa la barra della formula.** Fai clic nella barra della formula, accanto alla casella Nome, e digita lì. Anche in questo caso la cella mostra ciò che digiti.

Quando digiti sopra una cella, un tasto freccia completa l'immissione e sposta la selezione. Quando modifichi il contenuto della cella, i tasti freccia si muovono nel testo. Premi **F2** per passare dall'uno all'altro.

## Completare l'immissione

| Tasti | Cosa fanno |
| --- | --- |
| Invio | Conferma l'immissione e scende alla riga successiva (oppure nella direzione indicata in **File > Opzioni > Modifica**) |
| Maiusc+Invio | Conferma l'immissione e torna alla riga precedente |
| Ctrl+Invio | Inserisce ciò che hai digitato in tutte le celle selezionate, e resta dove sei |
| Tab | Conferma l'immissione e va a destra |
| Maiusc+Tab | Conferma l'immissione e va a sinistra |
| Tasti freccia | Confermano l'immissione e spostano di una cella (solo quando hai digitato sopra la cella) |
| Esc | Annulla l'immissione |
| Alt+Invio | Va a capo all'interno della cella |
| Ctrl+; | Inserisce la data di oggi (nell'immissione, quando stai digitando una data) |
| Ctrl+Maiusc+; | Inserisce l'ora attuale |
| Alt+Freccia giù | Elenca le parole già presenti nella colonna e ne sceglie una |

**Ctrl+Invio** con più celle selezionate inserisce ciò che hai digitato in tutte. Una formula viene scritta per la cella in cui l'hai digitata, e nelle altre si comporta come una sua copia: digitando **=A1** con l'intervallo E6:F7 selezionato, in F7 compare **=B2**. Occupa un solo passo di Annulla.

**Alt+Freccia giù** su una cella elenca, nell'ordine e una sola volta ciascuna, le parole presenti nelle celle sopra e sotto di essa, purché non siano separate da una cella vuota. Scegliendo una parola, questa viene digitata. Se la cella ha un elenco proprio, viene mostrato quello.

In **File > Opzioni > Modifica** puoi scegliere se **Invio** sposta verso il basso, a destra, verso l'alto o a sinistra, oppure non sposta affatto.

Se digiti in più celle di una riga con **Tab** e poi premi **Invio**, la selezione scende alla riga successiva, nella colonna da cui hai iniziato. Se prima selezioni un intervallo di celle, **Invio** e **Tab** si spostano all'interno dell'intervallo.

## Che cosa diventa ciò che digiti

JeffCrunch interpreta ciò che digiti secondo le impostazioni del computer. Il separatore decimale, il separatore delle migliaia, il simbolo della valuta e l'ordine di giorno e mese in una data derivano da tali impostazioni. Un numero digitato in una cella che non ha un formato proprio riceve il formato che lo mostra come l'hai digitato.

| Digiti | La cella contiene | E mostra |
| --- | --- | --- |
| 1234,5 | il numero 1234,5 | 1234,5 |
| 5% | il numero 0,05 | 5% |
| 12,50 € | il numero 12,5 | 12,50 € |
| 1 1/2 | il numero 1,5 | 1 1/2 |
| 3/4/2025 | la data, come numero di giorni | 3/4/2025 |
| 22:30 | l'ora, come frazione di giorno | 22:30 |
| VERO | il valore logico vero | VERO |
| #N/D | l'errore #N/D | #N/D |
| =B2*2 | una formula | il risultato della formula |
| qualsiasi altra cosa | testo | il testo |

Una cella che ha già un formato proprio, come percentuale o data, lo mantiene. Una cella con formato **Testo** conserva tutto ciò che digiti come testo, tranne una formula.

Per conservare come testo qualcosa che sembra un numero, come un codice **007**, digita prima un apostrofo: **'007**. L'apostrofo non fa parte del testo. Quando modifichi la cella, lo rivedi.

## Formule

Una formula inizia con il segno di uguale. Per esempio, **=B2*2** raddoppia il numero in B2. Puoi digitare i nomi delle funzioni in maiuscolo o minuscolo: JeffCrunch li scrive in maiuscolo.

I nomi delle funzioni sono sempre quelli inglesi, in ogni lingua: digita **SUM**, non il nome che la tua lingua gli dà. I segni di separazione seguono le impostazioni del computer. Dove il separatore decimale è la virgola, come in italiano, gli argomenti di una funzione si separano con il punto e virgola, e nei numeri si scrive la virgola: **=SUM(A1;A2)*0,5**. La barra della formula e **F2** mostrano le formule nello stesso modo. Dove il separatore decimale è il punto, gli argomenti si separano con la virgola: **=SUM(A1,A2)*0.5**. Il file conserva sempre la forma che ogni programma legge, con le virgole tra gli argomenti e i punti nei numeri, così si apre allo stesso modo ovunque.

Se una formula contiene un errore, per esempio una parentesi che non viene mai chiusa, JeffCrunch ti indica che cosa non va. L'immissione resta come l'hai digitata, con il cursore vicino all'errore, così puoi correggerla. Non viene cambiato nulla finché la formula non è corretta.

Una cella contiene fino a 32.767 caratteri e una formula fino a 8.192. JeffCrunch ti avvisa quando un'immissione è più lunga.

## Cancellare le celle

Seleziona le celle e premi **Canc**. Vengono cancellati i contenuti e i formati restano invariati; è un passo che puoi annullare. JeffCrunch cancella fino a 100.000 celle alla volta.

Nella scheda **Home**, il pulsante **Cancella** ha un menu. **Cancella tutto** svuota le celle, i loro formati, le loro note e i loro collegamenti. **Cancella formati** mantiene il contenuto delle celle e ripristina il formato normale. **Cancella contenuto** equivale a **Canc**. **Elimina commenti e note** rimuove solo le note dalle celle selezionate. **Rimuovi collegamenti ipertestuali** rimuove solo i collegamenti, e le celle conservano il loro testo. Una scelta è disattivata quando le celle selezionate non hanno nulla da cancellare.

## Annulla e Ripristina

**{key:Undo}** annulla l'ultima modifica e **{key:Redo}** la ripete. Sono anche nella barra di accesso rapido in alto. Ogni cella in cui digiti è un passo, e così ogni cancellazione. Annulla rimette la cella com'era: il suo valore, la sua formula, il suo formato e le formule che dipendono da essa. JeffCrunch ricorda fino a 1.000 passi, meno quando le modifiche riguardano moltissime celle, così da non occupare molta memoria.

## Mentre JeffCrunch calcola le formule

Quando modifichi una cella, JeffCrunch calcola in background le formule che dipendono da essa. La cella modificata mostra subito il nuovo contenuto, e le altre si aggiornano poco dopo; la barra di stato indica **Calcolo in corso** finché non hanno finito. Nel frattempo puoi continuare a digitare.

## Che cosa non si può modificare

- Una cella che fa parte di una **formula matriciale** o di una tabella dei dati non si può modificare da sola.
- In un **foglio protetto** non si può modificare una cella bloccata. Tutte le celle sono bloccate, a meno che la cartella di lavoro non indichi il contrario. JeffCrunch ti avvisa quando tenti la modifica. Per modificare la cella, scegli Rimuovi protezione foglio nella scheda **Revisione**.
- La prima riga di una **tabella** contiene i nomi delle sue colonne. Digita un nuovo nome in una di quelle celle per rinominare la colonna: le formule che usano il vecchio nome lo seguono, e la tabella nel file resta valida. Un nome già usato da un'altra colonna riceve un numero dopo, un nome non può essere vuoto, e **Canc** non cancella quella riga.
- Una cartella di lavoro che non è stata letta per intero può essere consultata, ma non modificata né salvata.
- Una funzione che JeffCrunch non supporta ancora lascia una nuova formula senza valore nella cella; la cartella di lavoro chiede al programma che la apre di calcolare la formula.
- La formattazione, l'ordinamento, e trova e sostituisci saranno disponibili in una versione successiva. Inserire ed eliminare, tagliare e incollare, riempire e le dimensioni di righe e colonne sono descritti in [Inserire ed eliminare righe, colonne e celle](insert-delete), [Tagliare, copiare e incollare](clipboard), [Riempire le celle e sommarle](fill-sum) e [Modificare la larghezza delle colonne e l'altezza delle righe](sizes).

## Nuove cartelle di lavoro

In **File > Opzioni > Nuove cartelle di lavoro** scegli il tipo di carattere, la dimensione del carattere e il numero di fogli con cui inizia ogni nuova cartella di lavoro. Le cartelle di lavoro già esistenti non vengono modificate.
