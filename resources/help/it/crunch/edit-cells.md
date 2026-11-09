# Inserire il testo nelle celle
<!-- keywords: correzione automatica, virgolette tipografiche, trattino, frazione, collegamento, indirizzo, parola in grassetto, parte di una cella, testo con formattazione, formattare parte del testo, colorare una parola, esecuzioni, digitare, digitazione, invio, modificare, modifica, cambiare, cella, voce, barra della formula, F2, doppio clic, Tab, Esc, annullare, annulla, ripeti, eliminare, cancellare, percentuale, valuta, denaro, data, ora, numero, testo, apostrofo, formula, uguale, sostituire, limite, caratteri, scrittura, inserimento, immettere -->

Per inserire un contenuto in una cella, seleziona la cella e inizia a digitare. Ciò che digiti sostituisce il contenuto della cella. Premi **Invio** per confermare, oppure **Esc** per annullare l'inserimento e lasciare la cella com'era.

## Tre modi per iniziare

- **Digita direttamente nella cella.** Seleziona una cella e digita. Il primo carattere avvia l'inserimento e la cella mostra ciò che digiti man mano.
- **Modifica il contenuto della cella.** Premi **F2** oppure fai doppio clic sulla cella. La cella si apre con il suo contenuto e il cursore si trova alla fine. Usa i tasti freccia per spostarti nel testo.
- **Usa la barra della formula.** Fai clic nella barra della formula, accanto alla casella Nome, e digita lì. Anche la cella mostra ciò che digiti.

Quando digiti direttamente nella cella, un tasto freccia conclude l'inserimento e sposta la selezione. Quando modifichi il contenuto della cella, i tasti freccia si muovono nel testo. Premi **F2** per passare dall'una all'altra modalità.

## Concludere l'inserimento

