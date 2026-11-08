# Inserire dati nelle celle

<!-- keywords: grassetto parola, parte di una cella, testo formattato, formatta parte del testo, colora una parola, esecuzioni, digitare, digitazione, invio, modificare, modifica, cambiare, cella, voce, barra della formula, F2, doppio clic, Tab, Esc, annullare, annulla, ripeti, ripristina, eliminare, cancellare, cancella, percentuale, valuta, denaro, data, ora, numero, testo, apostrofo, formula, uguale, sostituire, limite, caratteri -->

Per inserire un contenuto in una cella, selezionala e inizia a digitare. Quello che digiti sostituisce il contenuto della cella. Premi **Invio** per confermarlo oppure **Esc** per annullare la voce e lasciare la cella com'era.

## Tre modi per iniziare

- **Digitare direttamente nella cella.** Seleziona una cella e digita. Il primo carattere che digiti avvia la voce, e la cella mostra ciò che scrivi man mano.
- **Modificare il contenuto della cella.** Premi **F2** oppure fai doppio clic sulla cella. La cella si apre con il suo contenuto e il cursore si trova alla fine. Usa i tasti freccia per spostarti nel testo.
- **Usare la barra della formula.** Fai clic nella barra della formula, accanto alla casella Nome, e digita lì. Anche in questo caso la cella mostra ciò che scrivi.

Quando digiti direttamente nella cella, un tasto freccia conferma la voce e sposta la selezione. Quando modifichi il contenuto della cella, i tasti freccia si spostano nel testo. Premi **F2** per passare dall'una all'altra modalità.

## Confermare la voce

| Tasti | Cosa fanno |
| --- | --- |
| Invio | Conferma la voce e scende (oppure nella direzione indicata in **File > Opzioni > Modifica**) |
| Maiusc+Invio | Conferma la voce e sale |
| Ctrl+Invio | Inserisce la voce in tutte le celle selezionate e resta dove si trova |
| Tab | Conferma la voce e va a destra |
| Maiusc+Tab | Conferma la voce e va a sinistra |
| Tasti freccia | Conferma la voce e si sposta di una cella (solo quando hai digitato direttamente nella cella) |
| Esc | Annulla la voce |
| Alt+Invio | Va a capo all'interno della cella |
| Ctrl+; | Inserisce la data odierna (nella voce, quando la stai digitando) |
| Ctrl+Maiusc+; | Inserisce l'ora corrente |
| Alt+Freccia giù | Elenca le parole già presenti nella colonna e ne sceglie una |

**Ctrl+Invio** con più celle selezionate inserisce ciò che hai digitato in tutte. Una formula è scritta per la cella in cui digiti, e nelle altre si comporta come una copia: digitando **=A1** con il blocco E6:F7 selezionato si ottiene **=B2** in F7. È un solo passaggio di Annulla.

**Alt+Freccia giù** su una cella elenca, nell'ordine e una sola volta, le parole presenti nelle celle sopra e sotto di essa che non sono separate da una cella vuota. Scegliendone una, la digiti. Una cella con un elenco proprio mostra invece quell'elenco.

In **File > Opzioni > Modifica** scegli se **Invio** fa scendere, andare a destra, salire o a sinistra, oppure non spostare la selezione.

Se digiti in più celle di una riga con **Tab** e poi premi **Invio**, la selezione scende alla riga successiva, nella colonna da cui sei partito. Se prima selezioni un blocco di celle, **Invio** e **Tab** si spostano all'interno del blocco.

## Formattare una parte del testo

Alcune parole di una cella possono avere un aspetto diverso dal resto. Per esempio, una parola può essere in grassetto o in rosso.

1. Premi F2 per modificare la cella (oppure fai doppio clic su di essa).
2. Seleziona le parole da cambiare, con Maiusc e i tasti freccia oppure con il mouse.
3. Usa **Grassetto**, **Corsivo**, **Sottolineato**, **Barrato**, **Carattere**, **Dimensione carattere** o **Colore carattere** nella scheda **Home**, oppure Ctrl+B, Ctrl+I e Ctrl+U.
4. Premi Invio.

Cambiano solo le parole selezionate. Il resto della cella mantiene l'aspetto proprio della cella. Durante la modifica la casella di immissione mostra le parole con un solo aspetto, mentre la cella, quando premi Invio, mostra ogni parte con il proprio aspetto. Se in seguito digiti o cancelli parole, la formattazione si sposta insieme alle parole. Scegliere un carattere o una dimensione senza aver selezionato parole cambia l'intera cella, come sempre.

Le celle che contengono una formattazione interna al testo, proveniente da un altro programma, la mostrano, la conservano e la salvano di nuovo. Una cella che modifichi mantiene la propria formattazione. La formattazione interna al testo si applica al testo, non ai numeri o alle formule. Una copia salvata come file .xls meno recente o come .ods conserva le parole, e al salvataggio viene segnalato che la formattazione interna non viene conservata.

## Che cosa diventa ciò che digiti

JeffCrunch interpreta ciò che digiti secondo le impostazioni del computer. Il separatore decimale, il separatore delle migliaia, il simbolo della valuta e l'ordine di giorno e mese in una data derivano da tali impostazioni. Un numero digitato in una cella che non ha un formato proprio riceve il formato che lo mostra come l'hai digitato.

| Digiti | La cella contiene | E mostra |
| --- | --- | --- |
| 1234,5 | il numero 1234,5 | 1234,5 |
| 5% | il numero 0,05 | 5% |
| 12,50 € | il numero 12,5 | 12,50 € |
| 1 1/2 | il numero 1,5 | 1 1/2 |
| 3/4/2025 | la data, come numero di giorni | 03/04/2025 |
| 22:30 | l'ora, come frazione di giorno | 22:30 |
| VERO | il valore logico vero | VERO |
| #N/D | l'errore #N/D | #N/D |
| =B2*2 | una formula | il risultato della formula |
| qualsiasi altra cosa | testo | il testo |

