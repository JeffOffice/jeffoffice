# Digitare nelle celle
<!-- keywords: digitare, digitazione, immettere, inserire, modificare, cambiare, cella, voce, barra della formula, F2, doppio clic, Tab, Esc, annullare, annulla, ripeti, eliminare, cancellare, percentuale, valuta, denaro, euro, data, ora, numero, testo, apostrofo, formula, uguale, sostituire, limite, caratteri -->

Per inserire un contenuto in una cella, selezionala e inizia a digitare. Il testo digitato sostituisce il contenuto della cella. Premi **INVIO** per confermarlo oppure **ESC** per rinunciare e lasciare la cella com'era.

## Tre modi per iniziare

- **Digitare sopra la cella.** Seleziona una cella e digita. Il primo carattere digitato avvia l'immissione e la cella mostra il testo man mano che lo digiti.
- **Modificare il contenuto della cella.** Premi **F2** oppure fai doppio clic sulla cella. La cella si apre con il suo contenuto e il cursore si trova alla fine. Usa i tasti di direzione per spostarti nel testo.
- **Usare la barra della formula.** Fai clic nella barra della formula, accanto alla casella Nome, e digita lì. Anche la cella mostra il testo digitato.

Quando digiti sopra una cella, un tasto di direzione completa l'immissione e sposta la selezione. Quando modifichi il contenuto della cella, i tasti di direzione si spostano nel testo. Premi **F2** per passare da una modalità all'altra.

## Completare l'immissione

