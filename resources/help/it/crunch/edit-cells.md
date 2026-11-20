# Inserire testo nelle celle
<!-- keywords: simbolo, carattere simbolo, caratteri speciali, tasto di scelta rapida per un simbolo, correzione automatica, virgolette intelligenti, trattino, frazione, collegamento, indirizzo, parola in grassetto, parte di una cella, testo formattato, formatta parte del testo, colora una parola, digitare, immissione, inserire, modificare, cambiare, cella, voce, barra della formula, F2, doppio clic, Tab, Esc, annulla immissione, annulla, ripeti, elimina, cancella, percentuale, valuta, denaro, data, ora, numero, testo, apostrofo, formula, uguale, sostituisci, limite, caratteri -->

Per inserire un valore in una cella, seleziona la cella e inizia a digitare. Quello che digiti sostituisce il contenuto della cella. Premi **Invio** per confermare, oppure **Esc** per annullare l'immissione e lasciare la cella com'era.

## Tre modi per iniziare

- **Digita direttamente nella cella.** Seleziona una cella e digita. Il primo carattere avvia l'immissione, e la cella mostra quello che digiti man mano.
- **Modifica il contenuto della cella.** Premi **F2** oppure fai doppio clic sulla cella. La cella si apre con il suo contenuto, e il cursore si trova alla fine. Usa i tasti freccia per muoverti nel testo.
- **Usa la barra della formula.** Fai clic nella barra della formula, accanto alla casella Nome, e digita lì. Anche in questo caso la cella mostra quello che digiti.

Quando digiti direttamente nella cella, un tasto freccia conclude l'immissione e sposta la selezione. Quando modifichi il contenuto della cella, i tasti freccia si muovono nel testo. Premi **F2** per passare dall'una all'altra modalità.

## Concludere l'immissione

