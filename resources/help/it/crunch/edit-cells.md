# Inserire dati nelle celle

<!-- keywords: simbolo, carattere simbolo, caratteri speciali, tasto di scelta rapida per un simbolo, correzione automatica, virgolette curve, trattino, frazione, collegamento, indirizzo, parola in grassetto, parte di una cella, testo formattato, formatta parte del testo, colora una parola, inserire, digitare, digitazione, invio, modifica, cambia, cella, voce, barra della formula, F2, doppio clic, Tab, Esc, annulla, ripeti, elimina, cancella, percentuale, valuta, denaro, data, ora, numero, testo, apostrofo, formula, uguale, sostituisci, limite, caratteri -->

Per inserire un contenuto in una cella, seleziona la cella e inizia a digitare. Quello che digiti sostituisce il contenuto della cella. Premi **Invio** per confermare, oppure **Esc** per annullare l'inserimento e lasciare la cella com'era.

## Tre modi per iniziare

- **Digita direttamente sulla cella.** Seleziona una cella e digita. Il primo carattere che digiti avvia l'inserimento, e la cella mostra quello che scrivi man mano.
- **Modifica il contenuto della cella.** Premi **F2** oppure fai doppio clic sulla cella. La cella si apre con il suo contenuto, e il cursore si trova alla fine. Usa i tasti freccia per spostarti nel testo.
- **Usa la barra della formula.** Fai clic nella barra della formula, accanto alla casella Nome, e digita lì. Anche in questo caso la cella mostra quello che scrivi.

Quando digiti direttamente sulla cella, un tasto freccia conferma l'inserimento e sposta la selezione. Quando modifichi il contenuto della cella, i tasti freccia si muovono nel testo. Premi **F2** per passare dall'uno all'altro.

## Confermare l'inserimento