| Tasti | Funzione |
| --- | --- |
| INVIO | Conferma l'immissione e sposta in basso (o nella direzione indicata in **File > Opzioni > Modifica**) |
| MAIUSC+INVIO | Conferma l'immissione e sposta nella direzione opposta |
| CTRL+INVIO | Inserisce il contenuto in tutte le celle selezionate, senza spostarsi |
| TAB | Conferma l'immissione e sposta a destra |
| MAIUSC+TAB | Conferma l'immissione e sposta a sinistra |
| Tasti di direzione | Confermano l'immissione e spostano di una cella (solo se hai digitato sopra la cella) |
| ESC | Annulla l'immissione |
| ALT+INVIO | Inizia una nuova riga all'interno della cella |
| CTRL+; | Inserisce la data odierna (nell'immissione, se ne stai digitando una) |
| CTRL+MAIUSC+; | Inserisce l'ora corrente |
| ALT+GIÙ | Elenca le parole già presenti nella colonna per sceglierne una |

**CTRL+INVIO** con più celle selezionate inserisce il testo digitato in tutte. Una formula viene scritta per la cella in cui hai digitato e, in ciascuna delle altre, si comporta come una sua copia: digitando **=A1** con il blocco E6:F7 selezionato, in F7 si ottiene **=B2**. L'operazione corrisponde a un solo passaggio di Annulla.

**ALT+GIÙ** su una cella elenca, in ordine e una sola volta ciascuna, le parole presenti nelle celle sopra e sotto di essa non separate da una cella vuota. Scegliendone una, viene digitata. Una cella con un proprio elenco mostra invece quell'elenco.

In **File > Opzioni > Modifica** puoi scegliere se **INVIO** sposta la selezione in basso, a destra, in alto, a sinistra o se non la sposta.

Se digiti in più celle di una riga usando **TAB** e poi premi **INVIO**, la selezione passa alla riga successiva, nella colonna da cui eri partito. Se prima selezioni un blocco di celle, **INVIO** e **TAB** si spostano all'interno del blocco.

## Come viene interpretato il testo digitato

JeffCrunch interpreta il testo digitato in base alle impostazioni del computer. Il separatore decimale, il separatore delle migliaia, il simbolo di valuta e l'ordine di giorno e mese nelle date dipendono da queste impostazioni. A un numero digitato in una cella senza un formato proprio viene assegnato il formato che lo mostra così come è stato digitato.

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

Una cella che ha già un formato proprio, ad esempio percentuale o data, lo mantiene. Una cella con formato **Testo** memorizza come testo tutto ciò che digiti, tranne le formule.

Per mantenere come testo qualcosa che sembra un numero, ad esempio il codice **007**, digita prima un apostrofo: **'007**. L'apostrofo non fa parte del testo, ma lo rivedi quando modifichi la cella.

## Formule

Una formula inizia con il segno di uguale. Ad esempio, **=B2*2** raddoppia il numero in B2. Puoi digitare i nomi delle funzioni in maiuscolo o in minuscolo: JeffCrunch li scrive in maiuscolo.

I nomi delle funzioni sono sempre quelli inglesi, in tutte le lingue: digita **SUM**, non il nome italiano. I separatori seguono invece le impostazioni del computer. Dove il separatore decimale è la virgola, come in italiano, separa gli argomenti di una funzione con il punto e virgola e usa la virgola nei numeri: **=SUM(A1;A2)*0,5**. La barra della formula e **F2** mostrano le formule nello stesso modo. Dove il separatore decimale è il punto, separa gli argomenti con la virgola: **=SUM(A1,A2)*0.5**. Il file conserva sempre la forma che tutti i programmi sanno leggere, con le virgole tra gli argomenti e i punti nei numeri, così si apre allo stesso modo ovunque.

Se una formula contiene un errore, ad esempio una parentesi mai chiusa, JeffCrunch indica il problema e l'immissione resta come l'hai digitata, con il cursore vicino all'errore, così puoi correggerla. Non viene modificato nulla finché la formula non è corretta.

Una cella può contenere fino a 32.767 caratteri e una formula fino a 8.192. JeffCrunch avvisa se un'immissione è più lunga.

## Cancellare le celle

Seleziona le celle e premi **CANC**. Il contenuto delle celle viene cancellato, ma i formati restano, e l'operazione è un unico passaggio annullabile. JeffCrunch cancella fino a 100.000 celle alla volta.

## Annulla e Ripeti

**{key:Undo}** annulla l'ultima modifica e **{key:Redo}** la ripete. Questi comandi si trovano anche nella barra di accesso rapido in alto. Ogni cella in cui digiti è un passaggio, così come ogni cancellazione. Annulla ripristina la cella com'era: valore, formula, formato e formule che dipendono da essa. JeffCrunch ricorda fino a 1.000 passaggi, meno se hanno modificato moltissime celle, per non occupare troppa memoria.

## Mentre JeffCrunch calcola le formule

Quando modifichi una cella, JeffCrunch calcola in background le formule che dipendono da essa. La cella modificata mostra subito il nuovo contenuto e le altre celle si aggiornano poco dopo; nel frattempo la barra di stato indica **Calcolo**. Puoi continuare a digitare.

## Cosa non è possibile modificare

- Una cella che fa parte di una **formula matrice** o di una tabella dati non può essere modificata singolarmente.
- In un **foglio protetto**, una cella bloccata non può essere modificata. Tutte le celle sono bloccate, a meno che la cartella di lavoro non indichi il contrario. JeffCrunch ti avvisa quando provi a modificarle. Per modificare la cella, scegli Rimuovi protezione foglio nella scheda Revisione.
- La prima riga di una **tabella** contiene i nomi delle colonne. Digita un nuovo nome in una di queste celle per rinominare la colonna: le formule che usano il vecchio nome si aggiornano e la tabella nel file resta valida. A un nome già usato da un'altra colonna viene aggiunto un numero, un nome non può essere vuoto e **CANC** non cancella quella riga.
- Una cartella di lavoro che non è stato possibile leggere per intero può essere visualizzata, ma non modificata né salvata.
- Una funzione non ancora disponibile in JeffCrunch lascia una nuova formula senza valore nella cella; la cartella di lavoro chiede al programma che la aprirà la volta successiva di calcolarla.
- Formattazione, ordinamento e trova e sostituisci saranno disponibili in una versione successiva. Inserimento ed eliminazione, taglia e incolla, riempimento e dimensioni di righe e colonne sono descritti in [Inserire ed eliminare righe, colonne e celle](insert-delete), [Tagliare, copiare e incollare](clipboard), [Riempire le celle e sommarle](fill-sum) e [Modificare la larghezza delle colonne e l'altezza delle righe](sizes).
