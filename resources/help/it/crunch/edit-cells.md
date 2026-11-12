# Digitare nelle celle
<!-- keywords: simbolo, carattere simbolo, caratteri speciali, tasto di scelta rapida per un simbolo, correzione automatica, virgolette curve, trattino, frazione, collegamento, indirizzo, parola in grassetto, parte di una cella, testo formattato, formattare parte del testo, colorare una parola, esecuzione, digitare, inserire, modificare, cambiare, cella, voce, barra della formula, F2, doppio clic, Tab, Esc, annullare, annulla, ripeti, eliminare, cancellare, percentuale, valuta, denaro, data, ora, numero, testo, apostrofo, formula, uguale, sostituire, limite, caratteri -->

Per inserire un contenuto in una cella, selezionala e inizia a digitare. Quello che digiti sostituisce il contenuto della cella. Premi **Invio** per confermare, oppure **Esc** per annullare la voce e lasciare la cella com'era.

## Tre modi per iniziare

- **Digitare sopra la cella.** Seleziona una cella e digita. Il primo carattere avvia la voce, e la cella mostra quello che digiti man mano.
- **Modificare il contenuto della cella.** Premi **F2**, oppure fai doppio clic sulla cella. La cella si apre con il suo contenuto, e il cursore si trova alla fine. Usa i tasti freccia per muoverti nel testo.
- **Usare la barra della formula.** Fai clic nella barra della formula, accanto alla casella Nome, e digita lì. Anche la cella mostra quello che digiti.

Quando digiti sopra una cella, un tasto freccia completa la voce e sposta la selezione. Quando modifichi il contenuto della cella, i tasti freccia si muovono nel testo. Premi **F2** per passare da un comportamento all'altro.

## Completare la voce

| Tasti | Cosa fanno |
| --- | --- |
| Invio | Conferma la voce e sposta in basso (o dove indicato in **File > Opzioni > Modifica**) |
| Maiusc+Invio | Conferma la voce e sposta nella direzione opposta |
| Ctrl+Invio | Inserisce la voce in tutte le celle selezionate, senza muoversi |
| Tab | Conferma la voce e sposta a destra |
| Maiusc+Tab | Conferma la voce e sposta a sinistra |
| Tasti freccia | Conferma la voce e sposta di una cella (solo quando hai digitato sopra la cella) |
| Esc | Annulla la voce |
| Alt+Invio | Inizia una nuova riga all'interno della cella |
| Ctrl+; | Inserisce la data di oggi (nella voce, quando la stai digitando) |
| Ctrl+Maiusc+; | Inserisce l'ora attuale |
| Alt+Freccia giù | Elenca le parole già presenti nella colonna, e ne scegli una |

**Ctrl+Invio** con più celle selezionate inserisce quello che hai digitato in tutte. Una formula è scritta per la cella in cui l'hai digitata, e nelle altre si comporta come una copia: digitando **=A1** con il blocco E6:F7 selezionato si ottiene **=B2** in F7. È un unico passo di Annulla.

**Alt+Freccia giù** su una cella elenca, in ordine e una sola volta ciascuna, le parole presenti nelle celle sopra e sotto di essa che non sono separate da una cella vuota. Scegliendone una, la digiti. Una cella con un elenco proprio mostra invece quell'elenco.

In **File > Opzioni > Modifica** scegli se **Invio** sposta in basso, a destra, in alto o a sinistra, oppure non sposta affatto.

Se digiti in più celle di una riga con **Tab**, poi premi **Invio**, la selezione scende alla riga successiva, nella colonna da cui hai iniziato. Se prima selezioni un blocco di celle, **Invio** e **Tab** si spostano all'interno del blocco.

## Correzione automatica durante la digitazione

Quando digiti uno spazio o un segno come il punto dopo una parola, JeffCrunch può correggere la parola non appena la completi. Corregge i comuni errori di battitura (ad esempio "teh" al posto di "the"), due lettere maiuscole all'inizio di una parola, una lettera minuscola che inizia una frase o un giorno della settimana, e le virgolette dritte (diventano curve). Inoltre trasforma due trattini tra parole in un trattino lungo, e 1/2, 1/4 e 3/4 in ½, ¼ e ¾ quando la voce contiene parole. Un numero come 1/2 digitato da solo resta una data o una frazione, come sempre.

Ogni correzione è un passo a sé. Premi **{key:Undo}** subito dopo una correzione per annullare solo quella correzione e mantenere quello che hai digitato. Nelle formule non cambia nulla: una voce che inizia con il segno uguale, oppure con un più o un meno seguito da una formula, resta come l'hai digitata. Un indirizzo web (come https://example.org) o un indirizzo email che costituisce l'intera voce diventa un collegamento nella sua cella quando completi la voce, se **Trasforma gli indirizzi web in collegamenti ipertestuali durante la digitazione** è attivo.

Attiva o disattiva queste opzioni in **File > Opzioni > Strumenti di correzione**. Una parola errata viene evidenziata mentre digiti: vedi [Controllo ortografia](notes-links).

## Formattare una parte del testo

