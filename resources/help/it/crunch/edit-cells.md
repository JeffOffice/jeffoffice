# Modifica del contenuto delle celle
<!-- keywords: parola in grassetto, parte di una cella, testo formattato, formatta parte del testo, colora una parola, digitare, digitazione, inserire, immettere, modificare, cambiare, cella, voce, barra della formula, F2, doppio clic, Tab, Esc, annullare, annulla, ripristina, ripeti, eliminare, cancellare, percentuale, valuta, denaro, data, ora, numero, testo, apostrofo, formula, uguale, sostituire, limite, caratteri -->

Per inserire qualcosa in una cella, selezionala e inizia a digitare. Quello che digiti sostituisce il contenuto della cella. Premi **Invio** per confermare, oppure **Esc** per annullare l'immissione e lasciare la cella com'era.

## Tre modi per iniziare

- **Digitare sulla cella.** Seleziona una cella e digita. Il primo carattere avvia l'immissione, e la cella mostra ciò che digiti man mano.
- **Modificare il contenuto della cella.** Premi **F2**, oppure fai doppio clic sulla cella. La cella si apre con il suo contenuto, e il cursore si trova alla fine. Usa i tasti freccia per spostarti nel testo.
- **Usare la barra della formula.** Fai clic nella barra della formula, accanto alla casella Nome, e digita lì. Anche la cella mostra ciò che digiti.

Quando digiti sulla cella, un tasto freccia conclude l'immissione e sposta la selezione. Quando modifichi il contenuto della cella, i tasti freccia si muovono nel testo. Premi **F2** per passare dall'una all'altra modalità.

## Concludere l'immissione