Una cella che ha già un formato proprio, come una percentuale o una data, lo mantiene. Una cella il cui formato è **Testo** conserva tutto ciò che digiti come testo, tranne una formula.

Per conservare come testo qualcosa che sembra un numero, come un codice **007**, digita prima un apostrofo: **'007**. L'apostrofo non fa parte del testo. Quando modifichi la cella, lo vedi di nuovo.

## Formule

Una formula inizia con il segno di uguale. Per esempio, **=B2*2** raddoppia il numero in B2. Puoi scrivere i nomi delle funzioni in maiuscolo o minuscolo; JeffCrunch li scrive in maiuscolo.

I nomi delle funzioni sono sempre quelli inglesi, in ogni lingua: digita **SUM**, non il nome che la tua lingua gli attribuisce. I separatori tra gli elementi seguono le impostazioni del computer. Dove il separatore decimale è la virgola, come in italiano, separi gli argomenti di una funzione con il punto e virgola e scrivi la virgola in un numero: **=SOMMA(A1;A2)*0,5**. La barra della formula e **F2** mostrano le formule nello stesso modo. Dove il separatore decimale è il punto, gli argomenti si separano con la virgola: **=SUM(A1,A2)*0.5**. Il file conserva sempre la forma che tutti i programmi leggono, con le virgole tra gli argomenti e i punti nei numeri, così si apre allo stesso modo ovunque.

Se una formula contiene un errore, per esempio una parentesi che non viene chiusa, JeffCrunch te lo segnala e la voce resta come l'hai digitata, con il cursore vicino all'errore, così puoi correggerla. Non viene modificato nulla finché la formula non è corretta.

Una cella contiene fino a 32.767 caratteri, e una formula fino a 8.192. JeffCrunch ti avvisa quando una voce è più lunga.

## Cancellare le celle

Seleziona le celle e premi **Canc**. Questo cancella il contenuto delle celle e mantiene i loro formati, ed è un passaggio che puoi annullare. JeffCrunch cancella fino a 100.000 celle alla volta.

Nella scheda **Home**, il pulsante **Cancella** ha un menu. **Cancella tutto** svuota le celle, cioè il loro contenuto, i formati, le note, i commenti (le discussioni con risposte) e i collegamenti. Una discussione con più di 500 commenti viene conservata, e JeffCrunch lo segnala, perché Annulla non potrebbe ripristinare i commenti che non ha letto. **Cancella formati** mantiene il contenuto delle celle e ripristina il formato normale. **Cancella contenuto** equivale a **Canc**. **Cancella commenti e note** rimuove soltanto le note e i commenti dalle celle selezionate. **Cancella collegamenti** rimuove soltanto i collegamenti, e le celle conservano il proprio testo. Una voce è disattivata quando nelle celle selezionate non c'è nulla da cancellare.

## Annulla e Ripeti

**{key:Annulla}** annulla l'ultima modifica e **{key:Ripeti}** la ripete. Sono presenti anche nella barra di accesso rapido in alto. Ogni cella in cui digiti è un passaggio, e così ogni cancellazione. Annulla riporta la cella com'era: il suo valore, la sua formula, il suo formato e le formule che dipendono da essa. JeffCrunch ricorda fino a 1.000 passaggi, meno quando ne modificano moltissime, in modo da non occupare molta memoria.

## Mentre JeffCrunch calcola le formule

Quando modifichi una cella, JeffCrunch calcola in background le formule che ne dipendono. La cella che hai modificato mostra subito il suo nuovo contenuto, e le altre celle si aggiornano poco dopo; la barra di stato indica **Calcolo in corso** finché il calcolo non è terminato. Nel frattempo puoi continuare a digitare.

## Che cosa non si può modificare

- Una cella che fa parte di una **formula matriciale** o di una tabella dati non può essere modificata da sola.
- In un **foglio protetto**, una cella bloccata non può essere modificata. Ogni cella è bloccata, a meno che la cartella di lavoro non indichi il contrario. JeffCrunch ti avvisa quando tenti la modifica. Per modificare la cella, scegli Rimuovi protezione foglio nella scheda Revisione.
- La prima riga di una **tabella** contiene i nomi delle sue colonne. Digita un nuovo nome in una di quelle celle per rinominare la colonna: le formule che usano il vecchio nome lo seguono, e la tabella nel file resta valida. Un nome già usato da un'altra colonna riceve un numero alla fine, un nome non può essere vuoto, e **Canc** non cancella quella riga.
- Una cartella di lavoro che non è stato possibile leggere per intero può essere consultata, ma non modificata né salvata.
- Una funzione che JeffCrunch non supporta ancora lascia una nuova formula senza valore nella cella; la cartella di lavoro chiede al programma che la apre successivamente di calcolare la formula.
- La formattazione, l'ordinamento e trova e sostituisci saranno disponibili in una versione successiva. Inserimento ed eliminazione, taglia e incolla, riempimento e dimensioni di righe e colonne sono descritti in [Inserire ed eliminare righe, colonne e celle](insert-delete), [Taglia, copia e incolla](clipboard), [Riempire le celle e sommarle](fill-sum) e [Modificare la larghezza delle colonne e l'altezza delle righe](sizes).

## Nuove cartelle di lavoro

In **File > Opzioni > Nuove cartelle di lavoro** puoi scegliere il carattere, la dimensione del carattere e il numero di fogli con cui inizia ogni nuova cartella di lavoro. Le cartelle di lavoro già esistenti non vengono modificate.
