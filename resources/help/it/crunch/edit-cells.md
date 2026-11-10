# Inserire il testo nelle celle
<!-- keywords: correzione automatica, virgolette intelligenti, trattino, frazione, collegamento, indirizzo, parola in grassetto, parte di una cella, testo formattato, formattare parte del testo, colorare una parola, esecuzione, digitare, digitazione, invio, modificare, cambiare, cella, voce, barra della formula, F2, doppio clic, Tab, Esc, annullare, annulla, ripeti, eliminare, cancellare, percentuale, valuta, denaro, data, ora, numero, testo, apostrofo, formula, uguale, sostituire, limite, caratteri -->

Per inserire qualcosa in una cella, seleziona la cella e inizia a digitare. Quello che digiti sostituisce il contenuto della cella. Premi **Invio** per confermarlo, oppure **Esc** per annullare l'inserimento e lasciare la cella com'era.

## Tre modi per iniziare

- **Digitare sopra la cella.** Seleziona una cella e digita. Il primo carattere che digiti avvia l'inserimento, e la cella mostra ciò che digiti man mano.
- **Modificare il contenuto della cella.** Premi **F2**, oppure fai doppio clic sulla cella. La cella si apre con il suo contenuto, e il cursore si trova alla fine. Usa i tasti freccia per spostarti nel testo.
- **Usare la barra della formula.** Fai clic nella barra della formula, che si trova accanto alla casella Nome, e digita lì. Anche la cella mostra ciò che digiti.

Quando digiti sopra una cella, un tasto freccia conferma l'inserimento e sposta la selezione. Quando modifichi il contenuto della cella, i tasti freccia si spostano nel testo. Premi **F2** per passare da un comportamento all'altro.

## Confermare l'inserimento

| Tasti | Che cosa fanno |
| --- | --- |
| Invio | Conferma l'inserimento e scende alla cella successiva (oppure secondo quanto indicato in **File > Opzioni > Modifica**) |
| Maiusc+Invio | Conferma l'inserimento e sale |
| Ctrl+Invio | Inserisce ciò che hai digitato in tutte le celle selezionate, e resta dove sei |
| Tab | Conferma l'inserimento e va a destra |
| Maiusc+Tab | Conferma l'inserimento e va a sinistra |
| Tasti freccia | Conferma l'inserimento e sposta la selezione di una cella (solo quando hai digitato sopra la cella) |
| Esc | Annulla l'inserimento |
| Alt+Invio | Va a capo all'interno della cella |
| Ctrl+; | Inserisce la data di oggi (nella voce, quando la stai digitando) |
| Ctrl+Maiusc+; | Inserisce l'ora attuale |
| Alt+Freccia giù | Elenca le parole già presenti nella colonna e ne scegli una |

**Ctrl+Invio** con più celle selezionate inserisce ciò che hai digitato in tutte. Una formula è scritta per la cella in cui l'hai digitata, e nelle altre si comporta come una sua copia: digitando **=A1** con il blocco E6:F7 selezionato si ottiene **=B2** in F7. È un solo passaggio di Annulla.

**Alt+Freccia giù** su una cella elenca, in ordine e una sola volta, le parole presenti nelle celle sopra e sotto di essa che non sono separate da una cella vuota. Scegliendone una, la si digita. Una cella con un elenco proprio mostra invece quell'elenco.

In **File > Opzioni > Modifica** puoi scegliere se **Invio** sposta verso il basso, verso destra, verso l'alto o verso sinistra, oppure se non sposta affatto.

Se digiti in più celle di una riga con **Tab**, poi premi **Invio**, la selezione scende alla riga successiva, nella colonna da cui hai iniziato. Se prima selezioni un blocco di celle, **Invio** e **Tab** si spostano all'interno del blocco.

## Correzione automatica durante la digitazione

Quando digiti uno spazio o un segno di punteggiatura, come un punto, dopo una parola, JeffCrunch può correggere la parola non appena la completi. Corregge i più comuni errori di battitura (ad esempio "teh" al posto di "the"), due maiuscole all'inizio di una parola, una minuscola che inizia una frase o un giorno della settimana, e le virgolette dritte (che diventano curve). Sostituisce anche due trattini tra parole con un trattino lungo, e 1/2, 1/4 e 3/4 con ½, ¼ e ¾ quando la voce contiene parole. Un numero come 1/2 digitato da solo resta una data o una frazione, come sempre.

Ogni correzione è un passaggio a sé. Premi **{key:Undo}** subito dopo una correzione per annullare solo quella correzione e mantenere ciò che hai digitato. Nelle formule non cambia nulla: una voce che inizia con il segno uguale, oppure con un più o un meno seguito da una formula, resta come l'hai digitata. Un indirizzo web (ad esempio https://example.org) o un indirizzo di posta elettronica che costituisce tutta la voce diventa un collegamento nella sua cella quando confermi l'inserimento, se è attiva l'opzione **Trasforma gli indirizzi web in collegamenti ipertestuali durante la digitazione**.