| Tasti | Cosa fanno |
| --- | --- |
| Invio | Conferma l'immissione e scende alla cella successiva (oppure nella direzione indicata in **File > Opzioni > Modifica**) |
| Maiusc+Invio | Conferma l'immissione e si sposta nella direzione opposta |
| Ctrl+Invio | Inserisce l'immissione in tutte le celle selezionate, e resti dove sei |
| Tab | Conferma l'immissione e si sposta a destra |
| Maiusc+Tab | Conferma l'immissione e si sposta a sinistra |
| Tasti freccia | Conferma l'immissione e si sposta di una cella (solo quando hai digitato sulla cella) |
| Esc | Annulla l'immissione |
| Alt+Invio | Va a capo all'interno della cella |
| Ctrl+; | Inserisce la data odierna (nell'immissione, quando stai digitando una) |
| Ctrl+Maiusc+; | Inserisce l'ora attuale |
| Alt+Freccia giù | Elenca le parole già presenti nella colonna e ne scegli una |

**Ctrl+Invio** con più celle selezionate inserisce ciò che hai digitato in tutte. Una formula è scritta per la cella in cui l'hai digitata, e nelle altre si comporta come se fosse stata copiata: digitando **=A1** con l'intervallo E6:F7 selezionato, in F7 compare **=B2**. Corrisponde a un solo passaggio di Annulla.

**Alt+Freccia giù** su una cella elenca, nell'ordine e una sola volta ciascuna, le parole presenti nelle celle sopra e sotto, purché non siano separate da una cella vuota. Scegliendone una, la digiti. Se una cella ha un elenco proprio, viene mostrato quello.

In **File > Opzioni > Modifica** puoi scegliere se **Invio** sposta verso il basso, a destra, verso l'alto o a sinistra, oppure non sposta affatto.

Se digiti in più celle di una riga con **Tab**, poi premi **Invio**, la selezione scende alla riga successiva, nella colonna da cui hai iniziato. Se prima selezioni un intervallo di celle, **Invio** e **Tab** si spostano all'interno dell'intervallo.

## Formattare una parte del testo

Alcune parole di una cella possono apparire diverse dal resto. Ad esempio, una parola può essere in grassetto o rossa.

1. Premi F2 per modificare la cella (oppure fai doppio clic su di essa).
2. Seleziona le parole da modificare, con Maiusc e i tasti freccia oppure con il mouse.
3. Usa **Grassetto**, **Corsivo**, **Sottolineato**, **Barrato**, **Carattere**, **Dimensione carattere** o **Colore carattere** nella scheda **Home**, oppure Ctrl+B, Ctrl+I e Ctrl+U.
4. Premi Invio.

Cambiano solo le parole selezionate. Il resto della cella conserva l'aspetto proprio della cella. Mentre modifichi, la casella di immissione mostra le parole con un unico aspetto, e quando premi Invio la cella mostra ogni parte con il suo aspetto. Se in seguito digiti o elimini parole, la formattazione si sposta insieme alle parole. Scegliere un carattere o una dimensione senza aver selezionato parole cambia l'intera cella, come sempre.

Le celle che contengono formattazione all'interno del testo proveniente da un altro programma la mostrano, la conservano e la salvano di nuovo. Una cella che modifichi mantiene la sua formattazione. La formattazione all'interno di un testo si applica al testo, non a numeri o formule. Una copia salvata in un file .xls meno recente o in formato .ods conserva le parole, e il salvataggio segnala che la formattazione interna non viene mantenuta.

## Cosa diventa ciò che digiti

JeffCrunch interpreta ciò che digiti secondo le impostazioni del computer. Il separatore decimale, il separatore delle migliaia, il simbolo della valuta e l'ordine di giorno e mese in una data derivano da quelle impostazioni. Un numero digitato in una cella che non ha un formato proprio riceve il formato che lo mostra come l'hai digitato.

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

Una cella che ha già un formato proprio, come una percentuale o una data, lo conserva. Una cella il cui formato è **Testo** conserva tutto ciò che digiti come testo, tranne una formula.

Per conservare come testo qualcosa che sembra un numero, come un codice **007**, digita prima un apostrofo: **'007**. L'apostrofo non fa parte del testo. Quando modifichi la cella, lo vedi di nuovo.

## Formule

Una formula inizia con il segno di uguale. Ad esempio, **=B2*2** raddoppia il numero in B2. Digita i nomi delle funzioni in maiuscolo o minuscolo; JeffCrunch li scrive in maiuscolo.

I nomi delle funzioni sono sempre quelli inglesi, in tutte le lingue: digita **SUM**, non il nome che la tua lingua dà alla funzione. I segni tra gli elementi seguono le impostazioni del computer. Dove il separatore decimale è la virgola, come in italiano, separi gli argomenti di una funzione con il punto e virgola e scrivi la virgola nei numeri: **=SUM(A1;A2)*0,5**. La barra della formula e **F2** mostrano le formule nello stesso modo. Dove il separatore decimale è il punto, separi gli argomenti con la virgola: **=SUM(A1,A2)*0.5**. Il file conserva sempre la forma che tutti i programmi leggono, con virgole tra gli argomenti e punti nei numeri, così si apre allo stesso modo ovunque.

Se una formula contiene un errore, ad esempio una parentesi che non viene chiusa, JeffCrunch ti segnala il problema. L'immissione resta come l'hai digitata, con il cursore vicino all'errore, così puoi correggerla. Non viene cambiato nulla finché la formula non è corretta.

Una cella contiene fino a 32.767 caratteri, e una formula fino a 8.192. JeffCrunch ti avvisa quando un'immissione è più lunga.

## Cancellare le celle

Seleziona le celle e premi **Canc**. Questo cancella ciò che le celle contengono e conserva i loro formati, ed è un passaggio che puoi annullare. JeffCrunch cancella fino a 100.000 celle alla volta.

Nella scheda **Home**, il pulsante **Cancella** ha un menu. **Cancella tutto** svuota le celle, i loro formati, le loro note e i loro collegamenti. **Cancella formati** conserva ciò che le celle contengono e ripristina il formato normale. **Cancella contenuto** equivale a **Canc**. **Cancella commenti e note** rimuove solo le note dalle celle selezionate. **Cancella collegamenti ipertestuali** rimuove solo i collegamenti, e le celle conservano il loro testo. Una voce è disattivata quando le celle selezionate non hanno nulla da cancellare.

## Annulla e Ripeti

**{key:Undo}** annulla l'ultima modifica, e **{key:Redo}** la ripete. Sono presenti anche nella barra di accesso rapido in alto. Ogni cella in cui digiti è un passaggio, così come ogni cancellazione. Annulla riporta la cella com'era: il suo valore, la sua formula, il suo formato e le formule che ne dipendono. JeffCrunch ricorda fino a 1.000 passaggi, meno quando ne sono stati modificati moltissimi, in modo da non occupare mai molta memoria.

## Mentre JeffCrunch calcola le formule

Quando modifichi una cella, JeffCrunch calcola in background le formule che dipendono da essa. La cella modificata mostra subito il nuovo contenuto, e le altre celle si aggiornano poco dopo; la barra di stato indica **Calcolo in corso** finché non hanno finito. Nel frattempo puoi continuare a digitare.

## Ciò che non si può modificare

- Una cella che fa parte di una **formula matriciale** o di una tabella dati non può essere modificata da sola.
- In un **foglio protetto**, una cella bloccata non può essere modificata. Ogni cella è bloccata, a meno che la cartella di lavoro non indichi il contrario. JeffCrunch ti avvisa quando ci provi. Per modificare la cella, scegli Rimuovi protezione foglio nella scheda **Revisione**.
- La prima riga di una **tabella** contiene i nomi delle sue colonne. Digita un nuovo nome in una di quelle celle per rinominare la colonna: le formule che usano il vecchio nome si aggiornano, e la tabella nel file resta valida. Un nome già usato da un'altra colonna riceve un numero alla fine, un nome non può essere vuoto, e **Canc** non cancella quella riga.
- Una cartella di lavoro che non può essere letta per intero può essere consultata, ma non modificata né salvata.
- Una funzione che JeffCrunch non supporta ancora lascia in una nuova formula nessun valore nella cella; la cartella di lavoro chiede al programma che la apre di calcolare la formula.
- La formattazione, l'ordinamento e trova e sostituisci saranno disponibili in una versione successiva. Inserire ed eliminare, taglia e incolla, riempi e le dimensioni di righe e colonne sono descritti in [Inserire ed eliminare righe, colonne e celle](insert-delete), [Taglia, copia e incolla](clipboard), [Riempire le celle e sommarle](fill-sum) e [Modificare la larghezza delle colonne e l'altezza delle righe](sizes).

## Nuove cartelle di lavoro

In **File > Opzioni > Nuove cartelle di lavoro** puoi scegliere il carattere, la dimensione del carattere e il numero di fogli con cui inizia ogni nuova cartella di lavoro. Le cartelle di lavoro già esistenti non vengono modificate.