Alcune parole di una cella possono apparire diverse dal resto. Per esempio, una parola può essere in grassetto o in rosso.

1. Premi F2 per modificare la cella (oppure fai doppio clic su di essa).
2. Seleziona le parole da cambiare, con Maiusc e i tasti freccia oppure con il mouse.
3. Usa **Grassetto**, **Corsivo**, **Sottolineato**, **Barrato**, **Carattere**, **Dimensione carattere** o **Colore carattere** nella scheda **Home**, oppure Ctrl+G, Ctrl+C e Ctrl+S.
4. Premi Invio.

Cambiano solo le parole selezionate. Il resto della cella mantiene l'aspetto proprio della cella. La casella di modifica mostra le parole con un solo aspetto mentre modifichi, mentre la cella mostra ogni parte con il proprio aspetto quando premi Invio. Se in seguito digiti o elimini parole, la formattazione si sposta insieme alle parole. Scegliere un carattere o una dimensione senza parole selezionate cambia l'intera cella, come sempre.

Le celle che contengono una formattazione interna al testo proveniente da un altro programma la mostrano, la conservano e la salvano di nuovo. Una cella che modifichi mantiene la sua formattazione. La formattazione interna al testo si applica al testo, non ai numeri o alle formule. Una copia salvata in un file .xls meno recente o in .ods conserva le parole, e il salvataggio ti avvisa che la formattazione interna non viene mantenuta.

## Cosa diventa quello che digiti

JeffCrunch legge quello che digiti secondo le impostazioni del computer. Il separatore decimale, il separatore delle migliaia, il simbolo della valuta e l'ordine di giorno e mese in una data derivano da quelle impostazioni. Un numero digitato in una cella che non ha un formato proprio riceve il formato che lo mostra come l'hai digitato.

| Digiti | La cella contiene | E mostra |
| --- | --- | --- |
| 1234.5 | il numero 1234.5 | 1234.5 |
| 5% | il numero 0,05 | 5% |
| $12.50 | il numero 12,5 | $12.50 |
| 1 1/2 | il numero 1,5 | 1 1/2 |
| 3/4/2025 | la data, come numero di giorni | 3/4/2025 |
| 10:30 PM | l'ora, come frazione di giorno | 10:30 PM |
| TRUE | il valore logico vero | TRUE |
| #N/A | l'errore #N/D | #N/D |
| =B2*2 | una formula | il risultato della formula |
| qualsiasi altra cosa | testo | il testo |

Una cella che ha già un formato proprio, come percentuale o data, lo mantiene. Una cella il cui formato è **Testo** mantiene come testo tutto ciò che digiti, tranne una formula.

Per conservare come testo qualcosa che sembra un numero, come un codice **007**, digita prima un apostrofo: **'007**. L'apostrofo non fa parte del testo. Quando modifichi la cella, lo rivedi.

## Formule

Una formula inizia con il segno uguale. Per esempio, **=B2*2** raddoppia il numero in B2. Scrivi i nomi delle funzioni in maiuscolo o minuscolo: JeffCrunch le scrive in maiuscolo.

I nomi delle funzioni sono sempre quelli inglesi, in ogni lingua: digita **SUM**, non il nome che la tua lingua gli dà. I segni tra un elemento e l'altro seguono le impostazioni del computer. Dove il separatore decimale è la virgola, come in italiano, tedesco e francese, gli argomenti di una funzione si separano con il punto e virgola, e nei numeri si scrive la virgola: **=SOMMA(A1;A2)*0,5**. La barra della formula e **F2** mostrano le formule nello stesso modo. Dove il separatore decimale è il punto, gli argomenti si separano con la virgola: **=SUM(A1,A2)*0.5**. Il file conserva sempre la forma che ogni programma legge, con virgole tra gli argomenti e punti nei numeri, così si apre allo stesso modo ovunque.

Se una formula contiene un errore, come una parentesi mai chiusa, JeffCrunch ti dice che cosa non va, e la voce resta come l'hai digitata, con il cursore vicino all'errore, così puoi correggerla. Non cambia nulla finché la formula non è corretta.

Una cella contiene fino a 32.767 caratteri, e una formula fino a 8.192. JeffCrunch ti avvisa quando una voce è più lunga.

## Cancellare le celle

Seleziona le celle e premi **Canc**. Questo cancella il contenuto delle celle e mantiene i loro formati, ed è un unico passo che puoi annullare. JeffCrunch cancella fino a 100.000 celle alla volta.

Nella scheda **Home**, il pulsante **Cancella** ha un menu. **Cancella tutto** svuota le celle, i loro formati, le note, i commenti (le discussioni con le risposte) e i collegamenti. Una discussione con più di 500 commenti viene mantenuta, e JeffCrunch lo segnala, perché un Annulla non potrebbe recuperare i commenti che non ha letto. **Cancella formati** mantiene il contenuto delle celle e ripristina il formato normale. **Cancella contenuto** equivale a **Canc**. **Cancella commenti e note** rimuove solo le note e i commenti dalle celle selezionate. **Cancella collegamenti ipertestuali** rimuove solo i collegamenti, e le celle mantengono il loro testo. Una scelta è disattivata quando le celle selezionate non hanno nulla da cancellare.

