# Inserire il testo nelle celle
<!-- keywords: simbolo, carattere simbolo, caratteri speciali, tasto di scelta rapida per un simbolo, correzione automatica, virgolette intelligenti, trattino, frazione, collegamento, indirizzo, parola in grassetto, parte di una cella, testo formattato, formattare parte del testo, colorare una parola, esecuzione, digitare, digitazione, invio, modificare, modifica, cambiare, cella, voce, barra della formula, F2, doppio clic, Tab, Esc, annullare, annulla, ripeti, eliminare, cancellare, percentuale, valuta, denaro, data, ora, numero, testo, apostrofo, formula, uguale, sostituire, limite, caratteri -->

Per inserire un contenuto in una cella, selezionala e inizia a digitare. Quello che digiti sostituisce il contenuto della cella. Premi **Invio** per confermarlo oppure **Esc** per annullare l'immissione e lasciare la cella com'era.

## Tre modi per iniziare

- **Scrivere direttamente nella cella.** Seleziona una cella e digita. Il primo carattere che digiti avvia l'immissione, e la cella mostra ciò che scrivi man mano.
- **Modificare il contenuto della cella.** Premi **F2** oppure fai doppio clic sulla cella. La cella si apre con il suo contenuto, e il cursore si trova alla fine. Usa i tasti freccia per muoverti nel testo.
- **Usare la barra della formula.** Fai clic nella barra della formula, accanto alla casella Nome, e digita lì. Anche la cella mostra ciò che scrivi.

Quando scrivi direttamente nella cella, un tasto freccia conferma l'immissione e sposta la selezione. Quando modifichi il contenuto della cella, i tasti freccia si muovono nel testo. Premi **F2** per passare dall'una all'altra modalità.

## Concludere l'immissione