| Tasti | Cosa fanno |
| --- | --- |
| Invio | Conferma l'immissione e scende alla cella successiva (oppure nella direzione indicata in **File > Opzioni > Modifica**) |
| Maiusc+Invio | Conferma l'immissione e va nella direzione opposta |
| Ctrl+Invio | Inserisce l'immissione in tutte le celle selezionate, e resti dove sei |
| Tab | Conferma l'immissione e va a destra |
| Maiusc+Tab | Conferma l'immissione e va a sinistra |
| Tasti freccia | Conferma l'immissione e si sposta di una cella (solo quando hai digitato direttamente nella cella) |
| Esc | Annulla l'immissione |
| Alt+Invio | Va a capo all'interno della cella |
| Ctrl+; | Inserisce la data di oggi (nell'immissione, quando stai digitando) |
| Ctrl+Maiusc+; | Inserisce l'ora attuale |
| Alt+Freccia giù | Elenca le parole già presenti nella colonna e ne permette la scelta |

**Ctrl+Invio** con più celle selezionate inserisce quanto digitato in tutte. Una formula è scritta per la cella in cui digiti, e nelle altre si comporta come una copia: digitando **=A1** con l'intervallo E6:F7 selezionato, in F7 compare **=B2**. Si tratta di un'unica operazione annullabile.

**Alt+Freccia giù** su una cella elenca, nell'ordine e una sola volta, le parole contenute nelle celle sopra e sotto di essa, purché non siano separate da una cella vuota. Scegliendone una, la si digita. Se una cella ha un elenco proprio, viene mostrato quello.

In **File > Opzioni > Modifica** puoi scegliere se **Invio** scende, va a destra, sale, va a sinistra, oppure non si sposta.

Se digiti in più celle di una riga con **Tab** e poi premi **Invio**, la selezione scende alla riga successiva, nella colonna da cui hai iniziato. Se prima selezioni un intervallo di celle, **Invio** e **Tab** si muovono all'interno dell'intervallo.

## Correzione automatica durante la digitazione

Quando digiti uno spazio o un segno di punteggiatura, come un punto, dopo una parola, JeffCrunch può correggere la parola appena completata. Corregge gli errori di digitazione più comuni (ad esempio "dlelo" per "delo"), due lettere maiuscole all'inizio di una parola, una lettera minuscola che inizia una frase o un giorno della settimana, e le virgolette diritte, che diventano curve. Inoltre trasforma due trattini tra parole in un trattino lungo, e 1/2, 1/4 e 3/4 in ½, ¼ e ¾ quando l'immissione contiene parole. Un numero come 1/2 digitato da solo resta una data o una frazione, come sempre.

Ogni correzione è un'operazione a sé. Premi **{key:Undo}** subito dopo una correzione per annullare solo quella correzione e mantenere quanto digitato. Le formule non vengono modificate: un'immissione che inizia con il segno di uguale, oppure con un più o un meno seguito da una formula, resta come l'hai digitata. Un indirizzo web (come https://example.org) o un indirizzo di posta elettronica che costituisce tutta l'immissione diventa un collegamento nella cella, quando concludi l'immissione, se è attiva l'opzione **Trasforma gli indirizzi web in collegamenti durante la digitazione**.

Queste opzioni si attivano o disattivano in **File > Opzioni > Strumenti di correzione**. Una parola errata viene segnalata mentre digiti: vedi [Ortografia](notes-links).

## Formattare parte del testo

Alcune parole di una cella possono avere un aspetto diverso dal resto. Ad esempio, una parola può essere in grassetto o in rosso.

1. Premi F2 per modificare la cella (oppure fai doppio clic su di essa).
2. Seleziona le parole da cambiare, con Maiusc e i tasti freccia oppure con il mouse.
3. Usa **Grassetto**, **Corsivo**, **Sottolineato**, **Barrato**, **Carattere**, **Dimensione carattere** o **Colore carattere** nella scheda **Home**, oppure Ctrl+G, Ctrl+C e Ctrl+S (per grassetto, corsivo e sottolineato).
4. Premi Invio.

Cambiano solo le parole selezionate. Il resto della cella conserva l'aspetto proprio della cella. La casella di immissione mostra le parole con un solo aspetto mentre modifichi, e la cella mostra ogni parte con il suo aspetto quando premi Invio. Se in seguito digiti o elimini parole, la formattazione si sposta insieme alle parole. Scegliere un carattere o una dimensione senza parole selezionate modifica l'intera cella, come sempre.

## La mini barra degli strumenti

Una piccola barra con la formattazione che usi più spesso compare vicino al punto in cui lavori, così non devi salire fino alla barra multifunzione. Non sottrae mai la tastiera alla cella: quello che stavi digitando resta dov'era.

- **Testo selezionato in una cella:** seleziona le parole con il mouse mentre modifichi una cella, e la barra compare sopra di esse. Contiene le caselle **carattere** e **dimensione**, **Grassetto**, **Corsivo**, **Sottolineato**, **Aumenta dimensione carattere**, **Riduci dimensione carattere** e **Colore carattere**. Modificano solo le parole selezionate, come nella barra multifunzione.
- **Celle selezionate:** fai clic con il pulsante destro sulle celle, e la barra si trova in cima al menu di scelta rapida. Contiene anche **Colore riempimento**, **Bordi**, **Valuta**, **Stile percentuale**, **Stile separatore**, **Aumenta decimali**, **Diminuisci decimali**, **Unisci e allinea al centro** e **Copia formato**. Sono i comandi della barra multifunzione e ciascuno è un'operazione singola che puoi annullare.

La barra sopra il testo selezionato sfuma quando allontani il puntatore, e scompare quando il puntatore è lontano. Scompare anche quando digiti, premi Esc o scorri. Per disattivarla, deseleziona **Mostra la Mini barra degli strumenti alla selezione** nella pagina **Generale** di **File > Opzioni**; anche le altre app di JeffOffice seguono questa scelta. Il pulsante Analisi rapida che compare sotto un intervallo selezionato non è presente in questa versione.

Le celle che contengono una formattazione interna al testo, proveniente da un altro programma, la mostrano, la conservano e la salvano di nuovo. Una cella modificata conserva la sua formattazione. La formattazione interna al testo si applica al testo, non ai numeri o alle formule. Una copia salvata come file .xls meno recente o come .ods conserva le parole, e il salvataggio segnala che la formattazione interna non viene mantenuta.

## Cosa diventa quello che digiti

JeffCrunch interpreta quello che digiti in base alle impostazioni del computer. Il separatore decimale, il separatore delle migliaia, il simbolo della valuta e l'ordine di giorno e mese in una data derivano da tali impostazioni. Un numero digitato in una cella che non ha un formato proprio riceve il formato che lo mostra come l'hai digitato.

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
| qualsiasi altro testo | testo | il testo |

Una cella che ha già un formato proprio, come una percentuale o una data, lo conserva. Una cella il cui formato è **Testo** conserva come testo tutto quello che digiti, tranne una formula.

Per mantenere come testo qualcosa che sembra un numero, come il codice **007**, digita prima un apostrofo: **'007**. L'apostrofo non fa parte del testo. Quando modifichi la cella, lo vedi di nuovo.

## Formule

Una formula inizia con il segno di uguale. Ad esempio, **=B2*2** raddoppia il numero in B2. Puoi digitare i nomi delle funzioni in maiuscolo o in minuscolo: JeffCrunch li scrive in maiuscolo.

I nomi delle funzioni sono sempre quelli inglesi, in tutte le lingue: digita **SUM**, non il nome che la tua lingua gli dà. I separatori tra gli elementi della formula seguono le impostazioni del computer. Dove il separatore decimale è la virgola, come in italiano, separi gli argomenti di una funzione con il punto e virgola e scrivi la virgola nei numeri: **=SUM(A1;A2)*0,5**. La barra della formula e **F2** mostrano le formule nello stesso modo. Dove il separatore decimale è il punto, separi gli argomenti con la virgola: **=SUM(A1,A2)*0.5**. Il file conserva sempre la forma che tutti i programmi leggono, con virgole tra gli argomenti e punti nei numeri, così si apre allo stesso modo ovunque.

Se una formula contiene un errore, ad esempio una parentesi che non viene chiusa, JeffCrunch ti dice che cosa non va, e l'immissione resta com'è stata digitata, con il cursore vicino all'errore, così puoi correggerla. Nulla viene modificato finché la formula non è corretta.

Una cella contiene fino a 32.767 caratteri, e una formula fino a 8.192. JeffCrunch ti avvisa quando un'immissione è più lunga.

## Cancellare le celle

Seleziona le celle e premi **Canc**. Questo cancella il contenuto delle celle e conserva i loro formati, ed è un'operazione che puoi annullare. JeffCrunch cancella fino a 100.000 celle alla volta.

Nella scheda **Home**, il pulsante **Cancella** ha un menu. **Cancella tutto** svuota le celle, i loro formati, le loro note, i loro commenti (le discussioni con risposte) e i loro collegamenti. Una discussione con più di 500 commenti viene conservata, e JeffCrunch lo segnala, perché un Annulla non potrebbe ripristinare i commenti che non ha letto. **Cancella formati** conserva il contenuto delle celle e ripristina il formato normale. **Cancella contenuto** equivale a **Canc**. **Cancella commenti e note** elimina soltanto le note e i commenti dalle celle selezionate. **Cancella collegamenti ipertestuali** elimina soltanto i collegamenti, e le celle conservano il loro testo. Una scelta è disattivata quando le celle selezionate non hanno nulla da cancellare.

## Annulla e Ripeti

**{key:Undo}** annulla l'ultima modifica, e **{key:Redo}** la ripete. Sono anche nella barra degli strumenti rapida in alto. Ogni immissione in una cella è un'operazione, e così ogni cancellazione. Annulla riporta la cella com'era: il suo valore, la sua formula, il suo formato e le formule che ne dipendono. JeffCrunch ricorda fino a 1.000 operazioni, meno quando hanno modificato moltissime celle, così da non occupare molta memoria.

## Mentre JeffCrunch calcola le formule

Quando modifichi una cella, JeffCrunch calcola in background le formule che ne dipendono. La cella che hai modificato mostra subito il suo nuovo contenuto, e le altre celle si aggiornano poco dopo; la barra di stato indica **Calcolo in corso** finché non hanno finito. Nel frattempo puoi continuare a digitare.

## Cosa non si può modificare

- Una cella che fa parte di una **formula matriciale** o di una tabella dati non si può modificare da sola. Per creare una formula matriciale, seleziona le celle che deve riempire, digita la formula e premi **Ctrl+Maiusc+Invio**. Una sola formula riempie così tutte le celle, e la barra della formula la mostra tra parentesi graffe, come **{=A1:A3*2}**. Per modificarla, seleziona una cella qualsiasi della matrice, premi **F2**, cambia la formula e premi di nuovo **Ctrl+Maiusc+Invio**. Per eliminarla, seleziona tutte le sue celle e premi **Canc**. Entrambe le operazioni riguardano l'intera matrice e si annullano con un unico Annulla. F2 su una qualsiasi cella di una matrice apre la formula della matrice, e un foglio protetto o un intervallo protetto da password rifiuta la modifica o la chiede, come per qualsiasi altra cella. Copiare una matrice, o una sua parte, e riempire a partire da una delle sue celle, restituisce i valori che la matrice mostra, non una formula.
- In un **foglio protetto**, una cella bloccata non si può modificare. Ogni cella è bloccata, a meno che la cartella di lavoro non indichi il contrario. JeffCrunch ti avvisa quando ci provi. Per modificare la cella, scegli Rimuovi protezione foglio nella scheda Revisione.
- La prima riga di una **tabella** contiene i nomi delle sue colonne. Digita un nuovo nome in una di quelle celle per rinominare la colonna: le formule che usano il vecchio nome si adeguano, e la tabella nel file resta valida. Un nome già usato da un'altra colonna riceve un numero alla fine, un nome non può essere vuoto, e **Canc** non cancella quella riga.
- Una cartella di lavoro che non è stata possibile leggere per intero può essere consultata, ma non modificata né salvata.
- Una funzione che JeffCrunch non supporta ancora lascia una nuova formula senza valore nella cella; la cartella di lavoro chiede al programma che la apre di calcolare la formula.
- La formattazione, l'ordinamento, e trova e sostituisci saranno disponibili in una versione successiva. Inserimento ed eliminazione, taglia e incolla, riempimento e dimensioni di righe e colonne sono descritti in [Inserire ed eliminare righe, colonne e celle](insert-delete), [Taglia, copia e incolla](clipboard), [Riempire le celle e sommarle](fill-sum) e [Cambiare la larghezza delle colonne e l'altezza delle righe](sizes).