## Annulla e Ripeti

**{key:Undo}** annulla l'ultima modifica, e **{key:Redo}** la ripete. Sono anche nella barra degli strumenti rapida in alto. Ogni cella che digiti è un passo, e così ogni cancellazione. Annulla rimette la cella com'era: il suo valore, la sua formula, il suo formato e le formule che ne dipendono. JeffCrunch ricorda fino a 1.000 passi, meno quando sono state modificate moltissime celle, in modo da non occupare mai molta memoria.

## Mentre JeffCrunch calcola le formule

Quando modifichi una cella, JeffCrunch calcola in background le formule che ne dipendono. La cella modificata mostra subito il suo nuovo contenuto, e le altre celle si aggiornano poco dopo; la barra di stato indica **Calcolo in corso** finché non hanno finito. Nel frattempo puoi continuare a digitare.

## Ciò che non si può modificare

- Una cella che fa parte di una **formula matriciale** o di una tabella dati non può essere modificata da sola.
- In un **foglio protetto**, una cella bloccata non può essere modificata. Ogni cella è bloccata, a meno che la cartella di lavoro non indichi il contrario. JeffCrunch ti avvisa quando ci provi. Per modificare la cella, scegli Rimuovi protezione foglio nella scheda Revisione.
- La prima riga di una **tabella** contiene i nomi delle sue colonne. Digita un nuovo nome in una di quelle celle per rinominare la colonna: le formule che usano il vecchio nome lo seguono, e la tabella nel file resta valida. Un nome già usato da un'altra colonna riceve un numero alla fine, un nome non può essere vuoto, e **Canc** non cancella quella riga.
- Una cartella di lavoro che non è stata letta per intero può essere consultata, ma non modificata né salvata.
- Una funzione che JeffCrunch non ha ancora lascia una nuova formula senza valore nella cella; la cartella di lavoro chiede al programma che la apre accanto di calcolare la formula.
- Formattazione, ordinamento e trova e sostituisci arriveranno in una versione successiva. Inserire ed eliminare, taglia e incolla, riempimento e dimensioni di righe e colonne sono descritti in [Inserire ed eliminare righe, colonne e celle](insert-delete), [Taglia, copia e incolla](clipboard), [Riempire le celle e sommarle](fill-sum) e [Modificare la larghezza delle colonne e l'altezza delle righe](sizes).

## Inserire un simbolo

Fai clic su **Inserisci > Simboli > Simbolo** per aprire la finestra Simbolo. Scegli un carattere e fai clic su **Inserisci**, oppure fai doppio clic su di esso. Viene inserito nel cursore della voce che stai digitando. Se non stai digitando, si apre la voce della cella con il simbolo aggiunto alla fine, e **Invio** lo inserisce nella cella. Esempio: con una cella che contiene la parola *Totale*, scegliendo il simbolo dell'euro si ottiene la voce *Totale€*. I simboli usati per ultimi sono elencati per primi.

**Prendere il carattere da un tipo di carattere.** Scegli un tipo di carattere nella casella **Carattere**, poi scegli il carattere. Il carattere viene inserito in un tratto proprio, impostato in quel tipo di carattere, e il resto del testo della cella mantiene il carattere della cella. Esempio: in una cella con *Totale*, un segno di spunta scelto da un tipo di carattere di simboli produce *Totale✓*, con solo il segno di spunta in quel carattere. Con **(testo normale)** selezionato, è il carattere della cella a disegnare il simbolo, quindi un simbolo che il carattere non possiede appare come un riquadro. In una formula viene inserito solo il carattere.

**Caratteri speciali.** La scheda **Caratteri speciali** elenca lo spazio unificatore e il trattino unificatore, il trattino facoltativo, il trattino medio e il trattino lungo, lo spazio medio e lo spazio largo, le virgolette, e l'interruzione facoltativa senza larghezza, ciascuno con il suo nome. Fai doppio clic su uno di essi per inserirlo.

**Assegnare un tasto di scelta rapida a un simbolo.** Scegli un simbolo o un carattere speciale, fai clic su **Tasto di scelta rapida**, premi i tasti che desideri, per esempio **Ctrl+Alt+Maiusc+J**, e fai clic su **OK**. I tasti inseriscono poi quel simbolo, allo stesso modo della finestra. Lascia i tasti vuoti e fai clic su **OK** per rimuoverli. JeffCrunch ti avvisa quando un altro comando usa già quei tasti. Il simbolo diventa un comando, quindi compare anche in **File > Opzioni > Personalizza > Tastiera**, dove puoi cambiare i suoi tasti (vedi [Scorciatoie da tastiera](keyboard)).

## Nuove cartelle di lavoro

In **File > Opzioni > Nuove cartelle di lavoro** scegli il carattere, la dimensione del carattere e il numero di fogli con cui inizia ogni nuova cartella di lavoro. Le cartelle di lavoro già esistenti non vengono modificate.