Puoi attivare o disattivare queste correzioni in **File > Opzioni > Strumenti di correzione**. Una parola errata viene evidenziata mentre digiti: vedi [Controllo ortografia](notes-links).

## Formattare parte del testo

Alcune parole di una cella possono avere un aspetto diverso dal resto. Ad esempio, una parola può essere in grassetto o in rosso.

1. Premi F2 per modificare la cella (oppure fai doppio clic su di essa).
2. Seleziona le parole da modificare, con Maiusc e i tasti freccia oppure con il mouse.
3. Usa **Grassetto**, **Corsivo**, **Sottolineato**, **Barrato**, **Carattere**, **Dimensione carattere** o **Colore carattere** nella scheda **Home**, oppure Ctrl+G, Ctrl+C e Ctrl+S.

Hmm, correction needed: the shortcuts should be Ctrl+B, Ctrl+I, and Ctrl+U.

3. Usa **Grassetto**, **Corsivo**, **Sottolineato**, **Barrato**, **Carattere**, **Dimensione carattere** o **Colore carattere** nella scheda **Home**, oppure Ctrl+B, Ctrl+I e Ctrl+U.
4. Premi Invio.

Cambia solo le parole selezionate. Il resto della cella mantiene l'aspetto proprio della cella. Mentre modifichi, la casella di inserimento mostra le parole con un solo aspetto, e quando premi Invio la cella mostra ogni parte con il proprio aspetto. Se in seguito digiti o elimini parole, la formattazione segue le parole. Scegliere un carattere o una dimensione senza aver selezionato parole cambia l'intera cella, come sempre.

Le celle che contengono formattazione all'interno del testo proveniente da un altro programma la mostrano, la conservano e la salvano di nuovo. Una cella modificata mantiene la propria formattazione. La formattazione all'interno di un testo si applica al testo, non ai numeri o alle formule. Una copia salvata come file .xls meno recente o come .ods conserva le parole, e il salvataggio ti avvisa che la formattazione al loro interno non viene conservata.

## Che cosa diventa ciò che digiti

JeffCrunch interpreta ciò che digiti in base alle impostazioni del computer. Il separatore decimale, il separatore delle migliaia, il simbolo della valuta e l'ordine di giorno e mese in una data derivano da quelle impostazioni. Un numero digitato in una cella che non ha un formato proprio riceve il formato che lo mostra come l'hai digitato.

| Digiti | La cella contiene | E mostra |
| --- | --- | --- |
| 1234,5 | il numero 1234,5 | 1234,5 |
| 5% | il numero 0,05 | 5% |
| 12,50 € | il numero 12,5 | 12,50 € |
| 1 1/2 | il numero 1,5 | 1 1/2 |
| 3/4/2025 | la data, come numero di giorni | 3/4/2025 |
| 22:30 | l'ora, come parte di un giorno | 22:30 |
| VERO | il valore logico vero | VERO |
| #N/D | l'errore #N/D | #N/D |
| =B2*2 | una formula | il risultato della formula |
| qualsiasi altro testo | testo | il testo |

Una cella che ha già un formato proprio, come una percentuale o una data, lo mantiene. Una cella con formato **Testo** mantiene come testo tutto ciò che digiti, tranne una formula.

Per conservare come testo qualcosa che sembra un numero, come un codice **007**, digita prima un apostrofo: **'007**. L'apostrofo non fa parte del testo. Quando modifichi la cella, lo rivedi.

## Formule

Una formula inizia con il segno uguale. Ad esempio, **=B2*2** raddoppia il numero presente in B2. Digita i nomi delle funzioni in maiuscolo o minuscolo; JeffCrunch li scrive in maiuscolo.

I nomi delle funzioni sono sempre quelli inglesi, in ogni lingua: digita **SUM**, non il nome che la tua lingua assegna alla funzione. I segni tra gli argomenti seguono le impostazioni del computer. Dove il separatore decimale è la virgola, come in italiano, separi gli argomenti di una funzione con il punto e virgola e scrivi la virgola nei numeri: **=SOMMA(A1;A2)*0,5**. Anche la barra della formula e **F2** mostrano le formule nello stesso modo. Dove il separatore decimale è il punto, gli argomenti si separano con la virgola: **=SUM(A1,A2)*0.5**. Il file conserva sempre la forma che tutti i programmi leggono, con le virgole tra gli argomenti e i punti nei numeri, così si apre allo stesso modo ovunque.