| Tasti | Cosa fanno |
| --- | --- |
| Invio | Conferma l'immissione e scende (oppure nella direzione indicata in **File > Opzioni > Modifica**) |
| Maiusc+Invio | Conferma l'immissione e si sposta in senso opposto |
| Ctrl+Invio | Inserisce l'immissione in tutte le celle selezionate, restando dove sei |
| Tab | Conferma l'immissione e si sposta a destra |
| Maiusc+Tab | Conferma l'immissione e si sposta a sinistra |
| Tasti freccia | Conferma l'immissione e sposta di una cella (solo quando hai scritto direttamente nella cella) |
| Esc | Annulla l'immissione |
| Alt+Invio | Va a capo all'interno della cella |
| Ctrl+; | Inserisce la data odierna (nell'immissione, quando la stai digitando) |
| Ctrl+Maiusc+; | Inserisce l'ora attuale |
| Alt+Freccia giù | Elenca le parole già presenti nella colonna e ne fa scegliere una |

**Ctrl+Invio** con più celle selezionate inserisce ciò che hai digitato in tutte. Una formula è scritta per la cella in cui la digiti, e nelle altre si comporta come una sua copia: digitando **=A1** con il blocco E6:F7 selezionato si ottiene **=B2** in F7. È un solo passaggio di Annulla.

**Alt+Freccia giù** su una cella elenca, in ordine e una volta sola, le parole contenute nelle celle sopra e sotto di essa che non sono separate da una cella vuota. Scegliendone una, la si digita. Una cella con un elenco proprio mostra invece quell'elenco.

In **File > Opzioni > Modifica** puoi scegliere se **Invio** scende, va a destra, sale, va a sinistra oppure non si sposta.

Se digiti in più celle di una riga con **Tab** e poi premi **Invio**, la selezione scende alla riga successiva, nella colonna da cui hai iniziato. Se prima selezioni un blocco di celle, **Invio** e **Tab** si muovono all'interno del blocco.

## Correzione automatica durante la digitazione

Quando digiti uno spazio o un segno di punteggiatura, come un punto, dopo una parola, JeffCrunch può correggere la parola appena completata. Corregge i più comuni errori di battitura (come "teh" al posto di "the"), due maiuscole all'inizio di una parola, una minuscola che inizia una frase o un giorno della settimana, e le virgolette dritte, che diventano curve. Trasforma inoltre due trattini tra parole in un trattino lungo, e 1/2, 1/4 e 3/4 in ½, ¼ e ¾ quando l'immissione contiene parole. Un numero come 1/2 digitato da solo resta una data o una frazione, come sempre.

Ogni correzione è un passaggio a sé. Subito dopo una correzione, premi **{key:Undo}** per annullare solo quella correzione e conservare ciò che hai digitato. Nelle formule non cambia nulla: un'immissione che inizia con il segno uguale, oppure con un segno più o meno seguito da una formula, resta come l'hai digitata. Un indirizzo web (come https://example.org) o un indirizzo di posta elettronica che costituisce l'intera immissione diventa un collegamento nella sua cella quando concludi l'immissione, se è attiva l'opzione **Trasforma gli indirizzi web in collegamenti ipertestuali durante la digitazione**.

Puoi attivare o disattivare queste opzioni in **File > Opzioni > Strumenti di correzione**. Una parola errata viene segnalata mentre digiti: vedi [Controllo ortografia](notes-links).

## Formattare una parte del testo

Alcune parole di una cella possono avere un aspetto diverso dal resto. Ad esempio, una parola può essere in grassetto o in rosso.

1. Premi F2 per modificare la cella (oppure fai doppio clic su di essa).
2. Seleziona le parole da modificare, con Maiusc e i tasti freccia oppure con il mouse.
3. Usa **Grassetto**, **Corsivo**, **Sottolineatura**, **Barrato**, **Carattere**, **Dimensione carattere** o **Colore carattere** nella scheda **Home**, oppure Ctrl+B, Ctrl+I e Ctrl+U.
4. Premi Invio.

Cambiano solo le parole selezionate. Il resto della cella mantiene l'aspetto proprio della cella. Mentre modifichi, la casella di immissione mostra le parole con un unico aspetto, e alla pressione di Invio la cella mostra ogni parte con il proprio. Se in seguito digiti o elimini parole, la formattazione si sposta insieme alle parole. Scegliere un carattere o una dimensione senza parole selezionate modifica l'intera cella, come sempre.

## La mini barra degli strumenti

Una piccola barra con la formattazione che usi più spesso compare vicino al punto in cui lavori, così non devi salire alla barra multifunzione. Non sottrae mai la tastiera alla cella: ciò che stavi digitando resta dov'era.

- **Testo selezionato in una cella:** seleziona parole con il mouse mentre modifichi una cella, e la barra compare sopra di esse. Contiene le caselle **carattere** e **dimensione**, **Grassetto**, **Corsivo**, **Sottolineatura**, **Aumenta dimensione carattere**, **Riduci dimensione carattere** e **Colore carattere**. Modificano solo le parole selezionate, come nella barra multifunzione.
- **Celle selezionate:** fai clic con il pulsante destro sulle celle, e la barra si trova in cima al menu di scelta rapida. Contiene inoltre **Colore riempimento**, **Bordi**, **Valuta**, **Stile percentuale**, **Stile separatore**, **Aumenta decimali**, **Riduci decimali**, **Unisci e allinea al centro** e **Copia formato**. Sono i comandi della barra multifunzione, e ciascuno è un singolo passaggio annullabile.

La barra sul testo selezionato si attenua mentre allontani il puntatore e scompare quando è lontano. Scompare anche quando digiti, premi Esc o scorri. Per disattivarla, deseleziona **Mostra la Mini barra degli strumenti alla selezione** nella pagina **Generale** di **File > Opzioni**; anche le altre app di JeffOffice seguono questa scelta. Il pulsante Analisi rapida che compare sotto un intervallo selezionato non è presente in questa versione.

Le celle che contengono una formattazione interna al testo, proveniente da un altro programma, la mostrano, la conservano e la salvano di nuovo. Una cella che modifichi mantiene la propria formattazione. La formattazione interna al testo si applica al testo, non ai numeri o alle formule. Una copia salvata come file .xls meno recente o come .ods conserva le parole, e il salvataggio ti avvisa che la formattazione interna non viene conservata.

## Ciò che digiti diventa

JeffCrunch interpreta ciò che digiti in base alle impostazioni del computer. Il separatore decimale, il separatore delle migliaia, il simbolo della valuta e l'ordine di giorno e mese in una data derivano da tali impostazioni. Un numero digitato in una cella che non ha un formato proprio riceve il formato che lo mostra come l'hai scritto.

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
| qualsiasi altro testo | testo | il testo |

Una cella che ha già un formato proprio, come una percentuale o una data, lo conserva. Una cella il cui formato è **Testo** conserva tutto ciò che digiti come testo, tranne le formule.

Per conservare come testo qualcosa che sembra un numero, come un codice **007**, digita prima un apostrofo: **'007**. L'apostrofo non fa parte del testo. Quando modifichi la cella, lo vedi di nuovo.

## Formule

Una formula inizia con il segno uguale. Ad esempio, **=B2*2** raddoppia il numero contenuto in B2. I nomi delle funzioni puoi digitarli in maiuscolo o in minuscolo: JeffCrunch li scrive in maiuscolo.

I nomi delle funzioni sono sempre quelli inglesi, in tutte le lingue: digita **SUM**, non il nome che la tua lingua gli attribuisce. I segni di punteggiatura tra gli elementi seguono le impostazioni del computer. Dove il separatore decimale è la virgola, come in tedesco e francese, gli argomenti di una funzione si separano con il punto e virgola, e nei numeri si scrive la virgola: **=SOMMA(A1;A2)*0,5**. La barra della formula e **F2** mostrano le formule nello stesso modo. Dove il separatore decimale è il punto, gli argomenti si separano con la virgola: **=SOMMA(A1,A2)*0.5**. Il file conserva sempre la forma che tutti i programmi leggono, con le virgole tra gli argomenti e i punti nei numeri, così si apre allo stesso modo ovunque.

Se una formula contiene un errore, come una parentesi che non viene chiusa, JeffCrunch ti dice che cosa non va. L'immissione resta come l'hai digitata, con il cursore vicino all'errore, così puoi correggerla. Non cambia nulla finché la formula non è corretta.

Una cella contiene fino a 32.767 caratteri, e una formula fino a 8.192. JeffCrunch ti avvisa quando un'immissione è più lunga.

## Cancellare celle

Seleziona le celle e premi **Canc**. Questo cancella il contenuto delle celle e mantiene i loro formati; è un passaggio che puoi annullare. JeffCrunch cancella fino a 100.000 celle alla volta.

Nella scheda **Home**, il pulsante **Cancella** ha un menu. **Cancella tutto** svuota le celle, i loro formati, le note, i commenti (le discussioni con risposte) e i collegamenti. Una discussione con più di 500 commenti viene conservata, e JeffCrunch lo segnala, perché Annulla non potrebbe ripristinare i commenti che non ha letto. **Cancella formati** mantiene il contenuto delle celle e ripristina il formato normale. **Cancella contenuto** equivale a **Canc**. **Cancella commenti e note** elimina solo le note e i commenti dalle celle selezionate. **Cancella collegamenti ipertestuali** elimina solo i collegamenti, e le celle conservano il loro testo. Una voce è disattivata quando le celle selezionate non hanno nulla da cancellare.

## Annulla e Ripeti

**{key:Undo}** annulla l'ultima modifica, e **{key:Redo}** la ripete. Sono anche nella barra degli strumenti rapida in alto. Ogni cella in cui digiti è un passaggio, e lo è ogni cancellazione. Annulla riporta la cella com'era: il valore, la formula, il formato e le formule che dipendono da essa. JeffCrunch ricorda fino a 1.000 passaggi, meno quando sono state modificate moltissime celle, così da non occupare troppa memoria.

**Tornare indietro di più passaggi insieme.** Nella barra degli strumenti rapida, **Annulla** e **Ripeti** hanno accanto una freccia. Fai clic sulla freccia per vedere i passaggi, dal più recente (ne vengono elencati fino a 100). Spostando il puntatore lungo l'elenco scegli quanti passaggi annullare (la riga sotto l'elenco dice, ad esempio, "Annulla 3 azioni"), e fai clic per eseguirli. Scegliendo il terzo passaggio si annullano i primi tre. I passaggi restano in ordine, quindi Ripeti li riporta indietro allo stesso modo.

## Ripetere

**F4** ripete l'ultimo comando sulle celle attualmente selezionate. Esempio: rendi **A1** in grassetto, seleziona **C4:C9** e premi **F4**: le celle di C4:C9 diventano in grassetto, in un unico passaggio annullabile. Ripeti funziona per la formattazione, gli stili delle celle, la cancellazione e l'inserimento o l'eliminazione di righe e colonne. La descrizione del pulsante, o il comando **Ripeti**, indica che cosa farebbe ("Ripeti Grassetto"). Anche **Ctrl+Y** ripete, ma solo quando non c'è un passaggio da ripristinare (quando invece c'è, **Ctrl+Y** è Ripeti). Mentre digiti una formula, **F4** continua a cambiare un riferimento tra relativo e fisso, come prima. Non vengono ripetuti i comandi che aprono una finestra, né la digitazione nelle celle.

## Mentre JeffCrunch calcola le formule

Quando modifichi una cella, JeffCrunch calcola in background le formule che ne dipendono. La cella modificata mostra subito il suo nuovo contenuto, e le altre celle si aggiornano poco dopo; la barra di stato indica **Calcolo in corso** finché non hanno finito. Nel frattempo puoi continuare a digitare.

## Ciò che non si può modificare

- Una cella che fa parte di una **formula matrice** o di una tabella dati non si può modificare da sola. Per creare una formula matrice, seleziona le celle che deve riempire, digita la formula e premi **Ctrl+Maiusc+Invio**. Una sola formula riempie così tutte le celle, e la barra della formula la mostra tra parentesi graffe, ad esempio **{=A1:A3*2}**. Per modificarla, seleziona una qualsiasi cella della matrice, premi **F2**, cambia la formula e premi di nuovo **Ctrl+Maiusc+Invio**. Per eliminarla, seleziona tutte le sue celle e premi **Canc**. Entrambe le operazioni agiscono sull'intera matrice, in un unico passaggio di Annulla. F2 su una qualsiasi cella di una matrice apre la formula della matrice, e un foglio protetto o un intervallo con password rifiuta la modifica o la chiede, come per qualsiasi altra cella. Copiare una matrice, o una sua parte, e riempire a partire da una sua cella, danno i valori che la matrice mostra, non una formula.
- In un **foglio protetto**, una cella bloccata non si può modificare. Ogni cella è bloccata, a meno che la cartella di lavoro non indichi il contrario. JeffCrunch ti avvisa quando ci provi. Per modificare la cella, scegli Rimuovi protezione foglio nella scheda Revisione.
- La prima riga di una **tabella** contiene i nomi delle sue colonne. Digita un nuovo nome in una di queste celle per rinominare la colonna: le formule che usano il vecchio nome lo seguono, e la tabella nel file resta valida. Un nome già usato da un'altra colonna riceve un numero alla fine, un nome non può essere vuoto, e **Canc** non cancella quella riga.
- Una cartella di lavoro che non è stata possibile leggere per intero si può consultare, ma non modificare né salvare.
- Una funzione che JeffCrunch non ha ancora lascia una nuova formula senza valore nella cella; la cartella di lavoro chiede al programma che la apre di calcolare la formula.
- La formattazione, l'ordinamento e trova e sostituisci saranno disponibili in una versione successiva. Inserimento ed eliminazione, taglia e incolla, riempimento e dimensioni di righe e colonne sono descritti in [Inserire ed eliminare righe, colonne e celle](insert-delete), [Taglia, copia e incolla](clipboard), [Riempire le celle e sommarle](fill-sum) e [Modificare la larghezza delle colonne e l'altezza delle righe](sizes).

## Inserire un simbolo

Fai clic su **Inserisci > Simboli > Simbolo** per aprire la finestra Simbolo. Scegli un carattere e fai clic su **Inserisci**, oppure fai doppio clic su di esso. Il simbolo viene inserito nel punto del cursore dell'immissione che stai digitando. Se non stai digitando, si apre l'immissione della cella con il simbolo aggiunto alla fine, e **Invio** lo inserisce nella cella. Esempio: con una cella che contiene la parola *Totale*, scegliendo il simbolo dell'euro si ottiene l'immissione *Totale€*. I simboli usati per ultimi compaiono per primi nell'elenco.

**Prendere il carattere da un tipo di carattere.** Scegli un carattere nella casella **Carattere**, poi scegli il simbolo. Il simbolo viene inserito in un tratto proprio, impostato con quel carattere, e il resto del testo della cella mantiene il carattere della cella. Esempio: in una cella che contiene *Totale*, un segno di spunta scelto da un carattere simbolo produce *Totale✓*, con solo il segno di spunta in quel carattere. Con **(testo normale)** selezionato, il carattere della cella disegna il simbolo, quindi un simbolo che il carattere non possiede appare come un quadratino. In una formula viene inserito solo il carattere.

**Caratteri speciali.** La scheda **Caratteri speciali** elenca lo spazio e il trattino unificatori, il trattino facoltativo, il trattino medio e lungo, lo spazio medio e quello largo, le virgolette, e l'interruzione facoltativa senza larghezza, ciascuno con il proprio nome. Fai doppio clic su uno di essi per inserirlo.

**Assegnare un tasto di scelta rapida a un simbolo.** Scegli un simbolo o un carattere speciale, fai clic su **Tasto di scelta rapida**, premi i tasti che desideri, ad esempio **Ctrl+Alt+Maiusc+J**, e fai clic su **OK**. Da quel momento i tasti inseriscono quel simbolo, nello stesso modo della finestra. Lascia vuoti i tasti e fai clic su **OK** per rimuoverli. JeffCrunch ti avvisa se i tasti sono già assegnati a un altro comando. Il simbolo diventa un comando, quindi compare anche in **File > Opzioni > Personalizza > Tastiera**, dove puoi cambiarne i tasti (vedi [Scorciatoie da tastiera](keyboard)).

## Nuove cartelle di lavoro

In **File > Opzioni > Nuove cartelle di lavoro** scegli il carattere, la dimensione del carattere e il numero di fogli con cui inizia ogni nuova cartella di lavoro. Le cartelle di lavoro già esistenti non vengono modificate.
