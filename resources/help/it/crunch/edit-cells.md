# Digitare nelle celle
<!-- keywords: digitare, digitazione, immettere, inserire, modificare, cambiare, cella, voce, barra della formula, F2, doppio clic, TAB, ESC, annullare, ripetere, eliminare, cancellare, percentuale, valuta, denaro, data, ora, numero, testo, apostrofo, formula, uguale, sostituire, limite, caratteri -->

Per inserire un contenuto in una cella, selezionala e inizia a digitare. Il testo digitato sostituisce il contenuto precedente della cella. Premi **INVIO** per confermarlo oppure **ESC** per rinunciare e lasciare la cella com'era.

## Tre modi per iniziare

- **Digitare sopra la cella.** Seleziona una cella e digita. Il primo carattere digitato avvia l'immissione e la cella mostra il testo man mano che digiti.
- **Modificare il contenuto della cella.** Premi **F2** oppure fai doppio clic sulla cella. La cella si apre con il suo contenuto e il cursore alla fine. Usa i tasti di direzione per spostarti nel testo.
- **Usare la barra della formula.** Fai clic nella barra della formula, accanto alla casella Nome, e digita lì. Anche la cella mostra ciò che digiti.

Quando digiti sopra una cella, un tasto di direzione conclude l'immissione e sposta la selezione. Quando modifichi il contenuto della cella, i tasti di direzione si spostano nel testo. Premi **F2** per passare da una modalità all'altra.

## Concludere l'immissione

| Tasti | Funzione |
| --- | --- |
| INVIO | Conferma l'immissione e sposta in basso |
| MAIUSC+INVIO | Conferma l'immissione e sposta in alto |
| TAB | Conferma l'immissione e sposta a destra |
| MAIUSC+TAB | Conferma l'immissione e sposta a sinistra |
| Tasti di direzione | Conferma l'immissione e sposta di una cella (solo se hai digitato sopra la cella) |
| ESC | Annulla l'immissione |
| ALT+INVIO | Inizia una nuova riga all'interno della cella |

Se digiti in più celle lungo una riga usando **TAB** e poi premi **INVIO**, la selezione scende alla riga successiva, nella colonna da cui eri partito. Se selezioni prima un blocco di celle, **INVIO** e **TAB** si spostano all'interno del blocco.

## Come viene interpretato ciò che digiti

JeffCrunch interpreta ciò che digiti in base alle impostazioni del computer. Il separatore decimale, il separatore delle migliaia, il simbolo di valuta e l'ordine di giorno e mese nelle date dipendono da queste impostazioni. A un numero digitato in una cella senza un formato proprio viene assegnato il formato che lo mostra così come è stato digitato.

| Digiti | La cella contiene | E mostra |
| --- | --- | --- |
| 1234,5 | il numero 1234,5 | 1234,5 |
| 5% | il numero 0,05 | 5% |
| 12,50 € | il numero 12,5 | 12,50 € |
| 4/3/2025 | la data, come numero di giorni | 4/3/2025 |
| 22:30 | l'ora, come frazione di giorno | 22:30 |
| VERO | il valore logico vero | VERO |
| #N/D | l'errore #N/D | #N/D |
| =B2*2 | una formula | il risultato della formula |
| qualsiasi altra cosa | testo | il testo |

Una cella che ha già un formato proprio, ad esempio percentuale o data, lo mantiene. Una cella con formato **Testo** conserva come testo tutto ciò che digiti, tranne le formule.

Per conservare come testo qualcosa che sembra un numero, ad esempio un codice **007**, digita prima un apostrofo: **'007**. L'apostrofo non fa parte del testo, ma lo rivedi quando modifichi la cella.

## Formule

Una formula inizia con un segno di uguale. Ad esempio, **=B2*2** raddoppia il numero in B2. Puoi digitare i nomi delle funzioni in maiuscolo o minuscolo: JeffCrunch li scrive in maiuscolo.

I nomi delle funzioni sono sempre quelli inglesi, in qualsiasi lingua: digita **SUM**, non il nome italiano. I separatori seguono le impostazioni del computer. Dove il separatore decimale è la virgola, come in italiano, separi gli argomenti di una funzione con il punto e virgola e scrivi i numeri con la virgola: **=SUM(A1;A2)*0,5**. La barra della formula e **F2** mostrano le formule nello stesso modo. Dove il separatore decimale è il punto, separi gli argomenti con la virgola: **=SUM(A1,A2)*0.5**. Il file conserva sempre la forma leggibile da qualsiasi programma, con virgole tra gli argomenti e punti nei numeri, quindi si apre allo stesso modo ovunque.

Se una formula contiene un errore, ad esempio una parentesi mai chiusa, JeffCrunch indica il problema e l'immissione rimane come l'hai digitata, con il cursore vicino all'errore, così puoi correggerlo. Non viene modificato nulla finché la formula non è corretta.

Una cella può contenere fino a 32.767 caratteri e una formula fino a 8.192. JeffCrunch ti avvisa quando un'immissione è più lunga.

## Cancellare le celle

Seleziona le celle e premi **CANC**. Il contenuto delle celle viene cancellato ma i formati vengono mantenuti, e l'operazione è un unico passaggio annullabile. JeffCrunch cancella fino a 100.000 celle alla volta.

## Annulla e Ripeti

**{key:Undo}** annulla l'ultima modifica e **{key:Redo}** la ripete. Questi comandi si trovano anche nella barra di accesso rapido in alto. Ogni cella in cui digiti è un passaggio, così come ogni cancellazione. Annulla ripristina la cella com'era: valore, formula, formato e formule che dipendono da essa. JeffCrunch ricorda fino a 1.000 passaggi, meno se hanno modificato moltissime celle, in modo da non occupare mai troppa memoria.

## Mentre JeffCrunch calcola le formule

Quando modifichi una cella, JeffCrunch calcola in background le formule che dipendono da essa. La cella modificata mostra subito il nuovo contenuto e le altre celle si aggiornano poco dopo; fino ad allora la barra di stato mostra **Calcolo in corso**. Nel frattempo puoi continuare a digitare.

## Cosa non è possibile modificare

- Una cella che fa parte di una **formula di matrice** o di una tabella dati non può essere modificata da sola.
- In un **foglio protetto**, una cella bloccata non può essere modificata. Tutte le celle sono bloccate, a meno che la cartella di lavoro non indichi diversamente. JeffCrunch ti avvisa quando ci provi. Per modificare la cella, scegli Rimuovi protezione foglio nella scheda Revisione.
- La prima riga di una **tabella** contiene i nomi delle colonne. Digita un nuovo nome in una di queste celle per rinominare la colonna: le formule che usano il vecchio nome si aggiornano e la tabella nel file rimane valida. A un nome già usato da un'altra colonna viene aggiunto un numero, un nome non può essere vuoto e **CANC** non cancella quella riga.
- Una cartella di lavoro che non è stato possibile leggere per intero può essere visualizzata, ma non modificata né salvata.
- Una funzione non ancora disponibile in JeffCrunch lascia una nuova formula senza valore nella cella; la cartella di lavoro chiede al programma che la aprirà successivamente di calcolare la formula.
- Formattazione, ordinamento e trova e sostituisci saranno disponibili in una versione successiva. Inserimento ed eliminazione, taglia e incolla, riempimento e dimensioni di righe e colonne sono descritti in [Inserire ed eliminare righe, colonne e celle](insert-delete), [Tagliare, copiare e incollare](clipboard), [Riempire le celle e sommarle](fill-sum) e [Modificare la larghezza delle colonne e l'altezza delle righe](sizes).