## Inserire un simbolo

Fai clic su **Inserisci > Simboli > Simbolo** per aprire la finestra Simbolo. Scegli un carattere e fai clic su **Inserisci**, oppure fai doppio clic su di esso. Il simbolo viene inserito nel punto del cursore dell'immissione che stai digitando. Se non stai digitando, si apre l'immissione della cella con il simbolo aggiunto alla fine, e **Invio** lo inserisce nella cella. Esempio: con una cella che contiene la parola *Totale*, scegliendo il simbolo dell'euro si ottiene l'immissione *Totale€*. I simboli usati per ultimi compaiono per primi nell'elenco.

**Prendere il carattere da un tipo di carattere.** Scegli un carattere nella casella **Carattere**, poi scegli il simbolo. Il carattere viene inserito in una porzione propria, impostata con quel carattere, e il resto del testo della cella mantiene il carattere della cella. Esempio: in una cella che contiene *Totale*, un segno di spunta scelto da un tipo di carattere di simboli dà *Totale✓*, con solo il segno di spunta in quel carattere. Con **(testo normale)** selezionato, è il carattere della cella a disegnare il simbolo, quindi un simbolo che il carattere non possiede appare come un quadratino. In una formula viene inserito solo il carattere.

**Caratteri speciali.** La scheda **Caratteri speciali** elenca lo spazio unificatore e il trattino unificatore, il trattino di sillabazione facoltativo, il trattino medio e lungo, lo spazio medio e quadratino, le virgolette e l'interruzione di riga opzionale senza larghezza, ciascuno con il suo nome. Fai doppio clic su uno di essi per inserirlo.

**Assegnare a un simbolo un tasto di scelta rapida.** Scegli un simbolo o un carattere speciale, fai clic su **Tasto di scelta rapida**, premi i tasti che desideri, ad esempio **Ctrl+Alt+Maiusc+J**, e fai clic su **OK**. Da quel momento i tasti inseriscono quel simbolo, come fa la finestra. Lascia i tasti vuoti e fai clic su **OK** per rimuoverli. JeffCrunch ti avvisa se un altro comando usa già quei tasti. Il simbolo diventa un comando, quindi compare anche in **File > Opzioni > Personalizza > Tastiera**, dove puoi cambiarne i tasti (vedi [Scorciatoie da tastiera](keyboard)).

## Nuove cartelle di lavoro

In **File > Opzioni > Nuove cartelle di lavoro** scegli il carattere, la dimensione del carattere e il numero di fogli con cui inizia ogni nuova cartella di lavoro. Le cartelle di lavoro già esistenti non vengono modificate.