| Tasti | Che cosa fanno |
| --- | --- |
| Invio | Conferma l'inserimento e scende alla cella successiva (oppure nella direzione indicata in **File > Opzioni > Modifica**) |
| Maiusc+Invio | Conferma l'inserimento e torna alla cella precedente |
| Ctrl+Invio | Inserisce il contenuto in tutte le celle selezionate, senza spostarsi |
| Tab | Conferma l'inserimento e va a destra |
| Maiusc+Tab | Conferma l'inserimento e va a sinistra |
| Tasti freccia | Conferma l'inserimento e si sposta di una cella (solo quando hai digitato direttamente sulla cella) |
| Esc | Annulla l'inserimento |
| Alt+Invio | Va a capo all'interno della cella |
| Ctrl+; | Inserisce la data di oggi (nell'inserimento, quando stai digitando) |
| Ctrl+Maiusc+; | Inserisce l'ora attuale |
| Alt+Freccia giù | Elenca le parole già presenti nella colonna, e ne scegli una |

**Ctrl+Invio** con più celle selezionate inserisce quello che hai digitato in tutte. Una formula è scritta per la cella in cui digiti, e nelle altre celle si comporta come se fosse stata copiata: digitando **=A1** con l'intervallo E6:F7 selezionato, in F7 compare **=B2**. Si tratta di un solo passaggio di Annulla.

**Alt+Freccia giù** su una cella elenca, in ordine e una volta sola, le parole contenute nelle celle sopra e sotto, purché non siano separate da una cella vuota. Scegliendone una, la digiti. Se una cella ha un elenco proprio, viene mostrato quello.

In **File > Opzioni > Modifica** puoi scegliere se **Invio** sposta verso il basso, a destra, in alto o a sinistra, oppure se non si sposta affatto.

Se digiti in più celle di una riga con **Tab** e poi premi **Invio**, la selezione scende alla riga successiva, nella colonna da cui hai iniziato. Se prima selezioni un intervallo di celle, **Invio** e **Tab** si spostano all'interno dell'intervallo.

## Correzione automatica durante la digitazione

Quando digiti uno spazio o un segno di punteggiatura, come un punto, dopo una parola, JeffCrunch può correggerla non appena la completi. Corregge gli errori di battitura più comuni (come "teh" al posto di "the"), due lettere maiuscole all'inizio di una parola, una lettera minuscola che inizia una frase o un giorno della settimana, e le virgolette dritte (che diventano curve). Trasforma anche due trattini tra parole in un trattino lungo, e 1/2, 1/4 e 3/4 in ½, ¼ e ¾ quando l'inserimento contiene parole. Un numero come 1/2 digitato da solo resta una data o una frazione, come sempre.

Ogni correzione è un passaggio a sé. Premi **{key:Undo}** subito dopo una correzione per annullare solo quella correzione e mantenere ciò che hai digitato. Nelle formule non viene cambiato nulla: un inserimento che inizia con il segno uguale, oppure con un più o un meno seguito da una formula, resta come lo hai digitato. Un indirizzo web (come https://example.org) o un indirizzo di posta elettronica che costituisce tutto l'inserimento diventa un collegamento nella sua cella quando confermi, se è attiva l'opzione **Trasforma gli indirizzi web in collegamenti ipertestuali durante la digitazione**.

Queste opzioni si attivano o disattivano in **File > Opzioni > Strumenti di correzione**. Una parola errata viene segnalata mentre digiti: vedi [Controllo ortografia](notes-links).

## Formattare una parte del testo

Alcune parole di una cella possono avere un aspetto diverso dal resto. Ad esempio, una parola può essere in grassetto o in rosso.

1. Premi F2 per modificare la cella (oppure fai doppio clic).
2. Seleziona le parole da cambiare, con Maiusc e i tasti freccia oppure con il mouse.
3. Usa **Grassetto**, **Corsivo**, **Sottolineatura**, **Barrato**, **Carattere**, **Dimensione carattere** o **Colore carattere** nella scheda **Home**, oppure Ctrl+G, Ctrl+C e Ctrl+S.
4. Premi Invio.

Cambiano solo le parole selezionate. Il resto della cella mantiene l'aspetto proprio della cella. Mentre modifichi, la casella di inserimento mostra le parole con un unico aspetto, e quando premi Invio la cella mostra ogni parte con il proprio aspetto. Se in seguito digiti o cancelli parole, la formattazione si sposta insieme alle parole. Scegliere un carattere o una dimensione senza parole selezionate cambia l'intera cella, come sempre.

## La mini barra degli strumenti

Una piccola barra con la formattazione che usi più spesso compare vicino al punto in cui lavori, così non devi salire fino alla barra multifunzione. Non sottrae mai la tastiera alla cella: quello che stavi scrivendo resta dov'era.

- **Testo selezionato in una cella:** seleziona parole con il mouse mentre modifichi una cella, e la barra compare sopra di esse. Contiene le caselle **carattere** e **dimensione**, **Grassetto**, **Corsivo**, **Sottolineatura**, **Aumenta dimensione carattere**, **Riduci dimensione carattere** e **Colore carattere**. Modificano solo le parole selezionate, come sulla barra multifunzione.
- **Celle selezionate:** fai clic con il tasto destro sulle celle, e la barra si trova in cima al menu di scelta rapida. Contiene anche **Colore riempimento**, **Bordi**, **Valuta**, **Stile percentuale**, **Stile separatore**, **Aumenta decimali**, **Diminuisci decimali**, **Unisci e allinea al centro** e **Copia formato**. Sono i comandi della barra multifunzione, e ciascuno è un singolo passaggio che puoi annullare.

La barra sul testo selezionato si attenua quando allontani il puntatore, e scompare quando il puntatore è molto lontano. Scompare anche quando digiti, premi Esc o scorri. Per disattivarla, deseleziona **Mostra la Mini barra degli strumenti alla selezione** nella pagina **Generale** di **File > Opzioni**; anche le altre app di JeffOffice seguono questa scelta. Il pulsante Analisi rapida che compare sotto un intervallo selezionato non è presente in questa versione.

Le celle che contengono formattazione all'interno del testo, proveniente da un altro programma, la mostrano, la conservano e la salvano di nuovo. Una cella che modifichi mantiene la sua formattazione. La formattazione all'interno di un testo si applica al testo, non ai numeri o alle formule. Una copia salvata come file .xls meno recente o come .ods conserva le parole, e il salvataggio segnala che la formattazione interna non viene mantenuta.

## Che cosa diventa quello che digiti

JeffCrunch interpreta quello che digiti secondo le impostazioni del computer. Il separatore decimale, il separatore delle migliaia, il simbolo della valuta e l'ordine di giorno e mese in una data derivano da tali impostazioni. Un numero digitato in una cella che non ha un formato proprio riceve il formato che lo mostra come lo hai digitato.

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

Una cella che ha già un formato proprio, come una percentuale o una data, lo conserva. Una cella il cui formato è **Testo** conserva tutto quello che digiti come testo, tranne una formula.

Per conservare come testo qualcosa che sembra un numero, come il codice **007**, digita prima un apostrofo: **'007**. L'apostrofo non fa parte del testo. Quando modifichi la cella, lo vedi di nuovo.

## Formule

Una formula inizia con il segno uguale. Ad esempio, **=B2*2** raddoppia il numero in B2. Puoi scrivere i nomi delle funzioni in maiuscolo o minuscolo: JeffCrunch li scrive in maiuscolo.

I nomi delle funzioni sono sempre quelli inglesi, in tutte le lingue: digita **SUM**, non il nome che la tua lingua dà alla funzione. I segni di separazione seguono le impostazioni del computer. Dove il separatore decimale è la virgola, come in italiano, separi gli argomenti di una funzione con il punto e virgola e scrivi la virgola nei numeri: **=SUM(A1;A2)*0,5**. La barra della formula e **F2** mostrano le formule nello stesso modo. Dove il separatore decimale è il punto, gli argomenti si separano con la virgola: **=SUM(A1,A2)*0.5**. Il file mantiene sempre la forma che tutti i programmi leggono, con le virgole tra gli argomenti e i punti nei numeri, così si apre allo stesso modo ovunque.

Se una formula contiene un errore, come una parentesi che non viene mai chiusa, JeffCrunch ti dice che cosa non va, e l'inserimento resta com'è stato digitato, con il cursore vicino all'errore, così puoi correggerlo. Non cambia nulla finché la formula non è corretta.

Una cella contiene fino a 32.767 caratteri, e una formula fino a 8.192. JeffCrunch ti avvisa quando un inserimento è più lungo.

## Cancellare il contenuto delle celle

Seleziona le celle e premi **Canc**. Questo cancella il contenuto delle celle e mantiene i loro formati, ed è un passaggio che puoi annullare. JeffCrunch cancella fino a 100.000 celle alla volta.

Nella scheda **Home**, il pulsante **Cancella** ha un menu. **Cancella tutto** svuota le celle, cioè elimina i loro formati, le note, i commenti (le discussioni con risposte) e i collegamenti. Una discussione con più di 500 commenti viene mantenuta, e JeffCrunch lo segnala, perché Annulla non potrebbe ripristinare i commenti che non ha letto. **Cancella formati** mantiene il contenuto delle celle e ripristina il formato normale. **Cancella contenuto** equivale a **Canc**. **Cancella commenti e note** rimuove solo le note e i commenti dalle celle selezionate. **Cancella collegamenti ipertestuali** rimuove solo i collegamenti, e le celle conservano il loro testo. Una scelta è disattivata quando le celle selezionate non hanno nulla da cancellare.

## Annulla e Ripeti

**{key:Undo}** annulla l'ultima modifica, e **{key:Redo}** la ripete. Sono anche nella barra degli strumenti rapida in alto. Ogni cella in cui digiti è un passaggio, e lo è anche ogni cancellazione. Annulla riporta la cella com'era: il suo valore, la sua formula, il suo formato e le formule che ne dipendono. JeffCrunch ricorda fino a 1.000 passaggi, meno quando ne sono stati modificati moltissimi, in modo da non occupare molta memoria.

## Mentre JeffCrunch calcola le formule

Quando cambi una cella, JeffCrunch calcola in background le formule che dipendono da essa. La cella che hai modificato mostra subito il suo nuovo contenuto, e le altre celle si aggiornano poco dopo; la barra di stato indica **Calcolo in corso** finché non hanno finito. Nel frattempo puoi continuare a digitare.

## Che cosa non si può modificare

- Una cella che fa parte di una **formula matriciale** o di una tabella dati non si può modificare da sola.
- In un **foglio protetto**, una cella bloccata non si può modificare. Ogni cella è bloccata, a meno che la cartella di lavoro non indichi il contrario. JeffCrunch ti avvisa quando ci provi. Per modificare la cella, scegli Rimuovi protezione foglio nella scheda Revisione.
- La prima riga di una **tabella** contiene i nomi delle sue colonne. Digita un nuovo nome in una di quelle celle per rinominare la colonna: le formule che usano il vecchio nome lo seguono, e la tabella nel file resta valida. Un nome già usato da un'altra colonna riceve un numero alla fine, un nome non può essere vuoto, e **Canc** non cancella quella riga.
- Una cartella di lavoro che non è stata letta per intero può essere consultata, ma non modificata né salvata.
- Una funzione che JeffCrunch non supporta ancora lascia nella cella una nuova formula senza valore; la cartella di lavoro chiede al programma che la apre di calcolare la formula.
- Formattazione, ordinamento e trova e sostituisci saranno disponibili in una versione successiva. Inserimento ed eliminazione, taglia e incolla, riempimento e dimensioni di righe e colonne sono trattati in [Inserire ed eliminare righe, colonne e celle](insert-delete), [Taglia, copia e incolla](clipboard), [Riempire le celle e sommarle](fill-sum) e [Modificare la larghezza delle colonne e l'altezza delle righe](sizes).

## Inserire un simbolo

Fai clic su **Inserisci > Simboli > Simbolo** per aprire la finestra Simbolo. Scegli un carattere e fai clic su **Inserisci**, oppure fai doppio clic su di esso. Il carattere viene inserito nel punto del cursore dell'inserimento che stai digitando. Se non stai digitando, si apre l'inserimento della cella con il simbolo aggiunto alla fine, e **Invio** lo mette nella cella. Esempio: con una cella che contiene la parola *Totale*, scegliendo il simbolo dell'euro l'inserimento diventa *Totale€*. I simboli usati per ultimi sono elencati per primi.

**Prendere il carattere da un tipo di carattere.** Scegli un carattere nella casella **Carattere**, poi scegli il simbolo. Il carattere va in un tratto a sé, composto con quel carattere, e il resto del testo della cella mantiene il carattere della cella. Esempio: in una cella che contiene *Totale*, un segno di spunta scelto da un carattere simbolo diventa *Totale✓*, con solo il segno di spunta in quel carattere. Con **(testo normale)** selezionato, il carattere della cella disegna il simbolo, quindi un simbolo che il carattere non possiede appare come un quadratino. In una formula viene inserito solo il carattere.

**Caratteri speciali.** La scheda **Caratteri speciali** elenca lo spazio unificatore e il trattino unificatore, il trattino facoltativo, il trattino medio e lungo, lo spazio medio e quadratino, le virgolette e l'interruzione di riga facoltativa senza larghezza, ciascuno con il suo nome. Fai doppio clic su uno di essi per inserirlo.

**Assegnare un tasto di scelta rapida a un simbolo.** Scegli un simbolo o un carattere speciale, fai clic su **Tasto di scelta rapida**, premi i tasti che vuoi, come **Ctrl+Alt+Maiusc+J**, e fai clic su **OK**. I tasti inseriranno poi quel simbolo, allo stesso modo della finestra. Lascia vuoto il campo e fai clic su **OK** per rimuovere i tasti. JeffCrunch ti avvisa quando un altro comando usa già quei tasti. Il simbolo diventa un comando, quindi compare anche in **File > Opzioni > Personalizza > Tastiera**, dove puoi cambiarne i tasti (vedi [Scorciatoie da tastiera](keyboard)).

## Nuove cartelle di lavoro

In **File > Opzioni > Nuove cartelle di lavoro** scegli il carattere, la dimensione del carattere e il numero di fogli con cui inizia ogni nuova cartella di lavoro. Le cartelle di lavoro già esistenti non vengono modificate.