Se una formula contiene un errore, ad esempio una parentesi che non viene mai chiusa, JeffCrunch ti dice che cosa non va, e la voce resta come l'hai digitata, con il cursore vicino all'errore, così puoi correggerla. Nulla cambia finché la formula non è corretta.

Una cella contiene fino a 32.767 caratteri, e una formula fino a 8.192. JeffCrunch ti avvisa quando una voce è più lunga.

## Cancellare le celle

Seleziona le celle e premi **Canc**. Vengono cancellati i contenuti delle celle e i loro formati restano invariati; è un passaggio che puoi annullare. JeffCrunch cancella fino a 100.000 celle alla volta.

Nella scheda **Home**, il pulsante **Cancella** ha un menu. **Cancella tutto** svuota le celle, i loro formati, le loro note, i loro commenti (le discussioni con risposte) e i loro collegamenti. Una discussione con più di 500 commenti viene conservata, e JeffCrunch lo segnala, perché Annulla non potrebbe ripristinare i commenti che non ha letto. **Cancella formati** mantiene il contenuto delle celle e ripristina il formato normale. **Cancella contenuto** equivale a **Canc**. **Cancella commenti e note** rimuove solo le note e i commenti dalle celle selezionate. **Cancella collegamenti ipertestuali** rimuove solo i collegamenti, e le celle mantengono il loro testo. Una voce è disattivata quando le celle selezionate non hanno nulla da cancellare.

## Annulla e Ripeti

**{key:Undo}** annulla l'ultima modifica, e **{key:Redo}** la ripete. I comandi si trovano anche nella barra degli strumenti rapida in alto. Ogni cella in cui digiti è un passaggio, e così ogni cancellazione. Annulla riporta la cella com'era: il suo valore, la sua formula, il suo formato e le formule che dipendono da essa. JeffCrunch ricorda fino a 1.000 passaggi, meno quando sono state modificate moltissime celle, così da non occupare mai molta memoria.

## Mentre JeffCrunch calcola le formule

Quando modifichi una cella, JeffCrunch calcola in background le formule che dipendono da essa. La cella che hai modificato mostra subito il nuovo contenuto, e le altre celle si aggiornano poco dopo; la barra di stato indica **Calcolo in corso…** finché non hanno finito. Nel frattempo puoi continuare a digitare.

## Che cosa non si può modificare

- Una cella che fa parte di una **formula matriciale** o di una tabella dati non può essere modificata da sola.
- In un **foglio protetto** non si può modificare una cella bloccata. Ogni cella è bloccata, salvo che la cartella di lavoro indichi il contrario. JeffCrunch ti avvisa quando ci provi. Per modificare la cella, scegli Rimuovi protezione foglio nella scheda Revisione.
- La prima riga di una **tabella** contiene i nomi delle sue colonne. Digita un nuovo nome in una di queste celle per rinominare la colonna: le formule che usano il vecchio nome si adeguano, e la tabella nel file resta valida. Un nome già usato da un'altra colonna riceve un numero alla fine, un nome non può essere vuoto, e **Canc** non cancella quella riga.
- Una cartella di lavoro che non è stato possibile leggere per intero può essere consultata, ma non modificata né salvata.
- Una funzione che JeffCrunch non ha ancora lascia nella cella una nuova formula senza valore; la cartella di lavoro chiede al programma che la apre successivamente di calcolare la formula.
- La formattazione, l'ordinamento e trova e sostituisci saranno disponibili in una versione successiva. Inserire ed eliminare, tagliare e incollare, riempire e le dimensioni di righe e colonne sono descritti in [Inserire ed eliminare righe, colonne e celle](insert-delete), [Tagliare, copiare e incollare](clipboard), [Riempire le celle e sommarle](fill-sum) e [Cambiare la larghezza delle colonne e l'altezza delle righe](sizes).

## Inserire un simbolo

Fai clic su **Inserisci > Simboli > Simbolo** per aprire la finestra Simbolo. Scegli un carattere e fai clic su **Inserisci**, oppure fai doppio clic su di esso. Il simbolo viene inserito nel punto del cursore della voce che stai digitando. Se non stai digitando, si apre il contenuto della cella con il simbolo aggiunto alla fine, e **Invio** lo inserisce nella cella. Esempio: con una cella che contiene la parola *Totale*, scegliendo il simbolo dell'euro la voce diventa *Totale€*. I simboli usati per ultimi compaiono per primi nell'elenco. Il carattere della cella disegna il simbolo, quindi un simbolo che il carattere non contiene appare come un quadratino.

## Nuove cartelle di lavoro

In **File > Opzioni > Nuove cartelle di lavoro** puoi scegliere il carattere, la dimensione del carattere e il numero di fogli con cui inizia ogni nuova cartella di lavoro. Le cartelle di lavoro già esistenti non vengono modificate.