| Tasti | Cosa fanno |
| --- | --- |
| Invio | Conferma l'inserimento e scende alla cella successiva (oppure nella direzione indicata in **File > Opzioni > Modifica**) |
| Maiusc+Invio | Conferma l'inserimento e torna alla cella precedente |
| Ctrl+Invio | Inserisce ciò che hai digitato in tutte le celle selezionate, e resti dove sei |
| Tab | Conferma l'inserimento e va a destra |
| Maiusc+Tab | Conferma l'inserimento e va a sinistra |
| Tasti freccia | Conferma l'inserimento e sposta la selezione di una cella (solo quando hai digitato direttamente nella cella) |
| Esc | Annulla l'inserimento |
| Alt+Invio | Va a capo all'interno della cella |
| Ctrl+; | Inserisce la data odierna (nell'inserimento, se stai digitandone uno) |
| Ctrl+Maiusc+; | Inserisce l'ora attuale |
| Alt+Freccia giù | Elenca le parole già presenti nella colonna e ne permette la scelta |

**Ctrl+Invio** con più celle selezionate inserisce ciò che hai digitato in tutte. Una formula è scritta per la cella in cui hai digitato, e nelle altre viene riportata come farebbe una copia: digitando **=A1** con l'intervallo E6:F7 selezionato si ottiene **=B2** in F7. È un unico passo di Annulla.

**Alt+Freccia giù** su una cella elenca, nell'ordine e una sola volta ciascuna, le parole contenute nelle celle sopra e sotto di essa, purché non siano separate da una cella vuota. Scegliendone una, la si digita. Se una cella ha un elenco proprio, mostra quello.

In **File > Opzioni > Modifica** puoi scegliere se **Invio** sposta verso il basso, a destra, verso l'alto o a sinistra, oppure non sposta affatto.

Se inserisci dati in più celle di una riga con **Tab** e poi premi **Invio**, la selezione scende alla riga successiva, nella colonna da cui sei partito. Se prima selezioni un intervallo di celle, **Invio** e **Tab** si spostano all'interno dell'intervallo.

## Correzione automatica durante la digitazione

Quando digiti uno spazio o un segno di punteggiatura, come il punto, dopo una parola, JeffCrunch può correggere la parola non appena la completi. Corregge i più comuni errori di battitura (come "teh" al posto di "the"), due lettere maiuscole all'inizio di una parola, una lettera minuscola che inizia una frase o un giorno della settimana, e le virgolette dritte (che diventano curve). Inoltre trasforma due trattini tra parole in un trattino lungo, e 1/2, 1/4 e 3/4 in ½, ¼ e ¾ quando l'inserimento contiene delle parole. Un numero come 1/2 digitato da solo resta una data o una frazione, come sempre.

Ogni correzione è un passo a sé. Premi **Annulla** subito dopo una correzione per annullarla soltanto e conservare ciò che hai digitato. Le formule non vengono mai modificate: un inserimento che inizia con il segno di uguale, oppure con un segno più o meno seguito da una formula, resta com'è stato digitato. Un indirizzo web (come https://example.org) o un indirizzo di posta elettronica che costituisce l'intero inserimento diventa un collegamento nella sua cella, al termine dell'inserimento, se è attiva l'opzione **Trasforma gli indirizzi web in collegamenti ipertestuali durante la digitazione**.

Puoi attivare o disattivare queste opzioni in **File > Opzioni > Strumenti di correzione**. Una parola errata viene segnalata mentre digiti: vedi [Controllo ortografia](notes-links).

## Formattare una parte del testo

Alcune parole di una cella possono avere un aspetto diverso dal resto. Ad esempio, una parola può essere in grassetto o in rosso.

1. Premi F2 per modificare la cella (oppure fai doppio clic su di essa).
2. Seleziona le parole da cambiare, con Maiusc e i tasti freccia oppure con il mouse.
3. Usa **Grassetto**, **Corsivo**, **Sottolineatura**, **Barrato**, **Carattere**, **Dimensioni carattere** o **Colore carattere** nella scheda **Home**, oppure Ctrl+B, Ctrl+I e Ctrl+U.
4. Premi Invio.

Cambiano solo le parole selezionate. Il resto della cella mantiene l'aspetto proprio della cella. Mentre modifichi, la casella di inserimento mostra le parole con un unico aspetto, e quando premi Invio la cella mostra ogni parte con il proprio aspetto. Se in seguito digiti o elimini parole, la formattazione si sposta insieme alle parole. Scegliere un carattere o una dimensione senza aver selezionato parole cambia l'intera cella, come sempre.

Le celle che contengono una formattazione interna al testo, proveniente da un altro programma, la mostrano, la conservano e la salvano di nuovo. Una cella modificata mantiene la sua formattazione. La formattazione interna a un testo si applica al testo, non ai numeri o alle formule. Una copia salvata come file .xls meno recente o come .ods conserva le parole, e il salvataggio ti avvisa che la formattazione interna non viene mantenuta.

## Ciò che digiti diventa

JeffCrunch interpreta ciò che digiti in base alle impostazioni del computer. Il separatore decimale, il separatore delle migliaia, il simbolo della valuta e l'ordine di giorno e mese in una data derivano da tali impostazioni. Un numero digitato in una cella che non ha un formato proprio riceve il formato che lo mostra come l'hai digitato.

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

Una cella che ha già un formato proprio, come una percentuale o una data, lo conserva. Una cella il cui formato è **Testo** conserva come testo tutto ciò che digiti, tranne una formula.

Per conservare come testo un contenuto che sembra un numero, come un codice **007**, digita prima un apostrofo: **'007**. L'apostrofo non fa parte del testo. Quando modifichi la cella, lo vedi di nuovo.

## Formule

Una formula inizia con il segno di uguale. Ad esempio, **=B2*2** raddoppia il numero in B2. I nomi delle funzioni possono essere digitati in maiuscolo o in minuscolo; JeffCrunch li scrive in maiuscolo.

I nomi delle funzioni sono sempre quelli inglesi, in ogni lingua: digita **SUM**, non il nome che la tua lingua gli dà. I segni di separazione seguono le impostazioni del computer. Dove il separatore decimale è la virgola, come in italiano, separi gli argomenti di una funzione con il punto e virgola e scrivi la virgola nei numeri: **=SOMMA(A1;A2)*0,5**. La barra della formula e **F2** mostrano le formule nello stesso modo. Dove il separatore decimale è il punto, gli argomenti si separano con la virgola: **=SUM(A1,A2)*0.5**. Il file conserva sempre la forma che tutti i programmi leggono, con le virgole tra gli argomenti e i punti nei numeri, così si apre allo stesso modo ovunque.

Se una formula contiene un errore, ad esempio una parentesi che non viene mai chiusa, JeffCrunch ti dice che cosa non va. L'inserimento resta come lo hai digitato, con il cursore vicino all'errore, così puoi correggerlo. Non cambia nulla finché la formula non è corretta.

Una cella contiene fino a 32.767 caratteri, e una formula fino a 8.192. JeffCrunch ti avvisa quando un inserimento è più lungo.

## Cancellare le celle

Seleziona le celle e premi **Canc**. Questo cancella ciò che le celle contengono e mantiene i loro formati, ed è un passo che puoi annullare. JeffCrunch cancella fino a 100.000 celle alla volta.

Nella scheda **Home**, il pulsante **Cancella** ha un menu. **Cancella tutto** svuota le celle, i loro formati, le note, i commenti (le discussioni con risposte) e i collegamenti. Una discussione con più di 500 commenti viene conservata, e JeffCrunch lo segnala, perché Annulla non potrebbe ripristinare i commenti che non ha letto. **Cancella formati** mantiene ciò che le celle contengono e ripristina il formato normale. **Cancella contenuto** equivale a **Canc**. **Cancella commenti e note** rimuove soltanto le note e i commenti dalle celle selezionate. **Cancella collegamenti ipertestuali** rimuove soltanto i collegamenti, e le celle mantengono il loro testo. Una scelta è disattivata quando nelle celle selezionate non c'è nulla da cancellare.

## Annulla e Ripeti

**Annulla** annulla l'ultima modifica, e **Ripeti** la ripristina. I comandi sono anche nella barra degli strumenti rapida in alto. Ogni cella in cui digiti è un passo, e così ogni cancellazione. Annulla riporta la cella com'era: il suo valore, la sua formula, il suo formato e le formule che ne dipendono. JeffCrunch ricorda fino a 1.000 passi, meno quando ne sono stati modificati moltissimi, così da non occupare mai molta memoria.

## Mentre JeffCrunch calcola le formule

Quando cambi una cella, JeffCrunch calcola in background le formule che ne dipendono. La cella modificata mostra subito il suo nuovo contenuto, e le altre celle si aggiornano poco dopo; la barra di stato mostra **Calcolo in corso** finché non hanno finito. Nel frattempo puoi continuare a digitare.

## Ciò che non si può modificare

- Una cella che fa parte di una **formula matriciale** o di una tabella dati non può essere modificata da sola.
- In un **foglio protetto**, una cella bloccata non può essere modificata. Tutte le celle sono bloccate, a meno che la cartella di lavoro non indichi il contrario. JeffCrunch ti avvisa quando tenti la modifica. Per modificare la cella, scegli **Rimuovi protezione foglio** nella scheda **Revisione**.
- La prima riga di una **tabella** contiene i nomi delle sue colonne. Digita un nuovo nome in una di queste celle per rinominare la colonna: le formule che usano il vecchio nome si adeguano, e la tabella nel file resta valida. Un nome già usato da un'altra colonna riceve un numero alla fine, un nome non può essere vuoto, e **Canc** non cancella quella riga.
- Una cartella di lavoro che non è stata letta per intero può essere consultata, ma non modificata né salvata.
- Una funzione che JeffCrunch non conosce ancora lascia nella cella una nuova formula senza valore; la cartella di lavoro chiede al programma che la apre di calcolare la formula.
- La formattazione, l'ordinamento e trova e sostituisci saranno disponibili in una versione successiva. Inserimento ed eliminazione, taglia e incolla, riempimento e dimensioni di righe e colonne sono descritti in [Inserire ed eliminare righe, colonne e celle](insert-delete), [Taglia, copia e incolla](clipboard), [Riempire le celle e sommarle](fill-sum) e [Modificare la larghezza delle colonne e l'altezza delle righe](sizes).

## Nuove cartelle di lavoro

In **File > Opzioni > Nuove cartelle di lavoro** puoi scegliere il carattere, la dimensione del carattere e il numero di fogli con cui inizia ogni nuova cartella di lavoro. Le cartelle di lavoro già esistenti non vengono modificate.
